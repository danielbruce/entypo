# Makefile.am

# Copyright (C) 2011 by Werner Lemberg.
#
# This file is part of the ttfautohint library, and may only be used,
# modified, and distributed under the terms given in `COPYING'.  By
# continuing to use, modify, or distribute this file you indicate that you
# have read `COPYING' and understand and accept it fully.
#
# The file `COPYING' mentioned in the previous paragraph is distributed
# with the ttfautohint library.

SUFFIXES = .moc.cpp .h

.h.moc.cpp:
	$(MOC) $(QT_CPPFLAGS) $(EXTRA_CPPFLAGS) $< -o $@

DISTCLEANFILES = $(BUILT_SOURCES)

AM_CPPFLAGS = -I$(top_srcdir)/lib \
              -I$(top_builddir)/gnulib/src \
              -I$(top_srcdir)/gnulib/src \
              $(FREETYPE_CPPFLAGS)
LDADD = $(top_builddir)/lib/libttfautohint.a \
        $(top_builddir)/gnulib/src/libgnu.la \
        $(LTLIBINTL) \
        $(LTLIBTHREAD) \
        $(FREETYPE_LIBS)

bin_PROGRAMS = ttfautohint
ttfautohint_SOURCES = info.cpp \
                      main.cpp
dist_man_MANS = ttfautohint.1

if USE_QT
  bin_PROGRAMS += ttfautohintGUI
  ttfautohintGUI_SOURCES = info.cpp \
                           main.cpp \
                           maingui.cpp \
                           maingui.h
  nodist_ttfautohintGUI_SOURCES = maingui.moc.cpp
  ttfautohintGUI_CXXFLAGS = $(QT_CXXFLAGS)
  ttfautohintGUI_LDFLAGS = $(QT_LDFLAGS)
  ttfautohintGUI_CPPFLAGS = $(AM_CPPFLAGS) \
                            $(QT_CPPFLAGS) \
                            -DBUILD_GUI
  ttfautohintGUI_LDADD = $(LDADD) \
                         $(QT_LIBS)

  BUILT_SOURCES = maingui.moc.cpp
  dist_man_MANS += ttfautohintGUI.1
endif


ttfautohint.1: $(top_srcdir)/frontend/main.cpp $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) ttfautohint$(EXEEXT)
	$(HELP2MAN) --output=$@ \
	            --no-info \
	            --name="add new auto-generated hints to a TrueType font" \
	            ./ttfautohint$(EXEEXT)

ttfautohintGUI.1: $(top_srcdir)/frontend/main.cpp $(top_srcdir)/configure.ac
	$(MAKE) $(AM_MAKEFLAGS) ttfautohint$(EXEEXT)
	$(HELP2MAN) --output=$@ \
	            --no-info \
	            --name="add new auto-generated hints to a TrueType font" \
	            --help-option=--help-all \
	            ./ttfautohintGUI$(EXEEXT)

# end of Makefile.am
