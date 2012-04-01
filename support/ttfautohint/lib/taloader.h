/* taloader.h */

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


/* originally file `afloader.h' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#ifndef __TALOADER_H__
#define __TALOADER_H__

#include "tahints.h"
#include "taglobal.h"
#include "tagloadr.h"


typedef struct TA_LoaderRec_
{
  FT_Face face; /* current face */
  TA_FaceGlobals globals; /* current face globals */
  TA_GlyphLoader gloader; /* glyph loader */
  TA_GlyphHintsRec hints;
  TA_ScriptMetrics metrics;
  FT_Bool transformed;
  FT_Matrix trans_matrix;
  FT_Vector trans_delta;
  FT_Vector pp1;
  FT_Vector pp2;
  /* we don't handle vertical phantom points */
} TA_LoaderRec, *TA_Loader;


FT_Error
ta_loader_init(TA_Loader loader);


FT_Error
ta_loader_reset(TA_Loader loader,
                FT_Face face,
                FT_UInt fallback_script);

void
ta_loader_done(TA_Loader loader);


FT_Error
ta_loader_load_glyph(TA_Loader loader,
                     FT_Face face,
                     FT_UInt gindex,
                     FT_Int32 load_flags);


void
ta_loader_register_hints_recorder(TA_Loader loader,
                                  TA_Hints_Recorder hints_recorder,
                                  void* user);

#endif /* __TALOADER_H__ */

/* end of taloader.h */
