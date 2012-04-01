// info.h

// Copyright (C) 2012 by Werner Lemberg.
//
// This file is part of the ttfautohint library, and may only be used,
// modified, and distributed under the terms given in `COPYING'.  By
// continuing to use, modify, or distribute this file you indicate that you
// have read `COPYING' and understand and accept it fully.
//
// The file `COPYING' mentioned in the previous paragraph is distributed
// with the ttfautohint library.


#ifndef __INFO_H__
#define __INFO_H__

#include <ttfautohint.h>

extern "C" {

typedef struct Info_Data_
{
  unsigned char* data;
  unsigned char* data_wide;
  unsigned short data_len;
  unsigned short data_wide_len;

  int hinting_range_min;
  int hinting_range_max;
  int hinting_limit;

  bool pre_hinting;
  bool increase_x_height;
  int latin_fallback;
  bool symbol;
} Info_Data;


void
build_version_string(Info_Data* idata);

TA_Error
info(unsigned short platform_id,
     unsigned short encoding_id,
     unsigned short language_id,
     unsigned short name_id,
     unsigned short* str_len,
     unsigned char** str,
     void* user);

} // extern "C"

#endif // __INFO_H__

// end of info.h
