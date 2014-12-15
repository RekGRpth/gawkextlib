EXTRA_DIST = \
	testerr.awk \
	testerr.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	errno-msg-start errno errno-msg-end
	@$(MAKE) pass-fail

errno: testerr

errno-msg-start:
	@echo "======== Starting errno tests ========"

errno-msg-end:
	@echo "======== Done with errno tests ========"

testerr::
	@echo $@
	@$(AWK) -l errno -f $(srcdir)/$@.awk >_$@.0 2>&1 || echo EXIT CODE: $$? >>_$@.0
	@$(AWK) '!/errno2name: called with invalid argument/' _$@.0 >_$@
	@-rm -f _$@.0
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
