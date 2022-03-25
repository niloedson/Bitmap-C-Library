/**
 * @file bmp.h
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief Bitmap C library
 * @version 0.4
 * @date 2022-03-16
 * 
 * @copyright Copyright (c) 2022
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

// 16bpp bit masks ========================================
#define BMP_BITFIELDS_R5G5B5_R5 0x7C00
#define BMP_BITFIELDS_R5G5B5_G5 0x03E0
#define BMP_BITFIELDS_R5G5B5_B5 0x001F

#define BMP_BITFIELDS_R5G6B5_R5 0xF800
#define BMP_BITFIELDS_R5G6B5_G6 0x07E0
#define BMP_BITFIELDS_R5G6B5_B5 0x001F

#define BMP_ALPHABITFIELDS_R4G4B4A4_R4 0x0F00
#define BMP_ALPHABITFIELDS_R4G4B4A4_G4 0x00F0
#define BMP_ALPHABITFIELDS_R4G4B4A4_B4 0x000F
#define BMP_ALPHABITFIELDS_R4G4B4A4_A4 0xF000

// 32bpp bit masks ========================================
#define BMP_ALPHABITFIELDS_R8G8B8A8_R8 0x00FF0000
#define BMP_ALPHABITFIELDS_R8G8B8A8_G8 0x0000FF00
#define BMP_ALPHABITFIELDS_R8G8B8A8_B8 0x000000FF
#define BMP_ALPHABITFIELDS_R8G8B8A8_A8 0xFF000000

#define BMP_ALPHABITFIELDS_R8G7B9A5_R8 0x0001FE00
#define BMP_ALPHABITFIELDS_R8G7B9A5_G7 0x00FE0000
#define BMP_ALPHABITFIELDS_R8G7B9A5_B9 0x000001FF
#define BMP_ALPHABITFIELDS_R8G7B9A5_A5 0x1F000000

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

typedef enum bmp_ncolours {
    BMP_NCOLOURS_256 = 256,
    BMP_NCOLOURS_128 = 128,
    BMP_NCOLOURS_64 = 64,
    BMP_NCOLOURS_32 = 32,
    BMP_NCOLOURS_16 = 16,
    BMP_NCOLOURS_8 = 8,
    BMP_NCOLOURS_4 = 4,
    BMP_NCOLOURS_2 = 2
} bmp_ncolours;

// (from https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logcolorspacea)
// (from https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/eb4bbd50-b3ce-4917-895c-be31f214797f) 
typedef enum bmp_bv4cstype {
    BMP_LCS_CALIBRATED_RGB = 0x00000000,
    BMP_LCS_sRGB = 0x73524742,
    BMP_LCS_WINDOWS_COLOR_SPACE = 0x57696E20
} bmp_bv4cstype;

// (from https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/3c289fe1-c42e-42f6-b125-4b5fc49a2b20)
typedef enum bmp_bv5cstype {
    BMP_LCS_PROFILE_LINKED = 0x4C494E4B,
    BMP_LCS_PROFILE_EMBEDDED = 0x4D424544
} bmp_bv5cstype;

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
    bmp_rgbtriple * bmciColors;
} bmp_coreinfo;

// BITMAPINFO structure
#pragma pack(1)
typedef struct bmp_info {
    bmp_infoheader bmiHeader;
    bmp_v4header bmiv4Header;
    bmp_v5header bmiv5Header;
    bmp_rgbquad * bmiColors;
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

/* file related functions -----------------------------------------------------*/

/**
 * @brief Read a Bitmap image and returns its metadata organized 
 * in a <bmp_image> struct pointer.
 * 
 * @param filename string specifying the filename to be read.
 * @return bmp_image* - pointer to the struct loaded with the 
 *                      <filename> image data.
 */
bmp_image * bmp_read(const char * filename);

/**
 * @brief Creates a Bitmap file with the <bmp_image> metadata.
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @param filename string specifying the filename to be created.
 * @return int - returns 0 if something goes wrong, 1 otherwise.
 */
int bmp_save(const bmp_image * img, const char * filename);

/* RGB functions --------------------------------------------------------------*/

/**
 * @brief Get the specified color value from the image[x,y] pixel.
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @param x the pixel x position.
 * @param y the pixel y position.
 * @param color the specified color.
 * @return uint8_t - color value retrieved.
 */
uint8_t bmp_getpixelcolor(bmp_image * img, int x, int y, bmp_color color);

/**
 * @brief Find the gray-scale equivalent value from RGB entries.
 * 
 * @param red red color entry value.
 * @param green green color entry value.
 * @param blue blue color entry value.
 * @return uint8_t - calculated gray-scale equivalent value.
 */
uint8_t bmp_findgray(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Converts an RGB (24bpp) image into a indexed gray level image (8bpp) 
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @param ncolours how many colours to use in the gray palette.
 * @return bmp_image* pointer to the new indexed gray level image.
 */
bmp_image * bmp_rgb2gray(bmp_image * img, bmp_ncolours ncolours);

/**
 * @brief Filter an RGB (24bpp) image by the specified color.
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @param color the specified color to filter.
 */
void bmp_filtercolor(bmp_image * img, bmp_color color);

/**
 * @brief Invert colors from an RGB (24bpp) image.
 * 
 * @param img pointer to the <bmp_image> metadata.
 */
void bmp_invert(bmp_image * img);

/* printing functions ---------------------------------------------------------*/

/**
 * @brief Print the image's metadata from its headers.
 * 
 * @param img pointer to the <bmp_image> metadata.
 */
void bmp_printdetails(bmp_image * img);

/**
 * @brief Print the specified image[x,y] pixel value.
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @param x the pixel x position.
 * @param y the pixel y position.
 */
void bmp_printpixel(bmp_image * img, int x, int y);

/* helper functions -----------------------------------------------------------*/

/**
 * @brief Clean both pointers.
 * 
 * @param fptr file pointer.
 * @param img <bmp_image> pointer.
 * @return bmp_image* always NULL.
 */
bmp_image * bmp_cleanup(FILE * fptr , bmp_image * img);

/**
 * @brief Check file headers to evaluate Bitmap conformity.
 * 
 * @param img pointer to the <bmp_image> metadata.
 * @return int - returns 0 if headers have some problem, 1 otherwise.
 */
int bmp_checkheaders(bmp_image * img);

/* Easter eggs ----------------------------------------------------------------*/

/**
 * @brief Retrieves the Redbricks.bmp as a 4bpp sample image.
 * 
 * @return bmp_image* - pointer to the metadata generated.
 */
bmp_image * bmp_getredbricks();

/**
 * @brief Retrieves an 8bpp BI_RGB sample image of black fading to white.
 * 
 * @return bmp_image* - pointer to the metadata generated.
 */
bmp_image * bmp_8bpp_sample();

/**
 * @brief Retrieves an 16bpp BI_RGB sample mixing red and blue.
 * 
 * @return bmp_image* - pointer to the metadata generated.
 */
bmp_image * bmp_16bpp_sample();

/**
 * @brief Retrieves an 32bpp BI_RGB sample mixing green and blue.
 * 
 * @return bmp_image* - pointer to the metadata generated.
 */
bmp_image * bmp_32bpp_sample();

#endif