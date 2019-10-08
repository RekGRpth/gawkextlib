@load "strptime"

BEGIN {
	ENVIRON["TZ"] = "PST8PDT"
	format = "%b %d %H:%M:%S %Z %Y"
	the_date = "Feb 11 13:12:11 PST 1990"
	then = strptime(the_date, format)
	when = strftime(format, then)
	print then, "<" the_date ">", "<<" when ">>"
}
