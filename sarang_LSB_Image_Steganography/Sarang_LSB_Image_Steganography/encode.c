#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp( FILE *fptr_image )
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    // Print_sleep("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    // Print_sleep("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

 
/* Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo -> fptr_src_image = fopen( encInfo -> src_image_fname, "rb");
    // Do Error handling
    if (encInfo -> fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo -> fptr_secret = fopen(encInfo -> secret_fname, "rb");
    // Do Error handling
    if (encInfo -> fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo -> fptr_stego_image = fopen(encInfo -> stego_image_fname, "wb");
    // Do Error handling
    if (encInfo -> fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Checks the Operation mode and returns corresponding value */
OperationType check_operation_type( char *argv[] )
{
    // Check for command line argument
    if( argv[1] == NULL )
        return e_unsupported;
    
    else if( strcmp( argv[1], "-e") == 0 )
        return e_encode;

    else if( strcmp( argv[1], "-d") == 0 )
        return e_decode;

    else
        return e_unsupported;

}

/* Validates the CLA and reads the file names */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //   0           1   2   3     4 
    // ./lsb_steg   -e .bmp .txt [.bmp]

    // Check for .bmp
    if( strstr( argv[2], ".bmp" ) == NULL )
    {
        return e_failure;  // Not .bmp file
    }
    encInfo -> src_image_fname = argv[2]; // Saves .bmp file name

    // Check for . extensions
    char* ext = strrchr( argv[3], '.' );
    if( ext == NULL )
    {
        return e_failure;  // No extension found
    }
    encInfo -> secret_fname = argv[3]; // Saves the file name of any extension
    strcpy( encInfo -> extn_secret_file, ext ); // Saves any extension

    // Check if 4th argument exists
    if( argv[4] != NULL )
    {
        if( strstr(argv[4], ".bmp" ) == NULL ) // Not a bmp file
        {
            return e_failure;
        }
        encInfo -> stego_image_fname = argv[4]; // Creates stego image with input name

        return e_success;  
    }

    else
    {
        print_sleep("INFO: Output File not mentioned. Creating stego_img.bmp as default\n");
        encInfo -> stego_image_fname = "stego_img.bmp"; // Creates stego image with default name
        return e_success;
    }
}

/* Checks if the source .bmp file has enough capacity to store data to be encoded*/
Status check_capacity( EncodeInfo *encInfo )
{
    // Store the image capacity
    uint img_size = get_image_size_for_bmp( encInfo -> fptr_src_image );
    encInfo -> image_capacity = img_size;

    // Get secret file size
    uint file_size = get_file_size( encInfo -> fptr_secret );
    
    if( file_size == 0 )
    {
        print_sleep("INFO: Empty Secret String\n");
        exit(1);
    }
    encInfo -> size_secret_file = file_size; // Store secret string size
    print_sleep("INFO: Done. Not empty\n");


    // Get secret file extension size
    char *extn_ptr = strrchr( encInfo -> secret_fname, '.' );
    if( extn_ptr == NULL )
    {
        print_sleep("INFO: Empty Secret Extension\n");
        exit(1);
    }
    encInfo -> size_extn_file = strlen( extn_ptr );

    // Checks if total encoding size required is less than source file size without header size
    print_sleep("INFO: Checking for %s capacity to handle %s\n", encInfo -> src_image_fname, encInfo -> secret_fname );
    if( ( 18 + strlen( extn_ptr ) + file_size ) * 8 > ( img_size - 54 ) )  // 2 MS + 4 extndata + 4 secretdata ( 10 )
        return e_failure;

    return e_success;

}

/* Gets the secret file size */
uint get_file_size( FILE *fptr )
{
    fseek( fptr, 0, SEEK_END );
    uint size = ftell( fptr );
    fseek( fptr, 0, SEEK_SET );
    return size;
}

/* Copies the .bmp header to stego file as it is */
Status copy_bmp_header( FILE *fptr_src_image, FILE *fptr_dest_image )
{
    char buffer[55];

    // Reset file pointers
    fseek( fptr_src_image, 0, SEEK_SET );
    fseek( fptr_dest_image, 0, SEEK_SET );

    fread( buffer, 54, 1, fptr_src_image );   // Reads 54 bytes from source
    fwrite( buffer, 54, 1, fptr_dest_image ); // Writes 54 bytes to stego

    return e_success;
}

/* Reads 8 bytes from source to encode 1 byte of secret file data */
Status encode_data_to_image( const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image )
{
    char image_buffer[8] = {};
    
    for( int i = 0; i < size; i++ )
    {
        fread( image_buffer, 8, 1, fptr_src_image );    // Read 8 bytes
        encode_byte_to_lsb( data[i], image_buffer );    // Steg 1 byte to 8 byte
        fwrite( image_buffer, 8, 1, fptr_stego_image ); // Write 8 byte after encoding
    }

    return e_success;
}

/* LSB of 8 bytes read will be encoded with 1 byte of secret file data */
Status encode_byte_to_lsb( char data, char *image_buffer )
{
    for( int i = 0; i < 8; i++ )
    {
        image_buffer[i] = ( ( data >> i ) & 1 ) | ( image_buffer[i] & ( ~1 ) ); // Encodes the 1byte( 8 bit ) to LSB of 8 bytes
    }

    return e_success;
}

/* Encodes the magic string */
Status encode_magic_string( const char *magic_string, EncodeInfo *encInfo )
{
    int len = strlen( magic_string );

    encode_data_to_image( magic_string, len, encInfo -> fptr_src_image, encInfo -> fptr_stego_image );

    return e_success;
}

/* Encodes the size of secret file extension, which will be an integer */
Status encode_secret_file_extn_size( long size_extn_file, EncodeInfo *encInfo )
{
    uchar* extn_size_len = ( uchar* )&size_extn_file; // Character pointer to access each byte to encode

    encode_data_to_image( extn_size_len, sizeof( long ), encInfo -> fptr_src_image, encInfo -> fptr_stego_image );

    return e_success;
}

/* Encodes the secret file extension( .txt ) */
Status encode_secret_file_extn( const char *file_extn, EncodeInfo *encInfo )
{
    int extn_len = strlen( encInfo -> extn_secret_file );
    
    encode_data_to_image( file_extn, extn_len, encInfo -> fptr_src_image, encInfo -> fptr_stego_image );
    
    return e_success;
 
}

/* Encodes the secret file size, which will be an integer */
Status encode_secret_file_size( long file_size, EncodeInfo *encInfo )
{
    uchar* file_size_len = ( uchar* )&file_size; // Character pointer allowing each byte to be accessed and encoded, here all 8 bytes.

    encode_data_to_image( file_size_len, sizeof( long ), encInfo -> fptr_src_image, encInfo -> fptr_stego_image );

    return e_success;
}

/* Encodes the secret file data in a chunk */
Status encode_secret_file_data( EncodeInfo *encInfo )
{
    char secret_buff[1024]; // Buffer to encode a chunk of data
    size_t read_bytes;
    
    //set the pointer of secret file to start
    fseek( encInfo -> fptr_secret , 0, SEEK_SET );

    // Reads chunk of data from secret file
    while( ( read_bytes = fread( secret_buff, 1, sizeof( secret_buff ), encInfo -> fptr_secret ) ) > 0 )
    {
        encode_data_to_image( secret_buff, read_bytes, encInfo -> fptr_src_image, encInfo -> fptr_stego_image );
    }

    return e_success;
}

/* Copies the reamining data from source after completing encode to stego file */
Status copy_remaining_img_data( FILE* fptr_src, FILE* fptr_dest )
{
    char data_buff[1024]; // Buffer to copy a chunk of data
    size_t read_bytes;

    while( ( read_bytes = fread( data_buff, 1, sizeof( data_buff ), fptr_src )) > 0 ) // Reads chunk of data to copy
    {
        fwrite( data_buff, 1, read_bytes, fptr_dest ); // Writes the read data byte by byte
    }

    return e_success;
}

/* To do the encoding process and calls each required function
 * Displays required informations and success, failure messages
 * Closes all the opened files
 */
Status do_encoding( EncodeInfo *encInfo )
{
    print_sleep("INFO: Opening required files\n");
    if( open_files( encInfo ) == e_success )
    {
        print_sleep("INFO: Opened %s\n", encInfo -> src_image_fname );
        print_sleep("INFO: Opened %s\n", encInfo -> secret_fname );
        print_sleep("INFO: Opened %s\n", encInfo -> stego_image_fname );      
    }
    else
        exit(1);

    print_sleep("INFO: Done\n");

    print_sleep("INFO: ## Encoding Procedure Started ##\n");

    print_sleep("INFO: Checking for %s size\n", encInfo -> secret_fname );
    if( check_capacity( encInfo ) == e_success )
    {
        print_sleep("INFO: Done. Found OK\n");
    }
    else
    {
        print_sleep("INFO: %s cannot handle the %s\n", encInfo -> src_image_fname, encInfo -> secret_fname );
        exit(0);
    }

    // Start encoding
    // Copying header to stego
    print_sleep("INFO: Copying Image Header\n");
    if( copy_bmp_header( encInfo -> fptr_src_image, encInfo -> fptr_stego_image ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying image header\n");
        exit(1);
    }


    // Encoding magic string
    print_sleep("INFO: Encoding Magic String Signature\n");
    if( encode_magic_string( "#*", encInfo ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying magic string\n");
        exit(1);
    }
    

    // Encoding secret file extension size
    print_sleep("INFO: Encoding %s File Extenstion Size\n", encInfo -> secret_fname );

    if( encode_secret_file_extn_size( encInfo -> size_extn_file, encInfo ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying secret file extension size\n");
        exit(1);
    }


    // Encoding secret file extension
    print_sleep("INFO: Encoding %s File Extenstion\n", encInfo -> secret_fname );

    if( encode_secret_file_extn( encInfo -> extn_secret_file, encInfo ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying secret file extension\n");
        exit(1);
    }


    // Encoding Secret file size
    print_sleep("INFO: Encoding %s File Size\n", encInfo -> secret_fname );
    if( encode_secret_file_size( encInfo -> size_secret_file, encInfo ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying secret file size\n");
        exit(1);
    }


    // Encode secret file data
    print_sleep("INFO: Encoding %s File Data\n", encInfo -> secret_fname );
    if( encode_secret_file_data( encInfo ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying secret file data\n");
        exit(1);
    }


    // Copy remaining data to stego file 
    print_sleep("INFO: Copying Left Over Data\n");
    if( copy_remaining_img_data( encInfo -> fptr_src_image, encInfo -> fptr_stego_image ) == e_success )
    {
        print_sleep("INFO: Done\n");
    }
    else
    {
        print_sleep("INFO: Error copying secret file data\n");
        exit(1);
    }

    print_sleep("INFO: ## Encoding Done Successfully ##\n");

    // Close all the files
    fclose( encInfo -> fptr_src_image );
    fclose( encInfo -> fptr_secret );
    fclose( encInfo -> fptr_stego_image );

    return e_success;
}