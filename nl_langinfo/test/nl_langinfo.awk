BEGIN {
	print nl_langinfo(LANGINFO["D_T_FMT"])
	a[1] = "apple"
	print nl_langinfo(a)
}
