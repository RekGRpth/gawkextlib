@load "time"

BEGIN {
   mypid = PROCINFO["pid"]

   cmd["echo hello; sleep 1; echo midway; sleep 2; echo goodbye"] = "|<"

   print "previous sighup disposition:", select_signal("hup", "trap")

   for (i in cmd)
      set_non_blocking(i, cmd[i])

   start = gettimeofday()
   while (length(cmd) > 0) {
      delete readfds
      for (i in cmd)
	 readfds[i] = cmd[i]
      rc = select(readfds, writefds, exceptfds, "", sigs)
      toff = sprintf("%.0f", gettimeofday()-start)
      for (i in sigs)
	 printf "%s Caught signal %d (%s)\n", toff, i, sigs[i]
      switch (rc) {
      case -1:
	 if (tolower(ERRNO) ~ /interrupt/)
	    # EINTR, so try again
	    break
	 printf "Error: select failed: %s\n", ERRNO > "/dev/stderr"
	 exit 1
      case 0:
	 # try again
	 break
      default:
	 for (i in readfds) {
	    while ((rc = (i | getline x)) > 0)
	       printf "%s [%s] -> %s\n", toff, i, x
	    if (rc != -2) {
	       delete cmd[i]
	       if (rc < 0) {
		  printf "Error: getline(%s) returned %s, errno = %s\n",
			 i, rc, ERRNO
		  erc = 1
	       }
	       if ((rc = close(i)) != 0)
		  printf "Error: close(%s) failed with rc %s, ERRNO %s\n",
		  	 i, rc, ERRNO
	    }
	    else
	       print toff, "kill", kill(mypid, "hup")
	 }
	 break
      }
   }
   exit erc+0
}
