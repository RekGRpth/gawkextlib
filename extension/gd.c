/*
 * gd.c - Subset of GD Boutell's Graphic Library
 *
 * Víctor Paesa, 2006-04-15
 */

/*
 * Copyright (C) 2001, 2004, 2005, 2006, 2007 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "awk.h"
#include <gd.h>

#define GD_NOK (-1)
#define GD_OK  (0)
#define SET_RETURN_NOK set_value(tmp_number((AWKNUM) GD_NOK))
#define SET_RETURN_OK  set_value(tmp_number((AWKNUM) GD_OK))

static strhash *gdimgs;

/*
# I compiled from source my GD library.
# This is how I configured my GD, to remove the Xlib dependency
./configure --x-includes=/usr/include/noX/ --x-libraries=/usr/lib/noX --with-xpm=/usr/lib/noX --without-x

# I used this to know which GD API was used in my GD C programs:
gawk 'match($0, /gd.*\(/) {print substr($0, RSTART, RLENGTH-1)}' mygdprograms.c |sort -u

First stage funcions:
gdImageCreateFromFile instead of gdImageCreateFromJpeg, gdImageCreateFromPng, gdImageCreateFromGif
gdImageCopyResampled
gdImageCreateTrueColor
gdImageDestroy
gdImagePngName instead of gdImagePng
gdImageStringFT
gdImageColorAllocate

Second stage:
gdImageFilledRectangle
gdImageRectangle
gdImageSetAntiAliasedDontBlend
gdImageSetThickness
gdImageSX
gdImageSY

Third stage (to be done)
gdImageSaveName instead of gdImagePng, gdImageJpeg, gdImageGIF
*/

/* get_intarg is taken from pgsql.c */
static inline int
get_intarg(NODE *tree, unsigned int argnum)
{
	NODE *argnode = get_scalar_argument(tree, argnum, FALSE);
	/* Do we need to worry about rounding here? */
	int arg = force_number(argnode);
	free_temp(argnode);
	return arg;
}

/* find_handle is taken from pgsql.c */
static void *
find_handle(strhash *ht, NODE *tree, unsigned int argnum)
{
	NODE *handle;
	strhash_entry *ent;

	handle = get_scalar_argument(tree, argnum, FALSE);
	force_string(handle);
	ent = strhash_get(ht, handle->stptr, handle->stlen, 0);
	free_temp(handle);
	return ent ? ent->data : NULL;
}

static NODE *
img_handle(gdImagePtr im)
{
	static unsigned long hnum = 0;
	char handle[32];
	size_t sl;

	snprintf(handle, sizeof(handle), "gdimg%lu", hnum++);
	sl = strlen(handle);
	strhash_get(gdimgs, handle, sl, 1)->data = im;
	return tmp_string(handle, sl);
}


/* void gdImageCreateFromFile(gdImagePtr im, char *file_name) */ /* It doesn't exist in GD */
/*  do_gdImageCreateFromFile --- provide dynamically loaded do_gdImageCreateFromFile() builtin for gawk */

static NODE *
do_gdImageCreateFromFile(NODE *tree)
{
	NODE *fName;
	gdImagePtr im;
	FILE *in;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("do_gdImageCreateFromFile: called with incorrect number of arguments");

	fName = get_scalar_argument(tree, 0, FALSE);
	(void) force_string(fName);
	in = fopen(fName->stptr, "rb");
	im= gdImageCreateFromPng(in);
	if (!(im)) {
		rewind(in);
		im= gdImageCreateFromJpeg(in);
	}
	if (!(im)) {
		rewind(in);
		im= gdImageCreateFromGif(in);
	}
	fclose(in);
	free_temp(fName);

	if (im)
		set_value(img_handle(im));
        else {
		set_ERRNO("gdImageCreateFromFile failed");
		set_value(Nnull_string);
	}

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageDestroy(gdImagePtr im) */
/*  do_gdImageDestroy --- provide dynamically loaded do_gdImageDestroy() builtin for gawk */

static NODE *
do_gdImageDestroy(NODE *tree)
{
	NODE *handle;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("do_gdImageDestroy: called with incorrect number of arguments");

	handle = get_scalar_argument(tree, 0, FALSE);
	force_string(handle);
	if (strhash_delete(gdimgs, handle->stptr, handle->stlen,
			(strhash_delete_func)gdImageDestroy, NULL) < 0) {
		set_ERRNO("gdImageDestroy called with unknown image handle");
		SET_RETURN_NOK;
	}
	else
		SET_RETURN_OK;

	free_temp(handle);
	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageCreateTrueColor(int sx, int sy) */
/*  do_gdImageCreateTrueColor --- provide dynamically loaded do_gdImageCreateTrueColor() builtin for gawk */

static NODE *
do_gdImageCreateTrueColor(NODE *tree)
{
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 2)
		lintwarn("do_gdImageCreateTrueColor: called with incorrect number of arguments");

	im= gdImageCreateTrueColor(get_intarg(tree, 0), get_intarg(tree, 1));

	if (im)
		set_value(img_handle(im));
        else {
		set_ERRNO("gdImageCreateTrueColor failed");
		set_value(Nnull_string);
	}

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/*
void gdImageCopyResampled(gdImagePtr dst, gdImagePtr src,
	int dstX, int dstY, int srcX, int srcY, int destW, int destH, int srcW, int srcH)
*/

/*  do_gdImageCopyResampled --- provide dynamically loaded do_gdImageCopyResampled() builtin for gawk */

static NODE *
do_gdImageCopyResampled(NODE *tree)
{
	gdImagePtr dst, src;

	if (do_lint && get_curfunc_arg_count() != 10)
		lintwarn("do_gdImageCopyResampled: called with incorrect number of arguments");

	if (!(dst = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageCopyResampled called with unknown dst image handle");
		SET_RETURN_NOK;
	}
	else if (!(src = find_handle(gdimgs, tree, 1))) {
		set_ERRNO("gdImageCopyResampled called with unknown src image handle");
		SET_RETURN_NOK;
	}
	else {
		gdImageCopyResampled(dst, src,
			get_intarg(tree, 2), get_intarg(tree, 3), get_intarg(tree, 4), get_intarg(tree, 5),
			get_intarg(tree, 6), get_intarg(tree, 7), get_intarg(tree, 8), get_intarg(tree, 9));
		SET_RETURN_OK;
	}

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImagePng(gdImagePtr im, FILE *out) */
/*  do_gdImagePngName --- provide dynamically loaded do_gdImagePngName() builtin for gawk */

static NODE *
do_gdImagePngName(NODE *tree)
{
	NODE *fName;
	gdImagePtr im;
	FILE *out;

	if (do_lint && get_curfunc_arg_count() != 2)
		lintwarn("do_gdImagePngName: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImagePngName called with unknown image handle");
		SET_RETURN_NOK;
	}
	else {
		fName = get_scalar_argument(tree, 1, FALSE);
		(void) force_string(fName);
		out = fopen(fName->stptr, "wb");
		if (out) {
			gdImagePng(im, out);
			fclose(out);
			SET_RETURN_OK;
		}
		else
			SET_RETURN_NOK;
		free_temp(fName);
	}

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* char *gdImageStringFT(gdImagePtr im, int *brect, int fg, char *fontname, double ptsize, double angle, int x, int y, char *string) */
/*  do_gdImageStringFT --- provide dynamically loaded do_gdImageStringFT() builtin for gawk */

// We return here "" if things are OK, or an error message otherwise !!!

static NODE *
do_gdImageStringFT(NODE *tree)
{
	int n;
	NODE **aptr;
	NODE *argnode;
	char * errStr;

	gdImagePtr im;
	NODE *brect_a;
	int brect[8];
	int fg;
    NODE *fontName;
	double ptsize, angle;
	int x, y;
	NODE *string;
    int str_len;

	if (do_lint && get_curfunc_arg_count() != 9)
		lintwarn("do_gdImageStringFT: called with incorrect number of arguments");

	string = get_scalar_argument(tree, 0, FALSE);
	(void) force_string(string);
    str_len = string->stlen;
    free_temp(string);

    if (!(str_len)) { /* empty AWK strings mean NULL img pointers in GD */
		im = NULL;
    }
	else	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageStringFT called with unknown image handle");
		set_value(tmp_string("unknown image handle", 20));
	}

	if (!(str_len) || im) { //gdImageStringFT accepts either NULL or valid img pointers

		/* brect array to hold results is second argument */
		brect_a = get_array_argument(tree, 1, FALSE);
		/* empty out the array */
		assoc_clear(brect_a);

		fg = get_intarg(tree, 2);

		fontName = get_scalar_argument(tree, 3, FALSE);
		(void) force_string(fontName);

		argnode = get_scalar_argument(tree, 4, FALSE);
		ptsize = force_number(argnode);
		free_temp(argnode);

		argnode = get_scalar_argument(tree, 5, FALSE);
		angle = force_number(argnode);
		free_temp(argnode);

		x = get_intarg(tree, 6);
		y = get_intarg(tree, 7);

		string = get_scalar_argument(tree, 8, FALSE);
		(void) force_string(string);

		errStr = gdImageStringFT(im, brect, fg, fontName->stptr, ptsize, angle, x, y, string->stptr);
		if (!(errStr))
			errStr="";

		/* fill in the array brect_a */
		for (n=0; n<8; n++) {
			aptr = assoc_lookup(brect_a, tmp_number((AWKNUM) n), FALSE);
			*aptr = make_number((AWKNUM) brect[n]);
		}

		free_temp(fontName);
		free_temp(string);

		/* Set the return value */
		set_value(tmp_string(errStr, strlen(errStr)));
	}

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* int gdImageColorAllocate(gdImagePtr im, int r, int g, int b)  */
/*  do_gdImageColorAllocate --- provide dynamically loaded do_gdImageColorAllocate() builtin for gawk */

static NODE *
do_gdImageColorAllocate(NODE *tree)
{
	int ret;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 4)
		lintwarn("do_gdImageColorAllocate: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageColorAllocate called with unknown image handle");
		ret = GD_NOK;
	}
	else
		ret = gdImageColorAllocate(im, get_intarg(tree, 1), get_intarg(tree, 2), get_intarg(tree, 3));

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)  */
/*  do_gdImageFilledRectangle --- provide dynamically loaded do_gdImageFilledRectangle() builtin for gawk */

static NODE *
do_gdImageFilledRectangle(NODE *tree)
{
	int ret = GD_OK;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 6)
		lintwarn("do_gdImageFilledRectangle: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageFilledRectangle called with unknown image handle");
		ret = GD_NOK;
	}
	else
		gdImageFilledRectangle(im,
			get_intarg(tree, 1), get_intarg(tree, 2), get_intarg(tree, 3), get_intarg(tree, 4), get_intarg(tree, 5));

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)  */
/*  do_gdImageRectangle --- provide dynamically loaded do_gdImageRectangle() builtin for gawk */

static NODE *
do_gdImageRectangle(NODE *tree)
{
	int ret = GD_OK;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 6)
		lintwarn("do_gdImageRectangle: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageRectangle called with unknown image handle");
		ret = GD_NOK;
	}
	else
		gdImageRectangle(im,
			get_intarg(tree, 1), get_intarg(tree, 2), get_intarg(tree, 3), get_intarg(tree, 4), get_intarg(tree, 5));

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageSetAntiAliasedDontBlend(gdImagePtr im, int c, int dont_blend)  */
/*  do_gdImageSetAntiAliasedDontBlend --- provide dynamically loaded do_gdImageSetAntiAliasedDontBlend() builtin for gawk */

static NODE *
do_gdImageSetAntiAliasedDontBlend(NODE *tree)
{
	int ret = GD_OK;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 3)
		lintwarn("do_gdImageSetAntiAliasedDontBlend: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageSetAntiAliasedDontBlend called with unknown image handle");
		ret = GD_NOK;
	}
	else
		gdImageSetAntiAliasedDontBlend(im, get_intarg(tree, 1), get_intarg(tree, 2));

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageSetThickness(gdImagePtr im, int thickness)   */
/*  do_gdImageSetThickness --- provide dynamically loaded do_gdImageSetThickness() builtin for gawk */

static NODE *
do_gdImageSetThickness(NODE *tree)
{
	int ret = GD_OK;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 2)
		lintwarn("do_gdImageSetThickness: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageSetThickness called with unknown image handle");
		ret = GD_NOK;
	}
	else
		gdImageSetThickness(im, get_intarg(tree, 1));

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageSX(gdImagePtr im)   */
/*  do_gdImageSX --- provide dynamically loaded do_gdImageSX() builtin for gawk */

static NODE *
do_gdImageSX(NODE *tree)
{
	int ret;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("do_gdImageSX: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageSX called with unknown image handle");
		ret = GD_NOK;
	}
	else
		ret = gdImageSX(im);

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* void gdImageSY(gdImagePtr im)   */
/*  do_gdImageSY --- provide dynamically loaded do_gdImageSY() builtin for gawk */

static NODE *
do_gdImageSY(NODE *tree)
{
	int ret;
	gdImagePtr im;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("do_gdImageSY: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageSY called with unknown image handle");
		ret = GD_NOK;
	}
	else
		ret = gdImageSY(im);

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* int gdImageCompare(gdImagePtr im1, gdImagePtr im2) */
/*  do_gdImageCompare --- provide dynamically loaded do_gdImageCompare() builtin for gawk */

static NODE *
do_gdImageCompare(NODE *tree)
{
	gdImagePtr im1, im2;
	int ret;

	if (do_lint && get_curfunc_arg_count() != 10)
		lintwarn("do_gdImageCompare: called with incorrect number of arguments");

	if (!(im1 = find_handle(gdimgs, tree, 0))) {
		set_ERRNO("gdImageCompare called with unknown im1 image handle");
		ret = 1<<14;
	}
	else if (!(im2 = find_handle(gdimgs, tree, 1))) {
		set_ERRNO("gdImageCompare called with unknown im2 image handle");
		ret = 1<<15;
	}
	else {
		ret = gdImageCompare(im1, im2);
	}

	/* Set the return value */
	set_value(tmp_number((AWKNUM) ret));

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

/* dlload --- load new builtins in this library */

#ifdef BUILD_STATIC_EXTENSIONS
#define dlload dlload_gd
#endif

NODE *
dlload(NODE *tree ATTRIBUTE_UNUSED, void *dl ATTRIBUTE_UNUSED)
{
	make_builtin("gdImageCreateFromFile", do_gdImageCreateFromFile, 1);
	make_builtin("gdImageDestroy", do_gdImageDestroy, 1);
	make_builtin("gdImageCreateTrueColor", do_gdImageCreateTrueColor, 2);
	make_builtin("gdImageCopyResampled", do_gdImageCopyResampled, 10);
	make_builtin("gdImagePngName", do_gdImagePngName, 2);
	make_builtin("gdImageStringFT", do_gdImageStringFT, 9);
	make_builtin("gdImageColorAllocate", do_gdImageColorAllocate, 4);

	make_builtin("gdImageFilledRectangle", do_gdImageFilledRectangle, 6);
	make_builtin("gdImageRectangle", do_gdImageRectangle, 6);
	make_builtin("gdImageSetAntiAliasedDontBlend", do_gdImageSetAntiAliasedDontBlend, 3);
	make_builtin("gdImageSetThickness", do_gdImageSetThickness, 2);
	make_builtin("gdImageSX", do_gdImageSX, 1);
	make_builtin("gdImageSY", do_gdImageSY, 1);

	make_builtin("gdImageCompare", do_gdImageCompare, 2);

	/* Create hash tables. */
	gdimgs = strhash_create(0);

	return tmp_number((AWKNUM) 0);
}
