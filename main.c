#include "main.h"

//TODO: Implement convertion to gray-scale images
//TODO: Implement "Look-Up Tables"

void main()
{
    bmp_image * img = NULL;

    img = bmp_open("girl.bmp");

    bmp_details(img);

    bmp_invert(img);
    bmp_save(img, "girl_negative.bmp");
	
	bmp_invert(img);

    bmp_filter_color(img, BMP_COLOR_BLUE);
    bmp_save(img, "girl_filtered.bmp");

}