# configure.ac

# Copyright (C) 2011-2012 by Werner Lemberg.
#
# This file is part of the ttfautohint library, and may only be used,
# modified, and distributed under the terms given in `COPYING'.  By
# continuing to use, modify, or distribute this file you indicate that you
# have read `COPYING' and understand and accept it fully.
#
# The file `COPYING' mentioned in the previous paragraph is distributed
# with the ttfautohint library.

AC_INIT([ttfautohint],
        [0.8],
        [freetype-devel@nongnu.org])
AC_CONFIG_AUX_DIR([gnulib])
AM_INIT_AUTOMAKE([-Wall -Werror gnits])
AC_CONFIG_MACRO_DIR([gnulib/m4
                     m4])
AM_SILENT_RULES([yes])

AC_PROG_CPP
AC_PROG_CC
AC_PROG_CXX

gl_EARLY
gl_INIT

AC_TYPE_UINT64_T

AT_WITH_QT
AT_REQUIRE_QT_VERSION([4.6])

if test x"$with_qt" != x"no"; then
  AC_MSG_CHECKING([for QLocale::quoteString])
  AS_VERSION_COMPARE([$QT_VERSION], [4.8],
    [AC_MSG_RESULT(no)],
    [AC_MSG_RESULT(no)],
    [AC_MSG_RESULT(yes)
     AC_DEFINE([HAVE_QT_QUOTESTRING], [1],
       [Define if Qt function QLocale::quoteString is available.])])
fi

AM_CONDITIONAL([USE_QT], [test x"$with_qt" != x"no"])

LT_INIT
LT_LTLIZE_LANG([C])


AC_ARG_WITH(freetype-config,
            [  --with-freetype-config=PROG   Use FreeType configuration program PROG],
            freetype_config=$withval,
            freetype_config=yes)

if test "$freetype_config" = "yes"; then 
  AC_PATH_PROG(ft_config,
               freetype-config,
               no)
  if test "$ft_config" = "no"; then
    AC_MSG_ERROR([FreeType library is missing; see http://www.freetype.org/])
  fi
else
  ft_config="$freetype_config"
fi

FREETYPE_CPPFLAGS="`$ft_config --cflags`"
FREETYPE_LIBS="`$ft_config --libtool`"

AC_SUBST(FREETYPE_CPPFLAGS)
AC_SUBST(FREETYPE_LIBS)


AC_MSG_CHECKING([whether FreeType header files are version 2.4.5 or higher])
old_CPPFLAGS="$CPPFLAGS"
CPPFLAGS=$FREETYPE_CPPFLAGS
AC_PREPROC_IFELSE([AC_LANG_SOURCE([[

#include <ft2build.h>
#include FT_FREETYPE_H
#if (FREETYPE_MAJOR*1000 + FREETYPE_MINOR)*1000 + FREETYPE_PATCH < 2004005
#error Freetype version too low.
#endif

]])],
[AC_MSG_RESULT(yes)
 CPPFLAGS="$old_CPPFLAGS"],
[AC_MSG_ERROR([Need FreeType version 2.4.5 or higher])])


AC_MSG_CHECKING([whether FreeType library is version 2.4.5 or higher])
old_CPPFLAGS="$CPPFLAGS"
CPPFLAGS=$FREETYPE_CPPFLAGS
old_LIBS="$LIBS"
LIBS=$FREETYPE_LIBS
AC_LANG_PUSH([LTLIZED C])
AC_RUN_IFELSE([AC_LANG_SOURCE([[

#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int
main()
{
  FT_Error error;
  FT_Library library;
  FT_Int major, minor, patch;

  error = FT_Init_FreeType(&library);
  if (error)
  {
    printf("(test program reports error code %d)... ", error);
    exit(EXIT_FAILURE);
  }

  FT_Library_Version(library, &major, &minor, &patch);

  printf("(found %d.%d.%d)... ", major, minor, patch);

  if (((major*1000 + minor)*1000 + patch) >= 2004005)
    exit(EXIT_SUCCESS);
  exit(EXIT_FAILURE);
}

]])],
[AC_MSG_RESULT(yes)
 CPPFLAGS="$old_CPPFLAGS"
 LIBS="$old_LIBS"],
[AC_MSG_ERROR([Need FreeType version 2.4.5 or higher])])
AC_LANG_POP


if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 gnulib/src/Makefile
                 lib/Makefile
                 frontend/Makefile])
AC_OUTPUT

# end of configure.ac
