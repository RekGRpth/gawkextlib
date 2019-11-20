BEGIN {
	ENVIRON["TZ"] = "PST8PDT"
	format = "%b %d %H:%M:%S %Y"
	the_date = "Feb 11 13:12:11 1990"
	then = strptime(the_date, format)
	when = strftime(format, then)
	print then, "<" the_date ">", "<<" when ">>"

	now = systime()
	sleep(2)
	later = systime()

	if (later - now < 2)
		printf("FAIL: did not sleep 2 seconds, now %d vs later %d\n", now, later)
	else
		print "SUCCESS: slept at least 2 seconds"

	simple_now = systime()
	time_of_day = int(gettimeofday())

	if (simple_now != time_of_day)
		printf("FAIL: now %d vs time %d\n", simple_now, time_of_day)
	else
		print "SUCCESS: got same times"
}
