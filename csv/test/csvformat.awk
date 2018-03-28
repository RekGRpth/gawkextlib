@include "csv"
BEGIN {
    CSVMODE = 1
#    print "CSVFS: <" CSVFS "> " length(CSVFS) 
}
{
    print "CSVRECORD <" CSVRECORD ">"
    print "$0 <" $0 ">"
    print "csvformat <" csvformat($0, CSVFS) ">"
}
