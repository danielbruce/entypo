/* tafile.c */

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
TA_font_file_read(FONT* font,
                  FILE* in_file)
{
  fseek(in_file, 0, SEEK_END);
  font->in_len = ftell(in_file);
  fseek(in_file, 0, SEEK_SET);

  /* a valid TTF can never be that small */
  if (font->in_len < 100)
    return FT_Err_Invalid_Argument;

  font->in_buf = (FT_Byte*)malloc(font->in_len);
  if (!font->in_buf)
    return FT_Err_Out_Of_Memory;

  if (fread(font->in_buf, 1, font->in_len, in_file) != font->in_len)
    return FT_Err_Invalid_Stream_Read;

  return TA_Err_Ok;
}


FT_Error
TA_font_file_write(FONT* font,
                   FILE* out_file)
{
  if (fwrite(font->out_buf, 1, font->out_len, out_file) != font->out_len)
    return TA_Err_Invalid_Stream_Write;

  return TA_Err_Ok;
}

/* end of tafile.c */
