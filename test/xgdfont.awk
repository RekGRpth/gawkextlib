BEGIN {
  print "Creating a 256x256 true color image in memory"
  im=gdImageCreateTrueColor(256, 256)
  print "Allocating a blue background color"
  blue=gdImageColorAllocate(im, 0, 0, 128)
  print "Filling the image with the background color"
  gdImageFilledRectangle(im, 0, 0, gdImageSX(im), gdImageSY(im), blue)
  if (!("GDFONTPATH" in ENVIRON))
    print "NOK, GDFONTPATH not set"
  red=gdImageColorAllocate(im, 255, 0, 0)
  print "Draw the GD logo (using the xgd.ttf font)"
  err=gdImageStringFT("", brect, red, "xgd.ttf", 128.0, 0.0, gdImageSX(im)/2, gdImageSY(im)/2, "G")
  if (err=="") {
    err=gdImageStringFT(im, brect, red, "xgd.ttf", 128.0, 0.0, \
      int(gdImageSX(im)/2) -int((brect[2]-brect[0])/2), \
      int(gdImageSY(im)/2) -int((brect[5]-brect[1])/2), "G")
    gdImagePngName(im, "xgdfont._.png")
    print "Loading the reference image"
    im2=gdImageCreateFromFile("xgdfont.ok.png")
    if (im2) {
      print "Compare what we loaded with what we drew"
      if (0==gdImageCompare(im, im2))
        print "OK, reference image and drew one are equal"
      else
        print "NOK, reference image and drew one are different"
      gdImageDestroy(im2)
    }
    else
      print "NOK, reference image xgdfont.ok.png not found"
  }
  else
    print "NOK, xgd.ttf font not found"
  print "Destroying the image in memory"
  gdImageDestroy(im)
}
