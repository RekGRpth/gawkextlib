EXTRA_DIST = \
	json.awk \
	json.ok \
	json-mdim.awk \
	json-mdim.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

mytests: json json-mdim

test-msg-start:
	@echo "======== Starting json tests ========"

test-msg-end:
	@echo "======== Done with json tests ========"

json::
	@echo $@
	@$(AWK) -l json -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

json-mdim::
	@echo $@
	@$(AWK) -l json -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
