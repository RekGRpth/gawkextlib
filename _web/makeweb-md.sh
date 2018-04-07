# Convert markdown files to HTML and PDF
#
# makeweb-md  input-file  output-dir

FILE_NAME=`basename $1`
BASE_NAME=`basename $1 .md`
FILE_DIR=`dirname $1`

pushd $FILE_DIR 1>&2
pandoc -f markdown -V geometry:margin=2.5cm -s -o $BASE_NAME.pdf $FILE_NAME
if [ -e $2/gawkextlib.css ]
then
pandoc -f markdown -c gawkextlib.css -s -o $BASE_NAME.html $FILE_NAME
else
pandoc -f markdown -c ../gawkextlib.css -s -o $BASE_NAME.html $FILE_NAME
fi

popd 1>&2
mv $FILE_DIR/$BASE_NAME.pdf $2
mv $FILE_DIR/$BASE_NAME.html $2
