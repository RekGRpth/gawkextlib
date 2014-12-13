info_TEXINFOS = $(PKG).texi

EXTRA_DIST = fdl.texi 

# Get rid of generated files when cleaning
CLEANFILES = *.ps *.html *.dvi *~ $(PKG).pdf

MAKEINFO = @MAKEINFO@ --no-split --force

postscript: $(PKG).ps

$(PKG).ps: $(PKG).dvi
	dvips -o $(PKG).ps $(PKG).dvi

