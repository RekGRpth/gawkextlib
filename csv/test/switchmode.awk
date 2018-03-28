BEGINFILE {
    CSVMODE = (FILENAME ~ /\.csv$/)
    print ""
    print "----> " FILENAME
}

@include "csv"

{
    print ""
    print
    csvprint0()
    csvprint()
}

/,/ {
    print "<has comma>"
}
