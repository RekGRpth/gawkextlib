EXTRA_DIST = \
	reclen.awk \
	reclen.in \
	reclen.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

mytests: reclen

test-msg-start:
	@echo "======== Starting reclen tests ========"

test-msg-end:
	@echo "======== Done with reclen tests ========"

reclen::
	@echo $@
	@$(AWK) -l reclen -f $(srcdir)/$@.awk < $(srcdir)/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
