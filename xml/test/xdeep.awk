# Let's see if tags can be nested to depth 10000.
BEGIN {
  print "<?xml version='1.0' encoding='UTF-8'?>"
  deep(10000)
}

function deep(depth) {
  if (depth <= 0)
    return
  printf("<d%d>\n", depth)
  deep(depth-1)
  printf("</d%d>\n", depth)
}
