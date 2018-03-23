#ifndef TABLE_H
#define TABLE_H

#include <linux/rcupdate.h>
#include <linux/slab.h>

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
	WARN_ON(!tbl_empty(tbl));

	kfree(tbl->free_node);
}

/** tbl_lookup - Lookup value for key in the table.
 * @tbl:	Table root.
 * @key:	Key for lookup.
 *
 * Synopsis:
 *
 * rcu_read_lock();
 * val = tbl_lookup(&my_table, key);
 * if (val)
 *     my_val_routine(val);
 * rcu_read_unlock();
 *
 * Return: Value for key.
 */
static inline void *tbl_lookup(struct tbl_root *tbl, uint32_t key)
{
	struct tbl_node	*node;
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (WARN_ON(key >> TBL_KEY_SHIFT))
		return NULL;

	node = rcu_dereference(tbl->node[node_i]);
	if (!node)
		return NULL;

	return rcu_dereference(node->val[key & TBL_NODE_MASK]);
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
		tbl->free_node = kzalloc(sizeof(*tbl->free_node), GFP_KERNEL);
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
	kfree(tbl->free_node);
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
 * mutex_lock(&my_table_mut);
 * tbl_alloc_node(&my_table);
 * tbl_insert(&my_table, val, key);
 * mutex_unlock(&my_table_mut);
 */
static inline void tbl_insert(struct tbl_root *tbl, void *val, uint32_t key)
{
	struct tbl_node	*node;
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (WARN_ON(key >> TBL_KEY_SHIFT))
		return;

	node = tbl->node[node_i];
	if (!node)
		node = tbl->free_node;

	if (WARN_ON(!node))
		return;

	/* warning: old value will leak */
	WARN_ON(node->val[key & TBL_NODE_MASK]);

	rcu_assign_pointer(node->val[key & TBL_NODE_MASK], val);

	if (!tbl->refcount[node_i]) {
		rcu_assign_pointer(tbl->node[node_i], node);
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
 * mutex_lock(&my_table_mut);
 * tbl_free_node(&my_table);
 * tbl_delete(&my_table, key);
 * mutex_unlock(&my_table_mut);
 * synchronize_rcu();
 * kfree(old_value_at_key);
 */
static inline void tbl_delete(struct tbl_root *tbl, uint32_t key)
{
	struct tbl_node	*node;
	uint32_t node_i = key >> TBL_NODE_SHIFT;

	if (WARN_ON(key >> TBL_KEY_SHIFT))
		return;

	node = tbl->node[node_i];
	if (WARN_ON(!node) || WARN_ON(!node->val[key & TBL_NODE_MASK]))
		return;

	RCU_INIT_POINTER(node->val[key & TBL_NODE_MASK], NULL);

	--tbl->refcount[node_i];

	if (!tbl->refcount[node_i]) {
		/* warning: old free node will leak */
		WARN_ON(tbl->free_node);

		tbl->free_node = node;
		RCU_INIT_POINTER(tbl->node[node_i], NULL);
	}
}

#endif /* TABLE_H */
