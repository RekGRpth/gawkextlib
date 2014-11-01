## process this file with automake to produce Makefile.in

EXTRA_DIST = \
	beginfile.awk \
	beginfile.ok \
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
	xload.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* core core.* junk out1 out2 out3 test1 test2 seq *~

XML_TESTS = xdocbook xdeep2 xattr xfujutf8 xotlsjis xfujeucj xload xmlinterleave beginfile

CMP = cmp

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

# For Cygwin to find libgawkextlib, we need to set the PATH.
# Similarly, Mac OS requires us to set DYLD_LIBRARY_PATH.  Just to
# be safe, we will set LD_LIBRARY_PATH also.  I hope that covers all
# platforms.
if GELIBDIR
GELIBPFX=$(GAWKEXTLIBDIR)$(PATH_SEPARATOR)
else
GELIBPFX=
endif

AWK = LC_ALL=$${GAWKLOCALE:-C} LANG=$${GAWKLOCALE:-C} AWKLIBPATH=../.libs PATH=$(GELIBPFX)$$PATH LD_LIBRARY_PATH=$(GELIBPFX)$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(GELIBPFX)$$DYLD_LIBRARY_PATH $(VALGRIND) $(GAWKPROG)

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	msg printlang xml-msg-start    xml             xml-msg-end
	@$(MAKE) pass-fail

xml:	xmlmesg $(XML_TESTS)

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

xmlmesg::
	@echo These tests only work if the XML extension was successfully built.
	@echo Note that your system must have an XML parser \(Expat\) installed.

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
	@$(AWK) $(XMLLIB) -f $(srcdir)/$@.awk $(srcdir)/xdocbook.in $(srcdir)/../sample.xml $(srcdir)/xfujeucj.in $(srcdir)/xfujutf8.in $(srcdir)/xotlsjis.in > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@ 
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

# Check that the BEGINFILE rule can select the parser
beginfile::
	@echo $@
	@$(AWK) $(XMLLIB) -f $(srcdir)/$@.awk $(srcdir)/$@.awk $(srcdir)/../sample.xml $(srcdir)/$@.awk $(srcdir)/../sample.xml > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@ 
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

# An attempt to print something that can be grepped for in build logs
pass-fail:
	@COUNT=`ls _* 2>/dev/null | wc -l` ; \
	if test $$COUNT = 0 ; \
	then	echo ALL TESTS PASSED ; \
	else	echo $$COUNT TESTS FAILED ; exit 1; \
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
