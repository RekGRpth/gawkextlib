BEGIN {
	for (i = 1; i < 1000; i++) {
		if ((s = errno2name(i)) != "") {
			if ((e = name2errno(s)) != i)
				printf "error: %d -> %s -> %s\n", i, s, e
			else
				good++
			if (tolower(t = strerror(i)) ~ /unknown error/) {
				strbad++
				#printf "error: strerror(%d) = %s\n", i, t
			}
		}
#		else if (tolower(t = strerror(i)) !~ /unknown error/)
#			printf "error: strerror(%d) = [%s] for invalid errno\n",
#			       i, t
	}
	mingood = 20
	if (good+0 < mingood)
		printf "error: # of successful inversions %d < minimum %d\n",
		       good, mingood
	if (strbad+0 > .03*good)
		printf "error: strerror does not recognize %d of %d known error codes\n", strbad, good
}
