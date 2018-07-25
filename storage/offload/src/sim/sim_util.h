/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_SIM_UTIL_H__
#define __PNSO_SIM_UTIL_H__

#include "osal_stdtypes.h"
#include "pnso_api.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t sim_buflist_len(const struct pnso_buffer_list *src_list);

/* Copy a buffer list into a flat buffer.
 * Assumes the flat buffer has been initialized with available length,
 * or 0 for PNSO_MAX_BUFFER_LEN.
 * Return count of bytes copied.
 */
uint32_t sim_memcpy_list_to_flat_buf(struct pnso_flat_buffer *dst,
				     const struct pnso_buffer_list *src_list);

/* Copy a flat buffer into a buffer list.
 * Assumes the buffer list lengths have been filled out with allocated size,
 * or 0 for default block size.
 * Return count of bytes copied.
 */
uint32_t sim_memcpy_flat_buf_to_list(struct pnso_buffer_list *dst_list,
				     const struct pnso_flat_buffer *src);

uint32_t sim_flat_buffer_block_count(const struct pnso_flat_buffer *buf,
				     uint32_t block_sz);

void sim_flat_buffer_to_block(const struct pnso_flat_buffer *src,
			      struct pnso_flat_buffer *dst,
			      uint32_t block_sz,
			      uint32_t block_idx);

/* Modifies buffer to add zero padding to end of last block.
 * Returns number of bytes appended.
 */
uint32_t sim_flat_buffer_pad(struct pnso_flat_buffer *buf,
			     uint32_t block_sz);

/*
 * Copy data between a TLV (type-length-value) and a buffer.
 */
void sim_tlv_to_buf(uint8_t *dst, uint32_t len, uint64_t val);
void sim_buf_to_tlv(const uint8_t *src, uint32_t len, uint64_t *val);

struct slab_desc;
struct slab_desc *sim_slab_init(uint8_t *data,
				uint32_t size);
void *sim_slab_alloc(struct slab_desc *slab, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif				/* __PNSO_SIM_UTIL_H__ */
