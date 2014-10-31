/*
 * pdf.c - Subset of libharu PDF Library
 *
 *  Author: Hiroshi Saito
 *
 *  HDF_PDF library with configure set options as below.
 * ./configure --with-zlib --with-png
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2004 the Free Software Foundation, Inc.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "common.h"

#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif

#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif

#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif

#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif

#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif

#include "pdf.h"

#define RETURN_NOK return make_number(PDF_NOK, result)
#define RETURN_OK  return make_number(PDF_OK, result)

/* const variables for HPDF. */
static awk_scalar_t HPDF_TRUE_node;
static awk_scalar_t HPDF_FALSE_node;

/* defalt page-size */
static awk_scalar_t HPDF_DEF_PAGE_WIDTH_node;
static awk_scalar_t HPDF_DEF_PAGE_HEIGHT_node;

/* compression mode */
static awk_scalar_t HPDF_COMP_NONE_node;
static awk_scalar_t HPDF_COMP_TEXT_node;
static awk_scalar_t HPDF_COMP_IMAGE_node;
static awk_scalar_t HPDF_COMP_METADATA_node;
static awk_scalar_t HPDF_COMP_ALL_node;

/* InfoType */
static awk_scalar_t HPDF_INFO_CREATION_DATE_node;
static awk_scalar_t HPDF_INFO_MOD_DATE_node;
static awk_scalar_t HPDF_INFO_AUTHOR_node;
static awk_scalar_t HPDF_INFO_CREATOR_node;
static awk_scalar_t HPDF_INFO_PRODUCER_node;
static awk_scalar_t HPDF_INFO_TITLE_node;
static awk_scalar_t HPDF_INFO_SUBJECT_node;
static awk_scalar_t HPDF_INFO_KEYWORDS_node;

/* PdfVer */
static awk_scalar_t HPDF_VER_12_node;
static awk_scalar_t HPDF_VER_13_node;
static awk_scalar_t HPDF_VER_14_node;
static awk_scalar_t HPDF_VER_15_node;
static awk_scalar_t HPDF_VER_16_node;
static awk_scalar_t HPDF_VER_17_node;

/* EncryptMode */
static awk_scalar_t HPDF_ENCRYPT_R2_node;
static awk_scalar_t HPDF_ENCRYPT_R3_node;

/* PageSizes  */
static awk_scalar_t HPDF_PAGE_SIZE_LETTER_node;
static awk_scalar_t HPDF_PAGE_SIZE_LEGAL_node;
static awk_scalar_t HPDF_PAGE_SIZE_A3_node;
static awk_scalar_t HPDF_PAGE_SIZE_A4_node;
static awk_scalar_t HPDF_PAGE_SIZE_A5_node;
static awk_scalar_t HPDF_PAGE_SIZE_B4_node;
static awk_scalar_t HPDF_PAGE_SIZE_B5_node;
static awk_scalar_t HPDF_PAGE_SIZE_EXECUTIVE_node;
static awk_scalar_t HPDF_PAGE_SIZE_US4x6_node;
static awk_scalar_t HPDF_PAGE_SIZE_US4x8_node;
static awk_scalar_t HPDF_PAGE_SIZE_US5x7_node;
static awk_scalar_t HPDF_PAGE_SIZE_COMM10_node;

/*  ColorSpace */
static awk_scalar_t HPDF_CS_DEVICE_GRAY_node;
static awk_scalar_t HPDF_CS_DEVICE_RGB_node;
static awk_scalar_t HPDF_CS_DEVICE_CMYK_node;
static awk_scalar_t HPDF_CS_CAL_GRAY_node;
static awk_scalar_t HPDF_CS_CAL_RGB_node;
static awk_scalar_t HPDF_CS_LAB_node;
static awk_scalar_t HPDF_CS_ICC_BASED_node;
static awk_scalar_t HPDF_CS_SEPARATION_node;
static awk_scalar_t HPDF_CS_DEVICE_N_node;
static awk_scalar_t HPDF_CS_INDEXED_node;
static awk_scalar_t HPDF_CS_PATTERN_node;

/*  LineCap */
static awk_scalar_t HPDF_BUTT_END_node;
static awk_scalar_t HPDF_ROUND_END_node;
static awk_scalar_t HPDF_PROJECTING_SCUARE_END_node;

/*  _LineJoin */
static awk_scalar_t HPDF_MITER_JOIN_node;
static awk_scalar_t HPDF_ROUND_JOIN_node;
static awk_scalar_t HPDF_BEVEL_JOIN_node;

/*  TextRenderingMode */
static awk_scalar_t HPDF_FILL_node;
static awk_scalar_t HPDF_STROKE_node;
static awk_scalar_t HPDF_FILL_THEN_STROKE_node;
static awk_scalar_t HPDF_INVISIBLE_node;
static awk_scalar_t HPDF_FILL_CLIPPING_node;
static awk_scalar_t HPDF_STROKE_CLIPPING_node;
static awk_scalar_t HPDF_FILL_STROKE_CLIPPING_node;
static awk_scalar_t HPDF_CLIPPING_node;

/*  WritingMode */
static awk_scalar_t HPDF_WMODE_HORIZONTAL_node;
static awk_scalar_t HPDF_WMODE_VERTICAL_node;

/*  PageLayout */
static awk_scalar_t HPDF_PAGE_LAYOUT_SINGLE_node;
static awk_scalar_t HPDF_PAGE_LAYOUT_ONE_COLUMN_node;
static awk_scalar_t HPDF_PAGE_LAYOUT_TWO_COLUMN_LEFT_node;
static awk_scalar_t HPDF_PAGE_LAYOUT_TWO_COLUMN_RIGHT_node;

/* PageMode */
static awk_scalar_t HPDF_PAGE_MODE_USE_NONE_node;
static awk_scalar_t HPDF_PAGE_MODE_USE_OUTLINE_node;
static awk_scalar_t HPDF_PAGE_MODE_USE_THUMBS_node;
static awk_scalar_t HPDF_PAGE_MODE_FULL_SCREEN_node;

/* PageNumStyle */
static awk_scalar_t HPDF_PAGE_NUM_STYLE_DECIMAL_node;
static awk_scalar_t HPDF_PAGE_NUM_STYLE_UPPER_ROMAN_node;
static awk_scalar_t HPDF_PAGE_NUM_STYLE_LOWER_ROMAN_node;
static awk_scalar_t HPDF_PAGE_NUM_STYLE_UPPER_LETTERS_node;
static awk_scalar_t HPDF_PAGE_NUM_STYLE_LOWER_LETTERS_node;

/*  DestinationType */
static awk_scalar_t HPDF_XYZ_node;
static awk_scalar_t HPDF_FIT_node;
static awk_scalar_t HPDF_FIT_H_node;
static awk_scalar_t HPDF_FIT_V_node;
static awk_scalar_t HPDF_FIT_R_node;
static awk_scalar_t HPDF_FIT_B_node;
static awk_scalar_t HPDF_FIT_BH_node;
static awk_scalar_t HPDF_FIT_BV_node;

/* AnnotType */
static awk_scalar_t HPDF_ANNOT_TEXT_NOTES_node;
static awk_scalar_t HPDF_ANNOT_LINK_node;
static awk_scalar_t HPDF_ANNOT_SOUND_node;
static awk_scalar_t HPDF_ANNOT_FREE_TEXT_node;
static awk_scalar_t HPDF_ANNOT_STAMP_node;
static awk_scalar_t HPDF_ANNOT_SQUARE_node;
static awk_scalar_t HPDF_ANNOT_CIRCLE_node;
static awk_scalar_t HPDF_ANNOT_STRIKE_OUT_node;
static awk_scalar_t HPDF_ANNOT_HIGHTLIGHT_node;
static awk_scalar_t HPDF_ANNOT_UNDERLINE_node;
static awk_scalar_t HPDF_ANNOT_INK_node;
static awk_scalar_t HPDF_ANNOT_FILE_ATTACHMENT_node;
static awk_scalar_t HPDF_ANNOT_POPUP_node;
static awk_scalar_t HPDF_ANNOT_3D_node;

/*  AnnotFlgs  */
static awk_scalar_t HPDF_ANNOT_INVISIBLE_node;
static awk_scalar_t HPDF_ANNOT_HIDDEN_node;
static awk_scalar_t HPDF_ANNOT_PRINT_node;
static awk_scalar_t HPDF_ANNOT_NOZOOM_node;
static awk_scalar_t HPDF_ANNOT_NOROTATE_node;
static awk_scalar_t HPDF_ANNOT_NOVIEW_node;
static awk_scalar_t HPDF_ANNOT_READONLY_node;

/* AnnotHighlightMode */
static awk_scalar_t HPDF_ANNOT_NO_HIGHTLIGHT_node;
static awk_scalar_t HPDF_ANNOT_INVERT_BOX_node;
static awk_scalar_t HPDF_ANNOT_INVERT_BORDER_node;
static awk_scalar_t HPDF_ANNOT_DOWN_APPEARANCE_node;

/* AnnotIcon */
static awk_scalar_t HPDF_ANNOT_ICON_COMMENT_node;
static awk_scalar_t HPDF_ANNOT_ICON_KEY_node;
static awk_scalar_t HPDF_ANNOT_ICON_NOTE_node;
static awk_scalar_t HPDF_ANNOT_ICON_HELP_node;
static awk_scalar_t HPDF_ANNOT_ICON_NEW_PARAGRAPH_node;
static awk_scalar_t HPDF_ANNOT_ICON_PARAGRAPH_node;
static awk_scalar_t HPDF_ANNOT_ICON_INSERT_node;

/* BSSubtype */
static awk_scalar_t HPDF_BS_SOLID_node;
static awk_scalar_t HPDF_BS_DASHED_node;
static awk_scalar_t HPDF_BS_BEVELED_node;
static awk_scalar_t HPDF_BS_INSET_node;
static awk_scalar_t HPDF_BS_UNDERLINED_node;

/* BlendMode */
static awk_scalar_t HPDF_BM_NORMAL_node;
static awk_scalar_t HPDF_BM_MULTIPLY_node;
static awk_scalar_t HPDF_BM_SCREEN_node;
static awk_scalar_t HPDF_BM_OVERLAY_node;
static awk_scalar_t HPDF_BM_DARKEN_node;
static awk_scalar_t HPDF_BM_LIGHTEN_node;
static awk_scalar_t HPDF_BM_COLOR_DODGE_node;
static awk_scalar_t HPDF_BM_COLOR_BUM_node;
static awk_scalar_t HPDF_BM_HARD_LIGHT_node;
static awk_scalar_t HPDF_BM_SOFT_LIGHT_node;
static awk_scalar_t HPDF_BM_DIFFERENCE_node;
static awk_scalar_t HPDF_BM_EXCLUSHON_node;

/* _TransitionStyle */
static awk_scalar_t HPDF_TS_WIPE_RIGHT_node;
static awk_scalar_t HPDF_TS_WIPE_UP_node;
static awk_scalar_t HPDF_TS_WIPE_LEFT_node;
static awk_scalar_t HPDF_TS_WIPE_DOWN_node;
static awk_scalar_t HPDF_TS_BARN_DOORS_HORIZONTAL_OUT_node;
static awk_scalar_t HPDF_TS_BARN_DOORS_HORIZONTAL_IN_node;
static awk_scalar_t HPDF_TS_BARN_DOORS_VERTICAL_OUT_node;
static awk_scalar_t HPDF_TS_BARN_DOORS_VERTICAL_IN_node;
static awk_scalar_t HPDF_TS_BOX_OUT_node;
static awk_scalar_t HPDF_TS_BOX_IN_node;
static awk_scalar_t HPDF_TS_BLINDS_HORIZONTAL_node;
static awk_scalar_t HPDF_TS_BLINDS_VERTICAL_node;
static awk_scalar_t HPDF_TS_DISSOLVE_node;
static awk_scalar_t HPDF_TS_GLITTER_RIGHT_node;
static awk_scalar_t HPDF_TS_GLITTER_DOWN_node;
static awk_scalar_t HPDF_TS_GLITTER_TOP_LEFT_TO_BOTTOM_RIGHT_node;
static awk_scalar_t HPDF_TS_REPLACE_node;

/*  PageDirection */
static awk_scalar_t HPDF_PAGE_PORTRAIT_node;
static awk_scalar_t HPDF_PAGE_LANDSCAPE_node;

/* EncoderType */
static awk_scalar_t HPDF_ENCODER_TYPE_SINGLE_BYTE_node;
static awk_scalar_t HPDF_ENCODER_TYPE_DOUBLE_BYTE_node;
static awk_scalar_t HPDF_ENCODER_TYPE_UNINITIALIZED_node;
static awk_scalar_t HPDF_ENCODER_UNKNOWN_node;

/* ByteType */
static awk_scalar_t HPDF_BYTE_TYPE_SINGLE_node;
static awk_scalar_t HPDF_BYTE_TYPE_LEAD_node;
static awk_scalar_t HPDF_BYTE_TYPE_TRIAL_node;
static awk_scalar_t HPDF_BYTE_TYPE_UNKNOWN_node;

/* TextAlignment */
static awk_scalar_t HPDF_TALIGN_LEFT_node;
static awk_scalar_t HPDF_TALIGN_RIGHT_node;
static awk_scalar_t HPDF_TALIGN_CENTER_node;
static awk_scalar_t HPDF_TALIGN_JUSTIFY_node;

static const struct varinit varinit[] = {
	ENTRY(HPDF_TRUE, 1)
	ENTRY(HPDF_FALSE, 1)
	ENTRY(HPDF_DEF_PAGE_WIDTH, 1)
	ENTRY(HPDF_DEF_PAGE_HEIGHT, 1)
	ENTRY(HPDF_COMP_NONE, 1)
	ENTRY(HPDF_COMP_TEXT, 1)
	ENTRY(HPDF_COMP_IMAGE, 1)
	ENTRY(HPDF_COMP_METADATA, 1)
	ENTRY(HPDF_COMP_ALL, 1)
	ENTRY(HPDF_INFO_CREATION_DATE, 1)
	ENTRY(HPDF_INFO_MOD_DATE, 1)
	ENTRY(HPDF_INFO_AUTHOR, 1)
	ENTRY(HPDF_INFO_CREATOR, 1)
	ENTRY(HPDF_INFO_PRODUCER, 1)
	ENTRY(HPDF_INFO_TITLE, 1)
	ENTRY(HPDF_INFO_SUBJECT, 1)
	ENTRY(HPDF_INFO_KEYWORDS, 1)
	ENTRY(HPDF_VER_12, 1)
	ENTRY(HPDF_VER_13, 1)
	ENTRY(HPDF_VER_14, 1)
	ENTRY(HPDF_VER_15, 1)
	ENTRY(HPDF_VER_16, 1)
	ENTRY(HPDF_VER_17, 1)
	ENTRY(HPDF_ENCRYPT_R2, 1)
	ENTRY(HPDF_ENCRYPT_R3, 1)
	ENTRY(HPDF_PAGE_SIZE_LETTER, 1)
	ENTRY(HPDF_PAGE_SIZE_LEGAL, 1)
	ENTRY(HPDF_PAGE_SIZE_A3, 1)
	ENTRY(HPDF_PAGE_SIZE_A4, 1)
	ENTRY(HPDF_PAGE_SIZE_A5, 1)
	ENTRY(HPDF_PAGE_SIZE_B4, 1)
	ENTRY(HPDF_PAGE_SIZE_B5, 1)
	ENTRY(HPDF_PAGE_SIZE_EXECUTIVE, 1)
	ENTRY(HPDF_PAGE_SIZE_US4x6, 1)
	ENTRY(HPDF_PAGE_SIZE_US4x8, 1)
	ENTRY(HPDF_PAGE_SIZE_US5x7, 1)
	ENTRY(HPDF_PAGE_SIZE_COMM10, 1)
	ENTRY(HPDF_CS_DEVICE_GRAY, 1)
	ENTRY(HPDF_CS_DEVICE_RGB, 1)
	ENTRY(HPDF_CS_DEVICE_CMYK, 1)
	ENTRY(HPDF_CS_CAL_GRAY, 1)
	ENTRY(HPDF_CS_CAL_RGB, 1)
	ENTRY(HPDF_CS_LAB, 1)
	ENTRY(HPDF_CS_ICC_BASED, 1)
	ENTRY(HPDF_CS_SEPARATION, 1)
	ENTRY(HPDF_CS_DEVICE_N, 1)
	ENTRY(HPDF_CS_INDEXED, 1)
	ENTRY(HPDF_CS_PATTERN, 1)
	ENTRY(HPDF_BUTT_END, 1)
	ENTRY(HPDF_ROUND_END, 1)
	ENTRY(HPDF_PROJECTING_SCUARE_END, 1)
	ENTRY(HPDF_MITER_JOIN, 1)
	ENTRY(HPDF_ROUND_JOIN, 1)
	ENTRY(HPDF_BEVEL_JOIN, 1)
	ENTRY(HPDF_FILL, 1)
	ENTRY(HPDF_STROKE, 1)
	ENTRY(HPDF_FILL_THEN_STROKE, 1)
	ENTRY(HPDF_INVISIBLE, 1)
	ENTRY(HPDF_FILL_CLIPPING, 1)
	ENTRY(HPDF_STROKE_CLIPPING, 1)
	ENTRY(HPDF_FILL_STROKE_CLIPPING, 1)
	ENTRY(HPDF_CLIPPING, 1)
	ENTRY(HPDF_WMODE_HORIZONTAL, 1)
	ENTRY(HPDF_WMODE_VERTICAL, 1)
	ENTRY(HPDF_PAGE_LAYOUT_SINGLE, 1)
	ENTRY(HPDF_PAGE_LAYOUT_ONE_COLUMN, 1)
	ENTRY(HPDF_PAGE_LAYOUT_TWO_COLUMN_LEFT, 1)
	ENTRY(HPDF_PAGE_LAYOUT_TWO_COLUMN_RIGHT, 1)
	ENTRY(HPDF_PAGE_MODE_USE_NONE, 1)
	ENTRY(HPDF_PAGE_MODE_USE_OUTLINE, 1)
	ENTRY(HPDF_PAGE_MODE_USE_THUMBS, 1)
	ENTRY(HPDF_PAGE_MODE_FULL_SCREEN, 1)
	ENTRY(HPDF_PAGE_NUM_STYLE_DECIMAL, 1)
	ENTRY(HPDF_PAGE_NUM_STYLE_UPPER_ROMAN, 1)
	ENTRY(HPDF_PAGE_NUM_STYLE_LOWER_ROMAN, 1)
	ENTRY(HPDF_PAGE_NUM_STYLE_UPPER_LETTERS, 1)
	ENTRY(HPDF_PAGE_NUM_STYLE_LOWER_LETTERS, 1)
	ENTRY(HPDF_XYZ, 1)
	ENTRY(HPDF_FIT, 1)
	ENTRY(HPDF_FIT_H, 1)
	ENTRY(HPDF_FIT_V, 1)
	ENTRY(HPDF_FIT_R, 1)
	ENTRY(HPDF_FIT_B, 1)
	ENTRY(HPDF_FIT_BH, 1)
	ENTRY(HPDF_FIT_BV, 1)
	ENTRY(HPDF_ANNOT_TEXT_NOTES, 1)
	ENTRY(HPDF_ANNOT_LINK, 1)
	ENTRY(HPDF_ANNOT_SOUND, 1)
	ENTRY(HPDF_ANNOT_FREE_TEXT, 1)
	ENTRY(HPDF_ANNOT_STAMP, 1)
	ENTRY(HPDF_ANNOT_SQUARE, 1)
	ENTRY(HPDF_ANNOT_CIRCLE, 1)
	ENTRY(HPDF_ANNOT_STRIKE_OUT, 1)
	ENTRY(HPDF_ANNOT_HIGHTLIGHT, 1)
	ENTRY(HPDF_ANNOT_UNDERLINE, 1)
	ENTRY(HPDF_ANNOT_INK, 1)
	ENTRY(HPDF_ANNOT_FILE_ATTACHMENT, 1)
	ENTRY(HPDF_ANNOT_POPUP, 1)
	ENTRY(HPDF_ANNOT_3D, 1)
	ENTRY(HPDF_ANNOT_INVISIBLE, 1)
	ENTRY(HPDF_ANNOT_HIDDEN, 1)
	ENTRY(HPDF_ANNOT_PRINT, 1)
	ENTRY(HPDF_ANNOT_NOZOOM, 1)
	ENTRY(HPDF_ANNOT_NOROTATE, 1)
	ENTRY(HPDF_ANNOT_NOVIEW, 1)
	ENTRY(HPDF_ANNOT_READONLY, 1)
	ENTRY(HPDF_ANNOT_NO_HIGHTLIGHT, 1)
	ENTRY(HPDF_ANNOT_INVERT_BOX, 1)
	ENTRY(HPDF_ANNOT_INVERT_BORDER, 1)
	ENTRY(HPDF_ANNOT_DOWN_APPEARANCE, 1)
	ENTRY(HPDF_ANNOT_ICON_COMMENT, 1)
	ENTRY(HPDF_ANNOT_ICON_KEY, 1)
	ENTRY(HPDF_ANNOT_ICON_NOTE, 1)
	ENTRY(HPDF_ANNOT_ICON_HELP, 1)
	ENTRY(HPDF_ANNOT_ICON_NEW_PARAGRAPH, 1)
	ENTRY(HPDF_ANNOT_ICON_PARAGRAPH, 1)
	ENTRY(HPDF_ANNOT_ICON_INSERT, 1)
	ENTRY(HPDF_BS_SOLID, 1)
	ENTRY(HPDF_BS_DASHED, 1)
	ENTRY(HPDF_BS_BEVELED, 1)
	ENTRY(HPDF_BS_INSET, 1)
	ENTRY(HPDF_BS_UNDERLINED, 1)
	ENTRY(HPDF_BM_NORMAL, 1)
	ENTRY(HPDF_BM_MULTIPLY, 1)
	ENTRY(HPDF_BM_SCREEN, 1)
	ENTRY(HPDF_BM_OVERLAY, 1)
	ENTRY(HPDF_BM_DARKEN, 1)
	ENTRY(HPDF_BM_LIGHTEN, 1)
	ENTRY(HPDF_BM_COLOR_DODGE, 1)
	ENTRY(HPDF_BM_COLOR_BUM, 1)
	ENTRY(HPDF_BM_HARD_LIGHT, 1)
	ENTRY(HPDF_BM_SOFT_LIGHT, 1)
	ENTRY(HPDF_BM_DIFFERENCE, 1)
	ENTRY(HPDF_BM_EXCLUSHON, 1)
	ENTRY(HPDF_TS_WIPE_RIGHT, 1)
	ENTRY(HPDF_TS_WIPE_UP, 1)
	ENTRY(HPDF_TS_WIPE_LEFT, 1)
	ENTRY(HPDF_TS_WIPE_DOWN, 1)
	ENTRY(HPDF_TS_BARN_DOORS_HORIZONTAL_OUT, 1)
	ENTRY(HPDF_TS_BARN_DOORS_HORIZONTAL_IN, 1)
	ENTRY(HPDF_TS_BARN_DOORS_VERTICAL_OUT, 1)
	ENTRY(HPDF_TS_BARN_DOORS_VERTICAL_IN, 1)
	ENTRY(HPDF_TS_BOX_OUT, 1)
	ENTRY(HPDF_TS_BOX_IN, 1)
	ENTRY(HPDF_TS_BLINDS_HORIZONTAL, 1)
	ENTRY(HPDF_TS_BLINDS_VERTICAL, 1)
	ENTRY(HPDF_TS_DISSOLVE, 1)
	ENTRY(HPDF_TS_GLITTER_RIGHT, 1)
	ENTRY(HPDF_TS_GLITTER_DOWN, 1)
	ENTRY(HPDF_TS_GLITTER_TOP_LEFT_TO_BOTTOM_RIGHT, 1)
	ENTRY(HPDF_TS_REPLACE, 1)
	ENTRY(HPDF_PAGE_PORTRAIT, 1)
	ENTRY(HPDF_PAGE_LANDSCAPE, 1)
	ENTRY(HPDF_ENCODER_TYPE_SINGLE_BYTE, 1)
	ENTRY(HPDF_ENCODER_TYPE_DOUBLE_BYTE, 1)
	ENTRY(HPDF_ENCODER_TYPE_UNINITIALIZED, 1)
	ENTRY(HPDF_ENCODER_UNKNOWN, 1)
	ENTRY(HPDF_BYTE_TYPE_SINGLE, 1)
	ENTRY(HPDF_BYTE_TYPE_LEAD, 1)
	ENTRY(HPDF_BYTE_TYPE_TRIAL, 1)
	ENTRY(HPDF_BYTE_TYPE_UNKNOWN, 1)
	ENTRY(HPDF_TALIGN_LEFT, 1)
	ENTRY(HPDF_TALIGN_RIGHT, 1)
	ENTRY(HPDF_TALIGN_CENTER, 1)
	ENTRY(HPDF_TALIGN_JUSTIFY, 1)
};

/* strhas handles */
static strhash *pdfdatas;		/* HPDF_Doc */
static strhash *pdfpages;		/* HPDF_Page */
static strhash *pdfimages;		/* HPDF_Image */
static strhash *pdffonts;		/* HPDF_Font */
static strhash *pdfoutlines;		/* HPDF_Outline */
static strhash *pdfencoders;		/* HPDF_Encoder */
static strhash *pdfdestinations;	/* HPDF_Destination */
static strhash *pdfannotations;		/* HPDF_Annotation */
static strhash *pdfextgstates;		/* HPDF_ExtGState */
static strhash *pdfontdefs;		/* HPDF_FontDef */

/* 
 * ----------------------------------------------------------------------------------
 * load constant valiable 
 * ----------------------------------------------------------------------------------
 */

static void
load_vars(void)
{
	const struct varinit *vp;
	size_t i;

	for(vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++)
	{
		awk_value_t val;

		if(vp->read_only)
		{
			if (!gawk_varinit_constant(vp->name,
						   make_number(vp->dfltval, &val), vp->spec))
				fatal(ext_id, _("Cannot create PDF reserved scalar constant `%s'."), vp->name);
		}
		else if (!gawk_varinit_scalar(vp->name,
						  make_number(vp->dfltval, &val), 1, vp->spec))
			fatal(ext_id, _("PDF reserved scalar variable `%s' already used with incompatible type."), vp->name);
	}
}

/* 
 * ----------------------------------------------------------------------------------
 * find_handle is taken from c 
 * ----------------------------------------------------------------------------------
 */

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
data_handle(HPDF_Doc pdf, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfdata%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfdatas, handle, sl, 1)->data = pdf;
	return sl;
}

static size_t
page_handle(HPDF_Page page, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfpage%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfpages, handle, sl, 1)->data = page;
	return sl;
}

static size_t
image_handle(HPDF_Image image, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfimage%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfimages, handle, sl, 1)->data = image;
	return sl;
}

static size_t
font_handle(HPDF_Font font, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdffont%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdffonts, handle, sl, 1)->data = font;
	return sl;
}

static size_t
outline_handle(HPDF_Outline outline, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfoutline%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfoutlines, handle, sl, 1)->data = outline;
	return sl;
}

static size_t
encoder_handle(HPDF_Encoder encoder, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfencoder%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfencoders, handle, sl, 1)->data = encoder;
	return sl;
}

static size_t
destination_handle(HPDF_Destination destination, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfdestination%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfdestinations, handle, sl, 1)->data = destination;
	return sl;
}

static size_t
annotation_handle(HPDF_Annotation annotation, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfannotation%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfannotations, handle, sl, 1)->data = annotation;
	return sl;
}

static size_t
extgstate_handle(HPDF_ExtGState extgstate, char *handle, size_t handle_size)
{
	static unsigned long hnum = 0;
	size_t sl;

	snprintf(handle, handle_size, "pdfextgstate%lu", hnum++);
	sl = strlen(handle);
	strhash_get(pdfextgstates, handle, sl, 1)->data = extgstate;
	return sl;
}

/* 
 * ----------------------------------------------------------------------------------
 * do HPDF API 
 * ----------------------------------------------------------------------------------
 */

 /* const char * HPDF_GetVersion(void); */
static awk_value_t *
do_HPDF_GetVersion(int nargs, awk_value_t *result)
{
	const char *version = NULL;

	if (do_lint && nargs != 0)
		lintwarn(ext_id, _("HPDF_GetVersion: error handle does not have a built-in."));

	version = HPDF_GetVersion();

	if(version) {
		return make_string_malloc(version, strlen(version), result);
	}

	set_ERRNO(_("HPDF_GetVersion failed"));
	RET_NULSTR;
}

/* HPDF_Doc HPDF_New(HPDF_Error_Handler user_error_fn, void *user_data); */
static awk_value_t *
do_HPDF_New(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t handle;

	if (do_lint && nargs > 3)
		lintwarn(ext_id, _("HPDF_New: error handle does not have a built-in."));

	/* handler not implementation. */
	if (get_argument(0, AWK_STRING, &handle)) {
		if (!strncmp(handle.str_value.str, "0", 1))
			lintwarn(ext_id, _("HPDF_New: does not use handler argument, sorry.."));
	}

	pdf = HPDF_New(NULL, NULL);

	if(pdf) {
		char hdl[HANDLE_SIZE];
		size_t hlen = data_handle(pdf, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_New failed"));
	RET_NULSTR;
}

/* void HPDF_Free(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_Free(int nargs, awk_value_t *result)
{
	awk_value_t handle;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Free: called with incorrect number of arguments"));

	if (!get_argument(0, AWK_STRING, &handle)) {
		set_ERRNO(_("HPDF_Free: missing required pdf handle argument"));
		RETURN_NOK;
	}

	if (strhash_delete(pdfdatas, handle.str_value.str, handle.str_value.len,
			   (strhash_delete_func)HPDF_Free, NULL) < 0) {
		set_ERRNO(_("HPDF_Free called with unknown pdf handle"));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_NewDoc(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_NewDoc(int nargs, awk_value_t *result)
{

	HPDF_Doc pdf_old = NULL;
	HPDF_Doc pdf_new = NULL;
	awk_value_t handle;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_NewDoc: called with incorrect number of arguments"));

	
	if (!get_argument(0, AWK_STRING, &handle)) {
		set_ERRNO(_("HPDF_Free: missing required pdf handle argument"));
		RETURN_NOK;
	}	

	pdf_new = pdf_old = find_handle(pdfdatas, 0);

	if (HPDF_NewDoc(pdf_old) != HPDF_OK) {
		set_ERRNO(_("HPDF_NewDoc: can't reset pdf document."));
		RETURN_NOK;
	}

	if(!pdf_new) {
		char hdl[HANDLE_SIZE];
		size_t hlen = data_handle(pdf_old, hdl, sizeof(hdl));
		make_string_malloc(hdl, hlen, result);
	}

	RETURN_OK;
}

/* void HPDF_FreeDoc(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_FreeDoc(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t handle;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_FreeDoc: called with incorrect number of arguments"));

	if (!get_argument(0, AWK_STRING, &handle)) {
		set_ERRNO(_("HPDF_FreeDoc: missing required pdf handle argument"));
		RETURN_NOK;
	}	

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_FreeDoc called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (strhash_delete(pdfdatas, handle.str_value.str, handle.str_value.len,
			   (strhash_delete_func)HPDF_FreeDoc, NULL) < 0) {
		set_ERRNO(_("HPDF_FreeDoc called with unknown pdf handle"));
		RETURN_NOK;
	}

	RETURN_OK;
}


/* HPDF_BOOL HPDF_HasDoc(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_HasDoc(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_HasDoc: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_HasDoc called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!HPDF_HasDoc(pdf)) {
		set_ERRNO(_("HPDF_HasDoc: not has pdf handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* void HPDF_FreeDocAll(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_FreeDocAll(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t handle;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_FreeDocAll: called with incorrect number of arguments"));

	if (!get_argument(0, AWK_STRING, &handle)) {
		set_ERRNO(_("HPDF_FreeDocAll: missing required pdf handle argument"));
		RETURN_NOK;
	}

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_FreeDocAll called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (strhash_delete(pdfdatas, handle.str_value.str, handle.str_value.len,
			   (strhash_delete_func)HPDF_FreeDocAll, NULL) < 0) {
		set_ERRNO(_("HPDF_FreeDocAll called with unknown pdf handle"));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_GetContents(HPDF_Doc pdf, HPDF_BYTE *buf, HPDF_UINT32 *size); */
static awk_value_t *
do_HPDF_GetContents(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t buf;
	awk_value_t size;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_GetContents: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetContents called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &buf)) {
		set_ERRNO(_("HPDF_GetContents: missing required buf argument"));
		RETURN_NOK;
	}
	if (!get_argument(2, AWK_NUMBER, &size)) {
		set_ERRNO(_("HPDF_GetContents: missing required size argument"));
		RETURN_NOK;
	}

	if (HPDF_GetContents(pdf, (HPDF_BYTE *)&buf.str_value.str, (HPDF_UINT32 *)&size.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_GetContents: can't get contents."));
		RETURN_NOK;
	}

	RETURN_OK;

}

/* HPDF_STATUS HPDF_SaveToFile(HPDF_Doc pdf, const char *file_name); */
static awk_value_t *
do_HPDF_SaveToFile(int nargs, awk_value_t *result)
{
	awk_value_t fName;
	HPDF_Doc pdf;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SaveToFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SaveToFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &fName)) {
		set_ERRNO(_("HPDF_SaveToFile: missing required filename argument"));
		RETURN_NOK;
	}

	if (HPDF_SaveToFile(pdf, fName.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_SaveToFile: can't save to file."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_GetError(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetError(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_STATUS status;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetError: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetError called with unknown pdf handle"));
		RETURN_NOK;
	}

	status = HPDF_GetError(pdf);
	return make_number(status, result);

}

/* HPDF_STATUS HPDF_GetErrorDetail(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetErrorDetail(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_STATUS status;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetErrorDetail: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetErrorDetail called with unknown pdf handle"));
		RETURN_NOK;
	}
/*
	if (HPDF_GetErrorDetail(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_GetErrorDetail: can't has pdf handle."));
		RETURN_NOK;
	}
*/
	status = HPDF_GetErrorDetail(pdf);
	return make_number(status, result);

}

/* void HPDF_ResetError(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_ResetError(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_ResetError: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_ResetError called with unknown pdf handle"));
		RETURN_NOK;
	}

	HPDF_ResetError(pdf);
	RETURN_OK;
}

/* HPDF_STATUS HPDF_CheckError(HPDF_Error error); */
static awk_value_t *
do_HPDF_CheckError(int nargs, awk_value_t *result)
{
	awk_value_t error_arg;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_CheckError: called with incorrect number of arguments"));

	if (!get_argument(0, AWK_NUMBER, &error_arg)) {
		set_ERRNO(_("HPDF_CheckError: missing required embedding argument"));
		RETURN_NOK;
	}

	if (!HPDF_CheckError((HPDF_Error)&error_arg.num_value)) {
		set_ERRNO(_("HPDF_CheckError: can't has pdf handle."));
		RETURN_NOK;
	}

	RETURN_OK;

}

/* HPDF_STATUS HPDF_SetPagesConfiguration(HPDF_Doc pdf, HPDF_UINT page_per_pages); */
static awk_value_t *
do_HPDF_SetPagesConfiguration(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t page_per_pages;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetPagesConfiguration: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetPagesConfiguration called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &page_per_pages)) {
		set_ERRNO(_("HPDF_SetPagesConfiguration: missing required page_per_pages argument"));
		RETURN_NOK;
	}

	if (HPDF_SetPagesConfiguration(pdf, page_per_pages.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetPagesConfiguration: can't set page configuration."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_Page HPDF_GetPageByIndex(HPDF_Doc pdf, HPDF_UINT index); */
static awk_value_t *
do_HPDF_GetPageByIndex(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Page page;
	awk_value_t index;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_GetPageByIndex: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetPageByIndex called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &index)) {
		set_ERRNO(_("HPDF_GetPageByIndex: layout required index argument"));
		RETURN_NOK;
	}

	page = HPDF_GetPageByIndex(pdf, index.num_value);

	if(page) {
		char hdl[HANDLE_SIZE];
		size_t hlen = page_handle(page, hdl, sizeof(hdl));
		return make_string_malloc((const char *)page, hlen, result);
	}

	set_ERRNO(_("HPDF_GetPageByIndex failed"));
	RET_NULSTR;
}

/*---------------------------------------------------------------------------*/

/* HPDF_PageLayout HPDF_GetPageLayout(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetPageLayout(int nargs, awk_value_t *result)
{

	HPDF_Doc pdf;
	HPDF_PageLayout layout;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetPageLayout: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetPageLayout called with unknown pdf handle"));
		RETURN_NOK;
	}

	layout = HPDF_GetPageLayout(pdf);
	
	if(layout) {
		return make_number(layout, result);
	}

	set_ERRNO(_("HPDF_GetPageLayout failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_STATUS HPDF_SetPageLayout(HPDF_Doc pdf, HPDF_PageLayout layout); */
static awk_value_t *
do_HPDF_SetPageLayout(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t layout;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetPageLayout: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetPageLayout called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &layout)) {
		set_ERRNO(_("HPDF_SetPageLayout: layout required layout argument"));
		RETURN_NOK;
	}

	if (HPDF_SetPagesConfiguration(pdf, layout.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetPagesConfiguration: can't set page layout."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_PageMode HPDF_GetPageMode(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetPageMode(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_PageMode pagemode;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetPageMode: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetPageMode called with unknown pdf handle"));
		RETURN_NOK;
	}

	pagemode = HPDF_GetPageMode(pdf);
	
	if(pagemode) {
		return make_number(pagemode, result);
	}

	set_ERRNO(_("HPDF_GetPageMode failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_STATUS HPDF_SetPageMode(HPDF_Doc pdf, HPDF_PageMode mode); */
static awk_value_t *
do_HPDF_SetPageMode(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t mode;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetPageMode: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetPageMode called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_SetPageMode: layout required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_SetPageMode(pdf, mode.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetPageMode: can't set page layout."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_UINT HPDF_GetViewerPreference(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetViewerPreference(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_UINT viewpre;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetViewerPreference: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetViewerPreference called with unknown pdf handle"));
		RETURN_NOK;
	}

	viewpre = HPDF_GetViewerPreference(pdf);
	
	if(viewpre) {
		return make_number(viewpre, result);
	}

	set_ERRNO(_("HPDF_GetViewerPreference failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_STATUS HPDF_SetViewerPreference(HPDF_Doc pdf, HPDF_UINT value); */
static awk_value_t *
do_HPDF_SetViewerPreference(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetViewerPreferencen: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetViewerPreference called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_SetViewerPreference: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_SetViewerPreference(pdf, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetViewerPreference: can't set viewer preference."));
		RETURN_NOK;
	}

	RETURN_OK;
}


/* HPDF_STATUS HPDF_SetOpenAction(HPDF_Doc pdf, HPDF_Destination open_action); */
static awk_value_t *
do_HPDF_SetOpenAction(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Destination open_action;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetOpenAction: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetOpenAction called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!(open_action = find_handle(pdfdestinations, 1))) {
		set_ERRNO(_("HPDF_SetOpenAction called with unknown open_action handle"));
		RETURN_NOK;
	}

	if (HPDF_SetOpenAction(pdf, open_action) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetOpenAction: can't set open sction."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- page handling -------------------------------------------------------*/

/* HPDF_Page HPDF_GetCurrentPage(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetCurrentPage(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Page page_get;
        strhash_entry *page_out;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetCurrentPage: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetCurrentPage called with unknown pdf handle"));
		RETURN_NOK;
	}

	page_get = HPDF_GetCurrentPage(pdf);

/* DEBUG */

        page_out = strhash_get(pdfimages, (const char *)page_get, strlen((const char *)page_get), 0);

	if(page_out) {
		return make_string_malloc((const char *)page_out, strlen((const char *)page_get), result);
	}

	set_ERRNO(_("HPDF_GetCurrentPage failed"));
	RET_NULSTR;
}

/* HPDF_Page HPDF_AddPage(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_AddPage(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_AddPage: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_AddPage called with unknown pdf handle"));
		RETURN_NOK;
	}

	page = HPDF_AddPage(pdf);

	if(page) {
		char hdl[HANDLE_SIZE];
		size_t hlen = page_handle(page, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_AddPage failed"));
	RET_NULSTR;
}


/* HPDF_Page HPDF_InsertPage(HPDF_Doc pdf, HPDF_Page page); */
static awk_value_t *
do_HPDF_InsertPage(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Page page_in;
	HPDF_Page page_out;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_InsertPage: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_InsertPage called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!(page_in = find_handle(pdfpages, 1))) {
		set_ERRNO(_("HPDF_InsertPage called with unknown page input handle"));
		RETURN_NOK;
	}

	page_out = HPDF_InsertPage(pdf, page_in);

	if(page_out) {
		char hdl[HANDLE_SIZE];
		size_t hlen = page_handle(page_out, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_InsertPage failed"));
	RET_NULSTR;
}


/* HPDF_STATUS HPDF_Page_SetWidth(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetWidth(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetWidth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetWidth called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetWidth: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetWidth(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetWidth: can't set width to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetHeight(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetHeight(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetHeight: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetHeight called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetHeight: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetHeight(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetHeight: can't set width to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetSize(HPDF_Page page, HPDF_PageSizes size, HPDF_PageDirection direction); */
static awk_value_t *
do_HPDF_Page_SetSize(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t size;
	awk_value_t direction;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_SetSize: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetSize called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &size)) {
		set_ERRNO(_("HPDF_Page_SetSize: missing required size argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &direction)) {
		set_ERRNO(_("HPDF_Page_SetSize: missing required size argument"));
		RETURN_NOK;
	}


	if (HPDF_Page_SetSize(page, size.num_value, direction.num_value) != HPDF_OK) { 
		set_ERRNO(_("HPDF_Page_SetSize: can't set size to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetRotate(HPDF_Page page, HPDF_UINT16 angle); */
static awk_value_t *
do_HPDF_Page_SetRotate(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t angle;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetRotate: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetRotate called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &angle)) {
		set_ERRNO(_("HPDF_Page_SetRotate: missing required size argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetRotate(page, angle.num_value) != HPDF_OK) { 
		set_ERRNO(_("HPDF_Page_SetRotate: can't set rotate to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetZoom(HPDF_Page page, HPDF_REAL zoom); */
static awk_value_t *
do_HPDF_Page_SetZoom(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t zoom;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetZoom: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetZoom called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &zoom)) {
		set_ERRNO(_("HPDF_Page_SetZoom: missing required zoom argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetZoom(page, zoom.num_value) != HPDF_OK) { 
		set_ERRNO(_("HPDF_Page_SetZoom: can't set zoom to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- font handling -------------------------------------------------------*/

/* HPDF_Font HPDF_GetFont(HPDF_Doc pdf, const char *font_name, const char *encoding_name); */
static awk_value_t *
do_HPDF_GetFont(int nargs, awk_value_t *result)
{
	awk_value_t font_name;
	awk_value_t encoding_name;
	HPDF_Doc pdf;
	HPDF_Font font;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_GetFont: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetFont called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &font_name)) {
		set_ERRNO(_("HPDF_GetFont: missing required font_name argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_STRING, &encoding_name)) {
		set_ERRNO(_("HPDF_GetFont: missing required encoding_name argument"));
		RETURN_NOK;
	}

	if (encoding_name.str_value.len <= 1)  /* NULL is 0 string.. */
		font = HPDF_GetFont(pdf, font_name.str_value.str, NULL);
	else
		font = HPDF_GetFont(pdf, font_name.str_value.str, encoding_name.str_value.str);

	if(font) {
		char hdl[HANDLE_SIZE];
		size_t hlen = font_handle(font, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_GetFont failed"));
	RET_NULSTR;
}


/* const char* HPDF_LoadType1FontFromFile(HPDF_Doc pdf, const char *afm_file_name, const char *data_file_name); */
static awk_value_t *
do_HPDF_LoadType1FontFromFile(int nargs, awk_value_t *result)
{

	const char *font_name;
	awk_value_t afm_file_name;
	awk_value_t data_file_name;
	
	HPDF_Doc pdf;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_LoadType1FontFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadType1FontFromFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &afm_file_name)) {
		set_ERRNO(_("HPDF_LoadType1FontFromFile: missing required afm_file_name argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_STRING, &data_file_name)) {
		set_ERRNO(_("HPDF_LoadType1FontFromFile: missing required data_file_name argument"));
		RETURN_NOK;
	}

	font_name = HPDF_LoadType1FontFromFile(pdf, afm_file_name.str_value.str, data_file_name.str_value.str);

	if(font_name) {
		return make_string_malloc(font_name, strlen(font_name), result);
	}

	set_ERRNO(_("HPDF_LoadType1FontFromFile failed"));
	RET_NULSTR;

}

/* HPDF_FontDef HPDF_GetTTFontDefFromFile(HPDF_Doc pdf, const char *file_name, HPDF_BOOL embedding); */
static awk_value_t *
do_HPDF_GetTTFontDefFromFile(int nargs, awk_value_t *result)
{
	HPDF_FontDef fdef;
	HPDF_Doc pdf;
	awk_value_t file_name;
	awk_value_t embedding_in;
	HPDF_BOOL embedding;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_GetTTFontDefFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetTTFontDefFromFile called with unknown pdf handle"));
			return make_number(PDF_NOK, result);
	}

	if (!get_argument(1, AWK_STRING, &file_name)) {
		set_ERRNO(_("HPDF_LoadType1FontFromFile: missing required file_name argument"));
			return make_number(PDF_NOK, result);
	}

	if (!get_argument(2, AWK_NUMBER, &embedding_in)) {
		set_ERRNO(_("HPDF_LoadTTFontFromFile: missing required embedding argument"));
			return make_number(PDF_NOK, result);
	}

	if (embedding_in.num_value == 0)
		embedding = HPDF_TRUE;
	else
		embedding = HPDF_FALSE;

	fdef = HPDF_GetTTFontDefFromFile(pdf, (const char *)file_name.str_value.str, (HPDF_BOOL)embedding);
 
	return make_string_malloc((char*)&fdef, strlen((char*)&fdef), result);
}

/* const char* HPDF_LoadTTFontFromFile(HPDF_Doc pdf, const char *file_name, HPDF_BOOL embedding); */
static awk_value_t *
do_HPDF_LoadTTFontFromFile(int nargs, awk_value_t *result)
{

	const char *font_name;
	awk_value_t font_file;
	awk_value_t embedding;
	
	HPDF_Doc pdf;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_LoadTTFontFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadTTFontFromFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &font_file)) {
		set_ERRNO(_("HPDF_LoadTTFontFromFile: missing required font_file argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &embedding)) {
		set_ERRNO(_("HPDF_LoadTTFontFromFile: missing required embedding argument"));
		RETURN_NOK;
	}

	font_name = HPDF_LoadTTFontFromFile(pdf, font_file.str_value.str, embedding.num_value);

	if(font_name) {
		return make_string_malloc(font_name, strlen(font_name), result);
	}
	
	set_ERRNO(_("HPDF_LoadTTFontFromFile failed"));
	RET_NULSTR;

}

/* HPDF_STATUS HPDF_AddPageLabel(HPDF_Doc pdf, HPDF_UINT  page_num, HPDF_PageNumStyle style, HPDF_UINT first_page, const char *prefix); */
static awk_value_t *
do_HPDF_AddPageLabel(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t page_num;
	awk_value_t style;
	awk_value_t first_page;
	awk_value_t prefix;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_AddPageLabel: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_AddPageLabel called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &page_num)) {
		set_ERRNO(_("HPDF_AddPageLabel: missing required page_num argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &style)) {
		set_ERRNO(_("HPDF_AddPageLabel: missing required style argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &first_page)) {
		set_ERRNO(_("HPDF_AddPageLabel: missing required first_page argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_STRING, &prefix)) {
		set_ERRNO(_("HPDF_AddPageLabel: missing required prefix argument"));
		RETURN_NOK;
	}

	if (HPDF_AddPageLabel(pdf, page_num.num_value, style.num_value, first_page.num_value, prefix.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_AddPageLabel: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseJPFonts(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseJPFonts(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseJPFonts: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseJPFonts called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseJPFonts(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseJPFonts: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseKRFonts(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseKRFonts(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseKRFonts: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseKRFonts called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseKRFonts(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseKRFonts: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseCNSFonts(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseCNSFonts(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseCNSFonts: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseCNSFonts called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseCNSFonts(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseCNSFonts: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseCNTFonts(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseCNTFonts(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseCNTFonts: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseCNTFonts called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseCNTFonts(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseCNTFonts: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- outline ------------------------------------------------------------*/

/* HPDF_Outline HPDF_CreateOutline(HPDF_Doc pdf, HPDF_Outline parent, const char *title, HPDF_Encoder encoder); */
static awk_value_t *
do_HPDF_CreateOutline(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Outline outline = NULL;
	HPDF_Outline parent = NULL;
	HPDF_Encoder encoder;
	awk_value_t parent_arg;
	awk_value_t title;
	awk_value_t encoder_in;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_CreateOutline: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_CreateOutline called with unknown pdf handle"));
		RETURN_NOK;
	}

	/* NULL is 0 string */
	if (get_argument(1, AWK_STRING, &parent_arg)) {
		if (!(parent = find_handle(pdfoutlines, 1))) {
			parent = NULL;
		}
	}

	if (!get_argument(2, AWK_STRING, &title)) {
		set_ERRNO(_("HPDF_CreateOutline: missing required title argument"));
		RETURN_NOK;
	}

	if (get_argument(3, AWK_NUMBER, &encoder_in)) {
		if (encoder_in.num_value != 0) {
			if (!(encoder = find_handle(pdfencoders, 3))) {
				outline = HPDF_CreateOutline(pdf, parent, 
					(const char *)title.str_value.str, NULL);
			}
			else {
				outline = HPDF_CreateOutline(pdf, parent, 
					(const char *)title.str_value.str, encoder);
			}
		}
		else {
			outline = HPDF_CreateOutline(pdf, parent, 
				(const char *)title.str_value.str, NULL);
		}
	}

	if(outline) {
		char hdl[HANDLE_SIZE];
		size_t hlen = outline_handle(outline, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_CreateOutline failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_Outline_SetOpened(HPDF_Outline outline, HPD F_BOOL opened); */
static awk_value_t *
do_HPDF_Outline_SetOpened(int nargs, awk_value_t *result)
{
	HPDF_Outline outline;
	awk_value_t opened;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Outline_SetOpened: called with incorrect number of arguments"));

	if (!(outline = find_handle(pdfoutlines, 0))) {
		set_ERRNO(_("HPDF_Outline_SetOpened called with unknown outline handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &opened)) {
		set_ERRNO(_("HPDF_AddPageLabel: missing required first_page argument"));
		RETURN_NOK;
	}

	if (HPDF_Outline_SetOpened(outline, opened.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Outline_SetOpened: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Outline_SetDestination(HPDF_Outline outline, HPDF_Destination dst); */
static awk_value_t *
do_HPDF_Outline_SetDestination(int nargs, awk_value_t *result)
{
	HPDF_Outline outline;
	HPDF_Destination dst;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Outline_SetDestination: called with incorrect number of arguments"));

	if (!(outline = find_handle(pdfoutlines, 0))) {
		set_ERRNO(_("HPDF_Outline_SetDestination called with unknown outline handle"));
		RETURN_NOK;
	}

	if (!(dst = find_handle(pdfdestinations, 1))) {
		set_ERRNO(_("HPDF_Outline_SetDestination called with unknown destination handle"));
		RETURN_NOK;
	}

	if (HPDF_Outline_SetDestination(outline, dst) != HPDF_OK) {
		set_ERRNO(_("HPDF_Outline_SetDestination: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- destination --------------------------------------------------------*/

/* HPDF_Destination HPDF_Page_CreateDestination(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_CreateDestination(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Destination dst;
	awk_value_t page_check;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_CreateDestination: called with incorrect number of arguments"));

	/* NULL is 0  check */
	if (get_argument(0, AWK_STRING, &page_check)) {
		if (page_check.str_value.len != 1) {
			if ((page = find_handle(pdfpages, 0)))
				dst = HPDF_Page_CreateDestination(page);
			else
				dst = HPDF_Page_CreateDestination(NULL);
		}
		else {
			dst = HPDF_Page_CreateDestination(NULL);
		}
	}
	else {
		dst = HPDF_Page_CreateDestination(NULL);
	}

	if(dst) {
		char hdl[HANDLE_SIZE];
		size_t hlen = destination_handle(dst, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_Page_CreateDestination failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_Destination_SetXYZ(HPDF_Destination dst, HPDF_REAL left, HPDF_REAL top, HPDF_REAL zoom); */
static awk_value_t *
do_HPDF_Destination_SetXYZ(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t left;
	awk_value_t top;
	awk_value_t zoom;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Destination_SetXYZ: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetXYZ called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &left)) {
		set_ERRNO(_("HPDF_Destination_SetXYZ: missing required left argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &top)) {
		set_ERRNO(_("HPDF_Destination_SetXYZ: missing required top argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &zoom)) {
		set_ERRNO(_("HPDF_Destination_SetXYZ: missing required zoom argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetXYZ(dst, left.num_value, top.num_value, zoom.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetXYZ: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFit(HPDF_Destination dst); */
static awk_value_t *
do_HPDF_Destination_SetFit(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Destination_SetFit: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFit called with unknown destination handle"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFit(dst) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFit: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitH(HPDF_Destination dst, HPDF_REAL top); */
static awk_value_t *
do_HPDF_Destination_SetFitH(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t top;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Destination_SetFitH: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitH called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &top)) {
		set_ERRNO(_("HPDF_Destination_SetFitH: missing required top argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitH(dst, top.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitH: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitV(HPDF_Destination dst, HPDF_REAL left); */
static awk_value_t *
do_HPDF_Destination_SetFitV(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t left;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Destination_SetFitV: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitV called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &left)) {
		set_ERRNO(_("HPDF_Destination_SetFitV: missing required left argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitV(dst, left.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitV: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitR(HPDF_Destination dst, HPDF_REAL left, HPDF_REAL bottom, HPDF_REAL right, HPDF_REAL top); */
static awk_value_t *
do_HPDF_Destination_SetFitR(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t left;
	awk_value_t bottom;
	awk_value_t right;
	awk_value_t top;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Destination_SetFitR: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitR called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &left)) {
		set_ERRNO(_("HPDF_Destination_SetFitR: missing required left argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &bottom)) {
		set_ERRNO(_("HPDF_Destination_SetFitR: missing required bottom argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &right)) {
		set_ERRNO(_("HPDF_Destination_SetFitR: missing required right argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &top)) {
		set_ERRNO(_("HPDF_Destination_SetFitR: missing required top argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitR(dst, left.num_value, bottom.num_value, right.num_value, top.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitR: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitB(HPDF_Destination dst); */
static awk_value_t *
do_HPDF_Destination_SetFitB(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Destination_SetFitB: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitB called with unknown destination handle"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitB(dst) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitB: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitBH(HPDF_Destination dst, HPDF_REAL top); */
static awk_value_t *
do_HPDF_Destination_SetFitBH(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t top;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Destination_SetFitBH: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitBH called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &top)) {
		set_ERRNO(_("HPDF_Destination_SetFitBH: missing required top argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitBH(dst, top.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitBH: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Destination_SetFitBV(HPDF_Destination dst, HPDF_REAL left); */
static awk_value_t *
do_HPDF_Destination_SetFitBV(int nargs, awk_value_t *result)
{
	HPDF_Destination dst;
	awk_value_t left;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Destination_SetFitBV: called with incorrect number of arguments"));

	if (!(dst = find_handle(pdfdestinations, 0))) {
		set_ERRNO(_("HPDF_Destination_SetFitBV called with unknown destination handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &left)) {
		set_ERRNO(_("HPDF_Destination_SetFitBV: missing required left argument"));
		RETURN_NOK;
	}

	if (HPDF_Destination_SetFitBV(dst, left.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Destination_SetFitBV: can't set destination."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- encoder ------------------------------------------------------------*/

/* HPDF_Encoder HPDF_GetEncoder(HPDF_Doc pdf, const char *encoding_name); */
static awk_value_t *
do_HPDF_GetEncoder(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Encoder encode;
	awk_value_t encoding_name;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_GetEncoder: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetEncoder called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &encoding_name)) {
		set_ERRNO(_("HPDF_GetEncoder: missing required encoding_name argument"));
		RETURN_NOK;
	}

	encode = HPDF_GetEncoder(pdf, (const char *)encoding_name.str_value.str);

	if(encode) {
		char hdl[HANDLE_SIZE];
		size_t hlen = encoder_handle(encode, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_GetEncoder failed"));
	RET_NULSTR;
}

/* HPDF_Encoder HPDF_GetCurrentEncoder(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_GetCurrentEncoder(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Encoder encode;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetCurrentEncoder: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetCurrentEncoder called with unknown pdf handle"));
		RET_NULSTR;
	}

	encode = HPDF_GetCurrentEncoder(pdf);

	if(encode) {
		char hdl[HANDLE_SIZE];
		size_t hlen = encoder_handle(encode, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_GetCurrentEncoder failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_SetCurrentEncoder(HPDF_Doc pdf, const char *encoding_name); */
static awk_value_t *
do_HPDF_SetCurrentEncoder(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t encoding_name;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetCurrentEncoder: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetCurrentEncoder called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &encoding_name)) {
		set_ERRNO(_("HPDF_SetCurrentEncoder: missing required encoding_name argument"));
		RETURN_NOK;
	}

	if (HPDF_SetCurrentEncoder(pdf, (const char *)encoding_name.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetCurrentEncoder: can't set encoding_name."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_EncoderType HPDF_Encoder_GetType(HPDF_Encoder encoder); */
static awk_value_t *
do_HPDF_Encoder_GetType(int nargs, awk_value_t *result)
{
	HPDF_Encoder encoder;
	HPDF_EncoderType encodertype;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Encoder_GetType: called with incorrect number of arguments"));

	if (!(encoder = find_handle(pdfencoders, 0))) {
		set_ERRNO(_("HPDF_Encoder_GetType called with unknown encoder handle"));
		return make_number(PDF_NOK, result);
	}

	encodertype = HPDF_Encoder_GetType(encoder);

	if(encodertype) {
		return make_number(encodertype, result);
	}

	set_ERRNO(_("HPDF_Encoder_GetType failed"));
	return make_number(PDF_NOK, result);

}
/* HPDF_ByteType HPDF_Encoder_GetByteType(HPDF_Encoder encoder, const char *text, HPDF_UINT index); */
static awk_value_t *
do_HPDF_Encoder_GetByteType(int nargs, awk_value_t *result)
{
	HPDF_Encoder encoder;
	HPDF_ByteType bytetype;
	awk_value_t text;
	awk_value_t index;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Encoder_GetByteType: called with incorrect number of arguments"));

	if (!(encoder = find_handle(pdfencoders, 0))) {
		set_ERRNO(_("HPDF_Encoder_GetByteType called with unknown encoder handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Encoder_GetByteType: missing required text argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &index)) {
		set_ERRNO(_("HPDF_Encoder_GetByteType: missing required index argument"));
		RETURN_NOK;
	}

	bytetype = HPDF_Encoder_GetByteType(encoder, (const char *)text.str_value.str, index.num_value);

	if(bytetype) {
		return make_number(bytetype, result);
	}

	set_ERRNO(_("HPDF_Encoder_GetByteType failed"));
	return make_number(PDF_NOK, result);

}

/* HPDF_UNICODE HPDF_Encoder_GetUnicode(HPDF_Encoder encoder, HPDF_UINT16 code); */
static awk_value_t *
do_HPDF_Encoder_GetUnicode(int nargs, awk_value_t *result)
{
	HPDF_Encoder encode;
	HPDF_UNICODE unicode;
	awk_value_t code;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Encoder_GetUnicode: called with incorrect number of arguments"));

	if (!(encode = find_handle(pdfencoders, 0))) {
		set_ERRNO(_("HPDF_Encoder_GetUnicode called with unknown encoder handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &code)) {
		set_ERRNO(_("HPDF_Encoder_GetUnicode: missing required index argument"));
		RETURN_NOK;
	}

	unicode = HPDF_Encoder_GetUnicode(encode, (HPDF_UINT16)code.num_value);

	if(unicode) {
		return make_number(unicode, result);
	}

	set_ERRNO(_("HPDF_Encoder_GetUnicode failed"));
	return make_number(PDF_NOK, result);

}
/* HPDF_WritingMode HPDF_Encoder_GetWritingMode(HPDF_Encoder encoder); */
static awk_value_t *
do_HPDF_Encoder_GetWritingMode(int nargs, awk_value_t *result)
{
	HPDF_Encoder encode;
	HPDF_WritingMode writemode;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Encoder_GetWritingMode: called with incorrect number of arguments"));

	if (!(encode = find_handle(pdfencoders, 0))) {
		set_ERRNO(_("HPDF_Encoder_GetWritingMode called with unknown encoder handle"));
		RETURN_NOK;
	}

	writemode = HPDF_Encoder_GetWritingMode(encode);

	if(writemode) {
		return make_number(writemode, result);
	}

	set_ERRNO(_("HPDF_Encoder_GetWritingMode failed"));
	RETURN_NOK;

}

/* HPDF_STATUS HPDF_UseJPEncodings(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseJPEncodings(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseJPEncodings: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseJPEncodings called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseJPEncodings(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseJPEncodings: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseKREncodings(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseKREncodings(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseKREncodings: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseKREncodings called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseKREncodings(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseKREncodings: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseCNSEncodings(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseCNSEncodings(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseCNSEncodings: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseCNSEncodings called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseCNSEncodings(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseCNSEncodings: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_UseCNTEncodings(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_UseCNTEncodings(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_UseCNTEncodings: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_UseCNTEncodings called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (HPDF_UseCNTEncodings(pdf) != HPDF_OK) {
		set_ERRNO(_("HPDF_UseCNTEncodings: can't set encoding."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- annotation ---------------------------------------------------------*/

#if (HPDF_MAJOR_VERSION == 2) && (HPDF_MINOR_VERSION < 4)

/* HPDF_Annotation HPDF_Page_Create3DAnnot(HPDF_Page page, HPDF_Rect rect, HPDF_U3D u3d); */
static awk_value_t *
do_HPDF_Page_Create3DAnnot(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Annotation annotation;
	awk_value_t u3d;

	awk_value_t rect_in;
	HPDF_Rect rect;
	awk_flat_array_t *flat_array;
	size_t count = 0;
	
	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_Create3DAnnot: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_ARRAY, &rect_in)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect argument"));
		RETURN_NOK;
	}

	if (!get_element_count(rect_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(rect_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if ((int)flat_array->count != 4) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[left, bottom, right, top]"));
		RETURN_NOK;
	}
	
	rect.left = flat_array->elements[0].value.num_value;
	rect.bottom = flat_array->elements[1].value.num_value;
	rect.right = flat_array->elements[2].value.num_value;
	rect.top = flat_array->elements[3].value.num_value;

	if (!get_argument(2, AWK_NUMBER, &u3d)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required text argument"));
		RETURN_NOK;
	}

	annotation = HPDF_Page_Create3DAnnot(page, (HPDF_Rect)rect, (HPDF_U3D)&u3d.num_value);

	if(annotation) {
		char hdl[HANDLE_SIZE];
		size_t hlen = annotation_handle(annotation, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_Create3DAnnot failed"));
	RET_NULSTR;
}

#else  /* Ver 2.4.0 */

/* Ugaa..., Ver 2.4.0 interface change 2013.12.05
 HPDF_Annotation HPDF_Page_Create3DAnnot(HPDF_Page page, HPDF_Rect rect, HPDF_BOOL tb, HPDF_BOOL np, HPDF_U3D u3d, HPDF_Image ap); */
static awk_value_t *
do_HPDF_Page_Create3DAnnot(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Annotation annotation;
	HPDF_Image image;
	awk_value_t tb;
	awk_value_t np;
	awk_value_t u3d;

	awk_value_t rect_in;
	HPDF_Rect rect;
	awk_flat_array_t *flat_array;
	size_t count = 0;
	
	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_Create3DAnnot: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_ARRAY, &rect_in)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect argument"));
		RETURN_NOK;
	}

	if (!get_element_count(rect_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(rect_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if ((int)flat_array->count != 4) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required rect[left, bottom, right, top]"));
		RETURN_NOK;
	}
	
	rect.left = flat_array->elements[0].value.num_value;
	rect.bottom = flat_array->elements[1].value.num_value;
	rect.right = flat_array->elements[2].value.num_value;
	rect.top = flat_array->elements[3].value.num_value;

	if (!get_argument(2, AWK_NUMBER, &tb)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required tb argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &np)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required np argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &u3d)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required u3d argument"));
		RETURN_NOK;
	}

	if (!(image = find_handle(pdfimages, 5))) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot called with unknown image handle"));
		RETURN_NOK;
	}

	annotation = HPDF_Page_Create3DAnnot(page, (HPDF_Rect)rect, (HPDF_BOOL)&tb.num_value, (HPDF_BOOL)&np.num_value, (HPDF_U3D)&u3d.num_value, image);

	if(annotation) {
		char hdl[HANDLE_SIZE];
		size_t hlen = annotation_handle(annotation, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_Create3DAnnot failed"));
	RET_NULSTR;
}

#endif /* end Ver 2.4.0 */

/* HPDF_Annotation HPDF_Page_CreateTextAnnot(HPDF_Page page, HPDF_Rect rect, const char *text, HPDF_Encoder encoder); */
static awk_value_t *
do_HPDF_Page_CreateTextAnnot(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Encoder encoder;
	HPDF_Annotation annotation;
	awk_value_t text;
	awk_value_t encoder_in;

	awk_value_t rect_in;
	HPDF_Rect rect;
	awk_flat_array_t *flat_array;
	size_t count = 0;
	
	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_CreateTextAnnot: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_ARRAY, &rect_in)) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot: missing required rect argument"));
		RETURN_NOK;
	}

	if (!get_element_count(rect_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(rect_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot: clear rect[n] error"));
		RETURN_NOK;
	}

	if ((int)flat_array->count != 4) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot: missing required rect[left, bottom, right, top]"));
		RETURN_NOK;
	}

	rect.left = flat_array->elements[0].value.num_value;
	rect.bottom = flat_array->elements[1].value.num_value;
	rect.right = flat_array->elements[2].value.num_value;
	rect.top = flat_array->elements[3].value.num_value;

	if (!get_argument(2, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot: missing required text argument"));
		RETURN_NOK;
	}


	if (!get_argument(3, AWK_NUMBER, &encoder_in)) {
		set_ERRNO(_("HPDF_Page_Create3DAnnot: missing required text argument"));
		RETURN_NOK;
	}

	if (encoder_in.num_value == 0) {
		annotation = HPDF_Page_CreateTextAnnot(page, 
			(HPDF_Rect)rect, (const char *)text.str_value.str, NULL);
	}
	else {	
		if (!(encoder = find_handle(pdfencoders, 3))) {
			set_ERRNO(_("HPDF_Page_CreateTextAnnot called with unknown encoder handle"));
			RETURN_NOK;
		}
		annotation = HPDF_Page_CreateTextAnnot(page, 
			(HPDF_Rect)rect, (const char *)text.str_value.str, encoder);
	}

	if(annotation) {
		char hdl[HANDLE_SIZE];
		size_t hlen = annotation_handle(annotation, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_CreateTextAnnot failed"));
	RET_NULSTR;
}

/* HPDF_Annotation HPDF_Page_CreateLinkAnnot(HPDF_Page page, HPDF_Rect rect, HPDF_Destination dst); */
static awk_value_t *
do_HPDF_Page_CreateLinkAnnot(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Destination dst;
	HPDF_Annotation annotation;

	awk_value_t rect_in;
	HPDF_Rect rect;
	awk_flat_array_t *flat_array;
	size_t count = 0;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_CreateLinkAnnot: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CreateLinkAnnot called with unknown page handle"));
		RETURN_NOK;
	}

	/* rect struct suport */
	if (!get_argument(1, AWK_ARRAY, &rect_in)) {
		set_ERRNO(_("HPDF_Page_CreateLinkAnnot: missing required rect argument"));
		RETURN_NOK;
	}

	if (!get_element_count(rect_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_CreateLinkAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(rect_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_CreateLinkAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if ((int)flat_array->count != 4) {
		set_ERRNO(_("HPDF_Page_CreateLinkAnnot: missing required rect[left, bottom, right, top]"));
		RETURN_NOK;
	}
	
	rect.left = flat_array->elements[0].value.num_value;
	rect.bottom = flat_array->elements[1].value.num_value;
	rect.right = flat_array->elements[2].value.num_value;
	rect.top = flat_array->elements[3].value.num_value;

	if (!(dst = find_handle(pdfdestinations, 2))) {
		set_ERRNO(_("HPDF_Page_CreateTextAnnot called with unknown destination handle"));
		RETURN_NOK;
	}

	annotation = HPDF_Page_CreateLinkAnnot(page, (HPDF_Rect)rect, dst);

	if(annotation) {
		char hdl[HANDLE_SIZE];
		size_t hlen = annotation_handle(annotation, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_CreateLinkAnnot failed"));
	RET_NULSTR;
}

/* HPDF_Annotation HPDF_Page_CreateURILinkAnnot(HPDF_Page page, HPDF_Rect rect, const char *uri); */
static awk_value_t *
do_HPDF_Page_CreateURILinkAnnot(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Annotation annotation;
	awk_value_t uri;

	awk_value_t rect_in;
	HPDF_Rect rect;
	awk_flat_array_t *flat_array;
	size_t count = 0;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_CreateURILinkAnnot: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_ARRAY, &rect_in)) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot: missing required rect argument"));
		RETURN_NOK;
	}

	if (!get_element_count(rect_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(rect_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot: missing required rect[n] argument"));
		RETURN_NOK;
	}

	if ((int)flat_array->count != 4) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot: missing required rect[left, bottom, right, top]"));
		RETURN_NOK;
	}
	
	rect.left = flat_array->elements[0].value.num_value;
	rect.bottom = flat_array->elements[1].value.num_value;
	rect.right = flat_array->elements[2].value.num_value;
	rect.top = flat_array->elements[3].value.num_value;


	if (!get_argument(2, AWK_STRING, &uri)) {
		set_ERRNO(_("HPDF_Page_CreateURILinkAnnot: missing required uri argument"));
		RETURN_NOK;
	}

	annotation = HPDF_Page_CreateURILinkAnnot(page, (HPDF_Rect)rect, (const char *)uri.str_value.str);

	if(annotation) {
		char hdl[HANDLE_SIZE];
		size_t hlen = annotation_handle(annotation, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_CreateURILinkAnnot failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_LinkAnnot_SetHighlightMode(HPDF_Annotation annot, HPDF_AnnotHighlightMode mode); */
static awk_value_t *
do_HPDF_LinkAnnot_SetHighlightMode(int nargs, awk_value_t *result)
{
	HPDF_Annotation annot;
	awk_value_t mode;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_LinkAnnot_SetHighlightMode: called with incorrect number of arguments"));

	if (!(annot = find_handle(pdfannotations, 0))) {
		set_ERRNO(_("HPDF_LinkAnnot_SetHighlightMode called with unknown annotation handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_LinkAnnot_SetHighlightMode: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_LinkAnnot_SetHighlightMode(annot, mode.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_LinkAnnot_SetHighlightMode: can't get annotation handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_LinkAnnot_SetBorderStyle(HPDF_Annotation  annot, HPDF_REAL width, HPDF_UINT16  dash_on, HPDF_UINT16  dash_off); */
static awk_value_t *
do_HPDF_LinkAnnot_SetBorderStyle(int nargs, awk_value_t *result)
{
	HPDF_Annotation annot;
	awk_value_t width;
	awk_value_t dash_on_in;
	HPDF_UINT16  dash_on;
	awk_value_t dash_off_in;
	HPDF_UINT16  dash_off;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_LinkAnnot_SetBorderStyle: called with incorrect number of arguments"));

	if (!(annot = find_handle(pdfannotations, 0))) {
		set_ERRNO(_("HPDF_LinkAnnot_SetBorderStyle called with unknown annotation handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_LinkAnnot_SetBorderStyle: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &dash_on_in)) {
		set_ERRNO(_("HPDF_LinkAnnot_SetBorderStyle: missing required dash_on argument"));
		RETURN_NOK;
	}

	dash_on = dash_on_in.num_value;

	if (!get_argument(3, AWK_NUMBER, &dash_off_in)) {
		set_ERRNO(_("HPDF_LinkAnnot_SetBorderStyle: missing required dash_off argument"));
		RETURN_NOK;
	}

	dash_off = dash_off_in.num_value;

	if (HPDF_LinkAnnot_SetBorderStyle(annot, width.num_value, dash_on, dash_off) != HPDF_OK) {
		set_ERRNO(_("HPDF_LinkAnnot_SetBorderStyle: can't get annotation handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_TextAnnot_SetIcon(HPDF_Annotation annot, HPDF_AnnotIcon icon); */
static awk_value_t *
do_HPDF_TextAnnot_SetIcon(int nargs, awk_value_t *result)
{
	HPDF_Annotation annot;
	HPDF_AnnotIcon icon;
	awk_value_t icon_in;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_TextAnnot_SetIcon: called with incorrect number of arguments"));

	if (!(annot = find_handle(pdfannotations, 0))) {
		set_ERRNO(_("HPDF_TextAnnot_SetIcon called with unknown annotation handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &icon_in)) {
		set_ERRNO(_("HPDF_TextAnnot_SetIcon: missing required icon argument"));
		RETURN_NOK;
	}

	icon = (HPDF_AnnotIcon)icon_in.num_value;

	if (HPDF_TextAnnot_SetIcon(annot, (HPDF_AnnotIcon)icon) != HPDF_OK) {
		set_ERRNO(_("HPDF_TextAnnot_SetIcon: can't get annotation handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_TextAnnot_SetOpened(HPDF_Annotation annot, HPDF_BOOL opened); */
static awk_value_t *
do_HPDF_TextAnnot_SetOpened(int nargs, awk_value_t *result)
{
	HPDF_Annotation annot;
	HPDF_BOOL opened;
	awk_value_t opened_in;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_TextAnnot_SetOpened: called with incorrect number of arguments"));

	if (!(annot = find_handle(pdfannotations, 0))) {
		set_ERRNO(_("HPDF_TextAnnot_SetOpened called with unknown annotation handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &opened_in)) {
		set_ERRNO(_("HPDF_TextAnnot_SetOpened: missing required opened argument"));
		RETURN_NOK;
	}

	if (opened_in.num_value == 1)
		opened = HPDF_TRUE;
	else
		opened = HPDF_FALSE;

	if (HPDF_TextAnnot_SetOpened(annot, opened) != HPDF_OK) {
		set_ERRNO(_("HPDF_TextAnnot_SetOpened: can't get annotation handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- image data ---------------------------------------------------------*/

/* HPDF_Image HPDF_LoadPngImageFromMem(HPDF_Doc pdf, const HPDF_BYTE *buffer, HPDF_UINT size); */
static awk_value_t *
do_HPDF_LoadPngImageFromMem(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t buffer;
	awk_value_t size;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_LoadPngImageFromMem: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadPngImageFromMem called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &buffer)) {
		set_ERRNO(_("HPDF_LoadPngImageFromMem: missing required buffer argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &size)) {
		set_ERRNO(_("HPDF_LoadPngImageFromMem: missing required size argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadPngImageFromMem(pdf, (const HPDF_BYTE *)buffer.str_value.str, size.num_value);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_LoadPngImageFromMem failed"));
	RET_NULSTR;
}

/* HPDF_Image HPDF_LoadPngImageFromFile(HPDF_Doc pdf, const char *filename); */
static awk_value_t *
do_HPDF_LoadPngImageFromFile(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t filename;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_LoadPngImageFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadPngImageFromFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &filename)) {
		set_ERRNO(_("HPDF_LoadPngImageFromFile: missing required filename argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadPngImageFromFile(pdf, (const char *)filename.str_value.str);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_LoadPngImageFromFile failed"));
	RET_NULSTR;
}

/* HPDF_Image HPDF_LoadJpegImageFromFile(HPDF_Doc pdf, const char *filename); */
static awk_value_t *
do_HPDF_LoadJpegImageFromFile(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t filename;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_LoadJpegImageFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadJpegImageFromFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &filename)) {
		set_ERRNO(_("HPDF_LoadJpegImageFromFile: missing required filename argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadJpegImageFromFile(pdf, filename.str_value.str);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_LoadJpegImageFromFile failed"));
	RET_NULSTR;
}

/* HPDF_Image HPDF_LoadU3DFromFile(HPDF_Doc pdf, const char *filename); */
static awk_value_t *
do_HPDF_LoadU3DFromFile(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t filename;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_LoadU3DFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadU3DFromFile called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &filename)) {
		set_ERRNO(_("HPDF_LoadU3DFromFile: missing required filename argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadU3DFromFile(pdf, (const char *)filename.str_value.str);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_LoadU3DFromFile failed"));
	RET_NULSTR;
}

/* HPDF_Image HPDF_LoadRawImageFromMem(HPDF_Doc pdf, const HPDF_BYTE *buf, HPDF_UINT width, HPDF_UINT height, HPDF_ColorSpace color_space, HPDF_UINT bits_per_component); */
static awk_value_t *
do_HPDF_LoadRawImageFromMem(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t buf;
	awk_value_t width;
	awk_value_t height;
	awk_value_t color_space;
	awk_value_t bits_per_component;

	if (do_lint && nargs != 6)
		lintwarn(ext_id, _("HPDF_LoadRawImageFromMem: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &buf)) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem: missing required buf argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &height)) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem: missing required height argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &color_space)) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem: missing required color_space argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &bits_per_component)) {
		set_ERRNO(_("HPDF_LoadRawImageFromMem: missing required bits_per_component argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadRawImageFromMem(pdf, (const HPDF_BYTE *)buf.str_value.str, 
			width.num_value, height.num_value, color_space.num_value, 
			bits_per_component.num_value);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}

	set_ERRNO(_("HPDF_LoadRawImageFromMem failed"));
	RET_NULSTR;
}

/* HPDF_Image HPDF_LoadRawImageFromFile(HPDF_Doc pdf,
  const char *filename, HPDF_UINT width, HPDF_UINT height, HPDF_ColorSpace color_space); */
static awk_value_t *
do_HPDF_LoadRawImageFromFile(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_Image image;
	awk_value_t filename;
	awk_value_t width;
	awk_value_t height;
	awk_value_t color_space;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_LoadRawImageFromFile: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_LoadRawImageFromFile called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &filename)) {
		set_ERRNO(_("HPDF_LoadRawImageFromFile: missing required filename argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_LoadRawImageFromFile: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &height)) {
		set_ERRNO(_("HPDF_LoadRawImageFromFile: missing required height argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &color_space)) {
		set_ERRNO(_("HPDF_LoadRawImageFromFile: missing required color_space argument"));
		RETURN_NOK;
	}

	image = HPDF_LoadRawImageFromFile(pdf, (const char *)filename.str_value.str, 
					width.num_value, height.num_value, color_space.num_value);

	if(image) {
		char hdl[HANDLE_SIZE];
		size_t hlen = image_handle(image, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_LoadRawImageFromFile failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_Image_AddSMask(HPDF_Image image, HPDF_Image smask); */
static awk_value_t *
do_HPDF_Image_AddSMask(int nargs, awk_value_t *result)
{
	HPDF_Image image;
	HPDF_Image smask;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Image_AddSMask: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_AddSMask called with unknown image handle"));
		RETURN_NOK;
	}

	if (!(smask = find_handle(pdfimages, 1))) {
		set_ERRNO(_("HPDF_Image_AddSMask called with unknown smask handle"));
		RETURN_NOK;
	}

	if (HPDF_Image_AddSMask(image, smask) != HPDF_OK) {
		set_ERRNO(_("HPDF_Image_AddSMask: can't get image handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_Point HPDF_Image_GetSize(HPDF_Image image); */
static awk_value_t *
do_HPDF_Image_GetSize(int nargs, awk_value_t *result)
{
	HPDF_Point point;
	HPDF_Image image;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Image_GetSize: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetSize called with unknown image handle"));
		RETURN_NOK;
	}

	point = HPDF_Image_GetSize(image);
	
	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3.3f,%-3.3f", point.x, point.y);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
	}
	
	// return make_string_malloc((char*)&point, strlen((char*)&point), result);

}

/* HPDF_STATUS HPDF_Image_GetSize2(HPDF_Image image, HPDF_Point *size); */
static awk_value_t *
do_HPDF_Image_GetSize2(int nargs, awk_value_t *result)
{
	HPDF_Image image;
	awk_value_t size;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Image_GetSize2: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetSize2 called with unknown image handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &size)) {
		set_ERRNO(_("HPDF_Image_GetSize2: missing required filename argument"));
		RETURN_NOK;
	}
	
	if (HPDF_Image_GetSize2(image, (HPDF_Point *)&size.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Image_GetSize2: can't get image handle."));
		RETURN_NOK;
	}

	RETURN_OK;

}

/* HPDF_UINT HPDF_Image_GetWidth(HPDF_Image image); */
static awk_value_t *
do_HPDF_Image_GetWidth(int nargs, awk_value_t *result)
{
	HPDF_UINT width;
	HPDF_Image image;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Image_GetWidth: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetWidth called with unknown image handle"));
		RETURN_NOK;
	}

	width = HPDF_Image_GetWidth(image);
	
	if(width) {
		return make_number((double)width, result);
	}

	set_ERRNO(_("HPDF_Image_GetWidth failed"));
	return make_number(PDF_NOK, result);

}

/* HPDF_UINT HPDF_Image_GetHeight(HPDF_Image image); */
static awk_value_t *
do_HPDF_Image_GetHeight(int nargs, awk_value_t *result)
{
	HPDF_UINT height;
	HPDF_Image image;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Image_GetHeight: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetHeight called with unknown image handle"));
		RETURN_NOK;
	}

	height = HPDF_Image_GetHeight(image);
	
	if(height) {
		return make_number((double)height, result);
	}

	set_ERRNO(_("HPDF_Image_GetHeight failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_UINT HPDF_Image_GetBitsPerComponent(HPDF_Image image); */
static awk_value_t *
do_HPDF_Image_GetBitsPerComponent(int nargs, awk_value_t *result)
{
	int imbit;
	HPDF_Image image;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Image_GetBitsPerComponent: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetBitsPerComponent called with unknown image handle"));
		RETURN_NOK;
	}

	imbit = HPDF_Image_GetBitsPerComponent(image);
	
	if(imbit) {
		return make_number(imbit, result);
	}

	set_ERRNO(_("HPDF_Image_GetBitsPerComponent failed"));
	return make_number(PDF_NOK, result);
}

/* const char* HPDF_Image_GetColorSpace(HPDF_Image image); */
static awk_value_t *
do_HPDF_Image_GetColorSpace(int nargs, awk_value_t *result)
{

	const char *color_space;	
	HPDF_Image image;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Image_GetColorSpace: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_GetColorSpace called with unknown image handle"));
		RETURN_NOK;
	}

	color_space = HPDF_Image_GetColorSpace(image);

	if(color_space) {
		return make_string_malloc(color_space, strlen(color_space), result);
	}

	set_ERRNO(_("HPDF_Image_GetColorSpace failed"));
	RET_NULSTR;

}

/* HPDF_STATUS HPDF_Image_SetColorMask(HPDF_Image image, HPDF_UINT rmin, HPDF_UINT rmax, HPDF_UINT gmin, HPDF_UINT gmax,
 HPDF_UINT bmin, HPDF_UINT bmax); */
static awk_value_t *
do_HPDF_Image_SetColorMask(int nargs, awk_value_t *result)
{
	HPDF_Image image;
	awk_value_t rmin;
	awk_value_t rmax;
	awk_value_t gmin;
	awk_value_t gmax;
	awk_value_t bmin;
	awk_value_t bmax;

	if (do_lint && nargs != 7)
		lintwarn(ext_id, _("HPDF_Image_SetColorMask: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_SetColorMask called with unknown image handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &rmin)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required rmin argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &rmax)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required rmax argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &gmin)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required gmin argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &gmax)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required gmax argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &bmin)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required bmin argument"));
		RETURN_NOK;
	}

	if (!get_argument(6, AWK_NUMBER, &bmax)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required bmax argument"));
		RETURN_NOK;
	}

	if (HPDF_Image_SetColorMask(image, (HPDF_UINT)rmin.num_value, (HPDF_UINT)rmax.num_value,
			(HPDF_UINT)gmin.num_value, (HPDF_UINT)gmax.num_value,
			(HPDF_UINT)bmin.num_value, (HPDF_UINT)bmax.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Image_SetColorMask: can't get image handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Image_SetMaskImage(HPDF_Image image, HPDF_Image mask_image); */
static awk_value_t *
do_HPDF_Image_SetMaskImage(int nargs, awk_value_t *result)
{
	HPDF_Image image;
	HPDF_Image mask_image;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Image_SetMaskImage: called with incorrect number of arguments"));

	if (!(image = find_handle(pdfimages, 0))) {
		set_ERRNO(_("HPDF_Image_SetMaskImage called with unknown image handle"));
		RETURN_NOK;
	}

	if (!(mask_image = find_handle(pdfimages, 1))) {
		set_ERRNO(_("HPDF_Image_SetMaskImage called with unknown smask handle"));
		RETURN_NOK;
	}

	if (HPDF_Image_SetMaskImage(image, mask_image) != HPDF_OK) {
		set_ERRNO(_("HPDF_Image_SetMaskImage: can't get image handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- info dictionary ----------------------------------------------------*/

/* HPDF_STATUS HPDF_SetInfoAttr(HPDF_Doc pdf, HPDF_InfoType type, const char *value); */
static awk_value_t *
do_HPDF_SetInfoAttr(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t type;
	awk_value_t value_arg;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_SetInfoAttr: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetInfoAttr called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &type)) {
		set_ERRNO(_("HPDF_SetInfoAttr: missing required type argument"));
		RETURN_NOK;
	}
	if (!get_argument(2, AWK_STRING, &value_arg)) {
		set_ERRNO(_("HPDF_SetInfoAttr: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_SetInfoAttr(pdf, type.num_value, value_arg.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetInfoAttr: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* const char* HPDF_GetInfoAttr(HPDF_Doc pdf, HPDF_InfoType type); */
static awk_value_t *
	do_HPDF_GetInfoAttr(int nargs, awk_value_t *result)
{
	const char *info_attr;	
	HPDF_Doc pdf;
	awk_value_t type;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_GetInfoAttr: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_GetInfoAttr called with unknown pdf handle"));
		RET_NULSTR;
	}

	if (!get_argument(1, AWK_NUMBER, &type)) {
		set_ERRNO(_("HPDF_Image_SetColorMask: missing required type argument"));
		RET_NULSTR;
	}

	info_attr = HPDF_GetInfoAttr(pdf, (HPDF_InfoType)type.num_value);

	if(info_attr) {
		return make_string_malloc(info_attr, strlen(info_attr), result);
	}

	set_ERRNO(_("HPDF_GetInfoAttr failed"));
	RET_NULSTR;

}

/* HPDF_STATUS HPDF_SetInfoDateAttr(HPDF_Doc pdf, HPDF_InfoType type, HPDF_Date value); */
static awk_value_t *
do_HPDF_SetInfoDateAttr(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t type;
	awk_value_t value_arg;

	// struct tm date_arg;
	char str_work[12];
	HPDF_Date date_cnv;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_SetInfoDateAttr: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetInfoDateAttr called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &type)) {
		set_ERRNO(_("HPDF_SetInfoDateAttr: missing required type argument"));
		RETURN_NOK;
	}

	/* only format YYYYMMDD or YYYYMMDDhhmmss */
	if (!get_argument(2, AWK_STRING, &value_arg)) {
		set_ERRNO(_("HPDF_SetInfoDateAttr: missing required date argument"));
		RETURN_NOK;
	}

	if ((value_arg.str_value.len != 8) && (value_arg.str_value.len != 14)) {
		set_ERRNO(_("HPDF_SetInfoDateAttr: date argument is YYYYMMDD or YYYYMMDDhhmmss."));
		RETURN_NOK;
	}

	// strptime(value_arg.str_value.len, "%Y%m%d%H%M%S", &date_arg);
	
	date_cnv.year = atoi(strncpy(str_work, value_arg.str_value.str, 4));
	date_cnv.month = atoi(strncpy(str_work, value_arg.str_value.str+4, 2));
	date_cnv.day = atoi(strncpy(str_work, value_arg.str_value.str+6, 2));

	if (value_arg.str_value.len == 14) {
		date_cnv.hour = atoi(strncpy(str_work, value_arg.str_value.str+8, 2));
		date_cnv.minutes = atoi(strncpy(str_work, value_arg.str_value.str+10, 2));
		date_cnv.seconds = atoi(strncpy(str_work, value_arg.str_value.str+12, 2));
	}
	else {
		date_cnv.hour = 0;
		date_cnv.minutes = 0;
		date_cnv.seconds = 0;
	}

	date_cnv.ind = (int)"";
	date_cnv.off_hour = 0;
	date_cnv.off_minutes = 0;

	if (HPDF_SetInfoDateAttr(pdf, type.num_value, (HPDF_Date)date_cnv) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetInfoDateAttr: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}
/*----- encryption ---------------------------------------------------------*/

/* HPDF_STATUS HPDF_SetPassword(HPDF_Doc pdf, const char *owner_passwd, const char *user_passwd); */
static awk_value_t *
do_HPDF_SetPassword(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t owner_passwd;
	awk_value_t user_passwd;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_SetPassword: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetPassword called with unknown pdf handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &owner_passwd)) {
		set_ERRNO(_("HPDF_SetPassword: missing required owner_passwd argument"));
		RETURN_NOK;
	}
	if (!get_argument(2, AWK_STRING, &user_passwd)) {
		set_ERRNO(_("HPDF_SetPassword: missing required user_passwd argument"));
		RETURN_NOK;
	}

	if (HPDF_SetPassword(pdf, owner_passwd.str_value.str, user_passwd.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetPassword: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_SetPermission(HPDF_Doc pdf, HPDF_UINT permission); */
static awk_value_t *
do_HPDF_SetPermission(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t permission;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetPermission: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetPermission called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &permission)) {
		set_ERRNO(_("HPDF_SetPermission: missing required permission argument"));
		RETURN_NOK;
	}

	if (HPDF_SetPermission(pdf, permission.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetPermission: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_SetEncryptionMode(HPDF_Doc pdf, HPDF_EncryptMode mode, HPDF_UINT key_len); */
static awk_value_t *
do_HPDF_SetEncryptionMode(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t mode;
	awk_value_t key_len;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_SetEncryptionMode: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetEncryptionMode called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_SetEncryptionMode: missing required mode argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &key_len)) {
		set_ERRNO(_("HPDF_SetEncryptionMode: missing required key_len argument"));
		RETURN_NOK;
	}

	if (HPDF_SetEncryptionMode(pdf, mode.num_value, key_len.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetEncryptionMode: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*----- compression --------------------------------------------------------*/

/* HPDF_STATUS HPDF_SetCompressionMode(HPDF_Doc pdf, HPDF_UINT mode); */
static awk_value_t *
do_HPDF_SetCompressionMode(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	awk_value_t mode;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_SetCompressionMode: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_SetCompressionMode called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_SetCompressionMode: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_SetCompressionMode(pdf, mode.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_SetCompressionMode: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}


/*----- font ---------------------------------------------------------------*/

/* const char* HPDF_Font_GetFontName(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetFontName(int nargs, awk_value_t *result)
{

	const char *font_name;	
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetFontName: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetFontName called with unknown font handle"));
		RETURN_NOK;
	}

	font_name = HPDF_Font_GetFontName(font);

	if(font_name) {
		return make_string_malloc(font_name, strlen(font_name), result);
	}

	set_ERRNO(_("HPDF_Font_GetFontName failed"));
	RET_NULSTR;

}

/* const char* HPDF_Font_GetEncodingName(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetEncodingName(int nargs, awk_value_t *result)
{

	const char *encode_name;	
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetEncodingName: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetEncodingName called with unknown font handle"));
		RETURN_NOK;
	}

	encode_name = HPDF_Font_GetEncodingName(font);

	if(encode_name) {
		return make_string_malloc(encode_name, strlen(encode_name), result);
	}

	set_ERRNO(_("HPDF_Font_GetEncodingName failed"));
	RET_NULSTR;

}

/* HPDF_INT HPDF_Font_GetUnicodeWidth(HPDF_Font font, HPDF_UNICODE code); */
static awk_value_t *
do_HPDF_Font_GetUnicodeWidth(int nargs, awk_value_t *result)
{
	HPDF_INT width;
	HPDF_Font font;
	awk_value_t code;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Font_GetUnicodeWidth: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetUnicodeWidth called with unknown font handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &code)) {
		set_ERRNO(_("HPDF_Font_GetUnicodeWidth: missing required code argument"));
		RETURN_NOK;
	}

	width = HPDF_Font_GetUnicodeWidth(font, code.num_value);
	
	if(width) {
		return make_number(width, result);
	}

	set_ERRNO(_("HPDF_Font_GetUnicodeWidth failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_Box HPDF_Font_GetBBox(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetBBox(int nargs, awk_value_t *result)
{
	HPDF_Box box;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetBBox: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetBBox called with unknown font handle"));
		RETURN_NOK;
	}

	box = HPDF_Font_GetBBox(font);

	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3.3f,%-3.3f,%-3.3f,%-3.3f", 
			box.left, box.bottom, box.right, box.top);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
	}
	
	// return make_string_malloc((char*)&box, strlen((char*)&box), result);

}

/* HPDF_INT HPDF_Font_GetAscent(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetAscent(int nargs, awk_value_t *result)
{
	HPDF_INT ascent;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetAscent: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetAscent called with unknown font handle"));
		RETURN_NOK;
	}

	ascent = HPDF_Font_GetAscent(font);
	
	if(ascent) {
		return make_number(ascent, result);
	}

	set_ERRNO(_("HPDF_Font_GetAscent failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_INT HPDF_Font_GetDescent(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetDescent(int nargs, awk_value_t *result)
{
	HPDF_INT descent;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetDescent: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetDescent called with unknown font handle"));
		RETURN_NOK;
	}

	descent = HPDF_Font_GetDescent(font);
	
	if(descent) {
		return make_number(descent, result);
	}

	set_ERRNO(_("HPDF_Font_GetDescent failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_UINT HPDF_Font_GetXHeight(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetXHeight(int nargs, awk_value_t *result)
{
	HPDF_UINT xheight;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetXHeight: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetXHeight called with unknown font handle"));
		RETURN_NOK;
	}

	xheight = HPDF_Font_GetXHeight(font);
	
	if(xheight) {
		return make_number(xheight, result);
	}

	set_ERRNO(_("HPDF_Font_GetXHeight failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_UINT HPDF_Font_GetCapHeight(HPDF_Font font); */
static awk_value_t *
do_HPDF_Font_GetCapHeight(int nargs, awk_value_t *result)
{
	HPDF_UINT capheight;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Font_GetCapHeight: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_GetCapHeight called with unknown font handle"));
		RETURN_NOK;
	}

	capheight = HPDF_Font_GetCapHeight(font);
	
	if(capheight) {
		return make_number(capheight, result);
	}

	set_ERRNO(_("HPDF_Font_GetCapHeight failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_TextWidth HPDF_Font_TextWidth(HPDF_Font font, const HPDF_BYTE *text, HPDF_UINT len); */
static awk_value_t *
do_HPDF_Font_TextWidth(int nargs, awk_value_t *result)
{
	HPDF_Font font;
	HPDF_TextWidth textwidth;
	awk_value_t text;
	awk_value_t len;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Font_TextWidth: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_TextWidth called with unknown font handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Font_TextWidth: missing required text argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &len)) {
		set_ERRNO(_("HPDF_Font_TextWidth: missing required text argument"));
		RETURN_NOK;
	}

	textwidth = HPDF_Font_TextWidth(font, (const HPDF_BYTE *)&text.str_value.str, len.num_value);
	
	return make_string_malloc((char*)&textwidth, strlen((char*)&textwidth), result);

}

/* HPDF_UINT HPDF_Font_MeasureText(HPDF_Font font, const HPDF_BYTE *text, HPDF_UINT len, HPDF_REAL width,
 HPDF_REAL font_size, HPDF_REAL char_space, HPDF_REAL word_space, HPDF_BOOL wordwrap, HPDF_REAL *real_width); */
static awk_value_t *
do_HPDF_Font_MeasureText(int nargs, awk_value_t *result)
{
	HPDF_Font font;
	HPDF_UINT matext;
	awk_value_t text;
	awk_value_t len;
	awk_value_t width;
	awk_value_t font_size;
	awk_value_t char_space;
	awk_value_t word_space;
	awk_value_t wordwrap;
	awk_value_t real_width;

	if (do_lint && nargs != 9)
		lintwarn(ext_id, _("HPDF_Font_MeasureText: called with incorrect number of arguments"));

	if (!(font = find_handle(pdffonts, 0))) {
		set_ERRNO(_("HPDF_Font_MeasureText called with unknown font handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required text argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &len)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required len argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &font_size)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required font_size argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &char_space)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required char_space argument"));
		RETURN_NOK;
	}

	if (!get_argument(6, AWK_NUMBER, &word_space)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required word_space argument"));
		RETURN_NOK;
	}

	if (!get_argument(7, AWK_NUMBER, &wordwrap)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required wordwrap argument"));
		RETURN_NOK;
	}

	if (!get_argument(8, AWK_NUMBER, &real_width)) {
		set_ERRNO(_("HPDF_Font_MeasureText: missing required wordwrap argument"));
		RETURN_NOK;
	}

	matext = HPDF_Font_MeasureText(font, (const HPDF_BYTE *)&text.str_value.str, len.num_value,
				width.num_value, font_size.num_value,  char_space.num_value, word_space.num_value, 
				wordwrap.num_value, (HPDF_REAL *)&real_width.num_value);	
	
	if(matext) {
		return make_number(matext, result);
	}

	set_ERRNO(_("HPDF_Font_MeasureText failed"));
	return make_number(PDF_NOK, result);
}

/*----- extended graphics state --------------------------------------------*/

/* HPDF_ExtGState HPDF_CreateExtGState(HPDF_Doc pdf); */
static awk_value_t *
do_HPDF_CreateExtGState(int nargs, awk_value_t *result)
{
	HPDF_Doc pdf;
	HPDF_ExtGState extgstate;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_CreateExtGState: called with incorrect number of arguments"));

	if (!(pdf = find_handle(pdfdatas, 0))) {
		set_ERRNO(_("HPDF_CreateExtGState called with unknown pdf handle"));
		RETURN_NOK;
	}

	extgstate = HPDF_CreateExtGState(pdf);

	if(extgstate) {
		char hdl[HANDLE_SIZE];
		size_t hlen = extgstate_handle(extgstate, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_CreateExtGState failed"));
	RET_NULSTR;
}

/* HPDF_STATUS HPDF_ExtGState_SetAlphaStroke(HPDF_ExtGState ext_gstate, HPDF_REAL value); */
static awk_value_t *
do_HPDF_ExtGState_SetAlphaStroke(int nargs, awk_value_t *result)
{
	HPDF_ExtGState ext_gstate;
	awk_value_t value;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_ExtGState_SetAlphaStroke: called with incorrect number of arguments"));

	if (!(ext_gstate = find_handle(pdfextgstates, 0))) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &value)) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaStroke: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_ExtGState_SetAlphaStroke(ext_gstate, value.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaStroke: can't get extgstate handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_ExtGState_SetAlphaFill(HPDF_ExtGState ext_gstate, HPDF_REAL value); */
static awk_value_t *
do_HPDF_ExtGState_SetAlphaFill(int nargs, awk_value_t *result)
{
	HPDF_ExtGState ext_gstate;
	awk_value_t value;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_ExtGState_SetAlphaFill: called with incorrect number of arguments"));

	if (!(ext_gstate = find_handle(pdfextgstates, 0))) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaFill called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &value)) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaFill: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_ExtGState_SetAlphaFill(ext_gstate, value.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_ExtGState_SetAlphaFill: can't get extgstate handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_ExtGState_SetBlendMode(HPDF_ExtGState ext_gstate, HPDF_BlendMode mode); */
static awk_value_t *
do_HPDF_ExtGState_SetBlendMode(int nargs, awk_value_t *result)
{
	HPDF_ExtGState ext_gstate;
	awk_value_t mode;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_ExtGState_SetBlendMode: called with incorrect number of arguments"));

	if (!(ext_gstate = find_handle(pdfextgstates, 0))) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_ExtGState_SetBlendMode(ext_gstate, mode.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode: can't get extgstate handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetExtGState (HPDF_Page page, HPDF_ExtGState ext_gstate); */
static awk_value_t *
do_HPDF_Page_SetExtGState(int nargs, awk_value_t *result)
{

	HPDF_Page page;
	HPDF_ExtGState ext_gstate;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetExtGState: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetExtGState called with unknown page handle"));
		RETURN_NOK;
	}

	if (!(ext_gstate = find_handle(pdfextgstates, 1))) {
		set_ERRNO(_("HPDF_Page_SetExtGState called with unknown gstate handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetExtGState(page, ext_gstate) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetExtGState: can't get extgstate handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--------------------------------------------------------------------------*/

/* HPDF_REAL HPDF_Page_TextWidth(HPDF_Page page, const char *text); */
static awk_value_t *
do_HPDF_Page_TextWidth(int nargs, awk_value_t *result)
{

	HPDF_REAL width;
	awk_value_t text;
	HPDF_Page page;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_TextWidth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_TextWidth called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_TextWidth: missing required text argument"));
		RETURN_NOK;
	}

	width = HPDF_Page_TextWidth(page, text.str_value.str);
	
	if(width) {
		return make_number(width, result);
	}

	set_ERRNO(_("HPDF_Page_TextWidth failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_UINT HPDF_Page_MeasureText(HPDF_Page page, const char *text, HPDF_REAL width, HPDF_BOOL wordwrap, HPDF_REAL *real_width); */
static awk_value_t *
do_HPDF_Page_MeasureText(int nargs, awk_value_t *result)
{
	HPDF_UINT mtext;

	HPDF_Page page;
	awk_value_t text;
	awk_value_t width;
	awk_value_t wordwrap_in;
	HPDF_BOOL wordwrap;
	awk_value_t real_width;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_MeasureText: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_MeasureText called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_MeasureText: missing required text argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &wordwrap_in)) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode: missing required wordwrap argument"));
		RETURN_NOK;
	}
	
	if (wordwrap_in.num_value == 0)
		wordwrap  = HPDF_TRUE;
	else
		wordwrap  = HPDF_FALSE;
	
	if (!get_argument(4, AWK_NUMBER, &real_width)) {
		set_ERRNO(_("HPDF_ExtGState_SetBlendMode: missing required real_width argument"));
		RETURN_NOK;
	}

	mtext = HPDF_Page_MeasureText(page, (const char *)&text.str_value.str, 
		(HPDF_REAL)width.num_value, (HPDF_BOOL)wordwrap, (HPDF_REAL *)&real_width.num_value);
	
	if(mtext) {
		return make_number(mtext, result);
	}

	set_ERRNO(_("HPDF_Page_MeasureText failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetWidth(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetWidth(int nargs, awk_value_t *result)
{
	HPDF_REAL width;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("do_HPDF_Page_GetWidth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetWidth called with unknown page handle"));
		RETURN_NOK;
	}

	width = HPDF_Page_GetWidth(page);
	if(width) {
		return make_number(width, result);
	}

	set_ERRNO(_("HPDF_Page_GetWidth failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetHeight(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetHeight(int nargs, awk_value_t *result)
{
	int height;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("do_HPDF_Page_GetHeight: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetHeight called with unknown page handle"));
		RETURN_NOK;
	}

	height = HPDF_Page_GetHeight(page);
	if(height) {
		return make_number(height, result);
	}

	set_ERRNO(_("HPDF_Page_GetHeight failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_UINT16 HPDF_Page_GetGMode(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetGMode(int nargs, awk_value_t *result)
{

	int mode;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetGMode: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetGMode called with unknown page handle"));
		RETURN_NOK;
	}

	mode = HPDF_Page_GetGMode(page);
	
	if(mode) {
		return make_number(mode, result);
	}

	set_ERRNO(_("HPDF_Page_GetGMode failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_Point HPDF_Page_GetCurrentPos(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCurrentPos(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Point point;

	/* new array *
	awk_array_t point_array;
	awk_value_t index,value;

		point_array = create_array();

	*/

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentPos: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCurrentPos called with unknown page handle"));
		RETURN_NOK;
	}

	point = HPDF_Page_GetCurrentPos(page);

	/*  Umm, array result is difficult....
	make_const_string("x", 1, &index);
	make_number(point.x, &value);
	set_array_element(point_array, &index, &value);
	make_const_string("y", 1, &index);
	make_number(point.y, &value);
	set_array_element(point_array, &index, &value);

	result->val_type = AWK_ARRAY;
		result->array_cookie = point_array;
	*/

	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3.3f,%-3.3f", point.x, point.y);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
	}


	// return make_string_malloc((char*)point, strlen((char*)point), result);

}

/* HPDF_STATUS HPDF_Page_GetCurrentPos2(HPDF_Page page, HPDF_Point *pos); */
static awk_value_t *
do_HPDF_Page_GetCurrentPos2(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t pos;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentPos2: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCurrentPos2 called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &pos)) {
		set_ERRNO(_("HPDF_Page_GetCurrentPos2: missing required point argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_GetCurrentPos2(page, (HPDF_Point *)&pos.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_GetCurrentPos2: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_Point HPDF_Page_GetCurrentTextPos(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCurrentTextPos(int nargs, awk_value_t *result)
{
	HPDF_Point point;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentTextPos: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCurrentTextPos called with unknown page handle"));
		RETURN_NOK;
	}

	point = HPDF_Page_GetCurrentTextPos(page);

	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3.3f,%-3.3f", point.x, point.y);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
	}
	
	// return make_string_malloc((char*)&point, strlen((char*)&point), result);

}

/* HPDF_STATUS HPDF_Page_GetCurrentTextPos2(HPDF_Page page, HPDF_Point *pos); */
static awk_value_t *
do_HPDF_Page_GetCurrentTextPos2(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t pos;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentTextPos2: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCurrentTextPos2 called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &pos)) {
		set_ERRNO(_("HPDF_Page_GetCurrentTextPos2: missing required point argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_GetCurrentTextPos2(page, (HPDF_Point *)&pos.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_GetCurrentTextPos2: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_Font HPDF_Page_GetCurrentFont(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCurrentFont(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Font font;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentFont: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 1))) {
		set_ERRNO(_("HPDF_Page_GetCurrentFont called with unknown page input handle"));
		RETURN_NOK;
	}

	font = HPDF_Page_GetCurrentFont(page);

	if(font) {
		char hdl[HANDLE_SIZE];
		size_t hlen = font_handle(font, hdl, sizeof(hdl));
		return make_string_malloc(hdl, hlen, result);
	}
	set_ERRNO(_("HPDF_Page_GetCurrentFont failed"));
	RET_NULSTR;
}

/* HPDF_REAL HPDF_Page_GetCurrentFontSize(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCurrentFontSize(int nargs, awk_value_t *result)
{
	int size;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCurrentFontSize: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCurrentFontSize called with unknown page handle"));
		RETURN_NOK;
	}

	size = HPDF_Page_GetCurrentFontSize(page);
	
	if(size) {
		return make_number(size, result);
	}

	set_ERRNO(_("HPDF_Page_GetCurrentFontSize failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_TransMatrix HPDF_Page_GetTransMatrix(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTransMatrix(int nargs, awk_value_t *result)
{
	HPDF_TransMatrix matrix;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTransMatrix: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTransMatrix called with unknown page handle"));
		RETURN_NOK;
	}

	matrix = HPDF_Page_GetTransMatrix(page);
	
	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3f, %-3f, %-3f, %-3f, %-3f, %-3f", 
			matrix.a, matrix.b, matrix.c, matrix.d, matrix.x, matrix.y);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
        }

	// return make_string_malloc((char*)&matrix, strlen((char*)&matrix), result);

}

/* HPDF_REAL HPDF_Page_GetLineWidth(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetLineWidth(int nargs, awk_value_t *result)
{
	HPDF_REAL width;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetLineWidth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetLineWidth called with unknown page handle"));
		RETURN_NOK;
	}

	width = HPDF_Page_GetLineWidth(page);
	
	if(width) {
		return make_number((double)width, result);
	}

	set_ERRNO(_("HPDF_Page_GetLineWidth failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_LineCap HPDF_Page_GetLineCap(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetLineCap(int nargs, awk_value_t *result)
{
	int linec;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetLineCap: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetLineCap called with unknown page handle"));
		RETURN_NOK;
	}

	linec = HPDF_Page_GetLineCap(page);
	
	if(linec) {
		return make_number(linec, result);
	}

	set_ERRNO(_("HPDF_Page_GetLineCap failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_LineJoin HPDF_Page_GetLineJoin(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetLineJoin(int nargs, awk_value_t *result)
{
	int linej;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetLineJoin: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetLineJoin called with unknown page handle"));
		RETURN_NOK;
	}

	linej = HPDF_Page_GetLineJoin(page);
	
	if(linej) {
		return make_number(linej, result);
	}

	set_ERRNO(_("HPDF_Page_GetLineJoin failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetMiterLimit(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetMiterLimit(int nargs, awk_value_t *result)
{
	int limit;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetMiterLimit: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetMiterLimit called with unknown page handle"));
		RETURN_NOK;
	}

	limit = HPDF_Page_GetMiterLimit(page);
	
	if(limit) {
		return make_number(limit, result);
	}

	set_ERRNO(_("HPDF_Page_GetMiterLimit failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_DashMode HPDF_Page_GetDash(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetDash(int nargs, awk_value_t *result)
{
	HPDF_DashMode mode;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetDash: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetDash called with unknown page handle"));
		RETURN_NOK;
	}

	mode = HPDF_Page_GetDash(page);

	return make_string_malloc((char*)&mode, strlen((char*)&mode), result);

}

/* HPDF_REAL HPDF_Page_GetFlat(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetFlat(int nargs, awk_value_t *result)
{
	int flat;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetFlat: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetFlat called with unknown page handle"));
		RETURN_NOK;
	}

	flat = HPDF_Page_GetFlat(page);
	
	if(flat) {
		return make_number(flat, result);
	}

	set_ERRNO(_("HPDF_Page_GetFlat failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetCharSpace(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCharSpace(int nargs, awk_value_t *result)
{
	int space;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCharSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCharSpace called with unknown page handle"));
		RETURN_NOK;
	}

	space = HPDF_Page_GetCharSpace(page);
	
	if(space) {
		return make_number(space, result);
	}

	set_ERRNO(_("HPDF_Page_GetCharSpace failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetWordSpace(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetWordSpace(int nargs, awk_value_t *result)
{
	int space;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetWordSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetWordSpace called with unknown page handle"));
		RETURN_NOK;
	}

	space = HPDF_Page_GetWordSpace(page);
	
	if(space) {
		return make_number(space, result);
	}

	set_ERRNO(_("HPDF_Page_GetWordSpace failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetHorizontalScalling(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetHorizontalScalling(int nargs, awk_value_t *result)
{
	int horizontal;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetHorizontalScalling: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetHorizontalScalling called with unknown page handle"));
		RETURN_NOK;
	}

	horizontal = HPDF_Page_GetHorizontalScalling(page);
	
	if(horizontal) {
		return make_number(horizontal, result);
	}

	set_ERRNO(_("HPDF_Page_GetHorizontalScalling failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetTextLeading(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTextLeading(int nargs, awk_value_t *result)
{
	int textl;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTextLeading: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTextLeading called with unknown page handle"));
		RETURN_NOK;
	}

	textl = HPDF_Page_GetTextLeading(page);
	
	if(textl) {
		return make_number(textl, result);
	}

	set_ERRNO(_("HPDF_Page_GetTextLeading failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_TextRenderingMode HPDF_Page_GetTextRenderingMode(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTextRenderingMode(int nargs, awk_value_t *result)
{
	int mode;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTextRenderingMode: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTextRenderingMode called with unknown page handle"));
		RETURN_NOK;
	}

	mode = HPDF_Page_GetTextRenderingMode(page);
	
	if(mode) {
		return make_number(mode, result);
	}

	set_ERRNO(_("HPDF_Page_GetTextRenderingMode failed"));
	return make_number(PDF_NOK, result);
}

/* This function is obsolete. Use HPDF_Page_GetTextRise.  */

/* HPDF_REAL HPDF_Page_GetTextRaise(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTextRaise(int nargs, awk_value_t *result)
{
	int textr;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTextRaise: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTextRaise called with unknown page handle"));
		RETURN_NOK;
	}

	textr = HPDF_Page_GetTextRaise(page);
	
	if(textr) {
		return make_number(textr, result);
	}

	set_ERRNO(_("HPDF_Page_GetTextRaise failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_REAL HPDF_Page_GetTextRise(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTextRise(int nargs, awk_value_t *result)
{
	int textr;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTextRise: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTextRise called with unknown page handle"));
		RETURN_NOK;
	}

	textr = HPDF_Page_GetTextRise(page);
	
	if(textr) {
		return make_number(textr, result);
	}

	set_ERRNO(_("HPDF_Page_GetTextRise failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_RGBColor HPDF_Page_GetRGBFill(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetRGBFill(int nargs, awk_value_t *result)
{
	HPDF_RGBColor rgbc;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetRGBFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetRGBFill called with unknown page handle"));
		RETURN_NOK;
	}

	rgbc = HPDF_Page_GetRGBFill(page);

	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3f, %-3f, %-3f", rgbc.r, rgbc.g, rgbc.b);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
        }
	
	// return make_string_malloc((char*)&rgbc, strlen((char*)&rgbc), result);

}

/* HPDF_RGBColor HPDF_Page_GetRGBStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetRGBStroke(int nargs, awk_value_t *result)
{
	HPDF_RGBColor rgbc;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetRGBStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetRGBStroke called with unknown page handle"));
		RETURN_NOK;
	}

	rgbc = HPDF_Page_GetRGBStroke(page);
	
	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3f, %-3f, %-3f", rgbc.r, rgbc.g, rgbc.b);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
        }

	// return make_string_malloc((char*)&rgbc, strlen((char*)&rgbc), result);

}

/* HPDF_CMYKColor HPDF_Page_GetCMYKFill(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCMYKFill(int nargs, awk_value_t *result)
{
	HPDF_CMYKColor cmykc;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCMYKFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCMYKFill called with unknown page handle"));
		RETURN_NOK;
	}

	cmykc = HPDF_Page_GetCMYKFill(page);
	
	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3f, %-3f, %-3f, %-3f", cmykc.c, cmykc.m, cmykc.y, cmykc.k);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
        }

	// return make_string_malloc((char*)&rgbc, strlen((char*)&rgbc), result);

}

/* HPDF_CMYKColor HPDF_Page_GetCMYKStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetCMYKStroke(int nargs, awk_value_t *result)
{
	HPDF_CMYKColor cmykc;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetCMYKStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetCMYKStroke called with unknown page handle"));
		RETURN_NOK;
	}

	cmykc = HPDF_Page_GetCMYKStroke(page);

	{ /* please string result by split command */
		char str_result[HANDLE_SIZE];
		sprintf((char*)str_result,"%-3f, %-3f, %-3f, %-3f", cmykc.c, cmykc.m, cmykc.y, cmykc.k);
		return make_string_malloc(str_result, strlen((char*)str_result), result);
        }

	// return make_string_malloc((char*)&rgbc, strlen((char*)&rgbc), result);

}

/* HPDF_REAL HPDF_Page_GetGrayFill(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetGrayFill(int nargs, awk_value_t *result)
{
	int gfill;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetGrayFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetGrayFill called with unknown page handle"));
		RETURN_NOK;
	}

	gfill = HPDF_Page_GetGrayFill(page);
	
	if(gfill) {
		return make_number(gfill, result);
	}

	set_ERRNO(_("HPDF_Page_GetGrayFill failed"));
	return make_number(PDF_NOK, result);

}

/* HPDF_REAL HPDF_Page_GetGrayStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetGrayStroke(int nargs, awk_value_t *result)
{
	int stroke;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetGrayStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetGrayStroke called with unknown page handle"));
		RETURN_NOK;
	}

	stroke = HPDF_Page_GetGrayStroke(page);
	
	if(stroke) {
		return make_number(stroke, result);
	}

	set_ERRNO(_("HPDF_Page_GetGrayStroke failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_ColorSpace HPDF_Page_GetStrokingColorSpace (HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetStrokingColorSpace(int nargs, awk_value_t *result)
{
	int clspace;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetStrokingColorSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetStrokingColorSpace called with unknown page handle"));
		RETURN_NOK;
	}

	clspace = HPDF_Page_GetStrokingColorSpace(page);
	
	if(clspace) {
		return make_number(clspace, result);
	}

	set_ERRNO(_("HPDF_Page_GetStrokingColorSpace failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_ColorSpace HPDF_Page_GetFillingColorSpace (HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetFillingColorSpace(int nargs, awk_value_t *result)
{
	int clspace;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetFillingColorSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetFillingColorSpace called with unknown page handle"));
		RETURN_NOK;
	}

	clspace = HPDF_Page_GetFillingColorSpace(page);
	
	if(clspace) {
		return make_number(clspace, result);
	}

	set_ERRNO(_("HPDF_Page_GetFillingColorSpace failed"));
	return make_number(PDF_NOK, result);
}

/* HPDF_TransMatrix HPDF_Page_GetTextMatrix(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetTextMatrix(int nargs, awk_value_t *result)
{
	HPDF_TransMatrix matrix;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetTextMatrix: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetTextMatrix called with unknown page handle"));
		RETURN_NOK;
	}

	matrix = HPDF_Page_GetTextMatrix(page);

	return make_string_malloc((char*)&matrix, strlen((char*)&matrix), result);
}

/* HPDF_UINT HPDF_Page_GetGStateDepth(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GetGStateDepth(int nargs, awk_value_t *result)
{
	int depth;
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GetGStateDepth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GetGStateDepth called with unknown page handle"));
		RETURN_NOK;
	}

	depth = HPDF_Page_GetGStateDepth(page);
	
	if(depth) {
		return make_number(depth, result);
	}

	set_ERRNO(_("HPDF_Page_GetGStateDepth failed"));
	return make_number(PDF_NOK, result);
}

/*--- General graphics state ---------------------------------------------*/

/* HPDF_STATUS HPDF_Page_SetLineWidth(HPDF_Page page, PDF_REAL line_width); */
static awk_value_t *
do_HPDF_Page_SetLineWidth(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t line_width;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetLineWidth: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetLineWidth called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &line_width)) {
		set_ERRNO(_("HPDF_Page_SetLineWidth: missing required line_width argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetLineWidth(page, line_width.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetLineWidth: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetLineCap(HPDF_Page page, HPDF_LineCap line_cap); */
static awk_value_t *
do_HPDF_Page_SetLineCap(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t line_cap;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetLineCap: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetLineCap called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &line_cap)) {
		set_ERRNO(_("HPDF_Page_SetLineCap: missing required line_cap argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetLineCap(page, line_cap.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetLineCap: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetLineJoin(HPDF_Page page, HPDF_LineJoin line_join); */
static awk_value_t *
do_HPDF_Page_SetLineJoin(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t line_join;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetLineJoin: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetLineJoin called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &line_join)) {
		set_ERRNO(_("HPDF_Page_SetLineJoin: missing required line_join argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetLineJoin(page, line_join.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetLineJoin: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetMiterLimit(HPDF_Page page, HPDF_REAL miter_limit); */
static awk_value_t *
do_HPDF_Page_SetMiterLimit(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t miter_limit;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetMiterLimit: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetMiterLimit called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &miter_limit)) {
		set_ERRNO(_("HPDF_Page_SetMiterLimit: missing required miter_limit argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetMiterLimit(page, miter_limit.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetMiterLimit: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetDash(HPDF_Page page, const HPDF_UINT16 *dash_ptn, HPDF_UINT num_param, HPDF_UINT phase); */
static awk_value_t *
do_HPDF_Page_SetDash(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_UINT16 dash_ptn[] = {0, 0, 0, 0};

	awk_value_t dash_ptn_in;
	awk_flat_array_t *flat_array;
	size_t count = 0;
	int n;

	awk_value_t num_param;
	awk_value_t phase;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_SetDash: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetDash called with unknown page handle"));
		RETURN_NOK;
	}

	/* dash struct suport */
	if (!get_argument(1, AWK_ARRAY, &dash_ptn_in)) {
		set_ERRNO(_("HPDF_Page_SetDash: missing required dash_ptn argument"));
		RETURN_NOK;
	}

	if (!get_element_count(dash_ptn_in.array_cookie, &count)) {
		set_ERRNO(_("HPDF_Page_SetDash: missing required dash_ptn[n] argument"));
		RETURN_NOK;
	}

	if (!flatten_array(dash_ptn_in.array_cookie, &flat_array)) {
		set_ERRNO(_("HPDF_Page_SetDash: missing required dash_ptn[n] argument"));
		RETURN_NOK;
	}

/*
	if (!clear_array(dash_ptn_in.array_cookie)) {
		set_ERRNO(_("HPDF_Page_SetDash: clear dash_ptn[n] error."));
		RETURN_NOK;
	}
*/
		for (n = 0; n < (int)flat_array->count; n++) {
		dash_ptn[n] = flat_array->elements[n].value.num_value;
	}

	if (!get_argument(2, AWK_NUMBER, &num_param)) {
		set_ERRNO(_("HPDF_Page_SetDash: missing required num_param argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &phase)) {
		set_ERRNO(_("HPDF_Page_SetDash: missing required phase argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetDash(page, (const HPDF_UINT16 *)dash_ptn, 
			num_param.num_value, phase.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetDash: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Special graphic state operator --------------------------------------*/

/* HPDF_STATUS HPDF_Page_GSave(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GSave(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GSave: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GSave called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_GSave(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_GSave: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_GRestore(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_GRestore(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_GRestore: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_GRestore called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_GRestore(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_GRestore: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Concat(HPDF_Page page, HPDF_REAL a, HPDF_REAL b, HPDF_REAL c, HPDF_REAL d, HPDF_REAL x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_Concat(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t A;
	awk_value_t B;
	awk_value_t C;
	awk_value_t D;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 7)
		lintwarn(ext_id, _("HPDF_Page_Concat: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Concat called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &A)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required A argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &B)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required B argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &C)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required C argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &D)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required D argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(6, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_Concat: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_Concat(page, A.num_value, B.num_value, C.num_value, D.num_value, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Concat: can't move to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Path construction operator ------------------------------------------*/

/* HPDF_STATUS HPDF_Page_MoveTo(HPDF_Page page, HPDF_REA x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_MoveTo(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_MoveTo: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_MoveTo called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_MoveTo: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_MoveTo: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_MoveTo(page, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_MoveTo: can't move to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_LineTo(HPDF_Page page, HPDF_REAL x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_LineTo(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_LineTo: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_LineTo called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_LineTo: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_LineTo: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_LineTo(page, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_LineTo: can't line to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_CurveTo(HPDF_Page page,
  HPDF_REAL x1, HPDF_REAL y1, HPDF_REAL x2, HPDF_REAL y2, HPDF_REAL x3, HPDF_REAL y3); */
static awk_value_t *
do_HPDF_Page_CurveTo(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X1;
	awk_value_t Y1;
	awk_value_t X2;
	awk_value_t Y2;
	awk_value_t X3;
	awk_value_t Y3;

	if (do_lint && nargs != 7)
		lintwarn(ext_id, _("HPDF_Page_CurveTo: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CurveTo called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X1)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required X1 argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y1)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required Y1 argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &X2)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required X2 argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &Y2)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required Y2 argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &X3)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required X3 argument"));
		RETURN_NOK;
	}

	if (!get_argument(6, AWK_NUMBER, &Y3)) {
		set_ERRNO(_("HPDF_Page_CurveTo: missing required Y3 argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_CurveTo(page, X1.num_value, Y1.num_value, X2.num_value, Y2.num_value, X3.num_value, Y3.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_CurveTo: can't line to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_CurveTo2(HPDF_Page page, HPDF_REAL x2, HPDF_REAL y2, HPDF_REAL x3, HPDF_REAL y3); */
static awk_value_t *
do_HPDF_Page_CurveTo2(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X2;
	awk_value_t Y2;
	awk_value_t X3;
	awk_value_t Y3;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_CurveTo2: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CurveTo2 called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X2)) {
		set_ERRNO(_("HPDF_Page_CurveTo2: missing required X2 argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y2)) {
		set_ERRNO(_("HPDF_Page_CurveTo2: missing required Y2 argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &X3)) {
		set_ERRNO(_("HPDF_Page_CurveTo2: missing required X3 argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &Y3)) {
		set_ERRNO(_("HPDF_Page_CurveTo2: missing required Y3 argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_CurveTo2(page, X2.num_value, Y2.num_value, X3.num_value, Y3.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_CurveTo2: can't curve2 to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_CurveTo3(HPDF_Page page, HPDF_REAL x1, HPDF_REAL y1, HPDF_REAL x3, HPDF_REAL y3); */
static awk_value_t *
do_HPDF_Page_CurveTo3(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X1;
	awk_value_t Y1;
	awk_value_t X3;
	awk_value_t Y3;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_CurveTo3: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_CurveTo3 called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X1)) {
		set_ERRNO(_("HPDF_Page_CurveTo3: missing required X1 argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y1)) {
		set_ERRNO(_("HPDF_Page_CurveTo3: missing required Y1 argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &X3)) {
		set_ERRNO(_("HPDF_Page_CurveTo3: missing required X3 argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &Y3)) {
		set_ERRNO(_("HPDF_Page_CurveTo3: missing required Y3 argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_CurveTo3(page, X1.num_value, Y1.num_value, X3.num_value, Y3.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_CurveTo3: can't curve3 to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ClosePath(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_ClosePath(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_ClosePath: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ClosePath called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_ClosePath(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ClosePath: can't curve3 to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Rectangle(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL width, HPDF_REAL height); */
static awk_value_t *
do_HPDF_Page_Rectangle(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;
	awk_value_t width;
	awk_value_t height;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_Rectangle: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Rectangle called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_Rectangle: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_Rectangle: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_Page_Rectangle: missing required width argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &height)) {
		set_ERRNO(_("HPDF_Page_Rectangle: missing required height argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_Rectangle(page, X.num_value, Y.num_value, width.num_value, height.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Rectangle: can't rectangle to page."));
		RETURN_NOK;
	}

	RETURN_OK;
}


/*--- Path painting operator ---------------------------------------------*/

/* HPDF_STATUS HPDF_Page_Stroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_Stroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_Stroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Stroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Stroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Stroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ClosePathStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_ClosePathStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_ClosePathStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ClosePathStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_ClosePathStroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ClosePathStroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Fill(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_Fill(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_Fill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Fill called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Fill(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Fill: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Eofill(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_Eofill(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_Eofill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Eofill called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Eofill(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Eofill: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_FillStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_FillStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_FillStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_FillStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_FillStroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_FillStroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_EofillStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_EofillStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_EofillStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_EofillStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_EofillStroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_EofillStroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ClosePathFillStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_ClosePathFillStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_ClosePathFillStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ClosePathFillStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_ClosePathFillStroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ClosePathFillStroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ClosePathEofillStroke(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_ClosePathEofillStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_ClosePathEofillStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ClosePathEofillStroke called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_ClosePathEofillStroke(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ClosePathEofillStroke: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_EndPath(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_EndPath(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_EndPath: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_EndPath called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Clip(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_EndPath: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Clipping paths operator --------------------------------------------*/

/* HPDF_STATUS HPDF_Page_Clip(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_Clip(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_Clip: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Clip called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Clip(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Clip: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Eoclip(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_Eoclip(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_Eoclip: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Eoclip called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_Eoclip(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Eoclip: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Text object operator -----------------------------------------------*/

/* HPDF_STATUS HPDF_Page_BeginText(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_BeginText(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_BeginText: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_BeginText called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_BeginText(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_BeginText: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_EndText(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_EndText(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_EndText: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_EndText called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_EndText(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_EndText: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Text state ---------------------------------------------------------*/

/* HPDF_STATUS HPDF_Page_SetCharSpace(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetCharSpace(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetCharSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetCharSpace called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetCharSpace: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetCharSpace(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetCharSpace: can't set char spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetWordSpace(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetWordSpace(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetWordSpace: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetWordSpace called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetWordSpace: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetWordSpace(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetWordSpace: can't set word spaces."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetHorizontalScalling(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetHorizontalScalling(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetHorizontalScalling: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetHorizontalScalling called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetHorizontalScalling: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetHorizontalScalling(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetHorizontalScalling: can't set horizontal scalling."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetTextLeading(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetTextLeading(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetTextLeading: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetTextLeading called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetTextLeading: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetTextLeading(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetTextLeading: can't set text leading."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetFontAndSize(HPDF_Page page, HPDF_Font font, HPDF_REAL size); */
static awk_value_t *
do_HPDF_Page_SetFontAndSize(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Font font;
	awk_value_t size;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_SetFontAndSize: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetFontAndSize called with unknown page handle"));
		RETURN_NOK;
	}

	if (!(font = find_handle(pdffonts, 1))) {
		set_ERRNO(_("HPDF_Page_SetFontAndSize called with unknown font handle"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &size)) {
		set_ERRNO(_("HPDF_Page_SetFontAndSize: missing required size argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetFontAndSize(page, font, size.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_BeginText: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetTextRenderingMode(HPDF_Page page, HPDF_TextRenderingMode  mode); */
static awk_value_t *
do_HPDF_Page_SetTextRenderingMode(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t mode;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetTextRenderingMode: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetTextRenderingMode called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &mode)) {
		set_ERRNO(_("HPDF_Page_SetTextRenderingMode: missing required mode argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetTextRenderingMode(page, mode.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetTextRenderingMode: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetTextRise(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetTextRise(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_arg;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetTextRise: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetTextRise called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_arg)) {
		set_ERRNO(_("HPDF_Page_SetTextRise: missing required value argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetTextRise(page, val_arg.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetTextRise: can't set text rise."));
		RETURN_NOK;
	}

	RETURN_OK;
}


/* This function is obsolete. Use HPDF_Page_SetTextRise.  */

/* HPDF_STATUS HPDF_Page_SetTextRaise(HPDF_Page page, HPDF_REAL value); */
static awk_value_t *
do_HPDF_Page_SetTextRaise(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t val_str;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetTextRaise: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetTextRaise called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &val_str)) {
		set_ERRNO(_("HPDF_Page_SetTextRaise: missing required X argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetTextRaise(page, val_str.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetTextRaise: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Text positioning ---------------------------------------------------*/

/* HPDF_STATUS HPDF_Page_MoveTextPos(HPDF_Page page, HPDF_REAL x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_MoveTextPos(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_MoveTextPos: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_MoveTextPos called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_MoveTextPos: missing required X argument"));
		RETURN_NOK;
	}
	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_MoveTextPos: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_MoveTextPos(page, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_MoveTextPos: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_MoveTextPos2(HPDF_Page page, HPDF_REAL x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_MoveTextPos2(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 3)
		lintwarn(ext_id, _("HPDF_Page_MoveTextPos2: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_MoveTextPos2 called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_MoveTextPos2: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_MoveTextPos2: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_MoveTextPos2(page, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_MoveTextPos2: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetTextMatrix(HPDF_Page page, HPDF_REAL a, HPDF_REAL b, HPDF_REAL c, HPDF_REAL d, HPDF_REAL x, HPDF_REAL y); */
static awk_value_t *
do_HPDF_Page_SetTextMatrix(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t A;
	awk_value_t B;
	awk_value_t C;
	awk_value_t D;
	awk_value_t X;
	awk_value_t Y;

	if (do_lint && nargs != 7)
		lintwarn(ext_id, _("HPDF_Page_SetTextMatrix: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &A)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required A argument"));
		RETURN_NOK;
	}
	if (!get_argument(2, AWK_NUMBER, &B)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required B argument"));
		RETURN_NOK;
	}
	if (!get_argument(3, AWK_NUMBER, &C)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required C argument"));
		RETURN_NOK;
	}
	if (!get_argument(4, AWK_NUMBER, &D)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required D argument"));
		RETURN_NOK;
	}
	if (!get_argument(5, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required X argument"));
		RETURN_NOK;
	}
	if (!get_argument(6, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: missing required Y argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetTextMatrix(page, A.num_value, B.num_value, C.num_value, D.num_value, X.num_value, Y.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetTextMatrix: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_MoveToNextLine(HPDF_Page page); */
static awk_value_t *
do_HPDF_Page_MoveToNextLine(int nargs, awk_value_t *result)
{
	HPDF_Page page;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, _("HPDF_Page_MoveToNextLine: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_MoveToNextLine called with unknown page handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_MoveToNextLine(page) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_MoveToNextLine: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- Text showing -------------------------------------------------------*/

/* HPDF_STATUS HPDF_Page_ShowText(HPDF_Page page, const char *text); */
static awk_value_t *
do_HPDF_Page_ShowText(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t text;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_ShowText: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ShowText: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_ShowText: missing required size argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_ShowText(page, (const char *)text.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ShowText: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ShowTextNextLine(HPDF_Page page, const char *text); */
static awk_value_t *
do_HPDF_Page_ShowTextNextLine(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t text;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_ShowTextNextLine: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLine: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLine: missing required size argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_ShowTextNextLine(page, (const char *)text.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLine: can't get page handle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_ShowTextNextLineEx(HPDF_Page page, HPDF_REAL word_space, HPDF_REAL char_space, const char *text); */
static awk_value_t *
do_HPDF_Page_ShowTextNextLineEx(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t word_space;
	awk_value_t char_space;
	awk_value_t text;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_ShowTextNextLineEx: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLineEx: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &word_space)) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLineEx: missing required word_space argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &char_space)) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLineEx: missing required char_space argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLineEx: missing required text argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_ShowTextNextLineEx(page, word_space.num_value, char_space.num_value, text.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ShowTextNextLineEx: can't set gray fill."));
		RETURN_NOK;
	}

	RETURN_OK;
}


/*--- Color showing ------------------------------------------------------*/

/* HPDF_STATUS HPDF_Page_SetGrayFill(HPDF_Page page, HPDF_REAL gray); */
static awk_value_t *
do_HPDF_Page_SetGrayFill(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t gray;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetGrayFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetGrayFill: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &gray)) {
		set_ERRNO(_("HPDF_Page_SetGrayFill: missing required gray argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetGrayFill(page, gray.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetGrayFill: can't set gray fill."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetGrayStroke(HPDF_Page page, HPDF_REAL gray); */
static awk_value_t *
do_HPDF_Page_SetGrayStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t gray;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_SetGrayStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetGrayStroke: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &gray)) {
		set_ERRNO(_("HPDF_Page_SetGrayStroke: missing required gray argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetGrayStroke(page, gray.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetGrayStroke: can't set gray stroke."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetRGBFill(HPDF_Page page, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b); */
static awk_value_t *
do_HPDF_Page_SetRGBFill(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t R;
	awk_value_t G;
	awk_value_t B;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_SetRGBFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetRGBFill: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &R)) {
		set_ERRNO(_("HPDF_Page_SetRGBFill: missing required R argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &G)) {
		set_ERRNO(_("HPDF_Page_SetRGBFill: missing required R argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &B)) {
		set_ERRNO(_("HPDF_Page_SetRGBFill: missing required R argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetRGBFill(page, R.num_value, G.num_value, B.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetRGBFill: can't set RGB fill."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetRGBStroke(HPDF_Page page, HPDF_REAL r, HPDF_REAL g, HPDF_REAL b); */
static awk_value_t *
do_HPDF_Page_SetRGBStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t R;
	awk_value_t G;
	awk_value_t B;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_SetRGBStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetRGBStroke: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &R)) {
		set_ERRNO(_("HPDF_Page_SetRGBStroke: missing required R argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &G)) {
		set_ERRNO(_("HPDF_Page_SetRGBStroke: missing required R argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &B)) {
		set_ERRNO(_("HPDF_Page_SetRGBStroke: missing required R argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetRGBStroke(page, R.num_value, G.num_value, B.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetRGBStroke: can't set RGB stroke."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetCMYKFill(HPDF_Page page, HPDF_REAL c, HPDF_REAL m, HPDF_REAL y, HPDF_REAL k); */
static awk_value_t *
do_HPDF_Page_SetCMYKFill(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t C;
	awk_value_t M;
	awk_value_t Y;
	awk_value_t K;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_SetCMYKFill: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &C)) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: missing required C argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &M)) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: missing required M argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &K)) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: missing required K argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetCMYKFill(page, C.num_value, M.num_value, Y.num_value, K.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetCMYKFill: can't set CMYK fill."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetCMYKStroke(HPDF_Page page, HPDF_REAL c, HPDF_REAL m, HPDF_REAL y, HPDF_REAL k); */
static awk_value_t *
do_HPDF_Page_SetCMYKStroke(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t C;
	awk_value_t M;
	awk_value_t Y;
	awk_value_t K;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_SetCMYKStroke: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &C)) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: missing required C argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &M)) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: missing required M argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &K)) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: missing required K argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetCMYKStroke(page, C.num_value, M.num_value, Y.num_value, K.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetCMYKStroke: can't set CMYK stroke."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*--- XObjects -----------------------------------------------------------*/

/* Do */
/* HPDF_STATUS HPDF_Page_ExecuteXObject(HPDF_Page page, HPDF_XObject obj); */
static awk_value_t *
do_HPDF_Page_ExecuteXObject(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Image image;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, _("HPDF_Page_ExecuteXObject: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_ExecuteXObject: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!(image = find_handle(pdfimages, 1))) {
		set_ERRNO(_("HPDF_Page_ExecuteXObject: called with unknown image handle"));
		RETURN_NOK;
	}

	if (HPDF_Page_ExecuteXObject(page, image) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_ExecuteXObject: can't execute image."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_DrawImage(HPDF_Page page, HPDF_Image image, HPDF_REAL x, HPDF_REAL y, HPDF_REAL width, HPDF_REAL height); */
static awk_value_t *
do_HPDF_Page_DrawImage(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	HPDF_Image image;
	awk_value_t X;
	awk_value_t Y;
	awk_value_t width;
	awk_value_t height;

	if (do_lint && nargs != 6)
		lintwarn(ext_id, _("HPDF_Page_DrawImage: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_DrawImage: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!(image = find_handle(pdfimages, 1))) {
		set_ERRNO(_("HPDF_Page_DrawImage: called with unknown image handle"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_DrawImage: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_DrawImage: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &width)) {
		set_ERRNO(_("HPDF_Page_DrawImage: missing required ray argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &height)) {
		set_ERRNO(_("HPDF_Page_DrawImage: missing required ray argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_DrawImage(page, image, (HPDF_REAL)X.num_value, (HPDF_REAL)Y.num_value, 
			(HPDF_REAL)width.num_value, (HPDF_REAL)height.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_DrawImage: can't set draw image."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Circle(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL ray); */
static awk_value_t *
do_HPDF_Page_Circle(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;
	awk_value_t ray;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_Circle: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Circle: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_Circle: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_Circle: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &ray)) {
		set_ERRNO(_("HPDF_Page_Circle: missing required ray argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_Circle(page, X.num_value, Y.num_value, ray.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Circle: can't set page circle."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Ellipse(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL xray, HPDF_REAL yray); */
static awk_value_t *
do_HPDF_Page_Ellipse(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;
	awk_value_t xray;
	awk_value_t yray;

	if (do_lint && nargs != 5)
		lintwarn(ext_id, _("HPDF_Page_Ellipse: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Ellipse: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_Ellipse: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_Ellipse: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &xray)) {
		set_ERRNO(_("HPDF_Page_Ellipse: missing required xray argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &yray)) {
		set_ERRNO(_("HPDF_Page_Ellipse: missing required yray argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_Ellipse(page, X.num_value, Y.num_value, xray.num_value, yray.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Ellipse: can't set page ellipse."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_Arc(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, HPDF_REAL ray, HPDF_REAL ang1, HPDF_REAL ang2); */
static awk_value_t *
do_HPDF_Page_Arc(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t X;
	awk_value_t Y;
	awk_value_t ray;
	awk_value_t ang1;
	awk_value_t ang2;

	if (do_lint && nargs != 6)
		lintwarn(ext_id, _("HPDF_Page_Arc: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_Arc: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &X)) {
		set_ERRNO(_("HPDF_Page_Arc: missing required X argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &Y)) {
		set_ERRNO(_("HPDF_Page_Arc: missing required Y argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &ray)) {
		set_ERRNO(_("HPDF_Page_Arc: missing required ray argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &ang1)) {
		set_ERRNO(_("HPDF_Page_Arc: missing required ang1 argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_NUMBER, &ang2)) {
		set_ERRNO(_("HPDF_Page_Arc: missing required ang2 argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_Arc(page, X.num_value, Y.num_value, ray.num_value, ang1.num_value, ang2.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_Arc: can't set page arc."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_TextOut(HPDF_Page page, HPDF_REAL xpos, HPDF_REAL ypos, const char *text); */
static awk_value_t *
do_HPDF_Page_TextOut(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t xpos;
	awk_value_t ypos;
	awk_value_t text;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_TextOut: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_TextOut: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &xpos)) {
		set_ERRNO(_("HPDF_Page_TextOut: missing required xpos argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &ypos)) {
		set_ERRNO(_("HPDF_Page_TextOut: missing required ypos argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_TextOut: missing required text argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_TextOut(page, xpos.num_value, ypos.num_value, (const char *)text.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_TextOut: can't set page text out."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_TextRect(HPDF_Page page,
  HPDF_REAL left, HPDF_REAL top, HPDF_REAL right, HPDF_REAL bottom, const char *text, HPDF_TextAlignment align, HPDF_UINT  *len); */
static awk_value_t *
do_HPDF_Page_TextRect(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t left;
	awk_value_t top;
	awk_value_t right;
	awk_value_t bottom;
	awk_value_t text;
	awk_value_t align;
	awk_value_t len;

	if (do_lint && nargs != 8)
		lintwarn(ext_id, _("HPDF_Page_TextRect: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_TextRect: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &left)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required left argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &top)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required top argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &right)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required right argument"));
		RETURN_NOK;
	}

	if (!get_argument(4, AWK_NUMBER, &bottom)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required bottom argument"));
		RETURN_NOK;
	}

	if (!get_argument(5, AWK_STRING, &text)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required text argument"));
		RETURN_NOK;
	}

	if (!get_argument(6, AWK_NUMBER, &align)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required align argument"));
		RETURN_NOK;
	}

	if (!get_argument(7, AWK_STRING, &len)) {
		set_ERRNO(_("HPDF_Page_TextRect: missing required len argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_TextRect(page, left.num_value, top.num_value, right.num_value, bottom.num_value, 
			(const char *)text.str_value.str, align.num_value, (HPDF_UINT *)&len.str_value.str) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_TextRect: can't set page text rect."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/* HPDF_STATUS HPDF_Page_SetSlideShow(HPDF_Page page, HPDF_TransitionStyle type, HPDF_REAL disp_time, HPDF_REAL trans_time); */
static awk_value_t *
do_HPDF_Page_SetSlideShow(int nargs, awk_value_t *result)
{
	HPDF_Page page;
	awk_value_t type;
	awk_value_t disp_time;
	awk_value_t trans_time;

	if (do_lint && nargs != 4)
		lintwarn(ext_id, _("HPDF_Page_SetSlideShow: called with incorrect number of arguments"));

	if (!(page = find_handle(pdfpages, 0))) {
		set_ERRNO(_("HPDF_Page_SetSlideShow: called with unknown page handle"));
		RETURN_NOK;
	}

	if (!get_argument(1, AWK_NUMBER, &type)) {
		set_ERRNO(_("HPDF_Page_SetSlideShow: missing required type argument"));
		RETURN_NOK;
	}

	if (!get_argument(2, AWK_NUMBER, &disp_time)) {
		set_ERRNO(_("HPDF_Page_SetSlideShow: missing required disp_time argument"));
		RETURN_NOK;
	}

	if (!get_argument(3, AWK_NUMBER, &trans_time)) {
		set_ERRNO(_("HPDF_Page_SetSlideShow: missing required trans_time argument"));
		RETURN_NOK;
	}

	if (HPDF_Page_SetSlideShow(page, type.num_value, disp_time.num_value, trans_time.num_value) != HPDF_OK) {
		set_ERRNO(_("HPDF_Page_SetSlideShow: can't set page set slideShow."));
		RETURN_NOK;
	}

	RETURN_OK;
}

/*
 * ---------------------------------------------------------------------
 * function tables
 * ---------------------------------------------------------------------
 */
static awk_ext_func_t func_table[] = {
	/* functions */
	{"HPDF_GetVersion",	do_HPDF_GetVersion,0},
	{"HPDF_New",	do_HPDF_New,2},
	{"HPDF_Free",	do_HPDF_Free,1},
	{"HPDF_NewDoc",	do_HPDF_NewDoc,1},
	{"HPDF_FreeDoc",	do_HPDF_FreeDoc,1},
	{"HPDF_HasDoc",	do_HPDF_HasDoc,1},
	{"HPDF_FreeDocAll",	do_HPDF_FreeDocAll,1},
	{"HPDF_GetContents",	do_HPDF_GetContents,3},
	{"HPDF_SaveToFile",	do_HPDF_SaveToFile,2},
	{"HPDF_GetError",	do_HPDF_GetError,1},
	{"HPDF_GetErrorDetail",	do_HPDF_GetErrorDetail,1},
	{"HPDF_ResetError",	do_HPDF_ResetError,1},
	{"HPDF_CheckError",	do_HPDF_CheckError,1},
	{"HPDF_SetPagesConfiguration",	do_HPDF_SetPagesConfiguration,2},
	{"HPDF_GetPageByIndex",	do_HPDF_GetPageByIndex,2},
	{"HPDF_GetPageLayout",	do_HPDF_GetPageLayout,1},
	{"HPDF_SetPageLayout",	do_HPDF_SetPageLayout,2},
	{"HPDF_GetPageMode",	do_HPDF_GetPageMode,1},
	{"HPDF_SetPageMode",	do_HPDF_SetPageMode,2},
	{"HPDF_GetViewerPreference",	do_HPDF_GetViewerPreference,1},
	{"HPDF_SetViewerPreference",	do_HPDF_SetViewerPreference,2},
	{"HPDF_SetOpenAction",	do_HPDF_SetOpenAction,2},
	{"HPDF_GetCurrentPage",	do_HPDF_GetCurrentPage,1},
	{"HPDF_AddPage",	do_HPDF_AddPage,1},
	{"HPDF_InsertPage",	do_HPDF_InsertPage,2},
	{"HPDF_Page_SetWidth",	do_HPDF_Page_SetWidth,2},
	{"HPDF_Page_SetHeight",	do_HPDF_Page_SetHeight,2},
	{"HPDF_Page_SetSize",	do_HPDF_Page_SetSize,3},
	{"HPDF_Page_SetRotate",	do_HPDF_Page_SetRotate,2},
	{"HPDF_Page_SetZoom",	do_HPDF_Page_SetZoom,2},
	{"HPDF_GetFont",	do_HPDF_GetFont,3},
	{"HPDF_LoadType1FontFromFile",	do_HPDF_LoadType1FontFromFile,3},
	{"HPDF_GetTTFontDefFromFile",	do_HPDF_GetTTFontDefFromFile,3},
	{"HPDF_LoadTTFontFromFile",	do_HPDF_LoadTTFontFromFile,3},
	{"HPDF_AddPageLabel",	do_HPDF_AddPageLabel,5},
	{"HPDF_UseJPFonts",	do_HPDF_UseJPFonts,1},
	{"HPDF_UseKRFonts",	do_HPDF_UseKRFonts,1},
	{"HPDF_UseCNSFonts",	do_HPDF_UseCNSFonts,1},
	{"HPDF_UseCNTFonts",	do_HPDF_UseCNTFonts,1},
	{"HPDF_CreateOutline",	do_HPDF_CreateOutline,4},
	{"HPDF_Outline_SetOpened",	do_HPDF_Outline_SetOpened,2},
	{"HPDF_Outline_SetDestination",	do_HPDF_Outline_SetDestination,2},
	{"HPDF_Page_CreateDestination",	do_HPDF_Page_CreateDestination,1},
	{"HPDF_Destination_SetXYZ",	do_HPDF_Destination_SetXYZ,4},
	{"HPDF_Destination_SetFit",	do_HPDF_Destination_SetFit,1},
	{"HPDF_Destination_SetFitH",	do_HPDF_Destination_SetFitH,2},
	{"HPDF_Destination_SetFitV",	do_HPDF_Destination_SetFitV,2},
	{"HPDF_Destination_SetFitR",	do_HPDF_Destination_SetFitR,5},
	{"HPDF_Destination_SetFitB",	do_HPDF_Destination_SetFitB,1},
	{"HPDF_Destination_SetFitBH",	do_HPDF_Destination_SetFitBH,2},
	{"HPDF_Destination_SetFitBV",	do_HPDF_Destination_SetFitBV,2},
	{"HPDF_GetEncoder",	do_HPDF_GetEncoder,2},
	{"HPDF_GetCurrentEncoder",	do_HPDF_GetCurrentEncoder,1},
	{"HPDF_SetCurrentEncoder",	do_HPDF_SetCurrentEncoder,2},
	{"HPDF_Encoder_GetType",	do_HPDF_Encoder_GetType,1},
	{"HPDF_Encoder_GetByteType",	do_HPDF_Encoder_GetByteType,3},
	{"HPDF_Encoder_GetUnicode",	do_HPDF_Encoder_GetUnicode,2},
	{"HPDF_Encoder_GetWritingMode",	do_HPDF_Encoder_GetWritingMode,1},
	{"HPDF_UseJPEncodings",	do_HPDF_UseJPEncodings,1},
	{"HPDF_UseKREncodings",	do_HPDF_UseKREncodings,1},
	{"HPDF_UseCNSEncodings",	do_HPDF_UseCNSEncodings,1},
	{"HPDF_UseCNTEncodings",	do_HPDF_UseCNTEncodings,1},
#if (HPDF_MAJOR_VERSION == 2) && (HPDF_MINOR_VERSION < 4)
	{"HPDF_Page_Create3DAnnot",	do_HPDF_Page_Create3DAnnot,3},
#else
	{"HPDF_Page_Create3DAnnot",	do_HPDF_Page_Create3DAnnot,6},
#endif
	{"HPDF_Page_CreateTextAnnot",	do_HPDF_Page_CreateTextAnnot,4},
	{"HPDF_Page_CreateLinkAnnot",	do_HPDF_Page_CreateLinkAnnot,3},
	{"HPDF_Page_CreateURILinkAnnot",	do_HPDF_Page_CreateURILinkAnnot,3},
	{"HPDF_LinkAnnot_SetHighlightMode",	do_HPDF_LinkAnnot_SetHighlightMode,2},
	{"HPDF_LinkAnnot_SetBorderStyle",	do_HPDF_LinkAnnot_SetBorderStyle,4},
	{"HPDF_TextAnnot_SetIcon",	do_HPDF_TextAnnot_SetIcon,2},
	{"HPDF_TextAnnot_SetOpened",	do_HPDF_TextAnnot_SetOpened,2},
	{"HPDF_LoadPngImageFromMem",	do_HPDF_LoadPngImageFromMem,3},
	{"HPDF_LoadPngImageFromFile",	do_HPDF_LoadPngImageFromFile,2},
	{"HPDF_LoadJpegImageFromFile",	do_HPDF_LoadJpegImageFromFile,2},
	{"HPDF_LoadU3DFromFile",	do_HPDF_LoadU3DFromFile,2},
	{"HPDF_LoadRawImageFromMem",	do_HPDF_LoadRawImageFromMem,6},
	{"HPDF_LoadRawImageFromFile",	do_HPDF_LoadRawImageFromFile,5},
	{"HPDF_Image_AddSMask",	do_HPDF_Image_AddSMask,2},
	{"HPDF_Image_GetSize",	do_HPDF_Image_GetSize,1},
	{"HPDF_Image_GetSize2",	do_HPDF_Image_GetSize2,2},
	{"HPDF_Image_GetWidth",	do_HPDF_Image_GetWidth,1},
	{"HPDF_Image_GetHeight",	do_HPDF_Image_GetHeight,1},
	{"HPDF_Image_GetBitsPerComponent",	do_HPDF_Image_GetBitsPerComponent,1},
	{"HPDF_Image_GetColorSpace",	do_HPDF_Image_GetColorSpace,1},
	{"HPDF_Image_SetColorMask",	do_HPDF_Image_SetColorMask,7},
	{"HPDF_Image_SetMaskImage",	do_HPDF_Image_SetMaskImage,2},
	{"HPDF_SetInfoAttr",	do_HPDF_SetInfoAttr,3},
	{"HPDF_GetInfoAttr",	do_HPDF_GetInfoAttr,2},
	{"HPDF_SetInfoDateAttr",	do_HPDF_SetInfoDateAttr,3},
	{"HPDF_SetPassword",	do_HPDF_SetPassword,3},
	{"HPDF_SetPermission",	do_HPDF_SetPermission,2},
	{"HPDF_SetEncryptionMode",	do_HPDF_SetEncryptionMode,3},
	{"HPDF_SetCompressionMode",	do_HPDF_SetCompressionMode,2},
	{"HPDF_Font_GetFontName",	do_HPDF_Font_GetFontName,1},
	{"HPDF_Font_GetEncodingName",	do_HPDF_Font_GetEncodingName,1},
	{"HPDF_Font_GetUnicodeWidth",	do_HPDF_Font_GetUnicodeWidth,2},
	{"HPDF_Font_GetBBox",	do_HPDF_Font_GetBBox,1},
	{"HPDF_Font_GetAscent",	do_HPDF_Font_GetAscent,1},
	{"HPDF_Font_GetDescent",	do_HPDF_Font_GetDescent,1},
	{"HPDF_Font_GetXHeight",	do_HPDF_Font_GetXHeight,1},
	{"HPDF_Font_GetCapHeight",	do_HPDF_Font_GetCapHeight,1},
	{"HPDF_Font_TextWidth",	do_HPDF_Font_TextWidth,3},
	{"HPDF_Font_MeasureText",	do_HPDF_Font_MeasureText,9},
	{"HPDF_CreateExtGState",	do_HPDF_CreateExtGState,1},
	{"HPDF_ExtGState_SetAlphaStroke",	do_HPDF_ExtGState_SetAlphaStroke,2},
	{"HPDF_ExtGState_SetAlphaFill",	do_HPDF_ExtGState_SetAlphaFill,2},
	{"HPDF_ExtGState_SetBlendMode",	do_HPDF_ExtGState_SetBlendMode,2},
	{"HPDF_Page_SetExtGState",	do_HPDF_Page_SetExtGState,2},
	{"HPDF_Page_TextWidth",	do_HPDF_Page_TextWidth,2},
	{"HPDF_Page_MeasureText",	do_HPDF_Page_MeasureText,5},
	{"HPDF_Page_GetWidth",	do_HPDF_Page_GetWidth,1},
	{"HPDF_Page_GetHeight",	do_HPDF_Page_GetHeight,1},
	{"HPDF_Page_GetGMode",	do_HPDF_Page_GetGMode,1},
	{"HPDF_Page_GetCurrentPos",	do_HPDF_Page_GetCurrentPos,1},
	{"HPDF_Page_GetCurrentPos2",	do_HPDF_Page_GetCurrentPos2,2},
	{"HPDF_Page_GetCurrentTextPos",	do_HPDF_Page_GetCurrentTextPos,1},
	{"HPDF_Page_GetCurrentTextPos2",	do_HPDF_Page_GetCurrentTextPos2,2},
	{"HPDF_Page_GetCurrentFont",	do_HPDF_Page_GetCurrentFont,1},
	{"HPDF_Page_GetCurrentFontSize",	do_HPDF_Page_GetCurrentFontSize,1},
	{"HPDF_Page_GetTransMatrix",	do_HPDF_Page_GetTransMatrix,1},
	{"HPDF_Page_GetLineWidth",	do_HPDF_Page_GetLineWidth,1},
	{"HPDF_Page_GetLineCap",	do_HPDF_Page_GetLineCap,1},
	{"HPDF_Page_GetLineJoin",	do_HPDF_Page_GetLineJoin,1},
	{"HPDF_Page_GetMiterLimit",	do_HPDF_Page_GetMiterLimit,1},
	{"HPDF_Page_GetDash",	do_HPDF_Page_GetDash,1},
	{"HPDF_Page_GetFlat",	do_HPDF_Page_GetFlat,1},
	{"HPDF_Page_GetCharSpace",	do_HPDF_Page_GetCharSpace,1},
	{"HPDF_Page_GetWordSpace",	do_HPDF_Page_GetWordSpace,1},
	{"HPDF_Page_GetHorizontalScalling",	do_HPDF_Page_GetHorizontalScalling,1},
	{"HPDF_Page_GetTextLeading",	do_HPDF_Page_GetTextLeading,1},
	{"HPDF_Page_GetTextRenderingMode",	do_HPDF_Page_GetTextRenderingMode,1},
	{"HPDF_Page_GetTextRaise",	do_HPDF_Page_GetTextRaise,1},
	{"HPDF_Page_GetTextRise",	do_HPDF_Page_GetTextRise,1},
	{"HPDF_Page_GetRGBFill",	do_HPDF_Page_GetRGBFill,1},
	{"HPDF_Page_GetRGBStroke",	do_HPDF_Page_GetRGBStroke,1},
	{"HPDF_Page_GetCMYKFill",	do_HPDF_Page_GetCMYKFill,1},
	{"HPDF_Page_GetCMYKStroke",	do_HPDF_Page_GetCMYKStroke,1},
	{"HPDF_Page_GetGrayFill",	do_HPDF_Page_GetGrayFill,1},
	{"HPDF_Page_GetGrayStroke",	do_HPDF_Page_GetGrayStroke,1},
	{"HPDF_Page_GetStrokingColorSpace",	do_HPDF_Page_GetStrokingColorSpace,1},
	{"HPDF_Page_GetFillingColorSpace",	do_HPDF_Page_GetFillingColorSpace,1},
	{"HPDF_Page_GetTextMatrix",	do_HPDF_Page_GetTextMatrix,1},
	{"HPDF_Page_GetGStateDepth",	do_HPDF_Page_GetGStateDepth,1},
	{"HPDF_Page_SetLineWidth",	do_HPDF_Page_SetLineWidth,2},
	{"HPDF_Page_SetLineCap",	do_HPDF_Page_SetLineCap,2},
	{"HPDF_Page_SetLineJoin",	do_HPDF_Page_SetLineJoin,2},
	{"HPDF_Page_SetMiterLimit",	do_HPDF_Page_SetMiterLimit,2},
	{"HPDF_Page_SetDash",	do_HPDF_Page_SetDash,4},
	{"HPDF_Page_GSave",	do_HPDF_Page_GSave,1},
	{"HPDF_Page_GRestore",	do_HPDF_Page_GRestore,1},
	{"HPDF_Page_Concat",	do_HPDF_Page_Concat,7},
	{"HPDF_Page_MoveTo",	do_HPDF_Page_MoveTo,3},
	{"HPDF_Page_LineTo",	do_HPDF_Page_LineTo,3},
	{"HPDF_Page_CurveTo",	do_HPDF_Page_CurveTo,7},
	{"HPDF_Page_CurveTo2",	do_HPDF_Page_CurveTo2,5},
	{"HPDF_Page_CurveTo3",	do_HPDF_Page_CurveTo3,5},
	{"HPDF_Page_ClosePath",	do_HPDF_Page_ClosePath,1},
	{"HPDF_Page_Rectangle",	do_HPDF_Page_Rectangle,5},
	{"HPDF_Page_Stroke",	do_HPDF_Page_Stroke,1},
	{"HPDF_Page_ClosePathStroke",	do_HPDF_Page_ClosePathStroke,1},
	{"HPDF_Page_Fill",	do_HPDF_Page_Fill,1},
	{"HPDF_Page_Eofill",	do_HPDF_Page_Eofill,1},
	{"HPDF_Page_FillStroke",	do_HPDF_Page_FillStroke,1},
	{"HPDF_Page_EofillStroke",	do_HPDF_Page_EofillStroke,1},
	{"HPDF_Page_ClosePathFillStroke",	do_HPDF_Page_ClosePathFillStroke,1},
	{"HPDF_Page_ClosePathEofillStroke",	do_HPDF_Page_ClosePathEofillStroke,1},
	{"HPDF_Page_EndPath",	do_HPDF_Page_EndPath,1},
	{"HPDF_Page_Clip",	do_HPDF_Page_Clip,1},
	{"HPDF_Page_Eoclip",	do_HPDF_Page_Eoclip,1},
	{"HPDF_Page_BeginText",	do_HPDF_Page_BeginText,1},
	{"HPDF_Page_EndText",	do_HPDF_Page_EndText,1},
	{"HPDF_Page_SetCharSpace",	do_HPDF_Page_SetCharSpace,2},
	{"HPDF_Page_SetWordSpace",	do_HPDF_Page_SetWordSpace,2},
	{"HPDF_Page_SetHorizontalScalling",	do_HPDF_Page_SetHorizontalScalling,2},
	{"HPDF_Page_SetTextLeading",	do_HPDF_Page_SetTextLeading,2},
	{"HPDF_Page_SetFontAndSize",	do_HPDF_Page_SetFontAndSize,3},
	{"HPDF_Page_SetTextRenderingMode",	do_HPDF_Page_SetTextRenderingMode,2},
	{"HPDF_Page_SetTextRise",	do_HPDF_Page_SetTextRise,2},
	{"HPDF_Page_SetTextRaise",	do_HPDF_Page_SetTextRaise,2},
	{"HPDF_Page_MoveTextPos",	do_HPDF_Page_MoveTextPos,3},
	{"HPDF_Page_MoveTextPos2",	do_HPDF_Page_MoveTextPos2,3},
	{"HPDF_Page_SetTextMatrix",	do_HPDF_Page_SetTextMatrix,7},
	{"HPDF_Page_MoveToNextLine",	do_HPDF_Page_MoveToNextLine,1},
	{"HPDF_Page_ShowText",	do_HPDF_Page_ShowText,2},
	{"HPDF_Page_ShowTextNextLine",	do_HPDF_Page_ShowTextNextLine,2},
	{"HPDF_Page_ShowTextNextLineEx",	do_HPDF_Page_ShowTextNextLineEx,4},
	{"HPDF_Page_SetGrayFill",	do_HPDF_Page_SetGrayFill,2},
	{"HPDF_Page_SetGrayStroke",	do_HPDF_Page_SetGrayStroke,2},
	{"HPDF_Page_SetRGBFill",	do_HPDF_Page_SetRGBFill,4},
	{"HPDF_Page_SetRGBStroke",	do_HPDF_Page_SetRGBStroke,4},
	{"HPDF_Page_SetCMYKFill",	do_HPDF_Page_SetCMYKFill,5},
	{"HPDF_Page_SetCMYKStroke",	do_HPDF_Page_SetCMYKStroke,5},
	{"HPDF_Page_ExecuteXObject",	do_HPDF_Page_ExecuteXObject,2},
	{"HPDF_Page_DrawImage",	do_HPDF_Page_DrawImage,6},
	{"HPDF_Page_Circle",	do_HPDF_Page_Circle,4},
	{"HPDF_Page_Ellipse",	do_HPDF_Page_Ellipse,5},
	{"HPDF_Page_Arc",	do_HPDF_Page_Arc,6},
	{"HPDF_Page_TextOut",	do_HPDF_Page_TextOut,4},
	{"HPDF_Page_TextRect",	do_HPDF_Page_TextRect,8},
	{"HPDF_Page_SetSlideShow",	do_HPDF_Page_SetSlideShow,4},
};

static awk_bool_t
init_my_module(void)
{
	load_vars();

	/* strhash handle create */
	pdfdatas = strhash_create(0);
	pdfpages = strhash_create(0);
	pdfimages = strhash_create(0);
	pdffonts = strhash_create(0);
	pdfoutlines = strhash_create(0);
	pdfencoders = strhash_create(0);
	pdfdestinations = strhash_create(0);
	pdfannotations = strhash_create(0);
	pdfextgstates = strhash_create(0);
	pdfontdefs = strhash_create(0);

	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char ext_version[] = "PDF extension: version 1.0";

dl_load_func(func_table, pdf, "")
