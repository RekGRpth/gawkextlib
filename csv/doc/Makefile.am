PKG = gawk-csv

include doc.makefile

xhtml_files = csv.xhtml csvparse.xhtml csvmode.xhtml
man_MANS = $(xhtml_files:%.xhtml=%.3am)
man_pdf_FILES = $(man_MANS:%.3am=%.pdf)
man_html_FILES = $(man_MANS:%.3am=%.html)
dist_man_MANS = csvconvert.3am csvsplit.3am
#info_TEXINFOS += csv.texi csvparse.texi csvmode.texi
#info_TEXINFOS += $(in_xhtml_files:%.in.xhtml=%.texi)
MAN_FILES = $(man_MANS)
TEXI_FILES = $(MAN_FILES:%.3am=%.texi) gawk-csv-tutorial.texi

# ********** Temporary fix: overrides the rule in shared/doc.makefile ************
EXTRA_DIST = fdl-1.3.texi
# ********************************************************************************

EXTRA_DIST += csvconvert.3am csvsplit.3am \
			  gawk-csv-tutorial.xhtml \
			  $(xhtml_files) \
			  $(MAN_FILES) \
			  $(TEXI_FILES)

pdf-local: $(man_pdf_FILES)
#ps-local: $(man_MANS:%.3am=%.ps)
html-local: $(man_html_FILES)

gawk-csv.info: gawk-csv.texi $(TEXI_FILES)
gawk-csv.pdf: gawk-csv.texi $(TEXI_FILES)
gawk-csv.ps: gawk-csv.texi $(TEXI_FILES)
gawk-csv.html: gawk-csv.texi $(TEXI_FILES)

# Make info CHAPTERs from in.xhtml files
xhtml2texi_chapter_flags=-v TOPLEVEL=chapter -v NODELEVEL=2 -v NUMBERLEVEL=2

gawk-csv-tutorial.texi: gawk-csv-tutorial.xhtml
	$(GAWKPROG) $(xhtml2texi_chapter_flags) -f xhtml2texi.awk $<

# Make each manpage section a single node inside the info manual
xhtml2texi_section_flags=-v NODELEVEL=1 -v NUMBERLEVEL=1

.xhtml.3am:
	$(GAWKPROG) -f xhtml2man.awk $<
.xhtml.texi:
	$(GAWKPROG) $(xhtml2texi_section_flags) -f xhtml2texi.awk $<
.3am.pdf:
	-groff -man $< | ps2pdf - $@
.3am.html:
	-groff -man -T html $< > $@

maintainer-clean-generic:
	-test -z "$(TEXI_FILES)" || rm -f $(TEXI_FILES)
