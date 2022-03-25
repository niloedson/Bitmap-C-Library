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

    bmp_printdetails(img);

    bmp_save(img, "monarchs-copy.bmp");

    bmp_image * grayed = bmp_rgb2gray(img, BMP_NCOLOURS_256);

    bmp_save(grayed, "monarchs-grayed.bmp");

    bmp_invert(grayed);

    bmp_save(grayed, "monarchs-grayed-negative.bmp");

    bmp_filtercolor(img, BMP_COLOR_BLUE);

    bmp_save(img, "monarchs-blue-filter.bmp");

    bmp_cleanup(NULL, grayed);

    bmp_cleanup(NULL, img);
}