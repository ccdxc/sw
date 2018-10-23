/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>
#include <net.h>
#include <kernel.h>

#include "osal_logger.h"
#include "osal_setup.h"
#include "osal_rmem.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"
#include "pnso_api.h"
#include "pnso_init.h"
#include "pnso_mpool.h"
#include "pnso_cpdc.h"
#include "pnso_crypto.h"
#include "pnso_chain.h"
#include "pnso_chain_params.h"

uint64_t pad_buffer;

static uint32_t
seq_sq_descs_total_get(struct lif *lif);

static pnso_error_t
pc_res_init(struct pc_res_init_params *pc_init,
	    struct per_core_resource *pc_res);
static void
pc_res_deinit(struct per_core_resource *pc_res);

static pnso_error_t
pc_res_interm_buf_init(struct pc_res_init_params *pc_init,
		       struct per_core_resource *pc_res,
		       uint32_t pc_num_bufs);
static void
pc_res_interm_buf_deinit(struct per_core_resource *pc_res);

pnso_error_t
pnso_init(struct pnso_init_params *pnso_init)
{
	struct lif			*lif = sonic_get_lif();
	struct per_core_resource	*pc_res;
	struct pc_res_init_params	pc_init;
	uint32_t			num_pc_res;
	uint32_t			avail_bufs;
	uint32_t			pc_num_bufs;
	uint32_t			i;
	pnso_error_t			err = PNSO_OK;

	pad_buffer = osal_rmem_aligned_calloc(PNSO_MEM_ALIGN_PAGE,
			PNSO_MEM_ALIGN_PAGE);
	if (!osal_rmem_addr_valid(pad_buffer)) {
		OSAL_LOG_ERROR("failed to allocate global pad buffer!");
		err = ENOMEM;
		goto out;
	}
	OSAL_LOG_DEBUG("pad buffer allocated: 0x%llx", pad_buffer);

	memset(&pc_init, 0, sizeof(pc_init));
	pc_init.pnso_init = *pnso_init;
	pc_init.rmem_page_size = sonic_rmem_page_size_get();
	pc_init.max_seq_sq_descs = max((uint32_t)pnso_init->per_core_qdepth,
				       seq_sq_descs_total_get(lif));
	/*
	 * We use 2 passes to initialize per-core resources:
	 * Pass 1: allocate accelerator desc resources including any
	 *         rmem status descriptors. Note that rmem_total_pages
	 *         gets continually adjusted as resources get allocated.
	 */
	num_pc_res = sonic_get_num_per_core_res(lif);
	if (!num_pc_res) {
		OSAL_LOG_ERROR("num_pc_res must be at least 1");
		err = EPERM;
		goto out;
	}

	for (i = 0; (err == PNSO_OK) && (i < num_pc_res); i++) {
		pc_res = sonic_get_per_core_res_by_res_id(lif, i);
		err = pc_res_init(&pc_init, pc_res);
	}

	/*
	 * Calculate remaining total rmem bufs
	 */
	avail_bufs = sonic_rmem_avail_pages_get();
	if (pc_init.pnso_init.block_size < pc_init.rmem_page_size)
		avail_bufs = (pc_init.rmem_page_size /
			      pc_init.pnso_init.block_size) *
			     avail_bufs;
	else
		avail_bufs = avail_bufs  /
			     (pc_init.pnso_init.block_size /
			      pc_init.rmem_page_size);
	pc_num_bufs = avail_bufs / num_pc_res;
	OSAL_LOG_DEBUG("avail_bufs %u pc_num_bufs %u", avail_bufs, pc_num_bufs);
	if ((err == PNSO_OK) && (!avail_bufs || !pc_num_bufs)) {
		OSAL_LOG_ERROR("invalid avail_bufs %u or pc_num_bufs %u",
			       avail_bufs, pc_num_bufs);
		err = ENOMEM;
		goto out;
	}

	/*
	 * Pass 2: use the calculated pc_num_bufs to allocate
	 *         intermediate buffers.
	 */
	for (i = 0; (err == PNSO_OK) && (i < num_pc_res); i++) {
		pc_res = sonic_get_per_core_res_by_res_id(lif, i);
		err = pc_res_interm_buf_init(&pc_init, pc_res, pc_num_bufs);
	}

out:
	if (err != PNSO_OK)
		pnso_deinit();

	return err;
}
OSAL_EXPORT_SYMBOL(pnso_init);

void
pnso_deinit(void)
{
	struct lif			*lif = sonic_get_lif();
	struct per_core_resource	*pc_res;
	uint32_t			num_pc_res;
	uint32_t			i;

	num_pc_res = sonic_get_num_per_core_res(lif);
	for (i = 0; i < num_pc_res; i++) {
		pc_res = sonic_get_per_core_res_by_res_id(lif, i);
		pc_res_deinit(pc_res);
	}

	if (osal_rmem_addr_valid(pad_buffer))
		osal_rmem_free(pad_buffer, PNSO_MEM_ALIGN_PAGE);
}

static pnso_error_t
pc_res_init(struct pc_res_init_params *pc_init,
	    struct per_core_resource *pc_res)
{
	pnso_error_t err;

	if (!is_power_of_2(pc_init->pnso_init.block_size) ||
	    !is_power_of_2(pc_init->rmem_page_size)) {

		OSAL_LOG_ERROR("block_size or rmem_page_size not power of 2");
		return EINVAL;
	}

	if (!sonic_rmem_avail_pages_get()) {
		OSAL_LOG_ERROR("no rmem pages left");
		return ENOMEM;
	}

	err = cpdc_init_accelerator(pc_init, pc_res);
	if (err == PNSO_OK)
		err = crypto_init_accelerator(pc_init, pc_res);

	return err;
}

static void
pc_res_deinit(struct per_core_resource *pc_res)
{
	cpdc_deinit_accelerator(pc_res);
	crypto_deinit_accelerator(pc_res);
	pc_res_interm_buf_deinit(pc_res);
}

static pnso_error_t
pc_res_interm_buf_init(struct pc_res_init_params *pc_init,
		       struct per_core_resource *pc_res,
		       uint32_t pc_num_bufs)
{
	uint32_t	num_buf_vecs;
	pnso_error_t	err;

	/*
	 * Intermediate buffers are allocated as vectors of contiguous buffers,
	 * each vector handles the max request buffer size.
	 */
        num_buf_vecs = pc_num_bufs / INTERM_BUF_NOMINAL_NUM_BUFS;
	if (!num_buf_vecs) {
		OSAL_LOG_ERROR("failure: zero num_buf_vecs");
		return ENOMEM;
	}

	err = mpool_create(MPOOL_TYPE_RMEM_INTERM_BUF, num_buf_vecs,
			   INTERM_BUF_NOMINAL_NUM_BUFS,
			   pc_init->pnso_init.block_size, PNSO_MEM_ALIGN_NONE,
			   &pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
	if (!err)
		err = mpool_create(MPOOL_TYPE_CHAIN_SGL_PDMA,
			   pc_init->max_seq_sq_descs,
			   MPOOL_VEC_ELEM_SINGLE,
			   sizeof(struct chain_sgl_pdma),
			   sizeof(struct chain_sgl_pdma),
			   &pc_res->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
	if (!err) {
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
	}

	return err;
}

static void
pc_res_interm_buf_deinit(struct per_core_resource *pc_res)
{
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
}


static enum sonic_queue_type seq_sq_tbl[] = {
	SONIC_QTYPE_CP_SQ,
	SONIC_QTYPE_DC_SQ,
	SONIC_QTYPE_CRYPTO_ENC_SQ,
	SONIC_QTYPE_CRYPTO_DEC_SQ
};

static uint32_t
seq_sq_descs_total_get(struct lif *lif)
{
	uint32_t	total_descs = 0;
	int		i;

	for (i = 0; i < ARRAY_SIZE(seq_sq_tbl); i++) {
		total_descs +=
			sonic_get_seq_sq_num_descs(lif, seq_sq_tbl[i]);
	}

	return total_descs;
}
