/* tadsig.c */

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


#include "ta.h"


/* we build a dummy `DSIG' table only */

FT_Error
TA_table_build_DSIG(FT_Byte** DSIG)
{
  FT_Byte* buf;


  buf = (FT_Byte*)malloc(DSIG_LEN);
  if (!buf)
    return FT_Err_Out_Of_Memory;

  /* version */
  buf[0] = 0x00;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = 0x01;

  /* zero signatures */
  buf[4] = 0x00;
  buf[5] = 0x00;

  /* permission flags */
  buf[6] = 0x00;
  buf[7] = 0x00;

  *DSIG = buf;

  return TA_Err_Ok;
}


/* end of tadsig.c */
