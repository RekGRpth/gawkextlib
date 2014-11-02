EXTRA_DIST = \
	xgdrect.awk \
	xgdrect.ok \
	xgdrect.ok.png \
	xgdfont.awk \
	xgdfont.ok \
	xgdfont.ok.c \
	xgd.ttf

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* xgdfont.ok.??? junk out1 out2 out3 test1 test2 seq *~

# How do we load libraries needed for tests?
GDLIB   = -l gd

include test.makefile

check:	gd-msg-start gd gd-msg-end
	@$(MAKE) pass-fail

gd: xgdrect xgdfont

gd-msg-start:
	@echo "======== Starting GD extension tests ========"

gd-msg-end:
	@echo "======== Done with GD extension tests ========"

xgdrect::
	@echo $@
	@$(AWK) $(GDLIB) -v srcdir=$(srcdir) -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@

xgdfont.ok.exe: $(srcdir)/xgdfont.ok.c
	$(CC) -o $@ $^ -lgd `gdlib-config --libs`

xgdfont:: xgdfont.ok.exe
	@echo $@
	@GDFONTPATH=$(srcdir); export GDFONTPATH; ./xgdfont.ok.exe >_$@ 2>&1; $(AWK) $(GDLIB) -f $(srcdir)/$@.awk >>_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
