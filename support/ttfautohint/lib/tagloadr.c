/* tagloadr.c */

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


/* originally file `ftgloadr.c' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#include <stdlib.h>
#include <string.h>

#include "tatypes.h"
#include "tagloadr.h"


  /*************************************************************************/
  /*                                                                       */
  /* The glyph loader is a simple object which is used to load a set of    */
  /* glyphs easily.  It is critical for the correct loading of composites. */
  /*                                                                       */
  /* Ideally, one can see it as a stack of abstract `glyph' objects.       */
  /*                                                                       */
  /*   loader.base     Is really the bottom of the stack.  It describes a  */
  /*                   single glyph image made of the juxtaposition of     */
  /*                   several glyphs (those `in the stack').              */
  /*                                                                       */
  /*   loader.current  Describes the top of the stack, on which a new      */
  /*                   glyph can be loaded.                                */
  /*                                                                       */
  /*   Rewind          Clears the stack.                                   */
  /*   Prepare         Set up `loader.current' for addition of a new glyph */
  /*                   image.                                              */
  /*   Add             Add the `current' glyph image to the `base' one,    */
  /*                   and prepare for another one.                        */
  /*                                                                       */
  /*************************************************************************/


/* create a new glyph loader */
FT_Error
TA_GlyphLoader_New(TA_GlyphLoader *aloader)
{
  TA_GlyphLoader loader;


  loader = (TA_GlyphLoader)calloc(1, sizeof (TA_GlyphLoaderRec));

  if (!loader)
    return FT_Err_Out_Of_Memory;

  *aloader = loader;
  return FT_Err_Ok;
}


/* rewind the glyph loader - reset counters to 0 */
void
TA_GlyphLoader_Rewind(TA_GlyphLoader loader)
{
  TA_GlyphLoad base = &loader->base;
  TA_GlyphLoad current = &loader->current;


  base->outline.n_points = 0;
  base->outline.n_contours = 0;
  base->num_subglyphs = 0;

  *current = *base;
}


/* reset the glyph loader, frees all allocated tables */
/* and starts from zero */
void
TA_GlyphLoader_Reset(TA_GlyphLoader loader)
{
  free(loader->base.outline.points);
  free(loader->base.outline.tags);
  free(loader->base.outline.contours);
  free(loader->base.extra_points);
  free(loader->base.subglyphs);

  loader->base.outline.points = NULL;
  loader->base.outline.tags = NULL;
  loader->base.outline.contours = NULL;
  loader->base.extra_points = NULL;
  loader->base.subglyphs = NULL;

  loader->base.extra_points2 = NULL;

  loader->max_points = 0;
  loader->max_contours = 0;
  loader->max_subglyphs = 0;

  TA_GlyphLoader_Rewind(loader);
}


/* delete a glyph loader */
void
TA_GlyphLoader_Done(TA_GlyphLoader loader)
{
  if (loader)
  {
    TA_GlyphLoader_Reset(loader);
    free(loader);
  }
}


/* re-adjust the `current' outline fields */
static void
TA_GlyphLoader_Adjust_Points(TA_GlyphLoader loader)
{
  FT_Outline* base = &loader->base.outline;
  FT_Outline* current = &loader->current.outline;


  current->points = base->points + base->n_points;
  current->tags = base->tags + base->n_points;
  current->contours = base->contours + base->n_contours;

  /* handle extra points table - if any */
  if (loader->use_extra)
  {
    loader->current.extra_points = loader->base.extra_points
                                   + base->n_points;

    loader->current.extra_points2 = loader->base.extra_points2
                                    + base->n_points;
  }
}


FT_Error
TA_GlyphLoader_CreateExtra(TA_GlyphLoader loader)
{
  loader->base.extra_points =
    (FT_Vector*)calloc(1, 2 * loader->max_points * sizeof (FT_Vector));
  if (!loader->base.extra_points)
    return FT_Err_Out_Of_Memory;

  loader->use_extra = 1;
  loader->base.extra_points2 = loader->base.extra_points
                               + loader->max_points;

  TA_GlyphLoader_Adjust_Points(loader);

  return FT_Err_Ok;
}


/* re-adjust the `current' subglyphs field */
static void
TA_GlyphLoader_Adjust_Subglyphs(TA_GlyphLoader loader)
{
  TA_GlyphLoad base = &loader->base;
  TA_GlyphLoad current = &loader->current;


  current->subglyphs = base->subglyphs + base->num_subglyphs;
}


/* ensure that we can add `n_points' and `n_contours' to our glyph -- */
/* this function reallocates its outline tables if necessary, */
/* but it DOESN'T change the number of points within the loader */
FT_Error
TA_GlyphLoader_CheckPoints(TA_GlyphLoader loader,
                           FT_UInt n_points,
                           FT_UInt n_contours)
{
  FT_Outline* base = &loader->base.outline;
  FT_Outline* current = &loader->current.outline;
  FT_Bool adjust = 0;

  FT_UInt new_max, old_max;


  /* check points & tags */
  new_max = base->n_points + current->n_points + n_points;
  old_max = loader->max_points;

  if (new_max > old_max)
  {
    FT_Vector* points_new;
    char* tags_new;


    new_max = TA_PAD_CEIL(new_max, 8);

    if (new_max > FT_OUTLINE_POINTS_MAX)
      return FT_Err_Array_Too_Large;

    points_new = (FT_Vector*)realloc(base->points,
                                     new_max * sizeof (FT_Vector));
    if (!points_new)
      return FT_Err_Out_Of_Memory;
    base->points = points_new;

    tags_new = (char*)realloc(base->tags,
                              new_max * sizeof (char));
    if (!tags_new)
      return FT_Err_Out_Of_Memory;
    base->tags = tags_new;

    if (loader->use_extra)
    {
      FT_Vector* extra_points_new;


      extra_points_new =
        (FT_Vector*)realloc(loader->base.extra_points,
                            new_max * 2 * sizeof (FT_Vector));
      if (!extra_points_new)
        return FT_Err_Out_Of_Memory;
      loader->base.extra_points = extra_points_new;

      memmove(loader->base.extra_points + new_max,
              loader->base.extra_points + old_max,
              old_max * sizeof (FT_Vector));

      loader->base.extra_points2 = loader->base.extra_points + new_max;
    }

    adjust = 1;
    loader->max_points = new_max;
  }

  /* check contours */
  old_max = loader->max_contours;
  new_max = base->n_contours + current->n_contours + n_contours;
  if (new_max > old_max)
  {
    short* contours_new;


    new_max = TA_PAD_CEIL(new_max, 4);

    if (new_max > FT_OUTLINE_CONTOURS_MAX)
      return FT_Err_Array_Too_Large;

    contours_new = (short*)realloc(base->contours,
                                   new_max * sizeof (short));
    if (!contours_new)
      return FT_Err_Out_Of_Memory;
    base->contours = contours_new;

    adjust = 1;
    loader->max_contours = new_max;
  }

  if (adjust)
    TA_GlyphLoader_Adjust_Points(loader);

  return FT_Err_Ok;
}


/* ensure that we can add `n_subglyphs' to our glyph -- */
/* this function reallocates its subglyphs table if necessary, */
/* but it DOES NOT change the number of subglyphs within the loader */
FT_Error
TA_GlyphLoader_CheckSubGlyphs(TA_GlyphLoader loader,
                              FT_UInt n_subs)
{
  FT_UInt new_max, old_max;

  TA_GlyphLoad base = &loader->base;
  TA_GlyphLoad current = &loader->current;


  new_max = base->num_subglyphs + current->num_subglyphs + n_subs;
  old_max = loader->max_subglyphs;
  if (new_max > old_max)
  {
    TA_SubGlyph subglyphs_new;


    new_max = TA_PAD_CEIL(new_max, 2);
    subglyphs_new = (TA_SubGlyph)realloc(base->subglyphs,
                                         new_max * sizeof (TA_SubGlyphRec));
    if (!subglyphs_new)
      return FT_Err_Out_Of_Memory;
    base->subglyphs = subglyphs_new;

    loader->max_subglyphs = new_max;

    TA_GlyphLoader_Adjust_Subglyphs(loader);
  }

  return FT_Err_Ok;
}


/* prepare loader for the addition of a new glyph on top of the base one */
void
TA_GlyphLoader_Prepare(TA_GlyphLoader loader)
{
  TA_GlyphLoad current = &loader->current;


  current->outline.n_points = 0;
  current->outline.n_contours = 0;
  current->num_subglyphs = 0;

  TA_GlyphLoader_Adjust_Points (loader);
  TA_GlyphLoader_Adjust_Subglyphs(loader);
}


/* add current glyph to the base image -- and prepare for another */
void
TA_GlyphLoader_Add(TA_GlyphLoader loader)
{
  TA_GlyphLoad base;
  TA_GlyphLoad current;

  FT_UInt n_curr_contours;
  FT_UInt n_base_points;
  FT_UInt n;


  if (!loader)
    return;

  base = &loader->base;
  current = &loader->current;

  n_curr_contours = current->outline.n_contours;
  n_base_points = base->outline.n_points;

  base->outline.n_points =
    (short)(base->outline.n_points + current->outline.n_points);
  base->outline.n_contours =
    (short)(base->outline.n_contours + current->outline.n_contours);

  base->num_subglyphs += current->num_subglyphs;

  /* adjust contours count in newest outline */
  for (n = 0; n < n_curr_contours; n++)
    current->outline.contours[n] =
      (short)(current->outline.contours[n] + n_base_points);

  /* prepare for another new glyph image */
  TA_GlyphLoader_Prepare(loader);
}


FT_Error
TA_GlyphLoader_CopyPoints(TA_GlyphLoader target,
                          TA_GlyphLoader source)
{
  FT_Error error;
  FT_UInt num_points = source->base.outline.n_points;
  FT_UInt num_contours = source->base.outline.n_contours;


  error = TA_GlyphLoader_CheckPoints(target, num_points, num_contours);
  if (!error)
  {
    FT_Outline* out = &target->base.outline;
    FT_Outline* in = &source->base.outline;


    memcpy(out->points, in->points, num_points * sizeof (FT_Vector));
    memcpy(out->contours, in->contours, num_contours * sizeof (short));
    memcpy(out->tags, in->tags, num_points * sizeof (char));

    /* do we need to copy the extra points? */
    if (target->use_extra && source->use_extra)
    {
      memcpy(target->base.extra_points, source->base.extra_points,
             num_points * sizeof (FT_Vector));
      memcpy(target->base.extra_points2, source->base.extra_points2,
             num_points * sizeof (FT_Vector));
    }

    out->n_points = (short)num_points;
    out->n_contours = (short)num_contours;

    TA_GlyphLoader_Adjust_Points(target);
  }

  return error;
}

/* end of tagloadr.c */
