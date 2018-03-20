BEGIN {
    print csvconvert("a,b,c", "|")
}

{
    print csvconvert($0, "|")
}
