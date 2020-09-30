@load "../.libs/aregex"

BEGIN {
  str = "abcdễfbc"
  print "String    : " str
  re = "^a(bc)d(ễ)(f)$"
  print "RE        : " re

  # Set some costs
  cost["max_cost"] = 6
  print "max_cost  : " cost["max_cost"]
  cost["cost_ins"] = 2
  cost["cost_del"] = 3

  print "Match sr  : " amatch(str, re)
  print "Match src : " amatch(str, re, 6.1)
  print "Match srcb: " amatch(str, re, 6, b), b[1]
  print "Match srC : " amatch(str, re, cost)
  print "Match srCb: " amatch(str, re, cost, out)

  print "Submatches: "
  print "   i  substr    pos  len"
  print "   -  ------    ---  ---"
  printf "  %2d  %-10s %2d %2d\n", 0, out[0], index(str, out[0]), length(out[0])
  p = 1
  for (i = 1; i < length(out); i++) {
    idx = index(substr(str, p), out[i])
    len = length(out[i])
    printf "  %2d  %-10s %2d %2d\n", i, out[i], idx+p-1, len
    p = p + idx + len
  }

  print "cost      : " cost["cost"]
  print "num_ins   : " cost["num_ins"]
  print "num_del   : " cost["num_del"]
  print "num_subst : " cost["num_subst"]

  # check that amatch can handle invalid regex
  if(-1 == amatch(str,"^(")) print ERRNO > "/dev/stderr"
}
