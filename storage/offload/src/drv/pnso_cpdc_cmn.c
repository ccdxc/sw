/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_svc.h"
#include "pnso_mpool.h"
#include "pnso_batch.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_utils.h"
#include "pnso_seq.h"

pnso_error_t
cpdc_poll(struct service_info *svc_info,
		volatile struct cpdc_status_desc *status_desc)
{
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	if (status_desc == NULL)
		status_desc =
			(struct cpdc_status_desc *) svc_info->si_status_desc.desc;

	while (1) {
		err = status_desc->csd_valid ? PNSO_OK : EBUSY;
		if (!err)
			break;

		if (svc_poll_expiry_check(svc_info)) {
			err = ETIMEDOUT;
			OSAL_LOG_ERROR("poll-time limit reached! service: %s status_desc: 0x" PRIx64 " err: %d",
					svc_get_type_str(svc_info->si_type),
					(uint64_t) status_desc, err);
			/* Initiate error reset recovery */
			if (pnso_lif_error_reset_recovery_en_get())
				pnso_lif_reset_ctl_start();
			break;
		}

		if (!(svc_info->si_flags & CHAIN_SFLAG_MODE_SYNC)) {
			OSAL_LOG_DEBUG("transient err: %d", err);
			break;
		}

		if (pnso_lif_reset_ctl_pending()) {
			err = PNSO_LIF_IO_ERROR;
			break;
		}

		osal_yield();
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

static void __attribute__((unused))
pprint_sgl(uint64_t sgl_pa)
{
	const struct cpdc_sgl *sgl;

	if(!OSAL_LOG_ON(OSAL_LOG_LEVEL_DEBUG))
		return;
	sgl = (const struct cpdc_sgl *) sonic_phy_to_virt(sgl_pa);
	if (!sgl)
		return;

	while (sgl) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 " ==> 0x" PRIx64,
			"",
			(uint64_t) sgl, sgl_pa);

		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d",
				"",
				sgl->cs_addr_0, sgl->cs_len_0, sgl->cs_rsvd_0,
				sgl->cs_addr_1, sgl->cs_len_1, sgl->cs_rsvd_1,
				sgl->cs_addr_2, sgl->cs_len_2, sgl->cs_rsvd_2);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/0x" PRIx64,
				"",
				sgl->cs_next, sgl->cs_rsvd_swlink);

		CPDC_SGL_SWLINK_GET(sgl, sgl);

		if (sgl)
			sgl_pa = sonic_virt_to_phy((void *) sgl);
	}
}

static void __attribute__((unused))
pprint_cpdc_cmd(const struct cpdc_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_DEBUG("%30s: %d", "cc_enabled", cmd->cc_enabled);

	OSAL_LOG_DEBUG("%30s: %d", "cc_header_present", cmd->cc_header_present);
	OSAL_LOG_DEBUG("%30s: %d", "cc_insert_header", cmd->cc_insert_header);

	OSAL_LOG_DEBUG("%30s: %d", "cc_db_on", cmd->cc_db_on);
	OSAL_LOG_DEBUG("%30s: %d", "cc_otag_on", cmd->cc_otag_on);

	OSAL_LOG_DEBUG("%30s: %d", "cc_src_is_list", cmd->cc_src_is_list);
	OSAL_LOG_DEBUG("%30s: %d", "cc_dst_is_list", cmd->cc_dst_is_list);

	OSAL_LOG_DEBUG("%30s: %d", "cc_chksum_verify_enabled",
			cmd->cc_chksum_verify_enabled);
	OSAL_LOG_DEBUG("%30s: %d", "cc_chksum_adler", cmd->cc_chksum_adler);

	OSAL_LOG_DEBUG("%30s: %d", "cc_hash_enabled", cmd->cc_hash_enabled);
	OSAL_LOG_DEBUG("%30s: %d", "cc_hash_type", cmd->cc_hash_type);

	OSAL_LOG_DEBUG("%30s: %d", "cc_integrity_src", cmd->cc_integrity_src);
	OSAL_LOG_DEBUG("%30s: %d", "cc_integrity_type", cmd->cc_integrity_type);
}

void __attribute__((unused))
cpdc_pprint_desc(const struct cpdc_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cpdc_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_src", desc->cd_src);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_dst", desc->cd_dst);

	OSAL_LOG_DEBUG("%30s:", "=== cpdc_cmd");
	pprint_cpdc_cmd(&desc->u.cd_bits);

	OSAL_LOG_DEBUG("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_threshold_len", desc->cd_threshold_len);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_status_addr",
			desc->cd_status_addr);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_db_data", desc->cd_db_data);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_otag_addr", desc->cd_otag_addr);
	OSAL_LOG_DEBUG("%30s: %d", "cd_otag_data", desc->cd_otag_data);
	OSAL_LOG_DEBUG("%30s: %d", "cd_status_data", desc->cd_status_data);

	if (desc->u.cd_bits.cc_src_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== src_sgl", desc->cd_src);
		pprint_sgl(desc->cd_src);
	}
	if (desc->u.cd_bits.cc_dst_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== dst_sgl", desc->cd_dst);
		pprint_sgl(desc->cd_dst);
	}
}

void __attribute__((unused))
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	if (!status_desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== status_desc",
			(uint64_t) status_desc);

	OSAL_LOG_DEBUG("%30s: %d", "csd_err", status_desc->csd_err);
	OSAL_LOG_DEBUG("%30s: %d", "csd_valid", status_desc->csd_valid);

	OSAL_LOG_DEBUG("%30s: %d", "csd_output_data_len",
			status_desc->csd_output_data_len);
	OSAL_LOG_DEBUG("%30s: %d", "csd_partial_data",
			status_desc->csd_partial_data);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "csd_integrity_data",
			status_desc->csd_integrity_data);
}

uint32_t
cpdc_get_desc_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_desc) + pad_size;
}

uint32_t
cpdc_get_status_desc_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_status_desc) + pad_size;
}

uint32_t
cpdc_get_sgl_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_sgl),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_sgl) + pad_size;
}

static struct cpdc_desc *
get_next_desc(struct cpdc_desc *desc, uint32_t object_size)
{
	char *obj;

	obj = (char *) desc;
	obj += object_size;
	desc = (struct cpdc_desc *) obj;

	return desc;
}

struct cpdc_status_desc *
cpdc_get_next_status_desc(struct cpdc_status_desc *desc, uint32_t object_size)
{
	char *obj;

	obj = (char *) desc;
	obj += object_size;
	desc = (struct cpdc_status_desc *) obj;

	return desc;
}

static struct cpdc_sgl *
get_next_sgl(struct cpdc_sgl *sgl, uint32_t object_size)
{
	char *obj;

	obj = (char *) sgl;
	obj += object_size;
	sgl = (struct cpdc_sgl *) obj;

	return sgl;
}

static struct cpdc_desc *
get_desc(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC];

	return (struct cpdc_desc *) mpool_get_object(mpool);
}

static struct cpdc_desc *
get_bo_desc(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_PB_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_VECTOR];

	return (struct cpdc_desc *) mpool_get_object(mpool);
}

static struct cpdc_desc *
get_batch_desc(struct service_info *svc_info)
{
	struct service_batch_info *svc_batch_info;
	struct batch_info *batch_info;
	struct batch_page *batch_page;
	struct batch_page_tags *page_tags;
	struct cpdc_desc *desc;
	uint32_t idx;

	svc_batch_info = &svc_info->si_batch_info;
	batch_info = svc_batch_info->sbi_batch_info;

	if (batch_info->bi_mpool_type == MPOOL_TYPE_CPDC_DESC_PB_VECTOR) {
		batch_page = batch_info->bi_pages[
			svc_batch_info->sbi_bulk_desc_idx];
		page_tags = &batch_page->bp_tags;

		idx = (svc_info->si_type == PNSO_SVC_TYPE_HASH) ?
			page_tags->bpt_num_hashes :
			page_tags->bpt_num_chksums;

		desc = &svc_batch_info->u.sbi_cpdc_desc[idx];

		OSAL_LOG_DEBUG("per-block num_entries: %d bulk_desc_idx: %d idx: %u bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64,
				svc_batch_info->sbi_num_entries,
				svc_batch_info->sbi_bulk_desc_idx,
				idx,
				(uint64_t) svc_batch_info->u.sbi_cpdc_desc,
				(uint64_t) desc);
	}  else {
		desc = &svc_batch_info->u.sbi_cpdc_desc[
				svc_batch_info->sbi_desc_idx];

		OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64,
				svc_batch_info->sbi_num_entries,
				svc_batch_info->sbi_desc_idx,
				(uint64_t) svc_batch_info->u.sbi_cpdc_desc,
				(uint64_t) desc);
	}

	return desc;
}

struct cpdc_desc *
cpdc_get_desc(struct service_info *svc_info, bool per_block)
{
	if (putil_is_bulk_desc_in_use(svc_info->si_flags))
		return get_batch_desc(svc_info);

	return (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) ?
		get_bo_desc(svc_info->si_pcr, per_block) :
		get_desc(svc_info->si_pcr, per_block);
}

/* 'batch' is the caller, not the services ... */
struct cpdc_desc *
cpdc_get_batch_bulk_desc(struct mem_pool *mpool)
{
	struct cpdc_desc *desc;

	desc = (struct cpdc_desc *) mpool_get_object(mpool);
	if (!desc) {
		OSAL_LOG_DEBUG("cannot obtain cpdc bulk object from pool!");
		return NULL;
	}

	return desc;
}

static void
put_desc(struct per_core_resource *pcr, bool per_block, struct cpdc_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC];

	mpool_put_object(mpool, desc);
}

static void
put_bo_desc(struct per_core_resource *pcr, bool per_block,
		struct cpdc_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_PB_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_VECTOR];

	mpool_put_object(mpool, desc);
}

static void
put_batch_desc(const struct service_info *svc_info, struct cpdc_desc *desc)
{
	struct service_batch_info *svc_batch_info;

	svc_batch_info = (struct service_batch_info *) &svc_info->si_batch_info;

	/* do nothing */

	OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64,
			svc_batch_info->sbi_num_entries,
			svc_batch_info->sbi_desc_idx,
			(uint64_t) svc_batch_info->u.sbi_cpdc_desc,
			(uint64_t) desc);
}

void
cpdc_put_desc(const struct service_info *svc_info, bool per_block,
		struct cpdc_desc *desc)
{
	if (putil_is_bulk_desc_in_use(svc_info->si_flags)) {
		put_batch_desc(svc_info, desc);
		return;
	}

	if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL)
		put_bo_desc(svc_info->si_pcr, per_block, desc);
	else
		put_desc(svc_info->si_pcr, per_block, desc);
}

/* 'batch' is the caller, not the services ... */
void
cpdc_put_batch_bulk_desc(struct mem_pool *mpool, struct cpdc_desc *desc)
{
	mpool_put_object(mpool, desc);
}

static inline enum mem_pool_type
cpdc_get_status_type(bool per_block)
{
	return per_block ?
		MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR :
		MPOOL_TYPE_CPDC_STATUS_DESC;
}

pnso_error_t
cpdc_setup_status_desc(struct service_info *svc_info,
		       bool per_block)
{
	pnso_error_t err = PNSO_OK;
	struct per_core_resource *pcr;

	pcr = svc_info->si_pcr;
	err = pc_res_svc_status_get(pcr,
			cpdc_get_status_type(per_block),
			&svc_info->si_status_desc);
	if (err) {
		OSAL_LOG_DEBUG("cannot obtain status desc! svc_type: %d per_block: %d err: %d",
				svc_info->si_type, per_block, err);
		goto out;
	}
out:
	return err;
}

void
cpdc_teardown_status_desc(struct service_info *svc_info)
{
	pc_res_svc_status_put(svc_info->si_pcr,
			&svc_info->si_status_desc);
}

static inline enum mem_pool_type
cpdc_get_rmem_status_type(bool per_block)
{
	return per_block ?
		MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC_VECTOR :
		MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC;
}

void
cpdc_teardown_rmem_status_desc(struct service_info *svc_info)
{
	pc_res_svc_status_put(svc_info->si_pcr,	&svc_info->si_istatus_desc);
}

pnso_error_t
cpdc_setup_rmem_status_desc(struct service_info *svc_info,
			    bool per_block)
{
	pnso_error_t err = PNSO_OK;
	struct per_core_resource *pcr;

	pcr = svc_info->si_pcr;
	if (chn_service_has_sub_chain(svc_info) ||
	    chn_service_is_cp_padding_applic(svc_info)) {

		/*
		 * Note: for rmem status, the responsibility of clearing
		 * rests with the caller.
		 */
		err = pc_res_svc_status_get(pcr,
				cpdc_get_rmem_status_type(per_block),
				&svc_info->si_istatus_desc);
		if (err) {
			PAS_INC_NUM_OUT_OF_RMEM_STATUS(svc_info->si_pcr);
		}
	}
	return err;
}

pnso_error_t
cpdc_setup_status_chain_dma(struct service_info *svc_info,
			    struct cpdc_chain_params *chain_params)
{
	pnso_error_t err;

	err = svc_status_desc_addr_get(&svc_info->si_status_desc, 0,
			&chain_params->ccp_status_addr_0, 0);
	if (err)
		goto out;

	if (chn_service_has_interm_status(svc_info)) {
		chain_params->ccp_status_addr_1 = chain_params->ccp_status_addr_0;
		err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, 0,
				&chain_params->ccp_status_addr_0, 0);
		if (err)
			goto out;

		chain_params->ccp_status_len = sizeof(struct cpdc_status_desc);
		chain_params->ccp_cmd.ccpc_status_dma_en = 1;
	}

out:
	return err;
}

struct cpdc_sgl *
cpdc_get_sgl(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_SGL];

	return (struct cpdc_sgl *) mpool_get_object(mpool);
}

void
cpdc_put_sgl(struct per_core_resource *pcr, bool per_block,
		struct cpdc_sgl *sgl)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_SGL];

	mpool_put_object(mpool, sgl);
}

uint32_t
cpdc_fill_per_block_desc(struct service_info *svc_info,
		uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct service_buf_list *svc_blist,
		struct cpdc_sgl *sgl, struct cpdc_desc *desc,
		fill_desc_fn_t fill_desc_fn)
{
	pnso_error_t err;
	struct cpdc_desc *pb_desc;
	struct cpdc_sgl *pb_sgl;
	uint32_t desc_object_size, sgl_object_size;
	uint32_t block_cnt, pb_len, total_len, i = 0;
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct buffer_addr_len addr_len;

	block_cnt = REQ_SZ_TO_NUM_BLKS(svc_blist->len, block_size);
	pb_desc = desc;
	pb_sgl = sgl;

       desc_object_size = cpdc_get_desc_size();
       sgl_object_size = cpdc_get_sgl_size();

       total_len = svc_blist->len;
       iter = buffer_list_iter_init(&buffer_list_iter, svc_blist, total_len);
       for (i = 0; iter && (i < block_cnt); i++) {
	       memset(pb_sgl, 0, sizeof(struct cpdc_sgl));

	       iter = buffer_list_iter_addr_len_get(iter, block_size,
			       &addr_len);

	       BUFFER_ADDR_LEN_SET(pb_sgl->cs_addr_0, pb_sgl->cs_len_0,
			       addr_len);
	       pb_len = pb_sgl->cs_len_0;

	       total_len -= pb_len;
	       if (total_len && (pb_len < block_size)) {
		       err = EINVAL;
		       OSAL_LOG_ERROR("source buffers are not block multiple! block: %d total_len: %d pb_len: %d err: %d",
				       i, total_len, pb_len, err);
		       goto out;
	       }

	       err = fill_desc_fn(svc_info, algo_type, pb_len,
			       pb_sgl, pb_desc, i);
	       if (err) {
		       OSAL_LOG_ERROR("fill_desc_fn block %u err: %d", i, err);
		       goto out;
	       }

	       pb_desc = get_next_desc(pb_desc, desc_object_size);

	       pb_sgl = get_next_sgl(pb_sgl, sgl_object_size);
       }

	return i;
out:
	return 0;
}

pnso_error_t
cpdc_setup_desc_blocks(struct service_info *svc_info, uint32_t algo_type,
		fill_desc_fn_t fill_desc_fn)
{
	pnso_error_t err;
	struct cpdc_desc *desc, *bof_desc;
	struct cpdc_sgl *sgl, *bof_sgl;

	OSAL_LOG_DEBUG("enter ...");

	desc = svc_info->si_desc;
	bof_desc = svc_info->si_bof_desc;
	if (svc_info->si_flags & CHAIN_SFLAG_PER_BLOCK) {
		sgl = cpdc_get_sgl(svc_info->si_pcr, true);
		if (!sgl) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("cannot obtain sgl from pool! err: %d",
					err);
			goto out;
		}
		svc_info->si_pb_sgl = sgl;

		svc_info->si_num_tags = cpdc_fill_per_block_desc(svc_info, algo_type,
				svc_info->si_block_size,
				svc_info->si_src_blist.len,
				&svc_info->si_src_blist, svc_info->si_pb_sgl,
				desc, fill_desc_fn);
		if (svc_info->si_num_tags == 0) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup per-block desc! svc_type: %d err: %d",
					svc_info->si_type, err);
			goto out;
		}
		svc_info->si_num_bytes += svc_info->si_src_blist.len;

		if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
			bof_sgl = cpdc_get_sgl(svc_info->si_pcr, true);
			if (!bof_sgl) {
				err = EAGAIN;
				OSAL_LOG_DEBUG("cannot obtain pb/bof sgl from pool! err: %d",
						err);
				goto out;
			}
			svc_info->si_pb_bof_sgl = bof_sgl;

			bof_desc = &desc[svc_info->si_num_tags];
			svc_info->si_num_bof_tags = cpdc_fill_per_block_desc(svc_info,
					algo_type, svc_info->si_block_size,
					svc_info->si_bof_blist.len,
					&svc_info->si_bof_blist,
					svc_info->si_pb_bof_sgl,
					bof_desc, fill_desc_fn);
			if (svc_info->si_num_bof_tags == 0) {
				err = EINVAL;
				OSAL_LOG_ERROR("failed to setup bypass onfail per-block desc! svc_type: %d num_tags: %d desc: 0x" PRIx64 " bof_desc: 0x" PRIx64 " err: %d",
					svc_info->si_type, svc_info->si_num_bof_tags,
					(uint64_t) desc, (uint64_t) bof_desc,
					err);
				goto out;
			}
		}

		OSAL_LOG_DEBUG("svc_type: %d num_tags: %d desc: 0x" PRIx64 " bof_desc: 0x" PRIx64,
				svc_info->si_type, svc_info->si_num_tags,
				(uint64_t) desc, (uint64_t) bof_desc);
	} else {
		err = cpdc_update_service_info_src_sgl(svc_info);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain src sgl from pool! svc_type: %d err: %d",
					svc_info->si_type, err);
			goto out;
		}

		fill_desc_fn(svc_info, algo_type, svc_info->si_src_blist.len,
				svc_info->si_src_sgl.sgl,
				svc_info->si_desc, 0);

		if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
			bof_desc = (struct cpdc_desc *) ((char *) desc +
					sizeof(struct cpdc_desc));

			OSAL_LOG_DEBUG("svc_type: %d desc: 0x" PRIx64 " bof_desc: 0x" PRIx64,
					svc_info->si_type,
					(uint64_t) desc, (uint64_t) bof_desc);

			err = pc_res_sgl_packed_get(svc_info->si_pcr,
					&svc_info->si_bof_blist,
					svc_info->si_block_size,
					MPOOL_TYPE_CPDC_SGL,
					&svc_info->si_bof_sgl);
			if (err) {
				OSAL_LOG_ERROR("cannot obtain src bof sgl from pool! svc_type: %d err: %d",
						svc_info->si_type, err);
				goto out;
			}

			fill_desc_fn(svc_info, algo_type, svc_info->si_bof_blist.len,
					svc_info->si_bof_sgl.sgl, bof_desc, 0);
		}

		svc_info->si_num_bytes += svc_info->si_src_blist.len;
		svc_info->si_num_tags = 1;
                svc_info->si_num_bof_tags = 1;
	}
	cpdc_update_batch_tags(svc_info, svc_info->si_num_tags);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;
out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
update_service_info_prepad_src_sgl(struct service_info *svc_info)
{
	struct service_info *svc_prev;
	pnso_error_t err;

	/*
	 * P4+ chainer always expects a vector of SGLs (post CP/DC) for
	 * ease of modification and this is true regardless of whether
	 * the next service is operating per-block or full block.
	 *
	 * When a service is first in chain, there are no constraints
	 * and it can use any types of SGL.
	 *
	 * Note also that if the parent service is not CP/DC, then
	 * SGL modifications wouldn't be applicable so the current
	 * service can also use any types of SGL.
	 */
	if (chn_service_is_starter(svc_info) ||
	    !chn_service_prev_svc_type_is_cpdc(svc_info)) {
		if ((svc_info->si_type == PNSO_SVC_TYPE_COMPRESS) &&
				(svc_info->si_src_blist.len >=
				 CPDC_MIN_USER_BUFFER_LEN)) {
			err = putil_get_cp_prepad_packed_sgl(svc_info->si_pcr,
					&svc_info->si_src_blist,
					CPDC_SGL_TUPLE_LEN_MAX,
					MPOOL_TYPE_CPDC_SGL,
					&svc_info->si_src_sgl);
		} else {
			err = pc_res_sgl_packed_get(svc_info->si_pcr,
					&svc_info->si_src_blist,
					CPDC_SGL_TUPLE_LEN_MAX,
					MPOOL_TYPE_CPDC_SGL,
					&svc_info->si_src_sgl);
		}
	} else {
		svc_prev = chn_service_prev_svc_get(svc_info);
		err = pc_res_sgl_vec_packed_get(svc_info->si_pcr,
				&svc_info->si_src_blist,
				svc_info->si_block_size,
				MPOOL_TYPE_CPDC_SGL_VECTOR,
				&svc_info->si_src_sgl,
				chn_service_is_cp_padding_applic(svc_prev));
	}

	return err;
}

pnso_error_t
cpdc_update_service_info_src_sgl(struct service_info *svc_info)
{
	struct service_info *svc_prev;
	pnso_error_t err;

	/*
	 * P4+ chainer always expects a vector of SGLs (post CP/DC) for
	 * ease of modification and this is true regardless of whether
	 * the next service is operating per-block or full block.
	 *
	 * When a service is first in chain, there are no constraints
	 * and it can use any types of SGL.
	 *
	 * Note also that if the parent service is not CP/DC, then
	 * SGL modifications wouldn't be applicable so the current
	 * service can also use any types of SGL.
	 */
	if (chn_service_is_starter(svc_info) ||
	    !chn_service_prev_svc_type_is_cpdc(svc_info)) {
		err = pc_res_sgl_packed_get(svc_info->si_pcr,
				&svc_info->si_src_blist,
				CPDC_SGL_TUPLE_LEN_MAX, MPOOL_TYPE_CPDC_SGL,
				&svc_info->si_src_sgl);
	} else {
		svc_prev = chn_service_prev_svc_get(svc_info);
		err = ((svc_prev->si_type == PNSO_SVC_TYPE_DECOMPRESS) &&
				(svc_prev->si_dst_blist.len >=
				 CPDC_MIN_USER_BUFFER_LEN)) ?
			pc_res_sgl_packed_get(svc_info->si_pcr,
				&svc_info->si_src_blist,
				CPDC_SGL_TUPLE_LEN_MAX, MPOOL_TYPE_CPDC_SGL,
				&svc_info->si_src_sgl) :
			pc_res_sgl_vec_packed_get(svc_info->si_pcr,
				&svc_info->si_src_blist,
				svc_info->si_block_size,
				MPOOL_TYPE_CPDC_SGL_VECTOR,
				&svc_info->si_src_sgl,
				chn_service_is_cp_padding_applic(svc_prev));
	}

	return err;
}

pnso_error_t
cpdc_update_service_info_dst_sgl(struct service_info *svc_info)
{
	pnso_error_t err;

	if (!chn_service_has_sub_chain(svc_info) &&
	    chn_service_is_cp_padding_applic(svc_info)) {
		err = pc_res_sgl_vec_packed_get(svc_info->si_pcr,
				&svc_info->si_dst_blist,
				svc_info->si_block_size,
				MPOOL_TYPE_CPDC_SGL_VECTOR,
				&svc_info->si_dst_sgl, false);
	} else {
		if (svc_info->si_type == PNSO_SVC_TYPE_DECOMPRESS) {
			err = (svc_info->si_dst_blist.len > PAGE_SIZE) ?
				putil_get_dc_prepad_packed_sgl(
						svc_info->si_pcr,
						&svc_info->si_dst_blist,
						CPDC_SGL_TUPLE_LEN_MAX,
						MPOOL_TYPE_CPDC_SGL,
						&svc_info->si_dst_sgl) :
				putil_get_dc_packed_sgl(svc_info->si_pcr,
						&svc_info->si_dst_blist,
						CPDC_SGL_TUPLE_LEN_MAX,
						MPOOL_TYPE_CPDC_SGL,
						&svc_info->si_dst_sgl);
			goto out;
		}

		err = pc_res_sgl_packed_get(svc_info->si_pcr,
				&svc_info->si_dst_blist,
				CPDC_SGL_TUPLE_LEN_MAX,
				MPOOL_TYPE_CPDC_SGL,
				&svc_info->si_dst_sgl);
	}

out:
	return err;
}

pnso_error_t
cpdc_update_service_info_sgls(struct service_info *svc_info)
{
	pnso_error_t err;

	err = update_service_info_prepad_src_sgl(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain src sgl from pool! err: %d", err);
		goto out;
	}

	err = cpdc_update_service_info_dst_sgl(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dst sgl from pool! err: %d", err);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);
	}
out:
	return err;
}

uint32_t
cpdc_sgl_total_len_get(const struct service_cpdc_sgl *svc_sgl)
{
	struct cpdc_sgl *sgl;
	uint32_t        total_len = 0;

	sgl = svc_sgl->sgl;
	while (sgl) {
		total_len += sgl->cs_len_0 + sgl->cs_len_1 + sgl->cs_len_2;

		/*
		 * Follow swlink based on the HW link as P4+ might have
		 * truncated the list as a result of pad processing.
		 */
		if (!sgl->cs_next)
			break;
		CPDC_SGL_SWLINK_GET(sgl, sgl);
	}

	return total_len;
}

pnso_error_t
cpdc_convert_desc_error(int error)
{
	switch (error) {
	case CP_STATUS_SUCCESS:
		return PNSO_OK;
	case CP_STATUS_AXI_TIMEOUT:
		return PNSO_ERR_CPDC_AXI_TIMEOUT;
	case CP_STATUS_AXI_DATA_ERROR:
		return PNSO_ERR_CPDC_AXI_DATA_ERROR;
	case CP_STATUS_AXI_ADDR_ERROR:
		return PNSO_ERR_CPDC_AXI_ADDR_ERROR;
	case CP_STATUS_COMPRESSION_FAILED:
		return PNSO_ERR_CPDC_COMPRESSION_FAILED;
	case CP_STATUS_DATA_TOO_LONG:
		return PNSO_ERR_CPDC_DATA_TOO_LONG;
	case CP_STATUS_CHECKSUM_FAILED:
		return PNSO_ERR_CPDC_CHECKSUM_FAILED;
	case CP_STATUS_SGL_DESC_ERROR:
		return PNSO_ERR_CPDC_SGL_DESC_ERROR;
	/*
	 * TODO-cpdc: handle other errors
	 *	return PNSO_ERR_CPDC_HDR_IDX_INVALID
	 *	return PNSO_ERR_CPDC_ALGO_INVALID
	 *
	 */
	default:
		OSAL_ASSERT(0);	/* unreachable code */
		return EOPNOTSUPP;
	}
}

static void
update_batch_tags(struct service_info *svc_info, uint32_t num_tags)
{
	struct batch_info *batch_info;
	struct batch_page *batch_page;
	struct service_chain *chain;
	struct service_batch_info *svc_batch_info;

	chain = svc_info->si_centry->ce_chain_head;
	svc_batch_info = &svc_info->si_batch_info;

	batch_info = chain->sc_batch_info;
	batch_page = batch_info->bi_pages[svc_batch_info->sbi_bulk_desc_idx];

	OSAL_LOG_DEBUG("batch_info: 0x" PRIx64 " batch_page: 0x" PRIx64 " desc_idx: %u bulk_desc: %u",
			(uint64_t) batch_info, (uint64_t) batch_page,
			svc_batch_info->sbi_desc_idx,
			svc_batch_info->sbi_bulk_desc_idx);

	if (svc_info->si_type == PNSO_SVC_TYPE_HASH)
		batch_page->bp_tags.bpt_num_hashes += num_tags;
	else
		batch_page->bp_tags.bpt_num_chksums += num_tags;

	OSAL_LOG_DEBUG("svc_type: %d desc_idx: %u bpt_num_hashes: %u bpt_num_chksums: %u num_tags: %u",
			svc_info->si_type, svc_batch_info->sbi_desc_idx,
			batch_page->bp_tags.bpt_num_hashes,
			batch_page->bp_tags.bpt_num_chksums, num_tags);
}

void
cpdc_update_batch_tags(struct service_info *svc_info, uint32_t num_tags)
{
	if (!putil_is_bulk_desc_in_use(svc_info->si_flags))
		return;

	if (svc_info->si_type == PNSO_SVC_TYPE_HASH) {
		if (svc_info->si_desc_flags & PNSO_HASH_DFLAG_PER_BLOCK)
			update_batch_tags(svc_info, num_tags);
		return;
	}

	if (svc_info->si_type == PNSO_SVC_TYPE_CHKSUM) {
		if (svc_info->si_desc_flags & PNSO_CHKSUM_DFLAG_PER_BLOCK)
			update_batch_tags(svc_info, num_tags);
		return;
	}

	OSAL_ASSERT(0);
}

void
cpdc_update_seq_batch_size(const struct service_info *svc_info)
{
	struct batch_info *batch_info;
	struct batch_page *batch_page;
	struct service_chain *chain;
	struct service_batch_info *svc_batch_info;

	struct sequencer_desc *seq_desc;
	struct sequencer_info *seq_info;

	if (!putil_is_bulk_desc_in_use(svc_info->si_flags))
		return;

	OSAL_ASSERT((svc_info->si_type == PNSO_SVC_TYPE_HASH) ||
			(svc_info->si_type == PNSO_SVC_TYPE_CHKSUM));

	if (!((svc_info->si_desc_flags & PNSO_HASH_DFLAG_PER_BLOCK) ||
		(svc_info->si_desc_flags & PNSO_CHKSUM_DFLAG_PER_BLOCK)))
		return;

	seq_info = (struct sequencer_info *) &svc_info->si_seq_info;
	seq_desc = seq_info->sqi_desc;

	if (seq_desc->sd_batch_mode) {
		svc_batch_info =
			(struct service_batch_info *) &svc_info->si_batch_info;
		chain = svc_info->si_centry->ce_chain_head;
		batch_info = chain->sc_batch_info;
		batch_page =
			batch_info->bi_pages[svc_batch_info->sbi_bulk_desc_idx];

		if (svc_info->si_type == PNSO_SVC_TYPE_HASH) {
			seq_desc->sd_batch_size =
				cpu_to_be16(batch_page->bp_tags.bpt_num_hashes);

			seq_info->sqi_batch_size =
				batch_page->bp_tags.bpt_num_hashes;
		} else {
			seq_desc->sd_batch_size = cpu_to_be16(
					batch_page->bp_tags.bpt_num_chksums);

			seq_info->sqi_batch_size =
				batch_page->bp_tags.bpt_num_chksums;
		}

		OSAL_LOG_DEBUG("svc_type: %d bpt_num_hashes: %u bpt_num_chksums: %u num_tags: %u",
			svc_info->si_type,
			batch_page->bp_tags.bpt_num_hashes,
			batch_page->bp_tags.bpt_num_chksums,
			seq_info->sqi_batch_size);
	}
}

void
cpdc_update_bof_interrupt_params(struct service_info *svc_info)
{
	struct cpdc_desc *desc, *bof_desc;
	uint32_t bof_index;

	if (!(svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL))
		return;

	OSAL_ASSERT((svc_info->si_type == PNSO_SVC_TYPE_HASH) ||
		(svc_info->si_type == PNSO_SVC_TYPE_CHKSUM));

	desc = svc_info->si_desc;
	if (!desc)
		return;

	bof_index = (svc_info->si_num_tags * 2) - 1;
	bof_desc = &desc[bof_index];
	OSAL_LOG_DEBUG("update hash/chksum bof/desc. desc: 0x" PRIx64 " bof_desc: 0x" PRIx64 " bof_index: %d num_tags: %d",
			(uint64_t) desc, (uint64_t) bof_desc,
			bof_index, svc_info->si_num_tags);

	bof_desc->u.cd_bits.cc_otag_on = desc->u.cd_bits.cc_otag_on;
	bof_desc->u.cd_bits.cc_db_on = desc->u.cd_bits.cc_db_on;

	bof_desc->cd_db_addr = desc->cd_db_addr;
	bof_desc->cd_db_data = desc->cd_db_data;
	bof_desc->cd_otag_addr = desc->cd_otag_addr;
	bof_desc->cd_otag_data = desc->cd_otag_data;

	// CPDC_PPRINT_DESC(desc);
	// CPDC_PPRINT_DESC(bof_desc);
}

pnso_error_t
cpdc_setup_interrupt_params(struct service_info *svc_info, void *poll_ctx)
{
	struct cpdc_chain_params *cpdc_chain;
	pnso_error_t err = PNSO_OK;
	struct cpdc_desc *cp_desc;
	struct per_core_resource *pcr;
	struct service_chain *chain;
	uint64_t db_addr;

	chain = svc_info->si_centry->ce_chain_head;

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	pcr = svc_info->si_pcr;

	OSAL_LOG_DEBUG("cp_desc: 0x" PRIx64 " pcr: 0x" PRIx64 " poll_ctx: 0x" PRIx64,
			(uint64_t) cp_desc, (uint64_t) pcr,
			(uint64_t) poll_ctx);

	/*
	 * When a chain successor is present, that service will take care of
	 * setting up the interrupt.
	 */
	if (chn_service_has_sub_chain(svc_info)) {
		OSAL_LOG_DEBUG("subordinate service will set up interrupt");
		goto out;
	}

	cp_desc->u.cd_bits.cc_otag_on = 1;

	if (chn_service_is_cp_padding_applic(svc_info)) {
		chain->sc_async_evid = sonic_intr_get_ev_id(pcr,
							(uint64_t) poll_ctx,
							&db_addr);
		if (!chain->sc_async_evid) {
			err = EINVAL;
			OSAL_LOG_DEBUG("cp/pad failed to get async event err: %d",
					err);
			goto out;
		}
		cp_desc->cd_otag_addr = db_addr;
		cp_desc->cd_otag_data = sonic_intr_get_fire_data32();

		cpdc_chain = &svc_info->si_cpdc_chain;
		cpdc_chain->ccp_intr_addr =
			sonic_get_per_core_intr_assert_addr(pcr);
		cpdc_chain->ccp_intr_data = sonic_get_intr_assert_data();
		cpdc_chain->ccp_cmd.ccpc_intr_en = 1;
		err = seq_setup_cpdc_chain_status_desc(svc_info);
		if (err != PNSO_OK)
			OSAL_LOG_DEBUG("failed setup chain status desc: err %d",
					err);
	} else {
		chain->sc_async_evid = sonic_intr_get_ev_id(pcr,
						(uint64_t) poll_ctx,
						&db_addr);
		if (!chain->sc_async_evid) {
			err = EINVAL;
			OSAL_LOG_DEBUG("failed to get db addr err: %d", err);
			goto out;
		}

		cp_desc->cd_db_addr = db_addr;
		cp_desc->u.cd_bits.cc_db_on = 1;
		cp_desc->cd_db_data = sonic_intr_get_fire_data64();

		cp_desc->cd_otag_addr =
			sonic_get_per_core_intr_assert_addr(pcr);
		cp_desc->cd_otag_data = sonic_get_intr_assert_data();
	}

out:
	return  err;
}

void
cpdc_report_mpools(struct per_core_resource *pcr)
{
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_SGL]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_BATCH_PAGE]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_BATCH_INFO]);
       MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC]);
}

/* TODO-cpdc:
 *	code is repeated due to slight changes between crypto/cpdc,
 *	collapse/refactor as/if needed.
 *
 */
pnso_error_t
cpdc_setup_rmem_dst_blist(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct service_buf_list orig_dst_blist;

	/*
	 * Produce output to intermediate buffers if there is a chain
	 * subordinate
	 *
	 */
	if (chn_service_has_sub_chain(svc_info)) {
		orig_dst_blist = svc_info->si_dst_blist;

		err = putil_get_interm_buf_list(svc_info);
		if (err) {
			OSAL_LOG_DEBUG("intermediate buffers not available, "
					"using supplied host buffers (if any)");
			PAS_INC_NUM_OUT_OF_RMEM_BUFS(svc_info->si_pcr);
			err = PNSO_OK;
		}

		if (chn_service_has_interm_blist(svc_info) &&
				orig_dst_blist.len) {
			svc_info->si_sgl_pdma =
				pc_res_sgl_pdma_packed_get(svc_info->si_pcr,
						&orig_dst_blist);
			if (!svc_info->si_sgl_pdma) {
				OSAL_LOG_DEBUG("SGL PDMA constraints exceeded, "
						"using original buffers");
				PAS_INC_NUM_PDMA_EXCEED_CONSTRAINTS(svc_info->si_pcr);
				putil_put_interm_buf_list(svc_info);
				svc_info->si_dst_blist = orig_dst_blist;
			}
		}
	}

	/*
	 * Validate a destination buffer list exists (it could come from app or
	 * intermediate pool as evaluated above and elsewhere).
	 *
	 */
	OSAL_LOG_DEBUG("src_total_len %u dst_total_len %u",
		       svc_info->si_src_blist.len, svc_info->si_dst_blist.len);

	if ((svc_info->si_src_blist.len == 0) ||
	    (svc_info->si_dst_blist.len == 0)) {
		err = EINVAL;
		OSAL_LOG_ERROR("length error: src_len %u dst_len %u err: %d",
				svc_info->si_src_blist.len,
				svc_info->si_dst_blist.len, err);
		return err;
	}

	return PNSO_OK;
}

void
cpdc_teardown_rmem_dst_blist(struct service_info *svc_info)
{
	putil_put_interm_buf_list(svc_info);
	pc_res_sgl_pdma_put(svc_info->si_pcr, svc_info->si_sgl_pdma);
}

void
cpdc_update_tags(struct service_info *svc_info)
{
	uint32_t orig_num_tags;

	if (chn_service_deps_data_len_set_from_parent(svc_info)) {
		/*
		 * In debug mode, verify the padding adjustment in the dst SGL.
		 *
		 * CAUTION: it can be costly to invoke cpdc_sgl_total_len_get()
		 * so do not call it outside of the DEBUG macro.
		 */
		OSAL_LOG_DEBUG("my data_len %u parent data_len %u",
				cpdc_sgl_total_len_get(&svc_info->si_src_sgl),
				chn_service_deps_data_len_get(svc_info));
	}

	orig_num_tags = svc_info->si_num_tags;
	if ((svc_info->si_type == PNSO_SVC_TYPE_HASH &&
		svc_info->si_desc_flags & PNSO_HASH_DFLAG_PER_BLOCK) ||
		(svc_info->si_type == PNSO_SVC_TYPE_CHKSUM &&
		 svc_info->si_desc_flags & PNSO_CHKSUM_DFLAG_PER_BLOCK)) {
		svc_info->si_num_tags = chn_service_deps_num_blks_get(svc_info);
		svc_info->si_num_bytes =
			chn_service_deps_data_len_get(svc_info);

		OSAL_ASSERT(svc_info->si_num_tags >= 1);
	} else
		OSAL_ASSERT(svc_info->si_num_tags == 1);

	OSAL_LOG_INFO("block_size: %d new num_tags: %d old num_tags: %d",
			svc_info->si_block_size,
			svc_info->si_num_tags, orig_num_tags);

	svc_info->tags_updated = true;
}

pnso_error_t
cpdc_poll_all(struct service_info *svc_info)
{
	pnso_error_t err = PNSO_OK;
	struct cpdc_status_desc *st_desc;
	uint32_t obj_size, i;

	OSAL_LOG_DEBUG("enter ...");

	err = cpdc_poll(svc_info, NULL);
	if (err)
		goto out;

	st_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
	if (!st_desc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid base status desc! err: %d", err);
		goto out;
	}

	if (!svc_info->tags_updated)
		cpdc_update_tags(svc_info);

	obj_size = cpdc_get_status_desc_size();
	for (i = 0; i < svc_info->si_num_tags; i++) {
		err = cpdc_poll(svc_info, st_desc);
		if (err) {
			OSAL_LOG_DEBUG("valid bit not set! num_tags: %d tag: %d st_desc: 0x" PRIx64 " err: %d",
					svc_info->si_num_tags, i,
					(uint64_t) st_desc, err);
			goto out;
		}

		st_desc = cpdc_get_next_status_desc(st_desc, obj_size);
	}

	OSAL_LOG_DEBUG("exit! all desc(s) poll success! num_tags: %d",
			svc_info->si_num_tags);
out:
	return err;
}

static void
pprint_suspect_sgl(uint64_t sgl_pa)
{
	const struct cpdc_sgl *sgl;

	if (!sgl_pa)
		return;

	sgl = (const struct cpdc_sgl *) sonic_phy_to_virt(sgl_pa);
	while (sgl) {
		OSAL_LOG_NOTICE("%30s: 0x" PRIx64 " ==> 0x" PRIx64,
			"",
			(uint64_t) sgl, sgl_pa);

		OSAL_LOG_NOTICE("%30s: 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d",
				"",
				sgl->cs_addr_0, sgl->cs_len_0, sgl->cs_rsvd_0,
				sgl->cs_addr_1, sgl->cs_len_1, sgl->cs_rsvd_1,
				sgl->cs_addr_2, sgl->cs_len_2, sgl->cs_rsvd_2);
		OSAL_LOG_NOTICE("%30s: 0x" PRIx64 "/0x" PRIx64,
				"",
				sgl->cs_next, sgl->cs_rsvd_swlink);

		sgl_pa = sgl->cs_next;

		CPDC_SGL_SWLINK_GET(sgl, sgl);
		if (sgl)
			sgl_pa = sonic_virt_to_phy((void *) sgl);
	}
}

static void
pprint_suspect_sgl_ex(uint64_t sgl_pa)
{
	const struct cpdc_sgl *sgl;
	uint64_t hostpa;

	if (!sgl_pa)
		return;

	sgl = (const struct cpdc_sgl *) sonic_phy_to_virt(sgl_pa);
	while (sgl) {
		hostpa = sonic_devpa_to_hostpa(sgl->cs_addr_0);
		OSAL_LOG_SUSPECT("%30s: %s: 0x" PRIx64 " %s: %d",
				"",
				"buffer", hostpa,
				"length", sgl->cs_len_0);

		hostpa = sonic_devpa_to_hostpa(sgl->cs_addr_1);
		if (hostpa)
			OSAL_LOG_SUSPECT("%30s: %s: 0x" PRIx64 " %s: %d",
					"",
					"buffer", hostpa,
					"length", sgl->cs_len_1);

		hostpa = sonic_devpa_to_hostpa(sgl->cs_addr_2);
		if (hostpa)
			OSAL_LOG_SUSPECT("%30s: %s: 0x" PRIx64 " %s: %d",
					"",
					"buffer", hostpa,
					"length", sgl->cs_len_2);

		sgl = (const struct cpdc_sgl *) sgl->cs_next;
		if (sgl)
			sgl = (const struct cpdc_sgl *)
				sonic_phy_to_virt((uint64_t) sgl);
	}
}

void
cpdc_report_suspect_desc(const struct cpdc_desc *desc)
{
	struct cpdc_cmd *cmd;

	if (!desc)
		return;

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cpdc descriptor", (uint64_t) desc);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_src", desc->cd_src);
	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_dst", desc->cd_dst);

	OSAL_LOG_NOTICE("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_NOTICE("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_NOTICE("%30s: %d", "cd_threshold_len",
			desc->cd_threshold_len);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_status_addr",
			desc->cd_status_addr);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_db_data", desc->cd_db_data);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "cd_otag_addr",
			desc->cd_otag_addr);
	OSAL_LOG_NOTICE("%30s: %d", "cd_otag_data", desc->cd_otag_data);
	OSAL_LOG_NOTICE("%30s: %d", "cd_status_data", desc->cd_status_data);

	OSAL_LOG_NOTICE("%30s:", "=== cpdc_cmd");
	cmd = (struct cpdc_cmd *) &desc->u.cd_bits;
	OSAL_LOG_NOTICE("%30s: %d", "cc_enabled", cmd->cc_enabled);

	OSAL_LOG_NOTICE("%30s: %d", "cc_header_present",
			cmd->cc_header_present);
	OSAL_LOG_NOTICE("%30s: %d", "cc_insert_header", cmd->cc_insert_header);

	OSAL_LOG_NOTICE("%30s: %d", "cc_db_on", cmd->cc_db_on);
	OSAL_LOG_NOTICE("%30s: %d", "cc_otag_on", cmd->cc_otag_on);

	OSAL_LOG_NOTICE("%30s: %d", "cc_src_is_list", cmd->cc_src_is_list);
	OSAL_LOG_NOTICE("%30s: %d", "cc_dst_is_list", cmd->cc_dst_is_list);

	OSAL_LOG_NOTICE("%30s: %d", "cc_chksum_verify_enabled",
			cmd->cc_chksum_verify_enabled);
	OSAL_LOG_NOTICE("%30s: %d", "cc_chksum_adler", cmd->cc_chksum_adler);

	OSAL_LOG_NOTICE("%30s: %d", "cc_hash_enabled", cmd->cc_hash_enabled);
	OSAL_LOG_NOTICE("%30s: %d", "cc_hash_type", cmd->cc_hash_type);

	OSAL_LOG_NOTICE("%30s: %d", "cc_integrity_src", cmd->cc_integrity_src);
	OSAL_LOG_NOTICE("%30s: %d", "cc_integrity_type",
			cmd->cc_integrity_type);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "src buffer list",
			desc->cd_src);
	pprint_suspect_sgl(desc->cd_src);

	OSAL_LOG_NOTICE("%30s: 0x" PRIx64, "dst buffer list",
			desc->cd_dst);
	pprint_suspect_sgl(desc->cd_dst);
}

void
cpdc_report_suspect_desc_ex(const struct cpdc_desc *desc)
{
	uint64_t hostpa;

	if (!desc)
		return;

	OSAL_LOG_SUSPECT("%30s: %d", "length", desc->cd_datain_len);
	OSAL_LOG_SUSPECT("%30s: %d", "threshold length", desc->cd_threshold_len);

	OSAL_LOG_SUSPECT("%30s: ", "source buffer list");
	if (desc->u.cd_bits.cc_src_is_list)
		pprint_suspect_sgl_ex(desc->cd_src);
	else {
		hostpa = sonic_devpa_to_hostpa(desc->cd_src);
		if (hostpa)
			OSAL_LOG_SUSPECT("%30s: %s: 0x" PRIx64 " %s: %d",
				"",
				"source buffer", hostpa,
				"length", desc->cd_datain_len);
	}

	OSAL_LOG_SUSPECT("%30s: ", "destination buffer list");
	if (desc->u.cd_bits.cc_dst_is_list)
		pprint_suspect_sgl_ex(desc->cd_dst);
	else {
		hostpa = sonic_devpa_to_hostpa(desc->cd_dst);
		if (hostpa)
			OSAL_LOG_SUSPECT("%30s: %s: 0x" PRIx64 " %s: %d",
				"",
				"destination buffer", hostpa,
				"length", desc->cd_threshold_len);
	}
}
