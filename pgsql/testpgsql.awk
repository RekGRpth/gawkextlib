@load "pgsql"

# This is intended to demonstrate usage of the gawk postgresql interface.
# The API is copied from the libpq C API (but of course adapted to gawk).

# This code implements a command-line interface somewhat similar to
# the psql program in the postgresql distribution.  Basically, it just reads
# SQL commands from stdin and prints the results on stdout.  Note that it
# attempts to support bulk copies.  When executing a COPY FROM command,
# it reads data from stdin until it reads a line containing ":COPY EOF:".
# It treats that as a sentinel indicating that the copy should be terminated.

BEGIN {
  # Note: should pass an argument to pg_connect containing PQconnectdb
  # options, as discussed here:
  #  http://www.postgresql.org/docs/8.0/interactive/libpq.html#LIBPQ-CONNECT
  # Or the parameters can be set in the environment, as discussed here:
  #  http://www.postgresql.org/docs/8.0/interactive/libpq-envars.html
  # For example, a typical call might be:
  #   pg_connect("host=pgsql_server dbname=my_database")
  if ((conn = pg_connect()) == "") {
    printf "pg_connect failed: %s\n", ERRNO > "/dev/stderr"
    exit 1
  }
}

function process_result(res,   nf, nr, col, row, rowdata)
{
  printf "RESULT = %s\n", res
  if (res ~ /^ERROR/) {
    printf "error message: %s\n", ERRNO > "/dev/stderr"
    if (res ~ /^ERROR BADCONN/) {
      print "Attempting to reset the connection." > "/dev/stderr"
      if (pg_reset(conn) < 0) {
	printf "pg_reset failed: %s\n", ERRNO > "/dev/stderr"
	exit 1
      }
    }
    return
  }
  if (res ~ /^OK/)
    return
  if (res ~ /^TUPLES /) {
     nf = pg_nfields(res)
     for (col = 0; col < nf; col++) {
       if (col > 0)
         printf "|"
       printf "%s", pg_fname(res, col)
     }
     printf "\n"
     nr = pg_ntuples(res)
     for (row = 0; row < nr; row++) {
       for (col = 0; col < nf; col++) {
	 if (col > 0)
	   printf "|"
	 printf "%s",
	 	(pg_getisnull(res,row,col) ? "<NULL>" : pg_getvalue(res,row,col))

       }
       printf "\n"
     }
     pg_clear(res)
     return
  }
  if (res ~ /^COPY_OUT /) {
    row = 0
    while ((rowdata = pg_getcopydata(conn)) != "")
      printf "COPY OUT row %d = %s\n", row++, rowdata
    if (ERRNO != "")
      printf "Warning: pg_getcopydata returned error: %s\n",
	     ERRNO > "/dev/stderr"
    printf "COPY OUT completed after %d rows, check next result for status\n",
    	   row
    return
  }
  if (res ~ /^COPY_IN /) {
    row = 0
    while (1) {
      if (getline <= 0) {
        print "unexpected EOF during COPY IN" > "/dev/stderr"
	exit 2
      }
      if ($0 == ":COPY EOF:")
        break
      if (pg_putcopydata(conn, ($0"\n")) < 0) {
        printf "pg_putcopydata failed: %s\n", ERRNO > "/dev/stderr"
	return
      }
      row++
    }
    if (pg_putcopyend(conn) < 0) {
      printf "pg_putcopyend failed: %s\n", ERRNO > "/dev/stderr"
      return
    }
    printf "COPY IN complete for %d rows, check next result for status\n", row
    return
  }
  printf "unknown result type: %s\n", res > "/dev/stderr"
}

{
  if (pg_sendquery(conn, $0) == 0) {
    printf "pg_sendquery(%s) failed: %s\n", $0, ERRNO > "/dev/stderr"
    print "Attempting to reset the connection..." > "/dev/stderr"
    if (pg_reset(conn) < 0) {
      printf "pg_reset failed: %s\n", $0, ERRNO > "/dev/stderr"
      exit 1
    }
    print "Resending the query..." > "/dev/stderr"
    if (pg_sendquery(conn, $0) == 0) {
      printf "pg_sendquery(%s) failed again after reset: %s\n",
	     $0, ERRNO > "/dev/stderr"
      exit 1
    }
  }
  numres = 0
  while ((res = pg_getresult(conn)) != "") {
    process_result(res)
    numres++
  }
  if (numres+0 < 1) {
    printf "Warning: no results returned from query: %s\n", $0 > "/dev/stderr"
    printf "current error message: %s\n", pg_errormessage(conn) > "/dev/stderr"
  }
}

END {
  if (conn != "")
    pg_disconnect(conn)
}
