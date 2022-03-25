/**
 * @file bmp.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief library to handle bitmap image files (*.bmp)
 * @version 0.1
 * @date 2022-02-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <bmp.h>

bmp_image * bmp_clean_up(FILE * fptr , bmp_image * img)
{
    if (fptr != NULL) fclose(fptr);

    if (img != NULL) {
        if (img->data != NULL) free(img->data);
        free(img);
    }

    return NULL;
}

int bmp_check_header(bmp_header * hdr)
{
    if (hdr->type != BMP_INIT_HEADER) return 0;
        
    switch (hdr->bits) {
    case BMP_32_BITS: break;
    case BMP_24_BITS: break;
    case BMP_16_BITS: break;
    case BMP_8_BITS: break;
    case BMP_4_BITS: break;
    case BMP_2_BITS: break;
    case BMP_1_BIT: break;
    default : return 0;
    }

    if (hdr->planes != BMP_N_COLOR_PLANES) return 0;

    switch (hdr->compression) {
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

bmp_image * bmp_open(const char * filename)
{
    FILE * fptr = NULL;
    bmp_image * img = NULL;

    fptr = fopen(filename, "r");
    if (fptr == NULL) return bmp_clean_up(fptr, img);

    img = malloc(sizeof(bmp_image));
    if (img == NULL) return bmp_clean_up(fptr, img);

    if (fread( &img->header, sizeof(bmp_header), 1, fptr) != 1) return bmp_clean_up(fptr, img);

    if (bmp_check_header(&img->header) == 0) return bmp_clean_up(fptr, img);

    img->data_size = img->header.size - sizeof(bmp_header);
    img->width = img->header.width;
    img->height = img->header.height;
    img->bytes_per_pixel = img->header.bits / BMP_BITS_PER_BYTE;

    img->data = malloc(sizeof(unsigned char) * img->data_size);
    if (img->data == NULL) return bmp_clean_up(fptr, img);

    if (fread(img->data, sizeof(char), img->data_size, fptr) != img->data_size) return bmp_clean_up(fptr, img);

    char onebyte;
    if (fread(&onebyte, sizeof(char), 1, fptr) != 0) return bmp_clean_up(fptr, img);

    fclose(fptr);

    return img;
}

bmp_image * bmp_load(unsigned char * imptr)
{
    bmp_image * img = NULL;

    img = malloc(sizeof(bmp_image));

    __uint8_t * byte = (__uint8_t *) imptr;

    img->header.type = (((__uint16_t) byte[1] << 8) 
                    | byte[0]);
    
    img->header.size = (((__uint32_t) byte[5] << 24) 
                    | ((__uint32_t) byte[4] << 16) 
                    | ((__uint32_t) byte[3] << 8) 
                    | (byte[2]));
    
    img->header.reserved1 = (((__uint16_t) byte[7] << 8) 
                    | (byte[6]));
    
    img->header.reserved2 = (((__uint16_t) byte[9] << 8) 
                    | (byte[8]));
    
    img->header.offset = (((__uint32_t) byte[13] << 24) 
                    | ((__uint32_t) byte[12] << 16) 
                    | ((__uint32_t) byte[11] << 8) 
                    | (byte[10]));
    
    img->header.header_size = (((__uint32_t) byte[17] << 24) 
                    | ((__uint32_t) byte[16] << 16) 
                    | ((__uint32_t) byte[15] << 8) 
                    | (byte[14]));
    
    img->header.width = (((__uint32_t) byte[21] << 24) 
                    | ((__uint32_t) byte[20] << 16) 
                    | ((__uint32_t) byte[19] << 8) 
                    | (byte[18]));

    img->header.height = (((__uint32_t) byte[25] << 24) 
                    | ((__uint32_t) byte[24] << 16) 
                    | ((__uint32_t) byte[23] << 8) 
                    | (byte[22]));
    
    img->header.planes = (((__uint16_t) byte[27] << 8) 
                    | (byte[26]));

    img->header.bits = (((__uint16_t) byte[29] << 8) 
                    | (byte[28]));

    img->header.compression = (((__uint32_t) byte[33] << 24) 
                    | ((__uint32_t) byte[32] << 16) 
                    | ((__uint32_t) byte[31] << 8) 
                    | (byte[30]));

    img->header.imagesize = (((__uint32_t) byte[37] << 24) 
                    | ((__uint32_t) byte[36] << 16) 
                    | ((__uint32_t) byte[35] << 8) 
                    | (byte[34]));
                
    img->header.xresolution = (((__uint32_t) byte[41] << 24) 
                    | ((__uint32_t) byte[40] << 16) 
                    | ((__uint32_t) byte[39] << 8) 
                    | (byte[38]));

    img->header.yresolution = (((__uint32_t) byte[45] << 24) 
                    | ((__uint32_t) byte[44] << 16) 
                    | ((__uint32_t) byte[43] << 8) 
                    | (byte[42]));

    img->header.ncolours = (((__uint32_t) byte[49] << 24) 
                    | ((__uint32_t) byte[48] << 16) 
                    | ((__uint32_t) byte[47] << 8) 
                    | (byte[46]));

    img->header.importantcolours = (((__uint32_t) byte[53] << 24) 
                    | ((__uint32_t) byte[52] << 16) 
                    | ((__uint32_t) byte[51] << 8) 
                    | (byte[50]));

    img->data_size = img->header.size - sizeof(bmp_header);
    img->width = img->header.width;
    img->height = img->header.height;
    img->bytes_per_pixel = img->header.bits / BMP_BITS_PER_BYTE;

    img->data = malloc(sizeof(unsigned char) * img->data_size);

    for (int i = 0; i < img->data_size; i++) {
        img->data[i] = imptr[sizeof(bmp_header)+i];
    }

    return img;
}

int bmp_save(const bmp_image * img, const char * filename)
{
    FILE * fptr = NULL;
    fptr = fopen(filename, "w");

    if (fptr == NULL) return 0;

    if (fwrite(&img->header, sizeof(bmp_header), 1, fptr) != 1) {
        fclose(fptr);
        return 0;
    }

    if (fwrite(img->data, sizeof(char), img->data_size, fptr) != img->data_size) {
        fclose(fptr);
        return 0;
    }

    fclose(fptr);

    return 1;
}

__uint8_t bmp_getpixel(bmp_image * img, int x, int y, bmp_color color)
{
    return img->data[3*(y*img->width + x) + color];
}

__uint8_t bmp_findgray(__uint8_t red, __uint8_t green, __uint8_t blue)
{
    double gray = 0.2989 * red + 0.5870 * green + 0.1140 * blue;
    return  (__uint8_t) gray;
}

void bmp_filter_color(bmp_image * img, bmp_color color)
{
    for (int pxl = color; pxl < img->data_size; pxl++) {
        if ((pxl % 3) != color) {
            img->data[pxl] = 0;
        }
    }
}

void bmp_invert(bmp_image * img)
{
    int pixel;

    for (pixel = 0; pixel < img->data_size; pixel++) {
        img->data[pixel] = 255 - img->data[pixel];
    }
}

void bmp_equalize(bmp_image * img)
{
    int pixel;

    unsigned char redmin = 255;
    unsigned char redmax = 0;
    unsigned char greenmin = 255;
    unsigned char greenmax = 0;
    unsigned char bluemin = 255;
    unsigned char bluemax = 0;

    for (pixel = 0; pixel < (img->data_size); pixel += 3) {
        unsigned char red = img->data[pixel+2];
        unsigned char green = img->data [pixel+1];
        unsigned char blue = img->data [pixel];

        if (redmin > red) redmin = red;
        if (redmax < red) redmax = red;

        if (greenmin > green) greenmin = green;
        if (greenmax < green) greenmax = green;

        if (bluemin > blue) bluemin = blue;
        if (bluemax < blue) bluemax = blue;
    }

    double redscale = 1.0;
    double greenscale = 1.0;
    double bluescale = 1.0;

    if (redmax > redmin) redscale = 255.0 / (redmax - redmin);
    
    if (greenmax > greenmin) greenscale = 255.0 / (greenmax - greenmin);
    
    if (bluemax > bluemin) bluescale = 255.0 / (bluemax - bluemin);
    
    for (pixel = 0; pixel < img->data_size; pixel += 3) {
        if (redmax > redmin) 
            img->data[pixel+2] = (int) (redscale * (img->data[pixel+2] - redmin));
        
        if (greenmax > greenmin) 
            img->data[pixel+1] = (int) (greenscale * (img->data[pixel+1] - greenmin));
        
        if (bluemax > bluemin) 
            img->data[pixel] = (int) (bluescale * (img->data[pixel] - bluemin));
    }
}

void bmp_details(bmp_image * img)
{
    printf("\n");

    printf("type:             \t%u  \t0x%x\n", img->header.type, img->header.type);
    printf("size:             \t%u  \t0x%x\n", img->header.size, img->header.size);
    printf("reserved1:        \t%u  \t0x%x\n", img->header.reserved1, img->header.reserved1);
    printf("reserved2:        \t%u  \t0x%x\n", img->header.reserved2, img->header.reserved2);
    printf("offset:           \t%u  \t0x%x\n", img->header.offset, img->header.offset);
    printf("header_size:      \t%u  \t0x%x\n", img->header.header_size, img->header.header_size);
    printf("width:            \t%u  \t0x%x\n", img->header.width, img->header.width);
    printf("height:           \t%u  \t0x%x\n", img->header.height, img->header.height);
    printf("planes:           \t%u  \t0x%x\n", img->header.planes, img->header.planes);
    printf("bits:             \t%u  \t0x%x\n", img->header.bits, img->header.bits);
    printf("compression:      \t%u  \t0x%x\n", img->header.compression, img->header.compression);
    printf("imagesize:        \t%u  \t0x%x\n", img->header.imagesize, img->header.imagesize);
    printf("xresolution:      \t%u  \t0x%x\n", img->header.xresolution, img->header.xresolution);
    printf("yresolution:      \t%u  \t0x%x\n", img->header.yresolution, img->header.yresolution);
    printf("ncolours:         \t%u  \t0x%x\n", img->header.ncolours, img->header.ncolours);
    printf("importantcolours: \t%u  \t0x%x\n", img->header.importantcolours, img->header.importantcolours);

    printf("\n");

    printf("data_size:        \t%u  \t0x%x\n", img->data_size, img->data_size);
    printf("width:            \t%u  \t0x%x\n", img->width, img->width);
    printf("height:           \t%u  \t0x%x\n", img->height, img->height);
    printf("bytes_per_pixel:  \t%u  \t0x%x\n", img->bytes_per_pixel, img->bytes_per_pixel);
    
    printf("\n");
}

void bmp_printpixel(bmp_image * img, int x, int y)
{
    __uint8_t r = bmp_getpixel(img, x, y, BMP_COLOR_RED);
    __uint8_t g = bmp_getpixel(img, x, y, BMP_COLOR_GREEN);
    __uint8_t b = bmp_getpixel(img, x, y, BMP_COLOR_BLUE);
    printf("img[%i][%i] = (%u, %u, %u)\n", x, y, r, g, b);
}