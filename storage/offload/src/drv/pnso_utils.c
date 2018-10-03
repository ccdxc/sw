/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_api_int.h"

#include "pnso_utils.h"

pnso_error_t
ring_spec_info_fill(uint32_t ring_id,
		    struct ring_spec *spec,
		    void *desc,
		    uint32_t num_descs)
{
	struct accel_ring *ring;

	ring = sonic_get_accel_ring(ring_id);
	if (ring) {
		spec->rs_ring_addr = ring->ring_base_pa;
		spec->rs_pndx_addr = ring->ring_pndx_pa;
		spec->rs_pndx_shadow_addr = ring->ring_shadow_pndx_pa;
		spec->rs_desc_size = (uint8_t) ilog2(ring->ring_desc_size);
		spec->rs_pndx_size = (uint8_t) ilog2(ring->ring_pndx_size);
		spec->rs_ring_size = (uint8_t) ilog2(ring->ring_size);
		spec->rs_desc_addr = osal_virt_to_phy(desc);
		spec->rs_num_descs = num_descs;
        	return PNSO_OK;
	}

	return EINVAL;
}

/*
 * Format a packed chain SGL for PDMA purposes. Note that TxDMA mem2mem has
 * a transfer limit so each SGL addr/len must be within this limit.
 */
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pc_res,
			   const struct pnso_buffer_list *buf_list)
{
	struct chain_sgl_pdma	*sgl_pdma;
	struct buffer_list_iter	buffer_list_iter;
	struct buffer_list_iter	*iter;
	uint32_t		total_len = 0;
	uint32_t		i;

	sgl_pdma = pc_res_mpool_object_get(pc_res, MPOOL_TYPE_CHAIN_SGL_PDMA);
	if (sgl_pdma) {
		memset(sgl_pdma, 0, sizeof(*sgl_pdma));
		iter = buffer_list_iter_init(&buffer_list_iter, buf_list);
		for (i = 0; iter && (i < ARRAY_SIZE(sgl_pdma->tuple)); i++) {
			iter = buffer_list_iter_addr_len_get(iter, 
				SGL_PDMA_TUPLE_MAX_LEN, &sgl_pdma->tuple[i].addr,
				&sgl_pdma->tuple[i].len);
			total_len += sgl_pdma->tuple[i].len;
		}

		if (iter) {
			OSAL_LOG_ERROR("buffer_list exceeds all SGL PDMA tuples, "
				       "current_len %u", total_len);
			goto out;
		}
	}

	return sgl_pdma;
out:
	pc_res_mpool_object_put(pc_res, MPOOL_TYPE_CHAIN_SGL_PDMA, sgl_pdma);
	return NULL;
}

void
pc_res_sgl_pdma_put(const struct per_core_resource *pc_res,
		    struct chain_sgl_pdma *sgl_pdma)
{
	pc_res_mpool_object_put(pc_res, MPOOL_TYPE_CHAIN_SGL_PDMA, sgl_pdma);
}

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

struct interm_buf_list *
pc_res_interm_buf_list_get(const struct per_core_resource *pc_res,
			   enum mem_pool_type blist_type,
			   enum mem_pool_type buf_type)
{
        struct interm_buf_list	*iblist;
        uint32_t		buf_size;

	iblist = pc_res_mpool_object_get(pc_res, blist_type);
	if (iblist) {
		memset(iblist, 0, sizeof(*iblist));
		iblist->blist_type = blist_type;
		iblist->buf_type = buf_type;
		iblist->buf_obj = pc_res_mpool_object_get_with_size(pc_res, buf_type,
                                                                    &buf_size);
		if (iblist->buf_obj) {

			/* Note that rmem_obj address is already physical */
			iblist->blist.count = 1;
			iblist->blist.buffers[0].buf = mpool_type_is_rmem(buf_type) ?
						       (uint64_t)iblist->buf_obj :
						        osal_virt_to_phy(iblist->buf_obj);
			iblist->blist.buffers[0].len = buf_size;
			return iblist;
		}
	}

	pc_res_interm_buf_list_put(pc_res, iblist);
	return NULL;
}

void
pc_res_interm_buf_list_put(const struct per_core_resource *pc_res,
                           struct interm_buf_list *iblist)
{
	if (iblist) {
		if (iblist->buf_obj)
			pc_res_mpool_object_put(pc_res, iblist->buf_type, iblist->buf_obj);
		pc_res_mpool_object_put(pc_res, iblist->blist_type, iblist);
        }
}

struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pc_res,
		 enum mem_pool_type type)
{
	if (pc_res && mpool_type_is_valid(type)) {
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

void *
pc_res_mpool_object_get_with_size(const struct per_core_resource *pc_res,
				  enum mem_pool_type type,
                                  uint32_t *ret_size)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	*ret_size = 0;
	mpool = pc_res_mpool_get(pc_res, type);
	if (mpool) {
		*ret_size = mpool_get_object_size(mpool);
		obj = mpool_get_object(mpool);
		if (!obj) {
			OSAL_LOG_ERROR("cannot obtain pc_res object from pool %s",
					mem_pool_get_type_str(type));
		}
	}
	return obj;
}

void *
pc_res_mpool_object_get_with_count(const struct per_core_resource *pc_res,
				  enum mem_pool_type type,
                                  uint32_t *ret_count)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	*ret_count = 0;
	mpool = pc_res_mpool_get(pc_res, type);
	if (mpool) {
		*ret_count = mpool_get_object_count(mpool);
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


