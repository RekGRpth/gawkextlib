# Do not use cmp because MinGW cmp does not ignore DOS CR-LF diffs.
CMP = diff -qa --strip-trailing-cr

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

# MSYS bash converts colons to semi-colons, which mucks up the path,
# so try to protect by replacing "<drive letter>:/" with "/<drive letter>/"
# in the AWKLIBPATH value. We unset AWKLIBPATH to protect against a user-set
# value in the environment; I think we want only the built-in value here.
AWK = LC_ALL=$${GAWKLOCALE:-C} LANG=$${GAWKLOCALE:-C} AWKLIBPATH=../.libs$(PATH_SEPARATOR)`unset AWKLIBPATH && $(GAWKPROG) 'BEGIN {print gensub(/\<([[:alpha:]]):\//, "/\\\1/", "g", ENVIRON["AWKLIBPATH"])}'` PATH=$(GEBINPFX)$$PATH LD_LIBRARY_PATH=$(GELIBPFX)$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(GELIBPFX)$$DYLD_LIBRARY_PATH $(VALGRIND) $(GAWKPROG)

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
	done

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
