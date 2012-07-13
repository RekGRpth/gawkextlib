/*
 * mpfr.c - Builtin functions that provide MPFR functions.
 *
 * Juergen Kahrs
 * Juergen.Kahrs@users.sourceforge.net
 * 1/2006
 * Revised 1/2006
 */

/*
 * Copyright (C) 2001, 2004 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "common.h"
#include <gmp.h>
#include <mpfr.h>
#include <stdio.h>
#include <locale.h>

/* Some variables contain global defaults for use with MPFR. */
/* The following can be set by the user. */
static awk_scalar_t MPFR_ROUND_node;
static awk_scalar_t MPFR_PRECISION_node;
static awk_scalar_t MPFR_EXACT_node;
static awk_scalar_t MPFR_BASE_node;

#define DEFAULT_MPFR_ROUND     GMP_RNDN
#define DEFAULT_MPFR_PRECISION       53
#define DEFAULT_MPFR_EXACT            0
#define DEFAULT_MPFR_BASE            10

struct varinit {
	awk_scalar_t *spec;
	const char *name;
	int dfltval;
};

#define ENTRY(VAR) { &VAR##_node, #VAR, DEFAULT_##VAR },

/* These are all the scalar variables set by xml getline: */
static const struct varinit varinit[] = {
	ENTRY(MPFR_ROUND)
	ENTRY(MPFR_PRECISION)
	ENTRY(MPFR_EXACT)
	ENTRY(MPFR_BASE)
};

#define NUM_SCALARS     (sizeof(varinit)/sizeof(varinit[0]))
#define NUM_RESET       (NUM_SCALARS-1)


static void
load_vars(void)
{
        const struct varinit *vp;
        size_t i;

        /* This initializes the variables to their default values, overriding
	   any previous value set by the user. */
        for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++) {
		awk_value_t val;

		if (!gawk_varinit_scalar(vp->name,
					 make_number(vp->dfltval, &val), 1,
					 vp->spec))
			fatal(ext_id, _("MPFR reserved scalar variable `%s' already used with incompatible type."), vp->name);
        }
}

size_t
mpfr_out_string (char *outstr, int base, size_t n_digits, mpfr_srcptr op, mp_rnd_t rnd_mode)
{
	char *instr, *instr0;
	size_t len;
	mp_exp_t expo;

	if (outstr == NULL)
		return 0;

	instr = mpfr_get_str (NULL, &expo, base, n_digits, op, rnd_mode);
	instr0 = instr;
	len = strlen (instr) + 1;
	if (*instr == '-')
		* outstr ++ = *instr++;

	if (strcmp(instr, "@NaN@") == 0 || strcmp(instr, "@Inf@") == 0)
	{
		instr++;
		* outstr ++ = *instr++;
		* outstr ++ = *instr++;
		* outstr ++ = *instr++;
		mpfr_free_str(instr0);
		return len-3;
	}

	/* Copy leading digit of mantissa into result. */
	* outstr ++ = *instr++;
	expo--; /* leading digit */

	/* There seems to be a bug with the decimal point recognition
	 * in the old MPFR version that comes with GMP 4.1.4.
	 * With my locale (de_DE.UTF-8), conversion sets any fractional
	 * part of a number to 0. This problem disappears after installing
	 * GMP (without its internal MPFR) and then installing MPFR 2.2.0
	 * in a separate run.
	 */

        /* Insert a decimal point with the proper locale.  */
	* outstr ++ = localeconv()->decimal_point[0];
	while (*instr)
		* outstr ++ = *instr++;

	mpfr_free_str(instr0);

	/* Copy exponent into result. */
	if (expo)
		len += sprintf (outstr, (base <= 10 ? "E%ld" : "@%ld"), (long) expo);
	return len;
}

typedef int (*   unop_t)   (mpfr_ptr, mpfr_srcptr,              mp_rnd_t);
typedef int (*  binop_t)   (mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mp_rnd_t);
typedef int (*constop_t)   (mpfr_t,                             mp_rnd_t);
typedef int (*  binpred_t) (mpfr_srcptr, mpfr_srcptr);
typedef int (*   unpred_t) (mpfr_srcptr);
typedef int (*constpred_t) (void);

static inline double
get_number(awk_scalar_t cookie)
{
	awk_value_t val;
	/* on error, return 0 */
	return sym_lookup_scalar(cookie, AWK_NUMBER, &val) ? val.num_value : 0;
}

#define NUMVAL(WHAT) get_number(WHAT##_node)

static mp_rnd_t
mpfr_get_round (char * round)
{
	if (strcmp(round, mpfr_print_rnd_mode(GMP_RNDN)) == 0)
		return GMP_RNDN;
	if (strcmp(round, mpfr_print_rnd_mode(GMP_RNDZ)) == 0)
		return GMP_RNDZ;
	if (strcmp(round, mpfr_print_rnd_mode(GMP_RNDU)) == 0)
		return GMP_RNDU;
	if (strcmp(round, mpfr_print_rnd_mode(GMP_RNDD)) == 0)
		return GMP_RNDD;
	return NUMVAL(MPFR_ROUND);
}

/* update MPFR_EXACT value */
static inline void
set_exact(double x)
{
	awk_value_t val;
	sym_update_scalar(MPFR_EXACT_node, make_number(x, &val));
}

static awk_value_t *
mpfr_ordinary_op (int argc, awk_value_t *result,
		  int arity, int is_predicate, void * ordinary_op)
{
	mpfr_t number_mpfr[10];
	char * result_func;
	int    result_pred;
	size_t len;
	int i, base, precision;
	mp_rnd_t round;

	base      = NUMVAL(MPFR_BASE);
	round     = NUMVAL(MPFR_ROUND);
	precision = NUMVAL(MPFR_PRECISION);

	if (argc < arity)
		fatal(ext_id, _("too few arguments to MPFR function"));
	if (argc > 5)
		fatal(ext_id, _("too many arguments to MPFR function"));

	/* First optional argument is rounding mode. */
	if (argc > arity) {
		awk_value_t val;
		if (!get_argument(arity+1, AWK_STRING, &val))
			fatal(ext_id, _("optional round argument must be a string"));
		round = mpfr_get_round(val.str_value.str);
	}

	mpfr_set_default_prec(precision);

	/* Second optional argument is precision mode. */
	if (argc > arity+1) {
		awk_value_t val;
		if (!get_argument(arity+2, AWK_NUMBER, &val))
			fatal(ext_id, _("optional precision argument must be numeric"));
		precision = val.num_value;
	}

	for (i=0; i < arity; i++)
	{
		awk_value_t val;
		if (!get_argument(i, AWK_STRING, &val))
			fatal(ext_id, _("missing required argument"));
		mpfr_init2(number_mpfr[i], precision);
		mpfr_set_str(number_mpfr[i], val.str_value.str, base, round);
	}

	switch (arity)
	{
		case 0:
			mpfr_init_set_str(number_mpfr[0], "0", base, round);
			if (is_predicate)
			{
				result_pred = ((constpred_t) ordinary_op) ();
			} else {
				set_exact(((constop_t) ordinary_op) (number_mpfr[0], round));
			}
			break;
		case 1:
			if (is_predicate)
			{
				result_pred = ((unpred_t   ) ordinary_op) (number_mpfr[0]);
			} else {
				set_exact(((unop_t   ) ordinary_op) (number_mpfr[0], number_mpfr[0], round));
			}
			break;
		case 2:
			if (is_predicate)
			{
				result_pred = ((binpred_t  ) ordinary_op) (number_mpfr[0], number_mpfr[1]);
			} else {
				set_exact(((binop_t  ) ordinary_op) (number_mpfr[0], number_mpfr[0], number_mpfr[1], round));
			}
			break;
	}

	if (is_predicate)
	{
		make_number(result_pred, result);
	} else {
 		result_func = malloc(10*(int) NUMVAL(MPFR_PRECISION));
		len = mpfr_out_string(result_func, base, 0, number_mpfr[0], round);
		make_string_malloc(result_func, len, result);
		free(result_func);
	}

	for (i=0; i < arity; i++)
		mpfr_clear(number_mpfr[i]);

	return result;
}

static awk_value_t *
do_mpfr_add(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_add);
}

static awk_value_t *
do_mpfr_sub(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_sub);
}

static awk_value_t *
do_mpfr_mul(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_mul);
}

static awk_value_t *
do_mpfr_div(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_div);
}

static awk_value_t *
do_mpfr_pow(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_pow);
}

static awk_value_t *
do_mpfr_sqr(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_sqr);
}

static awk_value_t *
do_mpfr_sqrt(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_sqrt);
}

static awk_value_t *
do_mpfr_neg(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_neg);
}

static awk_value_t *
do_mpfr_abs(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_abs);
}

static awk_value_t *
do_mpfr_log(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_log);
}

static awk_value_t *
do_mpfr_log2(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_log2);
}

static awk_value_t *
do_mpfr_log10(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_log10);
}

static awk_value_t *
do_mpfr_exp(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_exp);
}

static awk_value_t *
do_mpfr_exp2(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_exp2);
}

static awk_value_t *
do_mpfr_exp10(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_exp10);
}

static awk_value_t *
do_mpfr_sin(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_sin);
}

static awk_value_t *
do_mpfr_cos(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_cos);
}

static awk_value_t *
do_mpfr_tan(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_tan);
}

static awk_value_t *
do_mpfr_asin(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_asin);
}

static awk_value_t *
do_mpfr_acos(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_acos);
}

static awk_value_t *
do_mpfr_atan(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_atan);
}

#if (defined(MPFR_VERSION) && (MPFR_VERSION >= MPFR_VERSION_NUM(2,2,0)))
static awk_value_t *
do_mpfr_atan2(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_atan2);
}

static awk_value_t *
do_mpfr_erfc(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_erfc);
}

static awk_value_t *
do_mpfr_lngamma(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_lngamma);
}

static awk_value_t *
do_mpfr_eint(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_eint);
}

#endif

static awk_value_t *
do_mpfr_const_log2(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 0, mpfr_const_log2);
}

static awk_value_t *
do_mpfr_gamma(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_gamma);
}

static awk_value_t *
do_mpfr_erf(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_erf);
}

static awk_value_t *
do_mpfr_hypot(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_hypot);
}

static awk_value_t *
do_mpfr_const_pi(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 0, mpfr_const_pi);
}

static awk_value_t *
do_mpfr_const_euler(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 0, mpfr_const_euler);
}

static awk_value_t *
do_mpfr_rint(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_rint);
}

static awk_value_t *
do_mpfr_ceil(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_ceil);
}

static awk_value_t *
do_mpfr_floor(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_floor);
}

static awk_value_t *
do_mpfr_round(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_round);
}

static awk_value_t *
do_mpfr_trunc(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_trunc);
}

static awk_value_t *
do_mpfr_frac(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 0, mpfr_frac);
}

static awk_value_t *
convert_base(int nargs, awk_value_t *resval, int to_internal_base)
{
	awk_value_t number_node, base_node;
	mpfr_t val;
	char * result;
	size_t len;
	int from_base, to_base;

	if (do_lint && nargs != 2)
		lintwarn(ext_id, "convert_base: called with incorrect number of arguments");

	mpfr_set_default_prec((int) NUMVAL(MPFR_PRECISION));

	if (!get_argument(0, AWK_STRING, &number_node))
		fatal(ext_id, _("first argument must be a string"));
	if (!get_argument(1, AWK_NUMBER, &base_node))
		fatal(ext_id, _("second argument must be a number"));

	if (to_internal_base)
	{
		from_base = base_node.num_value;
		to_base   = NUMVAL(MPFR_BASE);
	} else {
		from_base = NUMVAL(MPFR_BASE);
		to_base   = base_node.num_value;
	}

	mpfr_init_set_str(val, number_node.str_value.str, from_base, (int) NUMVAL(MPFR_ROUND));

	/* Set the return value */
	result = malloc(10*(int) NUMVAL(MPFR_PRECISION));
	len = mpfr_out_string(result, to_base, 0, val, (int) NUMVAL(MPFR_ROUND));
	make_string_malloc(result, len, resval);
	free(result);
	mpfr_clear(val);
	return resval;
}

static awk_value_t *
do_mpfr_out_str(int nargs, awk_value_t *result)
{
	return convert_base(nargs, result, 0);
}

static awk_value_t *
do_mpfr_inp_str(int nargs, awk_value_t *result)
{
	return convert_base(nargs, result, 1);
}

static awk_value_t *
do_mpfr_min(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_min);
}

static awk_value_t *
do_mpfr_max(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 0, mpfr_max);
}

static awk_value_t *
do_mpfr_cmp(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_cmp);
}

static awk_value_t *
do_mpfr_cmpabs(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_cmpabs);
}

static awk_value_t *
do_mpfr_nan_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 1, mpfr_nan_p);
}

static awk_value_t *
do_mpfr_inf_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 1, mpfr_inf_p);
}

static awk_value_t *
do_mpfr_number_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 1, mpfr_number_p);
}

static awk_value_t *
do_mpfr_zero_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 1, mpfr_zero_p);
}

static awk_value_t *
do_mpfr_sgn(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 1, 1, mpfr_sgn);
}

static awk_value_t *
do_mpfr_greater_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_greater_p);
}

static awk_value_t *
do_mpfr_greaterequal_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_greaterequal_p);
}

static awk_value_t *
do_mpfr_less_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_less_p);
}

static awk_value_t *
do_mpfr_lessequal_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_lessequal_p);
}

static awk_value_t *
do_mpfr_lessgreater_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_lessgreater_p);
}

static awk_value_t *
do_mpfr_equal_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_equal_p);
}

static awk_value_t *
do_mpfr_unordered_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 2, 1, mpfr_unordered_p);
}

static awk_value_t *
do_mpfr_underflow_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 1, mpfr_underflow_p);
}

static awk_value_t *
do_mpfr_overflow_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 1, mpfr_overflow_p);
}

static awk_value_t *
do_mpfr_nanflag_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 1, mpfr_nanflag_p);
}

static awk_value_t *
do_mpfr_inexflag_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 1, mpfr_inexflag_p);
}

static awk_value_t *
do_mpfr_erangeflag_p(int nargs, awk_value_t *result)
{
	return mpfr_ordinary_op(nargs, result, 0, 1, mpfr_erangeflag_p);
}

static awk_ext_func_t func_table[] = {
	{ "mpfr_add", do_mpfr_add, 2},
	{ "mpfr_sub", do_mpfr_sub, 2},
	{ "mpfr_mul", do_mpfr_mul, 2},
	{ "mpfr_div", do_mpfr_div, 2},
	{ "mpfr_pow", do_mpfr_pow, 2},
	{ "mpfr_sqr", do_mpfr_sqr, 1},
	{ "mpfr_sqrt", do_mpfr_sqrt, 1},
	{ "mpfr_neg", do_mpfr_neg, 1},
	{ "mpfr_abs", do_mpfr_abs, 1},
	{ "mpfr_log", do_mpfr_log, 1},
	{ "mpfr_log2", do_mpfr_log2, 1},
	{ "mpfr_log10", do_mpfr_log10, 1},
	{ "mpfr_exp", do_mpfr_exp, 1},
	{ "mpfr_exp2", do_mpfr_exp2, 1},
	{ "mpfr_exp10", do_mpfr_exp10, 1},
	{ "mpfr_sin", do_mpfr_sin, 1},
	{ "mpfr_cos", do_mpfr_cos, 1},
	{ "mpfr_tan", do_mpfr_tan, 1},
	{ "mpfr_acos", do_mpfr_acos, 1},
	{ "mpfr_asin", do_mpfr_asin, 1},
	{ "mpfr_atan", do_mpfr_atan, 1},
#if (defined(MPFR_VERSION) && (MPFR_VERSION >= MPFR_VERSION_NUM(2,2,0)))
	{ "mpfr_atan2", do_mpfr_atan2, 2},
	{ "mpfr_eint", do_mpfr_eint, 2},
	{ "mpfr_lngamma", do_mpfr_lngamma, 1},
	{ "mpfr_erfc", do_mpfr_erfc, 1},
#endif
	{ "mpfr_gamma", do_mpfr_gamma, 1},
	{ "mpfr_erf", do_mpfr_erf, 1},
	{ "mpfr_hypot", do_mpfr_hypot, 2},
	{ "mpfr_const_log2", do_mpfr_const_log2, 0},
	{ "mpfr_const_pi", do_mpfr_const_pi, 0},
	{ "mpfr_const_euler", do_mpfr_const_euler, 0},
	{ "mpfr_rint", do_mpfr_rint, 1},
	{ "mpfr_ceil", do_mpfr_ceil, 1},
	{ "mpfr_floor", do_mpfr_floor, 1},
	{ "mpfr_round", do_mpfr_round, 1},
	{ "mpfr_trunc", do_mpfr_trunc, 1},
	{ "mpfr_frac", do_mpfr_frac, 1},
	{ "mpfr_inp_str", do_mpfr_inp_str, 2},
	{ "mpfr_out_str", do_mpfr_out_str, 2},
	{ "mpfr_min", do_mpfr_min, 2},
	{ "mpfr_max", do_mpfr_max, 2},

	{ "mpfr_cmp",    do_mpfr_cmp, 2},
	{ "mpfr_cmpabs", do_mpfr_cmpabs, 2},
	{ "mpfr_nan_p", do_mpfr_nan_p, 1},
	{ "mpfr_inf_p", do_mpfr_inf_p, 1},
	{ "mpfr_number_p", do_mpfr_number_p, 1},
	{ "mpfr_zero_p", do_mpfr_zero_p, 1},
	{ "mpfr_sgn", do_mpfr_sgn, 1},
	{ "mpfr_greater_p", do_mpfr_greater_p, 2},
	{ "mpfr_greaterequal_p", do_mpfr_greaterequal_p, 2},
	{ "mpfr_less_p", do_mpfr_less_p, 2},
	{ "mpfr_lessequal_p", do_mpfr_lessequal_p, 2},
	{ "mpfr_lessgreater_p", do_mpfr_lessgreater_p, 2},
	{ "mpfr_equal_p", do_mpfr_equal_p, 2},
	{ "mpfr_unordered_p", do_mpfr_unordered_p, 2},
	{ "mpfr_underflow_p", do_mpfr_underflow_p, 0},
	{ "mpfr_overflow_p", do_mpfr_overflow_p, 0},
	{ "mpfr_nanflag_p", do_mpfr_nanflag_p, 0},
	{ "mpfr_inexflag_p", do_mpfr_inexflag_p, 0},
	{ "mpfr_erangeflag_p", do_mpfr_erangeflag_p, 0},
};

static awk_bool_t
init_my_module(void)
{
	load_vars();
	mpfr_set_default_prec((int) NUMVAL(MPFR_PRECISION));
	return 1;
}

static awk_bool_t (*init_func)(void) = init_my_module;

dl_load_func(func_table, mpfr, "")
