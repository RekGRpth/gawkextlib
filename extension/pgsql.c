/*
 * pgsql.c - Builtin functions that provide interface to the PostgreSQL libpq
 *		library.
 *
 * Andrew J. Schorr, Thu Apr 14 15:39:51 EDT 2005
 */

/*
 * Copyright (C) 2005 the Free Software Foundation, Inc.
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
#include <libpq-fe.h>

static strhash *conns;
static strhash *results;

/* Just to make the interpreter happy */
#define RETURN return tmp_number((AWKNUM) 0)

static inline int
get_intarg(NODE *tree, unsigned int argnum)
{
  NODE *argnode = get_scalar_argument(tree, argnum, FALSE);
  int arg = force_number(argnode);
  free_temp(argnode);
  return arg;
}

static NODE *
do_pg_connect(NODE *tree)
{
  NODE *conninfo;
  PGconn *conn;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_connect: called with too many arguments");

  /* grab optional connection options argument */
  if ((conninfo = get_scalar_argument(tree, 0, TRUE)) != NULL) {
    force_string(conninfo);
    conn = PQconnectdb(conninfo->stptr);
    free_temp(conninfo);
  }
  else
    conn = PQconnectdb("");

  if (PQstatus(conn) != CONNECTION_OK) {
    /* error */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    if (conn)
      PQfinish(conn);
    /* Set the return value */
    set_value(Nnull_string);
  }
  else {
    /* good connection: return a handle */
    static int hnum = 0;
    char handle[20];
    size_t sl;

    snprintf(handle, sizeof(handle), "pgconn%d", hnum++);
    sl = strlen(handle);
    strhash_get(conns, handle, sl, 1)->data = conn;
    set_value(tmp_string(handle, sl));
  }

  RETURN;
}

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

static void
do_finish(strhash *ht ATTRIBUTE_UNUSED, strhash_entry *ent,
	  void *opaque ATTRIBUTE_UNUSED)
{
  PQfinish((PGconn *)(ent->data));
}

static NODE *
do_pg_disconnect(NODE *tree)
{
  NODE *handle;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_disconnect: called with too many arguments");

  handle = get_scalar_argument(tree, 0, FALSE);
  force_string(handle);
  if (strhash_delete(conns, handle->stptr, handle->stlen,
		     do_finish, NULL) < 0) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_disconnect called with unknown handle");
  }
  else
    set_value(tmp_number(0));
  free_temp(handle);
  RETURN;
}

static NODE *
do_pg_reset(NODE *tree)
{
  PGconn *conn;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_reset: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_reset called with unknown handle");
  }
  else {
    PQreset(conn);	/* no return value */
    if (PQstatus(conn) != CONNECTION_OK) {
      set_value(tmp_number(-1));
      set_ERRNO_no_gettext(PQerrorMessage(conn));
    }
    else
      set_value(tmp_number(0));
  }
  RETURN;
}

static NODE *
do_pg_exec(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_exec: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_exec called with unknown handle");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQexec(conn, command->stptr);
  free_temp(command);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_value((PQstatus(conn) != CONNECTION_OK) ?
	      tmp_string("BADCONN", 7) : Nnull_string);
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    RETURN;
  }

  switch (PQresultStatus(res)) {
  case PGRES_TUPLES_OK:
    {
      static int hnum = 0;
      char handle[32];
      size_t sl;

      snprintf(handle, sizeof(handle), "%d pgresult%d", PQntuples(res), hnum++);
      sl = strlen(handle);
      strhash_get(results, handle, sl, 1)->data = res;
      set_value(tmp_string(handle, sl));
    }
    break;
  case PGRES_COMMAND_OK:
  case PGRES_EMPTY_QUERY:
    {
      char result[32];
      int cnt;

      if (sscanf(PQcmdTuples(res), "%d", &cnt) != 1)
        cnt = 0;
      snprintf(result, sizeof(result), "%d OK", cnt);
      PQclear(res);
      set_value(tmp_string(result, strlen(result)));
    }
    break;
  case PGRES_COPY_IN:
  case PGRES_COPY_OUT:
    set_value(Nnull_string);
    set_ERRNO("Not allowed to start COPY with pg_exec");
    PQclear(res);
    break;
  default: /* error */
    set_value((PQstatus(conn) != CONNECTION_OK) ?
	      tmp_string("BADCONN", 7) : Nnull_string);
    set_ERRNO(PQresultErrorMessage(res));
    PQclear(res);
  }
  RETURN;
}

static void
do_clear(strhash *ht ATTRIBUTE_UNUSED, strhash_entry *ent,
	 void *opaque ATTRIBUTE_UNUSED)
{
  PQclear((PGresult *)(ent->data));
}

static NODE *
do_pg_clear(NODE *tree)
{
  NODE *handle;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_clear: called with too many arguments");

  handle = get_scalar_argument(tree, 0, FALSE);
  force_string(handle);
  if (strhash_delete(results, handle->stptr, handle->stlen,
		     do_clear, NULL) < 0) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_clear called with unknown handle");
  }
  else
    set_value(tmp_number(0));
  free_temp(handle);
  RETURN;
}

static NODE *
do_pg_ntuples(NODE *tree)
{
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_ntuples: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_ntuples called with unknown handle");
  }
  else
    set_value(tmp_number(PQntuples(res)));
  RETURN;
}

static NODE *
do_pg_nfields(NODE *tree)
{
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_nfields: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_nfields called with unknown handle");
  }
  else
    set_value(tmp_number(PQnfields(res)));
  RETURN;
}

static NODE *
do_pg_fname(NODE *tree)
{
  PGresult *res;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_fname: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_fname called with unknown handle");
    RETURN;
  }

  if (((col = get_intarg(tree, 1)) < 0) || (col >= PQnfields(res))) {
    set_value(Nnull_string);
    set_ERRNO("pg_fname: 2nd argument col_number is out of range");
    RETURN;
  }

  {
    char *fname = PQfname(res, col);
    set_value(tmp_string(fname, strlen(fname)));
  }
  RETURN;
}

static NODE *
do_pg_fields(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int nf;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_fields: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fields called with unknown handle");
    RETURN;
  }

  array = get_array_argument(tree, 1, FALSE);
  assoc_clear(array);

  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    NODE **aptr;
    char *fname;

    aptr = assoc_lookup(array, tmp_number(col), FALSE);
    fname = PQfname(res, col);
    *aptr = make_string(fname, strlen(fname));
  }
  set_value(tmp_number(nf));
  RETURN;
}

static NODE *
do_pg_fields_byname(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int nf;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_fields_byname: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fields_byname called with unknown handle");
    RETURN;
  }

  array = get_array_argument(tree, 1, FALSE);
  assoc_clear(array);

  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    char *fname;
    NODE **aptr;

    fname = PQfname(res, col);
    aptr = assoc_lookup(array, tmp_string(fname, strlen(fname)), FALSE);
    *aptr = make_number(col);
  }
  set_value(tmp_number(nf));
  RETURN;
}

static NODE *
do_pg_getvalue(NODE *tree)
{
  PGresult *res;
  int row;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_getvalue: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_getvalue called with unknown handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(Nnull_string);
    set_ERRNO("pg_getvalue: 2nd argument row_number is out of range");
    RETURN;
  }

  if (((col = get_intarg(tree, 2)) < 0) || (col >= PQnfields(res))) {
    set_value(Nnull_string);
    set_ERRNO("pg_getvalue: 3rd argument col_number is out of range");
    RETURN;
  }

  {
    char *val = PQgetvalue(res, row, col);
    set_value(make_string(val, strlen(val)));
  }
  RETURN;
}

static NODE *
do_pg_getisnull(NODE *tree)
{
  PGresult *res;
  int row;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_getisnull: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_getisnull called with unknown handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_getisnull: 2nd argument row_number is out of range");
    RETURN;
  }

  if (((col = get_intarg(tree, 2)) < 0) || (col >= PQnfields(res))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_getisnull: 3rd argument col_number is out of range");
    RETURN;
  }

  set_value(tmp_number(PQgetisnull(res, row, col)));
  RETURN;
}

static NODE *
do_pg_fetchrow(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_fetchrow: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fetchrow called with unknown handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fetchrow: 2nd argument row_number is out of range");
    RETURN;
  }

  array = get_array_argument(tree, 2, FALSE);
  assoc_clear(array);

  found = 0;
  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    if (!PQgetisnull(res, row, col)) {
      NODE **aptr;
      char *val;
      aptr = assoc_lookup(array, tmp_number(col), FALSE);
      val = PQgetvalue(res, row, col);
      *aptr = make_string(val, strlen(val));
      found++;
    }
  }
  set_value(tmp_number(found));
  RETURN;
}

static NODE *
do_pg_fetchrow_byname(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_fetchrow_byname: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fetchrow_byname called with unknown handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(tmp_number(-1));
    set_ERRNO("pg_fetchrow_byname: 2nd argument row_number is out of range");
    RETURN;
  }

  array = get_array_argument(tree, 2, FALSE);
  assoc_clear(array);

  found = 0;
  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    if (!PQgetisnull(res, row, col)) {
      char *fname;
      NODE **aptr;
      char *val;

      fname = PQfname(res, col);
      aptr = assoc_lookup(array, tmp_string(fname, strlen(fname)), FALSE);
      val = PQgetvalue(res, row, col);
      *aptr = make_string(val, strlen(val));
      found++;
    }
  }
  set_value(tmp_number(found));
  RETURN;
}

#ifdef BUILD_STATIC_EXTENSIONS
#define dlload dlload_pgsql
#endif

NODE *
dlload(NODE *tree ATTRIBUTE_UNUSED, void *dl ATTRIBUTE_UNUSED)
{
  make_builtin("pg_connect", do_pg_connect, 1);
  make_builtin("pg_connectdb", do_pg_connect, 1);  /* alias for pg_connect */
  make_builtin("pg_exec", do_pg_exec, 2);
  make_builtin("pg_nfields", do_pg_nfields, 1);
  make_builtin("pg_ntuples", do_pg_ntuples, 1);
  make_builtin("pg_fname", do_pg_fname, 2);
  make_builtin("pg_fields", do_pg_fields, 2);
  make_builtin("pg_fields_byname", do_pg_fields_byname, 2);
  make_builtin("pg_fetchrow", do_pg_fetchrow, 3);
  make_builtin("pg_fetchrow_byname", do_pg_fetchrow_byname, 3);
  make_builtin("pg_getvalue", do_pg_getvalue, 3);
  make_builtin("pg_getisnull", do_pg_getisnull, 3);
  make_builtin("pg_clear", do_pg_clear, 1);
  make_builtin("pg_disconnect", do_pg_disconnect, 1);
  make_builtin("pg_finish", do_pg_disconnect, 1);  /* alias for pg_disconnect */
  make_builtin("pg_reset", do_pg_reset, 1);
  make_builtin("pg_reconnect", do_pg_reset, 1);  /* alias for pg_reset */
  conns = strhash_create(0);
  results = strhash_create(0);

  return tmp_number((AWKNUM) 0);
}
