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
	printf "%d IEEE special values: [+-]?NaN and [+-]?Inf:\n",numtests
	printf "\n"
}

function check_equal(x,str,fsfpasses,  s1,s2) {

	s1 = sprintf("%f",x)
	s2 = sprintf("%f",str+0)
	if (s1 != s2) {
		header()
		printf "Warning: (sprintf(%%f,%s) = %s) != (sprintf(%%f,\"%s\"+0) = %s)\n",
		       x,s1,str,s2
		if (fsfpasses)
			printf "\tError: this behavior differs from %s\n",fsfname
	}
	else if (!fsfpasses) {
		printf "Warning: %s fails this test, but we do not:\n",fsfname
		printf "\t(sprintf(%%f,%s) = %s) == (sprintf(%%f,\"%s\"+0) = %s)\n",
		       x,s1,str,s2
			
	}
}

BEGIN {
	fsfname = "mainline FSF gawk-3.1.6"
	nan = sqrt(-1)
	nan_str = sprintf("%f",nan)
	inf = -log(0)
	inf_str = sprintf("%f",inf)
	failed = 0

	numtests = 6
	check_equal(nan,nan_str,0)
	check_equal(nan,"+"nan_str,1)
	check_equal(-nan,"-"nan_str,1)
	check_equal(inf,inf_str,0)
	check_equal(inf,"+"inf_str,1)
	check_equal(-inf,"-"inf_str,1)

	if (failed == 0)
		print "Congratulations, this system converts IEEE special values properly."
	else if (failed == numtests) {
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
		printf "of the %d cases, and it converts them\n",numtests
		printf "properly in the other %d cases.\n",numtests-failed
	}
}
