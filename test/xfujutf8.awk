# outline.awk
# Reads an XML document from standard input and
# prints an element outline on standard output.
# JK 2004-06-03
# JK 2005-03-26

BEGIN        { XMLMODE=1 ; XMLCHARSET="UTF-8" }

XMLSTARTELEM {
  printf("%*s%s", 2*XMLDEPTH-2, "", XMLSTARTELEM)
  for (i=1; i<=NF; i++)
    printf(" %s='%s'", $i, XMLATTR[$i])
  print ""
}

END {
  if (ERRNO)
    printf "Error at doc %2d NR %2d FNR %2d row %2d col %2d len %2d depth %2d: %s",
           XMLDOCNUM, NR, FNR, XMLROW, XMLCOL, XMLLEN, XMLDEPTH, ERRNO
}
