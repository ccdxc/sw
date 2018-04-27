
#include "pnso_sim.h"

/* Copy a buffer list into a flat buffer.
 * Assumes the flat buffer has been initialized with available length, or 0 for PNSO_MAX_BUFFER_LEN.
 * Return count of bytes copied.
 */
uint32_t pnso_memcpy_list_to_flat_buf(struct pnso_flat_buffer *dst,
				      const struct pnso_buffer_list
				      *src_list)
{
	const struct pnso_flat_buffer *src;
	uint32_t cpy_len, max_len =
	    dst->len ? dst->len : PNSO_MAX_BUFFER_LEN;
	size_t buf_i;

	dst->len = 0;
	for (buf_i = 0; buf_i < src_list->count && dst->len < max_len;
	     buf_i++) {
		src = &src_list->buffers[buf_i];
		cpy_len = src->len;
		if (dst->len + cpy_len > max_len) {
			cpy_len = max_len - dst->len;
		}
		/* TODO: use a macro for memcpy, for use in kernel and userspace */
		memcpy((uint8_t *) dst->buf + dst->len,
		       (uint8_t *) src->buf, cpy_len);
		dst->len += cpy_len;
	}

	return dst->len;
}

/* Copy a flat buffer into a buffer list.
 * Assumes the buffer list lengths have been filled out with allocated size, or 0 for default block size.
 * Return count of bytes copied.
 */
uint32_t pnso_memcpy_flat_buf_to_list(struct pnso_buffer_list * dst_list,
				      const struct pnso_flat_buffer * src)
{
	struct pnso_flat_buffer *dst;
	uint32_t cpy_len, total = 0;
	size_t buf_i;

	for (buf_i = 0; buf_i < dst_list->count && total < src->len;
	     buf_i++) {
		dst = &dst_list->buffers[buf_i];
		cpy_len = dst->len ? dst->len : PNSO_DEFAULT_BLOCK_SZ;
		if (cpy_len > src->len - total) {
			cpy_len = src->len - total;
		}
		/* TODO: use a macro for memcpy, for use in kernel and userspace */
		memcpy((uint8_t *) dst->buf, (uint8_t *) src->buf + total,
		       cpy_len);
		dst->len = cpy_len;
		total += cpy_len;
	}
	dst_list->count = buf_i;

	return total;
}

#define KEY_ENTRY_BLOCK_COUNT 16

struct pnso_sim_key_entry {
	void *key1;
	void *key2;
	uint32_t key_size;
};

struct pnso_sim_key_entry_block {
	struct pnso_sim_key_entry_block *next;
	struct pnso_sim_key_entry keys[KEY_ENTRY_BLOCK_COUNT];
};

static struct {
	struct slab_desc *slab;
	struct pnso_sim_key_entry_block *first_block;
} g_pnso_sim_key_list = {
NULL, NULL};

void pnso_sim_key_entry_block_init(struct pnso_sim_key_entry_block *block)
{
	block->next = NULL;
	memset(block->keys, 0, sizeof(block->keys));
}

struct pnso_sim_key_entry *pnso_sim_key_get_entry(uint32_t key_idx,
						  bool alloc)
{
	struct pnso_sim_key_entry_block *block, *prev_block = NULL;
	uint32_t cur_idx = 0;

	/* Find associated block */
	block = g_pnso_sim_key_list.first_block;
	while (key_idx >= cur_idx) {
		if (!block) {
			if (!alloc)
				return NULL;
			/* Allocate new block */
			block =
			    pnso_sim_slab_alloc(g_pnso_sim_key_list.slab,
						sizeof(struct
						       pnso_sim_key_entry_block));
			if (!block)
				return NULL;
			pnso_sim_key_entry_block_init(block);
			/* Insert into list */
			if (!g_pnso_sim_key_list.first_block) {
				g_pnso_sim_key_list.first_block = block;
			}
			if (prev_block) {
				prev_block->next = block;
			}
		}
		prev_block = block;
		block = block->next;
		cur_idx += KEY_ENTRY_BLOCK_COUNT;
	}

	if (prev_block) {
		cur_idx = key_idx - (cur_idx - KEY_ENTRY_BLOCK_COUNT);
		return &prev_block->keys[cur_idx];
	}
	return NULL;
}

pnso_error_t pnso_sim_key_store_init(uint8_t * slab, uint32_t slab_size)
{
	struct pnso_sim_key_entry_block *block;

	g_pnso_sim_key_list.slab = pnso_sim_slab_init(slab, slab_size);

	block =
	    pnso_sim_slab_alloc(g_pnso_sim_key_list.slab,
				sizeof(struct pnso_sim_key_entry_block));
	g_pnso_sim_key_list.first_block = block;
	if (block) {
		pnso_sim_key_entry_block_init(block);
	}

	return 0;
}

pnso_error_t pnso_set_key_desc_idx(const void *key1,
				   const void *key2,
				   uint32_t key_size, uint32_t key_idx)
{
	struct pnso_sim_key_entry *entry;
	entry = pnso_sim_key_get_entry(key_idx, true);
	if (!entry) {
		return -1;
	}

	if (entry->key_size < key_size) {
		/* Note: in case of larger key, old memory is lost */
		entry->key1 =
		    pnso_sim_slab_alloc(g_pnso_sim_key_list.slab,
					key_size * 2);
		if (!entry->key1) {
			return -1;
		}
		entry->key2 = entry->key1 + key_size;
	}
	entry->key_size = key_size;
	memcpy(entry->key1, key1, key_size);
	memcpy(entry->key2, key2, key_size);

	return 0;
}

pnso_error_t pnso_sim_get_key_desc_idx(void **key1,
				       void **key2,
				       uint32_t * key_size,
				       uint32_t key_idx)
{
	struct pnso_sim_key_entry *entry;
	entry = pnso_sim_key_get_entry(key_idx, false);
	if (!entry) {
		return -1;
	}

	*key1 = entry->key1;
	*key2 = entry->key2;
	*key_size = entry->key_size;
	return 0;
}
