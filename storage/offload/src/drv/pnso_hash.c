/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_seq.h"

#ifdef NDEBUG
#define CPDC_PPRINT_STATUS_DESC(d)
#define CPDC_VALIDATE_SETUP_INPUT(i, p)	PNSO_OK
#else
#define CPDC_PPRINT_STATUS_DESC(d)	cpdc_pprint_status_desc(d)
#define CPDC_VALIDATE_SETUP_INPUT(i, p)	validate_setup_input(i, p)
#endif

static inline bool
is_hash_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
	case PNSO_HASH_TYPE_SHA2_256:
		return true;
	default:
		return false;
	}

	return false;
}

static inline bool
is_hash_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

static inline bool
is_dflag_per_block_enabled(uint16_t flags)
{
	return (flags & PNSO_HASH_DFLAG_PER_BLOCK) ? true : false;
}

static bool
is_hash_desc_valid(const struct pnso_hash_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_hash_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid hash algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_hash_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid hash flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_INFO("hash desc is valid algo_type: %hu flags: %hu",
			desc->algo_type, desc->flags);

	return true;
}

static pnso_error_t __attribute__((unused))
validate_setup_input(const struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err = EINVAL;
	size_t len;

	if (!svc_info || !svc_params) {
		OSAL_LOG_ERROR("invalid input specified! svc_info: %p svc_params: %p err: %d",
				svc_info, svc_params, err);
		return err;
	}

	if (!svc_params->sp_src_blist || !svc_params->sp_dst_blist) {
		OSAL_LOG_ERROR("invalid src/dst buffers specified! sp_src_blist: %p sp_dst_blist: %p err: %d",
				svc_params->sp_src_blist, svc_params->sp_dst_blist,
				err);
		return err;
	}

	len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (len == 0 || len > MAX_CPDC_SRC_BUF_LEN) {
		OSAL_LOG_ERROR("invalid src buf len specified! len: %zu err: %d",
				len, err);
		return err;
	}

	if (!svc_params->u.sp_hash_desc) {
		OSAL_LOG_ERROR("invalid desc specified! desc: 0x%llx err: %d",
				(uint64_t) svc_params->u.sp_hash_desc, err);
		return err;
	}

	return PNSO_OK;
}

static void
fill_hash_desc(uint32_t algo_type, uint32_t buf_len,
		bool flat_buf, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) osal_virt_to_phy(src_buf);

	desc->u.cd_bits.cc_enabled = 0;
	desc->u.cd_bits.cc_src_is_list = flat_buf ? 0 : 1;
	desc->u.cd_bits.cc_hash_enabled = 1;
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
		desc->u.cd_bits.cc_hash_type = 0;
		break;
	case PNSO_HASH_TYPE_SHA2_256:
		desc->u.cd_bits.cc_hash_type = 1;
		break;
	default:
		OSAL_ASSERT(0);
		break;
	}

	desc->cd_datain_len = buf_len;
	desc->cd_status_addr = (uint64_t) osal_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_HASH_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
hash_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_hash_desc *pnso_hash_desc;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl;
	struct per_core_resource *pc_res;
	size_t src_blist_len;
	bool per_block;
	uint16_t flags;
	uint32_t num_tags;

	OSAL_LOG_DEBUG("enter ...");

	err = CPDC_VALIDATE_SETUP_INPUT(svc_info, svc_params);
	if (err)
		goto out;

	pnso_hash_desc = (struct pnso_hash_desc *) svc_params->u.sp_hash_desc;
	if (!is_hash_desc_valid(pnso_hash_desc)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid hash desc specified! err: %d", err);
		goto out;
	}
	flags = pnso_hash_desc->flags;
	per_block = is_dflag_per_block_enabled(flags);

	pc_res = svc_info->si_pc_res;
	hash_desc = cpdc_get_desc(pc_res, per_block);
	if (!hash_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash desc from pool err: %d!",
				err);
		goto out;
	}

	/* TODO: needed for p4+ sequencer, avoid using for entire block  */
	sgl = cpdc_get_sgl(pc_res, per_block);
	if (!sgl) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash sgl from pool! err: %d",
					err);
		goto out_hash_desc;
	}

	status_desc = cpdc_get_status_desc(pc_res, per_block);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash status desc from pool! err: %d",
				err);
		goto out_sgl_desc;
	}

	src_blist_len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (per_block) {
		num_tags = cpdc_fill_per_block_desc_ex(pnso_hash_desc->algo_type,
				svc_info->si_block_size, src_blist_len,
				svc_params->sp_src_blist, sgl,
				hash_desc, status_desc, fill_hash_desc);
	} else {
		err = cpdc_update_service_info_sgl(svc_info, svc_params);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain hash src sgl from pool! err: %d",
					err);
			goto out_status_desc;
		}

		fill_hash_desc(pnso_hash_desc->algo_type, src_blist_len, false,
				svc_info->si_src_sgl, hash_desc, status_desc);
		num_tags = 1;
	}

	svc_info->si_type = PNSO_SVC_TYPE_HASH;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = hash_desc;
	svc_info->si_status_desc = status_desc;
	svc_info->si_num_tags = num_tags;
	svc_info->si_p4_sgl = sgl;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		if (num_tags > 1) {
			svc_info->si_seq_info.sqi_batch_mode = true;
			svc_info->si_seq_info.sqi_batch_size = num_tags;
		}
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				hash_desc, sizeof(*hash_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
			goto out_status_desc;
		}
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out_status_desc:
	err = cpdc_put_status_desc(pc_res, per_block, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out_sgl_desc:
	err = cpdc_put_sgl(pc_res, per_block, sgl);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash sgl to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out_hash_desc:
	err = cpdc_put_desc(pc_res, per_block, hash_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_chain(struct chain_entry *centry)
{
	pnso_error_t err;
	struct service_info *svc_info;
	struct cpdc_desc *hash_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(centry);
	svc_info = &centry->ce_svc_info;

	if (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) {
		OSAL_LOG_DEBUG("chaining not needed! si_type: %d si_flags: %d",
				svc_info->si_type, svc_info->si_flags);
		goto done;
	}

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = seq_setup_hash_chain_params(centry, svc_info, hash_desc,
			svc_info->si_p4_sgl, svc_info->si_num_tags);
	if (err) {
		OSAL_LOG_ERROR("failed to setup hash in chain! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_DESC(hash_desc);

done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_schedule(const struct service_info *svc_info)
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
hash_poll(const struct service_info *svc_info)
{
	volatile struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	OSAL_ASSERT(status_desc);

	/*
	 * TODO-chain: poll service op was added for testing sync requests
	 * temporarily; remove/readjust when sync/async/batch logic kicks-in
	 *
	 */
	while (status_desc->csd_valid == 0)
		osal_yield();

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
}

static inline uint32_t
get_num_tags(const struct service_info *svc_info)
{
	struct service_deps *svc_deps;
	uint32_t len, block_size, num_tags;

	/*
	 * first assume that this is a lone-service, and init number of tags
	 * with what was discovered at setup()
	 *
	 */
	num_tags = svc_info->si_num_tags;

	svc_deps = cpdc_get_service_deps(svc_info);
	if (!svc_deps)
		goto out;

	len = svc_deps->sd_dst_data_len;
	if (len == 0)
		goto out;

	block_size = svc_info->si_block_size;
	num_tags = (len + (block_size - 1)) / block_size;

	OSAL_LOG_INFO("block_size: %d len: %d num_tags: %d",
			block_size, len, num_tags);
	return num_tags;

out:
	OSAL_LOG_INFO("lone service ... num_tags: %d", num_tags);
	return num_tags;
}

static pnso_error_t
hash_read_status_per_block(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc, *st_desc;
	uint32_t i, status_object_size, num_tags;

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
		OSAL_ASSERT(err);
	}
	st_desc = status_desc;

	status_object_size = cpdc_get_status_desc_size();
	num_tags = get_num_tags(svc_info);
	svc_status->u.hash.num_tags = num_tags;

	OSAL_LOG_INFO("block_size: %d object_size: %d si_num_tags: %d num_tags: %d",
			svc_info->si_block_size, status_object_size,
			svc_info->si_num_tags, num_tags);

	for (i = 0; i < num_tags; i++) {

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
hash_read_status_buffer(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	struct pnso_service_status *svc_status;

	OSAL_LOG_DEBUG("enter ...");

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	svc_status = svc_info->si_svc_status;

	err = cpdc_common_read_status(hash_desc, status_desc);
	if (err)
		goto out;

	svc_status->u.hash.num_tags = svc_info->si_num_tags;

	OSAL_LOG_DEBUG("exit! status verification success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	err = per_block ? hash_read_status_per_block(svc_info) :
		hash_read_status_buffer(svc_info);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_write_result_per_block(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc, *st_desc;
	uint32_t i, status_object_size, num_tags;

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

	num_tags = svc_status->u.hash.num_tags;
	OSAL_ASSERT(num_tags >= 0);

	status_object_size = cpdc_get_status_desc_size();
	st_desc = status_desc;
	for (i = 0; i < num_tags; i++) {
		memcpy(svc_status->u.hash.tags[i].hash,
				st_desc->csd_sha,
				PNSO_HASH_TAG_LEN);

		CPDC_PPRINT_STATUS_DESC(st_desc);
		OSAL_LOG_INFO("tag: %d status_desc: 0x%llx hash: %*phN",
			i, (uint64_t) status_desc, 64,
			svc_status->u.hash.tags[i].hash);

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
hash_write_result_buffer(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
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

	svc_status->u.hash.num_tags = svc_info->si_num_tags;
	OSAL_ASSERT(svc_info->si_num_tags == 1);

	memcpy(svc_status->u.hash.tags[0].hash,
			status_desc->csd_sha,
			PNSO_HASH_TAG_LEN);

	OSAL_LOG_INFO("tag: 0 status_desc: 0x%llx hash: %*phN",
			(uint64_t) status_desc, 64,
			svc_status->u.hash.tags[0].hash);

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
	pnso_error_t err;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	err = per_block ? hash_write_result_per_block(svc_info) :
		hash_write_result_buffer(svc_info);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
hash_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl;
	struct per_core_resource *pc_res;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	OSAL_LOG_DEBUG("hash_desc: %p flags: %d", svc_info->si_desc,
			svc_info->si_desc_flags);

	if (!per_block) {
		cpdc_release_sgl(svc_info->si_dst_sgl);
		cpdc_release_sgl(svc_info->si_src_sgl);
	}

	pc_res = svc_info->si_pc_res;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	err = cpdc_put_status_desc(pc_res, per_block, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	sgl = (struct cpdc_sgl *) svc_info->si_p4_sgl;
	err = cpdc_put_sgl(pc_res, per_block, sgl);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash sgl to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = cpdc_put_desc(pc_res, per_block, hash_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops hash_ops = {
	.setup = hash_setup,
	.chain = hash_chain,
	.schedule = hash_schedule,
	.poll = hash_poll,
	.read_status = hash_read_status,
	.write_result = hash_write_result,
	.teardown = hash_teardown
};
