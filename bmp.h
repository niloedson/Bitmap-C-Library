/**
 * @file bmp.h
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief Bitmap images C library
 * @version 0.2
 * @date 2022-03-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __BMP_H_
#define __BMP_H_

/* Includes -------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Defines --------------------------------------------------------------------*/
#define BMP_FILETYPE_BM 0x4d42
#define BMP_FILETYPE_BA 0x4142
#define BMP_FILETYPE_CI 0x4943
#define BMP_FILETYPE_CP 0x5043
#define BMP_FILETYPE_IC 0x4349
#define BMP_FILETYPE_PT 0x5054

#define BMP_DEFAULT_COLORPLANES 1
#define BMP_FILEHEADER_SIZE 14

#pragma pack(1)

/* Enumerations ---------------------------------------------------------------*/

typedef enum bmp_dibheader {
    BMP_LEGACYHEADER = 24,
    BMP_COREHEADER = 12,
    BMP_INFOHEADER = 40,
    BMP_V4HEADER = 108,
    BMP_V5HEADER = 124
} bmp_dibheader;

typedef enum bmp_bpp {
    BMP_32_BITS = 32,
    BMP_24_BITS = 24,
    BMP_16_BITS = 16,
    BMP_8_BITS = 8,
    BMP_4_BITS = 4,
    BMP_2_BITS = 2,
    BMP_1_BIT = 1, 
    BMP_0_BITS = 0
} bmp_bpp;

typedef enum bmp_compression {
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
} bmp_compression;

typedef enum bmp_color {
    BMP_COLOR_ALPHA = 3,
    BMP_COLOR_RED = 2,
    BMP_COLOR_GREEN = 1,
    BMP_COLOR_BLUE = 0
} bmp_color;

// (from https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logcolorspacea)
typedef enum bmp_bv4cstype {
    BMP_LCS_CALIBRATED_RGB,
    BMP_LCS_sRGB,
    BMP_LCS_WINDOWS_COLOR_SPACE
} bmp_bv4cstype;

/* Coordinates structures -----------------------------------------------------*/

// CIEXYZ structure
#pragma pack(1)
typedef struct bmp_ciexyz {
    uint32_t ciexyzX; // all 3 are FXPT2DOT30 type variables.
    uint32_t ciexyzY; // 32-bit floats
    uint32_t ciexyzZ; // still need to clarify these types
} bmp_ciexyz;

// CIEXYZTRIPLE structure
#pragma pack(1)
typedef struct bmp_ciexyztriple {
    bmp_ciexyz ciexyzRed;
    bmp_ciexyz ciexyzGreen;
    bmp_ciexyz ciexyzBlue;
} bmp_ciexyztriple;

// BITMAP legacy structure (from https://docs.microsoft.com/pt-br/windows/win32/api/wingdi/ns-wingdi-bitmap)
#pragma pack(1)
typedef struct bmp_legacy {
  int32_t bmType;
  int32_t bmWidth;
  int32_t bmHeight;
  int32_t bmWidthBytes;
  uint16_t bmPlanes;
  uint16_t bmBitsPixel;
  uint8_t * bmBits;
} bmp_legacy;

/* FILE Header ----------------------------------------------------------------*/

// BITMAPFILEHEADER structure
#pragma pack(1)
typedef struct bmp_fileheader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} bmp_fileheader;

/* DIB Headers ----------------------------------------------------------------*/

// BITMAPCOREHEADER structure
#pragma pack(1)
typedef struct bmp_coreheader {
    uint32_t bcSize;
    uint16_t bcWidth;
    uint16_t bcHeight;
    uint16_t bcPlanes;
    uint16_t bcBitCount;
} bmp_coreheader;

// BITMAPINFOHEADER structure
#pragma pack(1)
typedef struct bmp_infoheader {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} bmp_infoheader;

// BITMAPV4HEADER structure
#pragma pack(1)
typedef struct bmp_v4header {
    uint32_t bV4Size;
    int32_t bV4Width;
    int32_t bV4Height;
    uint16_t bV4Planes;
    uint16_t bV4BitCount;
    uint32_t bV4Compression;
    uint32_t bV4SizeImage;
    int32_t bV4XPelsPerMeter;
    int32_t bV4YPelsPerMeter;
    uint32_t bV4ClrUsed;
    uint32_t bV4ClrImportant;
    uint32_t bV4RedMask;
    uint32_t bV4GreenMask;
    uint32_t bV4BlueMask;
    uint32_t bV4AlphaMask;
    uint32_t bV4CSType;
    bmp_ciexyztriple bV4Endpoints;
    uint32_t bV4GammaRed;
    uint32_t bV4GammaGreen;
    uint32_t bV4GammaBlue;
} bmp_v4header;

// BITMAPV5HEADER structure
#pragma pack(1)
typedef struct bmp_v5header {
    uint32_t bV5Size;
    int32_t bV5Width;
    int32_t bV5Height;
    uint16_t bV5Planes;
    uint16_t bV5BitCount;
    uint32_t bV5Compression;
    uint32_t bV5SizeImage;
    int32_t bV5XPelsPerMeter;
    int32_t bV5YPelsPerMeter;
    uint32_t bV5ClrUsed;
    uint32_t bV5ClrImportant;
    uint32_t bV5RedMask;
    uint32_t bV5GreenMask;
    uint32_t bV5BlueMask;
    uint32_t bV5AlphaMask;
    uint32_t bV5CSType;
    bmp_ciexyztriple bV5Endpoints;
    uint32_t bV5GammaRed;
    uint32_t bV5GammaGreen;
    uint32_t bV5GammaBlue;
    uint32_t bV5Intent;
    uint32_t bV5ProfileData;
    uint32_t bV5ProfileSize;
    uint32_t bV5Reserved;
} bmp_v5header;

/* Colour Palettes ------------------------------------------------------------*/ 

// RGBQUAD structure
#pragma pack(1)
typedef struct bmp_rgbquad {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} bmp_rgbquad;

// RGBTRIPLE structure
#pragma pack(1)
typedef struct bmp_rgbtriple {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
} bmp_rgbtriple;

/* Joint structures -----------------------------------------------------------*/

// BITMAPCOREINFO structure
#pragma pack(1)
typedef struct bmp_coreinfo {
    bmp_coreheader bmciHeader;
    bmp_rgbtriple * bmciColours;
} bmp_coreinfo;

// BITMAPINFO structure
#pragma pack(1)
typedef struct bmp_info {
    bmp_infoheader bmiHeader;
    bmp_v4header bmiv4Header;
    bmp_v5header bmiv5Header;
    bmp_rgbquad * bmiColours;
} bmp_info;

/* Main Structure -------------------------------------------------------------*/
//  (from https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-storage?redirectedfrom=MSDN)

#pragma pack(1)
typedef struct bmp_image {
    bmp_fileheader fileheader;
    bmp_coreinfo coreinfo;
    bmp_info info;
    uint8_t * ciPixelArray;
} bmp_image;

/* Functions ------------------------------------------------------------------*/

// file related functions
bmp_image * bmp_read(const char * filename);
int bmp_save(const bmp_image * img, const char * filename);

// action functions
uint8_t bmp_getpixelcolor(bmp_image * img, int x, int y, bmp_color color);
uint8_t bmp_findgray(uint8_t red, uint8_t green, uint8_t blue);

// processing functions
void bmp_rgb2gray(bmp_image * img);
void bmp_filtercolor(bmp_image * img, bmp_color color);
void bmp_invert(bmp_image * img);

// misc functions
void bmp_details(bmp_image * img);
void bmp_printpixel(bmp_image * img, int x, int y);

// helper functions
bmp_image * bmp_cleanup(FILE * fptr , bmp_image * img);
int bmp_checkheaders(bmp_image * img);

#endif