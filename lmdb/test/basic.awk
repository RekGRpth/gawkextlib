BEGIN {
	printf "MDB_SUCCESS = %s\n", MDB_SUCCESS
	printf "MDB_KEY = %s\n", MDB_KEY
	printf "MDB_DATA = %s\n", MDB_DATA
	printf "MDB_ERRNO = %s\n", MDB_ERRNO
	print (length(MDB) >= 68)
	print MDB["SUCCESS"]
	print (MDB["NOSUBDIR"] > 0)

	ERRNO = ""
	x = mdb_version(f)
	print ERRNO
	print (MDB_ERRNO == MDB_SUCCESS)
	print (x ~ /LMDB/)
	n = asorti(f, g)
	for (i = 1; i <= n; i++)
		print g[i]

	ERRNO = ""
	print "\nmdb_env_create()"
	env = mdb_env_create()
	print mdb_strerror(MDB_ERRNO)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_set_mapsize(fubar, 1000000)",
	      (rc = mdb_env_set_mapsize("fubar", 1000000))
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_set_mapsize(dbenv, -5)",
	      (rc = mdb_env_set_mapsize(env, -5))
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_set_mapsize(env, 1000000.5)",
	      (rc = mdb_env_set_mapsize(env, 1000000.5))
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_set_mapsize(env, 1000000)",
	      (rc = mdb_env_set_mapsize(env, 1000000))
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_open(fubar)"
	rc = mdb_env_open("fubar")
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_open(fubar, /dev/null)"
	rc = mdb_env_open("fubar", "/dev/null", or(MDB["NOSUBDIR"], MDB["NOSYNC"], MDB["NOLOCK"]), 0644)
	print mdb_strerror(rc)
	print ERRNO

	ERRNO = ""
	print "\nmdb_env_open(env, /dev/null)"
	rc = mdb_env_open(env, "/dev/null", or(MDB["NOSUBDIR"], MDB["NOSYNC"], MDB["NOLOCK"]), 0644)
	print (mdb_strerror(rc) ~ /device/)
	print ERRNO
}
