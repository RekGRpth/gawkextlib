BEGIN {
	print "you should see this line and exit status 7"
	print abort(7)
}

END {
	print "you should not see this!"
}
