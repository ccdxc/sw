#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif /* likely and unlikely */

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

/** tbl_empty - Check if the table is empty.
 * @tbl:	Table root.
 *
 * Return: Table is empty.
 */
static inline bool tbl_empty(struct tbl_root *tbl)
{
	uint32_t node_i;

	for (node_i = 0; node_i < TBL_ROOT_CAPACITY; ++node_i) {
		if (tbl->node[node_i])
			return false;
	}

	return true;
}

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
static inline void tbl_destroy(struct tbl_root *tbl)
{
	assert(tbl_empty(tbl));

	free(tbl->free_node);
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
