/* tatables.c */

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


#include <stdlib.h>

#include "ta.h"


FT_Error
TA_sfnt_add_table_info(SFNT* sfnt)
{
  SFNT_Table_Info* table_infos_new;


  sfnt->num_table_infos++;
  table_infos_new =
    (SFNT_Table_Info*)realloc(sfnt->table_infos, sfnt->num_table_infos
                                                 * sizeof (SFNT_Table_Info));
  if (!table_infos_new)
  {
    sfnt->num_table_infos--;
    return FT_Err_Out_Of_Memory;
  }
  else
    sfnt->table_infos = table_infos_new;

  sfnt->table_infos[sfnt->num_table_infos - 1] = MISSING;

  return TA_Err_Ok;
}


FT_ULong
TA_table_compute_checksum(FT_Byte* buf,
                          FT_ULong len)
{
  FT_Byte* end_buf = buf + len;
  FT_ULong checksum = 0;


  /* we expect that the length of `buf' is a multiple of 4 */

  while (buf < end_buf)
  {
    checksum += *(buf++) << 24;
    checksum += *(buf++) << 16;
    checksum += *(buf++) << 8;
    checksum += *(buf++);
  }

  return checksum;
}


FT_Error
TA_font_add_table(FONT* font,
                  SFNT_Table_Info* table_info,
                  FT_ULong tag,
                  FT_ULong len,
                  FT_Byte* buf)
{
  SFNT_Table* tables_new;
  SFNT_Table* table_last;


  font->num_tables++;
  tables_new = (SFNT_Table*)realloc(font->tables,
                                    font->num_tables * sizeof (SFNT_Table));
  if (!tables_new)
  {
    font->num_tables--;
    return FT_Err_Out_Of_Memory;
  }
  else
    font->tables = tables_new;

  table_last = &font->tables[font->num_tables - 1];

  table_last->tag = tag;
  table_last->len = len;
  table_last->buf = buf;
  table_last->checksum = TA_table_compute_checksum(buf, len);
  table_last->offset = 0; /* set in `TA_font_compute_table_offsets' */
  table_last->data = NULL;
  table_last->processed = 0;

  /* link table and table info */
  *table_info = font->num_tables - 1;

  return TA_Err_Ok;
}


void
TA_sfnt_sort_table_info(SFNT* sfnt,
                        FONT* font)
{
  /* Looking into an arbitrary TTF (with a `DSIG' table), tags */
  /* starting with an uppercase letter are sorted before lowercase */
  /* letters.  In other words, the alphabetical ordering (as */
  /* mandated by signing a font) is a simple numeric comparison of */
  /* the 32bit tag values. */

  SFNT_Table* tables = font->tables;

  SFNT_Table_Info* table_infos = sfnt->table_infos;
  FT_ULong num_table_infos = sfnt->num_table_infos;

  FT_ULong i;
  FT_ULong j;


  /* bubble sort */
  for (i = 1; i < num_table_infos; i++)
  {
    for (j = i; j > 0; j--)
    {
      SFNT_Table_Info swap;
      FT_ULong tag1;
      FT_ULong tag2;


      tag1 = (table_infos[j] == MISSING)
               ? 0
               : tables[table_infos[j]].tag;
      tag2 = (table_infos[j - 1] == MISSING)
               ? 0
               : tables[table_infos[j - 1]].tag;

      if (tag1 > tag2)
        break;

      swap = table_infos[j];
      table_infos[j] = table_infos[j - 1];
      table_infos[j - 1] = swap;
    }
  }
}


void
TA_font_compute_table_offsets(FONT* font,
                              FT_ULong start)
{
  FT_ULong i;
  FT_ULong offset = start;


  for (i = 0; i < font->num_tables; i++)
  {
    SFNT_Table* table = &font->tables[i];


    table->offset = offset;

    /* table offsets must be multiples of 4; */
    /* this also fits the actual buffer lengths */
    offset += (table->len + 3) & ~3;
  }
}

/* end of tatables.c */
