/* ttfautohint.h */

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


#ifndef __TTFAUTOHINT_H__
#define __TTFAUTOHINT_H__

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Some default values. */

#define TA_HINTING_RANGE_MIN 8
#define TA_HINTING_RANGE_MAX 1000
#define TA_HINTING_LIMIT 1000

/* Error type. */

typedef int TA_Error;


/*
 * A callback function to get progress information.  `curr_idx' gives the
 * currently processed glyph index; if it is negative, an error has
 * occurred.  `num_glyphs' holds the total number of glyphs in the font
 * (this value can't be larger than 65535).
 *
 * `curr_sfnt' gives the current subfont within a TrueType Collection (TTC),
 * and `num_sfnts' the total number of subfonts.  Currently, the ttfautohint
 * library only hints glyphs from the `glyf' table used in subfont 0.
 *
 * If the return value is non-zero, `TTF_autohint' aborts with
 * `TA_Err_Canceled'.  Use this for a `Cancel' button or similar features in
 * interactive use.
 *
 * `progress_data' is a void pointer to user supplied data.
 */

typedef int
(*TA_Progress_Func)(long curr_idx,
                    long num_glyphs,
                    long curr_sfnt,
                    long num_sfnts,
                    void* progress_data);


/*
 * A callback function to manipulate strings in the `name' table.
 * `platform_id', `encoding_id', `language_id', and `name_id' are the
 * identifiers of a name table entry pointed to by `str' with a length
 * pointed to by `str_len' (in bytes; the string has no trailing NULL byte).
 * Please refer to the OpenType specification for a detailed description of
 * the various parameters, in particular which encoding is used for a given
 * platform and encoding ID.
 *
 *   http://www.microsoft.com/typography/otspec/name.htm
 *
 * `str' is allocated with `malloc'; the application should reallocate the
 * data if necessary, ensuring that the string length doesn't exceed 0xFFFF.
 *
 * `info_data' is a void pointer to user supplied data.
 *
 * If an error occurs, return a non-zero value and don't modify `str' and
 * `str_len' (such errors are handled as non-fatal).
 */

typedef int
(*TA_Info_Func)(unsigned short platform_id,
                unsigned short encoding_id,
                unsigned short language_id,
                unsigned short name_id,
                unsigned short* str_len,
                unsigned char** str,
                void* info_data);


/*
 * Error values in addition to the FT_Err_XXX constants from FreeType.
 *
 * All error values specific to ttfautohint start with `TA_Err_'.
 */
#include <ttfautohint-errors.h>

/*
 * Read a TrueType font, remove existing bytecode (in the SFNT tables
 * `prep', `fpgm', `cvt ', and `glyf'), and write a new TrueType font with
 * new bytecode based on the autohinting of the FreeType library.
 *
 * It expects a format string `options' and a variable number of arguments,
 * depending on the fields in `options'.  The fields are comma separated;
 * whitespace within the format string is not significant, a trailing comma
 * is ignored.  Fields are parsed from left to right; if a field occurs
 * multiple times, the last field's argument wins.  The same is true for
 * fields which are mutually exclusive.  Depending on the field, zero or one
 * argument is expected.
 *
 * Note that fields marked as `not implemented yet' are subject to change.
 *
 *   in-file
 *     A pointer of type `FILE*' to the data stream of the input font,
 *     opened for binary reading.  Mutually exclusive with `in-buffer'.
 *
 *   in-buffer
 *     A pointer of type `const char*' to a buffer which contains the input
 *     font.  Needs `in-buffer-len'.  Mutually exclusive with `in-file'.
 *
 *   in-buffer-len
 *     A value of type `size_t', giving the length of the input buffer.
 *     Needs `in-buffer'.
 *
 *   out-file
 *      A pointer of type `FILE*' to the data stream of the output font,
 *      opened for binary writing.  Mutually exclusive with `out-buffer'.
 *
 *   out-buffer
 *     A pointer of type `char**' to a buffer which contains the output
 *     font.  Needs `out-buffer-len'.  Mutually exclusive with `out-file'.
 *     Deallocate the memory with `free'.
 *
 *   out-buffer-len
 *     A pointer of type `size_t*' to a value giving the length of the
 *     output buffer.  Needs `out-buffer'.
 *
 *   progress-callback
 *     A pointer of type `TA_Progress_Func', specifying a callback function
 *     for progress reports.  This function gets called after a single glyph
 *     has been processed.  If this field is not set or set to NULL, no
 *     progress callback function is used.
 *
 *   progress-callback-data
 *     A pointer of type `void*' to user data which is passed to the
 *     progress callback function.
 *
 *   error-string
 *     A pointer of type `unsigned char**' to a string (in UTF-8 encoding)
 *     which verbally describes the error code.  You must not change the
 *     returned value.
 *
 *   hinting-range-min
 *     An integer (which must be larger than or equal to 2) giving the
 *     lowest PPEM value used for autohinting.  If this field is not set, it
 *     defaults to TA_HINTING_RANGE_MIN.
 *
 *   hinting-range-max
 *     An integer (which must be larger than or equal to the value of
 *     `hinting-range-min') giving the highest PPEM value used for
 *     autohinting.  If this field is not set, it defaults to
 *     TA_HINTING_RANGE_MAX.
 *
 *   hinting-limit
 *     An integer (which must be larger than or equal to the value of
 *     `hinting-range-max') which gives the largest PPEM value at which
 *     hinting is applied.  For larger values, hinting is switched off.  If
 *     this field is not set, it defaults to TA_HINTING_LIMIT.  If it is set
 *     to 0, no hinting limit is added to the bytecode.
 *
 *   pre-hinting
 *     An integer (1 for `on' and 0 for `off', which is the default) to
 *     specify whether native TrueType hinting shall be applied to all
 *     glyphs before passing them to the (internal) autohinter.  The used
 *     resolution is the em-size in font units; for most fonts this is
 *     2048ppem.  Use this if the hints move or scale subglyphs
 *     independently of the output resolution.
 *
 *   info-callback
 *     A pointer of type `TA_Info_Func', specifying a callback function for
 *     manipulating the `name' table.  This function gets called for each
 *     name table entry.  If not set or set to NULL, the table data stays
 *     unmodified.
 *
 *   info-callback-data
 *     A pointer of type `void*' to user data which is passed to the info
 *     callback function.
 *
 *   increase-x-height
 *     An integer (1 for `on' and 0 for `off', which is the default).  For
 *     PPEM values in the range 5 < PPEM < 15, round up the font's x height
 *     much more often than normally if this flag is set.  Use this if holes
 *     in letters like `e' get filled, for example.
 *
 *   x-height-snapping-exceptions
 *     A pointer of type `const char*' to a null-terminated string which
 *     gives a list of comma separated PPEM values or value ranges at which
 *     no x-height snapping shall be applied.  A value range has the form
 *     `value1-value2', meaning `value1' <= PPEM <= `value2'.  Whitespace is
 *     not significant; a trailing comma is ignored.  If the supplied
 *     argument is NULL, no x-height snapping takes place at all.  By
 *     default, there are no snapping exceptions.  Not implemented yet.
 *
 *   ignore-permissions
 *     If the font has set bit 1 in the `fsType' field of the `OS/2' table,
 *     the ttfautohint library refuses to process the font since a
 *     permission to do that is required from the font's legal owner.  In
 *     case you have such a permission you might set the integer argument to
 *     value 1 to make ttfautohint handle the font.  The default value is 0.
 *
 *   fallback-script
 *     An integer which specifies the default script for glyphs not in the
 *     `latin' range.  If set to 1, the `latin' script is used (other
 *     scripts are not supported yet).  By default, no script is used (value
 *     0; this disables autohinting for such glyphs).
 *
 *   symbol
 *     Set this integer to 1 if you want to process a font which lacks the
 *     characters of a supported script, for example, a symbol font.
 *     ttfautohint then uses default values for the standard stem width and
 *     height instead of deriving these values from a script's key character
 *     (for the latin script, it is character `o').  The default value is 0.
 *
 * Remarks:
 *
 * o Obviously, it is necessary to have an input and an output data stream.
 *   All other options are optional.
 *
 * o `hinting-range-min' and `hinting-range-max' specify the range for which
 *   the autohinter generates optimized hinting code.  If a PPEM value is
 *   smaller than the value of `hinting-range-min', hinting still takes
 *   place but the configuration created for `hinting-range-min' is used.
 *   The analogous action is taken for `hinting-range-max', only limited by
 *   the value given with `hinting-limit'.  The font's `gasp' table is set
 *   up to always use grayscale rendering with grid-fitting for standard
 *   hinting, and symmetric grid-fitting and symmetric smoothing for
 *   horizontal subpixel hinting (ClearType).
 *
 * o ttfautohint can't process a font a second time (well, it can, if the
 *   font doesn't contain composite glyphs).  Just think of ttfautohint as
 *   being a compiler, a tool which also can't process its created output
 *   again.
 */

TA_Error
TTF_autohint(const char* options,
             ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __TTFAUTOHINT_H__ */

/* end of ttfautohint.h */
