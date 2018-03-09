# Convert markdown files to HTML and PDF
#
# makeweb-md  input-file  output-dir

FILE_NAME=`basename $1`
BASE_NAME=`basename $1 .md`
FILE_DIR=`dirname $1`

pushd $FILE_DIR 1>&2
pandoc -f markdown -s -o $BASE_NAME.pdf $FILE_NAME
pandoc -f markdown -s -o $BASE_NAME.html $FILE_NAME

popd 1>&2
mv $FILE_DIR/$BASE_NAME.pdf $2
mv $FILE_DIR/$BASE_NAME.html $2
