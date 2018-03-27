# Detailed contents of a CSV file

@load "csv"

BEGIN {
    CSVMODE = 1
    CSVFS = "|"
}

{
    print "<" CSVRECORD ">"
    print "<" $0 ">"
    for (k=1; k<=NF; k++) {
        print k "-<" $k ">"
    }
}
