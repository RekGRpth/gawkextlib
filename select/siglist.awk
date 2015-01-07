function procfile(fn,  x, f) {
  while ((getline x < fn) > 0) {
    if (match(x, /^[[:space:]]*#[[:space:]]*define[[:space:]]+(SIG[[:alnum:]]+)[[:space:]]+[[:alnum:]]+/, f)) {
      if (!(f[1] in name2signal)) {
	name2signal[f[1]]
        printf "#ifdef %s\n", f[1]
        printf "init_sig (%s, \"%s\")\n", f[1], substr(f[1], 4)
        print "#endif"
      }
    }
  }
  close(fn)
}

{
  for (i = 1; i <= NF; i++) {
    if ($i ~ /\.h$/)
      procfile($i)
    else if ($i ~ /\.h:$/)
      procfile(substr($i, 1, length($i)-1))
  }
}
