@include "csv"
BEGIN {CSVMODE=0; CSVFS="|"}
{
    print "CSVRECORD <" CSVRECORD ">"
    print "$0 <" $0 ">"
    printf("<%s> <%s>\n<%s>\n<%s> <%s>\n\n", $1, $2, $3, $4, $5)
}
