#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    printf("width = %u\n", width);
    printf("height = %u\n\n", height);

    rewind(fptr_image);                 /* IMPORTANT */
    return width * height * 3;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = (uint)ftell(fptr);
    rewind(fptr);
    return size;
}

/* Validate secret file extensions */
int is_valid_secret_extn(const char *ext)
{
    if (!ext)
        return 0;

    return (strcmp(ext, ".txt") == 0 ||
            strcmp(ext, ".c")   == 0 ||
            strcmp(ext, ".h")   == 0 ||
            strcmp(ext, ".sh")  == 0);
}

/* Validate arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ext;

    ext = strrchr(argv[2], '.');
    if (!ext || strcmp(ext, ".bmp") != 0)
    {
        fprintf(stderr, "ERROR: Source image must be a .bmp file\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    ext = strrchr(argv[3], '.');
    if (!is_valid_secret_extn(ext))
    {
        fprintf(stderr,
            "ERROR: Secret file must be .txt / .c / .h / .sh\n");
        return e_failure;
    }

    encInfo->secret_fname = argv[3];
    strcpy(encInfo->extn_secret_file, ext);

    if (argv[4])
        encInfo->dest_image_fname = argv[4];
    else
        encInfo->dest_image_fname = "stego.bmp";

    return e_success;
}

/* Open files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image)
    {
        perror("ERROR opening source image");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (!encInfo->fptr_secret)
    {
        perror("ERROR opening secret file");
        return e_failure;
    }

    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "wb");
    if (!encInfo->fptr_dest_image)
    {
        perror("ERROR opening output image");
        return e_failure;
    }

    return e_success;
}

/* Check capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity   =
        get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    long bits_required = 0;
    bits_required += 32;
    bits_required += strlen(MAGIC_STRING) * 8;
    bits_required += 32;
    bits_required += strlen(encInfo->extn_secret_file) * 8;
    bits_required += 32;
    bits_required += encInfo->size_secret_file * 8;

    printf("Total Image Capacity = %u\n", encInfo->image_capacity);
    printf("Secret file size: %u bytes\n\n",
           encInfo->size_secret_file);

    if (encInfo->image_capacity < bits_required)
    {
        fprintf(stderr,
            "ERROR: Insufficient image capacity\n");
        return e_failure;
    }

    printf("Encoding is possible ✓\n");
    return e_success;
}

/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        fprintf(stderr, "ERROR: Failed to read BMP header\n");
        return e_failure;
    }

    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        fprintf(stderr, "ERROR: Failed to write BMP header\n");
        return e_failure;
    }

    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; magic_string[i]; i++)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed while encoding magic string\n");
            return e_failure;
        }

        encode_byte_to_lsb(magic_string[i], buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_dest_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed to write magic string\n");
            return e_failure;
        }
    }

    return e_success;
}

/* Encode extension size */
Status encode_secret_file_extn_size(int size,
                                    EncodeInfo *encInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32,
              encInfo->fptr_src_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed to read extension size data\n");
        return e_failure;
    }

    encode_size_to_lsb(size, buffer);

    if (fwrite(buffer, 1, 32,
               encInfo->fptr_dest_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed to write extension size\n");
        return e_failure;
    }

    return e_success;
}

/* Encode extension */
Status encode_secret_file_extn(const char *file_extn,
                               EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; file_extn[i]; i++)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed while encoding extension\n");
            return e_failure;
        }

        encode_byte_to_lsb(file_extn[i], buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_dest_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed to write extension\n");
            return e_failure;
        }
    }

    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32,
              encInfo->fptr_src_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed while encoding file size\n");
        return e_failure;
    }

    encode_size_to_lsb((int)file_size, buffer);

    if (fwrite(buffer, 1, 32,
               encInfo->fptr_dest_image) != 32)
    {
        fprintf(stderr,
            "ERROR: Failed to write file size\n");
        return e_failure;
    }

    return e_success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_byte;
    char buffer[8];

    while (fread(&secret_byte, 1, 1,
                 encInfo->fptr_secret) == 1)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed while encoding secret data\n");
            return e_failure;
        }

        encode_byte_to_lsb(secret_byte, buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_dest_image) != 8)
        {
            fprintf(stderr,
                "ERROR: Failed to write secret data\n");
            return e_failure;
        }
    }

    return e_success;
}

/* Copy remaining image data */
Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            fprintf(stderr,
                "ERROR: Failed while copying remaining image data\n");
            return e_failure;
        }
    }

    return e_success;
}

/* Encode byte to LSB */
Status encode_byte_to_lsb(char data,
                          char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) |
            ((data >> (7 - i)) & 0x01);
    }
    return e_success;
}

/* Encode size to LSB */
Status encode_size_to_lsb(int size,
                          char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] =
            (imageBuffer[i] & 0xFE) |
            ((size >> (31 - i)) & 0x01);
    }
    return e_success;
}

/* Controller */
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) != e_success)
        return e_failure;

    if (check_capacity(encInfo) != e_success)
        return e_failure;

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_dest_image) != e_success)
        return e_failure;

    printf("BMP header copied successfully ✓\n");

    fseek(encInfo->fptr_src_image, 54, SEEK_SET);

    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_extn_size(
            strlen(encInfo->extn_secret_file),
            encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_extn(
            encInfo->extn_secret_file,
            encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_size(
            encInfo->size_secret_file,
            encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;

    if (copy_remaining_img_data(
            encInfo->fptr_src_image,
            encInfo->fptr_dest_image) != e_success)
        return e_failure;

    printf("Successfully completed ENCODING 🎉\n");
    return e_success;
}
