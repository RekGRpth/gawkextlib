/*
 * ext.c - Builtin function that links external gawk functions and related
 *	   utilities.
 *
 * Christos Zoulas, Thu Jun 29 17:40:41 EDT 1995
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 */

/*
 * Copyright (C) 1995 - 2001, 2003, 2004 the Free Software Foundation, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "awk.h"

#ifdef DYNAMIC

#include <dlfcn.h>

#ifdef __GNUC__
static unsigned long long dummy;	/* fake out gcc for dynamic loading? */
#endif

extern int errcount;


static NODE *
load_run(const char *libname, const char *funcname, NODE *tree)
{
        static const char *savepath = NULL;
        static int first = TRUE;
	char *path;
	int already_loaded;
	NODE *(*func) P((NODE *, void *));
	void *dl;

#ifdef __GNUC__
	AWKNUM junk;

	junk = (AWKNUM) dummy;
#endif

	if (first) {
		const char *awkpath;
		first = FALSE;
		if ((awkpath = getenv("AWKLIBPATH")) != NULL && *awkpath)
			savepath = awkpath;	/* used for restarting */
		else
			savepath = deflibpath;
	}

	if (!(path = path_find(savepath, libname, FALSE, SHLIBEXT,
			       FILETYPE_LIBRARY, &already_loaded)))
		fatal(_("extension: cannot find dynamic library `%s'\n"),
		      libname);
	if (already_loaded & FILETYPE_LIBRARY) {
		if (do_lint)
			lintwarn(_("extension: dynamic library `%s' [%s] has already been loaded."), libname, path);
		return tmp_number((AWKNUM) 0);
	}

#ifdef RTLD_GLOBAL
#define FLAGS (RTLD_LAZY|RTLD_GLOBAL)
#else
#define FLAGS RTLD_LAZY
#endif
	if (!(dl = dlopen(path, FLAGS)))
		fatal(_("extension: cannot load `%s' [%s]: %s\n"), libname,
		      path, dlerror());
#undef FLAGS

	func = (NODE *(*) P((NODE *, void *))) dlsym(dl, funcname);
	if (func == NULL)
		fatal(_("extension: library `%s' [%s]: cannot call function `%s' (%s)\n"),
				libname, path, funcname, dlerror());

	return (*func)(tree, dl);
}

/* do_ext --- load an extension */

NODE *
do_ext(NODE *tree)
{
	NODE *obj;
	NODE *fun;
	NODE *result;

	if (do_lint)
		lintwarn(_("`extension' is a gawk extension"));

	if (do_traditional || do_posix) {
		errcount++;
		error(_("`extension' is a gawk extension"));
	}

	obj = tree_eval(tree->lnode);
	force_string(obj);

	fun = tree_eval(tree->rnode->lnode);
	force_string(fun);

	result = load_run(obj->stptr, fun->stptr, tree);

	free_temp(obj);
	free_temp(fun);

	return result;
}

void
load_extension(const char *name)
{
	NODE *res;
	AWKNUM rc;

	if (do_lint)
		lintwarn(_("loading dynamic library `%s' is a gawk extension"),
			 name);
	res = load_run(name, "dlload", NULL);
	if ((rc = force_number(res)) != 0)
		warning(_("extension library `%s' dlload routine returned non-zero: %g"), name, rc);
	free_temp(res);
}

/* make_builtin --- register name to be called as func with a builtin body */

void
make_builtin(char *name, NODE *(*func) P((NODE *)), int count)
{
	NODE *p, *b, *f;
	char **vnames, *parm_names, *sp;
	char c, buf[200];
	int space_needed, i;

	sp = name;
	if (sp == NULL || *sp == '\0')
		fatal(_("extension: missing function name")); 

	while ((c = *sp++) != '\0')
		if ((sp == &name[1] && c != '_' && ! ISALPHA(c))
				|| (sp > &name[1] && ! is_identchar(c)))
			fatal(_("extension: illegal character `%c' in function name `%s'"), c, name);

	f = lookup(name);
	if (f != NULL) {
		if (f->type == Node_func) {
			if (f->rnode->type != Node_builtin)			/* user-defined function */
				fatal(_("extension: can't redefine function `%s'"), name);
			else {
				/* multiple extension() calls etc. */ 
				if (do_lint)
					lintwarn(_("extension: function `%s' already defined"), name);
				return;
			}
		} else {
			if (check_special(name) >= 0)
				fatal(_("extension: can't use gawk built-in `%s' as function name"), name); 
			/* variable name etc. */ 
			fatal(_("extension: function name `%s' previously defined"), name);
		}
	}

	/* count parameters, create artificial list of param names */
	space_needed = 0;
	for (i = 0; i < count; i++) {
		sprintf(buf, "p%d", i);
		space_needed += strlen(buf) + 1;
	}
	emalloc(parm_names, char *, space_needed, "make_builtin");
	emalloc(vnames, char **, count * sizeof(char  *), "make_builtin");
	sp = parm_names;
	for (i = 0; i < count; i++) {
		sprintf(sp, "p%d",i);
		vnames[i] = sp;
		sp += strlen(sp) + 1;
	}

	getnode(p);
	p->type = Node_param_list;
	p->flags |= FUNC;
	p->rnode = NULL;
	p->param = name;
	p->param_cnt = count;
#if 0
	/* setting these  blows away the param_cnt. dang unions! */
	p->source_line = __LINE__;
	p->source_file = __FILE__;
#endif

	getnode(b);
	b->type = Node_builtin;
	b->builtin = func;
	b->subnode = p;
	b->source_line = __LINE__;
	b->source_file = __FILE__;

	f = node(p, Node_func, b);
	f->parmlist = vnames;
	install(name, f);
}

/* get_argument --- Get the n'th argument of a dynamically linked function */

NODE *
get_argument(NODE *tree, int i)
{
	extern NODE **stack_ptr;
	int actual_args;

	actual_args = get_curfunc_arg_count();
	if (i < 0 || i >= tree->param_cnt || i >= actual_args)
		return NULL;

	tree = stack_ptr[i];
	if (tree->type == Node_array_ref)
		tree = tree->orig_array;
	if (tree->type == Node_var_new || tree->type == Node_var_array)
		return tree;

	return tree->lnode;
}

/* get_actual_argument --- get a scalar or array, allowed to be optional */

NODE *
get_actual_argument(NODE *tree, unsigned int i, int optional, int want_array)
{
	/* optional : if TRUE and i th argument not present return NULL, else fatal. */

	NODE *t;

	t = get_argument(tree, i);

	if (t == NULL) {
		if (i >= tree->param_cnt)		/* must be fatal */
			fatal(_("function `%s' defined to take no more than `%d' argument(s)"),
					tree->param, tree->param_cnt);
		if (! optional)
			fatal(_("function `%s': missing argument #%d"),
					tree->param, i + 1);
		return NULL;
	}

	if (t->type == Node_var_new)
		return (want_array ? get_array(t) : tree_eval(t));

	if (want_array) {
		if (t->type != Node_var_array)
			fatal(_("function `%s': argument #%d: attempt to use scalar as an array"),
				tree->param, i + 1);
	} else {
		if (t->type != Node_val)
			fatal(_("function `%s': argument #%d: attempt to use array as a scalar"),
				tree->param, i + 1);
	}
	return t;
}

/* set_value --- set the return value of a dynamically linked function */

void
set_value(NODE *tree)
{
	extern NODE *ret_node;

	if (tree)
		ret_node = tree;
	else
		ret_node = Nnull_string;
}
#else

/* do_ext --- dummy version if extensions not available */

NODE *
do_ext(NODE *tree)
{
	const char *emsg = _("Operation Not Supported");

	unref(ERRNO_node->var_value);
	ERRNO_node->var_value = make_string((char *) emsg, strlen(emsg));
	return tmp_number((AWKNUM) -1);
}

void
load_extension(const char *name)
{
	warning(_("cannot load extension library `%s' on this platform"), name);
}

#endif
