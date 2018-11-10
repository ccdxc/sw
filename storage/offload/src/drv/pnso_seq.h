/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SEQ_H__
#define __PNSO_SEQ_H__

#include "pnso_crypto.h"
#include "pnso_chain_params.h"

#ifdef __cplusplus
extern "C" {
#endif

void *seq_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size);

void seq_ring_db(const struct service_info *svc_info);

pnso_error_t seq_setup_cp_chain_params(struct service_info *svc_info,
	struct cpdc_desc *cp_desc, struct cpdc_status_desc *status_desc);

pnso_error_t seq_setup_cp_pad_chain_params(struct service_info *svc_info,
	struct cpdc_desc *cp_desc, struct cpdc_status_desc *status_desc);

pnso_error_t seq_setup_hash_chain_params(struct cpdc_chain_params *chain_params,
		struct service_info *svc_info,
		struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl,
		uint32_t num_hash_blks);

void *seq_setup_cpdc_chain_desc(struct service_info *svc_info,
		const void *src_desc, size_t desc_size);

void seq_cleanup_cpdc_chain(const struct service_info *svc_info);

pnso_error_t seq_setup_crypto_chain(struct service_info *svc_info,
			struct crypto_desc *desc);
void seq_cleanup_crypto_chain(const struct service_info *svc_info);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_SEQ_H__ */
