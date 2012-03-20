/* tagpos.c */

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


/* the code below contains many redundancies; */
/* it has been written for clarity */

#define VALUE(val, p) val = *(p++) << 8; \
                      val += *(p++)
#define OFFSET(val, base, p) val = base; \
                             val += *(p++) << 8; \
                             val += *(p++)


/* this simple `Coverage_table' structure wastes memory... */

typedef struct Coverage_table_
{
  FT_UShort num_glyph_idxs;
  FT_UShort* glyph_idxs;
} Coverage_table;


static FT_Error
TA_read_coverage_table(FT_Byte* p,
                       Coverage_table* cov,
                       SFNT* sfnt,
                       FONT* font)
{
  SFNT_Table* GPOS_table = &font->tables[sfnt->GPOS_idx];

  FT_UShort* glyph_idxs;

  FT_UShort CoverageFormat;
  FT_UShort i;


  cov->num_glyph_idxs = 0;
  cov->glyph_idxs = NULL;

  VALUE(CoverageFormat, p);

  if (CoverageFormat == 1)
  {
    FT_UShort GlyphCount;


    VALUE(GlyphCount, p);

    /* rough sanity checks */
    if (GlyphCount * 2 > GPOS_table->len)
      return FT_Err_Invalid_Table;
    if (p - GPOS_table->buf > (ptrdiff_t)(GPOS_table->len - GlyphCount * 2))
      return FT_Err_Invalid_Table;

    glyph_idxs = (FT_UShort*)malloc(GlyphCount * sizeof (FT_UShort));
    if (!glyph_idxs)
      return FT_Err_Out_Of_Memory;

    /* loop over p */
    for (i = 0; i < GlyphCount; i++)
    {
      FT_UShort idx;


      VALUE(idx, p);
      glyph_idxs[i] = idx;
    }

    cov->num_glyph_idxs = GlyphCount;
  }

  else if (CoverageFormat == 2)
  {
    FT_UShort RangeCount;
    FT_UShort start;
    FT_UShort end;
    FT_UShort count;
    FT_Byte* p_start;


    VALUE(RangeCount, p);

    /* rough sanity checks */
    if (RangeCount * 6 > GPOS_table->len)
      return FT_Err_Invalid_Table;
    if (p - GPOS_table->buf > (ptrdiff_t)(GPOS_table->len - RangeCount * 6))
      return FT_Err_Invalid_Table;

    p_start = p;
    count = 0;

    /* loop over p */
    for (i = 0; i < RangeCount; i++)
    {
      /* collect number of glyphs */
      VALUE(start, p);
      VALUE(end, p);

      if (end < start)
        return FT_Err_Invalid_Table;

      p += 2; /* skip StartCoverageIndex */
      count += end - start + 1;
    }

    glyph_idxs = (FT_UShort*)malloc(count * sizeof (FT_UShort));
    if (!glyph_idxs)
      return FT_Err_Out_Of_Memory;

    p = p_start;
    count = 0;

    /* loop again over p */
    for (i = 0; i < RangeCount; i++)
    {
      FT_UShort j;


      VALUE(start, p);
      VALUE(end, p);
      p += 2; /* skip StartCoverageIndex */

      for (j = start; j <= end; j++)
        glyph_idxs[count++] = j;
    }

    cov->num_glyph_idxs = count;
  }
  else
    return FT_Err_Invalid_Table;

  cov->glyph_idxs = glyph_idxs;

  return TA_Err_Ok;
}


/* We add a subglyph for each composite glyph. */
/* Since subglyphs must contain at least one point, */
/* we have to adjust all AnchorPoints in GPOS AnchorTables accordingly. */
/* Using the `pointsums' array of the `GLYPH' structure */
/* it is straightforward to do that: */
/* Assuming that anchor point x is in the interval */
/* pointsums[n] <= x < pointsums[n + 1], */
/* the new point index is x + n. */

static FT_Error
TA_update_anchor(FT_Byte* p,
                 FT_UShort glyph_idx,
                 SFNT* sfnt,
                 FONT* font)
{
  SFNT_Table* GPOS_table = &font->tables[sfnt->GPOS_idx];
  SFNT_Table* glyf_table = &font->tables[sfnt->glyf_idx];
  glyf_Data* data = (glyf_Data*)glyf_table->data;
  GLYPH* glyph = &data->glyphs[glyph_idx];

  FT_UShort AnchorFormat;


  /* nothing to do for simple glyphs */
  if (!glyph->num_components)
    return TA_Err_Ok;

  VALUE(AnchorFormat, p);

  if (AnchorFormat == 2)
  {
    FT_UShort AnchorPoint;
    FT_UShort i;


    p += 4; /* skip XCoordinate and YCoordinate */
    VALUE(AnchorPoint, p);

    /* sanity check */
    if (p > GPOS_table->buf + GPOS_table->len)
      return FT_Err_Invalid_Table;

    /* search point offset */
    for (i = 0; i < glyph->num_pointsums; i++)
      if (AnchorPoint < glyph->pointsums[i])
        break;

    *(p - 2) = HIGH(AnchorPoint + i);
    *(p - 1) = LOW(AnchorPoint + i);
  }

  return TA_Err_Ok;
}


static FT_Error
TA_handle_cursive_lookup(FT_Byte* Lookup,
                         FT_Byte* p,
                         SFNT* sfnt,
                         FONT* font)
{
  FT_UShort SubTableCount;
  Coverage_table cov;
  FT_Error error;


  p += 2; /* skip LookupFlag */
  VALUE(SubTableCount, p);

  /* loop over p */
  for (; SubTableCount > 0; SubTableCount--)
  {
    FT_Byte* CursivePosFormat1;
    FT_Byte* Coverage;
    FT_UShort EntryExitCount;

    FT_Byte* q;
    FT_UShort i;


    OFFSET(CursivePosFormat1, Lookup, p);

    q = CursivePosFormat1;
    q += 2; /* skip PosFormat */
    OFFSET(Coverage, CursivePosFormat1, q);
    VALUE(EntryExitCount, q);

    error = TA_read_coverage_table(Coverage, &cov, sfnt, font);
    if (error)
      return error;

    /* sanity check */
    if (cov.num_glyph_idxs != EntryExitCount)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < EntryExitCount; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_Byte* EntryAnchor;
      FT_Byte* ExitAnchor;
      FT_Error error;


      OFFSET(EntryAnchor, CursivePosFormat1, q);
      error = TA_update_anchor(EntryAnchor, glyph_idx, sfnt, font);
      if (error)
        goto Fail;

      OFFSET(ExitAnchor, CursivePosFormat1, q);
      error = TA_update_anchor(ExitAnchor, glyph_idx, sfnt, font);
      if (error)
        goto Fail;
    }

    free(cov.glyph_idxs);
    cov.glyph_idxs = NULL;
  }

  return TA_Err_Ok;

Fail:
  free(cov.glyph_idxs);
  return error;
}


static FT_Error
TA_handle_markbase_lookup(FT_Byte* Lookup,
                          FT_Byte* p,
                          SFNT* sfnt,
                          FONT* font)
{
  FT_UShort SubTableCount;
  Coverage_table cov;
  FT_Error error;


  p += 2; /* skip LookupFlag */
  VALUE(SubTableCount, p);

  /* loop over p */
  for (; SubTableCount > 0; SubTableCount--)
  {
    FT_Byte* MarkBasePosFormat1;
    FT_Byte* MarkCoverage;
    FT_Byte* BaseCoverage;
    FT_UShort ClassCount;
    FT_UShort MarkCount;
    FT_Byte* MarkArray;
    FT_UShort BaseCount;
    FT_Byte* BaseArray;

    FT_Byte* q;
    FT_UShort i;


    OFFSET(MarkBasePosFormat1, Lookup, p);

    q = MarkBasePosFormat1;
    q += 2; /* skip PosFormat */
    OFFSET(MarkCoverage, MarkBasePosFormat1, q);
    OFFSET(BaseCoverage, MarkBasePosFormat1, q);
    VALUE(ClassCount, q);
    OFFSET(MarkArray, MarkBasePosFormat1, q);
    OFFSET(BaseArray, MarkBasePosFormat1, q);

    error = TA_read_coverage_table(MarkCoverage, &cov, sfnt, font);
    if (error)
      return error;

    q = MarkArray;
    VALUE(MarkCount, q);

    /* sanity check */
    if (cov.num_glyph_idxs != MarkCount)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < MarkCount; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_Byte* MarkAnchor;
      FT_Error error;


      q += 2; /* skip Class */
      OFFSET(MarkAnchor, MarkArray, q);
      error = TA_update_anchor(MarkAnchor, glyph_idx, sfnt, font);
      if (error)
        return error;
    }

    free(cov.glyph_idxs);

    error = TA_read_coverage_table(BaseCoverage, &cov, sfnt, font);
    if (error)
      return error;

    q = BaseArray;
    VALUE(BaseCount, q);

    /* sanity check */
    if (cov.num_glyph_idxs != BaseCount)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < BaseCount; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_UShort cc = ClassCount;


      for (; cc > 0; cc--)
      {
        FT_Byte* BaseAnchor;
        FT_Error error;


        OFFSET(BaseAnchor, BaseArray, q);
        error = TA_update_anchor(BaseAnchor, glyph_idx, sfnt, font);
        if (error)
          return error;
      }
    }

    free(cov.glyph_idxs);
    cov.glyph_idxs = NULL;
  }

  return TA_Err_Ok;

Fail:
  free(cov.glyph_idxs);
  return error;
}


static FT_Error
TA_handle_marklig_lookup(FT_Byte* Lookup,
                         FT_Byte* p,
                         SFNT* sfnt,
                         FONT* font)
{
  FT_UShort SubTableCount;
  Coverage_table cov;
  FT_Error error;


  p += 2; /* skip LookupFlag */
  VALUE(SubTableCount, p);

  /* loop over p */
  for (; SubTableCount > 0; SubTableCount--)
  {
    FT_Byte* MarkLigPosFormat1;
    FT_Byte* MarkCoverage;
    FT_Byte* LigatureCoverage;
    FT_UShort ClassCount;
    FT_UShort MarkCount;
    FT_Byte* MarkArray;
    FT_UShort LigatureCount;
    FT_Byte* LigatureArray;

    FT_Byte* q;
    FT_UShort i;


    OFFSET(MarkLigPosFormat1, Lookup, p);

    q = MarkLigPosFormat1;
    q += 2; /* skip PosFormat */
    OFFSET(MarkCoverage, MarkLigPosFormat1, q);
    OFFSET(LigatureCoverage, MarkLigPosFormat1, q);
    VALUE(ClassCount, q);
    OFFSET(MarkArray, MarkLigPosFormat1, q);
    OFFSET(LigatureArray, MarkLigPosFormat1, q);

    error = TA_read_coverage_table(MarkCoverage, &cov, sfnt, font);
    if (error)
      return error;

    q = MarkArray;
    VALUE(MarkCount, q);

    /* sanity check */
    if (cov.num_glyph_idxs != MarkCount)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < MarkCount; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_Byte* MarkAnchor;
      FT_Error error;


      q += 2; /* skip Class */
      OFFSET(MarkAnchor, MarkArray, q);
      error = TA_update_anchor(MarkAnchor, glyph_idx, sfnt, font);
      if (error)
        return error;
    }

    free(cov.glyph_idxs);

    error = TA_read_coverage_table(LigatureCoverage, &cov, sfnt, font);
    if (error)
      return error;

    q = LigatureArray;
    VALUE(LigatureCount, q);

    /* sanity check */
    if (cov.num_glyph_idxs != LigatureCount)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < LigatureCount; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_Byte* LigatureAttach;
      FT_UShort ComponentCount;
      FT_Byte* r;


      OFFSET(LigatureAttach, LigatureArray, q);

      r = LigatureAttach;
      VALUE(ComponentCount, r);

      /* loop over r */
      for (; ComponentCount > 0; ComponentCount--)
      {
        FT_UShort cc = ClassCount;


        for (; cc > 0; cc--)
        {
          FT_Byte* LigatureAnchor;
          FT_Error error;


          OFFSET(LigatureAnchor, LigatureAttach, r);
          error = TA_update_anchor(LigatureAnchor, glyph_idx, sfnt, font);
          if (error)
            return error;
        }
      }
    }

    free(cov.glyph_idxs);
    cov.glyph_idxs = NULL;
  }

  return TA_Err_Ok;

Fail:
  free(cov.glyph_idxs);
  return error;
}


static FT_Error
TA_handle_markmark_lookup(FT_Byte* Lookup,
                          FT_Byte* p,
                          SFNT* sfnt,
                          FONT* font)
{
  FT_UShort SubTableCount;
  Coverage_table cov;
  FT_Error error;


  p += 2; /* skip LookupFlag */
  VALUE(SubTableCount, p);

  /* loop over p */
  for (; SubTableCount > 0; SubTableCount--)
  {
    FT_Byte* MarkMarkPosFormat1;
    FT_Byte* Mark1Coverage;
    FT_Byte* Mark2Coverage;
    FT_UShort ClassCount;
    FT_UShort Mark1Count;
    FT_Byte* Mark1Array;
    FT_UShort Mark2Count;
    FT_Byte* Mark2Array;

    FT_Byte* q;
    FT_UShort i;


    OFFSET(MarkMarkPosFormat1, Lookup, p);

    q = MarkMarkPosFormat1;
    q += 2; /* skip PosFormat */
    OFFSET(Mark1Coverage, MarkMarkPosFormat1, q);
    OFFSET(Mark2Coverage, MarkMarkPosFormat1, q);
    VALUE(ClassCount, q);
    OFFSET(Mark1Array, MarkMarkPosFormat1, q);
    OFFSET(Mark2Array, MarkMarkPosFormat1, q);

    error = TA_read_coverage_table(Mark1Coverage, &cov, sfnt, font);
    if (error)
      return error;

    q = Mark1Array;
    VALUE(Mark1Count, q);

    /* sanity check */
    if (cov.num_glyph_idxs != Mark1Count)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < Mark1Count; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_Byte* Mark1Anchor;
      FT_Error error;


      q += 2; /* skip Class */
      OFFSET(Mark1Anchor, Mark1Array, q);
      error = TA_update_anchor(Mark1Anchor, glyph_idx, sfnt, font);
      if (error)
        return error;
    }

    free(cov.glyph_idxs);

    error = TA_read_coverage_table(Mark2Coverage, &cov, sfnt, font);
    if (error)
      return error;

    q = Mark2Array;
    VALUE(Mark2Count, q);

    /* sanity check */
    if (cov.num_glyph_idxs != Mark2Count)
    {
      error = FT_Err_Invalid_Table;
      goto Fail;
    }

    /* loop over q */
    for (i = 0; i < Mark2Count; i++)
    {
      FT_UShort glyph_idx = cov.glyph_idxs[i];
      FT_UShort cc = ClassCount;


      for (; cc > 0; cc--)
      {
        FT_Byte* Mark2Anchor;
        FT_Error error;


        OFFSET(Mark2Anchor, Mark2Array, q);
        error = TA_update_anchor(Mark2Anchor, glyph_idx, sfnt, font);
        if (error)
          return error;
      }
    }

    free(cov.glyph_idxs);
    cov.glyph_idxs = NULL;
  }

  return TA_Err_Ok;

Fail:
  free(cov.glyph_idxs);
  return error;
}


#define Cursive 3
#define MarkBase 4
#define MarkLig 5
#define MarkMark 6

FT_Error
TA_sfnt_update_GPOS_table(SFNT* sfnt,
                          FONT* font)
{
  SFNT_Table* GPOS_table = &font->tables[sfnt->GPOS_idx];
  FT_Byte* buf = GPOS_table->buf;

  FT_Byte* LookupList;
  FT_UShort LookupCount;
  FT_Byte* p;


  p = buf;

  if (GPOS_table->processed)
    return TA_Err_Ok;

  p += 8; /* skip Version, ScriptList, and FeatureList */
  OFFSET(LookupList, buf, p);

  p = LookupList;
  VALUE(LookupCount, p);

  /* loop over p */
  for (; LookupCount > 0; LookupCount--)
  {
    FT_Byte* Lookup;
    FT_UShort LookupType;
    FT_Byte* q;
    FT_Error error = TA_Err_Ok;


    OFFSET(Lookup, LookupList, p);

    q = Lookup;
    VALUE(LookupType, q);

    if (LookupType == Cursive)
      error = TA_handle_cursive_lookup(Lookup, q, sfnt, font);
    else if (LookupType == MarkBase)
      error = TA_handle_markbase_lookup(Lookup, q, sfnt, font);
    else if (LookupType == MarkLig)
      error = TA_handle_marklig_lookup(Lookup, q, sfnt, font);
    else if (LookupType == MarkMark)
      error = TA_handle_markmark_lookup(Lookup, q, sfnt, font);

    if (error)
      return error;
  }

  GPOS_table->checksum = TA_table_compute_checksum(GPOS_table->buf,
                                                   GPOS_table->len);
  GPOS_table->processed = 1;

  return TA_Err_Ok;
}

/* end of tagpos.c */
