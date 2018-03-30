@include "csv"
BEGIN {
    CSVMODE=1
}
{
    print "CSVRECORD <" CSVRECORD ">"
    print "$0 <" $0 ">"
    printf("<%s> <%s>\n<%s>\n<%s> <%s>\n\n", $1, $2, $3, csvfield("4"), $5)
}
