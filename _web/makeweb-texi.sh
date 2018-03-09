# Convert .texi files to HTML and PDF
#
# makeweb-texi  input-file  output-dir

FILE_NAME=`basename $1 .texi`
INPUT_DIR=`dirname $1`
texi2dvi --pdf --batch --build-dir=$INPUT_DIR/$FILE_NAME.t2d -o $2/$FILE_NAME.pdf $1 1>&2
makeinfo --html --no-split --force -o $2 $1
