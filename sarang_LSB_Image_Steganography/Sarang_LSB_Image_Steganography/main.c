/*
 * Title       : sarang_Mp3_LSB_Image_Steganography_25017E
 * Project     : Project_2 - LSB Image Steganography
 * Developer   : Sarang krishna
 * Date        : 18/09/2025
 * Description : A project to design and implement an Image Steganography tool that allows users to
 *               hide (encode) and retrieve (decode) secret data within BMP image files.
 *               It demonstrates concepts of structures, file handling, bitwise operations,
 *               encoding/decoding algorithms, data parsing, memory management,
 *               and error handling. The project covers the complete workflow of
 *               embedding a secret file into an image and later extracting it
 *               using well-defined steps and verification mechanisms./
 */

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main( int argc, char *argv[] )
{
    EncodeInfo enc_info;
    DecodeInfo dec_info;

    if( check_operation_type( argv ) ==  e_encode )
    {
        if( argc >= 4 && read_and_validate_encode_args( argv, &enc_info ) == e_success )
        {
            do_encoding( &enc_info );
        }

        else
        {
            printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]\n");
            return 1;
        }
    }

    if( check_operation_type( argv ) ==  e_decode )
    {
        if( argc >= 3 && read_and_validate_decode_bmp( argv, &dec_info ) == d_success )
        {
            do_decoding( &dec_info, argv );
        }

        else
        {
            printf("./lsb_steg: Decoding: ./lsb_steg -d <.bmp file> [output file]\n");
            return 1;
        }
    }


    if( check_operation_type( argv ) ==  e_unsupported )
    {
        printf("./lsb_steg: Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]");
        printf("\n./lsb_steg: Decoding: ./lsb_steg -d <.bmp file> [output file]\n");
        return 1;
    }

    return 0;
}
