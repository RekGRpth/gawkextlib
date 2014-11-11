AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'

# Remove .la files that are not useful in this context.
install-data-hook:
	for i in $(pkgextension_LTLIBRARIES) ; do \
	   $(RM) $(DESTDIR)$(pkgextensiondir)/$$i ; \
	done

diffout valgrind-scan:
	@cd test && $(MAKE) $(AM_MAKEFLAGS) $@

valgrind:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=full --log-file=log.%p"; \
	make valgrind-scan

valgrind-noleak:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=no --log-file=log.%p"; \
	make valgrind-scan
