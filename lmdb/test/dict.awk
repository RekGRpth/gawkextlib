BEGIN {
	fname = "./test.lmdb"
	if ((env = mdb_env_create()) == "") {
		printf "mdb_env_create failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	if (mdb_env_open(env, fname,
			 or(MDB["NOSUBDIR"], MDB["NOSYNC"], MDB["NOLOCK"]),
			 0600) != MDB_SUCCESS) {
		printf "mdb_env_open failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	if ((txn = mdb_txn_begin(env, "", 0)) == "") {
		printf "mdb_txn_begin failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	if ((dbi = mdb_dbi_open(txn, "", MDB["CREATE"])) == "") {
		printf "mdb_dbi_open failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
}

NF > 0 {
	switch ($1) {
	case "i":
		if (mdb_put(txn, dbi, $2, $3, MDB["NOOVERWRITE"]) != MDB_SUCCESS)
			printf "Warning: mdb_put(%s, %s) failed: %s\n",
			       $2, $3, mdb_strerror(MDB_ERRNO)
		break
	case "d":
		if (mdb_del(txn, dbi, $2, $3) != MDB_SUCCESS)
			printf "Warning: mdb_del(%s, %s) failed: %s\n",
			       $2, $3, mdb_strerror(MDB_ERRNO)
		break
	case "l":
		data = mdb_get(txn, dbi, $2)
		if (MDB_ERRNO != MDB_SUCCESS)
			printf "Warning: mdb_get(%s) failed: %s\n",
			       $2, mdb_strerror(MDB_ERRNO)
		else
			printf "lookup(%s) = [%s]\n", $2, data
		break
	case "dump":
		if ((cursor = mdb_cursor_open(txn, dbi)) == "")
			printf "Error: mdb_cursor_open failed: %s\n",
			       mdb_strerror(MDB_ERRNO)
		else {
			which = MDB["FIRST"]
			print "DUMP started:"
			while ((rc = mdb_cursor_get(cursor, f, which)) == MDB_SUCCESS) {
				printf "[%s] -> [%s]\n", f[0], f[1]
				which = MDB["NEXT"]
			}
			print "DUMP done."
			if (rc != MDB["NOTFOUND"])
				printf "oops, dump terminated abnormally: %s\n",
				       mdb_strerror(rc)
		}
		break
	}
}

END {
	if (mdb_txn_commit(txn) != MDB_SUCCESS) {
		printf "mdb_txn_commit failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	if (mdb_dbi_close(env, dbi) != MDB_SUCCESS) {
		printf "mdb_txn_commit failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	if (mdb_env_close(env, dbi) != MDB_SUCCESS) {
		printf "mdb_env_close failed: %s\n", mdb_strerror(MDB_ERRNO)
		exit 1
	}
	print system("rm -f " fname)
}
