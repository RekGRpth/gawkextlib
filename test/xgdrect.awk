BEGIN {
  print "Creating a 720x576 true color image in memory"
  im=gdImageCreateTrueColor(720, 576)
  print "Allocating a blue background color"
  blue = gdImageColorAllocate(im, 0, 0, 128)
  print "Filling the image with the background color"
  gdImageFilledRectangle(im, 0, 0, 720, 576, blue)
  gdImagePngName(im, "xgdrect._.png")
  print "Loading the reference image"
  im2=gdImageCreateFromFile(srcdir "/xgdrect.ok.png")
  if (im2) {
    print "Compare what we loaded with what we drew"
    if (0==gdImageCompare(im, im2))
      print "OK, reference image and drew one are equal"
    else
      print "NOK, reference image and drew one are different"
    gdImageDestroy(im2)
  }
  else
    print "NOK, reference image xgdrect.ok.png not found"
  print "Destroying the image in memory"
  gdImageDestroy(im)
}
