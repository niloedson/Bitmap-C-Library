/**
 * @file bmp.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief Bitmap C library
 * @version 0.5
 * @date 2022-03-16
 * 
 * @copyright Copyright (c) 2022
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
    
    //TODO: implement acquisition for headers other than the BITMAPINFOHEADER
    if (fread( &img->info.bmiHeader, sizeof(bmp_infoheader), 1, fptr) != 1) 
        return bmp_cleanup(fptr, img);

    if (bmp_checkheaders(img) == 0) 
        return bmp_cleanup(fptr, img);

    uint32_t palettesize = pow(2,img->info.bmiHeader.biBitCount);

    switch (img->info.bmiHeader.biBitCount) {
    case BMP_1_BIT:
    case BMP_4_BITS:
    case BMP_8_BITS:
        if (fread( img->info.bmiColors, sizeof(bmp_rgbquad), palettesize, fptr) != palettesize) 
            return bmp_cleanup(fptr, img);
        break;
    case BMP_16_BITS:
        //TODO: interpret the 3 4-byte bitfields mask values
        break;
    case BMP_32_BITS:
        //TODO: interpret the 4 4-byte bitfields mask values
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
        archive = (uint8_t *) img->info.bmiColors;
        if (fwrite(archive, sizeof(uint8_t), palettesize, fptr) != palettesize) {
            fclose(fptr);
            return 0;
        }
        break;
    case BMP_16_BITS:
        //TODO: write the 3 4-byte bitfields mask values
        break;
    case BMP_32_BITS:
        //TODO: write the 4 4-byte bitfields mask values
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

bmp_image * bmp_rgb2gray(bmp_image * img, bmp_ncolours ncolours)
{
    if (img == NULL) return NULL;
    
    //TODO: extend support to convert 16bpp and 32bpp images

    if (img->info.bmiHeader.biBitCount != BMP_24_BITS) return NULL;
    
    bmp_image * new = malloc(sizeof(bmp_image));
    if (new == NULL) return bmp_cleanup(NULL, new);

    new->info.bmiHeader.biSize = BMP_INFOHEADER;
    new->info.bmiHeader.biWidth = img->info.bmiHeader.biWidth;
    new->info.bmiHeader.biHeight = img->info.bmiHeader.biHeight;
    new->info.bmiHeader.biPlanes = BMP_DEFAULT_COLORPLANES;
    new->info.bmiHeader.biBitCount = BMP_8_BITS;
    new->info.bmiHeader.biCompression = BMP_BI_RGB;
    new->info.bmiHeader.biSizeImage = new->info.bmiHeader.biWidth 
                    * new->info.bmiHeader.biHeight 
                    * new->info.bmiHeader.biBitCount / BMP_8_BITS;
    new->info.bmiHeader.biXPelsPerMeter = img->info.bmiHeader.biXPelsPerMeter;
    new->info.bmiHeader.biYPelsPerMeter = img->info.bmiHeader.biYPelsPerMeter;
    new->info.bmiHeader.biClrUsed = 0;
    new->info.bmiHeader.biClrImportant = 0;

    uint32_t colourpalettesize = pow(2,new->info.bmiHeader.biBitCount) 
                    * sizeof(bmp_rgbquad);
    
    new->fileheader.bfType = BMP_FILETYPE_BM;
    new->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + new->info.bmiHeader.biSize 
                    + colourpalettesize 
                    + new->info.bmiHeader.biSizeImage;
    
    new->fileheader.bfReserved1 = 0;
    new->fileheader.bfReserved2 = 0;
    
    new->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + new->info.bmiHeader.biSize 
                    + colourpalettesize;
    
    //TODO: add support to 4bpp, 2bpp and 1bpp generation

    new->info.bmiColors = malloc(colourpalettesize);

    if (ncolours == 0) ncolours = BMP_NCOLOURS_256;

    uint32_t maxcolours = pow(2, new->info.bmiHeader.biBitCount);
    uint32_t steps = (maxcolours/ncolours);

    for (uint32_t i = 0; i < maxcolours; i = i + steps) {
        for (uint8_t k = 0; k < steps; k++) {
            new->info.bmiColors[i+k].rgbBlue = i;
            new->info.bmiColors[i+k].rgbGreen = i;
            new->info.bmiColors[i+k].rgbRed = i;
            new->info.bmiColors[i+k].rgbReserved = 0;
        }
    }

    new->ciPixelArray = malloc(new->info.bmiHeader.biSizeImage);

    for (size_t x = 0; x < new->info.bmiHeader.biWidth; x++) {
        for (size_t y = 0; y < new->info.bmiHeader.biHeight; y++) {

            uint8_t ReqG = bmp_getpixelcolor(img, x, y, BMP_COLOR_RED) == bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN);
            uint8_t GeqB = bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN) == bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE);
            uint8_t BeqR = bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE) == bmp_getpixelcolor(img, x, y, BMP_COLOR_RED);

            if (ReqG && GeqB && BeqR) {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_getpixelcolor(img, x, y, BMP_COLOR_RED);
            } else {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_findgray(
                        bmp_getpixelcolor(img, x, y, BMP_COLOR_RED),
                        bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN),
                        bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE)
                    );
            }
            
        }
    }

    return new;
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

void bmp_addpad(bmp_image * img, uint32_t rows, uint32_t columns, bmp_padtype type)
{
    bmp_padv(img, rows, type);
    bmp_padh(img, columns, type);
}

void bmp_padh(bmp_image * img, uint32_t num, bmp_padtype type)
{
    if (img->info.bmiHeader.biBitCount != 8) return;
    //TODO: add support to other bit per pixel configurations.

    uint32_t newWidth = img->info.bmiHeader.biWidth + 2*num;

    uint32_t datasize = newWidth 
                    * img->info.bmiHeader.biHeight 
                    * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    uint8_t * newPixelArray = malloc(datasize);

    for (uint32_t y = 0; y < img->info.bmiHeader.biHeight; y++)
    {
        for (uint32_t x = 0; x < num; x++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->info.bmiHeader.biWidth + 0];
                break;
            
            default:
                return;
                break;
            }
        }

        for (uint32_t x = num; x < img->info.bmiHeader.biWidth + num; x++)
        {
            newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->info.bmiHeader.biWidth + x - num];
        }

        for (uint32_t x = img->info.bmiHeader.biWidth + num; x < newWidth; x++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->info.bmiHeader.biWidth + img->info.bmiHeader.biWidth - 1];
                break;
            
            default:
                return;
                break;
            }
        }       
    }
    
    free(img->ciPixelArray);
    
    img->ciPixelArray = malloc(datasize);
    for (size_t i = 0; i < datasize; i++)
    {
        img->ciPixelArray[i] = newPixelArray[i];
    }
    
    free(newPixelArray);

    img->fileheader.bfSize = img->fileheader.bfSize - img->info.bmiHeader.biSizeImage + datasize;
    img->info.bmiHeader.biSizeImage = datasize;
    img->info.bmiHeader.biWidth = newWidth;
}

void bmp_padv(bmp_image * img, uint32_t num, bmp_padtype type)
{
    if (img->info.bmiHeader.biBitCount != 8) return;
    //TODO: add support to other bit per pixel configurations.

    uint32_t newHeight = img->info.bmiHeader.biHeight + 2*num;

    uint32_t datasize = img->info.bmiHeader.biWidth 
                    * newHeight 
                    * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    uint8_t * newPixelArray = malloc(datasize);

    for (uint32_t x = 0; x < img->info.bmiHeader.biWidth; x++)
    {
        for (uint32_t y = 0; y < num; y++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = img->ciPixelArray[0 + x];
                break;
            
            default:
                return;
                break;
            }
        }

        for (uint32_t y = num; y < img->info.bmiHeader.biHeight + num; y++)
        {
            newPixelArray[y * img->info.bmiHeader.biWidth + x] = img->ciPixelArray[(y - num) * img->info.bmiHeader.biWidth + x];
        }

        for (uint32_t y = img->info.bmiHeader.biHeight + num; y < newHeight; y++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = img->ciPixelArray[(img->info.bmiHeader.biHeight - 1) * img->info.bmiHeader.biWidth + x];
                break;
            
            default:
                return;
                break;
            }
        }
    }

    free(img->ciPixelArray);
    
    img->ciPixelArray = malloc(datasize);
    for (size_t i = 0; i < datasize; i++)
    {
        img->ciPixelArray[i] = newPixelArray[i];
    }
    
    free(newPixelArray);
    
    img->fileheader.bfSize = img->fileheader.bfSize - img->info.bmiHeader.biSizeImage + datasize;
    img->info.bmiHeader.biSizeImage = datasize;
    img->info.bmiHeader.biHeight = newHeight;
}

void bmp_printdetails(bmp_image * img)
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
        if (img->coreinfo.bmciColors != NULL) free(img->coreinfo.bmciColors);
        if (img->info.bmiColors != NULL) free(img->info.bmiColors);
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

bmp_image * bmp_getredbricks()
{
    bmp_image * img = malloc(sizeof(bmp_image));
    if (img == NULL) return NULL;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = 630;
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    img->fileheader.bfOffBits = 118;

    img->info.bmiHeader.biSize = 40;
    img->info.bmiHeader.biWidth = 32;
    img->info.bmiHeader.biHeight = 32;
    img->info.bmiHeader.biPlanes = 1;
    img->info.bmiHeader.biBitCount = BMP_4_BITS;
    img->info.bmiHeader.biCompression = BMP_BI_RGB;
    img->info.bmiHeader.biSizeImage = 0;
    img->info.bmiHeader.biXPelsPerMeter = 0;
    img->info.bmiHeader.biYPelsPerMeter = 0;
    img->info.bmiHeader.biClrUsed = 0;
    img->info.bmiHeader.biClrImportant = 0;

    img->info.bmiColors = malloc(sizeof(bmp_rgbquad)*pow(2,img->info.bmiHeader.biBitCount));

    img->info.bmiColors[0].rgbBlue = 0x00; img->info.bmiColors[0].rgbGreen = 0x00; img->info.bmiColors[0].rgbRed = 0x00; img->info.bmiColors[0].rgbReserved = 0x00;
    img->info.bmiColors[1].rgbBlue = 0x00; img->info.bmiColors[1].rgbGreen = 0x00; img->info.bmiColors[1].rgbRed = 0x80; img->info.bmiColors[1].rgbReserved = 0x00;
    img->info.bmiColors[2].rgbBlue = 0x00; img->info.bmiColors[2].rgbGreen = 0x80; img->info.bmiColors[2].rgbRed = 0x00; img->info.bmiColors[2].rgbReserved = 0x00;
    img->info.bmiColors[3].rgbBlue = 0x00; img->info.bmiColors[3].rgbGreen = 0x80; img->info.bmiColors[3].rgbRed = 0x80; img->info.bmiColors[3].rgbReserved = 0x00;
    
    img->info.bmiColors[4].rgbBlue = 0x80; img->info.bmiColors[4].rgbGreen = 0x00; img->info.bmiColors[4].rgbRed = 0x00; img->info.bmiColors[4].rgbReserved = 0x00;
    img->info.bmiColors[5].rgbBlue = 0x80; img->info.bmiColors[5].rgbGreen = 0x00; img->info.bmiColors[5].rgbRed = 0x80; img->info.bmiColors[5].rgbReserved = 0x00;
    img->info.bmiColors[6].rgbBlue = 0x80; img->info.bmiColors[6].rgbGreen = 0x80; img->info.bmiColors[6].rgbRed = 0x00; img->info.bmiColors[6].rgbReserved = 0x00;
    img->info.bmiColors[7].rgbBlue = 0x80; img->info.bmiColors[7].rgbGreen = 0x80; img->info.bmiColors[7].rgbRed = 0x80; img->info.bmiColors[7].rgbReserved = 0x00;
    
    img->info.bmiColors[8].rgbBlue = 0xc0; img->info.bmiColors[8].rgbGreen = 0xc0; img->info.bmiColors[8].rgbRed = 0xc0; img->info.bmiColors[8].rgbReserved = 0x00;
    img->info.bmiColors[9].rgbBlue = 0x00; img->info.bmiColors[9].rgbGreen = 0x00; img->info.bmiColors[9].rgbRed = 0xff; img->info.bmiColors[9].rgbReserved = 0x00;
    img->info.bmiColors[10].rgbBlue = 0x00; img->info.bmiColors[10].rgbGreen = 0xff; img->info.bmiColors[10].rgbRed = 0x00; img->info.bmiColors[10].rgbReserved = 0x00;
    img->info.bmiColors[11].rgbBlue = 0x00; img->info.bmiColors[11].rgbGreen = 0xff; img->info.bmiColors[11].rgbRed = 0xff; img->info.bmiColors[11].rgbReserved = 0x00;
    
    img->info.bmiColors[12].rgbBlue = 0xff; img->info.bmiColors[12].rgbGreen = 0x00; img->info.bmiColors[12].rgbRed = 0x00; img->info.bmiColors[12].rgbReserved = 0x00;
    img->info.bmiColors[13].rgbBlue = 0xff; img->info.bmiColors[13].rgbGreen = 0x00; img->info.bmiColors[13].rgbRed = 0xff; img->info.bmiColors[13].rgbReserved = 0x00;
    img->info.bmiColors[14].rgbBlue = 0xff; img->info.bmiColors[14].rgbGreen = 0xff; img->info.bmiColors[14].rgbRed = 0x00; img->info.bmiColors[14].rgbReserved = 0x00;
    img->info.bmiColors[15].rgbBlue = 0xff; img->info.bmiColors[15].rgbGreen = 0xff; img->info.bmiColors[15].rgbRed = 0xff; img->info.bmiColors[15].rgbReserved = 0x00;

    unsigned char ciPixelArray[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x11, 0x11, 0x01, 0x19, 0x11, 0x01, 0x10, 0x10, 0x09, 0x09, 0x01, 0x09, 0x11, 0x11, 0x01, 0x90, 
        0x11, 0x01, 0x19, 0x09, 0x09, 0x91, 0x11, 0x10, 0x09, 0x11, 0x09, 0x11, 0x19, 0x10, 0x90, 0x11, 
        0x19, 0x01, 0x19, 0x19, 0x10, 0x10, 0x11, 0x10, 0x09, 0x01, 0x91, 0x10, 0x91, 0x09, 0x10, 0x10, 
        0x11, 0x10, 0x11, 0x91, 0x99, 0x11, 0x09, 0x90, 0x09, 0x91, 0x01, 0x11, 0x11, 0x11, 0x91, 0x10, 
        0x90, 0x99, 0x11, 0x11, 0x11, 0x11, 0x19, 0x00, 0x09, 0x01, 0x91, 0x01, 0x01, 0x19, 0x00, 0x99, 
        0x09, 0x19, 0x01, 0x00, 0x11, 0x90, 0x91, 0x10, 0x09, 0x01, 0x11, 0x99, 0x10, 0x01, 0x11, 0x11, 
        0x91, 0x11, 0x11, 0x19, 0x10, 0x11, 0x99, 0x10, 0x09, 0x10, 0x01, 0x11, 0x11, 0x11, 0x19, 0x10, 
        0x11, 0x09, 0x09, 0x10, 0x19, 0x10, 0x10, 0x10, 0x09, 0x01, 0x11, 0x19, 0x00, 0x01, 0x10, 0x19, 
        0x10, 0x11, 0x11, 0x01, 0x99, 0x01, 0x11, 0x90, 0x09, 0x19, 0x11, 0x91, 0x11, 0x91, 0x01, 0x11, 
        0x19, 0x10, 0x99, 0x00, 0x01, 0x19, 0x09, 0x10, 0x09, 0x19, 0x10, 0x91, 0x11, 0x01, 0x11, 0x11, 
        0x91, 0x01, 0x91, 0x19, 0x11, 0x00, 0x99, 0x90, 0x09, 0x01, 0x01, 0x99, 0x19, 0x01, 0x91, 0x10, 
        0x19, 0x91, 0x91, 0x09, 0x11, 0x99, 0x11, 0x10, 0x09, 0x91, 0x11, 0x10, 0x11, 0x91, 0x99, 0x10, 
        0x90, 0x11, 0x01, 0x11, 0x11, 0x19, 0x11, 0x90, 0x09, 0x11, 0x00, 0x19, 0x10, 0x11, 0x01, 0x11, 
        0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x09, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x99, 0x11, 0x11, 0x11, 0x19, 0x10, 0x19, 0x19, 0x11, 0x09, 0x10, 0x90, 0x91, 0x90, 0x91, 0x00, 
        0x91, 0x19, 0x19, 0x09, 0x01, 0x10, 0x09, 0x01, 0x11, 0x11, 0x91, 0x11, 0x11, 0x11, 0x10, 0x00, 
        0x91, 0x11, 0x01, 0x19, 0x10, 0x11, 0x10, 0x01, 0x01, 0x11, 0x90, 0x11, 0x11, 0x11, 0x91, 0x00, 
        0x99, 0x09, 0x19, 0x10, 0x11, 0x90, 0x09, 0x90, 0x91, 0x01, 0x19, 0x09, 0x91, 0x11, 0x01, 0x00, 
        0x90, 0x10, 0x19, 0x11, 0x00, 0x11, 0x11, 0x00, 0x10, 0x11, 0x01, 0x10, 0x11, 0x19, 0x11, 0x00, 
        0x90, 0x19, 0x10, 0x91, 0x01, 0x90, 0x19, 0x99, 0x00, 0x11, 0x91, 0x01, 0x11, 0x01, 0x91, 0x00, 
        0x99, 0x09, 0x09, 0x01, 0x10, 0x11, 0x91, 0x01, 0x10, 0x91, 0x99, 0x11, 0x10, 0x90, 0x91, 0x00, 
        0x91, 0x11, 0x00, 0x10, 0x11, 0x01, 0x10, 0x19, 0x19, 0x09, 0x10, 0x00, 0x99, 0x01, 0x01, 0x00, 
        0x91, 0x01, 0x19, 0x91, 0x19, 0x91, 0x11, 0x09, 0x10, 0x11, 0x00, 0x91, 0x00, 0x10, 0x90, 0x00, 
        0x99, 0x01, 0x11, 0x10, 0x09, 0x10, 0x10, 0x19, 0x09, 0x01, 0x91, 0x90, 0x11, 0x09, 0x11, 0x00, 
        0x90, 0x99, 0x11, 0x11, 0x11, 0x90, 0x19, 0x01, 0x19, 0x01, 0x91, 0x01, 0x01, 0x19, 0x09, 0x00, 
        0x91, 0x10, 0x11, 0x91, 0x99, 0x09, 0x09, 0x90, 0x11, 0x91, 0x01, 0x19, 0x11, 0x11, 0x91, 0x00, 
        0x91, 0x19, 0x01, 0x00, 0x11, 0x00, 0x91, 0x10, 0x11, 0x01, 0x11, 0x11, 0x10, 0x01, 0x11, 0x00, 
        0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x90 
    };

    uint32_t datasize = img->info.bmiHeader.biWidth * img->info.bmiHeader.biHeight * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    img->ciPixelArray = malloc(datasize);
    
    for (unsigned int i = 0; i < datasize; i++) {
        img->ciPixelArray[i] = (uint8_t) ciPixelArray[i];
    }

    return img;
}

bmp_image * bmp_8bpp_sample()
{
    bmp_image * img = malloc(sizeof(bmp_image));

    /**
     * BITMAPINFOHEADER 
     * - only format supported until this release
     */
    img->info.bmiHeader.biSize = 40;
    img->info.bmiHeader.biWidth = 256;
    img->info.bmiHeader.biHeight = 256;
    img->info.bmiHeader.biPlanes = 1;
    img->info.bmiHeader.biBitCount = BMP_8_BITS;
    img->info.bmiHeader.biCompression = BMP_BI_RGB;
    img->info.bmiHeader.biSizeImage = 0;
    img->info.bmiHeader.biXPelsPerMeter = 0;
    img->info.bmiHeader.biYPelsPerMeter = 0;
    img->info.bmiHeader.biClrUsed = 0;
    img->info.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - size of colour palette is set to maximum if 'biClrUsed' is zero
     *   according to 'biBitCount'
     */    
    uint32_t colourpalettesize = pow(2,img->info.bmiHeader.biBitCount) 
                    * sizeof(bmp_rgbquad);
    
    uint32_t datasize = img->info.bmiHeader.biWidth 
                    * img->info.bmiHeader.biHeight 
                    * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize;

    /**
     * COLOUR PALETTE
     * - images with 1bpp, 2bpp, 4bpp and 8bpp implement this field 
     */
    img->info.bmiColors = malloc(colourpalettesize);

    for (size_t i = 0; i < pow(2,img->info.bmiHeader.biBitCount); i++) {
        img->info.bmiColors[i].rgbBlue = i;
        img->info.bmiColors[i].rgbGreen = i;
        img->info.bmiColors[i].rgbRed = i;
        img->info.bmiColors[i].rgbReserved = 0;
    }

    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    uint32_t newline = 0;

    for (size_t i = 0; i < datasize; i++) {
        if (newline >= img->info.bmiHeader.biWidth) {
            newline = 0;
        }
        img->ciPixelArray[i] = newline;
        newline++;
    }

    return img;
}

bmp_image * bmp_16bpp_sample()
{
    bmp_image * img = malloc(sizeof(bmp_image));

    /**
     * BITMAPINFOHEADER 
     * - only format supported until this release
     */
    img->info.bmiHeader.biSize = 40;
    img->info.bmiHeader.biWidth = pow(2,5);
    img->info.bmiHeader.biHeight = pow(2,5);
    img->info.bmiHeader.biPlanes = 1;
    img->info.bmiHeader.biBitCount = BMP_16_BITS;
    img->info.bmiHeader.biCompression = BMP_BI_RGB;
    img->info.bmiHeader.biSizeImage = 0;
    img->info.bmiHeader.biXPelsPerMeter = 0;
    img->info.bmiHeader.biYPelsPerMeter = 0;
    img->info.bmiHeader.biClrUsed = 0;
    img->info.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - 16bpp with BI_RGB compression have no bmiColors member
     */    
    uint32_t colourpalettesize = 0;
    
    uint32_t datasize = img->info.bmiHeader.biWidth 
                    * img->info.bmiHeader.biHeight 
                    * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize;
    
    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    uint16_t * raw = (uint16_t *) img->ciPixelArray;

    for (uint16_t x = 0; x < img->info.bmiHeader.biWidth; x++)
    {
        for (uint16_t y = 0; y < img->info.bmiHeader.biHeight; y++)
        {
            raw[y * img->info.bmiHeader.biWidth + x] = x + (y << 10);
        }
    }

    return img;
}

bmp_image * bmp_32bpp_sample()
{
    bmp_image * img = malloc(sizeof(bmp_image));

    /**
     * BITMAPINFOHEADER 
     * - only format supported until this release
     */
    img->info.bmiHeader.biSize = 40;
    img->info.bmiHeader.biWidth = pow(2,8);
    img->info.bmiHeader.biHeight = pow(2,8);
    img->info.bmiHeader.biPlanes = 1;
    img->info.bmiHeader.biBitCount = BMP_32_BITS;
    img->info.bmiHeader.biCompression = BMP_BI_RGB;
    img->info.bmiHeader.biSizeImage = 0;
    img->info.bmiHeader.biXPelsPerMeter = 0;
    img->info.bmiHeader.biYPelsPerMeter = 0;
    img->info.bmiHeader.biClrUsed = 0;
    img->info.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - 32bpp with BI_RGB compression have no bmiColors member
     */    
    uint32_t colourpalettesize = 0;
    
    uint32_t datasize = img->info.bmiHeader.biWidth 
                    * img->info.bmiHeader.biHeight 
                    * img->info.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->info.bmiHeader.biSize 
                    + colourpalettesize;
    
    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    printf("\ndatasize: %u \n", datasize);
    printf("\nimg(%u, %u) \n", img->info.bmiHeader.biWidth, img->info.bmiHeader.biHeight);

    uint32_t * raw = (uint32_t *) img->ciPixelArray;

    for (uint32_t x = 0; x < img->info.bmiHeader.biWidth; x++)
    {
        for (uint32_t y = 0; y < img->info.bmiHeader.biHeight; y++)
        {
            raw[y * img->info.bmiHeader.biWidth + x] = x  + (y << 8);
        }
    }

    return img;
}