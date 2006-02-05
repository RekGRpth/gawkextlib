#!/usr/bin/awk -f
# http://www.prothsearch.net/fermat.html#Prime
# http://mathworld.wolfram.com/FermatsFactorizationMethod.html
# http://www.cc.utah.edu/~nahaj/factoring/fermat.c
# http://en.wikipedia.org/wiki/Fermat_number
# http://en.wikipedia.org/wiki/Fermat's_factorization_method
# ftp://ftp.comlab.ox.ac.uk/pub/Documents/techpapers/Richard.Brent/rpb161.ps.gz
# Fermat F1 = 2 ^ (2 ^ 0) + 1 = 3                    = prime
# Fermat F1 = 2 ^ (2 ^ 1) + 1 = 5                    = prime
# Fermat F2 = 2 ^ (2 ^ 2) + 1 = 17                   = prime
# Fermat F3 = 2 ^ (2 ^ 3) + 1 = 257                  = prime
# Fermat F4 = 2 ^ (2 ^ 4) + 1 = 65537                = prime
# Fermat F5 = 2 ^ (2 ^ 5) + 1 = 4294967297           = 641 * 6700417
# Fermat F6 = 2 ^ (2 ^ 6) + 1 = 18446744073709551617 = 274177 * 67280421310721
# Fermat F7 = 2 ^ (2 ^ 7) + 1 = 340282366920938463463374607431768211457
#        F7 =                   59649589127497217 * 5704689200685129054721
# IEEE arithmetic can factorize F4, but not F5 or larger.

BEGIN {
  if (ARGC != 2) {
    print "FERMAT - factorize a number using Fermat's algorithm" 
    print "IN:"
    print "    a number as first and only command line paramater"
    print "OUT:"
    print "    two factors"
    print "CALL:"
    print "    gawk -f fermat.awk number"
    print "EXAMPLE:"
    print "    gawk -f fermat.awk 65537 "
    print "JK 2002-03-14"
    print "JK 2006-01-27"
    exit
  }
  MPFR_MANTISSA=200
  n=ARGV[1]; ARGV[1] = ""
  if (n<1)          { print "number must be greater than zero\n" ; exit }
  if ((n % 2) == 0) { print "number must be odd\n"               ; exit }
  xx = 1
  yy = 1
  e = mpfr_sub(yy-xx, n)
  while (mpfr_zero_p(e) == 0 ) {
    if (mpfr_less_p(e,0)) {
      e  = mpfr_add(e, yy)
      yy = mpfr_add(yy, 2)
    } else {
      e  = mpfr_sub(e, xx)
      xx = mpfr_add(xx, 2)
    }
  }
  x = mpfr_div(mpfr_sub(xx, 1), 2);
  y = mpfr_div(mpfr_sub(yy, 1), 2);
  q = mpfr_add(y, x);
  p = mpfr_sub(y, x); 
  print p, q
}
