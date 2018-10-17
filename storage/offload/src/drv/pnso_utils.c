/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_api_int.h"

#include "pnso_utils.h"

struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
                      const struct pnso_buffer_list *buf_list)
{
	memset(iter, 0, sizeof(*iter));
	if (buf_list->count) {
		iter->cur_count = buf_list->count;
		iter->cur_list = &buf_list->buffers[0];
		iter->cur_len = iter->cur_list->len;
		iter->cur_addr = iter->cur_list->buf;
		return iter;
	}
	return NULL;
}

static struct buffer_list_iter *
buffer_list_iter_next(struct buffer_list_iter *iter)
{
	if (iter->cur_list && --iter->cur_count) {
		iter->cur_list++;
		iter->cur_len = iter->cur_list->len;
		iter->cur_addr = iter->cur_list->buf;
		return iter;
	}
	return NULL;
}

struct buffer_list_iter *
buffer_list_iter_addr_len_get(struct buffer_list_iter *iter,
			      uint32_t max_len,
			      uint64_t *ret_addr,
			      uint32_t *ret_len)
{
	uint32_t len = 0;

	OSAL_ASSERT(max_len);
	*ret_addr = 0;
	*ret_len = 0;
	while (iter) {
		if (iter->cur_len == 0) {
			iter = buffer_list_iter_next(iter);
			continue;
		}

		if (len) {
			break;
		}
		len = iter->cur_len > max_len ? max_len : iter->cur_len;
		*ret_addr = sonic_hostpa_to_devpa(iter->cur_addr);
		*ret_len = len;
		iter->cur_addr += len;
		iter->cur_len -= len;
	}
	return iter;
}

struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pc_res,
		 enum mem_pool_type type)
{
	if (pc_res && ((type != MPOOL_TYPE_NONE) && (type < MPOOL_TYPE_MAX))) {
		return pc_res->mpools[type];
	}

	OSAL_LOG_ERROR("invalid pc_res 0x%llx or mpool type %s", (uint64_t)pc_res,
			mem_pool_get_type_str(type));
	return NULL;
}

void *
pc_res_mpool_object_get(const struct per_core_resource *pc_res,
			enum mem_pool_type type)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	mpool = pc_res_mpool_get(pc_res, type);
	if (mpool) {
		obj = mpool_get_object(mpool);
		if (!obj) {
			OSAL_LOG_ERROR("cannot obtain pc_res object from pool %s",
					mem_pool_get_type_str(type));
		}
	}
	return obj;
}

void
pc_res_mpool_object_put(const struct per_core_resource *pc_res,
			enum mem_pool_type type,
			void *obj)
{
	struct mem_pool *mpool;

	mpool = pc_res_mpool_get(pc_res, type);
	if (mpool) {
		if (mpool_put_object(mpool, obj)) {
			OSAL_LOG_ERROR("cannot return pc_res object to pool %s",
					mem_pool_get_type_str(type));
		}
	}
}


