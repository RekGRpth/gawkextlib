@load "csv"

function test(csvrecord) {
#    print csvrecord
	awkrecord = csvconvert(csvrecord, "|")
#	print awkrecord
    print "numfields: " split(awkrecord, af, "|")
    for (k=1; k in af; k++) print k " -> <" af[k] ">"
	# delete af
    # print "numfields: " csvsplit(csvrecord, af)
    # for (k=1; k in af; k++) print k " -> <" af[k] ">"
}

BEGIN {
    test("a,b,c")
}

{
    test($0)
}
