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
seq_setup_desc(uint32_t ring_id, uint16_t *index,
		void *src_desc, size_t desc_size)
{
	return g_sequencer_ops->setup_desc(ring_id, index,
			src_desc, desc_size);
}

void
seq_ring_db(const struct service_info *svc_info, uint16_t index)
{
	g_sequencer_ops->ring_db(svc_info, index);
}
