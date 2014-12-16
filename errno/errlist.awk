function procfile(fn,  x, f) {
	while ((getline x < fn) > 0) {
		if (match(x, /^[[:space:]]*#[[:space:]]*define[[:space:]]+(E[[:alnum:]]+)[[:space:]]+([[:alnum:]]+)/, f)) {
			found[f[1]] = f[2]
		}
	}
	close(fn)
}

{
	for (i = 1; i <= NF; i++) {
		if ($i ~ /\.h$/)
			procfile($i)
		else if ($i ~ /\.h:$/)
			procfile(substr($i, 1, length($i)-1))
	}
}

END {
	for (i in found) {
		if ((dupes == "no") && (found[i] in found))
			printf "Notice: suppressing %s defined as %s\n", i, found[i] > "/dev/stderr"
		else {
		   printf "#ifdef %s\n", i
		   printf "init_errno (%s, \"%s\")\n", i, i
		   print "#endif"
		}
	}
}
