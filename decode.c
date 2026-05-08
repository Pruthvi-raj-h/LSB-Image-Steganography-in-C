#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Ask magic string from user */
Status get_magic_string_from_user(DecodeInfo *decInfo)
{
    printf("Enter magic string: ");
    scanf("%s", decInfo->user_magic_string);
    return e_success;
}

/* Decode one byte from LSB */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0;
    for (int i = 0; i < 8; i++)
    {
        *data = (*data << 1) | (image_buffer[i] & 0x01);
    }
    return e_success;
}

/* Decode 32-bit size from LSB */
Status decode_size_from_lsb(int *size, char *image_buffer)
{
    *size = 0;
    for (int i = 0; i < 32; i++)
    {
        *size = (*size << 1) | (image_buffer[i] & 0x01);
    }
    return e_success;
}

/* Validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ext = strrchr(argv[2], '.');

    if (!ext || strcmp(ext, ".bmp") != 0)
    {
        fprintf(stderr, "ERROR: Stego image must be a .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if (argv[3])
        strcpy(decInfo->secret_fname, argv[3]);
    else
        strcpy(decInfo->secret_fname, "decoded");

    return e_success;
}

/* Open files for decoding */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (!decInfo->fptr_stego_image)
    {
        perror("ERROR opening stego image");
        return e_failure;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if (!decInfo->fptr_secret)
    {
        perror("ERROR opening output file");
        return e_failure;
    }

    return e_success;
}

/* Decode and verify magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded_char;

    for (int i = 0; decInfo->user_magic_string[i] != '\0'; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed while reading magic string data\n");
            return e_failure;
        }

        decode_byte_from_lsb(&decoded_char, buffer);

        if (decoded_char != decInfo->user_magic_string[i])
        {
            fprintf(stderr,
                "ERROR: Magic string mismatch. Unauthorized decoding.\n");
            return e_failure;
        }
    }

    printf("Magic string verified successfully ✓\n");
    return e_success;
}

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed to decode extension size\n");
        return e_failure;
    }

    decode_size_from_lsb(&decInfo->extn_size, buffer);
    return e_success;
}

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (int i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed to decode file extension\n");
            return e_failure;
        }

        decode_byte_from_lsb(&ch, buffer);
        decInfo->extn_secret_file[i] = ch;
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed to decode secret file size\n");
        return e_failure;
    }

    decode_size_from_lsb(&decInfo->secret_file_size, buffer);
    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded_char;

    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed while decoding secret data\n");
            return e_failure;
        }

        decode_byte_from_lsb(&decoded_char, buffer);
        fwrite(&decoded_char, 1, 1, decInfo->fptr_secret);
    }

    return e_success;
}

/* Decode controller */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) != e_success)
        return e_failure;

    /* Skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Ask user for magic string */
    if (get_magic_string_from_user(decInfo) != e_success)
        return e_failure;

    /* Verify magic string */
    if (decode_magic_string(decInfo) != e_success)
        return e_failure;

    if (decode_secret_file_extn_size(decInfo) != e_success)
        return e_failure;

    if (decode_secret_file_extn(decInfo) != e_success)
        return e_failure;

    /* Reopen secret file with correct extension */
    strcat(decInfo->secret_fname, decInfo->extn_secret_file);
    decInfo->fptr_secret =
        freopen(decInfo->secret_fname, "w", decInfo->fptr_secret);

    if (!decInfo->fptr_secret)
    {
        perror("ERROR reopening output file");
        return e_failure;
    }

    if (decode_secret_file_size(decInfo) != e_success)
        return e_failure;

    if (decode_secret_file_data(decInfo) != e_success)
        return e_failure;

    return e_success;
}
