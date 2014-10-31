// xgdfont.ok.c creates exactly the same image as xgdfont.awk
// gcc -oxgdfont.ok.exe xgdfont.ok.c -lgd $(gdlib-config --libs)

#include <stdio.h>
#include <stdlib.h>
#include "gd.h"

int main(int argc, char **argv)
{
  gdImagePtr im, im2;
  FILE *pngout;
  int blue, red;
  int brect[8];
  char *err;

  puts("C Creating a 256x256 true color image in memory");
  im=gdImageCreateTrueColor(256, 256);
  puts("C Allocating a blue background color");
  blue=gdImageColorAllocate(im, 0, 0, 128);
  puts("C Filling the image with the background color");
  gdImageFilledRectangle(im, 0, 0, gdImageSX(im), gdImageSY(im), blue);
  red=gdImageColorAllocate(im, 255, 0, 0);
  puts("C Draw the GD logo (using the xgd.ttf font)");
  err=gdImageStringFT(NULL, brect, red, "xgd.ttf", 128.0, 0.0, gdImageSX(im)/2, gdImageSY(im)/2, "G");
  if (err==NULL) {
    err=gdImageStringFT(im, brect, red, "xgd.ttf", 128.0, 0.0,
      gdImageSX(im)/2 -(brect[2]-brect[0])/2,
      gdImageSY(im)/2 -(brect[5]-brect[1])/2, "G");

    if (pngout = fopen("xgdfont.ok.png", "wb")) {
      gdImagePng(im, pngout);
      fclose(pngout);
    }
    else
      puts("NOK, C reference image xgdfont.ok.png not created");

  }
  else
    puts("NOK, C xgd.ttf font not found");
  puts("C Destroying the image in memory");
  gdImageDestroy(im);
}
