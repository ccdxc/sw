/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "pnso_global.h"
#include "pnso_emul_ops.h"

/* run on model/dol or on real hardware */
#if PNSO_API_ON_MODEL
const struct emulator_ops *g_emulator_ops = &model_emul_ops;
#else
const struct emulator_ops *g_emulator_ops = &hw_emul_ops; 
#endif

bool
emul_read_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

bool
emul_write_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

bool
emul_read_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

bool
emul_write_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

void *
emul_setup_seq_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
		void *src_desc, size_t desc_size)
{
	return g_emulator_ops->setup_seq_desc(pnso_qid, pnso_index,
			src_desc, desc_size);
}

void
emul_setup_comp_hash_chain(void *comp_desc, size_t comp_desc_size,
		void *hash_desc, size_t hash_desc_size)
{
	g_emulator_ops->setup_comp_hash_chain(comp_desc, comp_desc_size,
			hash_desc, hash_desc_size);
}

void
emul_ring_seq_db(uint32_t pnso_qid, uint16_t pnso_index)
{
	g_emulator_ops->ring_seq_db(pnso_qid, pnso_index);
}
