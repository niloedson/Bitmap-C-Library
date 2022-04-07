/**
 * @file main.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief 
 * @version 3.9
 * @date 2022-04-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "main.h"

void main()
{
    /* code */

    bmp_image * img = bmp_read("./samples/salt-pepper.bmp");

    bmp_printdetails(img);

    printf("file size:      \t%u \n", bmp_getfilesize(img));
    printf("offset:         \t%u \n", bmp_getoffset(img));
    printf("data size:      \t%u \n", bmp_getdatasize(img));
    printf("DIB format:     \t%u \n", bmp_getdibformat(img));
    printf("bit count:      \t%u \n", bmp_getbitcount(img));
    printf("palette size:   \t%u \n", bmp_getpalettesize(img));
    printf("compression:    \t%u \n", bmp_getcompression(img));
    printf("n pixels:       \t%u \n", bmp_getnpixels(img));

    bmp_cleanup(NULL, img);
}