@load time

BEGIN {
   delta = 2.347
   printf "Start time = %.6f [systime = %s]\n",t0 = gettimeofday(), systime()
   printf "Sleep(%s) return code = %s\n",delta,sleep(delta)
   printf "End time = %.6f [systime = %s]\n",t1 = gettimeofday(), systime()
   printf "Elapsed time = %.6f\n",t1-t0
}
