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
is_cp_algo_type_valid(uint16_t algo_type)
{
	return (algo_type == PNSO_COMPRESSION_TYPE_LZRW1A) ? true : false;
}

static inline bool
is_cp_threshold_len_valid(uint16_t threshold_len)
{
	return (threshold_len > (MAX_CPDC_SRC_BUF_LEN -
			sizeof(struct pnso_compression_header))) ? false : true;
}

static inline bool
is_cp_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

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

static inline void __attribute__((unused))
clear_list_bits_in_desc(struct cpdc_desc *desc)
{
	desc->u.cd_bits.cc_src_is_list = 0;
	desc->u.cd_bits.cc_dst_is_list = 0;
}

static inline void
pad_buffer_with_zeroes(uint16_t flags, struct pnso_buffer_list *buf_list)
{
	if (is_dflag_zero_pad_enabled(flags)) {
		/* TODO-cp: handle padding */
	}
}

static bool
is_compression_desc_valid(const struct pnso_compression_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_cp_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid cp algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_cp_threshold_len_valid(desc->threshold_len)) {
		OSAL_LOG_ERROR("invalid cp threshold len specified! threshold_len: %hu err: %d",
				desc->threshold_len, err);
		return false;
	}

	if (!is_cp_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid cp flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_INFO("compression desc is valid algo_type: %hu threshold_len: %hu flags: %hu",
			desc->algo_type, desc->threshold_len, desc->flags);

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
				svc_params->sp_src_blist,
				svc_params->sp_dst_blist, err);
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
fill_cp_desc(struct cpdc_desc *desc, void *src_buf, void *dst_buf,
		struct cpdc_status_desc *status_desc, uint32_t src_buf_len,
		uint16_t threshold_len)
{
	memset(desc, 0, sizeof(*desc));
	memset(status_desc, 0, sizeof(*status_desc));

	desc->cd_src = (uint64_t) osal_virt_to_phy(src_buf);
	desc->cd_dst = (uint64_t) osal_virt_to_phy(dst_buf);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_insert_header = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len =
		(src_buf_len == MAX_CPDC_SRC_BUF_LEN) ? 0 : src_buf_len;
	desc->cd_threshold_len = threshold_len;

	desc->cd_status_addr = (uint64_t) osal_virt_to_phy(status_desc);
	desc->cd_status_data = CPDC_CP_STATUS_DATA;

	CPDC_PPRINT_DESC(desc);
}

static pnso_error_t
compress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_compression_desc *pnso_cp_desc;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct per_core_resource *pc_res;
	struct mem_pool *cpdc_mpool, *cpdc_status_mpool;
	size_t src_buf_len;
	uint16_t flags, threshold_len;

	OSAL_LOG_INFO("enter ...");

	err = CPDC_VALIDATE_SETUP_INPUT(svc_info, svc_params);
	if (err)
		goto out;

	pnso_cp_desc = (struct pnso_compression_desc *)
		svc_params->u.sp_cp_desc;
	if (!is_compression_desc_valid(pnso_cp_desc)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid cp desc specified! err: %d", err);
		goto out;
	}
	flags = pnso_cp_desc->flags;
	threshold_len = pnso_cp_desc->threshold_len;

	src_buf_len = pbuf_get_buffer_list_len(svc_params->sp_src_blist);
	if (src_buf_len == 0 || src_buf_len > MAX_CPDC_SRC_BUF_LEN) {
		OSAL_LOG_ERROR("invalid src buf len specified! src_buf_len: %zu err: %d",
				src_buf_len, err);
		goto out;
	}

	pc_res = svc_info->si_pc_res;
	cpdc_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_DESC];
	cp_desc = (struct cpdc_desc *) mpool_get_object(cpdc_mpool);
	if (!cp_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp desc from pool! err: %d", err);
		goto out;
	}

	cpdc_status_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];
	status_desc = (struct cpdc_status_desc *)
		mpool_get_object(cpdc_status_mpool);
	if (!status_desc) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain cp status desc from pool! err: %d",
				err);
		goto out_cp_desc;
	}

	err = cpdc_update_service_info_sgls(svc_info, svc_params);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain cp src/dst sgl from pool! err: %d",
				err);
		goto out_status_desc;
	}

	fill_cp_desc(cp_desc, svc_info->si_src_sgl, svc_info->si_dst_sgl,
			status_desc, src_buf_len, threshold_len);
	clear_insert_header(flags, cp_desc);
	pad_buffer_with_zeroes(flags, svc_params->sp_src_blist);

	svc_info->si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info->si_desc_flags = flags;
	svc_info->si_desc = cp_desc;
	svc_info->si_status_desc = status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				cp_desc, sizeof(*cp_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d",
					err);
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
out_cp_desc:
	err = mpool_put_object(cpdc_mpool, cp_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return cp desc to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
compress_chain(struct chain_entry *centry)
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
compress_schedule(const struct service_info *svc_info)
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
compress_poll(const struct service_info *svc_info)
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
compress_read_status(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct cpdc_sgl	*dst_sgl;
	struct pnso_compression_header *cp_hdr;
	uint16_t datain_len;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(svc_info);

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;
	if (!status_desc) {
		OSAL_LOG_ERROR("invalid cp status desc! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_STATUS_DESC(status_desc);

	if (!status_desc->csd_valid) {
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	cp_desc = svc_info->si_desc;
	if (!cp_desc) {
		OSAL_LOG_ERROR("invalid cp desc! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != cp_desc->cd_status_data) {
		OSAL_LOG_ERROR("partial data mismatch, expected %u received: %u err: %d",
				cp_desc->cd_status_data,
				status_desc->csd_partial_data, err);
	}

	/* TODO-cp: handle bypass on fail flag */
	if (status_desc->csd_err) {
		err = status_desc->csd_err;
		OSAL_LOG_ERROR("hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, err);
		goto out;
	}

	if (cp_desc->u.cd_bits.cc_enabled &&
			cp_desc->u.cd_bits.cc_insert_header) {
		dst_sgl = svc_info->si_dst_sgl;
		cp_hdr = (struct pnso_compression_header *)
			osal_phy_to_virt(dst_sgl->cs_addr_0);

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

	err = PNSO_OK;
	OSAL_LOG_INFO("exit! status/result update success!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
compress_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;
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
	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	err = mpool_put_object(cpdc_mpool, cp_desc);
	if (err) {
		OSAL_LOG_ERROR("failed to return cp desc to pool! cp_desc: %p err: %d",
				cp_desc, err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_INFO("exit!");
}

struct service_ops cp_ops = {
	.setup = compress_setup,
	.chain = compress_chain,
	.schedule = compress_schedule,
	.poll = compress_poll,
	.read_status = compress_read_status,
	.write_result = compress_write_result,
	.teardown = compress_teardown
};
