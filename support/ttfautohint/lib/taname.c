/* taname.c */

/*
 * Copyright (C) 2012 by Werner Lemberg.
 *
 * This file is part of the ttfautohint library, and may only be used,
 * modified, and distributed under the terms given in `COPYING'.  By
 * continuing to use, modify, or distribute this file you indicate that you
 * have read `COPYING' and understand and accept it fully.
 *
 * The file `COPYING' mentioned in the previous paragraph is distributed
 * with the ttfautohint library.
 */


#include <string.h>
#include "ta.h"


typedef struct Lang_Tag_Record_
{
  FT_UShort len;
  FT_Byte* str;
} Lang_Tag_Record;

typedef struct Name_Record_
{
  FT_UShort platform_id;
  FT_UShort encoding_id;
  FT_UShort language_id;
  FT_UShort name_id;

  FT_UShort len;
  FT_Byte* str;
} Name_Record;

typedef struct Naming_Table_
{
  FT_UShort format;
  FT_UShort string_offset;

  FT_UShort name_count;
  Name_Record* name_records;

  FT_UShort lang_tag_count;
  Lang_Tag_Record* lang_tag_records;
} Naming_Table;


static FT_Error
parse_name_header(FT_Byte** curp,
                  Naming_Table* n,
                  FT_ULong buf_len,
                  FT_Byte** start,
                  FT_Byte** end)
{
  FT_Byte* buf = *curp;
  FT_Byte* p;
  FT_Byte* startp;
  FT_Byte* endp;


  p = *curp;

  n->format = *(p++) << 8;
  n->format += *(p++);
  n->name_count = *(p++) << 8;
  n->name_count += *(p++);
  n->string_offset = *(p++) << 8;
  n->string_offset += *(p++);

  n->name_records = NULL;
  n->lang_tag_records = NULL;

  /* all name strings must be between `startp' and `endp' */
  startp = buf + 6 + 12 * n->name_count;
  endp = buf + buf_len;

  /* format 1 also has language tag records */
  if (n->format == 1)
  {
    FT_Byte* q;


    q = startp;
    if (q + 2 > endp)
      return FT_Err_Invalid_Table;

    n->lang_tag_count = *(q++) << 8;
    n->lang_tag_count += *q;

    startp += 2 + 4 * n->lang_tag_count;
  }
  else
    n->lang_tag_count = 0;

  if (startp > endp)
    return FT_Err_Invalid_Table;

  *start = startp;
  *end = endp;
  *curp = p;

  return TA_Err_Ok;
}


static FT_Error
parse_name_records(FT_Byte** curp,
                   Naming_Table* n,
                   FT_Byte* buf,
                   FT_Byte* startp,
                   FT_Byte* endp,
                   FONT* font)
{
  FT_UShort i, count;
  FT_Byte* p;


  p = *curp;

  if (!n->name_count)
    return TA_Err_Ok;

  /* allocate name records array */
  n->name_records = (Name_Record*)calloc(1, n->name_count
                                            * sizeof (Name_Record));
  if (!n->name_records)
    return FT_Err_Out_Of_Memory;

  /* walk over all name records */
  count = 0;
  for (i = 0; i < n->name_count; i++)
  {
    Name_Record* r = &n->name_records[count];

    FT_UShort offset;
    FT_Byte* s;
    FT_UShort l;


    r->platform_id = *(p++) << 8;
    r->platform_id += *(p++);
    r->encoding_id = *(p++) << 8;
    r->encoding_id += *(p++);
    r->language_id = *(p++) << 8;
    r->language_id += *(p++);
    r->name_id = *(p++) << 8;
    r->name_id += *(p++);

    r->len = *(p++) << 8;
    r->len += *(p++);

    offset = *(p++) << 8;
    offset += *(p++);

    s = buf + n->string_offset + offset;

    /* ignore invalid entries */
    if (s < startp || s + r->len > endp)
      continue;

    /* mac encoding or non-Unicode Windows encoding? */
    if (r->platform_id == 1
        || (r->platform_id == 3 && !(r->encoding_id == 1
                                     || r->encoding_id == 10)))
    {
      /* one-byte or multi-byte encodings */

      /* skip everything after a NULL byte (if any) */
      for (l = 0; l < r->len; l++)
        if (!s[l])
          break;

      /* ignore zero-length entries */
      if (!l)
        continue;

      r->len = l;
    }
    else
    {
      /* (two-byte) UTF-16BE for everything else */

      /* we need an even number of bytes */
      r->len &= ~1;

      /* ignore entries which contain only NULL bytes */
      for (l = 0; l < r->len; l++)
        if (s[l])
          break;
      if (l == r->len)
        continue;
    }

    r->str = (FT_Byte*)malloc(r->len);
    if (!r->str)
      return FT_Err_Out_Of_Memory;
    memcpy(r->str, s, r->len);

    if (font->info)
    {
      if (font->info(r->platform_id,
                     r->encoding_id,
                     r->language_id,
                     r->name_id,
                     &r->len,
                     &r->str,
                     font->info_data))
        continue;
    }
    count++;
  }

  /* shrink name record array if necessary */
  n->name_records = (Name_Record*)realloc(n->name_records,
                                          count * sizeof (Name_Record));
  n->name_count = count;

  return TA_Err_Ok;
}


FT_Error
parse_lang_tag_records(FT_Byte** curp,
                       Naming_Table* n,
                       FT_Byte* buf,
                       FT_Byte* startp,
                       FT_Byte* endp)
{
  FT_UShort i;
  FT_Byte* p;


  p = *curp;

  if (!n->lang_tag_count)
    return TA_Err_Ok;

  /* allocate language tags array */
  n->lang_tag_records = (Lang_Tag_Record*)calloc(
                          1, n->lang_tag_count * sizeof (Lang_Tag_Record));
  if (!n->lang_tag_records)
    return FT_Err_Out_Of_Memory;

  /* walk over all language tag records (if any) */
  for (i = 0; i < n->lang_tag_count; i++)
  {
    Lang_Tag_Record* r = &n->lang_tag_records[i];

    FT_UShort offset;
    FT_Byte* s;


    r->len = *(p++) << 8;
    r->len += *(p++);

    offset = *(p++) << 8;
    offset += *(p++);

    s = buf + n->string_offset + offset;

    /* ignore an invalid entry -- */
    /* contrary to name records, we can't simply remove it */
    /* because references to it should still work */
    /* (we don't apply more fixes */
    /* since ttfautohint is not a TrueType sanitizing tool) */
    if (s < startp || s + r->len > endp)
      continue;

    /* we don't massage the data since we only make a copy */
    r->str = (FT_Byte*)malloc(r->len);
    if (!r->str)
      return FT_Err_Out_Of_Memory;

    memcpy(r->str, s, r->len);
  }

  return TA_Err_Ok;
}


/* we build a non-optimized `name' table, this is, */
/* we don't fold strings `foo' and `foobar' into one string */

static FT_Error
build_name_table(Naming_Table* n,
                 SFNT_Table* name_table)
{
  FT_Byte* buf_new;
  FT_Byte* buf_new_resized;
  FT_ULong buf_new_len;
  FT_ULong len;

  FT_Byte* p;
  FT_Byte* q;
  FT_Byte* base;

  FT_UShort i;
  FT_UShort string_offset;
  FT_ULong data_offset;
  FT_ULong data_len;


  /* we reallocate the array to its real size later on */
  buf_new_len= 6 + 12 * n->name_count;
  if (n->format == 1)
    buf_new_len += 2 + 4 * n->lang_tag_count;

  buf_new = (FT_Byte*)malloc(buf_new_len);
  if (!buf_new)   
    return FT_Err_Out_Of_Memory;

  /* note that the OpenType specification says that `string_offset' is the */
  /* `offset to the start of string storage (from start of table)', */
  /* but this isn't enforced by the major rendering engines */
  /* as long as the final string offsets are valid */
  string_offset = (buf_new_len <= 0xFFFF) ? buf_new_len : 0xFFFF;

  p = buf_new;

  *(p++) = HIGH(n->format);
  *(p++) = LOW(n->format);
  *(p++) = HIGH(n->name_count);
  *(p++) = LOW(n->name_count);
  *(p++) = HIGH(string_offset);
  *(p++) = LOW(string_offset);

  /* first pass */

  data_len = 0;
  for (i = 0; i < n->name_count; i++)
  {
    Name_Record* r = &n->name_records[i];


    *(p++) = HIGH(r->platform_id);
    *(p++) = LOW(r->platform_id);
    *(p++) = HIGH(r->encoding_id);
    *(p++) = LOW(r->encoding_id);
    *(p++) = HIGH(r->language_id);
    *(p++) = LOW(r->language_id);
    *(p++) = HIGH(r->name_id);
    *(p++) = LOW(r->name_id);

    *(p++) = HIGH(r->len);
    *(p++) = LOW(r->len);

    /* the offset field gets filled in later */
    p += 2;

    data_len += r->len;
  }

  if (n->format == 1)
  {
    *(p++) = HIGH(n->lang_tag_count);
    *(p++) = LOW(n->lang_tag_count);

    for (i = 0; i < n->lang_tag_count; i++)
    {
      Lang_Tag_Record* r = &n->lang_tag_records[i];


      *(p++) = HIGH(r->len);
      *(p++) = LOW(r->len);

      /* the offset field gets filled in later */
      p += 2;

      data_len += r->len;
    }
  }

  if (buf_new_len + data_len > 2 * 0xFFFF)
  {
    /* the table would become too large, so we do nothing */
    free(buf_new);
    return TA_Err_Ok;
  }

  data_offset = buf_new_len;

  /* reallocate the buffer to fit its real size */
  buf_new_len += data_len;
  /* make the allocated buffer length a multiple of 4 */
  len = (buf_new_len + 3) & ~3;

  buf_new_resized = (FT_Byte*)realloc(buf_new, len);
  if (!buf_new_resized)
  {
    free(buf_new);
    return FT_Err_Out_Of_Memory;
  }
  buf_new = buf_new_resized;

  base = buf_new + string_offset;
  p = buf_new + data_offset;

  /* second pass */

  /* the first name record offset */
  q = &buf_new[6 + 10];
  for (i = 0; i < n->name_count; i++)
  {
    Name_Record* r = &n->name_records[i];


    /* fill in offset */
    *q = HIGH(p - base);
    *(q + 1) = LOW(p - base);

    /* copy string */
    memcpy(p, r->str, r->len);

    p += r->len;
    q += 12;
  }

  if (n->format == 1)
  {
    /* the first language tag record offset */
    q = &buf_new[6 + 12 * n->name_count + 2 + 2]; 
    for (i = 0; i < n->lang_tag_count; i++)
    {
      Lang_Tag_Record* r = &n->lang_tag_records[i];


      /* fill in offset */
      *q = HIGH(p - base);
      *(q + 1) = LOW(p - base);

      /* copy string */
      memcpy(p, r->str, r->len);

      p += r->len;
      q += 4;
    }
  }

  /* pad end of buffer with zeros */
  switch (buf_new_len % 4)
  {
  case 1:
    *(p++) = 0x00;
  case 2:
    *(p++) = 0x00;
  case 3:
    *(p++) = 0x00;
  default:
    break;
  }

  /* we are done; replace the old buffer with the new one */
  free(name_table->buf);

  name_table->buf = buf_new;
  name_table->len = buf_new_len;

  return TA_Err_Ok;
}


/* we handle the `name' table as optional; */
/* if there are problems not related to allocation, */
/* simply return (or continue, if possible) without signaling an error, */
/* and the original `name' table is not modified */

FT_Error
TA_sfnt_update_name_table(SFNT* sfnt,
                          FONT* font)
{
  FT_Error error;

  SFNT_Table* name_table;
  FT_Byte* buf;
  FT_ULong buf_len;

  Naming_Table n;

  FT_Byte* p;
  FT_Byte* startp;
  FT_Byte* endp;

  FT_UShort i;


  if (sfnt->name_idx == MISSING)
    return TA_Err_Ok;

  name_table = &font->tables[sfnt->name_idx];
  buf = name_table->buf;
  buf_len = name_table->len;

  if (name_table->processed)
    return TA_Err_Ok;

  p = buf;

  error = parse_name_header(&p, &n, buf_len, &startp, &endp);
  if (error)
    return TA_Err_Ok;

  /* due to the structure of the `name' table, */
  /* we must parse it completely, apply our changes, */
  /* and rebuild it from scratch */
  error = parse_name_records(&p, &n, buf, startp, endp, font);
  if (error)
    goto Exit;

  error = parse_lang_tag_records(&p, &n, buf, startp, endp);
  if (error)
    goto Exit;

  error = build_name_table(&n, name_table);
  if (error)
    goto Exit;

  name_table->checksum = TA_table_compute_checksum(name_table->buf,
                                                   name_table->len);

Exit:
  for (i = 0; i < n.name_count; i++)
    free(n.name_records[i].str);
  for (i = 0; i < n.lang_tag_count; i++)
    free(n.lang_tag_records[i].str);

  free(n.name_records);
  free(n.lang_tag_records);

  name_table->processed = 1;

  return error;
}

/* end of taname.c */
