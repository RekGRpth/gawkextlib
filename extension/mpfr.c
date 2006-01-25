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
#include <gmp.h>
#include <mpfr.h>

/* Some variables contain global defaults for use with MPFR. */
/* The following can be set by the user. */
static NODE *MPFR_ROUND_node;
static NODE *MPFR_PRECISION_node;
static NODE *MPFR_EXACT_node;
static NODE *MPFR_BASE_node;

#define DEFAULT_MPFR_ROUND     GMP_RNDN
#define DEFAULT_MPFR_PRECISION 53
#define DEFAULT_MPFR_BASE      10

struct varinit {
	NODE **spec;
	const char *name;
};

#define ENTRY(VAR) { &VAR##_node, #VAR },

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
xml_load_vars()
{
        const struct varinit *vp;
        size_t i;

        /* This initializes most of the variables, including XMLCHARSET,
         * to a value of "". 
         */
        for (vp = varinit, i = 0; i < NUM_SCALARS; i++, vp++) {
                if ((*vp->spec = lookup(vp->name)) != NULL) {
#define N (*vp->spec)
                        /* The name is already in use.  Check the type. */
                        if (N->type == Node_var_new) {
                                N->type = Node_var;
                                N->var_value = Nnull_string;
                        }
                        else if (N->type != Node_var)
                                fatal(_("MPFR reserved scalar variable `%s' already used with incompatible type."), vp->name);
#undef N
                }
                else
                        *vp->spec = install((char *)vp->name,
                                            node(Nnull_string, Node_var, NULL));
        }

	MPFR_ROUND_node->type = Node_var;
	MPFR_ROUND_node->var_value = make_number(DEFAULT_MPFR_ROUND);
	MPFR_PRECISION_node->type = Node_var;
	MPFR_PRECISION_node->var_value = make_number(DEFAULT_MPFR_PRECISION);
	MPFR_BASE_node->type = Node_var;
	MPFR_BASE_node->var_value = make_number(DEFAULT_MPFR_BASE);
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
		(*__gmp_free_func) (instr0, len);
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

	(*__gmp_free_func) (instr0, len);

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
	return (int) force_number(MPFR_ROUND_node->var_value);
}

static NODE *
mpfr_ordinary_op (NODE * tree, int arity, int is_predicate, void * ordinary_op)
{
	NODE * number_awk[10];
	mpfr_t number_mpfr[10];
	char * result_func;
	int    result_pred;
	size_t len;
	int i, base, argc, precision;
	mp_rnd_t round;

	base      = (int) force_number(MPFR_BASE_node->var_value);
	round     = (int) force_number(MPFR_ROUND_node->var_value);
	precision = (int) force_number(MPFR_PRECISION_node->var_value);

	argc = get_curfunc_arg_count();
	if (argc < arity)
		fatal(_("too few arguments to MPFR function"));
	if (argc > 5)
		fatal(_("too many arguments to MPFR function"));

	/* First optional argument is rounding mode. */
	if (argc > arity)
		round = mpfr_get_round((get_array_argument(tree, arity+1, FALSE))->stptr);

	mpfr_set_default_prec(precision);

	/* Second optional argument is precision mode. */
	if (argc > arity+1)
		precision = (int) force_number((get_array_argument(tree, arity+2, FALSE))->stptr);

	for (i=0; i < arity; i++)
	{
		number_awk[i] = get_scalar_argument(tree, i, FALSE);
		(void) force_string(number_awk[i]);
		mpfr_init2(number_mpfr[i], precision);
		mpfr_set_str(number_mpfr[i], number_awk[i]->stptr, base, round);
	}

	switch (arity)
	{
		case 0:
			mpfr_init_set_str(number_mpfr[0], "0", base, round);
			if (is_predicate)
				result_pred = ((constpred_t) ordinary_op) ();
			else
				((constop_t) ordinary_op) (number_mpfr[0], round);
			break;
		case 1:
			if (is_predicate)
				result_pred = ((unpred_t   ) ordinary_op) (number_mpfr[0]);
			else
				((unop_t   ) ordinary_op) (number_mpfr[0], number_mpfr[0], round);
			break;
		case 2:
			if (is_predicate)
				result_pred = ((binpred_t  ) ordinary_op) (number_mpfr[0], number_mpfr[1]);
			else
				((binop_t  ) ordinary_op) (number_mpfr[0], number_mpfr[0], number_mpfr[1], round);
			break;
	}

	if (is_predicate)
	{
		set_value(tmp_number(result_pred));
	} else {
 		result_func = malloc(10*(int) force_number(MPFR_PRECISION_node->var_value));
		len = mpfr_out_string(result_func, base, 0, number_mpfr[0], round);
		set_value(tmp_string(result_func, len));
		free(result_func);
	}

	for (i=0; i < arity; i++)
		mpfr_clear(number_mpfr[i]);

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

static NODE *
do_mpfr_add(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_add);
}

static NODE *
do_mpfr_sub(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_sub);
}

static NODE *
do_mpfr_mul(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_mul);
}

static NODE *
do_mpfr_div(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_div);
}

static NODE *
do_mpfr_pow(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_pow);
}

static NODE *
do_mpfr_sqr(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_sqr);
}

static NODE *
do_mpfr_sqrt(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_sqrt);
}

static NODE *
do_mpfr_neg(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_neg);
}

static NODE *
do_mpfr_abs(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_set4);
}

static NODE *
do_mpfr_log(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_log);
}

static NODE *
do_mpfr_log2(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_log2);
}

static NODE *
do_mpfr_log10(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_log10);
}

static NODE *
do_mpfr_exp(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_exp);
}

static NODE *
do_mpfr_exp2(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_exp2);
}

static NODE *
do_mpfr_exp10(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_exp10);
}

static NODE *
do_mpfr_sin(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_sin);
}

static NODE *
do_mpfr_cos(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_cos);
}

static NODE *
do_mpfr_tan(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_tan);
}

static NODE *
do_mpfr_asin(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_asin);
}

static NODE *
do_mpfr_acos(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_acos);
}

static NODE *
do_mpfr_atan(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_atan);
}

static NODE *
do_mpfr_atan2(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_atan2);
}

static NODE *
do_mpfr_const_log2(NODE * tree)
{
	mpfr_ordinary_op(tree, 0, 0, mpfr_log2);
}

static NODE *
do_mpfr_erf(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_erf);
}

static NODE *
do_mpfr_erfc(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_erfc);
}

static NODE *
do_mpfr_hypot(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_hypot);
}

static NODE *
do_mpfr_const_pi(NODE * tree)
{
	mpfr_ordinary_op(tree, 0, 0, mpfr_const_pi);
}

static NODE *
do_mpfr_const_euler(NODE * tree)
{
	mpfr_ordinary_op(tree, 0, 0, mpfr_const_euler);
}

static NODE *
do_mpfr_rint(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_rint);
}

static NODE *
do_mpfr_ceil(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_ceil);
}

static NODE *
do_mpfr_floor(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_floor);
}

static NODE *
do_mpfr_round(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_round);
}

static NODE *
do_mpfr_trunc(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_trunc);
}

static NODE *
do_mpfr_frac(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 0, mpfr_frac);
}

static NODE *
convert_base(NODE * tree, int to_internal_base)
{
	NODE *number_node, *base_node;
	mpfr_t val;
	char * result;
	size_t len;
	int from_base, to_base;

	if (do_lint && get_curfunc_arg_count() != 2)
		lintwarn("do_mpfr_out_str: called with incorrect number of arguments");

	mpfr_set_default_prec((int) force_number(MPFR_PRECISION_node->var_value));

	number_node = get_scalar_argument(tree, 0, FALSE);
	base_node   = get_scalar_argument(tree, 1, FALSE);
	(void) force_string(number_node);
	(void) force_number(base_node);

	if (to_internal_base)
	{
		from_base = (int) force_number(base_node);
		to_base   = (int) force_number(MPFR_BASE_node->var_value);
	} else {
		from_base = (int) force_number(MPFR_BASE_node->var_value);
		to_base   = (int) force_number(base_node);
	}

	mpfr_init_set_str(val, number_node->stptr, from_base, (int) force_number(MPFR_ROUND_node->var_value));

	/* Set the return value */
	result = malloc(10*(int) force_number(MPFR_PRECISION_node->var_value));
	len = mpfr_out_string(result, to_base, 0, val, (int) force_number(MPFR_ROUND_node->var_value));
	set_value(tmp_string(result, len));
	free(result);
	mpfr_clear(val);

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

static NODE *
do_mpfr_out_str(NODE * tree)
{
	convert_base(tree, 0);

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

static NODE *
do_mpfr_inp_str(NODE * tree)
{
	convert_base(tree, 1);

	/* Just to make the interpreter happy */
	return tmp_number((AWKNUM) 0);
}

static NODE *
do_mpfr_min(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_min);
}

static NODE *
do_mpfr_max(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 0, mpfr_max);
}

static NODE *
do_mpfr_cmp(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_cmp);
}

static NODE *
do_mpfr_cmpabs(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_cmpabs);
}

static NODE *
do_mpfr_nan_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 1, mpfr_nan_p);
}

static NODE *
do_mpfr_inf_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 1, mpfr_inf_p);
}

static NODE *
do_mpfr_number_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 1, mpfr_number_p);
}

static NODE *
do_mpfr_zero_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 1, mpfr_zero_p);
}

static NODE *
do_mpfr_sgn(NODE * tree)
{
	mpfr_ordinary_op(tree, 1, 1, mpfr_sgn);
}

static NODE *
do_mpfr_greater_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_greater_p);
}

static NODE *
do_mpfr_greaterequal_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_greaterequal_p);
}

static NODE *
do_mpfr_less_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_less_p);
}

static NODE *
do_mpfr_lessequal_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_lessequal_p);
}

static NODE *
do_mpfr_lessgreater_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_lessgreater_p);
}

static NODE *
do_mpfr_equal_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_equal_p);
}

static NODE *
do_mpfr_unordered_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_unordered_p);
}

static NODE *
do_mpfr_underflow_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_underflow_p);
}

static NODE *
do_mpfr_overflow_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_overflow_p);
}

static NODE *
do_mpfr_nanflag_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_nanflag_p);
}

static NODE *
do_mpfr_inexflag_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_inexflag_p);
}

static NODE *
do_mpfr_erangeflag_p(NODE * tree)
{
	mpfr_ordinary_op(tree, 2, 1, mpfr_erangeflag_p);
}


/* dlload --- load new builtins in this library */

#ifdef BUILD_STATIC_EXTENSIONS
#define dlload dlload_mpfr
#endif

NODE *
dlload(tree, dl)
NODE *tree;
void *dl;
{
	xml_load_vars();

	mpfr_set_default_prec((int) force_number(MPFR_PRECISION_node->var_value));
	make_builtin("mpfr_add", do_mpfr_add, 2);
	make_builtin("mpfr_sub", do_mpfr_sub, 2);
	make_builtin("mpfr_mul", do_mpfr_mul, 2);
	make_builtin("mpfr_div", do_mpfr_div, 2);
	make_builtin("mpfr_pow", do_mpfr_pow, 2);
	make_builtin("mpfr_sqr", do_mpfr_sqr, 1);
	make_builtin("mpfr_sqrt", do_mpfr_sqrt, 1);
	make_builtin("mpfr_neg", do_mpfr_neg, 1);
	make_builtin("mpfr_abs", do_mpfr_abs, 1);
	make_builtin("mpfr_log", do_mpfr_log, 1);
	make_builtin("mpfr_log2", do_mpfr_log2, 1);
	make_builtin("mpfr_log10", do_mpfr_log10, 1);
	make_builtin("mpfr_exp", do_mpfr_exp, 1);
	make_builtin("mpfr_exp2", do_mpfr_exp2, 1);
	make_builtin("mpfr_exp10", do_mpfr_exp10, 1);
	make_builtin("mpfr_sin", do_mpfr_sin, 1);
	make_builtin("mpfr_cos", do_mpfr_cos, 1);
	make_builtin("mpfr_tan", do_mpfr_tan, 1);
	make_builtin("mpfr_acos", do_mpfr_acos, 1);
	make_builtin("mpfr_asin", do_mpfr_asin, 1);
	make_builtin("mpfr_atan", do_mpfr_atan, 1);
	make_builtin("mpfr_atan2", do_mpfr_atan2, 2);
	make_builtin("mpfr_erf", do_mpfr_erf, 2);
	make_builtin("mpfr_erfc", do_mpfr_erfc, 2);
	make_builtin("mpfr_hypot", do_mpfr_hypot, 2);
	make_builtin("mpfr_const_log2", do_mpfr_const_log2, 0);
	make_builtin("mpfr_const_pi", do_mpfr_const_pi, 0);
	make_builtin("mpfr_const_euler", do_mpfr_const_euler, 0);
	make_builtin("mpfr_rint", do_mpfr_rint, 1);
	make_builtin("mpfr_ceil", do_mpfr_ceil, 1);
	make_builtin("mpfr_floor", do_mpfr_floor, 1);
	make_builtin("mpfr_round", do_mpfr_round, 1);
	make_builtin("mpfr_trunc", do_mpfr_trunc, 1);
	make_builtin("mpfr_frac", do_mpfr_frac, 1);
	make_builtin("mpfr_inp_str", do_mpfr_inp_str, 2);
	make_builtin("mpfr_out_str", do_mpfr_out_str, 2);
	make_builtin("mpfr_min", do_mpfr_min, 2);
	make_builtin("mpfr_max", do_mpfr_max, 2);

	make_builtin("mpfr_cmp",    do_mpfr_cmp, 2);
	make_builtin("mpfr_cmpabs", do_mpfr_cmpabs, 2);
	make_builtin("mpfr_nan_p", do_mpfr_nan_p, 1);
	make_builtin("mpfr_inf_p", do_mpfr_inf_p, 1);
	make_builtin("mpfr_number_p", do_mpfr_number_p, 1);
	make_builtin("mpfr_zero_p", do_mpfr_zero_p, 1);
	make_builtin("mpfr_sgn", do_mpfr_sgn, 1);
	make_builtin("mpfr_greater_p", do_mpfr_greater_p, 2);
	make_builtin("mpfr_greaterequal_p", do_mpfr_greaterequal_p, 2);
	make_builtin("mpfr_less_p", do_mpfr_less_p, 2);
	make_builtin("mpfr_lessequal_p", do_mpfr_lessequal_p, 2);
	make_builtin("mpfr_lessgreater_p", do_mpfr_lessgreater_p, 2);
	make_builtin("mpfr_equal_p", do_mpfr_equal_p, 2);
	make_builtin("mpfr_unordered_p", do_mpfr_unordered_p, 2);
	make_builtin("mpfr_underflow_p", do_mpfr_underflow_p, 2);
	make_builtin("mpfr_overflow_p", do_mpfr_overflow_p, 2);
	make_builtin("mpfr_nanflag_p", do_mpfr_nanflag_p, 2);
	make_builtin("mpfr_inexflag_p", do_mpfr_inexflag_p, 2);
	make_builtin("mpfr_erangeflag_p", do_mpfr_erangeflag_p, 2);
	return tmp_number((AWKNUM) 0);
}
