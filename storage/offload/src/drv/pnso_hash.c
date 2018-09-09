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
#define CPDC_PPRINT_DESC(d)
#define CPDC_PPRINT_STATUS_DESC(d)
#define CPDC_VALIDATE_SETUP_INPUT(i, p)	PNSO_OK
#else
#define CPDC_PPRINT_DESC(d)		cpdc_pprint_desc(d)
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

	if (!svc_params->u.sp_cp_desc) {
		OSAL_LOG_ERROR("invalid desc specified! sp_desc: %p err: %d",
				svc_params->u.sp_cp_desc, err);
		return err;
	}

	return PNSO_OK;
}

static void
fill_hash_desc(enum pnso_hash_type algo_type, uint32_t buf_len,
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

static void
fill_hash_desc_per_block(enum pnso_hash_type algo_type,
		uint32_t block_size, uint32_t src_buf_len,
		const struct pnso_flat_buffer *interm_fbuf,
		struct cpdc_desc *hash_desc,
		struct cpdc_status_desc *status_desc)
{
	struct cpdc_desc *desc;
	struct cpdc_status_desc *st_desc;
	char *buf, *obj;
	uint32_t desc_object_size, status_object_size, pad_size;
	uint32_t i, len, block_cnt, buf_len;

	block_cnt = pbuf_get_flat_buffer_block_count(interm_fbuf, block_size);
	desc = hash_desc;
	st_desc = status_desc;

	OSAL_LOG_INFO("block_cnt: %d block_size: %d src_buf_len: %d buf: %llx hash_desc: %p status_desc: %p",
			block_cnt, block_size, src_buf_len, interm_fbuf->buf,
			hash_desc, status_desc);

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	desc_object_size = sizeof(struct cpdc_desc) + pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	status_object_size = sizeof(struct cpdc_status_desc) + pad_size;

	buf_len = src_buf_len;
	for (i = 0; buf_len && (i < block_cnt); i++) {
		buf = (char *) interm_fbuf->buf + (i * block_size);
		len = buf_len > block_size ? block_size : buf_len;

		OSAL_LOG_DEBUG("blk_num: %d buf: %p, len: %d hash_desc: %p status_desc: %p",
			i, buf, len, desc, st_desc);
		fill_hash_desc(algo_type, len, true, buf, desc, st_desc);
		buf_len -= len;

		/* move to next descriptor */
		obj = (char *) desc;
		obj += desc_object_size;
		desc = (struct cpdc_desc *) obj;

		/* move to next status descriptor */
		obj = (char *) st_desc;
		obj += status_object_size;
		st_desc = (struct cpdc_status_desc *) obj;
	}
}

static pnso_error_t
hash_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_hash_desc *pnso_hash_desc;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	struct per_core_resource *pc_res;
	size_t src_blist_len;
	bool per_block;
	uint16_t flags;

	OSAL_LOG_INFO("enter ...");

	/* TODO-hash: validate interm_fbuf */
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

	status_desc = cpdc_get_status_desc(pc_res, per_block);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash status desc from pool! err: %d",
				err);
		goto out_hash_desc;
	}

	if (per_block) {
		err = cpdc_update_service_info_sgls(svc_info, svc_params);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain hash src/dst sgl from pool! err: %d",
					err);
			goto out_status_desc;
		}

		src_blist_len = svc_params->sp_interm_fbuf->len;
		fill_hash_desc_per_block(pnso_hash_desc->algo_type,
				svc_info->si_block_size, src_blist_len,
				svc_info->si_interm_fbuf,
				hash_desc, status_desc);
	} else {
		err = cpdc_update_service_info_sgl(svc_info, 
				svc_params);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain hash src sgl from pool! err: %d",
					err);
			goto out_status_desc;
		}
		src_blist_len =
			pbuf_get_buffer_list_len(svc_params->sp_src_blist);

		fill_hash_desc(pnso_hash_desc->algo_type, src_blist_len, false,
				svc_info->si_src_sgl, hash_desc, status_desc);
	}

	svc_info->si_type = PNSO_SVC_TYPE_HASH;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = hash_desc;
	svc_info->si_status_desc = status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				hash_desc, sizeof(*hash_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d", err);
			goto out_status_desc;
		}
	}

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! service initialized!");
	return err;

out_status_desc:
	err = cpdc_put_status_desc(pc_res, per_block, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! err: %d",
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

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		OSAL_LOG_ERROR("failed to chain err: %d", err);
		goto out;
	}

out:
	OSAL_LOG_INFO("exit!");
	return err;
}

static pnso_error_t
hash_schedule(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	const struct sequencer_info *seq_info;
	bool ring_db;

	OSAL_LOG_INFO("enter ... ");

	OSAL_ASSERT(svc_info);

	ring_db = (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
	if (ring_db) {
		OSAL_LOG_INFO("ring door bell <===");

		seq_info = &svc_info->si_seq_info;
		seq_ring_db(svc_info, seq_info->sqi_index);

		err = PNSO_OK;
	}

	OSAL_LOG_INFO("exit!");
	return err;
}

static pnso_error_t
hash_poll(const struct service_info *svc_info)
{
	volatile struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	OSAL_ASSERT(status_desc);

	while (status_desc->csd_valid == 0)
		osal_yield();

	OSAL_LOG_INFO("exit!");
	return PNSO_OK;
}

static pnso_error_t
hash_read_status_per_block(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
hash_read_status_buffer(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_STATUS_DESC(status_desc);

	if (!status_desc->csd_valid) {
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	hash_desc = svc_info->si_desc;
	if (!hash_desc) {
		OSAL_LOG_ERROR("invalid hash desc! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != hash_desc->cd_status_data) {
		OSAL_LOG_ERROR("partial data mismatch, expected %u received: %u",
				hash_desc->cd_status_data,
				status_desc->csd_partial_data);
	}

	if (status_desc->csd_err) {
		err = status_desc->csd_err;
		OSAL_LOG_ERROR("hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, err);
		goto out;
	}

	/* TODO-hash: verify SHA, etc.  */

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! status verification success!");
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

	OSAL_LOG_INFO("enter ...");

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
	return EOPNOTSUPP;
}

static pnso_error_t
hash_write_result_buffer(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

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

	/* TODO: handle more SHAs/tags instead of one */
	svc_status->u.hash.num_tags = 1;
	memcpy(svc_status->u.hash.tags->hash, status_desc->csd_sha,
			PNSO_HASH_TAG_LEN);

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! status/result update success!");
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

	OSAL_LOG_INFO("enter ...");

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
	struct per_core_resource *pc_res;
	bool per_block;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	OSAL_LOG_INFO("hash_desc: %p flags: %d", svc_info->si_desc,
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

	hash_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = cpdc_put_desc(pc_res, per_block, hash_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_INFO("exit!");
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
