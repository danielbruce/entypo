/* taglobal.c */

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


/* originally file `afglobal.c' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#include <stdlib.h>

#include "taglobal.h"

#include "tadummy.h"
#include "talatin.h"

#if 0
#include "tacjk.h"
#include "taindic.h"

#ifdef FT_OPTION_AUTOFIT2
#include "talatin2.h"
#endif
#endif /* 0 */

/* populate this list when you add new scripts */
static TA_ScriptClass const ta_script_classes[] =
{
  &ta_dummy_script_class,
#ifdef FT_OPTION_AUTOFIT2
  &ta_latin2_script_class,
#endif
  &ta_latin_script_class,
#if 0
  &ta_cjk_script_class,
  &ta_indic_script_class,
#endif
  NULL /* do not remove */
};


/* a bit mask indicating an uncovered glyph */
#define TA_SCRIPT_LIST_NONE 0x7F
/* if this flag is set, we have an ASCII digit */
#define TA_DIGIT 0x80


/* note that glyph_scripts[] is used to map each glyph into */
/* an index into the `ta_script_classes' array. */
typedef struct TA_FaceGlobalsRec_
{
  FT_Face face;
  FT_Long glyph_count; /* same as face->num_glyphs */
  FT_Byte* glyph_scripts;

  TA_ScriptMetrics metrics[TA_SCRIPT_MAX];
} TA_FaceGlobalsRec;


/* Compute the script index of each glyph within a given face. */

static FT_Error
ta_face_globals_compute_script_coverage(TA_FaceGlobals globals,
                                        FT_UInt fallback_script)
{
  FT_Error error = FT_Err_Ok;
  FT_Face face = globals->face;
  FT_CharMap old_charmap = face->charmap;
  FT_Byte* gscripts = globals->glyph_scripts;
  FT_UInt ss;
  FT_UInt i;


  /* the value TA_SCRIPT_LIST_NONE means `uncovered glyph' */
  memset(globals->glyph_scripts, TA_SCRIPT_LIST_NONE, globals->glyph_count);

  error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  if (error)
  {
    /* ignore this error; we simply use the fallback script */
    /* XXX: Shouldn't we rather disable hinting? */
    error = FT_Err_Ok;
    goto Exit;
  }

  /* scan each script in a Unicode charmap */
  for (ss = 0; ta_script_classes[ss]; ss++)
  {
    TA_ScriptClass clazz = ta_script_classes[ss];
    TA_Script_UniRange range;


    if (clazz->script_uni_ranges == NULL)
      continue;

    /* scan all Unicode points in the range and */
    /* set the corresponding glyph script index */
    for (range = clazz->script_uni_ranges; range->first != 0; range++)
    {
      FT_ULong charcode = range->first;
      FT_UInt gindex;


      gindex = FT_Get_Char_Index(face, charcode);

      if (gindex != 0
          && gindex < (FT_ULong)globals->glyph_count
          && gscripts[gindex] == TA_SCRIPT_LIST_NONE)
        gscripts[gindex] = (FT_Byte)ss;

      for (;;)
      {
        charcode = FT_Get_Next_Char(face, charcode, &gindex);

        if (gindex == 0 || charcode > range->last)
          break;

        if (gindex < (FT_ULong)globals->glyph_count
            && gscripts[gindex] == TA_SCRIPT_LIST_NONE)
          gscripts[gindex] = (FT_Byte)ss;
      }
    }
  }

  /* mark ASCII digits */
  for (i = 0x30; i <= 0x39; i++)
  {
    FT_UInt gindex = FT_Get_Char_Index(face, i);


    if (gindex != 0
        && gindex < (FT_ULong)globals->glyph_count)
      gscripts[gindex] |= TA_DIGIT;
  }

Exit:
  /* by default, all uncovered glyphs are set to the fallback script */
  /* XXX: Shouldn't we disable hinting or do something similar? */
  {
    FT_Long nn;


    for (nn = 0; nn < globals->glyph_count; nn++)
    {
      if ((gscripts[nn] & ~TA_DIGIT) == TA_SCRIPT_LIST_NONE)
      {
        gscripts[nn] &= ~TA_SCRIPT_LIST_NONE;
        gscripts[nn] |= fallback_script;
      }
    }
  }

  FT_Set_Charmap(face, old_charmap);
  return error;
}


FT_Error
ta_face_globals_new(FT_Face face,
                    TA_FaceGlobals *aglobals,
                    FT_UInt fallback_script)
{
  FT_Error error;
  TA_FaceGlobals globals;


  globals = (TA_FaceGlobals)calloc(1, sizeof (TA_FaceGlobalsRec) +
                                      face->num_glyphs * sizeof (FT_Byte));
  if (!globals)
  {
    error = FT_Err_Out_Of_Memory;
    goto Err;
  }

  globals->face = face;
  globals->glyph_count = face->num_glyphs;
  globals->glyph_scripts = (FT_Byte*)(globals + 1);

  error = ta_face_globals_compute_script_coverage(globals, fallback_script);
  if (error)
  {
    ta_face_globals_free(globals);
    globals = NULL;
  }

Err:
  *aglobals = globals;
  return error;
}


void
ta_face_globals_free(TA_FaceGlobals globals)
{
  if (globals)
  {
    FT_UInt nn;


    for (nn = 0; nn < TA_SCRIPT_MAX; nn++)
    {
      if (globals->metrics[nn])
      {
        TA_ScriptClass clazz = ta_script_classes[nn];


#if 0
        FT_ASSERT(globals->metrics[nn]->clazz == clazz);
#endif

        if (clazz->script_metrics_done)
          clazz->script_metrics_done(globals->metrics[nn]);

        free(globals->metrics[nn]);
        globals->metrics[nn] = NULL;
      }
    }

    globals->glyph_count = 0;
    globals->glyph_scripts = NULL; /* no need to free this one! */
    globals->face = NULL;

    free(globals);
    globals = NULL;
  }
}


FT_Error
ta_face_globals_get_metrics(TA_FaceGlobals globals,
                            FT_UInt gindex,
                            FT_UInt options,
                            TA_ScriptMetrics *ametrics)
{
  TA_ScriptMetrics metrics = NULL;
  FT_UInt gidx;
  TA_ScriptClass clazz;
  FT_UInt script = options & 15;
  const FT_Offset script_max = sizeof (ta_script_classes)
                               / sizeof (ta_script_classes[0]);
  FT_Error error = FT_Err_Ok;


  if (gindex >= (FT_ULong)globals->glyph_count)
  {
    error = FT_Err_Invalid_Argument;
    goto Exit;
  }

  gidx = script;
  if (gidx == 0
      || gidx + 1 >= script_max)
    gidx = globals->glyph_scripts[gindex] & TA_SCRIPT_LIST_NONE;

  clazz = ta_script_classes[gidx];
  if (script == 0)
    script = clazz->script;

  metrics = globals->metrics[clazz->script];
  if (metrics == NULL)
  {
    /* create the global metrics object if necessary */
    metrics = (TA_ScriptMetrics)calloc(1, clazz->script_metrics_size);
    if (!metrics)
    {
      error = FT_Err_Out_Of_Memory;
      goto Exit;
    }

    metrics->clazz = clazz;

    if (clazz->script_metrics_init)
    {
      error = clazz->script_metrics_init(metrics, globals->face);
      if (error)
      {
        if (clazz->script_metrics_done)
          clazz->script_metrics_done(metrics);

        free(metrics);
        metrics = NULL;
        goto Exit;
      }
    }

    globals->metrics[clazz->script] = metrics;
  }

Exit:
  *ametrics = metrics;

  return error;
}


FT_Bool
ta_face_globals_is_digit(TA_FaceGlobals globals,
                         FT_UInt gindex)
{
  if (gindex < (FT_ULong)globals->glyph_count)
    return (FT_Bool)(globals->glyph_scripts[gindex] & TA_DIGIT);

  return (FT_Bool)0;
}

/* end of taglobal.c */
