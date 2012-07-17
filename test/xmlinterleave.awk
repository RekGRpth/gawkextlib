function procrec(fn,nr,  rc,idx,i,nattr) {
  switch (rc = (getline < fn)) {
    case -1:
      # should not happen
      printf "Error: getline(%s) failed with ERRNO %s\n", fn, ERRNO
      erc = 1
      return -1
    case 0:
      printf "%s: EOF\n", fn
      return -1
    case 1:
      printf "%s nr %d NF %d", fn, nr, NF
      if (NF > 0)
	 printf " $0 [%s]", $0
      # print all 19 scalars set by the XML extension:
      printf " row [%s]", XMLROW
      printf " col [%s]", XMLCOL
      printf " len [%s]", XMLLEN
      printf " depth [%s]", XMLDEPTH
      printf " event [%s]", XMLEVENT
      printf " name [%s]", XMLNAME
      printf " declaration [%s]", XMLDECLARATION
      printf " startelem [%s]", XMLSTARTELEM
      printf " endelem [%s]", XMLENDELEM
      printf " chardata [%s]", XMLCHARDATA
      printf " procinst [%s]", XMLPROCINST
      printf " comment [%s]", XMLCOMMENT
      printf " startcdata [%s]", XMLSTARTCDATA
      printf " endcdata [%s]", XMLENDCDATA
      printf " startdoct [%s]", XMLSTARTDOCT
      printf " enddoct [%s]", XMLENDDOCT
      printf " unparsed [%s]", XMLUNPARSED
      printf " error [%s]", XMLERROR
      printf " enddocument [%s]", XMLENDDOCUMENT
      # and the XMLATTR array
      nattr = asorti(XMLATTR, idx)
      for (i = 1; i <= nattr; i++)
	printf " ATTRIBUTE %s='%s'", idx[i], XMLATTR[idx[i]]
      printf "\n"
      return 0
    default:
      # should not happen
      printf "Error: getline(%s) returned %s with ERRNO %s\n", fn, rc, ERRNO
      erc = 1
      return -1
  }
}

BEGIN {
  printf "XMLMODE [%s], XMLCHARSET [%s]\n", XMLMODE, XMLCHARSET
  nopen = ARGC-1
  while (nopen > 0) {	# while there is at least one file open
    for (i = 1; i < ARGC; i++) {
      # if the file is still open, process one record from that file
      if ((i in ARGV) && (procrec(ARGV[i], ++cnt[i]) < 0)) {
	# procrec failed, so close the file
        nopen--
	delete ARGV[i]
       }
    }
  }
  exit erc+0
}
