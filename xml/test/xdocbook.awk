BEGIN {
   XMLMODE=-1
   XMLDOCNUM = 0
}

{
  printf "doc %d NR %d FNR %d row %2d col %2d len %2d depth %2d: %s",
  	 XMLDOCNUM, NR, FNR, XMLROW, XMLCOL, XMLLEN, XMLDEPTH, XMLEVENT
  if (XMLNAME != "")
    printf " NAME %s", XMLNAME
  nattr = asorti(XMLATTR, idx)
  for (i = 1; i <= nattr; i++)
    printf " ATTRIBUTE %s='%s'", idx[i], XMLATTR[idx[i]]
  if ($0 != "")
    printf "; TEXT = '%s'", $0
  printf "\n"
  if (XMLEVENT == "ENDDOCUMENT")
    XMLDOCNUM++
}

END {
  if (ERRNO)
    printf "Error at doc %2d NR %2d FNR %2d row %2d col %2d len %2d depth %2d: %s",
  	   XMLDOCNUM, NR, FNR, XMLROW, XMLCOL, XMLLEN, XMLDEPTH, ERRNO
}
