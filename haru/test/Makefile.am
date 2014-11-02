EXTRA_DIST = \
	pdftest.awk \
	pdftest.ok \
	pdftest.png

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	haru-msg-start haru haru-msg-end
	@$(MAKE) pass-fail

haru: pdftest

haru-msg-start:
	@echo "======== Starting haru tests ========"

haru-msg-end:
	@echo "======== Done with haru tests ========"

pdftest::
	@echo $@
	@$(AWK) -v "srcdir=$(srcdir)" -f $(srcdir)/$@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
