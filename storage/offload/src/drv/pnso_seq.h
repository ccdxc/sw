/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SEQ_H__
#define __PNSO_SEQ_H__

#ifdef __cplusplus
extern "C" {
#endif

void *seq_setup_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
	void *src_desc, size_t desc_size);

void seq_ring_db(uint32_t pnso_qid, uint16_t pnso_index);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_SEQ_H__ */
