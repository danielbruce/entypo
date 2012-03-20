/* tagasp.c */

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


static FT_Error
TA_table_build_gasp(FT_Byte** gasp)
{
  FT_Byte* buf;


  buf = (FT_Byte*)malloc(GASP_LEN);
  if (!buf)
    return FT_Err_Out_Of_Memory;

  /* version */
  buf[0] = 0x00;
  buf[1] = 0x01;

  /* one range */
  buf[2] = 0x00;
  buf[3] = 0x01;

  /* entry valid for all sizes */
  buf[4] = 0xFF;
  buf[5] = 0xFF;
  buf[6] = 0x00;
  buf[7] = 0x0F; /* always use grayscale rendering with grid-fitting, */
                 /* symmetric grid-fitting and symmetric smoothing */

  *gasp = buf;

  return TA_Err_Ok;
}


FT_Error
TA_sfnt_build_gasp_table(SFNT* sfnt,
                         FONT* font)
{
  FT_Error error;

  FT_Byte* gasp_buf;


  error = TA_sfnt_add_table_info(sfnt);
  if (error)
    return error;

  error = TA_table_build_gasp(&gasp_buf);
  if (error)
    return error;

  /* in case of success, `gasp_buf' gets linked */
  /* and is eventually freed in `TA_font_unload' */
  error = TA_font_add_table(font,
                            &sfnt->table_infos[sfnt->num_table_infos - 1],
                            TTAG_gasp, GASP_LEN, gasp_buf);
  if (error)
  {
    free(gasp_buf);
    return error;
  }

  return FT_Err_Ok;
}

/* end of tagasp.c */
