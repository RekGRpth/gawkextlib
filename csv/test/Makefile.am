EXTRA_DIST = \
	comma.csv \
	comma.txt \
	csv.csv \
	csv.ok \
	csvconvert.awk \
	csvconvert.ok \
	csvdump.awk \
	csvformat.awk \
	csvformat.ok \
	csvmode.awk \
	csvmode.csv \
	csvmode.ok \
	csvmode0.awk \
	csvmode0.ok \
	csvsplit.awk \
	csvsplit.ok \
	manyfields.csv \
	manyfields.ok \
	nonascii.csv \
	nonascii.ok \
	switchmode.awk \
	switchmode.ok

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

mytests: csv csvconvert csvformat csvmode csvmode0 csvsplit manyfields nonascii switchmode

test-msg-start:
	@echo "======== Starting csv tests ========"

test-msg-end:
	@echo "======== Done with csv tests ========"

csv::
	@echo $@
	@$(TEST_AWK) -l csv --version | $(TEST_AWK) "/csv Extension/" >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvconvert::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk csv.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvformat::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk csvmode.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvmode::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk csv.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvmode0::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk csvmode.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

csvsplit::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk csv.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

manyfields::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/csvdump.awk $@.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

nonascii::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/csvdump.awk $@.csv >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

switchmode::
	@echo $@
	@$(TEST_AWK) -f $(srcdir)/$@.awk comma.txt comma.csv comma.txt >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
