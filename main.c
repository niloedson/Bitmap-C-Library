/**
 * @file main.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief 
 * @version 3.7
 * @date 2022-03-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "main.h"

typedef enum padtype {
    PADTYPE_ZEROS,
    PADTYPE_REPLICATE
} padtype;

void addpad(bmp_image * img, uint32_t rows, uint32_t columns, padtype type);
void padh(bmp_image * img, uint32_t num, padtype type);
void padv(bmp_image * img, uint32_t num, padtype type);

void main()
{
    /* code */

    bmp_image * img = bmp_read("./samples/monarchs.bmp");

    bmp_image * grayed = bmp_rgb2gray(img, BMP_NCOLOURS_256);

    addpad(grayed, 50, 50, PADTYPE_REPLICATE);
    addpad(grayed, 50, 50, PADTYPE_ZEROS);

    bmp_save(grayed, "monarchs-grayed-padded.bmp");

    bmp_cleanup(NULL, grayed);

    bmp_cleanup(NULL, img);
}

void addpad(bmp_image * img, uint32_t rows, uint32_t columns, padtype type)
{
    padh(img, columns, type);
    padv(img, rows, type);
}

void padh(bmp_image * img, uint32_t num, padtype type)
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
            case PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case PADTYPE_REPLICATE:
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
            case PADTYPE_ZEROS:
                newPixelArray[y * newWidth + x] = 0;
                break;
            
            case PADTYPE_REPLICATE:
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

void padv(bmp_image * img, uint32_t num, padtype type)
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
            case PADTYPE_ZEROS:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = 0;
                break;
            
            case PADTYPE_REPLICATE:
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
            case PADTYPE_ZEROS:
                newPixelArray[y * img->info.bmiHeader.biWidth + x] = 0;
                break;
            
            case PADTYPE_REPLICATE:
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