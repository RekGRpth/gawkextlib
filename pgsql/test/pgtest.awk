# check that all pgsql functions work properly!

function insert_record(rec,  i,nf,f,param,res) {
  nf = split(rec, f, "|")
  # insert the record into the database
  for (i = 1; i <= nf; i++) {
    if (f[i] != "") {
      if (f[i] == empty)
	f[i] = ""
      param[i] = f[i]
    }
  }
  if ((res = pg_execprepared(dbconn, insert_statement,
			     ncols, param)) !~ /^OK /) {
    printf "Error: insert failed: %s, ERRNO = %s\n",
	   res, ERRNO > "/dev/stderr"
    exit 1
  }
}

function bulk_insert(rec,  nf,f,i,sql) {
  nf = split(rec, f, "|")
  sql = ""
  for (i = 1; i <= nf; i++) {
    if (sql != "")
      sql = (sql ",")
    if (f[i] != "") {
      if (f[i] == empty)
	f[i] = ""
      sql = (sql "\"" gensub(/"/,"\"\"","g",f[i]) "\"")
    }
  }
  sql = (sql "\n")
  if (pg_putcopydata(dbconn, sql) < 0) {
    printf "Error: pg_putcopydata(%s) failed: %s\n", sql, ERRNO > "/dev/stderr"
    exit 1
  }
}

function do_compare(res,nf,f,   nnull,j,rc,g,expval) {
  nnull = 0
  for (j = 1; j <= nf; j++) {
    if (f[j] == "")
      nnull++
  }
  if (pg_ntuples(res) != 1) {
    printf "Error: query %s returned %d rows != expected 1\n",
	   sql, pg_ntuples(res)
    exit 1
  }

  if ((rc = pg_getrow(res, 0, g)) != (nf-nnull)) {
    printf "Error: pg_getrow(%s) returned %d != expected %d\n",
	   sql, rc, nf-nnull
    exit 1
  }
  for (j = 1; j <= nf; j++) {
    if (f[j] == "") {
      # should be null
      if (!pg_getisnull(res, 0, j-1))
	printf "Error: query %s column %d should be null!\n", sql, j-1
      if ((j-1) in g)
	printf "Error: query %s column %d [%s] is not null\n",
	       sql, j-1, g[j-1]
    }
    else {
      if (pg_getisnull(res, 0, j-1))
	printf "Error: query %s column %d should not be null!\n", sql, j-1
      expval = ((f[j] == empty) ? "" : f[j])
      if (!((j-1) in g))
	printf "Error: query %s column %d is null, not [%s]\n",
	       sql, j-1, expval
      else if (g[j-1] != expval)
	printf "Error: query %s column %d [%s] != expected [%s]\n",
	       sql, j-1, g[j-1], expval
    }
  }

  if ((rc = pg_getrowbyname(res, 0, g)) != (nf-nnull)) {
    printf "Error: pg_getrowbyname(%s) returned %d != expected %d\n",
	   sql, rc, nf-nnull
    exit 1
  }
  for (j = 1; j <= nf; j++) {
    if (f[j] == "") {
      # should be null
      if (col[j] in g)
	printf "Error: query %s column %s [%s] is not null\n",
	       sql, col[j], g[col[j]]
    }
    else {
      expval = ((f[j] == empty) ? "" : f[j])
      if (!(col[j] in g))
	printf "Error: query %s column %s is null, not [%s]\n",
	       sql, col[j], expval
      else if (g[col[j]] != expval)
	printf "Error: query %s column %s [%s] != expected [%s]\n",
	       sql, col[j], g[col[j]], expval
    }
  }

  if (pg_clear(res) < 0) {
    printf "Error: pg_clear(%s) failed, ERRNO = %s\n",
	   res, ERRNO > "/dev/stderr"
    exit 1
  }
}

function proc_async(sql,f,nf,  recsret,res) {
  recsret = 0
  while ((res = pg_getresult(dbconn)) != "") {
    if (res !~ /^TUPLES /) {
      printf "Error: pg_getresult(%s) = %s, ERRNO = %s\n",
	     sql, res, ERRNO > "/dev/stderr"
      exit 1
    }
    if (++recsret != 1) {
      printf "Error: more than one result returned from %s\n", sql
      exit 1
    }
    do_compare(res, nf, f)
  }
}

BEGIN {
  empty = "_"	# an empty (as opposed to NULL) value

  # test every pgsql function call to make sure everthing works!
  if ((dbconn = pg_connect(PQCONNINFO)) == "") {
    printf "pg_connect(%s) failed: %s\n", PQCONNINFO, ERRNO > "/dev/stderr"
    exit 1
  }

  if (pg_reset(dbconn) != 0) {
    printf "pg_reset(%s) failed: %s\n", dbconn, ERRNO > "/dev/stderr"
    exit 1
  }

  if ((version = pg_serverversion(dbconn)) <= 0)
    printf "Error: pg_serverversion returned non-positive value %s\n", version

  sql = "XXXJUNK"
  if ((res = pg_exec(dbconn, sql)) !~ /^ERROR /)
    printf "Error: pg_exec(%s) returned %s (not ERROR)\n", sql, res
  if (ERRNO != (rc = pg_errormessage(dbconn)))
    printf "Error: ERRNO [%s] != pg_errormessage [%s]\n", ERRNO, rc
  if (tolower(ERRNO) !~ /syntax/)
    printf "Error: ERRNO does not mention a syntax error [%s]\n", ERRNO

  # these are the columns in the table
  ncols = split("name email work cell company address", col)

  # create a temporary table
  sql = "CREATE TEMPORARY TABLE tmp ("
  for (i = 1; i <= ncols; i++) {
    if (i > 1)
      sql = (sql ",")
    sql = (sql " " col[i] " varchar")
  }
  sql = (sql ")")
  if ((res = pg_exec(dbconn, sql)) !~ /^OK /) {
    printf "Cannot create temporary table: %s, ERRNO = %s\n",
	   res, ERRNO > "/dev/stderr"
    exit 1
  }

  # create a prepared insert statement
  sql = ("INSERT INTO tmp (" col[1])
  for (i = 2; i <= ncols; i++)
    sql = (sql ", " col[i])
  sql = (sql ") VALUES ($1")
  for (i = 2; i <= ncols; i++)
    sql = (sql ", $" i)
  sql = (sql" )")
  if ((insert_statement = pg_prepare(dbconn, sql)) == "") {
    printf "pg_prepare(%s) failed: %s\n",sql,ERRNO > "/dev/stderr"
    exit 1
  }

  rec[0] = "Joe Smith|joe.smith@acme.com|1-212-555-1212|1-917-555-1212|Acme|32 Maple St., New York, NY"
  rec[1] = "Ellen Jones|ellen.jones@widget.com|1-310-555-1212||Widget Inc.|137 Main St., Los Angeles, CA"
  rec[2] = ("Ralph Simpson||" empty \
	    "|1-773-555-1212|General Motors|13 Elm St., Chicago, IL")

  rec[3] = ("Ronald Reagan|ronald.reagan@whitehouse.gov|1-202-555-1212|" empty \
	    "|\"POTUS\"|1600 Pennsylvania Ave NW, Washington, DC 20500")
  rec[4] = "Jimmy Carter|jimmy.carter@whitehouse.gov||1-999-555-1212|Peanut Farmer|Plains, GA"

  for (i = 0; i <= 2; i++)
    insert_record(rec[i])

  # start bulk copy insert (faster than individual inserts)
  sql = ("COPY tmp ( " col[1])
  for (i = 2; i <= ncols; i++)
    sql = (sql ", " col[i])
  sql = (sql ") FROM STDIN WITH CSV")
  if (!((res = pg_exec(dbconn, sql)) ~ /^COPY_IN /)) {
    printf "Error: Cannot start copy command [%s]: %s, ERRNO = %s\n",
	   sql, res, ERRNO > "/dev/stderr"
    exit 1
  }
  
  for (i = 3; i <= 4; i++)
    bulk_insert(rec[i])

  # end bulk copy
  if (pg_putcopyend(dbconn) < 0) {
    printf "Error: pg_putcopyend failed: %s\n", ERRNO > "/dev/stderr"
    exit 1
  }
  if (!((res = pg_getresult(dbconn)) ~ /^OK /)) {
    printf "ERROR: copy failed: %s, ERRNO = %s\n", res, ERRNO > "/dev/stderr"
    exit 1
  }

  # let's take a look at what we have accomplished
  if ((res = pg_exec(dbconn, "SELECT * FROM tmp ORDER BY name")) !~ /^TUPLES /) {
    printf "Error selecting * from tmp: %s, ERRNO = %s\n",
	   res, ERRNO > "/dev/stderr"
    exit 1
  }

  # print query results
  nf = pg_nfields(res)
  for (i = 0; i < nf; i++) {
    if (i > 0)
      printf "|"
    printf "%s", pg_fname(res, i)
  }
  printf "\n"
  nr = pg_ntuples(res)
  for (row = 0; row < nr; row++) {
    for (i = 0; i < nf; i++) {
      if (i > 0)
	printf "|"
      printf "%s",
	     (pg_getisnull(res,row,i) ? "<NULL>" : pg_getvalue(res,row,i))
    }
    printf "\n"
  }

  # more checks on query results to exercise additional functions
  if ((nf = pg_fields(res, f)) != ncols)
    printf "Error: pg_fields returned %d cols != expected %d\n", nf, ncol
  for (i = 0; i < nf; i++) {
    if (f[i] != col[i+1])
      printf "Error: pg_fields[%d] = %s != expected %s\n", i, f[i], col[i+1]
  }

  if ((nf = pg_fieldsbyname(res, f)) != ncols)
    printf "Error: pg_fieldsbyname returned %d cols != expected %d\n", nf, ncol
  for (i = 1; i <= ncols; i++) {
    if (!(col[i] in f))
      printf "Error: cannot find column %s in pg_fieldsbyname output\n", col[i]
    else if (f[col[i]] != i-1)
      printf "Error: pg_fieldsbyname[%s] = %d != expected %d\n",
	     col[i], f[col[i]], i-1
  }

  # release query results
  if (pg_clear(res) < 0) {
    printf "Error: pg_clear(%s) failed, ERRNO = %s\n",
    	   res, ERRNO > "/dev/stderr"
    exit 1
  }

  prepsql = "SELECT * FROM tmp WHERE name = $1 AND address = $2"
  if ((hdl = pg_sendprepare(dbconn, prepsql)) == "") {
    printf "Error: pg_sendprepare(%s) failed, ERRNO = %s\n",
	   prepsql, ERRNO > "/dev/stderr"
    exit 1
  }
  if ((res = pg_getresult(dbconn)) !~ /^OK /) {
    printf "Error: pg_getresult(prepare %s) returned %s, ERRNO = %s\n",
	   prepsql, res, ERRNO > "/dev/stderr"
    exit 1
  }
  if ((res = pg_getresult(dbconn)) != "") {
    printf "Error: pg_getresult(prepare %s) also returned %s, ERRNO = %s\n",
	   prepsql, res, ERRNO > "/dev/stderr"
    exit 1
  }

  # test various query methods
  for (i in rec) {
    nf = split(rec[i], f, "|")
    sql = ("SELECT * FROM tmp WHERE name = '" f[1] "'")
    if (pg_sendquery(dbconn, sql) < 0) {
      printf "Error: pg_sendquery(%s) failed, ERRNO = %s\n",
	     sql, ERRNO > "/dev/stderr"
      exit 1
    }
    proc_async(sql, f, nf)

    delete param
    param[1] = f[1]
    param[2] = f[nf]
    if ((res = pg_execparams(dbconn, prepsql, 2, param)) !~ /^TUPLES /) {
      printf "Error: pg_execparams(%s, %s) failed: %s, ERRNO = %s\n",
	     prepsql, param[1], res, ERRNO > "/dev/stderr"
      exit 1
    }
    do_compare(res, nf, f)

    if (pg_sendqueryparams(dbconn, prepsql, 2, param) < 0) {
      printf "Error: pg_sendqueryparams(%s) failed, ERRNO = %s\n",
	     prepsql, ERRNO > "/dev/stderr"
      exit 1
    }
    proc_async(prepsql, f, nf)

    if (pg_sendqueryprepared(dbconn, hdl, 2, param) < 0) {
      printf "Error: pg_sendqueryprepared(%s) failed, ERRNO = %s\n",
	     prepsql, ERRNO > "/dev/stderr"
      exit 1
    }
    proc_async(prepsql, f, nf)
  }

  # bulk data copy 
  sql = "COPY tmp to STDOUT WITH CSV"
  if (!((res = pg_exec(dbconn, sql)) ~ /^COPY_OUT /)) {
    printf "Error: Cannot start copy command [%s]: %s, ERRNO = %s\n",
	   sql, res, ERRNO > "/dev/stderr"
    exit 1
  }
  print res
  nr = 0
  while ((data = pg_getcopydata(dbconn)) != "")
    printf "copy row %d: %s", nr++, data
  if (ERRNO != "")
    printf "Warning: pg_getcopydata returned ERRNO = %s\n", ERRNO
  if ((res = pg_getresult(dbconn)) !~ /^OK /) {
    printf "ERROR: copy failed: %s, ERRNO = %s\n", res, ERRNO > "/dev/stderr"
    exit 1
  }

  badconn = (dbconn "_invalid_junk")
  if ((rc = pg_disconnect(badconn)) != -1)
    printf "Error: pg_disconnect(invalid handle %s) returned %s\n", badconn, rc
  if (pg_finish(dbconn) < 0) {
    printf "pg_finish(%s) failed: %s\n", dbconn, ERRNO > "/dev/stderr"
    exit 1
  }
}
