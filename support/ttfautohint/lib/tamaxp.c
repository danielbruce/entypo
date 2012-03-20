/* tamaxp.c */

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
TA_sfnt_update_maxp_table(SFNT* sfnt,
                          FONT* font)
{
  SFNT_Table* maxp_table = &font->tables[sfnt->maxp_idx];
  SFNT_Table* glyf_table = &font->tables[sfnt->glyf_idx];
  glyf_Data* data = (glyf_Data*)glyf_table->data;
  FT_Byte* buf = maxp_table->buf;


  if (maxp_table->processed)
    return TA_Err_Ok;

  if (maxp_table->len != MAXP_LEN)
    return FT_Err_Invalid_Table;

  if (sfnt->max_components)
  {
    buf[MAXP_NUM_GLYPHS] = HIGH(data->num_glyphs);
    buf[MAXP_NUM_GLYPHS + 1] = LOW(data->num_glyphs);
    buf[MAXP_MAX_COMPOSITE_POINTS] = HIGH(sfnt->max_composite_points);
    buf[MAXP_MAX_COMPOSITE_POINTS + 1] = LOW(sfnt->max_composite_points);
    buf[MAXP_MAX_COMPOSITE_CONTOURS] = HIGH(sfnt->max_composite_contours);
    buf[MAXP_MAX_COMPOSITE_CONTOURS + 1] = LOW(sfnt->max_composite_contours);
  }

  buf[MAXP_MAX_ZONES_OFFSET] = 0;
  buf[MAXP_MAX_ZONES_OFFSET + 1] = 2;
  buf[MAXP_MAX_TWILIGHT_POINTS_OFFSET] = HIGH(sfnt->max_twilight_points);
  buf[MAXP_MAX_TWILIGHT_POINTS_OFFSET + 1] = LOW(sfnt->max_twilight_points);
  buf[MAXP_MAX_STORAGE_OFFSET] = HIGH(sfnt->max_storage);
  buf[MAXP_MAX_STORAGE_OFFSET + 1] = LOW(sfnt->max_storage);
  buf[MAXP_MAX_FUNCTION_DEFS_OFFSET] = 0;
  buf[MAXP_MAX_FUNCTION_DEFS_OFFSET + 1] = NUM_FDEFS;
  buf[MAXP_MAX_INSTRUCTION_DEFS_OFFSET] = 0;
  buf[MAXP_MAX_INSTRUCTION_DEFS_OFFSET + 1] = 0;
  buf[MAXP_MAX_STACK_ELEMENTS_OFFSET] = HIGH(sfnt->max_stack_elements);
  buf[MAXP_MAX_STACK_ELEMENTS_OFFSET + 1] = LOW(sfnt->max_stack_elements);
  buf[MAXP_MAX_INSTRUCTIONS_OFFSET] = HIGH(sfnt->max_instructions);
  buf[MAXP_MAX_INSTRUCTIONS_OFFSET + 1] = LOW(sfnt->max_instructions);
  buf[MAXP_MAX_COMPONENTS_OFFSET] = HIGH(sfnt->max_components);
  buf[MAXP_MAX_COMPONENTS_OFFSET + 1] = LOW(sfnt->max_components);


  maxp_table->checksum = TA_table_compute_checksum(maxp_table->buf,
                                                   maxp_table->len);
  maxp_table->processed = 1;

  return TA_Err_Ok;
}

/* end of tamaxp.c */
