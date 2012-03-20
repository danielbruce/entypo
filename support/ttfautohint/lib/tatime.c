/* tatime.c */

/*
 * Copyright (C) 2011-2012 by Werner Lemberg.
 *
 * This file is part of the ttfautohint library, and may only be used,
 * modified, and distributed under the terms given in `COPYING'.  By
 * continuing to use, modify, or distribute this file you indicate that you
 * have read `COPYING' and understand and accept it fully.
 *
 * The file `COPYING' mentioned in the previous paragraph is distributed
 * with the ttfautohint library.
 */


#include <time.h>

#include "ta.h"

/* we need an unsigned 64bit data type */

/* make `stdint.h' define `uintXX_t' for C++ */
#undef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

#if defined UINT64_MAX || defined uint64_t
typedef uint64_t TA_ULongLong;
#else
#  error "No unsigned 64bit wide data type found."
#endif


void
TA_get_current_time(FT_ULong* high,
                    FT_ULong* low)
{
  /* there have been 24107 days between January 1st, 1904 (the epoch of */
  /* OpenType), and January 1st, 1970 (the epoch of the `time' function) */
  TA_ULongLong seconds_to_1970 = 24107 * 24 * 60 * 60;
  TA_ULongLong seconds_to_today = seconds_to_1970 + time(NULL);


  *high = (FT_ULong)(seconds_to_today >> 32);
  *low = (FT_ULong)seconds_to_today;
}

/* end of tatime.c */
