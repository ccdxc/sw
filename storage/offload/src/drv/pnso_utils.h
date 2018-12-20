/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_UTILS_H__
#define __PNSO_UTILS_H__

#include "osal.h"
#include "pnso_api.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_chain_params.h"
#include "sonic_dev.h"

#ifdef NDEBUG
#define SGL_PDMA_PPRINT(d)
#else
#define SGL_PDMA_PPRINT(d)	pprint_chain_sgl_pdma(d)
#endif

struct buffer_list_iter {
	enum service_buf_list_type blist_type;
	const struct pnso_flat_buffer *cur_list;
	uint32_t total_len_max;
	uint32_t cur_count;
	uint32_t cur_len;
	uint64_t cur_addr;
};

/*
 * Mostly used as intermediate repository prior to transfer to
 * packed structure fields.
 */
struct buffer_addr_len {
	uint64_t addr;
	uint32_t len;
};

#define BUFFER_ADDR_LEN_SET(_addr, _len, _addr_len)	\
	do {						\
		_addr = _addr_len.addr;			\
		_len  = _addr_len.len;			\
	} while (0)

pnso_error_t ring_spec_info_fill(struct sonic_accel_ring *ring,
				 struct ring_spec *spec,
				 void *desc,
				 uint32_t num_descs);
pnso_error_t
pc_res_sgl_packed_get(const struct per_core_resource *pcr,
		      const struct service_buf_list *svc_blist,
		      uint32_t block_size,
		      enum mem_pool_type mpool_type,
		      struct service_cpdc_sgl *svc_sgl);
void pc_res_sgl_put(const struct per_core_resource *pcr,
		    struct service_cpdc_sgl *svc_sgl);
pnso_error_t
pc_res_sgl_vec_packed_get(const struct per_core_resource *pcr,
			  const struct service_buf_list *svc_blist,
			  uint32_t block_size,
			  enum mem_pool_type vec_type,
			  struct service_cpdc_sgl *svc_sgl);
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pcr,
			   const struct service_buf_list *svc_blist);
void pc_res_sgl_pdma_put(const struct per_core_resource *pcr,
			 struct chain_sgl_pdma *sgl_pdma);
struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
		      const struct service_buf_list *svc_blist,
		      uint32_t total_len_max);

struct buffer_list_iter *
buffer_list_iter_addr_len_get(struct buffer_list_iter *iter,
			      uint32_t max_len,
			      struct buffer_addr_len *ret_addr_len);
pnso_error_t svc_interm_buf_list_get(struct service_info *svc_info);
void svc_interm_buf_list_put(struct service_info *svc_info);

uint64_t
svc_poll_expiry_start(const struct service_info *svc_info);

bool
svc_poll_expiry_check(const struct service_info *svc_info,
		      uint64_t start_ts,
		      uint64_t per_svc_timeout);

struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pcr,
		 enum mem_pool_type type);
void *pc_res_mpool_object_get(const struct per_core_resource *pcr,
			      enum mem_pool_type type);
void *pc_res_mpool_object_get_with_size(const struct per_core_resource *pcr,
					enum mem_pool_type type,
					uint32_t *ret_size);
void *pc_res_mpool_object_get_with_num_vec_elems(const struct per_core_resource *pcr,
						 enum mem_pool_type type,
						 uint32_t *ret_num_vec_elems);
void pc_res_mpool_object_put(const struct per_core_resource *pcr,
			     enum mem_pool_type type,
			     void *obj);
void pprint_chain_sgl_pdma(uint64_t sgl_pa);

bool putil_is_service_in_batch(uint8_t flags);

struct per_core_resource *putil_get_per_core_resource(void);

#endif  /* __PNSO_UTILS_H__ */
