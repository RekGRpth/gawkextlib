# Convert man pages to HTML and PDF
#
# makeweb-man  input-file  output-dir

# echo $0 $@

TOOLDIR=`dirname $0`

FILE_NAME=`basename $1`
groff -man $1 | ps2pdf - $2/$FILE_NAME.pdf
groff -man -T html $1 | gawk -f $TOOLDIR/makeweb-css.awk > $2/$FILE_NAME.html
