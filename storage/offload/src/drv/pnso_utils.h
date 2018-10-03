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
#include "pnso_chain_params.h"
#include "sonic_dev.h"

struct buffer_list_iter {
	const struct pnso_flat_buffer *cur_list;
	uint32_t cur_count;
	uint32_t cur_len;
	uint64_t cur_addr;
};

pnso_error_t ring_spec_info_fill(uint32_t ring_id,
		    		 struct ring_spec *spec,
		    		 void *desc,
		    		 uint32_t num_descs);
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pc_res,
			   const struct pnso_buffer_list *buf_list);
void pc_res_sgl_pdma_put(const struct per_core_resource *pc_res,
			 struct chain_sgl_pdma *sgl_pdma);
struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
                      const struct pnso_buffer_list *buf_list);

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
#endif  /* __PNSO_UTILS_H__ */
