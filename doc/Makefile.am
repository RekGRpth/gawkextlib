#
# doc/Makefile.am --- automake input file for gawk
#
# Copyright (C) 2000, 2001, 2002, 2004, 2005, 2007
# the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

## process this file with automake to produce Makefile.in

info_TEXINFOS = gawk.texi gawkinet.texi xmlgawk.texi
INFO_DEPS = $(srcdir)/xmlgawk.info
if INSTALL_GAWK
INFO_DEPS += $(srcdir)/gawk.info $(srcdir)/gawkinet.info
endif

man_MANS = gawk.1

EXTRA_DIST = *ChangeLog README.card ad.block setter.outline \
	awkcard.in awkforai.txt texinfo.tex cardfonts \
	macros colors no.colors $(man_MANS) \
	lflashlight.eps rflashlight.eps \
	statist.jpg statist.eps \
	fdl.texi \
	awk_proc.eps rflashlight.eps soap_reply.eps docbook_chapter.eps \
	rss_inq.eps soap_request.eps soap_error.eps \
	awk_proc.jpg rss_inq.jpg soap_reply.jpg \
	docbook_chapter.jpg  soap_error.jpg  soap_request.jpg \
	awk_proc.pdf docbook_chapter.pdf lflashlight.pdf rflashlight.pdf \
	rss_inq.pdf soap_error.pdf soap_reply.pdf soap_request.pdf statist.pdf

# Get rid of generated files when cleaning
CLEANFILES = *.ps *.html *.dvi *~ awkcard.nc awkcard.tr gawk.pdf gawkinet.pdf xmlgawk.pdf

MAKEINFO = @MAKEINFO@ --no-split --force

TROFF = groff -t -Tps -U
SEDME = sed -e "s/^level0 restore/level0 restore flashme 100 72 moveto (Copyright `date '+%m-%d-%y %T'`, FSF, Inc. (all)) show/" \
		-e "s/^\/level0 save def/\/level0 save def 30 -48 translate/"

CARDSRC = $(srcdir)/macros $(srcdir)/cardfonts $(srcdir)/colors awkcard.tr
CARDSRC_N = $(srcdir)/macros $(srcdir)/cardfonts $(srcdir)/no.colors awkcard.tr
CARDFILES= $(CARDSRC) ad.block awkcard.in setter.outline

# Use this if your troff can correctly handle macros from 'colors' file
AWKCARD = awkcard.ps

# Uncomment the following definition of AWKCARD if your troff can produce
# Postscript but still has troubles with macros from 'colors'.  As this
# is not groff you will have to change TROFF macro as well.  Do not forget
# to ensure that awkcard.tr is processed by tbl.
#AWKCARD = awkcard.nc

# The following is patterned after the main Makefile.am.  The point is to
# make pgawk.1 a link to gawk.1 in the installed man directory.

# We want hard links for install-data-hook, below
LN= ln

# Link gawk.1 to pgawk.1
install-data-hook:
	(cd $(DESTDIR)$(man1dir); \
	$(LN) $(PACKAGE).1 p$(PACKAGE).1 2>/dev/null ; \
	exit 0)

# Undo the above when uninstalling
uninstall-hook:
	cd $(DESTDIR)$(man1dir); rm -f p$(PACKAGE).1 ; exit 0

postscript: gawk.ps gawkinet.ps xmlgawk.ps gawk.1.ps $(AWKCARD)

gawk.ps: gawk.dvi
	dvips -o gawk.ps gawk.dvi

gawkinet.ps: gawkinet.dvi
	dvips -o gawkinet.ps gawkinet.dvi

xmlgawk.ps: xmlgawk.dvi
	dvips -o xmlgawk.ps xmlgawk.dvi

gawk.1.ps: gawk.1
	-groff -man $(srcdir)/gawk.1 > gawk.1.ps

awkcard.tr: awkcard.in
	sed 's:SRCDIR:$(srcdir):' < $(srcdir)/awkcard.in > awkcard.tr

awkcard.ps: $(CARDFILES)
	$(TROFF) $(CARDSRC) | $(SEDME) | cat $(srcdir)/setter.outline - > awkcard.ps

awkcard.nc: $(CARDFILES)
	$(TROFF) $(CARDSRC_N) | $(SEDME) | cat $(srcdir)/setter.outline - > awkcard.ps && touch awkcard.nc

