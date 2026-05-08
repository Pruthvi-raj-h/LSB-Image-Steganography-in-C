#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Maximum length for user-entered magic string */
#define MAX_MAGIC_LEN 20

/* Decode Info structure */
typedef struct _DecodeInfo
{
    /* Stego image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output secret file info */
    char secret_fname[100];
    FILE *fptr_secret;

    /* Decoded metadata */
    int extn_size;
    char extn_secret_file[10];
    int secret_file_size;

    /* User-entered magic string */
    char user_magic_string[MAX_MAGIC_LEN];

} DecodeInfo;

/* Argument validation */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* File handling */
Status open_decode_files(DecodeInfo *decInfo);

/* Magic string handling */
Status get_magic_string_from_user(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode helpers */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

/* LSB helper functions */
Status decode_byte_from_lsb(char *data, char *image_buffer);
Status decode_size_from_lsb(int *size, char *image_buffer);

/* Controller */
Status do_decoding(DecodeInfo *decInfo);

#endif /* DECODE_H */
