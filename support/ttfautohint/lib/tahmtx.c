/* tahmtx.c */

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


FT_Error
TA_sfnt_update_hmtx_table(SFNT* sfnt,
                          FONT* font)
{
  SFNT_Table* hmtx_table;
  FT_Byte* buf_new;
  FT_ULong buf_len;
  FT_ULong i;


  if (sfnt->hmtx_idx == MISSING)
    return TA_Err_Ok;

  hmtx_table = &font->tables[sfnt->hmtx_idx];

  if (hmtx_table->processed)
    return TA_Err_Ok;

  /* the metrics of the added composite element doesn't matter; */
  /* for this reason, we simply append two zero bytes, */
  /* indicating a zero value in the `leftSideBearing' array */
  /* (this works because we don't increase the `numberOfHMetrics' field) */

  hmtx_table->len += 2;
  /* make the allocated buffer length a multiple of 4 */
  buf_len = (hmtx_table->len + 3) & ~3;
  buf_new = (FT_Byte*)realloc(hmtx_table->buf, buf_len);
  if (!buf_new)
  {
    hmtx_table->len -= 2;
    return FT_Err_Out_Of_Memory;
  }

  /* pad end of buffer with zeros */
  for (i = hmtx_table->len - 2; i < buf_len; i++)
    buf_new[i] = 0x00;

  hmtx_table->buf = buf_new;
  hmtx_table->checksum = TA_table_compute_checksum(hmtx_table->buf,
                                                   hmtx_table->len);
  hmtx_table->processed = 1;

  return TA_Err_Ok;
}

/* end of tahmtx.c */
