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
is_chksum_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_CHKSUM_TYPE_MCRC64:
	case PNSO_CHKSUM_TYPE_CRC32C:
	case PNSO_CHKSUM_TYPE_ADLER32:
	case PNSO_CHKSUM_TYPE_MADLER32:
		return true;
	default:
		return false;
	}

	return false;
}

static inline bool
is_chksum_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

static inline bool
is_dflag_per_block_enabled(uint16_t flags)
{
	return (flags & PNSO_CHKSUM_DFLAG_PER_BLOCK) ? true : false;
}

static bool
is_chksum_desc_valid(const struct pnso_checksum_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_chksum_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid chksum algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_chksum_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid chksum flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_INFO("chksum desc is valid algo_type: %hu flags: %hu",
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

	if (!svc_params->u.sp_chksum_desc) {
		OSAL_LOG_ERROR("invalid desc specified! desc: 0x%llx err: %d",
				(uint64_t) svc_params->u.sp_chksum_desc, err);
		return err;
	}

	return PNSO_OK;
}

static void
fill_chksum_desc(uint32_t algo_type, uint32_t buf_len,
		bool flat_buf, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) sonic_virt_to_phy(src_buf);

	desc->u.cd_bits.cc_enabled = 0;
	desc->u.cd_bits.cc_integrity_src = 1;
	desc->u.cd_bits.cc_src_is_list = flat_buf ? 0 : 1;
	switch (algo_type) {
	case PNSO_CHKSUM_TYPE_MCRC64:
		desc->u.cd_bits.cc_integrity_type = 1;
		break;
	case PNSO_CHKSUM_TYPE_CRC32C:
		desc->u.cd_bits.cc_integrity_type = 2;
		break;
	case PNSO_CHKSUM_TYPE_ADLER32:
		desc->u.cd_bits.cc_integrity_type = 3;
		break;
	case PNSO_CHKSUM_TYPE_MADLER32:
		desc->u.cd_bits.cc_integrity_type = 4;
		break;
	default:
		OSAL_ASSERT(0);
		break;
	}

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
	struct per_core_resource *pc_res;
	size_t src_blist_len;
	bool per_block;
	uint16_t flags;
	uint32_t num_tags;

	OSAL_LOG_DEBUG("enter ...");

	err = CPDC_VALIDATE_SETUP_INPUT(svc_info, svc_params);
	if (err)
		goto out;

	pnso_chksum_desc =
		(struct pnso_checksum_desc *) svc_params->u.sp_chksum_desc;
	if (!is_chksum_desc_valid(pnso_chksum_desc)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid chksum desc specified! err: %d", err);
		goto out;
	}
	flags = pnso_chksum_desc->flags;
	per_block = is_dflag_per_block_enabled(flags);

	pc_res = svc_info->si_pc_res;
	chksum_desc = cpdc_get_desc(pc_res, per_block);
	if (!chksum_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum desc from pool err: %d!",
				err);
		goto out;
	}

	sgl = cpdc_get_sgl(pc_res, per_block);
	if (!sgl) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum sgl from pool! err: %d",
					err);
		goto out_chksum_desc;
	}

	status_desc = cpdc_get_status_desc(pc_res, per_block);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain chksum status desc from pool! err: %d",
				err);
		goto out_sgl_desc;
	}

	src_blist_len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (per_block) {
		num_tags =
			cpdc_fill_per_block_desc(pnso_chksum_desc->algo_type,
					svc_info->si_block_size, src_blist_len,
					svc_params->sp_src_blist, sgl,
					chksum_desc, status_desc,
					fill_chksum_desc);
	} else {
		err = cpdc_update_service_info_sgl(svc_info, svc_params);
		if (err) {
			OSAL_LOG_ERROR("cannot obtain chksum src sgl from pool! err: %d",
					err);
			goto out_status_desc;
		}

		fill_chksum_desc(pnso_chksum_desc->algo_type, src_blist_len,
				false, svc_info->si_src_sgl,
				chksum_desc, status_desc);
		num_tags = 1;
	}

	svc_info->si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = chksum_desc;
	svc_info->si_status_desc = status_desc;
	svc_info->si_num_tags = num_tags;
	svc_info->si_p4_sgl = sgl;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		if (num_tags) {
			svc_info->si_seq_info.sqi_batch_mode = true;
			svc_info->si_seq_info.sqi_batch_size = num_tags;
		}
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				chksum_desc, sizeof(*chksum_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d",
					err);
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
		OSAL_LOG_ERROR("failed to return chksum sgl to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out_chksum_desc:
	err = cpdc_put_desc(pc_res, per_block, chksum_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return chksum desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
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
chksum_schedule(const struct service_info *svc_info)
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
chksum_poll(const struct service_info *svc_info)
{
	volatile struct cpdc_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	OSAL_ASSERT(status_desc);

	while (status_desc->csd_valid == 0)
		osal_yield();

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
}

static pnso_error_t
chksum_read_status_per_block(const struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
		OSAL_ASSERT(err);
	}
	st_desc = status_desc;

	status_object_size = cpdc_get_status_desc_size();
	svc_status->u.chksum.num_tags = svc_info->si_num_tags;

	OSAL_LOG_INFO("block_size: %d object_size: %d si_num_tags: %d",
			svc_info->si_block_size, status_object_size,
			svc_info->si_num_tags);

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
chksum_read_status_buffer(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	struct pnso_service_status *svc_status;

	OSAL_LOG_DEBUG("enter ...");

	chksum_desc = (struct cpdc_desc *) svc_info->si_desc;
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	svc_status = svc_info->si_svc_status;

	err = cpdc_common_read_status(chksum_desc, status_desc);
	if (err)
		goto out;

	svc_status->u.chksum.num_tags = svc_info->si_num_tags;
	OSAL_LOG_DEBUG("exit! status verification success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
chksum_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	err = per_block ? chksum_read_status_per_block(svc_info) :
		chksum_read_status_buffer(svc_info);

	if (!err) {
		OSAL_LOG_DEBUG("exit!");
	} else {
		OSAL_LOG_ERROR("exit! err: %d", err);
	}
	return err;
}

static pnso_error_t
chksum_write_result_per_block(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
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

	svc_status->u.chksum.num_tags = svc_info->si_num_tags;
	OSAL_ASSERT(svc_status->u.chksum.num_tags >= 1);

	status_object_size = cpdc_get_status_desc_size();
	st_desc = status_desc;
	for (i = 0; i < svc_info->si_num_tags; i++) {
		memcpy(svc_status->u.chksum.tags[i].chksum,
			&status_desc->csd_integrity_data,
			PNSO_CHKSUM_TAG_LEN);

		CPDC_PPRINT_STATUS_DESC(st_desc);
		OSAL_LOG_INFO("tag: %d status_desc: 0x%llx chksum: %*phN",
			i, (uint64_t) st_desc, 8,
			svc_status->u.chksum.tags[i].chksum);

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
chksum_write_result_buffer(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid chksum status desc! err: %d", err);
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

	svc_status->u.chksum.num_tags = svc_info->si_num_tags;
	OSAL_ASSERT(svc_info->si_num_tags == 1);

	memcpy(svc_status->u.chksum.tags[0].chksum,
			&status_desc->csd_integrity_data,
			PNSO_CHKSUM_TAG_LEN);

	OSAL_LOG_INFO("tag: 0 status_desc: 0x%llx chksum: %*phN",
			(uint64_t) status_desc, 8,
			svc_status->u.chksum.tags[0].chksum);

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
	pnso_error_t err;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	err = per_block ? chksum_write_result_per_block(svc_info) :
		chksum_write_result_buffer(svc_info);

	if (!err) {
		OSAL_LOG_DEBUG("exit!");
	} else {
		OSAL_LOG_ERROR("exit! err: %d", err);
	}
	return err;
}

static void
chksum_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl *sgl;
	struct per_core_resource *pc_res;
	bool per_block;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(svc_info);

	per_block = is_dflag_per_block_enabled(svc_info->si_desc_flags);
	OSAL_LOG_DEBUG("chksum_desc: %p flags: %d", svc_info->si_desc,
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

	chksum_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = cpdc_put_desc(pc_res, per_block, chksum_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return chksum desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops chksum_ops = {
	.setup = chksum_setup,
	.chain = chksum_chain,
	.schedule = chksum_schedule,
	.poll = chksum_poll,
	.read_status = chksum_read_status,
	.write_result = chksum_write_result,
	.teardown = chksum_teardown
};
