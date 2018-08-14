/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_seq_ops.h"

/* run on model/dol or on real hardware */
#ifdef PNSO_API_ON_MODEL
const struct sequencer_ops *g_sequencer_ops = &model_seq_ops;
#else
const struct sequencer_ops *g_sequencer_ops = &hw_seq_ops;
#endif

void *
seq_setup_desc(uint32_t *pnso_qid, uint16_t *pnso_index,
		void *src_desc, size_t desc_size)
{
	return g_sequencer_ops->setup_desc(pnso_qid, pnso_index,
			src_desc, desc_size);
}

void
seq_ring_db(uint32_t pnso_qid, uint16_t pnso_index)
{
	g_sequencer_ops->ring_db(pnso_qid, pnso_index);
}
