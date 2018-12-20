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

static void
fill_chksum_desc(uint32_t algo_type, uint32_t buf_len,
		bool flat_buf, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_buf);

	desc->u.cd_bits.cc_integrity_src = 1;
	desc->u.cd_bits.cc_src_is_list = flat_buf ? 0 : 1;

	/*
	 * due to PNSO_CHKSUM_TYPE_NONE, subtract by 1 to align with
	 * HW/SW constants
	 *
	 */
	desc->u.cd_bits.cc_integrity_type = algo_type - 1;

	desc->cd_datain_len = buf_len;

	desc->cd_status_addr = (uint64_t) sonic_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_CHKSUM_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
chksum_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_checksum_desc *pnso_chksum_desc;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl;
	bool per_block;
	uint32_t num_tags;

	OSAL_LOG_DEBUG("enter ...");

	pnso_chksum_desc =
		(struct pnso_checksum_desc *) svc_params->u.sp_chksum_desc;
	per_block = svc_is_chksum_per_block_enabled(pnso_chksum_desc->flags);

	chksum_desc = cpdc_get_desc(svc_info, per_block);
	if (!chksum_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum desc from pool err: %d!",
				err);
		goto out;
	}
	svc_info->si_desc = chksum_desc;

	sgl = cpdc_get_sgl(svc_info->si_pcr, per_block);
	if (!sgl) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum sgl from pool! err: %d",
					err);
		goto out;
	}
	svc_info->si_p4_sgl = sgl;

	status_desc = cpdc_get_status_desc(svc_info->si_pcr, per_block);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum status desc from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_status_desc = status_desc;

	if (per_block) {
		num_tags =
			cpdc_fill_per_block_desc(pnso_chksum_desc->algo_type,
					svc_info->si_block_size,
					svc_info->si_src_blist.len,
					svc_info->si_src_blist.blist, sgl,
					chksum_desc, status_desc,
					fill_chksum_desc);
	} else {
		err = cpdc_update_service_info_sgl(svc_info);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain chksum src sgl from pool! err: %d",
					err);
			goto out;
		}

		fill_chksum_desc(pnso_chksum_desc->algo_type,
				svc_info->si_src_blist.len, false,
				svc_info->si_src_sgl.sgl,
				chksum_desc, status_desc);
		num_tags = 1;
	}
	svc_info->si_num_tags = num_tags;
	cpdc_update_batch_tags(svc_info, num_tags);

	chn_service_hw_ring_take_set(svc_info, num_tags);

	err = cpdc_setup_seq_desc(svc_info, chksum_desc, num_tags);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out;
	}
	PAS_INC_NUM_CHKSUM_REQUESTS(svc_info->si_pcr);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
chksum_chain(struct chain_entry *centry)
{
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		OSAL_LOG_ERROR("failed to chain err: %d", err);
		goto out;
	}

out:
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
chksum_sub_chain_from_cpdc(struct service_info *svc_info,
			   struct cpdc_chain_params *cpdc_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_sub_chain_from_crypto(struct service_info *svc_info,
			     struct crypto_chain_params *crypto_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static void
chksum_disable_interrupt(struct service_info *svc_info)
{
	return cpdc_cleanup_interrupt_params(svc_info);
}

static pnso_error_t
chksum_ring_db(struct service_info *svc_info)
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

		/* in batch mode, seq desc needs an update to its batch size */
		cpdc_update_seq_batch_size(svc_info);

		seq_info = &svc_info->si_seq_info;
		seq_ring_db(svc_info);

		err = PNSO_OK;
	}

	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
chksum_poll(const struct service_info *svc_info)
{
	return cpdc_poll(svc_info);
}

static pnso_error_t
chksum_read_status(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc, *st_desc;
	uint32_t i, status_object_size, orig_num_tags;

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
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
		OSAL_ASSERT(!err);
		goto out;
	}
	st_desc = status_desc;
	status_object_size = cpdc_get_status_desc_size();

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
	if (svc_is_chksum_per_block_enabled(svc_info->si_desc_flags)) {
		svc_info->si_num_tags = chn_service_deps_num_blks_get(svc_info);
		OSAL_ASSERT(svc_info->si_num_tags >= 1);
	} else
		OSAL_ASSERT(svc_info->si_num_tags == 1);

	OSAL_LOG_INFO("block_size: %d object_size: %d new num_tags: %d old num_tags: %d",
			svc_info->si_block_size, status_object_size,
			svc_info->si_num_tags, orig_num_tags);

	for (i = 0; i < svc_info->si_num_tags; i++) {

		CPDC_PPRINT_STATUS_DESC(st_desc);

		if (!st_desc->csd_valid) {
			svc_status->err = err;
			OSAL_LOG_ERROR("valid bit not set! err: %d", err);
			goto out;
		}

		if (st_desc->csd_err) {
			svc_status->err =
				cpdc_convert_desc_error(st_desc->csd_err);
			OSAL_LOG_ERROR("service failed! err: %d", err);
			err = svc_status->err;
			goto out;
		}

		st_desc = cpdc_get_next_status_desc(st_desc,
				status_object_size);
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
chksum_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc, *st_desc;
	uint32_t i, status_object_size;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	svc_status = svc_info->si_svc_status;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
		OSAL_ASSERT(!err);
		goto out;
	}

	OSAL_ASSERT(svc_is_chksum_per_block_enabled(svc_info->si_desc_flags) ?
                    svc_info->si_num_tags >= 1 : svc_info->si_num_tags == 1);
	svc_status->u.chksum.num_tags = svc_info->si_num_tags;

	status_object_size = cpdc_get_status_desc_size();
	st_desc = status_desc;
	for (i = 0; i < svc_info->si_num_tags; i++) {
		memcpy(svc_status->u.chksum.tags[i].chksum,
			&status_desc->csd_integrity_data,
			PNSO_CHKSUM_TAG_LEN);

		CPDC_PPRINT_STATUS_DESC(st_desc);
		OSAL_LOG_INFO("tag: %d status_desc: 0x" PRIx64 " chksum: %*phN",
			i, (uint64_t) st_desc, 8,
			svc_status->u.chksum.tags[i].chksum);

		st_desc = cpdc_get_next_status_desc(st_desc,
				status_object_size);
	}
	PAS_INC_NUM_CHKSUMS(svc_info->si_pcr, svc_info->si_num_tags);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
chksum_teardown(struct service_info *svc_info)
{
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	/*
	 * Trace the dst/SGL once more to verify any padding applied
	 * by sequencer.
	 */
	CPDC_PPRINT_DESC(svc_info->si_desc);

	seq_cleanup_desc(svc_info);

	per_block = svc_is_chksum_per_block_enabled(svc_info->si_desc_flags);
	if (!per_block) {
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);
	}

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(svc_info->si_pcr, per_block, status_desc);

	sgl = (struct cpdc_sgl *) svc_info->si_p4_sgl;
	cpdc_put_sgl(svc_info->si_pcr, per_block, sgl);

	chksum_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, per_block, chksum_desc);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops chksum_ops = {
	.setup = chksum_setup,
	.chain = chksum_chain,
	.sub_chain_from_cpdc = chksum_sub_chain_from_cpdc,
	.sub_chain_from_crypto = chksum_sub_chain_from_crypto,
	.enable_interrupt = chksum_enable_interrupt,
	.disable_interrupt = chksum_disable_interrupt,
	.ring_db = chksum_ring_db,
	.poll = chksum_poll,
	.read_status = chksum_read_status,
	.write_result = chksum_write_result,
	.teardown = chksum_teardown
};
