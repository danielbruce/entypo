/* tattc.c */

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
TA_font_build_TTC_header(FONT* font,
                         FT_Byte** header_buf,
                         FT_ULong* header_len)
{
  SFNT* sfnts = font->sfnts;
  FT_Long num_sfnts = font->num_sfnts;

  SFNT_Table* tables = font->tables;
  FT_ULong num_tables = font->num_tables;

  FT_ULong TTF_offset;
  FT_ULong DSIG_offset;

  FT_Byte* buf;
  FT_ULong len;

  FT_Long i;
  FT_Byte* p;


  len = 24 + 4 * num_sfnts;
  buf = (FT_Byte*)malloc(len);
  if (!buf)
    return FT_Err_Out_Of_Memory;

  p = buf;

  /* TTC ID string */
  *(p++) = 't';
  *(p++) = 't';
  *(p++) = 'c';
  *(p++) = 'f';

  /* TTC header version */
  *(p++) = 0x00;
  *(p++) = font->have_DSIG ? 0x02 : 0x01;
  *(p++) = 0x00;
  *(p++) = 0x00;

  /* number of subfonts */
  *(p++) = BYTE1(num_sfnts);
  *(p++) = BYTE2(num_sfnts);
  *(p++) = BYTE3(num_sfnts);
  *(p++) = BYTE4(num_sfnts);

  /* the first TTF subfont header immediately follows the TTC header */
  TTF_offset = len;

  /* loop over all subfonts */
  for (i = 0; i < num_sfnts; i++)
  {
    SFNT* sfnt = &sfnts[i];
    FT_ULong l;


    TA_sfnt_sort_table_info(sfnt, font);
    /* only get header length */
    (void)TA_sfnt_build_TTF_header(sfnt, font, NULL, &l, 0);

    *(p++) = BYTE1(TTF_offset);
    *(p++) = BYTE2(TTF_offset);
    *(p++) = BYTE3(TTF_offset);
    *(p++) = BYTE4(TTF_offset);

    TTF_offset += l;
  }

  /* the first SFNT table immediately follows the subfont TTF headers */
  TA_font_compute_table_offsets(font, TTF_offset);

  if (font->have_DSIG)
  {
    /* DSIG tag */
    *(p++) = 'D';
    *(p++) = 'S';
    *(p++) = 'I';
    *(p++) = 'G';

    /* DSIG length */
    *(p++) = 0x00;
    *(p++) = 0x00;
    *(p++) = 0x00;
    *(p++) = 0x08;

    /* DSIG offset; in a TTC this is always the last SFNT table */
    DSIG_offset = tables[num_tables - 1].offset;

    *(p++) = BYTE1(DSIG_offset);
    *(p++) = BYTE2(DSIG_offset);
    *(p++) = BYTE3(DSIG_offset);
    *(p++) = BYTE4(DSIG_offset);
  }

  *header_buf = buf;
  *header_len = len;

  return TA_Err_Ok;
}


FT_Error
TA_font_build_TTC(FONT* font)
{
  SFNT* sfnts = font->sfnts;
  FT_Long num_sfnts = font->num_sfnts;

  SFNT_Table* tables;
  FT_ULong num_tables;

  FT_Byte* DSIG_buf;
  SFNT_Table_Info dummy;

  FT_Byte* TTC_header_buf;
  FT_ULong TTC_header_len;

  FT_Byte** TTF_header_bufs = NULL;
  FT_ULong* TTF_header_lens = NULL;

  FT_ULong offset;
  FT_Long i;
  FT_ULong j;
  FT_Error error;


  /* replace an existing `DSIG' table with a dummy */

  if (font->have_DSIG)
  {
    error = TA_table_build_DSIG(&DSIG_buf);
    if (error)
      return error;

    /* in case of success, `DSIG_buf' gets linked */
    /* and is eventually freed in `TA_font_unload' */
    error = TA_font_add_table(font, &dummy, TTAG_DSIG, DSIG_LEN, DSIG_buf);
    if (error)
    {
      free(DSIG_buf);
      return error;
    }
  }

  /* this also computes the SFNT table offsets */
  error = TA_font_build_TTC_header(font,
                                   &TTC_header_buf, &TTC_header_len);
  if (error)
    return error;

  TTF_header_bufs = (FT_Byte**)calloc(1, num_sfnts * sizeof (FT_Byte*));
  if (!TTF_header_bufs)
    goto Err;

  TTF_header_lens = (FT_ULong*)malloc(num_sfnts * sizeof (FT_ULong));
  if (!TTF_header_lens)
    goto Err;

  for (i = 0; i < num_sfnts; i++)
  {
    error = TA_sfnt_build_TTF_header(&sfnts[i], font,
                                     &TTF_header_bufs[i],
                                     &TTF_header_lens[i], 1);
    if (error)
      goto Err;
  }

  /* build font */

  tables = font->tables;
  num_tables = font->num_tables;

  /* get font length from last SFNT table array element */
  font->out_len = tables[num_tables - 1].offset
                  + ((tables[num_tables - 1].len + 3) & ~3);
  font->out_buf = (FT_Byte*)malloc(font->out_len);
  if (!font->out_buf)
  {
    error = FT_Err_Out_Of_Memory;
    goto Err;
  }

  memcpy(font->out_buf, TTC_header_buf, TTC_header_len);

  offset = TTC_header_len;

  for (i = 0; i < num_sfnts; i++)
  {
    memcpy(font->out_buf + offset,
           TTF_header_bufs[i], TTF_header_lens[i]);

    offset += TTF_header_lens[i];
  }

  for (j = 0; j < num_tables; j++)
  {
    SFNT_Table* table = &tables[j];


    /* buffer length is a multiple of 4 */
    memcpy(font->out_buf + table->offset,
           table->buf, (table->len + 3) & ~3);
  }

  error = TA_Err_Ok;

Err:
  free(TTC_header_buf);
  if (TTF_header_bufs)
  {
    for (i = 0; i < font->num_sfnts; i++)
      free(TTF_header_bufs[i]);
    free(TTF_header_bufs);
  }
  free(TTF_header_lens);

  return error;
}


/* end of tattc.c */
