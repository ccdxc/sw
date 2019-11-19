/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"

#include "pnso_utils.h"
#include "pnso_chain.h"
#include "pnso_seq.h"

extern uint64_t cp_scratch_buffer_1;
extern uint64_t cp_scratch_buffer_2;
extern uint64_t dc_scratch_buffer;

pnso_error_t
ring_spec_info_fill(struct sonic_accel_ring *ring,
		    struct ring_spec *spec,
		    void *desc,
		    uint32_t num_descs)
{
	spec->rs_ring_addr = ring->accel_ring.ring_base_pa;
	spec->rs_pndx_addr = ring->accel_ring.ring_pndx_pa;
	spec->rs_pndx_shadow_addr = ring->accel_ring.ring_shadow_pndx_pa;
	spec->rs_desc_size = (uint8_t) ilog2(ring->accel_ring.ring_desc_size);
	spec->rs_pndx_size = (uint8_t) ilog2(ring->accel_ring.ring_pndx_size);
	spec->rs_ring_size = (uint8_t) ilog2(ring->accel_ring.ring_size);
	spec->rs_desc_addr = sonic_virt_to_phy(desc);
	spec->rs_num_descs = num_descs;

	return PNSO_OK;
}

static uint64_t
set_scratch_buffer(const struct service_buf_list *svc_blist,
		uint64_t src_buf_pa, uint32_t len)
{
	char *p;

	/*
	 * TODO: First SGE of user input buffer list is assumed to be always
	 * 4KB for NetApp, remove this assumption
	 *
	 */
	p = (char *) svc_blist->blist->buffer_0_va;
	return (p[PAGE_SIZE-CPDC_SCRATCH_BUFFER_LEN]) ?
		cp_scratch_buffer_1 : cp_scratch_buffer_2;
}

pnso_error_t
putil_get_cp_prepad_packed_sgl(const struct per_core_resource *pcr,
		const struct service_buf_list *svc_blist,
		uint32_t block_size,
		enum mem_pool_type mpool_type,
		struct service_cpdc_sgl *svc_sgl)
{
	pnso_error_t err;
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl;
	struct buffer_addr_len addr_len;
	uint32_t total_len, magic_byte;

	struct cpdc_sgl *first_sgl = NULL;
	bool use_iter = true;

	if (!svc_blist->blist || svc_blist->blist->count == 0) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer list null/empty! err: %d", err);
		return err;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
			svc_blist->len);

	if (iter) {
		OSAL_LOG_DEBUG("svc_blist->len: %u", svc_blist->len);

		/* prefetch the magic/hack page */
		magic_byte = PAGE_SIZE-CPDC_SCRATCH_BUFFER_LEN;
		iter->user_block = (char *) svc_blist->blist->buffer_0_va;
		if (iter->user_block) {
			__builtin_prefetch(&iter->user_block[magic_byte], 0, 1);
			OSAL_LOG_DEBUG("iter->user_block: 0x" PRIx64 " magic_byte: %d",
					(uint64_t) iter->user_block,
					iter->user_block[magic_byte]);
		}
	}

	svc_sgl->mpool_type = mpool_type;
	svc_sgl->sgl = NULL;
	total_len = 0;
	while (iter) {
		sgl = pc_res_mpool_object_get(pcr, mpool_type);
		if (!sgl) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("cannot obtain sgl_vec from pool, current_len %u err: %d",
				       total_len, err);
			goto out;
		}
		memset(sgl, 0, sizeof(*sgl));

		if (!first_sgl) {
			first_sgl = sgl;

			/* set to any one of the CP scratch buffers */
			first_sgl->cs_addr_0 = cp_scratch_buffer_1;
			first_sgl->cs_len_0 = CPDC_SCRATCH_BUFFER_LEN;

			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
		} else {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl->cs_addr_0, sgl->cs_len_0,
					addr_len);
		}

		if (iter) {
			if (!use_iter) {
				iter = buffer_list_iter_addr_len_get(iter,
						block_size, &addr_len);
			} else
				use_iter = false;

			BUFFER_ADDR_LEN_SET(sgl->cs_addr_1, sgl->cs_len_1,
					addr_len);
		}
		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl->cs_addr_2, sgl->cs_len_2,
					addr_len);
		}

		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl->cs_addr_0) {
			pc_res_mpool_object_put(pcr, svc_sgl->mpool_type, sgl);
			break;
		}
		total_len += sgl->cs_len_0 + sgl->cs_len_1 + sgl->cs_len_2;

		if (!svc_sgl->sgl)
			svc_sgl->sgl = sgl;
		else {
			sgl_prev->cs_next = sonic_virt_to_phy(sgl);
			CPDC_SGL_SWLINK_SET(sgl_prev, sgl);
		}
		sgl_prev = sgl;
	}

	if (first_sgl) {
		OSAL_LOG_DEBUG("=== total_len: %u scratch_len: %u",
				total_len, first_sgl->cs_len_0);

		first_sgl->cs_addr_0 = set_scratch_buffer(svc_blist,
				first_sgl->cs_addr_1, first_sgl->cs_len_1);
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list is empty! err: %d", err);
		goto out;
	}

	return PNSO_OK;
out:
	pc_res_sgl_put(pcr, svc_sgl);
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
putil_get_dc_prepad_packed_sgl(const struct per_core_resource *pcr,
		      const struct service_buf_list *svc_blist,
		      uint32_t block_size,
		      enum mem_pool_type mpool_type,
		      struct service_cpdc_sgl *svc_sgl)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl;
	struct buffer_addr_len addr_len;
	uint32_t total_len;
	pnso_error_t err;

	struct cpdc_sgl *first_sgl = NULL;
	bool use_iter = true;

	if (!svc_blist->blist || svc_blist->blist->count == 0) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer list null/empty! err: %d", err);
		return err;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
			svc_blist->len);

	svc_sgl->mpool_type = mpool_type;
	svc_sgl->sgl = NULL;
	total_len = 0;
	while (iter) {
		sgl = pc_res_mpool_object_get(pcr, mpool_type);
		if (!sgl) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("cannot obtain sgl_vec from pool, current_len %u err: %d",
				       total_len, err);
			goto out;
		}
		memset(sgl, 0, sizeof(*sgl));

		if (!first_sgl) {
			first_sgl = sgl;

			first_sgl->cs_addr_0 = dc_scratch_buffer;
			first_sgl->cs_len_0 = CPDC_SCRATCH_BUFFER_LEN;

			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
		} else {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);

			BUFFER_ADDR_LEN_SET(sgl->cs_addr_0, sgl->cs_len_0,
					addr_len);
		}

		if (iter) {
			if (!use_iter) {
				iter = buffer_list_iter_addr_len_get(iter,
						block_size, &addr_len);
			} else
				use_iter = false;

			BUFFER_ADDR_LEN_SET(sgl->cs_addr_1, sgl->cs_len_1,
					addr_len);
		}
		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl->cs_addr_2, sgl->cs_len_2,
					addr_len);
		}

		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl->cs_addr_0) {
			pc_res_mpool_object_put(pcr, svc_sgl->mpool_type, sgl);
			break;
		}
		total_len += sgl->cs_len_0 + sgl->cs_len_1 + sgl->cs_len_2;

		if (!svc_sgl->sgl)
			svc_sgl->sgl = sgl;
		else {
			sgl_prev->cs_next = sonic_virt_to_phy(sgl);
			CPDC_SGL_SWLINK_SET(sgl_prev, sgl);
		}
		sgl_prev = sgl;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list is empty! err: %d", err);
		goto out;
	}

	return PNSO_OK;
out:
	pc_res_sgl_put(pcr, svc_sgl);
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
putil_get_dc_packed_sgl(const struct per_core_resource *pcr,
		      const struct service_buf_list *svc_blist,
		      uint32_t block_size,
		      enum mem_pool_type mpool_type,
		      struct service_cpdc_sgl *svc_sgl)
{
	pnso_error_t err;
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl;
	struct buffer_addr_len addr_len;
	uint32_t total_len;

	/*
	 * This routne programs single-sgl, in such a way that first and
	 * second pair of addr/len to point to the user-supplied output
	 * buffer/len to avoid truncation issue in HW.
	 *
	 * This routine assumes that the caller supplied buffer len is less
	 * than or equal to 4K.
	 *
	 */
	if (!svc_blist->blist || svc_blist->blist->count != 1) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer list null/empty! err: %d", err);
		return err;
	}

	svc_sgl->mpool_type = mpool_type;
	svc_sgl->sgl = NULL;
	total_len = 0;

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
			svc_blist->len);
	if (iter) {
		sgl = pc_res_mpool_object_get(pcr, mpool_type);
		if (!sgl) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("cannot obtain sgl from pool! err: %d",
					err);
			goto out;
		}
		memset(sgl, 0, sizeof(*sgl));

		iter = buffer_list_iter_addr_len_get(iter, block_size,
				&addr_len);

		sgl->cs_addr_0 = addr_len.addr;
		sgl->cs_len_0 = addr_len.len;

		sgl->cs_addr_1 = addr_len.addr;
		sgl->cs_len_1 = addr_len.len;

		total_len = addr_len.len;
		svc_sgl->sgl = sgl;

		OSAL_LOG_DEBUG("addr_len.addr: 0x" PRIx64 " addr_len.len: %u cs_addr_0: 0x" PRIx64 " cs_len_0: %u cs_addr_1: 0x" PRIx64 " cs_len_1: %u",
				addr_len.addr, addr_len.len,
				sgl->cs_addr_0, sgl->cs_len_0, 
				sgl->cs_addr_1, sgl->cs_len_1);
	}

	if (!total_len) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list is empty! err: %d", err);
		goto out;
	}

	return PNSO_OK;
out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
pc_res_sgl_packed_get(const struct per_core_resource *pcr,
		      const struct service_buf_list *svc_blist,
		      uint32_t block_size,
		      enum mem_pool_type mpool_type,
		      struct service_cpdc_sgl *svc_sgl)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl;
	struct buffer_addr_len addr_len;
	uint32_t total_len;
	pnso_error_t err;

	if (!svc_blist->blist || svc_blist->blist->count == 0) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer list null/empty! err: %d", err);
		return err;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
			svc_blist->len);

	svc_sgl->mpool_type = mpool_type;
	svc_sgl->sgl = NULL;
	total_len = 0;
	while (iter) {
		sgl = pc_res_mpool_object_get(pcr, mpool_type);
		if (!sgl) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("cannot obtain sgl_vec from pool, current_len %u err: %d",
				       total_len, err);
			goto out;
		}
		memset(sgl, 0, sizeof(*sgl));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
				&addr_len);
		BUFFER_ADDR_LEN_SET(sgl->cs_addr_0, sgl->cs_len_0, addr_len);

		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl->cs_addr_1, sgl->cs_len_1,
					addr_len);
		}
		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl->cs_addr_2, sgl->cs_len_2,
					addr_len);
		}

		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl->cs_addr_0) {
			pc_res_mpool_object_put(pcr, svc_sgl->mpool_type, sgl);
			break;
		}
		total_len += sgl->cs_len_0 + sgl->cs_len_1 + sgl->cs_len_2;

		if (!svc_sgl->sgl)
			svc_sgl->sgl = sgl;
		else {
			sgl_prev->cs_next = sonic_virt_to_phy(sgl);
			CPDC_SGL_SWLINK_SET(sgl_prev, sgl);
		}
		sgl_prev = sgl;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list is empty! err: %d", err);
		goto out;
	}

	return PNSO_OK;
out:
	pc_res_sgl_put(pcr, svc_sgl);
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

void
pc_res_sgl_put(const struct per_core_resource *pcr,
	       struct service_cpdc_sgl *svc_sgl)
{
	struct cpdc_sgl *sgl_next;
	struct cpdc_sgl *sgl;

	sgl = svc_sgl->sgl;
	switch (svc_sgl->mpool_type) {

	case MPOOL_TYPE_CPDC_SGL:
		while (sgl) {
			CPDC_SGL_SWLINK_GET(sgl_next, sgl);
			pc_res_mpool_object_put(pcr, svc_sgl->mpool_type, sgl);
			sgl = sgl_next;
		}
		break;

	default:
		/*
		 * Vector cases
		 */
		pc_res_mpool_object_put(pcr, svc_sgl->mpool_type, sgl);
		break;
	}
	svc_sgl->sgl = NULL;
}

pnso_error_t
pc_res_sgl_vec_packed_get(const struct per_core_resource *pcr,
			  const struct service_buf_list *svc_blist,
			  uint32_t block_size,
			  enum mem_pool_type vec_type,
			  struct service_cpdc_sgl *svc_sgl,
                          bool append_extra_sgl)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl_vec;
	struct buffer_addr_len addr_len;
	uint32_t total_len;
	uint32_t num_vec_elems;
	uint32_t elem_size;
	uint32_t cur_count;
	pnso_error_t err;

	if (!svc_blist->blist || svc_blist->blist->count == 0) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer list null/empty! err: %d", err);
		return err;
	}

	total_len = 0;
	svc_sgl->mpool_type = vec_type;
	svc_sgl->sgl =
		pc_res_mpool_object_get_with_num_vec_elems(pcr, vec_type,
				&num_vec_elems, &elem_size);
	if (!svc_sgl->sgl) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("cannot obtain sgl_vec from pool %s err: %d",
			       mpool_get_type_str(vec_type), err);
		goto out;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
			svc_blist->len);
	sgl_vec = svc_sgl->sgl;
	cur_count = 0;
	while (iter && (cur_count < num_vec_elems)) {
		memset(sgl_vec, 0, sizeof(*sgl_vec));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
				&addr_len);
		BUFFER_ADDR_LEN_SET(sgl_vec->cs_addr_0, sgl_vec->cs_len_0,
				addr_len);

		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl_vec->cs_addr_1,
					sgl_vec->cs_len_1, addr_len);
		}
		if (iter) {
			iter = buffer_list_iter_addr_len_get(iter, block_size,
					&addr_len);
			BUFFER_ADDR_LEN_SET(sgl_vec->cs_addr_2,
					sgl_vec->cs_len_2, addr_len);
		}

		/*
		 * See comments in crypto_aol_packed_get() regarding dropping
		 * AOL when ca_len_0 == 0. The same logic applies here.
		 */
		if (!sgl_vec->cs_addr_0)
			break;

		total_len += sgl_vec->cs_len_0 + sgl_vec->cs_len_1 +
			     sgl_vec->cs_len_2;
		if (sgl_prev) {
			sgl_prev->cs_next = sonic_virt_to_phy(sgl_vec);
			CPDC_SGL_SWLINK_SET(sgl_prev, sgl_vec);
		}

		sgl_prev = sgl_vec++;
		cur_count++;
	}

	/*
	 * Currently the only use case for append_extra_sgl is for P4+ chainer
         * padding modifications, and it's only needed if all tuples in the
         * last SGL were filled.
	 */
	if (append_extra_sgl && sgl_prev && sgl_prev->cs_addr_2) {
		if (cur_count >= num_vec_elems) {
			err = ENOSPC;
			OSAL_LOG_ERROR("no room in SGL vector for extra, cur_count %u err: %d",
					cur_count, err);
			goto out;
		}
		memset(sgl_vec, 0, sizeof(*sgl_vec));
		sgl_prev->cs_next = sonic_virt_to_phy(sgl_vec);
		CPDC_SGL_SWLINK_SET(sgl_prev, sgl_vec);
	}

	if (buffer_list_iter_more_data(iter)) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list total length exceeds SGL vector, total_len %u err: %d",
			       total_len, err);
		goto out;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to
	 * begin with.
	 *
	 */
	if (!total_len) {
		err = EINVAL;
		OSAL_LOG_ERROR("buffer_list is empty! err: %d", err);
		goto out;
	}

	return PNSO_OK;
out:
	pc_res_sgl_put(pcr, svc_sgl);
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

/*
 * Format a packed chain SGL for PDMA purposes. Note that TxDMA mem2mem has
 * a transfer limit so each SGL addr/len must be within this limit.
 */
struct chain_sgl_pdma *
pc_res_sgl_pdma_packed_get(const struct per_core_resource *pcr,
			   const struct service_buf_list *svc_blist)
{
	struct chain_sgl_pdma	*sgl_pdma;
	struct buffer_list_iter	buffer_list_iter;
	struct buffer_list_iter	*iter;
	struct buffer_addr_len	addr_len;
	uint32_t		total_len = 0;
	uint32_t		i;

	sgl_pdma = pc_res_mpool_object_get(pcr, MPOOL_TYPE_CHAIN_SGL_PDMA);
	if (sgl_pdma) {
		memset(sgl_pdma, 0, sizeof(*sgl_pdma));
		iter = buffer_list_iter_init(&buffer_list_iter, svc_blist,
					     svc_blist->len);
		for (i = 0; iter && (i < ARRAY_SIZE(sgl_pdma->tuple)); i++) {
			iter = buffer_list_iter_addr_len_get(iter,
				SGL_PDMA_TUPLE_MAX_LEN, &addr_len);
			BUFFER_ADDR_LEN_SET(sgl_pdma->tuple[i].addr,
					    sgl_pdma->tuple[i].len, addr_len);
			total_len += sgl_pdma->tuple[i].len;
		}

		if (buffer_list_iter_more_data(iter)) {
			OSAL_LOG_ERROR("buffer_list exceeds all SGL PDMA tuples, total_len %u",
				       total_len);
			goto out;
		}
	}

	return sgl_pdma;
out:
	pc_res_mpool_object_put(pcr, MPOOL_TYPE_CHAIN_SGL_PDMA, sgl_pdma);
	return NULL;
}

void
pc_res_sgl_pdma_put(const struct per_core_resource *pcr,
		    struct chain_sgl_pdma *sgl_pdma)
{
	pc_res_mpool_object_put(pcr, MPOOL_TYPE_CHAIN_SGL_PDMA, sgl_pdma);
}

pnso_error_t
pc_res_svc_status_get(const struct per_core_resource *pcr,
		      enum mem_pool_type mpool_type,
		      struct service_status_desc *svc_status)
{
	pnso_error_t err = EAGAIN;

	svc_status->desc = pc_res_mpool_object_get_with_num_vec_elems(pcr,
					mpool_type, &svc_status->num_elems,
					&svc_status->elem_size);
	if (!svc_status->desc) {
		if (!mpool_type_is_soft_get_error(mpool_type)) {
			OSAL_LOG_DEBUG("cannot obtain service_status from pool "
				"%s: err: %d", mpool_get_type_str(mpool_type),
				err);
		}
		goto out;
	}
	svc_status->mpool_type = mpool_type;
	svc_status->status_addr = mpool_get_object_phy_addr(mpool_type,
					svc_status->desc);
	err = PNSO_OK;
out:
	return err;
}

void
pc_res_svc_status_put(const struct per_core_resource *pcr,
		      struct service_status_desc *svc_status)
{
	if (svc_status->desc) {
		pc_res_mpool_object_put(pcr, svc_status->mpool_type,
					svc_status->desc);
		svc_status->desc = NULL;
	}
}

pnso_error_t
svc_status_desc_addr_get(struct service_status_desc *svc_status,
			 uint32_t elem_idx,
			 uint64_t *ret_addr,
			 uint32_t clear_size)
{
	pnso_error_t err = EINVAL;
	uint32_t offset;

	*ret_addr = 0;
	if (!svc_status->desc) {
		OSAL_LOG_ERROR("empty service_status_desc: err: %d", err);
		goto out;
	}
	if (elem_idx >= svc_status->num_elems) {
		OSAL_LOG_ERROR("elem_idx %u exceeds num_elems %u: err: %d",
			       elem_idx, svc_status->num_elems, err);
		goto out;
	}

	offset = elem_idx * svc_status->elem_size;
	*ret_addr = svc_status->status_addr + offset;
	if (clear_size) {
		clear_size = min(svc_status->elem_size, clear_size);
		if (mpool_type_is_rmem(svc_status->mpool_type))
			osal_rmem_set(*ret_addr, 0, clear_size);
		else
			memset((char *)svc_status->desc + offset, 0,
					clear_size);
	}

	err = PNSO_OK;
out:
	return err;
}

struct buffer_list_iter *
buffer_list_iter_init(struct buffer_list_iter *iter,
		      const struct service_buf_list *svc_blist,
		      uint32_t total_len_max)
{
	const struct pnso_buffer_list *buf_list = svc_blist->blist;

	OSAL_ASSERT(total_len_max);
	memset(iter, 0, sizeof(*iter));
	iter->blist_type = svc_blist->type;
	iter->total_len_max = total_len_max;
	if (buf_list->count) {
		iter->cur_count = buf_list->count;
		iter->cur_list = &buf_list->buffers[0];
		iter->cur_len = iter->cur_list->len;
		iter->cur_addr =
			iter->blist_type == SERVICE_BUF_LIST_TYPE_HOST ?
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
		iter->cur_addr =
			iter->blist_type == SERVICE_BUF_LIST_TYPE_HOST ?
			sonic_hostpa_to_devpa(iter->cur_list->buf) :
			iter->cur_list->buf;

		return iter;
	}

	return NULL;
}

struct buffer_list_iter *
buffer_list_iter_addr_len_get(struct buffer_list_iter *iter,
			      uint32_t tuple_len_max,
			      struct buffer_addr_len *ret_addr_len)
{
	uint32_t len = 0;

	OSAL_ASSERT(tuple_len_max);
	ret_addr_len->addr = 0;
	ret_addr_len->len = 0;

	/*
	 * Truncate list once total_len_max has been reached
	 */
	if (!iter->total_len_max)
		iter = NULL;

	while (iter && iter->total_len_max) {
		if (iter->cur_len == 0) {
			iter = buffer_list_iter_next(iter);
			continue;
		}

		if (len)
			break;

		len = min(iter->cur_len, tuple_len_max);
		len = min(len, iter->total_len_max);
		ret_addr_len->addr = iter->cur_addr;
		ret_addr_len->len = len;
		iter->cur_addr += len;
		iter->cur_len -= len;
		iter->total_len_max -= len;
	}

	return iter;
}

bool
buffer_list_iter_more_data(struct buffer_list_iter *iter)
{
	if (iter && iter->total_len_max) {
		OSAL_LOG_DEBUG("buffer list iterator type: %u total_len_max: %u cur_count: %u cur_len: %u",
				iter->blist_type, iter->total_len_max,
				iter->cur_count, iter->cur_len);
		return true;
	}

	return false;
}

pnso_error_t
putil_get_interm_buf_list(struct service_info *svc_info)
{
	struct interm_buf_list	*iblist;
	struct pnso_flat_buffer	*iblist_buf;
	void			*ibuf;
	uint32_t		max_nominal_bufs;
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
	    (req_size <= INTERM_BUF_NOMINAL_SIZE())) {

		iblist->buf_type = MPOOL_TYPE_RMEM_INTERM_BUF;
		size_left = req_size;
		max_nominal_bufs = INTERM_BUF_MAX_NUM_NOMINAL_BUFS();
		while (size_left  && (iblist->blist.count < max_nominal_bufs)) {

			ibuf = pc_res_mpool_object_get_with_size(
				svc_info->si_pcr, iblist->buf_type, &buf_size);
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
	putil_put_interm_buf_list(svc_info);
	return ENOMEM;
}

void
putil_put_interm_buf_list(struct service_info *svc_info)
{
	struct interm_buf_list	*iblist = &svc_info->si_iblist;
	struct pnso_flat_buffer	*iblist_buf = &iblist->blist.buffers[0];
	void			*ibuf;

	while (iblist->blist.count) {
		OSAL_ASSERT(iblist_buf->buf);

		ibuf = mpool_get_object_alloc_addr(iblist->buf_type,
				iblist_buf->buf);

		pc_res_mpool_object_put(svc_info->si_pcr,
				iblist->buf_type, ibuf);
		iblist_buf++;
		iblist->blist.count--;
	}
}

#define POLL_LOOP_BASE_TIMEOUT (4 * OSAL_NSEC_PER_SEC)

static inline bool
_svc_poll_expiry_check(uint64_t *ts)
{
	uint64_t cur_ts, delta;

	cur_ts = osal_get_clock_nsec();
	if (!(*ts)) {
		*ts = cur_ts;
		return false;
	}

	delta = cur_ts - *ts;
	if (delta >= POLL_LOOP_BASE_TIMEOUT) {
		OSAL_LOG_WARN("Delta " PRIu64 " larger than allowed timeout " PRIu64,
			      delta, (uint64_t) POLL_LOOP_BASE_TIMEOUT);
		return true;
	}

	return false;
}

bool
svc_poll_expiry_check(struct service_info *svc_info)
{
	struct service_chain *chain = svc_info->si_centry->ce_chain_head;

	if (chain->sc_batch_info) {
		return _svc_poll_expiry_check(&chain->sc_batch_info->bi_poll_ts);
	} else {
		return _svc_poll_expiry_check(&chain->sc_poll_ts);
	}
}

static pnso_error_t
svc_batch_seq_desc_setup(struct service_info *svc_info,
			 void *seq_desc,
			 uint32_t desc_size)
{
	pnso_error_t err;
	struct service_batch_info *svc_batch_info;
	struct batch_page *page;
	uint32_t batch_size, remaining;
        struct rate_limit_control rl;

	svc_batch_info = &svc_info->si_batch_info;

	/*
	 * Calculate cumulative data length for all starter services
	 * in the page. Services in sub-chains are accounted for using
	 * chain_params data_len.
	 */
	if (chn_service_is_starter(svc_info)) {
		page = svc_batch_info->sbi_page;
		if (page) {
			svc_rate_limit_control_eval(svc_info, &rl);
			if (rl.rate_limit_src_en) {
				page->bp_src_data_len +=
					svc_info->si_seq_info.sqi_src_data_len;
				page->bp_rl_control.rate_limit_src_en = true;
			}
			if (rl.rate_limit_dst_en) {
				page->bp_dst_data_len +=
					svc_info->si_seq_info.sqi_dst_data_len;
				page->bp_rl_control.rate_limit_dst_en = true;
			}
			page->bp_rl_control.rate_limit_en |= rl.rate_limit_en;
		}
	}

	if (svc_batch_info->sbi_desc_idx != 0) {
		OSAL_LOG_DEBUG("sequencer setup not needed!");
		return PNSO_OK;
	}

	OSAL_ASSERT(svc_batch_info->sbi_num_entries);
	remaining = svc_batch_info->sbi_num_entries -
		(svc_batch_info->sbi_bulk_desc_idx * MAX_PAGE_ENTRIES);
	batch_size = (remaining / MAX_PAGE_ENTRIES) ? MAX_PAGE_ENTRIES :
		remaining;

	/* indicate batch processing only for 1st entry in the batch */
	svc_info->si_seq_info.sqi_batch_mode = true;
	svc_info->si_seq_info.sqi_batch_size = batch_size;

	err = seq_setup_desc(svc_info, seq_desc, desc_size,
			&svc_info->si_seq_info.sqi_desc);
	if (err)
		OSAL_LOG_DEBUG("failed to setup sequencer desc!");

	return err;
}

pnso_error_t
svc_seq_desc_setup(struct service_info *svc_info,
		   void *seq_desc,
		   uint32_t desc_size,
		   uint32_t num_tags)
{
	pnso_error_t err = PNSO_OK;

	if (chn_service_is_batch_starter(svc_info)) {
		err = svc_batch_seq_desc_setup(svc_info, seq_desc, desc_size);
		if (err)
			OSAL_LOG_DEBUG("failed to setup batch sequencer desc! err: %d",
					err);
		goto out;
	}

	if (chn_service_is_starter(svc_info)) {
		if (num_tags > 1) {
			svc_info->si_seq_info.sqi_batch_mode = true;
			svc_info->si_seq_info.sqi_batch_size = num_tags;
		}

		err = seq_setup_desc(svc_info, seq_desc, desc_size,
				&svc_info->si_seq_info.sqi_desc);
		if (err) {
			OSAL_LOG_DEBUG("failed to setup sequencer desc! num_tags: %d flags: %d err: %d",
						num_tags, svc_info->si_flags, err);
			goto out;
		}
	}

out:
	return err;
}

struct mem_pool *
pc_res_mpool_get(const struct per_core_resource *pcr,
		 enum mem_pool_type type)
{
	if (pcr && mpool_type_is_valid(type))
		return pcr->mpools[type];

	OSAL_LOG_ERROR("invalid pcr 0x"PRIx64" or mpool type %s",
		       (uint64_t)pcr, mpool_get_type_str(type));

	return NULL;
}

void *
pc_res_mpool_object_get(const struct per_core_resource *pcr,
			enum mem_pool_type type)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	mpool = pc_res_mpool_get(pcr, type);
	if (mpool) {
		obj = mpool_get_object(mpool);
		if (!obj && !mpool_type_is_soft_get_error(type))
			OSAL_LOG_ERROR("cannot obtain object from pool %s",
					mpool_get_type_str(type));
	}

	return obj;
}

void *
pc_res_mpool_object_get_with_size(const struct per_core_resource *pcr,
				  enum mem_pool_type type,
				  uint32_t *ret_size)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	*ret_size = 0;
	mpool = pc_res_mpool_get(pcr, type);
	if (mpool) {
		*ret_size = mpool_get_object_size(mpool);
		obj = mpool_get_object(mpool);
		if (!obj && !mpool_type_is_soft_get_error(type))
			OSAL_LOG_ERROR("cannot obtain pcr object from pool %s",
					mpool_get_type_str(type));
	}

	return obj;
}

void *
pc_res_mpool_object_get_with_num_vec_elems(const struct per_core_resource *pcr,
					   enum mem_pool_type type,
					   uint32_t *ret_num_vec_elems,
					   uint32_t *ret_elem_size)
{
	struct mem_pool *mpool;
	void *obj = NULL;

	*ret_num_vec_elems = 0;
	mpool = pc_res_mpool_get(pcr, type);
	if (mpool) {
		*ret_num_vec_elems = mpool_get_object_num_vec_elems(mpool);
		*ret_elem_size = mpool_get_object_base_size(mpool) +
				 mpool_get_object_pad_size(mpool);
		obj = mpool_get_object(mpool);
		if (!obj && !mpool_type_is_soft_get_error(type))
			OSAL_LOG_DEBUG("cannot obtain pcr object from pool %s",
					mpool_get_type_str(type));
	}

	return obj;
}

void
pc_res_mpool_object_put(const struct per_core_resource *pcr,
			enum mem_pool_type type,
			void *obj)
{
	struct mem_pool *mpool;

	if (obj) {
		mpool = pc_res_mpool_get(pcr, type);
		mpool_put_object(mpool, obj);
	}
}

void
pprint_chain_sgl_pdma(uint64_t sgl_pa)
{
	const struct chain_sgl_pdma *sgl;
	const struct chain_sgl_pdma_tuple *tuple;

	if(!OSAL_LOG_ON(OSAL_LOG_LEVEL_DEBUG))
		return;
	if (sgl_pa) {
		sgl = (const struct chain_sgl_pdma *) sonic_phy_to_virt(sgl_pa);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 " ==> 0x" PRIx64,
				"", (uint64_t)sgl, sgl_pa);

		tuple = sgl->tuple;
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/%d 0x" PRIx64 "/%d 0x" PRIx64 "/%d 0x" PRIx64 "/%d",
				"",
				tuple[0].addr, tuple[0].len,
				tuple[1].addr, tuple[1].len,
				tuple[2].addr, tuple[2].len,
				tuple[3].addr, tuple[3].len);
	}
}

bool
putil_is_bulk_desc_in_use(uint16_t flags)
{
	return ((flags & CHAIN_SFLAG_IN_BATCH) &&
			((flags & CHAIN_SFLAG_LONE_SERVICE) ||
			 (flags & CHAIN_SFLAG_FIRST_SERVICE))) ? true : false;
}

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

	if (!sonic_try_reserve_per_core_res(pcr)) {
		pcr = NULL;
	}

	return pcr;
out:
	return NULL;
}

void
putil_put_per_core_resource(struct per_core_resource *pcr)
{
	sonic_unreserve_per_core_res(pcr);
}
