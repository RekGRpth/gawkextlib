@include "csv"
function test(csvrecord) {
    print "numfields: " csvsplit(csvrecord, af)
    for (k=1; k in af; k++) print k " -> <" af[k] ">"
}
BEGIN {
    CSVMODE = 0
    test("a,b,c")
}
{
    test($0)
}
