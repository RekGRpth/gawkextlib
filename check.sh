#!/bin/sh

if [ $# -lt 1 ]; then
   echo "Usage: `basename $0` <directory> [<configure arguments>...]

This script cds into the <directory> and runs these commands:

	autoreconf -i && ./configure <arguments> && make clean && make && \\
		make check && make install && make dist

It then unpacks the distribution tarball and runs these commands to verify
that the distribution was built correctly:

	./configure <arguments> && make && make check && make install 

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

check_tarball () {
   tarball=`pwd`/`ls -t *.tar.gz | head -1`
   echo "
Trying to build from tarball $tarball"
   tmpdir=`mktemp -dt gawkextbuild.XXXXXXXXXX` || \
      { echo "Error: mktemp failed" 1>&2 ; exit 1; }
   trap "rm -rf $tmpdir" 0
   trap 'exit 2' 1 2 3 6 13 15
   doit cd $tmpdir && doit tar xf "$tarball" && doit cd * && doit ./configure "$@" && doit make && doit make check && doit make install && echo "Built $dir from tarball successfully."
}

doit cd "$dir" && doit autoreconf -i && doit ./configure "$@" && doit make clean && doit make && doit make check && doit make install && doit make dist && check_tarball "$@" && echo "Built and verified $dir successfully."
