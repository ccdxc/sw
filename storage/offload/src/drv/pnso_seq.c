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
seq_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size)
{
	return g_sequencer_ops->setup_desc(svc_info, src_desc, desc_size);
}

void
seq_ring_db(const struct service_info *svc_info)
{
	g_sequencer_ops->ring_db(svc_info);
}

pnso_error_t
seq_setup_cp_chain_params(struct chain_entry *centry,
			struct service_info *svc_info,
			struct cpdc_desc *cp_desc,
			struct cpdc_status_desc *status_desc)
{
	return g_sequencer_ops->setup_cp_chain_params(centry, svc_info,
			cp_desc, status_desc);
}

pnso_error_t
seq_setup_hash_chain_params(struct cpdc_chain_params *chain_params,
			struct service_info *svc_info,
			struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl,
			uint32_t num_hash_blks)
{
	return g_sequencer_ops->setup_hash_chain_params(chain_params, svc_info,
			hash_desc, sgl, num_hash_blks);
}

uint8_t *
seq_setup_cpdc_chain_desc(struct chain_entry *centry,
		struct service_info *svc_info,
		const void *src_desc, size_t desc_size)
{
	return g_sequencer_ops->setup_cpdc_chain_desc(centry, svc_info,
			src_desc, desc_size);
}

void
seq_cleanup_cpdc_chain(const struct service_info *svc_info)
{
	g_sequencer_ops->cleanup_cpdc_chain(svc_info);
}

void *
seq_setup_crypto_chain(struct service_info *svc_info,
			struct crypto_desc *desc)
{
	return g_sequencer_ops->setup_crypto_chain(svc_info, desc);
}

void
seq_cleanup_crypto_chain(const struct service_info *svc_info)
{
	g_sequencer_ops->cleanup_crypto_chain(svc_info);
}
