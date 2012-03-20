/* tasort.c */

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


/* originally file `afangles.c' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#include "tatypes.h"
#include "tasort.h"


/* two bubble sort routines */

void
ta_sort_pos(FT_UInt count,
            FT_Pos* table)
{
  FT_UInt i;
  FT_UInt j;
  FT_Pos swap;


  for (i = 1; i < count; i++)
  {
    for (j = i; j > 0; j--)
    {
      if (table[j] > table[j - 1])
        break;

      swap = table[j];
      table[j] = table[j - 1];
      table[j - 1] = swap;
    }
  }
}


void
ta_sort_widths(FT_UInt count,
               TA_Width table)
{
  FT_UInt i;
  FT_UInt j;
  TA_WidthRec swap;


  for (i = 1; i < count; i++)
  {
    for (j = i; j > 0; j--)
    {
      if (table[j].org > table[j - 1].org)
        break;

      swap = table[j];
      table[j] = table[j - 1];
      table[j - 1] = swap;
    }
  }
}

/* end of tasort.c */
