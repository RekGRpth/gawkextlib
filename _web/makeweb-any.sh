#!/bin/bash
# Convert a document file to HTML and also PDF (if required)
#
# makeweb-any  input-file  output-dir

TOOLDIR=`dirname $0`

if [ ! -e $1 ] && [ $1 == *.md ] && [ ! -e `dirname $1`/`basename $1 .md` ]
then exit 2
fi

# Manual page
if [[ $1 == *.3am ]]
then $TOOLDIR/makeweb-man.sh $1 $2

# Texinfo document
elif [[ $1 == *.texi ]]
then $TOOLDIR/makeweb-texi.sh $1 $2

# awk source file
elif [[ $1 == *.awk ]]
then $TOOLDIR/makeweb-code.sh $1 $2

# Markdown document
elif [[ $1 == *.md ]]
then
  if [ -e $1 ]
  then
    src=$1
  else
    srcdir=`dirname $1`
    srcname=`basename $1 .md`
    src=$srcdir/$srcname
  fi
  $TOOLDIR/makeweb-md.sh $src $2

# Plain text file
else
  $TOOLDIR/makeweb-txt.sh $1 $2
fi
