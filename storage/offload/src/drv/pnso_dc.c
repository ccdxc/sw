/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "sonic_api_int.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_svc.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_seq.h"
#include "pnso_utils.h"

static inline void
clear_dc_header_present(uint16_t flags, struct cpdc_desc *desc)
{
	if (!(flags & PNSO_DC_DFLAG_HEADER_PRESENT))
		desc->u.cd_bits.cc_header_present = 0;
}

static void
fill_dc_desc(struct service_info *svc_info, struct cpdc_desc *desc)
{
	struct cpdc_status_desc *status_desc = svc_info->si_status_desc;
	uint32_t src_buf_len = svc_info->si_src_blist.len;
	uint32_t dst_buf_len = svc_info->si_dst_blist.len;

	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(svc_info->si_src_sgl.sgl);
	desc->cd_dst = (uint64_t) sonic_virt_to_phy(svc_info->si_dst_sgl.sgl);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_header_present = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(src_buf_len == MAX_CPDC_SRC_BUF_LEN) ? 0 : src_buf_len;
	desc->cd_threshold_len =
		(dst_buf_len == MAX_CPDC_DST_BUF_LEN) ? 0 : dst_buf_len;

	if (svc_info->si_istatus_desc) {
		desc->cd_status_addr = mpool_get_object_phy_addr(
				MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC,
				svc_info->si_istatus_desc);
		osal_rmem_set(desc->cd_status_addr, 0,
				min(sizeof(*status_desc), (size_t) 8));
	} else
		desc->cd_status_addr = (uint64_t)
			sonic_virt_to_phy(status_desc);

	desc->cd_status_data = CPDC_DC_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
decompress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	dc_desc = cpdc_get_desc(svc_info, false);
	if (!dc_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc desc from pool! err: %d", err);
		goto out;
	}
	svc_info->si_desc = dc_desc;

	status_desc = cpdc_get_status_desc(svc_info->si_pcr, false);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc status desc from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_status_desc = status_desc;

	err = cpdc_setup_rmem_status_desc(svc_info, false);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dc rmem status desc from pool! err: %d",
				err);
		goto out;
	}

	err = cpdc_setup_rmem_dst_blist(svc_info, svc_params);
	if (err) {
		OSAL_LOG_ERROR("failed to setup dc rmem dst buffer list! err: %d",
				err);
		goto out;
	}

	err = cpdc_update_service_info_sgls(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dc src/dst sgl from pool! err: %d",
				err);
		goto out;
	}

	fill_dc_desc(svc_info, dc_desc);
	clear_dc_header_present(svc_info->si_desc_flags, dc_desc);

	err = cpdc_setup_seq_desc(svc_info, dc_desc, 0);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out;
	}

	PAS_INC_NUM_DC_REQUESTS(svc_info->si_pcr);
	PAS_INC_NUM_DC_BYTES_IN(svc_info->si_pcr, svc_info->si_src_blist.len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
decompress_chain(struct chain_entry *centry)
{
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		OSAL_LOG_ERROR("failed to chain! err: %d", err);
		goto out;
	}

out:
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
decompress_sub_chain_from_cpdc(struct service_info *svc_info,
			       struct cpdc_chain_params *cpdc_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_sub_chain_from_crypto(struct service_info *svc_info,
				 struct crypto_chain_params *crypto_chain)
{
	struct cpdc_desc *dc_desc;

	if (crypto_chain->ccp_cmd.ccpc_comp_sgl_src_en) {
		dc_desc = svc_info->si_desc;
		dc_desc->cd_src = crypto_chain->ccp_comp_sgl_src_addr;
		dc_desc->u.cd_bits.cc_src_is_list = true;
		CPDC_PPRINT_DESC(dc_desc);
	}
	crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	crypto_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &crypto_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
decompress_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static void
decompress_disable_interrupt(struct service_info *svc_info)
{
	return cpdc_cleanup_interrupt_params(svc_info);
}

static pnso_error_t
decompress_ring_db(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	const struct sequencer_info *seq_info;
	bool ring_db;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	ring_db = (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
	if (ring_db) {
		OSAL_LOG_INFO("ring door bell <===");

		seq_info = &svc_info->si_seq_info;
		seq_ring_db(svc_info);

		err = PNSO_OK;
	}

	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
decompress_poll(struct service_info *svc_info)
{
	return cpdc_poll(svc_info, NULL);
}

static pnso_error_t
decompress_read_status(struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	dc_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

	err = cpdc_common_read_status(dc_desc, status_desc);
	if (err)
		goto out;

	OSAL_LOG_DEBUG("exit! status verification success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
decompress_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d err: %d",
			svc_status->svc_type,  svc_info->si_type, err);
		goto out;
	}

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid dc status desc! err: %d", err);
		OSAL_ASSERT(err);
	}

	if (!status_desc->csd_valid) {
		svc_status->err = err;
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	if (status_desc->csd_err) {
		svc_status->err = cpdc_convert_desc_error(status_desc->csd_err);
		OSAL_LOG_ERROR("service failed! err: %d", err);
		goto out;
	}

	svc_status->u.dst.data_len = status_desc->csd_output_data_len;
	chn_service_deps_data_len_set(svc_info,
			status_desc->csd_output_data_len);
	PAS_INC_NUM_DC_BYTES_OUT(svc_info->si_pcr,
			status_desc->csd_output_data_len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
decompress_teardown(struct service_info *svc_info)
{
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	/*
	 * Trace the desc/SGLs once more to verify any padding applied
	 * by sequencer.
	 */
	CPDC_PPRINT_DESC(svc_info->si_desc);

	seq_cleanup_cpdc_chain(svc_info);
	seq_cleanup_desc(svc_info);

	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);

	cpdc_teardown_rmem_dst_blist(svc_info);
	cpdc_teardown_rmem_status_desc(svc_info, false);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(svc_info->si_pcr, false, status_desc);

	dc_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, false, dc_desc);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops dc_ops = {
	.setup = decompress_setup,
	.chain = decompress_chain,
	.sub_chain_from_cpdc = decompress_sub_chain_from_cpdc,
	.sub_chain_from_crypto = decompress_sub_chain_from_crypto,
	.enable_interrupt = decompress_enable_interrupt,
	.disable_interrupt = decompress_disable_interrupt,
	.ring_db = decompress_ring_db,
	.poll = decompress_poll,
	.read_status = decompress_read_status,
	.write_result = decompress_write_result,
	.teardown = decompress_teardown
};
