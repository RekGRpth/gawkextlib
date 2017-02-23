## process this file with automake to produce Makefile.in

EXTRA_DIST = csv/*.awk

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	for i in $(srcdir)/csv/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done

# pkglibexecdir and pkgdatadir are removed in the top level Makefile's uninstall
uninstall-local:
	rm -fr $(DESTDIR)$(pkgdatadir)/*
