@load "time"

function runtest(tout,   cmd, mode, start, readfds, writefds, exceptfds, i, rc, x) {
   cmd = "echo before sleep; sleep 1; echo after sleep #"
   mode = "|<"
   set_non_blocking(cmd, mode)
   start = gettimeofday()
   while (cmd) {
      delete readfds
      readfds[cmd] = mode
      rc = select(readfds, writefds, exceptfds, tout)
      switch (rc) {
      case -1:
	 if (tolower(ERRNO) ~ /interrupt/)
	    # EINTR, so try again
	    break
	 printf "Error: select failed: %s\n", ERRNO > "/dev/stderr"
	 exit 1
      case 0:
	 printf "%.1f timeout\n", gettimeofday()-start
	 sleep(0.1)	# protect against tight loop
	 break
      default:
	 for (i in readfds) {
	    while ((rc = (i | getline x)) > 0)
	       printf "%.0f [%s] -> %s\n", gettimeofday()-start, i, x
	    if (rc != -2) {
	       cmd = ""
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
}

BEGIN {
   t[1] = 2	# number
   split("2", f)
   t[2] = f[1]	# strnum
   t[3] = "2"	# numeric string value
   t[4] = "apple"	# non-numeric string should have infinite timeout

   for (i = 1; i <= length(t); i++) {
      printf "\n\tTesting timeout %s type %s\n", t[i], typeof(t[i])
      runtest(t[i])
   }
   exit erc+0
}
