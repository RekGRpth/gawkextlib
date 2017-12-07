# Note: we no longer include the spec file in the distribution tarball.
# With the specfile in the tarball, rpmbuild can work
#  directly on the tarball (e.g. rpmbuild -tb <tarball>).
# The problem is that every time the spec file changes, one must bump the
# version of the whole package, and this is problematic. It doesn't make sense
# to have to change the entire package version just because the spec file
# needed a patch.

#EXTRA_DIST = $(PACKAGE).spec.in $(PACKAGE).spec
