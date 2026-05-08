/*=====================================================================================================================================
Project name    : LSB - Based Image Steganography in BMP using C
Name            : Pruthvi Raj H
Description     : This project is written in C.
                  It allows users to encode secret data inside BMP images by modifying the least significant bits.
                  Users can also decode the hidden data from stego images.
                  The program supports encoding/decoding of secret files with variable extensions.
                  To decode the secret message the mutual thing between the encoder and decoder is the magic string.
                  Input validation is performed to ensure correct file formats and successful data processing.
    command line : gcc -Wall main.c encode.c decode.c -o stego
  ======================================================================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function prototype */
OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    /* Minimum argument check */
    if (argc < 4)
    {
        fprintf(stderr, "ERROR: Insufficient arguments\n");
        return 1;
    }

    OperationType op_type = check_operation_type(argv[1]);

    /* ---------------- ENCODE ---------------- */
    if (op_type == e_encode)
    {
        EncodeInfo encInfo = {0};

        printf("Encoding Mode selected\n");
        printf("Encoding Mode started ...\n\n");

        /* Encoding supports argc = 4 or 5 */
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding completed successfully 🎉\n");
            }
            else
            {
                fprintf(stderr, "ERROR: Encoding failed\n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid encode arguments\n");
            return 1;
        }
    }

    /* ---------------- DECODE ---------------- */
    else if (op_type == e_decode)
    {
        if (argc < 4)
        {
            fprintf(stderr, "ERROR: Insufficient arguments for decoding\n");
            return 1;
        }

        printf("Decoding Mode selected\n");
        printf("Decoding started ...\n\n");

        DecodeInfo decInfo = {0};

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding completed successfully 🎉\n");
            }
            else
            {
                printf("Decoding failed \n");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Invalid decode arguments\n");
            return 1;
        }
    }

    /* ---------------- INVALID OPTION ---------------- */
    else
    {
        fprintf(stderr, "ERROR: Unsupported operation\n");
        return 1;
    }

    return 0;
}

/* Check operation type */
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
