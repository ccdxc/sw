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

static int
fill_chksum_desc(struct service_info *svc_info,
		uint32_t algo_type, uint32_t buf_len,
		void *src_buf,
		struct cpdc_desc *desc, uint32_t block_no)
{
	uint64_t aligned_addr;
	pnso_error_t err;

	memset(desc, 0, sizeof(*desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_buf);

	desc->u.cd_bits.cc_integrity_src = 1;
	desc->u.cd_bits.cc_src_is_list = 1;

	/*
	 * due to PNSO_CHKSUM_TYPE_NONE, subtract by 1 to align with
	 * HW/SW constants
	 *
	 */
	desc->u.cd_bits.cc_integrity_type = algo_type - 1;

	desc->cd_datain_len =
		cpdc_desc_data_len_set_eval(PNSO_SVC_TYPE_CHKSUM, buf_len);
	/*
	 * See comments in chksum_setup() and chksum_chain() regarding
	 * how chksum makes use of rmem status.
	 */
	err = svc_status_desc_addr_get(&svc_info->si_status_desc, block_no,
			&aligned_addr, CPDC_STATUS_MIN_CLEAR_SZ);
	desc->cd_status_addr = aligned_addr;
	if (err)
		goto out;

	if (chn_service_has_interm_status(svc_info) && (block_no == 0)) {
		err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, block_no,
				&aligned_addr, CPDC_STATUS_MIN_CLEAR_SZ);
		desc->cd_status_addr = aligned_addr;
	}

	desc->cd_status_data = CPDC_CHKSUM_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
out:
	return err;
}

static pnso_error_t
chksum_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_checksum_desc *pnso_chksum_desc;
	struct cpdc_desc *chksum_desc;
	struct cpdc_sgl *sgl, *bof_sgl;
	bool per_block;
	uint32_t num_tags;

	OSAL_LOG_DEBUG("enter ...");

	pnso_chksum_desc =
		(struct pnso_checksum_desc *) svc_params->u.sp_chksum_desc;
	per_block = svc_is_chksum_per_block_enabled(pnso_chksum_desc->flags);

	if (svc_params->sp_bof_blist)
		svc_info->si_flags |= CHAIN_SFLAG_BYPASS_ONFAIL;

	if (per_block)
		svc_info->si_flags |= CHAIN_SFLAG_PER_BLOCK;

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

	if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
		bof_sgl = cpdc_get_sgl(svc_info->si_pcr, per_block);
		if (!bof_sgl) {
			err = ENOMEM;
			OSAL_LOG_ERROR("cannot obtain bypass-chksum sgl from pool! err: %d",
					err);
			goto out;
		}
		svc_info->si_p4_bof_sgl = bof_sgl;
	}

	err = cpdc_setup_status_desc(svc_info, per_block);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain chksum status desc from pool! err: %d",
				err);
		goto out;
	}

	/*
	 * NOTE: when chksum has a subchain, rmem status will be used but
	 * only for the 1st chksum, hence the per-block flag is forced to
	 * to false below.
	 * 
	 * See additional comments in chksum_chain().
	 */
	err = cpdc_setup_rmem_status_desc(svc_info, false);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain cp rmem status desc from pool! err: %d",
				err);
		goto out;
	}

	err = cpdc_setup_desc_blocks(svc_info, pnso_chksum_desc->algo_type,
			fill_chksum_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to setup chksum desc block(s)! err: %d",
				err);
		goto out;
	}
	num_tags = svc_info->si_num_tags;

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
	struct service_info		*svc_info = &centry->ce_svc_info;
	struct cpdc_chain_params	*cpdc_chain = &svc_info->si_cpdc_chain;
	struct service_info		*svc_next;
	pnso_error_t			err = PNSO_OK;

	if (chn_service_has_sub_chain(svc_info)) {

		/*
		 * If chksum_setup() had created any rmem status, it would have
		 * done so for only the 1st chksum operation.
		 */
		err = svc_status_desc_addr_get(&svc_info->si_istatus_desc, 0,
				&cpdc_chain->ccp_status_addr_0, 0);
		if (err)
			goto out;
		err = svc_status_desc_addr_get(&svc_info->si_status_desc, 0,
				&cpdc_chain->ccp_status_addr_1, 0);
		if (err)
			goto out;
		cpdc_chain->ccp_status_len = sizeof(struct cpdc_status_desc);

		/*
		 * Chksum does not produce any data output for consumption by
		 * the next service in the chain so we leave
		 * ccpc_stop_chain_on_error at 0. Besides, P4+ chainer would only
		 * examine status for one block (i.e., one operation result).
		 */
		cpdc_chain->ccp_cmd.ccpc_status_dma_en = true;

		svc_next = chn_service_next_svc_get(svc_info);
		OSAL_ASSERT(svc_next);
		err = svc_next->si_ops.sub_chain_from_cpdc(svc_next,
				cpdc_chain);
		/*
		 * Similarly, completion of the 1st chksum should kick
		 * off the next service.
		 */
		if (!err)
			err = seq_setup_cpdc_chain(svc_info,
					svc_info->si_desc);
	}
out:
	if (err)
		OSAL_LOG_ERROR("failed seq_setup_cpdc_chain: err %d", err);

	return err;
}

static pnso_error_t
chksum_sub_chain_from_cpdc(struct service_info *svc_info,
			   struct cpdc_chain_params *cpdc_chain)
{
	pnso_error_t err;
	struct cpdc_desc *chksum_desc;

	OSAL_LOG_DEBUG("enter ...");

	chksum_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = seq_setup_chksum_chain_params(cpdc_chain, svc_info, chksum_desc,
			svc_info->si_p4_sgl, svc_info->si_num_tags);
	if (err) {
		OSAL_LOG_ERROR("failed to setup checksum in chain! err: %d",
				err);
		goto out;
	}
	CPDC_PPRINT_DESC(chksum_desc);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
chksum_sub_chain_from_crypto(struct service_info *svc_info,
			     struct crypto_chain_params *crypto_chain)
{
	crypto_chain->ccp_cmd.ccpc_next_doorbell_en = true;
	crypto_chain->ccp_cmd.ccpc_next_db_action_ring_push = true;
	return ring_spec_info_fill(svc_info->si_seq_info.sqi_ring,
				   &crypto_chain->ccp_ring_spec,
				   svc_info->si_desc, svc_info->si_num_tags);
}

static pnso_error_t
chksum_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	pnso_error_t err;

	err = cpdc_setup_interrupt_params(svc_info, poll_ctx);
	if (err) {
		OSAL_LOG_ERROR("failed to setup chksum interrupt params! err: %d",
				err);
		goto out;
	}

	cpdc_update_bof_interrupt_params(svc_info);
out:
	return err;
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
chksum_poll(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_status_desc *st_desc;
	uint32_t status_object_size;

	err = cpdc_poll(svc_info, NULL);
	if (err != PNSO_OK)
		return err;

	st_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
	if (!st_desc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
		OSAL_ASSERT(!err);
		return err;
	}
	status_object_size = cpdc_get_status_desc_size();

	if (!svc_info->tags_updated)
		cpdc_update_tags(svc_info);

	if (svc_info->si_num_tags > 1) {
		st_desc = cpdc_get_next_status_desc(st_desc,
				status_object_size *
				(svc_info->si_num_tags - 1));
		return cpdc_poll(svc_info, st_desc);
	}

	return err;
}

static pnso_error_t
chksum_read_status(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc, *st_desc;
	uint32_t i, status_object_size;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d err: %d",
			svc_status->svc_type,  svc_info->si_type, err);
		goto out;
	}

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
		OSAL_ASSERT(!err);
		goto out;
	}
	st_desc = status_desc;
	status_object_size = cpdc_get_status_desc_size();

	if (!svc_info->tags_updated)
		cpdc_update_tags(svc_info);

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
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc.desc;
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
	struct cpdc_sgl *sgl;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	/*
	 * Trace the dst/SGL once more to verify any padding applied
	 * by sequencer.
	 */
	CPDC_PPRINT_DESC(svc_info->si_desc);

	seq_cleanup_cpdc_chain(svc_info);
	seq_cleanup_desc(svc_info);

	per_block = svc_is_chksum_per_block_enabled(svc_info->si_desc_flags);
	if (!per_block) {
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_bof_sgl);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);
	}

	cpdc_teardown_rmem_status_desc(svc_info);
	cpdc_teardown_status_desc(svc_info);

	sgl = (struct cpdc_sgl *) svc_info->si_p4_sgl;
	cpdc_put_sgl(svc_info->si_pcr, per_block, sgl);

	if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
		sgl = (struct cpdc_sgl *) svc_info->si_p4_bof_sgl;
		cpdc_put_sgl(svc_info->si_pcr, per_block, sgl);
	}

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
