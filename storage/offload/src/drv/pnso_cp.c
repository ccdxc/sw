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

static inline void
clear_insert_header(uint16_t flags, struct cpdc_desc *desc)
{
	if (!is_dflag_insert_header_enabled(flags))
		desc->u.cd_bits.cc_insert_header = 0;
}

static void
fill_cp_desc(struct service_info *svc_info, struct cpdc_desc *desc,
		uint16_t threshold_len)
{
	struct cpdc_status_desc *status_desc = svc_info->si_status_desc;

	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(svc_info->si_src_sgl.sgl);
	desc->cd_dst = (uint64_t) sonic_virt_to_phy(svc_info->si_dst_sgl.sgl);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_insert_header = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(svc_info->si_src_blist.len == MAX_CPDC_SRC_BUF_LEN) ?
		0 : svc_info->si_src_blist.len;

	desc->cd_threshold_len = threshold_len;

	if (svc_info->si_istatus_desc) {
		desc->cd_status_addr = mpool_get_object_phy_addr(
				MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC,
				svc_info->si_istatus_desc);
		osal_rmem_set(desc->cd_status_addr, 0,
				min(sizeof(*status_desc), (size_t) 8));
	} else 
		desc->cd_status_addr = (uint64_t)
			sonic_virt_to_phy(status_desc);

	desc->cd_status_data = CPDC_CP_STATUS_DATA;
}

static pnso_error_t
compress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_compression_desc *pnso_cp_desc;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	uint16_t threshold_len;

	OSAL_LOG_DEBUG("enter ...");

	pnso_cp_desc = (struct pnso_compression_desc *)
		svc_params->u.sp_cp_desc;

	threshold_len = pnso_cp_desc->threshold_len;

	cp_desc = cpdc_get_desc(svc_info, false);
	if (!cp_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp desc from pool! err: %d", err);
		goto out;
	}
	svc_info->si_desc = cp_desc;

	status_desc = cpdc_get_status_desc(svc_info->si_pcr, false);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp status desc from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_status_desc = status_desc;

	err = cpdc_setup_rmem_status_desc(svc_info, false);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain cp rmem status desc from pool! err: %d",
				err);
		goto out;
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

	fill_cp_desc(svc_info, cp_desc, threshold_len);
	clear_insert_header(svc_info->si_desc_flags, cp_desc);

	err = cpdc_setup_seq_desc(svc_info, cp_desc, 0);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out;
	}

	CPDC_PPRINT_DESC(cp_desc);

	PAS_INC_NUM_CP_REQUESTS(svc_info->si_pcr);
	PAS_INC_NUM_CP_BYTES_IN(svc_info->si_pcr, svc_info->si_src_blist.len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
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
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_LAST_SERVICE)) {
		if (is_dflag_zero_pad_enabled(svc_info->si_desc_flags)) {
			err = seq_setup_cp_pad_chain_params(svc_info, cp_desc,
					status_desc);
			if (err) {
				OSAL_LOG_ERROR("failed to setup cp/pad params! err: %d",
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
		OSAL_LOG_ERROR("failed to setup cp in chain! err: %d", err);
		goto out;
	}

	if (centry->ce_next) {
		next_svc_info = &centry->ce_next->ce_svc_info;
		err = next_svc_info->si_ops.sub_chain_from_cpdc(next_svc_info,
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
		OSAL_LOG_ERROR("failed to setup sequencer status desc! err: %d", err);
		goto out;
	}

	CPDC_PPRINT_DESC(cp_desc);
	OSAL_LOG_INFO("setup of sequencer desc for cp chain done!");

done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
compress_sub_chain_from_cpdc(struct service_info *svc_info,
			     struct cpdc_chain_params *cpdc_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
compress_sub_chain_from_crypto(struct service_info *svc_info,
			       struct crypto_chain_params *crypto_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
compress_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static void
compress_disable_interrupt(struct service_info *svc_info)
{
	return cpdc_cleanup_interrupt_params(svc_info);
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
		seq_ring_db(svc_info);

		err = PNSO_OK;
	}

	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
compress_poll(struct service_info *svc_info)
{
	pnso_error_t err;
	volatile struct cpdc_status_desc *status_desc;
	uint64_t start_ts;

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

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_MODE_POLL) ||
		(svc_info->si_flags & CHAIN_SFLAG_MODE_ASYNC)) {
		err = (status_desc->csd_integrity_data ==
				CPDC_PAD_STATUS_DATA) ? PNSO_OK : EBUSY;
		OSAL_LOG_DEBUG("cp/pad async/poll mode. transient err: %d", err);
		goto out;
	}

	start_ts = svc_poll_expiry_start(svc_info);
	while (1) {
		/* poll on padding opaque tag updated by P4+ */
		if (status_desc->csd_integrity_data == CPDC_PAD_STATUS_DATA) {
			OSAL_LOG_DEBUG("cp/pad status data matched!");
			break;
		}

		if (svc_poll_expiry_check(svc_info, start_ts,
					  CPDC_POLL_LOOP_TIMEOUT)) {
			err = ETIMEDOUT;
			OSAL_LOG_ERROR("cp/pad poll-time limit reached! service: %s status_desc: 0x" PRIx64 " err: %d",
					svc_get_type_str(svc_info->si_type),
					(uint64_t) status_desc, err);
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
compress_read_status(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl	*dst_sgl;
	struct chain_sgl_pdma_tuple *tuple;
	struct pnso_compression_header *cp_hdr = NULL;
	uint64_t cp_hdr_pa;
	uint32_t datain_len;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

	OSAL_LOG_DEBUG("sbi_num_entries: %d sbi_bulk_desc_idx: %d sbi_desc_idx: %d",
			svc_info->si_batch_info.sbi_num_entries,
			svc_info->si_batch_info.sbi_bulk_desc_idx,
			svc_info->si_batch_info.sbi_desc_idx);

	err = cpdc_common_read_status(cp_desc, status_desc);
	if (err)
		goto out;

	/* TODO-cp: handle bypass on fail flag */

	if (cp_desc->u.cd_bits.cc_enabled &&
			cp_desc->u.cd_bits.cc_insert_header) {
		dst_sgl = svc_info->si_dst_sgl.sgl;

		if (svc_info->si_sgl_pdma) {
			tuple = &svc_info->si_sgl_pdma->tuple[0];
			if (tuple->len >= sizeof (*cp_hdr))
				cp_hdr = sonic_phy_to_virt(tuple->addr);
		} else if (svc_info->si_dst_blist.type ==
				SERVICE_BUF_LIST_TYPE_HOST) {
			dst_sgl = svc_info->si_dst_sgl.sgl;
			cp_hdr_pa = sonic_devpa_to_hostpa(dst_sgl->cs_addr_0);
			cp_hdr = (struct pnso_compression_header *)
				sonic_phy_to_virt(cp_hdr_pa);
		}

		if (!cp_hdr) {
			OSAL_LOG_DEBUG("skip cp header checks");
			goto done;
		}

		OSAL_LOG_DEBUG("compress_read_status: dst_sgl=0x" PRIx64 ", cs_addr_0=0x" PRIx64 ", cp_hdr-0x" PRIx64 "\n",
			       (uint64_t) dst_sgl,
			       (uint64_t) dst_sgl->cs_addr_0,
			       (uint64_t) cp_hdr);

		/* TODO-cp: verify hard-coded CP version, etc. */

		if (cp_hdr->chksum == 0) {
			err = EINVAL;	/* PNSO_ERR_CPDC_CHECKSUM_FAILED?? */
			OSAL_LOG_ERROR("zero checksum in header! err: %d",
					CP_STATUS_CHECKSUM_FAILED);
			goto out;
		}

		datain_len = cp_desc->cd_datain_len == 0 ?
			MAX_CPDC_SRC_BUF_LEN : cp_desc->cd_datain_len;
		if ((cp_hdr->data_len == 0) ||
				(cp_hdr->data_len > datain_len)) {
			err = EINVAL;
			OSAL_LOG_ERROR("invalid data len! datain_len: %u hdr_len: %u err: %d",
					datain_len, cp_hdr->data_len, err);
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
	}

done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
compress_write_result(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid cp status desc! err: %d", err);
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
	chn_service_deps_data_len_set(svc_info, status_desc->csd_output_data_len);
	PAS_INC_NUM_CP_BYTES_OUT(svc_info->si_pcr,
			status_desc->csd_output_data_len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
compress_teardown(struct service_info *svc_info)
{
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	seq_cleanup_cpdc_chain(svc_info);
	seq_cleanup_desc(svc_info);

	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);

	cpdc_teardown_rmem_dst_blist(svc_info);
	cpdc_teardown_rmem_status_desc(svc_info, false);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(svc_info->si_pcr, false, status_desc);

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
	.disable_interrupt = compress_disable_interrupt,
	.ring_db = compress_ring_db,
	.poll = compress_poll,
	.read_status = compress_read_status,
	.write_result = compress_write_result,
	.teardown = compress_teardown
};
