#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "decode.h"
#include "common.h"

/* Function Definitions */

/* Reads and validates the encoded bmp image 
 * bmp image name from command line input is verified and read into
 * structure.
 * Returns d_success upon verifying .bmp file else returns d_failure
 */
Status read_and_validate_decode_bmp( char* argv[], DecodeInfo *decInfo )
{
    // 0    1   2    3
    //argv -d .bmp [.txt]

    if( strstr( argv[2], ".bmp") == NULL )
    {
        return d_failure;
    }
    decInfo -> stego_image_fname = argv[2];
    
    return d_success;
}

/* Checks for output file, then Reads and validates
 * output image name from command line input is verified and read into
 * structure if present
 * If no output file name is given, stores it as NULL
 * Returns d_success upon verifying output file or d_failure
 */
Status read_and_validate_decode_output( char* argv[], DecodeInfo *decInfo )
{
    // Check for output file
    if( argv[3] != NULL )
    {
        char *dot = strrchr(argv[3], '.');  // find last '.'

        if (dot == NULL)
        {
            decInfo -> secret_fname = NULL; // no extension present
            return d_success;
        }
        else
        {
            
            if( strcmp( dot, decInfo -> extn_secret_file ) == 0 )
            {
                decInfo -> secret_fname = argv[3];
                return d_success;
            }
            
            static char output_fname[100];
            strncpy( output_fname, argv[3], dot - argv[3] ); // Copies filename only
            output_fname[ dot - argv[3] ] = '\0';

            strcat( output_fname, decInfo -> extn_secret_file ); // Concatinates filename with decoded extension
            decInfo -> secret_fname = output_fname;
    
            print_sleep("INFO: Output file extension does not match, Creating %s\n", decInfo -> secret_fname );

            return d_success;
        }
        
    }
    decInfo -> secret_fname = NULL; // Secret file not found

    return d_success;
}

/* Get file pointer for the stegged image file 
 * Returns d_success file open,
 * else returns d_failure
 */
Status open_stego( DecodeInfo *decInfo )
{
    // Stego img
    decInfo -> fptr_stego_image = fopen( decInfo -> stego_image_fname, "rb" );

    // Error handling
    if ( decInfo -> fptr_stego_image == NULL )
    {
    	perror( "fopen" );
    	fprintf( stderr, "ERROR: Unable to open file %s\n", decInfo -> stego_image_fname );
    	return d_failure;
    }

    fseek( decInfo -> fptr_stego_image, 54, SEEK_SET ); // Skip the header as no information is encoded in header 
    return d_success; // Opened stego file 
}

/* Get file pointer for the output image file 
 * If no output file name given:
    creates default file with
    decoded extension name.
 * If output file name given: 
    checks if given extension matches,
        if matches, creates and open using given file name
        else takes the output filename and open file with decoded extension
 * Returns d_success if file opened,
 * else returns d_failure
 */
Status open_secret( DecodeInfo *decInfo, char* argv[] )
{
    // Secret file

    if( decInfo -> secret_fname == NULL || decInfo -> secret_fname[0] == '\0' )
    {
        static char def_fname[100];

        if( argv[3] )
        {
            strcpy( def_fname, argv[3] );
            print_sleep("INFO: Output File extension not mentioned. ");
        }
        else
        {
            strcpy( def_fname, "decoded" );
            print_sleep("INFO: Output File not mentioned. ");
        }

        strcat( def_fname, decInfo -> extn_secret_file );

        decInfo -> secret_fname = def_fname;
        print_sleep("Creating %s as default\n", decInfo -> secret_fname );
    }

    // Open Secret file
    decInfo -> fptr_secret = fopen( decInfo -> secret_fname, "wb" );

    // Error handling
    if( decInfo -> fptr_secret == NULL )
    {
        perror("fopen");
    	fprintf( stderr, "ERROR: Unable to open file %s\n", decInfo -> secret_fname );
    	return d_failure;
    }

    return d_success; // Opened secret file
}

/* Reads 8 byte from stego and decodes */
char decode_data_from_image( DecodeInfo *decinfo )
{
    char decode_buff[8];

    fread( decode_buff, 8, 1, decinfo -> fptr_stego_image ); // Read 8 byte from stego image
    
    char ch = decode_byte_from_lsb( decode_buff ); // Decode the 1 byte chara from 8 byte

    return ch;
}

/* Decodes 8 byte in image_buffer to character */
char decode_byte_from_lsb( char *image_buffer )
{
    char ch = 0;

    /* Decode 1 character from 8 byte bit by bit */ 
    for( int i = 0; i < 8; i++ )
    {
        ch = ch | ( image_buffer[i] & 1 ) << i;
    }

    return ch; // Return decoded character
}

/* Decode the magic string to identify if file is stegged */
Status decode_magic_string( DecodeInfo *decInfo )
{
    
    int len = strlen( "#*" );
    char ch;
    
    for( int i = 0; i < len; i++ )
    {
        ch = decode_data_from_image( decInfo );
        if( i == 0 && ch != '#')
        {
            return d_failure;
        }
        if( i == 1 && ch != '*')
        {
            return d_failure;
        }
    }

    return d_success;
}

/* Decodes the secret file extension size */
Status decode_file_extn_size( DecodeInfo *decInfo )
{
    long size = 0;
    char* ch = ( char* )&size;  // Gets each byte to store the size byte by byte

    for( int i = 0; i < sizeof( long ); i++ )
    {
        ch[i] = decode_data_from_image( decInfo );  // Decode each byte of size
    }

    decInfo -> extn_file_size = size;

    return d_success;
}

/* Decode the extension of the encoded secret file to create a file of same extension*/
Status decode_file_extn( DecodeInfo *decInfo )
{
    long size = decInfo -> extn_file_size;

    for( int i = 0; i < size; i++ )
    {
        decInfo -> extn_secret_file[i] = decode_data_from_image( decInfo );
    }
    
    decInfo -> extn_secret_file[size] = '\0'; // NULL terminate the extension name

    return d_success;
}

/* Decode the size of encoded secret file */
Status decode_file_size( DecodeInfo *decInfo )
{
    long size;
    char* ch = (char*)&size; // Gets each byte to store the size byte by byte
    for( int i = 0; i < sizeof( long ); i++ )
    {
        ch[i] = decode_data_from_image( decInfo ); // Decode size byte by byte
    }

    decInfo -> file_size = size;

    return d_success;
}

/* Decode the secret message from bmp file */
Status decode_file_data( DecodeInfo *decInfo )
{
    long size = decInfo -> file_size;

    for ( long i = 0; i < size; i++ )
    {
        char ch = decode_data_from_image ( decInfo ); // Decode 1 byte
        fputc( ch, decInfo -> fptr_secret );          // Write decoded byte
    }
    
    return d_success;

}

Status do_decoding( DecodeInfo *decInfo, char* argv[] )
{
    print_sleep("INFO: ## Decoding Procedure Started ##\n");

    print_sleep("INFO: Opening required files\n");

    // Open stego file
    if( open_stego( decInfo ) == d_success )
    {
        print_sleep("INFO: Opened %s\n", decInfo -> stego_image_fname );
    }
    else
    {
        exit(1);
    }

    print_sleep("INFO: Decoding Magic String Signature\n");

    // Decode magic string
    if( decode_magic_string( decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Magic string not present, Image is not Stegged\n");
        exit(1);
    }

    // Decode file extension size
    print_sleep("INFO: Decoding file extension size from %s\n", decInfo -> stego_image_fname );
    if( decode_file_extn_size( decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }

    else
    {
        print_sleep("INFO: error decoding file extension\n");
        exit(1);
    }

    // Decode file extension
    print_sleep("INFO: Decoding file extension from %s\n", decInfo -> stego_image_fname );
    if( decode_file_extn( decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }

    else
    {
        print_sleep("INFO: error decoding file extension\n");
        exit(1);
    }

    // Check for output file
    print_sleep("INFO: Validating output file name\n");
    if(read_and_validate_decode_output( argv, decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }

    else
    {
        print_sleep("INFO: Error validating output file\n");
        exit(1);
    }


    // Open output file with decoded extension
    if( open_secret( decInfo, argv ) == d_success )
    {
        print_sleep("INFO: Opened %s\n", decInfo -> secret_fname );
        print_sleep("INFO: Done, Opened all required files\n" );
    }
    else
    {
        exit(1);
    }

    // Decode the file size
    print_sleep("INFO: Decoding file size from %s\n", decInfo -> stego_image_fname );
    if( decode_file_size( decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: error decoding file size\n");
        exit(1);
    }

    // Decode the encoded message from bmp file
    print_sleep("INFO: Decoding file data from %s\n", decInfo -> stego_image_fname );
    if( decode_file_data( decInfo ) == d_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: error decoding file data\n");
        exit(1);
    }

    // Successfully did the encoding operation
    print_sleep("INFO: ## Decoding done successfully ##\n");

    // Close all the files
    fclose( decInfo -> fptr_secret );
    fclose( decInfo -> fptr_stego_image );

    return d_success;
}