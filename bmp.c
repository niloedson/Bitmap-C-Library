/**
 * @file bmp.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief Bitmap images C library
 * @version 0.2
 * @date 2022-03-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "bmp.h"

bmp_image * bmp_read(const char * filename)
{
    FILE * fptr = NULL;
    bmp_image * img = NULL;

    fptr = fopen(filename, "r");
    if (fptr == NULL) return bmp_cleanup(fptr, img);

    img = malloc(sizeof(bmp_image));
    if (img == NULL) return bmp_cleanup(fptr, img);

    if (fread( &img->fileheader, sizeof(bmp_fileheader), 1, fptr) != 1) 
        return bmp_cleanup(fptr, img);
    
    //TODO: implement acquisition for other headers than the BITMAPINFOHEADER
    if (fread( &img->info.bmiHeader, sizeof(bmp_infoheader), 1, fptr) != 1) 
        return bmp_cleanup(fptr, img);

    if (bmp_checkheaders(img) == 0) 
        return bmp_cleanup(fptr, img);

    uint32_t palettesize = pow(2,img->info.bmiHeader.biBitCount);

    switch (img->info.bmiHeader.biBitCount) {
    case BMP_1_BIT:
    case BMP_4_BITS:
    case BMP_8_BITS:
        if (fread( img->info.bmiColours, sizeof(bmp_rgbquad), palettesize, fptr) != palettesize) 
            return bmp_cleanup(fptr, img);
        break;
    case BMP_16_BITS:
        //TODO: interpret 2-byte bitfields mask values
        break;
    case BMP_32_BITS:
        //TODO: interpret 4-byte bitfields mask values
        break;
    case BMP_24_BITS:
        // never expect a BMP color palette
    default:
        break;
    }

    uint32_t datasize = img->fileheader.bfSize - img->fileheader.bfOffBits;

    img->ciPixelArray = malloc(sizeof(uint8_t) * datasize);
    if (img->ciPixelArray == NULL) 
        return bmp_cleanup(fptr, img);

    if (fread(img->ciPixelArray, sizeof(uint8_t), datasize, fptr) != datasize) 
        return bmp_cleanup(fptr, img);

    fclose(fptr);

    return img;
}

int bmp_save(const bmp_image * img, const char * filename)
{
    FILE * fptr = NULL;
    fptr = fopen(filename, "w");

    if (fptr == NULL) return 0;

    if (fwrite(&img->fileheader, sizeof(bmp_fileheader), 1, fptr) != 1) {
        fclose(fptr);
        return 0;
    }

    if (fwrite(&img->info.bmiHeader, sizeof(bmp_infoheader), 1, fptr) != 1) {
        fclose(fptr);
        return 0;
    }

    uint8_t * archive;
    
    uint32_t palettesize = sizeof(bmp_rgbquad) * pow(2,img->info.bmiHeader.biBitCount);

    switch (img->info.bmiHeader.biBitCount) {
    case BMP_1_BIT:
    case BMP_4_BITS:
    case BMP_8_BITS:
        archive = (uint8_t *) img->info.bmiColours;
        if (fwrite(archive, sizeof(uint8_t), palettesize, fptr) != palettesize) {
            fclose(fptr);
            return 0;
        }
        break;
    case BMP_16_BITS:
        //TODO: write 2-byte bitfields mask values
        break;
    case BMP_32_BITS:
        //TODO: write 4-byte bitfields mask values
        break;
    case BMP_24_BITS:
        // never write a BMP color palette
    default:
        break;
    }

    uint32_t datasize = img->fileheader.bfSize - img->fileheader.bfOffBits;

    if (fwrite(img->ciPixelArray, sizeof(uint8_t), datasize, fptr) != datasize) {
        fclose(fptr);
        return 0;
    }

    fclose(fptr);

    return 1;
}

uint8_t bmp_getpixelcolor(bmp_image * img, int x, int y, bmp_color color)
{
    //TODO: add support for 1, 4, 16 and 32 bits per pixel images
    if (img->info.bmiHeader.biBitCount == BMP_8_BITS) 
        return img->ciPixelArray[ y*img->info.bmiHeader.biWidth + x ];
    
    return img->ciPixelArray[3*(y*img->info.bmiHeader.biWidth + x) + color];
}
uint8_t bmp_findgray(uint8_t red, uint8_t green, uint8_t blue)
{
    double gray = 0.2989 * red + 0.5870 * green + 0.1140 * blue;
    return  (uint8_t) gray;
}

void bmp_rgb2gray(bmp_image * img)
{
    //TODO: add support for 1, 4, 8, 16 and 32 bits per pixel images
    for (unsigned int x = 0; x < img->info.bmiHeader.biWidth; x++) {
        for (unsigned int y = 0; y < img->info.bmiHeader.biHeight; y++) {
            uint8_t value = 
                bmp_findgray(
                bmp_getpixelcolor(img, x, y, BMP_COLOR_RED),
                bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN),
                bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE)
                );
            img->ciPixelArray[3*(y*img->info.bmiHeader.biWidth + x) + BMP_COLOR_RED] = value;
            img->ciPixelArray[3*(y*img->info.bmiHeader.biWidth + x) + BMP_COLOR_GREEN] = value;
            img->ciPixelArray[3*(y*img->info.bmiHeader.biWidth + x) + BMP_COLOR_BLUE] = value;
        }
        
    }
    
}
void bmp_filtercolor(bmp_image * img, bmp_color color)
{
    //TODO: add support for 1, 4, 8, 16 and 32 bits per pixel images
    uint32_t datasize = img->fileheader.bfSize - img->fileheader.bfOffBits;

    for (unsigned int pixel = color; pixel < datasize; pixel++) {
        if ((pixel % 3) != color) {
            img->ciPixelArray[pixel] = 0;
        }
    }
}
void bmp_invert(bmp_image * img)
{
    //TODO: add support for 1, 4, 16 and 32 bits per pixel images
    uint32_t datasize = img->fileheader.bfSize - img->fileheader.bfOffBits;

    for (unsigned int index = 0; index < datasize; index++) {
        img->ciPixelArray[index] = 255 - img->ciPixelArray[index];
    }
}

void bmp_details(bmp_image * img)
{
    printf("\n");
    printf("From BITMAPFILEHEADER: \n");

    printf("filetype:         \t%u  \t0x%x\n", img->fileheader.bfType, img->fileheader.bfType);
    printf("filesize:         \t%u  \t0x%x\n", img->fileheader.bfSize, img->fileheader.bfSize);
    printf("reserved1:        \t%u  \t0x%x\n", img->fileheader.bfReserved1, img->fileheader.bfReserved1);
    printf("reserved2:        \t%u  \t0x%x\n", img->fileheader.bfReserved2, img->fileheader.bfReserved2);
    printf("offset bits:      \t%u  \t0x%x\n", img->fileheader.bfOffBits, img->fileheader.bfOffBits);

    printf("\n");
    printf("From BITMAPINFOHEADER: \n");

    printf("size:             \t%u  \t0x%x\n", img->info.bmiHeader.biSize, img->info.bmiHeader.biSize);
    printf("width:            \t%u  \t0x%x\n", img->info.bmiHeader.biWidth, img->info.bmiHeader.biWidth);
    printf("height:           \t%u  \t0x%x\n", img->info.bmiHeader.biHeight, img->info.bmiHeader.biHeight);
    printf("planes:           \t%u  \t0x%x\n", img->info.bmiHeader.biPlanes, img->info.bmiHeader.biPlanes);
    printf("bits per pixel:   \t%u  \t0x%x\n", img->info.bmiHeader.biBitCount, img->info.bmiHeader.biBitCount);
    printf("compression:      \t%u  \t0x%x\n", img->info.bmiHeader.biCompression, img->info.bmiHeader.biCompression);
    printf("imagesize:        \t%u  \t0x%x\n", img->info.bmiHeader.biSizeImage, img->info.bmiHeader.biSizeImage);
    printf("xresolution:      \t%u  \t0x%x\n", img->info.bmiHeader.biXPelsPerMeter, img->info.bmiHeader.biXPelsPerMeter);
    printf("yresolution:      \t%u  \t0x%x\n", img->info.bmiHeader.biYPelsPerMeter, img->info.bmiHeader.biYPelsPerMeter);
    printf("colours used:     \t%u  \t0x%x\n", img->info.bmiHeader.biClrUsed, img->info.bmiHeader.biClrUsed);
    printf("main colours:     \t%u  \t0x%x\n", img->info.bmiHeader.biClrImportant, img->info.bmiHeader.biClrImportant);

    printf("\n");
}
void bmp_printpixel(bmp_image * img, int x, int y)
{
    printf(
        "img[%i][%i] = (%u, %u, %u)\n", x, y, 
        bmp_getpixelcolor(img, x, y, BMP_COLOR_RED), 
        bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN), 
        bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE)
    );
}

bmp_image * bmp_cleanup(FILE * fptr , bmp_image * img)
{
    if (fptr != NULL) fclose(fptr);

    if (img != NULL) {
        if (img->coreinfo.bmciColours != NULL) free(img->coreinfo.bmciColours);
        if (img->info.bmiColours != NULL) free(img->info.bmiColours);
        if (img->ciPixelArray != NULL) free(img->ciPixelArray);
        free(img);
    }

    return NULL;
}
int bmp_checkheaders(bmp_image * img)
{
    //TODO: implement checking for other headers than the BITMAPINFOHEADER
    if (img->fileheader.bfType != BMP_FILETYPE_BM) return 0;

    switch (img->info.bmiHeader.biBitCount) {
    case BMP_0_BITS: break;
    case BMP_1_BIT: break;
    case BMP_4_BITS: break;
    case BMP_8_BITS: break;
    case BMP_16_BITS: break;
    case BMP_24_BITS: break;
    case BMP_32_BITS: break;
    default : return 0;
    }

    if (img->info.bmiHeader.biPlanes != BMP_DEFAULT_COLORPLANES) return 0;

    switch (img->info.bmiHeader.biCompression) {
    case BMP_BI_RGB: break;
    case BMP_BI_RLE8: break;
    case BMP_BI_RLE4: break;
    case BMP_BI_BITFIELDS: break;
    case BMP_BI_JPEG: break;
    case BMP_BI_PNG: break;
    case BMP_BI_ALPHABITFIELDS: break;
    case BMP_BI_CMYK: break;
    case BMP_BI_CMYKRLE8: break;
    case BMP_BI_CMYKRLE4: break;
    default : return 0;
    }

    return 1;
}