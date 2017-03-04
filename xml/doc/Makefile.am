PKG = gawk-xml

include doc.makefile

# Invoke the xml extension locally, before being installed 
# (sets PATH, AWKLIBPATH and locales)
include test.makefile

# Set also AWKPATH locally at build time (*** SHOULD BE IN test.makefile ***)
DOC_AWK = AWKPATH=.:../awklib/xml $(AWK)

# XHTML source documents
xhtml_files = xmlbase.xhtml xmlcopy.xhtml \
	xmlsimple.xhtml xmltree.xhtml xmlwrite.xhtml

# extra documents
man3_MANS = $(xhtml_files:%.xhtml=%.3am)
man_pdf_FILES = $(man3_MANS:%.3am=%.3am.pdf)
man_html_FILES = $(man3_MANS:%.3am=%.3am.html)
MAN_FILES = $(man3_MANS)
texi_FILES = $(MAN_FILES:%.3am=%.texi)
TEXI_FILES = $(texi_FILES)

CLEANFILES += $(man3_MANS) $(man_pdf_FILES) $(man_html_FILES) $(texi_FILES)

image_source_FILES = docbook_chapter.gv rss_inq.gv \
	soap_error.gv soap_reply.gv soap_request.gv
image_eps_FILES = $(image_source_FILES:%.gv=%.eps)
image_pdf_FILES = $(image_source_FILES:%.gv=%.pdf)
image_png_FILES = $(image_source_FILES:%.gv=%.png)
image_txt_FILES = $(image_source_FILES:%.gv=%.txt)

#redefine EXTRA_DIST because we have more files.
EXTRA_DIST += *ChangeLog \
	lflashlight.eps rflashlight.eps \
	lflashlight.pdf rflashlight.pdf \
	$(image_source_FILES) \
	$(image_eps_FILES) \
	$(image_pdf_FILES) \
	$(image_png_FILES) \
	$(image_txt_FILES) \
	README.graphviz \
	xhtml2man.awk xhtml2texi.awk \
	$(xhtml_files) \
	$(MAN_FILES) \
	$(TEXI_FILES)

# extra manpage targets
pdf-local: $(man_pdf_FILES)
#ps-local: $(man_MANS:%.3am=%.ps)
html-local: $(man_html_FILES)

# Make info CHAPTERs from xhtml files
xhtml2texi_chapter_flags=-v TOPLEVEL=chapter -v NODELEVEL=2 -v NUMBERLEVEL=2

# Make each manpage section a single node inside the info manual
xhtml2texi_section_flags=-v NODELEVEL=1 -v NUMBERLEVEL=1

# Additional dependencies
gawk-xml.info: gawk-xml.texi $(TEXI_FILES) $(image_txt_FILES)
gawk-xml.pdf: gawk-xml.texi $(TEXI_FILES) $(image_pdf_FILES)
gawk-xml.ps: gawk-xml.texi $(TEXI_FILES) $(image_eps_FILES)
gawk-xml.html: gawk-xml.texi $(TEXI_FILES) $(image_png_FILES)

.xhtml.3am:
	$(DOC_AWK) -f xhtml2man.awk $<
.xhtml.texi:
	$(DOC_AWK) $(xhtml2texi_section_flags) -f xhtml2texi.awk $<

xmlbase.3am.pdf: xmlbase.3am
	-groff -man $< | ps2pdf - $@
xmlcopy.3am.pdf: xmlcopy.3am
	-groff -man $< | ps2pdf - $@
xmlsimple.3am.pdf: xmlsimple.3am
	-groff -man $< | ps2pdf - $@
xmltree.3am.pdf: xmltree.3am
	-groff -man $< | ps2pdf - $@
xmlwrite.3am.pdf: xmlwrite.3am
	-groff -man $< | ps2pdf - $@

xmlbase.3am.html: xmlbase.3am
	-groff -man -T html $< > $@
xmlcopy.3am.html: xmlcopy.3am
	-groff -man -T html $< > $@
xmlsimple.3am.html: xmlsimple.3am
	-groff -man -T html $< > $@
xmltree.3am.html: xmltree.3am
	-groff -man -T html $< > $@
xmlwrite.3am.html: xmlwrite.3am
	-groff -man -T html $< > $@

maintainer-clean-generic:
	-test -z "$(TEXI_FILES)" || rm -f $(TEXI_FILES)
