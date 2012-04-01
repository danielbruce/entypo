/* tahints.c */

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


/* originally file `afhints.c' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#include <string.h>
#include <stdlib.h>
#include "tahints.h"


/* get new segment for given axis */

FT_Error
ta_axis_hints_new_segment(TA_AxisHints axis,
                          TA_Segment* asegment)
{
  FT_Error error = FT_Err_Ok;
  TA_Segment segment = NULL;


  if (axis->num_segments >= axis->max_segments)
  {
    TA_Segment segments_new;

    FT_Int old_max = axis->max_segments;
    FT_Int new_max = old_max;
    FT_Int big_max = (FT_Int)(FT_INT_MAX / sizeof (*segment));


    if (old_max >= big_max)
    {
      error = FT_Err_Out_Of_Memory;
      goto Exit;
    }

    new_max += (new_max >> 2) + 4;
    if (new_max < old_max
        || new_max > big_max)
      new_max = big_max;

    segments_new = (TA_Segment)realloc(axis->segments,
                                       new_max * sizeof (TA_SegmentRec));
    if (!segments_new)
      return FT_Err_Out_Of_Memory;

    axis->segments = segments_new;
    axis->max_segments = new_max;
  }

  segment = axis->segments + axis->num_segments++;

Exit:
  *asegment = segment;
  return error;
}


/* get new edge for given axis, direction, and position */

FT_Error
ta_axis_hints_new_edge(TA_AxisHints axis,
                       FT_Int fpos,
                       TA_Direction dir,
                       TA_Edge* anedge)
{
  FT_Error error = FT_Err_Ok;
  TA_Edge edge = NULL;
  TA_Edge edges;


  if (axis->num_edges >= axis->max_edges)
  {
    TA_Edge edges_new;

    FT_Int old_max = axis->max_edges;
    FT_Int new_max = old_max;
    FT_Int big_max = (FT_Int)(FT_INT_MAX / sizeof (*edge));


    if (old_max >= big_max)
    {
      error = FT_Err_Out_Of_Memory;
      goto Exit;
    }

    new_max += (new_max >> 2) + 4;
    if (new_max < old_max
        || new_max > big_max)
      new_max = big_max;

    edges_new = (TA_Edge)realloc(axis->edges,
                                 new_max * sizeof (TA_EdgeRec));
    if (!edges_new)
      return FT_Err_Out_Of_Memory;

    axis->edges = edges_new;
    axis->max_edges = new_max;
  }

  edges = axis->edges;
  edge = edges + axis->num_edges;

  while (edge > edges)
  {
    if (edge[-1].fpos < fpos)
      break;

    /* we want the edge with same position and minor direction */
    /* to appear before those in the major one in the list */
    if (edge[-1].fpos == fpos
        && dir == axis->major_dir)
      break;

    edge[0] = edge[-1];
    edge--;
  }

  axis->num_edges++;

  memset(edge, 0, sizeof (TA_EdgeRec));
  edge->fpos = (FT_Short)fpos;
  edge->dir = (FT_Char)dir;

Exit:
  *anedge = edge;
  return error;
}


#ifdef TA_DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


void
_ta_message(const char *format,
            ...)
{
  va_list ap;


  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}


static const char*
ta_dir_str(TA_Direction dir)
{
  const char* result;


  switch (dir)
  {
  case TA_DIR_UP:
    result = "up";
    break;
  case TA_DIR_DOWN:
    result = "down";
    break;
  case TA_DIR_LEFT:
    result = "left";
    break;
  case TA_DIR_RIGHT:
    result = "right";
    break;
  default:
    result = "none";
  }

  return result;
}


#define TA_INDEX_NUM(ptr, base) \
          ((ptr) ? ((ptr) - (base)) \
                 : -1)


void
ta_glyph_hints_dump_points(TA_GlyphHints hints)
{
  TA_Point points = hints->points;
  TA_Point limit = points + hints->num_points;
  TA_Point point;


  fprintf(stderr, "Table of points:\n");
  fprintf(stderr, "  [ index |  xorg |  yorg | xscale | yscale"
                  " |  xfit |  yfit |  flags ]\n");

  for (point = points; point < limit; point++)
  {
    fprintf(stderr, "  [ %5d | %5d | %5d | %6.2f | %6.2f"
                    " | %5.2f | %5.2f | %c%c%c%c%c%c ]\n",
            point - points,
            point->fx,
            point->fy,
            point->ox / 64.0,
            point->oy / 64.0,
            point->x / 64.0,
            point->y / 64.0,
            (point->flags & TA_FLAG_WEAK_INTERPOLATION) ? 'w' : ' ',
            (point->flags & TA_FLAG_INFLECTION) ? 'i' : ' ',
            (point->flags & TA_FLAG_EXTREMA_X) ? '<' : ' ',
            (point->flags & TA_FLAG_EXTREMA_Y) ? 'v' : ' ',
            (point->flags & TA_FLAG_ROUND_X) ? '(' : ' ',
            (point->flags & TA_FLAG_ROUND_Y) ? 'u' : ' ');
  }
  fprintf(stderr, "\n");
}


static const char*
ta_edge_flags_to_string(FT_Byte flags)
{
  static char temp[32];
  int pos = 0;


  if (flags & TA_EDGE_ROUND)
  {
    memcpy(temp + pos, "round", 5);
    pos += 5;
  }
  if (flags & TA_EDGE_SERIF)
  {
    if (pos > 0)
      temp[pos++] = ' ';
    memcpy(temp + pos, "serif", 5);
    pos += 5;
  }
  if (pos == 0)
    return "normal";

  temp[pos] = '\0';

  return temp;
}


/* dump the array of linked segments */

void
ta_glyph_hints_dump_segments(TA_GlyphHints hints)
{
  FT_Int dimension;


  for (dimension = TA_DEBUG_STARTDIM;
       dimension >= TA_DEBUG_ENDDIM;
       dimension--)
  {
    TA_AxisHints axis = &hints->axis[dimension];
    TA_Point points = hints->points;
    TA_Edge edges = axis->edges;
    TA_Segment segments = axis->segments;
    TA_Segment limit = segments + axis->num_segments;
    TA_Segment seg;


    fprintf(stderr, "Table of %s segments:\n",
            dimension == TA_DIMENSION_HORZ ? "vertical"
                                           : "horizontal");
    fprintf(stderr, "  [ index |  pos  |  dir  | from |  to  | link | serif | edge |"
                    " height | extra |    flags    ]\n");

    for (seg = segments; seg < limit; seg++)
    {
      fprintf(stderr, "  [ %5d | %5.2g | %5s | %4d | %4d | %4d | %5d | %4d |"
                      " %6d | %5d | %11s ]\n",
              seg - segments,
              dimension == TA_DIMENSION_HORZ ? (int)seg->first->ox / 64.0
                                             : (int)seg->first->oy / 64.0,
              ta_dir_str((TA_Direction)seg->dir),
              TA_INDEX_NUM(seg->first, points),
              TA_INDEX_NUM(seg->last, points),
              TA_INDEX_NUM(seg->link, segments),
              TA_INDEX_NUM(seg->serif, segments),
              TA_INDEX_NUM(seg->edge, edges),
              seg->height,
              seg->height - (seg->max_coord - seg->min_coord),
              ta_edge_flags_to_string(seg->flags));
    }
    fprintf(stderr, "\n");
  }
}


/* dump the array of linked edges */

void
ta_glyph_hints_dump_edges(TA_GlyphHints hints)
{
  FT_Int dimension;


  for (dimension = TA_DEBUG_STARTDIM;
       dimension >= TA_DEBUG_ENDDIM;
       dimension--)
  {
    TA_AxisHints axis = &hints->axis[dimension];
    TA_Edge edges = axis->edges;
    TA_Edge limit = edges + axis->num_edges;
    TA_Edge edge;


    /* note that TA_DIMENSION_HORZ corresponds to _vertical_ edges */
    /* since they have a constant X coordinate */
    fprintf(stderr, "Table of %s edges:\n",
            dimension == TA_DIMENSION_HORZ ? "vertical"
                                           : "horizontal");
    fprintf(stderr, "  [ index |  pos  |  dir  | link |"
                    " serif | blue | opos  |  pos  |    flags    ]\n");

    for (edge = edges; edge < limit; edge++)
    {
      fprintf(stderr, "  [ %5d | %5.2g | %5s | %4d |"
                      " %5d |   %c  | %5.2f | %5.2f | %11s ]\n",
              edge - edges,
              (int)edge->opos / 64.0,
              ta_dir_str((TA_Direction)edge->dir),
              TA_INDEX_NUM(edge->link, edges),
              TA_INDEX_NUM(edge->serif, edges),
              edge->blue_edge ? 'y' : 'n',
              edge->opos / 64.0,
              edge->pos / 64.0,
              ta_edge_flags_to_string(edge->flags));
    }
    fprintf(stderr, "\n");
  }
}


void
ta_glyph_hints_dump_edge_links(TA_GlyphHints hints)
{
  FT_Int dimension;


  for (dimension = TA_DEBUG_STARTDIM;
       dimension >= TA_DEBUG_ENDDIM;
       dimension--)
  {
    TA_AxisHints axis = &hints->axis[dimension];
    TA_Segment segments = axis->segments;
    TA_Edge edges = axis->edges;
    TA_Edge limit = edges + axis->num_edges;

    TA_Edge edge;
    TA_Segment seg;


    fprintf(stderr, "%s edges consist of the following segments:\n",
            dimension == TA_DIMENSION_HORZ ? "Vertical"
                                           : "Horizontal");
    for (edge = edges; edge < limit; edge++)
    {
      fprintf(stderr, "  %2d:", edge - edges);

      seg = edge->first;
      do
      {
        fprintf(stderr, " %d", seg - segments);
        seg = seg->edge_next;
      } while (seg != edge->first);

      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
  }
}

#endif /* TA_DEBUG */


/* compute the direction value of a given vector */

TA_Direction
ta_direction_compute(FT_Pos dx,
                     FT_Pos dy)
{
  FT_Pos ll, ss;  /* long and short arm lengths */
  TA_Direction dir; /* candidate direction */


  if (dy >= dx)
  {
    if (dy >= -dx)
    {
      dir = TA_DIR_UP;
      ll = dy;
      ss = dx;
    }
    else
    {
      dir = TA_DIR_LEFT;
      ll = -dx;
      ss = dy;
    }
    }
  else /* dy < dx */
  {
    if (dy >= -dx)
    {
      dir = TA_DIR_RIGHT;
      ll = dx;
      ss = dy;
    }
    else
    {
      dir = TA_DIR_DOWN;
      ll = dy;
      ss = dx;
    }
  }

  /* return no direction if arm lengths differ too much */
  /* (value 14 is heuristic)                            */
  ss *= 14;
  if (TA_ABS(ll) <= TA_ABS(ss))
    dir = TA_DIR_NONE;

  return dir;
}


void
ta_glyph_hints_init(TA_GlyphHints hints)
{
  memset(hints, 0, sizeof (TA_GlyphHintsRec));
}


void
ta_glyph_hints_done(TA_GlyphHints hints)
{
  int dim;


  if (!hints)
    return;

  /* we don't need to free the segment and edge buffers */
  /* since they are really within the hints->points array */
  for (dim = 0; dim < TA_DIMENSION_MAX; dim++)
  {
    TA_AxisHints axis = &hints->axis[dim];


    axis->num_segments = 0;
    axis->max_segments = 0;
    free(axis->segments);
    axis->segments = NULL;

    axis->num_edges = 0;
    axis->max_edges = 0;
    free(axis->edges);
    axis->edges = NULL;
  }

  free(hints->contours);
  hints->contours = NULL;
  hints->max_contours = 0;
  hints->num_contours = 0;

  free(hints->points);
  hints->points = NULL;
  hints->num_points = 0;
  hints->max_points = 0;
}


/* reset metrics */

void
ta_glyph_hints_rescale(TA_GlyphHints hints,
                       TA_ScriptMetrics metrics)
{
  hints->metrics = metrics;
  hints->scaler_flags = metrics->scaler.flags;
}


/* from FreeType's ftcalc.c */

static FT_Int
ta_corner_is_flat(FT_Pos in_x,
                  FT_Pos in_y,
                  FT_Pos out_x,
                  FT_Pos out_y)
{
  FT_Pos ax = in_x;
  FT_Pos ay = in_y;

  FT_Pos d_in, d_out, d_corner;


  if (ax < 0)
    ax = -ax;
  if (ay < 0)
    ay = -ay;
  d_in = ax + ay;

  ax = out_x;
  if (ax < 0)
    ax = -ax;
  ay = out_y;
  if (ay < 0)
    ay = -ay;
  d_out = ax + ay;

  ax = out_x + in_x;
  if (ax < 0)
    ax = -ax;
  ay = out_y + in_y;
  if (ay < 0)
    ay = -ay;
  d_corner = ax + ay;

  return (d_in + d_out - d_corner) < (d_corner >> 4);
}


/* recompute all TA_Point in TA_GlyphHints */
/* from the definitions in a source outline */

FT_Error
ta_glyph_hints_reload(TA_GlyphHints hints,
                      FT_Outline* outline)
{
  FT_Error error = FT_Err_Ok;
  TA_Point points;
  FT_UInt old_max, new_max;

  FT_Fixed x_scale = hints->x_scale;
  FT_Fixed y_scale = hints->y_scale;
  FT_Pos x_delta = hints->x_delta;
  FT_Pos y_delta = hints->y_delta;


  hints->num_points = 0;
  hints->num_contours = 0;

  hints->axis[0].num_segments = 0;
  hints->axis[0].num_edges = 0;
  hints->axis[1].num_segments = 0;
  hints->axis[1].num_edges = 0;

  /* first of all, reallocate the contours array if necessary */
  new_max = (FT_UInt)outline->n_contours;
  old_max = hints->max_contours;
  if (new_max > old_max)
  {
    TA_Point* contours_new;


    new_max = (new_max + 3) & ~3; /* round up to a multiple of 4 */

    contours_new = (TA_Point*)realloc(hints->contours,
                                      new_max * sizeof (TA_Point));
    if (!contours_new)
      return FT_Err_Out_Of_Memory;

    hints->contours = contours_new;
    hints->max_contours = new_max;
  }

  /* reallocate the points arrays if necessary -- we reserve */
  /* two additional point positions, used to hint metrics appropriately */
  new_max = (FT_UInt)(outline->n_points + 2);
  old_max = hints->max_points;
  if (new_max > old_max)
  {
    TA_Point  points_new;


    new_max = (new_max + 2 + 7) & ~7; /* round up to a multiple of 8 */

    points_new = (TA_Point)realloc(hints->points,
                                   new_max * sizeof (TA_PointRec));
    if (!points_new)
      return FT_Err_Out_Of_Memory;

    hints->points = points_new;
    hints->max_points = new_max;
  }

  hints->num_points = outline->n_points;
  hints->num_contours = outline->n_contours;

  /* we can't rely on the value of `FT_Outline.flags' to know the fill */
  /* direction used for a glyph, given that some fonts are broken */
  /* (e.g. the Arphic ones); we thus recompute it each time we need to */

  hints->axis[TA_DIMENSION_HORZ].major_dir = TA_DIR_UP;
  hints->axis[TA_DIMENSION_VERT].major_dir = TA_DIR_LEFT;

  if (FT_Outline_Get_Orientation(outline) == FT_ORIENTATION_POSTSCRIPT)
  {
    hints->axis[TA_DIMENSION_HORZ].major_dir = TA_DIR_DOWN;
    hints->axis[TA_DIMENSION_VERT].major_dir = TA_DIR_RIGHT;
  }

  hints->x_scale = x_scale;
  hints->y_scale = y_scale;
  hints->x_delta = x_delta;
  hints->y_delta = y_delta;

  hints->xmin_delta = 0;
  hints->xmax_delta = 0;

  points = hints->points;
  if (hints->num_points == 0)
    goto Exit;

  {
    TA_Point point;
    TA_Point point_limit = points + hints->num_points;


    /* compute coordinates & Bezier flags, next and prev */
    {
      FT_Vector* vec = outline->points;
      char* tag = outline->tags;

      TA_Point end = points + outline->contours[0];
      TA_Point prev = end;

      FT_Int contour_index = 0;


      for (point = points; point < point_limit; point++, vec++, tag++)
      {
        point->fx = (FT_Short)vec->x;
        point->fy = (FT_Short)vec->y;
        point->ox = point->x = FT_MulFix(vec->x, x_scale) + x_delta;
        point->oy = point->y = FT_MulFix(vec->y, y_scale) + y_delta;

        switch (FT_CURVE_TAG(*tag))
        {
        case FT_CURVE_TAG_CONIC:
          point->flags = TA_FLAG_CONIC;
          break;
        case FT_CURVE_TAG_CUBIC:
          point->flags = TA_FLAG_CUBIC;
          break;
        default:
          point->flags = TA_FLAG_NONE;
        }

        point->prev = prev;
        prev->next = point;
        prev = point;

        if (point == end)
        {
          if (++contour_index < outline->n_contours)
          {
            end = points + outline->contours[contour_index];
            prev = end;
          }
        }
      }
    }

    /* set up the contours array */
    {
      TA_Point* contour = hints->contours;
      TA_Point* contour_limit = contour + hints->num_contours;

      short* end = outline->contours;
      short idx = 0;


      for (; contour < contour_limit; contour++, end++)
      {
        contour[0] = points + idx;
        idx = (short)(end[0] + 1);
      }
    }

    /* compute directions of in & out vectors */
    {
      TA_Point first = points;
      TA_Point prev = NULL;

      FT_Pos in_x = 0;
      FT_Pos in_y = 0;

      TA_Direction in_dir = TA_DIR_NONE;


      for (point = points; point < point_limit; point++)
      {
        TA_Point next;
        FT_Pos out_x, out_y;


        if (point == first)
        {
          prev = first->prev;
          in_x = first->fx - prev->fx;
          in_y = first->fy - prev->fy;
          in_dir = ta_direction_compute(in_x, in_y);
          first = prev + 1;
        }

        point->in_dir = (FT_Char)in_dir;

        next = point->next;
        out_x = next->fx - point->fx;
        out_y = next->fy - point->fy;

        in_dir = ta_direction_compute(out_x, out_y);
        point->out_dir = (FT_Char)in_dir;

        /* check for weak points */

        if (point->flags & TA_FLAG_CONTROL)
        {
        Is_Weak_Point:
          point->flags |= TA_FLAG_WEAK_INTERPOLATION;
        }
        else if (point->out_dir == point->in_dir)
        {
          if (point->out_dir != TA_DIR_NONE)
            goto Is_Weak_Point;

          if (ta_corner_is_flat(in_x, in_y, out_x, out_y))
            goto Is_Weak_Point;
        }
        else if (point->in_dir == -point->out_dir)
          goto Is_Weak_Point;

        in_x = out_x;
        in_y = out_y;
        prev = point;
      }
    }
  }

Exit:
  return error;
}


/* store the hinted outline in an FT_Outline structure */

void
ta_glyph_hints_save(TA_GlyphHints hints,
                    FT_Outline* outline)
{
  TA_Point point = hints->points;
  TA_Point limit = point + hints->num_points;

  FT_Vector* vec = outline->points;
  char* tag = outline->tags;


  for (; point < limit; point++, vec++, tag++)
  {
    vec->x = point->x;
    vec->y = point->y;

    if (point->flags & TA_FLAG_CONIC)
      tag[0] = FT_CURVE_TAG_CONIC;
    else if (point->flags & TA_FLAG_CUBIC)
      tag[0] = FT_CURVE_TAG_CUBIC;
    else
      tag[0] = FT_CURVE_TAG_ON;
  }
}


/****************************************************************
 *
 *                     EDGE POINT GRID-FITTING
 *
 ****************************************************************/


/* align all points of an edge to the same coordinate value, */
/* either horizontally or vertically */

void
ta_glyph_hints_align_edge_points(TA_GlyphHints hints,
                                 TA_Dimension dim)
{
  TA_AxisHints axis = &hints->axis[dim];
  TA_Segment segments = axis->segments;
  TA_Segment segment_limit = segments + axis->num_segments;
  TA_Segment seg;


  if (dim == TA_DIMENSION_HORZ)
  {
    for (seg = segments; seg < segment_limit; seg++)
    {
      TA_Edge edge = seg->edge;
      TA_Point point, first, last;


      if (edge == NULL)
        continue;

      first = seg->first;
      last = seg->last;
      point = first;
      for (;;)
      {
        point->x = edge->pos;
        point->flags |= TA_FLAG_TOUCH_X;

        if (point == last)
          break;

        point = point->next;
      }
    }
  }
  else
  {
    for (seg = segments; seg < segment_limit; seg++)
    {
      TA_Edge edge = seg->edge;
      TA_Point point, first, last;


      if (edge == NULL)
        continue;

      first = seg->first;
      last = seg->last;
      point = first;
      for (;;)
      {
        point->y = edge->pos;
        point->flags |= TA_FLAG_TOUCH_Y;

        if (point == last)
          break;

        point = point->next;
      }
    }
  }
}


/****************************************************************
 *
 *                    STRONG POINT INTERPOLATION
 *
 ****************************************************************/


/* hint the strong points -- */
/* this is equivalent to the TrueType `IP' hinting instruction */

void
ta_glyph_hints_align_strong_points(TA_GlyphHints hints,
                                   TA_Dimension dim)
{
  TA_Point points = hints->points;
  TA_Point point_limit = points + hints->num_points;

  TA_AxisHints axis = &hints->axis[dim];

  TA_Edge edges = axis->edges;
  TA_Edge edge_limit = edges + axis->num_edges;

  FT_UShort touch_flag;


  if (dim == TA_DIMENSION_HORZ)
    touch_flag = TA_FLAG_TOUCH_X;
  else
    touch_flag = TA_FLAG_TOUCH_Y;

  if (edges < edge_limit)
  {
    TA_Point point;
    TA_Edge edge;


    for (point = points; point < point_limit; point++)
    {
      FT_Pos u, ou, fu;  /* point position */
      FT_Pos delta;


      if (point->flags & touch_flag)
        continue;

      /* if this point is candidate to weak interpolation, we */
      /* interpolate it after all strong points have been processed */

      if ((point->flags & TA_FLAG_WEAK_INTERPOLATION)
          && !(point->flags & TA_FLAG_INFLECTION))
        continue;

      if (dim == TA_DIMENSION_VERT)
      {
        u = point->fy;
        ou = point->oy;
      }
      else
      {
        u = point->fx;
        ou = point->ox;
      }

      fu = u;

      /* is the point before the first edge? */
      edge = edges;
      delta = edge->fpos - u;
      if (delta >= 0)
      {
        u = edge->pos - (edge->opos - ou);

        if (hints->recorder)
          hints->recorder(ta_ip_before, hints, dim,
                          point, NULL, NULL, NULL, NULL);

        goto Store_Point;
      }

      /* is the point after the last edge? */
      edge = edge_limit - 1;
      delta = u - edge->fpos;
      if (delta >= 0)
      {
        u = edge->pos + (ou - edge->opos);

        if (hints->recorder)
          hints->recorder(ta_ip_after, hints, dim,
                          point, NULL, NULL, NULL, NULL);

        goto Store_Point;
      }

      {
        FT_PtrDist min, max, mid;
        FT_Pos fpos;


        /* find enclosing edges */
        min = 0;
        max = edge_limit - edges;

        /* for a small number of edges, a linear search is better */
        if (max <= 8)
        {
          FT_PtrDist nn;


          for (nn = 0; nn < max; nn++)
            if (edges[nn].fpos >= u)
              break;

          if (edges[nn].fpos == u)
          {
            u = edges[nn].pos;

            if (hints->recorder)
              hints->recorder(ta_ip_on, hints, dim,
                              point, &edges[nn], NULL, NULL, NULL);

            goto Store_Point;
          }
          min = nn;
        }
        else
          while (min < max)
          {
            mid = (max + min) >> 1;
            edge = edges + mid;
            fpos = edge->fpos;

            if (u < fpos)
              max = mid;
            else if (u > fpos)
              min = mid + 1;
            else
            {
              /* we are on the edge */
              u = edge->pos;

              if (hints->recorder)
                hints->recorder(ta_ip_on, hints, dim,
                                point, edge, NULL, NULL, NULL);

              goto Store_Point;
            }
          }

        /* point is not on an edge */
        {
          TA_Edge before = edges + min - 1;
          TA_Edge after = edges + min + 0;


          /* assert(before && after && before != after) */
          if (before->scale == 0)
            before->scale = FT_DivFix(after->pos - before->pos,
                                      after->fpos - before->fpos);

          u = before->pos + FT_MulFix(fu - before->fpos,
                                      before->scale);

          if (hints->recorder)
            hints->recorder(ta_ip_between, hints, dim,
                            point, before, after, NULL, NULL);
        }
      }

    Store_Point:
      /* save the point position */
      if (dim == TA_DIMENSION_HORZ)
        point->x = u;
      else
        point->y = u;

      point->flags |= touch_flag;
    }
  }
}


/****************************************************************
 *
 *                    WEAK POINT INTERPOLATION
 *
 ****************************************************************/


/* shift the original coordinates of all points between `p1' and */
/* `p2' to get hinted coordinates, using the same difference as */
/* given by `ref' */

static void
ta_iup_shift(TA_Point p1,
             TA_Point p2,
             TA_Point ref)
{
  TA_Point p;
  FT_Pos delta = ref->u - ref->v;


  if (delta == 0)
    return;

  for (p = p1; p < ref; p++)
    p->u = p->v + delta;

  for (p = ref + 1; p <= p2; p++)
    p->u = p->v + delta;
}


/* interpolate the original coordinates of all points between `p1' and */
/* `p2' to get hinted coordinates, using `ref1' and `ref2' as the */
/* reference points;  the `u' and `v' members are the current and */
/* original coordinate values, respectively. */

/* details can be found in the TrueType bytecode specification */

static void
ta_iup_interp(TA_Point p1,
              TA_Point p2,
              TA_Point ref1,
              TA_Point ref2)
{
  TA_Point p;
  FT_Pos u;
  FT_Pos v1 = ref1->v;
  FT_Pos v2 = ref2->v;
  FT_Pos d1 = ref1->u - v1;
  FT_Pos d2 = ref2->u - v2;


  if (p1 > p2)
    return;

  if (v1 == v2)
  {
    for (p = p1; p <= p2; p++)
    {
      u = p->v;

      if (u <= v1)
        u += d1;
      else
        u += d2;

      p->u = u;
    }
    return;
  }

  if (v1 < v2)
  {
    for (p = p1; p <= p2; p++)
    {
      u = p->v;

      if (u <= v1)
        u += d1;
      else if (u >= v2)
        u += d2;
      else
        u = ref1->u + FT_MulDiv(u - v1, ref2->u - ref1->u, v2 - v1);

      p->u = u;
    }
  }
  else
  {
    for (p = p1; p <= p2; p++)
    {
      u = p->v;

      if (u <= v2)
        u += d2;
      else if (u >= v1)
        u += d1;
      else
        u = ref1->u + FT_MulDiv(u - v1, ref2->u - ref1->u, v2 - v1);

      p->u = u;
    }
  }
}


/* hint the weak points -- */
/* this is equivalent to the TrueType `IUP' hinting instruction */

void
ta_glyph_hints_align_weak_points(TA_GlyphHints hints,
                                 TA_Dimension dim)
{
  TA_Point points = hints->points;
  TA_Point point_limit = points + hints->num_points;

  TA_Point* contour = hints->contours;
  TA_Point* contour_limit = contour + hints->num_contours;

  FT_UShort touch_flag;
  TA_Point point;
  TA_Point end_point;
  TA_Point first_point;


  /* pass 1: move segment points to edge positions */

  if (dim == TA_DIMENSION_HORZ)
  {
    touch_flag = TA_FLAG_TOUCH_X;

    for (point = points; point < point_limit; point++)
    {
      point->u = point->x;
      point->v = point->ox;
    }
  }
  else
  {
    touch_flag = TA_FLAG_TOUCH_Y;

    for (point = points; point < point_limit; point++)
    {
      point->u = point->y;
      point->v = point->oy;
    }
  }

  point = points;

  for (; contour < contour_limit; contour++)
  {
    TA_Point first_touched, last_touched;


    point = *contour;
    end_point = point->prev;
    first_point = point;

    /* find first touched point */
    for (;;)
    {
      if (point > end_point) /* no touched point in contour */
        goto NextContour;

      if (point->flags & touch_flag)
        break;

      point++;
    }

    first_touched = point;
    last_touched = point;

    for (;;)
    {
      /* skip any touched neighbours */
      while (point < end_point
             && (point[1].flags & touch_flag) != 0)
        point++;

      last_touched = point;

      /* find the next touched point, if any */
      point++;
      for (;;)
      {
        if (point > end_point)
          goto EndContour;

        if ((point->flags & touch_flag) != 0)
          break;

        point++;
      }

      /* interpolate between last_touched and point */
      ta_iup_interp(last_touched + 1, point - 1,
                    last_touched, point);
    }

  EndContour:
    /* special case: only one point was touched */
    if (last_touched == first_touched)
      ta_iup_shift(first_point, end_point, first_touched);

    else /* interpolate the last part */
    {
      if (last_touched < end_point)
        ta_iup_interp(last_touched + 1, end_point,
                      last_touched, first_touched);

      if (first_touched > points)
        ta_iup_interp(first_point, first_touched - 1,
                      last_touched, first_touched);
    }

  NextContour:
    ;
  }

  /* now save the interpolated values back to x/y */
  if (dim == TA_DIMENSION_HORZ)
  {
    for (point = points; point < point_limit; point++)
      point->x = point->u;
  }
  else
  {
    for (point = points; point < point_limit; point++)
      point->y = point->u;
  }
}


#ifdef TA_CONFIG_OPTION_USE_WARPER

/* apply (small) warp scale and warp delta for given dimension */

static void
ta_glyph_hints_scale_dim(TA_GlyphHints hints,
                         TA_Dimension dim,
                         FT_Fixed scale,
                         FT_Pos delta)
{
  TA_Point points = hints->points;
  TA_Point points_limit = points + hints->num_points;
  TA_Point point;


  if (dim == TA_DIMENSION_HORZ)
  {
    for (point = points; point < points_limit; point++)
      point->x = FT_MulFix(point->fx, scale) + delta;
  }
  else
  {
    for (point = points; point < points_limit; point++)
      point->y = FT_MulFix(point->fy, scale) + delta;
  }
}

#endif /* TA_CONFIG_OPTION_USE_WARPER */

/* end of tahints.c */
