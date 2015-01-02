function procfile(fn,  x, f) {
	while ((getline x < fn) > 0) {
		if (match(x, /^[[:space:]]*#[[:space:]]*define[[:space:]]+(SIG[[:alnum:]]+)[[:space:]]+([[:alnum:]]+)/, f)) {
			name2signal[f[1]] = f[2]
			if (f[2] in signal2name)
				printf "Notice: suppressing signal2name duplicate %s defined as %s (-> %s)\n", f[1], f[2], signal2name[f[2]] > "/dev/stderr"
			else
				signal2name[f[2]] = f[1]
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
	if (mode ~ /name2signal/) {
		for (i in name2signal) {
			printf "#ifdef %s\n", i
			printf "init_sig (%s, \"%s\")\n", i, substr(i, 4)
			print "#endif"
		}
	}
	else {
		for (j in signal2name) {
			i = signal2name[j]
			if (j in name2signal)
				printf "Notice: suppressing signal2name %s since value %s in name2signal [%s]\n", i, j, name2signal[j] > "/dev/stderr"
			else {
				i = signal2name[j]
				printf "#ifdef %s\n", i
				printf "init_sig (%s, \"%s\")\n", i, substr(i, 4)
				print "#endif"
			}
		}
	}
}
