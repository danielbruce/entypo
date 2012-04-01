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
#else
#  include "info.h"
#endif

#include <ttfautohint.h>

using namespace std;


#ifndef BUILD_GUI
extern "C" {

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

} // extern "C"
#endif // !BUILD_GUI


#ifdef CONSOLE_OUTPUT
static void
show_help(bool
#ifdef BUILD_GUI
               all
#endif
                  ,
          bool is_error)
{
  FILE* handle = is_error ? stderr : stdout;

  fprintf(handle,
#ifdef BUILD_GUI
"Usage: ttfautohintGUI [OPTION]...\n"
"A GUI application to replace hints in a TrueType font.\n"
#else
"Usage: ttfautohint [OPTION]... IN-FILE OUT-FILE\n"
"Replace hints in TrueType font IN-FILE and write output to OUT-FILE.\n"
#endif
"The new hints are based on FreeType's autohinter.\n"
"\n"
"This program is a simple front-end to the `ttfautohint' library.\n"
"\n");

  fprintf(handle,
"Long options can be given with one or two dashes,\n"
"and with and without equal sign between option and argument.\n"
"This means that the following forms are acceptable:\n"
"`-foo=bar', `--foo=bar', `-foo bar', `--foo bar'.\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n"
#ifdef BUILD_GUI
"Options not related to Qt or X11 set default values.\n"
#endif
"\n"
);

  fprintf(handle,
"Options:\n"
"  -f, --latin-fallback       set fallback script to latin\n"
"  -G, --hinting-limit=N      switch off hinting above this PPEM value\n"
"                             (default: %d); value 0 means no limit\n"
"  -h, --help                 display this help and exit\n"
#ifdef BUILD_GUI
"      --help-all             show Qt and X11 specific options also\n"
#endif
"  -i, --ignore-permissions   override font license restrictions\n"
"  -l, --hinting-range-min=N  the minimum PPEM value for hint sets\n"
"                             (default: %d)\n"
"  -n  --no-info              don't add ttfautohint info\n"
"                             to the version string(s) in the `name' table\n"
"  -p, --pre-hinting          apply original hints in advance\n",
          TA_HINTING_LIMIT, TA_HINTING_RANGE_MIN);
  fprintf(handle,
"  -r, --hinting-range-max=N  the maximum PPEM value for hint sets\n"
"                             (default: %d)\n"
"  -s, --symbol               input is symbol font\n"
"  -v, --verbose              show progress information\n"
"  -V, --version              print version information and exit\n"
"  -x, --increase-x-height    increase x height for small sizes\n"
"  -X, --x-height-snapping-exceptions=STRING\n"
"                             specify a comma-separated list of\n"
"                             x-height snapping exceptions\n"
"\n",
          TA_HINTING_RANGE_MAX);

#ifdef BUILD_GUI
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
#endif // BUILD_GUI

  fprintf(handle,
"The program accepts both TTF and TTC files as input.\n"
"Use option -i only if you have a legal permission to modify the font.\n"
"If option -f is not set, glyphs not in the latin range stay unhinted.\n"
"The used PPEM value for option -p is FUnits per em, normally 2048.\n"
"With option -s, use default values for standard stem width and height,\n"
"otherwise they are derived from latin character `o'.\n"
"\n");
  fprintf(handle,
"A hint set contains the optimal hinting for a certain PPEM value;\n"
"the larger the hint set range, the more hint sets get computed,\n"
"usually increasing the output font size.  Note, however,\n"
"that the `gasp' table of the output file enables grayscale hinting\n"
"for all sizes (limited by option -G which is handled in the bytecode).\n"
"\n");
  fprintf(handle,
#ifdef BUILD_GUI
"A command-line version of this program is called `ttfautohint'.\n"
#else
"A GUI version of this program is called `ttfautohintGUI'.\n"
#endif
"\n"
"Report bugs to: freetype-devel@nongnu.org\n"
"ttfautohint home page: <http://www.freetype.org/ttfautohint>\n");

  if (is_error)
    exit(EXIT_FAILURE);
  else
    exit(EXIT_SUCCESS);
}


static void
show_version()
{
  fprintf(stdout,
#ifdef BUILD_GUI
"ttfautohintGUI " VERSION "\n"
#else
"ttfautohint " VERSION "\n"
#endif
"Copyright (C) 2011-2012 Werner Lemberg <wl@gnu.org>.\n"
"License: FreeType License (FTL) or GNU GPLv2.\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n");

  exit(EXIT_SUCCESS);
}
#endif // CONSOLE_OUTPUT


int
main(int argc,
     char** argv)
{
  int hinting_range_min = 0;
  int hinting_range_max = 0;
  int hinting_limit = 0;
  bool have_hinting_range_min = false;
  bool have_hinting_range_max = false;
  bool have_hinting_limit = false;

  bool ignore_permissions = false;
  bool pre_hinting = false;
  bool increase_x_height = false;
  bool no_info = false;
  int latin_fallback = 0; // leave it as int; this probably gets extended
  bool symbol = false;

#ifndef BUILD_GUI
  TA_Progress_Func progress_func = NULL;
  TA_Info_Func info_func = info;
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
#ifdef BUILD_GUI
      {"help-all", no_argument, NULL, HELP_ALL_OPTION},
#endif

      // ttfautohint options
      {"hinting-limit", required_argument, NULL, 'G'},
      {"hinting-range-max", required_argument, NULL, 'r'},
      {"hinting-range-min", required_argument, NULL, 'l'},
      {"ignore-permissions", no_argument, NULL, 'i'},
      {"increase-x-height", no_argument, NULL, 'x'},
      {"latin-fallback", no_argument, NULL, 'f'},
      {"no-info", no_argument, NULL, 'n'},
      {"pre-hinting", no_argument, NULL, 'p'},
      {"symbol", no_argument, NULL, 's'},
      {"verbose", no_argument, NULL, 'v'},
      {"version", no_argument, NULL, 'V'},
      {"x-height-snapping-exceptions", required_argument, NULL, 'X'},

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
    int c = getopt_long_only(argc, argv, "fG:hil:npr:stVvxX:",
                             long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
    case 'f':
      latin_fallback = 1;
      break;

    case 'G':
      hinting_limit = atoi(optarg);
      have_hinting_limit = true;
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

    case 'n':
      no_info = true;
      break;

    case 'p':
      pre_hinting = true;
      break;

    case 'r':
      hinting_range_max = atoi(optarg);
      have_hinting_range_max = true;
      break;

    case 's':
      symbol = true;
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
      increase_x_height = true;
      break;

    case 'X':
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
    hinting_range_min = TA_HINTING_RANGE_MIN;
  if (!have_hinting_range_max)
    hinting_range_max = TA_HINTING_RANGE_MAX;
  if (!have_hinting_limit)
    hinting_limit = TA_HINTING_LIMIT;

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
  if (hinting_limit != 0 && hinting_limit < hinting_range_max)
  {
    fprintf(stderr, "A non-zero hinting limit must not be smaller"
                    " than the hinting range maximum (%d)\n",
                    hinting_range_max);
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

  unsigned char version_data[128];
  unsigned char version_data_wide[256];

  const unsigned char* error_string;
  Progress_Data progress_data = {-1, 1, 0};
  Info_Data info_data;

  if (no_info)
    info_func = NULL;
  else
  {
    info_data.data = version_data;
    info_data.data_wide = version_data_wide;

    info_data.hinting_range_min = hinting_range_min;
    info_data.hinting_range_max = hinting_range_max;
    info_data.hinting_limit = hinting_limit;

    info_data.pre_hinting = pre_hinting;
    info_data.increase_x_height = increase_x_height;
    info_data.latin_fallback = latin_fallback;
    info_data.symbol = symbol;

    build_version_string(&info_data);
  }

  TA_Error error =
    TTF_autohint("in-file, out-file,"
                 "hinting-range-min, hinting-range-max, hinting-limit,"
                 "error-string,"
                 "progress-callback, progress-callback-data,"
                 "info-callback, info-callback-data,"
                 "ignore-permissions, pre-hinting, increase-x-height,"
                 "fallback-script, symbol",
                 in, out,
                 hinting_range_min, hinting_range_max, hinting_limit,
                 &error_string,
                 progress_func, &progress_data,
                 info_func, &info_data,
                 ignore_permissions, pre_hinting, increase_x_height,
                 latin_fallback, symbol);

  if (error)
  {
    if (error == TA_Err_Invalid_FreeType_Version)
      fprintf(stderr,
              "FreeType version 2.4.5 or higher is needed.\n"
              "Perhaps using a wrong FreeType DLL?\n");
    else if (error == TA_Err_Already_Processed)
      fprintf(stderr,
              "This font has already been processed with ttfautohint.\n");
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

#else // BUILD_GUI

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

  Main_GUI gui(hinting_range_min, hinting_range_max, hinting_limit,
               ignore_permissions, pre_hinting, increase_x_height,
               no_info, latin_fallback, symbol);
  gui.show();

  return app.exec();

#endif // BUILD_GUI
}

// end of main.cpp
