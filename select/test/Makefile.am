EXTRA_DIST = \
	inputfd.awk \
	inputfd.ok \
	kill.awk \
	kill.ok \
	nonblock.awk \
	nonblock.ok \
	outputfd.awk \
	outputfd.ok \
	select.awk \
	select.ok \
	signal.awk \
	signal.ok \
	timeout_parse.awk \
	timeout_parse.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

LINTTESTS = kill inputfd outputfd
REGTESTS = nonblock select signal timeout_parse

mytests: $(LINTTESTS) $(REGTESTS)

test-msg-start:
	@echo "======== Starting select tests ========"

test-msg-end:
	@echo "======== Done with select tests ========"

$(LINTTESTS):
	@echo $@
	@$(AWK) --lint -l select -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

$(REGTESTS):
	@echo $@
	@$(AWK) -l select -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
