BEGIN {
	fname = "./test_dict_cursor.lmdb"
	if ((env = mdb_env_create()) == "") {
		printf "mdb_env_create failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if (mdb_env_open(env, fname,
			 or(MDB["NOSUBDIR"], MDB["NOSYNC"], MDB["NOLOCK"]),
			 0600) != MDB_SUCCESS) {
		printf "mdb_env_open failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if ((txn = mdb_txn_begin(env, "", 0)) == "") {
		printf "mdb_txn_begin failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if ((dbi = mdb_dbi_open(txn, "", MDB["CREATE"])) == "") {
		printf "mdb_dbi_open failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if (mdb_txn_env(txn) != env)
		print "Error, mdb_txn_env not working properly"
	if ((cursor = mdb_cursor_open(txn, dbi)) == "") {
		printf "Error: mdb_cursor_open failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if (mdb_cursor_dbi(cursor) != dbi)
		print "Error, mdb_cursor_dbi not working properly"
	if (mdb_cursor_txn(cursor) != txn)
		print "Error, mdb_cursor_txn not working properly"
}

NF > 0 {
	switch ($1) {
	case "i":
		if (mdb_cursor_put(cursor, $2, $3, MDB["NOOVERWRITE"]) != MDB_SUCCESS)
			printf "Warning: mdb_cursor_put(%s, %s) failed: %s [%s]\n",
			       $2, $3, mdb_strerror(MDB_ERRNO), ERRNO
		break
	case "d":
		# first position the cursor
		f[MDB_KEY] = $2
		if (mdb_cursor_get(cursor, f, MDB["SET"]) != MDB_SUCCESS)
			printf "Warning: cannot delete because mdb_cursor_get(%s) failed: %s [%s]\n",
			       $2, mdb_strerror(MDB_ERRNO), ERRNO
		else if (mdb_cursor_del(cursor, 0) != MDB_SUCCESS)
			printf "Error: mdb_cursor_del(%s) failed: %s [%s]\n",
			       $2, mdb_strerror(MDB_ERRNO), ERRNO
		break
	case "l":
		f[MDB_KEY] = $2
		if (mdb_cursor_get(cursor, f, MDB["SET"]) != MDB_SUCCESS)
			printf "Warning: mdb_cursor_get(%s) failed: %s [%s]\n",
			       $2, mdb_strerror(MDB_ERRNO), ERRNO
		else
			printf "lookup(%s) = [%s]\n", $2, f[MDB_DATA]
		break
	case "dump":
		print "DUMP started:"
		for (which = MDB["FIRST"];
		     mdb_cursor_get(cursor, f, which) == MDB_SUCCESS;
		     which = MDB["NEXT"])
			printf "[%s] -> [%s]\n", f[MDB_KEY], f[MDB_DATA]
		print "DUMP done."
		if (MDB_ERRNO != MDB["NOTFOUND"])
			printf "error: dump terminated abnormally: %s [%s]\n",
			       mdb_strerror(MDB_ERRNO), ERRNO
		break
	}
}

END {
	if (mdb_cursor_close(cursor) != MDB_SUCCESS)
		printf "Error: mdb_cursor_close failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
	if (mdb_txn_commit(txn) != MDB_SUCCESS) {
		printf "mdb_txn_commit failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if (mdb_dbi_close(env, dbi) != MDB_SUCCESS) {
		printf "mdb_txn_commit failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	if (mdb_env_close(env, dbi) != MDB_SUCCESS) {
		printf "mdb_env_close failed: %s [%s]\n",
		       mdb_strerror(MDB_ERRNO), ERRNO
		exit 1
	}
	print system("rm -f " fname)
}
