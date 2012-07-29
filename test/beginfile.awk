BEGINFILE {
	printf "BEGINFILE [%s]\n", FILENAME
	XMLMODE = (FILENAME ~ /\.xml$/)
}

FNR == 1 {
	printf "FILENAME '%s'  NF %d  $0 '%s'  RT '%s'  XMLEVENT '%s'  XMLNAME '%s'\n",
	       FILENAME, NF, $0, RT, XMLEVENT, XMLNAME
}

ENDFILE {
	if (ERRNO)
		printf "Error processing %s: %s\n", FILENAME, ERRNO
}
