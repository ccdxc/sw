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
#define CPDC_VALIDATE_SETUP_INPUT(i, p)	PNSO_OK
#define CPDC_PPRINT_DESC(d)
#define CPDC_PPRINT_STATUS_DESC(d)
#else
#define CPDC_PPRINT_DESC(d)		cpdc_pprint_desc(d)
#define CPDC_PPRINT_STATUS_DESC(d)	cpdc_pprint_status_desc(d)
#define CPDC_VALIDATE_SETUP_INPUT(i, p)	validate_setup_input(i, p)
#endif

static inline bool
is_dc_algo_type_valid(uint16_t algo_type)
{
	return (algo_type == PNSO_COMPRESSION_TYPE_LZRW1A) ? true : false;
}

static inline bool
is_dc_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

static inline void
clear_dc_header_present(uint16_t flags, struct cpdc_desc *desc)
{
	if (!(flags & PNSO_DC_DFLAG_HEADER_PRESENT))
		desc->u.cd_bits.cc_header_present = 0;
}

static bool
is_dc_desc_valid(const struct pnso_decompression_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_dc_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid dc algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_dc_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid dc flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_INFO("decompression desc is valid algo_type: %hu flags: %hu",
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
				svc_params->sp_src_blist, svc_params->sp_dst_blist, err);
		return err;
	}

	len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (len == 0 || len > MAX_CPDC_SRC_BUF_LEN) {
		OSAL_LOG_ERROR("invalid src buf len specified! len: %zu err: %d",
				len, err);
		return err;
	}

	len = pbuf_get_buffer_list_len(svc_params->sp_dst_blist);
	if (len == 0 || len > MAX_CPDC_DST_BUF_LEN) {
		OSAL_LOG_ERROR("invalid dst buf len specified! len: %zu err: %d",
				len, err);
		return err;
	}

	if (!svc_params->u.sp_dc_desc) {
		OSAL_LOG_ERROR("invalid desc specified! sp_desc: %p err: %d",
				svc_params->u.sp_dc_desc, err);
		return err;
	}

	return PNSO_OK;
}

static void
fill_dc_desc(struct cpdc_desc *desc, void *src_buf, void *dst_buf,
		struct cpdc_status_desc *status_desc,
		uint32_t src_buf_len, uint32_t dst_buf_len)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) osal_virt_to_phy(src_buf);
	desc->cd_dst = (uint64_t) osal_virt_to_phy(dst_buf);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_header_present = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(src_buf_len == MAX_CPDC_SRC_BUF_LEN) ? 0 : src_buf_len;
	desc->cd_threshold_len =
		(dst_buf_len == MAX_CPDC_DST_BUF_LEN) ? 0 : dst_buf_len;

	desc->cd_status_addr = (uint64_t) osal_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_DC_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
decompress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_decompression_desc *pnso_dc_desc;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct per_core_resource *pc_res;
	struct mem_pool *cpdc_mpool, *cpdc_status_mpool;
	size_t src_buf_len, dst_buf_len;
	uint16_t flags;

	OSAL_LOG_INFO("enter ...");

	err = CPDC_VALIDATE_SETUP_INPUT(svc_info, svc_params);
	if (err)
		goto out;

	pnso_dc_desc = (struct pnso_decompression_desc *)
		svc_params->u.sp_dc_desc;
	if (!is_dc_desc_valid(pnso_dc_desc)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid dc desc specified! err: %d", err);
		goto out;
	}
	flags = pnso_dc_desc->flags;

	src_buf_len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (src_buf_len == 0 || src_buf_len > MAX_CPDC_SRC_BUF_LEN) {
		OSAL_LOG_ERROR("invalid src buf len specified! src_buf_len: %zu err: %d",
				src_buf_len, err);
		goto out;
	}

	dst_buf_len = pbuf_get_buffer_list_len(svc_params->sp_dst_blist);
	if (dst_buf_len == 0) {
		OSAL_LOG_ERROR("invalid dst  buf len specified! dst_buf_len: %zu err: %d",
				dst_buf_len, err);
		goto out;
	}

	pc_res = svc_info->si_pc_res;
	cpdc_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_DESC];
	dc_desc = (struct cpdc_desc *) mpool_get_object(cpdc_mpool);
	if (!dc_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc desc from pool! err: %d", err);
		goto out;
	}

	cpdc_status_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];
	status_desc = (struct cpdc_status_desc *)
		mpool_get_object(cpdc_status_mpool);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain dc status desc from pool! err: %d",
				err);
		goto out_dc_desc;
	}
	memset(status_desc, 0, sizeof(*status_desc));

	err = cpdc_update_service_info_sgls(svc_info, svc_params);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dc src/dst sgl from pool! err: %d",
				err);
		goto out_status_desc;
	}

	fill_dc_desc(dc_desc, svc_info->si_src_sgl, svc_info->si_dst_sgl,
			status_desc, src_buf_len, dst_buf_len);
	clear_dc_header_present(flags, dc_desc);

	svc_info->si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = dc_desc;
	svc_info->si_status_desc = status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				dc_desc, sizeof(*dc_desc));
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
	err = mpool_put_object(cpdc_status_mpool, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out_dc_desc:
	err = mpool_put_object(cpdc_mpool, dc_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return dc desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
decompress_chain(struct chain_entry *centry)
{
	pnso_error_t err;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		OSAL_LOG_ERROR("failed to chain! err: %d", err);
		goto out;
	}

out:
	OSAL_LOG_INFO("exit!");
	return err;
}

static pnso_error_t
decompress_schedule(const struct service_info *svc_info)
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
decompress_poll(const struct service_info *svc_info)
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
decompress_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid dc status desc! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_STATUS_DESC(status_desc);

	if (!status_desc->csd_valid) {
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	dc_desc = svc_info->si_desc;
	if (!dc_desc) {
		OSAL_LOG_ERROR("invalid dc desc! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != dc_desc->cd_status_data) {
		OSAL_LOG_ERROR("partial data mismatch, expected %u received: %u",
				dc_desc->cd_status_data,
				status_desc->csd_partial_data);
	}

	if (status_desc->csd_err) {
		err = status_desc->csd_err;
		OSAL_LOG_ERROR("hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, err);
		goto out;
	}

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! status verification success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err:%d", err);
	return err;
}

static pnso_error_t
decompress_write_result(struct service_info *svc_info)
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
		OSAL_LOG_ERROR("invalid dc status desc! err: %d", err);
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

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
decompress_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct per_core_resource *pc_res;
	struct mem_pool *cpdc_mpool, *cpdc_status_mpool;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(svc_info);

	cpdc_release_sgl(svc_info->si_dst_sgl);
	cpdc_release_sgl(svc_info->si_src_sgl);

	pc_res = svc_info->si_pc_res;
	cpdc_status_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];
	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	err = mpool_put_object(cpdc_status_mpool, status_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return status desc to pool! status_desc: %p err: %d",
				status_desc, err);
		OSAL_ASSERT(0);
	}

	cpdc_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_DESC];
	dc_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = mpool_put_object(cpdc_mpool, dc_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return dc desc to pool! dc_desc: %p err: %d",
				dc_desc, err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_INFO("exit!");
}

struct service_ops dc_ops = {
	.setup = decompress_setup,
	.chain = decompress_chain,
	.schedule = decompress_schedule,
	.poll = decompress_poll,
	.read_status = decompress_read_status,
	.write_result = decompress_write_result,
	.teardown = decompress_teardown
};
