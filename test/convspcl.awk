function header() {
	if (failed++)
		return

	print "The Open Group awk specification says that a string value shall"
	print "be converted to a numeric value using the ISO C standard atof()"
	print "function.  The awk spec is here:"
	printf "\n"
	printf "\thttp://www.opengroup.org/onlinepubs/009695399/utilities/awk.html\n"
	printf "\n"
	print "And the ISO C standard says that atof shall work the same way"
	print "as strtod, and strtod must recognize the IEEE special values"
	print "for NaN (not-a-number) and Infinity."
	printf "\n"
	print "Historically, gawk has been inconsistent in its treatment"
	print "of these values, and has generally not complied with the"
	print "standard.  So we check here to see how this build handles the"
	print "4 IEEE special values: +/- NaN and +/- Inf:"
	printf "\n"
}

function check_equal(x,str, s1,s2) {
	s1 = sprintf("%f",x)
	s2 = sprintf("%f",str+0)
	if (s1 != s2) {
		header()
		printf "Warning: (sprintf(%%f,%s) = %s) != (sprintf(%%f,\"%s\"+0) = %s)\n",
		       x,s1,str,s2
	}
}

BEGIN {
	nan = sqrt(-1)
	nan_str = sprintf("%f",nan)
	inf = -log(0)
	inf_str = sprintf("%f",inf)
	failed = 0

	check_equal(nan,nan_str)
	check_equal(-nan,"-"nan_str)
	check_equal(inf,inf_str)
	check_equal(-inf,"-"inf_str)

	if (failed == 0)
		print "Congratulations, this system converts IEEE special values properly."
	else if (failed == 4) {
		printf "\n"
		print "This system does not comply with the spec, but the"
		print "good news is that it is at least consistent in always"
		print "converting the IEEE special values to zero."
	}
	else {
		printf "\n"
		print "Bad news: this system does not comply with the spec,"
		print "and it is not consistent in its behavior."
		printf "It converts the special values to zero in %d\n",failed
		printf "of the 4 cases, and it converts them properly in the\n"
		printf "other %d cases.\n",4-failed
	}
}
