#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "types.h"

/* Encode Info structure */
typedef struct _EncodeInfo
{
    /* Source image info */
    char *src_image_fname;
    FILE *fptr_src_image;

    /* Secret file info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[10];
    uint size_secret_file;

    /* Stego image info */
    char *dest_image_fname;
    FILE *fptr_dest_image;

    /* Image capacity */
    uint image_capacity;

} EncodeInfo;

/* Encode argument handling */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);
Status open_files(EncodeInfo *encInfo);
Status check_capacity(EncodeInfo *encInfo);

/* Encode core functions */
Status copy_bmp_header(FILE *src, FILE *dest);
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo);
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);
Status encode_secret_file_data(EncodeInfo *encInfo);
Status copy_remaining_img_data(FILE *src, FILE *dest);

/* LSB helper functions */
Status encode_byte_to_lsb(char data, char *image_buffer);
Status encode_size_to_lsb(int size, char *image_buffer);

/* Controller */
Status do_encoding(EncodeInfo *encInfo);

/* Validation helper (DECLARATION ONLY) */
int is_valid_secret_extn(const char *ext);

#endif /* ENCODE_H */
