/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SEQ_OPS_H__
#define __PNSO_SEQ_OPS_H__

#include "osal.h"
#include "pnso_chain.h"
#include "pnso_chain_params.h"
#include "pnso_cpdc.h"
#include "pnso_crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sequencer_ops {
	void * (*setup_desc)(struct service_info *svc_info,
			const void *src_desc, size_t desc_size);

	void (*ring_db)(const struct service_info *svc_info);

	pnso_error_t (*setup_cp_chain_params)(struct chain_entry *centry,
			struct service_info *svc_info,
			struct cpdc_desc *cp_desc,
			struct cpdc_status_desc *status_desc);

	pnso_error_t (*setup_hash_chain_params)(
			struct cpdc_chain_params *chain_params,
			struct service_info *svc_info,
			struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl,
			uint32_t num_hash_blks);

	void * (*setup_cpdc_chain_desc)(struct chain_entry *centry,
		struct service_info *svc_info,
		const void *src_desc, size_t desc_size);

	void (*cleanup_cpdc_chain)(const struct service_info *svc_info);

	pnso_error_t (*setup_crypto_chain)(struct service_info *svc_info,
			struct crypto_desc *desc);

	void (*cleanup_crypto_chain)(const struct service_info *svc_info);

};

extern const struct sequencer_ops model_seq_ops;
extern const struct sequencer_ops hw_seq_ops;

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_SEQ_OPS_H__ */
