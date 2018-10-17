/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_UTILS_H__
#define __PNSO_UTILS_H__

#include "osal.h"
#include "pnso_mpool.h"
#include "sonic_dev.h"

struct buffer_list_iter {
	const struct pnso_flat_buffer *cur_list;
	uint32_t cur_count;
	uint32_t cur_len;
	uint64_t cur_addr;
};

struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
                      const struct pnso_buffer_list *buf_list);

struct buffer_list_iter *
buffer_list_iter_addr_len_get(struct buffer_list_iter *iter,
			      uint32_t max_len,
			      uint64_t *ret_addr,
			      uint32_t *ret_len);
struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pc_res,
		 enum mem_pool_type type);
void *pc_res_mpool_object_get(const struct per_core_resource *pc_res,
			       enum mem_pool_type type);
void pc_res_mpool_object_put(const struct per_core_resource *pc_res,
			     enum mem_pool_type type,
			     void *obj);

#endif  /* __PNSO_UTILS_H__ */
