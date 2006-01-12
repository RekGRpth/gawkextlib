function abs(x) {
	return (x+0 >= 0) ? x : -x
}

function check(x,what,  f,res) {
	for (f in formats) {
		res = sprintf(f,x)
		if (abs(res-x) > 1e-5*abs(x))
			printf "(sprintf(%s,%s) = %s)-%g = %g\n",
			       f,what,res,x,res-x
	}
}

BEGIN {
	formats["%s"] = ""
	formats["%d"] = ""
	formats["%.0f"] = ""

	check(0,"0")
	for (i = 0; i <= 308; i++) {
		check(10^i,"10^"i)
		check(-10^i,"-10^"i)
	}
	for (i = 0; i <= 1023; i++) {
		check(2^i,"2^"i)
		check(-2^i,"-2^"i)
	}

	# check another problem in gawk 3.1.5: precision over 30 crashes
	printf "%.55d\n",1
}
