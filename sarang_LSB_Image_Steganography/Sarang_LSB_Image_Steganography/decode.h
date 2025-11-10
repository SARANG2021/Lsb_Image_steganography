#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h" // Contains user defined types

#define MAX_FILE_SUFFIX 5

/* 
 * Structure to store information required for
 * decoding encoded file to output file
 * Info about decoded and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    uint image_capacity;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[ MAX_FILE_SUFFIX ];
    uint extn_file_size;
    uint file_size;

} DecodeInfo;


/* Decoding function prototypes */

/* Read and validate Decode bmp name from argv */
Status read_and_validate_decode_bmp( char* argv[], DecodeInfo *decInfo );

/* Read and validate Decode output name from argv */
Status read_and_validate_decode_output( char* argv[], DecodeInfo *decInfo );

/* Perform the decoding */
Status do_decoding( DecodeInfo *decInfo, char* argv[] );

/* Decode stego file extension size */
Status decode_file_extn_size( DecodeInfo *decInfo );

/* Decode stego file extension */
Status decode_file_extn( DecodeInfo *decInfo );

/* Get File pointer for i/p ( Stego image ) */
Status open_stego( DecodeInfo *decInfo );

/* Get File pointer for o/p file */
Status open_secret( DecodeInfo *decInfo, char* argv[] );

/* Decode Magic String */
Status decode_magic_string( DecodeInfo *decInfo );

/* Decode stego file size */
Status decode_file_size( DecodeInfo *decInfo );

/* Decode stego file data */
Status decode_file_data( DecodeInfo *decInfo );

/* Decode function, which does real decoding */
char decode_data_from_image( DecodeInfo *decinfo );

/* Decode a character from LSB's of each byte from stego image */
char decode_byte_from_lsb( char *image_buffer );

#endif