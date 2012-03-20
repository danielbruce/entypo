/* tagloadr.h */

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


/* originally file `ftgloadr.h' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#ifndef __TAGLOADR_H__
#define __TAGLOADR_H__

#include <ft2build.h>
#include FT_FREETYPE_H


typedef struct TA_SubGlyphRec_
{
  FT_Int index;
  FT_UShort flags;

  FT_Int arg1;
  FT_Int arg2;

  FT_Matrix transform;
} TA_SubGlyphRec, *TA_SubGlyph;


typedef struct TA_GlyphLoadRec_
{
  FT_Outline outline; /* outline */

  FT_Vector* extra_points; /* extra points table */
  FT_Vector* extra_points2; /* second extra points table */

  FT_UInt num_subglyphs; /* number of subglyphs */
  TA_SubGlyph subglyphs; /* subglyphs */
} TA_GlyphLoadRec, *TA_GlyphLoad;


typedef struct TA_GlyphLoaderRec_
{
  FT_UInt max_points;
  FT_UInt max_contours;
  FT_UInt max_subglyphs;
  FT_Bool use_extra;

  TA_GlyphLoadRec base;
  TA_GlyphLoadRec current;
} TA_GlyphLoaderRec, *TA_GlyphLoader;


/* create new empty glyph loader */
FT_Error
TA_GlyphLoader_New(TA_GlyphLoader *aloader);

/* add an extra points table to a glyph loader */
FT_Error
TA_GlyphLoader_CreateExtra(TA_GlyphLoader loader);

/* destroy a glyph loader */
void
TA_GlyphLoader_Done(TA_GlyphLoader loader);

/* reset a glyph loader (frees everything int it) */
void
TA_GlyphLoader_Reset(TA_GlyphLoader loader);

/* rewind a glyph loader */
void
TA_GlyphLoader_Rewind(TA_GlyphLoader loader);

/* check that there is enough space to add */
/* `n_points' and `n_contours' to the glyph loader */
FT_Error
TA_GlyphLoader_CheckPoints(TA_GlyphLoader loader,
                           FT_UInt n_points,
                           FT_UInt n_contours);


#define TA_GLYPHLOADER_CHECK_P(_loader, _count) \
   ((_count) == 0 || ((_loader)->base.outline.n_points \
                      + (_loader)->current.outline.n_points \
                      + (unsigned long)(_count)) <= (_loader)->max_points)

#define TA_GLYPHLOADER_CHECK_C(_loader, _count) \
   ((_count) == 0 || ((_loader)->base.outline.n_contours \
                      + (_loader)->current.outline.n_contours \
                      + (unsigned long)(_count)) <= (_loader)->max_contours)

#define TA_GLYPHLOADER_CHECK_POINTS(_loader, _points, _contours) \
   ((TA_GLYPHLOADER_CHECK_P(_loader, _points) \
     && TA_GLYPHLOADER_CHECK_C(_loader, _contours)) \
    ? 0 \
    : TA_GlyphLoader_CheckPoints((_loader), (_points), (_contours)))


/* check that there is enough space to add */
/* `n_subs' sub-glyphs to a glyph loader */
FT_Error
TA_GlyphLoader_CheckSubGlyphs(TA_GlyphLoader loader,
                              FT_UInt n_subs);

/* prepare a glyph loader, i.e. empty the current glyph */
void
TA_GlyphLoader_Prepare(TA_GlyphLoader loader);

/* add the current glyph to the base glyph */
void
TA_GlyphLoader_Add(TA_GlyphLoader loader);

/* copy points from one glyph loader to another */
FT_Error
TA_GlyphLoader_CopyPoints(TA_GlyphLoader target,
                          TA_GlyphLoader source);

#endif /* __TAGLOADR_H__ */

/* end of tagloadr.h */
