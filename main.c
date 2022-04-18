/**
 * @file main.c
 * @author Nilo Edson (niloedson.ms@gmail.com)
 * @brief 
 * @version 4.1
 * @date 2022-04-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "main.h"

void main()
{
    /* code */

    bmp_image * img = bmp_read("./samples/salt-pepper.bmp");

    bmp_image * decoded = bmp_rle8decoder(img);

    bmp_save(decoded, "salt-pepper-decoded.bmp");

    bmp_cleanup(NULL, img);
    bmp_cleanup(NULL, decoded);
}