/* tapost.c */

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


#include <string.h>

#include "ta.h"


FT_Error
TA_sfnt_update_post_table(SFNT* sfnt,
                          FONT* font)
{
  SFNT_Table* post_table;
  FT_Byte* buf;
  FT_ULong buf_len;
  FT_Byte* buf_new;
  FT_ULong buf_new_len;

  FT_ULong version;


  if (sfnt->post_idx == MISSING)
    return TA_Err_Ok;

  post_table = &font->tables[sfnt->post_idx];
  buf = post_table->buf;
  buf_len = post_table->len;

  if (post_table->processed)
    return TA_Err_Ok;

  version = buf[0] << 24;
  version += buf[1] << 16;
  version += buf[2] << 8;
  version += buf[3];

  /* since we have to add a non-standard glyph name, */
  /* we must convert `name' tables in formats 1.0 and 2.5 into format 2.0 */

  if (version == 0x10000)
  {
    /* XXX */
  }
  else if (version == 0x20000)
  {
    FT_UShort num_glyphs;
    FT_UShort max_name_idx;
    FT_ULong len;
    FT_UShort i;
    FT_Byte* p;
    FT_Byte* p_new;


    num_glyphs = buf[32] << 8;
    num_glyphs += buf[33];

    p = buf + 34;
    max_name_idx = 0;

    /* get number of glyph names stored in `names' array */
    for (i = 0; i < num_glyphs; i++)
    {
      FT_UShort idx;


      idx = *(p++) << 8;
      idx += *(p++);
      if (idx >= 258)
      {
        idx -= 257;
        if (idx > max_name_idx)
          max_name_idx = idx;
      }
    }

    /* we add one entry to the `glyphNameIndex' array, */
    /* and append TTFAUTOHINT_GLYPH_LEN bytes to the `names' array */
    buf_new_len = post_table->len + 2 + TTFAUTOHINT_GLYPH_LEN;

    /* make the allocated buffer length a multiple of 4 */
    len = (buf_new_len + 3) & ~3;
    buf_new = (FT_Byte*)malloc(len);
    if (!buf_new)
      return FT_Err_Out_Of_Memory;

    /* pad end of buffer with zeros */
    buf_new[len - 1] = 0x00;
    buf_new[len - 2] = 0x00;
    buf_new[len - 3] = 0x00;

    /* copy the table and insert the new data */
    p = buf;
    p_new = buf_new;

    memcpy(p_new, p, 32); /* header */
    p += 32;
    p_new += 32;

    *p_new = HIGH(num_glyphs + 1);
    *(p_new + 1) = LOW(num_glyphs + 1);
    p += 2;
    p_new += 2;

    memcpy(p_new, p, num_glyphs * 2); /* glyphNameIndex */
    p += num_glyphs * 2;
    p_new += num_glyphs * 2;

    *p_new = HIGH(max_name_idx + 1 + 257); /* new entry */
    *(p_new + 1) = LOW(max_name_idx + 1 + 257);
    p_new += 2;

    memcpy(p_new, p, buf + buf_len - p); /* names */
    p_new += buf + buf_len - p;

    strncpy((char*)p_new, TTFAUTOHINT_GLYPH_FIRST_BYTE TTFAUTOHINT_GLYPH,
            TTFAUTOHINT_GLYPH_LEN); /* new entry */

    free(buf);
    post_table->buf = buf_new;
    post_table->len = buf_new_len;
  }
  else if (version == 0x28000)
  {
    /* XXX */
  }
  else
    goto Done; /* nothing to do for format 3.0 */

  post_table->checksum = TA_table_compute_checksum(post_table->buf,
                                                   post_table->len);
Done:
  post_table->processed = 1;

  return TA_Err_Ok;
}

/* end of tapost.c */
