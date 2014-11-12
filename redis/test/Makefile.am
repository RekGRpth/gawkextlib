EXTRA_DIST = \
	testredis.awk \
	testredis.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# How do we load libraries needed for tests?
REDISLIB = -l redis

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	redis-msg-start   redis             redis-msg-end
	@$(MAKE) pass-fail

redis:	testredis

redis-msg-start:
	@echo "======== Starting REDIS extension tests ========"

redis-msg-end:
	@echo "======== Done with REDIS extension tests ========"

testredis::
	@echo $@
	@$(AWK) $(REDISLIB) -f $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) $(srcdir)/$@.ok _$@ && rm -f _$@
