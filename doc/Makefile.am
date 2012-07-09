## process this file with automake to produce Makefile.in

info_TEXINFOS = xmlgawk.texi

EXTRA_DIST = *ChangeLog \
	lflashlight.eps rflashlight.eps \
	fdl.texi \
	awk_proc.eps soap_reply.eps docbook_chapter.eps \
	rss_inq.eps soap_request.eps soap_error.eps \
	awk_proc.jpg rss_inq.jpg soap_reply.jpg \
	docbook_chapter.jpg  soap_error.jpg  soap_request.jpg \
	awk_proc.pdf docbook_chapter.pdf lflashlight.pdf rflashlight.pdf \
	rss_inq.pdf soap_error.pdf soap_reply.pdf soap_request.pdf

# Get rid of generated files when cleaning
CLEANFILES = *.ps *.html *.dvi *~ xmlgawk.pdf

MAKEINFO = @MAKEINFO@ --no-split --force

TROFF = groff -t -Tps -U
SEDME = sed -e "s/^level0 restore/level0 restore flashme 100 72 moveto (Copyright `date '+%m-%d-%y %T'`, FSF, Inc. (all)) show/" \
		-e "s/^\/level0 save def/\/level0 save def 30 -48 translate/"

postscript: xmlgawk.ps

xmlgawk.ps: xmlgawk.dvi
	dvips -o xmlgawk.ps xmlgawk.dvi

