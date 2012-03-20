/* tacvt.c */

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
TA_sfnt_compute_global_hints(SFNT* sfnt,
                             FONT* font)
{
  FT_Error error;
  FT_Face face = sfnt->face;
  FT_UInt idx;


  error = ta_loader_init(font->loader);
  if (error)
    return error;

  error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  if (error)
    return TA_Err_Missing_Unicode_CMap;

  /* load glyph `o' to trigger all initializations; */
  /* XXX make this configurable for non-latin scripts */
  /* XXX make this configurable to use a different letter */
  idx = FT_Get_Char_Index(face, 'o');
  if (!idx)
    return TA_Err_Missing_Glyph;

  error = ta_loader_load_glyph(font->loader, face, idx,
                               font->fallback_script << 30);

  return error;
}


static FT_Error
TA_table_build_cvt(FT_Byte** cvt,
                   FT_ULong* cvt_len,
                   SFNT* sfnt,
                   FONT* font)
{
  TA_LatinAxis haxis;
  TA_LatinAxis vaxis;

  FT_UInt i;
  FT_UInt buf_len;
  FT_UInt len;
  FT_Byte* buf;
  FT_Byte* buf_p;

  FT_Error error;


  error = TA_sfnt_compute_global_hints(sfnt, font);
  if (error)
    return error;

  haxis = &((TA_LatinMetrics)font->loader->hints.metrics)->axis[0];
  vaxis = &((TA_LatinMetrics)font->loader->hints.metrics)->axis[1];

  buf_len = 2 * (cvtl_max_runtime /* runtime values */
                 + 2 /* vertical and horizontal standard width */
                 + haxis->width_count
                 + vaxis->width_count
                 + 2 * vaxis->blue_count);

  /* buffer length must be a multiple of four */
  len = (buf_len + 3) & ~3;
  buf = (FT_Byte*)malloc(len);
  if (!buf)
    return FT_Err_Out_Of_Memory;

  /* pad end of buffer with zeros */
  buf[len - 1] = 0x00;
  buf[len - 2] = 0x00;
  buf[len - 3] = 0x00;

  buf_p = buf;

  /* some CVT values are initialized (and modified) at runtime; */
  /* see the `cvtl_xxx' macros in tabytecode.h */
  for (i = 0; i < cvtl_max_runtime; i++)
  {
    *(buf_p++) = 0;
    *(buf_p++) = 0;
  }

  if (haxis->width_count > 0)
  {
    *(buf_p++) = HIGH(haxis->widths[0].org);
    *(buf_p++) = LOW(haxis->widths[0].org);
  }
  else
  {
    *(buf_p++) = 0;
    *(buf_p++) = 50;
  }
  if (vaxis->width_count > 0)
  {
    *(buf_p++) = HIGH(vaxis->widths[0].org);
    *(buf_p++) = LOW(vaxis->widths[0].org);
  }
  else
  {
    *(buf_p++) = 0;
    *(buf_p++) = 50;
  }

  for (i = 0; i < haxis->width_count; i++)
  {
    if (haxis->widths[i].org > 0xFFFF)
      goto Err;
    *(buf_p++) = HIGH(haxis->widths[i].org);
    *(buf_p++) = LOW(haxis->widths[i].org);
  }

  for (i = 0; i < vaxis->width_count; i++)
  {
    if (vaxis->widths[i].org > 0xFFFF)
      goto Err;
    *(buf_p++) = HIGH(vaxis->widths[i].org);
    *(buf_p++) = LOW(vaxis->widths[i].org);
  }

  for (i = 0; i < vaxis->blue_count; i++)
  {
    if (vaxis->blues[i].ref.org > 0xFFFF)
      goto Err;
    *(buf_p++) = HIGH(vaxis->blues[i].ref.org);
    *(buf_p++) = LOW(vaxis->blues[i].ref.org);
  }

  for (i = 0; i < vaxis->blue_count; i++)
  {
    if (vaxis->blues[i].shoot.org > 0xFFFF)
      goto Err;
    *(buf_p++) = HIGH(vaxis->blues[i].shoot.org);
    *(buf_p++) = LOW(vaxis->blues[i].shoot.org);
  }

#if 0
  TA_LOG(("--------------------------------------------------\n"));
  TA_LOG(("glyph %d:\n", idx));
  ta_glyph_hints_dump_edges(_ta_debug_hints);
  ta_glyph_hints_dump_segments(_ta_debug_hints);
  ta_glyph_hints_dump_points(_ta_debug_hints);
#endif

  *cvt = buf;
  *cvt_len = buf_len;

  return FT_Err_Ok;

Err:
  free(buf);
  return TA_Err_Hinter_Overflow;
}


FT_Error
TA_sfnt_build_cvt_table(SFNT* sfnt,
                        FONT* font)
{
  FT_Error error;

  FT_Byte* cvt_buf;
  FT_ULong cvt_len;


  error = TA_sfnt_add_table_info(sfnt);
  if (error)
    return error;

  error = TA_table_build_cvt(&cvt_buf, &cvt_len, sfnt, font);
  if (error)
    return error;

  /* in case of success, `cvt_buf' gets linked */
  /* and is eventually freed in `TA_font_unload' */
  error = TA_font_add_table(font,
                            &sfnt->table_infos[sfnt->num_table_infos - 1],
                            TTAG_cvt, cvt_len, cvt_buf);
  if (error)
  {
    free(cvt_buf);
    return error;
  }

  return FT_Err_Ok;
}

/* end of tacvt.c */
