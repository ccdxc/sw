/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_seq_ops.h"

static void *
model_setup_desc(uint32_t ring_id, uint16_t *index,
		void *src_desc, size_t desc_size)
{
	return NULL;	/* EOPNOTSUPP */
}

static void
model_ring_db(const struct service_info *svc_info, uint16_t index)
{
	/* EOPNOTSUPP */
}

const struct sequencer_ops model_seq_ops = {
	.setup_desc = model_setup_desc,
	.ring_db = model_ring_db,
};
