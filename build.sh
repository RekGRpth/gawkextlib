#!/bin/sh

if [ $# -lt 1 ]; then
   echo "Usage: `basename $0` <directory> [<configure arguments>...]

This script cds into the <directory> and runs these commands:

	autoreconf -i && ./configure <arguments> && make clean && make && \\
		make check && make install

The script exit status indicates whether the commands completed
successfully." 1>&2
   exit 1
fi

doit () {
   echo "	Running: $@"
   "$@" || {
      echo "*** Error: $@ failed with status $? ***"
      return 1
   }
}

dir="$1"
shift 1
echo "
Building in directory $dir"
doit cd "$dir" && doit autoreconf -i && doit ./configure "$@" && doit make clean && doit make && doit make check && doit make install && echo "Built $dir successfully."
