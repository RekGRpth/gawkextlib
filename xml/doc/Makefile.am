PKG = gawk-xml

include doc.makefile

# Set AWKPATH and AWKLIBPATH locally at build time
AWK = AWKPATH=.:../awklib/xml AWKLIBPATH=../.libs $(GAWKPROG)

# XHTML source documents
xhtml_files = xmlbase.xhtml xmlcopy.xhtml \
	xmlsimple.xhtml xmltree.xhtml xmlwrite.xhtml

# extra documents
man_MANS = $(xhtml_files:%.xhtml=%.3am)
man_pdf_FILES = $(man_MANS:%.3am=%.pdf)
MAN_FILES = $(man_MANS)
texi_FILES = $(MAN_FILES:%.3am=%.texi)
TEXI_FILES = $(texi_FILES)

CLEANFILES += $(man_MANS) $(man_pdf_FILES) $(texi_FILES)

#redefine EXTRA_DIST because we have more files.
EXTRA_DIST += *ChangeLog \
	lflashlight.eps rflashlight.eps \
	awk_proc.eps soap_reply.eps docbook_chapter.eps \
	rss_inq.eps soap_request.eps soap_error.eps \
	awk_proc.jpg rss_inq.jpg soap_reply.jpg \
	docbook_chapter.jpg  soap_error.jpg  soap_request.jpg \
	awk_proc.pdf docbook_chapter.pdf lflashlight.pdf rflashlight.pdf \
	rss_inq.pdf soap_error.pdf soap_reply.pdf soap_request.pdf \
	$(xhtml_files) \
	$(MAN_FILES) \
	$(TEXI_FILES)

# extra manpage targets
pdf-local: $(man_MANS:%.3am=%.pdf)
#ps-local: $(man_MANS:%.3am=%.ps)
html-local: $(man_MANS:%.3am=%.html)

# Make info CHAPTERs from xhtml files
xhtml2texi_chapter_flags=-v TOPLEVEL=chapter -v NODELEVEL=2 -v NUMBERLEVEL=2

# Make each manpage section a single node inside the info manual
xhtml2texi_section_flags=-v NODELEVEL=1 -v NUMBERLEVEL=1

# Additional dependencies
gawk-xml.info: gawk-xml.texi $(TEXI_FILES)
gawk-xml.pdf: gawk-xml.texi $(TEXI_FILES)

.xhtml.3am:
	$(AWK) -f xhtml2man.awk $<
.xhtml.texi:
	$(AWK) $(xhtml2texi_section_flags) -f xhtml2texi.awk $<
.3am.pdf:
	-groff -man $< | ps2pdf - $@
.3am.html:
	-groff -man -T html $< > $@

maintainer-clean-generic:
	-test -z "$(TEXI_FILES)" || rm -f $(TEXI_FILES)
