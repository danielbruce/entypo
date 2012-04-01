/* tabytecode.h */

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


#ifndef __TABYTECODE_H__
#define __TABYTECODE_H__


/* symbolic names for bytecode instruction codes */

#define SVTCA_y      0x00 /* set freedom and projection vectors to y axis */
#define SVTCA_x      0x01 /* set freedom and projection vectors to x axis */
#define SPVTCA_y     0x02 /* set projection vector to y axis */
#define SPVTCA_x     0x03 /* set projection vector to x axis */
#define SFVTCA_y     0x04 /* set freedom vector to y axis */
#define SFVTCA_x     0x05 /* set freedom vector to x axis */
#define SPVTL_para   0x06 /* set projection vector parallel to line */
#define SPVTL_perp   0x07 /* set projection vector perpendicular to line */
#define SFVTL_para   0x08 /* set freedom vector parallel to line */
#define SFVTL_perp   0x09 /* set freedom vector perpendicular to line */
#define SPVFS        0x0A /* set projection vector from stack */
#define SFVFS        0x0B /* set freedom vector from stack */
#define GPV          0x0C /* get projection vector */
#define GFV          0x0D /* get freedom vector */
#define SFVTPV       0x0E /* set freedom vector to projection vector */
#define ISECT        0x0F /* move point to intersection of lines */

#define SRP0         0x10 /* set reference point 0 */
#define SRP1         0x11 /* set reference point 1 */
#define SRP2         0x12 /* set reference point 2 */
#define SZP0         0x13 /* set zone pointer 0 */
#define SZP1         0x14 /* set zone pointer 1 */
#define SZP2         0x15 /* set zone pointer 2 */
#define SZPS         0x16 /* set zone pointers */
#define SLOOP        0x17 /* set loop counter */
#define RTG          0x18 /* round to grid */
#define RTHG         0x19 /* round to half grid */
#define SMD          0x1A /* set `minimum_distance' */
#define ELSE         0x1B /* begin of `else' clause */
#define JMPR         0x1C /* jump relative */
#define SCVTCI       0x1D /* set `control_value_cut_in' */
#define SSWCI        0x1E /* set `single_width_cut_in' */
#define SSW          0x1F /* set `single_width_value' */

#define DUP          0x20 /* duplicate top stack element */
#define POP          0x21 /* pop top stack element */
#define CLEAR        0x22 /* clear entire stack */
#define SWAP         0x23 /* swap top two elements of stack */
#define DEPTH        0x24 /* get depth of stack */
#define CINDEX       0x25 /* copy indexed element to top of stack */
#define MINDEX       0x26 /* move indexed element to top of stack */
#define ALIGNPTS     0x27 /* align points */
#define INS_28       0x28 /* undefined */
#define UTP          0x29 /* untouch point */
#define LOOPCALL     0x2A /* loop and call function */
#define CALL         0x2B /* call function */
#define FDEF         0x2C /* define function */
#define ENDF         0x2D /* end of function */
#define MDAP_noround 0x2E /* move direct absolute point without rounding */
#define MDAP_round   0x2F /* move direct absolute point with rounding */

#define IUP_y        0x30 /* interpolate untouched points along y axis */
#define IUP_x        0x31 /* interpolate untouched points along x axis */
#define SHP_rp2      0x32 /* shift point using rp2 */
#define SHP_rp1      0x33 /* shift point using rp1 */
#define SHC_rp2      0x34 /* shift contour using rp2 */
#define SHC_rp1      0x35 /* shift contour using rp1 */
#define SHZ_rp2      0x36 /* shift zone using rp2 */
#define SHZ_rp1      0x37 /* shift zone using rp1 */
#define SHPIX        0x38 /* shift point by pixel amount */
#define IP           0x39 /* interpolate point */
#define MSIRP_norp0  0x3A /* move stack indirect relative point, don't set rp0 */
#define MSIRP_rp0    0x3B /* move stack indirect relative point, set rp0 */
#define ALIGNRP      0x3C /* align relative point */
#define RTDG         0x3D /* round to double grid */
#define MIAP_noround 0x3E /* move indirect absolute point without rounding */
#define MIAP_round   0x3F /* move indirect absolute point with rounding */

#define NPUSHB       0x40 /* push `n' bytes */
#define NPUSHW       0x41 /* push `n' words */
#define WS           0x42 /* write to storage area */
#define RS           0x43 /* read from storage area */
#define WCVTP        0x44 /* write to CVT in pixel units */
#define RCVT         0x45 /* read from CVT */
#define GC_cur       0x46 /* get (projected) coordinate, current position */
#define GC_orig      0x47 /* get (projected) coordinate, original position */
#define SCFS         0x48 /* set coordinate from stack */
#define MD_cur       0x49 /* measure distance, current positions */
#define MD_orig      0x4A /* measure distance, original positions */
#define MPPEM        0x4B /* measure PPEM */
#define MPS          0x4C /* measure point size */
#define FLIPON       0x4D /* set `auto_flip' to TRUE */
#define FLIPOFF      0x4E /* set `auto_flip' to FALSE */
#define DEBUG        0x4F /* ignored */

#define LT           0x50 /* lower than */
#define LTEQ         0x51 /* lower than or equal */
#define GT           0x52 /* greater than */
#define GTEQ         0x53 /* greater than or equal */
#define EQ           0x54 /* equal */
#define NEQ          0x55 /* not equal */
#define ODD          0x56 /* TRUE if odd */
#define EVEN         0x57 /* TRUE if even */
#define IF           0x58 /* start of `if' clause */
#define EIF          0x59 /* end of `if' or `else' clause */
#define AND          0x5A /* logical AND */
#define OR           0x5B /* logical OR */
#define NOT          0x5C /* logical NOT */
#define DELTAP1      0x5D /* delta point exception 1 */
#define SDB          0x5E /* set `delta_base' */
#define SDS          0x5F /* set `delta_shift' */

#define ADD          0x60 /* addition */
#define SUB          0x61 /* subtraction */
#define DIV          0x62 /* division */
#define MUL          0x63 /* multiplication */
#define ABS          0x64 /* absolute value */
#define NEG          0x65 /* negation */
#define FLOOR        0x66 /* floor operation */
#define CEILING      0x67 /* ceiling operation */
#define ROUND_gray   0x68 /* round value with gray compensation */
#define ROUND_black  0x69 /* round value with black compensation */
#define ROUND_white  0x6A /* round value with white compensation */
#define ROUND_3      0x6B /* undefined */
#define NROUND_gray  0x6C /* apply gray compensation */
#define NROUND_black 0x6D /* apply black compensation */
#define NROUND_white 0x6E /* apply white compensation */
#define NROUND_3     0x6F /* undefined */

#define WCVTF        0x70 /* write to CVT in font units */
#define DELTAP2      0x71 /* delta point exception 2 */
#define DELTAP3      0x72 /* delta point exception 3 */
#define DELTAC1      0x73 /* delta cvt exception 1 */
#define DELTAC2      0x74 /* delta cvt exception 2 */
#define DELTAC3      0x75 /* delta cvt exception 3 */
#define SROUND       0x76 /* super round  */
#define S45Round     0x77 /* super round at 45 degrees */
#define JROT         0x78 /* jump relative on TRUE */
#define JROF         0x79 /* jump relative on FALSE */
#define ROFF         0x7A /* turn off rounding */
#define INS_7B       0x7B /* undefined */
#define RUTG         0x7C /* round up to grid */
#define RDTG         0x7D /* round down to grid */
#define SANGW        0x7E /* ignored, obsolete */
#define AA           0x7F /* ignored, obsolete */

#define FLIPPT       0x80 /* flip point on-curve to off-curve and vice versa */
#define FLIPRGON     0x81 /* flip range of points to be on-curve */
#define FlIPRGOFF    0x82 /* flip range of points to be off-curve */
#define INS_83       0x83 /* undefined */
#define INS_84       0x84 /* undefined */
#define SCANCTRL     0x85 /* scan conversion control */
#define SDPVTL_para  0x86 /* set dual projection vector parallel to line */
#define SDPVTL_perp  0x87 /* set dual projection vector perpendicular to line */
#define GETINFO      0x88 /* get information about font scaler and current glyph */
#define IDEF         0x89 /* define instruction */
#define ROLL         0x8A /* roll top three stack elements */
#define MAX          0x8B /* maximum */
#define MIN          0x8C /* minimum */
#define SCANTYPE     0x8D /* set scan conversion rules */
#define INSTCTRL     0x8E /* set instruction control state */
#define INS_8F       0x8F /* undefined */

#define INS_90       0x90 /* undefined */
#define INS_91       0x91
#define INS_92       0x92
#define INS_93       0x93
#define INS_94       0x94
#define INS_95       0x95
#define INS_96       0x96
#define INS_97       0x97
#define INS_98       0x98
#define INS_99       0x99
#define INS_9A       0x9A
#define INS_9B       0x9B
#define INS_9C       0x9C
#define INS_9D       0x9D
#define INS_9E       0x9E
#define INS_9F       0x9F

#define INS_A0       0xA0 /* undefined */
#define INS_A1       0xA1
#define INS_A2       0xA2
#define INS_A3       0xA3
#define INS_A4       0xA4
#define INS_A5       0xA5
#define INS_A6       0xA6
#define INS_A7       0xA7
#define INS_A8       0xA8
#define INS_9A       0x9A
#define INS_AA       0xAA
#define INS_AB       0xAB
#define INS_AC       0xAC
#define INS_AD       0xAD
#define INS_AE       0xAE
#define INS_AF       0xAF

#define PUSHB_1      0xB0 /* push 1 byte */
#define PUSHB_2      0xB1 /* push 2 bytes */
#define PUSHB_3      0xB2 /* push 3 bytes */
#define PUSHB_4      0xB3 /* push 4 bytes */
#define PUSHB_5      0xB4 /* push 5 bytes */
#define PUSHB_6      0xB5 /* push 6 bytes */
#define PUSHB_7      0xB6 /* push 7 bytes */
#define PUSHB_8      0xB7 /* push 8 bytes */
#define PUSHW_1      0xB8 /* push 1 word */
#define PUSHW_2      0xB9 /* push 2 words */
#define PUSHW_3      0xBA /* push 3 words */
#define PUSHW_4      0xBB /* push 4 words */
#define PUSHW_5      0xBC /* push 5 words */
#define PUSHW_6      0xBD /* push 6 words */
#define PUSHW_7      0xBE /* push 7 words */
#define PUSHW_8      0xBF /* push 8 words */

#define MDRP_norp0_nokeep_noround_gray  0xC0 /* move direct relative point */
#define MDRP_norp0_nokeep_noround_black 0xC1
#define MDRP_norp0_nokeep_noround_white 0xC2
#define MDRP_norp0_nokeep_noround_3     0xC3 /* undefined */
#define MDRP_norp0_nokeep_round_gray    0xC4
#define MDRP_norp0_nokeep_round_black   0xC5
#define MDRP_norp0_nokeep_round_white   0xC6
#define MDRP_norp0_nokeep_round_3       0xC7
#define MDRP_norp0_keep_noround_gray    0xC8
#define MDRP_norp0_keep_noround_black   0xC9
#define MDRP_norp0_keep_noround_white   0xCA
#define MDRP_norp0_keep_noround_3       0xCB
#define MDRP_norp0_keep_round_gray      0xCC
#define MDRP_norp0_keep_round_black     0xCD
#define MDRP_norp0_keep_round_white     0xCE
#define MDRP_norp0_keep_round_3         0xCF

#define MDRP_rp0_nokeep_noround_gray    0xD0
#define MDRP_rp0_nokeep_noround_black   0xD1
#define MDRP_rp0_nokeep_noround_white   0xD2
#define MDRP_rp0_nokeep_noround_3       0xD3
#define MDRP_rp0_nokeep_round_gray      0xD4
#define MDRP_rp0_nokeep_round_black     0xD5
#define MDRP_rp0_nokeep_round_white     0xD6
#define MDRP_rp0_nokeep_round_3         0xD7
#define MDRP_rp0_keep_noround_gray      0xD8
#define MDRP_rp0_keep_noround_black     0xD9
#define MDRP_rp0_keep_noround_white     0xDA
#define MDRP_rp0_keep_noround_3         0xDB
#define MDRP_rp0_keep_round_gray        0xDC
#define MDRP_rp0_keep_round_black       0xDD
#define MDRP_rp0_keep_round_white       0xDE
#define MDRP_rp0_keep_round_3           0xDF

#define MIRP_norp0_nokeep_noround_gray  0xE0 /* move indirect relative point */
#define MIRP_norp0_nokeep_noround_black 0xE1
#define MIRP_norp0_nokeep_noround_white 0xE2
#define MIRP_norp0_nokeep_noround_3     0xE3 /* undefined */
#define MIRP_norp0_nokeep_round_gray    0xE4
#define MIRP_norp0_nokeep_round_black   0xE5
#define MIRP_norp0_nokeep_round_white   0xE6
#define MIRP_norp0_nokeep_round_3       0xE7
#define MIRP_norp0_keep_noround_gray    0xE8
#define MIRP_norp0_keep_noround_black   0xE9
#define MIRP_norp0_keep_noround_white   0xEA
#define MIRP_norp0_keep_noround_3       0xEB
#define MIRP_norp0_keep_round_gray      0xEC
#define MIRP_norp0_keep_round_black     0xED
#define MIRP_norp0_keep_round_white     0xEE
#define MIRP_norp0_keep_round_3         0xEF

#define MIRP_rp0_nokeep_noround_gray    0xF0
#define MIRP_rp0_nokeep_noround_black   0xF1
#define MIRP_rp0_nokeep_noround_white   0xF2
#define MIRP_rp0_nokeep_noround_3       0xF3
#define MIRP_rp0_nokeep_round_gray      0xF4
#define MIRP_rp0_nokeep_round_black     0xF5
#define MIRP_rp0_nokeep_round_white     0xF6
#define MIRP_rp0_nokeep_round_3         0xF7
#define MIRP_rp0_keep_noround_gray      0xF8
#define MIRP_rp0_keep_noround_black     0xF9
#define MIRP_rp0_keep_noround_white     0xFA
#define MIRP_rp0_keep_noround_3         0xFB
#define MIRP_rp0_keep_round_gray        0xFC
#define MIRP_rp0_keep_round_black       0xFD
#define MIRP_rp0_keep_round_white       0xFE
#define MIRP_rp0_keep_round_3           0xFF


/* symbolic names for storage area locations */

#define sal_i 0
#define sal_j sal_i + 1
#define sal_k sal_j + 1
#define sal_temp1 sal_k + 1
#define sal_temp2 sal_temp1 + 1
#define sal_temp3 sal_temp2 + 1
#define sal_limit sal_temp3 + 1
#define sal_func sal_limit +1
#define sal_anchor sal_func + 1
#define sal_point_min sal_anchor + 1
#define sal_point_max sal_point_min + 1
#define sal_base sal_point_max + 1
#define sal_num_packed_segments sal_base + 1
#define sal_segment_offset sal_num_packed_segments + 1 /* must be last */


/* bytecode function numbers */

/* 0 */
#define bci_round 0
#define bci_compute_stem_width bci_round + 1
#define bci_loop bci_compute_stem_width + 1
#define bci_cvt_rescale bci_loop + 1
#define bci_blue_round bci_cvt_rescale + 1
#define bci_decrement_component_counter bci_blue_round + 1
#define bci_get_point_extrema bci_decrement_component_counter + 1
#define bci_nibbles bci_get_point_extrema + 1

/* 8 */
#define bci_create_segment bci_nibbles + 1
#define bci_create_segments bci_create_segment + 1

/* 10 */
/* the next ten entries must stay in this order */
#define bci_create_segments_0 bci_create_segments + 1
#define bci_create_segments_1 bci_create_segments_0 + 1
#define bci_create_segments_2 bci_create_segments_1 + 1
#define bci_create_segments_3 bci_create_segments_2 + 1
#define bci_create_segments_4 bci_create_segments_3 + 1
#define bci_create_segments_5 bci_create_segments_4 + 1
#define bci_create_segments_6 bci_create_segments_5 + 1
#define bci_create_segments_7 bci_create_segments_6 + 1
#define bci_create_segments_8 bci_create_segments_7 + 1
#define bci_create_segments_9 bci_create_segments_8 + 1

#define bci_create_segments_composite bci_create_segments_9 + 1

/* 21 */
/* the next ten entries must stay in this order */
#define bci_create_segments_composite_0 bci_create_segments_composite + 1
#define bci_create_segments_composite_1 bci_create_segments_composite_0 + 1
#define bci_create_segments_composite_2 bci_create_segments_composite_1 + 1
#define bci_create_segments_composite_3 bci_create_segments_composite_2 + 1
#define bci_create_segments_composite_4 bci_create_segments_composite_3 + 1
#define bci_create_segments_composite_5 bci_create_segments_composite_4 + 1
#define bci_create_segments_composite_6 bci_create_segments_composite_5 + 1
#define bci_create_segments_composite_7 bci_create_segments_composite_6 + 1
#define bci_create_segments_composite_8 bci_create_segments_composite_7 + 1
#define bci_create_segments_composite_9 bci_create_segments_composite_8 + 1

/* 31 */
#define bci_align_segment bci_create_segments_composite_9 + 1
#define bci_align_segments bci_align_segment + 1

/* 33 */
#define bci_scale_contour bci_align_segments + 1
#define bci_scale_glyph bci_scale_contour + 1
#define bci_scale_composite_glyph bci_scale_glyph + 1
#define bci_shift_contour bci_scale_composite_glyph + 1
#define bci_shift_subglyph bci_shift_contour + 1

/* 38 */
#define bci_ip_outer_align_point bci_shift_subglyph + 1
#define bci_ip_on_align_points bci_ip_outer_align_point + 1
#define bci_ip_between_align_point bci_ip_on_align_points + 1
#define bci_ip_between_align_points bci_ip_between_align_point + 1

/* 42 */
#define bci_adjust_common bci_ip_between_align_points + 1
#define bci_stem_common bci_adjust_common + 1
#define bci_serif_common bci_stem_common + 1
#define bci_serif_anchor_common bci_serif_common + 1
#define bci_serif_link1_common bci_serif_anchor_common + 1
#define bci_serif_link2_common bci_serif_link1_common + 1

/* 48 */
#define bci_lower_bound bci_serif_link2_common + 1
#define bci_upper_bound bci_lower_bound + 1
#define bci_lower_upper_bound bci_upper_bound + 1

/* 51 */
#define bci_adjust_bound bci_lower_upper_bound + 1
#define bci_stem_bound bci_adjust_bound + 1
#define bci_link bci_stem_bound + 1
#define bci_anchor bci_link + 1
#define bci_adjust bci_anchor + 1
#define bci_stem bci_adjust + 1

/* the order of the `bci_action_*' entries must correspond */
/* to the order of the TA_Action enumeration entries (in `tahints.h') */

/* 57 */
#define bci_action_ip_before bci_stem + 1
#define bci_action_ip_after bci_action_ip_before + 1
#define bci_action_ip_on bci_action_ip_after + 1
#define bci_action_ip_between bci_action_ip_on + 1

/* 61 */
#define bci_action_blue bci_action_ip_between + 1
#define bci_action_blue_anchor bci_action_blue + 1

/* 63 */
#define bci_action_anchor bci_action_blue_anchor + 1
#define bci_action_anchor_serif bci_action_anchor + 1
#define bci_action_anchor_round bci_action_anchor_serif + 1
#define bci_action_anchor_round_serif bci_action_anchor_round + 1

/* 67 */
#define bci_action_adjust bci_action_anchor_round_serif + 1
#define bci_action_adjust_serif bci_action_adjust + 1
#define bci_action_adjust_round bci_action_adjust_serif + 1
#define bci_action_adjust_round_serif bci_action_adjust_round + 1
#define bci_action_adjust_bound bci_action_adjust_round_serif + 1
#define bci_action_adjust_bound_serif bci_action_adjust_bound + 1
#define bci_action_adjust_bound_round bci_action_adjust_bound_serif + 1
#define bci_action_adjust_bound_round_serif bci_action_adjust_bound_round + 1

/* 75 */
#define bci_action_link bci_action_adjust_bound_round_serif + 1
#define bci_action_link_serif bci_action_link + 1
#define bci_action_link_round bci_action_link_serif + 1
#define bci_action_link_round_serif bci_action_link_round + 1

/* 79 */
#define bci_action_stem bci_action_link_round_serif + 1
#define bci_action_stem_serif bci_action_stem + 1
#define bci_action_stem_round bci_action_stem_serif + 1
#define bci_action_stem_round_serif bci_action_stem_round + 1
#define bci_action_stem_bound bci_action_stem_round_serif + 1
#define bci_action_stem_bound_serif bci_action_stem_bound + 1
#define bci_action_stem_bound_round bci_action_stem_bound_serif + 1
#define bci_action_stem_bound_round_serif bci_action_stem_bound_round + 1

/* 87 */
#define bci_action_serif bci_action_stem_bound_round_serif + 1
#define bci_action_serif_lower_bound bci_action_serif + 1
#define bci_action_serif_upper_bound bci_action_serif_lower_bound + 1
#define bci_action_serif_lower_upper_bound bci_action_serif_upper_bound + 1

/* 91 */
#define bci_action_serif_anchor bci_action_serif_lower_upper_bound + 1
#define bci_action_serif_anchor_lower_bound bci_action_serif_anchor + 1
#define bci_action_serif_anchor_upper_bound bci_action_serif_anchor_lower_bound + 1
#define bci_action_serif_anchor_lower_upper_bound bci_action_serif_anchor_upper_bound + 1

/* 95 */
#define bci_action_serif_link1 bci_action_serif_anchor_lower_upper_bound + 1
#define bci_action_serif_link1_lower_bound bci_action_serif_link1 + 1
#define bci_action_serif_link1_upper_bound bci_action_serif_link1_lower_bound + 1
#define bci_action_serif_link1_lower_upper_bound bci_action_serif_link1_upper_bound + 1

/* 99 */
#define bci_action_serif_link2 bci_action_serif_link1_lower_upper_bound + 1
#define bci_action_serif_link2_lower_bound bci_action_serif_link2 + 1
#define bci_action_serif_link2_upper_bound bci_action_serif_link2_lower_bound + 1
#define bci_action_serif_link2_lower_upper_bound bci_action_serif_link2_upper_bound + 1

/* 103 */
#define bci_handle_action bci_action_serif_link2_lower_upper_bound + 1
#define bci_hint_glyph bci_handle_action + 1

#define NUM_FDEFS bci_hint_glyph + 1 /* must be last */

/* the first action handler */
#define ACTION_OFFSET bci_action_ip_before


/* symbolic names for run-time CVT locations */
/* (assigned in `prep' or `fpgm') */

#define cvtl_temp 0 /* used for creating twilight points */
#define cvtl_0x10000 cvtl_temp + 1
#define cvtl_scale cvtl_0x10000 + 1
#define cvtl_funits_to_pixels cvtl_scale + 1
#define cvtl_is_extra_light cvtl_funits_to_pixels + 1
#define cvtl_is_subglyph cvtl_is_extra_light + 1
#define cvtl_max_runtime cvtl_is_subglyph + 1 /* must be last */

/* symbolic names for compile-time CVT locations */
/* (assigned in `cvt') */

/* the horizontal and vertical standard widths */
#define CVT_HORZ_STANDARD_WIDTH_OFFSET(font) cvtl_max_runtime
#define CVT_VERT_STANDARD_WIDTH_OFFSET(font) \
          CVT_HORZ_STANDARD_WIDTH_OFFSET(font) + 1

/* the horizontal stem widths */
#define CVT_HORZ_WIDTHS_OFFSET(font) \
          CVT_VERT_STANDARD_WIDTH_OFFSET(font) + 1
#define CVT_HORZ_WIDTHS_SIZE(font) \
          ((font->loader->hints.metrics->clazz->script == TA_SCRIPT_NONE) \
           ? 0 \
           : ((TA_LatinMetrics)font->loader->hints.metrics)->axis[0].width_count)

/* the vertical stem widths */
#define CVT_VERT_WIDTHS_OFFSET(font) \
          CVT_HORZ_WIDTHS_OFFSET(font) + CVT_HORZ_WIDTHS_SIZE(font)
#define CVT_VERT_WIDTHS_SIZE(font) \
          ((font->loader->hints.metrics->clazz->script == TA_SCRIPT_NONE) \
           ? 0 \
           : ((TA_LatinMetrics)font->loader->hints.metrics)->axis[1].width_count)

/* the number of blue zones */
#define CVT_BLUES_SIZE(font) \
          ((font->loader->hints.metrics->clazz->script == TA_SCRIPT_NONE) \
           ? 0 \
           : ((TA_LatinMetrics)font->loader->hints.metrics)->axis[1].blue_count)

/* the blue zone values for flat and round edges */
#define CVT_BLUE_REFS_OFFSET(font) \
          CVT_VERT_WIDTHS_OFFSET(font) + CVT_VERT_WIDTHS_SIZE(font)
#define CVT_BLUE_SHOOTS_OFFSET(font) \
          CVT_BLUE_REFS_OFFSET(font) + CVT_BLUES_SIZE(font)

#endif /* __TABYTECODE_H__ */

/* end of tabytecode.h */
