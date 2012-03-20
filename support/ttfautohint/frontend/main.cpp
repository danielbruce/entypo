// main.cpp

// Copyright (C) 2011-2012 by Werner Lemberg.
//
// This file is part of the ttfautohint library, and may only be used,
// modified, and distributed under the terms given in `COPYING'.  By
// continuing to use, modify, or distribute this file you indicate that you
// have read `COPYING' and understand and accept it fully.
//
// The file `COPYING' mentioned in the previous paragraph is distributed
// with the ttfautohint library.


// This program is a wrapper for `TTF_autohint'.

#ifdef BUILD_GUI
#  ifndef _WIN32
#    define CONSOLE_OUTPUT
#  endif
#else
#  define CONSOLE_OUTPUT
#endif

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <vector>
#include <string>

#if BUILD_GUI
#  include <QApplication>
#  include "maingui.h"
#endif

#include <ttfautohint.h>

using namespace std;


#ifndef BUILD_GUI
typedef struct Progress_Data_
{
  long last_sfnt;
  bool begin;
  int last_percent;
} Progress_Data;


int
progress(long curr_idx,
         long num_glyphs,
         long curr_sfnt,
         long num_sfnts,
         void* user)
{
  Progress_Data* data = (Progress_Data*)user;

  if (num_sfnts > 1 && curr_sfnt != data->last_sfnt)
  {
    fprintf(stderr, "subfont %ld of %ld\n", curr_sfnt + 1, num_sfnts);
    data->last_sfnt = curr_sfnt;
    data->begin = true;
  }

  if (data->begin)
  {
    fprintf(stderr, "  %ld glyphs\n"
                    "   ", num_glyphs);
    data->begin = false;
  }

  // print progress approx. every 10%
  int curr_percent = curr_idx * 100 / num_glyphs;
  int curr_diff = curr_percent - data->last_percent;

  if (curr_diff >= 10)
  {
    fprintf(stderr, " %d%%", curr_percent);
    data->last_percent = curr_percent - curr_percent % 10;
  }

  if (curr_idx + 1 == num_glyphs)
    fprintf(stderr, "\n");

  return 0;
}
#endif /* !BUILD_GUI */


#ifdef CONSOLE_OUTPUT
static void
show_help(bool all,
          bool is_error)
{
  FILE* handle = is_error ? stderr : stdout;

  fprintf(handle,
"Usage: ttfautohint [OPTION]... IN-FILE OUT-FILE\n"
"  or:  ttfautohintGUI [OPTION]...\n"
"Replace hints in TrueType font IN-FILE and write output to OUT-FILE.\n"
"The new hints are based on FreeType's autohinter.\n"
"\n"
"These programs (for console and GUI, respectively)\n"
"are simple front-ends to the `ttfautohint' library.\n"
"\n");

  fprintf(handle,
"Long options can be given with one or two dashes,\n"
"and with and without equal sign between option and argument.\n"
"This means that the following forms are acceptable:\n"
"`-foo=bar', `--foo=bar', `-foo bar', `--foo bar'.\n"
"\n");

  fprintf(handle,
"Options:\n"
"  -f, --latin-fallback       set fallback script to latin\n"
"  -h, --help                 display this help and exit\n"
"      --help-all             show Qt and X11 specific options also\n"
"  -i, --ignore-permissions   override font license restrictions\n"
"  -l, --hinting-range-min=N  the minimum ppem value for hint sets\n"
"  -p, --pre-hinting          apply original hints in advance\n");
  fprintf(handle,
"  -r, --hinting-range-max=N  the maximum ppem value for hint sets\n"
"  -v, --verbose              show progress information\n"
"  -V, --version              print version information and exit\n"
"  -x, --x-height-snapping-exceptions=STRING\n"
"                             specify a comma-separated list of\n"
"                             x-height snapping exceptions\n"
"\n");

  if (all)
  {
    fprintf(handle,
"Qt Options:\n"
"      --graphicssystem=SYSTEM\n"
"                             select a different graphics system backend\n"
"                             instead of the default one\n"
"                             (possible values: `raster', `opengl')\n"
"      --reverse              set layout direction to right-to-left\n");
    fprintf(handle,
"      --session=ID           restore the application for the given ID\n"
"      --style=STYLE          set application GUI style\n"
"                             (possible values: motif, windows, platinum)\n"
"      --stylesheet=SHEET     apply the given Qt stylesheet\n"
"                             to the application widgets\n"
"\n");

    fprintf(handle,
"X11 options:\n"
"      --background=COLOR     set the default background color\n"
"                             and an application palette\n"
"                             (light and dark shades are calculated)\n"
"      --bg=COLOR             same as --background\n"
"      --btn=COLOR            set the default button color\n"
"      --button=COLOR         same as --btn\n"
"      --cmap                 use a private color map on an 8-bit display\n"
"      --display=NAME         use the given X-server display\n");
    fprintf(handle,
"      --fg=COLOR             set the default foreground color\n"
"      --fn=FONTNAME          set the application font\n"
"      --font=FONTNAME        same as --fn\n"
"      --foreground=COLOR     same as --fg\n"
"      --geometry=GEOMETRY    set the client geometry of first window\n"
"      --im=SERVER            set the X Input Method (XIM) server\n"
"      --inputstyle=STYLE     set X Input Method input style\n"
"                             (possible values: onthespot, overthespot,\n"
"                             offthespot, root)\n");
    fprintf(handle,
"      --name=NAME            set the application name\n"
"      --ncols=COUNT          limit the number of colors allocated\n"
"                             in the color cube on an 8-bit display,\n"
"                             if the application is using the\n"
"                             QApplication::ManyColor color specification\n"
"      --title=TITLE          set the application title (caption)\n"
"      --visual=VISUAL        force the application\n"
"                             to use the given visual on an 8-bit display\n"
"                             (only possible value: TrueColor)\n"
"\n");
  }

  fprintf(handle,
"The programs accept both TTF and TTC files as input.\n"
"Use option -i only if you have a legal permission to modify the font.\n"
"If option -f is not set, glyphs not in the latin range stay unhinted.\n"
"The used ppem value for option -p is FUnits per em, normally 2048.\n"
"\n");
  fprintf(handle,
"A hint set contains the optimal hinting for a certain PPEM value;\n"
"the larger the hint set range, the more hint sets get computed,\n"
"usually increasing the output font size.  Note, however,\n"
"that the `gasp' table of the output file enables grayscale hinting\n"
"for all sizes.\n"
"\n");
  fprintf(handle,
"If run in GUI mode, options not related to Qt or X11 set default values.\n"
"Additionally, there is no output to the console.\n"
"\n"
"GUI support might be disabled at compile time.\n"
"\n"
"Report bugs to: freetype-devel@nongnu.org\n"
"FreeType home page: <http://www.freetype.org>\n");

  if (is_error)
    exit(EXIT_FAILURE);
  else
    exit(EXIT_SUCCESS);
}


static void
show_version()
{
  fprintf(stdout,
"ttfautohint " VERSION "\n"
"Copyright (C) 2011-2012 Werner Lemberg <wl@gnu.org>.\n"
"License: FreeType License (FTL) or GNU GPLv2.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n");

  exit(EXIT_SUCCESS);
}
#endif /* CONSOLE_OUTPUT */


int
main(int argc,
     char** argv)
{
  int hinting_range_min = 0;
  int hinting_range_max = 0;
  bool have_hinting_range_min = false;
  bool have_hinting_range_max = false;

  bool ignore_permissions = false;
  bool pre_hinting = false;
  int latin_fallback = 0; // leave it as int; this probably gets extended

#ifndef BUILD_GUI
  TA_Progress_Func progress_func = NULL;
#endif

  // make GNU, Qt, and X11 command line options look the same;
  // we allow `--foo=bar', `--foo bar', `-foo=bar', `-foo bar',
  // and short options specific to ttfautohint

  // set up a new argument string
  vector<string> new_arg_string;
  new_arg_string.push_back(argv[0]);

  while (1)
  {
    // use pseudo short options for long-only options
    enum
    {
      PASS_THROUGH = CHAR_MAX + 1,
      HELP_ALL_OPTION
    };

    static struct option long_options[] =
    {
      {"help", no_argument, NULL, 'h'},
      {"help-all", no_argument, NULL, HELP_ALL_OPTION},

      // ttfautohint options
      {"hinting-range-max", required_argument, NULL, 'r'},
      {"hinting-range-min", required_argument, NULL, 'l'},
      {"ignore-permissions", no_argument, NULL, 'i'},
      {"latin-fallback", no_argument, NULL, 'f'},
      {"pre-hinting", no_argument, NULL, 'p'},
      {"verbose", no_argument, NULL, 'v'},
      {"version", no_argument, NULL, 'V'},
      {"x-height-snapping-exceptions", required_argument, NULL, 'x'},

      // Qt options
      {"graphicssystem", required_argument, NULL, PASS_THROUGH},
      {"reverse", no_argument, NULL, PASS_THROUGH},
      {"session", required_argument, NULL, PASS_THROUGH},
      {"style", required_argument, NULL, PASS_THROUGH},
      {"stylesheet", required_argument, NULL, PASS_THROUGH},

      // X11 options
      {"background", required_argument, NULL, PASS_THROUGH},
      {"bg", required_argument, NULL, PASS_THROUGH},
      {"btn", required_argument, NULL, PASS_THROUGH},
      {"button", required_argument, NULL, PASS_THROUGH},
      {"cmap", no_argument, NULL, PASS_THROUGH},
      {"display", required_argument, NULL, PASS_THROUGH},
      {"fg", required_argument, NULL, PASS_THROUGH},
      {"fn", required_argument, NULL, PASS_THROUGH},
      {"font", required_argument, NULL, PASS_THROUGH},
      {"foreground", required_argument, NULL, PASS_THROUGH},
      {"geometry", required_argument, NULL, PASS_THROUGH},
      {"im", required_argument, NULL, PASS_THROUGH},
      {"inputstyle", required_argument, NULL, PASS_THROUGH},
      {"name", required_argument, NULL, PASS_THROUGH},
      {"ncols", required_argument, NULL, PASS_THROUGH},
      {"title", required_argument, NULL, PASS_THROUGH},
      {"visual", required_argument, NULL, PASS_THROUGH},

      {NULL, 0, NULL, 0}
    };

    int option_index;
    int c = getopt_long_only(argc, argv, "fhil:r:ptVvx:",
                             long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
    case 'f':
      latin_fallback = 1;
      break;

    case 'h':
#ifdef CONSOLE_OUTPUT
      show_help(false, false);
#endif
      break;

    case 'i':
      ignore_permissions = true;
      break;

    case 'l':
      hinting_range_min = atoi(optarg);
      have_hinting_range_min = true;
      break;

    case 'r':
      hinting_range_max = atoi(optarg);
      have_hinting_range_max = true;
      break;

    case 'p':
      pre_hinting = true;
      break;

    case 'v':
#ifndef BUILD_GUI
      progress_func = progress;
#endif
      break;

    case 'V':
#ifdef CONSOLE_OUTPUT
      show_version();
#endif
      break;

    case 'x':
#ifdef CONSOLE_OUTPUT
      fprintf(stderr, "Option `-x' not implemented yet\n");
#endif
      break;

    case HELP_ALL_OPTION:
#ifdef CONSOLE_OUTPUT
      show_help(true, false);
#endif
      break;

    case PASS_THROUGH:
      {
        // append argument with proper syntax for Qt
        string arg;
        arg += '-';
        arg += long_options[option_index].name;

        new_arg_string.push_back(arg);
        if (optarg)
          new_arg_string.push_back(optarg);
        break;
      }

    default:
      exit(EXIT_FAILURE);
    }
  }

  if (!have_hinting_range_min)
    hinting_range_min = 8;
  if (!have_hinting_range_max)
    hinting_range_max = 1000;

#ifndef BUILD_GUI

  if (hinting_range_min < 2)
  {
    fprintf(stderr, "The hinting range minimum must be at least 2\n");
    exit(EXIT_FAILURE);
  }
  if (hinting_range_max < hinting_range_min)
  {
    fprintf(stderr, "The hinting range maximum must not be smaller"
                    " than the minimum (%d)\n",
                    hinting_range_min);
    exit(EXIT_FAILURE);
  }
  // on the console we need in and out file arguments
  if (argc - optind != 2)
    show_help(false, true);

  FILE* in = fopen(argv[optind], "rb");
  if (!in)
  {
    fprintf(stderr, "The following error occurred while opening font `%s':\n"
                    "\n"
                    "  %s\n",
                    argv[optind], strerror(errno));
    exit(EXIT_FAILURE);
  }

  FILE* out = fopen(argv[optind + 1], "wb");
  if (!out)
  {
    fprintf(stderr, "The following error occurred while opening font `%s':\n"
                    "\n"
                    "  %s\n",
                    argv[optind + 1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  const unsigned char* error_string;
  Progress_Data progress_data = {-1, 1, 0};

  TA_Error error =
    TTF_autohint("in-file, out-file,"
                 "hinting-range-min, hinting-range-max,"
                 "error-string,"
                 "progress-callback, progress-callback-data,"
                 "ignore-permissions, pre-hinting, fallback-script",
                 in, out,
                 hinting_range_min, hinting_range_max,
                 &error_string,
                 progress_func, &progress_data,
                 ignore_permissions, pre_hinting, latin_fallback);

  if (error)
  {
    if (error == TA_Err_Invalid_FreeType_Version)
      fprintf(stderr,
              "FreeType version 2.4.5 or higher is needed.\n"
              "Perhaps using a wrong FreeType DLL?\n");
    else if (error == TA_Err_Missing_Legal_Permission)
      fprintf(stderr,
              "Bit 1 in the `fsType' field of the `OS/2' table is set:\n"
              "This font must not be modified"
                " without permission of the legal owner.\n"
              "Use command line option `-i' to continue"
                " if you have such a permission.\n");
    else if (error == TA_Err_Missing_Unicode_CMap)
      fprintf(stderr,
              "No Unicode character map.\n");
    else if (error == TA_Err_Missing_Glyph)
      fprintf(stderr,
              "No glyph for the key character"
              " to derive standard width and height.\n"
              "For the latin script, this key character is `o' (U+006F).\n");
    else
      fprintf(stderr,
              "Error code `0x%02x' while autohinting font:\n"
              "  %s\n", error, error_string);
    exit(EXIT_FAILURE);
  }

  fclose(in);
  fclose(out);

  exit(EXIT_SUCCESS);

  return 0; // never reached

#else /* BUILD_GUI */

  int new_argc = new_arg_string.size();
  char** new_argv = new char*[new_argc];

  // construct new argc and argv variables from collected data
  for (int i = 0; i < new_argc; i++)
    new_argv[i] = const_cast<char*>(new_arg_string[i].data());

  QApplication app(new_argc, new_argv);
  app.setApplicationName("TTFautohint");
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("FreeType");
  app.setOrganizationDomain("freetype.org");

  Main_GUI gui(hinting_range_min, hinting_range_max,
               ignore_permissions, pre_hinting, latin_fallback);
  gui.show();

  return app.exec();

#endif /* BUILD_GUI */
}

// end of main.cpp
