/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_seq_ops.h"

static void *
ut_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size)
{
	return NULL;	/* EOPNOTSUPP */
}

static void
ut_ring_db(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

const struct sequencer_ops ut_seq_ops = {
	.setup_desc = ut_setup_desc,
	.ring_db = ut_ring_db,
};
