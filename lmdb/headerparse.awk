function procfile(fn,   x, f, nf, group, grouptype, val) {
   printf "Parsing header file %s\n", fn > "/dev/stderr"
   while ((getline x < fn) > 0) {
      nf = split(x, f)
      if (f[2] == "@defgroup") {
	 group = tolower(f[3])
	 grouptype = "def"
      }
      else if (f[2] == "enum")  {
	 group = tolower(f[3])
	 grouptype = "enum"
      }
      else {
	 switch (grouptype) {
	 case "def":
	    if ((nf == 3) && (f[1] == "#define") &&
		(f[2] ~ /^MDB_/) && (f[3] !~ /[,\\]/) &&
		(f[3] ~ /[[:xdigit:]]/) && (f[2] != "MDB_LAST_ERRCODE")) {
	       cnt[group]++
	       printf "init_lmdb(%s, %s, %s)\n", group, substr(f[2], 5), f[2]
	    }
	    break
	 case "enum":
	    if ((nf == 2) && (f[1] == "}") && (tolower(f[2]) == (group ";")))
	       grouptype = ""
	    else if ((nf > 2) && (f[1] ~ /^MDB_/) && (f[2] ~ /^\/\*/)) {
	       cnt[group]++
	       val = gensub(/,$/, "", 1, f[1])
	       printf "init_lmdb(%s, %s, %s)\n", group, substr(val, 5), val
	    }
	    break
	 }
      }
      prev = x
   }
   close(fn)
}

{
   for (i = 1; i <= NF; i++) {
      if ($i ~ /lmdb.*\.h$/)
	 procfile($i)
      else if ($i ~ /lmdb.*\.h:$/)
	 procfile(substr($i, 1, length($i)-1))
   }
}

END {
   nf = split("version|mdb_env|mdb_dbi_open|mdb_put|mdb_copy|errors|mdb_cursor_op", f, "|")
   for (i = 1; i <= nf; i++) {
      if (!(f[i] in cnt)) {
	 printf "Error: did not find %s defines\n", f[i] > "/dev/stderr"
	 erc = 1
      }
   }
   exit erc+0
}
