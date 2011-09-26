# Top-level Makefile.am
#
# Copyright (c) 2009 Reuben Thomas

SUBDIRS = lib

ACLOCAL_AMFLAGS = -I m4

ChangeLog:
	git2cl > ChangeLog

release: distcheck
	agrep -d '^Release' $(VERSION) NEWS | tail -n +3 | head -n -1 > release-notes && \
	git diff --exit-code && \
	git tag -a -m "Release tag" rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	woger gnu,freshmeat $(PACKAGE) "$(PACKAGE_NAME)" $(VERSION) "the tiny Emacs clone" release-notes && \
	rm -f release-notes

CLOC = cloc --force-lang="perl",ggo

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
LDADD = $(top_builddir)/lib/libgnu.a

bin_PROGRAMS = occurs

occurs_LDADD = $(LDADD) $(LIBOBJS)

occurs_SOURCES = occurs.c cmdline.c cmdline.h

cmdline.c: occurs.ggo
	gengetopt < occurs.ggo --unamed-opts

EXTRA_DIST = occurs.ggo m4/gnulib-cache.m4

loc:
	$(CLOC) --no3 occurs.c occurs.ggo Makefile.am configure.ac
