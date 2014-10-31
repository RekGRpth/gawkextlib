#
# Author: Hiroshi Saito(hiroshi@winpg.jp)
#
# This is intended to demonstrate usage of the gawk pdf interface.
# useful demonstration program exists in the tree of a libharu project. 
# Please visit that. it is the information for mastering a pdf module. 
#

@load "pdf"

# ----
# main
# ----

BEGIN {

    fname = "pdftest.pdf";
    png = (srcdir "/pdftest.png");

# Initial
    pdf = HPDF_New(NULL, NULL);
    if (ERRNO !="")
       print ERRNO;
    font = HPDF_GetFont(pdf, "Times-Roman", "WinAnsiEncoding");
    if (ERRNO !="")
       print ERRNO;
    page = HPDF_AddPage(pdf);
    if (ERRNO !="")
       print ERRNO;

# Page size
    if (HPDF_Page_SetWidth(page, 400))
       print ERRNO;
    if (HPDF_Page_SetHeight(page, 500))
       print ERRNO;

# Rectangle
    if (HPDF_Page_SetLineWidth(page, 1))
       print ERRNO;
    if (HPDF_Page_Rectangle(page, 50, 50, HPDF_Page_GetWidth(page) - 100,
              HPDF_Page_GetHeight(page) - 110))
       print ERRNO;
    if (HPDF_Page_Stroke(page))
       print ERRNO;

# Powerd by gawkextlib
    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_SetFontAndSize(page, font, 9))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, 10, 480))
       print ERRNO;
    if (HPDF_Page_ShowText(page, "Powerd by gawkextlib"))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# Title
    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_SetFontAndSize(page, font, 16))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, 90, 450))
       print ERRNO;
    if (HPDF_Page_ShowText(page, "gawk extension pdf module"))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# libharu ref.
    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_SetFontAndSize(page, font, 10))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, 90, 400))
       print ERRNO;
    if (HPDF_Page_SetRGBFill(page, 0.5, 0.5, 0.5))
       print ERRNO;
    if (HPDF_Page_ShowText(page, "Please visit libharu project."))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# url link
    # libharu project link.
    uri = "http://libharu.org/";
    # rect structure number is 1:left 2:bottom 3:right 4:top
    rect[1] = 85;rect[2] = 395;rect[3] = 220;rect[4] = 380;

    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_SetFontAndSize(page, font, 12))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, rect[1] + 5 , rect[2] -10))
       print ERRNO;
    if (HPDF_Page_SetRGBFill(page, 0.2, 0.7, 0.1))
       print ERRNO;
    if (HPDF_Page_ShowText(page, uri))
       print ERRNO;
    annot = HPDF_Page_CreateURILinkAnnot(page, rect, uri);
    if (ERRNO !="")
       print ERRNO;
    if (HPDF_LinkAnnot_SetBorderStyle(annot, 1, 3, 2))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# comment text
    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_SetFontAndSize(page, font, 10))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, 90, 350))
       print ERRNO;
    if (HPDF_Page_SetRGBFill(page, 0.1, 0.4, 0.9))
       print ERRNO;
    if (HPDF_Page_ShowText(page, "it is the very helpful information for pdf module. "))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# png image
    image = HPDF_LoadPngImageFromFile(pdf, png);
    if (ERRNO !="")
       print ERRNO;
    width = HPDF_Image_GetWidth(image);
    if (ERRNO !="")
       print ERRNO;
    height = HPDF_Image_GetHeight(image);
    if (ERRNO !="")
       print ERRNO;
    if (HPDF_Page_DrawImage(page, image, 90, 250, width/2, height/2))
       print ERRNO;

# Annotation
    # rect structure number is 1:left 2:bottom 3:right 4:top
    rect[1] =  90;rect[2] = 200;rect[3] = 100;rect[4] = 220;
    annot = HPDF_Page_CreateTextAnnot(page, rect, 
    "This is intended to demonstrate usage of the gawk pdf interface.\n" \
    "We think this source code is useful to you.\n"i \
    "Thanks!", NULL);
    if (ERRNO !="")
       print ERRNO;
    if (HPDF_TextAnnot_SetIcon(annot, HPDF_ANNOT_ICON_COMMENT))
       print ERRNO;
    if (HPDF_TextAnnot_SetOpened(annot, HPDF_TRUE))
       print ERRNO;

    if (HPDF_Page_BeginText(page))
       print ERRNO;
    if (HPDF_Page_MoveTextPos(page, rect[1] + 20, rect[2] + 10))
       print ERRNO;
    if (HPDF_Page_ShowText(page, "Annotation."))
       print ERRNO;
    if (HPDF_Page_EndText(page))
       print ERRNO;

# save the document to a file 
    if (HPDF_SaveToFile(pdf, fname))
       print ERRNO;

# clean up 
    if (HPDF_Free(pdf))
       print ERRNO;

}

