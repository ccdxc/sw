/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_CRYPTO_CMN_H__
#define __PNSO_CRYPTO_CMN_H__

#include "pnso_chain.h"

#ifdef NDEBUG
#define CRYPTO_PPRINT_DESC(d)
#else
#define CRYPTO_PPRINT_DESC(d)						       \
	OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
	crypto_pprint_desc(d);						       \
	OSAL_LOG_DEBUG("%.*s", 30, "=========================================");
#endif

/* status reported by hardware */
#define CRYPTO_STATUS_SUCCESS		0
#define CRYPTO_LEN_NOT_MULTI_SECTORS	1

/* CRYPTO common/utility functions */
void crypto_pprint_aol(uint64_t aol_pa);
void crypto_pprint_desc(const struct crypto_desc *desc);

pnso_error_t
crypto_aol_packed_get(const struct per_core_resource *pcr,
		      const struct service_buf_list *svc_blist,
		      uint32_t total_len_max,
		      struct service_crypto_aol *svc_aol);
pnso_error_t
crypto_aol_vec_sparse_get(const struct per_core_resource *pcr,
			  uint32_t block_size,
			  const struct service_buf_list *svc_blist,
			  uint32_t total_len_max,
			  struct service_crypto_aol *svc_aol);
void crypto_aol_put(const struct per_core_resource *pcr,
		    struct service_crypto_aol *svc_aol);
uint32_t crypto_aol_total_len_get(const struct service_crypto_aol *svc_aol);
pnso_error_t crypto_desc_status_convert(uint64_t status);

pnso_error_t crypto_setup_batch_desc(struct service_info *svc_info,
		struct crypto_desc *desc);

struct crypto_desc *crypto_get_batch_bulk_desc(struct mem_pool *mpool);

void crypto_put_batch_bulk_desc(struct mem_pool *mpool,
		struct crypto_desc *desc);

struct crypto_desc *crypto_get_desc(struct service_info *svc_info,
		bool per_block);

void crypto_put_desc(const struct service_info *svc_info,
		bool per_block, struct crypto_desc *desc);

pnso_error_t crypto_setup_seq_desc(struct service_info *svc_info,
		struct crypto_desc *desc);

pnso_error_t crypto_status_desc_setup(struct service_info *svc_info);
void crypto_status_desc_teardown(struct service_info *svc_info);
pnso_error_t crypto_rmem_status_desc_setup(struct service_info *svc_info);
void crypto_rmem_status_desc_teardown(struct service_info *svc_info);

#endif /* __PNSO_CRYPTO_CMN_H__ */
