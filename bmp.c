/**
 * @file bmp.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief Bitmap C library
 * @version 0.7
 * @date 2022-04-06
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
    
    if (fread( &img->dib.bmiHeader, sizeof(bmp_infoheader), 1, fptr) != 1) 
        return bmp_cleanup(fptr, img);
    
    if (img->dib.bmiHeader.biSize >= BMP_V4HEADER)
    {
        if (fread( &img->dib.bmiv4Header, sizeof(bmp_v4header), 1, fptr) != 1) 
            return bmp_cleanup(fptr, img);
    }

    if (img->dib.bmiHeader.biSize >= BMP_V5HEADER)
    {
        if (fread( &img->dib.bmiv5Header, sizeof(bmp_v5header), 1, fptr) != 1) 
            return bmp_cleanup(fptr, img);
    }

    if (bmp_checkheaders(img) == 0) return bmp_cleanup(fptr, img);

    uint32_t palettesize = bmp_getpalettesize(img);

    switch (img->dib.bmiHeader.biBitCount) {
    case BMP_1_BIT:
    case BMP_4_BITS:
    case BMP_8_BITS:
        img->dib.bmiColors = malloc(palettesize);
        if (fread( img->dib.bmiColors, palettesize, 1, fptr) != 1) 
            return bmp_cleanup(fptr, img);
        break;
    case BMP_16_BITS:
    case BMP_32_BITS:
        if (img->dib.bmiHeader.biCompression == BMP_BI_BITFIELDS)
        {
            img->dib.bmiColors = malloc(palettesize);
            if (fread( img->dib.bmiColors, palettesize, 1, fptr) != 1) 
                return bmp_cleanup(fptr, img);
        }
        break;
    case BMP_24_BITS:
        // never expect a BMP color palette
    default:
        break;
    }

    uint32_t datasize = bmp_getdatasize(img);

    img->ciPixelArray = malloc(sizeof(uint8_t)*datasize);
    
    if (img->ciPixelArray == NULL) 
        return bmp_cleanup(fptr, img);

    if (fread(img->ciPixelArray, sizeof(uint8_t), datasize, fptr) != datasize) 
        return bmp_cleanup(fptr, img);

    fclose(fptr);

    return img;
}

int bmp_save(bmp_image * img, const char * filename)
{
    FILE * fptr = NULL;
    fptr = fopen(filename, "w");

    if (fptr == NULL) return 0;

    if (fwrite(&img->fileheader, sizeof(bmp_fileheader), 1, fptr) != 1) {
        fclose(fptr);
        return 0;
    }

    if (fwrite(&img->dib.bmiHeader, sizeof(bmp_infoheader), 1, fptr) != 1) {
        fclose(fptr);
        return 0;
    }

    if (img->dib.bmiHeader.biSize >= BMP_V4HEADER)
    {
        if (fwrite(&img->dib.bmiv4Header, sizeof(bmp_v4header), 1, fptr) != 1) {
            fclose(fptr);
            return 0;
        }
    }
    
    if (img->dib.bmiHeader.biSize >= BMP_V5HEADER)
    {
        if (fwrite(&img->dib.bmiv5Header, sizeof(bmp_v5header), 1, fptr) != 1) {
            fclose(fptr);
            return 0;
        }
    }
    
    uint32_t palettesize = bmp_getpalettesize(img);

    if (palettesize > 0)
    {
        if (fwrite(img->dib.bmiColors, palettesize, 1, fptr) != 1) {
            fclose(fptr);
            return 0;
        }
    }

    uint32_t datasize = bmp_getdatasize(img);

    uint32_t rows = img->dib.bmiHeader.biHeight;
    uint32_t columns = img->dib.bmiHeader.biWidth / 8;

    uint8_t * datapadded;

    if (img->dib.bmiHeader.biWidth % 8)
    {
        columns = columns + 1;
        columns = columns * 8;
        
        datapadded = malloc(sizeof(uint8_t)*rows*columns);

        for (uint32_t i = 0; i < rows*columns; i++) datapadded[i] = 0;
        
        for (uint32_t y = 0; y < rows; y++)
        {
            for (uint32_t x = 0; x < img->dib.bmiHeader.biWidth; x++)
            {
                datapadded[y*columns + x] = img->ciPixelArray[y*img->dib.bmiHeader.biWidth + x];
            }
        }

        if (fwrite(datapadded, sizeof(uint8_t), rows*columns, fptr) != rows*columns) {
            fclose(fptr);
            free(datapadded);
            return 0;
        }

        free(datapadded);
    }
    else
    {
        if (fwrite(img->ciPixelArray, sizeof(uint8_t), datasize, fptr) != datasize) {
            fclose(fptr);
            return 0;
        }
    }

    fclose(fptr);

    return 1;
}

uint8_t bmp_getpixelcolor(bmp_image * img, int x, int y, bmp_color color)
{
    switch (img->dib.bmiHeader.biBitCount)
    {
    case BMP_0_BITS:
    case BMP_1_BIT:
    case BMP_2_BITS:
    case BMP_4_BITS:
        //TODO: add support for these formats.
        return 0;
        break;
    case BMP_8_BITS:
        return img->ciPixelArray[y*img->dib.bmiHeader.biWidth+x];
        break;
    case BMP_16_BITS:
        //TODO: add support for this format.
        return 0;
        break;
    case BMP_24_BITS:
        return img->ciPixelArray[3*(y*img->dib.bmiHeader.biWidth+x)+color];
        break;
    case BMP_32_BITS:
        return img->ciPixelArray[4*(y*img->dib.bmiHeader.biWidth+x)+color];
        break;

    default:
        return 0;
        break;
    }
    // if (img->dib.bmiHeader.biBitCount == BMP_8_BITS) 
    //     return img->ciPixelArray[ y*img->dib.bmiHeader.biWidth + x ];
    // return img->ciPixelArray[3*(y*img->dib.bmiHeader.biWidth + x) + color];
}

uint8_t bmp_findgray(uint8_t red, uint8_t green, uint8_t blue)
{
    double gray = 0.2989 * red + 0.5870 * green + 0.1140 * blue;
    return  (uint8_t) gray;
}

bmp_image * bmp_rgb2gray(bmp_image * img, bmp_setncolours ncolours)
{
    if (img == NULL) return NULL;
    
    //TODO: extend support to convert 16bpp and 32bpp images
    if (img->dib.bmiHeader.biBitCount != BMP_24_BITS) return NULL;
    
    bmp_image * new = malloc(sizeof(bmp_image));
    if (new == NULL) return bmp_cleanup(NULL, new);

    new->dib.bmiHeader.biSize = BMP_INFOHEADER;
    new->dib.bmiHeader.biWidth = img->dib.bmiHeader.biWidth;
    new->dib.bmiHeader.biHeight = img->dib.bmiHeader.biHeight;
    new->dib.bmiHeader.biPlanes = BMP_DEFAULT_COLORPLANES;
    new->dib.bmiHeader.biBitCount = BMP_8_BITS;
    new->dib.bmiHeader.biCompression = BMP_BI_RGB;
    new->dib.bmiHeader.biSizeImage = new->dib.bmiHeader.biWidth 
                    * new->dib.bmiHeader.biHeight 
                    * new->dib.bmiHeader.biBitCount / BMP_8_BITS;
    new->dib.bmiHeader.biXPelsPerMeter = img->dib.bmiHeader.biXPelsPerMeter;
    new->dib.bmiHeader.biYPelsPerMeter = img->dib.bmiHeader.biYPelsPerMeter;
    new->dib.bmiHeader.biClrUsed = 0;
    new->dib.bmiHeader.biClrImportant = 0;

    uint32_t palettesize = pow(2, new->dib.bmiHeader.biBitCount) 
                    * sizeof(bmp_rgbquad);
    
    new->fileheader.bfType = BMP_FILETYPE_BM;
    new->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + new->dib.bmiHeader.biSize 
                    + palettesize 
                    + new->dib.bmiHeader.biSizeImage;
    
    new->fileheader.bfReserved1 = 0;
    new->fileheader.bfReserved2 = 0;
    
    new->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + new->dib.bmiHeader.biSize 
                    + palettesize;
    
    //TODO: add support to 4bpp, 2bpp and 1bpp generation

    new->dib.bmiColors = malloc(palettesize);

    if (ncolours == 0) ncolours = BMP_SET_256_COLOURS;

    uint32_t maxcolours = pow(2, new->dib.bmiHeader.biBitCount);
    uint32_t steps = (maxcolours/ncolours);

    for (uint32_t i = 0; i < maxcolours; i = i + steps) {
        for (uint8_t k = 0; k < steps; k++) {
            new->dib.bmiColors[i+k].rgbBlue = i;
            new->dib.bmiColors[i+k].rgbGreen = i;
            new->dib.bmiColors[i+k].rgbRed = i;
            new->dib.bmiColors[i+k].rgbReserved = 0;
        }
    }

    new->ciPixelArray = malloc(new->dib.bmiHeader.biSizeImage);

    for (size_t x = 0; x < new->dib.bmiHeader.biWidth; x++) {
        for (size_t y = 0; y < new->dib.bmiHeader.biHeight; y++) {

            uint8_t ReqG = bmp_getpixelcolor(img, x, y, BMP_COLOR_RED) == bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN);
            uint8_t GeqB = bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN) == bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE);
            uint8_t BeqR = bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE) == bmp_getpixelcolor(img, x, y, BMP_COLOR_RED);

            if (ReqG && GeqB && BeqR) {
                new->ciPixelArray[y*new->dib.bmiHeader.biWidth + x] = 
                    bmp_getpixelcolor(img, x, y, BMP_COLOR_RED);
            } else {
                new->ciPixelArray[y*new->dib.bmiHeader.biWidth + x] = 
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
    //TODO: add support for 16 and 32 bits per pixel images.
    if (img->dib.bmiHeader.biBitCount != BMP_24_BITS) return;
    
    //TODO: add support for compressed images.
    if (img->dib.bmiHeader.biCompression != BMP_BI_RGB) return;
    
    uint32_t datasize = bmp_getdatasize(img);

    for (unsigned int pixel = color; pixel < datasize; pixel++) {
        if ((pixel % 3) != color) {
            img->ciPixelArray[pixel] = 0;
        }
    }
}

void bmp_invert(bmp_image * img)
{
    uint32_t datasize = bmp_getdatasize(img);
    
    switch (img->dib.bmiHeader.biBitCount)
    {
    case BMP_1_BIT:
    case BMP_2_BITS:
    case BMP_4_BITS:
    case BMP_16_BITS:
        //TODO: implement this behavior.
        break;
    case BMP_32_BITS:
        for (uint32_t i = 0; i < datasize; i++) {
            if ((i+1)%4) img->ciPixelArray[i] = ~img->ciPixelArray[i];
        }
        break;
    case BMP_8_BITS:
    case BMP_24_BITS:
        for (uint32_t i = 0; i < datasize; i++) {
            img->ciPixelArray[i] = ~img->ciPixelArray[i];
        }
        break;
    default:
        break;
    }
}

void bmp_addpad(bmp_image * img, uint32_t rows, uint32_t columns, bmp_padtype type)
{
    bmp_padv(img, rows, type);
    bmp_padh(img, columns, type);
}

void bmp_padh(bmp_image * img, uint32_t num, bmp_padtype type)
{
    if (img->dib.bmiHeader.biBitCount != BMP_8_BITS) return;
    //TODO: add support to other bit per pixel configurations.

    uint32_t newWidth = img->dib.bmiHeader.biWidth + 2*num;

    uint32_t datasize = newWidth 
                    * img->dib.bmiHeader.biHeight 
                    * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

    uint8_t * newPixelArray = malloc(datasize);

    for (uint32_t y = 0; y < img->dib.bmiHeader.biHeight; y++)
    {
        for (uint32_t x = 0; x < num; x++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->dib.bmiHeader.biWidth + 0];
                break;
            
            default:
                return;
                break;
            }
        }

        for (uint32_t x = num; x < img->dib.bmiHeader.biWidth + num; x++)
        {
            newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->dib.bmiHeader.biWidth + x - num];
        }

        for (uint32_t x = img->dib.bmiHeader.biWidth + num; x < newWidth; x++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * newWidth + x] = img->ciPixelArray[y * img->dib.bmiHeader.biWidth + img->dib.bmiHeader.biWidth - 1];
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

    img->fileheader.bfSize = img->fileheader.bfSize - img->dib.bmiHeader.biSizeImage + datasize;
    img->dib.bmiHeader.biSizeImage = datasize;
    img->dib.bmiHeader.biWidth = newWidth;
}

void bmp_padv(bmp_image * img, uint32_t num, bmp_padtype type)
{
    if (img->dib.bmiHeader.biBitCount != BMP_8_BITS) return;
    //TODO: add support to other bit per pixel configurations.

    uint32_t newHeight = img->dib.bmiHeader.biHeight + 2*num;

    uint32_t datasize = img->dib.bmiHeader.biWidth 
                    * newHeight 
                    * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

    uint8_t * newPixelArray = malloc(datasize);

    for (uint32_t x = 0; x < img->dib.bmiHeader.biWidth; x++)
    {
        for (uint32_t y = 0; y < num; y++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * img->dib.bmiHeader.biWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * img->dib.bmiHeader.biWidth + x] = img->ciPixelArray[0 + x];
                break;
            
            default:
                return;
                break;
            }
        }

        for (uint32_t y = num; y < img->dib.bmiHeader.biHeight + num; y++)
        {
            newPixelArray[y * img->dib.bmiHeader.biWidth + x] = img->ciPixelArray[(y - num) * img->dib.bmiHeader.biWidth + x];
        }

        for (uint32_t y = img->dib.bmiHeader.biHeight + num; y < newHeight; y++)
        {
            switch (type)
            {
            case BMP_PADTYPE_ZEROS:
                newPixelArray[y * img->dib.bmiHeader.biWidth + x] = 0;
                break;
            
            case BMP_PADTYPE_REPLICATE:
                newPixelArray[y * img->dib.bmiHeader.biWidth + x] = img->ciPixelArray[(img->dib.bmiHeader.biHeight - 1) * img->dib.bmiHeader.biWidth + x];
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
    
    img->fileheader.bfSize = img->fileheader.bfSize - img->dib.bmiHeader.biSizeImage + datasize;
    img->dib.bmiHeader.biSizeImage = datasize;
    img->dib.bmiHeader.biHeight = newHeight;
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

    printf("size:             \t%u  \t0x%x\n", img->dib.bmiHeader.biSize, img->dib.bmiHeader.biSize);
    printf("width:            \t%u  \t0x%x\n", img->dib.bmiHeader.biWidth, img->dib.bmiHeader.biWidth);
    printf("height:           \t%u  \t0x%x\n", img->dib.bmiHeader.biHeight, img->dib.bmiHeader.biHeight);
    printf("planes:           \t%u  \t0x%x\n", img->dib.bmiHeader.biPlanes, img->dib.bmiHeader.biPlanes);
    printf("bits per pixel:   \t%u  \t0x%x\n", img->dib.bmiHeader.biBitCount, img->dib.bmiHeader.biBitCount);
    printf("compression:      \t%u  \t0x%x\n", img->dib.bmiHeader.biCompression, img->dib.bmiHeader.biCompression);
    printf("imagesize:        \t%u  \t0x%x\n", img->dib.bmiHeader.biSizeImage, img->dib.bmiHeader.biSizeImage);
    printf("xresolution:      \t%u  \t0x%x\n", img->dib.bmiHeader.biXPelsPerMeter, img->dib.bmiHeader.biXPelsPerMeter);
    printf("yresolution:      \t%u  \t0x%x\n", img->dib.bmiHeader.biYPelsPerMeter, img->dib.bmiHeader.biYPelsPerMeter);
    printf("colours used:     \t%u  \t0x%x\n", img->dib.bmiHeader.biClrUsed, img->dib.bmiHeader.biClrUsed);
    printf("main colours:     \t%u  \t0x%x\n", img->dib.bmiHeader.biClrImportant, img->dib.bmiHeader.biClrImportant);

    if (img->dib.bmiHeader.biSize >= BMP_V4HEADER)
    {
        printf("\n");
        printf("From BITMAPV4HEADER: \n");

        printf("red mask:       \t0x%x\n", img->dib.bmiv4Header.bV4RedMask);
        printf("green mask:     \t0x%x\n", img->dib.bmiv4Header.bV4GreenMask);
        printf("blue mask:      \t0x%x\n", img->dib.bmiv4Header.bV4BlueMask);
        printf("alpha mask:     \t0x%x\n", img->dib.bmiv4Header.bV4AlphaMask);
        printf("color space:    \t0x%x\n", img->dib.bmiv4Header.bV4CSType);
        printf("red (x):        \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzX);
        printf("red (y):        \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzY);
        printf("red (z):        \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzZ);
        printf("green (x):      \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzX);
        printf("green (y):      \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzY);
        printf("green (z):      \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzZ);
        printf("blue (x):       \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzX);
        printf("blue (y):       \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzY);
        printf("blue (z):       \t0x%x\n", img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzZ);
        printf("gamma red:      \t0x%x\n", img->dib.bmiv4Header.bV4GammaRed);
        printf("gamma green:    \t0x%x\n", img->dib.bmiv4Header.bV4GammaGreen);
        printf("gamma blue:     \t0x%x\n", img->dib.bmiv4Header.bV4GammaBlue);
    }
    
    if (img->dib.bmiHeader.biSize >= BMP_V5HEADER)
    {
        printf("\n");
        printf("From BITMAPV5HEADER: \n");

        printf("intent:         \t%u    \t0x%x\n", img->dib.bmiv5Header.bV5Intent, img->dib.bmiv5Header.bV5Intent);
        printf("profile data:   \t%u    \t0x%x\n", img->dib.bmiv5Header.bV5ProfileData, img->dib.bmiv5Header.bV5ProfileData);
        printf("profile size:   \t%u    \t0x%x\n", img->dib.bmiv5Header.bV5ProfileSize, img->dib.bmiv5Header.bV5ProfileSize);
        printf("reserved:       \t%u    \t0x%x\n", img->dib.bmiv5Header.bV5Reserved, img->dib.bmiv5Header.bV5Reserved);
    }

    printf("\n");
}

void bmp_printpixel(bmp_image * img, int x, int y)
{
    switch (img->dib.bmiHeader.biBitCount)
    {
    case BMP_1_BIT:
    case BMP_2_BITS:
    case BMP_4_BITS:
    case BMP_16_BITS:
        //TODO: implement support for these bpp.
        break;
    case BMP_8_BITS:
        printf(
            "img[%i][%i] = (%u)\n", x, y, 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_RED)
        );
        break;
    case BMP_24_BITS:
        printf(
            "img[%i][%i] = (%u, %u, %u)\n", x, y, 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_RED), 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN), 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE)
        );
        break;
    case BMP_32_BITS:
        printf(
            "img[%i][%i] = (%u, %u, %u, %u)\n", x, y, 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_RED), 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_GREEN), 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_BLUE), 
            bmp_getpixelcolor(img, x, y, BMP_COLOR_ALPHA)
        );
        break;
    default:
        break;
    }
}

bmp_image * bmp_cleanup(FILE * fptr , bmp_image * img)
{
    if (fptr != NULL) fclose(fptr);

    if (img != NULL) {
        if (img->dib.bmiColors != NULL) free(img->dib.bmiColors);
        if (img->ciPixelArray != NULL) free(img->ciPixelArray);
        free(img);
    }

    return NULL;
}

int bmp_checkheaders(bmp_image * img)
{
    if (img->fileheader.bfType != BMP_FILETYPE_BM)
        return 0;

    switch (img->dib.bmiHeader.biBitCount) {
    case BMP_0_BITS: break;
    case BMP_1_BIT: break;
    case BMP_2_BITS: break;
    case BMP_4_BITS: break;
    case BMP_8_BITS: break;
    case BMP_16_BITS: break;
    case BMP_24_BITS: break;
    case BMP_32_BITS: break;
    default : return 0;
    }

    if (img->dib.bmiHeader.biPlanes != BMP_DEFAULT_COLORPLANES) 
        return 0;

    switch (img->dib.bmiHeader.biCompression) {
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

    if (img->dib.bmiHeader.biSize >= BMP_V4HEADER)
    {
        switch (img->dib.bmiv4Header.bV4CSType)
        {
        case BMP_LCS_CALIBRATED_RGB: break;
        case BMP_LCS_sRGB: break;
        case BMP_LCS_WINDOWS_COLOR_SPACE: break;
        case BMP_LCS_PROFILE_LINKED: break;
        case BMP_LCS_PROFILE_EMBEDDED: break;
        default: return 0;
        }
    }
    
    if (img->dib.bmiHeader.biSize >= BMP_V5HEADER)
    {
        switch (img->dib.bmiv5Header.bV5Intent)
        {
        case BMP_LCS_GM_ABS_COLORIMETRIC: break;
        case BMP_LCS_GM_BUSINESS: break;
        case BMP_LCS_GM_GRAPHICS: break;
        case BMP_LCS_GM_IMAGES: break;
        default: return 0;
        }
    }
    
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

    img->dib.bmiHeader.biSize = 40;
    img->dib.bmiHeader.biWidth = 32;
    img->dib.bmiHeader.biHeight = 32;
    img->dib.bmiHeader.biPlanes = 1;
    img->dib.bmiHeader.biBitCount = BMP_4_BITS;
    img->dib.bmiHeader.biCompression = BMP_BI_RGB;
    img->dib.bmiHeader.biSizeImage = 0;
    img->dib.bmiHeader.biXPelsPerMeter = 0;
    img->dib.bmiHeader.biYPelsPerMeter = 0;
    img->dib.bmiHeader.biClrUsed = 0;
    img->dib.bmiHeader.biClrImportant = 0;

    img->dib.bmiColors = malloc(sizeof(bmp_rgbquad)*pow(2,img->dib.bmiHeader.biBitCount));

    img->dib.bmiColors[0].rgbBlue = 0x00; img->dib.bmiColors[0].rgbGreen = 0x00; img->dib.bmiColors[0].rgbRed = 0x00; img->dib.bmiColors[0].rgbReserved = 0x00;
    img->dib.bmiColors[1].rgbBlue = 0x00; img->dib.bmiColors[1].rgbGreen = 0x00; img->dib.bmiColors[1].rgbRed = 0x80; img->dib.bmiColors[1].rgbReserved = 0x00;
    img->dib.bmiColors[2].rgbBlue = 0x00; img->dib.bmiColors[2].rgbGreen = 0x80; img->dib.bmiColors[2].rgbRed = 0x00; img->dib.bmiColors[2].rgbReserved = 0x00;
    img->dib.bmiColors[3].rgbBlue = 0x00; img->dib.bmiColors[3].rgbGreen = 0x80; img->dib.bmiColors[3].rgbRed = 0x80; img->dib.bmiColors[3].rgbReserved = 0x00;
    
    img->dib.bmiColors[4].rgbBlue = 0x80; img->dib.bmiColors[4].rgbGreen = 0x00; img->dib.bmiColors[4].rgbRed = 0x00; img->dib.bmiColors[4].rgbReserved = 0x00;
    img->dib.bmiColors[5].rgbBlue = 0x80; img->dib.bmiColors[5].rgbGreen = 0x00; img->dib.bmiColors[5].rgbRed = 0x80; img->dib.bmiColors[5].rgbReserved = 0x00;
    img->dib.bmiColors[6].rgbBlue = 0x80; img->dib.bmiColors[6].rgbGreen = 0x80; img->dib.bmiColors[6].rgbRed = 0x00; img->dib.bmiColors[6].rgbReserved = 0x00;
    img->dib.bmiColors[7].rgbBlue = 0x80; img->dib.bmiColors[7].rgbGreen = 0x80; img->dib.bmiColors[7].rgbRed = 0x80; img->dib.bmiColors[7].rgbReserved = 0x00;
    
    img->dib.bmiColors[8].rgbBlue = 0xc0; img->dib.bmiColors[8].rgbGreen = 0xc0; img->dib.bmiColors[8].rgbRed = 0xc0; img->dib.bmiColors[8].rgbReserved = 0x00;
    img->dib.bmiColors[9].rgbBlue = 0x00; img->dib.bmiColors[9].rgbGreen = 0x00; img->dib.bmiColors[9].rgbRed = 0xff; img->dib.bmiColors[9].rgbReserved = 0x00;
    img->dib.bmiColors[10].rgbBlue = 0x00; img->dib.bmiColors[10].rgbGreen = 0xff; img->dib.bmiColors[10].rgbRed = 0x00; img->dib.bmiColors[10].rgbReserved = 0x00;
    img->dib.bmiColors[11].rgbBlue = 0x00; img->dib.bmiColors[11].rgbGreen = 0xff; img->dib.bmiColors[11].rgbRed = 0xff; img->dib.bmiColors[11].rgbReserved = 0x00;
    
    img->dib.bmiColors[12].rgbBlue = 0xff; img->dib.bmiColors[12].rgbGreen = 0x00; img->dib.bmiColors[12].rgbRed = 0x00; img->dib.bmiColors[12].rgbReserved = 0x00;
    img->dib.bmiColors[13].rgbBlue = 0xff; img->dib.bmiColors[13].rgbGreen = 0x00; img->dib.bmiColors[13].rgbRed = 0xff; img->dib.bmiColors[13].rgbReserved = 0x00;
    img->dib.bmiColors[14].rgbBlue = 0xff; img->dib.bmiColors[14].rgbGreen = 0xff; img->dib.bmiColors[14].rgbRed = 0x00; img->dib.bmiColors[14].rgbReserved = 0x00;
    img->dib.bmiColors[15].rgbBlue = 0xff; img->dib.bmiColors[15].rgbGreen = 0xff; img->dib.bmiColors[15].rgbRed = 0xff; img->dib.bmiColors[15].rgbReserved = 0x00;

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

    uint32_t datasize = img->dib.bmiHeader.biWidth * img->dib.bmiHeader.biHeight * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

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
    img->dib.bmiHeader.biSize = 40;
    img->dib.bmiHeader.biWidth = 256;
    img->dib.bmiHeader.biHeight = 256;
    img->dib.bmiHeader.biPlanes = 1;
    img->dib.bmiHeader.biBitCount = BMP_8_BITS;
    img->dib.bmiHeader.biCompression = BMP_BI_RGB;
    img->dib.bmiHeader.biSizeImage = 0;
    img->dib.bmiHeader.biXPelsPerMeter = 0;
    img->dib.bmiHeader.biYPelsPerMeter = 0;
    img->dib.bmiHeader.biClrUsed = 0;
    img->dib.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - size of colour palette is set to maximum if 'biClrUsed' is zero
     *   according to 'biBitCount'
     */    
    uint32_t colourpalettesize = pow(2,img->dib.bmiHeader.biBitCount) 
                    * sizeof(bmp_rgbquad);
    
    uint32_t datasize = img->dib.bmiHeader.biWidth 
                    * img->dib.bmiHeader.biHeight 
                    * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize;

    /**
     * COLOUR PALETTE
     * - images with 1bpp, 2bpp, 4bpp and 8bpp implement this field 
     */
    img->dib.bmiColors = malloc(colourpalettesize);

    for (size_t i = 0; i < pow(2,img->dib.bmiHeader.biBitCount); i++) {
        img->dib.bmiColors[i].rgbBlue = i;
        img->dib.bmiColors[i].rgbGreen = i;
        img->dib.bmiColors[i].rgbRed = i;
        img->dib.bmiColors[i].rgbReserved = 0;
    }

    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    uint32_t newline = 0;

    for (size_t i = 0; i < datasize; i++) {
        if (newline >= img->dib.bmiHeader.biWidth) {
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
    img->dib.bmiHeader.biSize = 40;
    img->dib.bmiHeader.biWidth = pow(2,5);
    img->dib.bmiHeader.biHeight = pow(2,5);
    img->dib.bmiHeader.biPlanes = 1;
    img->dib.bmiHeader.biBitCount = BMP_16_BITS;
    img->dib.bmiHeader.biCompression = BMP_BI_RGB;
    img->dib.bmiHeader.biSizeImage = 0;
    img->dib.bmiHeader.biXPelsPerMeter = 0;
    img->dib.bmiHeader.biYPelsPerMeter = 0;
    img->dib.bmiHeader.biClrUsed = 0;
    img->dib.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - 16bpp with BI_RGB compression have no bmiColors member
     */    
    uint32_t colourpalettesize = 0;
    
    uint32_t datasize = img->dib.bmiHeader.biWidth 
                    * img->dib.bmiHeader.biHeight 
                    * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize;
    
    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    uint16_t * raw = (uint16_t *) img->ciPixelArray;

    for (uint16_t x = 0; x < img->dib.bmiHeader.biWidth; x++)
    {
        for (uint16_t y = 0; y < img->dib.bmiHeader.biHeight; y++)
        {
            raw[y * img->dib.bmiHeader.biWidth + x] = x + (y << 10);
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
    img->dib.bmiHeader.biSize = 40;
    img->dib.bmiHeader.biWidth = pow(2,8);
    img->dib.bmiHeader.biHeight = pow(2,8);
    img->dib.bmiHeader.biPlanes = 1;
    img->dib.bmiHeader.biBitCount = BMP_32_BITS;
    img->dib.bmiHeader.biCompression = BMP_BI_RGB;
    img->dib.bmiHeader.biSizeImage = 0;
    img->dib.bmiHeader.biXPelsPerMeter = 0;
    img->dib.bmiHeader.biYPelsPerMeter = 0;
    img->dib.bmiHeader.biClrUsed = 0;
    img->dib.bmiHeader.biClrImportant = 0;

    /**
     * BITMAPFILEHEADER
     * - use BITMAPINFOHEADER data to build it
     * - 32bpp with BI_RGB compression have no bmiColors member
     */    
    uint32_t colourpalettesize = 0;
    
    uint32_t datasize = img->dib.bmiHeader.biWidth 
                    * img->dib.bmiHeader.biHeight 
                    * img->dib.bmiHeader.biBitCount / BMP_8_BITS;

    img->fileheader.bfType = BMP_FILETYPE_BM;
    img->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize 
                    + datasize;
    
    img->fileheader.bfReserved1 = 0;
    img->fileheader.bfReserved2 = 0;
    
    img->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + img->dib.bmiHeader.biSize 
                    + colourpalettesize;
    
    /**
     * PIXEL ARRAY
     * - raw data of the image
     */
    img->ciPixelArray = malloc(datasize);

    uint32_t * raw = (uint32_t *) img->ciPixelArray;

    for (uint32_t x = 0; x < img->dib.bmiHeader.biWidth; x++)
    {
        for (uint32_t y = 0; y < img->dib.bmiHeader.biHeight; y++)
        {
            raw[y * img->dib.bmiHeader.biWidth + x] = x  + (y << 8);
        }
    }

    return img;
}

uint32_t bmp_getfilesize(bmp_image * img)
{
    return img->fileheader.bfSize;
}

uint32_t bmp_getoffset(bmp_image * img)
{
    return img->fileheader.bfOffBits;
}

uint32_t bmp_getdatasize(bmp_image * img)
{
    return img->fileheader.bfSize - img->fileheader.bfOffBits;
}

uint32_t bmp_getdibformat(bmp_image * img)
{
    switch (img->dib.bmiHeader.biSize)
    {
    case BMP_INFOHEADER: return BMP_INFOHEADER;
    case BMP_V4HEADER: return BMP_V4HEADER;
    case BMP_V5HEADER: return BMP_V5HEADER;
    default:
        return BMP_UNKNOWN_HEADER;
    }
}

uint16_t bmp_getbitcount(bmp_image * img)
{
    return img->dib.bmiHeader.biBitCount;
}

uint32_t bmp_getpalettesize(bmp_image * img)
{
    switch (img->dib.bmiHeader.biBitCount)
    {
    case BMP_0_BITS:
    case BMP_1_BIT:
    case BMP_2_BITS:
    case BMP_4_BITS:
    case BMP_8_BITS:
        if (img->dib.bmiHeader.biClrUsed == 0) {
            return sizeof(bmp_rgbquad) * pow(2, img->dib.bmiHeader.biBitCount);
        } else {
            return sizeof(bmp_rgbquad) * img->dib.bmiHeader.biClrUsed;
        }
        break;
    case BMP_16_BITS:
        return sizeof(bmp_rgbquad) * 3;
        break;
    case BMP_32_BITS:
        return sizeof(bmp_rgbquad) * 4;
        break;
    case BMP_24_BITS:
        // never expect color palette
    default: return 0;
    }
}

uint32_t bmp_getcompression(bmp_image * img)
{
    switch (img->dib.bmiHeader.biCompression)
    {
    case BMP_BI_RGB: return BMP_BI_RGB;
    case BMP_BI_RLE8: return BMP_BI_RLE8;
    case BMP_BI_RLE4: return BMP_BI_RLE4;
    case BMP_BI_BITFIELDS: return BMP_BI_BITFIELDS;
    case BMP_BI_JPEG: return BMP_BI_JPEG;
    case BMP_BI_PNG: return BMP_BI_PNG;
    case BMP_BI_ALPHABITFIELDS: return BMP_BI_ALPHABITFIELDS;
    case BMP_BI_CMYK: return BMP_BI_CMYK;
    case BMP_BI_CMYKRLE8: return BMP_BI_CMYKRLE8;
    case BMP_BI_CMYKRLE4: return BMP_BI_CMYKRLE4;
    default:
        return BMP_UNKNOWN_COMPRESSION;
    }
}

uint32_t bmp_getnpixels(bmp_image * img)
{
    return img->dib.bmiHeader.biWidth * img->dib.bmiHeader.biHeight;
}

uint32_t bmp_getncolors(bmp_image * img)
{
    switch (img->dib.bmiHeader.biBitCount)
    {
    case BMP_0_BITS:
    case BMP_1_BIT:
    case BMP_2_BITS:
    case BMP_4_BITS:
    case BMP_8_BITS:
        if (img->dib.bmiHeader.biClrUsed == 0) {
            return pow(2, img->dib.bmiHeader.biBitCount);
        } else {
            return img->dib.bmiHeader.biClrUsed;
        }
        break;
    case BMP_16_BITS:
        return 3;
        break;
    case BMP_32_BITS:
        return 4;
        break;
    case BMP_24_BITS:
        // never expect color palette
    default: return 0;
    }
}

void bmp_cpdibs(bmp_image * new, bmp_image * img)
{
    if (new == NULL) return;
    if (img == NULL) return;

    new->dib.bmiHeader.biSize = img->dib.bmiHeader.biSize;
    new->dib.bmiHeader.biWidth = img->dib.bmiHeader.biWidth;
    new->dib.bmiHeader.biHeight = img->dib.bmiHeader.biHeight;
    new->dib.bmiHeader.biPlanes = img->dib.bmiHeader.biPlanes;
    new->dib.bmiHeader.biBitCount = img->dib.bmiHeader.biBitCount;
    new->dib.bmiHeader.biCompression = img->dib.bmiHeader.biCompression;
    new->dib.bmiHeader.biXPelsPerMeter = img->dib.bmiHeader.biXPelsPerMeter;
    new->dib.bmiHeader.biYPelsPerMeter = img->dib.bmiHeader.biYPelsPerMeter;
    new->dib.bmiHeader.biClrUsed = img->dib.bmiHeader.biClrUsed;
    new->dib.bmiHeader.biClrImportant = img->dib.bmiHeader.biClrImportant;

    if (new->dib.bmiHeader.biSize >= BMP_V4HEADER)
    {
        new->dib.bmiv4Header.bV4RedMask = img->dib.bmiv4Header.bV4RedMask;
        new->dib.bmiv4Header.bV4GreenMask = img->dib.bmiv4Header.bV4GreenMask;
        new->dib.bmiv4Header.bV4BlueMask = img->dib.bmiv4Header.bV4BlueMask;
        new->dib.bmiv4Header.bV4AlphaMask = img->dib.bmiv4Header.bV4AlphaMask;
        new->dib.bmiv4Header.bV4CSType = img->dib.bmiv4Header.bV4CSType;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzX = img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzX;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzY = img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzY;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzZ = img->dib.bmiv4Header.bV4Endpoints.ciexyzRed.ciexyzZ;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzX = img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzX;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzY = img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzY;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzZ = img->dib.bmiv4Header.bV4Endpoints.ciexyzGreen.ciexyzZ;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzX = img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzX;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzY = img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzY;
        new->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzZ = img->dib.bmiv4Header.bV4Endpoints.ciexyzBlue.ciexyzZ;
        new->dib.bmiv4Header.bV4GammaRed = img->dib.bmiv4Header.bV4GammaRed;
        new->dib.bmiv4Header.bV4GammaGreen = img->dib.bmiv4Header.bV4GammaGreen;
        new->dib.bmiv4Header.bV4GammaBlue = img->dib.bmiv4Header.bV4GammaBlue;
    }

    if (new->dib.bmiHeader.biSize >= BMP_V5HEADER)
    {
        new->dib.bmiv5Header.bV5Intent = img->dib.bmiv5Header.bV5Intent;
        new->dib.bmiv5Header.bV5ProfileData = img->dib.bmiv5Header.bV5ProfileData;
        new->dib.bmiv5Header.bV5ProfileSize = img->dib.bmiv5Header.bV5ProfileSize;
        new->dib.bmiv5Header.bV5Reserved = img->dib.bmiv5Header.bV5Reserved;
    }
}

bmp_image * bmp_rle8decoder(bmp_image * img)
{
    if (img == NULL) return NULL;
    if (bmp_getcompression(img) != BMP_BI_RLE8) return NULL;

    bmp_image * new = malloc(sizeof(bmp_image));

    bmp_cpdibs(new, img);
    
    new->dib.bmiHeader.biSize = BMP_INFOHEADER;
    new->dib.bmiHeader.biCompression = BMP_BI_RGB;
    new->dib.bmiHeader.biSizeImage = new->dib.bmiHeader.biWidth
                    * new->dib.bmiHeader.biHeight
                    * new->dib.bmiHeader.biBitCount / BMP_8_BITS;
    
    new->dib.bmiHeader.biClrUsed = 0;
    new->dib.bmiHeader.biClrImportant = 0;

    uint32_t palettesize = bmp_getpalettesize(img);
    
    new->fileheader.bfType = BMP_FILETYPE_BM;
    new->fileheader.bfSize = BMP_FILEHEADER_SIZE 
                    + new->dib.bmiHeader.biSize 
                    + palettesize 
                    + new->dib.bmiHeader.biSizeImage;
    
    new->fileheader.bfReserved1 = 0;
    new->fileheader.bfReserved2 = 0;
    
    new->fileheader.bfOffBits = BMP_FILEHEADER_SIZE 
                    + new->dib.bmiHeader.biSize 
                    + palettesize;

    if (palettesize > 0)
    {
        new->dib.bmiColors = malloc(palettesize);
        
        uint8_t * ncptr = (uint8_t *) new->dib.bmiColors;
        uint8_t * icptr = (uint8_t *) img->dib.bmiColors;

        for (uint32_t i = 0; i < palettesize; i++) {
            ncptr[i] = icptr[i];
        }
    }

    uint32_t datasize = bmp_getdatasize(img);
    
    bmp_rle8duo * duos = malloc(sizeof(bmp_rle8duo)*datasize/2);
    
    for (uint32_t i = 0; i < datasize/2; i++)
    {
        duos[i].count = img->ciPixelArray[2*i+0];
        duos[i].value = img->ciPixelArray[2*i+1];
    }
    
    new->ciPixelArray = malloc(new->dib.bmiHeader.biSizeImage);
    
    uint32_t index = 0;
    
    for (uint32_t pixel = 0; pixel < new->dib.bmiHeader.biSizeImage;)
    {
        if (duos[index+1].count == 0)
        {
            if (duos[index+1].value == 1) break;
            if (duos[index+1].value == 0) {
                if (duos[index].count > 0) duos[index].count -= 1;
            }
        }

        while (duos[index].count > 0)
        {
            new->ciPixelArray[pixel] = duos[index].value;
            pixel += 1; 
            duos[index].count -= 1;
        }

        index = index + 1;
    }

    free(duos);

    return new;
}