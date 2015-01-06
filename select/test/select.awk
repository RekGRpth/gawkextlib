@load "time"

BEGIN {
   cmd["echo A:msg1; sleep 2; echo A:msg2; echo A:msg3; sleep 2; echo A:msg4"] = "|<"
   cmd["sleep 1; echo B:msg1; echo B:msg2; sleep 2; echo B:msg3"] = "|<"

   for (i in cmd)
      set_non_blocking(i, cmd[i])

   start = gettimeofday()
   while (length(cmd) > 0) {
      delete readfds
      for (i in cmd)
	 readfds[i] = cmd[i]
      rc = select(readfds, writefds, exceptfds, "", sigs)
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
	       printf "%.0f [%s] -> %s\n", gettimeofday()-start, i, x
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
	 }
	 break
      }
   }
   exit erc+0
}
