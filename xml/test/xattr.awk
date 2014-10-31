# Let's see if a tag can have 10000 attributes.
BEGIN {
  print "<?xml version='1.0' encoding='UTF-8'?>"
  print "<root "
  for (i=1; i<=10000; i++)
    printf("a%d='%d'\n", i, i)
  print "/>"
}
