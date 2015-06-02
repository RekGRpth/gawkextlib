EXTRA_DIST = \
	nl_langinfo.awk \
	nl_langinfo.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

mytests: nl_langinfo

test-msg-start:
	@echo "======== Starting nl_langinfo tests ========"

test-msg-end:
	@echo "======== Done with nl_langinfo tests ========"

nl_langinfo::
	@echo $@
	@LC_ALL=C $(AWK) -l nl_langinfo -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
