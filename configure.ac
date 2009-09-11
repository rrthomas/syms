dnl configure.ac
dnl
dnl Copyright (c) 2009 Reuben Thomas

dnl Initialise autoconf and automake
AC_INIT(occurs, 0.591, rrt@sc3d.org)
AM_INIT_AUTOMAKE([-Wall -Werror std-options])

dnl Set up C compiler
AC_GNU_SOURCE
AC_PROG_CC
if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -W -Wmissing-prototypes -Wstrict-prototypes -D_FORTIFY_SOURCE=2"
fi
gl_EARLY
AM_PROG_CC_C_O

dnl Checks for functions and headers
gl_INIT
AC_HEADER_STDC

dnl Generate output
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES(Makefile lib/Makefile)
AC_OUTPUT
