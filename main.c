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

void main()
{
    /* code */

    bmp_image * img = bmp_read("./samples/monarchs.bmp");

    bmp_image * grayed = bmp_rgb2gray(img, BMP_NCOLOURS_256);

    bmp_addpad(grayed, 50, 50, BMP_PADTYPE_REPLICATE);
    bmp_addpad(grayed, 50, 50, BMP_PADTYPE_ZEROS);

    bmp_save(grayed, "monarchs-grayed-padded.bmp");

    bmp_cleanup(NULL, grayed);

    bmp_cleanup(NULL, img);
}