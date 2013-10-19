/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avm.h                                    |
 |                                                                            |
 |                             AVL Vector Mockup                              |
 |                                                                            |
 *----------------------------------------------------------------------------*/
/*
Copyright (c) 2013, Walter Tross <waltertross at gmail dot com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* A mockup of AVL functionality implemented on vectors for testing
 */

#ifndef AVM_H
#define AVM_H

#include <stddef.h>
#include <stdbool.h>

typedef struct avm_vect VECT;

#define AVM_USR  (0 << 1)
#define AVM_MBR  (1 << 1)
#define AVM_PTR  (2 << 1)
#define AVM_CHA  (3 << 1)
#define AVM_STR  (4 << 1)
#define AVM_LNG  (5 << 1)
#define AVM_INT  (6 << 1)
#define AVM_SHT  (7 << 1)
#define AVM_SCH  (8 << 1)
#define AVM_ULN  (9 << 1)
#define AVM_UIN (10 << 1)
#define AVM_USH (11 << 1)
#define AVM_UCH (12 << 1)
#define AVM_FLT (13 << 1)
#define AVM_DBL (14 << 1)

#define AVM_CHARS  AVM_CHA
#define AVM_LONG   AVM_LNG
#define AVM_SHORT  AVM_SHT
#define AVM_SCHAR  AVM_SCH
#define AVM_ULONG  AVM_ULN
#define AVM_UINT   AVM_UIN
#define AVM_USHORT AVM_USH
#define AVM_UCHAR  AVM_UCH
#define AVM_FLOAT  AVM_FLT
#define AVM_DOUBLE AVM_DBL

#define AVM_NODUP 0
#define AVM_DUP   1

#define avm_vect_nodup(usrcmp)                         avm_vect(AVM_USR, 0,                         (usrcmp))
#define avm_vect_nodup_mbr(   _struct, member, usrcmp) avm_vect(AVM_MBR, offsetof(_struct, member), (usrcmp))
#define avm_vect_nodup_ptr(   _struct, member, usrcmp) avm_vect(AVM_PTR, offsetof(_struct, member), (usrcmp))
#define avm_vect_nodup_chars( _struct, member)         avm_vect(AVM_CHA, offsetof(_struct, member), NULL)
#define avm_vect_nodup_str(   _struct, member)         avm_vect(AVM_STR, offsetof(_struct, member), NULL)
#define avm_vect_nodup_long(  _struct, member)         avm_vect(AVM_LNG, offsetof(_struct, member), NULL)
#define avm_vect_nodup_int(   _struct, member)         avm_vect(AVM_INT, offsetof(_struct, member), NULL)
#define avm_vect_nodup_short( _struct, member)         avm_vect(AVM_SHT, offsetof(_struct, member), NULL)
#define avm_vect_nodup_schar( _struct, member)         avm_vect(AVM_SCH, offsetof(_struct, member), NULL)
#define avm_vect_nodup_ulong( _struct, member)         avm_vect(AVM_ULN, offsetof(_struct, member), NULL)
#define avm_vect_nodup_uint(  _struct, member)         avm_vect(AVM_UIN, offsetof(_struct, member), NULL)
#define avm_vect_nodup_ushort(_struct, member)         avm_vect(AVM_USH, offsetof(_struct, member), NULL)
#define avm_vect_nodup_uchar( _struct, member)         avm_vect(AVM_UCH, offsetof(_struct, member), NULL)
#define avm_vect_nodup_float( _struct, member)         avm_vect(AVM_FLT, offsetof(_struct, member), NULL)
#define avm_vect_nodup_double(_struct, member)         avm_vect(AVM_DBL, offsetof(_struct, member), NULL)

#define avm_vect_dup(usrcmp)                         avm_vect(AVM_USR|AVM_DUP, 0,                         (usrcmp))
#define avm_vect_dup_mbr(   _struct, member, usrcmp) avm_vect(AVM_MBR|AVM_DUP, offsetof(_struct, member), (usrcmp))
#define avm_vect_dup_ptr(   _struct, member, usrcmp) avm_vect(AVM_PTR|AVM_DUP, offsetof(_struct, member), (usrcmp))
#define avm_vect_dup_chars( _struct, member)         avm_vect(AVM_CHA|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_str(   _struct, member)         avm_vect(AVM_STR|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_long(  _struct, member)         avm_vect(AVM_LNG|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_int(   _struct, member)         avm_vect(AVM_INT|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_short( _struct, member)         avm_vect(AVM_SHT|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_schar( _struct, member)         avm_vect(AVM_SCH|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_ulong( _struct, member)         avm_vect(AVM_ULN|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_uint(  _struct, member)         avm_vect(AVM_UIN|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_ushort(_struct, member)         avm_vect(AVM_USH|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_uchar( _struct, member)         avm_vect(AVM_UCH|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_float( _struct, member)         avm_vect(AVM_FLT|AVM_DUP, offsetof(_struct, member), NULL)
#define avm_vect_dup_double(_struct, member)         avm_vect(AVM_DBL|AVM_DUP, offsetof(_struct, member), NULL)

#define avm_string_vect_nodup() avm_vect(AVM_CHARS,         0, NULL);
#define avm_string_vect_dup()   avm_vect(AVM_CHARS|AVM_DUP, 0, NULL);

VECT *avm_vect(int vecttype, size_t keyoffs, int (*usrcmp)());

bool avm_has_fast_floats();
bool avm_has_fast_doubles();

bool avm_insert(VECT *vect, void *data);

void *avm_remove       (VECT *vect, void *key);
void *avm_remove_mbr   (VECT *vect, void *key);
void *avm_remove_ptr   (VECT *vect, void *key);
void *avm_remove_chars (VECT *vect, char *key);
void *avm_remove_str   (VECT *vect, char *key);
void *avm_remove_long  (VECT *vect, long           key);
void *avm_remove_int   (VECT *vect, int            key);
void *avm_remove_short (VECT *vect, short          key);
void *avm_remove_schar (VECT *vect, signed char    key);
void *avm_remove_ulong (VECT *vect, unsigned long  key);
void *avm_remove_uint  (VECT *vect, unsigned int   key);
void *avm_remove_ushort(VECT *vect, unsigned short key);
void *avm_remove_uchar (VECT *vect, unsigned char  key);
void *avm_remove_float (VECT *vect, float  key);
void *avm_remove_double(VECT *vect, double key);

void *avm_locate       (VECT *vect, void *key);
void *avm_locate_mbr   (VECT *vect, void *key);
void *avm_locate_ptr   (VECT *vect, void *key);
void *avm_locate_chars (VECT *vect, char *key);
void *avm_locate_str   (VECT *vect, char *key);
void *avm_locate_long  (VECT *vect, long           key);
void *avm_locate_int   (VECT *vect, int            key);
void *avm_locate_short (VECT *vect, short          key);
void *avm_locate_schar (VECT *vect, signed char    key);
void *avm_locate_ulong (VECT *vect, unsigned long  key);
void *avm_locate_uint  (VECT *vect, unsigned int   key);
void *avm_locate_ushort(VECT *vect, unsigned short key);
void *avm_locate_uchar (VECT *vect, unsigned char  key);
void *avm_locate_float (VECT *vect, float  key);
void *avm_locate_double(VECT *vect, double key);

void *avm_locate_ge       (VECT *vect, void *key);
void *avm_locate_gt       (VECT *vect, void *key);
void *avm_locate_le       (VECT *vect, void *key);
void *avm_locate_lt       (VECT *vect, void *key);
void *avm_locate_ge_mbr   (VECT *vect, void *key);
void *avm_locate_gt_mbr   (VECT *vect, void *key);
void *avm_locate_le_mbr   (VECT *vect, void *key);
void *avm_locate_lt_mbr   (VECT *vect, void *key);
void *avm_locate_ge_ptr   (VECT *vect, void *key);
void *avm_locate_gt_ptr   (VECT *vect, void *key);
void *avm_locate_le_ptr   (VECT *vect, void *key);
void *avm_locate_lt_ptr   (VECT *vect, void *key);
void *avm_locate_ge_chars (VECT *vect, char *key);
void *avm_locate_gt_chars (VECT *vect, char *key);
void *avm_locate_le_chars (VECT *vect, char *key);
void *avm_locate_lt_chars (VECT *vect, char *key);
void *avm_locate_ge_str   (VECT *vect, char *key);
void *avm_locate_gt_str   (VECT *vect, char *key);
void *avm_locate_le_str   (VECT *vect, char *key);
void *avm_locate_lt_str   (VECT *vect, char *key);
void *avm_locate_ge_long  (VECT *vect, long           key);
void *avm_locate_gt_long  (VECT *vect, long           key);
void *avm_locate_le_long  (VECT *vect, long           key);
void *avm_locate_lt_long  (VECT *vect, long           key);
void *avm_locate_ge_int   (VECT *vect, int            key);
void *avm_locate_gt_int   (VECT *vect, int            key);
void *avm_locate_le_int   (VECT *vect, int            key);
void *avm_locate_lt_int   (VECT *vect, int            key);
void *avm_locate_ge_short (VECT *vect, short          key);
void *avm_locate_gt_short (VECT *vect, short          key);
void *avm_locate_le_short (VECT *vect, short          key);
void *avm_locate_lt_short (VECT *vect, short          key);
void *avm_locate_ge_schar (VECT *vect, signed char    key);
void *avm_locate_gt_schar (VECT *vect, signed char    key);
void *avm_locate_le_schar (VECT *vect, signed char    key);
void *avm_locate_lt_schar (VECT *vect, signed char    key);
void *avm_locate_ge_ulong (VECT *vect, unsigned long  key);
void *avm_locate_gt_ulong (VECT *vect, unsigned long  key);
void *avm_locate_le_ulong (VECT *vect, unsigned long  key);
void *avm_locate_lt_ulong (VECT *vect, unsigned long  key);
void *avm_locate_ge_uint  (VECT *vect, unsigned int   key);
void *avm_locate_gt_uint  (VECT *vect, unsigned int   key);
void *avm_locate_le_uint  (VECT *vect, unsigned int   key);
void *avm_locate_lt_uint  (VECT *vect, unsigned int   key);
void *avm_locate_ge_ushort(VECT *vect, unsigned short key);
void *avm_locate_gt_ushort(VECT *vect, unsigned short key);
void *avm_locate_le_ushort(VECT *vect, unsigned short key);
void *avm_locate_lt_ushort(VECT *vect, unsigned short key);
void *avm_locate_ge_uchar (VECT *vect, unsigned char  key);
void *avm_locate_gt_uchar (VECT *vect, unsigned char  key);
void *avm_locate_le_uchar (VECT *vect, unsigned char  key);
void *avm_locate_lt_uchar (VECT *vect, unsigned char  key);
void *avm_locate_ge_float (VECT *vect, float  key);
void *avm_locate_gt_float (VECT *vect, float  key);
void *avm_locate_le_float (VECT *vect, float  key);
void *avm_locate_lt_float (VECT *vect, float  key);
void *avm_locate_ge_double(VECT *vect, double key);
void *avm_locate_gt_double(VECT *vect, double key);
void *avm_locate_le_double(VECT *vect, double key);
void *avm_locate_lt_double(VECT *vect, double key);

void *avm_locate_first(VECT *vect);
void *avm_locate_last (VECT *vect);

void *avm_scan(    VECT *vect, bool (*callback)());
void *avm_rev_scan(VECT *vect, bool (*callback)());

void *avm_scan_w_ctx(    VECT *vect, bool (*callback)(), void *context);
void *avm_rev_scan_w_ctx(VECT *vect, bool (*callback)(), void *context);

void avm_do(    VECT *vect, void (*callback)());
void avm_rev_do(VECT *vect, void (*callback)());

void avm_do_w_ctx(    VECT *vect, void (*callback)(), void *context);
void avm_rev_do_w_ctx(VECT *vect, void (*callback)(), void *context);

void *avm_first(VECT *vect);
void *avm_last (VECT *vect);

void *avm_start       (VECT *vect, void *key);
void *avm_start_mbr   (VECT *vect, void *key);
void *avm_start_ptr   (VECT *vect, void *key);
void *avm_start_chars (VECT *vect, char *key);
void *avm_start_str   (VECT *vect, char *key);
void *avm_start_long  (VECT *vect, long           key);
void *avm_start_int   (VECT *vect, int            key);
void *avm_start_short (VECT *vect, short          key);
void *avm_start_schar (VECT *vect, signed char    key);
void *avm_start_ulong (VECT *vect, unsigned long  key);
void *avm_start_uint  (VECT *vect, unsigned int   key);
void *avm_start_ushort(VECT *vect, unsigned short key);
void *avm_start_uchar (VECT *vect, unsigned char  key);
void *avm_start_float (VECT *vect, float  key);
void *avm_start_double(VECT *vect, double key);
void *avm_rev_start       (VECT *vect, void *key);
void *avm_rev_start_mbr   (VECT *vect, void *key);
void *avm_rev_start_ptr   (VECT *vect, void *key);
void *avm_rev_start_chars (VECT *vect, char *key);
void *avm_rev_start_str   (VECT *vect, char *key);
void *avm_rev_start_long  (VECT *vect, long           key);
void *avm_rev_start_int   (VECT *vect, int            key);
void *avm_rev_start_short (VECT *vect, short          key);
void *avm_rev_start_schar (VECT *vect, signed char    key);
void *avm_rev_start_ulong (VECT *vect, unsigned long  key);
void *avm_rev_start_uint  (VECT *vect, unsigned int   key);
void *avm_rev_start_ushort(VECT *vect, unsigned short key);
void *avm_rev_start_uchar (VECT *vect, unsigned char  key);
void *avm_rev_start_float (VECT *vect, float  key);
void *avm_rev_start_double(VECT *vect, double key);

void *avm_next(VECT *vect);
void *avm_prev(VECT *vect);

void avm_stop(VECT *vect);

#define avm_link(    tree, _struct, next)  avm_linked_list((tree), offsetof(_struct, next), false)
#define avm_backlink(tree, _struct, prev)  avm_linked_list((tree), offsetof(_struct, prev), true )
void *avm_linked_list(VECT *vect, size_t ptroffs, bool back);

long avm_nodes(VECT *vect);

VECT *avm_copy(VECT *vect);

void avm_empty(VECT *vect);

void avm_free(VECT *vect);

int avm_vect_type(VECT *vect);

#endif
