@include "csv"
BEGIN {
    CSVMODE = 0
    print csvconvert("a,b,c", "|")
}
{
    print csvconvert($0, "|")
}
