/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>

#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"

#include "pnso_utils.h"
#include "pnso_chain.h"

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

pnso_error_t
pc_res_sgl_packed_get(const struct per_core_resource *pc_res,
		      const struct service_buf_list *svc_blist,
		      uint32_t block_size,
		      enum mem_pool_type mpool_type,
		      struct service_cpdc_sgl *svc_sgl)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl;
	uint32_t total_len;
	pnso_error_t err;

	if (!svc_blist->blist || svc_blist->blist->count == 0)
		return EINVAL;

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
	svc_sgl->mpool_type = mpool_type;
	svc_sgl->sgl = NULL;
	total_len = 0;
	while (iter) {
		sgl = pc_res_mpool_object_get(pc_res, mpool_type);
		if (!sgl) {
			OSAL_LOG_ERROR("cannot obtain sgl_vec from pool, current_len %u",
				       total_len);
			err = ENOMEM;
			goto out;
		}
		memset(sgl, 0, sizeof(*sgl));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl->cs_addr_0, &sgl->cs_len_0);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl->cs_addr_1, &sgl->cs_len_1);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl->cs_addr_2, &sgl->cs_len_2);
		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl->cs_addr_0) {
			pc_res_mpool_object_put(pc_res, svc_sgl->mpool_type, sgl);
			break;
		}
		total_len += sgl->cs_len_0 + sgl->cs_len_1 + sgl->cs_len_2;

		if (!svc_sgl->sgl)
			svc_sgl->sgl = sgl;
		else
			sgl_prev->cs_next = sonic_virt_to_phy(sgl);
		sgl_prev = sgl;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		OSAL_LOG_ERROR("buffer_list is empty");
		err = EINVAL;
		goto out;
	}
	return PNSO_OK;
out:
	pc_res_sgl_put(pc_res, svc_sgl);
	return err;
}

void
pc_res_sgl_put(const struct per_core_resource *pc_res,
	       struct service_cpdc_sgl *svc_sgl)
{
	struct cpdc_sgl *sgl_next;
	struct cpdc_sgl *sgl;

	sgl = svc_sgl->sgl;
	while (sgl) {
		sgl_next = sgl->cs_next ? sonic_phy_to_virt(sgl->cs_next) :
					  NULL;
		pc_res_mpool_object_put(pc_res, svc_sgl->mpool_type, (void *)sgl);
		sgl = sgl_next;
	}
	svc_sgl->sgl = NULL;
}

pnso_error_t
pc_res_sgl_vec_packed_get(const struct per_core_resource *pc_res,
			  const struct service_buf_list *svc_blist,
			  uint32_t block_size,
			  enum mem_pool_type vec_type,
			  struct service_cpdc_sgl *svc_sgl)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl_vec;
	uint32_t total_len;
	uint32_t num_vec_elems;
	uint32_t cur_count;
	pnso_error_t err;

	if (!svc_blist->blist || svc_blist->blist->count == 0)
		return EINVAL;

	total_len = 0;
	svc_sgl->mpool_type = vec_type;
	svc_sgl->sgl = pc_res_mpool_object_get_with_num_vec_elems(pc_res, vec_type,
								  &num_vec_elems);
	if (!svc_sgl->sgl) {
		OSAL_LOG_ERROR("cannot obtain sgl_vec from pool %s",
			       mpool_get_type_str(vec_type));
		err = ENOMEM;
		goto out;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
	sgl_vec = svc_sgl->sgl;
	cur_count = 0;
	while (iter && (cur_count < num_vec_elems)) {
		memset(sgl_vec, 0, sizeof(*sgl_vec));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_0, &sgl_vec->cs_len_0);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_1, &sgl_vec->cs_len_1);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&sgl_vec->cs_addr_2, &sgl_vec->cs_len_2);
		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl_vec->cs_addr_0)
			break;

		total_len += sgl_vec->cs_len_0 + sgl_vec->cs_len_1 +
			     sgl_vec->cs_len_2;
		if (sgl_prev)
			sgl_prev->cs_next = sonic_virt_to_phy(sgl_vec);

		sgl_prev = sgl_vec++;
		cur_count++;
	}

	if (iter) {
		OSAL_LOG_ERROR("buffer_list total length exceeds SGL vector, current_len %u",
			       total_len);
		err = EINVAL;
		goto out;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		OSAL_LOG_ERROR("buffer_list is empty");
		err = EINVAL;
		goto out;
	}
	return PNSO_OK;
out:
	pc_res_sgl_vec_put(pc_res, svc_sgl);
	return err;
}

void
pc_res_sgl_vec_put(const struct per_core_resource *pc_res,
		   struct service_cpdc_sgl *svc_sgl)
{
	pc_res_mpool_object_put(pc_res, svc_sgl->mpool_type,
				svc_sgl->sgl);
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
			OSAL_LOG_ERROR("buffer_list exceeds all SGL PDMA tuples, current_len %u",
				       total_len);
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
	const struct pnso_buffer_list *buf_list = svc_blist->blist;

	memset(iter, 0, sizeof(*iter));
	iter->blist_type = svc_blist->type;
	if (buf_list->count) {
		iter->cur_count = buf_list->count;
		iter->cur_list = &buf_list->buffers[0];
		iter->cur_len = iter->cur_list->len;
		iter->cur_addr = iter->blist_type == SERVICE_BUF_LIST_TYPE_HOST ?
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
		iter->cur_addr = iter->blist_type == SERVICE_BUF_LIST_TYPE_HOST ?
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

pnso_error_t
svc_interm_buf_list_get(struct service_info *svc_info)
{
	struct interm_buf_list	*iblist;
	struct pnso_flat_buffer	*iblist_buf;
	void			*ibuf;
	uint32_t		buf_size;
	uint32_t		req_size;
	uint32_t		size_left;

	/*
	 * Produce output to intermediate buffers if there is a chain subordinate.
	 * Noe that when such buffers are involved, a PDMA could be needed to
	 * transfer the output data to the application's destination buffers.
	 * PDMA has the following requirements:
	 * 1) The source data must come from one single contiguous buffer, and
	 * 2) The entire transfer must fit in one single chain_sgl_pdma descriptor.
	 *
	 * The current implementation optimizes for the sweet spot of 8K size,
	 * i.e., one single source of up to 8K, which has these desired properties:
	 * a) The smaller size provides for a larger pool of intermediate buffers
	 *    from which large batch operations may draw, and
	 * b) A given transfer will not exceed a chain_sgl_pdma descriptor.
	 *
	 * Before we proceed, 2 observations can be made: first, if the
	 * application does not supply any destination buffers, then no PDMA
	 * will be needed. Second, if application destination buffers are
	 * present and they are longer than 8K, intermediate buffers will not
	 * be used and HW will be set up to output directly to the app's buffers.
	 */
	OSAL_ASSERT(chn_service_has_sub_chain(svc_info));
	iblist = &svc_info->si_iblist;
	iblist->blist.count = 0;
	iblist_buf = &iblist->blist.buffers[0];

	req_size = svc_info->si_dst_blist.len ?
		   svc_info->si_dst_blist.len : svc_info->si_src_blist.len;
	if (!svc_info->si_dst_blist.blist ||
	    (req_size <= INTERM_BUF_NOMINAL_BUF_SIZE)) {

		iblist->buf_type = MPOOL_TYPE_RMEM_INTERM_BUF;
		size_left = req_size;
		while (size_left  &&
		       (iblist->blist.count < INTERM_BUF_MAX_NUM_NOMINAL_BUFS)) {

			ibuf = pc_res_mpool_object_get_with_size(
				svc_info->si_pc_res, iblist->buf_type, &buf_size);
			if (!ibuf)
				goto out;

			iblist->blist.count++;
			iblist_buf->buf = mpool_get_object_phy_addr(
						iblist->buf_type, ibuf);
			iblist_buf->len = size_left > buf_size ?
					  buf_size : size_left;
			size_left -= iblist_buf->len;
			iblist_buf++;
		}

		/*
		 * Switch si_dst_blist to using intermediate buffers
		 */
		svc_info->si_dst_blist.type = SERVICE_BUF_LIST_TYPE_RMEM;
		svc_info->si_dst_blist.len = req_size;
		svc_info->si_dst_blist.blist = &iblist->blist;
	}

	return PNSO_OK;
out:
	svc_interm_buf_list_put(svc_info);
	return ENOMEM;
}

void
svc_interm_buf_list_put(struct service_info *svc_info)
{
	struct interm_buf_list	*iblist = &svc_info->si_iblist;
	struct pnso_flat_buffer	*iblist_buf = &iblist->blist.buffers[0];
	void			*ibuf;

	while (iblist->blist.count) {
		OSAL_ASSERT(iblist_buf->buf);
		ibuf = mpool_get_object_alloc_addr(iblist->buf_type, iblist_buf->buf);
		pc_res_mpool_object_put(svc_info->si_pc_res, iblist->buf_type, ibuf);
		iblist_buf++;
		iblist->blist.count--;
	}
}

struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pc_res,
		 enum mem_pool_type type)
{
	if (pc_res && mpool_type_is_valid(type)) {
		return pc_res->mpools[type];
	}

	OSAL_LOG_ERROR("invalid pc_res 0x"PRIx64" or mpool type %s",
		       (uint64_t)pc_res, mpool_get_type_str(type));
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
					mpool_get_type_str(type));
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
					mpool_get_type_str(type));
		}
	}
	return obj;
}

void *
pc_res_mpool_object_get_with_num_vec_elems(const struct per_core_resource *pc_res,
					   enum mem_pool_type type,
					   uint32_t *ret_num_vec_elems)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	*ret_num_vec_elems = 0;
	mpool = pc_res_mpool_get(pc_res, type);
	if (mpool) {
		*ret_num_vec_elems = mpool_get_object_num_vec_elems(mpool);
		obj = mpool_get_object(mpool);
		if (!obj) {
			OSAL_LOG_ERROR("cannot obtain pc_res object from pool %s",
					mpool_get_type_str(type));
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

	if (obj) {
		mpool = pc_res_mpool_get(pc_res, type);
		if (mpool && mpool_put_object(mpool, obj)) {
			OSAL_LOG_ERROR("cannot return pc_res object to pool %s",
					mpool_get_type_str(type));
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
		OSAL_LOG_DEBUG("%30s: 0x"PRIx64" ==> 0x"PRIx64, "", (uint64_t)sgl, sgl_pa);
		tuple = sgl->tuple;
		OSAL_LOG_DEBUG("%30s: 0x"PRIx64"/%d 0x"PRIx64"/%d 0x"PRIx64"/%d 0x"
				PRIx64"/%d", "",
				tuple[0].addr, tuple[0].len,
				tuple[1].addr, tuple[1].len,
				tuple[2].addr, tuple[2].len,
				tuple[3].addr, tuple[3].len);
	}
}

bool
putil_is_service_in_batch(uint8_t flags)
{
	return ((flags & CHAIN_SFLAG_IN_BATCH) &&
			((flags & CHAIN_SFLAG_LONE_SERVICE) ||
			 (flags & CHAIN_SFLAG_FIRST_SERVICE))) ? true : false;
}

/* TODO-stats: get rid of local 'get_per_core_resource()' */
struct per_core_resource *
putil_get_per_core_resource(void)
{
	struct lif *lif;
	struct per_core_resource *pcr;

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(0);
		goto out;
	}

	pcr = sonic_get_per_core_res(lif);
	if (!pcr) {
		OSAL_ASSERT(0);
		goto out;
	}

	return pcr;
out:
	return NULL;
}
