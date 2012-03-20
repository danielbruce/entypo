/* tatables.h */

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


#ifndef __TATABLES_H__
#define __TATABLES_H__

#include "ta.h"


FT_Error
TA_sfnt_add_table_info(SFNT* sfnt);

FT_ULong
TA_table_compute_checksum(FT_Byte* buf,
                          FT_ULong len);

FT_Error
TA_font_add_table(FONT* font,
                  SFNT_Table_Info* table_info,
                  FT_ULong tag,
                  FT_ULong len,
                  FT_Byte* buf);

void
TA_sfnt_sort_table_info(SFNT* sfnt,
                        FONT* font);

void
TA_font_compute_table_offsets(FONT* font,
                              FT_ULong start);

#endif /* __TATABLES_H__ */

/* end of tatables.h */
