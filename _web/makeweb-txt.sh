# Convert plain text files to HTML
#
# makeweb-txt  input-file  output-dir

FILENAME=`basename $1`
TOOLDIR=`dirname $0`

gawk -f $TOOLDIR/makeweb-txt.awk $1 > $2/$FILENAME.html
pandoc -f html -t pdf -V geometry:margin=2.5cm -o $2/$FILENAME.pdf $2/$FILENAME.html
