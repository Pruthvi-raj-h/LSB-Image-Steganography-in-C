# LSB-Image-Steganography-in-C
LSB-based Image Steganography project in C that hides secret files inside BMP images by modifying Least Significant Bits (LSB). Supports encoding and decoding of .txt, .c, .h, and .sh files with magic string verification for secure extraction. Demonstrates file handling, bit manipulation, and modular programming.

## Objective
To embed and extract secret files within an image using LSB manipulation while maintaining image quality.

## Technologies Used
- C Programming
- File Handling
- Bitwise Operations
- Structures
- Modular Programming
- BMP Image Format

## What is Steganography
Steganography is a technique used to hide confidential data inside another file such as an image, audio, or video so that the presence of the data is not easily detectable.

## Working Principle (LSB Technique)
- Each pixel in a BMP image is represented using bytes.
- The least significant bit of each byte is replaced with a bit of secret data.
- Since only the LSB is modified, visual distortion is minimal.

## Features
- Supports encoding and decoding of secret files
- Uses a magic string for authentication
- Preserves original image quality
- Modular and structured C code

## Compilation
```bash
gcc -Wall main.c encode.c decode.c -o steganography
```

## Encoding
```bash
./stego -e beautiful.bmp secret.txt stego.bmp
```

## Decoding
```bash
./stego -d stego.bmp output
```

## Magic String
```text
#*
```

## Author
Pruthvi Raj H
