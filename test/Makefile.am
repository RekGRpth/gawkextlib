## process this file with automake to produce Makefile.in

EXTRA_DIST = \
	*ChangeLog \
	Gentests \
	Maketests \
	pgtest.awk \
	pgtest.ok \
	printlang.awk \
	xmlinterleave.awk \
	xmlinterleave.ok \
	xdocbook.awk \
	xdocbook.in \
	xdocbook.ok \
	xdeep.awk \
	xdeep.ok \
	xdeep2.awk \
	xdeep2.ok \
	xattr.awk \
	xattr.ok \
	xfujutf8.awk \
	xfujutf8.in \
	xfujutf8.ok \
	xfujeucj.in \
	xfujeucj.ok \
	xotlsjis.in \
	xotlsjis.ok \
	xload.ok \
	xmpfrmat.awk \
	xmpfrmat.ok \
	xmpfrall.awk \
	xmpfrall.ok \
	xgdrect.awk \
	xgdrect.ok \
	xgdrect.ok.png \
	xgdfont.awk \
	xgdfont.ok \
	xgdfont.ok.c \
	xgd.ttf

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* xgdfont.ok.??? junk out1 out2 out3 strftime.ok test1 test2 seq *~

XML_TESTS = xdocbook xdeep2 xattr xfujutf8 xotlsjis xfujeucj xload xmlinterleave xuintmax

PGSQL_TESTS = pgtest

MPFR_TESTS = xmpfrmat xmpfrall

GD_TESTS = xgdrect xgdfont

# List of the files that appear in manual tests or are for reserve testing:
GENTESTS_UNUSED = Makefile.in

CMP = cmp

# How do we load libraries needed for tests?
PGSQLLIB   = -l pgsql
MPFRLIB = -l mpfr
GDLIB   = -l gd

# The expat XML parser returns data in UTF-8 encoding.  Specifying the
#  UTF-8 encoding for XMLCHARSET reduces problems on platforms with limited 
#  iconv capabilities (where iconv_open() may otherwise fail).
XMLLIB = -l xml -v XMLCHARSET=UTF-8

# Default for VALGRIND is empty unless overridden by a command-line argument.
# This protects against cruft in the environment.
VALGRIND = 

# This business forces the locale to be C for running the tests,
# unless we override it to something else for testing.
#
# This can also be done in individual tests where we wish to
# check things specifically not in the C locale.
AWK = LC_ALL=$${GAWKLOCALE:-C} LANG=$${GAWKLOCALE:-C} AWKLIBPATH=../extension/.libs $(VALGRIND) $(GAWKPROG)

if BUILD_XML
CHECKXML = xml-msg-start    xml             xml-msg-end
else
CHECKXML = 
endif

if TEST_PGSQL
CHECKPGSQL = pgsql-msg-start   pgsql             pgsql-msg-end
else
CHECKPGSQL = 
endif

if BUILD_MPFR
CHECKMPFR = mpfr-msg-start   mpfr             mpfr-msg-end
else
CHECKMPFR = 
endif

if BUILD_GD
CHECKGD = gd-msg-start    gd            gd-msg-end
else
CHECKGD =
endif

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	msg \
	printlang \
	$(CHECKXML) \
	$(CHECKPGSQL) \
	$(CHECKMPFR) \
	$(CHECKGD)
	@$(MAKE) pass-fail

xml:	xmlmesg $(XML_TESTS)

pgsql:	pgsqlmesg $(PGSQL_TESTS)

mpfr:	mpfrmesg $(MPFR_TESTS)

gd:	gdmesg $(GD_TESTS)

msg::
	@echo ''
	@echo 'Any output from "cmp" is bad news, although some differences'
	@echo 'in floating point values are probably benign -- in particular,'
	@echo 'some systems may omit a leading zero and the floating point'
	@echo 'precision may lead to slightly different output in a few cases.'

printlang::
	@echo ''
	@echo "AWK = $(AWK)"
	@$(AWK) -f $(srcdir)/printlang.awk

xml-msg-start:
	@echo "======== Starting XML extension tests ========"

xml-msg-end:
	@echo "======== Done with XML extension tests ========"

pgsql-msg-start:
	@echo "======== Starting PGSQL extension tests ========"

pgsql-msg-end:
	@echo "======== Done with PGSQL extension tests ========"

mpfr-msg-start:
	@echo "======== Starting MPFR extension tests ========"

mpfr-msg-end:
	@echo "======== Done with MPFR extension tests ========"

gd-msg-start:
	@echo "======== Starting GD extension tests ========"

gd-msg-end:
	@echo "======== Done with GD extension tests ========"

xmlmesg::
	@echo These tests only work if the XML extension was successfully built.
	@echo Notice that your system must have an XML parser \(Expat\) installed.

pgsqlmesg::
	@echo These tests work only if the PGSQL extension was successfully built.
	@echo They also require the ability to connect to a PostgreSQL server.
	@echo Notice that your system must have the libpq library installed.

mpfrmesg::
	@echo These tests only work if the MPFR extension was successfully built.
	@echo Notice that your system must have an MPFR library installed.

gdmesg::
	@echo These tests only work if the GD extension was successfully built.
	@echo Notice that your system must have the GD graphics library installed.


xmpfrmat::
	@echo $@
	@$(AWK) $(MPFRLIB) -f $(srcdir)/$@.awk 257  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

xmpfrall::
	@echo $@
	@$(AWK) $(MPFRLIB) -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

pgtest::
	@echo $@
	@$(AWK) $(PGSQLLIB) -v "PQCONNINFO=$(PQCONNINFO)" -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

xgdrect::
	@echo $@
	@$(AWK) $(GDLIB) -v srcdir=$(srcdir) -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

xgdfont.ok.exe: $(srcdir)/xgdfont.ok.c
	$(CC) -o $@ $^ -lgd `gdlib-config --libs`

xgdfont:: xgdfont.ok.exe
	@echo $@
	@GDFONTPATH=$(srcdir); export GDFONTPATH; ./xgdfont.ok.exe >_$@ 2>&1; $(AWK) $(GDLIB) -f $(srcdir)/$@.awk >>_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# An overall test case for detecting if the most important XML content types are recognized.
xdocbook::
	@echo $@
	@WHINY_USERS=1 $(AWK) $(XMLLIB) -f $(srcdir)/xdocbook.awk < $(srcdir)/xdocbook.in > _$@ 2>&1  || echo EXIT CODE: $$? >> _$@ 
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Test every XML variable maintained by the extension library where we
# alternate reading records from different files.  This is intended to be
# a comprehensive test.
xmlinterleave::
	@echo $@
	@$(AWK) $(XMLLIB) -f $(srcdir)/$@.awk $(srcdir)/xdocbook.in $(srcdir)/../extension/sample.xml $(srcdir)/xfujeucj.in $(srcdir)/xfujutf8.in $(srcdir)/xotlsjis.in > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@ 
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# This test fails on many systems because the stack size of the
# systems is limited. Therefore, it is not contained in the list
# of XML tests.
xdeep::
	@echo $@
	@$(AWK) -f $(srcdir)/xdeep.awk 2>&1 | $(AWK) $(XMLLIB) 'BEGIN {XMLMODE=1};XMLSTARTELEM{d=XMLDEPTH};END {print d}'  >_$@  2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Produce XML data with tags nested to depth 10000 and see if it can be read.
xdeep2::
	@echo $@
	@$(AWK) -f $(srcdir)/xdeep2.awk 2>&1 | $(AWK) $(XMLLIB) 'BEGIN {XMLMODE=1};XMLSTARTELEM{d=XMLDEPTH};END {print d}'  >_$@ 2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Produce XML data with 10000 attributes in one tag and see if it can be read.
xattr::
	@echo $@
	@$(AWK) -f $(srcdir)/xattr.awk 2>&1 | $(AWK) $(XMLLIB) 'BEGIN {XMLMODE=1};XMLSTARTELEM{print XMLATTR[$$NF]}'  >_$@ 2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Like outline.awk, prints an outline of a Japanese document, encoded with UTF-8.
xfujutf8::
	@echo $@
	@$(AWK) $(XMLLIB) -f $(srcdir)/xfujutf8.awk < $(srcdir)/xfujutf8.in 2>&1 > _$@ 2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Like outline.awk, prints an outline of a Japanese document, encoded with EUC-JP.
xfujeucj::
	@echo $@
	@$(AWK) $(XMLLIB) -f $(srcdir)/xfujutf8.awk < $(srcdir)/xfujeucj.in 2>&1 > _$@ 2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Like outline.awk, prints an outline of a Japanese document, encoded with x-sjis-cp932.
xotlsjis::
	@echo $@
	@$(AWK) $(XMLLIB) -f $(srcdir)/xfujutf8.awk < $(srcdir)/xotlsjis.in 2>&1 > _$@ 2>&1  || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Check if the loading of extensions works correctly.
xload::
	@$(AWK) $(XMLLIB) --lint $(XMLLIB)    'BEGIN {print NR, ERRNO}'   > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $(srcdir)/$@.ok _$@ &&  rm -f _$@

# Check if the platform supports 64 bit integers.
xuintmax::
	@echo $@
	@$(AWK) 'BEGIN {bignumber=lshift(1,55); if (bignumber==0) print "\tyour environment supports only 32 bit integers" ; else if (int(log(bignumber))==38) print "\tyour environment supports 64 bit integers" ; else print "\tyour environment has problems with large integers "}'


# Targets generated for other tests:

# Note: we do not want automake to process the "include" directive,
#   so we prepend a "-" (which has the undesirable side-effect of suppressing
#   make error messages if it is unable to include the file).  Prepending a
#   space also seems to work, but that seems rather fragile.
#   Is there some better way to escape this to prevent automake processing?
-include Maketests

$(srcdir)/Maketests: $(srcdir)/Makefile.am $(srcdir)/Gentests
	files=`cd "$(srcdir)" && echo *.awk *.in`; \
	$(AWK) -f $(srcdir)/Gentests "$(srcdir)/Makefile.am" $$files > $(srcdir)/Maketests

# An attempt to print something that can be grepped for in build logs
pass-fail:
	@COUNT=`ls _* 2>/dev/null | wc -l` ; \
	if test $$COUNT = 0 ; \
	then	echo ALL TESTS PASSED ; \
	else	echo $$COUNT TESTS FAILED ; \
	fi

# This target for my convenience to look at all the results
diffout:
	for i in _* ; \
	do  \
		if [ "$$i" != "_*" ]; then \
		echo ============== $$i ============= ; \
		if [ -r $${i#_}.ok ]; then \
		diff -c $${i#_}.ok $$i ; \
		else \
		diff -c $(srcdir)/$${i#_}.ok  $$i ; \
		fi ; \
		fi ; \
	done | more

# convenient way to scan valgrind results for errors
valgrind-scan:
	@echo "Scanning valgrind log files for problems:"
	@$(AWK) '\
	function show() {if (cmd) {printf "%s: %s\n",FILENAME,cmd; cmd = ""}; \
	  printf "\t%s\n",$$0}; \
	{$$1 = ""}; \
	$$2 == "Command:" {incmd = 1; $$2 = ""; cmd = $$0; next}; \
	incmd {if (/Parent PID:/) incmd = 0; else {cmd = (cmd $$0); next}}; \
	/ERROR SUMMARY:/ && !/: 0 errors from 0 contexts/ {show()}; \
	/definitely lost:/ && !/: 0 bytes in 0 blocks/ {show()}; \
	/possibly lost:/ && !/: 0 bytes in 0 blocks/ {show()}; \
	/ suppressed:/ && !/: 0 bytes in 0 blocks/ {show()}; \
	' log.[0-9]*

# This target is for testing with electric fence.
efence:
	for i in $$(ls _* | sed 's;_\(.*\);\1;') ; \
	do \
		bad=$$(wc -l < _$$i) \
		ok=$$(wc -l < $$i.ok) ; \
		if (( $$bad == $$ok + 2 )) ; \
		then \
			rm _$$i ; \
		fi ; \
	done
