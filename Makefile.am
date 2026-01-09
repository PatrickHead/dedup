AM_CPPFLAGS = -I $(srcdir)/include

AM_CFLAGS = -O3 -g0 -Wall

ARFLAGS = cr

lib_LIBRARIES = lib/libdedup.a
lib_libdedup_a_SOURCES = src/dedup.c src/libdedup.c src/strapp.c include/dedup.h include/strapp.h

bin_PROGRAMS = bin/dedup
bin_dedup_SOURCES = src/dedup.c
bin_dedup_LDADD = lib/libdedup.a -lavl -lllist

include_HEADERS = include/dedup.h include/strapp.h

EXTRA_DIST = examples man acdoxygen.m4 amdoxygen.am doxygen.dedup.cfg dedup.pc .gitignore

man1_MANS = man/dedup.1
dist_man_man1_MANS = man/dedup.1

super-clean: clean distclean
	@find . -name 'Makefile.in' -delete
	@find . -name 'config.h.in' -delete
	@find . -name '*.tar' -delete
	@find . -name '*.tar.*' -delete
	@find . -name '*~' -delete
	@rm -rf autom4te.cache
	@rm -f aclocal.m4
	@rm -f configure
	@rm -rf src/.deps
	@rm -f texput.log

include amdoxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

EXTRA_DIST += $(DX_CONFIG)

install-data-hook:
	@if test -d $(DX_HTML_DIR); \
	 then \
	   echo installing $(DX_HTML_DIR) to $(prefix)/share/doc/dedup; \
	   mkdir -p $(prefix)/share/doc/dedup; \
	   cp -r $(DX_HTML_DIR) $(prefix)/share/doc/dedup; \
	 fi
	@echo installing dedup.pc
	@mkdir -p $(prefix)/share/pkgconfig
	@cp dedup.pc $(prefix)/share/pkgconfig/dedup.pc

uninstall-hook:
	@rm -rf $(prefix)/share/doc/dedup
	@rm -rf $(prefix)/share/pkgconfig/dedup.pc

code-stats:
	@echo -n " total lines of source code: "; wc -l src/*.[ch] include/*.[ch] | tail -1 | sed -e 's/ total//' -e 's/^  *//'
	@echo -n "actual lines of source code: "; (gcc -fpreprocessed -dD -E src/*.[ch] include/*.[ch] | sed -e '/^$$/d' -e '/^# [0-9]/d' | wc -l)

