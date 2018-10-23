/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>
#include <net.h>
#include <kernel.h>

#include "osal.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "pnso_api.h"

#include "sonic_api_int.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_crypto_cmn.h"
#include "pnso_cpdc.h"
#include "pnso_seq.h"
#include "pnso_utils.h"
#include "sonic_api_int.h"

static enum crypto_algo_cmd_hi   crypto_algo_cmd_hi_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_HI_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_HI_AES_GCM,
};

static enum crypto_algo_cmd_lo   crypto_algo_cmd_lo_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_LO_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_LO_AES_GCM,
};

static pnso_error_t
crypto_validate_input(struct service_info *svc_info,
		      const struct service_params *svc_params)
{
	struct pnso_crypto_desc	*pnso_crypto_desc;

	if (!svc_info || !svc_params) {
		OSAL_LOG_ERROR("null svc_info or svc_params specified");
		return EINVAL;
	}

	if (!svc_info->si_src_blist.blist) {
		OSAL_LOG_ERROR("null si_src_blist specified");
		return EINVAL;
	}

	pnso_crypto_desc = svc_params->u.sp_crypto_desc;
	if (!pnso_crypto_desc) {
		OSAL_LOG_ERROR("null crypto_desc specified");
		return EINVAL;
	}

	if ((pnso_crypto_desc->algo_type == PNSO_CRYPTO_TYPE_NONE) ||
	    (pnso_crypto_desc->algo_type >= PNSO_CRYPTO_TYPE_MAX)) {
		OSAL_LOG_ERROR("invalid algo_type %u specified!",
				pnso_crypto_desc->algo_type);
		return EINVAL;
	}

	if (!pnso_crypto_desc->iv_addr) {
		OSAL_LOG_ERROR("null iv_addr specified");
		return EINVAL;
	}

	return PNSO_OK;
}

static inline void
crypto_desc_fill(struct service_info *svc_info,
		 struct pnso_crypto_desc *pnso_crypto_desc)
{
	struct crypto_desc *crypto_desc = svc_info->si_desc;
	struct crypto_status_desc *status_desc = svc_info->si_status_desc;

	/*
	 * Intermediate status is never directy "polled" so no need to clear it.
	 */
	memset(crypto_desc, 0, sizeof(*crypto_desc));
	memset(status_desc, 0, sizeof(*status_desc));
	if (svc_info->si_istatus_desc)
		crypto_desc->cd_status_addr =
		    mpool_get_object_phy_addr(MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS,
					      svc_info->si_istatus_desc);
	else
		crypto_desc->cd_status_addr = sonic_virt_to_phy(status_desc);

	crypto_desc->cd_in_aol = sonic_virt_to_phy(svc_info->si_src_aol.aol);
	crypto_desc->cd_out_aol = sonic_virt_to_phy(svc_info->si_dst_aol.aol);

	OSAL_ASSERT(pnso_crypto_desc->algo_type < PNSO_CRYPTO_TYPE_MAX);
	crypto_desc->cd_db_data = CRYPTO_ENCRYPT_CPL_DATA;
	if (svc_info->si_type == PNSO_SVC_TYPE_DECRYPT) {
		crypto_desc->cd_cmd.cc_is_decrypt = true;
		crypto_desc->cd_db_data = CRYPTO_DECRYPT_CPL_DATA;
	}
	crypto_desc->cd_cmd.cc_token_3 =
			    crypto_algo_cmd_lo_tbl[pnso_crypto_desc->algo_type];
	crypto_desc->cd_cmd.cc_token_4 =
			    crypto_algo_cmd_hi_tbl[pnso_crypto_desc->algo_type];
	crypto_desc->cd_key_desc_idx =
		     sonic_get_crypto_key_idx(pnso_crypto_desc->key_desc_idx);

	crypto_desc->cd_iv_addr = sonic_hostpa_to_devpa(pnso_crypto_desc->iv_addr);
	crypto_desc->cd_db_addr = crypto_desc->cd_status_addr +
				  sizeof(status_desc->csd_err);

	CRYPTO_PPRINT_DESC(crypto_desc);
}

static inline pnso_error_t
crypto_dst_blist_setup(struct service_info *svc_info,
		       const struct service_params *svc_params)
{
	const struct per_core_resource	*pcr = svc_info->si_pcr;
	struct service_buf_list		orig_dst_blist;
	pnso_error_t			err;

	/*
	 * Produce output to intermediate buffers if there is a chain subordinate.
	 */
	if (chn_service_has_sub_chain(svc_info)) {
		err = svc_interm_buf_list_get(svc_info);
		if (err) {
			OSAL_LOG_ERROR("failed to obtain intermediate buffers");
			return err;
		}

		svc_info->si_istatus_desc =
		    pc_res_mpool_object_get(pcr,
					    MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS);
		if (!svc_info->si_istatus_desc) {
			OSAL_LOG_ERROR("failed to obtain intermediate status_desc");
			return ENOMEM;
		}

		if (chn_service_has_interm_blist(svc_info) &&
		    svc_params->sp_dst_blist) {

			orig_dst_blist.type = SERVICE_BUF_LIST_TYPE_DFLT;
			orig_dst_blist.len = svc_info->si_dst_blist.len;
			orig_dst_blist.blist = svc_params->sp_dst_blist;
			svc_info->si_sgl_pdma =
				pc_res_sgl_pdma_packed_get(pcr, &orig_dst_blist);
			if (!svc_info->si_sgl_pdma) {
				OSAL_LOG_ERROR("failed to obtain chain SGL for PDMA");
				return ENOMEM;
			}
		}
	}

	/*
	 * Validate a destination buffer list exists (it could come from app or
	 * intermediate pool as evaluated above and elsewhere).
	 */
	OSAL_LOG_DEBUG("src_total_len %u dst_total_len %u",
		       svc_info->si_src_blist.len, svc_info->si_dst_blist.len);
	if ((svc_info->si_src_blist.len == 0) ||
	    (svc_info->si_dst_blist.len < svc_info->si_src_blist.len)) {
		OSAL_LOG_ERROR("length error: src_len %u dst_len %u",
				svc_info->si_src_blist.len, svc_info->si_dst_blist.len);
		return EINVAL;
	}
	return PNSO_OK;
}

static inline pnso_error_t
crypto_src_dst_aol_fill(struct service_info *svc_info)
{
	const struct per_core_resource	*pcr = svc_info->si_pcr;
	pnso_error_t			src_err;
	pnso_error_t			dst_err;

	/*
	 * 1) First in chain or the only service: enter the src/dst buf into AOLs
	 *    in packed format (i.e. more efficient usage of the AOLs).
	 * 2) Part of a chain (and is not first): use a sparse vector of AOLs
	 *    for the src/dst buf info to facilitate sequencer padding.
	 */
	if (!chn_service_is_in_chain(svc_info) || chn_service_is_first(svc_info)) {
		src_err = crypto_aol_packed_get(pcr, &svc_info->si_src_blist,
						&svc_info->si_src_aol);
		dst_err = crypto_aol_packed_get(pcr, &svc_info->si_dst_blist,
						&svc_info->si_dst_aol);
	} else {
		src_err = crypto_aol_vec_sparse_get(pcr, svc_info->si_block_size,
				&svc_info->si_src_blist, &svc_info->si_src_aol);
		dst_err = crypto_aol_vec_sparse_get(pcr, svc_info->si_block_size,
				&svc_info->si_dst_blist, &svc_info->si_dst_aol);
	}

	if (src_err) {
		OSAL_LOG_ERROR("failed to build crypto src_aol");
		return src_err;
	}
	if (dst_err) {
		OSAL_LOG_ERROR("failed to build crypto dst_aol");
		return dst_err;
	}

	return PNSO_OK;
}

static pnso_error_t
crypto_setup(struct service_info *svc_info,
	     const struct service_params *svc_params)
{
	const struct per_core_resource	*pcr;
	pnso_error_t			err;

	err = crypto_validate_input(svc_info, svc_params);
	if (err)
		return err;

	svc_info->si_desc = crypto_get_desc(svc_info, false);
	if (!svc_info->si_desc)
		return ENOMEM;

	pcr = svc_info->si_pcr;
	svc_info->si_status_desc = pc_res_mpool_object_get(pcr,
					  MPOOL_TYPE_CRYPTO_STATUS_DESC);
	if (!svc_info->si_status_desc)
		return ENOMEM;

	err = crypto_dst_blist_setup(svc_info, svc_params);
	if (err)
		return err;

	err = crypto_src_dst_aol_fill(svc_info);
	if (err)
		return err;

	svc_info->si_desc_flags = 0;
	crypto_desc_fill(svc_info, svc_params->u.sp_crypto_desc);

	if (putil_is_service_in_batch(svc_info->si_flags)) {
		err = crypto_setup_batch_desc(svc_info, svc_info->si_desc);
		if (err) {
			OSAL_LOG_ERROR("failed to setup batch sequencer desc! err: %d",
					err);
			return err;
		}
	} else {
		if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
				(svc_info->si_flags &
				 CHAIN_SFLAG_FIRST_SERVICE)) {
			svc_info->si_seq_info.sqi_desc =
				seq_setup_desc(svc_info, svc_info->si_desc,
						sizeof(struct crypto_desc));
			if (!svc_info->si_seq_info.sqi_desc) {
				err = EINVAL;
				OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d",
						err);
				return err;
			}
		}
	}

	return PNSO_OK;
}

static pnso_error_t
crypto_encrypt_setup(struct service_info *svc_info,
		     const struct service_params *svc_params)
{
	svc_info->si_type = PNSO_SVC_TYPE_ENCRYPT;
	return crypto_setup(svc_info, svc_params);
}

static pnso_error_t
crypto_decrypt_setup(struct service_info *svc_info,
		     const struct service_params *svc_params)
{
	svc_info->si_type = PNSO_SVC_TYPE_DECRYPT;
	return crypto_setup(svc_info, svc_params);
}

static pnso_error_t
crypto_chain(struct chain_entry *centry)
{
	struct service_info		*svc_info = &centry->ce_svc_info;
	struct crypto_chain_params	*crypto_chain = &svc_info->si_crypto_chain;
	struct interm_buf_list		*iblist;
	struct service_info		*svc_next;
	pnso_error_t			err = PNSO_OK;

	if (chn_service_has_sub_chain(svc_info)) {
		if (svc_info->si_sgl_pdma) {
			OSAL_ASSERT(chn_service_has_interm_blist(svc_info));
			iblist = &svc_info->si_iblist;
			crypto_chain->ccp_crypto_buf_addr = iblist->blist.buffers[0].buf;
			crypto_chain->ccp_data_len = iblist->blist.buffers[0].len;
			crypto_chain->ccp_sgl_pdma_dst_addr =
				sonic_virt_to_phy(svc_info->si_sgl_pdma);
			crypto_chain->ccp_cmd.ccpc_sgl_pdma_en = true;
			crypto_chain->ccp_cmd.ccpc_sgl_pdma_len_from_desc = true;
			SGL_PDMA_PPRINT(crypto_chain->ccp_sgl_pdma_dst_addr);
		}
		crypto_chain->ccp_status_addr_0 =
		  mpool_get_object_phy_addr(MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS,
					    svc_info->si_istatus_desc);
		crypto_chain->ccp_status_addr_1 =
			sonic_virt_to_phy(svc_info->si_status_desc);
		crypto_chain->ccp_status_len = sizeof(struct crypto_status_desc);
		crypto_chain->ccp_cmd.ccpc_status_dma_en = true;
		crypto_chain->ccp_cmd.ccpc_stop_chain_on_error = true;

		if (svc_info->si_type == PNSO_SVC_TYPE_DECRYPT) {
			err = pc_res_sgl_vec_packed_get(svc_info->si_pcr,
				    &svc_info->si_dst_blist, svc_info->si_block_size,
				    MPOOL_TYPE_CRYPTO_SGL_VECTOR, &svc_info->si_src_sgl);
			if (err) {
				OSAL_LOG_ERROR("failed to setup chain decrypt SGL");
				return err;
			}

			crypto_chain->ccp_cmd.ccpc_comp_len_update_en = true;
			crypto_chain->ccp_cmd.ccpc_comp_sgl_src_en = true;
			crypto_chain->ccp_cmd.ccpc_comp_sgl_src_vec_en = true;
			crypto_chain->ccp_blk_boundary_shift =
				(uint8_t)ilog2(svc_info->si_block_size);
			crypto_chain->ccp_comp_sgl_src_addr =
				sonic_virt_to_phy(svc_info->si_src_sgl.sgl);
		}


		OSAL_ASSERT(centry->ce_next);
		svc_next = &centry->ce_next->ce_svc_info;
		err = svc_next->si_ops.sub_chain_from_crypto(svc_next, crypto_chain);
		if (!err)
			err = seq_setup_crypto_chain(svc_info, svc_info->si_desc);
	}

	if (err)
		OSAL_LOG_ERROR("failed seq_setup_crypto_chain: err %d", err);

	return err;
}

static pnso_error_t
crypto_sub_chain_from_cpdc(struct service_info *svc_info,
			   struct cpdc_chain_params *cpdc_chain)
{
	cpdc_chain->ccp_aol_src_vec_addr = sonic_virt_to_phy(svc_info->si_src_aol.aol);
	cpdc_chain->ccp_aol_dst_vec_addr = sonic_virt_to_phy(svc_info->si_dst_aol.aol);
	cpdc_chain->ccp_cmd.ccpc_aol_pad_en = !!cpdc_chain->ccp_pad_buf_addr;
	cpdc_chain->ccp_cmd.ccpc_sgl_pad_en = cpdc_chain->ccp_cmd.ccpc_sgl_pdma_pad_only;
	cpdc_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	cpdc_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring_id,
				   &cpdc_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
crypto_sub_chain_from_crypto(struct service_info *svc_info,
			     struct crypto_chain_params *crypto_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 * For testing purposes, it is possible to chain encrypt to decrypt.
	 */
	crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	crypto_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring_id,
				   &crypto_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
crypto_schedule(const struct service_info *svc_info)
{
	if (!chn_service_is_in_chain(svc_info) || chn_service_is_first(svc_info)) {
		seq_ring_db(svc_info);
		return PNSO_OK;
	}
	return EINVAL;
}

static pnso_error_t
crypto_poll(const struct service_info *svc_info)
{
	pnso_error_t err;
	volatile struct crypto_status_desc *status_desc;
	uint64_t cpl_data;
	uint32_t attempt = 0;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = svc_info->si_status_desc;
	OSAL_ASSERT(status_desc);

	while (attempt < 16) {
		cpl_data = svc_info->si_type == PNSO_SVC_TYPE_DECRYPT ?
			CRYPTO_DECRYPT_CPL_DATA : CRYPTO_ENCRYPT_CPL_DATA;

		err = (status_desc->csd_cpl_data == cpl_data) ? PNSO_OK : EBUSY;
		if (!err)
			break;

		attempt++;
		OSAL_LOG_DEBUG("attempt: %d svc_type: %d status_desc: 0x%llx",
				attempt, svc_info->si_type,
				(uint64_t) status_desc);
		osal_yield();
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

static pnso_error_t
crypto_read_status(const struct service_info *svc_info)
{
	struct crypto_status_desc *status_desc;
	pnso_error_t err;

	status_desc = svc_info->si_status_desc;
	err = status_desc->csd_err;
	if (err) {
		OSAL_LOG_ERROR("hw error reported: %d", err);
	}
	return err;
}

static pnso_error_t
crypto_write_result(struct service_info *svc_info)
{
	struct pnso_service_status *svc_status;
	struct crypto_status_desc *status_desc;

	svc_status = svc_info->si_svc_status;
	status_desc = svc_info->si_status_desc;
	if (status_desc->csd_err) {
		svc_status->err = crypto_desc_status_convert(status_desc->csd_err);
		OSAL_LOG_ERROR("service failed: %d", svc_status->err);
		return EINVAL;
	}

	return PNSO_OK;
}

static void
crypto_teardown(struct service_info *svc_info)
{
	struct per_core_resource *pcr = svc_info->si_pcr;

	/*
	 * Trace the desc/AOLs once more to verify any padding applied
	 * by sequencer.
	 */
	CRYPTO_PPRINT_DESC(svc_info->si_desc);
	SGL_PDMA_PPRINT(svc_info->si_crypto_chain.ccp_sgl_pdma_dst_addr);

	crypto_aol_put(pcr, &svc_info->si_src_aol);
	crypto_aol_put(pcr, &svc_info->si_dst_aol);
	pc_res_mpool_object_put(pcr, MPOOL_TYPE_CRYPTO_STATUS_DESC,
				svc_info->si_status_desc);

	crypto_put_desc(svc_info, false, svc_info->si_desc);

	svc_interm_buf_list_put(svc_info);
	pc_res_mpool_object_put(pcr, MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS,
				svc_info->si_istatus_desc);
	pc_res_sgl_pdma_put(pcr, svc_info->si_sgl_pdma);
	pc_res_sgl_vec_put(pcr, &svc_info->si_src_sgl);
	seq_cleanup_crypto_chain(svc_info);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops encrypt_ops = {
	.setup = crypto_encrypt_setup,
	.chain = crypto_chain,
	.sub_chain_from_cpdc = crypto_sub_chain_from_cpdc,
	.sub_chain_from_crypto = crypto_sub_chain_from_crypto,
	.schedule = crypto_schedule,
	.poll = crypto_poll,
	.read_status = crypto_read_status,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};

struct service_ops decrypt_ops = {
	.setup = crypto_decrypt_setup,
	.chain = crypto_chain,
	.sub_chain_from_cpdc = crypto_sub_chain_from_cpdc,
	.sub_chain_from_crypto = crypto_sub_chain_from_crypto,
	.schedule = crypto_schedule,
	.poll = crypto_poll,
	.read_status = crypto_read_status,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};
