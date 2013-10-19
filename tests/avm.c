/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avm.c                                    |
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "avm.h"

#define CASE    break; case
#define DEFAULT break; default

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif

typedef unsigned long  ULONG;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef   signed char  SCHAR;

struct avm_vect {
   void **v;
   size_t alloc;
   size_t elems;
   void **cur;
   int  (*usrcmp)();
   size_t keyoffs;
   int    keytype;
   bool   dup;
};

#define USR_KEY  (AVM_USR >> 1)
#define MBR_KEY  (AVM_MBR >> 1)
#define PTR_KEY  (AVM_PTR >> 1)
#define CHA_KEY  (AVM_CHA >> 1)
#define STR_KEY  (AVM_STR >> 1)
#define LNG_KEY  (AVM_LNG >> 1)
#define INT_KEY  (AVM_INT >> 1)
#define SHT_KEY  (AVM_SHT >> 1)
#define SCH_KEY  (AVM_SCH >> 1)
#define ULN_KEY  (AVM_ULN >> 1)
#define UIN_KEY  (AVM_UIN >> 1)
#define USH_KEY  (AVM_USH >> 1)
#define UCH_KEY  (AVM_UCH >> 1)
#define FLT_KEY  (AVM_FLT >> 1)
#define DBL_KEY  (AVM_DBL >> 1)

#define NODUP 0
#define DUP   1

#define PREALLOC 32

/*---------------------------------------------------------------------------*/

#define PTRADD(ptr, offs) ((void *)((char *)(ptr) + (offs)))

#define VALCMP(v1, v2) ( (v1) > (v2) ?  1 : \
                        ((v1) < (v2) ? -1 : 0))

#define KEYCMP(type, k1, k2) VALCMP(*(type *)(k1), *(type *)(k2))

/*---------------------------------------------------------------------------*/

typedef union gkey { /* generic key */
   void  *p;
   long   l;
   int    i;
   short  s;
   SCHAR  sc;
   ULONG  ul;
   UINT   ui;
   USHORT us;
   UCHAR  uc;
   float  f;
   double d;
} GKEY;

/*---------------------------------------------------------------------------*/

typedef struct qkey /* qualified key */
{
   size_t keyoffs;
   int    keytype;
   int  (*usrcmp)(void *, void *);
   GKEY k;
} QKEY;

/*---------------------------------------------------------------------------*/

#define FILL_QKEY(qkey, vect, gkey) \
   (qkey).keyoffs = (vect)->keyoffs; \
   (qkey).keytype = (vect)->keytype; \
   (qkey).usrcmp  = (vect)->usrcmp; \
   (qkey).k       = (gkey);

/*---------------------------------------------------------------------------*/

static int qkey2pcmp(const void *p_qkey, const void *p_elem)
{
   QKEY *qk = (QKEY *)p_qkey;
   void *ek = PTRADD((void *)p_elem, qk->keyoffs);
   switch (qk->keytype) {
   CASE USR_KEY: return (*qk->usrcmp)(qk->k.p, ek);
   CASE MBR_KEY: return (*qk->usrcmp)(qk->k.p, ek);
   CASE PTR_KEY: return (*qk->usrcmp)(qk->k.p,  *(void **)ek);
   CASE CHA_KEY: return strcmp((char *)qk->k.p,  (char  *)ek);
   CASE STR_KEY: return strcmp((char *)qk->k.p, *(char **)ek);
   CASE LNG_KEY: return VALCMP(qk->k.l,  *(long  *)ek);
   CASE INT_KEY: return VALCMP(qk->k.i,  *(int   *)ek);
   CASE SHT_KEY: return VALCMP(qk->k.s,  *(short *)ek);
   CASE SCH_KEY: return VALCMP(qk->k.sc, *(SCHAR *)ek);
   CASE ULN_KEY: return VALCMP(qk->k.ul, *(ULONG *)ek);
   CASE UIN_KEY: return VALCMP(qk->k.ui, *(UINT  *)ek);
   CASE USH_KEY: return VALCMP(qk->k.us, *(USHORT*)ek);
   CASE UCH_KEY: return VALCMP(qk->k.uc, *(UCHAR *)ek);
   CASE FLT_KEY: return VALCMP(qk->k.f,  *(float *)ek);
   CASE DBL_KEY: return VALCMP(qk->k.d,  *(double*)ek);
   DEFAULT: assert( !"bad keytype"); return -1;
   }
}

/*---------------------------------------------------------------------------*/

static int qkey2ppcmp(const void *p_qkey, const void *pp_elem)
{
   const void *p_elem = *(const void **)pp_elem;
   return qkey2pcmp(p_qkey, p_elem);
}

/*===========================================================================*/

VECT *avm_vect(int vecttype, size_t keyoffs, int (*usrcmp)(void *, void *))
{
   VECT *vect = calloc(1, sizeof(VECT));
   vect->v = calloc(PREALLOC, sizeof(void *));
   vect->alloc = PREALLOC;
   vect->elems = 0;
   vect->cur   = NULL;
   vect->usrcmp  = usrcmp;
   vect->keyoffs = keyoffs;
   vect->keytype = vecttype >> 1;
   vect->dup     = vecttype & 1;
   return vect;
}

/*---------------------------------------------------------------------------*/

bool avm_insert(VECT *vect, void *data)
{
   size_t i;
   QKEY qkey;
   void *kp = PTRADD(data, vect->keyoffs);
   vect->cur = NULL;
   qkey.keyoffs = vect->keyoffs;
   qkey.keytype = vect->keytype;
   qkey.usrcmp  = vect->usrcmp;
   switch (vect->keytype) {
   CASE USR_KEY: qkey.k.p  = data;
   CASE MBR_KEY: qkey.k.p  = kp;
   CASE PTR_KEY: qkey.k.p  = *(void **)kp;
   CASE STR_KEY: qkey.k.p  = *(char **)kp;
   CASE CHA_KEY: qkey.k.p  =  (char  *)kp;
   CASE LNG_KEY: qkey.k.l  = *(long  *)kp;
   CASE INT_KEY: qkey.k.i  = *(int   *)kp;
   CASE SHT_KEY: qkey.k.s  = *(short *)kp;
   CASE SCH_KEY: qkey.k.sc = *(SCHAR *)kp;
   CASE ULN_KEY: qkey.k.ul = *(ULONG *)kp;
   CASE UIN_KEY: qkey.k.ui = *(UINT  *)kp;
   CASE USH_KEY: qkey.k.us = *(USHORT*)kp;
   CASE UCH_KEY: qkey.k.uc = *(UCHAR *)kp;
   CASE FLT_KEY: qkey.k.f  = *(float *)kp;
   CASE DBL_KEY: qkey.k.d  = *(double*)kp;
   DEFAULT: assert( !"bad keytype"); return false;
   }
   for (i = 0; i < vect->elems; i++) {
      if (qkey2pcmp(&qkey, vect->v[i]) < 0) {
         break;
      }
   }
   if ( !vect->dup && i > 0 && qkey2pcmp(&qkey, vect->v[i - 1]) == 0) {
      return false;
   }
   if (vect->alloc == vect->elems) {
      vect->alloc *= 2;
      vect->v = realloc(vect->v, vect->alloc * sizeof(void *));
   }
   if (i < vect->elems) {
      memmove(vect->v + i + 1, vect->v + i, (vect->elems - i) * sizeof(void *));
   }
   vect->elems++;
   vect->v[i] = data;
   return true;
}

/*---------------------------------------------------------------------------*/

static void *avm_remove_gkey(VECT *vect, GKEY gkey)
{
   QKEY qkey;
   void **p_elem;
   void *data;
   size_t i;
   vect->cur = NULL;
   if ( !vect->elems) {
      return NULL;
   }
   FILL_QKEY(qkey, vect, gkey)
   p_elem = bsearch(&qkey, vect->v, vect->elems, sizeof(void *), qkey2ppcmp);
   if ( !p_elem) {
      return NULL;
   }
   if (vect->dup) {
      while (p_elem > vect->v && qkey2ppcmp(&qkey, p_elem - 1) == 0) {
         p_elem--;
      }
   }
   data = *p_elem;
   i = p_elem - vect->v;
   vect->elems--;
   if (i < vect->elems) {
      memmove(vect->v + i, vect->v + i + 1, (vect->elems - i) * sizeof(void *));
   }
   return data;
}

void *avm_remove       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_remove_gkey(vect, gkey); }
void *avm_remove_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_remove_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_locate_gkey(VECT *vect, GKEY gkey)
{
   QKEY qkey;
   void **p_elem;
   if ( !vect->elems) {
      return NULL;
   }
   FILL_QKEY(qkey, vect, gkey)
   p_elem = bsearch(&qkey, vect->v, vect->elems, sizeof(void *), qkey2ppcmp);
   if ( !p_elem) {
      return NULL;
   }
   if (vect->dup) {
      while (p_elem > vect->v && qkey2ppcmp(&qkey, p_elem - 1) == 0) {
         p_elem--;
      }
   }
   return *p_elem;
}

void *avm_locate       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_locate_gkey(vect, gkey); }
void *avm_locate_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_locate_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_locate_ge_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = 0; i < vect->elems; i++) {
      if (qkey2pcmp(&qkey, vect->v[i]) <= 0) {
         return vect->v[i];
      }
   }
   return NULL;
}

void *avm_locate_ge       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_locate_ge_gkey(vect, gkey); }
void *avm_locate_ge_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_locate_ge_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_locate_gt_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = 0; i < vect->elems; i++) {
      if (qkey2pcmp(&qkey, vect->v[i]) < 0) {
         return vect->v[i];
      }
   }
   return NULL;
}

void *avm_locate_gt       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_locate_gt_gkey(vect, gkey); }
void *avm_locate_gt_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_locate_gt_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_locate_le_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      if (qkey2pcmp(&qkey, vect->v[i]) >= 0) {
         return vect->v[i];
      }
   }
   return NULL;
}

void *avm_locate_le       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_locate_le_gkey(vect, gkey); }
void *avm_locate_le_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_locate_le_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_locate_lt_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      if (qkey2pcmp(&qkey, vect->v[i]) > 0) {
         return vect->v[i];
      }
   }
   return NULL;
}

void *avm_locate_lt       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_locate_lt_gkey(vect, gkey); }
void *avm_locate_lt_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_locate_lt_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

void *avm_locate_first(VECT *vect)
{
   if ( !vect->elems) {
      return NULL;
   }
   return vect->v[0];
}

/*---------------------------------------------------------------------------*/

void *avm_locate_last(VECT *vect)
{
   if ( !vect->elems) {
      return NULL;
   }
   return vect->v[vect->elems - 1];
}

/*---------------------------------------------------------------------------*/

void *avm_scan(VECT *vect, bool (*callback)())
{
   size_t i;
   for (i = 0; i < vect->elems; i++) {
      if ((*callback)(vect->v[i])) return vect->v[i];
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avm_rev_scan(VECT *vect, bool (*callback)())
{
   size_t i;
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      if ((*callback)(vect->v[i])) return vect->v[i];
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avm_scan_w_ctx(VECT *vect, bool (*callback)(), void *context)
{
   size_t i;
   for (i = 0; i < vect->elems; i++) {
      if ((*callback)(vect->v[i], context)) return vect->v[i];
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avm_rev_scan_w_ctx(VECT *vect, bool (*callback)(), void *context)
{
   size_t i;
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      if ((*callback)(vect->v[i], context)) return vect->v[i];
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void avm_do(VECT *vect, void (*callback)())
{
   size_t i;
   for (i = 0; i < vect->elems; i++) {
      (*callback)(vect->v[i]);
   }
}

/*---------------------------------------------------------------------------*/

void avm_rev_do(VECT *vect, void (*callback)())
{
   size_t i;
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      (*callback)(vect->v[i]);
   }
}

/*---------------------------------------------------------------------------*/

void avm_do_w_ctx(VECT *vect, void (*callback)(), void *context)
{
   size_t i;
   for (i = 0; i < vect->elems; i++) {
      (*callback)(vect->v[i], context);
   }
}

/*---------------------------------------------------------------------------*/

void avm_rev_do_w_ctx(VECT *vect, void (*callback)(), void *context)
{
   size_t i;
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      (*callback)(vect->v[i], context);
   }
}

/*---------------------------------------------------------------------------*/

void *avm_first(VECT *vect)
{
   if ( !vect->elems) {
      return NULL;
   }
   vect->cur = vect->v;
   return *vect->cur;
}

/*---------------------------------------------------------------------------*/

void *avm_last(VECT *vect)
{
   if ( !vect->elems) {
      return NULL;
   }
   vect->cur = vect->v + (vect->elems - 1);
   return *vect->cur;
}

/*---------------------------------------------------------------------------*/

static void *avm_start_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = 0; i < vect->elems; i++) {
      if (qkey2pcmp(&qkey, vect->v[i]) <= 0) {
         vect->cur = &vect->v[i];
         return vect->v[i];
      }
   }
   vect->cur = NULL;
   return NULL;
}

void *avm_start       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_start_gkey(vect, gkey); }
void *avm_start_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_start_gkey(vect, gkey); }
void *avm_start_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_start_gkey(vect, gkey); }
void *avm_start_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_start_gkey(vect, gkey); }
void *avm_start_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_start_gkey(vect, gkey); }
void *avm_start_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_start_gkey(vect, gkey); }
void *avm_start_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_start_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

static void *avm_rev_start_gkey(VECT *vect, GKEY gkey)
{
   size_t i;
   QKEY qkey;
   FILL_QKEY(qkey, vect, gkey)
   /* we could do a binary search here, but let's play safe */
   for (i = vect->elems - 1; i != (size_t)-1; i--) {
      if (qkey2pcmp(&qkey, vect->v[i]) >= 0) {
         vect->cur = &vect->v[i];
         return vect->v[i];
      }
   }
   vect->cur = NULL;
   return NULL;
}

void *avm_rev_start       (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_mbr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_chars (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_ptr   (VECT *vect, void          *key) { GKEY gkey; gkey.p  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_str   (VECT *vect, char          *key) { GKEY gkey; gkey.p  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_long  (VECT *vect, long           key) { GKEY gkey; gkey.l  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_int   (VECT *vect, int            key) { GKEY gkey; gkey.i  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_short (VECT *vect, short          key) { GKEY gkey; gkey.s  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_schar (VECT *vect, signed char    key) { GKEY gkey; gkey.sc = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_ulong (VECT *vect, unsigned long  key) { GKEY gkey; gkey.ul = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_uint  (VECT *vect, unsigned int   key) { GKEY gkey; gkey.ui = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_ushort(VECT *vect, unsigned short key) { GKEY gkey; gkey.us = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_uchar (VECT *vect, unsigned char  key) { GKEY gkey; gkey.uc = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_float (VECT *vect, float          key) { GKEY gkey; gkey.f  = key; return avm_rev_start_gkey(vect, gkey); }
void *avm_rev_start_double(VECT *vect, double         key) { GKEY gkey; gkey.d  = key; return avm_rev_start_gkey(vect, gkey); }

/*---------------------------------------------------------------------------*/

void *avm_next(VECT *vect)
{
   if ( !vect->cur) {
      return NULL;
   }
   vect->cur++;
   if (vect->cur >= vect->v + vect->elems) {
      vect->cur = NULL;
      return NULL;
   }
   return *vect->cur;
}

/*---------------------------------------------------------------------------*/

void *avm_prev(VECT *vect)
{
   if (vect->cur <= vect->v) {
      vect->cur = NULL;
      return NULL;
   }
   vect->cur--;
   return *vect->cur;
}

/*---------------------------------------------------------------------------*/

void avm_stop(VECT *vect)
{
   vect->cur = NULL;
}

/*---------------------------------------------------------------------------*/

void *avm_linked_list(VECT *vect, size_t ptroffs, bool back)
{
   void *prev, *next;
   size_t i;

   if (back) {
      prev = NULL;
      for (i = 0; i < vect->elems; i++) {
         *(void **)((char *)vect->v[i] + ptroffs) = prev;
         prev = vect->v[i];
      }
      return prev;
   } else {
      next = NULL;
      for (i = vect->elems - 1; i != (size_t)-1; i--) {
         *(void **)((char *)vect->v[i] + ptroffs) = next;
         next = vect->v[i];
      }
      return next;
   }
}

/*---------------------------------------------------------------------------*/

long avm_nodes(VECT *vect)
{
   return vect->elems;
}

/*---------------------------------------------------------------------------*/

VECT *avm_copy(VECT *vect)
{
   VECT *copy = calloc(1, sizeof(VECT));
   copy->v = malloc(vect->alloc * sizeof(void *));
   memcpy(copy->v, vect->v, vect->elems * sizeof(void *));
   copy->alloc   = vect->alloc;
   copy->elems   = vect->elems;
   copy->cur     = NULL;
   copy->usrcmp  = vect->usrcmp;
   copy->keyoffs = vect->keyoffs;
   copy->keytype = vect->keytype;
   copy->dup     = vect->dup;
   return copy;
}

/*---------------------------------------------------------------------------*/

void avm_empty(VECT *vect)
{
   free(vect->v);
   vect->v = calloc(PREALLOC, sizeof(void *));
   vect->alloc = PREALLOC;
   vect->elems = 0;
   vect->cur   = NULL;
}

/*---------------------------------------------------------------------------*/

void avm_free(VECT *vect)
{
   free(vect->v);
   memset(vect, 0, sizeof(VECT));
   free(vect);
}

/*---------------------------------------------------------------------------*/

int avm_vect_type(VECT *vect)
{
   return (vect->keytype << 1) | vect->dup;
}
