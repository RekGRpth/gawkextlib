EXTRA_DIST = \
	pgtest.awk \
	pgtest.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# How do we load libraries needed for tests?
PGSQLLIB   = -l pgsql

if TEST_PGSQL
CHECKPGSQL = pgsql-msg-start   pgsql             pgsql-msg-end
else
CHECKPGSQL = 
endif

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	$(CHECKPGSQL)
	@$(MAKE) pass-fail

pgsql:	pgsqlmesg pgtest

pgsql-msg-start:
	@echo "======== Starting PGSQL extension tests ========"

pgsql-msg-end:
	@echo "======== Done with PGSQL extension tests ========"

pgsqlmesg::
	@echo These tests work only if the PGSQL extension was successfully built.
	@echo They also require the ability to connect to a PostgreSQL server.
	@echo Note that your system must have the libpq library installed.

pgtest::
	@echo $@
	@$(AWK) $(PGSQLLIB) -v "PQCONNINFO=$(PQCONNINFO)" -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
