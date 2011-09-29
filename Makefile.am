# Top-level Makefile.am
#
# Copyright (c) 2009, 2011 Reuben Thomas

SUBDIRS = lib

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
AM_CFLAGS = $(WARN_CFLAGS)
LDADD = $(top_builddir)/lib/libgnu.a

bin_PROGRAMS = occurs

occurs_LDADD = $(LDADD) $(LIBOBJS)
occurs_SOURCES = occurs.c cmdline.c cmdline.h

cmdline.c: occurs.ggo
	gengetopt < occurs.ggo --unamed-opts

EXTRA_SRCS = occurs.ggo occurs.lua occurs.py occurs.rb occurs.go occurs.sh
EXTRA_DIST = m4/gnulib-cache.m4 $(EXTRA_SRCS)

CLOC = cloc --force-lang="perl",ggo

ChangeLog:
	git2cl > ChangeLog

release: distcheck
	agrep -d '^Release' $(VERSION) NEWS | tail -n +3 | head -n -1 > release-notes && \
	git diff --exit-code && \
	git tag -a -m "Release tag" rel-`echo $(VERSION) | sed -e 's/\./-/g'` && \
	woger gnu,freshmeat $(PACKAGE) "$(PACKAGE_NAME)" $(VERSION) "the tiny Emacs clone" release-notes && \
	rm -f release-notes

pg100.txt:
	wget http://www.gutenberg.org/ebooks/100.txt.utf8 -O $@

check-local: pg100.txt occurs
	time ./occurs$(EXEEXT) pg100.txt >/dev/null

loc:
	$(CLOC) --no3 occurs.c $(EXTRA_SRCS) occurs.ggo Makefile.am configure.ac
