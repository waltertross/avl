/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avl.h                                    |
 |                                                                            |
 |             created by Walter Tross sometime before 1991-03-11             |
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

/* A simple but effective AVL trees library in C.
 *
 * Warning: sizeof(void *) == sizeof(long) is assumed.
 */

#ifndef AVL_H
#define AVL_H

#define AVL_AVAILABLE (sizeof(void *) == sizeof(long))

#include <stddef.h>
#include <stdbool.h>

/* Begin internal data structures - not for normal access.
 * All you should normally need is TREE * (like FILE *).
 */
union avl_key {
   void *ptr;
   long  val;
};

struct avl_node {
   union  avl_key   key;
   void            *data;
   struct avl_node *left;
   struct avl_node *right;
   int              bal;
};

#define AVL_MAX_PATHDEPTH (sizeof(long) * 8 * 3 / 2 - 2)

struct avl_path {
   struct avl_node **pathnode;
   char             *pathright;
   struct avl_node  *node [AVL_MAX_PATHDEPTH + 2];
   char              right[AVL_MAX_PATHDEPTH + 2];
};

typedef struct avl_tree {
   struct avl_node *root;
   struct avl_path *path;
   long             nodes;
   int            (*usrcmp)();
   unsigned short   keyinfo;
   unsigned short   keyoffs;
} TREE;

/* End internal data structures. */

#define AVL_USR  (0 << 1)
#define AVL_MBR  (1 << 1)
#define AVL_PTR  (2 << 1)
#define AVL_STR  (3 << 1)
#define AVL_CHA  (4 << 1)
#define AVL_LNG  (5 << 1)
#define AVL_INT  (6 << 1)
#define AVL_SHT  (7 << 1)
#define AVL_SCH  (8 << 1)
#define AVL_ULN  (9 << 1)
#define AVL_UIN (10 << 1)
#define AVL_USH (11 << 1)
#define AVL_UCH (12 << 1)

#define AVL_CHARS  AVL_CHA
#define AVL_LONG   AVL_LNG
#define AVL_SHORT  AVL_SHT
#define AVL_SCHAR  AVL_SCH
#define AVL_ULONG  AVL_ULN
#define AVL_UINT   AVL_UIN
#define AVL_USHORT AVL_USH
#define AVL_UCHAR  AVL_UCH

#define AVL_DUP 1

#define AVL_AVLCMP ((int(*)())0)

/* Tree creation macros (to be used like functions).
 * Nodup trees don't allow duplicate keys, dup trees do (conserving insertion order).
 * The usrcmp() function is passed two pointers and should return an int just like strcmp() (q.v.).
 * The pointers passed to usrcmp() are
 * - avl_tree_[no]dup     : the pointers to the data structs
 * - avl_tree_[no]dup_mbr : the pointers to the given members of the data structs
 * - avl_tree_[no]dup_ptr : the contents of the given pointer members of the data structs
 * avl_tree_[no]dup_str   is equivalent to avl_tree_nodup_ptr with strcmp as comparator.
 * avl_tree_[no]dup_chars is equivalent to avl_tree_nodup_mbr with strcmp as comparator.
 * avl_tree_[no]dup_T with T a numeric type is essentially equivalent to avl_tree_nodup_mbr
 * with a comparator that returns the signed difference of the two members (n1 - n2).
 * NULL is returned if sizeof(void *) != sizeof(long), or if memory allocation failed.
 */
#define avl_tree_nodup(usrcmp)                         avl_tree(AVL_USR, 0,                         (usrcmp))
#define avl_tree_nodup_mbr(   _struct, member, usrcmp) avl_tree(AVL_MBR, offsetof(_struct, member), (usrcmp))
#define avl_tree_nodup_ptr(   _struct, member, usrcmp) avl_tree(AVL_PTR, offsetof(_struct, member), (usrcmp))
#define avl_tree_nodup_str(   _struct, member)         avl_tree(AVL_STR, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_chars( _struct, member)         avl_tree(AVL_CHA, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_long(  _struct, member)         avl_tree(AVL_LNG, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_int(   _struct, member)         avl_tree(AVL_INT, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_short( _struct, member)         avl_tree(AVL_SHT, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_schar( _struct, member)         avl_tree(AVL_SCH, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_ulong( _struct, member)         avl_tree(AVL_ULN, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_uint(  _struct, member)         avl_tree(AVL_UIN, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_ushort(_struct, member)         avl_tree(AVL_USH, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_nodup_uchar( _struct, member)         avl_tree(AVL_UCH, offsetof(_struct, member), AVL_AVLCMP)

#define avl_tree_dup(usrcmp)                         avl_tree(AVL_USR|AVL_DUP, 0,                         (usrcmp))
#define avl_tree_dup_mbr(   _struct, member, usrcmp) avl_tree(AVL_MBR|AVL_DUP, offsetof(_struct, member), (usrcmp))
#define avl_tree_dup_ptr(   _struct, member, usrcmp) avl_tree(AVL_PTR|AVL_DUP, offsetof(_struct, member), (usrcmp))
#define avl_tree_dup_str(   _struct, member)         avl_tree(AVL_STR|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_chars( _struct, member)         avl_tree(AVL_CHA|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_long(  _struct, member)         avl_tree(AVL_LNG|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_int(   _struct, member)         avl_tree(AVL_INT|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_short( _struct, member)         avl_tree(AVL_SHT|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_schar( _struct, member)         avl_tree(AVL_SCH|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_ulong( _struct, member)         avl_tree(AVL_ULN|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_uint(  _struct, member)         avl_tree(AVL_UIN|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_ushort(_struct, member)         avl_tree(AVL_USH|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)
#define avl_tree_dup_uchar( _struct, member)         avl_tree(AVL_UCH|AVL_DUP, offsetof(_struct, member), AVL_AVLCMP)

/* String trees store strings instead of structs.
 */
#define avl_string_tree_nodup() avl_tree(AVL_CHARS,         0, AVL_AVLCMP);
#define avl_string_tree_dup()   avl_tree(AVL_CHARS|AVL_DUP, 0, AVL_AVLCMP);

/* It is suggested to use the avl_tree_[no]dup[_TYPE]() macros instead of avl_tree(),
 * unless the tree type has to be parametrized.
 */
TREE *avl_tree(int treetype, size_t keyoffs, int (*usrcmp)());

/* Insert data into the tree.
 * A node is created with a copy of the pointer to the key or a copy of the integer value of the key,
 * and a pointer to the data. The data is NOT owned by the tree.
 * True is returned for success, false for failure. Insertions can fail because
 * - the tree is a nodup tree and the key is already present, or
 * - memory allocation has failed.
 */
bool avl_insert(TREE *tree, void *data);

/* Remove a the (first) node with the given key from the tree (data is untouched).
 * The pointer to the data is returned, or NULL if the key was not found.
 */
#define avl_remove(tree, key)  avl_f_remove((tree), (long)(key))

/* Locate a key in the tree (the first/leftmost key in case of a tree with duplicates),
 * returning the pointer to the data, or NULL if not found.
 * Use this function before an avl_insert that can easily fail, because it is faster.
 */
#define avl_locate(tree, key)  avl_f_locate((tree), (long)(key))

/* Locate the first key that is >=, >, <= or < the given one,
 * returning the pointer to the data, or NULL if not found.
 * First means the leftmost for >= and >, and the rightmost for <= and <.
 */
#define avl_locate_ge(tree, key)  avl_f_locate_ge((tree), (long)(key))
#define avl_locate_gt(tree, key)  avl_f_locate_gt((tree), (long)(key))
#define avl_locate_le(tree, key)  avl_f_locate_le((tree), (long)(key))
#define avl_locate_lt(tree, key)  avl_f_locate_lt((tree), (long)(key))

/* Locate the first/last (leftmost/rightmost) node of the tree (NULL if not found).
 */
void *avl_locate_first(TREE *tree);
void *avl_locate_last (TREE *tree);

/* Scan a tree [backwards] passing all data pointers to a function.
 * The tree may not be altered during the scan.
 */
#define avl_scan(    tree, usrfun)  avl_f_scan((tree), (usrfun), false)
#define avl_backscan(tree, usrfun)  avl_f_scan((tree), (usrfun), true )

/* Begin macros and functions for scanning trees without callback functions.
 * E.g.: for (data *p = avl_first(tree); p; p = avl_next(tree)) { ... }
 * (you may find it convenient to enclose this for(...) in a macro, with or without the type of *p).
 * A structure is allocated to keep track of the path to the current node in the tree.
 * Any change to the tree deallocates the path, thus interrupting the scan.
 * The path is also deallocated any time NULL is returned because no matching node was found.
 */

/* Start a tree scan from the leftmost/rightmost node (NULL if the tree is empty).
 */
void *avl_first(TREE *tree);
void *avl_last (TREE *tree);

/* Start a tree scan from the leftmost/rightmost node with a given key (NULL if none matches).
 */
#define avl_start(    tree, key)  avl_f_start((tree), (long)(key), false)
#define avl_backstart(tree, key)  avl_f_start((tree), (long)(key), true )

/* Advance to the next/previous node (NULL if there is none).
 */
void *avl_next(TREE *tree);
void *avl_prev(TREE *tree);

/* Deallocate the path (not needed if the scan terminated because a NULL was returned).
 */
void avl_stop(TREE *tree);

/* End macros and functions for scanning trees without callback functions.
 */

/* Make a linked list out of the data in a tree by providing the "next"/"prev" pointer member,
 * returning the head of the list.
 */ 
#define avl_link(    tree, _struct, next)  avl_f_link((tree), offsetof(_struct, next), false)
#define avl_backlink(tree, _struct, prev)  avl_f_link((tree), offsetof(_struct, prev), true )

/* Return the number of nodes in a tree.
 */
#define avl_nodes(tree) ((tree)->nodes)

/* Copy a tree (the structure and the nodes, but not the data nor the scan path).
 */
TREE *avl_copy(TREE *tree);

/* Empty a tree releasing all its data to a function (e.g., the free() function).
 */
void avl_release(TREE *tree, void (*usrfun)());

/* Empty a tree (the data is untouched).
 */
void avl_reset(TREE *tree);

/* Close a tree for ever (the data is untouched).
 */
void avl_close(TREE *tree);

/* These functions are not meant to be used directly,
 * the corresponding macros should be used instead.
 */
void *avl_f_locate   (TREE *tree, long keyval);
void *avl_f_locate_ge(TREE *tree, long keyval);
void *avl_f_locate_gt(TREE *tree, long keyval);
void *avl_f_locate_le(TREE *tree, long keyval);
void *avl_f_locate_lt(TREE *tree, long keyval);
void *avl_f_remove   (TREE *tree, long keyval);
void  avl_f_scan (TREE *tree, void (*usrfun)(), bool back);
void *avl_f_start(TREE *tree, long keyval,      bool back);
void *avl_f_link (TREE *tree, size_t ptroffs,   bool back);

#endif
