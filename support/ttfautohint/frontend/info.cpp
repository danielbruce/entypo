// info.cpp

// Copyright (C) 2012 by Werner Lemberg.
//
// This file is part of the ttfautohint library, and may only be used,
// modified, and distributed under the terms given in `COPYING'.  By
// continuing to use, modify, or distribute this file you indicate that you
// have read `COPYING' and understand and accept it fully.
//
// The file `COPYING' mentioned in the previous paragraph is distributed
// with the ttfautohint library.


#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "info.h"


// build string which gets appended to the `Version' field(s)

extern "C" {

void
build_version_string(Info_Data* idata)
{
  char* d;
  char* dw;

  d = (char*)idata->data;
  d += sprintf(d, "; ttfautohint (v%s)", VERSION);

  if (idata->hinting_range_min != TA_HINTING_RANGE_MIN)
    d += sprintf(d, " -l %d", idata->hinting_range_min);
  if (idata->hinting_range_max != TA_HINTING_RANGE_MAX)
    d += sprintf(d, " -r %d", idata->hinting_range_max);
  if (idata->hinting_limit != TA_HINTING_LIMIT)
    d += sprintf(d, " -G %d", idata->hinting_limit);

  if (idata->pre_hinting)
    d += sprintf(d, " -p");
  if (idata->increase_x_height)
    d += sprintf(d, " -x");
  if (idata->latin_fallback)
    d += sprintf(d, " -f");
  if (idata->symbol)
    d += sprintf(d, " -s");

  idata->data_len = d - (char*)idata->data;

  // prepare UTF16-BE version data
  d = (char*)idata->data;
  dw = (char*)idata->data_wide;
  for (unsigned short i = 0; i < idata->data_len; i++)
  {
    *(dw++) = '\0';
    *(dw++) = *(d++);
  }
  idata->data_wide_len = idata->data_len << 1;
}


int
info(unsigned short platform_id,
     unsigned short encoding_id,
     unsigned short /* language_id */,
     unsigned short name_id,
     unsigned short* len,
     unsigned char** str,
     void* user)
{
  Info_Data* idata = (Info_Data*)user;
  unsigned char* v;
  unsigned short v_len;

  // if it is a version string, append our data
  if (name_id != 5)
    return 0;

  if (platform_id == 1
      || (platform_id == 3 && !(encoding_id == 1
                                || encoding_id == 10)))
  {
    // one-byte or multi-byte encodings
    v = idata->data;
    v_len = idata->data_len;
  }
  else
  {
    // (two-byte) UTF-16BE for everything else
    v = idata->data_wide;
    v_len = idata->data_wide_len;
  }

  // do nothing if the string would become too long
  if (*len > 0xFFFF - v_len)
    return 0;

  unsigned short len_new = *len + v_len;
  unsigned char* str_new = (unsigned char*)realloc(*str, len_new);
  if (!str_new)
    return 1;

  *str = str_new;
  memcpy(*str + *len, v, v_len);
  *len = len_new;

  return 0;
}

} // extern "C"

// end of info.cpp
