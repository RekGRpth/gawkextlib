BEGIN {
	OFS = ", "
	x = 4
	ret = (getline x < ".")
	print x, ret, ERRNO
}
