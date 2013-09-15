/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avl.c                                    |
 |                                                                            |
 |               A simple but effective AVL trees library in C,               |
 |             created by Walter Tross sometime before 1991-03-11             |
 |                                                                            |
 |                                  v 2.0.0                                   |
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
#include <assert.h>
#include "avl.h"

#define CASE    break; case
#define DEFAULT break; default

typedef unsigned long  ULONG;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef   signed char  SCHAR;

typedef int (*CMPFUN)(void *, void *);

typedef union avl_key {
   void *ptr;
   long  val;
} KEY;

typedef struct avl_node NODE;
struct avl_node {
   KEY   key;
   void *data;
   NODE *left;
   NODE *right;
   int   bal;
};

#define AVL_MAX_PATHDEPTH (sizeof(long) * 8 * 3 / 2 - 2)

typedef struct avl_path {
   NODE **pathnode;
   char  *pathright;
   NODE  *node [AVL_MAX_PATHDEPTH + 2];
   char   right[AVL_MAX_PATHDEPTH + 2];
} PATH;

struct avl_tree {
   NODE  *root;
   PATH  *path;
   long   nodes;
   CMPFUN usrcmp;
   USHORT keyinfo;
   USHORT keyoffs;
};

#define AVL_CHA AVL_CHARS
#define AVL_LNG AVL_LONG
#define AVL_SHT AVL_SHORT
#define AVL_SCH AVL_SCHAR
#define AVL_ULN AVL_ULONG
#define AVL_UIN AVL_UINT
#define AVL_USH AVL_USHORT
#define AVL_UCH AVL_UCHAR

#define USR_KEY (AVL_USR >> 1)
#define MBR_KEY (AVL_MBR >> 1)
#define CHA_KEY (AVL_CHA >> 1)
#define PTR_KEY (AVL_PTR >> 1)
#define STR_KEY (AVL_STR >> 1)
#define LNG_KEY (AVL_LNG >> 1)
#define INT_KEY (AVL_INT >> 1)
#define SHT_KEY (AVL_SHT >> 1)
#define SCH_KEY (AVL_SCH >> 1)
#define ULN_KEY (AVL_ULN >> 1)
#define UIN_KEY (AVL_UIN >> 1)
#define USH_KEY (AVL_USH >> 1)
#define UCH_KEY (AVL_UCH >> 1)

#define USR_CMP 0
#define STR_CMP 1
#define VAL_CMP 2
#define COR_CMP 3

#define NODUP 0
#define DUP   1

#define USR_NODUP (USR_CMP | NODUP << 2)
#define STR_NODUP (STR_CMP | NODUP << 2)
#define VAL_NODUP (VAL_CMP | NODUP << 2)
#define COR_NODUP (COR_CMP | NODUP << 2)
#define USR_DUP   (USR_CMP | DUP   << 2)
#define STR_DUP   (STR_CMP | DUP   << 2)
#define VAL_DUP   (VAL_CMP | DUP   << 2)
#define COR_DUP   (COR_CMP | DUP   << 2)

/* keyinfo: bits 6-3: KEYTYPE, bit 2: DUP, bits 1-0: CMPTYPE */
/*          bits 6-2: TREETYPE,            bits 2-0: LOCTYPE */

#define KEYTYPE( keyinfo) ((keyinfo) >> 3)
#define TREETYPE(keyinfo) ((keyinfo) >> 2)
#define CMPTYPE( keyinfo) ((keyinfo) & 0x3)
#define LOCTYPE( keyinfo) ((keyinfo) & 0x7)

#define UINT_CLASH (sizeof(UINT) == sizeof(long))

#define MINLONG ((long)~(((ULONG)(~0L))>>1))

#define CORRECT(u)       ((long)(u) + MINLONG)
#define CORR_IF(u, cond) ((cond) ? CORRECT(u) : (long)(u))

#define PTRCMP(usrcmp, ptr1, ptr2) ( \
   (usrcmp) ? (*(usrcmp))((void *)(ptr1), (void *)(ptr2)) \
            :    strcmp  ((char *)(ptr1), (char *)(ptr2)) \
)

#define BAL   0
#define LEFT  1
#define RIGHT 2

typedef enum avl_unbal {
   LEFTUNBAL,
   RIGHTUNBAL
} UNBAL;

typedef enum avl_depth {
   LESS,
   SAME
} DEPTH;

typedef enum avl_ins {
   NOTINS,
   INS,
   DEEPER
} INS_T;

#define LONG_ALIGNED_SIZE(obj) ((sizeof(obj)+sizeof(long)-1)&~(sizeof(long)-1))

#define SIZEOF_NODE  LONG_ALIGNED_SIZE(NODE)
#define SIZEOF_TREE  LONG_ALIGNED_SIZE(TREE)
#define SIZEOF_PATH  LONG_ALIGNED_SIZE(PATH)

#define NODE_LIST     0
#define TREE_LIST     (NODE_LIST + (SIZEOF_NODE != SIZEOF_TREE))
#define PATH_LIST     (TREE_LIST + 1)
#define N_FREE_LISTS  (PATH_LIST + 1)

#ifndef AVL_PREALLOC_BYTES
#define AVL_PREALLOC_BYTES  0x10000
#endif
#ifndef AVL_MALLOC_BYTES
#define AVL_MALLOC_BYTES    0x10000
#endif

#define PREALLOC_LONGS ((AVL_PREALLOC_BYTES - 16) / sizeof(long))
#define PREALLOC_SIZE  (PREALLOC_LONGS            * sizeof(long))
#define MALLOC_LONGS   ((AVL_MALLOC_BYTES   - 32) / sizeof(long))
#define MALLOC_SIZE    (MALLOC_LONGS              * sizeof(long))

static long Prealloc[PREALLOC_LONGS];

static void  *Free_List[N_FREE_LISTS]; /* init to 0 by C */
static char  *Avail_Base = (char *)Prealloc;
static size_t Avail_Size = PREALLOC_SIZE;

/*---------------------------------------------------------------------------*/

static void *get_memory(size_t sizeof_obj)
{
   if (sizeof_obj > SIZEOF_NODE) {
      while (Avail_Size >= SIZEOF_NODE) {
         char *base = Avail_Base + (Avail_Size -= SIZEOF_NODE);
         *(void **)base = Free_List[NODE_LIST];
                          Free_List[NODE_LIST] = (void *)base;
      }
   }
   Avail_Base = (char *)malloc(MALLOC_SIZE);
   if (Avail_Base) {
      Avail_Size = MALLOC_SIZE - sizeof_obj;
      return (void *)(Avail_Base + Avail_Size);
   } else {
      Avail_Size = 0;
      return NULL;
   }
}

/*---------------------------------------------------------------------------*/

#define ALLOC_OBJ(ptr, ptr_type, sizeof_obj, free_list) do { \
   if (Free_List[free_list]) { \
      (ptr) = (ptr_type)Free_List[free_list]; \
                        Free_List[free_list] = *(void **)(ptr); \
   } else if (Avail_Size >= sizeof_obj) { \
      (ptr) = (ptr_type)(Avail_Base + (Avail_Size -= sizeof_obj)); \
   } else { \
      (ptr) = (ptr_type)get_memory(sizeof_obj); \
   } \
} while(0)

#define FREE_OBJ(ptr, free_list) do { \
   *(void **)(ptr) = Free_List[free_list]; \
                     Free_List[free_list] = (void *)(ptr); \
} while(0)

#define ALLOC_NODE(node) ALLOC_OBJ((node), NODE *, SIZEOF_NODE, NODE_LIST)
#define ALLOC_TREE(tree) ALLOC_OBJ((tree), TREE *, SIZEOF_TREE, TREE_LIST)
#define ALLOC_PATH(path) ALLOC_OBJ((path), PATH *, SIZEOF_PATH, PATH_LIST)

#define FREE_NODE(node)  FREE_OBJ((node), NODE_LIST)
#define FREE_TREE(tree)  FREE_OBJ((tree), TREE_LIST)
#define FREE_PATH(path)  FREE_OBJ((path), PATH_LIST)

/*===========================================================================*/

TREE *avl_tree(int treetype, size_t keyoffs, CMPFUN usrcmp)
{
   TREE *tree;
   int   keyinfo;

   if (sizeof(void *) != sizeof(long)) {
      return NULL;
   }
   keyinfo = treetype << 2;
   switch (treetype & ~AVL_DUP) {
   CASE AVL_USR: keyinfo |= USR_CMP;
   CASE AVL_MBR: keyinfo |= USR_CMP;
   CASE AVL_CHA: keyinfo |= STR_CMP;
   CASE AVL_PTR: keyinfo |= USR_CMP;
   CASE AVL_STR: keyinfo |= STR_CMP;
   CASE AVL_LNG: keyinfo |= VAL_CMP;
   CASE AVL_INT: keyinfo |= VAL_CMP;
   CASE AVL_SHT: keyinfo |= VAL_CMP;
   CASE AVL_SCH: keyinfo |= VAL_CMP;
   CASE AVL_ULN: keyinfo |= COR_CMP;
   CASE AVL_UIN: keyinfo |= UINT_CLASH ? COR_CMP : VAL_CMP;
   CASE AVL_USH: keyinfo |= VAL_CMP;
   CASE AVL_UCH: keyinfo |= VAL_CMP;
   DEFAULT:
      return NULL;
   }
   ALLOC_TREE(tree);
   if ( !tree) {
      return NULL;
   }
   tree->root = NULL;
   tree->keyinfo = (USHORT)keyinfo;
   tree->keyoffs = (USHORT)keyoffs;
   tree->usrcmp  = usrcmp;
   tree->nodes   = 0L;
   tree->path    = NULL;

   return tree;
}

/*===========================================================================*/

static DEPTH rebalance(NODE **rootaddr, UNBAL unbal)
{
   NODE *root = *rootaddr;
   NODE *newroot;
   NODE *half;

   if (unbal == LEFTUNBAL) {
      switch (root->left->bal) {
      CASE LEFT: /* simple rotation, tree depth decreased */
         newroot = root->left;
         root->left = newroot->right;
         root->bal  = BAL;
         newroot->right = root;
         newroot->bal   = BAL;
         *rootaddr = newroot;
         return LESS;

      CASE BAL: /* simple rotation, tree depth unchanged */
         newroot = root->left;
         root->left = newroot->right;
         root->bal  = LEFT;
         newroot->right = root;
         newroot->bal   = RIGHT;
         *rootaddr = newroot;
         return SAME;

      CASE RIGHT: /* double rotation */
         half    = root->left;
         newroot = half->right;
         root->left  = newroot->right;
         half->right = newroot->left;
         switch (newroot->bal) {
         CASE BAL:
            root->bal = BAL;
            half->bal = BAL;
         CASE LEFT:
            root->bal = RIGHT;
            half->bal = BAL;
         CASE RIGHT:
            root->bal = BAL;
            half->bal = LEFT;
         }
         newroot->left  = half;
         newroot->right = root;
         newroot->bal   = BAL;
         *rootaddr = newroot;
         return LESS;
      }
   } else {
      switch (root->right->bal) {
      CASE RIGHT: /* simple rotation, tree depth decreased */
         newroot = root->right;
         root->right = newroot->left;
         root->bal   = BAL;
         newroot->left = root;
         newroot->bal  = BAL;
         *rootaddr = newroot;
         return LESS;

      CASE BAL: /* simple rotation, tree depth unchanged */
         newroot = root->right;
         root->right = newroot->left;
         root->bal   = RIGHT;
         newroot->left = root;
         newroot->bal  = LEFT;
         *rootaddr = newroot;
         return SAME;

      CASE LEFT: /* double rotation */
         half    = root->right;
         newroot = half->left;
         root->right = newroot->left;
         half->left  = newroot->right;
         switch (newroot->bal) {
         CASE BAL:
            root->bal = BAL;
            half->bal = BAL;
         CASE RIGHT:
            root->bal = LEFT;
            half->bal = BAL;
         CASE LEFT:
            root->bal = BAL;
            half->bal = RIGHT;
         }
         newroot->right = half;
         newroot->left  = root;
         newroot->bal   = BAL;
         *rootaddr = newroot;
         return LESS;
      }
   }
   assert( !"balanced subtrees");
   return -1;
}

/*===========================================================================*/

static INS_T insert_ptr(NODE **rootaddr, NODE *node, CMPFUN usrcmp, bool dup)
{
   NODE *root = *rootaddr;
   int   cmp;
   INS_T ins;

   cmp = PTRCMP(usrcmp, node->key.ptr, root->key.ptr);
   if (cmp < 0) {
      if (root->left) {
         ins = insert_ptr(&root->left, node, usrcmp, dup);
      } else {
         root->left = node;
         ins = DEEPER;
      }
      switch (ins) {
      CASE DEEPER:
         switch (root->bal) {
         CASE RIGHT:
            root->bal = BAL;
            return INS;
         CASE BAL:
            root->bal = LEFT;
            return DEEPER;
         CASE LEFT:
            return rebalance(rootaddr, LEFTUNBAL) == LESS ? INS : DEEPER;
         }
      CASE INS:
         return INS;
      CASE NOTINS:
         return NOTINS;
      }
   } else if (cmp > 0 || dup) {
      if (root->right) {
         ins = insert_ptr(&root->right, node, usrcmp, dup);
      } else {
         root->right = node;
         ins = DEEPER;
      }
      switch (ins) {
      CASE DEEPER:
         switch (root->bal) {
         CASE LEFT:
            root->bal = BAL;
            return INS;
         CASE BAL:
            root->bal = RIGHT;
            return DEEPER;
         CASE RIGHT:
            return rebalance(rootaddr, RIGHTUNBAL) == LESS ? INS : DEEPER;
         }
      CASE INS:
         return INS;
      CASE NOTINS:
         return NOTINS;
      }
   }
   return NOTINS;
}

/*---------------------------------------------------------------------------*/

static INS_T insert_val(NODE **rootaddr, NODE *node, bool dup)
{
   NODE *root = *rootaddr;
   INS_T ins;

   if (node->key.val < root->key.val) {
      if (root->left) {
         ins = insert_val(&root->left, node, dup);
      } else {
         root->left = node;
         ins = DEEPER;
      }
      switch (ins) {
      CASE DEEPER:
         switch (root->bal) {
         CASE RIGHT:
            root->bal = BAL;
            return INS;
         CASE BAL:
            root->bal = LEFT;
            return DEEPER;
         CASE LEFT:
            return rebalance(rootaddr, LEFTUNBAL) == LESS ? INS : DEEPER;
         }
      CASE INS:
         return INS;
      CASE NOTINS:
         return NOTINS;
      }
   } else if (node->key.val > root->key.val || dup) {
      if (root->right) {
         ins = insert_val(&root->right, node, dup);
      } else {
         root->right = node;
         ins = DEEPER;
      }
      switch (ins) {
      CASE DEEPER:
         switch (root->bal) {
         CASE LEFT:
            root->bal = BAL;
            return INS;
         CASE BAL:
            root->bal = RIGHT;
            return DEEPER;
         CASE RIGHT:
            return rebalance(rootaddr, RIGHTUNBAL) == LESS ? INS : DEEPER;
         }
      CASE INS:
         return INS;
      CASE NOTINS:
         return NOTINS;
      }
   }
   return NOTINS;
}

/*---------------------------------------------------------------------------*/

bool avl_insert(TREE *tree, void *data)
{
   NODE *node;
   int   keyinfo;
   INS_T ins;

   ALLOC_NODE(node);
   if ( !node) {
      return false;
   }
   node->data  = data;
   node->left  = NULL;
   node->right = NULL;
   node->bal   = BAL;
   keyinfo = tree->keyinfo;

   switch (KEYTYPE(keyinfo)) {
   CASE USR_KEY: node->key.ptr =          (void   *)((char *)data);
   CASE MBR_KEY: node->key.ptr =          (void   *)((char *)data + tree->keyoffs);
   CASE CHA_KEY: node->key.ptr =          (void   *)((char *)data + tree->keyoffs);
   CASE PTR_KEY: node->key.ptr =         *(void  **)((char *)data + tree->keyoffs);
   CASE STR_KEY: node->key.ptr =         *(void  **)((char *)data + tree->keyoffs);
   CASE LNG_KEY: node->key.val =         *(long   *)((char *)data + tree->keyoffs);
   CASE INT_KEY: node->key.val =         *(int    *)((char *)data + tree->keyoffs);
   CASE SHT_KEY: node->key.val =         *(short  *)((char *)data + tree->keyoffs);
   CASE SCH_KEY: node->key.val =         *(SCHAR  *)((char *)data + tree->keyoffs);
   CASE ULN_KEY: node->key.val = CORRECT(*(ULONG  *)((char *)data + tree->keyoffs));
   CASE UIN_KEY: node->key.val = CORR_IF(*(UINT   *)((char *)data + tree->keyoffs), UINT_CLASH);
   CASE USH_KEY: node->key.val =         *(USHORT *)((char *)data + tree->keyoffs);
   CASE UCH_KEY: node->key.val =         *(UCHAR  *)((char *)data + tree->keyoffs);
   DEFAULT:
      FREE_NODE(node);
      return false;
   }
   if (tree->root) {
      switch (LOCTYPE(keyinfo)) {
      CASE USR_NODUP: ins = insert_ptr(&tree->root, node, tree->usrcmp, NODUP);
      CASE STR_NODUP: ins = insert_ptr(&tree->root, node, NULL,         NODUP);
      CASE COR_NODUP:
      case VAL_NODUP: ins = insert_val(&tree->root, node,               NODUP);
      CASE USR_DUP:   ins = insert_ptr(&tree->root, node, tree->usrcmp, DUP);
      CASE STR_DUP:   ins = insert_ptr(&tree->root, node, NULL,         DUP);
      CASE COR_DUP:
      case VAL_DUP:   ins = insert_val(&tree->root, node,               DUP);
      DEFAULT:        ins = NOTINS; assert( !"unexpected LOCTYPE");
      }
      if (ins == NOTINS) {
         FREE_NODE(node);
         return false;
      }
   } else {
      tree->root = node;
   }
   tree->nodes++;
   if (tree->path) {
      FREE_PATH(tree->path);
      tree->path = NULL;
   }
   return true;
}

/*===========================================================================*/

static NODE *leftmost(NODE **rootaddr, DEPTH *depth)
{
   NODE *root = *rootaddr;
   NODE *node;

   if (root) {
      if (root->left) {
         node = leftmost(&root->left, depth);
         if (*depth == LESS) {
            /* left subtree depth decreased */
            switch (root->bal) {
            CASE LEFT:
               root->bal = BAL;
            CASE BAL:
               root->bal = RIGHT;
               *depth = SAME;
            CASE RIGHT:
               *depth = rebalance(rootaddr, RIGHTUNBAL);
            }
         }
         return node;
      } else {
         *rootaddr = root->right;
         *depth = LESS;
         return root;
      }
   } else {
      *depth = SAME;
      return NULL;
   }
}

/*---------------------------------------------------------------------------*/

static NODE *remove_ptr(NODE **rootaddr, void *keyptr, CMPFUN usrcmp, bool dup, DEPTH *depth)
{
   NODE *root = *rootaddr;
   NODE *node;
   int   cmp;

   cmp = PTRCMP(usrcmp, keyptr, root->key.ptr);
   if (cmp < 0) {
      if ( !root->left) {
         return NULL;
      }
      node = remove_ptr(&root->left, keyptr, usrcmp, dup, depth);
      if (node && *depth == LESS) {
         /* left subtree depth decreased */
         switch (root->bal) {
         CASE LEFT:
            root->bal = BAL;
         CASE BAL:
            root->bal = RIGHT;
            *depth = SAME;
         CASE RIGHT:
            *depth = rebalance(rootaddr, RIGHTUNBAL);
         }
      }
   } else if (cmp > 0) {
      if ( !root->right) {
         return NULL;
      }
      node = remove_ptr(&root->right, keyptr, usrcmp, dup, depth);
      if (node && *depth == LESS) {
         /* right subtree depth decreased */
         switch (root->bal) {
         CASE RIGHT:
            root->bal = BAL;
         CASE BAL:
            root->bal = LEFT;
            *depth = SAME;
         CASE LEFT:
            *depth = rebalance(rootaddr, LEFTUNBAL);
         }
      }
   } else {
      if (dup && root->left
              && (node = remove_ptr(&root->left, keyptr, usrcmp, dup, depth))) {
         if (*depth == LESS) {
            /* left subtree depth decreased */
            switch (root->bal) {
            CASE LEFT:
               root->bal = BAL;
            CASE BAL:
               root->bal = RIGHT;
               *depth = SAME;
            CASE RIGHT:
               *depth = rebalance(rootaddr, RIGHTUNBAL);
            }
         }
      } else {
         node = root;
         if ( !node->right) {
            *rootaddr = node->left;
            *depth = LESS;
         } else if ( !node->left) {
            *rootaddr = node->right;
            *depth = LESS;
         } else {
            /* replace by the leftmost node of the right subtree */
            root = leftmost(&node->right, depth);
            root->left  = node->left;
            root->right = node->right;
            if (*depth == LESS) {
               /* right subtree depth decreased */
               switch (node->bal) {
               CASE RIGHT:
                  root->bal = BAL;
               CASE BAL:
                  root->bal = LEFT;
                  *depth = SAME;
               CASE LEFT:
                  *depth = rebalance(&root, LEFTUNBAL);
               }
            } else {
               root->bal = node->bal;
               *depth = SAME;
            }
            *rootaddr = root;
         }
      }
   }
   return node;
}

/*---------------------------------------------------------------------------*/

static NODE *remove_val(NODE **rootaddr, long keyval, bool dup, DEPTH *depth)
{
   NODE *root = *rootaddr;
   NODE *node;

   if (keyval < root->key.val) {
      if ( !root->left) {
         return NULL;
      }
      node = remove_val(&root->left, keyval, dup, depth);
      if (node && *depth == LESS) {
         /* left subtree depth decreased */
         switch (root->bal) {
         CASE LEFT:
            root->bal = BAL;
         CASE BAL:
            root->bal = RIGHT;
            *depth = SAME;
         CASE RIGHT:
            *depth = rebalance(rootaddr, RIGHTUNBAL);
         }
      }
   } else if (keyval > root->key.val) {
      if ( !root->right) {
         return NULL;
      }
      node = remove_val(&root->right, keyval, dup, depth);
      if (node && *depth == LESS) {
         /* right subtree depth decreased */
         switch (root->bal) {
         CASE RIGHT:
            root->bal = BAL;
         CASE BAL:
            root->bal = LEFT;
            *depth = SAME;
         CASE LEFT:
            *depth = rebalance(rootaddr, LEFTUNBAL);
         }
      }
   } else {
      if (dup && root->left
              && (node = remove_val(&root->left, keyval, dup, depth))) {
         if (*depth == LESS) {
            /* left subtree depth decreased */
            switch (root->bal) {
            CASE LEFT:
               root->bal = BAL;
            CASE BAL:
               root->bal = RIGHT;
               *depth = SAME;
            CASE RIGHT:
               *depth = rebalance(rootaddr, RIGHTUNBAL);
            }
         }
      } else {
         node = root;
         if ( !node->right) {
            *rootaddr = node->left;
            *depth = LESS;
         } else if ( !node->left) {
            *rootaddr = node->right;
            *depth = LESS;
         } else {
            /* replace by the leftmost node of the right subtree */
            root = leftmost(&node->right, depth);
            root->left  = node->left;
            root->right = node->right;
            if (*depth == LESS) {
               /* right subtree depth decreased */
               switch (node->bal) {
               CASE RIGHT:
                  root->bal = BAL;
               CASE BAL:
                  root->bal = LEFT;
                  *depth = SAME;
               CASE LEFT:
                  *depth = rebalance(&root, LEFTUNBAL);
               }
            } else {
               root->bal = node->bal;
               *depth = SAME;
            }
            *rootaddr = root;
         }
      }
   }
   return node;
}

/*---------------------------------------------------------------------------*/

static void *avl_f_remove(TREE *tree, long keyval)
{
   NODE *node;
   void *data;
   DEPTH depth;

   if (tree->root) {
      switch (LOCTYPE(tree->keyinfo)) {
      CASE USR_NODUP: node = remove_ptr(&tree->root, (void *)keyval, tree->usrcmp, NODUP, &depth);
      CASE STR_NODUP: node = remove_ptr(&tree->root, (void *)keyval, NULL,         NODUP, &depth);
      CASE COR_NODUP: keyval = CORRECT(keyval);
      case VAL_NODUP: node = remove_val(&tree->root,         keyval,               NODUP, &depth);
      CASE USR_DUP:   node = remove_ptr(&tree->root, (void *)keyval, tree->usrcmp, DUP,   &depth);
      CASE STR_DUP:   node = remove_ptr(&tree->root, (void *)keyval, NULL,         DUP,   &depth);
      CASE COR_DUP:   keyval = CORRECT(keyval);
      case VAL_DUP:   node = remove_val(&tree->root,         keyval,               DUP,   &depth);
      DEFAULT:        node = NULL; assert( !"unexpected LOCTYPE");
      }
      if (node) {
         tree->nodes--;
         if (tree->path) {
            FREE_PATH(tree->path);
            tree->path = NULL;
         }
         data = node->data;
         FREE_NODE(node);
         return data;
      }
   }
   return NULL;
}

void *avl_remove       (TREE *tree, void          *key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_mbr   (TREE *tree, void          *key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_chars (TREE *tree, char          *key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_ptr   (TREE *tree, void          *key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_str   (TREE *tree, char          *key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_long  (TREE *tree, long           key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_int   (TREE *tree, int            key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_short (TREE *tree, short          key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_schar (TREE *tree, signed char    key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_ulong (TREE *tree, unsigned long  key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_uint  (TREE *tree, unsigned int   key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_ushort(TREE *tree, unsigned short key) { return avl_f_remove(tree, (long)key); }
void *avl_remove_uchar (TREE *tree, unsigned char  key) { return avl_f_remove(tree, (long)key); }

/*===========================================================================*/

#define SELECT_EQ_NODUP(node, val, this) { \
   if      ((this) < (val)         )                    (node) = (node)->right;   \
   else if (         (val) < (this))                    (node) = (node)->left;    \
   else return                                                   (node)->data;    \
}
#define SELECT_EQ_DUP(node, val, this, save) { \
   if      ((this) < (val)         )                    (node) = (node)->right;   \
   else if (         (val) < (this))                    (node) = (node)->left;    \
   else                              { (save) = (node); (node) = (node)->left;  } \
}
#define SELECT_GE_NODUP(node, val, this, save) { \
   if      (         (val) < (this)) { (save) = (node); (node) = (node)->left;  } \
   else if ((this) < (val)         )                    (node) = (node)->right;   \
   else return (node)->data;    \
}
#define SELECT_GE_DUP(node, val, this, save) { \
   if      ((this) < (val)         )                    (node) = (node)->right;   \
   else                              { (save) = (node); (node) = (node)->left;  } \
}
#define SELECT_GT(node, val, this, save) { \
   if      (         (val) < (this)) { (save) = (node); (node) = (node)->left;  } \
   else                                                 (node) = (node)->right;   \
}
#define SELECT_LE_NODUP(node, val, this, save) { \
   if      ((this) < (val)         ) { (save) = (node); (node) = (node)->right; } \
   else if (         (val) < (this))                    (node) = (node)->left;    \
   else return (node)->data;    \
}
#define SELECT_LE_DUP(node, val, this, save) { \
   if      (         (val) < (this))                    (node) = (node)->left;    \
   else                              { (save) = (node); (node) = (node)->right; } \
}
#define SELECT_LT(node, val, this, save) { \
   if      ((this) < (val)         ) { (save) = (node); (node) = (node)->right; } \
   else                                                 (node) = (node)->left;    \
}

/*---------------------------------------------------------------------------*/

static void *avl_f_locate(TREE *tree, long keyval)
{
   NODE  *node;
   NODE  *save;
   CMPFUN usrcmp;
   int    cmp;

   node = tree->root;
   switch (LOCTYPE(tree->keyinfo)) {
   CASE USR_NODUP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_EQ_NODUP(node, cmp, 0)
      }
   CASE STR_NODUP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_EQ_NODUP(node, cmp, 0)
      }
   CASE COR_NODUP:
      keyval = CORRECT(keyval);
   case VAL_NODUP:
      while (node) {
         SELECT_EQ_NODUP(node, keyval, node->key.val)
      }
   CASE USR_DUP:
      save = NULL;
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_EQ_DUP(node, cmp, 0, save)
      }
      if (save) {
         return save->data;
      }
   CASE STR_DUP:
      save = NULL;
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_EQ_DUP(node, cmp, 0, save)
      }
      if (save) {
         return save->data;
      }
   CASE COR_DUP:
      keyval = CORRECT(keyval);
   case VAL_DUP:
      save = NULL;
      while (node) {
         SELECT_EQ_DUP(node, keyval, node->key.val, save)
      }
      if (save) {
         return save->data;
      }
   }
   return NULL;
}

void *avl_locate       (TREE *tree, void          *key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_mbr   (TREE *tree, void          *key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_chars (TREE *tree, char          *key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_ptr   (TREE *tree, void          *key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_str   (TREE *tree, char          *key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_long  (TREE *tree, long           key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_int   (TREE *tree, int            key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_short (TREE *tree, short          key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_schar (TREE *tree, signed char    key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_ulong (TREE *tree, unsigned long  key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_uint  (TREE *tree, unsigned int   key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_ushort(TREE *tree, unsigned short key) { return avl_f_locate(tree, (long)key); }
void *avl_locate_uchar (TREE *tree, unsigned char  key) { return avl_f_locate(tree, (long)key); }

/*---------------------------------------------------------------------------*/

static void *avl_f_locate_ge(TREE *tree, long keyval)
{
   NODE  *node;
   NODE  *save;
   CMPFUN usrcmp;
   int    cmp;

   node = tree->root;
   save = NULL;
   switch (LOCTYPE(tree->keyinfo)) {
   CASE USR_NODUP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_GE_NODUP(node, cmp, 0, save)
      }
   CASE STR_NODUP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_GE_NODUP(node, cmp, 0, save)
      }
   CASE COR_NODUP:
      keyval = CORRECT(keyval);
   case VAL_NODUP:
      while (node) {
         SELECT_GE_NODUP(node, keyval, node->key.val, save)
      }
   CASE USR_DUP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_GE_DUP(node, cmp, 0, save)
      }
   CASE STR_DUP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_GE_DUP(node, cmp, 0, save)
      }
   CASE COR_DUP:
      keyval = CORRECT(keyval);
   case VAL_DUP:
      while (node) {
         SELECT_GE_DUP(node, keyval, node->key.val, save)
      }
   }
   return save ? save->data : NULL;
}

void *avl_locate_ge       (TREE *tree, void          *key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_mbr   (TREE *tree, void          *key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_chars (TREE *tree, char          *key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_ptr   (TREE *tree, void          *key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_str   (TREE *tree, char          *key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_long  (TREE *tree, long           key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_int   (TREE *tree, int            key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_short (TREE *tree, short          key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_schar (TREE *tree, signed char    key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_ulong (TREE *tree, unsigned long  key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_uint  (TREE *tree, unsigned int   key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_ushort(TREE *tree, unsigned short key) { return avl_f_locate_ge(tree, (long)key); }
void *avl_locate_ge_uchar (TREE *tree, unsigned char  key) { return avl_f_locate_ge(tree, (long)key); }

/*---------------------------------------------------------------------------*/

static void *avl_f_locate_gt(TREE *tree, long keyval)
{
   NODE  *node;
   NODE  *save;
   CMPFUN usrcmp;
   int    cmp;

   node = tree->root;
   save = NULL;
   switch (CMPTYPE(tree->keyinfo)) {
   CASE USR_CMP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_GT(node, cmp, 0, save)
      }
   CASE STR_CMP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_GT(node, cmp, 0, save)
      }
   CASE COR_CMP:
      keyval = CORRECT(keyval);
   case VAL_CMP:
      while (node) {
         SELECT_GT(node, keyval, node->key.val, save)
      }
   }
   return save ? save->data : NULL;
}

void *avl_locate_gt       (TREE *tree, void          *key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_mbr   (TREE *tree, void          *key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_chars (TREE *tree, char          *key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_ptr   (TREE *tree, void          *key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_str   (TREE *tree, char          *key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_long  (TREE *tree, long           key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_int   (TREE *tree, int            key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_short (TREE *tree, short          key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_schar (TREE *tree, signed char    key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_ulong (TREE *tree, unsigned long  key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_uint  (TREE *tree, unsigned int   key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_ushort(TREE *tree, unsigned short key) { return avl_f_locate_gt(tree, (long)key); }
void *avl_locate_gt_uchar (TREE *tree, unsigned char  key) { return avl_f_locate_gt(tree, (long)key); }

/*---------------------------------------------------------------------------*/

static void *avl_f_locate_le(TREE *tree, long keyval)
{
   NODE  *node;
   NODE  *save;
   CMPFUN usrcmp;
   int    cmp;

   node = tree->root;
   save = NULL;
   switch (LOCTYPE(tree->keyinfo)) {
   CASE USR_NODUP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_LE_NODUP(node, cmp, 0, save)
      }
   CASE STR_NODUP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_LE_NODUP(node, cmp, 0, save)
      }
   CASE COR_NODUP:
      keyval = CORRECT(keyval);
   case VAL_NODUP:
      while (node) {
         SELECT_LE_NODUP(node, keyval, node->key.val, save)
      }
   CASE USR_DUP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_LE_DUP(node, cmp, 0, save)
      }
   CASE STR_DUP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_LE_DUP(node, cmp, 0, save)
      }
   CASE COR_DUP:
      keyval = CORRECT(keyval);
   case VAL_DUP:
      while (node) {
         SELECT_LE_DUP(node, keyval, node->key.val, save)
      }
   }
   return save ? save->data : NULL;
}

void *avl_locate_le       (TREE *tree, void          *key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_mbr   (TREE *tree, void          *key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_chars (TREE *tree, char          *key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_ptr   (TREE *tree, void          *key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_str   (TREE *tree, char          *key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_long  (TREE *tree, long           key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_int   (TREE *tree, int            key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_short (TREE *tree, short          key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_schar (TREE *tree, signed char    key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_ulong (TREE *tree, unsigned long  key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_uint  (TREE *tree, unsigned int   key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_ushort(TREE *tree, unsigned short key) { return avl_f_locate_le(tree, (long)key); }
void *avl_locate_le_uchar (TREE *tree, unsigned char  key) { return avl_f_locate_le(tree, (long)key); }

/*---------------------------------------------------------------------------*/

static void *avl_f_locate_lt(TREE *tree, long keyval)
{
   NODE  *node;
   NODE  *save;
   CMPFUN usrcmp;
   int    cmp;

   node = tree->root;
   save = NULL;
   switch (CMPTYPE(tree->keyinfo)) {
   CASE USR_CMP:
      usrcmp = tree->usrcmp;
      while (node) {
         cmp = (*usrcmp)((void *)keyval, node->key.ptr);
         SELECT_LT(node, cmp, 0, save)
      }
   CASE STR_CMP:
      while (node) {
         cmp = strcmp((char *)keyval, (char *)node->key.ptr);
         SELECT_LT(node, cmp, 0, save)
      }
   CASE COR_CMP:
      keyval = CORRECT(keyval);
   case VAL_CMP:
      while (node) {
         SELECT_LT(node, keyval, node->key.val, save)
      }
   }
   return save ? save->data : NULL;
}

void *avl_locate_lt       (TREE *tree, void          *key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_mbr   (TREE *tree, void          *key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_chars (TREE *tree, char          *key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_ptr   (TREE *tree, void          *key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_str   (TREE *tree, char          *key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_long  (TREE *tree, long           key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_int   (TREE *tree, int            key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_short (TREE *tree, short          key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_schar (TREE *tree, signed char    key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_ulong (TREE *tree, unsigned long  key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_uint  (TREE *tree, unsigned int   key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_ushort(TREE *tree, unsigned short key) { return avl_f_locate_lt(tree, (long)key); }
void *avl_locate_lt_uchar (TREE *tree, unsigned char  key) { return avl_f_locate_lt(tree, (long)key); }

/*---------------------------------------------------------------------------*/

void *avl_locate_first(TREE *tree)
{
   NODE *node, *leftnode;

   node = tree->root;
   if ( !node) {
      return NULL;
   }
   while ((leftnode = node->left)) {
      node = leftnode;
   }
   return node->data;
}

/*---------------------------------------------------------------------------*/

void *avl_locate_last(TREE *tree)
{
   NODE *node, *rightnode;

   node = tree->root;
   if ( !node) {
      return NULL;
   }
   while ((rightnode = node->right)) {
      node = rightnode;
   }
   return node->data;
}

/*===========================================================================*/

static void *scan_subtree(NODE *root, bool (*callback)(void *))
{
   void *data;

   if (root->left) {
      if ((data = scan_subtree(root->left,  callback))) return data;
   }
   if ((*callback)(root->data)) return root->data;
   if (root->right) {
      if ((data = scan_subtree(root->right, callback))) return data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *backscan_subtree(NODE *root, bool (*callback)(void *))
{
   void *data;

   if (root->right) {
      if ((data = backscan_subtree(root->right, callback))) return data;
   }
   if ((*callback)(root->data)) return root->data;
   if (root->left) {
      if ((data = backscan_subtree(root->left,  callback))) return data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_scan(TREE *tree, bool (*callback)(void *))
{
   return tree->root ? scan_subtree(tree->root, callback) : NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_backscan(TREE *tree, bool (*callback)(void *))
{
   return tree->root ? backscan_subtree(tree->root, callback) : NULL;
}

/*===========================================================================*/

static void *walk_subtree(NODE *root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (root->left) {
      if ((data = walk_subtree(root->left,  callback, context))) return data;
   }
   if ((*callback)(root->data, context)) return root->data;
   if (root->right) {
      if ((data = walk_subtree(root->right, callback, context))) return data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

static void *backwalk_subtree(NODE *root, bool (*callback)(void *, void *), void *context)
{
   void *data;

   if (root->left) {
      if ((data = backwalk_subtree(root->left,  callback, context))) return data;
   }
   if ((*callback)(root->data, context)) return root->data;
   if (root->right) {
      if ((data = backwalk_subtree(root->right, callback, context))) return data;
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_walk(TREE *tree, bool (*callback)(void *, void *), void *context)
{
   return tree->root ? walk_subtree(tree->root, callback, context) : NULL;
}

/*---------------------------------------------------------------------------*/

void *avl_backwalk(TREE *tree, bool (*callback)(void *, void *), void *context)
{
   return tree->root ? backwalk_subtree(tree->root, callback, context) : NULL;
}

/*===========================================================================*/

void *avl_first(TREE *tree)
{
   PATH  *path;
   char  *pathright;
   NODE **pathnode;
   NODE  *node;

   if ( !tree->root) {
      return NULL;
   }
   if ( !tree->path) {
      ALLOC_PATH(path);
      if ( !path) {
         return NULL;
      }
      tree->path = path;
   } else {
      path = tree->path;
   }
   pathnode  = &path->node [0];
   pathright = &path->right[1];
   *  pathnode  = NULL; /* sentinels */
   *  pathright = true;
   *++pathnode  = NULL;
   *++pathright = false;
   *++pathnode  = node = tree->root;
   while ((node = node->left)) {
      *++pathright = false;
      *++pathnode  = node;
   }
   path->pathright = pathright;
   path->pathnode  = pathnode;
   return (*pathnode)->data;
}

/*---------------------------------------------------------------------------*/

void *avl_last(TREE *tree)
{
   PATH  *path;
   char  *pathright;
   NODE **pathnode;
   NODE  *node;

   if ( !tree->root) {
      return NULL;
   }
   if ( !tree->path) {
      ALLOC_PATH(path);
      if ( !path) {
         return NULL;
      }
      tree->path = path;
   } else {
      path = tree->path;
   }
   pathnode  = &path->node [0];
   pathright = &path->right[1];
   *  pathnode  = NULL; /* sentinels */
   *  pathright = false;
   *++pathnode  = NULL;
   *++pathright = true;
   *++pathnode  = node = tree->root;
   while ((node = node->right)) {
      *++pathright = true;
      *++pathnode  = node;
   }
   path->pathright = pathright;
   path->pathnode  = pathnode;
   return (*pathnode)->data;
}

/*---------------------------------------------------------------------------*/

#define DOWN_RIGHT_OR_BREAK(node, pathright, pathnode) { \
   (node) = (node)->right; \
   if (node) { \
      *++(pathright) = true; \
      *++(pathnode)  = (node); \
   } else { \
      break; \
   } \
}
#define DOWN_LEFT_OR_BREAK(node, pathright, pathnode) { \
   (node) = (node)->left; \
   if (node) { \
      *++(pathright) = false; \
      *++(pathnode)  = (node); \
   } else { \
      break; \
   } \
}
#define START_OK_AND_RETURN(path, _pathright, _pathnode) { \
   (path)->pathright = (_pathright); \
   (path)->pathnode  = (_pathnode); \
   return (*_pathnode)->data; \
}

/*---------------------------------------------------------------------------*/

static void *avl_f_start(TREE *tree, long keyval, bool back)
{
   PATH  *path;
   char  *pathright;
   NODE **pathnode;
   NODE  *node;
   char  *saveright;
   NODE **savenode;
   CMPFUN usrcmp;
   int    cmp;

   if ( !tree->root) {
      return NULL;
   }
   if ( !tree->path) {
      ALLOC_PATH(path);
      if ( !path) {
         return NULL;
      }
      tree->path = path;
   } else {
      path = tree->path;
   }
   pathnode  = &path->node [0];
   pathright = &path->right[1];
   saveright = NULL;
   savenode  = NULL;
   *  pathnode  = NULL; /* sentinels */
   *  pathright = !back;
   *++pathnode  = NULL;
   *++pathright = back;
   *++pathnode  = node = tree->root;
   switch (LOCTYPE(tree->keyinfo)) {
   CASE USR_NODUP:
   case STR_NODUP:
      usrcmp = tree->usrcmp;
      if (back) {
         for (;;) {
            cmp = PTRCMP(usrcmp, keyval, node->key.ptr);
            if (cmp > 0) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else if (cmp < 0) {
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else {
               START_OK_AND_RETURN(path, pathright, pathnode)
            }
         }
      } else {
         for (;;) {
            cmp = PTRCMP(usrcmp, keyval, node->key.ptr);
            if (cmp < 0) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else if (cmp > 0) {
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else {
               START_OK_AND_RETURN(path, pathright, pathnode)
            }
         }
      }
   CASE COR_NODUP:
      keyval = CORRECT(keyval);
   case VAL_NODUP:
      if (back) {
         for (;;) {
            if (keyval > node->key.val) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else if (keyval < node->key.val) {
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else {
               START_OK_AND_RETURN(path, pathright, pathnode)
            }
         }
      } else {
         for (;;) {
            if (keyval < node->key.val) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else if (keyval > node->key.val) {
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else {
               START_OK_AND_RETURN(path, pathright, pathnode)
            }
         }
      }
   CASE USR_DUP:
   case STR_DUP:
      usrcmp = tree->usrcmp;
      if (back) {
         for (;;) {
            cmp = PTRCMP(usrcmp, keyval, node->key.ptr);
            if (cmp >= 0) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else {
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            }
         }
      } else {
         for (;;) {
            cmp = PTRCMP(usrcmp, keyval, node->key.ptr);
            if (cmp <= 0) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else {
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            }
         }
      }
   CASE COR_DUP:
      keyval = CORRECT(keyval);
   case VAL_DUP:
      if (back) {
         for (;;) {
            if (keyval >= node->key.val) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            } else {
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            }
         }
      } else {
         for (;;) {
            if (keyval <= node->key.val) {
               saveright = pathright;
               savenode  = pathnode;
               DOWN_LEFT_OR_BREAK (node, pathright, pathnode)
            } else {
               DOWN_RIGHT_OR_BREAK(node, pathright, pathnode)
            }
         }
      }
   }
   if (savenode) {
      path->pathright = saveright;
      path->pathnode  = savenode;
      return (*savenode)->data;
   } else {
      FREE_PATH(path);
      tree->path = NULL;
      return NULL;
   }
}

void *avl_start       (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_mbr   (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_chars (TREE *tree, char          *key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_ptr   (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_str   (TREE *tree, char          *key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_long  (TREE *tree, long           key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_int   (TREE *tree, int            key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_short (TREE *tree, short          key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_schar (TREE *tree, signed char    key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_ulong (TREE *tree, unsigned long  key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_uint  (TREE *tree, unsigned int   key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_ushort(TREE *tree, unsigned short key) { return avl_f_start(tree, (long)key, false); }
void *avl_start_uchar (TREE *tree, unsigned char  key) { return avl_f_start(tree, (long)key, false); }

void *avl_backstart       (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_mbr   (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_chars (TREE *tree, char          *key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_ptr   (TREE *tree, void          *key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_str   (TREE *tree, char          *key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_long  (TREE *tree, long           key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_int   (TREE *tree, int            key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_short (TREE *tree, short          key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_schar (TREE *tree, signed char    key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_ulong (TREE *tree, unsigned long  key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_uint  (TREE *tree, unsigned int   key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_ushort(TREE *tree, unsigned short key) { return avl_f_start(tree, (long)key, true); }
void *avl_backstart_uchar (TREE *tree, unsigned char  key) { return avl_f_start(tree, (long)key, true); }

/*---------------------------------------------------------------------------*/

void *avl_next(TREE *tree)
{
   PATH  *path;
   char  *pathright;
   NODE **pathnode;
   NODE  *node;

   path = tree->path;
   if ( !path) {
      return NULL;
   }
   pathright = path->pathright;
   pathnode  = path->pathnode;
   node = (*pathnode)->right;
   if (node) {
      *++pathright = true;
      *++pathnode  = node;
      while ((node = node->left)) {
         *++pathright = false;
         *++pathnode  = node;
      }
   } else {
      while (*pathright) {
         --pathright;
         --pathnode;
      }
      --pathright;
      --pathnode;
      if ( !*pathnode) {
         FREE_PATH(path);
         tree->path = NULL;
         return NULL;
      }
   }
   path->pathright = pathright;
   path->pathnode  = pathnode;
   return (*pathnode)->data;
}

/*---------------------------------------------------------------------------*/

void *avl_prev(TREE *tree)
{
   PATH  *path;
   char  *pathright;
   NODE **pathnode;
   NODE  *node;

   path = tree->path;
   if ( !path) {
      return NULL;
   }
   pathright = path->pathright;
   pathnode  = path->pathnode;
   node = (*pathnode)->left;
   if (node) {
      *++pathright = false;
      *++pathnode  = node;
      while ((node = node->right)) {
         *++pathright = true;
         *++pathnode  = node;
      }
   } else {
      while ( !*pathright) {
         --pathright;
         --pathnode;
      }
      --pathright;
      --pathnode;
      if ( !*pathnode) {
         FREE_PATH(path);
         tree->path = NULL;
         return NULL;
      }
   }
   path->pathright = pathright;
   path->pathnode  = pathnode;
   return (*pathnode)->data;
}

/*---------------------------------------------------------------------------*/

void avl_stop(TREE *tree)
{
   if (tree->path) {
      FREE_PATH(tree->path);
      tree->path = NULL;
   }
}

/*===========================================================================*/

typedef struct {
   size_t offset;
   void  *data;
} LINK;

/*---------------------------------------------------------------------------*/

static void link_subtree(NODE *node, LINK *link)
{
   if (node->right) link_subtree(node->right, link);
   *(void **)((char *)node->data + link->offset) = link->data;
   link->data = node->data;
   if (node->left)  link_subtree(node->left, link);
}

/*---------------------------------------------------------------------------*/

static void backlink_subtree(NODE *node, LINK *link)
{
   if (node->left)  backlink_subtree(node->left, link);
   *(void **)((char *)node->data + link->offset) = link->data;
   link->data = node->data;
   if (node->right) backlink_subtree(node->right, link);
}

/*---------------------------------------------------------------------------*/

void *avl_linked_list(TREE *tree, size_t ptroffs, bool back)
{
   LINK link;
   link.offset = ptroffs;
   link.data   = NULL;
   if (tree->root) {
      if (back) backlink_subtree(tree->root, &link);
      else      link_subtree    (tree->root, &link);
   }
   return link.data;
}

/*===========================================================================*/

long avl_nodes(TREE *tree)
{
   return tree->nodes;
}

/*===========================================================================*/

static bool copy_subtree(NODE *newroot, NODE *root)
{
   newroot->key  = root->key;
   newroot->data = root->data;
   newroot->bal  = root->bal;

   if (root->left) {
      ALLOC_NODE(newroot->left);
      if ( !newroot->left) {
         return false;
      }
      if ( !copy_subtree(newroot->left,  root->left)) {
         FREE_NODE(newroot->left);
         return false;
      }
   } else {
      newroot->left = NULL;
   }
   if (root->right) {
      ALLOC_NODE(newroot->right);
      if ( !newroot->right) {
         return false;
      }
      if ( !copy_subtree(newroot->right, root->right)) {
         FREE_NODE(newroot->right);
         return false;
      }
   } else {
      newroot->right = NULL;
   }
   return true;
}

/*---------------------------------------------------------------------------*/

TREE *avl_copy(TREE *tree)
{
   TREE *newtree;

   ALLOC_TREE(newtree);
   if ( !newtree) {
      return NULL;
   }
   newtree->keyinfo = tree->keyinfo;
   newtree->keyoffs = tree->keyoffs;
   newtree->usrcmp  = tree->usrcmp;
   newtree->nodes   = tree->nodes;
   newtree->path    = NULL;

   if (tree->root) {
      ALLOC_NODE(newtree->root);
      if ( !copy_subtree(newtree->root, tree->root)) {
         FREE_NODE(newtree->root);
         avl_close(newtree);
         return NULL;
      }
   } else {
      newtree->root = NULL;
   }
   return newtree;
}

/*===========================================================================*/

static void release_subtree(NODE *root, void (*callback)(void *))
{
   if (root->left)  release_subtree(root->left,  callback);
   if (root->right) release_subtree(root->right, callback);
   (*callback)(root->data);
   FREE_NODE(root);
}

/*---------------------------------------------------------------------------*/

static void reset_subtree(NODE *root)
{
   if (root->left)  reset_subtree(root->left);
   if (root->right) reset_subtree(root->right);
   FREE_NODE(root);
}

/*---------------------------------------------------------------------------*/

void avl_release(TREE *tree, void (*callback)(void *))
{
   if (tree->root) {
      release_subtree(tree->root, callback);
   }
   tree->root  = NULL;
   tree->nodes = 0;
   if (tree->path) {
      FREE_PATH(tree->path);
      tree->path = NULL;
   }
}

/*---------------------------------------------------------------------------*/

void avl_reset(TREE *tree)
{
   if (tree->root) {
      reset_subtree(tree->root);
   }
   tree->root  = NULL;
   tree->nodes = 0;
   if (tree->path) {
      FREE_PATH(tree->path);
      tree->path = NULL;
   }
}

/*---------------------------------------------------------------------------*/

void avl_close(TREE *tree)
{
   if (tree->root) {
      reset_subtree(tree->root);
   }
   if (tree->path) {
     FREE_PATH(tree->path);
   }
   tree->keyinfo = (USHORT)-1;
   FREE_TREE(tree);
}
