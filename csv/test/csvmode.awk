@include "csv"
BEGIN {CSVMODE=1; CSVFS="|"}
{
    print "CSVRECORD <" CSVRECORD ">"
    print "$0 <" $0 ">"
    printf("<%s> <%s>\n<%s>\n<%s> <%s>\n\n", $1, $2, csvfield("Address"), $4, $5)
}
