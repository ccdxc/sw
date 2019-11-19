/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "pnso_api.h"

#include "sonic_api_int.h"

#include "pnso_pbuf.h"
#include "pnso_svc.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_crypto_cmn.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_seq.h"
#include "pnso_utils.h"
#include "sonic_api_int.h"

static enum crypto_algo_cmd_hi crypto_algo_cmd_hi_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_HI_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_HI_AES_GCM,
};

static enum crypto_algo_cmd_lo crypto_algo_cmd_lo_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_LO_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_LO_AES_GCM,
};

static pnso_error_t
crypto_validate_input(struct service_info *svc_info,
		      const struct service_params *svc_params)
{
	struct pnso_crypto_desc	*pnso_crypto_desc;
	uint32_t num_keys_max;

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

	if (!sonic_validate_crypto_key_idx(pnso_crypto_desc->key_desc_idx,
					   &num_keys_max)) {
		OSAL_LOG_ERROR("crypto key index %u exceeds max value %u",
			       pnso_crypto_desc->key_desc_idx, num_keys_max);
		return EINVAL;
	}

	return PNSO_OK;
}

static inline int
crypto_desc_fill(struct service_info *svc_info,
		 struct pnso_crypto_desc *pnso_crypto_desc)
{
	struct crypto_desc *crypto_desc = svc_info->si_desc;
	uint64_t aligned_addr;
	pnso_error_t err;

	memset(crypto_desc, 0, sizeof(*crypto_desc));
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

	crypto_desc->cd_iv_addr =
		sonic_hostpa_to_devpa(pnso_crypto_desc->iv_addr);

	/*
	 * when chaining or async mode is involved, cd_db_addr/cd_db_data
	 * may later be modified with other types of context data.
	 */
	err = svc_status_desc_addr_get(&svc_info->si_status_desc, 0,
			&aligned_addr, sizeof(struct crypto_status_desc));
	crypto_desc->cd_status_addr = aligned_addr;
	if (err)
		goto out;

	if (chn_service_has_interm_status(svc_info)) {
		err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, 0,
				&aligned_addr,
				sizeof(struct crypto_status_desc));
		crypto_desc->cd_status_addr = aligned_addr;
	}

	crypto_desc->cd_db_addr = crypto_desc->cd_status_addr +
		offsetof(struct crypto_status_desc, csd_cpl_data);

	CRYPTO_PPRINT_DESC(crypto_desc);
out:
	return err;
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
		orig_dst_blist = svc_info->si_dst_blist;
		err = putil_get_interm_buf_list(svc_info);
		if (err) {
			OSAL_LOG_DEBUG("intermediate buffers not available, "
					"using supplied host buffers (if any)");
			PAS_INC_NUM_OUT_OF_RMEM_BUFS(svc_info->si_pcr);
		}

		err = pc_res_svc_status_get(svc_info->si_pcr,
				MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS,
				&svc_info->si_istatus_desc);
		if (err) {
			OSAL_LOG_DEBUG("intermediate status not available, "
					"using host status");
			PAS_INC_NUM_OUT_OF_RMEM_STATUS(svc_info->si_pcr);
		}

		if (chn_service_has_interm_blist(svc_info) &&
				orig_dst_blist.len) {
			svc_info->si_sgl_pdma =
				pc_res_sgl_pdma_packed_get(pcr,
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
	 */
	OSAL_LOG_DEBUG("src_total_len %u dst_total_len %u",
		       svc_info->si_src_blist.len, svc_info->si_dst_blist.len);
	if ((svc_info->si_src_blist.len == 0) ||
	    (svc_info->si_dst_blist.len < svc_info->si_src_blist.len)) {
		OSAL_LOG_ERROR("length error: src_len %u dst_len %u",
				svc_info->si_src_blist.len,
				svc_info->si_dst_blist.len);
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
	if (chn_service_is_starter(svc_info)) {
		src_err = crypto_aol_packed_get(pcr, &svc_info->si_src_blist,
						svc_info->si_src_blist.len,
						&svc_info->si_src_aol);

		/*
		 * NOTE: earlier we had ensured dst list length is no less than
		 * src list length. Now we enforce HW requirement that both
		 * src and dst AOL must express the same total length.
		 */
		dst_err = crypto_aol_packed_get(pcr, &svc_info->si_dst_blist,
						svc_info->si_src_blist.len,
						&svc_info->si_dst_aol);
	} else {
		src_err = crypto_aol_vec_sparse_get(pcr,
				svc_info->si_block_size,
				&svc_info->si_src_blist,
				svc_info->si_src_blist.len,
				&svc_info->si_src_aol);
		/*
		 * Same NOTE as above.
		 */
		dst_err = crypto_aol_vec_sparse_get(pcr,
				svc_info->si_block_size,
				&svc_info->si_dst_blist,
				svc_info->si_src_blist.len,
				&svc_info->si_dst_aol);
	}

	if (src_err) {
		OSAL_LOG_DEBUG("failed to build crypto src_aol! err: %d",
				src_err);
		return src_err;
	}
	if (dst_err) {
		OSAL_LOG_DEBUG("failed to build crypto dst_aol! err: %d",
				dst_err);
		return dst_err;
	}

	return PNSO_OK;
}

static pnso_error_t
crypto_setup(struct service_info *svc_info,
	     const struct service_params *svc_params)
{
	pnso_error_t			err;

	err = crypto_validate_input(svc_info, svc_params);
	if (err)
		return err;

	svc_info->si_desc = crypto_get_desc(svc_info, false);
	if (!svc_info->si_desc)
		return ENOMEM;

	err = pc_res_svc_status_get(svc_info->si_pcr,
				    MPOOL_TYPE_CRYPTO_STATUS_DESC,
				    &svc_info->si_status_desc);
	if (err)
		return err;

	err = crypto_dst_blist_setup(svc_info, svc_params);
	if (err)
		return err;

	err = crypto_src_dst_aol_fill(svc_info);
	if (err)
		return err;

	svc_info->si_desc_flags = 0;
	crypto_desc_fill(svc_info, svc_params->u.sp_crypto_desc);

	err = svc_seq_desc_setup(svc_info, svc_info->si_desc,
				 sizeof(struct crypto_desc), 0);
	if (err)
		return err;

	if (svc_info->si_type == PNSO_SVC_TYPE_ENCRYPT)
		PAS_INC_NUM_ENC_REQUESTS(svc_info->si_pcr);
	else
		PAS_INC_NUM_DEC_REQUESTS(svc_info->si_pcr);

	return PNSO_OK;
}

static pnso_error_t
crypto_chain(struct chain_entry *centry)
{
	struct service_info		*svc_info = &centry->ce_svc_info;
	struct crypto_chain_params	*crypto_chain =
		&svc_info->si_crypto_chain;
	struct interm_buf_list		*iblist;
	struct service_info		*svc_next;
	pnso_error_t			err = PNSO_OK;

	if (chn_service_has_sub_chain(svc_info)) {

		/*
		 * Length must always be indicated to P4+ chainer (for rate
		 * limiting) even when PDMA is not applicable.
		 */
		crypto_chain->ccp_data_len = svc_info->si_dst_blist.len;
		if (chn_service_has_interm_blist(svc_info)) {
			iblist = &svc_info->si_iblist;
			crypto_chain->ccp_crypto_buf_addr =
				iblist->blist.buffers[0].buf;
			if (chn_service_has_sgl_pdma(svc_info)) {
				crypto_chain->ccp_sgl_pdma_dst_addr =
					sonic_virt_to_phy(svc_info->si_sgl_pdma);
				crypto_chain->ccp_cmd.ccpc_sgl_pdma_en = true;
				crypto_chain->ccp_cmd.ccpc_sgl_pdma_len_from_desc =
					true;
				SGL_PDMA_PPRINT(crypto_chain->ccp_sgl_pdma_dst_addr);
			}
		}

		err = svc_status_desc_addr_get(&svc_info->si_status_desc, 0,
				&crypto_chain->ccp_status_addr_0, 0);
		if (err)
			goto out;

		if (chn_service_has_interm_status(svc_info)) {
			crypto_chain->ccp_status_addr_1 = crypto_chain->ccp_status_addr_0;
			err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, 0,
						&crypto_chain->ccp_status_addr_0, 0);
			if (err)
				goto out;

			crypto_chain->ccp_status_len = sizeof(struct crypto_status_desc);
			crypto_chain->ccp_cmd.ccpc_status_dma_en = true;
		}
		crypto_chain->ccp_cmd.ccpc_stop_chain_on_error = true;

		svc_next = chn_service_next_svc_get(svc_info);
		OSAL_ASSERT(svc_next);

		if (chn_service_type_is_decrypt(svc_info) &&
		    chn_service_type_is_cpdc(svc_next)) {

			/*
			 * Crypto chaining does not do padding so does not
			 * require an extra SGL.
			 */
			err = pc_res_sgl_vec_packed_get(svc_info->si_pcr,
				    &svc_info->si_dst_blist,
				    svc_info->si_block_size,
				    MPOOL_TYPE_CPDC_SGL_VECTOR,
				    &svc_info->si_src_sgl, false);
			if (err) {
				OSAL_LOG_DEBUG("failed to setup chain decrypt SGL! err: %d",
						err);
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

		err = svc_next->si_ops.sub_chain_from_crypto(svc_next,
				crypto_chain);
		if (!err)
			err = seq_setup_crypto_chain(svc_info,
					svc_info->si_desc);
	}
out:
	if (err)
		OSAL_LOG_SPECIAL_ERROR("failed seq_setup_crypto_chain: err %d",
				err);

	return err;
}

static pnso_error_t
crypto_sub_chain_from_cpdc(struct service_info *svc_info,
			   struct cpdc_chain_params *cpdc_chain)
{
	struct service_info	*svc_prev = chn_service_prev_svc_get(svc_info);

	cpdc_chain->ccp_cmd.ccpc_aol_update_en =
		chn_service_type_is_cpdc(svc_prev);

        svc_rate_limit_control_eval(svc_info, &cpdc_chain->ccp_rl_control);
	if (cpdc_chain->ccp_cmd.ccpc_sgl_pdma_en)
		cpdc_chain->ccp_sgl_vec_addr =
			cpdc_chain->ccp_aol_dst_vec_addr;

	cpdc_chain->ccp_aol_src_vec_addr =
		sonic_virt_to_phy(svc_info->si_src_aol.aol);
	cpdc_chain->ccp_aol_dst_vec_addr =
		sonic_virt_to_phy(svc_info->si_dst_aol.aol);

	cpdc_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	cpdc_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	cpdc_chain->ccp_num_alt_descs = 1;

	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &cpdc_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
crypto_sub_chain_from_crypto(struct service_info *svc_info,
			     struct crypto_chain_params *crypto_chain)
{
	/*
	 * For testing purposes, it is possible to chain encrypt to decrypt.
	 */
        svc_rate_limit_control_eval(svc_info, &crypto_chain->ccp_rl_control);
	crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	crypto_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;

	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &crypto_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
crypto_enable_interrupt(struct service_info *svc_info,
			void *poll_ctx)
{
	struct crypto_chain_params	*crypto_chain;
	struct per_core_resource	*pcr;
	pnso_error_t			err = PNSO_OK;
	struct service_chain		*chain;

	chain = svc_info->si_centry->ce_chain_head;

	/*
	 * HW lacks ability to signal per-descriptor interrupt so P4+ chainer
	 * will be used for that purpose. Also note that when a chain successor
	 * is present, that service will take care of setting up the interrupt.
	 */
	OSAL_ASSERT(chn_service_is_mode_async(svc_info));
	if (!chn_service_has_sub_chain(svc_info)) {
		crypto_chain = &svc_info->si_crypto_chain;
		pcr = svc_info->si_pcr;
		OSAL_LOG_DEBUG("pcr: 0x"PRIx64" poll_ctx: 0x"PRIx64,
			       (uint64_t)pcr, (uint64_t)poll_ctx);

		chain->sc_async_evid = sonic_intr_get_ev_id(pcr,
				(uint64_t)poll_ctx,
				&crypto_chain->ccp_next_db_spec.nds_addr);
		if (!chain->sc_async_evid) {
			err = EINVAL;
			OSAL_LOG_DEBUG("crypto failed sonic_intr_get_ev_id err: %d",
				       err);
			goto out;
		}
		crypto_chain->ccp_next_db_spec.nds_data =
				cpu_to_be64(sonic_intr_get_fire_data64());
		crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;

		crypto_chain->ccp_intr_addr =
			sonic_get_per_core_intr_assert_addr(pcr);
		crypto_chain->ccp_intr_data = sonic_get_intr_assert_data();
		crypto_chain->ccp_cmd.ccpc_intr_en = true;

		/*
		 * Note that if crypto here is a chain subordinate, then effectively
		 * we're setting up one more chain level for async handling.
		 */
		err = seq_setup_crypto_chain(svc_info, svc_info->si_desc);
		if (err != PNSO_OK)
			OSAL_LOG_SPECIAL_ERROR("failed seq_setup_crypto_chain: err %d",
					err);
	}
out:
	return  err;
}

static pnso_error_t
crypto_ring_db(struct service_info *svc_info)
{
	if (chn_service_is_starter(svc_info)) {
		return seq_ring_db(svc_info);
	}

	return EINVAL;
}

static pnso_error_t
crypto_poll(struct service_info *svc_info)
{
	const struct crypto_chain_params *crypto_chain;
	pnso_error_t			err = PNSO_OK;

	volatile struct crypto_status_desc *status_desc;
	uint64_t cpl_data;

	OSAL_LOG_DEBUG("enter ...");

	/*
	 * When chaining is involved, crypto_desc's cd_db_addr would point
	 * to a seq statusQ's doorbell rather than status_desc. The completion
	 * of the next service will imply completion of this crypto service.
	 *
	 * Also, if this service had been programmed to generate interrupt
	 * for async mode, the same thing would have applied. That is,
	 * cd_db_addr would ring a seq statusQ doorbell which would be
	 * sufficient to indicate service completion.
	 */
	crypto_chain = &svc_info->si_crypto_chain;
	if (chn_service_has_sub_chain(svc_info) ||
			crypto_chain->ccp_cmd.ccpc_intr_en)
		goto out;

	status_desc = svc_info->si_status_desc.desc;

	cpl_data = chn_service_type_is_decrypt(svc_info) ?
		   CRYPTO_DECRYPT_CPL_DATA : CRYPTO_ENCRYPT_CPL_DATA;

	while (1) {
		err = (status_desc->csd_cpl_data == cpl_data) ? PNSO_OK : EBUSY;
		if (!err)
			break;

		if (svc_poll_expiry_check(svc_info)) {
			err = ETIMEDOUT;
			OSAL_LOG_ERROR("poll-time limit reached! service: %s status_desc: 0x" PRIx64 "err: %d",
					svc_get_type_str(svc_info->si_type),
					(uint64_t) status_desc, err);
			/* Initiate error reset recovery */
			if (pnso_lif_error_reset_recovery_en_get())
				pnso_lif_reset_ctl_start();
			break;
		}

		if (!(svc_info->si_flags & CHAIN_SFLAG_MODE_SYNC)) {
			OSAL_LOG_DEBUG("transient err: %d", err);
			goto out;
		}

		if (pnso_lif_reset_ctl_pending()) {
			err = PNSO_LIF_IO_ERROR;
			break;
		}

		osal_yield();
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
out:
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
crypto_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status	*svc_status;
	struct crypto_status_desc	*status_desc;
	bool from_parent;

	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		svc_status->err = err;
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d err: %d",
			svc_status->svc_type, svc_info->si_type, err);
		goto out;
	}
	status_desc = svc_info->si_status_desc.desc;

	if (status_desc->csd_err) {
		OSAL_LOG_ERROR("hw error reported: " PRIu64, status_desc->csd_err);
		svc_status->err = crypto_desc_status_convert(status_desc->csd_err);
		goto out;
	}

	/*
	 * In chaining case, the actual data length may have been determined
	 * in the parent service and propagated down the chain. For example,
	 * if the parent were the CP service, the output compressed data length
	 * plus any padding would have been ccomputed and stored.
	 */
	svc_status = svc_info->si_svc_status;
	from_parent = chn_service_deps_data_len_set_from_parent(svc_info);
	svc_status->u.dst.data_len = chn_service_deps_data_len_get(svc_info);
	if (from_parent) {

		/*
		 * In debug mode, verify the padding adjustment in the dst AOL.
		 *
		 * CAUTION: it can be costly to invoke crypto_aol_total_len_get()
		 * so do not call it outside of the DEBUG macro.
		 */
		OSAL_LOG_DEBUG("my data_len %u parent data_len %u",
				crypto_aol_total_len_get(&svc_info->si_src_aol),
				svc_status->u.dst.data_len);
	}

	if (svc_info->si_type == PNSO_SVC_TYPE_ENCRYPT)
		PAS_INC_NUM_ENC_BYTES(svc_info->si_pcr,
				svc_status->u.dst.data_len);
	else
		PAS_INC_NUM_DEC_BYTES(svc_info->si_pcr,
				svc_status->u.dst.data_len);

	err = PNSO_OK;
out:
	return err;
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

	seq_cleanup_crypto_chain(svc_info);
	seq_cleanup_desc(svc_info);

	pc_res_svc_status_put(svc_info->si_pcr,	&svc_info->si_status_desc);
	crypto_put_desc(svc_info, false, svc_info->si_desc);

	putil_put_interm_buf_list(svc_info);

	pc_res_svc_status_put(svc_info->si_pcr,	&svc_info->si_istatus_desc);
	pc_res_sgl_pdma_put(pcr, svc_info->si_sgl_pdma);
	pc_res_sgl_put(pcr, &svc_info->si_src_sgl);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops encrypt_ops = {
	.setup = crypto_setup,
	.chain = crypto_chain,
	.sub_chain_from_cpdc = crypto_sub_chain_from_cpdc,
	.sub_chain_from_crypto = crypto_sub_chain_from_crypto,
	.enable_interrupt = crypto_enable_interrupt,
	.ring_db = crypto_ring_db,
	.poll = crypto_poll,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};

struct service_ops decrypt_ops = {
	.setup = crypto_setup,
	.chain = crypto_chain,
	.sub_chain_from_cpdc = crypto_sub_chain_from_cpdc,
	.sub_chain_from_crypto = crypto_sub_chain_from_crypto,
	.enable_interrupt = crypto_enable_interrupt,
	.ring_db = crypto_ring_db,
	.poll = crypto_poll,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};
