/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avl.h                                    |
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

#ifndef AVL_H
#define AVL_H

#include <stddef.h>
#include <stdbool.h>

typedef struct avl_tree TREE;

/*      AVL_USR is guaranteed to be 0 */
#define AVL_USR      0
#define AVL_MBR    ( 1 << 1)
#define AVL_PTR    ( 2 << 1)
#define AVL_CHARS  ( 3 << 1)
#define AVL_STR    ( 4 << 1)
#define AVL_LONG   ( 5 << 1)
#define AVL_INT    ( 6 << 1)
#define AVL_SHORT  ( 7 << 1)
#define AVL_SCHAR  ( 8 << 1)
#define AVL_ULONG  ( 9 << 1)
#define AVL_UINT   (10 << 1)
#define AVL_USHORT (11 << 1)
#define AVL_UCHAR  (12 << 1)
#define AVL_FLOAT  (13 << 1)
#define AVL_DOUBLE (14 << 1)

/*      AVL_NODUP is guaranteed to be 0 */
#define AVL_NODUP 0
#define AVL_DUP   1

/* Tree creation macros (to be used like functions).
 * Nodup trees don't allow duplicate keys, dup trees do (conserving insertion order).
 * The usrcmp() function is passed two pointers and should return an int just like strcmp() (q.v.).
 * The pointers passed to usrcmp() are
 * - avl_tree_[no]dup     : the pointers to the data structs
 * - avl_tree_[no]dup_mbr : the pointers to the given members of the data structs
 * - avl_tree_[no]dup_ptr : the contents of the given pointer members of the data structs.
 * avl_tree_[no]dup_chars is equivalent to avl_tree_nodup_mbr with strcmp as comparator.
 * avl_tree_[no]dup_str   is equivalent to avl_tree_nodup_ptr with strcmp as comparator.
 * avl_tree_[no]dup_T with T a numeric type is essentially equivalent to avl_tree_nodup_mbr
 * with a comparator that returns the signed difference of the two members (n1 - n2).
 * NULL is returned if memory allocation failed.
 */
#define avl_tree_nodup(usrcmp)                         avl_tree(0,          0,                         (usrcmp))
#define avl_tree_nodup_mbr(   _struct, member, usrcmp) avl_tree(AVL_MBR,    offsetof(_struct, member), (usrcmp))
#define avl_tree_nodup_ptr(   _struct, member, usrcmp) avl_tree(AVL_PTR,    offsetof(_struct, member), (usrcmp))
#define avl_tree_nodup_chars( _struct, member)         avl_tree(AVL_CHARS,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_str(   _struct, member)         avl_tree(AVL_STR,    offsetof(_struct, member), NULL)
#define avl_tree_nodup_long(  _struct, member)         avl_tree(AVL_LONG,   offsetof(_struct, member), NULL)
#define avl_tree_nodup_int(   _struct, member)         avl_tree(AVL_INT,    offsetof(_struct, member), NULL)
#define avl_tree_nodup_short( _struct, member)         avl_tree(AVL_SHORT,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_schar( _struct, member)         avl_tree(AVL_SCHAR,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_ulong( _struct, member)         avl_tree(AVL_ULONG,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_uint(  _struct, member)         avl_tree(AVL_UINT,   offsetof(_struct, member), NULL)
#define avl_tree_nodup_ushort(_struct, member)         avl_tree(AVL_USHORT, offsetof(_struct, member), NULL)
#define avl_tree_nodup_uchar( _struct, member)         avl_tree(AVL_UCHAR,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_float( _struct, member)         avl_tree(AVL_FLOAT,  offsetof(_struct, member), NULL)
#define avl_tree_nodup_double(_struct, member)         avl_tree(AVL_DOUBLE, offsetof(_struct, member), NULL)

#define avl_tree_dup(usrcmp)                         avl_tree(           AVL_DUP, 0,                         (usrcmp))
#define avl_tree_dup_mbr(   _struct, member, usrcmp) avl_tree(AVL_MBR   |AVL_DUP, offsetof(_struct, member), (usrcmp))
#define avl_tree_dup_ptr(   _struct, member, usrcmp) avl_tree(AVL_PTR   |AVL_DUP, offsetof(_struct, member), (usrcmp))
#define avl_tree_dup_chars( _struct, member)         avl_tree(AVL_CHARS |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_str(   _struct, member)         avl_tree(AVL_STR   |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_long(  _struct, member)         avl_tree(AVL_LONG  |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_int(   _struct, member)         avl_tree(AVL_INT   |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_short( _struct, member)         avl_tree(AVL_SHORT |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_schar( _struct, member)         avl_tree(AVL_SCHAR |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_ulong( _struct, member)         avl_tree(AVL_ULONG |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_uint(  _struct, member)         avl_tree(AVL_UINT  |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_ushort(_struct, member)         avl_tree(AVL_USHORT|AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_uchar( _struct, member)         avl_tree(AVL_UCHAR |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_float( _struct, member)         avl_tree(AVL_FLOAT |AVL_DUP, offsetof(_struct, member), NULL)
#define avl_tree_dup_double(_struct, member)         avl_tree(AVL_DOUBLE|AVL_DUP, offsetof(_struct, member), NULL)

/* String trees store strings instead of structs.
 */
#define avl_string_tree_nodup() avl_tree(AVL_CHARS,         0, NULL);
#define avl_string_tree_dup()   avl_tree(AVL_CHARS|AVL_DUP, 0, NULL);

/* It is suggested to use the avl_tree_[no]dup[_TYPE]() macros instead of avl_tree(),
 * unless the tree type has to be parametric.
 */
TREE *avl_tree(int treetype, size_t keyoffs, int (*usrcmp)());

/* Functions to check whether floats/doubles are handled by "type punning" or by callbacks.
 */
bool avl_has_fast_floats (void);
bool avl_has_fast_doubles(void);

/* Insert data into the tree.
 * A node is created, with a pointer to the data and possibly with a [partial] copy of the key.
 * True is returned for success, false for failure. Insertions can fail because
 * - the tree is a nodup tree and the key is already present, or
 * - memory allocation has failed.
 * Float and double keys should never be NaN.
 */
bool avl_insert(TREE *tree, void *data);

/* Remove a node with the given key from the tree (data is untouched).
 * In case of dup trees, the oldest/leftmost node with the given key is removed.
 * The pointer to the data is returned, or NULL if the key was not found.
 * The functions with pointer keys are interchangeable, and so are those with integer keys.
 * E.g., you can avl_remove(tree, "abc") instead of avl_remove_str(tree, "abc").
 */
void *avl_remove       (TREE *tree, void *key);
void *avl_remove_mbr   (TREE *tree, void *key);
void *avl_remove_ptr   (TREE *tree, void *key);
void *avl_remove_chars (TREE *tree, char *key);
void *avl_remove_str   (TREE *tree, char *key);
void *avl_remove_long  (TREE *tree, long           key);
void *avl_remove_int   (TREE *tree, int            key);
void *avl_remove_short (TREE *tree, short          key);
void *avl_remove_schar (TREE *tree, signed char    key);
void *avl_remove_ulong (TREE *tree, unsigned long  key);
void *avl_remove_uint  (TREE *tree, unsigned int   key);
void *avl_remove_ushort(TREE *tree, unsigned short key);
void *avl_remove_uchar (TREE *tree, unsigned char  key);
void *avl_remove_float (TREE *tree, float          key);
void *avl_remove_double(TREE *tree, double         key);

/* Locate a key in the tree (the oldest/leftmost key in case of a tree with duplicates),
 * returning the pointer to the data, or NULL if not found.
 * Use these function before an avl_insert that can easily fail, because it is faster.
 * The functions with pointer keys are interchangeable, and so are those with integer keys.
 * E.g., you can avl_locate(tree, "abc") instead of avl_locate_str(tree, "abc").
 */
void *avl_locate       (TREE *tree, void *key);
void *avl_locate_mbr   (TREE *tree, void *key);
void *avl_locate_ptr   (TREE *tree, void *key);
void *avl_locate_chars (TREE *tree, char *key);
void *avl_locate_str   (TREE *tree, char *key);
void *avl_locate_long  (TREE *tree, long           key);
void *avl_locate_int   (TREE *tree, int            key);
void *avl_locate_short (TREE *tree, short          key);
void *avl_locate_schar (TREE *tree, signed char    key);
void *avl_locate_ulong (TREE *tree, unsigned long  key);
void *avl_locate_uint  (TREE *tree, unsigned int   key);
void *avl_locate_ushort(TREE *tree, unsigned short key);
void *avl_locate_uchar (TREE *tree, unsigned char  key);
void *avl_locate_float (TREE *tree, float          key);
void *avl_locate_double(TREE *tree, double         key);

/* Locate the first key that is >=, >, <= or < the given one,
 * returning the pointer to the data, or NULL if not found.
 * First means the leftmost for >= and >, and the rightmost for <= and <.
 * The functions with pointer keys are interchangeable, and so are those with value keys.
 * E.g., you can avl_locate_gt(tree, "abc") instead of avl_locate_gt_str(tree, "abc").
 */
void *avl_locate_ge       (TREE *tree, void *key);
void *avl_locate_gt       (TREE *tree, void *key);
void *avl_locate_le       (TREE *tree, void *key);
void *avl_locate_lt       (TREE *tree, void *key);
void *avl_locate_ge_mbr   (TREE *tree, void *key);
void *avl_locate_gt_mbr   (TREE *tree, void *key);
void *avl_locate_le_mbr   (TREE *tree, void *key);
void *avl_locate_lt_mbr   (TREE *tree, void *key);
void *avl_locate_ge_ptr   (TREE *tree, void *key);
void *avl_locate_gt_ptr   (TREE *tree, void *key);
void *avl_locate_le_ptr   (TREE *tree, void *key);
void *avl_locate_lt_ptr   (TREE *tree, void *key);
void *avl_locate_ge_chars (TREE *tree, char *key);
void *avl_locate_gt_chars (TREE *tree, char *key);
void *avl_locate_le_chars (TREE *tree, char *key);
void *avl_locate_lt_chars (TREE *tree, char *key);
void *avl_locate_ge_str   (TREE *tree, char *key);
void *avl_locate_gt_str   (TREE *tree, char *key);
void *avl_locate_le_str   (TREE *tree, char *key);
void *avl_locate_lt_str   (TREE *tree, char *key);
void *avl_locate_ge_long  (TREE *tree, long           key);
void *avl_locate_gt_long  (TREE *tree, long           key);
void *avl_locate_le_long  (TREE *tree, long           key);
void *avl_locate_lt_long  (TREE *tree, long           key);
void *avl_locate_ge_int   (TREE *tree, int            key);
void *avl_locate_gt_int   (TREE *tree, int            key);
void *avl_locate_le_int   (TREE *tree, int            key);
void *avl_locate_lt_int   (TREE *tree, int            key);
void *avl_locate_ge_short (TREE *tree, short          key);
void *avl_locate_gt_short (TREE *tree, short          key);
void *avl_locate_le_short (TREE *tree, short          key);
void *avl_locate_lt_short (TREE *tree, short          key);
void *avl_locate_ge_schar (TREE *tree, signed char    key);
void *avl_locate_gt_schar (TREE *tree, signed char    key);
void *avl_locate_le_schar (TREE *tree, signed char    key);
void *avl_locate_lt_schar (TREE *tree, signed char    key);
void *avl_locate_ge_ulong (TREE *tree, unsigned long  key);
void *avl_locate_gt_ulong (TREE *tree, unsigned long  key);
void *avl_locate_le_ulong (TREE *tree, unsigned long  key);
void *avl_locate_lt_ulong (TREE *tree, unsigned long  key);
void *avl_locate_ge_uint  (TREE *tree, unsigned int   key);
void *avl_locate_gt_uint  (TREE *tree, unsigned int   key);
void *avl_locate_le_uint  (TREE *tree, unsigned int   key);
void *avl_locate_lt_uint  (TREE *tree, unsigned int   key);
void *avl_locate_ge_ushort(TREE *tree, unsigned short key);
void *avl_locate_gt_ushort(TREE *tree, unsigned short key);
void *avl_locate_le_ushort(TREE *tree, unsigned short key);
void *avl_locate_lt_ushort(TREE *tree, unsigned short key);
void *avl_locate_ge_uchar (TREE *tree, unsigned char  key);
void *avl_locate_gt_uchar (TREE *tree, unsigned char  key);
void *avl_locate_le_uchar (TREE *tree, unsigned char  key);
void *avl_locate_lt_uchar (TREE *tree, unsigned char  key);
void *avl_locate_ge_float (TREE *tree, float          key);
void *avl_locate_gt_float (TREE *tree, float          key);
void *avl_locate_le_float (TREE *tree, float          key);
void *avl_locate_lt_float (TREE *tree, float          key);
void *avl_locate_ge_double(TREE *tree, double         key);
void *avl_locate_gt_double(TREE *tree, double         key);
void *avl_locate_le_double(TREE *tree, double         key);
void *avl_locate_lt_double(TREE *tree, double         key);

/* Locate the first/last (leftmost/rightmost) node of the tree (NULL if not found).
 */
void *avl_locate_first(TREE *tree);
void *avl_locate_last (TREE *tree);

/* Scan a tree [in reverse] passing all data pointers to a callback function,
 * which may return true to stop the scan and return the current data pointer.
 * If the callback never returns true, a full scan is made and NULL is returned.
 * The tree may not be altered during the scan.
 */
void *avl_scan    (TREE *tree, bool (*callback)());
void *avl_rev_scan(TREE *tree, bool (*callback)());

/* Like avl[_rev]_scan(), but passing a context pointer as a second argument to the callback function.
 */
void *avl_scan_w_ctx    (TREE *tree, bool (*callback)(), void *context);
void *avl_rev_scan_w_ctx(TREE *tree, bool (*callback)(), void *context);

/* Pass [in reverse order] all data pointers to a callback function.
 */
void avl_do    (TREE *tree, void (*callback)());
void avl_rev_do(TREE *tree, void (*callback)());

/* Like avl[_rev]_do(), but passing a context pointer as a second argument to the callback function.
 */
void avl_do_w_ctx    (TREE *tree, void (*callback)(), void *context);
void avl_rev_do_w_ctx(TREE *tree, void (*callback)(), void *context);

/* BEGIN Macros and functions for traversing trees without callback functions.
 * E.g.: mystruct *p; for (p = avl_first(tree); p; p = avl_next(tree)) { ... }
 * (you may find it convenient to enclose this for(...) in a macro).
 * A structure is allocated to keep track of the path to the current node in the tree.
 * Any change to the tree deallocates the path, thus interrupting the traversal.
 * The path is also deallocated any time NULL is returned because no matching node was found.
 */

/* Start a tree traversal from the leftmost/rightmost node (NULL if the tree is empty).
 */
void *avl_first(TREE *tree);
void *avl_last (TREE *tree);

/* Start a tree traversal from the node that would be found by avl_locate_ge [avl_locate_le] (q.v.).
 * The functions with pointer keys are interchangeable, and so are those with integer keys.
 * E.g., you can avl_start(tree, "abc") instead of avl_start_str(tree, "abc").
 */
void *avl_start       (TREE *tree, void *key);
void *avl_start_mbr   (TREE *tree, void *key);
void *avl_start_ptr   (TREE *tree, void *key);
void *avl_start_chars (TREE *tree, char *key);
void *avl_start_str   (TREE *tree, char *key);
void *avl_start_long  (TREE *tree, long           key);
void *avl_start_int   (TREE *tree, int            key);
void *avl_start_short (TREE *tree, short          key);
void *avl_start_schar (TREE *tree, signed char    key);
void *avl_start_ulong (TREE *tree, unsigned long  key);
void *avl_start_uint  (TREE *tree, unsigned int   key);
void *avl_start_ushort(TREE *tree, unsigned short key);
void *avl_start_uchar (TREE *tree, unsigned char  key);
void *avl_start_float (TREE *tree, float          key);
void *avl_start_double(TREE *tree, double         key);
void *avl_rev_start       (TREE *tree, void *key);
void *avl_rev_start_mbr   (TREE *tree, void *key);
void *avl_rev_start_ptr   (TREE *tree, void *key);
void *avl_rev_start_chars (TREE *tree, char *key);
void *avl_rev_start_str   (TREE *tree, char *key);
void *avl_rev_start_long  (TREE *tree, long           key);
void *avl_rev_start_int   (TREE *tree, int            key);
void *avl_rev_start_short (TREE *tree, short          key);
void *avl_rev_start_schar (TREE *tree, signed char    key);
void *avl_rev_start_ulong (TREE *tree, unsigned long  key);
void *avl_rev_start_uint  (TREE *tree, unsigned int   key);
void *avl_rev_start_ushort(TREE *tree, unsigned short key);
void *avl_rev_start_uchar (TREE *tree, unsigned char  key);
void *avl_rev_start_float (TREE *tree, float          key);
void *avl_rev_start_double(TREE *tree, double         key);

/* Advance to the next/previous node (NULL if there is none).
 * Normally you would only use avl_next() after avl_first() or avl_start(), and only
 * avl_prev() after avl_last() or avl_rev_start(), but you can also mix avl_next() and avl_prev().
 */
void *avl_next(TREE *tree);
void *avl_prev(TREE *tree);

/* Deallocate the path (not needed if the traversal terminated because a NULL was returned).
 */
void avl_stop(TREE *tree);

/* END Macros and functions for traversing trees without callback functions.
 */

/* Make a linked list out of the data in a tree by providing the "next"/"prev" pointer member,
 * returning the head of the list.
 */
#define avl_link(    tree, _struct, next)  avl_linked_list((tree), offsetof(_struct, next), false)
#define avl_rev_link(tree, _struct, prev)  avl_linked_list((tree), offsetof(_struct, prev), true )
void *avl_linked_list(TREE *tree, size_t ptroffs, bool rev);

/* Return the number of nodes in a tree. LONG_MIN means that the tree has reached its
 * maximum capacity (which is LONG_MAX + 1) and won't allow further inserts.
 */
long avl_nodes(TREE *tree);

/* Copy a tree (the structure and the nodes, but not the data nor the path used by avl_next()).
 */
TREE *avl_copy(TREE *tree);

/* Empty a tree and free all node and path memory (the data is untouched).
 */
void avl_empty(TREE *tree);

/* Free all tree memory (the data is untouched).
 */
void avl_free(TREE *tree);


#endif
