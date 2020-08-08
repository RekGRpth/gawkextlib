@load "reclen"

BEGIN { RECLEN = 6 }
NR == 1 { printf("got <%s>\n", $0); RECLEN = 10 ; next }
NR == 2 { printf("got <%s>\n", $0); RECLEN = 4 ; next }
NR == 3 { printf("got <%s>\n", $0) ; reclen::drop(FILENAME); next }
{ print }
