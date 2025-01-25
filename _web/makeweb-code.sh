# Convert awk source file to HTML and PDF
#
# makeweb-code  input-file  output-dir

FILE_NAME=`basename $1`
FILE_DIR=`dirname $1`

pushd $FILE_DIR 1>&2

# create a temporary markdown file
echo "$FILE_NAME" > $FILE_NAME.md
echo "========"  >> $FILE_NAME.md
echo ""  >> $FILE_NAME.md
echo "~~~awk" >> $FILE_NAME.md
cat $FILE_NAME >> $FILE_NAME.md
echo '~~~' >> $FILE_NAME.md

pandoc -f markdown -V geometry:margin=2.5cm -s -o $FILE_NAME.pdf $FILE_NAME.md
if [ -e $2/gawkextlib.css ]
then
pandoc -f markdown -c gawkextlib.css -s -o $FILE_NAME.html $FILE_NAME.md
else
pandoc -f markdown -c ../gawkextlib.css -s -o $FILE_NAME.html $FILE_NAME.md
fi

# discard the temporary file
rm -f $FILE_NAME.md

popd 1>&2
mv $FILE_DIR/$FILE_NAME.pdf $2
mv $FILE_DIR/$FILE_NAME.html $2
