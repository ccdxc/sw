/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_seq_ops.h"

static void *
sim_setup_desc(uint32_t ring_id, uint16_t *index,
		void *src_desc, size_t desc_size)
{
	return NULL;	/* EOPNOTSUPP */
}

static void
sim_ring_db(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

const struct sequencer_ops sim_seq_ops = {
	.setup_desc = sim_setup_desc,
	.ring_db = sim_ring_db,
};
