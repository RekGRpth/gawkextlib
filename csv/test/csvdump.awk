# Detailed contents of a CSV file
@include "csv"
BEGIN {
    CSVMODE = 1
    CSVFS = "|"
}
{
    print "<" CSVRECORD "><" RT ">"
    print "<" $0 ">"
    for (k=1; k<=NF; k++) {
        print k "-<" $k ">"
    }
}
