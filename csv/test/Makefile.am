EXTRA_DIST = \
	csv.in \
	csv.ok \
	csvconvert.awk \
	csvconvert.ok \
	csvmode.in \
	csvmode.awk \
	csvmode.ok \
	csvsplit.awk \
	csvsplit.ok \
	csvdump.awk \
	manyfields.in \
	manyfields.ok \
	nonascii.in \
	nonascii.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Set also AWKPATH locally at build time (*** SHOULD BE IN test.makefile ***)
TEST_AWK = AWKPATH=.:../awklib/csv $(AWK)

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@$(MAKE) pass-fail
#	@$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

mytests: csv csvconvert csvmode csvsplit manyfields nonascii

test-msg-start:
	@echo "======== Starting csv tests ========"

test-msg-end:
	@echo "======== Done with csv tests ========"

csv::
	@echo $@
	@$(AWK) -l csv --version | $(AWK) "/csv Extension/" >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvmode::
	@echo $@
	@$(TEST_AWK) -i csv -f $(srcdir)/$@.awk $@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvconvert::
	@echo $@
	@$(AWK) -l csv -f $(srcdir)/$@.awk csv.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvsplit::
	@echo $@
	@$(AWK) -l csv -f $(srcdir)/$@.awk csv.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

manyfields::
	@echo $@
	@$(TEST_AWK) -l csv -f $(srcdir)/csvdump.awk $@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

nonascii::
	@echo $@
	@$(TEST_AWK) -l csv -f $(srcdir)/csvdump.awk $@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
