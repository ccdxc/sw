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
	uint32_t cur_count;
	uint32_t cur_len;
	uint64_t cur_addr;
};

pnso_error_t ring_spec_info_fill(uint32_t ring_id,
		    		 struct ring_spec *spec,
		    		 void *desc,
		    		 uint32_t num_descs);
struct cpdc_sgl *
pc_res_sgl_vec_packed_get(const struct per_core_resource *pc_res,
			  uint32_t block_size,
			  const struct service_buf_list *svc_blist,
			  enum mem_pool_type vec_type,
			  uint32_t *ret_total_len);
void pc_res_sgl_vec_put(const struct per_core_resource *pc_res,
			enum mem_pool_type vec_type,
			struct cpdc_sgl *sgl_vec);
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pc_res,
			   const struct service_buf_list *svc_blist);
void pc_res_sgl_pdma_put(const struct per_core_resource *pc_res,
			 struct chain_sgl_pdma *sgl_pdma);
struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
                      const struct service_buf_list *svc_blist);

struct buffer_list_iter *
buffer_list_iter_addr_len_get(struct buffer_list_iter *iter,
			      uint32_t max_len,
			      uint64_t *ret_addr,
			      uint32_t *ret_len);
struct interm_buf_list *
pc_res_interm_buf_list_get(const struct per_core_resource *pc_res,
			   enum mem_pool_type blist_type,
			   enum mem_pool_type buf_type);
void pc_res_interm_buf_list_put(const struct per_core_resource *pc_res,
                                struct interm_buf_list *iblist);
struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pc_res,
		 enum mem_pool_type type);
void *pc_res_mpool_object_get(const struct per_core_resource *pc_res,
			      enum mem_pool_type type);
void *pc_res_mpool_object_get_with_size(const struct per_core_resource *pc_res,
					enum mem_pool_type type,
					uint32_t *ret_size);
void *pc_res_mpool_object_get_with_count(const struct per_core_resource *pc_res,
					enum mem_pool_type type,
					uint32_t *ret_count);
void pc_res_mpool_object_put(const struct per_core_resource *pc_res,
			     enum mem_pool_type type,
			     void *obj);
void pprint_chain_sgl_pdma(uint64_t sgl_pa);

#endif  /* __PNSO_UTILS_H__ */
