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
is_dflag_pblock_enabled(uint16_t flags)
{
	return (flags & PNSO_HASH_DFLAG_PER_BLOCK) ? true : false;
}

static void
fill_hash_desc(uint32_t algo_type, uint32_t buf_len, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_buf);

	desc->u.cd_bits.cc_enabled = 0;
	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_hash_enabled = 1;

	/*
	 * due to PNSO_HASH_TYPE_NONE, subtract by 1 to align with
	 * HW/SW constants
	 *
	 */
	desc->u.cd_bits.cc_hash_type = algo_type - 1;

	desc->cd_datain_len = 
		cpdc_desc_data_len_set_eval(PNSO_SVC_TYPE_HASH, buf_len);

	desc->cd_status_addr = (uint64_t) sonic_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_HASH_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
hash_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_hash_desc *pnso_hash_desc;
	struct cpdc_desc *hash_desc, *bof_hash_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl, *bof_sgl;
	bool per_block;
	uint32_t num_tags;

	OSAL_LOG_DEBUG("enter ...");

	pnso_hash_desc = (struct pnso_hash_desc *) svc_params->u.sp_hash_desc;
	per_block = is_dflag_pblock_enabled(pnso_hash_desc->flags);

	if (svc_params->sp_bof_blist)
		svc_info->si_flags |= CHAIN_SFLAG_BYPASS_ONFAIL;

	hash_desc = cpdc_get_desc(svc_info, per_block);
	if (!hash_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash desc from pool err: %d!",
				err);
		goto out;
	}
	svc_info->si_desc = hash_desc;

	sgl = cpdc_get_sgl(svc_info->si_pcr, per_block);
	if (!sgl) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash sgl from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_p4_sgl = sgl;

	bof_sgl = cpdc_get_sgl(svc_info->si_pcr, per_block);
	if (!bof_sgl) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain bypass-hash sgl from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_p4_bof_sgl = bof_sgl;

	status_desc = cpdc_get_status_desc(svc_info->si_pcr, per_block);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash status desc from pool! err: %d",
				err);
		goto out;
	}
	svc_info->si_status_desc = status_desc;

	if (per_block) {
		num_tags = cpdc_fill_per_block_desc(pnso_hash_desc->algo_type,
				svc_info->si_block_size,
				svc_info->si_src_blist.len,
				&svc_info->si_src_blist, sgl,
				hash_desc, status_desc, fill_hash_desc);
		if (num_tags == 0) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup hash per-block desc! err: %d",
					err);
			goto out;
		}

		if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
			bof_hash_desc =
				(struct cpdc_desc *) ((char *) hash_desc +
				((sizeof(struct cpdc_desc) * num_tags)));
			OSAL_LOG_DEBUG("num_tags: %d hash_desc: 0x" PRIx64 " bof_hash_desc: 0x" PRIx64,
					num_tags, (uint64_t) hash_desc,
					(uint64_t) bof_hash_desc);

			num_tags = cpdc_fill_per_block_desc(
					pnso_hash_desc->algo_type,
					svc_info->si_block_size,
					svc_info->si_bof_blist.len,
					&svc_info->si_bof_blist,
					svc_info->si_p4_bof_sgl,
					bof_hash_desc, status_desc,
					fill_hash_desc);
			if (num_tags == 0) {
				err = EINVAL;
				OSAL_LOG_ERROR("failed to setup hash bypass onfail per-block desc! err: %d",
						err);
				goto out;
			}
		}
	} else {
		err = cpdc_update_service_info_sgl(svc_info);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain hash src sgl from pool! err: %d",
					err);
			goto out;
		}

		fill_hash_desc(pnso_hash_desc->algo_type,
				svc_info->si_src_blist.len,
				svc_info->si_src_sgl.sgl,
				hash_desc, status_desc);

		if (svc_info->si_flags & CHAIN_SFLAG_BYPASS_ONFAIL) {
			bof_hash_desc =
				(struct cpdc_desc *) ((char *) hash_desc +
						sizeof(struct cpdc_desc));
			OSAL_LOG_DEBUG("hash_desc: 0x" PRIx64 " bof_hash_desc: 0x" PRIx64,
					(uint64_t) hash_desc,
					(uint64_t) bof_hash_desc);

			err = cpdc_update_service_info_bof_sgl(svc_info);
			if (err) {
				OSAL_LOG_ERROR("cannot obtain hash src bof sgl from pool! err: %d",
						err);
				goto out;
			}

			fill_hash_desc(pnso_hash_desc->algo_type,
					svc_info->si_bof_blist.len,
					svc_info->si_bof_sgl.sgl,
					bof_hash_desc, status_desc);
		}

		num_tags = 1;
	}
	svc_info->si_num_tags = num_tags;
	cpdc_update_batch_tags(svc_info, num_tags);

	chn_service_hw_ring_take_set(svc_info, num_tags);

	err = cpdc_setup_seq_desc(svc_info, hash_desc, num_tags);
	if (err) {
		OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
		goto out;
	}
	PAS_INC_NUM_HASH_REQUESTS(svc_info->si_pcr);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_chain(struct chain_entry *centry)
{
	pnso_error_t err;
	struct service_info *svc_info;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(centry);
	svc_info = &centry->ce_svc_info;

	if (!chn_service_has_sub_chain(svc_info)) {
		OSAL_LOG_DEBUG("lone or last service, chaining not needed! si_type: %d si_flags: %d",
				svc_info->si_type, svc_info->si_flags);
		goto done;
	}

	/*
	 * This is supportable when there's a valid use case.
	 */
	err = EOPNOTSUPP;
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
hash_sub_chain_from_cpdc(struct service_info *svc_info,
			 struct cpdc_chain_params *cpdc_chain)
{
	pnso_error_t err;
	struct cpdc_desc *hash_desc;

	OSAL_LOG_DEBUG("enter ...");

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = seq_setup_hash_chain_params(cpdc_chain, svc_info, hash_desc,
			svc_info->si_p4_sgl, svc_info->si_num_tags);
	if (err) {
		OSAL_LOG_ERROR("failed to setup hash in chain! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_DESC(hash_desc);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_sub_chain_from_crypto(struct service_info *svc_info,
			   struct crypto_chain_params *crypto_chain)
{
	/*
	 * This is supportable when there's a valid use case.
	 */
	return EOPNOTSUPP;
}

static pnso_error_t
hash_enable_interrupt(struct service_info *svc_info, void *poll_ctx)
{
	return cpdc_setup_interrupt_params(svc_info, poll_ctx);
}

static void
hash_disable_interrupt(struct service_info *svc_info)
{
	return cpdc_cleanup_interrupt_params(svc_info);
}

static pnso_error_t
hash_ring_db(struct service_info *svc_info)
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
hash_poll(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_status_desc *st_desc;
	uint32_t status_object_size;

	err = cpdc_poll(svc_info, NULL);
	if (err != PNSO_OK)
		return err;

	st_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!st_desc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
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
hash_read_status(struct service_info *svc_info)
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

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
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
			OSAL_LOG_ERROR("valid bit not set! status_desc: 0x" PRIx64 " err: %d",
					(uint64_t) status_desc, err);
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
hash_write_result(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
		OSAL_ASSERT(!err);
		goto out;
	}

	OSAL_ASSERT(is_dflag_pblock_enabled(svc_info->si_desc_flags) ?
		    svc_info->si_num_tags >= 1 : svc_info->si_num_tags == 1);
	svc_status->u.hash.num_tags = svc_info->si_num_tags;

	status_object_size = cpdc_get_status_desc_size();
	st_desc = status_desc;
	for (i = 0; i < svc_info->si_num_tags; i++) {
		memcpy(svc_status->u.hash.tags[i].hash,
				st_desc->csd_sha,
				PNSO_HASH_TAG_LEN);

		CPDC_PPRINT_STATUS_DESC(st_desc);
		OSAL_LOG_INFO("tag: %d status_desc: 0x" PRIx64 " hash: %*phN",
			i, (uint64_t) status_desc, 64,
			svc_status->u.hash.tags[i].hash);

		st_desc = cpdc_get_next_status_desc(st_desc,
				status_object_size);
	}
	PAS_INC_NUM_HASHES(svc_info->si_pcr, svc_info->si_num_tags);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
hash_teardown(struct service_info *svc_info)
{
	struct cpdc_desc *hash_desc;
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

	per_block = is_dflag_pblock_enabled(svc_info->si_desc_flags);
	if (!per_block) {
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_bof_sgl);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_dst_sgl);
		pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);
	}

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	cpdc_put_status_desc(svc_info->si_pcr, per_block, status_desc);

	sgl = (struct cpdc_sgl *) svc_info->si_p4_sgl;
	cpdc_put_sgl(svc_info->si_pcr, per_block, sgl);

	sgl = (struct cpdc_sgl *) svc_info->si_p4_bof_sgl;
	cpdc_put_sgl(svc_info->si_pcr, per_block, sgl);

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	cpdc_put_desc(svc_info, per_block, hash_desc);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops hash_ops = {
	.setup = hash_setup,
	.chain = hash_chain,
	.sub_chain_from_cpdc = hash_sub_chain_from_cpdc,
	.sub_chain_from_crypto = hash_sub_chain_from_crypto,
	.enable_interrupt = hash_enable_interrupt,
	.disable_interrupt = hash_disable_interrupt,
	.ring_db = hash_ring_db,
	.poll = hash_poll,
	.read_status = hash_read_status,
	.write_result = hash_write_result,
	.teardown = hash_teardown
};
