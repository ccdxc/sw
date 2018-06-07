/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"

#include "pnso_global.h"
#include "pnso_emul_ops.h"

#include "../../dol/test/storage/dol_wrap.h"

static bool
__model_read_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

static bool
__model_write_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

static bool
__model_read_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

static bool
__model_write_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

static void *
__model_setup_seq_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
		void *src_desc, size_t desc_size)
{
	return dlw_setup_seq_desc(pnso_qid, pnso_index, src_desc, desc_size);
}

static void
__model_setup_comp_hash_chain(void *comp_desc, size_t comp_desc_size,
		void *hash_desc, size_t hash_desc_size)
{
	dlw_setup_comp_hash_chain(comp_desc, comp_desc_size,
			hash_desc, hash_desc_size);
}

static void
__model_ring_seq_db(uint32_t pnso_qid, uint16_t pnso_index)
{
	dlw_ring_seq_db(pnso_qid, pnso_index);
}

const struct emulator_ops model_emul_ops = {
	.read_reg = __model_read_reg,
	.write_reg = __model_write_reg,
	.read_mem = __model_read_mem,
	.write_mem = __model_write_mem,
	.setup_seq_desc = __model_setup_seq_desc,
	.setup_comp_hash_chain = __model_setup_comp_hash_chain,
	.ring_seq_db = __model_ring_seq_db,
};
