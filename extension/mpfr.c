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

typedef int (*   unop_t) (mpfr_ptr, mpfr_srcptr,              mp_rnd_t);
typedef int (*  binop_t) (mpfr_ptr, mpfr_srcptr, mpfr_srcptr, mp_rnd_t);
typedef int (*comstop_t) (mpfr_t,                             mp_rnd_t);

static NODE *
mpfr_constop(NODE * tree, comstop_t constop)
{
	NODE *n;
	mpfr_t m;
	char * result;
	size_t len;

	if (do_lint && get_curfunc_arg_count() != 0)
		lintwarn("comstop: called with incorrect number of arguments");

	mpfr_set_default_prec((int) force_number(MPFR_PRECISION_node->var_value));

	mpfr_init_set_str(m, "0", (int) force_number(MPFR_BASE_node->var_value), (int) force_number(MPFR_ROUND_node->var_value));
	constop(m, (int) force_number(MPFR_ROUND_node->var_value));

	/* Set the return value */
 	result = malloc(10*(int) force_number(MPFR_PRECISION_node->var_value));
	len = mpfr_out_string(result, (int) force_number(MPFR_BASE_node->var_value), 0, m, (int) force_number(MPFR_ROUND_node->var_value));
	set_value(tmp_string(result, len));
	free(result);
	mpfr_clear(m);

        /* Just to make the interpreter happy */
        return tmp_number((AWKNUM) 0);
}

static NODE *
mpfr_unop(NODE * tree, unop_t unop)
{
	NODE *n;
	mpfr_t m;
	char * result;
	size_t len;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("unop: called with incorrect number of arguments");

	mpfr_set_default_prec((int) force_number(MPFR_PRECISION_node->var_value));

	n = get_scalar_argument(tree, 0, FALSE);
	(void) force_string(n);

	mpfr_init_set_str(m, n->stptr, (int) force_number(MPFR_BASE_node->var_value), (int) force_number(MPFR_ROUND_node->var_value));
	unop(m, m, (int) force_number(MPFR_ROUND_node->var_value));

	/* Set the return value */
 	result = malloc(10*(int) force_number(MPFR_PRECISION_node->var_value));
	len = mpfr_out_string(result, (int) force_number(MPFR_BASE_node->var_value), 0, m, (int) force_number(MPFR_ROUND_node->var_value));
	set_value(tmp_string(result, len));
	free(result);
	mpfr_clear(m);

        /* Just to make the interpreter happy */
        return tmp_number((AWKNUM) 0);
}

static NODE *
mpfr_binop(NODE * tree, binop_t binop)
{
	NODE *n1, *n2;
	mpfr_t m1, m2;
	char * result;
	size_t len;

	if (do_lint && get_curfunc_arg_count() != 2)
		lintwarn("binop: called with incorrect number of arguments");

	mpfr_set_default_prec((int) force_number(MPFR_PRECISION_node->var_value));

	n1 = get_scalar_argument(tree, 0, FALSE);
	n2 = get_scalar_argument(tree, 1, FALSE);
	(void) force_string(n1);
	(void) force_string(n2);

	mpfr_init_set_str(m1, n1->stptr, (int) force_number(MPFR_BASE_node->var_value), (int) force_number(MPFR_ROUND_node->var_value));
	mpfr_init_set_str(m2, n2->stptr, (int) force_number(MPFR_BASE_node->var_value), (int) force_number(MPFR_ROUND_node->var_value));
	binop(m1, m1, m2, (int) force_number(MPFR_ROUND_node->var_value));

	/* Set the return value */
 	result = malloc(10*(int) force_number(MPFR_PRECISION_node->var_value));
	len = mpfr_out_string(result, (int) force_number(MPFR_BASE_node->var_value), 0, m1, (int) force_number(MPFR_ROUND_node->var_value));
	set_value(tmp_string(result, len));
	free(result);
	mpfr_clear(m1);
	mpfr_clear(m2);

        /* Just to make the interpreter happy */
        return tmp_number((AWKNUM) 0);
}

static NODE *
do_mpfr_add(NODE * tree)
{
	mpfr_binop(tree, mpfr_add);
}

static NODE *
do_mpfr_sub(NODE * tree)
{
	mpfr_binop(tree, mpfr_sub);
}


static NODE *
do_mpfr_mul(NODE * tree)
{
	mpfr_binop(tree, mpfr_mul);
}

static NODE *
do_mpfr_div(NODE * tree)
{
	mpfr_binop(tree, mpfr_div);
}

static NODE *
do_mpfr_pow(NODE * tree)
{
	mpfr_binop(tree, mpfr_pow);
}

static NODE *
do_mpfr_sqr(NODE * tree)
{
	mpfr_binop(tree, mpfr_sqr);
}

static NODE *
do_mpfr_sqrt(NODE * tree)
{
	mpfr_unop(tree, mpfr_sqrt);
}

static NODE *
do_mpfr_neg(NODE * tree)
{
	mpfr_unop(tree, mpfr_neg);
}

static NODE *
do_mpfr_abs(NODE * tree)
{
	mpfr_unop(tree, mpfr_set4);
}

static NODE *
do_mpfr_log(NODE * tree)
{
	mpfr_unop(tree, mpfr_log);
}

static NODE *
do_mpfr_log2(NODE * tree)
{
	mpfr_unop(tree, mpfr_log2);
}

static NODE *
do_mpfr_log10(NODE * tree)
{
	mpfr_unop(tree, mpfr_log10);
}

static NODE *
do_mpfr_exp(NODE * tree)
{
	mpfr_unop(tree, mpfr_exp);
}

static NODE *
do_mpfr_exp2(NODE * tree)
{
	mpfr_unop(tree, mpfr_exp2);
}

static NODE *
do_mpfr_exp10(NODE * tree)
{
	mpfr_unop(tree, mpfr_exp10);
}

static NODE *
do_mpfr_sin(NODE * tree)
{
	mpfr_unop(tree, mpfr_sin);
}

static NODE *
do_mpfr_cos(NODE * tree)
{
	mpfr_unop(tree, mpfr_cos);
}

static NODE *
do_mpfr_tan(NODE * tree)
{
	mpfr_unop(tree, mpfr_tan);
}

static NODE *
do_mpfr_asin(NODE * tree)
{
	mpfr_unop(tree, mpfr_asin);
}

static NODE *
do_mpfr_acos(NODE * tree)
{
	mpfr_unop(tree, mpfr_acos);
}

static NODE *
do_mpfr_atan(NODE * tree)
{
	mpfr_unop(tree, mpfr_atan);
}

static NODE *
do_mpfr_const_log2(NODE * tree)
{
	mpfr_constop(tree, mpfr_const_log2);
}

static NODE *
do_mpfr_const_pi(NODE * tree)
{
	mpfr_constop(tree, mpfr_const_pi);
}

static NODE *
do_mpfr_const_euler(NODE * tree)
{
	mpfr_constop(tree, mpfr_const_euler);
}

static NODE *
do_mpfr_rint(NODE * tree)
{
	mpfr_unop(tree, mpfr_rint);
}

static NODE *
do_mpfr_ceil(NODE * tree)
{
	mpfr_constop(tree, mpfr_ceil);
}

static NODE *
do_mpfr_floor(NODE * tree)
{
	mpfr_constop(tree, mpfr_floor);
}

static NODE *
do_mpfr_round(NODE * tree)
{
	mpfr_constop(tree, mpfr_round);
}

static NODE *
do_mpfr_trunc(NODE * tree)
{
	mpfr_constop(tree, mpfr_trunc);
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
/*
	make_builtin("mpfr_atan2", do_mpfr_, 2);
	make_builtin("mpfr_hypot", do_mpfr_, 2);
*/
	make_builtin("mpfr_const_log2", do_mpfr_const_log2, 0);
	make_builtin("mpfr_const_pi", do_mpfr_const_pi, 0);
	make_builtin("mpfr_const_euler", do_mpfr_const_euler, 0);
	make_builtin("mpfr_rint", do_mpfr_rint, 1);
	make_builtin("mpfr_ceil", do_mpfr_ceil, 1);
	make_builtin("mpfr_floor", do_mpfr_floor, 1);
	make_builtin("mpfr_round", do_mpfr_round, 1);
	make_builtin("mpfr_trunc", do_mpfr_trunc, 1);

	return tmp_number((AWKNUM) 0);
}
