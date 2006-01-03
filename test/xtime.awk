BEGIN {
   delta = 1.3
   printf "gettimeofday - systime = %d\n", (t0 = gettimeofday())-systime()
   printf "sleep(%s) = %s\n",delta,sleep(delta)
   printf "gettimeofday - systime = %d\n", (t1 = gettimeofday())-systime()
   printf "elapsed time = %.1f\n",t1-t0
}
