@load gd

BEGIN {
  print "This is intended to demonstrate usage of the xgawk GD graphics interface."
  print "The API is very close to the GD C API, with minimal adaptions to gawk.\n"
  print "This example creates a solid blue filled png (gd_first.png) in a first step,"
  print "then it loads it, draws a lilac frame, draws a lilac greeting text on top of"
  print "a red background, draws eight green text lines showing the coordinates of"
  print "that greeting text and finally saves the result as gd_second.png\n"

  print "Creating a 720x576 true color image in memory"
  im=gdImageCreateTrueColor(720, 576)
  print "Allocating a blue background color"
  blue = gdImageColorAllocate(im, 0, 0, 128)
  print "Filling the image with the background color"
  gdImageFilledRectangle(im, 0, 0, 720, 576, blue)
  print "Saving the image as gd_first.png"
  gdImagePngName(im, "gd_first.png")
  print "Destroying the image in memory"
  gdImageDestroy(im)

  if (!("GDFONTPATH" in ENVIRON)) {
    print "\nRemember to set GDFONTPATH!!\nPlease do something like ...\n"
    print "GDFONTPATH=/cygdrive/c/WINDOWS/Fonts/:/usr/X11R6/lib/X11/fonts/TTF/; export GDFONTPATH\n"
    print "... and then re-run example."
    exit
  }
  print "Loading in memory the gd_first.png file"
  im=gdImageCreateFromFile("gd_first.png")
  print "Allocating a lilac background color"
  lila = gdImageColorAllocate(im, 128, 0, 255)
  gdImageSetThickness(im, 4)
  gdImageRectangle(im, 8, 8, gdImageSX(im)-8, gdImageSY(im)-8, lila)
  print "Allocating a red color"
  red = gdImageColorAllocate(im, 255, 0, 0)
  s="Hello, thanks for using the GD extension for GAWK"
  print "Getting text dimensions, without drawing it"
  err=gdImageStringFT("", brect, lila, "Verdana.ttf", 18.0, 0.0, 10, 30, s)
  if (err) {
    print err
    print "\nIf you do not have the Verdana font, then you may get it from "
    print "ftp://sunsite.dk/projects/cygwinports/release/X11/corefonts/corefonts-1-1.tar.bz2"
    print "(or you may modify this example to use other fonts you have)"
    exit
  }
  print "Drawing a red filled rect sized as text"
  gdImageFilledRectangle(im, brect[6], brect[7], brect[2], brect[3], red)
  print "Drawing the lilac greeting text on top of red rect"
  err=gdImageStringFT(im, brect, lila, "Verdana.ttf", 18.0, 0.0, 10, 30, s)
  s= \
    "lower left corner,  X position " sprintf("%3d", brect[0]) \
  "\nlower left corner,  Y position " sprintf("%3d", brect[1]) \
  "\nlower right corner, X position " sprintf("%3d", brect[2]) \
  "\nlower right corner, Y position " sprintf("%3d", brect[3]) \
  "\nupper right corner, X position " sprintf("%3d", brect[4]) \
  "\nupper right corner, Y position " sprintf("%3d", brect[5]) \
  "\nupper left corner,  X position " sprintf("%3d", brect[6]) \
  "\nupper left corner,  Y position " sprintf("%3d", brect[7])
  print "Allocating a green color"
  green = gdImageColorAllocate(im, 0, 128, 128)
  print "Printing the greeting text coordinates in green"
  err=gdImageStringFT(im, brect, green, "VeraMono.ttf", 18.0, 0.0, 20, 60, s)
  if (err) {
    print err
    print "\nIf you do not have the Vera Mono font, then you may get it from "
    print "http://www.gnome.org/fonts/"
    print "(or you may modify this example to use other fonts you have)"
    exit
  }
  print "Saving the image as gd_second.png"
  gdImagePngName(im, "gd_second.png")
  print "Destroying the image in memory"
  gdImageDestroy(im)
  print "That was all, folks!"
}
