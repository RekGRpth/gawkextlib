BEGIN {
   XMLMODE=-1
   XMLDOCNUM = 0
}

{
  printf "doc %d row %2d col %2d len %2d depth %2d: %s",
  	 XMLDOCNUM, XMLROW, XMLCOL, XMLLEN, XMLDEPTH, XMLEVENT
  if (XMLNAME != "")
    printf " NAME %s", XMLNAME
  for (a in XMLATTR)
    printf " ATTRIBUTE %s='%s'", a, XMLATTR[a]
  if ($0 != "")
    printf "; TEXT = '%s'", $0
  printf "\n"
  if (XMLEVENT == "ENDDOCUMENT")
    XMLDOCNUM++
}

END {
  if (ERRNO)
    printf "Error at doc %2d row %2d col %2d len %2d depth %2d: %s",
  	   XMLDOCNUM, XMLROW, XMLCOL, XMLLEN, XMLDEPTH, ERRNO
}
