BEGIN {
	fname = "./test_dict.lmdb"
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
}

NF > 0 {
	switch ($1) {
	case "i":
		if (mdb_put(txn, dbi, $2, $3, MDB["NOOVERWRITE"]) != MDB_SUCCESS)
			printf "Warning: mdb_put(%s, %s) failed: %s [%s]\n",
			       $2, $3, mdb_strerror(MDB_ERRNO), ERRNO
		break
	case "d":
		if (NF == 2)
			mdb_del(txn, dbi, $2)
		else
			mdb_del(txn, dbi, $2, $3)
		if (MDB_ERRNO != MDB_SUCCESS)
			printf "Warning: mdb_del(%s, %s) failed: %s [%s]\n",
			       $2, $3, mdb_strerror(MDB_ERRNO), ERRNO
		break
	case "l":
		data = mdb_get(txn, dbi, $2)
		if (MDB_ERRNO != MDB_SUCCESS)
			printf "Warning: mdb_get(%s) failed: %s [%s]\n",
			       $2, mdb_strerror(MDB_ERRNO), ERRNO
		else
			printf "lookup(%s) = [%s]\n", $2, data
		break
	case "dump":
		if ((cursor = mdb_cursor_open(txn, dbi)) == "")
			printf "Error: mdb_cursor_open failed: %s [%s]\n",
			       mdb_strerror(MDB_ERRNO), ERRNO
		else {
			if (mdb_cursor_dbi(cursor) != dbi)
				print "Error, mdb_cursor_dbi not working properly"
			if (mdb_cursor_txn(cursor) != txn)
				print "Error, mdb_cursor_txn not working properly"
			print "DUMP started:"
			# N.B. On the 1st call for a new cursor,
			# NEXT is equivalent to FIRST
			while (mdb_cursor_get(cursor, f, MDB["NEXT"]) == \
			       MDB_SUCCESS)
				printf "[%s] -> [%s]\n", f[MDB_KEY], f[MDB_DATA]
			print "DUMP done."
			if (MDB_ERRNO != MDB["NOTFOUND"])
				printf "oops, dump terminated abnormally: %s [%s]\n",
				       mdb_strerror(MDB_ERRNO), ERRNO
			if (mdb_cursor_close(cursor) != MDB_SUCCESS)
				printf "Error: mdb_cursor_close failed: %s [%s]\n",
				       mdb_strerror(MDB_ERRNO), ERRNO
		}
		break
	}
}

END {
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
