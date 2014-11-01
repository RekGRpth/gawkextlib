## process this file with automake to produce Makefile.in

EXTRA_DIST = \
	pgtest.awk \
	pgtest.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* core core.* junk out1 out2 out3 test1 test2 seq *~

CMP = cmp

# How do we load libraries needed for tests?
PGSQLLIB   = -l pgsql

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
GEBINPFX=$(GAWKEXTBINDIR)$(PATH_SEPARATOR)
else
GELIBPFX=
GEBINPFX=
endif

AWK = LC_ALL=$${GAWKLOCALE:-C} LANG=$${GAWKLOCALE:-C} AWKLIBPATH=../.libs PATH=$(GEBINPFX)$$PATH LD_LIBRARY_PATH=$(GELIBPFX)$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(GELIBPFX)$$DYLD_LIBRARY_PATH $(VALGRIND) $(GAWKPROG)

if TEST_PGSQL
CHECKPGSQL = pgsql-msg-start   pgsql             pgsql-msg-end
else
CHECKPGSQL = 
endif

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	msg $(CHECKPGSQL)
	@$(MAKE) pass-fail

pgsql:	pgsqlmesg pgtest

msg::
	@echo ''
	@echo 'Any output from "cmp" is bad news, although some differences'
	@echo 'in floating point values are probably benign -- in particular,'
	@echo 'some systems may omit a leading zero and the floating point'
	@echo 'precision may lead to slightly different output in a few cases.'

pgsql-msg-start:
	@echo "======== Starting PGSQL extension tests ========"

pgsql-msg-end:
	@echo "======== Done with PGSQL extension tests ========"

pgsqlmesg::
	@echo These tests work only if the PGSQL extension was successfully built.
	@echo They also require the ability to connect to a PostgreSQL server.
	@echo Note that your system must have the libpq library installed.

pgtest::
	@echo $@
	@$(AWK) $(PGSQLLIB) -v "PQCONNINFO=$(PQCONNINFO)" -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

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
