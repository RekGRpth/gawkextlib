@load "select"
@load "time"
@load "errno"

function trap_signal(sig,  res) {
   printf "trapping %s; previous signal handler: %s\n",
	  sig, (res = select_signal(sig, "trap", 0))
   if (res == "")
      printf "signal trapping failed, ERRNO = %s\n", ERRNO
}

BEGIN {
   print "My pid is", PROCINFO["pid"]
   trap_signal("fpe")	# should fail since the main gawk binary traps FPE
   trap_signal("int")
   trap_signal("chld")

   cmd["echo A:msg1; sleep 2; echo A:msg2; echo A:backtoback; sleep 4; echo A:msg3"] = "|<"
   cmd["sleep 3; echo B:msg1; echo B:again; sleep 2; echo B:msg2; sleep 4; echo B:msg3"] = "|<"

   for (i in cmd)
      set_non_blocking(i, cmd[i])

   delete writefds
   delete exceptds
   start = gettimeofday()
   EINTR = name2errno("EINTR")
   while (length(cmd) > 0) {
      delete readfds
      for (i in cmd)
	 readfds[i] = cmd[i]
      rc = select(readfds, writefds, exceptfds, "", sigs)
      for (i in sigs)
	 printf "Caught signal %d (%s)\n", i, sigs[i]
      switch (rc) {
      case -1:
	 if (PROCINFO["errno"] == EINTR) {
	    print "Select failed due to EINTR; trying again."
	    break
	 }
	 printf "Error: select failed: %s\n", ERRNO > "/dev/stderr"
	 exit 1
      case 0:
	 print "Timeout.  Trying again."
	 break
      default:
	 for (i in readfds) {
	    while ((rc = (i | getline x)) > 0)
	       printf "%.1f cmd [%s]: %s\n", gettimeofday()-start, i, x
	    if (rc != -2) {
	       delete cmd[i]
	       if (rc < 0) {
		  printf "Error: getline(%s) failed, errno = %s\n", i, ERRNO
		  erc = 1
	       }
	    }
	 }
	 break
      }
   }
   exit erc+0
}
