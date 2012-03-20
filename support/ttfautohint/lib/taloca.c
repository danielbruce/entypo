/* taloca.c */

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
TA_sfnt_build_loca_table(SFNT* sfnt,
                         FONT* font)
{
  SFNT_Table* loca_table = &font->tables[sfnt->loca_idx];
  SFNT_Table* glyf_table = &font->tables[sfnt->glyf_idx];
  SFNT_Table* head_table = &font->tables[sfnt->head_idx];

  glyf_Data* data = (glyf_Data*)glyf_table->data;
  GLYPH* glyph;

  FT_ULong offset;
  FT_Byte loca_format;
  FT_Byte* buf_new;
  FT_Byte* p;
  FT_UShort i;


  if (loca_table->processed)
    return TA_Err_Ok;

  /* get largest offset */
  offset = 0;
  glyph = data->glyphs;

  for (i = 0; i < data->num_glyphs; i++, glyph++)
  {
    /* glyph records should have offsets which are multiples of 4 */
    offset = (offset + 3) & ~3;
    offset += glyph->len1 + glyph->len2 + glyph->ins_len;
    /* add two bytes for the instructionLength field */
    if (glyph->len2 || glyph->ins_len)
      offset += 2;
  }

  if (offset > 0xFFFF * 2)
    loca_format = 1;
  else
    loca_format = 0;

  /* fill table */
  if (loca_format)
  {
    loca_table->len = (data->num_glyphs + 1) * 4;
    buf_new = (FT_Byte*)realloc(loca_table->buf, loca_table->len);
    if (!buf_new)
      return FT_Err_Out_Of_Memory;
    else
      loca_table->buf = buf_new;

    p = loca_table->buf;
    offset = 0;
    glyph = data->glyphs;

    for (i = 0; i < data->num_glyphs; i++, glyph++)
    {
      offset = (offset + 3) & ~3;

      *(p++) = BYTE1(offset);
      *(p++) = BYTE2(offset);
      *(p++) = BYTE3(offset);
      *(p++) = BYTE4(offset);

      offset += glyph->len1 + glyph->len2 + glyph->ins_len;
      if (glyph->len2 || glyph->ins_len)
        offset += 2;
    }

    /* last element holds the size of the `glyf' table */
    *(p++) = BYTE1(offset);
    *(p++) = BYTE2(offset);
    *(p++) = BYTE3(offset);
    *(p++) = BYTE4(offset);
  }
  else
  {
    loca_table->len = (data->num_glyphs + 1) * 2;
    buf_new = (FT_Byte*)realloc(loca_table->buf,
                                (loca_table->len + 3) & ~3);
    if (!buf_new)
      return FT_Err_Out_Of_Memory;
    else
      loca_table->buf = buf_new;

    p = loca_table->buf;
    offset = 0;
    glyph = data->glyphs;

    for (i = 0; i < data->num_glyphs; i++, glyph++)
    {
      offset = (offset + 1) & ~1;

      *(p++) = HIGH(offset);
      *(p++) = LOW(offset);

      offset += (glyph->len1 + glyph->len2 + glyph->ins_len + 1) >> 1;
      if (glyph->len2 || glyph->ins_len)
        offset += 1;
    }

    /* last element holds the size of the `glyf' table */
    *(p++) = HIGH(offset);
    *(p++) = LOW(offset);

    /* pad `loca' table to make its length a multiple of 4 */
    if (loca_table->len % 4 == 2)
    {
      *(p++) = 0;
      *(p++) = 0;
    }
  }

  loca_table->checksum = TA_table_compute_checksum(loca_table->buf,
                                                   loca_table->len);
  loca_table->processed = 1;

  head_table->buf[LOCA_FORMAT_OFFSET] = loca_format;

  return TA_Err_Ok;
}

/* end of taloca.c */
