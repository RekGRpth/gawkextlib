# Let's see if tags can be nested to depth 10000.
# Use a non-recurisve approach for machines with
# limited stacksize (like Cygwin or Mac OS X)
BEGIN {
  print "<?xml version='1.0' encoding='UTF-8'?>"
  max=10000
  for (i=max; i>=1; i--) printf("<d%d>\n", i)
  for (i=1; i<=max; i++) printf("</d%d>\n", i)
}
