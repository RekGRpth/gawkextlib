EXTRA_DIST = \
	xmpfrmat.awk \
	xmpfrmat.ok \
	xmpfrall.awk \
	xmpfrall.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# How do we load libraries needed for tests?
MPFRLIB = -l mpfr

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	mpfr-msg-start   mpfr             mpfr-msg-end
	@$(MAKE) pass-fail

mpfr:	xmpfrmat xmpfrall

mpfr-msg-start:
	@echo "======== Starting MPFR extension tests ========"

mpfr-msg-end:
	@echo "======== Done with MPFR extension tests ========"

xmpfrmat::
	@echo $@
	@$(AWK) $(MPFRLIB) -f $(srcdir)/$@.awk 257  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

xmpfrall::
	@echo $@
	@$(AWK) $(MPFRLIB) -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
