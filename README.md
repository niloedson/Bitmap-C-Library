# Bitmap C Library

The `bmp.h` library handles Bitmap format images and allows you to load, work with, convert or generate `*.bmp` files in an exclusive C subsystem environment.

The Portuguese version of this README is still to be written.

## Description

This project is intended to be used during the "Digital Image Processing" course (for the Electrical and Computer Engineering programs) at the Federal University of Amazonas, in Brazil.

## Sample images in use

- `Redbricks.bmp` is an example of a `4 bits per pixel` image (hexadecimal output taken from the [Microsoft Docs Bitmap Storage page](https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-storage))  given by the `bmp_getredbricks()` function.
- `gradient.bmp` is an example of a `8 bits per pixel` image generated by the `bmp_8bpp_sample()` function.
- `redblue.bmp` is an example of a `16 bits per pixel` image generated by the `bmp_16bpp_sample()` function.
- `monarchs.bmp` is an example of a `24 bits per pixel` image.
- `greenblue.bmp` is an example of a `32 bits per pixel` image generated by the `bmp_32bpp_sample()` function.

All the samples mentioned above were generated using my own `bmp.h` library while `monarchs.bmp` was simply taken from the "Yu-Gi-Oh! TCG (r)" card "Pantheism of the Monarchs".

### Some generic samples used during the classes

- `drip-bottle.bmp`;
- `girl.bmp`;
- `hidden-horse.bmp`;
- `moon.bmp`;
- `rose.bmp`;
- `salt-pepper.bmp`;
- `spillway-dark.bmp`;
- `testpattern.bmp`;

All of them are well known images used for this purpose. Notice that they are all in Bitmap format. All of them were converted from their respective `*.tif`/`*.tiff` format using some online TIF to BMP converter and then rearranged for a `8 bits per pixel` format using this `bmp.h` library.

`salt-pepper.bmp` in particular, is compressed using RLE8 (the specified `BMP_BI_RLE8` compression type).



## Using the `bmp.h` library

(still being written)
