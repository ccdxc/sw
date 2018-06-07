/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"

#include "pnso_global.h"
#include "pnso_emul_ops.h"

static bool
__hw_read_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

static bool
__hw_write_reg(uint64_t addr, uint32_t *data)
{
	assert(0);
	return false;
}

static bool
__hw_read_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

static bool
__hw_write_mem(uint64_t addr, uint8_t *data, size_t size)
{
	assert(0);
	return false;
}

static void *
__hw_setup_seq_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
		void *src_desc, size_t desc_size)
{
	assert(0);
	return NULL;
}

static void
__hw_setup_comp_hash_chain(void *comp_desc, size_t comp_desc_size,
		void *hash_desc, size_t hash_desc_size)
{
	assert(0);
}

static void
__hw_ring_seq_db(uint32_t pnso_qid, uint16_t pnso_index)
{
	assert(0);
}

const struct emulator_ops hw_ops = {
	.read_reg = __hw_read_reg,
	.write_reg = __hw_write_reg,
	.read_mem = __hw_read_mem,
	.write_mem = __hw_write_mem,
	.setup_seq_desc = __hw_setup_seq_desc,
	.setup_comp_hash_chain = __hw_setup_comp_hash_chain,
	.ring_seq_db = __hw_ring_seq_db,
};
