/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_atomic.h"

#include "sim.h"

/******* Buffer list utilities **********************************/

/* Copy a buffer list into a flat buffer.
 * Return count of bytes copied.
 * Do not modify the structure members, just the buffer data.
 */
uint32_t sim_memcpy_list_to_flat_buf(struct pnso_flat_buffer *dst,
				     const struct pnso_buffer_list *src_list)
{
	const struct pnso_flat_buffer *src;
	uint32_t cpy_len, total = 0;
	size_t buf_i;

	for (buf_i = 0; buf_i < src_list->count && total < dst->len;
	     buf_i++) {
		src = &src_list->buffers[buf_i];
		cpy_len = src->len;
		if (total + cpy_len > dst->len) {
			cpy_len = dst->len - total;
		}
		memcpy((uint8_t *) dst->buf + total,
		       (uint8_t *) src->buf, cpy_len);
		total += cpy_len;
	}

	return total;
}

/* Copy a flat buffer into a buffer list.
 * Assumes the buffer list lengths have been filled out with allocated size,
 * or 0 for default block size.
 * Return count of bytes copied.
 * Do not modify the structure members, just the buffer data.
 */
uint32_t sim_memcpy_flat_buf_to_list(struct pnso_buffer_list *dst_list,
				     const struct pnso_flat_buffer *src)
{
	struct pnso_flat_buffer *dst;
	uint32_t cpy_len, total = 0;
	size_t buf_i;

	for (buf_i = 0; buf_i < dst_list->count && total < src->len;
	     buf_i++) {
		dst = &dst_list->buffers[buf_i];
		cpy_len = dst->len;
		if (cpy_len > src->len - total) {
			cpy_len = src->len - total;
		}
		memcpy((uint8_t *) dst->buf, (uint8_t *) src->buf + total,
		       cpy_len);
		total += cpy_len;
	}

	return total;
}

uint32_t sim_flat_buffer_block_count(const struct pnso_flat_buffer *buf,
				     uint32_t block_sz)
{
	return (buf->len + (block_sz - 1)) / block_sz;
}

void sim_flat_buffer_to_block(const struct pnso_flat_buffer *src,
			      struct pnso_flat_buffer *dst,
			      uint32_t block_sz,
			      uint32_t block_idx)
{
	dst->buf = src->buf + (block_sz * block_idx);
	if (src->len >= (block_sz * (block_idx + 1))) {
		/* Full block */
		dst->len = block_sz;
	} else if (src->len >= (block_sz * block_idx)) {
		/* Last and partial block */
		dst->len = src->len % block_sz;
	} else {
		/* Beyond last block */
		dst->len = 0;
	}
}

/* Modifies buffer to add zero padding to end of last block.
 * Returns number of bytes appended.
 */
uint32_t sim_flat_buffer_pad(struct pnso_flat_buffer *buf,
			     uint32_t block_sz)
{
	uint32_t pad_len = 0;
	uint32_t block_count = sim_flat_buffer_block_count(buf, block_sz);
	struct pnso_flat_buffer block;

	sim_flat_buffer_to_block(buf, &block, block_sz, block_count - 1);

	if (block.len && block.len < block_sz) {
		/* Pad block with zeroes in-place */
		uint32_t pad_len = block_sz - block.len;

		memset((uint8_t *) (block.buf + block.len), 0, pad_len);
		buf->len += pad_len;
	}

	return pad_len;
}

/******* Simple memory slab utilities **********************************/

struct slab_desc {
	uint8_t *data;
	uint32_t total;
	osal_atomic_int_t allocated;
};

struct slab_desc *sim_slab_init(uint8_t *data, uint32_t size)
{
	struct slab_desc *slab;

	if (size < sizeof(struct slab_desc)) {
		return NULL;
	}
	slab = (struct slab_desc *) data;
	slab->data = data;
	slab->total = size;
	osal_atomic_init(&slab->allocated, sizeof(struct slab_desc));

	return slab;
}

void *sim_slab_alloc(struct slab_desc *slab, uint32_t size)
{
	void *ret = NULL;
	uint32_t temp;

	temp = osal_atomic_fetch_add(&slab->allocated, size);
	if (slab->total >= (temp + size)) {
		ret = (void *) slab->data + temp;
	} else {
		osal_atomic_fetch_sub(&slab->allocated, size);
	}
	return ret;
}

/******* Encryption key database utilities *****************************/

#define KEY_ENTRY_BLOCK_COUNT 16

struct sim_key_entry {
	void *key1;
	void *key2;
	uint32_t key_size;
};

struct sim_key_entry_block {
	struct sim_key_entry_block *next;
	struct sim_key_entry keys[KEY_ENTRY_BLOCK_COUNT];
};

static struct {
	struct slab_desc *slab;
	struct sim_key_entry_block *first_block;
} g_sim_key_list = {
NULL, NULL};

void sim_key_entry_block_init(struct sim_key_entry_block *block)
{
	block->next = NULL;
	memset(block->keys, 0, sizeof(block->keys));
}

struct sim_key_entry *sim_key_get_entry(uint32_t key_idx,
					bool alloc)
{
	struct sim_key_entry_block *block, *prev_block = NULL;
	uint32_t cur_idx = 0;

	/* Find associated block */
	block = g_sim_key_list.first_block;
	while (key_idx >= cur_idx) {
		if (!block) {
			if (!alloc)
				return NULL;
			/* Allocate new block */
			block = sim_slab_alloc(g_sim_key_list.slab,
					sizeof(struct sim_key_entry_block));
			if (!block)
				return NULL;
			sim_key_entry_block_init(block);
			/* Insert into list */
			if (!g_sim_key_list.first_block) {
				g_sim_key_list.first_block = block;
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

pnso_error_t sim_key_store_init(uint32_t size)
{
	struct sim_key_entry_block *block;
	uint8_t *slab = osal_alloc(size);

	if (!slab) {
		return ENOMEM;
	}

	g_sim_key_list.slab = sim_slab_init(slab, size);

	block = sim_slab_alloc(g_sim_key_list.slab,
			       sizeof(struct sim_key_entry_block));
	g_sim_key_list.first_block = block;
	if (block) {
		sim_key_entry_block_init(block);
	}

	return PNSO_OK;
}

void sim_key_store_finit(void)
{
	osal_free(g_sim_key_list.slab);
	g_sim_key_list.slab = NULL;
}

pnso_error_t pnso_set_key_desc_idx(const void *key1,
				   const void *key2,
				   uint32_t key_size, uint32_t key_idx)
{
	struct sim_key_entry *entry;

	entry = sim_key_get_entry(key_idx, true);
	if (!entry) {
		return PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG;
	}

	if (entry->key_size < key_size) {
		/* Note: in case of larger key, old memory is lost */
		entry->key1 = sim_slab_alloc(g_sim_key_list.slab,
					     key_size * 2);
		if (!entry->key1) {
			return ENOMEM;
		}
		entry->key2 = entry->key1 + key_size;
	}
	entry->key_size = key_size;
	memcpy(entry->key1, key1, key_size);
	memcpy(entry->key2, key2, key_size);

	return PNSO_OK;
}

pnso_error_t sim_get_key_desc_idx(void **key1,
				  void **key2,
				  uint32_t *key_size,
				  uint32_t key_idx)
{
	struct sim_key_entry *entry;

	entry = sim_key_get_entry(key_idx, false);
	if (!entry) {
		return PNSO_ERR_XTS_KEY_NOT_REGISTERED;
	}

	*key1 = entry->key1;
	*key2 = entry->key2;
	*key_size = entry->key_size;
	return PNSO_OK;
}

/******* TLV utilities ***************************************/

void sim_tlv_to_buf(uint8_t *dst, uint32_t len, uint64_t val)
{
	switch (len) {
	case 1:
		*dst = (uint8_t) val;
		break;
	case 2:
		*(uint16_t *)dst = (uint16_t) val;
		break;
	case 4:
		*(uint32_t *)dst = (uint32_t) val;
		break;
	case 8:
		*(uint64_t *)dst = val;
		break;
	default:
		/* TODO */
		break;
	}
}

void sim_buf_to_tlv(const uint8_t *src, uint32_t len, uint64_t *val)
{
	switch (len) {
	case 1:
		*val = *src;
		break;
	case 2:
		*val = *(uint16_t *)src;
		break;
	case 4:
		*val = *(uint32_t *)src;
		break;
	case 8:
		*val = *(uint64_t *)src;
		break;
	default:
		/* TODO */
		*val = 0;
		break;
	}
}

