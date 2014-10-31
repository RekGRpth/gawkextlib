## process this file with automake to produce Makefile.in

PKG = gawk-gd

info_TEXINFOS = $(PKG).texi

EXTRA_DIST = fdl.texi 

# Get rid of generated files when cleaning
CLEANFILES = *.ps *.html *.dvi *~ $(PKG).pdf

MAKEINFO = @MAKEINFO@ --no-split --force

TROFF = groff -t -Tps -U
SEDME = sed -e "s/^level0 restore/level0 restore flashme 100 72 moveto (Copyright `date '+%m-%d-%y %T'`, FSF, Inc. (all)) show/" \
		-e "s/^\/level0 save def/\/level0 save def 30 -48 translate/"

postscript: $(PKG).ps

$(PKG).ps: $(PKG).dvi
	dvips -o $(PKG).ps $(PKG).dvi

