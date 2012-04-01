/* tasfnt.c */

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
TA_sfnt_split_into_SFNT_tables(SFNT* sfnt,
                               FONT* font)
{
  FT_Error error;
  FT_ULong i;


  /* basic check whether font is a TTF or TTC */
  if (!FT_IS_SFNT(sfnt->face))
    return FT_Err_Invalid_Argument;

  error = FT_Sfnt_Table_Info(sfnt->face, 0, NULL, &sfnt->num_table_infos);
  if (error)
    return error;

  sfnt->table_infos = (SFNT_Table_Info*)malloc(sfnt->num_table_infos
                                               * sizeof (SFNT_Table_Info));
  if (!sfnt->table_infos)
    return FT_Err_Out_Of_Memory;

  /* collect SFNT tables and trace some of them */
  sfnt->glyf_idx = MISSING;
  sfnt->loca_idx = MISSING;
  sfnt->head_idx = MISSING;
  sfnt->hmtx_idx = MISSING;
  sfnt->maxp_idx = MISSING;
  sfnt->name_idx = MISSING;
  sfnt->post_idx = MISSING;
  sfnt->OS2_idx = MISSING;
  sfnt->GPOS_idx = MISSING;

  for (i = 0; i < sfnt->num_table_infos; i++)
  {
    SFNT_Table_Info* table_info = &sfnt->table_infos[i];
    FT_ULong tag;
    FT_ULong len;
    FT_Byte* buf;

    FT_ULong buf_len;
    FT_ULong j;


    *table_info = MISSING;

    error = FT_Sfnt_Table_Info(sfnt->face, i, &tag, &len);
    if (error)
    {
      if (error == FT_Err_Table_Missing)
        continue;
      else
        return error;
    }

    /* ignore zero-length tables */
    else if (!len)
      continue;

    /* ignore tables which we are going to create by ourselves, */
    /* or which would become invalid otherwise */
    else if (tag == TTAG_fpgm
             || tag == TTAG_prep
             || tag == TTAG_cvt
             || tag == TTAG_hdmx
             || tag == TTAG_VDMX
             || tag == TTAG_LTSH
             || tag == TTAG_gasp)
      continue;

    else if (tag == TTAG_DSIG)
    {
      font->have_DSIG = 1;
      continue;
    }

    /* make the allocated buffer length a multiple of 4 */
    buf_len = (len + 3) & ~3;
    buf = (FT_Byte*)malloc(buf_len);
    if (!buf)
      return FT_Err_Out_Of_Memory;

    /* pad end of buffer with zeros */
    buf[buf_len - 1] = 0x00;
    buf[buf_len - 2] = 0x00;
    buf[buf_len - 3] = 0x00;

    /* load table */
    error = FT_Load_Sfnt_Table(sfnt->face, tag, 0, buf, &len);
    if (error)
      goto Err;

    /* check whether we already have this table */
    for (j = 0; j < font->num_tables; j++)
    {
      SFNT_Table* table = &font->tables[j];


      if (table->tag == tag
          && table->len == len
          && !memcmp(table->buf, buf, len))
        break;
    }

    if (tag == TTAG_head)
      sfnt->head_idx = j;
    else if (tag == TTAG_glyf)
      sfnt->glyf_idx = j;
    else if (tag == TTAG_hmtx)
      sfnt->hmtx_idx = j;
    else if (tag == TTAG_loca)
      sfnt->loca_idx = j;
    else if (tag == TTAG_maxp)
    {
      sfnt->maxp_idx = j;

      sfnt->max_components = buf[MAXP_MAX_COMPONENTS_OFFSET] << 8;
      sfnt->max_components += buf[MAXP_MAX_COMPONENTS_OFFSET + 1];
    }
    else if (tag == TTAG_name)
      sfnt->name_idx = j;
    else if (tag == TTAG_post)
      sfnt->post_idx = j;
    else if (tag == TTAG_OS2)
      sfnt->OS2_idx = j;
    else if (tag == TTAG_GPOS)
      sfnt->GPOS_idx = j;

    if (j == font->num_tables)
    {
      /* add element to table array if it is missing or different; */
      /* in case of success, `buf' gets linked and is eventually */
      /* freed in `TA_font_unload' */
      error = TA_font_add_table(font, table_info, tag, len, buf);
      if (error)
        goto Err;
    }
    else
    {
      /* reuse existing SFNT table */
      free(buf);
      *table_info = j;
    }
    continue;

  Err:
    free(buf);
    return error;
  }

  /* no (non-empty) `glyf', `loca', `head', or `maxp' table; */
  /* this can't be a valid TTF with outlines */
  if (sfnt->glyf_idx == MISSING
      || sfnt->loca_idx == MISSING
      || sfnt->head_idx == MISSING
      || sfnt->maxp_idx == MISSING)
    return FT_Err_Invalid_Argument;

  return TA_Err_Ok;
}

/* end of tasfnt.c */
