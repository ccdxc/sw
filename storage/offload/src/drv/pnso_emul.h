/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_EMUL_H__
#define __PNSO_EMUL_H__

#include "pnso_types.h"

struct accelerator_ring {
	uint64_t ar_config_pd_idx;
	uint64_t ar_ring_base_addr;
	uint64_t ar_shadow_pd_idx;

	uint32_t ar_ring_size;
	uint32_t ar_desc_size;
	uint32_t ar_pi_size;

	uint32_t ar_curr_pd_idx;
	uint32_t ar_prev_pd_idx;

	uint32_t ar_curr_seq_qid;
	uint16_t ar_prev_seq_pd_idx;
};

enum accelerator_ring_type {
	ACC_RING_TYPE_NONE = 0,
	ACC_RING_TYPE_LONE = 1,
	ACC_RING_TYPE_BATCH = 2,
	ACC_RING_TYPE_MAX
};

struct accelerator_queue {
	uin16_t aq_lif;
	uin8_t aq_qtype;
	uin32_t aq_qid;
};

bool emul_read_reg(uint64_t addr, uint32_t *data);

bool emul_write_reg(uint64_t addr, uint32_t *data);

bool emul_read_mem(uint64_t addr, uint8_t *data, size_t size);

bool emul_write_mem(uint64_t addr, uint8_t *data, size_t size);

void *emul_setup_seq_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
	void *src_desc, size_t desc_size);

void emul_setup_comp_hash_chain(void *comp_desc, size_t comp_desc_size,
		void *hash_desc, size_t hash_desc_size);

void emul_ring_seq_db(uint32_t pnso_qid, uint16_t pnso_index);

void emul_submit_seq_desc(void *desc, struct accelerator_queue *queue,
		struct accelerator_ring_type ring_type,
		struct accelerator_ring *ring,
		uint32_t *index);

#endif /* __PNSO_EMUL_H__ */
