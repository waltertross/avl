/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avl.c                                    |
 |                                                                            |
 |                         An AVL Trees Library in C                          |
 |                                                                            |
 |                                  v 3.0.1                                   |
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

/*
 * For portable support of 2 kinds of tree nodes, this version contains
 * a lot of WET (write everything twice) code...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "avl.h"

/*---------------------------------------------------------------------------*/
/* Redefinable: */

#ifndef AVL_MALLOC
#define AVL_MALLOC malloc
#endif
#ifndef AVL_FREE
#define AVL_FREE free
#endif

#ifndef AVL_UINTPTR_T
#include <stdint.h>
#define UINTPTR uintptr_t
#else
#define UINTPTR AVL_UINTPTR_T
#endif

#ifndef AVL_NODE_INCREMENT_SHIFT
#define AVL_NODE_INCREMENT_SHIFT 1
#endif
#ifndef AVL_NODE_INCREMENT_MAX
#define AVL_NODE_INCREMENT_MAX (1024 * 1024 - 1)
#endif

/*---------------------------------------------------------------------------*/

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

typedef int (*CMPFUN)(void *, void *);

typedef struct avl_x_node X_NODE; /* external key node */
typedef struct avl_l_node L_NODE; /* long key node     */

struct avl_x_node {
   void *data;
   union {
      X_NODE *left;
      UINTPTR leftval;
   };
   union {
      X_NODE *right;
      UINTPTR rightval;
   };
};

struct avl_l_node {
   long key;
   union {
      L_NODE *left;
      UINTPTR leftval;
   };
   union {
      L_NODE *right;
      UINTPTR rightval;
   };
   void *data;
};

#define DEEPER          ((UINTPTR)1)
#define VAL_OF(nodeptr) ((UINTPTR)nodeptr)
#define PTR_OF(nodeptr) ((void   *)(VAL_OF(nodeptr) & ~DEEPER))
#define IS_DEEPER(nodeptr)         (VAL_OF(nodeptr) &  DEEPER)

#define LOG_2_MAX_NODES (sizeof(long) * CHAR_BIT - 1)
#define MAX_PATHDEPTH ((4761964 * LOG_2_MAX_NODES - 1083441) / 3305955)

typedef struct avl_path {
   union {
      void   **  pathnode;
      X_NODE **x_pathnode;
      L_NODE **l_pathnode;
   };
   char *pathright;
   union {
      void   *  node[MAX_PATHDEPTH + 2];
      X_NODE *x_node[MAX_PATHDEPTH + 2];
      L_NODE *l_node[MAX_PATHDEPTH + 2];
   };
   char right[MAX_PATHDEPTH + 2];
} PATH;

struct avl_tree {
   union {
      void   *  root;
      X_NODE *x_root;
      L_NODE *l_root;
   };
   CMPFUN usrcmp;
   PATH  *path;
   union {
      void   *  unused;
      X_NODE *x_unused;
      L_NODE *l_unused;
   };
   union {
      void   *  store;
      X_NODE *x_store;
      L_NODE *l_store;
   };
   long   nodes;
   long   alloc;
   int    avail;
   USHORT keyoffs;
   char   bits;
   char   type;
};

#define NODUP 0
#define DUP   1

#define AVL_CHA AVL_CHARS
#define AVL_LNG AVL_LONG
#define AVL_SHT AVL_SHORT
#define AVL_SCH AVL_SCHAR
#define AVL_ULN AVL_ULONG
#define AVL_UIN AVL_UINT
#define AVL_USH AVL_USHORT
#define AVL_UCH AVL_UCHAR
#define AVL_FLT AVL_FLOAT
#define AVL_DBL AVL_DOUBLE

#define USR_KEY (AVL_USR >> 1)
#define MBR_KEY (AVL_MBR >> 1)
#define PTR_KEY (AVL_PTR >> 1)
#define CHA_KEY (AVL_CHA >> 1)
#define STR_KEY (AVL_STR >> 1)
#define LNG_KEY (AVL_LNG >> 1)
#define INT_KEY (AVL_INT >> 1)
#define SHT_KEY (AVL_SHT >> 1)
#define SCH_KEY (AVL_SCH >> 1)
#define ULN_KEY (AVL_ULN >> 1)
#define UIN_KEY (AVL_UIN >> 1)
#define USH_KEY (AVL_USH >> 1)
#define UCH_KEY (AVL_UCH >> 1)
#define FLT_KEY (AVL_FLT >> 1)
#define DBL_KEY (AVL_DBL >> 1)

#define KEYTYPE(tree) ((tree)->type >> 1)

/* bits:          3 2 1 0
 *                X V I D
 * L_CHA_[NO]DUP  0 0 0 x
 * L_STR_[NO]DUP  0 0 1 x
 * L_VAL_[NO]DUP  0 1 0 x
 * L_COR_[NO]DUP  0 1 1 x
 * X_USR_[NO]DUP  1 0 0 x
 * X_PTR_[NO]DUP  1 0 1 x
 * X_MBR_[NO]DUP  1 1 0 x
 */
#define DUP_BIT  1
#define IND_BIT  2
#define CORR_BIT IND_BIT
#define VAL_BIT  4
#define MBR_BIT  VAL_BIT
#define X_BIT    8
#define IS_DUP(tree)  ((tree)->bits & DUP_BIT)
#define IS_IND(tree)  ((tree)->bits & IND_BIT)
#define IS_CORR(tree) ((tree)->bits & CORR_BIT)
#define IS_X(tree)    ((tree)->bits >= X_BIT)
#define CMPTYPE(tree) ((tree)->bits >> 1)

/* CMPTYPE */
#define X_USR_CMP (X_BIT >> 1)
#define X_MBR_CMP ((X_BIT | MBR_BIT) >> 1)
#define X_PTR_CMP ((X_BIT | IND_BIT) >> 1)
#define L_CHA_CMP 0
#define L_STR_CMP (IND_BIT >> 1)
#define L_VAL_CMP (VAL_BIT >> 1)
#define L_COR_CMP ((VAL_BIT | CORR_BIT) >> 1)

/* tree->bits */
#define X_USR_NODUP (X_USR_CMP << 1)
#define X_MBR_NODUP (X_MBR_CMP << 1)
#define X_PTR_NODUP (X_PTR_CMP << 1)
#define L_CHA_NODUP (L_CHA_CMP << 1)
#define L_STR_NODUP (L_STR_CMP << 1)
#define L_VAL_NODUP (L_VAL_CMP << 1)
#define L_COR_NODUP (L_COR_CMP << 1)
#define X_USR_DUP   (X_USR_CMP << 1 | DUP_BIT)
#define X_MBR_DUP   (X_MBR_CMP << 1 | DUP_BIT)
#define X_PTR_DUP   (X_PTR_CMP << 1 | DUP_BIT)
#define L_CHA_DUP   (L_CHA_CMP << 1 | DUP_BIT)
#define L_STR_DUP   (L_STR_CMP << 1 | DUP_BIT)
#define L_VAL_DUP   (L_VAL_CMP << 1 | DUP_BIT)
#define L_COR_DUP   (L_COR_CMP << 1 | DUP_BIT)

#define UIN_CORR (sizeof(UINT)   == sizeof(long))
#define USH_CORR (sizeof(USHORT) == sizeof(long))
#define UCH_CORR (sizeof(UCHAR)  == sizeof(long))

#define CORRECT(u)       ((long)(u) + LONG_MIN)
#define CORR_IF(u, cond) ((cond) ? CORRECT(u) : (long)(u))

typedef enum avl_unbal {
   LEFTUNBAL,
   RIGHTUNBAL
} UNBAL;

typedef enum avl_depth {
   LESS,
   SAME
} DEPTH;

typedef enum avl_ins {
   NOT_INS,
   INS,
   INS_DEEPER
} INS_T;

#define PTRADD(ptr, offs) ((void *)((char *)(ptr) + (offs)))
#define PTRSUB(ptr, offs) ((void *)((char *)(ptr) - (offs)))

#define PTRPUSH(list, ptr) ( \
   *(void **)(ptr) = (list), \
            (list) = (ptr) \
)
#define PTRPOP(list, ptr) ( \
   (ptr)  = (list), \
   (list) = *(void **)(ptr) \
)

#define CMP(usrcmp, key, data, keyoffs, ind) ( \
   (ind) ? (*(usrcmp))((key), *(void **)PTRADD((data), (keyoffs))) \
         : (*(usrcmp))((key),           PTRADD((data), (keyoffs))) \
)
#define TAILCMP(keytail, data, keyoffs, ind) ( \
   (ind) ? strcmp((keytail), *(char **)PTRADD((data), (keyoffs)) + sizeof(long)) \
         : strcmp((keytail),  (char  *)PTRADD((data), (keyoffs)) + sizeof(long)) \
)

#define AVL_FREE_AND_NULL(ptr) (AVL_FREE(ptr), (ptr) = NULL)

#define SIZEOF_P   (sizeof(void *))
#define SIZEOF_L   (sizeof(long))
#define SIZEOF_I   (sizeof(int))
#define SIZEOF_P_L (MAX(SIZEOF_P, SIZEOF_L))
#define SIZEOF_P_I (MAX(SIZEOF_P, SIZEOF_I))

/*===========================================================================*/

static int floatcmp(void *p1, void *p2)
{
   float  *f1 = p1, *f2 = p2;
   return *f1 > *f2 ? 1 : (*f1 < *f2 ? -1 : 0);
}

/*---------------------------------------------------------------------------*/

static int doublecmp(void *p1, void *p2)
{
   double *d1 = p1, *d2 = p2;
   return *d1 > *d2 ? 1 : (*d1 < *d2 ? -1 : 0);
}

/*===========================================================================*/

TREE *avl_tree(int treetype, size_t keyoffs, CMPFUN usrcmp)
{
   TREE *tree;
   char bits;

   if (sizeof(void *) != sizeof(char *)) {
      return NULL;
   }
   if (keyoffs > USHRT_MAX || (treetype >= AVL_CHARS) != !usrcmp) {
      return NULL;
   }
   switch (treetype >> 1) {
   CASE USR_KEY: bits = (X_USR_CMP << 1) | (treetype & 1);
   CASE MBR_KEY: bits = ((keyoffs ? X_MBR_CMP : X_USR_CMP) << 1) | (treetype & 1);
   CASE PTR_KEY: bits = (X_PTR_CMP << 1) | (treetype & 1);
   CASE CHA_KEY: bits = (L_CHA_CMP << 1) | (treetype & 1);
   CASE STR_KEY: bits = (L_STR_CMP << 1) | (treetype & 1);
   CASE LNG_KEY: bits = (L_VAL_CMP << 1) | (treetype & 1);
   CASE INT_KEY: bits = (L_VAL_CMP << 1) | (treetype & 1);
   CASE SHT_KEY: bits = (L_VAL_CMP << 1) | (treetype & 1);
   CASE SCH_KEY: bits = (L_VAL_CMP << 1) | (treetype & 1);
   CASE ULN_KEY: bits = (L_COR_CMP << 1) | (treetype & 1);
   CASE UIN_KEY: bits = ((UIN_CORR ? L_COR_CMP : L_VAL_CMP) << 1) | (treetype & 1);
   CASE USH_KEY: bits = ((USH_CORR ? L_COR_CMP : L_VAL_CMP) << 1) | (treetype & 1);
   CASE UCH_KEY: bits = ((UCH_CORR ? L_COR_CMP : L_VAL_CMP) << 1) | (treetype & 1);
   CASE FLT_KEY:
      if (avl_has_fast_floats())  { bits = (L_VAL_CMP << 1) | (treetype & 1); }
      else { usrcmp =  floatcmp;    bits = (X_MBR_CMP << 1) | (treetype & 1); }
   CASE DBL_KEY:
      if (avl_has_fast_doubles()) { bits = (L_VAL_CMP << 1) | (treetype & 1); }
      else { usrcmp =  doublecmp;   bits = (X_MBR_CMP << 1) | (treetype & 1); }
   DEFAULT:
      return NULL;
   }
   tree = AVL_MALLOC(sizeof(*tree));
   if ( !tree) return NULL;
   tree->root    = NULL;
   tree->usrcmp  = usrcmp;
   tree->path    = NULL;
   tree->unused  = NULL;
   tree->store   = NULL;
   tree->nodes   = 0;
   tree->alloc   = 0;
   tree->avail   = 0;
   tree->keyoffs = (USHORT)keyoffs;
   tree->bits    = bits;
   tree->type    = (char)treetype;
   return tree;
}

/*===========================================================================*/

static long l_key_of(char *str, char **p_keytail)
{
   char *c;
   int   o;
   long  key = 0L;

   o = (int)(sizeof(long) - 1) * CHAR_BIT;
   for (c = str; *c; c++) {
      key |= (long)(UCHAR)*c << o;
      if (o == 0) {
         if (p_keytail) *p_keytail = str + sizeof(long);
         return CORRECT(key);
      }
      o -= CHAR_BIT;
   }
   if (p_keytail) *p_keytail = NULL;
   return CORRECT(key);
}

/*---------------------------------------------------------------------------*/

static long flt2lng(float f)
{
   union { float f; long l; int i; } u;

   if (f > 0) {
      u.f = f;
      return sizeof(float) == sizeof(long) ?  u.l :  u.i;
   } else if (f < 0) {
      u.f = -f;
      return sizeof(float) == sizeof(long) ? -u.l : -u.i;
   } else {
      return 0; /* avoid -0.0 */
   }
}

/*---------------------------------------------------------------------------*/

static long dbl2lng(double d)
{
   union { double d; long l; int i; } u;

   if (d > 0) {
      u.d = d;
      return sizeof(double) == sizeof(long) ?  u.l :  u.i;
   } else if (d < 0) {
      u.d = -d;
      return sizeof(double) == sizeof(long) ? -u.l : -u.i;
   } else {
      return 0; /* avoid -0.0 */
   }
}

/*---------------------------------------------------------------------------*/

bool avl_has_fast_floats(void)
{
   return (sizeof(float) == sizeof(long) || sizeof(float) == sizeof(int)) &&
          flt2lng(1.00390625F) == 0x3F808000L;
}

/*---------------------------------------------------------------------------*/

bool avl_has_fast_doubles(void)
{
#if LONG_MAX >= 0x7FFFFFFFFFFFFFFF
   return (sizeof(double) == sizeof(long) || sizeof(double) == sizeof(int)) &&
          dbl2lng(1.000000476837158203125) == 0x3FF0000080000000L;
#else
   return false;
#endif
}

/*===========================================================================*/

static DEPTH rebalance_x(X_NODE **p_root, UNBAL unbal)
{
   X_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   X_NODE *root_left, *root_right, *half, *newroot;

   if (unbal == LEFTUNBAL) {
      root_left = PTR_OF(root->left);
      if (IS_DEEPER(root_left->left)) {
         /* simple rotation, tree depth decreased */
         newroot = root_left;
         root->leftval  = newroot->rightval & ~DEEPER;
         root->rightval &= ~DEEPER;
         newroot->right = root;
         newroot->leftval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else if (IS_DEEPER(root_left->right)) {
         /* double rotation */
         half    = root_left;
         newroot = PTR_OF(half->right);
         root->left  = newroot->right;
         half->right = newroot->left;
         if (IS_DEEPER(newroot->left)) {
            root->rightval |=  DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
         } else if (IS_DEEPER(newroot->right)) {
            root->rightval &= ~DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
            half->leftval  |=  DEEPER;
         } else {
            root->rightval &= ~DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
         }
         newroot->left  = half;
         newroot->right = root;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else {
         /* simple rotation, tree depth unchanged */
         newroot = root_left;
         root->leftval = newroot->rightval | DEEPER;
         root->rightval &= ~DEEPER;
         newroot->rightval = VAL_OF(root) | DEEPER;
         newroot->leftval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return SAME;
      }
   } else {
      root_right = PTR_OF(root->right);
      if (IS_DEEPER(root_right->right)) {
         /* simple rotation, tree depth decreased */
         newroot = root_right;
         root->rightval = newroot->leftval & ~DEEPER;
         root->leftval &= ~DEEPER;
         newroot->left = root;
         newroot->rightval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else if (IS_DEEPER(root_right->left)) {
         /* double rotation */
         half    = root_right;
         newroot = PTR_OF(half->left);
         root->right = newroot->left;
         half->left  = newroot->right;
         if (IS_DEEPER(newroot->right)) {
            root->leftval  |=  DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
         } else if (IS_DEEPER(newroot->left)) {
            root->leftval  &= ~DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
            half->rightval |=  DEEPER;
         } else {
            root->leftval  &= ~DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
         }
         newroot->right = half;
         newroot->left  = root;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else {
         /* simple rotation, tree depth unchanged */
         newroot = root_right;
         root->rightval = newroot->leftval | DEEPER;
         root->leftval &= ~DEEPER;
         newroot->leftval = VAL_OF(root) | DEEPER;
         newroot->rightval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return SAME;
      }
   }
}

/*---------------------------------------------------------------------------*/

static DEPTH rebalance_l(L_NODE **p_root, UNBAL unbal)
{
   L_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   L_NODE *root_left, *root_right, *half, *newroot;

   if (unbal == LEFTUNBAL) {
      root_left = PTR_OF(root->left);
      if (IS_DEEPER(root_left->left)) {
         /* simple rotation, tree depth decreased */
         newroot = root_left;
         root->leftval  = newroot->rightval & ~DEEPER;
         root->rightval &= ~DEEPER;
         newroot->right = root;
         newroot->leftval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else if (IS_DEEPER(root_left->right)) {
         /* double rotation */
         half    = root_left;
         newroot = PTR_OF(half->right);
         root->left  = newroot->right;
         half->right = newroot->left;
         if (IS_DEEPER(newroot->left)) {
            root->rightval |=  DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
         } else if (IS_DEEPER(newroot->right)) {
            root->rightval &= ~DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
            half->leftval  |=  DEEPER;
         } else {
            root->rightval &= ~DEEPER;
            root->leftval  &= ~DEEPER;
            half->rightval &= ~DEEPER;
         }
         newroot->left  = half;
         newroot->right = root;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else {
         /* simple rotation, tree depth unchanged */
         newroot = root_left;
         root->leftval = newroot->rightval | DEEPER;
         root->rightval &= ~DEEPER;
         newroot->rightval = VAL_OF(root) | DEEPER;
         newroot->leftval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return SAME;
      }
   } else {
      root_right = PTR_OF(root->right);
      if (IS_DEEPER(root_right->right)) {
         /* simple rotation, tree depth decreased */
         newroot = root_right;
         root->rightval = newroot->leftval & ~DEEPER;
         root->leftval &= ~DEEPER;
         newroot->left = root;
         newroot->rightval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else if (IS_DEEPER(root_right->left)) {
         /* double rotation */
         half    = root_right;
         newroot = PTR_OF(half->left);
         root->right = newroot->left;
         half->left  = newroot->right;
         if (IS_DEEPER(newroot->right)) {
            root->leftval  |=  DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
         } else if (IS_DEEPER(newroot->left)) {
            root->leftval  &= ~DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
            half->rightval |=  DEEPER;
         } else {
            root->leftval  &= ~DEEPER;
            root->rightval &= ~DEEPER;
            half->leftval  &= ~DEEPER;
         }
         newroot->right = half;
         newroot->left  = root;
         *p_root = PTRADD(newroot, root_deeper);
         return LESS;
      } else {
         /* simple rotation, tree depth unchanged */
         newroot = root_right;
         root->rightval = newroot->leftval | DEEPER;
         root->leftval &= ~DEEPER;
         newroot->leftval = VAL_OF(root) | DEEPER;
         newroot->rightval &= ~DEEPER;
         *p_root = PTRADD(newroot, root_deeper);
         return SAME;
      }
   }
}

/*===========================================================================*/

static INS_T insert_x(X_NODE **p_root, X_NODE *node, void *x_key, UINT keyoffs, bool ind, CMPFUN usrcmp, bool dup)
{
   X_NODE *root = PTR_OF(*p_root);
   int     cmp;
   INS_T   ins;

   cmp = CMP(usrcmp, x_key, root->data, keyoffs, ind);
   if (cmp < 0) {
      if (root->left) {
         ins = insert_x(&root->left, node, x_key, keyoffs, ind, usrcmp, dup);
      } else {
         root->left = PTRADD(node, IS_DEEPER(root->left));
         ins = INS_DEEPER;
      }
      switch (ins) {
      CASE INS_DEEPER:
         if (IS_DEEPER(root->left)) {
            return rebalance_x(p_root, LEFTUNBAL) == LESS ? INS : INS_DEEPER;
         } else if (IS_DEEPER(root->right)) {
            root->rightval &= ~DEEPER;
            return INS;
         } else {
            root->leftval |= DEEPER;
            return INS_DEEPER;
         }
      CASE INS:
         return INS;
      DEFAULT:
         return NOT_INS;
      }
   } else if (cmp > 0 || dup) {
      if (root->right) {
         ins = insert_x(&root->right, node, x_key, keyoffs, ind, usrcmp, dup);
      } else {
         root->right = PTRADD(node, IS_DEEPER(root->right));
         ins = INS_DEEPER;
      }
      switch (ins) {
      CASE INS_DEEPER:
         if (IS_DEEPER(root->right)) {
            return rebalance_x(p_root, RIGHTUNBAL) == LESS ? INS : INS_DEEPER;
         } else if (IS_DEEPER(root->left)) {
            root->leftval &= ~DEEPER;
            return INS;
         } else {
            root->rightval |= DEEPER;
            return INS_DEEPER;
         }
      CASE INS:
         return INS;
      DEFAULT:
         return NOT_INS;
      }
   } else {
      return NOT_INS;
   }
}

/*---------------------------------------------------------------------------*/

static INS_T insert_l(L_NODE **p_root, L_NODE *node, char *keytail, UINT keyoffs, bool ind, bool dup)
{
   L_NODE *root = PTR_OF(*p_root);
   INS_T   ins;

   if (node->key < root->key) {
      insert_lt:
      if (root->left) {
         ins = insert_l(&root->left, node, keytail, keyoffs, ind, dup);
      } else {
         root->left = PTRADD(node, IS_DEEPER(root->left));
         ins = INS_DEEPER;
      }
      switch (ins) {
      CASE INS_DEEPER:
         if (IS_DEEPER(root->left)) {
            return rebalance_l(p_root, LEFTUNBAL) == LESS ? INS : INS_DEEPER;
         } else if (IS_DEEPER(root->right)) {
            root->rightval &= ~DEEPER;
            return INS;
         } else {
            root->leftval |= DEEPER;
            return INS_DEEPER;
         }
      CASE INS:
         return INS;
      DEFAULT:
         return NOT_INS;
      }
   } else if (node->key > root->key) {
      insert_gt_or_dup:
      if (root->right) {
         ins = insert_l(&root->right, node, keytail, keyoffs, ind, dup);
      } else {
         root->right = PTRADD(node, IS_DEEPER(root->right));
         ins = INS_DEEPER;
      }
      switch (ins) {
      CASE INS_DEEPER:
         if (IS_DEEPER(root->right)) {
            return rebalance_l(p_root, RIGHTUNBAL) == LESS ? INS : INS_DEEPER;
         } else if (IS_DEEPER(root->left)) {
            root->leftval &= ~DEEPER;
            return INS;
         } else {
            root->rightval |= DEEPER;
            return INS_DEEPER;
         }
      CASE INS:
         return INS;
      DEFAULT:
         return NOT_INS;
      }
   } else if ( !keytail) {
      if (dup) {
         goto insert_gt_or_dup;
      } else {
         return NOT_INS;
      }
   } else {
      int cmp = TAILCMP(keytail, root->data, keyoffs, ind);
      if (cmp < 0) {
         goto insert_lt;
      } else if (cmp > 0 || dup) {
         goto insert_gt_or_dup;
      } else {
         return NOT_INS;
      }
   }
}

/*---------------------------------------------------------------------------*/

static X_NODE *alloc_node_x(TREE *tree)
{
   X_NODE *x_node;
   void   *old_alloc_base, *new_alloc_base;
   int     increment;

   increment = (tree->alloc >> AVL_NODE_INCREMENT_SHIFT) + 1;
   if (increment > AVL_NODE_INCREMENT_MAX) increment = AVL_NODE_INCREMENT_MAX;
   old_alloc_base = tree->x_store ? PTRSUB(tree->x_store, SIZEOF_P) : NULL;
   new_alloc_base = AVL_MALLOC(SIZEOF_P + increment * sizeof(X_NODE));
   if ( !new_alloc_base) return NULL;
   tree->alloc += increment;
   tree->avail = increment - 1;
   *(void **)new_alloc_base = old_alloc_base;
   x_node = ((X_NODE *)PTRADD(new_alloc_base, SIZEOF_P)) + (increment - 1);
   tree->x_store = x_node;
   return x_node;
}

/*---------------------------------------------------------------------------*/

static L_NODE *alloc_node_l(TREE *tree)
{
   L_NODE *l_node;
   void   *old_alloc_base, *new_alloc_base;
   int     increment;

   increment = (tree->alloc >> AVL_NODE_INCREMENT_SHIFT) + 1;
   if (increment > AVL_NODE_INCREMENT_MAX) increment = AVL_NODE_INCREMENT_MAX;
   old_alloc_base = tree->l_store ? PTRSUB(tree->l_store, SIZEOF_P_L) : NULL;
   new_alloc_base = AVL_MALLOC(SIZEOF_P_L + increment * sizeof(L_NODE));
   if ( !new_alloc_base) return NULL;
   tree->alloc += increment;
   tree->avail = increment - 1;
   *(void **)new_alloc_base = old_alloc_base;
   l_node = ((L_NODE *)PTRADD(new_alloc_base, SIZEOF_P_L)) + (increment - 1);
   tree->l_store = l_node;
   return l_node;
}

/*---------------------------------------------------------------------------*/

bool avl_insert(TREE *tree, void *data)
{
   X_NODE *x_node;
   L_NODE *l_node;
   void   *x_key;
   char   *keytail;

   if (tree->path) {
      AVL_FREE_AND_NULL(tree->path);
   }
   if (tree->nodes < 0) {
      return false;
   }
   if (IS_X(tree)) {
      if (tree->unused) {
         PTRPOP(tree->unused, x_node);
      } else if (tree->avail) {
         x_node = --tree->x_store;
         tree->avail--;
      } else {
         x_node = alloc_node_x(tree);
         if ( !x_node) return false;
      }
      x_node->data  = data;
      x_node->left  = NULL;
      x_node->right = NULL;
      switch (KEYTYPE(tree)) {
      CASE USR_KEY: x_key = data;
      CASE MBR_KEY:
      case FLT_KEY:
      case DBL_KEY: x_key =           PTRADD(data, tree->keyoffs);
      CASE PTR_KEY: x_key = *(void **)PTRADD(data, tree->keyoffs);
      DEFAULT: return false;
      }
      if (tree->x_root) {
         if (insert_x(&tree->x_root, x_node, x_key, tree->keyoffs, IS_IND(tree), tree->usrcmp, IS_DUP(tree)) == NOT_INS) {
            PTRPUSH(tree->unused, x_node);
            return false;
         }
      } else {
         tree->x_root = x_node;
      }
   } else {
      if (tree->unused) {
         PTRPOP(tree->unused, l_node);
      } else if (tree->avail) {
         l_node = --tree->l_store;
         tree->avail--;
      } else {
         l_node = alloc_node_l(tree);
         if ( !l_node) return false;
      }
      l_node->data  = data;
      l_node->left  = NULL;
      l_node->right = NULL;
      keytail = NULL;
      switch (KEYTYPE(tree)) {
      CASE CHA_KEY: l_node->key = l_key_of( (char  *)PTRADD(data, tree->keyoffs), &keytail);
      CASE STR_KEY: l_node->key = l_key_of(*(char **)PTRADD(data, tree->keyoffs), &keytail);
      CASE LNG_KEY: l_node->key =          *(long  *)PTRADD(data, tree->keyoffs);
      CASE INT_KEY: l_node->key =          *(int   *)PTRADD(data, tree->keyoffs);
      CASE SHT_KEY: l_node->key =          *(short *)PTRADD(data, tree->keyoffs);
      CASE SCH_KEY: l_node->key =          *(SCHAR *)PTRADD(data, tree->keyoffs);
      CASE ULN_KEY: l_node->key =  CORRECT(*(ULONG *)PTRADD(data, tree->keyoffs));
      CASE UIN_KEY: l_node->key =  CORR_IF(*(UINT  *)PTRADD(data, tree->keyoffs), UIN_CORR);
      CASE USH_KEY: l_node->key =  CORR_IF(*(USHORT*)PTRADD(data, tree->keyoffs), USH_CORR);
      CASE UCH_KEY: l_node->key =  CORR_IF(*(UCHAR *)PTRADD(data, tree->keyoffs), UCH_CORR);
      CASE FLT_KEY: l_node->key =  flt2lng(*(float *)PTRADD(data, tree->keyoffs));
      CASE DBL_KEY: l_node->key =  dbl2lng(*(double*)PTRADD(data, tree->keyoffs));
      DEFAULT: return false;
      }
      if (tree->l_root) {
         if (insert_l(&tree->l_root, l_node, keytail, tree->keyoffs, IS_IND(tree), IS_DUP(tree)) == NOT_INS) {
            PTRPUSH(tree->unused, l_node);
            return false;
         }
      } else {
         tree->l_root = l_node;
      }
   }
   tree->nodes++;
   return true;
}

/*===========================================================================*/

static X_NODE *fetch_leftmost_x(X_NODE **p_root, DEPTH *depth)
{
   X_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   X_NODE *node;

   if (root) {
      if (root->left) {
         node = fetch_leftmost_x(&root->left, depth);
         if (*depth == LESS) {
            /* left subtree depth decreased */
            if (IS_DEEPER(root->rightval)) {
               *depth = rebalance_x(p_root, RIGHTUNBAL);
            } else if (IS_DEEPER(root->leftval)) {
               root->leftval  &= ~DEEPER;
            } else {
               root->rightval |=  DEEPER;
               *depth = SAME;
            }
         }
         return node;
      } else {
         *p_root = PTRADD(PTR_OF(root->right), root_deeper);
         *depth = LESS;
         return root;
      }
   } else {
      *depth = SAME;
      return NULL;
   }
}

/*---------------------------------------------------------------------------*/

static L_NODE *fetch_leftmost_l(L_NODE **p_root, DEPTH *depth)
{
   L_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   L_NODE *node;

   if (root) {
      if (root->left) {
         node = fetch_leftmost_l(&root->left, depth);
         if (*depth == LESS) {
            /* left subtree depth decreased */
            if (IS_DEEPER(root->rightval)) {
               *depth = rebalance_l(p_root, RIGHTUNBAL);
            } else if (IS_DEEPER(root->leftval)) {
               root->leftval  &= ~DEEPER;
            } else {
               root->rightval |=  DEEPER;
               *depth = SAME;
            }
         }
         return node;
      } else {
         *p_root = PTRADD(PTR_OF(root->right), root_deeper);
         *depth = LESS;
         return root;
      }
   } else {
      *depth = SAME;
      return NULL;
   }
}

/*---------------------------------------------------------------------------*/

static X_NODE *remove_x(X_NODE **p_root, void *x_key, UINT keyoffs, bool ind, CMPFUN usrcmp, bool dup, DEPTH *depth)
{
   X_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   X_NODE *node;
   int     cmp;

   cmp = CMP(usrcmp, x_key, root->data, keyoffs, ind);
   if (cmp < 0) {
      if ( !root->left) {
         return NULL;
      }
      node = remove_x(&root->left, x_key, keyoffs, ind, usrcmp, dup, depth);
      if (node && *depth == LESS) {
         /* left subtree depth decreased */
         if (IS_DEEPER(root->right)) {
            *depth = rebalance_x(p_root, RIGHTUNBAL);
         } else if (IS_DEEPER(root->left)) {
            root->leftval  &= ~DEEPER;
         } else {
            root->rightval |=  DEEPER;
            *depth = SAME;
         }
      }
      return node;
   } else if (cmp > 0) {
      if ( !root->right) {
         return NULL;
      }
      node = remove_x(&root->right, x_key, keyoffs, ind, usrcmp, dup, depth);
      if (node && *depth == LESS) {
         /* right subtree depth decreased */
         if (IS_DEEPER(root->left)) {
            *depth = rebalance_x(p_root, LEFTUNBAL);
         } else if (IS_DEEPER(root->right)) {
            root->rightval &= ~DEEPER;
         } else {
            root->leftval  |=  DEEPER;
            *depth = SAME;
         }
      }
      return node;
   } else {
      if (dup && root->left && (node = remove_x(&root->left, x_key, keyoffs, ind, usrcmp, dup, depth))) {
         if (*depth == LESS) {
            /* left subtree depth decreased */
            if (IS_DEEPER(root->right)) {
               *depth = rebalance_x(p_root, RIGHTUNBAL);
            } else if (IS_DEEPER(root->left)) {
               root->leftval  &= ~DEEPER;
            } else {
               root->rightval |=  DEEPER;
               *depth = SAME;
            }
         }
      } else {
         node = root;
         if ( !node->right) {
            *p_root = PTRADD(PTR_OF(node->left), root_deeper);
            *depth = LESS;
         } else if ( !node->left) {
            *p_root = PTRADD(PTR_OF(node->right), root_deeper);
            *depth = LESS;
         } else {
            /* replace by the leftmost node of the right subtree */
            root = fetch_leftmost_x(&node->right, depth);
            root->left  = node->left;
            root->right = node->right;
            if (*depth == LESS) {
               /* right subtree depth decreased */
               if (IS_DEEPER(root->left)) {
                  *depth = rebalance_x(&root, LEFTUNBAL);
               } else if (IS_DEEPER(root->right)) {
                  root->rightval &= ~DEEPER;
               } else {
                  root->leftval  |=  DEEPER;
                  *depth = SAME;
               }
            } else {
               *depth = SAME;
            }
            *p_root = PTRADD(root, root_deeper);
         }
      }
      return node;
   }
}

/*---------------------------------------------------------------------------*/

static L_NODE *remove_l(L_NODE **p_root, long l_key, char *keytail, UINT keyoffs, bool ind, bool dup, DEPTH *depth)
{
   L_NODE *root        = PTR_OF   (*p_root);
   UINTPTR root_deeper = IS_DEEPER(*p_root);
   L_NODE *node;

   if (l_key < root->key) {
      remove_lt:
      if ( !root->left) {
         return NULL;
      }
      node = remove_l(&root->left, l_key, keytail, keyoffs, ind, dup, depth);
      if (node && *depth == LESS) {
         /* left subtree depth decreased */
         if (IS_DEEPER(root->right)) {
            *depth = rebalance_l(p_root, RIGHTUNBAL);
         } else if (IS_DEEPER(root->left)) {
            root->leftval  &= ~DEEPER;
         } else {
            root->rightval |=  DEEPER;
            *depth = SAME;
         }
      }
      return node;
   } else if (l_key > root->key) {
      remove_gt:
      if ( !root->right) {
         return NULL;
      }
      node = remove_l(&root->right, l_key, keytail, keyoffs, ind, dup, depth);
      if (node && *depth == LESS) {
         /* right subtree depth decreased */
         if (IS_DEEPER(root->left)) {
            *depth = rebalance_l(p_root, LEFTUNBAL);
         } else if (IS_DEEPER(root->right)) {
            root->rightval &= ~DEEPER;
         } else {
            root->leftval  |=  DEEPER;
            *depth = SAME;
         }
      }
      return node;
   } else if ( !keytail) {
      remove_eq:
      if (dup && root->left && (node = remove_l(&root->left, l_key, keytail, keyoffs, ind, dup, depth))) {
         if (*depth == LESS) {
            /* left subtree depth decreased */
            if (IS_DEEPER(root->right)) {
               *depth = rebalance_l(p_root, RIGHTUNBAL);
            } else if (IS_DEEPER(root->left)) {
               root->leftval  &= ~DEEPER;
            } else {
               root->rightval |=  DEEPER;
               *depth = SAME;
            }
         }
      } else {
         node = root;
         if ( !node->right) {
            *p_root = PTRADD(PTR_OF(node->left), root_deeper);
            *depth = LESS;
         } else if ( !node->left) {
            *p_root = PTRADD(PTR_OF(node->right), root_deeper);
            *depth = LESS;
         } else {
            /* replace by the leftmost node of the right subtree */
            root = fetch_leftmost_l(&node->right, depth);
            root->left  = node->left;
            root->right = node->right;
            if (*depth == LESS) {
               /* right subtree depth decreased */
               if (IS_DEEPER(root->left)) {
                  *depth = rebalance_l(&root, LEFTUNBAL);
               } else if (IS_DEEPER(root->right)) {
                  root->rightval &= ~DEEPER;
               } else {
                  root->leftval  |=  DEEPER;
                  *depth = SAME;
               }
            } else {
               *depth = SAME;
            }
            *p_root = PTRADD(root, root_deeper);
         }
      }
      return node;
   } else {
      int cmp = TAILCMP(keytail, root->data, keyoffs, ind);
      if (cmp < 0) {
         goto remove_lt;
      } else if (cmp > 0) {
         goto remove_gt;
      } else {
         goto remove_eq;
      }
   }
}

/*---------------------------------------------------------------------------*/

void *avl_remove(TREE *tree, void *key)
{
   X_NODE *x_node = NULL;
   L_NODE *l_node = NULL;
   long    l_key;
   char   *keytail;
   void   *data;
   DEPTH   depth;

   if (tree->path) {
      AVL_FREE_AND_NULL(tree->path);
   }
   if (tree->root) {
      if (IS_X(tree)) {
         x_node = remove_x(&tree->x_root, key, tree->keyoffs, IS_IND(tree), tree->usrcmp, IS_DUP(tree), &depth);
         if ( !x_node) return NULL;
         data = x_node->data;
         PTRPUSH(tree->unused, x_node);
      } else {
         l_key = l_key_of(key, &keytail);
         l_node = remove_l(&tree->l_root, l_key, keytail, tree->keyoffs, IS_IND(tree), IS_DUP(tree), &depth);
         if ( !l_node) return NULL;
         data = l_node->data;
         PTRPUSH(tree->unused, l_node);
      }
      tree->nodes--;
      return data;
   }
   return NULL;
}

void *avl_remove_mbr  (TREE *tree, void *key) { return avl_remove(tree, key); }
void *avl_remove_chars(TREE *tree, char *key) { return avl_remove(tree, key); }
void *avl_remove_ptr  (TREE *tree, void *key) { return avl_remove(tree, key); }
void *avl_remove_str  (TREE *tree, char *key) { return avl_remove(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_remove_long(TREE *tree, long key)
{
   L_NODE *l_node;
   void   *data;
   DEPTH   depth;

   if (tree->path) {
      AVL_FREE_AND_NULL(tree->path);
   }
   if (tree->root) {
      if (IS_X(tree)) {
         return NULL;
      }
      if (IS_CORR(tree)) {
         key = CORRECT(key);
      }
      l_node = remove_l(&tree->l_root, key, NULL, tree->keyoffs, IS_IND(tree), IS_DUP(tree), &depth);
      if ( !l_node) return NULL;
      data = l_node->data;
      PTRPUSH(tree->unused, l_node);
      tree->nodes--;
      return data;
   }
   return NULL;
}

void *avl_remove_int   (TREE *tree, int    key) { return avl_remove_long(tree, key); }
void *avl_remove_short (TREE *tree, short  key) { return avl_remove_long(tree, key); }
void *avl_remove_schar (TREE *tree, SCHAR  key) { return avl_remove_long(tree, key); }
void *avl_remove_ulong (TREE *tree, ULONG  key) { return avl_remove_long(tree, key); }
void *avl_remove_uint  (TREE *tree, UINT   key) { return avl_remove_long(tree, key); }
void *avl_remove_ushort(TREE *tree, USHORT key) { return avl_remove_long(tree, key); }
void *avl_remove_uchar (TREE *tree, UCHAR  key) { return avl_remove_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_remove_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_remove     (tree, &key);
   else            return avl_remove_long(tree, flt2lng(key));
}

void *avl_remove_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_remove     (tree, &key);
   else            return avl_remove_long(tree, dbl2lng(key));
}

/*===========================================================================*/

#define CONTINUE_LEFT( node)       ((node) = PTR_OF((node)->left))
#define CONTINUE_RIGHT(node)       ((node) = PTR_OF((node)->right))
#define SAVE_AND_LEFT( node, save) ((save) = (node), CONTINUE_LEFT( node))
#define SAVE_AND_RIGHT(node, save) ((save) = (node), CONTINUE_RIGHT(node))

/*---------------------------------------------------------------------------*/

void *avl_locate(TREE *tree, void *key)
{
   X_NODE *x_node, *x_save;
   L_NODE *l_node, *l_save;
   int     cmp;
   long    l_key;
   char   *keytail;

   switch (tree->bits) {
   CASE X_USR_NODUP:
      for (x_node = tree->x_root; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
   CASE X_MBR_NODUP:
      for (x_node = tree->x_root; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
   CASE X_PTR_NODUP:
      for (x_node = tree->x_root; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
   CASE L_CHA_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if      (cmp < 0) CONTINUE_LEFT (l_node);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
   CASE L_STR_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if      (cmp < 0) CONTINUE_LEFT (l_node);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
   CASE X_USR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_MBR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_PTR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if      (cmp < 0) CONTINUE_LEFT (x_node);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE L_CHA_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if      (cmp < 0) CONTINUE_LEFT (l_node);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              SAVE_AND_LEFT (l_node, l_save);
         } else               SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   CASE L_STR_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if      (cmp < 0) CONTINUE_LEFT (l_node);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              SAVE_AND_LEFT (l_node, l_save);
         } else               SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_mbr  (TREE *tree, void *key) { return avl_locate(tree, key); }
void *avl_locate_ptr  (TREE *tree, void *key) { return avl_locate(tree, key); }
void *avl_locate_chars(TREE *tree, char *key) { return avl_locate(tree, key); }
void *avl_locate_str  (TREE *tree, char *key) { return avl_locate(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_long(TREE *tree, long key)
{
   L_NODE *l_node, *l_save;

   switch (tree->bits) {
   CASE L_COR_NODUP:
      key = CORRECT(key);
   case L_VAL_NODUP:
      for (l_node = tree->l_root; l_node; ) {
         if      (key < l_node->key) CONTINUE_LEFT (l_node);
         else if (key > l_node->key) CONTINUE_RIGHT(l_node);
         else return l_node->data;
      }
   CASE L_COR_DUP:
      key = CORRECT(key);
   case L_VAL_DUP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (key < l_node->key) CONTINUE_LEFT (l_node);
         else if (key > l_node->key) CONTINUE_RIGHT(l_node);
         else                        SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_int   (TREE *tree, int    key) { return avl_locate_long(tree, key); }
void *avl_locate_short (TREE *tree, short  key) { return avl_locate_long(tree, key); }
void *avl_locate_schar (TREE *tree, SCHAR  key) { return avl_locate_long(tree, key); }
void *avl_locate_ulong (TREE *tree, ULONG  key) { return avl_locate_long(tree, key); }
void *avl_locate_uint  (TREE *tree, UINT   key) { return avl_locate_long(tree, key); }
void *avl_locate_ushort(TREE *tree, USHORT key) { return avl_locate_long(tree, key); }
void *avl_locate_uchar (TREE *tree, UCHAR  key) { return avl_locate_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_locate     (tree, &key);
   else            return avl_locate_long(tree, flt2lng(key));
}

void *avl_locate_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_locate     (tree, &key);
   else            return avl_locate_long(tree, dbl2lng(key));
}

/*---------------------------------------------------------------------------*/

void *avl_locate_ge(TREE *tree, void *key)
{
   X_NODE *x_node, *x_save;
   L_NODE *l_node, *l_save;
   int     cmp;
   long    l_key;
   char   *keytail;

   switch (tree->bits) {
   CASE X_USR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if      (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE X_MBR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if      (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE X_PTR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if      (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else if (cmp > 0) CONTINUE_RIGHT(x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE L_CHA_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if      (cmp < 0) SAVE_AND_LEFT (l_node, l_save);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE L_STR_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if      (cmp < 0) SAVE_AND_LEFT (l_node, l_save);
            else if (cmp > 0) CONTINUE_RIGHT(l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE X_USR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if (cmp > 0) CONTINUE_RIGHT(x_node);
         else         SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_MBR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if (cmp > 0) CONTINUE_RIGHT(x_node);
         else         SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_PTR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if (cmp > 0) CONTINUE_RIGHT(x_node);
         else         SAVE_AND_LEFT (x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE L_CHA_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if (cmp > 0) CONTINUE_RIGHT(l_node);
            else         SAVE_AND_LEFT (l_node, l_save);
         } else          SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   CASE L_STR_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if (cmp > 0) CONTINUE_RIGHT(l_node);
            else         SAVE_AND_LEFT (l_node, l_save);
         } else          SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_ge_mbr  (TREE *tree, void *key) { return avl_locate_ge(tree, key); }
void *avl_locate_ge_ptr  (TREE *tree, void *key) { return avl_locate_ge(tree, key); }
void *avl_locate_ge_chars(TREE *tree, char *key) { return avl_locate_ge(tree, key); }
void *avl_locate_ge_str  (TREE *tree, char *key) { return avl_locate_ge(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_ge_long(TREE *tree, long key)
{
   L_NODE *l_node, *l_save;

   switch (tree->bits) {
   CASE L_COR_NODUP:
      key = CORRECT(key);
   case L_VAL_NODUP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (key > l_node->key) CONTINUE_RIGHT(l_node);
         else                        return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE L_COR_DUP:
      key = CORRECT(key);
   case L_VAL_DUP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if (key > l_node->key) CONTINUE_RIGHT(l_node);
         else                   SAVE_AND_LEFT (l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_ge_int   (TREE *tree, int    key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_short (TREE *tree, short  key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_schar (TREE *tree, SCHAR  key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_ulong (TREE *tree, ULONG  key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_uint  (TREE *tree, UINT   key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_ushort(TREE *tree, USHORT key) { return avl_locate_ge_long(tree, key); }
void *avl_locate_ge_uchar (TREE *tree, UCHAR  key) { return avl_locate_ge_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_ge_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_locate_ge     (tree, &key);
   else            return avl_locate_ge_long(tree, flt2lng(key));
}

void *avl_locate_ge_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_locate_ge     (tree, &key);
   else            return avl_locate_ge_long(tree, dbl2lng(key));
}

/*---------------------------------------------------------------------------*/

void *avl_locate_gt(TREE *tree, void *key)
{
   X_NODE *x_node, *x_save;
   L_NODE *l_node, *l_save;
   int     cmp;
   long    l_key;
   char   *keytail;

   switch (CMPTYPE(tree)) {
   CASE X_USR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else         CONTINUE_RIGHT(x_node);
      }
      if (x_save) return x_save->data;
   CASE X_MBR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else         CONTINUE_RIGHT(x_node);
      }
      if (x_save) return x_save->data;
   CASE X_PTR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if (cmp < 0) SAVE_AND_LEFT (x_node, x_save);
         else         CONTINUE_RIGHT(x_node);
      }
      if (x_save) return x_save->data;
   CASE L_CHA_CMP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if (cmp < 0) SAVE_AND_LEFT (l_node, l_save);
            else         CONTINUE_RIGHT(l_node);
         } else          CONTINUE_RIGHT(l_node);
      }
      if (l_save) return l_save->data;
   CASE L_STR_CMP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else if (l_key > l_node->key) CONTINUE_RIGHT(l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if (cmp < 0) SAVE_AND_LEFT (l_node, l_save);
            else         CONTINUE_RIGHT(l_node);
         } else          CONTINUE_RIGHT(l_node);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_gt_mbr  (TREE *tree, void *key) { return avl_locate_gt(tree, key); }
void *avl_locate_gt_ptr  (TREE *tree, void *key) { return avl_locate_gt(tree, key); }
void *avl_locate_gt_chars(TREE *tree, char *key) { return avl_locate_gt(tree, key); }
void *avl_locate_gt_str  (TREE *tree, char *key) { return avl_locate_gt(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_gt_long(TREE *tree, long key)
{
   L_NODE *l_node, *l_save;

   switch (CMPTYPE(tree)) {
   CASE L_COR_CMP:
      key = CORRECT(key);
   case L_VAL_CMP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if (key < l_node->key) SAVE_AND_LEFT (l_node, l_save);
         else                   CONTINUE_RIGHT(l_node);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_gt_int   (TREE *tree, int    key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_short (TREE *tree, short  key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_schar (TREE *tree, SCHAR  key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_ulong (TREE *tree, ULONG  key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_uint  (TREE *tree, UINT   key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_ushort(TREE *tree, USHORT key) { return avl_locate_gt_long(tree, key); }
void *avl_locate_gt_uchar (TREE *tree, UCHAR  key) { return avl_locate_gt_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_gt_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_locate_gt     (tree, &key);
   else            return avl_locate_gt_long(tree, flt2lng(key));
}

void *avl_locate_gt_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_locate_gt     (tree, &key);
   else            return avl_locate_gt_long(tree, dbl2lng(key));
}

/*---------------------------------------------------------------------------*/

void *avl_locate_le(TREE *tree, void *key)
{
   X_NODE *x_node, *x_save;
   L_NODE *l_node, *l_save;
   int     cmp;
   long    l_key;
   char   *keytail;

   switch (tree->bits) {
   CASE X_USR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if      (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else if (cmp < 0) CONTINUE_LEFT (x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE X_MBR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if      (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else if (cmp < 0) CONTINUE_LEFT (x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE X_PTR_NODUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if      (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else if (cmp < 0) CONTINUE_LEFT (x_node);
         else              return x_node->data;
      }
      if (x_save) return x_save->data;
   CASE L_CHA_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if      (cmp > 0) SAVE_AND_RIGHT(l_node, l_save);
            else if (cmp < 0) CONTINUE_LEFT (l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE L_STR_NODUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if      (cmp > 0) SAVE_AND_RIGHT(l_node, l_save);
            else if (cmp < 0) CONTINUE_LEFT (l_node);
            else              return l_node->data;
         } else               return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE X_USR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if (cmp < 0) CONTINUE_LEFT (x_node);
         else         SAVE_AND_RIGHT(x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_MBR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if (cmp < 0) CONTINUE_LEFT (x_node);
         else         SAVE_AND_RIGHT(x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE X_PTR_DUP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if (cmp < 0) CONTINUE_LEFT (x_node);
         else         SAVE_AND_RIGHT(x_node, x_save);
      }
      if (x_save) return x_save->data;
   CASE L_CHA_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if (cmp < 0) CONTINUE_LEFT (l_node);
            else         SAVE_AND_RIGHT(l_node, l_save);
         } else          SAVE_AND_RIGHT(l_node, l_save);
      }
      if (l_save) return l_save->data;
   CASE L_STR_DUP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if (cmp < 0) CONTINUE_LEFT (l_node);
            else         SAVE_AND_RIGHT(l_node, l_save);
         } else          SAVE_AND_RIGHT(l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_le_mbr  (TREE *tree, void *key) { return avl_locate_le(tree, key); }
void *avl_locate_le_ptr  (TREE *tree, void *key) { return avl_locate_le(tree, key); }
void *avl_locate_le_chars(TREE *tree, char *key) { return avl_locate_le(tree, key); }
void *avl_locate_le_str  (TREE *tree, char *key) { return avl_locate_le(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_le_long(TREE *tree, long key)
{
   L_NODE *l_node, *l_save;

   switch (tree->bits) {
   CASE L_COR_NODUP:
      key = CORRECT(key);
   case L_VAL_NODUP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (key < l_node->key) CONTINUE_LEFT (l_node);
         else                        return l_node->data;
      }
      if (l_save) return l_save->data;
   CASE L_COR_DUP:
      key = CORRECT(key);
   case L_VAL_DUP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if (key < l_node->key) CONTINUE_LEFT (l_node);
         else                   SAVE_AND_RIGHT(l_node, l_save);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_le_int   (TREE *tree, int    key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_short (TREE *tree, short  key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_schar (TREE *tree, SCHAR  key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_ulong (TREE *tree, ULONG  key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_uint  (TREE *tree, UINT   key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_ushort(TREE *tree, USHORT key) { return avl_locate_le_long(tree, key); }
void *avl_locate_le_uchar (TREE *tree, UCHAR  key) { return avl_locate_le_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_le_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_locate_le     (tree, &key);
   else            return avl_locate_le_long(tree, flt2lng(key));
}

void *avl_locate_le_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_locate_le     (tree, &key);
   else            return avl_locate_le_long(tree, dbl2lng(key));
}

/*---------------------------------------------------------------------------*/

void *avl_locate_lt(TREE *tree, void *key)
{
   X_NODE *x_node, *x_save;
   L_NODE *l_node, *l_save;
   int     cmp;
   long    l_key;
   char   *keytail;

   switch (CMPTYPE(tree)) {
   CASE X_USR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, 0, false);
         if (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else         CONTINUE_LEFT (x_node);
      }
      if (x_save) return x_save->data;
   CASE X_MBR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, false);
         if (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else         CONTINUE_LEFT (x_node);
      }
      if (x_save) return x_save->data;
   CASE X_PTR_CMP:
      for (x_node = tree->x_root, x_save = NULL; x_node; ) {
         cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, true);
         if (cmp > 0) SAVE_AND_RIGHT(x_node, x_save);
         else         CONTINUE_LEFT (x_node);
      }
      if (x_save) return x_save->data;
   CASE L_CHA_CMP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, false);
            if (cmp > 0) SAVE_AND_RIGHT(l_node, l_save);
            else         CONTINUE_LEFT (l_node);
         } else          CONTINUE_LEFT (l_node);
      }
      if (l_save) return l_save->data;
   CASE L_STR_CMP:
      l_key = l_key_of(key, &keytail);
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if      (l_key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else if (l_key < l_node->key) CONTINUE_LEFT (l_node);
         else if (keytail) {
            cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, true);
            if (cmp > 0) SAVE_AND_RIGHT(l_node, l_save);
            else         CONTINUE_LEFT (l_node);
         } else          CONTINUE_LEFT (l_node);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_lt_mbr  (TREE *tree, void *key) { return avl_locate_lt(tree, key); }
void *avl_locate_lt_ptr  (TREE *tree, void *key) { return avl_locate_lt(tree, key); }
void *avl_locate_lt_chars(TREE *tree, char *key) { return avl_locate_lt(tree, key); }
void *avl_locate_lt_str  (TREE *tree, char *key) { return avl_locate_lt(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_lt_long(TREE *tree, long key)
{
   L_NODE *l_node, *l_save;

   switch (CMPTYPE(tree)) {
   CASE L_COR_CMP:
      key = CORRECT(key);
   case L_VAL_CMP:
      for (l_node = tree->l_root, l_save = NULL; l_node; ) {
         if (key > l_node->key) SAVE_AND_RIGHT(l_node, l_save);
         else                   CONTINUE_LEFT (l_node);
      }
      if (l_save) return l_save->data;
   }
   return NULL;
}

void *avl_locate_lt_int   (TREE *tree, int    key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_short (TREE *tree, short  key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_schar (TREE *tree, SCHAR  key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_ulong (TREE *tree, ULONG  key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_uint  (TREE *tree, UINT   key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_ushort(TREE *tree, USHORT key) { return avl_locate_lt_long(tree, key); }
void *avl_locate_lt_uchar (TREE *tree, UCHAR  key) { return avl_locate_lt_long(tree, key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_lt_float(TREE *tree, float key)
{
   if (IS_X(tree)) return avl_locate_lt     (tree, &key);
   else            return avl_locate_lt_long(tree, flt2lng(key));
}

void *avl_locate_lt_double(TREE *tree, double key)
{
   if (IS_X(tree)) return avl_locate_lt     (tree, &key);
   else            return avl_locate_lt_long(tree, dbl2lng(key));
}

/*---------------------------------------------------------------------------*/

void *avl_locate_first(TREE *tree)
{
   X_NODE *x_node;
   L_NODE *l_node;

   if (tree->root) {
      if (IS_X(tree)) {
         for (x_node = tree->root; x_node->left; CONTINUE_LEFT(x_node)) {}
         return x_node->data;
      } else {
         for (l_node = tree->root; l_node->left; CONTINUE_LEFT(l_node)) {}
         return l_node->data;
      }
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_locate_last(TREE *tree)
{
   X_NODE *x_node;
   L_NODE *l_node;

   if (tree->root) {
      if (IS_X(tree)) {
         for (x_node = tree->root; x_node->right; CONTINUE_RIGHT(x_node)) {}
         return x_node->data;
      } else {
         for (l_node = tree->root; l_node->right; CONTINUE_RIGHT(l_node)) {}
         return l_node->data;
      }
   }
   return NULL;
}

/*===========================================================================*/

static void *scan_x(X_NODE *x_root, bool (*callback)(void *))
{
   void *data;

   if (x_root->left  && (data = scan_x(PTR_OF(x_root->left),  callback))) return data;
   if ((*callback)(x_root->data)) return x_root->data;
   if (x_root->right && (data = scan_x(PTR_OF(x_root->right), callback))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *scan_l(L_NODE *l_root, bool (*callback)(void *))
{
   void *data;

   if (l_root->left  && (data = scan_l(PTR_OF(l_root->left),  callback))) return data;
   if ((*callback)(l_root->data)) return l_root->data;
   if (l_root->right && (data = scan_l(PTR_OF(l_root->right), callback))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *rev_scan_x(X_NODE *x_root, bool (*callback)(void *))
{
   void *data;

   if (x_root->right && (data = rev_scan_x(PTR_OF(x_root->right), callback))) return data;
   if ((*callback)(x_root->data)) return x_root->data;
   if (x_root->left  && (data = rev_scan_x(PTR_OF(x_root->left),  callback))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *rev_scan_l(L_NODE *l_root, bool (*callback)(void *))
{
   void *data;

   if (l_root->right && (data = rev_scan_l(PTR_OF(l_root->right), callback))) return data;
   if ((*callback)(l_root->data)) return l_root->data;
   if (l_root->left  && (data = rev_scan_l(PTR_OF(l_root->left),  callback))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_scan(TREE *tree, bool (*callback)(void *))
{
   if (tree->root) {
      if (IS_X(tree)) return scan_x(tree->x_root, callback);
      else            return scan_l(tree->l_root, callback);
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_rev_scan(TREE *tree, bool (*callback)(void *))
{
   if (tree->root) {
      if (IS_X(tree)) return rev_scan_x(tree->x_root, callback);
      else            return rev_scan_l(tree->l_root, callback);
   }
   return NULL;
}

/*===========================================================================*/

static void *scan_w_ctx_x(X_NODE *x_root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (x_root->left  && (data = scan_w_ctx_x(PTR_OF(x_root->left),  callback, context))) return data;
   if ((*callback)(x_root->data, context)) return x_root->data;
   if (x_root->right && (data = scan_w_ctx_x(PTR_OF(x_root->right), callback, context))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *scan_w_ctx_l(L_NODE *l_root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (l_root->left  && (data = scan_w_ctx_l(PTR_OF(l_root->left),  callback, context))) return data;
   if ((*callback)(l_root->data, context)) return l_root->data;
   if (l_root->right && (data = scan_w_ctx_l(PTR_OF(l_root->right), callback, context))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *rev_scan_w_ctx_x(X_NODE *x_root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (x_root->right && (data = rev_scan_w_ctx_x(PTR_OF(x_root->right), callback, context))) return data;
   if ((*callback)(x_root->data, context)) return x_root->data;
   if (x_root->left  && (data = rev_scan_w_ctx_x(PTR_OF(x_root->left),  callback, context))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *rev_scan_w_ctx_l(L_NODE *l_root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (l_root->right && (data = rev_scan_w_ctx_l(PTR_OF(l_root->right), callback, context))) return data;
   if ((*callback)(l_root->data, context)) return l_root->data;
   if (l_root->left  && (data = rev_scan_w_ctx_l(PTR_OF(l_root->left),  callback, context))) return data;
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_scan_w_ctx(TREE *tree, bool (*callback)(void *, void *), void *context)
{
   if (tree->root) {
      if (IS_X(tree)) return scan_w_ctx_x(tree->x_root, callback, context);
      else            return scan_w_ctx_l(tree->l_root, callback, context);
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_rev_scan_w_ctx(TREE *tree, bool (*callback)(void *, void *), void *context)
{
   if (tree->root) {
      if (IS_X(tree)) return rev_scan_w_ctx_x(tree->x_root, callback, context);
      else            return rev_scan_w_ctx_l(tree->l_root, callback, context);
   }
   return NULL;
}

/*===========================================================================*/

static void do_x(X_NODE *x_root, void (*callback)(void *))
{
   if (x_root->left)  do_x(PTR_OF(x_root->left),  callback);
   (*callback)(x_root->data);
   if (x_root->right) do_x(PTR_OF(x_root->right), callback);
}

/*---------------------------------------------------------------------------*/

static void do_l(L_NODE *l_root, void (*callback)(void *))
{
   if (l_root->left)  do_l(PTR_OF(l_root->left),  callback);
   (*callback)(l_root->data);
   if (l_root->right) do_l(PTR_OF(l_root->right), callback);
}

/*---------------------------------------------------------------------------*/

static void rev_do_x(X_NODE *x_root, void (*callback)(void *))
{
   if (x_root->right) rev_do_x(PTR_OF(x_root->right), callback);
   (*callback)(x_root->data);
   if (x_root->left)  rev_do_x(PTR_OF(x_root->left),  callback);
}

/*---------------------------------------------------------------------------*/

static void rev_do_l(L_NODE *l_root, void (*callback)(void *))
{
   if (l_root->right) rev_do_l(PTR_OF(l_root->right), callback);
   (*callback)(l_root->data);
   if (l_root->left)  rev_do_l(PTR_OF(l_root->left),  callback);
}

/*---------------------------------------------------------------------------*/

void avl_do(TREE *tree, void (*callback)(void *))
{
   if (tree->root) {
      if (IS_X(tree)) do_x(tree->x_root, callback);
      else            do_l(tree->l_root, callback);
   }
}

/*---------------------------------------------------------------------------*/

void avl_rev_do(TREE *tree, void (*callback)(void *))
{
   if (tree->root) {
      if (IS_X(tree)) rev_do_x(tree->x_root, callback);
      else            rev_do_l(tree->l_root, callback);
   }
}

/*===========================================================================*/

static void do_w_ctx_x(X_NODE *x_root, void (*callback)(void *, void *), void *context)
{
   if (x_root->left)  do_w_ctx_x(PTR_OF(x_root->left),  callback, context);
   (*callback)(x_root->data, context);
   if (x_root->right) do_w_ctx_x(PTR_OF(x_root->right), callback, context);
}

/*---------------------------------------------------------------------------*/

static void do_w_ctx_l(L_NODE *l_root, void (*callback)(void *, void *), void *context)
{
   if (l_root->left)  do_w_ctx_l(PTR_OF(l_root->left),  callback, context);
   (*callback)(l_root->data, context);
   if (l_root->right) do_w_ctx_l(PTR_OF(l_root->right), callback, context);
}

/*---------------------------------------------------------------------------*/

static void rev_do_w_ctx_x(X_NODE *x_root, void (*callback)(void *, void *), void *context)
{
   if (x_root->right) rev_do_w_ctx_x(PTR_OF(x_root->right), callback, context);
   (*callback)(x_root->data, context);
   if (x_root->left)  rev_do_w_ctx_x(PTR_OF(x_root->left),  callback, context);
}

/*---------------------------------------------------------------------------*/

static void rev_do_w_ctx_l(L_NODE *l_root, void (*callback)(void *, void *), void *context)
{
   if (l_root->right) rev_do_w_ctx_l(PTR_OF(l_root->right), callback, context);
   (*callback)(l_root->data, context);
   if (l_root->left)  rev_do_w_ctx_l(PTR_OF(l_root->left),  callback, context);
}

/*---------------------------------------------------------------------------*/

void avl_do_w_ctx(TREE *tree, void (*callback)(void *, void *), void *context)
{
   if (tree->root) {
      if (IS_X(tree)) do_w_ctx_x(tree->x_root, callback, context);
      else            do_w_ctx_l(tree->l_root, callback, context);
   }
}

/*---------------------------------------------------------------------------*/

void avl_rev_do_w_ctx(TREE *tree, void (*callback)(void *, void *), void *context)
{
   if (tree->root) {
      if (IS_X(tree)) rev_do_w_ctx_x(tree->x_root, callback, context);
      else            rev_do_w_ctx_l(tree->l_root, callback, context);
   }
}

/*===========================================================================*/

void *avl_first(TREE *tree)
{
   PATH   *path;
   char   *pathright;
   void  **pathnode;
   X_NODE *x_node;
   L_NODE *l_node;

   if ( !tree->root) return NULL;
   if ( !tree->path) {
      path = AVL_MALLOC(sizeof(*path));
      if ( !path) return NULL;
      tree->path = path;
   } else {
      path = tree->path;
   }
   pathnode  = &path->node [0];
   pathright = &path->right[1];
   *  pathnode  = NULL; /* sentinels */
   *  pathright = true;
   *++pathnode  = NULL;
   if (IS_X(tree)) {
      for (x_node = tree->root; x_node; CONTINUE_LEFT(x_node)) {
         *++pathright = false;
         *++pathnode  = x_node;
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((X_NODE *)(*pathnode))->data;
   } else {
      for (l_node = tree->root; l_node; CONTINUE_LEFT(l_node)) {
         *++pathright = false;
         *++pathnode  = l_node;
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((L_NODE *)(*pathnode))->data;
   }
}

/*---------------------------------------------------------------------------*/

void *avl_last(TREE *tree)
{
   PATH   *path;
   char   *pathright;
   void  **pathnode;
   X_NODE *x_node;
   L_NODE *l_node;

   if ( !tree->root) return NULL;
   if ( !tree->path) {
      path = AVL_MALLOC(sizeof(*path));
      if ( !path) return NULL;
      tree->path = path;
   } else {
      path = tree->path;
   }
   pathnode  = &path->node [0];
   pathright = &path->right[1];
   *  pathnode  = NULL; /* sentinels */
   *  pathright = false;
   *++pathnode  = NULL;
   if (IS_X(tree)) {
      for (x_node = tree->root; x_node; CONTINUE_RIGHT(x_node)) {
         *++pathright = true;
         *++pathnode  = x_node;
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((X_NODE *)(*pathnode))->data;
   } else {
      for (l_node = tree->root; l_node; CONTINUE_RIGHT(l_node)) {
         *++pathright = true;
         *++pathnode  = l_node;
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((L_NODE *)(*pathnode))->data;
   }
}

/*---------------------------------------------------------------------------*/

#define DOWN_RIGHT_OR_BREAK(node, pathright, pathnode) { \
   CONTINUE_RIGHT(node); \
   if (node) { \
      *++(pathright) = true; \
      *++(pathnode)  = (node); \
   } else { \
      break; \
   } \
}
#define DOWN_LEFT_OR_BREAK(node, pathright, pathnode) { \
   CONTINUE_LEFT(node); \
   if (node) { \
      *++(pathright) = false; \
      *++(pathnode)  = (node); \
   } else { \
      break; \
   } \
}

/*---------------------------------------------------------------------------*/

static void *start_x_l(TREE *tree, void *key, bool rev)
{
   PATH    *path;
   char    *pathright;
   X_NODE **x_pathnode;
   L_NODE **l_pathnode;
   X_NODE  *x_node;
   L_NODE  *l_node;
   char    *saveright;
   X_NODE **x_savenode;
   L_NODE **l_savenode;
   long     l_key;
   char    *keytail;
   int      cmp;
   bool     ind = false;

   if ( !tree->root) return NULL;
   if ( !tree->path) {
      path = AVL_MALLOC(sizeof(*path));
      if ( !path) return NULL;
      tree->path = path;
   } else {
      path = tree->path;
   }
   x_pathnode    = &path->x_node[0];
   l_pathnode    = &path->l_node[0];
   pathright     = &path->right [1];
   saveright     = NULL;
   x_savenode    = NULL;
   l_savenode    = NULL;
   *x_pathnode   = NULL; /* sentinels */
   *pathright    = !rev;
   *++x_pathnode = NULL;
    ++l_pathnode;
   *++pathright  = rev;
   *++x_pathnode = tree->root;
    ++l_pathnode;
   switch (tree->bits) {
   CASE X_PTR_NODUP:
      ind = true;
   case X_MBR_NODUP:
   case X_USR_NODUP:
      if (rev) {
         for (x_node = tree->root; ; ) {
            cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, ind);
            if (cmp > 0) {
               saveright  = pathright;
               x_savenode = x_pathnode;
               DOWN_RIGHT_OR_BREAK(x_node, pathright, x_pathnode)
            } else if (cmp < 0) {
               DOWN_LEFT_OR_BREAK (x_node, pathright, x_pathnode)
            } else {
               path->pathright  = pathright;
               path->x_pathnode = x_pathnode;
               return (*x_pathnode)->data;
            }
         }
      } else {
         for (x_node = tree->root; ; ) {
            cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, ind);
            if (cmp < 0) {
               saveright  = pathright;
               x_savenode = x_pathnode;
               DOWN_LEFT_OR_BREAK (x_node, pathright, x_pathnode)
            } else if (cmp > 0) {
               DOWN_RIGHT_OR_BREAK(x_node, pathright, x_pathnode)
            } else {
               path->pathright  = pathright;
               path->x_pathnode = x_pathnode;
               return (*x_pathnode)->data;
            }
         }
      }
   CASE L_STR_NODUP:
      ind = true;
   case L_CHA_NODUP:
      l_key = l_key_of(key, &keytail);
      if (rev) {
         for (l_node = tree->root; ; ) {
            if (l_key > l_node->key) {
               rev_start_nodup_gt:
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else if (l_key < l_node->key) {
               rev_start_nodup_lt:
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else if ( !keytail) {
               rev_start_nodup_eq:
               path->pathright  = pathright;
               path->l_pathnode = l_pathnode;
               return (*l_pathnode)->data;
            } else {
               cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, ind);
               if (cmp > 0) {
                  goto rev_start_nodup_gt;
               } else if (cmp < 0) {
                  goto rev_start_nodup_lt;
               } else {
                  goto rev_start_nodup_eq;
               }
            }
         }
      } else {
         for (l_node = tree->root; ; ) {
            if (l_key < l_node->key) {
               start_nodup_lt:
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else if (l_key > l_node->key) {
               start_nodup_gt:
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else if ( !keytail) {
               start_nodup_eq:
               path->pathright  = pathright;
               path->l_pathnode = l_pathnode;
               return (*l_pathnode)->data;
            } else {
               cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, ind);
               if (cmp < 0) {
                  goto start_nodup_lt;
               } else if (cmp > 0) {
                  goto start_nodup_gt;
               } else {
                  goto start_nodup_eq;
               }
            }
         }
      }
   CASE X_PTR_DUP:
      ind = true;
   case X_MBR_DUP:
   case X_USR_DUP:
      if (rev) {
         for (x_node = tree->root; ; ) {
            cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, ind);
            if (cmp >= 0) {
               saveright  = pathright;
               x_savenode = x_pathnode;
               DOWN_RIGHT_OR_BREAK(x_node, pathright, x_pathnode)
            } else {
               DOWN_LEFT_OR_BREAK (x_node, pathright, x_pathnode)
            }
         }
      } else {
         for (x_node = tree->root; ; ) {
            cmp = CMP(tree->usrcmp, key, x_node->data, tree->keyoffs, ind);
            if (cmp <= 0) {
               saveright  = pathright;
               x_savenode = x_pathnode;
               DOWN_LEFT_OR_BREAK (x_node, pathright, x_pathnode)
            } else {
               DOWN_RIGHT_OR_BREAK(x_node, pathright, x_pathnode)
            }
         }
      }
   CASE L_STR_DUP:
      ind = true;
   case L_CHA_DUP:
      l_key = l_key_of(key, &keytail);
      if (rev) {
         for (l_node = tree->root; ; ) {
            if (l_key < l_node->key) {
               rev_start_dup_lt:
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else if (l_key > l_node->key || !keytail) {
               rev_start_dup_ge:
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else {
               cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, ind);
               if (cmp >= 0) {
                  goto rev_start_dup_ge;
               } else {
                  goto rev_start_dup_lt;
               }
            }
         }
      } else {
         for (l_node = tree->root; ; ) {
            if (l_key > l_node->key) {
               start_dup_gt:
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else if (l_key < l_node->key || !keytail) {
               start_dup_le:
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else {
               cmp = TAILCMP(keytail, l_node->data, tree->keyoffs, ind);
               if (cmp <= 0) {
                  goto start_dup_le;
               } else {
                  goto start_dup_gt;
               }
            }
         }
      }
   }
   if (x_savenode) {
      path->pathright  = saveright;
      path->x_pathnode = x_savenode;
      return (*x_savenode)->data;
   }
   if (l_savenode) {
      path->pathright  = saveright;
      path->l_pathnode = l_savenode;
      return (*l_savenode)->data;
   }
   AVL_FREE_AND_NULL(tree->path);
   return NULL;
}

void *avl_start          (TREE *tree, void *key) { return start_x_l(tree, key, false); }
void *avl_start_mbr      (TREE *tree, void *key) { return start_x_l(tree, key, false); }
void *avl_start_ptr      (TREE *tree, void *key) { return start_x_l(tree, key, false); }
void *avl_start_chars    (TREE *tree, char *key) { return start_x_l(tree, key, false); }
void *avl_start_str      (TREE *tree, char *key) { return start_x_l(tree, key, false); }

void *avl_rev_start      (TREE *tree, void *key) { return start_x_l(tree, key, true); }
void *avl_rev_start_mbr  (TREE *tree, void *key) { return start_x_l(tree, key, true); }
void *avl_rev_start_ptr  (TREE *tree, void *key) { return start_x_l(tree, key, true); }
void *avl_rev_start_chars(TREE *tree, char *key) { return start_x_l(tree, key, true); }
void *avl_rev_start_str  (TREE *tree, char *key) { return start_x_l(tree, key, true); }

/*---------------------------------------------------------------------------*/

static void *start_l(TREE *tree, long key, bool rev)
{
   PATH    *path;
   char    *pathright;
   L_NODE **l_pathnode;
   L_NODE  *l_node;
   char    *saveright;
   L_NODE **l_savenode;

   if ( !tree->root) return NULL;
   if ( !tree->path) {
      path = AVL_MALLOC(sizeof(*path));
      if ( !path) return NULL;
      tree->path = path;
   } else {
      path = tree->path;
   }
   l_pathnode    = &path->l_node[0];
   pathright     = &path->right [1];
   saveright     = NULL;
   l_savenode    = NULL;
   *l_pathnode   = NULL; /* sentinels */
   *pathright    = !rev;
   *++l_pathnode = NULL;
   *++pathright  = rev;
   *++l_pathnode = tree->root;
   switch (tree->bits) {
   CASE L_COR_NODUP:
      key = CORRECT(key);
   case L_VAL_NODUP:
      if (rev) {
         for (l_node = tree->root; ; ) {
            if (key > l_node->key) {
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else if (key < l_node->key) {
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else {
               path->pathright  = pathright;
               path->l_pathnode = l_pathnode;
               return (*l_pathnode)->data;
            }
         }
      } else {
         for (l_node = tree->root; ; ) {
            if (key < l_node->key) {
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else if (key > l_node->key) {
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else {
               path->pathright  = pathright;
               path->l_pathnode = l_pathnode;
               return (*l_pathnode)->data;
            }
         }
      }
   CASE L_COR_DUP:
      key = CORRECT(key);
   case L_VAL_DUP:
      if (rev) {
         for (l_node = tree->root; ; ) {
            if (key >= l_node->key) {
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            } else {
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            }
         }
      } else {
         for (l_node = tree->root; ; ) {
            if (key <= l_node->key) {
               saveright  = pathright;
               l_savenode = l_pathnode;
               DOWN_LEFT_OR_BREAK (l_node, pathright, l_pathnode)
            } else {
               DOWN_RIGHT_OR_BREAK(l_node, pathright, l_pathnode)
            }
         }
      }
   }
   if (l_savenode) {
      path->pathright  = saveright;
      path->l_pathnode = l_savenode;
      return (*l_savenode)->data;
   }
   AVL_FREE_AND_NULL(tree->path);
   return NULL;
}

void *avl_start_long      (TREE *tree, long   key) { return start_l(tree, key, false); }
void *avl_start_int       (TREE *tree, int    key) { return start_l(tree, key, false); }
void *avl_start_short     (TREE *tree, short  key) { return start_l(tree, key, false); }
void *avl_start_schar     (TREE *tree, SCHAR  key) { return start_l(tree, key, false); }
void *avl_start_ulong     (TREE *tree, ULONG  key) { return start_l(tree, key, false); }
void *avl_start_uint      (TREE *tree, UINT   key) { return start_l(tree, key, false); }
void *avl_start_ushort    (TREE *tree, USHORT key) { return start_l(tree, key, false); }
void *avl_start_uchar     (TREE *tree, UCHAR  key) { return start_l(tree, key, false); }

void *avl_rev_start_long  (TREE *tree, long   key) { return start_l(tree, key, true); }
void *avl_rev_start_int   (TREE *tree, int    key) { return start_l(tree, key, true); }
void *avl_rev_start_short (TREE *tree, short  key) { return start_l(tree, key, true); }
void *avl_rev_start_schar (TREE *tree, SCHAR  key) { return start_l(tree, key, true); }
void *avl_rev_start_ulong (TREE *tree, ULONG  key) { return start_l(tree, key, true); }
void *avl_rev_start_uint  (TREE *tree, UINT   key) { return start_l(tree, key, true); }
void *avl_rev_start_ushort(TREE *tree, USHORT key) { return start_l(tree, key, true); }
void *avl_rev_start_uchar (TREE *tree, UCHAR  key) { return start_l(tree, key, true); }

/*---------------------------------------------------------------------------*/

void *avl_start_float(TREE *tree, float key)
{
   if (IS_X(tree)) return start_x_l(tree, &key,         false);
   else            return start_l  (tree, flt2lng(key), false);
}

void *avl_start_double(TREE *tree, double key)
{
   if (IS_X(tree)) return start_x_l(tree, &key,         false);
   else            return start_l  (tree, dbl2lng(key), false);
}

void *avl_rev_start_float(TREE *tree, float key)
{
   if (IS_X(tree)) return start_x_l(tree, &key,         true);
   else            return start_l  (tree, flt2lng(key), true);
}

void *avl_rev_start_double(TREE *tree, double key)
{
   if (IS_X(tree)) return start_x_l(tree, &key,         true);
   else            return start_l  (tree, dbl2lng(key), true);
}

/*---------------------------------------------------------------------------*/

void *avl_next(TREE *tree)
{
   PATH   *path;
   char   *pathright;
   void  **pathnode;
   X_NODE *x_node;
   L_NODE *l_node;

   path = tree->path;
   if ( !path) return NULL;
   pathright = path->pathright;
   pathnode  = path->pathnode;
   if (IS_X(tree)) {
      x_node = ((X_NODE *)*pathnode)->right;
      if (x_node) {
         x_node = PTR_OF(x_node);
         *++pathright = true;
         *++pathnode  = x_node;
         while ((x_node = x_node->left)) {
            x_node = PTR_OF(x_node);
            *++pathright = false;
            *++pathnode  = x_node;
         }
      } else {
         while (*pathright) {
            --pathright;
            --pathnode;
         }
         --pathright;
         --pathnode;
         if ( !*pathnode) {
            AVL_FREE_AND_NULL(tree->path);
            return NULL;
         }
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((X_NODE *)*pathnode)->data;
   } else {
      l_node = ((L_NODE *)*pathnode)->right;
      if (l_node) {
         l_node = PTR_OF(l_node);
         *++pathright = true;
         *++pathnode  = l_node;
         while ((l_node = l_node->left)) {
            l_node = PTR_OF(l_node);
            *++pathright = false;
            *++pathnode  = l_node;
         }
      } else {
         while (*pathright) {
            --pathright;
            --pathnode;
         }
         --pathright;
         --pathnode;
         if ( !*pathnode) {
            AVL_FREE_AND_NULL(tree->path);
            return NULL;
         }
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((L_NODE *)*pathnode)->data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_prev(TREE *tree)
{
   PATH  *path;
   char  *pathright;
   void **pathnode;
   X_NODE *x_node;
   L_NODE *l_node;

   path = tree->path;
   if ( !path) return NULL;
   pathright = path->pathright;
   pathnode  = path->pathnode;
   if (IS_X(tree)) {
      x_node = ((X_NODE *)*pathnode)->left;
      if (x_node) {
         x_node = PTR_OF(x_node);
         *++pathright = false;
         *++pathnode  = x_node;
         while ((x_node = x_node->right)) {
            x_node = PTR_OF(x_node);
            *++pathright = true;
            *++pathnode  = x_node;
         }
      } else {
         while ( !*pathright) {
            --pathright;
            --pathnode;
         }
         --pathright;
         --pathnode;
         if ( !*pathnode) {
            AVL_FREE_AND_NULL(tree->path);
            return NULL;
         }
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((X_NODE *)*pathnode)->data;
   } else {
      l_node = ((L_NODE *)*pathnode)->left;
      if (l_node) {
         l_node = PTR_OF(l_node);
         *++pathright = false;
         *++pathnode  = l_node;
         while ((l_node = l_node->right)) {
            l_node = PTR_OF(l_node);
            *++pathright = true;
            *++pathnode  = l_node;
         }
      } else {
         while ( !*pathright) {
            --pathright;
            --pathnode;
         }
         --pathright;
         --pathnode;
         if ( !*pathnode) {
            AVL_FREE_AND_NULL(tree->path);
            return NULL;
         }
      }
      path->pathright = pathright;
      path->pathnode  = pathnode;
      return ((L_NODE *)*pathnode)->data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void avl_stop(TREE *tree)
{
   if (tree->path) {
      AVL_FREE_AND_NULL(tree->path);
   }
}

/*===========================================================================*/

typedef struct {
   size_t offset;
   void  *data;
} LINK;

/*---------------------------------------------------------------------------*/

static void link_x(X_NODE *x_node, LINK *link)
{
   if (x_node->right) link_x(PTR_OF(x_node->right), link);
   *(void **)PTRADD(x_node->data, link->offset) = link->data;
   link->data = x_node->data;
   if (x_node->left)  link_x(PTR_OF(x_node->left),  link);
}

/*---------------------------------------------------------------------------*/

static void link_l(L_NODE *l_node, LINK *link)
{
   if (l_node->right) link_l(PTR_OF(l_node->right), link);
   *(void **)PTRADD(l_node->data, link->offset) = link->data;
   link->data = l_node->data;
   if (l_node->left)  link_l(PTR_OF(l_node->left),  link);
}

/*---------------------------------------------------------------------------*/

static void rev_link_x(X_NODE *x_node, LINK *link)
{
   if (x_node->left)  rev_link_x(PTR_OF(x_node->left),  link);
   *(void **)PTRADD(x_node->data, link->offset) = link->data;
   link->data = x_node->data;
   if (x_node->right) rev_link_x(PTR_OF(x_node->right), link);
}

/*---------------------------------------------------------------------------*/

static void rev_link_l(L_NODE *l_node, LINK *link)
{
   if (l_node->left)  rev_link_l(PTR_OF(l_node->left),  link);
   *(void **)PTRADD(l_node->data, link->offset) = link->data;
   link->data = l_node->data;
   if (l_node->right) rev_link_l(PTR_OF(l_node->right), link);
}

/*---------------------------------------------------------------------------*/

void *avl_linked_list(TREE *tree, size_t ptroffs, bool rev)
{
   LINK link;
   link.offset = ptroffs;
   link.data   = NULL;
   if (tree->root) {
      if (IS_X(tree)) {
         if (rev) rev_link_x(tree->root, &link);
         else     link_x    (tree->root, &link);
      } else {
         if (rev) rev_link_l(tree->root, &link);
         else     link_l    (tree->root, &link);
      }
   }
   return link.data;
}

/*===========================================================================*/

long avl_nodes(TREE *tree)
{
   return tree->nodes;
}

/*===========================================================================*/

static bool copy_x(TREE *newtree, X_NODE *x_newroot, X_NODE *x_root)
{
   x_newroot->data = x_root->data;
   if (x_root->left) {
      if (newtree->avail) {
         x_newroot->left = --newtree->x_store;
         newtree->avail--;
      } else {
         x_newroot->left = alloc_node_x(newtree);
         if ( !x_newroot->left) return false;
      }
      if ( !copy_x(newtree, x_newroot->left, PTR_OF(x_root->left))) {
         return false;
      }
      if (IS_DEEPER(x_root->left)) {
         x_newroot->leftval |= DEEPER;
      }
   } else {
      x_newroot->left = NULL;
   }
   if (x_root->right) {
      if (newtree->avail) {
         x_newroot->right = --newtree->x_store;
         newtree->avail--;
      } else {
         x_newroot->right = alloc_node_x(newtree);
         if ( !x_newroot->right) return false;
      }
      if ( !copy_x(newtree, x_newroot->right, PTR_OF(x_root->right))) {
         return false;
      }
      if (IS_DEEPER(x_root->right)) {
         x_newroot->rightval |= DEEPER;
      }
   } else {
      x_newroot->right = NULL;
   }
   return true;
}

/*---------------------------------------------------------------------------*/

static bool copy_l(TREE *newtree, L_NODE *l_newroot, L_NODE *l_root)
{
   l_newroot->key  = l_root->key;
   l_newroot->data = l_root->data;
   if (l_root->left) {
      if (newtree->avail) {
         l_newroot->left = --newtree->l_store;
         newtree->avail--;
      } else {
         l_newroot->left = alloc_node_l(newtree);
         if ( !l_newroot->left) return false;
      }
      if ( !copy_l(newtree, l_newroot->left,  PTR_OF(l_root->left))) {
         return false;
      }
      if (IS_DEEPER(l_root->left)) {
         l_newroot->leftval |= DEEPER;
      }
   } else {
      l_newroot->left = NULL;
   }
   if (l_root->right) {
      if (newtree->avail) {
         l_newroot->right = --newtree->l_store;
         newtree->avail--;
      } else {
         l_newroot->right = alloc_node_l(newtree);
         if ( !l_newroot->right) return false;
      }
      if ( !copy_l(newtree, l_newroot->right, PTR_OF(l_root->right))) {
         return false;
      }
      if (IS_DEEPER(l_root->right)) {
         l_newroot->rightval |= DEEPER;
      }
   } else {
      l_newroot->right = NULL;
   }
   return true;
}

/*---------------------------------------------------------------------------*/

TREE *avl_copy(TREE *tree)
{
   TREE *newtree;

   newtree = AVL_MALLOC(sizeof(*newtree));
   if ( !newtree) return NULL;
   newtree->usrcmp  = tree->usrcmp;
   newtree->path    = NULL;
   newtree->unused  = NULL;
   newtree->store   = NULL;
   newtree->nodes   = tree->nodes;
   newtree->alloc   = 0;
   newtree->avail   = 0;
   newtree->keyoffs = tree->keyoffs;
   newtree->bits    = tree->bits;
   newtree->type    = tree->type;
   if (tree->root) {
      if (IS_X(tree)) {
         newtree->x_root = alloc_node_x(newtree);
         if ( !newtree->x_root) {
            AVL_FREE(newtree);
            return NULL;
         }
         if ( !copy_x(newtree, newtree->x_root, tree->x_root)) {
            avl_free(newtree);
            return NULL;
         }
      } else {
         newtree->l_root = alloc_node_l(newtree);
         if ( !newtree->l_root) {
            AVL_FREE(newtree);
            return NULL;
         }
         if ( !copy_l(newtree, newtree->l_root, tree->l_root)) {
            avl_free(newtree);
            return NULL;
         }
      }
   } else {
      newtree->root = NULL;
   }
   return newtree;
}

/*===========================================================================*/

void avl_empty(TREE *tree)
{
   void *alloc_base, *next_alloc_base;

   if (tree->alloc) {
      if (tree->path) {
         AVL_FREE_AND_NULL(tree->path);
      }
      if (IS_X(tree)) {
         alloc_base = PTRSUB(tree->x_store - tree->avail, SIZEOF_P);
      } else {
         alloc_base = PTRSUB(tree->l_store - tree->avail, SIZEOF_P_L);
      }
      while (alloc_base) {
         next_alloc_base = *(void **)alloc_base;
         free(alloc_base);
         alloc_base = next_alloc_base;
      }
      tree->root   = NULL;
      tree->unused = NULL;
      tree->store  = NULL;
      tree->nodes  = 0;
      tree->avail  = 0;
      tree->alloc  = 0;
   }
}

/*---------------------------------------------------------------------------*/

void avl_free(TREE *tree)
{
   avl_empty(tree);
   tree->root = (void *)255;
   AVL_FREE(tree);
}

/*---------------------------------------------------------------------------*/
