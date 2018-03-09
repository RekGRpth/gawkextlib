# Convert plain text files to HTML
#
# makeweb-txt  input-file  output-dir

FILENAME=`basename $1`
TOOLDIR=`dirname $0`

gawk -f $TOOLDIR/makeweb-txt.awk $1 > $2/$FILENAME.html
