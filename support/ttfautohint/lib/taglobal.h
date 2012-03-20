/* taglobal.h */

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


/* originally file `afglobal.h' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#ifndef __TAGLOBAL_H__
#define __TAGLOBAL_H__

#include "tatypes.h"


/* this models the global hints data for a given face, */
/* decomposed into script-specific items */
typedef struct TA_FaceGlobalsRec_* TA_FaceGlobals;

FT_Error
ta_face_globals_new(FT_Face face,
                    TA_FaceGlobals *aglobals,
                    FT_UInt fallback_script);

FT_Error
ta_face_globals_get_metrics(TA_FaceGlobals globals,
                            FT_UInt gindex,
                            FT_UInt options,
                            TA_ScriptMetrics *ametrics);

void
ta_face_globals_free(TA_FaceGlobals globals);

FT_Bool
ta_face_globals_is_digit(TA_FaceGlobals globals,
                         FT_UInt gindex);

#endif /* __TAGLOBAL_H__ */

/* end of taglobal.h */
