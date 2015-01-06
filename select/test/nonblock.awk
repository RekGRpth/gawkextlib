@load "time"

BEGIN {
	cmd = "echo hello; sleep 1; echo goodbye"

	print "Normal non-retry behavior"
	fd = input_fd(cmd, "|<")
	# must use input_fd to avoid automatic RETRY configuration
	# by set_non_blocking
	print "set_non_blocking", set_non_blocking(fd)
	sleep(.3)	# sleep long enough for "hello" to arrive
	while ((rc = (cmd | getline x)) > 0)
		print x
	if (rc < 0)
		print rc, (PROCINFO["errno"] != 0), (ERRNO != "")
	print (close(cmd) != 0)

	print ""
	print "With non-blocking I/O"
	n = 0
	# N.B. set_non_blocking automatically enables RETRY mode
	print "set_non_blocking", set_non_blocking(cmd, "|<")
	sleep(.3)	# sleep long enough for "hello" to arrive
	while (((rc = (cmd | getline x)) > 0) || (rc == -2)) {
		if (rc > 0) {
			print x
			n = 0
		}
		else {
			print ++n, "would block; pausing..."
			sleep(.3)
		}
	}
	if (rc < 0)
		print rc, (PROCINFO["errno"] != 0), (ERRNO != "")
	print (close(cmd) != 0)
}
