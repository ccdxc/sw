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
		spec->rs_desc_addr = sonic_virt_to_phy(desc);
		spec->rs_num_descs = num_descs;
        	return PNSO_OK;
	}

	return EINVAL;
}

struct cpdc_sgl *
pc_res_sgl_vec_packed_get(const struct per_core_resource *pc_res,
			  uint32_t block_size,
			  const struct service_buf_list *svc_blist,
			  enum mem_pool_type vec_type,
			  uint32_t *ret_total_len)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_vec_head;
	struct cpdc_sgl *sgl_vec;
	uint32_t vec_count;
	uint32_t cur_count;

	*ret_total_len = 0;
	sgl_vec_head = pc_res_mpool_object_get_with_count(pc_res, vec_type,
							  &vec_count);
	if (!sgl_vec_head) {
		OSAL_LOG_ERROR("cannot obtain sgl_vec from pool %s",
                               mem_pool_get_type_str(vec_type));
		goto out;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
	sgl_vec = sgl_vec_head;
	cur_count = 0;
	while (iter && (cur_count < vec_count)) {
		memset(sgl_vec, 0, sizeof(*sgl_vec));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_0, &sgl_vec->cs_len_0);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_1, &sgl_vec->cs_len_1);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_2, &sgl_vec->cs_len_2);
		*ret_total_len += sgl_vec->cs_len_0 + sgl_vec->cs_len_1 +
				  sgl_vec->cs_len_2;

		sgl_vec++;
		cur_count++;
	}

	if (iter) {
		OSAL_LOG_ERROR("buffer_list total length exceeds SGL vector, "
			       "current_len %u", *ret_total_len);
		goto out;
        }

	return sgl_vec_head;
out:
	pc_res_mpool_object_put(pc_res, vec_type, sgl_vec_head);
	return NULL;
}

void
pc_res_sgl_vec_put(const struct per_core_resource *pc_res,
		   enum mem_pool_type vec_type,
		   struct cpdc_sgl *sgl_vec)
{
	pc_res_mpool_object_put(pc_res, vec_type, sgl_vec);
}

/*
 * Format a packed chain SGL for PDMA purposes. Note that TxDMA mem2mem has
 * a transfer limit so each SGL addr/len must be within this limit.
 */
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pc_res,
			   const struct service_buf_list *svc_blist)
{
	struct chain_sgl_pdma	*sgl_pdma;
	struct buffer_list_iter	buffer_list_iter;
	struct buffer_list_iter	*iter;
	uint32_t		total_len = 0;
	uint32_t		i;

	sgl_pdma = pc_res_mpool_object_get(pc_res, MPOOL_TYPE_CHAIN_SGL_PDMA);
	if (sgl_pdma) {
		memset(sgl_pdma, 0, sizeof(*sgl_pdma));
		iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
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
                      const struct service_buf_list *svc_blist)
{
	const struct pnso_buffer_list *buf_list = svc_blist->sbl_blist;

	memset(iter, 0, sizeof(*iter));
	iter->blist_type = svc_blist->sbl_type;
	if (buf_list->count) {
		iter->cur_count = buf_list->count;
		iter->cur_list = &buf_list->buffers[0];
		iter->cur_len = iter->cur_list->len;
		iter->cur_addr = iter->blist_type == SERVICE_BUF_LIST_TYPE_DFLT ?
				 sonic_hostpa_to_devpa(iter->cur_list->buf) :
			         iter->cur_list->buf;
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
		iter->cur_addr = iter->blist_type == SERVICE_BUF_LIST_TYPE_DFLT ?
				 sonic_hostpa_to_devpa(iter->cur_list->buf) :
			         iter->cur_list->buf;
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
		*ret_addr = iter->cur_addr;
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
		iblist->ibuf = pc_res_mpool_object_get_with_size(pc_res, buf_type,
                                                                 &buf_size);
		if (iblist->ibuf) {

			/* Note that rmem_obj address is already physical */
			iblist->blist.count = 1;
			iblist->blist.buffers[0].buf = mpool_get_object_phy_addr(buf_type,
									iblist->ibuf);
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
		if (iblist->ibuf)
			pc_res_mpool_object_put(pc_res, iblist->buf_type, iblist->ibuf);
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
	if (mpool && obj) {
		if (mpool_put_object(mpool, obj)) {
			OSAL_LOG_ERROR("cannot return pc_res object to pool %s",
					mem_pool_get_type_str(type));
		}
	}
}

void
pprint_chain_sgl_pdma(uint64_t sgl_pa)
{
	const struct chain_sgl_pdma *sgl;
	const struct chain_sgl_pdma_tuple *tuple;

	if (sgl_pa) {
		sgl = (const struct chain_sgl_pdma *) sonic_phy_to_virt(sgl_pa);
		OSAL_LOG_DEBUG("%30s: 0x%llx ==> 0x%llx", "", (uint64_t)sgl, sgl_pa);
		tuple = sgl->tuple;
		OSAL_LOG_DEBUG("%30s: 0x%llx/%d 0x%llx/%d 0x%llx/%d "
				"0x%llx/%d", "",
				tuple[0].addr, tuple[0].len,
				tuple[1].addr, tuple[1].len,
				tuple[2].addr, tuple[2].len,
				tuple[3].addr, tuple[3].len);
	}
}

