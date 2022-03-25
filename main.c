#include <stdio.h>

#include "bmp.h"

typedef enum number_of_colors {
    NCOLORS_256 = 256,
    NCOLORS_128 = 128,
    NCOLORS_64 = 64,
    NCOLORS_32 = 32,
    NCOLORS_16 = 16,
    NCOLORS_8 = 8,
    NCOLORS_4 = 4,
    NCOLORS_2 = 2
} number_of_colors;

bmp_image * convert_24bpp_to_graypalette(bmp_image * origin, number_of_colors ncolours);
bmp_image * convert_24bpp_to_8bpp(bmp_image * origin);

void generate_gradient_sample();

void main()
{
    /* application code */
    bmp_image * img = bmp_read("drip-bottle-256.bmp");

    // bmp_details(img);

    // bmp_image * new = convert_24bpp_to_8bpp(img);
    bmp_image * new = convert_24bpp_to_graypalette(img, NCOLORS_64);

    bmp_details(new);

    // bmp_save(new, "drip-bottle-256-8bpp.bmp");
    bmp_save(new, "drip-bottle-64.bmp");

    bmp_cleanup(NULL, img);
    bmp_cleanup(NULL, new);
}

bmp_image * convert_24bpp_to_graypalette(bmp_image * origin, number_of_colors ncolours)
{
    if (origin == NULL) return NULL;
    if (origin->info.bmiHeader.biBitCount != BMP_24_BITS) return NULL;

    bmp_image * new = malloc(sizeof(bmp_image));
    if (new == NULL) return bmp_cleanup(NULL, new);
    
    new->info.bmiHeader.biSize = BMP_INFOHEADER;
    new->info.bmiHeader.biWidth = origin->info.bmiHeader.biWidth;
    new->info.bmiHeader.biHeight = origin->info.bmiHeader.biHeight;
    new->info.bmiHeader.biPlanes = BMP_DEFAULT_COLORPLANES;
    new->info.bmiHeader.biBitCount = BMP_8_BITS;
    new->info.bmiHeader.biCompression = BMP_BI_RGB;
    new->info.bmiHeader.biSizeImage = new->info.bmiHeader.biWidth 
                    * new->info.bmiHeader.biHeight 
                    * new->info.bmiHeader.biBitCount / BMP_8_BITS;
    new->info.bmiHeader.biXPelsPerMeter = origin->info.bmiHeader.biXPelsPerMeter;
    new->info.bmiHeader.biYPelsPerMeter = origin->info.bmiHeader.biYPelsPerMeter;
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
    
    new->info.bmiColours = malloc(colourpalettesize);

    uint32_t maxcolours = pow(2, new->info.bmiHeader.biBitCount);
    uint32_t steps = (maxcolours/ncolours);

    for (uint32_t i = 0; i < maxcolours; i = i + steps) {
        for (uint8_t k = 0; k < steps; k++) {
            new->info.bmiColours[i+k].rgbBlue = i;
            new->info.bmiColours[i+k].rgbGreen = i;
            new->info.bmiColours[i+k].rgbRed = i;
            new->info.bmiColours[i+k].rgbReserved = 0;
        }
    }

    new->ciPixelArray = malloc(new->info.bmiHeader.biSizeImage);

    for (size_t x = 0; x < new->info.bmiHeader.biWidth; x++) {
        for (size_t y = 0; y < new->info.bmiHeader.biHeight; y++) {

            uint8_t ReqG = bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN);
            uint8_t GeqB = bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE);
            uint8_t BeqR = bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED);

            if (ReqG && GeqB && BeqR) {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED);
            } else {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_findgray(
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED),
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN),
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE)
                    );
            }
            
        }
    }

    return new;
}

bmp_image * convert_24bpp_to_8bpp(bmp_image * origin)
{
    if (origin == NULL) return NULL;
    if (origin->info.bmiHeader.biBitCount != BMP_24_BITS) return NULL;

    bmp_image * new = malloc(sizeof(bmp_image));
    if (new == NULL) return bmp_cleanup(NULL, new);
    
    new->info.bmiHeader.biSize = BMP_INFOHEADER;
    new->info.bmiHeader.biWidth = origin->info.bmiHeader.biWidth;
    new->info.bmiHeader.biHeight = origin->info.bmiHeader.biHeight;
    new->info.bmiHeader.biPlanes = BMP_DEFAULT_COLORPLANES;
    new->info.bmiHeader.biBitCount = BMP_8_BITS;
    new->info.bmiHeader.biCompression = BMP_BI_RGB;
    new->info.bmiHeader.biSizeImage = new->info.bmiHeader.biWidth 
                    * new->info.bmiHeader.biHeight 
                    * new->info.bmiHeader.biBitCount / BMP_8_BITS;
    new->info.bmiHeader.biXPelsPerMeter = origin->info.bmiHeader.biXPelsPerMeter;
    new->info.bmiHeader.biYPelsPerMeter = origin->info.bmiHeader.biYPelsPerMeter;
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
    
    new->info.bmiColours = malloc(colourpalettesize);

    for (size_t i = 0; i < pow(2,new->info.bmiHeader.biBitCount); i++) {
        new->info.bmiColours[i].rgbBlue = i;
        new->info.bmiColours[i].rgbGreen = i;
        new->info.bmiColours[i].rgbRed = i;
        new->info.bmiColours[i].rgbReserved = 0;
    }

    new->ciPixelArray = malloc(new->info.bmiHeader.biSizeImage);

    for (size_t x = 0; x < new->info.bmiHeader.biWidth; x++) {
        /* code */
        for (size_t y = 0; y < new->info.bmiHeader.biHeight; y++) {
            /* code */
            uint8_t ReqG = bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN);
            uint8_t GeqB = bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE);
            uint8_t BeqR = bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE) == bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED);

            if (ReqG && GeqB && BeqR) {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED);
            } else {
                new->ciPixelArray[y*new->info.bmiHeader.biWidth + x] = 
                    bmp_findgray(
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_RED),
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_GREEN),
                        bmp_getpixelcolor(origin, x, y, BMP_COLOR_BLUE)
                    );
            }
            
        }
    }

    return new;
}

void generate_gradient_sample()
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
    img->info.bmiColours = malloc(colourpalettesize);

    for (size_t i = 0; i < pow(2,img->info.bmiHeader.biBitCount); i++) {
        img->info.bmiColours[i].rgbBlue = i;
        img->info.bmiColours[i].rgbGreen = i;
        img->info.bmiColours[i].rgbRed = i;
        img->info.bmiColours[i].rgbReserved = 0;
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

    bmp_details(img);

    bmp_save(img, "gradient.bmp");

    bmp_cleanup(NULL, img);
}