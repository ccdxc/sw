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
fill_dc_desc(struct cpdc_desc *desc, struct cpdc_sgl *src_sgl,
		struct cpdc_sgl *dst_sgl, struct cpdc_status_desc *status_desc,
		uint32_t src_buf_len, uint32_t dst_buf_len)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_sgl);
	desc->cd_dst = (uint64_t) sonic_virt_to_phy(dst_sgl);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_header_present = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(src_buf_len == MAX_CPDC_SRC_BUF_LEN) ? 0 : src_buf_len;
	desc->cd_threshold_len =
		(dst_buf_len == MAX_CPDC_DST_BUF_LEN) ? 0 : dst_buf_len;

	desc->cd_status_addr = (uint64_t) sonic_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_DC_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
decompress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_decompression_desc *pnso_dc_desc;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct per_core_resource *pcr;
	size_t src_buf_len, dst_buf_len;

	OSAL_LOG_DEBUG("enter ...");

	pnso_dc_desc = (struct pnso_decompression_desc *)
		svc_params->u.sp_dc_desc;

	svc_info->si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info->si_desc_flags = pnso_dc_desc->flags;

	pcr = svc_info->si_pcr;

	dc_desc = cpdc_get_desc(svc_info, false);
	if (!dc_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc desc from pool! err: %d", err);
		goto out;
	}
	svc_info->si_desc = dc_desc;

	status_desc = cpdc_get_status_desc(pcr, false);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc status desc from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_status_desc = status_desc;

	err = cpdc_update_service_info_sgls(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dc src/dst sgl from pool! err: %d",
				err);
		goto out;
	}

	src_buf_len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	dst_buf_len = pbuf_get_buffer_list_len(svc_params->sp_dst_blist);

	fill_dc_desc(dc_desc, svc_info->si_src_sgl.sgl,
			svc_info->si_dst_sgl.sgl, status_desc,
			src_buf_len, dst_buf_len);
	clear_dc_header_present(svc_info->si_desc_flags, dc_desc);

	err = cpdc_setup_seq_desc(svc_info, dc_desc, 0);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out;
	}

	PAS_INC_NUM_DC_REQUESTS(pcr);
	PAS_INC_NUM_DC_BYTES_IN(pcr, src_buf_len);

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

static pnso_error_t
decompress_ring_db(const struct service_info *svc_info)
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
decompress_poll(const struct service_info *svc_info)
{
	return cpdc_poll(svc_info);
}

static pnso_error_t
decompress_read_status(const struct service_info *svc_info)
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
	struct per_core_resource *pcr;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	/*
	 * Trace the desc/SGLs once more to verify any padding applied
	 * by sequencer.
	 */
	CPDC_PPRINT_DESC(svc_info->si_desc);

	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);

	pcr = svc_info->si_pcr;

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(pcr, false, status_desc);

	dc_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, false, dc_desc);
	seq_cleanup_desc(svc_info);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops dc_ops = {
	.setup = decompress_setup,
	.chain = decompress_chain,
	.sub_chain_from_cpdc = decompress_sub_chain_from_cpdc,
	.sub_chain_from_crypto = decompress_sub_chain_from_crypto,
	.enable_interrupt = decompress_enable_interrupt,
	.ring_db = decompress_ring_db,
	.poll = decompress_poll,
	.read_status = decompress_read_status,
	.write_result = decompress_write_result,
	.teardown = decompress_teardown
};
