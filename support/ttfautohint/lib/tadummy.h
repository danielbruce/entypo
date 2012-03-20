/* tadummy.h */

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


/* originally file `afdummy.h' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#ifndef __TADUMMY_H__
#define __TADUMMY_H__

#include "tatypes.h"


/* a dummy script metrics class used when no hinting should */
/* be performed; this is the default for non-latin glyphs */

extern const TA_ScriptClassRec ta_dummy_script_class;

#endif /* __TADUMMY_H__ */

/* end of tadummy.h */
