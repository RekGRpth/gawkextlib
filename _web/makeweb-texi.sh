# Convert .texi files to HTML and PDF
#
# makeweb-texi  input-file  output-dir

FILE_NAME=`basename $1 .texi`
INPUT_DIR=`dirname $1`
texi2dvi --pdf --batch -o $2/$FILE_NAME.pdf $1 1>&2
makeinfo --html --no-split --force --css-ref="../texihtml.css" -o $2 $1
