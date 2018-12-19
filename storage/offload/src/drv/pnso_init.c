/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
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
#include "pnso_stats.h"

uint64_t pad_buffer;
static bool pnso_initialized;
static struct pc_res_init_params pc_initialized_params;

static uint32_t
seq_sq_descs_max_get(struct lif *lif);

static pnso_error_t
pc_res_init(struct pc_res_init_params *pc_init,
	    struct per_core_resource *pcr);
static void
pc_res_deinit(struct per_core_resource *pcr);

static pnso_error_t
pc_res_interm_buf_init(struct pc_res_init_params *pc_init,
		       struct per_core_resource *pcr,
		       uint32_t pc_num_bufs);
static void
pc_res_interm_buf_deinit(struct per_core_resource *pcr);

pnso_error_t
pnso_init(struct pnso_init_params *pnso_init)
{
	struct lif			*lif = sonic_get_lif();
	struct per_core_resource	*pcr;
	uint32_t			num_pc_res = sonic_get_num_per_core_res(lif);
	uint32_t			avail_bufs;
	uint32_t			pc_num_bufs;
	uint32_t			i;
	pnso_error_t			err = PNSO_OK;

	if(!sonic_is_accel_dev_ready())
		return EAGAIN;

	if (pnso_initialized) {
		if (pnso_init->per_core_qdepth >
		    pc_initialized_params.max_seq_sq_descs) {
			OSAL_LOG_ERROR("per_core_qdepth %u larger than max %u\n",
				       pnso_init->per_core_qdepth,
				       pc_initialized_params.max_seq_sq_descs);
			err = EINVAL;
		}
		if (pnso_init->block_size !=
		    pc_initialized_params.pnso_init.block_size) {
			OSAL_LOG_ERROR("block_size %u does not match previous cfg %u\n",
				       pnso_init->block_size,
				       pc_initialized_params.pnso_init.block_size);
			err = EINVAL;
		}
		if (pnso_init->core_count < 1 ||
		    pnso_init->core_count > num_pc_res) {
			OSAL_LOG_ERROR("invalid core_count %u, max is %u\n",
				       pnso_init->core_count, num_pc_res);
			err = EINVAL;
		}

		OSAL_LOG_INFO("pnso_init previously initialized, status %d\n",
			      err);
		return err;
	}

	pc_initialized_params.pnso_init = *pnso_init;
	pc_initialized_params.rmem_page_size = sonic_rmem_page_size_get();
	if (!is_power_of_2(pnso_init->block_size) ||
	    !is_power_of_2(pc_initialized_params.rmem_page_size)) {
		err = EINVAL;
		OSAL_LOG_ERROR("block_size or rmem_page_size not power of 2! err: %d",
				err);
		goto out;
	}

	pad_buffer = osal_rmem_aligned_calloc(PNSO_MEM_ALIGN_PAGE,
					      pnso_init->block_size);
	if (!osal_rmem_addr_valid(pad_buffer)) {
		OSAL_LOG_ERROR("failed to allocate global pad buffer!");
		err = ENOMEM;
		goto out;
	}
	OSAL_LOG_DEBUG("pad buffer allocated: 0x" PRIx64, pad_buffer);

	pc_initialized_params.max_seq_sq_descs =
		max((uint32_t)pnso_init->per_core_qdepth,
		     seq_sq_descs_max_get(lif));

	/*
	 * We use 2 passes to initialize per-core resources:
	 * Pass 1: allocate accelerator desc resources including any
	 *         rmem status descriptors. Note that rmem_total_pages
	 *         gets continually adjusted as resources get allocated.
	 */
	if (!num_pc_res) {
		OSAL_LOG_ERROR("num_pc_res must be at least 1");
		err = EPERM;
		goto out;
	}
	if (pnso_init->core_count < 1 ||
	    pnso_init->core_count > num_pc_res) {
		OSAL_LOG_ERROR("invalid core_count %u, max %u\n",
			       pnso_init->core_count, num_pc_res);
		err = EPERM;
		goto out;
	}

	for (i = 0; (err == PNSO_OK) && (i < num_pc_res); i++) {
		pcr = sonic_get_per_core_res_by_res_id(lif, i);
		err = pc_res_init(&pc_initialized_params, pcr);
	}
	if (err) {
		OSAL_LOG_ERROR("failed to init pc_res(%d)\n", i);
		goto out;
	}

	/*
	 * Calculate remaining total rmem bufs
	 */
	avail_bufs = sonic_rmem_avail_pages_get();
	if (pnso_init->block_size < pc_initialized_params.rmem_page_size)
		avail_bufs = (pc_initialized_params.rmem_page_size /
			      pnso_init->block_size) *
			     avail_bufs;
	else
		avail_bufs = avail_bufs  /
			     (pnso_init->block_size /
			      pc_initialized_params.rmem_page_size);
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
		pcr = sonic_get_per_core_res_by_res_id(lif, i);
		err = pc_res_interm_buf_init(&pc_initialized_params, pcr, pc_num_bufs);
	}
	if (err) {
		OSAL_LOG_ERROR("failed to init intermediate buffers\n");
		goto out;
	}
	OSAL_LOG_INFO("pnso_init success\n");

out:
	pnso_initialized = true;

	if (err != PNSO_OK)
		pnso_deinit();

	return err;
}
OSAL_EXPORT_SYMBOL(pnso_init);

void
pnso_set_log_level(int level)
{
	g_osal_log_level = level;
}
OSAL_EXPORT_SYMBOL(pnso_set_log_level);

void
pnso_deinit(void)
{
	struct lif			*lif = sonic_get_lif();
	struct per_core_resource	*pcr;
	uint32_t			num_pc_res;
	uint32_t			i;

	if (!pnso_initialized)
		return;
	//g_osal_log_level = OSAL_LOG_LEVEL_DEBUG;

	num_pc_res = sonic_get_num_per_core_res(lif);
	for (i = 0; i < num_pc_res; i++) {
		pcr = sonic_get_per_core_res_by_res_id(lif, i);
		pas_show_stats(&pcr->api_stats);
		//cpdc_pprint_mpools(pcr);
		sonic_pprint_seq_bmps(pcr);
		pc_res_deinit(pcr);
	}

	if (osal_rmem_addr_valid(pad_buffer))
		osal_rmem_free(pad_buffer, pc_initialized_params.pnso_init.block_size);

	pnso_initialized = false;
}

static pnso_error_t
pc_res_init(struct pc_res_init_params *pc_init, struct per_core_resource *pcr)
{
	pnso_error_t err;

	if (!sonic_rmem_avail_pages_get()) {
		err = ENOMEM;
		OSAL_LOG_ERROR("no rmem pages left! err: %d", err);
		goto out;
	}

	err = cpdc_init_accelerator(pc_init, pcr);
	if (err)
		goto out;

	err = crypto_init_accelerator(pc_init, pcr);
	if (err)
		goto out;

	pas_init(&pcr->api_stats);

out:
	return err;
}

static void
pc_res_deinit(struct per_core_resource *pcr)
{
	cpdc_deinit_accelerator(pcr);
	crypto_deinit_accelerator(pcr);
	pc_res_interm_buf_deinit(pcr);
}

static pnso_error_t
pc_res_interm_buf_init(struct pc_res_init_params *pc_init,
		       struct per_core_resource *pcr,
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
			   &pcr->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
	if (!err)
		err = mpool_create(MPOOL_TYPE_CHAIN_SGL_PDMA,
			   pc_init->max_seq_sq_descs,
			   MPOOL_VEC_ELEM_SINGLE,
			   sizeof(struct chain_sgl_pdma),
			   sizeof(struct chain_sgl_pdma),
			   &pcr->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
	if (!err) {
		MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
		MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
	}

	return err;
}

static void
pc_res_interm_buf_deinit(struct per_core_resource *pcr)
{
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_RMEM_INTERM_BUF]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CHAIN_SGL_PDMA]);
}


static enum sonic_queue_type seq_sq_tbl[] = {
	SONIC_QTYPE_CP_SQ,
	SONIC_QTYPE_DC_SQ,
	SONIC_QTYPE_CRYPTO_ENC_SQ,
	SONIC_QTYPE_CRYPTO_DEC_SQ
};

static uint32_t
seq_sq_descs_max_get(struct lif *lif)
{
	uint32_t	max_descs = 0;
	int		i;

	for (i = 0; i < ARRAY_SIZE(seq_sq_tbl); i++) {
		max_descs = max(max_descs,
				sonic_get_seq_sq_num_descs(lif, seq_sq_tbl[i]));
	}
	OSAL_LOG_DEBUG("max_descs: %u", max_descs);

	return max_descs;
}
