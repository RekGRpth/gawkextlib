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

/* For some reason, the tmp_number macro in awk.h does not cast to AWKNUM,
   and we want to avoid problems on platforms where prototypes may not
   be available. */
#define Tmp_number(x) tmp_number((AWKNUM)(x))

/* Just to make the interpreter happy */
#define RETURN return Tmp_number(0)

static inline int
get_intarg(NODE *tree, unsigned int argnum)
{
  NODE *argnode = get_scalar_argument(tree, argnum, FALSE);
  /* Do we need to worry about rounding here? */
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
    static unsigned long hnum = 0;
    char handle[32];
    size_t sl;

    snprintf(handle, sizeof(handle), "pgconn%lu", hnum++);
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

static NODE *
do_pg_disconnect(NODE *tree)
{
  NODE *handle;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_disconnect: called with too many arguments");

  handle = get_scalar_argument(tree, 0, FALSE);
  force_string(handle);
  if (strhash_delete(conns, handle->stptr, handle->stlen,
		     (strhash_delete_func)PQfinish, NULL) < 0) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_disconnect called with unknown connection handle");
  }
  else
    set_value(Tmp_number(0));
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
    set_value(Tmp_number(-1));
    set_ERRNO("pg_reset called with unknown connection handle");
  }
  else {
    PQreset(conn);	/* no return value */
    if (PQstatus(conn) != CONNECTION_OK) {
      set_value(Tmp_number(-1));
      set_ERRNO_no_gettext(PQerrorMessage(conn));
    }
    else
      set_value(Tmp_number(0));
  }
  RETURN;
}

static NODE *
do_pg_errormessage(NODE *tree)
{
  PGconn *conn;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_errormessage: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_errormessage called with unknown connection handle");
  }
  else {
    char *emsg = PQerrorMessage(conn);
    set_value(tmp_string(emsg, strlen(emsg)));
  }
  RETURN;
}

static int
get_params(NODE *tree, unsigned int argnum, const char ***pvp)
{
  NODE *paramValues_array;
  const char **paramValues;
  NODE *nParams_node = get_scalar_argument(tree, argnum, FALSE);
  int nParams = force_number(nParams_node);

  free_temp(nParams_node);
  if (nParams < 0)
    return nParams;

  if (!nParams ||
      !(paramValues_array = get_array_argument(tree, argnum+1, TRUE)) ||
      !paramValues_array->var_array)
    paramValues = NULL;
  else {
    int i;
    emalloc(paramValues, const char **, nParams*sizeof(*paramValues),
	    "get_params");
    for (i = 0; i < nParams; i++) {
      NODE *val;
      if (!(val = assoc_search(paramValues_array, Tmp_number(i))))
        paramValues[i] = NULL;
      else {
	force_string(val);
	paramValues[i] = val->stptr;
      }
    }
  }
  *pvp = paramValues;
  return nParams;
}

static NODE *
do_pg_sendquery(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_sendquery: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Tmp_number(0));
    set_ERRNO("pg_sendquery called with unknown connection handle");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQsendQuery(conn, command->stptr);
  free_temp(command);

  set_value(Tmp_number(res));
  if (!res)
    /* connection is probably bad */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  RETURN;
}

/* create a prepared statement identifier unique to this session */
static char *
prep_name(void)
{
  static unsigned long prepnum = 0;
  static char buf[32];

  snprintf(buf, sizeof(buf), "gawk_pg_%lu", prepnum++);
  return buf;
}

static NODE *
do_pg_sendprepare(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  char *stmtName;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_sendprepare: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_sendprepare called with unknown connection handle");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQsendPrepare(conn, (stmtName = prep_name()), command->stptr, 0, NULL);
  free_temp(command);

  if (!res) {
    /* connection is probably bad */
    set_value(Nnull_string);
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  }
  else
    set_value(tmp_string(stmtName, strlen(stmtName)));
  RETURN;
}

static NODE *
do_pg_sendqueryparams(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  int nParams;
  const char **paramValues;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 4))
    lintwarn("pg_sendqueryparams: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Tmp_number(0));
    set_ERRNO("pg_sendqueryparams called with unknown connection handle");
    RETURN;
  }

  if ((nParams = get_params(tree, 2, &paramValues)) < 0) {
    set_value(Tmp_number(0));
    set_ERRNO("pg_sendqueryparams called with negative nParams");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQsendQueryParams(conn, command->stptr, nParams,
  			  NULL, paramValues, NULL, NULL, 0);
  free_temp(command);
  if (paramValues)
    free(paramValues);

  set_value(Tmp_number(res));
  if (!res)
    /* connection is probably bad */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  RETURN;
}

static NODE *
do_pg_sendqueryprepared(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  int nParams;
  const char **paramValues;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 4))
    lintwarn("pg_sendqueryprepared: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Tmp_number(0));
    set_ERRNO("pg_sendqueryprepared called with unknown connection handle");
    RETURN;
  }

  if ((nParams = get_params(tree, 2, &paramValues)) < 0) {
    set_value(Tmp_number(0));
    set_ERRNO("pg_sendqueryprepared called with negative nParams");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQsendQueryPrepared(conn, command->stptr, nParams,
			    paramValues, NULL, NULL, 0);
  free_temp(command);
  if (paramValues)
    free(paramValues);

  set_value(Tmp_number(res));
  if (!res)
    /* connection is probably bad */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  RETURN;
}

static NODE *
do_pg_putcopydata(NODE *tree)
{
  PGconn *conn;
  NODE *buffer;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_putcopydata: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_putcopydata called with unknown connection handle");
    RETURN;
  }

  buffer = get_scalar_argument(tree, 1, FALSE);
  force_string(buffer);
  res = PQputCopyData(conn, buffer->stptr, buffer->stlen);
  free_temp(buffer);

  set_value(Tmp_number(res));
  if (res < 0)
    /* connection is probably bad */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  RETURN;
}

static NODE *
do_pg_putcopyend(NODE *tree)
{
  PGconn *conn;
  NODE *emsg;
  int res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_putcopyend: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_putcopyend called with unknown connection handle");
    RETURN;
  }

  if ((emsg = get_scalar_argument(tree, 1, TRUE)) != NULL)
    force_string(emsg);
  res = PQputCopyEnd(conn, (emsg ? emsg->stptr : NULL));
  if (emsg)
    free_temp(emsg);

  set_value(Tmp_number(res));
  if (res < 0)
    /* connection is probably bad */
    set_ERRNO_no_gettext(PQerrorMessage(conn));
  RETURN;
}

static NODE *
do_pg_getcopydata(NODE *tree)
{
  PGconn *conn;
  char *buffer;
  int rc;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_getcopydata: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_getcopydata called with unknown connection handle");
    RETURN;
  }

  buffer = NULL;
  switch (rc = PQgetCopyData(conn, &buffer, FALSE)) {
  /* case 0 can only happen if async is TRUE */
  case -1: /* copy done */
    set_value(Nnull_string);
    unset_ERRNO();
    break;
  case -2: /* error */
    set_value(Nnull_string);
    {
      const char *emsg = PQerrorMessage(conn);
      if (emsg)
        set_ERRNO_no_gettext(PQerrorMessage(conn));
      else
        set_ERRNO("PQgetCopyData failed, but no error message is available");
    }
    break;
  default: /* rc should be positive and equal # of bytes in row */
    if (rc > 0) {
      set_value(tmp_string(buffer, rc));
      unset_ERRNO();
    }
    else {
      /* this should not happen */
      char buf[256];
      set_value(Nnull_string);
      snprintf(buf, sizeof(buf), "PQgetCopyData returned invalid value %d: %s",
	       rc, PQerrorMessage(conn));
      set_ERRNO(buf);
    }
  }

  if (buffer)
    PQfreemem(buffer);
  RETURN;
}

static void
set_error(PGconn *conn, ExecStatusType status)
{
  char buf[100];
  snprintf(buf, sizeof(buf), "ERROR %s%s",
	   ((PQstatus(conn) != CONNECTION_OK) ? "BADCONN " : ""),
	   PQresStatus(status));
  set_value(tmp_string(buf, strlen(buf)));
}

static NODE *
process_result(PGconn *conn, PGresult *res)
{
  ExecStatusType rc;

  switch (rc = PQresultStatus(res)) {
  case PGRES_TUPLES_OK:
    {
      static unsigned long hnum = 0;
      char handle[64];
      size_t sl;

      snprintf(handle, sizeof(handle), "TUPLES %d pgres%lu",
	       PQntuples(res), hnum++);
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
      snprintf(result, sizeof(result), "OK %d", cnt);
      PQclear(res);
      set_value(tmp_string(result, strlen(result)));
    }
    break;
  case PGRES_COPY_IN:
    {
      char buf[100];
      snprintf(buf, sizeof(buf), "COPY_IN %d %s",
	       PQnfields(res), (PQbinaryTuples(res) ? "BINARY" : "TEXT"));
      set_value(tmp_string(buf, strlen(buf)));
      PQclear(res);
    }
    break;
  case PGRES_COPY_OUT:
    {
      char buf[100];
      snprintf(buf, sizeof(buf), "COPY_OUT %d %s",
	       PQnfields(res), (PQbinaryTuples(res) ? "BINARY" : "TEXT"));
      set_value(tmp_string(buf, strlen(buf)));
      PQclear(res);
    }
    break;
  default: /* error */
    set_error(conn, rc);
    set_ERRNO(PQresultErrorMessage(res));
    PQclear(res);
  }
  RETURN;
}

static NODE *
do_pg_getresult(NODE *tree)
{
  PGconn *conn;
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_getresult: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_getresult called with unknown connection handle");
    RETURN;
  }

  if (!(res = PQgetResult(conn))) {
    /* this just means there are no results currently available, so it is
       not necessarily an error */
    set_value(Nnull_string);
    RETURN;
  }
  return process_result(conn, res);
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
    set_ERRNO("pg_exec called with unknown connection handle");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQexec(conn, command->stptr);
  free_temp(command);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL));
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    RETURN;
  }
  return process_result(conn, res);
}

static NODE *
do_pg_prepare(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  char *stmtName;
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_prepare: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_prepare called with unknown connection handle");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQprepare(conn, (stmtName = prep_name()), command->stptr, 0, NULL);
  free_temp(command);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_value(Nnull_string);
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    RETURN;
  }

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    set_value(Nnull_string);
    set_ERRNO(PQresultErrorMessage(res));
    PQclear(res);
    RETURN;
  }

  PQclear(res);
  set_value(tmp_string(stmtName, strlen(stmtName)));
  RETURN;
}

static NODE *
do_pg_execparams(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  int nParams;
  const char **paramValues;
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 4))
    lintwarn("pg_execparams: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_execparams called with unknown connection handle");
    RETURN;
  }

  if ((nParams = get_params(tree, 2, &paramValues)) < 0) {
    set_value(Nnull_string);
    set_ERRNO("pg_execparams called with negative nParams");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQexecParams(conn, command->stptr, nParams,
		     NULL, paramValues, NULL, NULL, 0);
  free_temp(command);
  if (paramValues)
    free(paramValues);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL));
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    RETURN;
  }
  return process_result(conn, res);
}

static NODE *
do_pg_execprepared(NODE *tree)
{
  PGconn *conn;
  NODE *command;
  int nParams;
  const char **paramValues;
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 4))
    lintwarn("pg_execprepared: called with too many arguments");

  if (!(conn = find_handle(conns, tree, 0))) {
    set_value(Nnull_string);
    set_ERRNO("pg_execprepared called with unknown connection handle");
    RETURN;
  }

  if ((nParams = get_params(tree, 2, &paramValues)) < 0) {
    set_value(Nnull_string);
    set_ERRNO("pg_execprepared called with negative nParams");
    RETURN;
  }

  command = get_scalar_argument(tree, 1, FALSE);
  force_string(command);
  res = PQexecPrepared(conn, command->stptr, nParams, paramValues, NULL, NULL, 0);
  free_temp(command);
  if (paramValues)
    free(paramValues);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL));
    set_ERRNO_no_gettext(PQerrorMessage(conn));
    RETURN;
  }
  return process_result(conn, res);
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
		     (strhash_delete_func)PQclear, NULL) < 0) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_clear called with unknown result handle");
  }
  else
    set_value(Tmp_number(0));
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
    set_value(Tmp_number(-1));
    set_ERRNO("pg_ntuples called with unknown result handle");
  }
  else
    set_value(Tmp_number(PQntuples(res)));
  RETURN;
}

static NODE *
do_pg_nfields(NODE *tree)
{
  PGresult *res;

  if (do_lint && (get_curfunc_arg_count() > 1))
    lintwarn("pg_nfields: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_nfields called with unknown result handle");
  }
  else
    set_value(Tmp_number(PQnfields(res)));
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
    set_ERRNO("pg_fname called with unknown result handle");
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
    set_value(Tmp_number(-1));
    set_ERRNO("pg_fields called with unknown result handle");
    RETURN;
  }

  array = get_array_argument(tree, 1, FALSE);
  assoc_clear(array);

  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    NODE **aptr;
    char *fname;

    aptr = assoc_lookup(array, Tmp_number(col), FALSE);
    fname = PQfname(res, col);
    *aptr = make_string(fname, strlen(fname));
  }
  set_value(Tmp_number(nf));
  RETURN;
}

static NODE *
do_pg_fieldsbyname(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int nf;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 2))
    lintwarn("pg_fieldsbyname: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_fieldsbyname called with unknown result handle");
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
  set_value(Tmp_number(nf));
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
    set_ERRNO("pg_getvalue called with unknown result handle");
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
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getisnull called with unknown result handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getisnull: 2nd argument row_number is out of range");
    RETURN;
  }

  if (((col = get_intarg(tree, 2)) < 0) || (col >= PQnfields(res))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getisnull: 3rd argument col_number is out of range");
    RETURN;
  }

  set_value(Tmp_number(PQgetisnull(res, row, col)));
  RETURN;
}

static NODE *
do_pg_getrow(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_getrow: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getrow called with unknown result handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getrow: 2nd argument row_number is out of range");
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
      aptr = assoc_lookup(array, Tmp_number(col), FALSE);
      val = PQgetvalue(res, row, col);
      *aptr = make_string(val, strlen(val));
      found++;
    }
  }
  set_value(Tmp_number(found));
  RETURN;
}

static NODE *
do_pg_getrowbyname(NODE *tree)
{
  PGresult *res;
  NODE *array;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (get_curfunc_arg_count() > 3))
    lintwarn("pg_getrowbyname: called with too many arguments");

  if (!(res = find_handle(results, tree, 0))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getrowbyname called with unknown result handle");
    RETURN;
  }

  if (((row = get_intarg(tree, 1)) < 0) || (row >= PQntuples(res))) {
    set_value(Tmp_number(-1));
    set_ERRNO("pg_getrowbyname: 2nd argument row_number is out of range");
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
  set_value(Tmp_number(found));
  RETURN;
}

#ifdef BUILD_STATIC_EXTENSIONS
#define dlload dlload_pgsql
#endif

NODE *
dlload(NODE *tree ATTRIBUTE_UNUSED, void *dl ATTRIBUTE_UNUSED)
{
  /* Wrappers for libpq functions: */
  make_builtin("pg_connect", do_pg_connect, 1);
  make_builtin("pg_connectdb", do_pg_connect, 1);  /* alias for pg_connect */
  make_builtin("pg_errormessage", do_pg_errormessage, 1);
  make_builtin("pg_sendquery", do_pg_sendquery, 2);
  make_builtin("pg_sendqueryparams", do_pg_sendqueryparams, 4);
  make_builtin("pg_sendprepare", do_pg_sendprepare, 2);
  make_builtin("pg_sendqueryprepared", do_pg_sendqueryprepared, 4);
  make_builtin("pg_exec", do_pg_exec, 2);
  make_builtin("pg_execparams", do_pg_execparams, 4);
  make_builtin("pg_prepare", do_pg_prepare, 2);
  make_builtin("pg_execprepared", do_pg_execprepared, 4);
  make_builtin("pg_nfields", do_pg_nfields, 1);
  make_builtin("pg_ntuples", do_pg_ntuples, 1);
  make_builtin("pg_fname", do_pg_fname, 2);
  make_builtin("pg_getvalue", do_pg_getvalue, 3);
  make_builtin("pg_getisnull", do_pg_getisnull, 3);
  make_builtin("pg_clear", do_pg_clear, 1);
  make_builtin("pg_disconnect", do_pg_disconnect, 1);
  make_builtin("pg_finish", do_pg_disconnect, 1);  /* alias for pg_disconnect */
  make_builtin("pg_reset", do_pg_reset, 1);
  make_builtin("pg_reconnect", do_pg_reset, 1);  /* alias for pg_reset */
  make_builtin("pg_getresult", do_pg_getresult, 1);
  make_builtin("pg_putcopydata", do_pg_putcopydata, 2);
  make_builtin("pg_putcopyend", do_pg_putcopyend, 2);
  make_builtin("pg_getcopydata", do_pg_getcopydata, 1);

  /* Higher-level functions using awk associative arrays: */
  make_builtin("pg_fields", do_pg_fields, 2);
  make_builtin("pg_fieldsbyname", do_pg_fieldsbyname, 2);
  make_builtin("pg_getrow", do_pg_getrow, 3);
  make_builtin("pg_getrowbyname", do_pg_getrowbyname, 3);

  /* Create hash tables. */
  conns = strhash_create(0);
  results = strhash_create(0);

  return Tmp_number(0);
}
