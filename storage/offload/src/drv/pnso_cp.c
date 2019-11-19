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

static inline void __attribute__((unused))
verify_cp_header_update(struct service_info *svc_info,
		struct pnso_compression_header *cp_hdr)
{
	struct pnso_compression_header rmem_hdr;

	if (chn_service_has_interm_blist(svc_info)) {
		rmem_hdr.chksum = (uint32_t)~0;
		sonic_rmem_read(&rmem_hdr, 
				svc_info->si_iblist.blist.buffers[0].buf,
				sizeof(rmem_hdr));

		if (cp_hdr->chksum != rmem_hdr.chksum)
			OSAL_LOG_ERROR("cp_hdr chksum 0x%x != rmem chksum 0x%x",
					cp_hdr->chksum, rmem_hdr.chksum);

		if (cp_hdr->data_len != rmem_hdr.data_len)
			OSAL_LOG_ERROR("cp_hdr data_len %u != rmem data_len %u",
					cp_hdr->data_len, rmem_hdr.data_len);

		if (cp_hdr->version != rmem_hdr.version)
			OSAL_LOG_ERROR("cp_hdr version 0x%x != rmem version 0x%x",
					cp_hdr->version, rmem_hdr.version);
	}
}

static inline struct pnso_compression_header *
get_cp_header(struct service_info *svc_info)
{
	struct cpdc_sgl	*dst_sgl;
	struct chain_sgl_pdma_tuple *tuple;
	struct pnso_compression_header *cp_hdr = NULL;

	/*
	 * Go through some hoops to find the dst buffer where the cp_hdr
	 * was written: when RMEM was the destination, the original host
	 * dst, if any, could be obtained from the SGL pdma descriptor.
	 * When RMEM was not used, the host dst, if any, would be in
	 * si_dst_blist.
	 */
	dst_sgl = svc_info->si_dst_sgl.sgl;
	if (svc_info->si_dst_blist.blist->buffer_0_va) {
		cp_hdr = (struct pnso_compression_header *)
			svc_info->si_dst_blist.blist->buffer_0_va;
	} else {
		if (svc_info->si_sgl_pdma) {
			tuple = &svc_info->si_sgl_pdma->tuple[0];
			if (tuple->len >= sizeof(*cp_hdr))
				cp_hdr = sonic_phy_to_virt(tuple->addr);
		} else if (svc_info->si_dst_blist.type ==
				SERVICE_BUF_LIST_TYPE_HOST) {
			if (dst_sgl->cs_len_0 >= sizeof(*cp_hdr))
				cp_hdr = (struct pnso_compression_header *)
					sonic_phy_to_virt(dst_sgl->cs_addr_0);
		}
	}

	OSAL_LOG_DEBUG("dst_sgl: 0x" PRIx64 " cs_addr_0: 0x" PRIx64 " cp_hdr: 0x" PRIx64,
		       (uint64_t) dst_sgl, (uint64_t) dst_sgl->cs_addr_0,
		       (uint64_t) cp_hdr);

	return cp_hdr;
}

static inline bool
is_dflag_zero_pad_enabled(uint16_t flags)
{
	return (flags & PNSO_CP_DFLAG_ZERO_PAD) ? true : false;
}

static inline bool
is_dflag_insert_header_enabled(uint16_t flags)
{
	return (flags & PNSO_CP_DFLAG_INSERT_HEADER) ? true : false;
}

static inline bool __attribute__((unused))
is_dflag_bypass_onfail_enabled(uint16_t flags)
{
	return (flags & PNSO_CP_DFLAG_BYPASS_ONFAIL) ? true : false;
}

static int
fill_cp_desc(struct service_info *svc_info, struct cpdc_desc *desc,
		uint32_t threshold_len)
{
	uint64_t aligned_addr;
	pnso_error_t err;
	struct cpdc_sgl *sgl;
	uint32_t scratch_buf_len = 0;

	memset(desc, 0, sizeof(*desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(svc_info->si_src_sgl.sgl);
	desc->cd_dst = (uint64_t) sonic_virt_to_phy(svc_info->si_dst_sgl.sgl);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_insert_header =
		is_dflag_insert_header_enabled(svc_info->si_desc_flags);
	desc->u.cd_bits.cc_integrity_type = CPDC_INTEGRITY_TYPE_DFLT;
	desc->u.cd_bits.cc_integrity_src = CPDC_INTEGRITY_SRC_DFLT;
	desc->u.cd_bits.cc_chksum_adler = CPDC_CHKSUM_TYPE_DFLT;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	sgl = svc_info->si_src_sgl.sgl;
	if (sgl->cs_len_0 == CPDC_SCRATCH_BUFFER_LEN)
		scratch_buf_len = CPDC_SCRATCH_BUFFER_LEN;

	desc->cd_datain_len = 
		cpdc_desc_data_len_set_eval(svc_info->si_type,
				svc_info->si_src_blist.len + scratch_buf_len);

	desc->cd_threshold_len = cpdc_desc_data_len_set_eval(svc_info->si_type,
					threshold_len);
	err = svc_status_desc_addr_get(&svc_info->si_status_desc, 0,
			&aligned_addr, CPDC_STATUS_PAD_CLEAR_SZ);
	desc->cd_status_addr = aligned_addr;
	if (err)
		goto out;

	if (chn_service_has_interm_status(svc_info)) {
		err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, 0,
			&aligned_addr, CPDC_STATUS_MIN_CLEAR_SZ);
		desc->cd_status_addr = aligned_addr;
	}

	desc->cd_status_data = CPDC_CP_STATUS_DATA;
out:
	return err;
}

static pnso_error_t
compress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_compression_desc *pnso_cp_desc;
	struct cpdc_desc *cp_desc;
	uint32_t threshold_len;

	OSAL_LOG_DEBUG("enter ...");

	pnso_cp_desc = (struct pnso_compression_desc *)
		svc_params->u.sp_cp_desc;

	threshold_len = pnso_cp_desc->threshold_len;

	cp_desc = cpdc_get_desc(svc_info, false);
	if (!cp_desc) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("cannot obtain cp desc from pool! err: %d", err);
		goto out;
	}
	svc_info->si_desc = cp_desc;

	err = cpdc_setup_status_desc(svc_info, false);
	if (err) {
		OSAL_LOG_DEBUG("cannot obtain cp status desc from pool! err: %d",
				err);
		goto out;
	}

	err = cpdc_setup_rmem_status_desc(svc_info, false);
	if (err) {
		OSAL_LOG_DEBUG("intermediate status not available, "
				"using host status");
	}

	err = cpdc_setup_rmem_dst_blist(svc_info, svc_params);
	if (err) {
		OSAL_LOG_ERROR("failed to setup cp rmem dst buffer list! err: %d",
				err);
		goto out;
	}

	err = cpdc_update_service_info_sgls(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain cp src/dst sgl from pool! err: %d",
				err);
		goto out;
	}

	err = fill_cp_desc(svc_info, cp_desc, threshold_len);
	if (err) {
		OSAL_LOG_ERROR("cannot fill_cp_desc! err: %d", err);
		goto out;
	}

	err = svc_seq_desc_setup(svc_info, cp_desc, sizeof(*cp_desc), 0);
	if (err) {
		OSAL_LOG_DEBUG("failed to setup sequencer desc! err: %d", err);
		goto out;
	}

	CPDC_PPRINT_DESC(cp_desc);

	PAS_INC_NUM_CP_REQUESTS(svc_info->si_pcr);
	PAS_INC_NUM_CP_BYTES_IN(svc_info->si_pcr, svc_info->si_src_blist.len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
compress_chain(struct chain_entry *centry)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info *svc_info, *next_svc_info;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(centry);

	svc_info = &centry->ce_svc_info;
	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_LAST_SERVICE)) {
		if (is_dflag_zero_pad_enabled(svc_info->si_desc_flags)) {
			err = seq_setup_cp_pad_chain_params(svc_info, cp_desc,
					status_desc);
			if (err) {
				OSAL_LOG_DEBUG("failed to setup cp/pad params! err: %d",
						err);
				goto out;
			}
		}

		OSAL_LOG_DEBUG("lone or last service, chaining not needed! si_type: %d si_flags: %d",
				svc_info->si_type, svc_info->si_flags);
		goto done;
	}

	err = seq_setup_cp_chain_params(svc_info, cp_desc, status_desc);
	if (err) {
		OSAL_LOG_DEBUG("failed to setup cp in chain! err: %d", err);
		goto out;
	}

	if (centry->ce_next) {
		next_svc_info = &centry->ce_next->ce_svc_info;
		err = next_svc_info->si_ops.sub_chain_from_cpdc(
				next_svc_info,
				&svc_info->si_cpdc_chain);
		if (err) {
			OSAL_LOG_ERROR("failed to chain next service after cp! err: %d",
					err);
			goto out;
		}
		OSAL_LOG_INFO("chaining of services after cp done!");
	}

	err = seq_setup_cpdc_chain_status_desc(svc_info);
	if (err) {
		OSAL_LOG_DEBUG("failed to setup sequencer status desc! err: %d",
				err);
		goto out;
	}

	CPDC_PPRINT_DESC(cp_desc);
	OSAL_LOG_INFO("setup of sequencer desc for cp chain done!");

done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
compress_sub_chain_from_cpdc(struct service_info *svc_info,
			     struct cpdc_chain_params *cpdc_chain)
{
        svc_rate_limit_control_eval(svc_info, &cpdc_chain->ccp_rl_control);
	cpdc_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	cpdc_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	cpdc_chain->ccp_num_alt_descs = 1;

	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &cpdc_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
compress_sub_chain_from_crypto(struct service_info *svc_info,
			       struct crypto_chain_params *crypto_chain)
{
        svc_rate_limit_control_eval(svc_info, &crypto_chain->ccp_rl_control);
	crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	crypto_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;

	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &crypto_chain->ccp_ring_spec,
				   svc_info->si_desc, 1);
}

static pnso_error_t
compress_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static pnso_error_t
compress_ring_db(struct service_info *svc_info)
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
		err = seq_ring_db(svc_info);
	}

	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
compress_poll(struct service_info *svc_info)
{
	pnso_error_t err;

	volatile struct cpdc_status_desc *status_desc;
	volatile uint64_t *cp_pad_cpl_addr;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	err = cpdc_poll(svc_info, NULL);
	if (err)
		goto out;

	/*
	 * compression is done, however, bail-out only if service is not lone
	 * and if cp/pad is not requested
	 *
	 */
	if (!is_dflag_zero_pad_enabled(svc_info->si_desc_flags) ||
		chn_service_has_sub_chain(svc_info))
			goto out;

	OSAL_LOG_DEBUG("cp/pad lone or last service in chain");

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
	if (status_desc->csd_err) {
		OSAL_LOG_DEBUG("cp failed - no need to wait for pad result");
		goto out;
	}

	cp_pad_cpl_addr = cpdc_cp_pad_cpl_addr_get(status_desc);
	while (1) {
		/* poll on padding opaque tag updated by P4+ */
		if (*cp_pad_cpl_addr == CPDC_PAD_STATUS_DATA) {
			OSAL_LOG_DEBUG("cp/pad status data matched!");
			break;
		}

		if (svc_poll_expiry_check(svc_info)) {
			err = ETIMEDOUT;
			OSAL_LOG_ERROR("cp/pad poll-time limit reached! service: %s status_desc: 0x" PRIx64 " err: %d",
					svc_get_type_str(svc_info->si_type),
					(uint64_t) status_desc, err);
			/* Initiate error reset recovery */
			if (pnso_lif_error_reset_recovery_en_get())
				pnso_lif_reset_ctl_start();
			break;
		}

		if (!(svc_info->si_flags & CHAIN_SFLAG_MODE_SYNC)) {
			err = EBUSY;
			OSAL_LOG_DEBUG("cp/pad async/poll mode. transient err: %d",
				       err);
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
compress_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *cp_desc;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc;
	struct pnso_compression_header *cp_hdr = NULL;
	uint32_t len, hdr_version;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	OSAL_LOG_DEBUG("sbi_num_entries: %d sbi_bulk_desc_idx: %d sbi_desc_idx: %d",
			svc_info->si_batch_info.sbi_num_entries,
			svc_info->si_batch_info.sbi_bulk_desc_idx,
			svc_info->si_batch_info.sbi_desc_idx);

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
	CPDC_PPRINT_STATUS_DESC(status_desc);

	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		svc_status->err = err;
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d err: %d",
			svc_status->svc_type, svc_info->si_type, err);
		goto out;
	}

	if (!status_desc->csd_valid) {
		svc_status->err = err;
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != cp_desc->cd_status_data)
		OSAL_LOG_DEBUG("partial data mismatch, expected %u received: %u err: %d",
				cp_desc->cd_status_data,
				status_desc->csd_partial_data, err);

	if (status_desc->csd_err) {
		svc_status->err = cpdc_convert_desc_error(status_desc->csd_err);

		/* propagate error code; for perf, keep log level higher */
		OSAL_LOG_DEBUG("hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, svc_status->err);
		goto pass_err;
	}

	if (chn_service_is_cp_hdr_insert_applic(svc_info)) {
		cp_hdr = get_cp_header(svc_info);
		if (!cp_hdr) {
			OSAL_LOG_DEBUG("skip cp header checks");
			goto done;
		}

		/*
		 * When a chksum field is present in cp_hdr, overwrite it with
		 * integ_data from the status desc (due to a HW constraint).
		 * Note that in chaining case, even though P4+ chainer (if requested)
		 * would also have done the copy, it might have done so only to the
		 * intermediate RMEM destination buffer rather than to the host.
		 * Hence, we ensure that the host buffer overwrite happens here.
		 */
		if (cpdc_cp_hdr_chksum_info_get(svc_info, &len)) {
			if (len) {
				if (cpdc_desc_is_integ_data_wr_required(cp_desc))
					cp_hdr->chksum = (uint32_t)
						status_desc->csd_integrity_data;
			} else
				cp_hdr->chksum = 0;
		}

		hdr_version = cpdc_cp_hdr_version_info_get(svc_info);
		if (cpdc_cp_hdr_version_wr_required(hdr_version))
			cp_hdr->version = hdr_version;

#ifdef SIMPLE_CP_HDR_UPDATE_VERIFY
		verify_cp_header_update(svc_info, cp_hdr);
#endif

#ifndef NDEBUG
		len = cp_desc->cd_datain_len == 0 ?
			MAX_CPDC_SRC_BUF_LEN : cp_desc->cd_datain_len;
		if ((cp_hdr->data_len == 0) || (cp_hdr->data_len > len)) {
			err = EINVAL;
			OSAL_LOG_ERROR("invalid data len! len: %u hdr_len: %u err: %d",
					len, cp_hdr->data_len, err);
			goto out;
		}

		if (status_desc->csd_output_data_len != (cp_hdr->data_len +
				 sizeof(struct pnso_compression_header))) {
			err = EINVAL;
			OSAL_LOG_ERROR("output data len mismatch! output_data_len: %u hdr_len: %u err: %d",
					status_desc->csd_output_data_len,
					cp_hdr->data_len, err);
			goto out;
		}
#endif
	}

	svc_status->u.dst.data_len =
		cpdc_desc_data_len_get_eval(svc_info->si_type,
				status_desc->csd_output_data_len);
	chn_service_deps_data_len_set(svc_info, svc_status->u.dst.data_len);

	PAS_INC_NUM_CP_BYTES_OUT(svc_info->si_pcr,
			svc_status->u.dst.data_len);
done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("status/result update success!");
pass_err:
	OSAL_LOG_DEBUG("exit!");
	return err;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
compress_teardown(struct service_info *svc_info)
{
	struct cpdc_desc *cp_desc;

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
	cpdc_teardown_rmem_status_desc(svc_info);

	cpdc_teardown_status_desc(svc_info);

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, false, cp_desc);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops cp_ops = {
	.setup = compress_setup,
	.chain = compress_chain,
	.sub_chain_from_cpdc = compress_sub_chain_from_cpdc,
	.sub_chain_from_crypto = compress_sub_chain_from_crypto,
	.enable_interrupt = compress_enable_interrupt,
	.ring_db = compress_ring_db,
	.poll = compress_poll,
	.write_result = compress_write_result,
	.teardown = compress_teardown
};
