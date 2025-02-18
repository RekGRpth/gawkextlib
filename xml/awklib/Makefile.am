## process this file with automake to produce Makefile.in

EXTRA_DIST = \
	*ChangeLog \
	xml/*.awk \
	xml/*.sh \
	xml/*ChangeLog

bin_SCRIPTS = xmlgawk

all: $(bin_SCRIPTS)

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	for i in $(srcdir)/xml/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done

# pkglibexecdir and pkgdatadir are removed in the top level Makefile's uninstall
uninstall-local:
	for i in $(srcdir)/xml/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		rm -f $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done
	rm -f $(DESTDIR)$(bindir)/xmlgawk

clean-local:
	rm -f xmlgawk

xmlgawk: $(srcdir)/xml/xmlgawk.sh
	cp $^ $@ ; chmod 755 $@

