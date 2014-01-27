# Top-level Makefile.am
#
# Copyright (c) 2009, 2011-2012 Reuben Thomas

SUBDIRS = lib

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
AM_CFLAGS = $(WARN_CFLAGS)
LDADD = $(top_builddir)/lib/libgnu.a

bin_PROGRAMS = syms

syms_LDADD = $(LDADD) $(LIBOBJS)
syms_SOURCES = syms.c cmdline.c cmdline.h

cmdline.c: opts.ggo
	gengetopt < opts.ggo --unamed-opts

EXTRA_SRCS = opts.ggo syms.lua syms.py syms.rb syms.go syms.sh
EXTRA_DIST = m4/gnulib-cache.m4 $(EXTRA_SRCS)

CLOC = cloc --force-lang="perl",ggo

ChangeLog:
	git2cl > ChangeLog

pg100.txt:
	wget http://www.gutenberg.org/ebooks/100.txt.utf8 -O $@

check-local: pg100.txt syms
	time ./syms$(EXEEXT) pg100.txt >/dev/null

loc:
	$(CLOC) --no3 syms.c $(EXTRA_SRCS) opts.ggo Makefile.am configure.ac
