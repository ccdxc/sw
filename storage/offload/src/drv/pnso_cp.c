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

#if 0
static inline bool
is_dflag_zero_pad_enabled(uint16_t flags)
{
	return (flags & PNSO_CP_DFLAG_ZERO_PAD) ? true : false;
}
#endif

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

static inline void __attribute__((unused))
clear_list_bits_in_desc(struct cpdc_desc *desc)
{
	desc->u.cd_bits.cc_src_is_list = 0;
	desc->u.cd_bits.cc_dst_is_list = 0;
}

static void
fill_cp_desc(struct cpdc_desc *desc, struct cpdc_sgl *src_sgl,
		struct cpdc_sgl *dst_sgl, struct cpdc_status_desc *status_desc,
		uint32_t src_buf_len, uint16_t threshold_len)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_sgl);
	desc->cd_dst = (uint64_t) sonic_virt_to_phy(dst_sgl);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_insert_header = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(src_buf_len == MAX_CPDC_SRC_BUF_LEN) ? 0 : src_buf_len;
	desc->cd_threshold_len = threshold_len;

	desc->cd_status_addr = (uint64_t) sonic_virt_to_phy(status_desc);
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
	struct per_core_resource *pcr;
	uint16_t flags, threshold_len;

	OSAL_LOG_DEBUG("enter ...");

	pnso_cp_desc = (struct pnso_compression_desc *)
		svc_params->u.sp_cp_desc;
	flags = pnso_cp_desc->flags;
	threshold_len = pnso_cp_desc->threshold_len;

	cp_desc = cpdc_get_desc(svc_info, false);
	if (!cp_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp desc from pool! err: %d", err);
		goto out;
	}

	pcr = svc_info->si_pcr;
	status_desc = cpdc_get_status_desc(pcr, false);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp status desc from pool! err: %d",
				err);
		goto out_cp_desc;
	}

	err = cpdc_update_service_info_sgls(svc_info);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain cp src/dst sgl from pool! err: %d",
				err);
		goto out_status_desc;
	}

	fill_cp_desc(cp_desc, svc_info->si_src_sgl.sgl,
			svc_info->si_dst_sgl.sgl, status_desc,
			svc_info->si_src_blist.len, threshold_len);
	clear_insert_header(flags, cp_desc);
#if 0
	if (is_dflag_zero_pad_enabled(flags)) {
		err = seq_setup_cp_pad_chain_params(svc_info, cp_desc,
				status_desc);
		if (err) {
			OSAL_LOG_ERROR("failed to setup cp/pad params! err: %d",
					err);
			goto out_status_desc;
		}
	}
#endif
	svc_info->si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = cp_desc;
	svc_info->si_status_desc = status_desc;

	err = cpdc_setup_seq_desc(svc_info, cp_desc, 0);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out_status_desc;
	}

	PAS_INC_NUM_CP_REQUESTS(pcr);
	PAS_INC_NUM_CP_BYTES_IN(pcr, svc_info->si_src_blist.len);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out_status_desc:
	cpdc_put_status_desc(pcr, false, status_desc);
out_cp_desc:
	cpdc_put_desc(svc_info, false, cp_desc);
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

	if (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) {
		OSAL_LOG_DEBUG("lone service, chaining not needed! si_type: %d si_flags: %d",
				svc_info->si_type, svc_info->si_flags);
		goto done;
	}

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

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

	svc_info->si_seq_info.sqi_desc = seq_setup_cpdc_chain_desc(svc_info,
			cp_desc, sizeof(*cp_desc));
	if (!svc_info->si_seq_info.sqi_desc) {
		err = EINVAL;
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
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
compress_enable_interrupt(const struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static pnso_error_t
compress_ring_db(const struct service_info *svc_info)
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
compress_poll(const struct service_info *svc_info)
{
	OSAL_ASSERT(svc_info);
	return cpdc_poll(svc_info);
}

static pnso_error_t
compress_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl	*dst_sgl;
	struct pnso_compression_header *cp_hdr;
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
		cp_hdr_pa = sonic_devpa_to_hostpa(dst_sgl->cs_addr_0);
		cp_hdr = (struct pnso_compression_header *)
			sonic_phy_to_virt(cp_hdr_pa);
		OSAL_LOG_DEBUG("compress_read_status: dst_sgl=0x" PRIx64 ", cs_addr_0=0x" PRIx64 ", cp_hdr-0x" PRIx64 "\n",
			       (uint64_t)dst_sgl,
			       (uint64_t)dst_sgl->cs_addr_0,
			       (uint64_t)cp_hdr);

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
	struct service_deps *svc_deps;

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
	PAS_INC_NUM_CP_BYTES_OUT(svc_info->si_pcr,
			status_desc->csd_output_data_len);

	/* next service may need 'len' */
	svc_deps = cpdc_get_service_deps(svc_info);
	if (svc_deps)
		svc_deps->sd_dst_data_len = status_desc->csd_output_data_len;

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
	struct per_core_resource *pcr;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);
	CPDC_PPRINT_DESC(svc_info->si_desc);

	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);

	pcr = svc_info->si_pcr;

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(pcr, false, status_desc);

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, false, cp_desc);

	seq_cleanup_cpdc_chain(svc_info);

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
	.read_status = compress_read_status,
	.write_result = compress_write_result,
	.teardown = compress_teardown
};
