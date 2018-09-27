/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_seq_ops.h"

static void *
model_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size)
{
	return NULL;	/* EOPNOTSUPP */
}

static void
model_ring_db(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

static pnso_error_t
model_setup_cp_chain_params(struct chain_entry *centry,
		struct service_info *svc_info,
		struct cpdc_desc *cp_desc,
		struct cpdc_status_desc *status_desc)
{
	return EOPNOTSUPP;
}

static pnso_error_t
model_setup_hash_chain_params(struct chain_entry *centry,
		struct service_info *svc_info,
		struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl,
		uint32_t num_hash_blks)
{
	return EOPNOTSUPP;
}

static void *
model_setup_cpdc_chain_desc(struct chain_entry *centry,
		struct service_info *svc_info,
		const void *src_desc, size_t desc_size)
{
	return NULL;	/* EOPNOTSUPP */
}

const struct sequencer_ops model_seq_ops = {
	.setup_desc = model_setup_desc,
	.ring_db = model_ring_db,
	.setup_cp_chain_params = model_setup_cp_chain_params,
	.setup_hash_chain_params = model_setup_hash_chain_params,
	.setup_cpdc_chain_desc = model_setup_cpdc_chain_desc,
};
