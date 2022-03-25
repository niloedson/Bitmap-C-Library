/**
 * @file bmp.h
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief library to handle bitmap image files (*.bmp)
 * @version 0.1
 * @date 2022-02-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __BMP_H_
#define __BMP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define BMP_INIT_HEADER 0x4d42
#define BMP_N_COLOR_PLANES 1
#define BMP_BITS_PER_BYTE 8

#pragma pack(1)

typedef enum bmp_nbits_per_pixel {
    BMP_32_BITS = 32,
    BMP_24_BITS = 24,
    BMP_16_BITS = 16,
    BMP_8_BITS = 8,
    BMP_4_BITS = 4,
    BMP_2_BITS = 2,
    BMP_1_BIT = 1
} bmp_nbits_per_pixel;

typedef enum bmp_compression_method {
    BMP_BI_RGB = 0,
    BMP_BI_RLE8 = 1,
    BMP_BI_RLE4 = 2,
    BMP_BI_BITFIELDS = 3,
    BMP_BI_JPEG = 4,
    BMP_BI_PNG = 5,
    BMP_BI_ALPHABITFIELDS = 6,
    BMP_BI_CMYK = 11,
    BMP_BI_CMYKRLE8 = 12,
    BMP_BI_CMYKRLE4 = 13
} bmp_compression_method;

typedef struct bmp_header {
    __uint16_t type;
    __uint32_t size;
    __uint16_t reserved1;
    __uint16_t reserved2;
    __uint32_t offset;
    __uint32_t header_size;
    __uint32_t width;
    __uint32_t height;
    __uint16_t planes;
    __uint16_t bits;
    __uint32_t compression;
    __uint32_t imagesize;
    __uint32_t xresolution;
    __uint32_t yresolution;
    __uint32_t ncolours;
    __uint32_t importantcolours;
} bmp_header;

typedef struct bmp_image {
    bmp_header header;
    unsigned int data_size;
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel;
    unsigned char * data;
} bmp_image;

typedef enum bmp_color {
    BMP_COLOR_RED = 2,
    BMP_COLOR_GREEN = 1,
    BMP_COLOR_BLUE = 0
} bmp_color;

bmp_image * bmp_clean_up(FILE * fptr , bmp_image * img);
int bmp_check_header(bmp_header * hdr);

bmp_image * bmp_open(const char * filename);
bmp_image * bmp_load(unsigned char * imptr);
int bmp_save(const bmp_image * img, const char * filename);

__uint8_t bmp_getpixel(bmp_image * img, int x, int y, bmp_color color);
__uint8_t bmp_findgray(__uint8_t red, __uint8_t green, __uint8_t blue);

void bmp_filter_color(bmp_image * img, bmp_color color);
void bmp_invert(bmp_image * img);
void bmp_equalize(bmp_image * img);

void bmp_details(bmp_image * img);
void bmp_printpixel(bmp_image * img, int x, int y);

#endif