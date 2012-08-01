# to enable building in a different directory
function basename(fn) {
	return gensub(/^.*\//,"",1,fn)
}

BEGINFILE {
	printf "BEGINFILE [%s]\n", basename(FILENAME)
	XMLMODE = (FILENAME ~ /\.xml$/)
}

FNR == 1 {
	printf "FILENAME '%s'  NF %d  $0 '%s'  RT '%s'  XMLEVENT '%s'  XMLNAME '%s'\n",
	       basename(FILENAME), NF, $0, RT, XMLEVENT, XMLNAME
}

ENDFILE {
	if (ERRNO)
		printf "Error processing %s: %s\n", FILENAME, ERRNO
}
