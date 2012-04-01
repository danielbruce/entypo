/* tahints.h */

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


/* originally file `afhints.h' (2011-Mar-28) from FreeType */

/* heavily modified 2011 by Werner Lemberg <wl@gnu.org> */

#ifndef __TAHINTS_H__
#define __TAHINTS_H__

#include "tatypes.h"

#define xxTA_SORT_SEGMENTS


/* the definition of outline glyph hints; these are shared */
/* by all script analysis routines (until now) */

typedef enum TA_Dimension_
{
  TA_DIMENSION_HORZ = 0, /* x coordinates, i.e. vert. segments & edges */
  TA_DIMENSION_VERT = 1, /* y coordinates, i.e. horz. segments & edges */
  TA_DIMENSION_MAX /* do not remove */
} TA_Dimension;


/* hint directions -- the values are computed so that two vectors */
/* are in opposite directions iff `dir1 + dir2 == 0' */

typedef enum TA_Direction_
{
  TA_DIR_NONE = 4,
  TA_DIR_RIGHT = 1,
  TA_DIR_LEFT= -1,
  TA_DIR_UP = 2,
  TA_DIR_DOWN = -2
} TA_Direction;


/*
 *  The following explanations are mostly taken from the article
 *
 *    Real-Time Grid Fitting of Typographic Outlines
 *
 *  by David Turner and Werner Lemberg
 *
 *   http://www.tug.org/TUGboat/Articles/tb24-3/lemberg.pdf
 *
 *
 *  Segments
 *
 *    `ta_{cjk,latin,...}_hints_compute_segments' are the functions to
 *    find segments in an outline.  A segment is a series of consecutive
 *    points that are approximately aligned along a coordinate axis.  The
 *    analysis to do so is specific to a script.
 *
 *    A segment must have at least two points, except in the case of
 *    `fake' segments that are generated to hint metrics appropriately,
 *    and which consist of a single point.
 *
 *
 *  Edges
 *
 *    As soon as segments are defined, the auto-hinter groups them into
 *    edges.  An edge corresponds to a single position on the main
 *    dimension that collects one or more segments (allowing for a small
 *    threshold).
 *
 *    The auto-hinter first tries to grid fit edges, then to align
 *    segments on the edges unless it detects that they form a serif.
 *
 *    `ta_{cjk,latin,...}_hints_compute_edges' are the functions to find
 *    edges; they are specific to a script.
 *
 *
 *                      A          H
 *                       |        |
 *                       |        |
 *                       |        |
 *                       |        |
 *         C             |        |             F
 *          +------<-----+        +-----<------+
 *          |             B      G             |
 *          |                                  |
 *          |                                  |
 *          +--------------->------------------+
 *         D                                    E
 *
 *
 *  Stems
 *
 *    Segments need to be `linked' to other ones in order to detect stems.
 *    A stem is made of two segments that face each other in opposite
 *    directions and that are sufficiently close to each other.  Using
 *    vocabulary from the TrueType specification, stem segments form a
 *    `black distance'.
 *
 *    In the above ASCII drawing, the horizontal segments are BC, DE, and
 *    FG; the vertical segments are AB, CD, EF, and GH.
 *
 *    Each segment has at most one `best' candidate to form a black
 *    distance, or no candidate at all.  Notice that two distinct segments
 *    can have the same candidate, which frequently means a serif.
 *
 *    A stem is recognized by the following condition:
 *
 *      best segment_1 = segment_2 && best segment_2 = segment_1
 *
 *    The best candidate is stored in field `link' in structure
 *    `TA_Segment'.
 *
 *    Stems are detected by `ta_{cjk,latin,...}_hint_edges'.
 *
 *    In the above ASCII drawing, the best candidate for both AB and CD is
 *    GH, while the best candidate for GH is AB.  Similarly, the best
 *    candidate for EF and GH is AB, while the best candidate for AB is
 *    GH.
 *
 *
 *  Serifs
 *
 *    On the opposite, a serif has
 *
 *      best segment_1 = segment_2 && best segment_2 != segment_1
 *
 *    where segment_1 corresponds to the serif segment (CD and EF in the
 *    above ASCII drawing).
 *
 *    The best candidate is stored in field `serif' in structure
 *    `TA_Segment' (and `link' is set to NULL).
 *
 *    Serifs are detected by `ta_{cjk,latin,...}_hint_edges'.
 *
 *
 *  Touched points
 *
 *    A point is called `touched' if it has been processed somehow by the
 *    auto-hinter.  It basically means that it shouldn't be moved again
 *    (or moved only under certain constraints to preserve the already
 *    applied processing).
 *
 *
 *  Flat and round segments
 *
 *    Segments are `round' or `flat', depending on the series of points
 *    that define them.  A segment is round if the next and previous point
 *    of an extremum (which can be either a single point or sequence of
 *    points) are both conic or cubic control points.  Otherwise, a
 *    segment with an extremum is flat.
 *
 *
 *  Strong Points
 *
 *    Experience has shown that points which are not part of an edge need
 *    to be interpolated linearly between their two closest edges, even if
 *    these are not part of the contour of those particular points.
 *    Typical candidates for this are
 *
 *    - angle points (i.e., points where the `in' and `out' direction
 *      differ greatly)
 *
 *    - inflection points (i.e., where the `in' and `out' angles are the
 *      same, but the curvature changes sign)
 *
 *    `ta_glyph_hints_align_strong_points' is the function which takes
 *    care of such situations; it is equivalent to the TrueType `IP'
 *    hinting instruction.
 *
 *
 *  Weak Points
 *
 *    Other points in the outline must be interpolated using the
 *    coordinates of their previous and next unfitted contour neighbours.
 *    These are called `weak points' and are touched by the function
 *    `ta_glyph_hints_align_weak_points', equivalent to the TrueType `IUP'
 *    hinting instruction.  Typical candidates are control points and
 *    points on the contour without a major direction.
 *
 *    The major effect is to reduce possible distortion caused by
 *    alignment of edges and strong points, thus weak points are processed
 *    after strong points.
 */


/* point hint flags */
#define TA_FLAG_NONE 0

/* point type flags */
#define TA_FLAG_CONIC (1 << 0)
#define TA_FLAG_CUBIC (1 << 1)
#define TA_FLAG_CONTROL (TA_FLAG_CONIC | TA_FLAG_CUBIC)

/* point extremum flags */
#define TA_FLAG_EXTREMA_X (1 << 2)
#define TA_FLAG_EXTREMA_Y (1 << 3)

/* point roundness flags */
#define TA_FLAG_ROUND_X (1 << 4)
#define TA_FLAG_ROUND_Y (1 << 5)

/* point touch flags */
#define TA_FLAG_TOUCH_X (1 << 6)
#define TA_FLAG_TOUCH_Y (1 << 7)

/* candidates for weak interpolation have this flag set */
#define TA_FLAG_WEAK_INTERPOLATION (1 << 8)

/* all inflection points in the outline have this flag set */
#define TA_FLAG_INFLECTION (1 << 9)


/* edge hint flags */
#define TA_EDGE_NORMAL 0
#define TA_EDGE_ROUND (1 << 0)
#define TA_EDGE_SERIF (1 << 1)
#define TA_EDGE_DONE (1 << 2)


typedef struct TA_PointRec_* TA_Point;
typedef struct TA_SegmentRec_* TA_Segment;
typedef struct TA_EdgeRec_* TA_Edge;


typedef struct TA_PointRec_
{
  FT_UShort flags; /* point flags used by hinter */
  FT_Char in_dir; /* direction of inwards vector */
  FT_Char out_dir; /* direction of outwards vector */

  FT_Pos ox, oy; /* original, scaled position */
  FT_Short fx, fy; /* original, unscaled position (in font units) */
  FT_Pos x, y; /* current position */
  FT_Pos u, v; /* current (x,y) or (y,x) depending on context */

  TA_Point next; /* next point in contour */
  TA_Point prev; /* previous point in contour */
} TA_PointRec;


typedef struct TA_SegmentRec_
{
  FT_Byte flags; /* edge/segment flags for this segment */
  FT_Char dir; /* segment direction */
  FT_Short pos; /* position of segment */
  FT_Short min_coord; /* minimum coordinate of segment */
  FT_Short max_coord; /* maximum coordinate of segment */
  FT_Short height; /* the hinted segment height */

  TA_Edge edge; /* the segment's parent edge */
  TA_Segment edge_next; /* link to next segment in parent edge */

  TA_Segment link; /* (stem) link segment */
  TA_Segment serif; /* primary segment for serifs */
  FT_Pos num_linked; /* number of linked segments */
  FT_Pos score; /* used during stem matching */
  FT_Pos len; /* used during stem matching */

  TA_Point first; /* first point in edge segment */
  TA_Point last; /* last point in edge segment */
} TA_SegmentRec;


typedef struct TA_EdgeRec_
{
  FT_Short fpos; /* original, unscaled position (in font units) */
  FT_Pos opos; /* original, scaled position */
  FT_Pos pos; /* current position */

  FT_Byte flags; /* edge flags */
  FT_Char dir; /* edge direction */
  FT_Fixed scale; /* used to speed up interpolation between edges */

  TA_Width blue_edge; /* non-NULL if this is a blue edge */
  FT_UInt best_blue_idx; /* for the hinting recorder callback */
  FT_Bool best_blue_is_shoot; /* for the hinting recorder callback */

  TA_Edge link; /* link edge */
  TA_Edge serif; /* primary edge for serifs */
  FT_Short num_linked; /* number of linked edges */
  FT_Int score; /* used during stem matching */

  TA_Segment first; /* first segment in edge */
  TA_Segment last; /* last segment in edge */
} TA_EdgeRec;


typedef struct TA_AxisHintsRec_
{
  FT_Int num_segments; /* number of used segments */
  FT_Int max_segments; /* number of allocated segments */
  TA_Segment segments; /* segments array */
#ifdef TA_SORT_SEGMENTS
  FT_Int mid_segments;
#endif

  FT_Int num_edges; /* number of used edges */
  FT_Int max_edges; /* number of allocated edges */
  TA_Edge edges; /* edges array */

  TA_Direction major_dir; /* either vertical or horizontal */
} TA_AxisHintsRec, *TA_AxisHints;


typedef enum TA_Action_
{
  /* point actions */

  ta_ip_before,
  ta_ip_after,
  ta_ip_on,
  ta_ip_between,

  /* edge actions */

  ta_blue,
  ta_blue_anchor,

  ta_anchor,
  ta_anchor_serif,
  ta_anchor_round,
  ta_anchor_round_serif,

  ta_adjust,
  ta_adjust_serif,
  ta_adjust_round,
  ta_adjust_round_serif,
  ta_adjust_bound,
  ta_adjust_bound_serif,
  ta_adjust_bound_round,
  ta_adjust_bound_round_serif,

  ta_link,
  ta_link_serif,
  ta_link_round,
  ta_link_round_serif,

  ta_stem,
  ta_stem_serif,
  ta_stem_round,
  ta_stem_round_serif,
  ta_stem_bound,
  ta_stem_bound_serif,
  ta_stem_bound_round,
  ta_stem_bound_round_serif,

  ta_serif,
  ta_serif_lower_bound,
  ta_serif_upper_bound,
  ta_serif_lower_upper_bound,

  ta_serif_anchor,
  ta_serif_anchor_lower_bound,
  ta_serif_anchor_upper_bound,
  ta_serif_anchor_lower_upper_bound,

  ta_serif_link1,
  ta_serif_link1_lower_bound,
  ta_serif_link1_upper_bound,
  ta_serif_link1_lower_upper_bound,

  ta_serif_link2,
  ta_serif_link2_lower_bound,
  ta_serif_link2_upper_bound,
  ta_serif_link2_lower_upper_bound,

  ta_bound
} TA_Action;


typedef void
(*TA_Hints_Recorder)(TA_Action action,
                     TA_GlyphHints hints,
                     TA_Dimension dim,
                     void* arg1, /* TA_Point or TA_Edge */
                     TA_Edge arg2,
                     TA_Edge arg3,
                     TA_Edge lower_bound,
                     TA_Edge upper_bound);

typedef struct TA_GlyphHintsRec_
{
  FT_Fixed x_scale;
  FT_Pos x_delta;

  FT_Fixed y_scale;
  FT_Pos y_delta;

  FT_Int max_points; /* number of allocated points */
  FT_Int num_points; /* number of used points */
  TA_Point points; /* points array */

  FT_Int max_contours; /* number of allocated contours */
  FT_Int num_contours; /* number of used contours */
  TA_Point* contours; /* contours array */

  TA_AxisHintsRec axis[TA_DIMENSION_MAX];

  FT_UInt32 scaler_flags; /* copy of scaler flags */
  FT_UInt32 other_flags; /* free for script-specific implementations */
  TA_ScriptMetrics metrics;

  FT_Pos xmin_delta; /* used for warping */
  FT_Pos xmax_delta;

  TA_Hints_Recorder recorder;
  void* user;
} TA_GlyphHintsRec;


#define TA_HINTS_TEST_SCALER(h, f) \
          ((h)->scaler_flags & (f))
#define TA_HINTS_TEST_OTHER(h, f) \
          ((h)->other_flags & (f))


#ifdef TA_DEBUG

#define TA_HINTS_DO_HORIZONTAL(h) \
          (!_ta_debug_disable_horz_hints \
           && !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_HORIZONTAL))

#define TA_HINTS_DO_VERTICAL(h) \
          (!_ta_debug_disable_vert_hints \
           && !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_VERTICAL))

#define TA_HINTS_DO_ADVANCE(h) \
          !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_ADVANCE)

#define TA_HINTS_DO_BLUES(h) \
          (!_ta_debug_disable_blue_hints)

#else /* !TA_DEBUG */

#define TA_HINTS_DO_HORIZONTAL(h) \
          !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_HORIZONTAL)

#define TA_HINTS_DO_VERTICAL(h) \
          !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_VERTICAL)

#define TA_HINTS_DO_ADVANCE(h) \
          !TA_HINTS_TEST_SCALER(h, TA_SCALER_FLAG_NO_ADVANCE)

#define TA_HINTS_DO_BLUES(h) \
          1

#endif /* !TA_DEBUG */


TA_Direction
ta_direction_compute(FT_Pos dx,
                     FT_Pos dy);


FT_Error
ta_axis_hints_new_segment(TA_AxisHints axis,
                          TA_Segment* asegment);

FT_Error
ta_axis_hints_new_edge(TA_AxisHints axis,
                       FT_Int fpos,
                       TA_Direction dir,
                       TA_Edge* edge);

#ifdef TA_DEBUG
void
ta_glyph_hints_dump_points(TA_GlyphHints hints);

void
ta_glyph_hints_dump_segments(TA_GlyphHints hints);

void
ta_glyph_hints_dump_edges(TA_GlyphHints hints);
#endif

void
ta_glyph_hints_init(TA_GlyphHints hints);

void
ta_glyph_hints_rescale(TA_GlyphHints hints,
                       TA_ScriptMetrics metrics);

FT_Error
ta_glyph_hints_reload(TA_GlyphHints hints,
                      FT_Outline* outline);

void
ta_glyph_hints_save(TA_GlyphHints hints,
                    FT_Outline* outline);

void
ta_glyph_hints_align_edge_points(TA_GlyphHints hints,
                                 TA_Dimension dim);

void
ta_glyph_hints_align_strong_points(TA_GlyphHints hints,
                                   TA_Dimension dim);

void
ta_glyph_hints_align_weak_points(TA_GlyphHints hints,
                                 TA_Dimension dim);

#ifdef TA_CONFIG_OPTION_USE_WARPER
void
ta_glyph_hints_scale_dim(TA_GlyphHints hints,
                         TA_Dimension dim,
                         FT_Fixed scale,
                         FT_Pos delta);
#endif

void
ta_glyph_hints_done(TA_GlyphHints hints);


#define TA_SEGMENT_LEN(seg) \
          ((seg)->max_coord - (seg)->min_coord)

#define TA_SEGMENT_DIST(seg1, seg2) \
          (((seg1)->pos > (seg2)->pos) ? (seg1)->pos - (seg2)->pos \
                                       : (seg2)->pos - (seg1)->pos)

#endif /* __TAHINTS_H__ */

/* end of tahints.h */
