/*
 * reclen.c --- Provide fixed length input records.
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2020
 */

/*
 * Copyright (C) 2020 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define GAWKEXTLIB_NOT_NEEDED
#include "common.h"

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "reclen extension: version 1.0";

static awk_bool_t init_reclen(void);
static awk_bool_t (*init_func)(void) = init_reclen;

int plugin_is_GPL_compatible;

/* data type for the opaque pointer: */

typedef struct fixed_buffer {
	struct fixed_buffer *next;	/* linked list pointer */
	const char *name;		/* name of file */
	awk_input_buf_t *iobuf;		/* parent iobuf */

	char *buf;			/* data buffer */
	size_t buflen;			/* amount to read */
	size_t alloclen;		/* amount allocated */
	awk_scalar_t reclen_cookie;	/* cookie for RECLEN, could be global? */
} fixed_buffer_t;

static fixed_buffer_t file_list;


/* reclen_get_record --- get one record of length RECLEN */

static int
reclen_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len,
		const awk_fieldwidth_info_t **unused __UNUSED)
{
	fixed_buffer_t *fixed_buffer;
	awk_value_t reclen;
	int len = 0;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */
	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	fixed_buffer = (fixed_buffer_t *) iobuf->opaque;

	/* RECLEN might have changed, recheck it */
	if (! sym_lookup_scalar(fixed_buffer->reclen_cookie, AWK_NUMBER, & reclen)) {
		warning(ext_id, _("reclen_get_record: could not get value of RECLEN"));
		*errcode = EINVAL;
		update_ERRNO_int(EINVAL);
		return EOF;
	}

	if (reclen.num_value <= 0) {
		return EOF;
	}

	// adjust buffer size if necessary
	if (reclen.num_value > fixed_buffer->alloclen) {
		erealloc(fixed_buffer->buf, char *, (int) reclen.num_value, "reclen_get_record");
		fixed_buffer->alloclen = reclen.num_value;
	}

	fixed_buffer->buflen = reclen.num_value;

	// do the read
	errno = 0;
	len = read(iobuf->fd, fixed_buffer->buf, fixed_buffer->buflen);

	if (len < 0) {
		*errcode = errno;
		update_ERRNO_int(EINVAL);
		return EOF;
	} else if (len == 0)
		return EOF;

	// set output variables and return
	*out = fixed_buffer->buf;

	*rt_start = NULL;
	*rt_len = 0;	/* set RT to "" */
	return len;
}

/* remove_buffer_from_list --- remove this buffer from the list of controlled files */

static fixed_buffer_t *
remove_buffer_from_list(const char *name)
{
	fixed_buffer_t *item;
	fixed_buffer_t *prev;

	for (prev = &file_list, item = file_list.next; item != NULL;
					prev = item, item = item->next) {
		if (strcmp(item->name, name) == 0) {
			prev->next = item->next;
			return item;
			break;
		}
	}

	return NULL;
}

/* reclen_close --- close up when done */

static void
reclen_close(awk_input_buf_t *iobuf)
{
	fixed_buffer_t *fixed_buffer;

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	fixed_buffer = (fixed_buffer_t *) iobuf->opaque;

	remove_buffer_from_list(fixed_buffer->name);

	gawk_free(fixed_buffer->buf);
	gawk_free(fixed_buffer);

	(void) close(iobuf->fd);
	iobuf->fd = -1;
}

/* reclen_can_take_file --- return true if we want the file */

static awk_bool_t
reclen_can_take_file(const awk_input_buf_t *iobuf)
{
	awk_value_t reclen;

	if (iobuf == NULL)
		return awk_false;

	if (! sym_lookup("RECLEN", AWK_NUMBER, & reclen))
		return awk_false;

	if (reclen.num_value <= 0)
		return awk_false;

	return awk_true;
}

/*
 * reclen_take_control_of --- set up input parser.
 * We can assume that reclen_can_take_file just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
reclen_take_control_of(awk_input_buf_t *iobuf)
{
	char *buf;
	fixed_buffer_t *fixed_buffer;
	awk_value_t reclen;
	awk_scalar_t reclen_cookie;

	if (! sym_lookup("RECLEN", AWK_SCALAR, & reclen)) {
		warning(ext_id, _("reclen_take_control_of: could not get cookie for RECLEN"));
		update_ERRNO_int(EINVAL);
		return awk_false;
	}

	reclen_cookie = reclen.scalar_cookie;

	if (! sym_lookup_scalar(reclen_cookie, AWK_NUMBER, & reclen)) {
		warning(ext_id, _("reclen_take_control_of: could not get value of RECLEN"));
		update_ERRNO_int(EINVAL);
		return awk_false;
	}

	if (reclen.num_value <= 0) {
		warning(ext_id, _("reclen_take_control_of: RECLEN changed value!"));
		update_ERRNO_int(EINVAL);
		return awk_false;
	}

	emalloc(fixed_buffer, fixed_buffer_t *, sizeof(fixed_buffer_t), "reclen_take_control_of");
	emalloc(buf, char *, (int) reclen.num_value, "reclen_take_control_of");

	fixed_buffer->name = iobuf->name;
	fixed_buffer->iobuf = iobuf;

	fixed_buffer->buf = buf;
	fixed_buffer->alloclen = fixed_buffer->buflen = reclen.num_value;
	fixed_buffer->reclen_cookie = reclen_cookie;

	// push onto linked list
	fixed_buffer->next = file_list.next;
	file_list.next = fixed_buffer;

	iobuf->opaque = fixed_buffer;

	iobuf->get_record = reclen_get_record;
	iobuf->close_func = reclen_close;

	return awk_true;
}

/* do_reclen_drop --- totally drop a file from RECLEN processing */

static awk_value_t *
do_reclen_drop(int nargs __UNUSED, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t filename;
	int ret = 0;

	if (get_argument(0, AWK_STRING, & filename)) {
		fixed_buffer_t *buffer = remove_buffer_from_list(filename.str_value.str);
		buffer->iobuf->get_record = NULL;
	} else {
		warning(ext_id, _("reclen::drop: argument is not a string"));
		errno = EINVAL;
		update_ERRNO_int(EINVAL);
		ret = -1;
	}

	return make_number(ret, result);
}

/* init_reclen --- set things ups */

static awk_input_parser_t reclen_parser = {
	"reclen",
	reclen_can_take_file,
	reclen_take_control_of,
	NULL
};

static awk_bool_t
init_reclen(void)
{
	register_input_parser(& reclen_parser);
	GAWKEXTLIB_COMMON_INIT
	return awk_true;
}

static awk_ext_func_t func_table[] = {
	API_FUNC_MAXMIN("drop", do_reclen_drop, 1, 1)
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, reclen, "reclen")
