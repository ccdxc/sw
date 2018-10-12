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
pnso_error_t crypto_update_service_info_aols(struct service_info *svc_info,
		const struct service_params *svc_params);
struct crypto_aol *
crypto_aol_packed_get(const struct per_core_resource *pc_res,
		      const struct service_buf_list *svc_blist,
		      enum mem_pool_type *ret_mpool_type,
		      uint32_t *ret_total_len);
struct crypto_aol *
crypto_aol_vec_sparse_get(const struct per_core_resource *pc_res,
		          uint32_t block_size,
		          const struct service_buf_list *svc_blist,
		          enum mem_pool_type *ret_mpool_type,
		          uint32_t *ret_total_len);
void crypto_aol_put(const struct per_core_resource *pc_res,
		    enum mem_pool_type mpool_type,
		    struct crypto_aol *aol);
pnso_error_t crypto_desc_status_convert(uint64_t status);

#endif /* __PNSO_CRYPTO_CMN_H__ */
