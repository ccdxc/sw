/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <assert.h>
#define PNSO_ASSERT(x)  assert(x)
#else
#define PNSO_ASSERT(x)
#endif

#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * TODO-hash:
 *	remove attribute unused
 *	compile in kernel
 *	use common routines
 *	checkpatch
 *	memset status desc, desc in fill_cp, fill_hash -- not at the time of obtaining
 */
static inline enum pnso_hash_type
get_hash_algo_type(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
		return PNSO_HASH_TYPE_SHA2_512;
	case PNSO_HASH_TYPE_SHA2_256:
		return PNSO_HASH_TYPE_SHA2_256;
	default:
		PNSO_ASSERT(0);
		break;
	}

	return PNSO_HASH_TYPE_NONE;
}

static inline bool __attribute__((unused))
is_hash_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
	case PNSO_HASH_TYPE_SHA2_256:
		return true;
	default:
		PNSO_ASSERT(0);
		return false;
	}

	return false;
}

static inline bool __attribute__((unused))
is_hash_flags_valid(uint16_t flags)
{
	/* no special checks needed, as hash flags are independent ones today */
	return true;
}

static inline bool __attribute__((unused))
is_dflag_per_block_enabled(uint16_t flags)
{
	return (flags & PNSO_HASH_DFLAG_PER_BLOCK) ? true : false;
}

static bool __attribute__((unused))
is_hash_desc_valid(struct pnso_hash_desc *desc)
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

static void __attribute__((unused))
fill_hash_desc(struct cpdc_desc *desc, void *src_buf, void *dst_buf,
		void *status_buf, uint32_t sbuf_len,
		enum pnso_hash_type algo_type)
{
	memset(desc, 0, sizeof(*desc));

	desc->cd_src = (uint64_t) osal_virt_to_phy(src_buf);
	// desc->cd_dst = (uint64_t) pnso_virt_to_phys(dst_buf);

	desc->u.cd_bits.cc_enabled = 0;

	desc->u.cd_bits.cc_src_is_list = 1;
	// desc->u.cd_bits.cc_dst_is_list = 1;	/* TODO: not needed for hashing?? */

	desc->u.cd_bits.cc_hash_enabled = 1;
	switch (get_hash_algo_type(algo_type)) {
	case PNSO_HASH_TYPE_SHA2_512:
		desc->u.cd_bits.cc_hash_type = 0;
		break;
	case PNSO_HASH_TYPE_SHA2_256:
		desc->u.cd_bits.cc_hash_type = 1;
		break;
	default:
		PNSO_ASSERT(0);	/* TODO: handle error case */
		break;
	}

	desc->cd_datain_len = sbuf_len;
	desc->cd_status_addr = (uint64_t) osal_virt_to_phy(status_buf);
	desc->cd_status_data = 1234;

	cpdc_pprint_desc(desc);
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

	if (!svc_params->sp_src_buf || !svc_params->sp_dst_buf) {
		OSAL_LOG_ERROR("invalid src/dst buffers specified! sp_src_buf: %p sp_dst_buf: %p err: %d",
				svc_params->sp_src_buf, svc_params->sp_dst_buf, err);
		return err;
	}

	len = pbuf_get_buffer_list_len(svc_params->sp_src_buf);
	if (len == 0) {
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

#ifdef NDEBUG
#define CP_VALIDATE_SETUP_INPUT(i, p)	PNSO_OK
#else
#define CP_VALIDATE_SETUP_INPUT(i, p)	validate_setup_input(i, p)
#endif

static pnso_error_t
hash_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_hash_desc *pnso_hash_desc;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	size_t src_buf_len;

	OSAL_LOG_INFO("enter ...");

	err = CP_VALIDATE_SETUP_INPUT(svc_info, svc_params);
	if (err)
		goto out;

	pnso_hash_desc = (struct pnso_hash_desc *) svc_params->u.sp_hash_desc;
	if (!is_hash_desc_valid(pnso_hash_desc)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid hash desc specified! err: %d", err);
		goto out;
	}

	hash_desc = (struct cpdc_desc *) mpool_get_object(cpdc_mpool);
	if (!hash_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp/hash desc from pool err: %d!",
				err);
		goto out;
	}

	status_desc = (struct cpdc_status_desc *)
		mpool_get_object(cpdc_status_mpool);
	if (!status_desc) {
		err = -ENOMEM;
		OSAL_LOG_ERROR("cannot obtain hash status desc from pool! err: %d",
				err);
		goto out_hash_desc;
	}

	err = cpdc_convert_buffer_list_to_sgl(svc_info, svc_params->sp_src_buf,
			svc_params->sp_dst_buf);
	if (err) {
		err = EINVAL;
		OSAL_LOG_ERROR("cannot convert buffer list for hash! err: %d",
				err);
		goto out_status_desc;
	}

	err = cpdc_convert_buffer_list_to_sgl(svc_info, svc_params->sp_src_buf,
			svc_params->sp_dst_buf);
	if (err) {
		err = EINVAL;
		OSAL_LOG_ERROR("cannot obtain hash  src/dst sgl from pool! err: %d",
				err);
		goto out_status_desc;
	}

	svc_info->si_type = PNSO_SVC_TYPE_HASH;
	svc_info->si_desc = hash_desc;
	svc_info->si_status_buf = status_desc;

	src_buf_len = pbuf_get_buffer_list_len(svc_params->sp_src_buf);
	fill_hash_desc(hash_desc, svc_info->si_src_sgl, svc_info->si_dst_sgl,
			status_desc, src_buf_len, pnso_hash_desc->algo_type);

	/* TODO-hash: add seq stuff here */

	err = PNSO_OK;
	OSAL_LOG_INFO("service initialized!");
	OSAL_LOG_INFO("exit!");

	return err;

out_status_desc:
	/* see TODO in cp on cleanup */
	err = mpool_put_object(cpdc_status_mpool, status_desc);
	if (err)
		OSAL_LOG_ERROR("failed to return status desc to pool! err: %d",
				err);
out_hash_desc:
	err = mpool_put_object(cpdc_mpool, hash_desc);
	if (err)
		OSAL_LOG_ERROR("failed to return hash desc to pool! err: %d",
				err);
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_chain(struct chain_entry *centry)
{
	pnso_error_t err;

	OSAL_LOG_INFO("enter ...");

	PNSO_ASSERT(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		OSAL_LOG_INFO("failed to chain err: %d", err);
		goto out;
	}

	/* TODO-cp: anything more? */

out:
	OSAL_LOG_INFO("exit!");
	return err;
}

static pnso_error_t
hash_schedule(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	bool ring_db;

	OSAL_LOG_INFO("enter ... ");

	PNSO_ASSERT(svc_info);

	ring_db = (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
	if (ring_db) {
		OSAL_LOG_INFO("ring door bell <===");
		/* TODO-hash: add ringing the db logic here */
		err = PNSO_OK;
	}

	OSAL_LOG_INFO("exit!");
	return err;
}

static pnso_error_t
hash_poll(const struct service_info *svc_info)
{
	uint32_t i;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	PNSO_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_buf;
	PNSO_ASSERT(status_desc);

#define PNSO_UT_NUM_POLL 5	/* TODO-hash: */
	for (i = 0; i < PNSO_UT_NUM_POLL; i++) {
		OSAL_LOG_INFO("status updated (%d) status_desc: %p",
				i + 1, status_desc);

		if (status_desc->csd_valid) {
			OSAL_LOG_INFO("status updated (%d)", i + 1);
			break;
		}
		osal_yield();
	}

	OSAL_LOG_INFO("exit!");
	return PNSO_OK;
}

static pnso_error_t
hash_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = -EINVAL;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	PNSO_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_buf;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
		goto out;
	}
	cpdc_pprint_status_desc(status_desc);

	if (!status_desc->csd_valid) {
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	/* bail on success */
	if (!status_desc->csd_err) {
		err = PNSO_OK;
		OSAL_LOG_ERROR("no hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, err);
		goto out;
	}

	hash_desc = svc_info->si_desc;
	if (!hash_desc) {
		OSAL_LOG_ERROR("invalid hash desc! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != hash_desc->cd_status_data) {
		OSAL_LOG_ERROR("partial data mismatch, expected %u received: %u err: %d",
				hash_desc->cd_status_data,
				status_desc->csd_partial_data, err);
		goto out;
	}

	err = PNSO_OK;
	OSAL_LOG_INFO("status verification success!");
	OSAL_LOG_INFO("exit!");

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hash_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	PNSO_ASSERT(svc_info);

	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d err: %d",
			svc_status->svc_type,  svc_info->si_type, err);
		goto out;
	}

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_buf;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid hash status desc! err: %d", err);
		/* TODO-hash: need BUG_ON equivalent in OSAL */
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
	OSAL_LOG_INFO("status/result update success!");

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
hash_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	PNSO_ASSERT(svc_info);

	cpdc_release_sgl(svc_info->si_dst_sgl);
	cpdc_release_sgl(svc_info->si_src_sgl);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_buf;
	PNSO_ASSERT(status_desc);
	err = mpool_put_object(cpdc_status_mpool, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! status_desc: %p err: %d",
				status_desc, err);
		PNSO_ASSERT(0);
	}

	hash_desc = svc_info->si_desc;
	PNSO_ASSERT(hash_desc);

	err = mpool_put_object(cpdc_mpool, hash_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return hash desc to pool! hash_desc: %p err: %d",
				hash_desc, err);
		PNSO_ASSERT(0);
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
