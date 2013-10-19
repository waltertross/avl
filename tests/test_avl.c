/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                 test_avl.c                                 |
 |                                                                            |
 |                       test suite for the AVL library                       |
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
#include <string.h>
#include <float.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "avl_test.h"
#include "avm.h"
#include "test_utils.h"

/*---------------------------------------------------------------------------*/

#define N_TESTS   100
#define N_OBJ    2000
#define AVG_DUP    10
#define N_TESTS_2 100
#define N_OBJ_2   200

/*---------------------------------------------------------------------------*/

#define AVL_FOR(tree, p) for ((p) = avl_first(tree); (p); (p) = avl_next(tree))
#define AVM_FOR(vect, p) for ((p) = avm_first(vect); (p); (p) = avm_next(vect))

#ifndef MAX
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

typedef unsigned long  ULONG;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef   signed char  SCHAR;

#define STRLEN_MAX (sizeof(long) + 3)
#define CHA_SIZE (STRLEN_MAX + 1)

typedef struct obj {
   char   ch0[CHA_SIZE];
   char   ch1[CHA_SIZE];
   char  *str;
   int    idx;
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
} OBJ;

/*---------------------------------------------------------------------------*/

int obj_i_cmp(OBJ *obj1, OBJ *obj2)
{
   if      (obj1->i > obj2->i  ) return  1;
   else if (obj1->i < obj2->i  ) return -1;
   /*
   else if (obj1->idx > obj2->idx) return  1;
   else if (obj1->idx < obj2->idx) return -1;
   */
   else return 0;
}

/*---------------------------------------------------------------------------*/

void print_obj(OBJ *obj, bool *first)
{
   if (*first) {
      *first = false;
   } else {
      printf(", ");
   }
   printf("{idx: %d, i:%d}", obj->idx, obj->i);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void print_obj_tree(TREE *tree, char *name)
{
   bool first = true;
   printf("TREE(type %d) %s:\n[", avl_tree_type(tree), name);
   avl_do_w_ctx(tree, print_obj, &first);
   printf("]\n");
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void print_obj_vect(VECT *vect, char *name)
{
   bool first = true;
   printf("VECT(type %d) %s:\n[", avm_vect_type(vect), name);
   avm_do_w_ctx(vect, print_obj, &first);
   printf("]\n");
}

/*---------------------------------------------------------------------------*/

void dump_obj(OBJ *obj, int idx, int bal)
{
   if (obj) {
      printf("%c[%2d]%c idx:%d i:%d\n", bal < 0 ? '+' : ' ', idx, bal > 0 || bal == -2 ? '+' : ' ', obj->idx, obj->i);
   } else {
      printf(" (%2d)  ---\n", idx);
   }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void dump_tree(TREE *tree, char *title_fmt, ...)
{
   va_list extra_args;
   va_start(extra_args, title_fmt);
   vprintf(title_fmt, extra_args);
   va_end(extra_args);
   avl_dump(tree, dump_obj);
}

/*---------------------------------------------------------------------------*/

typedef struct cmp {
   VECT *vect;
   OBJ  *obj;
} CMP;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

bool find_ne(OBJ *obj, CMP *cmp)
{
   if (obj != cmp->obj) {
      return true;
   }
   cmp->obj = avm_next(cmp->vect);
   return false;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void check_eq(TREE *tree, VECT *vect)
{
   OBJ *obj;
   CMP cmp;
   cmp.vect = vect;
   cmp.obj  = avm_first(cmp.vect);
   obj = avl_scan_w_ctx(tree, find_ne, &cmp);
   if (obj || cmp.obj) {
      print_obj_tree(tree, "tree at failing assert");
      print_obj_vect(vect, "vect at failing assert");
   }
   assert(obj == NULL);
   assert(cmp.obj == NULL);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void check_and_cmp(TREE *tree, VECT *vect)
{
   check_eq(tree, vect);
   avl_check_balance(tree);
}

/*---------------------------------------------------------------------------*/

void test_avl_insert(TREE *tree, VECT *vect, OBJ *obj)
{
   bool tree_ins = avl_insert(tree, obj);
   bool vect_ins = avm_insert(vect, obj);
   assert(tree_ins == vect_ins);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove(TREE *tree, VECT *vect, void *key)
{
   OBJ *tree_obj = avl_remove(tree, key);
   OBJ *vect_obj = avm_remove(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_mbr(TREE *tree, VECT *vect, void *key)
{
   OBJ *tree_obj = avl_remove_mbr(tree, key);
   OBJ *vect_obj = avm_remove_mbr(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_ptr(TREE *tree, VECT *vect, void *key)
{
   OBJ *tree_obj = avl_remove_ptr(tree, key);
   OBJ *vect_obj = avm_remove_ptr(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_chars(TREE *tree, VECT *vect, char *key)
{
   OBJ *tree_obj = avl_remove_chars(tree, key);
   OBJ *vect_obj = avm_remove_chars(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_str(TREE *tree, VECT *vect, char *key)
{
   OBJ *tree_obj = avl_remove_str(tree, key);
   OBJ *vect_obj = avm_remove_str(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_long(TREE *tree, VECT *vect, long key)
{
   OBJ *tree_obj = avl_remove_long(tree, key);
   OBJ *vect_obj = avm_remove_long(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_int(TREE *tree, VECT *vect, int key)
{
   OBJ *tree_obj = avl_remove_int(tree, key);
   OBJ *vect_obj = avm_remove_int(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_short(TREE *tree, VECT *vect, short key)
{
   OBJ *tree_obj = avl_remove_short(tree, key);
   OBJ *vect_obj = avm_remove_short(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_schar(TREE *tree, VECT *vect, SCHAR key)
{
   OBJ *tree_obj = avl_remove_schar(tree, key);
   OBJ *vect_obj = avm_remove_schar(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_ulong(TREE *tree, VECT *vect, ULONG key)
{
   OBJ *tree_obj = avl_remove_ulong(tree, key);
   OBJ *vect_obj = avm_remove_ulong(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_uint(TREE *tree, VECT *vect, UINT key)
{
   OBJ *tree_obj = avl_remove_uint(tree, key);
   OBJ *vect_obj = avm_remove_uint(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_ushort(TREE *tree, VECT *vect, USHORT key)
{
   OBJ *tree_obj = avl_remove_ushort(tree, key);
   OBJ *vect_obj = avm_remove_ushort(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_uchar(TREE *tree, VECT *vect, UCHAR key)
{
   OBJ *tree_obj = avl_remove_uchar(tree, key);
   OBJ *vect_obj = avm_remove_uchar(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_float(TREE *tree, VECT *vect, float key)
{
   OBJ *tree_obj = avl_remove_float(tree, key);
   OBJ *vect_obj = avm_remove_float(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_remove_double(TREE *tree, VECT *vect, double key)
{
   OBJ *tree_obj = avl_remove_double(tree, key);
   OBJ *vect_obj = avm_remove_double(vect, key);
   assert(tree_obj == vect_obj);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

void test_avl_locate          (TREE *tree, VECT *vect, void  *key) { assert(avl_locate          (tree, key) == avm_locate          (vect, key)); }
void test_avl_locate_ge       (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_ge       (tree, key) == avm_locate_ge       (vect, key)); }
void test_avl_locate_gt       (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_gt       (tree, key) == avm_locate_gt       (vect, key)); }
void test_avl_locate_le       (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_le       (tree, key) == avm_locate_le       (vect, key)); }
void test_avl_locate_lt       (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_lt       (tree, key) == avm_locate_lt       (vect, key)); }
void test_avl_locate_mbr      (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_mbr      (tree, key) == avm_locate_mbr      (vect, key)); }
void test_avl_locate_ge_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_ge_mbr   (tree, key) == avm_locate_ge_mbr   (vect, key)); }
void test_avl_locate_gt_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_gt_mbr   (tree, key) == avm_locate_gt_mbr   (vect, key)); }
void test_avl_locate_le_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_le_mbr   (tree, key) == avm_locate_le_mbr   (vect, key)); }
void test_avl_locate_lt_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_lt_mbr   (tree, key) == avm_locate_lt_mbr   (vect, key)); }
void test_avl_locate_ptr      (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_ptr      (tree, key) == avm_locate_ptr      (vect, key)); }
void test_avl_locate_ge_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_ge_ptr   (tree, key) == avm_locate_ge_ptr   (vect, key)); }
void test_avl_locate_gt_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_gt_ptr   (tree, key) == avm_locate_gt_ptr   (vect, key)); }
void test_avl_locate_le_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_le_ptr   (tree, key) == avm_locate_le_ptr   (vect, key)); }
void test_avl_locate_lt_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_locate_lt_ptr   (tree, key) == avm_locate_lt_ptr   (vect, key)); }
void test_avl_locate_chars    (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_chars    (tree, key) == avm_locate_chars    (vect, key)); }
void test_avl_locate_ge_chars (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_ge_chars (tree, key) == avm_locate_ge_chars (vect, key)); }
void test_avl_locate_gt_chars (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_gt_chars (tree, key) == avm_locate_gt_chars (vect, key)); }
void test_avl_locate_le_chars (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_le_chars (tree, key) == avm_locate_le_chars (vect, key)); }
void test_avl_locate_lt_chars (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_lt_chars (tree, key) == avm_locate_lt_chars (vect, key)); }
void test_avl_locate_str      (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_str      (tree, key) == avm_locate_str      (vect, key)); }
void test_avl_locate_ge_str   (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_ge_str   (tree, key) == avm_locate_ge_str   (vect, key)); }
void test_avl_locate_gt_str   (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_gt_str   (tree, key) == avm_locate_gt_str   (vect, key)); }
void test_avl_locate_le_str   (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_le_str   (tree, key) == avm_locate_le_str   (vect, key)); }
void test_avl_locate_lt_str   (TREE *tree, VECT *vect, char  *key) { assert(avl_locate_lt_str   (tree, key) == avm_locate_lt_str   (vect, key)); }
void test_avl_locate_long     (TREE *tree, VECT *vect, long   key) { assert(avl_locate_long     (tree, key) == avm_locate_long     (vect, key)); }
void test_avl_locate_ge_long  (TREE *tree, VECT *vect, long   key) { assert(avl_locate_ge_long  (tree, key) == avm_locate_ge_long  (vect, key)); }
void test_avl_locate_gt_long  (TREE *tree, VECT *vect, long   key) { assert(avl_locate_gt_long  (tree, key) == avm_locate_gt_long  (vect, key)); }
void test_avl_locate_le_long  (TREE *tree, VECT *vect, long   key) { assert(avl_locate_le_long  (tree, key) == avm_locate_le_long  (vect, key)); }
void test_avl_locate_lt_long  (TREE *tree, VECT *vect, long   key) { assert(avl_locate_lt_long  (tree, key) == avm_locate_lt_long  (vect, key)); }
void test_avl_locate_int      (TREE *tree, VECT *vect, int    key) { assert(avl_locate_int      (tree, key) == avm_locate_int      (vect, key)); }
void test_avl_locate_ge_int   (TREE *tree, VECT *vect, int    key) { assert(avl_locate_ge_int   (tree, key) == avm_locate_ge_int   (vect, key)); }
void test_avl_locate_gt_int   (TREE *tree, VECT *vect, int    key) { assert(avl_locate_gt_int   (tree, key) == avm_locate_gt_int   (vect, key)); }
void test_avl_locate_le_int   (TREE *tree, VECT *vect, int    key) { assert(avl_locate_le_int   (tree, key) == avm_locate_le_int   (vect, key)); }
void test_avl_locate_lt_int   (TREE *tree, VECT *vect, int    key) { assert(avl_locate_lt_int   (tree, key) == avm_locate_lt_int   (vect, key)); }
void test_avl_locate_short    (TREE *tree, VECT *vect, short  key) { assert(avl_locate_short    (tree, key) == avm_locate_short    (vect, key)); }
void test_avl_locate_ge_short (TREE *tree, VECT *vect, short  key) { assert(avl_locate_ge_short (tree, key) == avm_locate_ge_short (vect, key)); }
void test_avl_locate_gt_short (TREE *tree, VECT *vect, short  key) { assert(avl_locate_gt_short (tree, key) == avm_locate_gt_short (vect, key)); }
void test_avl_locate_le_short (TREE *tree, VECT *vect, short  key) { assert(avl_locate_le_short (tree, key) == avm_locate_le_short (vect, key)); }
void test_avl_locate_lt_short (TREE *tree, VECT *vect, short  key) { assert(avl_locate_lt_short (tree, key) == avm_locate_lt_short (vect, key)); }
void test_avl_locate_schar    (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_locate_schar    (tree, key) == avm_locate_schar    (vect, key)); }
void test_avl_locate_ge_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_locate_ge_schar (tree, key) == avm_locate_ge_schar (vect, key)); }
void test_avl_locate_gt_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_locate_gt_schar (tree, key) == avm_locate_gt_schar (vect, key)); }
void test_avl_locate_le_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_locate_le_schar (tree, key) == avm_locate_le_schar (vect, key)); }
void test_avl_locate_lt_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_locate_lt_schar (tree, key) == avm_locate_lt_schar (vect, key)); }
void test_avl_locate_ulong    (TREE *tree, VECT *vect, ULONG  key) { assert(avl_locate_ulong    (tree, key) == avm_locate_ulong    (vect, key)); }
void test_avl_locate_ge_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_locate_ge_ulong (tree, key) == avm_locate_ge_ulong (vect, key)); }
void test_avl_locate_gt_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_locate_gt_ulong (tree, key) == avm_locate_gt_ulong (vect, key)); }
void test_avl_locate_le_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_locate_le_ulong (tree, key) == avm_locate_le_ulong (vect, key)); }
void test_avl_locate_lt_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_locate_lt_ulong (tree, key) == avm_locate_lt_ulong (vect, key)); }
void test_avl_locate_uint     (TREE *tree, VECT *vect, UINT   key) { assert(avl_locate_uint     (tree, key) == avm_locate_uint     (vect, key)); }
void test_avl_locate_ge_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_locate_ge_uint  (tree, key) == avm_locate_ge_uint  (vect, key)); }
void test_avl_locate_gt_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_locate_gt_uint  (tree, key) == avm_locate_gt_uint  (vect, key)); }
void test_avl_locate_le_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_locate_le_uint  (tree, key) == avm_locate_le_uint  (vect, key)); }
void test_avl_locate_lt_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_locate_lt_uint  (tree, key) == avm_locate_lt_uint  (vect, key)); }
void test_avl_locate_ushort   (TREE *tree, VECT *vect, USHORT key) { assert(avl_locate_ushort   (tree, key) == avm_locate_ushort   (vect, key)); }
void test_avl_locate_ge_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_locate_ge_ushort(tree, key) == avm_locate_ge_ushort(vect, key)); }
void test_avl_locate_gt_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_locate_gt_ushort(tree, key) == avm_locate_gt_ushort(vect, key)); }
void test_avl_locate_le_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_locate_le_ushort(tree, key) == avm_locate_le_ushort(vect, key)); }
void test_avl_locate_lt_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_locate_lt_ushort(tree, key) == avm_locate_lt_ushort(vect, key)); }
void test_avl_locate_uchar    (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_locate_uchar    (tree, key) == avm_locate_uchar    (vect, key)); }
void test_avl_locate_ge_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_locate_ge_uchar (tree, key) == avm_locate_ge_uchar (vect, key)); }
void test_avl_locate_gt_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_locate_gt_uchar (tree, key) == avm_locate_gt_uchar (vect, key)); }
void test_avl_locate_le_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_locate_le_uchar (tree, key) == avm_locate_le_uchar (vect, key)); }
void test_avl_locate_lt_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_locate_lt_uchar (tree, key) == avm_locate_lt_uchar (vect, key)); }
void test_avl_locate_float    (TREE *tree, VECT *vect, float  key) { assert(avl_locate_float    (tree, key) == avm_locate_float    (vect, key)); }
void test_avl_locate_ge_float (TREE *tree, VECT *vect, float  key) { assert(avl_locate_ge_float (tree, key) == avm_locate_ge_float (vect, key)); }
void test_avl_locate_gt_float (TREE *tree, VECT *vect, float  key) { assert(avl_locate_gt_float (tree, key) == avm_locate_gt_float (vect, key)); }
void test_avl_locate_le_float (TREE *tree, VECT *vect, float  key) { assert(avl_locate_le_float (tree, key) == avm_locate_le_float (vect, key)); }
void test_avl_locate_lt_float (TREE *tree, VECT *vect, float  key) { assert(avl_locate_lt_float (tree, key) == avm_locate_lt_float (vect, key)); }
void test_avl_locate_double   (TREE *tree, VECT *vect, double key) { assert(avl_locate_double   (tree, key) == avm_locate_double   (vect, key)); }
void test_avl_locate_ge_double(TREE *tree, VECT *vect, double key) { assert(avl_locate_ge_double(tree, key) == avm_locate_ge_double(vect, key)); }
void test_avl_locate_gt_double(TREE *tree, VECT *vect, double key) { assert(avl_locate_gt_double(tree, key) == avm_locate_gt_double(vect, key)); }
void test_avl_locate_le_double(TREE *tree, VECT *vect, double key) { assert(avl_locate_le_double(tree, key) == avm_locate_le_double(vect, key)); }
void test_avl_locate_lt_double(TREE *tree, VECT *vect, double key) { assert(avl_locate_lt_double(tree, key) == avm_locate_lt_double(vect, key)); }

void test_avl_first(TREE *tree, VECT *vect) { assert(avl_first(tree) == avm_first(vect)); }
void test_avl_last (TREE *tree, VECT *vect) { assert(avl_last (tree) == avm_last (vect)); }
void test_avl_next (TREE *tree, VECT *vect) { assert(avl_next (tree) == avm_next (vect)); }
void test_avl_prev (TREE *tree, VECT *vect) { assert(avl_prev (tree) == avm_prev (vect)); }

void test_avl_start       (TREE *tree, VECT *vect, void  *key) { assert(avl_start       (tree, key) == avm_start       (vect, key)); }
void test_avl_start_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_start_mbr   (tree, key) == avm_start_mbr   (vect, key)); }
void test_avl_start_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_start_ptr   (tree, key) == avm_start_ptr   (vect, key)); }
void test_avl_start_chars (TREE *tree, VECT *vect, void  *key) { assert(avl_start_chars (tree, key) == avm_start_chars (vect, key)); }
void test_avl_start_str   (TREE *tree, VECT *vect, void  *key) { assert(avl_start_str   (tree, key) == avm_start_str   (vect, key)); }
void test_avl_start_long  (TREE *tree, VECT *vect, long   key) { assert(avl_start_long  (tree, key) == avm_start_long  (vect, key)); }
void test_avl_start_int   (TREE *tree, VECT *vect, int    key) { assert(avl_start_int   (tree, key) == avm_start_int   (vect, key)); }
void test_avl_start_short (TREE *tree, VECT *vect, short  key) { assert(avl_start_short (tree, key) == avm_start_short (vect, key)); }
void test_avl_start_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_start_schar (tree, key) == avm_start_schar (vect, key)); }
void test_avl_start_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_start_ulong (tree, key) == avm_start_ulong (vect, key)); }
void test_avl_start_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_start_uint  (tree, key) == avm_start_uint  (vect, key)); }
void test_avl_start_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_start_ushort(tree, key) == avm_start_ushort(vect, key)); }
void test_avl_start_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_start_uchar (tree, key) == avm_start_uchar (vect, key)); }
void test_avl_start_float (TREE *tree, VECT *vect, float  key) { assert(avl_start_float (tree, key) == avm_start_float (vect, key)); }
void test_avl_start_double(TREE *tree, VECT *vect, double key) { assert(avl_start_double(tree, key) == avm_start_double(vect, key)); }
void test_avl_rev_start       (TREE *tree, VECT *vect, void  *key) { assert(avl_rev_start       (tree, key) == avm_rev_start       (vect, key)); }
void test_avl_rev_start_mbr   (TREE *tree, VECT *vect, void  *key) { assert(avl_rev_start_mbr   (tree, key) == avm_rev_start_mbr   (vect, key)); }
void test_avl_rev_start_ptr   (TREE *tree, VECT *vect, void  *key) { assert(avl_rev_start_ptr   (tree, key) == avm_rev_start_ptr   (vect, key)); }
void test_avl_rev_start_chars (TREE *tree, VECT *vect, void  *key) { assert(avl_rev_start_chars (tree, key) == avm_rev_start_chars (vect, key)); }
void test_avl_rev_start_str   (TREE *tree, VECT *vect, void  *key) { assert(avl_rev_start_str   (tree, key) == avm_rev_start_str   (vect, key)); }
void test_avl_rev_start_long  (TREE *tree, VECT *vect, long   key) { assert(avl_rev_start_long  (tree, key) == avm_rev_start_long  (vect, key)); }
void test_avl_rev_start_int   (TREE *tree, VECT *vect, int    key) { assert(avl_rev_start_int   (tree, key) == avm_rev_start_int   (vect, key)); }
void test_avl_rev_start_short (TREE *tree, VECT *vect, short  key) { assert(avl_rev_start_short (tree, key) == avm_rev_start_short (vect, key)); }
void test_avl_rev_start_schar (TREE *tree, VECT *vect, SCHAR  key) { assert(avl_rev_start_schar (tree, key) == avm_rev_start_schar (vect, key)); }
void test_avl_rev_start_ulong (TREE *tree, VECT *vect, ULONG  key) { assert(avl_rev_start_ulong (tree, key) == avm_rev_start_ulong (vect, key)); }
void test_avl_rev_start_uint  (TREE *tree, VECT *vect, UINT   key) { assert(avl_rev_start_uint  (tree, key) == avm_rev_start_uint  (vect, key)); }
void test_avl_rev_start_ushort(TREE *tree, VECT *vect, USHORT key) { assert(avl_rev_start_ushort(tree, key) == avm_rev_start_ushort(vect, key)); }
void test_avl_rev_start_uchar (TREE *tree, VECT *vect, UCHAR  key) { assert(avl_rev_start_uchar (tree, key) == avm_rev_start_uchar (vect, key)); }
void test_avl_rev_start_float (TREE *tree, VECT *vect, float  key) { assert(avl_rev_start_float (tree, key) == avm_rev_start_float (vect, key)); }
void test_avl_rev_start_double(TREE *tree, VECT *vect, double key) { assert(avl_rev_start_double(tree, key) == avm_rev_start_double(vect, key)); }

/*---------------------------------------------------------------------------*/

void test_avl_locate_first(TREE *tree, VECT *vect)
{  assert(avl_locate_first(tree) == avm_locate_first(vect)); }

void test_avl_locate_last(TREE *tree, VECT *vect)
{  assert(avl_locate_last(tree) == avm_locate_last(vect)); }

/*---------------------------------------------------------------------------*/

void test_avl_scan(TREE *tree, VECT *vect, bool (*callback)())
{  assert(avl_scan(tree, callback) == avm_scan(vect, callback)); }

void test_avl_rev_scan(TREE *tree, VECT *vect, bool (*callback)())
{  assert(avl_rev_scan(tree, callback) == avm_rev_scan(vect, callback)); }

void test_avl_scan_w_ctx(TREE *tree, VECT *vect, bool (*callback)(), void *context)
{  assert(avl_scan_w_ctx(tree, callback, context) == avm_scan_w_ctx(vect, callback, context)); }

void test_avl_rev_scan_w_ctx(TREE *tree, VECT *vect, bool (*callback)(), void *context)
{  assert(avl_rev_scan_w_ctx(tree, callback, context) == avm_rev_scan_w_ctx(vect, callback, context)); }

void test_avl_do(TREE *tree, VECT *vect, void (*callback)())
{  avl_do(tree, callback); avm_do(vect, callback); }

void test_avl_rev_do(TREE *tree, VECT *vect, void (*callback)())
{  avl_rev_do(tree, callback); avm_rev_do(vect, callback); }

void test_avl_do_w_ctx(TREE *tree, VECT *vect, void (*callback)(), void *context)
{  avl_do_w_ctx(tree, callback, context); avm_do_w_ctx(vect, callback, context); }

void test_avl_rev_do_w_ctx(TREE *tree, VECT *vect, void (*callback)(), void *context)
{  avl_rev_do_w_ctx(tree, callback, context); avm_rev_do_w_ctx(vect, callback, context); }

/*---------------------------------------------------------------------------*/

void test_avl_empty(TREE *tree, VECT *vect)
{
   avl_empty(tree);
   avm_empty(vect);
   check_and_cmp(tree, vect);
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
   #define DUP_MAX (N_OBJ / AVG_DUP - 1)

   OBJ *obj_v = calloc(MAX(N_OBJ, N_OBJ_2), sizeof(OBJ));
   int i, j, r, t;
   void *rig;

   for (i = 0; i < N_OBJ; i++) {
      obj_v[i].idx = i;
   }

   TREE *obj_tree_nodup = avl_tree_nodup       (obj_i_cmp);
   TREE *mbr_tree_nodup = avl_tree_nodup_mbr   (OBJ, ch1, strcmp);
   TREE *ptr_tree_nodup = avl_tree_nodup_ptr   (OBJ, str, strcmp);
   TREE *ch0_tree_nodup = avl_tree_nodup_chars (OBJ, ch0);
   TREE *ch1_tree_nodup = avl_tree_nodup_chars (OBJ, ch1);
   TREE *str_tree_nodup = avl_tree_nodup_str   (OBJ, str);
   TREE *lng_tree_nodup = avl_tree_nodup_long  (OBJ, l);
   TREE *int_tree_nodup = avl_tree_nodup_int   (OBJ, i);
   TREE *sht_tree_nodup = avl_tree_nodup_short (OBJ, s);
   TREE *sch_tree_nodup = avl_tree_nodup_schar (OBJ, sc);
   TREE *uln_tree_nodup = avl_tree_nodup_ulong (OBJ, ul);
   TREE *uin_tree_nodup = avl_tree_nodup_uint  (OBJ, ui);
   TREE *ush_tree_nodup = avl_tree_nodup_ushort(OBJ, us);
   TREE *uch_tree_nodup = avl_tree_nodup_uchar (OBJ, uc);
   TREE *flt_tree_nodup = avl_tree_nodup_float (OBJ, f);
   TREE *dbl_tree_nodup = avl_tree_nodup_double(OBJ, d);
   TREE *obj_tree_dup   = avl_tree_dup         (obj_i_cmp);
   TREE *mbr_tree_dup   = avl_tree_dup_mbr     (OBJ, ch1, strcmp);
   TREE *ptr_tree_dup   = avl_tree_dup_ptr     (OBJ, str, strcmp);
   TREE *ch0_tree_dup   = avl_tree_dup_chars   (OBJ, ch0);
   TREE *ch1_tree_dup   = avl_tree_dup_chars   (OBJ, ch1);
   TREE *str_tree_dup   = avl_tree_dup_str     (OBJ, str);
   TREE *lng_tree_dup   = avl_tree_dup_long    (OBJ, l);
   TREE *int_tree_dup   = avl_tree_dup_int     (OBJ, i);
   TREE *sht_tree_dup   = avl_tree_dup_short   (OBJ, s);
   TREE *sch_tree_dup   = avl_tree_dup_schar   (OBJ, sc);
   TREE *uln_tree_dup   = avl_tree_dup_ulong   (OBJ, ul);
   TREE *uin_tree_dup   = avl_tree_dup_uint    (OBJ, ui);
   TREE *ush_tree_dup   = avl_tree_dup_ushort  (OBJ, us);
   TREE *uch_tree_dup   = avl_tree_dup_uchar   (OBJ, uc);
   TREE *flt_tree_dup   = avl_tree_dup_float   (OBJ, f);
   TREE *dbl_tree_dup   = avl_tree_dup_double  (OBJ, d);

   VECT *obj_vect_nodup = avm_vect_nodup       (obj_i_cmp);
   VECT *mbr_vect_nodup = avm_vect_nodup_mbr   (OBJ, ch1, strcmp);
   VECT *ptr_vect_nodup = avm_vect_nodup_ptr   (OBJ, str, strcmp);
   VECT *ch0_vect_nodup = avm_vect_nodup_chars (OBJ, ch0);
   VECT *ch1_vect_nodup = avm_vect_nodup_chars (OBJ, ch1);
   VECT *str_vect_nodup = avm_vect_nodup_str   (OBJ, str);
   VECT *lng_vect_nodup = avm_vect_nodup_long  (OBJ, l);
   VECT *int_vect_nodup = avm_vect_nodup_int   (OBJ, i);
   VECT *sht_vect_nodup = avm_vect_nodup_short (OBJ, s);
   VECT *sch_vect_nodup = avm_vect_nodup_schar (OBJ, sc);
   VECT *uln_vect_nodup = avm_vect_nodup_ulong (OBJ, ul);
   VECT *uin_vect_nodup = avm_vect_nodup_uint  (OBJ, ui);
   VECT *ush_vect_nodup = avm_vect_nodup_ushort(OBJ, us);
   VECT *uch_vect_nodup = avm_vect_nodup_uchar (OBJ, uc);
   VECT *flt_vect_nodup = avm_vect_nodup_float (OBJ, f);
   VECT *dbl_vect_nodup = avm_vect_nodup_double(OBJ, d);
   VECT *obj_vect_dup   = avm_vect_dup         (obj_i_cmp);
   VECT *mbr_vect_dup   = avm_vect_dup_mbr     (OBJ, ch1, strcmp);
   VECT *ptr_vect_dup   = avm_vect_dup_ptr     (OBJ, str, strcmp);
   VECT *ch0_vect_dup   = avm_vect_dup_chars   (OBJ, ch0);
   VECT *ch1_vect_dup   = avm_vect_dup_chars   (OBJ, ch1);
   VECT *str_vect_dup   = avm_vect_dup_str     (OBJ, str);
   VECT *lng_vect_dup   = avm_vect_dup_long    (OBJ, l);
   VECT *int_vect_dup   = avm_vect_dup_int     (OBJ, i);
   VECT *sht_vect_dup   = avm_vect_dup_short   (OBJ, s);
   VECT *sch_vect_dup   = avm_vect_dup_schar   (OBJ, sc);
   VECT *uln_vect_dup   = avm_vect_dup_ulong   (OBJ, ul);
   VECT *uin_vect_dup   = avm_vect_dup_uint    (OBJ, ui);
   VECT *ush_vect_dup   = avm_vect_dup_ushort  (OBJ, us);
   VECT *uch_vect_dup   = avm_vect_dup_uchar   (OBJ, uc);
   VECT *flt_vect_dup   = avm_vect_dup_float   (OBJ, f);
   VECT *dbl_vect_dup   = avm_vect_dup_double  (OBJ, d);

   for (t = 0; t < N_TESTS; t++) {
      printf("%d ", t); fflush(stdout);
      for (i = 0; i < N_OBJ; i++) {
         obj_v[i].i   = random_int(-DUP_MAX / 2, -DUP_MAX / 2 + DUP_MAX);
         obj_v[i].f   = (float) obj_v[i].i;
         obj_v[i].d   = (double)obj_v[i].i;
               random_string( obj_v[i].ch0, random_int(0, STRLEN_MAX), "\001\177\200\377");
               random_string( obj_v[i].ch1, random_int(0, STRLEN_MAX), "\001\177\200\377");
         alloc_random_string(&obj_v[i].str, random_int(0, STRLEN_MAX), "\001\177\200\377");
      }
      for (i = 0; i < N_OBJ; i++) {
         test_avl_insert(obj_tree_nodup, obj_vect_nodup, &obj_v[i]);
         test_avl_insert(mbr_tree_nodup, mbr_vect_nodup, &obj_v[i]);
         test_avl_insert(ptr_tree_nodup, ptr_vect_nodup, &obj_v[i]);
         test_avl_insert(ch0_tree_nodup, ch0_vect_nodup, &obj_v[i]);
         test_avl_insert(ch1_tree_nodup, ch1_vect_nodup, &obj_v[i]);
         test_avl_insert(str_tree_nodup, str_vect_nodup, &obj_v[i]);
         test_avl_insert(int_tree_nodup, int_vect_nodup, &obj_v[i]);
         test_avl_insert(flt_tree_nodup, flt_vect_nodup, &obj_v[i]);
         test_avl_insert(dbl_tree_nodup, dbl_vect_nodup, &obj_v[i]);
         test_avl_insert(obj_tree_dup,   obj_vect_dup,   &obj_v[i]);
         test_avl_insert(mbr_tree_dup,   mbr_vect_dup,   &obj_v[i]);
         test_avl_insert(ptr_tree_dup,   ptr_vect_dup,   &obj_v[i]);
         test_avl_insert(ch0_tree_dup,   ch0_vect_dup,   &obj_v[i]);
         test_avl_insert(ch1_tree_dup,   ch1_vect_dup,   &obj_v[i]);
         test_avl_insert(str_tree_dup,   str_vect_dup,   &obj_v[i]);
         test_avl_insert(int_tree_dup,   int_vect_dup,   &obj_v[i]);
         test_avl_insert(flt_tree_dup,   flt_vect_dup,   &obj_v[i]);
         test_avl_insert(dbl_tree_dup,   dbl_vect_dup,   &obj_v[i]);
      }
      assert(avl_nodes(obj_tree_nodup) == avm_nodes(obj_vect_nodup));
      assert(avl_nodes(mbr_tree_nodup) == avm_nodes(mbr_vect_nodup));
      assert(avl_nodes(ptr_tree_nodup) == avm_nodes(ptr_vect_nodup));
      assert(avl_nodes(ch0_tree_nodup) == avm_nodes(ch0_vect_nodup));
      assert(avl_nodes(ch1_tree_nodup) == avm_nodes(ch1_vect_nodup));
      assert(avl_nodes(str_tree_nodup) == avm_nodes(str_vect_nodup));
      assert(avl_nodes(int_tree_nodup) == avm_nodes(int_vect_nodup));
      assert(avl_nodes(flt_tree_nodup) == avm_nodes(flt_vect_nodup));
      assert(avl_nodes(dbl_tree_nodup) == avm_nodes(dbl_vect_nodup));
      assert(avl_nodes(obj_tree_dup)   == avm_nodes(obj_vect_dup));
      assert(avl_nodes(mbr_tree_dup)   == avm_nodes(mbr_vect_dup));
      assert(avl_nodes(ptr_tree_dup)   == avm_nodes(ptr_vect_dup));
      assert(avl_nodes(ch0_tree_dup)   == avm_nodes(ch0_vect_dup));
      assert(avl_nodes(ch1_tree_dup)   == avm_nodes(ch1_vect_dup));
      assert(avl_nodes(str_tree_dup)   == avm_nodes(str_vect_dup));
      assert(avl_nodes(int_tree_dup)   == avm_nodes(int_vect_dup));
      assert(avl_nodes(flt_tree_dup)   == avm_nodes(flt_vect_dup));
      assert(avl_nodes(dbl_tree_dup)   == avm_nodes(dbl_vect_dup));

      rig = new_random_index_generator(N_OBJ);
      for (i = 0; i < N_OBJ; i++) {
         int r = random_index(rig);

         test_avl_locate(obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
         test_avl_remove(obj_tree_nodup, obj_vect_nodup, &obj_v[r]);

         test_avl_locate(mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
         test_avl_remove(mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);

         test_avl_locate(ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
         test_avl_remove(ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);

         test_avl_locate(ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
         test_avl_remove(ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);

         test_avl_locate(ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
         test_avl_remove(ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);

         test_avl_locate(str_tree_nodup, str_vect_nodup, obj_v[r].str);
         test_avl_remove(str_tree_nodup, str_vect_nodup, obj_v[r].str);

         test_avl_locate_long(int_tree_nodup, int_vect_nodup, obj_v[r].i);
         test_avl_remove_long(int_tree_nodup, int_vect_nodup, obj_v[r].i);

         test_avl_locate_float(flt_tree_nodup, flt_vect_nodup, obj_v[r].i);
         test_avl_remove_float(flt_tree_nodup, flt_vect_nodup, obj_v[r].i);

         test_avl_locate_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].i);
         test_avl_remove_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].i);

         test_avl_locate(obj_tree_dup, obj_vect_dup, &obj_v[r]);
         test_avl_remove(obj_tree_dup, obj_vect_dup, &obj_v[r]);

         test_avl_locate(mbr_tree_dup, mbr_vect_dup, obj_v[r].ch1);
         test_avl_remove(mbr_tree_dup, mbr_vect_dup, obj_v[r].ch1);

         test_avl_locate(ptr_tree_dup, ptr_vect_dup, obj_v[r].str);
         test_avl_remove(ptr_tree_dup, ptr_vect_dup, obj_v[r].str);

         test_avl_locate(ch0_tree_dup, ch0_vect_dup, obj_v[r].ch0);
         test_avl_remove(ch0_tree_dup, ch0_vect_dup, obj_v[r].ch0);

         test_avl_locate(ch1_tree_dup, ch1_vect_dup, obj_v[r].ch1);
         test_avl_remove(ch1_tree_dup, ch1_vect_dup, obj_v[r].ch1);

         test_avl_locate(str_tree_dup, str_vect_dup, obj_v[r].str);
         test_avl_remove(str_tree_dup, str_vect_dup, obj_v[r].str);

         test_avl_locate_long(int_tree_dup, int_vect_dup, obj_v[r].i);
         test_avl_remove_long(int_tree_dup, int_vect_dup, obj_v[r].i);

         test_avl_locate_float(flt_tree_dup, flt_vect_dup, obj_v[r].i);
         test_avl_remove_float(flt_tree_dup, flt_vect_dup, obj_v[r].i);

         test_avl_locate_double(dbl_tree_dup, dbl_vect_dup, obj_v[r].i);
         test_avl_remove_double(dbl_tree_dup, dbl_vect_dup, obj_v[r].i);
      }
      free_random_index_generator(rig);

      assert(avl_nodes(obj_tree_nodup) == 0); assert(avm_nodes(obj_vect_nodup) == 0);
      assert(avl_nodes(mbr_tree_nodup) == 0); assert(avm_nodes(mbr_vect_nodup) == 0);
      assert(avl_nodes(ptr_tree_nodup) == 0); assert(avm_nodes(ptr_vect_nodup) == 0);
      assert(avl_nodes(ch0_tree_nodup) == 0); assert(avm_nodes(ch0_vect_nodup) == 0);
      assert(avl_nodes(ch1_tree_nodup) == 0); assert(avm_nodes(ch1_vect_nodup) == 0);
      assert(avl_nodes(str_tree_nodup) == 0); assert(avm_nodes(str_vect_nodup) == 0);
      assert(avl_nodes(int_tree_nodup) == 0); assert(avm_nodes(int_vect_nodup) == 0);
      assert(avl_nodes(flt_tree_nodup) == 0); assert(avm_nodes(flt_vect_nodup) == 0);
      assert(avl_nodes(dbl_tree_nodup) == 0); assert(avm_nodes(dbl_vect_nodup) == 0);
      assert(avl_nodes(obj_tree_dup  ) == 0); assert(avm_nodes(obj_vect_dup  ) == 0);
      assert(avl_nodes(mbr_tree_dup  ) == 0); assert(avm_nodes(mbr_vect_dup  ) == 0);
      assert(avl_nodes(ptr_tree_dup  ) == 0); assert(avm_nodes(ptr_vect_dup  ) == 0);
      assert(avl_nodes(ch0_tree_dup  ) == 0); assert(avm_nodes(ch0_vect_dup  ) == 0);
      assert(avl_nodes(ch1_tree_dup  ) == 0); assert(avm_nodes(ch1_vect_dup  ) == 0);
      assert(avl_nodes(str_tree_dup  ) == 0); assert(avm_nodes(str_vect_dup  ) == 0);
      assert(avl_nodes(int_tree_dup  ) == 0); assert(avm_nodes(int_vect_dup  ) == 0);
      assert(avl_nodes(flt_tree_dup  ) == 0); assert(avm_nodes(flt_vect_dup  ) == 0);
      assert(avl_nodes(dbl_tree_dup  ) == 0); assert(avm_nodes(dbl_vect_dup  ) == 0);
   }
   test_avl_empty(obj_tree_nodup, obj_vect_nodup);
   test_avl_empty(mbr_tree_nodup, mbr_vect_nodup);
   test_avl_empty(ptr_tree_nodup, ptr_vect_nodup);
   test_avl_empty(ch0_tree_nodup, ch0_vect_nodup);
   test_avl_empty(ch1_tree_nodup, ch1_vect_nodup);
   test_avl_empty(str_tree_nodup, str_vect_nodup);
   test_avl_empty(int_tree_nodup, int_vect_nodup);
   test_avl_empty(flt_tree_nodup, flt_vect_nodup);
   test_avl_empty(dbl_tree_nodup, dbl_vect_nodup);
   test_avl_empty(obj_tree_dup  , obj_vect_dup  );
   test_avl_empty(mbr_tree_dup  , mbr_vect_dup  );
   test_avl_empty(ptr_tree_dup  , ptr_vect_dup  );
   test_avl_empty(ch0_tree_dup  , ch0_vect_dup  );
   test_avl_empty(ch1_tree_dup  , ch1_vect_dup  );
   test_avl_empty(str_tree_dup  , str_vect_dup  );
   test_avl_empty(int_tree_dup  , int_vect_dup  );
   test_avl_empty(flt_tree_dup  , flt_vect_dup  );
   test_avl_empty(dbl_tree_dup  , dbl_vect_dup  );

   for (i = 0;  i < N_OBJ; i++) {
      free(obj_v[i].str);
      obj_v[i].str = NULL;
   }
   printf("\nok\n"); fflush(stdout);

   i = 0;
   obj_v[i++].l = 0;
   obj_v[i++].l = 1;
   obj_v[i++].l = -1;
   obj_v[i++].l = LONG_MIN;
   obj_v[i++].l = LONG_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].l = random_long(LONG_MIN, LONG_MAX);
   }
   i = 0;
   obj_v[i++].ul = 0;
   obj_v[i++].ul = 1;
   obj_v[i++].ul = LONG_MAX;
   obj_v[i++].ul = (ULONG)LONG_MAX + 1;
   obj_v[i++].ul = ULONG_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].ul = random_ulong(0, ULONG_MAX);
   }
   i = 0;
   obj_v[i++].i = 0;
   obj_v[i++].i = 1;
   obj_v[i++].i = -1;
   obj_v[i++].i = INT_MIN;
   obj_v[i++].i = INT_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].i = random_int(INT_MIN, INT_MAX);
   }
   i = 0;
   obj_v[i++].ui = 0;
   obj_v[i++].ui = 1;
   obj_v[i++].ui = INT_MAX;
   obj_v[i++].ui = (UINT)INT_MAX + 1;
   obj_v[i++].ui = UINT_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].ui = random_uint(0, UINT_MAX);
   }
   i = 0;
   obj_v[i++].s = 0;
   obj_v[i++].s = 1;
   obj_v[i++].s = -1;
   obj_v[i++].s = SHRT_MIN;
   obj_v[i++].s = SHRT_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].s = (short)random_int(SHRT_MIN, SHRT_MAX);
   }
   i = 0;
   obj_v[i++].us = 0;
   obj_v[i++].us = 1;
   obj_v[i++].us = SHRT_MAX;
   obj_v[i++].us = (USHORT)SHRT_MAX + 1;
   obj_v[i++].us = USHRT_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].us = (USHORT)random_uint(0, USHRT_MAX);
   }
   i = 0;
   obj_v[i++].sc = 0;
   obj_v[i++].sc = 1;
   obj_v[i++].sc = -1;
   obj_v[i++].sc = SCHAR_MIN;
   obj_v[i++].sc = SCHAR_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].sc = (SCHAR)random_int(SCHAR_MIN, SCHAR_MAX);
   }
   i = 0;
   obj_v[i++].uc = 0;
   obj_v[i++].uc = 1;
   obj_v[i++].uc = SCHAR_MAX;
   obj_v[i++].uc = (UCHAR)SCHAR_MAX + 1;
   obj_v[i++].uc = UCHAR_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].uc = (UCHAR)random_uint(0, UCHAR_MAX);
   }
   i = 0;
   obj_v[i++].f = 0.0;
   obj_v[i++].f = 1.0;
   obj_v[i++].f = 1.0 - FLT_EPSILON;
   obj_v[i++].f = 1.0 + FLT_EPSILON;
   obj_v[i++].f = -1.0;
   obj_v[i++].f = -1.0 - FLT_EPSILON;
   obj_v[i++].f = -1.0 + FLT_EPSILON;
   obj_v[i++].f = FLT_EPSILON;
   obj_v[i++].f = -FLT_EPSILON;
   obj_v[i++].f = FLT_MIN;
   obj_v[i++].f = FLT_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].f = random_float();
   }
   i = 0;
   obj_v[i++].d = 0.0;
   obj_v[i++].d = 1.0;
   obj_v[i++].d = 1.0 - DBL_EPSILON;
   obj_v[i++].d = 1.0 + DBL_EPSILON;
   obj_v[i++].d = -1.0;
   obj_v[i++].d = -1.0 - DBL_EPSILON;
   obj_v[i++].d = -1.0 + DBL_EPSILON;
   obj_v[i++].d = DBL_EPSILON;
   obj_v[i++].d = -DBL_EPSILON;
   obj_v[i++].d = DBL_MIN;
   obj_v[i++].d = DBL_MAX;
   for ( ;  i < N_OBJ_2; i++) {
      obj_v[i++].d = random_double();
   }
   i = 0;
   memset(obj_v[i  ].ch0, -1, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\001");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\001");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\001");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\377");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\377\001");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\377\377");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\377\377\001");
   memset(obj_v[i  ].ch0, 0, sizeof(obj_v[i].ch0));
   strcpy(obj_v[i++].ch0, "\377\377\377\377\377\377");
   for ( ;  i < N_OBJ_2; i++) {
      random_string(obj_v[i].ch0, random_int(0, STRLEN_MAX), "\001\177\200\377");
   }
   i = 0;
   memset(obj_v[i  ].ch1, -1, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\001");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\001");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\001");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\377");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\377\001");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\377\377");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\377\377\001");
   memset(obj_v[i  ].ch1, 0, sizeof(obj_v[i].ch1));
   strcpy(obj_v[i++].ch1, "\377\377\377\377\377\377");
   for ( ;  i < N_OBJ_2; i++) {
      random_string(obj_v[i].ch1, random_int(0, STRLEN_MAX), "\001\177\200\377");
   }
   for (i = 0;  i < N_OBJ_2; i++) {
      obj_v[i].str = NULL;
   }
   i = 0;
   obj_v[i++].str = "";
   obj_v[i++].str = "\001";
   obj_v[i++].str = "\377";
   obj_v[i++].str = "\377\001";
   obj_v[i++].str = "\377\377";
   obj_v[i++].str = "\377\377\001";
   obj_v[i++].str = "\377\377\377";
   obj_v[i++].str = "\377\377\377\001";
   obj_v[i++].str = "\377\377\377\377";
   obj_v[i++].str = "\377\377\377\377\001";
   obj_v[i++].str = "\377\377\377\377\377";
   obj_v[i++].str = "\377\377\377\377\377\001";
   obj_v[i++].str = "\377\377\377\377\377\377";
   for ( ;  i < N_OBJ_2; i++) {
      alloc_random_string(&obj_v[i].str, random_int(0, STRLEN_MAX), "\001\177\200\377");
   }

   int r_v[N_OBJ_2];
   for (t = 0; t < N_TESTS_2; t++) {
      printf("%d ", t); fflush(stdout);
      if (t == 0) {
         for (i = 0; i < N_OBJ_2; i++) {
            r_v[i] = i;
         }
      } else if (t == 1) {
         for (i = 0; i < N_OBJ_2; i++) {
            r_v[i] = N_OBJ_2 - 1 - i;
         }
      } else {
         rig = new_random_index_generator(N_OBJ_2);
         for (i = 0; i < N_OBJ_2; i++) {
            r_v[i] = random_index(rig);
         }
         free_random_index_generator(rig);
      }
      for (i = 0; i < N_OBJ_2; i++) {
         test_avl_insert(obj_tree_nodup, obj_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(mbr_tree_nodup, mbr_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(ptr_tree_nodup, ptr_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(ch0_tree_nodup, ch0_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(ch1_tree_nodup, ch1_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(str_tree_nodup, str_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(lng_tree_nodup, lng_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(int_tree_nodup, int_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(sht_tree_nodup, sht_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(sch_tree_nodup, sch_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(uln_tree_nodup, uln_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(uin_tree_nodup, uin_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(ush_tree_nodup, ush_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(uch_tree_nodup, uch_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(flt_tree_nodup, flt_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(dbl_tree_nodup, dbl_vect_nodup, &obj_v[r_v[i]]);
         test_avl_insert(obj_tree_dup  , obj_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(mbr_tree_dup  , mbr_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(ptr_tree_dup  , ptr_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(ch0_tree_dup  , ch0_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(ch1_tree_dup  , ch1_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(str_tree_dup  , str_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(lng_tree_dup  , lng_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(int_tree_dup  , int_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(sht_tree_dup  , sht_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(sch_tree_dup  , sch_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(uln_tree_dup  , uln_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(uin_tree_dup  , uin_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(ush_tree_dup  , ush_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(uch_tree_dup  , uch_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(flt_tree_dup  , flt_vect_dup  , &obj_v[r_v[i]]);
         test_avl_insert(dbl_tree_dup  , dbl_vect_dup  , &obj_v[r_v[i]]);

         test_avl_locate_first(obj_tree_nodup, obj_vect_nodup);
         test_avl_locate_first(mbr_tree_nodup, mbr_vect_nodup);
         test_avl_locate_first(ptr_tree_nodup, ptr_vect_nodup);
         test_avl_locate_first(ch0_tree_nodup, ch0_vect_nodup);
         test_avl_locate_first(ch1_tree_nodup, ch1_vect_nodup);
         test_avl_locate_first(str_tree_nodup, str_vect_nodup);
         test_avl_locate_first(lng_tree_nodup, lng_vect_nodup);
         test_avl_locate_first(int_tree_nodup, int_vect_nodup);
         test_avl_locate_first(sht_tree_nodup, sht_vect_nodup);
         test_avl_locate_first(sch_tree_nodup, sch_vect_nodup);
         test_avl_locate_first(uln_tree_nodup, uln_vect_nodup);
         test_avl_locate_first(uin_tree_nodup, uin_vect_nodup);
         test_avl_locate_first(ush_tree_nodup, ush_vect_nodup);
         test_avl_locate_first(uch_tree_nodup, uch_vect_nodup);
         test_avl_locate_first(flt_tree_nodup, flt_vect_nodup);
         test_avl_locate_first(dbl_tree_nodup, dbl_vect_nodup);
         test_avl_locate_first(obj_tree_dup  , obj_vect_dup  );
         test_avl_locate_first(mbr_tree_dup  , mbr_vect_dup  );
         test_avl_locate_first(ptr_tree_dup  , ptr_vect_dup  );
         test_avl_locate_first(ch0_tree_dup  , ch0_vect_dup  );
         test_avl_locate_first(ch1_tree_dup  , ch1_vect_dup  );
         test_avl_locate_first(str_tree_dup  , str_vect_dup  );
         test_avl_locate_first(lng_tree_dup  , lng_vect_dup  );
         test_avl_locate_first(int_tree_dup  , int_vect_dup  );
         test_avl_locate_first(sht_tree_dup  , sht_vect_dup  );
         test_avl_locate_first(sch_tree_dup  , sch_vect_dup  );
         test_avl_locate_first(uln_tree_dup  , uln_vect_dup  );
         test_avl_locate_first(uin_tree_dup  , uin_vect_dup  );
         test_avl_locate_first(ush_tree_dup  , ush_vect_dup  );
         test_avl_locate_first(uch_tree_dup  , uch_vect_dup  );
         test_avl_locate_first(flt_tree_dup  , flt_vect_dup  );
         test_avl_locate_first(dbl_tree_dup  , dbl_vect_dup  );

         test_avl_locate_last(obj_tree_nodup, obj_vect_nodup);
         test_avl_locate_last(mbr_tree_nodup, mbr_vect_nodup);
         test_avl_locate_last(ptr_tree_nodup, ptr_vect_nodup);
         test_avl_locate_last(ch0_tree_nodup, ch0_vect_nodup);
         test_avl_locate_last(ch1_tree_nodup, ch1_vect_nodup);
         test_avl_locate_last(str_tree_nodup, str_vect_nodup);
         test_avl_locate_last(lng_tree_nodup, lng_vect_nodup);
         test_avl_locate_last(int_tree_nodup, int_vect_nodup);
         test_avl_locate_last(sht_tree_nodup, sht_vect_nodup);
         test_avl_locate_last(sch_tree_nodup, sch_vect_nodup);
         test_avl_locate_last(uln_tree_nodup, uln_vect_nodup);
         test_avl_locate_last(uin_tree_nodup, uin_vect_nodup);
         test_avl_locate_last(ush_tree_nodup, ush_vect_nodup);
         test_avl_locate_last(uch_tree_nodup, uch_vect_nodup);
         test_avl_locate_last(flt_tree_nodup, flt_vect_nodup);
         test_avl_locate_last(dbl_tree_nodup, dbl_vect_nodup);
         test_avl_locate_last(obj_tree_dup  , obj_vect_dup  );
         test_avl_locate_last(mbr_tree_dup  , mbr_vect_dup  );
         test_avl_locate_last(ptr_tree_dup  , ptr_vect_dup  );
         test_avl_locate_last(ch0_tree_dup  , ch0_vect_dup  );
         test_avl_locate_last(ch1_tree_dup  , ch1_vect_dup  );
         test_avl_locate_last(str_tree_dup  , str_vect_dup  );
         test_avl_locate_last(lng_tree_dup  , lng_vect_dup  );
         test_avl_locate_last(int_tree_dup  , int_vect_dup  );
         test_avl_locate_last(sht_tree_dup  , sht_vect_dup  );
         test_avl_locate_last(sch_tree_dup  , sch_vect_dup  );
         test_avl_locate_last(uln_tree_dup  , uln_vect_dup  );
         test_avl_locate_last(uin_tree_dup  , uin_vect_dup  );
         test_avl_locate_last(ush_tree_dup  , ush_vect_dup  );
         test_avl_locate_last(uch_tree_dup  , uch_vect_dup  );
         test_avl_locate_last(flt_tree_dup  , flt_vect_dup  );
         test_avl_locate_last(dbl_tree_dup  , dbl_vect_dup  );

         for (j = 0; j < N_OBJ_2; j++) {
            r = r_v[j];
            test_avl_locate       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
            test_avl_locate_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
            test_avl_locate_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
            test_avl_locate_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
            test_avl_locate_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
            test_avl_locate_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str);
            test_avl_locate_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);
            test_avl_locate_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);
            test_avl_locate_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);
            test_avl_locate_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);
            test_avl_locate_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);
            test_avl_locate_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);
            test_avl_locate_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);
            test_avl_locate_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);
            test_avl_locate_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);
            test_avl_locate_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);
            test_avl_locate       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);
            test_avl_locate_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1);
            test_avl_locate_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str);
            test_avl_locate_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0);
            test_avl_locate_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1);
            test_avl_locate_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str);
            test_avl_locate_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);
            test_avl_locate_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);
            test_avl_locate_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);
            test_avl_locate_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);
            test_avl_locate_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);
            test_avl_locate_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);
            test_avl_locate_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);
            test_avl_locate_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);
            test_avl_locate_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);
            test_avl_locate_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);

            test_avl_locate_ge       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
            test_avl_locate_ge_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
            test_avl_locate_ge_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
            test_avl_locate_ge_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
            test_avl_locate_ge_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
            test_avl_locate_ge_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str);
            test_avl_locate_ge_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);
            test_avl_locate_ge_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);
            test_avl_locate_ge_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);
            test_avl_locate_ge_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);
            test_avl_locate_ge_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);
            test_avl_locate_ge_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);
            test_avl_locate_ge_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);
            test_avl_locate_ge_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);
            test_avl_locate_ge_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);
            test_avl_locate_ge_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);
            test_avl_locate_ge       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);
            test_avl_locate_ge_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1);
            test_avl_locate_ge_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str);
            test_avl_locate_ge_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0);
            test_avl_locate_ge_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1);
            test_avl_locate_ge_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str);
            test_avl_locate_ge_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);
            test_avl_locate_ge_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);
            test_avl_locate_ge_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);
            test_avl_locate_ge_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);
            test_avl_locate_ge_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);
            test_avl_locate_ge_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);
            test_avl_locate_ge_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);
            test_avl_locate_ge_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);
            test_avl_locate_ge_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);
            test_avl_locate_ge_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);

            test_avl_locate_gt       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
            test_avl_locate_gt_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
            test_avl_locate_gt_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
            test_avl_locate_gt_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
            test_avl_locate_gt_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
            test_avl_locate_gt_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str);
            test_avl_locate_gt_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);
            test_avl_locate_gt_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);
            test_avl_locate_gt_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);
            test_avl_locate_gt_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);
            test_avl_locate_gt_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);
            test_avl_locate_gt_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);
            test_avl_locate_gt_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);
            test_avl_locate_gt_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);
            test_avl_locate_gt_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);
            test_avl_locate_gt_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);
            test_avl_locate_gt       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);
            test_avl_locate_gt_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1);
            test_avl_locate_gt_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str);
            test_avl_locate_gt_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0);
            test_avl_locate_gt_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1);
            test_avl_locate_gt_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str);
            test_avl_locate_gt_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);
            test_avl_locate_gt_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);
            test_avl_locate_gt_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);
            test_avl_locate_gt_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);
            test_avl_locate_gt_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);
            test_avl_locate_gt_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);
            test_avl_locate_gt_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);
            test_avl_locate_gt_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);
            test_avl_locate_gt_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);
            test_avl_locate_gt_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);

            test_avl_locate_le       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
            test_avl_locate_le_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
            test_avl_locate_le_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
            test_avl_locate_le_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
            test_avl_locate_le_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
            test_avl_locate_le_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str);
            test_avl_locate_le_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);
            test_avl_locate_le_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);
            test_avl_locate_le_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);
            test_avl_locate_le_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);
            test_avl_locate_le_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);
            test_avl_locate_le_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);
            test_avl_locate_le_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);
            test_avl_locate_le_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);
            test_avl_locate_le_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);
            test_avl_locate_le_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);
            test_avl_locate_le       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);
            test_avl_locate_le_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1);
            test_avl_locate_le_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str);
            test_avl_locate_le_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0);
            test_avl_locate_le_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1);
            test_avl_locate_le_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str);
            test_avl_locate_le_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);
            test_avl_locate_le_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);
            test_avl_locate_le_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);
            test_avl_locate_le_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);
            test_avl_locate_le_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);
            test_avl_locate_le_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);
            test_avl_locate_le_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);
            test_avl_locate_le_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);
            test_avl_locate_le_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);
            test_avl_locate_le_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);

            test_avl_locate_lt       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);
            test_avl_locate_lt_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1);
            test_avl_locate_lt_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str);
            test_avl_locate_lt_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0);
            test_avl_locate_lt_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1);
            test_avl_locate_lt_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str);
            test_avl_locate_lt_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);
            test_avl_locate_lt_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);
            test_avl_locate_lt_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);
            test_avl_locate_lt_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);
            test_avl_locate_lt_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);
            test_avl_locate_lt_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);
            test_avl_locate_lt_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);
            test_avl_locate_lt_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);
            test_avl_locate_lt_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);
            test_avl_locate_lt_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);
            test_avl_locate_lt       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);
            test_avl_locate_lt_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1);
            test_avl_locate_lt_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str);
            test_avl_locate_lt_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0);
            test_avl_locate_lt_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1);
            test_avl_locate_lt_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str);
            test_avl_locate_lt_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);
            test_avl_locate_lt_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);
            test_avl_locate_lt_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);
            test_avl_locate_lt_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);
            test_avl_locate_lt_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);
            test_avl_locate_lt_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);
            test_avl_locate_lt_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);
            test_avl_locate_lt_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);
            test_avl_locate_lt_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);
            test_avl_locate_lt_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);

            test_avl_start       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);    test_avl_next(obj_tree_nodup, obj_vect_nodup); test_avl_next(obj_tree_nodup, obj_vect_nodup); test_avl_prev(obj_tree_nodup, obj_vect_nodup);
            test_avl_start_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1); test_avl_next(mbr_tree_nodup, mbr_vect_nodup); test_avl_next(mbr_tree_nodup, mbr_vect_nodup); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup);
            test_avl_start_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str); test_avl_next(ptr_tree_nodup, ptr_vect_nodup); test_avl_next(ptr_tree_nodup, ptr_vect_nodup); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup);
            test_avl_start_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0); test_avl_next(ch0_tree_nodup, ch0_vect_nodup); test_avl_next(ch0_tree_nodup, ch0_vect_nodup); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup);
            test_avl_start_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1); test_avl_next(ch1_tree_nodup, ch1_vect_nodup); test_avl_next(ch1_tree_nodup, ch1_vect_nodup); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup);
            test_avl_start_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str); test_avl_next(str_tree_nodup, str_vect_nodup); test_avl_next(str_tree_nodup, str_vect_nodup); test_avl_prev(str_tree_nodup, str_vect_nodup);
            test_avl_start_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);   test_avl_next(lng_tree_nodup, lng_vect_nodup); test_avl_next(lng_tree_nodup, lng_vect_nodup); test_avl_prev(lng_tree_nodup, lng_vect_nodup);
            test_avl_start_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);   test_avl_next(int_tree_nodup, int_vect_nodup); test_avl_next(int_tree_nodup, int_vect_nodup); test_avl_prev(int_tree_nodup, int_vect_nodup);
            test_avl_start_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);   test_avl_next(sht_tree_nodup, sht_vect_nodup); test_avl_next(sht_tree_nodup, sht_vect_nodup); test_avl_prev(sht_tree_nodup, sht_vect_nodup);
            test_avl_start_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);  test_avl_next(sch_tree_nodup, sch_vect_nodup); test_avl_next(sch_tree_nodup, sch_vect_nodup); test_avl_prev(sch_tree_nodup, sch_vect_nodup);
            test_avl_start_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);  test_avl_next(uln_tree_nodup, uln_vect_nodup); test_avl_next(uln_tree_nodup, uln_vect_nodup); test_avl_prev(uln_tree_nodup, uln_vect_nodup);
            test_avl_start_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);  test_avl_next(uin_tree_nodup, uin_vect_nodup); test_avl_next(uin_tree_nodup, uin_vect_nodup); test_avl_prev(uin_tree_nodup, uin_vect_nodup);
            test_avl_start_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);  test_avl_next(ush_tree_nodup, ush_vect_nodup); test_avl_next(ush_tree_nodup, ush_vect_nodup); test_avl_prev(ush_tree_nodup, ush_vect_nodup);
            test_avl_start_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);  test_avl_next(uch_tree_nodup, uch_vect_nodup); test_avl_next(uch_tree_nodup, uch_vect_nodup); test_avl_prev(uch_tree_nodup, uch_vect_nodup);
            test_avl_start_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);   test_avl_next(flt_tree_nodup, flt_vect_nodup); test_avl_next(flt_tree_nodup, flt_vect_nodup); test_avl_prev(flt_tree_nodup, flt_vect_nodup);
            test_avl_start_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);   test_avl_next(dbl_tree_nodup, dbl_vect_nodup); test_avl_next(dbl_tree_nodup, dbl_vect_nodup); test_avl_prev(dbl_tree_nodup, dbl_vect_nodup);
            test_avl_start       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);    test_avl_next(obj_tree_dup  , obj_vect_dup  ); test_avl_next(obj_tree_dup  , obj_vect_dup  ); test_avl_prev(obj_tree_dup  , obj_vect_dup  );
            test_avl_start_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1); test_avl_next(mbr_tree_dup  , mbr_vect_dup  ); test_avl_next(mbr_tree_dup  , mbr_vect_dup  ); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  );
            test_avl_start_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str); test_avl_next(ptr_tree_dup  , ptr_vect_dup  ); test_avl_next(ptr_tree_dup  , ptr_vect_dup  ); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  );
            test_avl_start_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0); test_avl_next(ch0_tree_dup  , ch0_vect_dup  ); test_avl_next(ch0_tree_dup  , ch0_vect_dup  ); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  );
            test_avl_start_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1); test_avl_next(ch1_tree_dup  , ch1_vect_dup  ); test_avl_next(ch1_tree_dup  , ch1_vect_dup  ); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  );
            test_avl_start_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str); test_avl_next(str_tree_dup  , str_vect_dup  ); test_avl_next(str_tree_dup  , str_vect_dup  ); test_avl_prev(str_tree_dup  , str_vect_dup  );
            test_avl_start_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);   test_avl_next(lng_tree_dup  , lng_vect_dup  ); test_avl_next(lng_tree_dup  , lng_vect_dup  ); test_avl_prev(lng_tree_dup  , lng_vect_dup  );
            test_avl_start_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);   test_avl_next(int_tree_dup  , int_vect_dup  ); test_avl_next(int_tree_dup  , int_vect_dup  ); test_avl_prev(int_tree_dup  , int_vect_dup  );
            test_avl_start_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);   test_avl_next(sht_tree_dup  , sht_vect_dup  ); test_avl_next(sht_tree_dup  , sht_vect_dup  ); test_avl_prev(sht_tree_dup  , sht_vect_dup  );
            test_avl_start_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);  test_avl_next(sch_tree_dup  , sch_vect_dup  ); test_avl_next(sch_tree_dup  , sch_vect_dup  ); test_avl_prev(sch_tree_dup  , sch_vect_dup  );
            test_avl_start_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);  test_avl_next(uln_tree_dup  , uln_vect_dup  ); test_avl_next(uln_tree_dup  , uln_vect_dup  ); test_avl_prev(uln_tree_dup  , uln_vect_dup  );
            test_avl_start_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);  test_avl_next(uin_tree_dup  , uin_vect_dup  ); test_avl_next(uin_tree_dup  , uin_vect_dup  ); test_avl_prev(uin_tree_dup  , uin_vect_dup  );
            test_avl_start_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);  test_avl_next(ush_tree_dup  , ush_vect_dup  ); test_avl_next(ush_tree_dup  , ush_vect_dup  ); test_avl_prev(ush_tree_dup  , ush_vect_dup  );
            test_avl_start_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);  test_avl_next(uch_tree_dup  , uch_vect_dup  ); test_avl_next(uch_tree_dup  , uch_vect_dup  ); test_avl_prev(uch_tree_dup  , uch_vect_dup  );
            test_avl_start_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);   test_avl_next(flt_tree_dup  , flt_vect_dup  ); test_avl_next(flt_tree_dup  , flt_vect_dup  ); test_avl_prev(flt_tree_dup  , flt_vect_dup  );
            test_avl_start_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);   test_avl_next(dbl_tree_dup  , dbl_vect_dup  ); test_avl_next(dbl_tree_dup  , dbl_vect_dup  ); test_avl_prev(dbl_tree_dup  , dbl_vect_dup  );

            test_avl_rev_start       (obj_tree_nodup, obj_vect_nodup, &obj_v[r]);    test_avl_prev(obj_tree_nodup, obj_vect_nodup); test_avl_prev(obj_tree_nodup, obj_vect_nodup); test_avl_next(obj_tree_nodup, obj_vect_nodup);
            test_avl_rev_start_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r].ch1); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup); test_avl_next(mbr_tree_nodup, mbr_vect_nodup);
            test_avl_rev_start_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r].str); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup); test_avl_next(ptr_tree_nodup, ptr_vect_nodup);
            test_avl_rev_start_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r].ch0); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup); test_avl_next(ch0_tree_nodup, ch0_vect_nodup);
            test_avl_rev_start_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r].ch1); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup); test_avl_next(ch1_tree_nodup, ch1_vect_nodup);
            test_avl_rev_start_str   (str_tree_nodup, str_vect_nodup, obj_v[r].str); test_avl_prev(str_tree_nodup, str_vect_nodup); test_avl_prev(str_tree_nodup, str_vect_nodup); test_avl_next(str_tree_nodup, str_vect_nodup);
            test_avl_rev_start_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r].l);   test_avl_prev(lng_tree_nodup, lng_vect_nodup); test_avl_prev(lng_tree_nodup, lng_vect_nodup); test_avl_next(lng_tree_nodup, lng_vect_nodup);
            test_avl_rev_start_int   (int_tree_nodup, int_vect_nodup, obj_v[r].i);   test_avl_prev(int_tree_nodup, int_vect_nodup); test_avl_prev(int_tree_nodup, int_vect_nodup); test_avl_next(int_tree_nodup, int_vect_nodup);
            test_avl_rev_start_short (sht_tree_nodup, sht_vect_nodup, obj_v[r].s);   test_avl_prev(sht_tree_nodup, sht_vect_nodup); test_avl_prev(sht_tree_nodup, sht_vect_nodup); test_avl_next(sht_tree_nodup, sht_vect_nodup);
            test_avl_rev_start_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r].sc);  test_avl_prev(sch_tree_nodup, sch_vect_nodup); test_avl_prev(sch_tree_nodup, sch_vect_nodup); test_avl_next(sch_tree_nodup, sch_vect_nodup);
            test_avl_rev_start_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r].ul);  test_avl_prev(uln_tree_nodup, uln_vect_nodup); test_avl_prev(uln_tree_nodup, uln_vect_nodup); test_avl_next(uln_tree_nodup, uln_vect_nodup);
            test_avl_rev_start_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r].ui);  test_avl_prev(uin_tree_nodup, uin_vect_nodup); test_avl_prev(uin_tree_nodup, uin_vect_nodup); test_avl_next(uin_tree_nodup, uin_vect_nodup);
            test_avl_rev_start_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r].us);  test_avl_prev(ush_tree_nodup, ush_vect_nodup); test_avl_prev(ush_tree_nodup, ush_vect_nodup); test_avl_next(ush_tree_nodup, ush_vect_nodup);
            test_avl_rev_start_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r].uc);  test_avl_prev(uch_tree_nodup, uch_vect_nodup); test_avl_prev(uch_tree_nodup, uch_vect_nodup); test_avl_next(uch_tree_nodup, uch_vect_nodup);
            test_avl_rev_start_float (flt_tree_nodup, flt_vect_nodup, obj_v[r].f);   test_avl_prev(flt_tree_nodup, flt_vect_nodup); test_avl_prev(flt_tree_nodup, flt_vect_nodup); test_avl_next(flt_tree_nodup, flt_vect_nodup);
            test_avl_rev_start_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r].d);   test_avl_prev(dbl_tree_nodup, dbl_vect_nodup); test_avl_prev(dbl_tree_nodup, dbl_vect_nodup); test_avl_next(dbl_tree_nodup, dbl_vect_nodup);
            test_avl_rev_start       (obj_tree_dup  , obj_vect_dup  , &obj_v[r]);    test_avl_prev(obj_tree_dup  , obj_vect_dup  ); test_avl_prev(obj_tree_dup  , obj_vect_dup  ); test_avl_next(obj_tree_dup  , obj_vect_dup  );
            test_avl_rev_start_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r].ch1); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  ); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  ); test_avl_next(mbr_tree_dup  , mbr_vect_dup  );
            test_avl_rev_start_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r].str); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  ); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  ); test_avl_next(ptr_tree_dup  , ptr_vect_dup  );
            test_avl_rev_start_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r].ch0); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  ); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  ); test_avl_next(ch0_tree_dup  , ch0_vect_dup  );
            test_avl_rev_start_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r].ch1); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  ); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  ); test_avl_next(ch1_tree_dup  , ch1_vect_dup  );
            test_avl_rev_start_str   (str_tree_dup  , str_vect_dup  , obj_v[r].str); test_avl_prev(str_tree_dup  , str_vect_dup  ); test_avl_prev(str_tree_dup  , str_vect_dup  ); test_avl_next(str_tree_dup  , str_vect_dup  );
            test_avl_rev_start_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r].l);   test_avl_prev(lng_tree_dup  , lng_vect_dup  ); test_avl_prev(lng_tree_dup  , lng_vect_dup  ); test_avl_next(lng_tree_dup  , lng_vect_dup  );
            test_avl_rev_start_int   (int_tree_dup  , int_vect_dup  , obj_v[r].i);   test_avl_prev(int_tree_dup  , int_vect_dup  ); test_avl_prev(int_tree_dup  , int_vect_dup  ); test_avl_next(int_tree_dup  , int_vect_dup  );
            test_avl_rev_start_short (sht_tree_dup  , sht_vect_dup  , obj_v[r].s);   test_avl_prev(sht_tree_dup  , sht_vect_dup  ); test_avl_prev(sht_tree_dup  , sht_vect_dup  ); test_avl_next(sht_tree_dup  , sht_vect_dup  );
            test_avl_rev_start_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r].sc);  test_avl_prev(sch_tree_dup  , sch_vect_dup  ); test_avl_prev(sch_tree_dup  , sch_vect_dup  ); test_avl_next(sch_tree_dup  , sch_vect_dup  );
            test_avl_rev_start_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r].ul);  test_avl_prev(uln_tree_dup  , uln_vect_dup  ); test_avl_prev(uln_tree_dup  , uln_vect_dup  ); test_avl_next(uln_tree_dup  , uln_vect_dup  );
            test_avl_rev_start_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r].ui);  test_avl_prev(uin_tree_dup  , uin_vect_dup  ); test_avl_prev(uin_tree_dup  , uin_vect_dup  ); test_avl_next(uin_tree_dup  , uin_vect_dup  );
            test_avl_rev_start_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r].us);  test_avl_prev(ush_tree_dup  , ush_vect_dup  ); test_avl_prev(ush_tree_dup  , ush_vect_dup  ); test_avl_next(ush_tree_dup  , ush_vect_dup  );
            test_avl_rev_start_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r].uc);  test_avl_prev(uch_tree_dup  , uch_vect_dup  ); test_avl_prev(uch_tree_dup  , uch_vect_dup  ); test_avl_next(uch_tree_dup  , uch_vect_dup  );
            test_avl_rev_start_float (flt_tree_dup  , flt_vect_dup  , obj_v[r].f);   test_avl_prev(flt_tree_dup  , flt_vect_dup  ); test_avl_prev(flt_tree_dup  , flt_vect_dup  ); test_avl_next(flt_tree_dup  , flt_vect_dup  );
            test_avl_rev_start_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r].d);   test_avl_prev(dbl_tree_dup  , dbl_vect_dup  ); test_avl_prev(dbl_tree_dup  , dbl_vect_dup  ); test_avl_next(dbl_tree_dup  , dbl_vect_dup  );
         }
         test_avl_first(obj_tree_nodup, obj_vect_nodup); test_avl_next(obj_tree_nodup, obj_vect_nodup); test_avl_next(obj_tree_nodup, obj_vect_nodup); test_avl_prev(obj_tree_nodup, obj_vect_nodup);
         test_avl_first(mbr_tree_nodup, mbr_vect_nodup); test_avl_next(mbr_tree_nodup, mbr_vect_nodup); test_avl_next(mbr_tree_nodup, mbr_vect_nodup); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup);
         test_avl_first(ptr_tree_nodup, ptr_vect_nodup); test_avl_next(ptr_tree_nodup, ptr_vect_nodup); test_avl_next(ptr_tree_nodup, ptr_vect_nodup); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup);
         test_avl_first(ch0_tree_nodup, ch0_vect_nodup); test_avl_next(ch0_tree_nodup, ch0_vect_nodup); test_avl_next(ch0_tree_nodup, ch0_vect_nodup); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup);
         test_avl_first(ch1_tree_nodup, ch1_vect_nodup); test_avl_next(ch1_tree_nodup, ch1_vect_nodup); test_avl_next(ch1_tree_nodup, ch1_vect_nodup); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup);
         test_avl_first(str_tree_nodup, str_vect_nodup); test_avl_next(str_tree_nodup, str_vect_nodup); test_avl_next(str_tree_nodup, str_vect_nodup); test_avl_prev(str_tree_nodup, str_vect_nodup);
         test_avl_first(lng_tree_nodup, lng_vect_nodup); test_avl_next(lng_tree_nodup, lng_vect_nodup); test_avl_next(lng_tree_nodup, lng_vect_nodup); test_avl_prev(lng_tree_nodup, lng_vect_nodup);
         test_avl_first(int_tree_nodup, int_vect_nodup); test_avl_next(int_tree_nodup, int_vect_nodup); test_avl_next(int_tree_nodup, int_vect_nodup); test_avl_prev(int_tree_nodup, int_vect_nodup);
         test_avl_first(sht_tree_nodup, sht_vect_nodup); test_avl_next(sht_tree_nodup, sht_vect_nodup); test_avl_next(sht_tree_nodup, sht_vect_nodup); test_avl_prev(sht_tree_nodup, sht_vect_nodup);
         test_avl_first(sch_tree_nodup, sch_vect_nodup); test_avl_next(sch_tree_nodup, sch_vect_nodup); test_avl_next(sch_tree_nodup, sch_vect_nodup); test_avl_prev(sch_tree_nodup, sch_vect_nodup);
         test_avl_first(uln_tree_nodup, uln_vect_nodup); test_avl_next(uln_tree_nodup, uln_vect_nodup); test_avl_next(uln_tree_nodup, uln_vect_nodup); test_avl_prev(uln_tree_nodup, uln_vect_nodup);
         test_avl_first(uin_tree_nodup, uin_vect_nodup); test_avl_next(uin_tree_nodup, uin_vect_nodup); test_avl_next(uin_tree_nodup, uin_vect_nodup); test_avl_prev(uin_tree_nodup, uin_vect_nodup);
         test_avl_first(ush_tree_nodup, ush_vect_nodup); test_avl_next(ush_tree_nodup, ush_vect_nodup); test_avl_next(ush_tree_nodup, ush_vect_nodup); test_avl_prev(ush_tree_nodup, ush_vect_nodup);
         test_avl_first(uch_tree_nodup, uch_vect_nodup); test_avl_next(uch_tree_nodup, uch_vect_nodup); test_avl_next(uch_tree_nodup, uch_vect_nodup); test_avl_prev(uch_tree_nodup, uch_vect_nodup);
         test_avl_first(flt_tree_nodup, flt_vect_nodup); test_avl_next(flt_tree_nodup, flt_vect_nodup); test_avl_next(flt_tree_nodup, flt_vect_nodup); test_avl_prev(flt_tree_nodup, flt_vect_nodup);
         test_avl_first(dbl_tree_nodup, dbl_vect_nodup); test_avl_next(dbl_tree_nodup, dbl_vect_nodup); test_avl_next(dbl_tree_nodup, dbl_vect_nodup); test_avl_prev(dbl_tree_nodup, dbl_vect_nodup);
         test_avl_first(obj_tree_dup  , obj_vect_dup  ); test_avl_next(obj_tree_dup  , obj_vect_dup  ); test_avl_next(obj_tree_dup  , obj_vect_dup  ); test_avl_prev(obj_tree_dup  , obj_vect_dup  );
         test_avl_first(mbr_tree_dup  , mbr_vect_dup  ); test_avl_next(mbr_tree_dup  , mbr_vect_dup  ); test_avl_next(mbr_tree_dup  , mbr_vect_dup  ); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  );
         test_avl_first(ptr_tree_dup  , ptr_vect_dup  ); test_avl_next(ptr_tree_dup  , ptr_vect_dup  ); test_avl_next(ptr_tree_dup  , ptr_vect_dup  ); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  );
         test_avl_first(ch0_tree_dup  , ch0_vect_dup  ); test_avl_next(ch0_tree_dup  , ch0_vect_dup  ); test_avl_next(ch0_tree_dup  , ch0_vect_dup  ); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  );
         test_avl_first(ch1_tree_dup  , ch1_vect_dup  ); test_avl_next(ch1_tree_dup  , ch1_vect_dup  ); test_avl_next(ch1_tree_dup  , ch1_vect_dup  ); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  );
         test_avl_first(str_tree_dup  , str_vect_dup  ); test_avl_next(str_tree_dup  , str_vect_dup  ); test_avl_next(str_tree_dup  , str_vect_dup  ); test_avl_prev(str_tree_dup  , str_vect_dup  );
         test_avl_first(lng_tree_dup  , lng_vect_dup  ); test_avl_next(lng_tree_dup  , lng_vect_dup  ); test_avl_next(lng_tree_dup  , lng_vect_dup  ); test_avl_prev(lng_tree_dup  , lng_vect_dup  );
         test_avl_first(int_tree_dup  , int_vect_dup  ); test_avl_next(int_tree_dup  , int_vect_dup  ); test_avl_next(int_tree_dup  , int_vect_dup  ); test_avl_prev(int_tree_dup  , int_vect_dup  );
         test_avl_first(sht_tree_dup  , sht_vect_dup  ); test_avl_next(sht_tree_dup  , sht_vect_dup  ); test_avl_next(sht_tree_dup  , sht_vect_dup  ); test_avl_prev(sht_tree_dup  , sht_vect_dup  );
         test_avl_first(sch_tree_dup  , sch_vect_dup  ); test_avl_next(sch_tree_dup  , sch_vect_dup  ); test_avl_next(sch_tree_dup  , sch_vect_dup  ); test_avl_prev(sch_tree_dup  , sch_vect_dup  );
         test_avl_first(uln_tree_dup  , uln_vect_dup  ); test_avl_next(uln_tree_dup  , uln_vect_dup  ); test_avl_next(uln_tree_dup  , uln_vect_dup  ); test_avl_prev(uln_tree_dup  , uln_vect_dup  );
         test_avl_first(uin_tree_dup  , uin_vect_dup  ); test_avl_next(uin_tree_dup  , uin_vect_dup  ); test_avl_next(uin_tree_dup  , uin_vect_dup  ); test_avl_prev(uin_tree_dup  , uin_vect_dup  );
         test_avl_first(ush_tree_dup  , ush_vect_dup  ); test_avl_next(ush_tree_dup  , ush_vect_dup  ); test_avl_next(ush_tree_dup  , ush_vect_dup  ); test_avl_prev(ush_tree_dup  , ush_vect_dup  );
         test_avl_first(uch_tree_dup  , uch_vect_dup  ); test_avl_next(uch_tree_dup  , uch_vect_dup  ); test_avl_next(uch_tree_dup  , uch_vect_dup  ); test_avl_prev(uch_tree_dup  , uch_vect_dup  );
         test_avl_first(flt_tree_dup  , flt_vect_dup  ); test_avl_next(flt_tree_dup  , flt_vect_dup  ); test_avl_next(flt_tree_dup  , flt_vect_dup  ); test_avl_prev(flt_tree_dup  , flt_vect_dup  );
         test_avl_first(dbl_tree_dup  , dbl_vect_dup  ); test_avl_next(dbl_tree_dup  , dbl_vect_dup  ); test_avl_next(dbl_tree_dup  , dbl_vect_dup  ); test_avl_prev(dbl_tree_dup  , dbl_vect_dup  );

         test_avl_last (obj_tree_nodup, obj_vect_nodup); test_avl_prev(obj_tree_nodup, obj_vect_nodup); test_avl_prev(obj_tree_nodup, obj_vect_nodup); test_avl_next(obj_tree_nodup, obj_vect_nodup);
         test_avl_last (mbr_tree_nodup, mbr_vect_nodup); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup); test_avl_prev(mbr_tree_nodup, mbr_vect_nodup); test_avl_next(mbr_tree_nodup, mbr_vect_nodup);
         test_avl_last (ptr_tree_nodup, ptr_vect_nodup); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup); test_avl_prev(ptr_tree_nodup, ptr_vect_nodup); test_avl_next(ptr_tree_nodup, ptr_vect_nodup);
         test_avl_last (ch0_tree_nodup, ch0_vect_nodup); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup); test_avl_prev(ch0_tree_nodup, ch0_vect_nodup); test_avl_next(ch0_tree_nodup, ch0_vect_nodup);
         test_avl_last (ch1_tree_nodup, ch1_vect_nodup); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup); test_avl_prev(ch1_tree_nodup, ch1_vect_nodup); test_avl_next(ch1_tree_nodup, ch1_vect_nodup);
         test_avl_last (str_tree_nodup, str_vect_nodup); test_avl_prev(str_tree_nodup, str_vect_nodup); test_avl_prev(str_tree_nodup, str_vect_nodup); test_avl_next(str_tree_nodup, str_vect_nodup);
         test_avl_last (lng_tree_nodup, lng_vect_nodup); test_avl_prev(lng_tree_nodup, lng_vect_nodup); test_avl_prev(lng_tree_nodup, lng_vect_nodup); test_avl_next(lng_tree_nodup, lng_vect_nodup);
         test_avl_last (int_tree_nodup, int_vect_nodup); test_avl_prev(int_tree_nodup, int_vect_nodup); test_avl_prev(int_tree_nodup, int_vect_nodup); test_avl_next(int_tree_nodup, int_vect_nodup);
         test_avl_last (sht_tree_nodup, sht_vect_nodup); test_avl_prev(sht_tree_nodup, sht_vect_nodup); test_avl_prev(sht_tree_nodup, sht_vect_nodup); test_avl_next(sht_tree_nodup, sht_vect_nodup);
         test_avl_last (sch_tree_nodup, sch_vect_nodup); test_avl_prev(sch_tree_nodup, sch_vect_nodup); test_avl_prev(sch_tree_nodup, sch_vect_nodup); test_avl_next(sch_tree_nodup, sch_vect_nodup);
         test_avl_last (uln_tree_nodup, uln_vect_nodup); test_avl_prev(uln_tree_nodup, uln_vect_nodup); test_avl_prev(uln_tree_nodup, uln_vect_nodup); test_avl_next(uln_tree_nodup, uln_vect_nodup);
         test_avl_last (uin_tree_nodup, uin_vect_nodup); test_avl_prev(uin_tree_nodup, uin_vect_nodup); test_avl_prev(uin_tree_nodup, uin_vect_nodup); test_avl_next(uin_tree_nodup, uin_vect_nodup);
         test_avl_last (ush_tree_nodup, ush_vect_nodup); test_avl_prev(ush_tree_nodup, ush_vect_nodup); test_avl_prev(ush_tree_nodup, ush_vect_nodup); test_avl_next(ush_tree_nodup, ush_vect_nodup);
         test_avl_last (uch_tree_nodup, uch_vect_nodup); test_avl_prev(uch_tree_nodup, uch_vect_nodup); test_avl_prev(uch_tree_nodup, uch_vect_nodup); test_avl_next(uch_tree_nodup, uch_vect_nodup);
         test_avl_last (flt_tree_nodup, flt_vect_nodup); test_avl_prev(flt_tree_nodup, flt_vect_nodup); test_avl_prev(flt_tree_nodup, flt_vect_nodup); test_avl_next(flt_tree_nodup, flt_vect_nodup);
         test_avl_last (dbl_tree_nodup, dbl_vect_nodup); test_avl_prev(dbl_tree_nodup, dbl_vect_nodup); test_avl_prev(dbl_tree_nodup, dbl_vect_nodup); test_avl_next(dbl_tree_nodup, dbl_vect_nodup);
         test_avl_last (obj_tree_dup  , obj_vect_dup  ); test_avl_prev(obj_tree_dup  , obj_vect_dup  ); test_avl_prev(obj_tree_dup  , obj_vect_dup  ); test_avl_next(obj_tree_dup  , obj_vect_dup  );
         test_avl_last (mbr_tree_dup  , mbr_vect_dup  ); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  ); test_avl_prev(mbr_tree_dup  , mbr_vect_dup  ); test_avl_next(mbr_tree_dup  , mbr_vect_dup  );
         test_avl_last (ptr_tree_dup  , ptr_vect_dup  ); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  ); test_avl_prev(ptr_tree_dup  , ptr_vect_dup  ); test_avl_next(ptr_tree_dup  , ptr_vect_dup  );
         test_avl_last (ch0_tree_dup  , ch0_vect_dup  ); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  ); test_avl_prev(ch0_tree_dup  , ch0_vect_dup  ); test_avl_next(ch0_tree_dup  , ch0_vect_dup  );
         test_avl_last (ch1_tree_dup  , ch1_vect_dup  ); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  ); test_avl_prev(ch1_tree_dup  , ch1_vect_dup  ); test_avl_next(ch1_tree_dup  , ch1_vect_dup  );
         test_avl_last (str_tree_dup  , str_vect_dup  ); test_avl_prev(str_tree_dup  , str_vect_dup  ); test_avl_prev(str_tree_dup  , str_vect_dup  ); test_avl_next(str_tree_dup  , str_vect_dup  );
         test_avl_last (lng_tree_dup  , lng_vect_dup  ); test_avl_prev(lng_tree_dup  , lng_vect_dup  ); test_avl_prev(lng_tree_dup  , lng_vect_dup  ); test_avl_next(lng_tree_dup  , lng_vect_dup  );
         test_avl_last (int_tree_dup  , int_vect_dup  ); test_avl_prev(int_tree_dup  , int_vect_dup  ); test_avl_prev(int_tree_dup  , int_vect_dup  ); test_avl_next(int_tree_dup  , int_vect_dup  );
         test_avl_last (sht_tree_dup  , sht_vect_dup  ); test_avl_prev(sht_tree_dup  , sht_vect_dup  ); test_avl_prev(sht_tree_dup  , sht_vect_dup  ); test_avl_next(sht_tree_dup  , sht_vect_dup  );
         test_avl_last (sch_tree_dup  , sch_vect_dup  ); test_avl_prev(sch_tree_dup  , sch_vect_dup  ); test_avl_prev(sch_tree_dup  , sch_vect_dup  ); test_avl_next(sch_tree_dup  , sch_vect_dup  );
         test_avl_last (uln_tree_dup  , uln_vect_dup  ); test_avl_prev(uln_tree_dup  , uln_vect_dup  ); test_avl_prev(uln_tree_dup  , uln_vect_dup  ); test_avl_next(uln_tree_dup  , uln_vect_dup  );
         test_avl_last (uin_tree_dup  , uin_vect_dup  ); test_avl_prev(uin_tree_dup  , uin_vect_dup  ); test_avl_prev(uin_tree_dup  , uin_vect_dup  ); test_avl_next(uin_tree_dup  , uin_vect_dup  );
         test_avl_last (ush_tree_dup  , ush_vect_dup  ); test_avl_prev(ush_tree_dup  , ush_vect_dup  ); test_avl_prev(ush_tree_dup  , ush_vect_dup  ); test_avl_next(ush_tree_dup  , ush_vect_dup  );
         test_avl_last (uch_tree_dup  , uch_vect_dup  ); test_avl_prev(uch_tree_dup  , uch_vect_dup  ); test_avl_prev(uch_tree_dup  , uch_vect_dup  ); test_avl_next(uch_tree_dup  , uch_vect_dup  );
         test_avl_last (flt_tree_dup  , flt_vect_dup  ); test_avl_prev(flt_tree_dup  , flt_vect_dup  ); test_avl_prev(flt_tree_dup  , flt_vect_dup  ); test_avl_next(flt_tree_dup  , flt_vect_dup  );
         test_avl_last (dbl_tree_dup  , dbl_vect_dup  ); test_avl_prev(dbl_tree_dup  , dbl_vect_dup  ); test_avl_prev(dbl_tree_dup  , dbl_vect_dup  ); test_avl_next(dbl_tree_dup  , dbl_vect_dup  );
      }
      for (i = 0; i < N_OBJ_2; i++) {
         test_avl_remove       (obj_tree_nodup, obj_vect_nodup, &obj_v[r_v[i]]);
         test_avl_remove_mbr   (mbr_tree_nodup, mbr_vect_nodup, obj_v[r_v[i]].ch1);
         test_avl_remove_ptr   (ptr_tree_nodup, ptr_vect_nodup, obj_v[r_v[i]].str);
         test_avl_remove_chars (ch0_tree_nodup, ch0_vect_nodup, obj_v[r_v[i]].ch0);
         test_avl_remove_chars (ch1_tree_nodup, ch1_vect_nodup, obj_v[r_v[i]].ch1);
         test_avl_remove_str   (str_tree_nodup, str_vect_nodup, obj_v[r_v[i]].str);
         test_avl_remove_long  (lng_tree_nodup, lng_vect_nodup, obj_v[r_v[i]].l);
         test_avl_remove_int   (int_tree_nodup, int_vect_nodup, obj_v[r_v[i]].i);
         test_avl_remove_short (sht_tree_nodup, sht_vect_nodup, obj_v[r_v[i]].s);
         test_avl_remove_schar (sch_tree_nodup, sch_vect_nodup, obj_v[r_v[i]].sc);
         test_avl_remove_ulong (uln_tree_nodup, uln_vect_nodup, obj_v[r_v[i]].ul);
         test_avl_remove_uint  (uin_tree_nodup, uin_vect_nodup, obj_v[r_v[i]].ui);
         test_avl_remove_ushort(ush_tree_nodup, ush_vect_nodup, obj_v[r_v[i]].us);
         test_avl_remove_uchar (uch_tree_nodup, uch_vect_nodup, obj_v[r_v[i]].uc);
         test_avl_remove_float (flt_tree_nodup, flt_vect_nodup, obj_v[r_v[i]].f);
         test_avl_remove_double(dbl_tree_nodup, dbl_vect_nodup, obj_v[r_v[i]].d);
         test_avl_remove       (obj_tree_dup  , obj_vect_dup  , &obj_v[r_v[i]]);
         test_avl_remove_mbr   (mbr_tree_dup  , mbr_vect_dup  , obj_v[r_v[i]].ch1);
         test_avl_remove_ptr   (ptr_tree_dup  , ptr_vect_dup  , obj_v[r_v[i]].str);
         test_avl_remove_chars (ch0_tree_dup  , ch0_vect_dup  , obj_v[r_v[i]].ch0);
         test_avl_remove_chars (ch1_tree_dup  , ch1_vect_dup  , obj_v[r_v[i]].ch1);
         test_avl_remove_str   (str_tree_dup  , str_vect_dup  , obj_v[r_v[i]].str);
         test_avl_remove_long  (lng_tree_dup  , lng_vect_dup  , obj_v[r_v[i]].l);
         test_avl_remove_int   (int_tree_dup  , int_vect_dup  , obj_v[r_v[i]].i);
         test_avl_remove_short (sht_tree_dup  , sht_vect_dup  , obj_v[r_v[i]].s);
         test_avl_remove_schar (sch_tree_dup  , sch_vect_dup  , obj_v[r_v[i]].sc);
         test_avl_remove_ulong (uln_tree_dup  , uln_vect_dup  , obj_v[r_v[i]].ul);
         test_avl_remove_uint  (uin_tree_dup  , uin_vect_dup  , obj_v[r_v[i]].ui);
         test_avl_remove_ushort(ush_tree_dup  , ush_vect_dup  , obj_v[r_v[i]].us);
         test_avl_remove_uchar (uch_tree_dup  , uch_vect_dup  , obj_v[r_v[i]].uc);
         test_avl_remove_float (flt_tree_dup  , flt_vect_dup  , obj_v[r_v[i]].f);
         test_avl_remove_double(dbl_tree_dup  , dbl_vect_dup  , obj_v[r_v[i]].d);
      }
      assert(avl_nodes(obj_tree_nodup) == 0); assert(avm_nodes(obj_vect_nodup) == 0);
      assert(avl_nodes(mbr_tree_nodup) == 0); assert(avm_nodes(mbr_vect_nodup) == 0);
      assert(avl_nodes(ptr_tree_nodup) == 0); assert(avm_nodes(ptr_vect_nodup) == 0);
      assert(avl_nodes(ch0_tree_nodup) == 0); assert(avm_nodes(ch0_vect_nodup) == 0);
      assert(avl_nodes(ch1_tree_nodup) == 0); assert(avm_nodes(ch1_vect_nodup) == 0);
      assert(avl_nodes(str_tree_nodup) == 0); assert(avm_nodes(str_vect_nodup) == 0);
      assert(avl_nodes(lng_tree_nodup) == 0); assert(avm_nodes(lng_vect_nodup) == 0);
      assert(avl_nodes(int_tree_nodup) == 0); assert(avm_nodes(int_vect_nodup) == 0);
      assert(avl_nodes(sht_tree_nodup) == 0); assert(avm_nodes(sht_vect_nodup) == 0);
      assert(avl_nodes(sch_tree_nodup) == 0); assert(avm_nodes(sch_vect_nodup) == 0);
      assert(avl_nodes(uln_tree_nodup) == 0); assert(avm_nodes(uln_vect_nodup) == 0);
      assert(avl_nodes(uin_tree_nodup) == 0); assert(avm_nodes(uin_vect_nodup) == 0);
      assert(avl_nodes(ush_tree_nodup) == 0); assert(avm_nodes(ush_vect_nodup) == 0);
      assert(avl_nodes(uch_tree_nodup) == 0); assert(avm_nodes(uch_vect_nodup) == 0);
      assert(avl_nodes(flt_tree_nodup) == 0); assert(avm_nodes(flt_vect_nodup) == 0);
      assert(avl_nodes(dbl_tree_nodup) == 0); assert(avm_nodes(dbl_vect_nodup) == 0);
      assert(avl_nodes(obj_tree_dup  ) == 0); assert(avm_nodes(obj_vect_dup  ) == 0);
      assert(avl_nodes(mbr_tree_dup  ) == 0); assert(avm_nodes(mbr_vect_dup  ) == 0);
      assert(avl_nodes(ptr_tree_dup  ) == 0); assert(avm_nodes(ptr_vect_dup  ) == 0);
      assert(avl_nodes(ch0_tree_dup  ) == 0); assert(avm_nodes(ch0_vect_dup  ) == 0);
      assert(avl_nodes(ch1_tree_dup  ) == 0); assert(avm_nodes(ch1_vect_dup  ) == 0);
      assert(avl_nodes(str_tree_dup  ) == 0); assert(avm_nodes(str_vect_dup  ) == 0);
      assert(avl_nodes(lng_tree_dup  ) == 0); assert(avm_nodes(lng_vect_dup  ) == 0);
      assert(avl_nodes(int_tree_dup  ) == 0); assert(avm_nodes(int_vect_dup  ) == 0);
      assert(avl_nodes(sht_tree_dup  ) == 0); assert(avm_nodes(sht_vect_dup  ) == 0);
      assert(avl_nodes(sch_tree_dup  ) == 0); assert(avm_nodes(sch_vect_dup  ) == 0);
      assert(avl_nodes(uln_tree_dup  ) == 0); assert(avm_nodes(uln_vect_dup  ) == 0);
      assert(avl_nodes(uin_tree_dup  ) == 0); assert(avm_nodes(uin_vect_dup  ) == 0);
      assert(avl_nodes(ush_tree_dup  ) == 0); assert(avm_nodes(ush_vect_dup  ) == 0);
      assert(avl_nodes(uch_tree_dup  ) == 0); assert(avm_nodes(uch_vect_dup  ) == 0);
      assert(avl_nodes(flt_tree_dup  ) == 0); assert(avm_nodes(flt_vect_dup  ) == 0);
      assert(avl_nodes(dbl_tree_dup  ) == 0); assert(avm_nodes(dbl_vect_dup  ) == 0);
   }
   printf("\nok\n"); fflush(stdout);

   printf("%s OK\n", argv[0]);
   return 0;
}




