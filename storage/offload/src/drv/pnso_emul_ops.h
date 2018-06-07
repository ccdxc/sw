/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_EMUL_OPS_H__
#define __PNSO_EMUL_OPS_H__

#include "pnso_global.h"

/* TODO:
 * 	assert within the ops need to be removed with real stuff
 *
 */
struct emulator_ops {
	bool (*read_reg)(uint64_t addr, uint32_t *data);

	bool (*write_reg)(uint64_t addr, uint32_t *data);

	bool (*read_mem)(uint64_t addr, uint8_t *data, size_t size);

	bool (*write_mem)(uint64_t addr, uint8_t *data, size_t size);

	void * (*setup_seq_desc)(uint32_t *pnso_qid, uint16_t *pnso_index,
			void *src_desc, size_t desc_size);

	void (*setup_comp_hash_chain)(void *comp_desc, size_t comp_desc_size,
		void *hash_desc, size_t hash_desc_size);

	void (*ring_seq_db)(uint32_t pnso_qid, uint16_t pnso_index);
};

extern const struct emulator_ops model_emul_ops;
extern const struct emulator_ops hw_emul_ops;

#endif /* __PNSO_EMUL_OPS_H__ */
