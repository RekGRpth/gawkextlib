BEGIN {
  # test every pgsql function call to make sure everthing works!
  if ((conn = pg_connect(PQCONNINFO)) == "") {
    printf "pg_connect(%s) failed: %s\n", PQCONNINFO, ERRNO > "/dev/stderr"
    exit 1
  }
}
