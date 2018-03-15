# Generate the web directory of some extension: _web/extension
#
#   makeweb.sh  extension  [input-directory]
#
# - the input dir basename is used as the extension name
# - the webTOC file in the input directory specifies the web contents
#

# TOOLDIR/   == gawkextlib source tree root
# - makeweb.sh
# - <extension>/
  # - webTOC
  # - <files>
# - _web/
  # - makeweb.awk
  # - makeweb-man.sh
  # - makeweb-md.sh
  # - makeweb-texi.sh
  # - template.html
  # - <extension>/

# INPUTDIR/
  # - webTOC

# Input and output directories
TOOLDIR=`dirname $0`
EXTDIR=$TOOLDIR/$1
WEBDIR=$TOOLDIR/_web/$1
if [ $# -gt 1 ]
then INPUTDIR=$2
else INPUTDIR=$TOOLDIR/$1
fi

# Web page specification
if [ -e $INPUTDIR/webTOC ]
then WEBTOC=$INPUTDIR/webTOC
else if [ -e $EXTDIR/webTOC ]
then WEBTOC=$EXTDIR/webTOC
else echo "makeweb: File $EXTDIR/webTOC not found" && exit 1
fi
fi

# echo $TOOLDIR
# echo $INPUTDIR
# echo $WEBDIR

# Create the web directory if necessary
if [ ! -e $WEBDIR ]
then mkdir $WEBDIR
fi
if [ ! -d $WEBDIR ]
then echo "makeweb: Invalid web directory $WEBDIR" && exit 1
fi

# cat $INPUTDIR/webTOC

# Run the web generator
gawk -f $TOOLDIR/_web/makeweb.awk -v webdir=$WEBDIR -v inputdir=$INPUTDIR -v webtoc=$WEBTOC $TOOLDIR/_web/template.html > $WEBDIR/$1.html
