/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                 avl_test.c                                 |
 |                                                                            |
 |               additional test functions for the AVL library                |
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

#include "avl_test.h"
#include "../avl.c"
#include <assert.h>

/*---------------------------------------------------------------------------*/

static int depth_x(X_NODE *root, bool check)
{
   int depth_left, depth_right;

   depth_left  = root->left  ? depth_x(PTR_OF(root->left ), check) : 0;
   depth_right = root->right ? depth_x(PTR_OF(root->right), check) : 0;
   if (check) {
      assert(depth_left  <= depth_right + 1);
      assert(depth_right <= depth_left  + 1);
      if (depth_left > depth_right) {
         assert(  IS_DEEPER(root->left ));
         assert( !IS_DEEPER(root->right));
      } else if (depth_left < depth_right) {
         assert( !IS_DEEPER(root->left ));
         assert(  IS_DEEPER(root->right));
      } else {
         assert( !IS_DEEPER(root->left ));
         assert( !IS_DEEPER(root->right));
      }
   }
   return (depth_left > depth_right ? depth_left : depth_right) + 1;
}

/*---------------------------------------------------------------------------*/

static int depth_l(L_NODE *root, bool check)
{
   int depth_left, depth_right;

   depth_left  = root->left  ? depth_l(PTR_OF(root->left ), check) : 0;
   depth_right = root->right ? depth_l(PTR_OF(root->right), check) : 0;
   if (check) {
      assert(depth_left  <= depth_right + 1);
      assert(depth_right <= depth_left  + 1);
      if (depth_left > depth_right) {
         assert(  IS_DEEPER(root->left ));
         assert( !IS_DEEPER(root->right));
      } else if (depth_left < depth_right) {
         assert( !IS_DEEPER(root->left ));
         assert(  IS_DEEPER(root->right));
      } else {
         assert( !IS_DEEPER(root->left ));
         assert( !IS_DEEPER(root->right));
      }
   }
   return (depth_left > depth_right ? depth_left : depth_right) + 1;
}

/*---------------------------------------------------------------------------*/

int avl_depth(TREE *tree)
{
   if (tree->root) {
      if (IS_X(tree)) return depth_x(tree->x_root, false);
      else            return depth_l(tree->l_root, false);
   } else {
      return 0;
   }
}

/*---------------------------------------------------------------------------*/

void avl_check_balance(TREE *tree)
{
   if (tree->root) {
      assert( !IS_DEEPER(tree->root));
      if (IS_X(tree)) depth_x(tree->x_root, true);
      else            depth_l(tree->l_root, true);
   }
}

/*---------------------------------------------------------------------------*/

static void prepare_for_dump_x(X_NODE *x_root, int idx, void **data_v, int *bal_v)
{
   data_v[idx] = x_root->data;
   if (IS_DEEPER(x_root->left)) {
      bal_v[idx] = IS_DEEPER(x_root->right) ? -2 : -1;
   } else {
      bal_v[idx] = IS_DEEPER(x_root->right) ?  1 :  0;
   }
   if (x_root->left)  prepare_for_dump_x(PTR_OF(x_root->left ), idx << 1    , data_v, bal_v);
   if (x_root->right) prepare_for_dump_x(PTR_OF(x_root->right), idx << 1 | 1, data_v, bal_v);
}

/*---------------------------------------------------------------------------*/

static void prepare_for_dump_l(L_NODE *l_root, int idx, void **data_v, int *bal_v)
{
   data_v[idx] = l_root->data;
   if (IS_DEEPER(l_root->left)) {
      bal_v[idx] = IS_DEEPER(l_root->right) ? -2 : -1;
   } else {
      bal_v[idx] = IS_DEEPER(l_root->right) ?  1 :  0;
   }
   if (l_root->left)  prepare_for_dump_l(PTR_OF(l_root->left ), idx << 1    , data_v, bal_v);
   if (l_root->right) prepare_for_dump_l(PTR_OF(l_root->right), idx << 1 | 1, data_v, bal_v);
}

/*---------------------------------------------------------------------------*/

void avl_dump(TREE *tree, void (*callback)(void *data, int idx, int bal))
{
   if (tree->root) {
      int    max_depth = 0;
      int    idx, idx_top;
      void **data_v;
      int   *bal_v;

      if (IS_X(tree)) max_depth = depth_x(PTR_OF(tree->x_root), false);
      else            max_depth = depth_l(PTR_OF(tree->l_root), false);
      idx_top = 1 << max_depth;
      data_v = calloc(idx_top, sizeof(void*));
      bal_v  = calloc(idx_top, sizeof(int));
      if (IS_X(tree)) prepare_for_dump_x(PTR_OF(tree->x_root), 1, data_v, bal_v);
      else            prepare_for_dump_l(PTR_OF(tree->l_root), 1, data_v, bal_v);
      for (idx = 1; idx < idx_top; idx++) {
         (*callback)(data_v[idx], idx, bal_v[idx]);
      }
      free(data_v);
      free(bal_v);
   } else {
      (*callback)(NULL, 1, 0);
   }
}

/*---------------------------------------------------------------------------*/

void avl_dump_w_ctx(TREE *tree, void (*callback)(void *data, int idx, int bal, void *context), void *context)
{
   if (tree->root) {
      int    max_depth = 0;
      int    idx, idx_top;
      void **data_v;
      int   *bal_v;

      if (IS_X(tree)) max_depth = depth_x(PTR_OF(tree->x_root), false);
      else            max_depth = depth_l(PTR_OF(tree->l_root), false);
      idx_top = 1 << max_depth;
      data_v = calloc(idx_top, sizeof(void*));
      bal_v  = calloc(idx_top, sizeof(int));
      if (IS_X(tree)) prepare_for_dump_x(PTR_OF(tree->x_root), 1, data_v, bal_v);
      else            prepare_for_dump_l(PTR_OF(tree->l_root), 1, data_v, bal_v);
      for (idx = 1; idx < idx_top; idx++) {
         (*callback)(data_v[idx], idx, bal_v[idx], context);
      }
      free(data_v);
      free(bal_v);
   } else {
      (*callback)(NULL, 1, 0, context);
   }
}

/*---------------------------------------------------------------------------*/

int avl_tree_type(TREE *tree)
{
   return tree->type;
}
