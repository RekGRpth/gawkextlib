# Generate the web directory of some extension: _web/extension
#
#   makeweb.sh  extension  [input-directory]
#
# - the input dir basename is used as the extension name
# - the webTOC file in the input directory specifies the web contents
#

# TOOLDIR/
# - makeweb.sh
# - _web/
  # - makeweb.awk
  # - makeweb-man.sh
  # - makeweb-md.sh
  # - makeweb-texi.sh
  # - template.html
  # - extension/

# INPUTDIR/
  # - webTOC

TOOLDIR=`dirname $0`
WEBDIR=$TOOLDIR/_web/$1
if [ $# -gt 1 ]
then INPUTDIR=$2
else INPUTDIR=$TOOLDIR/$1
fi

# echo $TOOLDIR
# echo $INPUTDIR
# echo $WEBDIR

if [ ! -e $WEBDIR ]
then mkdir $WEBDIR
fi
if [ ! -d $WEBDIR ]
then echo "makeweb: Invalid web directory $WEBDIR" && exit 1
fi

if [ ! -e $INPUTDIR/webTOC ]
then echo "makeweb: File $INPUTDIR/webTOC not found" && exit 1
fi

# cat $INPUTDIR/webTOC

gawk -f $TOOLDIR/_web/makeweb.awk -v webdir=$WEBDIR -v inputdir=$INPUTDIR $TOOLDIR/_web/template.html > $WEBDIR/$1.html
