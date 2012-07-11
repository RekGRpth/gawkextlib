@load "xml"
@load "pgsql"

BEGIN {
  # Note: should pass an argument to pg_connect containing PQconnectdb
  # options, as discussed here:
  #  http://www.postgresql.org/docs/8.0/interactive/libpq.html#LIBPQ-CONNECT
  # Or the parameters can be set in the environment, as discussed here:
  #  http://www.postgresql.org/docs/8.0/interactive/libpq-envars.html
  # For example, a typical call might be:
  #   pg_connect("host=pgsql_server dbname=my_database")
  if ((dbconn = pg_connect()) == "") {
    printf "pg_connect failed: %s\n", ERRNO > "/dev/stderr"
    exit 1
  }

  # these are the columns in the table
  ncols = split("name email work cell company address", col)

  # create a temporary table
  sql = "CREATE TEMPORARY TABLE tmp ("
  for (i = 1; i <= ncols; i++) {
    if (i > 1)
      sql = (sql",")
    sql = (sql" "col[i]" varchar")
  }
  sql = (sql")")
  if ((res = pg_exec(dbconn, sql)) !~ /^OK /) {
    printf "Cannot create temporary table: %s, ERRNO = %s\n",
	   res, ERRNO > "/dev/stderr"
    exit 1
  }

  # create a prepared insert statement
  sql = ("INSERT INTO tmp ("col[1])
  for (i = 2; i <= ncols; i++)
    sql = (sql", "col[i])
  sql = (sql") VALUES ($1")
  for (i = 2; i <= ncols; i++)
    sql = (sql", $"i)
  sql = (sql")")
  if ((insert_statement = pg_prepare(dbconn, sql)) == "") {
    printf "pg_prepare(%s) failed: %s\n",sql,ERRNO > "/dev/stderr"
    exit 1
  }
}

{
  switch (XMLEVENT) {
    case "STARTELEM":
      if ("type" in XMLATTR)
        item[XMLPATH] = XMLATTR["type"]
      else
        item[XMLPATH] = XMLNAME
      break
    case "CHARDATA":
      if ($1 != "")
        data[item[XMLPATH]] = (data[item[XMLPATH]] $0)
      break
    case "ENDELEM":
      if (XMLNAME == "contact") {
	# insert the record into the database
	for (i = 1; i <= ncols; i++) {
	  if (col[i] in data)
	    param[i] = data[col[i]]
        }
	if ((res = pg_execprepared(dbconn, insert_statement,
				   ncols, param)) !~ /^OK /) {
	  printf "Error -- insert failed: %s, ERRNO = %s\n",
	         res, ERRNO > "/dev/stderr"
	  exit 1
        }
	delete item
	delete data
	delete param
      }
      break
  }
}

END {
  if (dbconn != "") {
    # let's take a look at what we have accomplished
    if ((res = pg_exec(dbconn, "SELECT * FROM tmp")) !~ /^TUPLES /)
      printf "Error selecting * from tmp: %s, ERRNO = %s\n",
	     res, ERRNO > "/dev/stderr"
    else {
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
    }
    pg_disconnect(dbconn)
  }
}
