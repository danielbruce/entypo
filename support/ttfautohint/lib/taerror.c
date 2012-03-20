/* taerror.c */

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


/* error message strings; */
/* we concatenate FreeType and ttfautohint messages into one structure */

typedef const struct TA_error_
{
  int err_code;
  const char* err_msg;
} TA_error;

TA_error TA_Errors[] =

#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s) { e, s },
#define FT_ERROR_START_LIST {
#define FT_ERROR_END_LIST /* empty */
#include FT_ERRORS_H

#undef __TTFAUTOHINT_ERRORS_H__
#define TA_ERRORDEF(e, v, s) { e, s },
#define TA_ERROR_START_LIST /* empty */
#define TA_ERROR_END_LIST { 0, NULL } };
#include <ttfautohint-errors.h>


const char*
TA_get_error_message(FT_Error error)
{
  TA_error* e = TA_Errors;


  while (e->err_code || e->err_msg)
  {
    if (e->err_code == error)
      return e->err_msg;
    e++;
  }

  return NULL;
}

/* end of taerror.c */
