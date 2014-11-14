#!/bin/sh

if [ $# -lt 1 ]; then
   echo "Usage: `basename $0` <command to run> <additional arguments>

For lib and each extension directory, this script will run the supplied
command with a first argument of the directory name and subsequent arguments
as supplied to this script.  If it fails on lib, we stop immediately.
Otherwise, we run for all extension directories and report on which ones
succeeeded and which ones failed." 1>&2
   exit 1
fi

cmd="$1"
shift 1

doit () {
   echo "	Running: $@"
   "$@" || {
      echo "*** Error: $@ failed with status $? ***"
      return 1
   }
}

doit "$cmd" lib "$@" || {
   echo "Error: command failed for lib, aborting..."
   exit 1
}

echo "
Command succeeded in lib; now attempting to run for the extensions"
good=lib
for d in * ; do
   if [ -d "$d" ]; then
      case "$d" in
      lib|shared) ;;
      *) if doit "$cmd" "$d" "$@" ; then
	    good="$good $d"
	 else
	    bad="$bad $d"
	 fi
	 ;;
      esac
   fi
done

echo "
SUCCESS: $good"
if [ -n "$bad" ]; then
   echo "ERROR:${bad}"
   exit 1
fi
exit 0
