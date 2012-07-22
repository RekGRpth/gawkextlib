/*
 * gd.c - Subset of GD Boutell's Graphic Library
 *
 * Víctor Paesa, 2006-04-15
 */

/*
 * Copyright (C) 2001, 2004, 2005, 2006, 2007 the Free Software Foundation, Inc.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "common.h"
#include <gd.h>

#define GD_NOK (-1)
#define GD_OK  (0)
#define RETURN_NOK return make_number(GD_NOK, result)
#define RETURN_OK  return make_number(GD_OK, result)

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

/* find_handle is taken from pgsql.c */
static void *
find_handle(strhash *ht, unsigned int argnum)
{
	awk_value_t handle;
	strhash_entry *ent;

	if (!get_argument(argnum, AWK_STRING, &handle))
		return NULL;
	ent = strhash_get(ht, handle.str_value.str, handle.str_value.len, 0);
	return ent ? ent->data : NULL;
}

static size_t
img_handle(gdImagePtr im, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "gdimg%lu", hnum++);
	sl = strlen(handle);
	strhash_get(gdimgs, handle, sl, 1)->data = im;
	return sl;
}


/* void gdImageCreateFromFile(gdImagePtr im, char *file_name) */ /* It doesn't exist in GD */
/*  do_gdImageCreateFromFile --- provide dynamically loaded do_gdImageCreateFromFile() builtin for gawk */

static awk_value_t *
do_gdImageCreateFromFile(int nargs, awk_value_t *result)
{
	awk_value_t fName;
	gdImagePtr im;
	FILE *in;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "gdImageCreateFromFile: called with incorrect number of arguments");

	if (!get_argument(0, AWK_STRING, &fName)) {
		set_ERRNO(_("gdImageCreateFromFile: missing required filename argument"));
		RET_NULSTR;
	}

	if ((in = fopen(fName.str_value.str, "rb")) != NULL) {
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
	}
	else
		im = NULL;

	if (im) {
		char hdl[32];
		size_t hlen = img_handle(im, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("gdImageCreateFromFile failed"));
	RET_NULSTR;
}

/* void gdImageDestroy(gdImagePtr im) */
/*  do_gdImageDestroy --- provide dynamically loaded do_gdImageDestroy() builtin for gawk */

static awk_value_t *
do_gdImageDestroy(int nargs, awk_value_t *result)
{
	awk_value_t handle;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "gdImageDestroy: called with incorrect number of arguments");

	if (!get_argument(0, AWK_STRING, &handle)) {
		set_ERRNO(_("gdImageDestroy: missing required handle argument"));
		RETURN_NOK;
	}

	if (strhash_delete(gdimgs, handle.str_value.str, handle.str_value.len,
			   (strhash_delete_func)gdImageDestroy, NULL) < 0) {
		set_ERRNO(_("gdImageDestroy called with unknown image handle"));
		RETURN_NOK;
	}
	RETURN_OK;
}

/* void gdImageCreateTrueColor(int sx, int sy) */
/*  do_gdImageCreateTrueColor --- provide dynamically loaded do_gdImageCreateTrueColor() builtin for gawk */

static awk_value_t *
do_gdImageCreateTrueColor(int nargs, awk_value_t *result)
{
	gdImagePtr im;
	awk_value_t sx, sy;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, "gdImageCreateTrueColor: called with incorrect number of arguments");

	if (!get_argument(0, AWK_NUMBER, &sx) ||
	    !get_argument(1, AWK_NUMBER, &sy)) {
		set_ERRNO(_("gdImageCreateTrueColor: two integer arguments are required"));
		RET_NULSTR;
	}

	im= gdImageCreateTrueColor(sx.num_value, sy.num_value);

	if (im) {
		char hdl[32];
		size_t hlen = img_handle(im, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("gdImageCreateTrueColor failed"));
	RET_NULSTR;
}

/*
void gdImageCopyResampled(gdImagePtr dst, gdImagePtr src,
	int dstX, int dstY, int srcX, int srcY, int destW, int destH, int srcW, int srcH)
*/

/*  do_gdImageCopyResampled --- provide dynamically loaded do_gdImageCopyResampled() builtin for gawk */

static awk_value_t *
do_gdImageCopyResampled(int nargs, awk_value_t *result)
{
	gdImagePtr dst, src;
	awk_value_t n[8];
	size_t i;

	if (do_lint && nargs != 10)
		lintwarn(ext_id, "do_gdImageCopyResampled: called with incorrect number of arguments");

	if (!(dst = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageCopyResampled called with unknown dst image handle"));
		RETURN_NOK;
	}
	if (!(src = find_handle(gdimgs, 1))) {
		set_ERRNO(_("gdImageCopyResampled called with unknown src image handle"));
		RETURN_NOK;
	}
	for (i = 0; i < sizeof(n)/sizeof(n[0]); i++) {
		if (!get_argument(2+i, AWK_NUMBER, &n[i])) {
			char emsg[512];
			snprintf(emsg, sizeof(emsg), _("%s missing required numeric argument #%zu"), __func__+3, i+3);
			set_ERRNO(emsg);
			RETURN_NOK;
		}
	}

	gdImageCopyResampled(dst, src,
			     n[0].num_value,
			     n[1].num_value,
			     n[2].num_value,
			     n[3].num_value,
			     n[4].num_value,
			     n[5].num_value,
			     n[6].num_value,
			     n[7].num_value);
	RETURN_OK;
}

/* void gdImagePng(gdImagePtr im, FILE *out) */
/*  do_gdImagePngName --- provide dynamically loaded do_gdImagePngName() builtin for gawk */

static awk_value_t *
do_gdImagePngName(int nargs, awk_value_t *result)
{
	awk_value_t fName;
	gdImagePtr im;
	FILE *out;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, "do_gdImagePngName: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImagePngName called with unknown image handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &fName)) {
		set_ERRNO(_("gdImagePngName: missing required filename argument"));
		RETURN_NOK;
	}
	if (!(out = fopen(fName.str_value.str, "wb")))
		RETURN_NOK;
	gdImagePng(im, out);
	fclose(out);
	RETURN_OK;
}

/* char *gdImageStringFT(gdImagePtr im, int *brect, int fg, char *fontname, double ptsize, double angle, int x, int y, char *string) */
/*  do_gdImageStringFT --- provide dynamically loaded do_gdImageStringFT() builtin for gawk */

// We return here "" if things are OK, or an error message otherwise !!!

static awk_value_t *
do_gdImageStringFT(int nargs, awk_value_t *result)
{
	int n;
	char * errStr;

	gdImagePtr im;
	awk_value_t brect_a;
	int brect[8];
	awk_value_t fg;
	awk_value_t fontName;
	awk_value_t ptsize, angle;
	awk_value_t x, y;
	awk_value_t string;
	int str_len;

	if (do_lint && nargs != 9)
		lintwarn(ext_id, "do_gdImageStringFT: called with incorrect number of arguments");

	if (!get_argument(0, AWK_STRING, &string)) {
		set_ERRNO(_("gdImageStringFT first argument must be empty or an image handle"));
		return make_string_malloc("unknown image handle", 20, result);
	}
    str_len = string.str_value.len;

    if (!str_len) { /* empty AWK strings mean NULL img pointers in GD */
		im = NULL;
    }
	else	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageStringFT called with unknown image handle"));
		return make_string_malloc("unknown image handle", 20, result);
	}

	/* gdImageStringFT accepts either NULL or valid img pointers */

	/* brect array to hold results is second argument */
	if (!get_argument(1, AWK_ARRAY, &brect_a)) {
		static char emsg[512];
		snprintf(emsg, sizeof(emsg), _("gdImageStringFT: 2nd argument must be an array"));
		return make_string_malloc(emsg, strlen(emsg), result);
	}
	/* empty out the array */
	clear_array(brect_a.array_cookie);

#define GETARG(N, T, X) 	\
	if (!get_argument(N, AWK_##T, &X)) {	\
		char emsg[512];	\
		snprintf(emsg, sizeof(emsg), \
			 _("%s: argument #%d must be a %s"),	\
			__func__+3, N+1, #T);	\
		return make_string_malloc(emsg, strlen(emsg), result);	\
	}

	GETARG(2, NUMBER, fg)
	GETARG(3, STRING, fontName)
	GETARG(4, NUMBER, ptsize)
	GETARG(5, NUMBER, angle)
	GETARG(6, NUMBER, x)
	GETARG(7, NUMBER, y)
	GETARG(8, STRING, string)
#undef GETARG

	errStr = gdImageStringFT(im, brect, fg.num_value, fontName.str_value.str, ptsize.num_value, angle.num_value, x.num_value, y.num_value, string.str_value.str);
	if (!(errStr))
		errStr="";

	/* fill in the array brect_a */
	for (n=0; n<8; n++) {
		awk_value_t idx, val;
		set_array_element(brect_a.array_cookie, make_number(n, &idx),
				  make_number(brect[n], &val));
	}

	/* Set the return value */
	return make_string_malloc(errStr, strlen(errStr), result);
}

/* int gdImageColorAllocate(gdImagePtr im, int r, int g, int b)  */
/*  do_gdImageColorAllocate --- provide dynamically loaded do_gdImageColorAllocate() builtin for gawk */

static awk_value_t *
do_gdImageColorAllocate(int nargs, awk_value_t *result)
{
	gdImagePtr im;
	awk_value_t n[3];
	size_t i;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, "do_gdImageColorAllocate: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageColorAllocate called with unknown image handle"));
		RETURN_NOK;
	}
	for (i = 0; i < sizeof(n)/sizeof(n[0]); i++) {
		if (!get_argument(1+i, AWK_NUMBER, &n[i])) {
			char emsg[512];
			snprintf(emsg, sizeof(emsg), _("%s missing required numeric argument #%zu"), __func__+3, i+2);
			set_ERRNO(emsg);
			RETURN_NOK;
		}
	}
	return make_number(gdImageColorAllocate(im, n[0].num_value,
						n[1].num_value, n[2].num_value),
			   result);
}

/* void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)  */
/*  do_gdImageFilledRectangle --- provide dynamically loaded do_gdImageFilledRectangle() builtin for gawk */

static awk_value_t *
do_gdImageFilledRectangle(int nargs, awk_value_t *result)
{
	gdImagePtr im;
	awk_value_t n[5];
	size_t i;

	if (do_lint && nargs != 6)
		lintwarn(ext_id, "do_gdImageFilledRectangle: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageFilledRectangle called with unknown image handle"));
		RETURN_NOK;
	}

	for (i = 0; i < sizeof(n)/sizeof(n[0]); i++) {
		if (!get_argument(1+i, AWK_NUMBER, &n[i])) {
			char emsg[512];
			snprintf(emsg, sizeof(emsg), _("%s missing required numeric argument #%zu"), __func__+3, i+2);
			set_ERRNO(emsg);
			RETURN_NOK;
		}
	}

	gdImageFilledRectangle(im,
			       n[0].num_value,
			       n[1].num_value,
			       n[2].num_value,
			       n[3].num_value,
			       n[4].num_value);
	RETURN_OK;
}

/* void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)  */
/*  do_gdImageRectangle --- provide dynamically loaded do_gdImageRectangle() builtin for gawk */

static awk_value_t *
do_gdImageRectangle(int nargs, awk_value_t *result)
{
	gdImagePtr im;
	awk_value_t n[5];
	size_t i;

	if (do_lint && nargs != 6)
		lintwarn(ext_id, "do_gdImageRectangle: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageRectangle called with unknown image handle"));
		RETURN_NOK;
	}
	for (i = 0; i < sizeof(n)/sizeof(n[0]); i++) {
		if (!get_argument(1+i, AWK_NUMBER, &n[i])) {
			char emsg[512];
			snprintf(emsg, sizeof(emsg), _("%s missing required numeric argument #%zu"), __func__+3, i+2);
			set_ERRNO(emsg);
			RETURN_NOK;
		}
	}
	gdImageRectangle(im,
			 n[0].num_value,
			 n[1].num_value,
			 n[2].num_value,
			 n[3].num_value,
			 n[4].num_value);
	RETURN_OK;
}

/* void gdImageSetAntiAliasedDontBlend(gdImagePtr im, int c, int dont_blend)  */
/*  do_gdImageSetAntiAliasedDontBlend --- provide dynamically loaded do_gdImageSetAntiAliasedDontBlend() builtin for gawk */

static awk_value_t *
do_gdImageSetAntiAliasedDontBlend(int nargs, awk_value_t *result)
{
	int ret = GD_OK;
	gdImagePtr im;
	awk_value_t c, dont_blend;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, "do_gdImageSetAntiAliasedDontBlend: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageSetAntiAliasedDontBlend called with unknown image handle"));
		ret = GD_NOK;
	}
	else if (!get_argument(1, AWK_NUMBER, &c)) {
		set_ERRNO(_("gdImageSetThickness needs a 2nd integer color argument"));
		ret = GD_NOK;
	}
	else if (!get_argument(2, AWK_NUMBER, &dont_blend)) {
		set_ERRNO(_("gdImageSetThickness needs a 3rd dont_blend argument"));
		ret = GD_NOK;
	}
	else
		gdImageSetAntiAliasedDontBlend(im, c.num_value,
					       dont_blend.num_value);

	return make_number(ret, result);
}

/* void gdImageSetThickness(gdImagePtr im, int thickness)   */
/*  do_gdImageSetThickness --- provide dynamically loaded do_gdImageSetThickness() builtin for gawk */

static awk_value_t *
do_gdImageSetThickness(int nargs, awk_value_t *result)
{
	int ret = GD_OK;
	gdImagePtr im;
	awk_value_t thickness;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, "do_gdImageSetThickness: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageSetThickness called with unknown image handle"));
		ret = GD_NOK;
	}
	else if (!get_argument(1, AWK_NUMBER, &thickness)) {
		set_ERRNO(_("gdImageSetThickness needs a 2nd integer thickness argument"));
		ret = GD_NOK;
	}
	else
		gdImageSetThickness(im, thickness.num_value);

	return make_number(ret, result);
}

/* void gdImageSX(gdImagePtr im)   */
/*  do_gdImageSX --- provide dynamically loaded do_gdImageSX() builtin for gawk */

static awk_value_t *
do_gdImageSX(int nargs, awk_value_t *result)
{
	int ret;
	gdImagePtr im;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "gdImageSX: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageSX called with unknown image handle"));
		ret = GD_NOK;
	}
	else
		ret = gdImageSX(im);

	return make_number(ret, result);
}

/* void gdImageSY(gdImagePtr im)   */
/*  do_gdImageSY --- provide dynamically loaded do_gdImageSY() builtin for gawk */

static awk_value_t *
do_gdImageSY(int nargs, awk_value_t *result)
{
	int ret;
	gdImagePtr im;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "gdImageSY: called with incorrect number of arguments");

	if (!(im = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageSY called with unknown image handle"));
		ret = GD_NOK;
	}
	else
		ret = gdImageSY(im);

	return make_number(ret, result);
}

/* int gdImageCompare(gdImagePtr im1, gdImagePtr im2) */
/*  do_gdImageCompare --- provide dynamically loaded do_gdImageCompare() builtin for gawk */

static awk_value_t *
do_gdImageCompare(int nargs, awk_value_t *result)
{
	gdImagePtr im1, im2;
	int ret;

	if (do_lint && nargs != 10)
		lintwarn(ext_id, "do_gdImageCompare: called with incorrect number of arguments");

	if (!(im1 = find_handle(gdimgs, 0))) {
		set_ERRNO(_("gdImageCompare called with unknown im1 image handle"));
		ret = 1<<14;
	}
	else if (!(im2 = find_handle(gdimgs, 1))) {
		set_ERRNO(_("gdImageCompare called with unknown im2 image handle"));
		ret = 1<<15;
	}
	else {
		ret = gdImageCompare(im1, im2);
	}
	return make_number(ret, result);
}

static awk_ext_func_t func_table[] = {
	{ "gdImageCreateFromFile", do_gdImageCreateFromFile, 1 },
	{ "gdImageDestroy", do_gdImageDestroy, 1 },
	{ "gdImageCreateTrueColor", do_gdImageCreateTrueColor, 2 },
	{ "gdImageCopyResampled", do_gdImageCopyResampled, 10 },
	{ "gdImagePngName", do_gdImagePngName, 2 },
	{ "gdImageStringFT", do_gdImageStringFT, 9 },
	{ "gdImageColorAllocate", do_gdImageColorAllocate, 4 },

	{ "gdImageFilledRectangle", do_gdImageFilledRectangle, 6 },
	{ "gdImageRectangle", do_gdImageRectangle, 6 },
	{ "gdImageSetAntiAliasedDontBlend", do_gdImageSetAntiAliasedDontBlend, 3 },
	{ "gdImageSetThickness", do_gdImageSetThickness, 2 },
	{ "gdImageSX", do_gdImageSX, 1 },
	{ "gdImageSY", do_gdImageSY, 1 },
	{ "gdImageCompare", do_gdImageCompare, 2 },
};

static awk_bool_t
init_my_module(void)
{
	/* strhash_create exits on failure, so no need to check return code */
	gdimgs = strhash_create(0);
	return 1;
}

static awk_bool_t (*init_func)(void) = init_my_module;

dl_load_func(func_table, gd, "")
