/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Number of valid bits in a key */
#define TBL_KEY_SHIFT           24

/* Number bits used for index in a node */
#define TBL_NODE_SHIFT          12

#define TBL_NODE_MASK           ((1u << TBL_NODE_SHIFT) - 1u)
#define TBL_NODE_CAPACITY       (1u << TBL_NODE_SHIFT)
#define TBL_ROOT_CAPACITY       (1u << (TBL_KEY_SHIFT - TBL_NODE_SHIFT))

struct tbl_node {
	void			*val[TBL_NODE_CAPACITY];
};

struct tbl_root {
	/* for lookup in table */
	struct tbl_node		*node[TBL_ROOT_CAPACITY];

	/* for insertion and deletion in table */
	int			refcount[TBL_ROOT_CAPACITY];
	struct tbl_node		*free_node;
};

/** tbl_init - Initialize a table.
 * @tbl:	Table root.
 */
static inline void tbl_init(struct tbl_root *tbl)
{
	uint32_t node_i;

	tbl->free_node = NULL;

	for (node_i = 0; node_i < TBL_ROOT_CAPACITY; ++node_i) {
		tbl->node[node_i] = NULL;
		tbl->refcount[node_i] = 0;
	}
}

/** tbl_init - Destroy the table, which should be empty.
 * @tbl:	Table root.
 */
static inline int tbl_destroy(struct tbl_root *tbl)
{
	uint32_t node_i;
	int rc = 0;

	/* The table should be empty.  If not empty, it means the context is
	 * being destroyed, but there are qps still in the table that have not
	 * been destroyed.
	 *
	 * The interface is such that freeing the context must succeed, so here
	 * will make a best effort to free table resources.  Any qps that were
	 * not destroyed, however will still refer to the context after it is
	 * freed.  Those qps must not be used, not even for ibv_destroy_qp, or
	 * the application will likely crash.
	 *
	 * This best-effort freeing of resources replaces an assert.  The
	 * assert was seen in perftest, which will destroy the context even if
	 * there is an error destroying a qp or other resource.
	 */
	for (node_i = 0; node_i < TBL_ROOT_CAPACITY; ++node_i) {
		if (tbl->node[node_i]) {
			/* Indicate to the caller that the table was not empty,
			 * but still make a best effort to free the table.
			 */
			rc = -EBUSY;
			free(tbl->node[node_i]);
		}
	}

	free(tbl->free_node);

	return rc;
}

/** tbl_lookup - Lookup value for key in the table.
 * @tbl:	Table root.
 * @key:	Key for lookup.
 *
 * Synopsis:
 *
 * pthread_spin_lock(&my_table_lock);
 * val = tbl_lookup(&my_table, key);
 * if (val)
 *     my_val_routine(val);
 * pthread_spin_unlock(&my_table_lock);
 *
 * Return: Value for key.
 */
static inline void *tbl_lookup(struct tbl_root *tbl, uint32_t key)
{
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (unlikely(key >> TBL_KEY_SHIFT))
		return NULL;

	if (unlikely(!tbl->node[node_i]))
		return NULL;

	return tbl->node[node_i]->val[key & TBL_NODE_MASK];
}

/** tbl_alloc_node - Allocate the free node prior to insertion.
 * @tbl:	Table root.
 *
 * This should be called before inserting.
 *
 * Synopsis: see tbl_insert().
 */
static inline void tbl_alloc_node(struct tbl_root *tbl)
{
	if (!tbl->free_node)
		tbl->free_node = calloc(1, sizeof(*tbl->free_node));
}

/** tbl_free_node - Free the free node prior to deletion.
 * @tbl:	Table root.
 *
 * This should be called before deleting.
 *
 * Synopsis: see tbl_delete().
 */
static inline void tbl_free_node(struct tbl_root *tbl)
{
	free(tbl->free_node);
	tbl->free_node = NULL;
}

/** tbl_insert - Insert a value for key in the table.
 * @tbl:	Table root.
 * @val:	Value to insert.
 * @key:	Key to insert.
 *
 * The tbl->free_node must not be null when inserting.
 *
 * Synopsis:
 *
 * pthread_mutex_lock(&my_table_mut);
 * tbl_alloc_node(&my_table);
 * tbl_insert(&my_table, val, key);
 * pthread_mutex_unlock(&my_table_mut);
 *
 * pthread_spin_lock(&my_table_lock);
 * pthread_spin_unlock(&my_table_lock);
 */
static inline void tbl_insert(struct tbl_root *tbl, void *val, uint32_t key)
{
	struct tbl_node	*node;
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (unlikely(key >> TBL_KEY_SHIFT)) {
		assert(key >> TBL_KEY_SHIFT == 0);
		return;
	}

	node = tbl->node[node_i];
	if (!node)
		node = tbl->free_node;

	if (unlikely(!node)) {
		assert(node != NULL);
		return;
	}

	/* warning: with NDEBUG the old value will leak */
	assert(node->val[key & TBL_NODE_MASK] == NULL);

	node->val[key & TBL_NODE_MASK] = val;

	if (!tbl->refcount[node_i]) {
		tbl->node[node_i] = node;
		tbl->free_node = NULL;
	}

	++tbl->refcount[node_i];
}

/** tbl_delete - Delete the value for key in the table.
 * @tbl:	Table root.
 * @val:	Value to insert.
 * @key:	Key to insert.
 *
 * The tbl->free_node must be null when deleting.
 *
 * Synopsis:
 *
 * pthread_mutex_lock(&my_table_mut);
 * tbl_free_node(&my_table);
 * tbl_delete(&my_table, key);
 * pthread_mutex_unlock(&my_table_mut);
 *
 * pthread_spin_lock(&my_table_lock);
 * pthread_spin_unlock(&my_table_lock);
 * free(old_val_at_key);
 */
static inline void tbl_delete(struct tbl_root *tbl, uint32_t key)
{
	struct tbl_node	*node;
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (unlikely(key >> TBL_KEY_SHIFT)) {
		assert(key >> TBL_KEY_SHIFT == 0);
		return;
	}

	node = tbl->node[node_i];
	if (unlikely(node == NULL)) {
		assert(node != NULL);
		return;
	}

	if (unlikely(!node->val[key & TBL_NODE_MASK])) {
		assert(node->val[key & TBL_NODE_MASK] != NULL);
		return;
	}

	node->val[key & TBL_NODE_MASK] = NULL;

	--tbl->refcount[node_i];

	if (!tbl->refcount[node_i]) {
		/* warning: with NDEBUG the old free node will leak */
		assert(node != NULL);
		tbl->free_node = node;
		tbl->node[node_i] = NULL;
	}
}

#endif /* TABLE_H */
