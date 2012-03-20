/* ta.h */

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


#ifndef __TA_H__
#define __TA_H__

#include <config.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include "taloader.h"
#include "tadummy.h"
#include "talatin.h"

#include <ttfautohint.h>


/* these macros convert 16bit and 32bit numbers into single bytes */
/* using the byte order needed within SFNT files */

#define HIGH(x) (FT_Byte)(((x) & 0xFF00) >> 8)
#define LOW(x) ((x) & 0x00FF)

#define BYTE1(x) (FT_Byte)(((x) & 0xFF000000UL) >> 24);
#define BYTE2(x) (FT_Byte)(((x) & 0x00FF0000UL) >> 16);
#define BYTE3(x) (FT_Byte)(((x) & 0x0000FF00UL) >> 8);
#define BYTE4(x) ((x) & 0x000000FFUL);


/* the length of a dummy `DSIG' table */
#define DSIG_LEN 8

/* the length of our `gasp' table */
#define GASP_LEN 8

/* an empty slot in the table info array */
#define MISSING (FT_ULong)~0

/* the offset to the loca table format in the `head' table */
#define LOCA_FORMAT_OFFSET 51

/* various offsets within the `maxp' table */
#define MAXP_NUM_GLYPHS 4
#define MAXP_MAX_COMPOSITE_POINTS 10
#define MAXP_MAX_COMPOSITE_CONTOURS 12
#define MAXP_MAX_ZONES_OFFSET 14
#define MAXP_MAX_TWILIGHT_POINTS_OFFSET 16
#define MAXP_MAX_STORAGE_OFFSET 18
#define MAXP_MAX_FUNCTION_DEFS_OFFSET 20
#define MAXP_MAX_INSTRUCTION_DEFS_OFFSET 22
#define MAXP_MAX_STACK_ELEMENTS_OFFSET 24
#define MAXP_MAX_INSTRUCTIONS_OFFSET 26
#define MAXP_MAX_COMPONENTS_OFFSET 28

#define MAXP_LEN 32

/* the offset of the type flags field in the `OS/2' table */
#define OS2_FSTYPE_OFFSET 8


/* flags in composite glyph records */
#define ARGS_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_XY_SCALE 0x0040
#define WE_HAVE_A_2X2 0x0080
#define WE_HAVE_INSTR 0x0100

/* flags in simple glyph records */
#define ON_CURVE 0x01
#define X_SHORT_VECTOR 0x02
#define Y_SHORT_VECTOR 0x04
#define REPEAT 0x08
#define SAME_X 0x10
#define SAME_Y 0x20


/* a single glyph */
typedef struct GLYPH_
{
  FT_ULong len1; /* number of bytes before instruction related data */
  FT_ULong len2; /* number of bytes after instruction related data; */
                 /* if zero, this indicates a composite glyph */
  FT_Byte* buf; /* extracted glyph data (without instruction related data) */
  FT_ULong flags_offset; /* offset to last flag in a composite glyph */

  FT_ULong ins_len; /* number of new instructions */
  FT_Byte* ins_buf; /* new instruction data */

  FT_Short num_contours; /* >= 0 for simple glyphs */
  FT_UShort num_points; /* number of points in a simple glyph */

  FT_UShort num_components;
  FT_UShort* components; /* the subglyph indices of a composite glyph */

  FT_UShort num_pointsums;
  FT_UShort* pointsums; /* the pointsums of all composite elements */
                        /* (after walking recursively over all subglyphs) */

  FT_UShort num_composite_contours; /* after recursion */
} GLYPH;

/* a representation of the data in the `glyf' table */
typedef struct glyf_Data_
{
  FT_UShort num_glyphs;
  GLYPH* glyphs;
} glyf_Data;

/* an SFNT table */
typedef struct SFNT_Table_
{
  FT_ULong tag;
  FT_ULong len;
  FT_Byte* buf; /* the table data */
  FT_ULong offset; /* from beginning of file */
  FT_ULong checksum;
  void* data; /* used e.g. for `glyf' table data */
  FT_Bool processed;
} SFNT_Table;

/* we use indices into the SFNT table array to */
/* represent table info records of the TTF header */
typedef FT_ULong SFNT_Table_Info;

/* this structure is used to model a TTF or a subfont within a TTC */
typedef struct SFNT_
{
  FT_Face face;

  SFNT_Table_Info* table_infos;
  FT_ULong num_table_infos;

  /* various SFNT table indices */
  FT_ULong glyf_idx;
  FT_ULong loca_idx;
  FT_ULong head_idx;
  FT_ULong hmtx_idx;
  FT_ULong maxp_idx;
  FT_ULong post_idx;
  FT_ULong OS2_idx;
  FT_ULong GPOS_idx;

  /* values necessary to update the `maxp' table */
  FT_UShort max_composite_points;
  FT_UShort max_composite_contours;
  FT_UShort max_storage;
  FT_UShort max_stack_elements;
  FT_UShort max_twilight_points;
  FT_UShort max_instructions;
  FT_UShort max_components;
} SFNT;

/* our font object */
typedef struct FONT_
{
  FT_Library lib;

  FT_Byte* in_buf;
  size_t in_len;

  FT_Byte* out_buf;
  size_t out_len;

  SFNT* sfnts;
  FT_Long num_sfnts;

  SFNT_Table* tables;
  FT_ULong num_tables;

  FT_Bool have_DSIG;

  TA_LoaderRec loader[1]; /* the interface to the autohinter */

  /* configuration options */
  TA_Progress_Func progress;
  void* progress_data;
  FT_UInt hinting_range_min;
  FT_UInt hinting_range_max;
  FT_Bool pre_hinting;
  FT_Bool no_x_height_snapping;
  FT_Byte* x_height_snapping_exceptions;
  FT_Bool ignore_permissions;
  FT_UInt fallback_script;
} FONT;


#include "tatables.h"
#include "tabytecode.h"


const char*
TA_get_error_message(FT_Error error);

void
TA_get_current_time(FT_ULong* high,
                    FT_ULong* low);

FT_Error
TA_font_init(FONT* font);
void
TA_font_unload(FONT* font,
               const char* in_buf,
               char** out_bufp);

FT_Error
TA_font_file_read(FONT* font,
                  FILE* in_file);
FT_Error
TA_font_file_write(FONT* font,
                   FILE* out_file);

FT_Error
TA_sfnt_build_glyph_instructions(SFNT* sfnt,
                                 FONT* font,
                                 FT_Long idx);

FT_Error
TA_sfnt_split_into_SFNT_tables(SFNT* sfnt,
                               FONT* font);

FT_Error
TA_sfnt_build_cvt_table(SFNT* sfnt,
                        FONT* font);

FT_Error
TA_table_build_DSIG(FT_Byte** DSIG);

FT_Error
TA_sfnt_build_fpgm_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_build_gasp_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_build_glyf_hints(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_split_glyf_table(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_build_glyf_table(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_create_glyf_data(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_update_GPOS_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_update_hmtx_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_build_loca_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_update_maxp_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_update_post_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_build_prep_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_build_TTF_header(SFNT* sfnt,
                         FONT* font,
                         FT_Byte** header_buf,
                         FT_ULong* header_len,
                         FT_Int do_complete);
FT_Error
TA_font_build_TTF(FONT* font);

FT_Error
TA_font_build_TTC(FONT* font);

#endif /* __TA_H__ */

/* end of ta.h */
