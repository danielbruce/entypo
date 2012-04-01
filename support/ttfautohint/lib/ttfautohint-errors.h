/* ttfautohint-errors.h */

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


#ifndef __TTFAUTOHINT_ERRORS_H__
#define __TTFAUTOHINT_ERRORS_H__

/* We duplicate FreeType's error handling macros for simplicity; */
/* however, we don't use module-specific error codes. */

#undef TA_ERR_XCAT
#undef TA_ERR_CAT
#define TA_ERR_XCAT(x, y) x ## y
#define TA_ERR_CAT(x, y) TA_ERR_XCAT(x, y)

#undef TA_ERR_PREFIX
#define TA_ERR_PREFIX TA_Err_

#ifndef TA_ERRORDEF
#  define TA_ERRORDEF(e, v, s) e = v,
#  define TA_ERROR_START_LIST enum {
#  define TA_ERROR_END_LIST TA_ERR_CAT(TA_ERR_PREFIX, Max) };
#endif

#define TA_ERRORDEF_(e, v, s) \
          TA_ERRORDEF(TA_ERR_CAT(TA_ERR_PREFIX, e), v, s)
#define TA_NOERRORDEF_ TA_ERRORDEF_


/* The error codes. */

#ifdef TA_ERROR_START_LIST
  TA_ERROR_START_LIST
#endif

TA_NOERRORDEF_(Ok, 0x00, \
               "no error")

TA_ERRORDEF_(Invalid_FreeType_Version, 0x0E, \
             "invalid FreeType version (need 2.4.5 or higher)")
TA_ERRORDEF_(Missing_Legal_Permission, 0x0F, \
             "legal permission bit in `OS/2' font table is set")
TA_ERRORDEF_(Invalid_Stream_Write,     0x5F, \
             "invalid stream write")
TA_ERRORDEF_(Hinter_Overflow,          0xF0, \
             "hinter overflow")
TA_ERRORDEF_(Missing_Glyph,            0xF1, \
             "missing key character glyph")
TA_ERRORDEF_(Missing_Unicode_CMap,     0xF2, \
             "missing Unicode character map")
TA_ERRORDEF_(Canceled,                 0xF3, \
             "execution canceled")
TA_ERRORDEF_(Already_Processed,        0xF4, \
             "font already processed by ttfautohint")

#ifdef TA_ERROR_END_LIST
  TA_ERROR_END_LIST
#endif


#undef TA_ERROR_START_LIST
#undef TA_ERROR_END_LIST
#undef TA_ERRORDEF
#undef TA_ERRORDEF_
#undef TA_NOERRORDEF_

#endif /* __TTFAUTOHINT_ERRORS_H__ */

/* end of ttfautohint-errors.h */
