/*
 * pgsql.c - Builtin functions that provide interface to the PostgreSQL libpq
 *		library.
 *
 * Andrew J. Schorr, Thu Apr 14 15:39:51 EDT 2005
 */

/*
 * Copyright (C) 2005 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "common.h"
#include <libpq-fe.h>

static strhash *conns;
static strhash *results;


static awk_value_t *
do_pg_connect(int nargs, awk_value_t *result)
{
  PGconn *conn;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_connect: called with too many arguments"));

  /* grab optional connection options argument */
  if (nargs > 0) {
    awk_value_t conninfo;
    if (!get_argument(0, AWK_STRING, &conninfo)) {
      set_ERRNO(_("pg_connect: argument is present but not a string"));
      RET_NULSTR;
    }
    conn = PQconnectdb(conninfo.str_value.str);
  }
  else
    conn = PQconnectdb("");

  if (PQstatus(conn) != CONNECTION_OK) {
    /* error */
    set_ERRNO(PQerrorMessage(conn));
    if (conn)
      PQfinish(conn);
    RET_NULSTR;
  }

  {
    /* good connection: return a handle */
    static unsigned long hnum = 0;
    char handle[32];
    size_t sl;

    snprintf(handle, sizeof(handle), "pgconn%lu", hnum++);
    sl = strlen(handle);
    strhash_get(conns, handle, sl, 1)->data = conn;
    return make_string_malloc(handle, sl, result);
  }
}

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

static awk_value_t *
do_pg_disconnect(int nargs, awk_value_t *result)
{
  awk_value_t handle;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_disconnect: called with too many arguments"));

  if (!get_argument(0, AWK_STRING, &handle)) {
    set_ERRNO(_("pg_disconnect requires a string handle argument"));
    RET_NUM(-1);
  }
  if (strhash_delete(conns, handle.str_value.str, handle.str_value.len,
		     (strhash_delete_func)PQfinish, NULL) < 0) {
    set_ERRNO(_("pg_disconnect called with unknown connection handle"));
    RET_NUM(-1);
  }
  RET_NUM(0);
}

static awk_value_t *
do_pg_reset(int nargs, awk_value_t *result)
{
  PGconn *conn;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_reset: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_reset called with unknown connection handle"));
    RET_NUM(-1);
  }
  PQreset(conn);	/* no return value */
  if (PQstatus(conn) != CONNECTION_OK) {
    set_ERRNO(PQerrorMessage(conn));
    RET_NUM(-1);
  }
  RET_NUM(0);
}

static awk_value_t *
do_pg_errormessage(int nargs, awk_value_t *result)
{
  PGconn *conn;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_errormessage: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_errormessage called with unknown connection handle"));
    RET_NULSTR;
  }
  {
    char *emsg = PQerrorMessage(conn);
    return make_string_malloc(emsg, strlen(emsg), result);
  }
}

static awk_value_t *
do_pg_serverversion(int nargs, awk_value_t *result)
{
  PGconn *conn;
  int version;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_serverversion: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_serverversion called with unknown connection handle"));
    RET_NUM(0);
  }
  if (!(version = PQserverVersion(conn)))
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(version);
}

/* Get parameters for calling PQsendQueryParams, PQsendQueryPrepared,
   PQexecParams, and PQexecPrepared.  For these functions, the SQL
   uses $1, $2, ... to represent the parameters, so we map the gawk
   associative array values array[1] -> $1, array[2] -> $2, etc.
   Note that the C API uses C arrays which start with subscript 0,
   so be careful about off-by-one errors. */
static int
get_params(unsigned int nargs, unsigned int argnum, const char ***pvp)
{
  awk_value_t paramValues_array;
  const char **paramValues;
  int nParams;

  {
    awk_value_t nParams_node;
    if (!get_argument(argnum, AWK_NUMBER, &nParams_node))
      return -1;
    nParams = nParams_node.num_value;
  }

  if (nParams < 0)
    return nParams;

  if (!nParams || (argnum+1 >= nargs))
    paramValues = NULL;
  else {
    int i;
    if (!get_argument(argnum+1, AWK_ARRAY, &paramValues_array))
      return -1;
    emalloc(paramValues, const char **, nParams*sizeof(*paramValues),
	    "get_params");
    for (i = 0; i < nParams; i++) {
      awk_value_t idx;
      awk_value_t val;
      /* Must add one when doing the array lookup because, for example, we
         must set paramValues[0] to the value for $1 (which is stored in
	 the gawk array with subscript [1]) */
      if (!get_array_element(paramValues_array.array_cookie,
      			     make_number(i+1, &idx), AWK_STRING, &val))
        paramValues[i] = NULL;
      else
	paramValues[i] = val.str_value.str;
    }
  }
  *pvp = paramValues;
  return nParams;
}

static awk_value_t *
do_pg_sendquery(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  int res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_sendquery: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_sendquery called with unknown connection handle"));
    RET_NUM(0);
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_sendquery 2nd argument should be a string"));
    RET_NUM(0);
  }

  res = PQsendQuery(conn, command.str_value.str);
  if (!res)
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(res);
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

static awk_value_t *
do_pg_sendprepare(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  char *stmtName;
  int res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_sendprepare: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_sendprepare called with unknown connection handle"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_sendprepare 2nd argument should be a string"));
    RET_NULSTR;
  }

  res = PQsendPrepare(conn, (stmtName = prep_name()),
		      command.str_value.str, 0, NULL);

  if (!res) {
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
    RET_NULSTR;
  }
  return make_string_malloc(stmtName, strlen(stmtName), result);
}

static awk_value_t *
do_pg_sendqueryparams(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  int nParams;
  const char **paramValues;
  int res;

  if (do_lint && (nargs > 4))
    lintwarn(ext_id, _("pg_sendqueryparams: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_sendqueryparams called with unknown connection handle"));
    RET_NUM(0);
  }

  if ((nParams = get_params(nargs, 2, &paramValues)) < 0) {
    set_ERRNO(_("pg_sendqueryparams called with negative nParams"));
    RET_NUM(0);
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_sendqueryparams 2nd argument should be a string"));
    RET_NUM(0);
  }

  res = PQsendQueryParams(conn, command.str_value.str, nParams,
  			  NULL, paramValues, NULL, NULL, 0);
  if (paramValues)
    free(paramValues);

  if (!res)
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(res);
}

static awk_value_t *
do_pg_sendqueryprepared(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  int nParams;
  const char **paramValues;
  int res;

  if (do_lint && (nargs > 4))
    lintwarn(ext_id, _("pg_sendqueryprepared: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_sendqueryprepared called with unknown connection handle"));
    RET_NUM(0);
  }

  if ((nParams = get_params(nargs, 2, &paramValues)) < 0) {
    set_ERRNO(_("pg_sendqueryprepared called with negative nParams"));
    RET_NUM(0);
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_sendqueryprepared 2nd argument should be a string"));
    RET_NUM(0);
  }

  res = PQsendQueryPrepared(conn, command.str_value.str, nParams,
			    paramValues, NULL, NULL, 0);
  if (paramValues)
    free(paramValues);

  if (!res)
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(res);
}

static awk_value_t *
do_pg_putcopydata(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t buffer;
  int res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_putcopydata: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_putcopydata called with unknown connection handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_STRING, &buffer)) {
    set_ERRNO(_("pg_putcopydata 2nd argument should be a string"));
    RET_NUM(-1);
  }

  res = PQputCopyData(conn, buffer.str_value.str, buffer.str_value.len);

  if (res < 0)
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(res);
}

static awk_value_t *
do_pg_putcopyend(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t emsg;
  int res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_putcopyend: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_putcopyend called with unknown connection handle"));
    RET_NUM(-1);
  }

  if (nargs > 1) {
    if (!get_argument(1, AWK_STRING, &emsg)) {
      set_ERRNO(_("pg_putcopyend optional 2nd argument should be a string"));
      RET_NUM(-1);
    }
  }
  else
    emsg.str_value.str = NULL;

  res = PQputCopyEnd(conn, emsg.str_value.str);
  if (res < 0)
    /* connection is probably bad */
    set_ERRNO(PQerrorMessage(conn));
  RET_NUM(res);
}

static awk_value_t *
do_pg_getcopydata(int nargs, awk_value_t *result)
{
  PGconn *conn;
  char *buffer;
  int rc;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_getcopydata: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_getcopydata called with unknown connection handle"));
    RET_NULSTR;
  }

  buffer = NULL;
  switch (rc = PQgetCopyData(conn, &buffer, FALSE)) {
  /* case 0 can only happen if async is TRUE */
  case -1: /* copy done */
    make_null_string(result);
    unset_ERRNO();
    break;
  case -2: /* error */
    make_null_string(result);
    {
      const char *emsg = PQerrorMessage(conn);
      if (emsg)
        set_ERRNO(PQerrorMessage(conn));
      else
        set_ERRNO(_("PQgetCopyData failed, but no error message is available"));
    }
    break;
  default: /* rc should be positive and equal # of bytes in row */
    if (rc > 0) {
      make_string_malloc(buffer, rc, result);
      unset_ERRNO();
    }
    else {
      /* this should not happen */
      char buf[512];
      make_null_string(result);
      snprintf(buf, sizeof(buf),
	       _("PQgetCopyData returned invalid value %d: %s"),
	       rc, PQerrorMessage(conn));
      set_ERRNO(buf);
    }
  }

  if (buffer)
    PQfreemem(buffer);
  return result;
}

static awk_value_t *
set_error(PGconn *conn, ExecStatusType status, awk_value_t *result)
{
  char buf[100];
  snprintf(buf, sizeof(buf), "ERROR %s%s",
	   ((PQstatus(conn) != CONNECTION_OK) ? "BADCONN " : ""),
	   PQresStatus(status));
  return make_string_malloc(buf, strlen(buf), result);
}

static awk_value_t *
process_result(PGconn *conn, PGresult *res, awk_value_t *resp)
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
      make_string_malloc(handle, sl, resp);
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
      make_string_malloc(result, strlen(result), resp);
    }
    break;
  case PGRES_COPY_IN:
    {
      char buf[100];
      snprintf(buf, sizeof(buf), "COPY_IN %d %s",
	       PQnfields(res), (PQbinaryTuples(res) ? "BINARY" : "TEXT"));
      make_string_malloc(buf, strlen(buf), resp);
      PQclear(res);
    }
    break;
  case PGRES_COPY_OUT:
    {
      char buf[100];
      snprintf(buf, sizeof(buf), "COPY_OUT %d %s",
	       PQnfields(res), (PQbinaryTuples(res) ? "BINARY" : "TEXT"));
      make_string_malloc(buf, strlen(buf), resp);
      PQclear(res);
    }
    break;
  default: /* error */
    set_error(conn, rc, resp);
    set_ERRNO(PQresultErrorMessage(res));
    PQclear(res);
  }
  return resp;
}

static awk_value_t *
do_pg_getresult(int nargs, awk_value_t *result)
{
  PGconn *conn;
  PGresult *res;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_getresult: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_getresult called with unknown connection handle"));
    RET_NULSTR;
  }

  if (!(res = PQgetResult(conn)))
    /* this just means there are no results currently available, so it is
       not necessarily an error */
    RET_NULSTR;
  return process_result(conn, res, result);
}

static awk_value_t *
do_pg_exec(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  PGresult *res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_exec: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_exec called with unknown connection handle"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_exec 2nd argument should be a string"));
    RET_NULSTR;
  }
  res = PQexec(conn, command.str_value.str);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL), result);
    set_ERRNO(PQerrorMessage(conn));
    return result;
  }
  return process_result(conn, res, result);
}

static awk_value_t *
do_pg_prepare(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  char *stmtName;
  PGresult *res;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_prepare: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_prepare called with unknown connection handle"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_prepare 2nd argument should be a string"));
    RET_NULSTR;
  }

  res = PQprepare(conn, (stmtName = prep_name()), command.str_value.str,
		  0, NULL);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_ERRNO(PQerrorMessage(conn));
    RET_NULSTR;
  }

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    set_ERRNO(PQresultErrorMessage(res));
    PQclear(res);
    RET_NULSTR;
  }

  PQclear(res);
  return make_string_malloc(stmtName, strlen(stmtName), result);
}

static awk_value_t *
do_pg_execparams(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  int nParams;
  const char **paramValues;
  PGresult *res;

  if (do_lint && (nargs > 4))
    lintwarn(ext_id, _("pg_execparams: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_execparams called with unknown connection handle"));
    RET_NULSTR;
  }

  if ((nParams = get_params(nargs, 2, &paramValues)) < 0) {
    set_ERRNO(_("pg_execparams called with negative nParams"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_execparams 2nd argument should be a string"));
    RET_NULSTR;
  }

  res = PQexecParams(conn, command.str_value.str, nParams,
		     NULL, paramValues, NULL, NULL, 0);
  if (paramValues)
    free(paramValues);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL), result);
    set_ERRNO(PQerrorMessage(conn));
    return result;
  }
  return process_result(conn, res, result);
}

static awk_value_t *
do_pg_execprepared(int nargs, awk_value_t *result)
{
  PGconn *conn;
  awk_value_t command;
  int nParams;
  const char **paramValues;
  PGresult *res;

  if (do_lint && (nargs > 4))
    lintwarn(ext_id, _("pg_execprepared: called with too many arguments"));

  if (!(conn = find_handle(conns, 0))) {
    set_ERRNO(_("pg_execprepared called with unknown connection handle"));
    RET_NULSTR;
  }

  if ((nParams = get_params(nargs, 2, &paramValues)) < 0) {
    set_ERRNO(_("pg_execprepared called with negative nParams"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_STRING, &command)) {
    set_ERRNO(_("pg_execprepared 2nd argument should be a string"));
    RET_NULSTR;
  }

  res = PQexecPrepared(conn, command.str_value.str, nParams, paramValues, NULL, NULL, 0);
  if (paramValues)
    free(paramValues);

  if (!res) {
    /* I presume the connection is probably bad, since no result returned */
    set_error(conn, PQresultStatus(NULL), result);
    set_ERRNO(PQerrorMessage(conn));
    return result;
  }
  return process_result(conn, res, result);
}

static awk_value_t *
do_pg_clear(int nargs, awk_value_t *result)
{
  awk_value_t handle;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_clear: called with too many arguments"));

  if (!get_argument(0, AWK_STRING, &handle)) {
    set_ERRNO(_("pg_clear argument should be a string handle"));
    RET_NUM(-1);
  }

  if (strhash_delete(results, handle.str_value.str, handle.str_value.len,
		     (strhash_delete_func)PQclear, NULL) < 0) {
    set_ERRNO(_("pg_clear called with unknown result handle"));
    RET_NUM(-1);
  }
  RET_NUM(0);
}

static awk_value_t *
do_pg_ntuples(int nargs, awk_value_t *result)
{
  PGresult *res;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_ntuples: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_ntuples called with unknown result handle"));
    RET_NUM(-1);
  }
  RET_NUM(PQntuples(res));
}

static awk_value_t *
do_pg_nfields(int nargs, awk_value_t *result)
{
  PGresult *res;

  if (do_lint && (nargs > 1))
    lintwarn(ext_id, _("pg_nfields: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_nfields called with unknown result handle"));
    RET_NUM(-1);
  }
  RET_NUM(PQnfields(res));
}

static awk_value_t *
do_pg_fname(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t colarg;
  int col;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_fname: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_fname called with unknown result handle"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_NUMBER, &colarg)) {
    set_ERRNO(_("pg_fname: 2nd argument must be a number"));
    RET_NULSTR;
  }
  col = colarg.num_value;

  if ((col < 0) || (col >= PQnfields(res))) {
    set_ERRNO(_("pg_fname: 2nd argument col_number is out of range"));
    RET_NULSTR;
  }

  {
    char *fname = PQfname(res, col);
    return make_string_malloc(fname, strlen(fname), result);
  }
}

static awk_value_t *
do_pg_fields(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t array;
  int nf;
  int col;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_fields: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_fields called with unknown result handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_ARRAY, &array)) {
    set_ERRNO(_("pg_fields 2nd argument must be an array"));
    RET_NUM(-1);
  }
  clear_array(array.array_cookie);

  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    char *fname;
    awk_value_t idx, val;

    fname = PQfname(res, col);
    set_array_element(array.array_cookie, make_number(col, &idx),
		      make_string_malloc(fname, strlen(fname), &val));
  }
  RET_NUM(nf);
}

static awk_value_t *
do_pg_fieldsbyname(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t array;
  int nf;
  int col;

  if (do_lint && (nargs > 2))
    lintwarn(ext_id, _("pg_fieldsbyname: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_fieldsbyname called with unknown result handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_ARRAY, &array)) {
    set_ERRNO(_("pg_fieldsbyname 2nd argument must be an array"));
    RET_NUM(-1);
  }
  clear_array(array.array_cookie);

  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    char *fname;
    awk_value_t idx, val;

    fname = PQfname(res, col);
    set_array_element(array.array_cookie,
		      make_string_malloc(fname, strlen(fname), &idx),
		      make_number(col, &val));
  }
  RET_NUM(nf);
}

static awk_value_t *
do_pg_getvalue(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t rowarg, colarg;
  int row;
  int col;

  if (do_lint && (nargs > 3))
    lintwarn(ext_id, _("pg_getvalue: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_getvalue called with unknown result handle"));
    RET_NULSTR;
  }

  if (!get_argument(1, AWK_NUMBER, &rowarg)) {
    set_ERRNO(_("pg_getvalue: 2nd argument must be a row number"));
    RET_NULSTR;
  }
  row = rowarg.num_value;

  if (!get_argument(2, AWK_NUMBER, &colarg)) {
    set_ERRNO(_("pg_getvalue: 3rd argument must be a column number"));
    RET_NULSTR;
  }
  col = colarg.num_value;

  if ((row < 0) || (row >= PQntuples(res))) {
    set_ERRNO(_("pg_getvalue: 2nd argument row_number is out of range"));
    RET_NULSTR;
  }

  if ((col < 0) || (col >= PQnfields(res))) {
    set_ERRNO(_("pg_getvalue: 3rd argument col_number is out of range"));
    RET_NULSTR;
  }

  {
    char *val = PQgetvalue(res, row, col);
    return make_string_malloc(val, strlen(val), result);
  }
}

static awk_value_t *
do_pg_getisnull(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t rowarg, colarg;
  int row;
  int col;

  if (do_lint && (nargs > 3))
    lintwarn(ext_id, _("pg_getisnull: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_getisnull called with unknown result handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_NUMBER, &rowarg)) {
    set_ERRNO(_("pg_getisnull: 2nd argument must be a row number"));
    RET_NUM(-1);
  }
  row = rowarg.num_value;

  if (!get_argument(2, AWK_NUMBER, &colarg)) {
    set_ERRNO(_("pg_getisnull: 3rd argument must be a column number"));
    RET_NUM(-1);
  }
  col = colarg.num_value;

  if ((row < 0) || (row >= PQntuples(res))) {
    set_ERRNO(_("pg_getisnull: 2nd argument row_number is out of range"));
    RET_NUM(-1);
  }

  if ((col < 0) || (col >= PQnfields(res))) {
    set_ERRNO(_("pg_getisnull: 3rd argument col_number is out of range"));
    RET_NUM(-1);
  }

  RET_NUM(PQgetisnull(res, row, col));
}

static awk_value_t *
do_pg_getrow(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t array;
  awk_value_t rowarg;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (nargs > 3))
    lintwarn(ext_id, _("pg_getrow: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_getrow called with unknown result handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_NUMBER, &rowarg)) {
    set_ERRNO(_("pg_getrow: 2nd argument must be a row number"));
    RET_NUM(-1);
  }
  row = rowarg.num_value;

  if ((row < 0) || (row >= PQntuples(res))) {
    set_ERRNO(_("pg_getrow: 2nd argument row_number is out of range"));
    RET_NUM(-1);
  }

  if (!get_argument(2, AWK_ARRAY, &array)) {
    set_ERRNO(_("pg_getrow 3rd argument must be an array"));
    RET_NUM(-1);
  }
  clear_array(array.array_cookie);

  found = 0;
  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    if (!PQgetisnull(res, row, col)) {
      char *val;
      awk_value_t idx, value;

      val = PQgetvalue(res, row, col);
      set_array_element(array.array_cookie, make_number(col, &idx),
			make_string_malloc(val, strlen(val), &value));
      found++;
    }
  }
  RET_NUM(found);
}

static awk_value_t *
do_pg_getrowbyname(int nargs, awk_value_t *result)
{
  PGresult *res;
  awk_value_t array;
  awk_value_t rowarg;
  int row;
  int nf;
  int found;
  int col;

  if (do_lint && (nargs > 3))
    lintwarn(ext_id, _("pg_getrowbyname: called with too many arguments"));

  if (!(res = find_handle(results, 0))) {
    set_ERRNO(_("pg_getrowbyname called with unknown result handle"));
    RET_NUM(-1);
  }

  if (!get_argument(1, AWK_NUMBER, &rowarg)) {
    set_ERRNO(_("pg_getrowbyname: 2nd argument must be a row number"));
    RET_NUM(-1);
  }
  row = rowarg.num_value;

  if ((row < 0) || (row >= PQntuples(res))) {
    set_ERRNO(_("pg_getrowbyname: 2nd argument row_number is out of range"));
    RET_NUM(-1);
  }

  if (!get_argument(2, AWK_ARRAY, &array)) {
    set_ERRNO(_("pg_getrowbyname 3rd argument must be an array"));
    RET_NUM(-1);
  }
  clear_array(array.array_cookie);

  found = 0;
  nf = PQnfields(res);
  for (col = 0; col < nf; col++) {
    if (!PQgetisnull(res, row, col)) {
      char *fname;
      char *val;
      awk_value_t idx, value;

      fname = PQfname(res, col);
      val = PQgetvalue(res, row, col);
      set_array_element(array.array_cookie,
      			make_string_malloc(fname, strlen(fname), &idx),
			make_string_malloc(val, strlen(val), &value));
      found++;
    }
  }
  RET_NUM(found);
}

/* Wrappers for libpq functions: */
static awk_ext_func_t func_table[] = {
  { "pg_connect", do_pg_connect, 1},
  { "pg_connectdb", do_pg_connect, 1},  /* alias for pg_connect */
  { "pg_serverversion", do_pg_serverversion, 1},
  { "pg_errormessage", do_pg_errormessage, 1},
  { "pg_sendquery", do_pg_sendquery, 2},
  { "pg_sendqueryparams", do_pg_sendqueryparams, 4},
  { "pg_sendprepare", do_pg_sendprepare, 2},
  { "pg_sendqueryprepared", do_pg_sendqueryprepared, 4},
  { "pg_exec", do_pg_exec, 2},
  { "pg_execparams", do_pg_execparams, 4},
  { "pg_prepare", do_pg_prepare, 2},
  { "pg_execprepared", do_pg_execprepared, 4},
  { "pg_nfields", do_pg_nfields, 1},
  { "pg_ntuples", do_pg_ntuples, 1},
  { "pg_fname", do_pg_fname, 2},
  { "pg_getvalue", do_pg_getvalue, 3},
  { "pg_getisnull", do_pg_getisnull, 3},
  { "pg_clear", do_pg_clear, 1},
  { "pg_disconnect", do_pg_disconnect, 1},
  { "pg_finish", do_pg_disconnect, 1},  /* alias for pg_disconnect */
  { "pg_reset", do_pg_reset, 1},
  { "pg_reconnect", do_pg_reset, 1},  /* alias for pg_reset */
  { "pg_getresult", do_pg_getresult, 1},
  { "pg_putcopydata", do_pg_putcopydata, 2},
  { "pg_putcopyend", do_pg_putcopyend, 2},
  { "pg_getcopydata", do_pg_getcopydata, 1},

  /* Higher-level functions using awk associative arrays: */
  { "pg_fields", do_pg_fields, 2},
  { "pg_fieldsbyname", do_pg_fieldsbyname, 2},
  { "pg_getrow", do_pg_getrow, 3},
  { "pg_getrowbyname", do_pg_getrowbyname, 3},
};

static awk_bool_t
init_my_module(void)
{
  /* strhash_create exits on failure, so no need to check return code */
  conns = strhash_create(0);
  results = strhash_create(0);
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char ext_version[] = "PostgreSQL extension: version 1.0";

dl_load_func(func_table, pgsql, "")
