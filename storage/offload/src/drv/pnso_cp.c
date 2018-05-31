/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

#define MAX_CP_THRESHOLD_LEN	(1 << 16)

static bool
__is_cp_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_COMPRESSION_TYPE_LZRW1A:
		return true;
	default:
		PNSO_ASSERT(0);	/* unreachable code */
		return false;
	}
	return false;
}

static bool
__is_cp_threshold_len_valid(uint16_t threshold_len)
{
	/* TODO-cp: check for i/p buffer len and max threshold len?? */
	if (threshold_len > MAX_CP_THRESHOLD_LEN)
		return false;

	return true;
}

static bool
__is_cp_flags_valid(uint16_t flags)
{
	/* no special checks needed, as cp flags are independent ones today */
	return true;
}

static bool __attribute__((unused))
__is_dflag_zero_pad_enabled(uint16_t flags)
{
	if (flags & PNSO_CP_DFLAG_ZERO_PAD)
		return true;

	return false;
}

static bool __attribute__((unused))
__is_dflag_insert_header_enabled(uint16_t flags)
{
	if (flags & PNSO_CP_DFLAG_INSERT_HEADER)
		return true;

	return false;
}

static bool __attribute__((unused))
__is_dflag_bypass_onfail_enabled(uint16_t flags)
{
	if (flags & PNSO_CP_DFLAG_BYPASS_ONFAIL)
		return true;

	return false;
}

static inline void
__clear_insert_header(struct cpdc_desc *desc)
{
	desc->u.cd_bits.cc_insert_header = 0;
}

static inline void
__clear_header_present(struct cpdc_desc *desc)
{
	/* reset to invalidate the verification against header */
	desc->u.cd_bits.cc_header_present = 0;
	desc->u.cd_bits.cc_chksum_verify_enabled = 0;
}

static bool
__is_compression_desc_valid(struct pnso_compression_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!__is_cp_algo_type_valid(desc->algo_type)) {
		PNSO_LOG_ERROR(err, "invalid cp algo type specified! algo_type: %hu",
				desc->algo_type);
		return false;
	}

	if (!__is_cp_threshold_len_valid(desc->threshold_len)) {
		PNSO_LOG_ERROR(err, "invalid cp threshold len specified! threshold_len: %hu",
				desc->threshold_len);
		return false;
	}

	if (!__is_cp_flags_valid(desc->flags)) {
		PNSO_LOG_ERROR(err, "invalid cp flags specified! flags: %hu",
				desc->flags);
		return false;
	}

	PNSO_LOG_INFO(PNSO_OK, "compression desc is valid algo_type: %hu threshold_len: %hu flags: %hu",
				desc->algo_type, desc->threshold_len,
				desc->flags);

	return true;
}

static void __attribute__((unused))
__clear_list_bits_in_desc(struct cpdc_desc *desc)
{
	desc->u.cd_bits.cc_src_is_list = 0;
	desc->u.cd_bits.cc_dst_is_list = 0;
}

static void
__fill_cp_desc(struct cpdc_desc *desc, void *src_buf, void *dst_buf,
		void *status_buf, uint32_t sbuf_len)
{
	memset(desc, 0, sizeof(*desc));

	desc->cd_src = (uint64_t) pnso_virt_to_phys(src_buf);
	desc->cd_dst = (uint64_t) pnso_virt_to_phys(dst_buf);

	desc->u.cd_bits.cc_enabled = 1;
	desc->u.cd_bits.cc_insert_header = 1;

	desc->u.cd_bits.cc_src_is_list = 1;
	desc->u.cd_bits.cc_dst_is_list = 1;

	desc->cd_datain_len = (sbuf_len == MAX_CP_THRESHOLD_LEN) ? 0 : sbuf_len;
	desc->cd_threshold_len = sbuf_len - sizeof(struct pnso_compression_header);
	desc->cd_status_addr = (uint64_t) pnso_virt_to_phys(status_buf);
	desc->cd_status_data = 1234;

	cpdc_pprint_desc(desc);
}

static pnso_error_t
compress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err;
	struct pnso_compression_desc *pnso_cp_desc;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_sha512 *status_sha512 = NULL;
	size_t sbuf_len, dbuf_len;
	bool setup_seq;

	if (!svc_info || !desc || !src_buf || !dst_buf) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid input specified! svc_info: %p desc %p src_buf: %p dst_buf: %p",
				svc_info, desc, src_buf, dst_buf);
		goto out;
	}

	sbuf_len = pbuf_get_buffer_list_len(src_buf);
	if (sbuf_len == 0) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid src buf len specified! sbuf_len: %d",
				sbuf_len);
		goto out;
	}

	dbuf_len = pbuf_get_buffer_list_len(dst_buf);
	if (dbuf_len == 0) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "input buf len mismatch! sbuf_len: %d dbuf_len: %d",
				sbuf_len, dbuf_len);
		goto out;
	}

	pnso_cp_desc = (struct pnso_compression_desc *) desc;
	if (!__is_compression_desc_valid(pnso_cp_desc)) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid cp desc specified!");
		goto out;
	}

	cp_desc = (struct cpdc_desc *) dpool_get_desc(cpdc_dpool);
	if (!cp_desc) {
		err = ENOMEM;
		PNSO_LOG_ERROR(err, "cannot obtain cp desc from pool!");
		goto out;
	}

#if USE_DPOOL
	status_sha512 = (struct cpdc_status_sha512 *)
		dpool_get_desc(cpdc_status_sha512_dpool);
#else
	err = pnso_memalign(PNSO_MEM_ALIGN_BUF,
			sizeof(struct cpdc_status_sha512),
			(void **) &status_sha512);
#endif
	if (!status_sha512) {
		err = ENOMEM;
		PNSO_LOG_ERROR(err, "cannot obtain cp status desc from pool!");
		goto out_cp_desc;
	}

	err = cpdc_convert_buffer_list_to_sgl(svc_info, src_buf, dst_buf);
	if (err) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "cannot obtain cp src/dst sgl from pool!");
		goto out_status_desc;
	}

	svc_info->si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info->si_desc = cp_desc;
	svc_info->si_status_buf = status_sha512;

	svc_info->si_sbuf = NULL;
	svc_info->si_dbuf = NULL;

	__fill_cp_desc(cp_desc, svc_info->si_src_sgl,
			svc_info->si_dst_sgl, status_sha512, sbuf_len);

	/* TODO: fix this initialization */
	memset(&svc_info->si_seq_info, 0, sizeof(struct sequencer_info));

	setup_seq = (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
	if (setup_seq)
		seq_submit_desc(cp_desc_pa, &seq_info->queue,
				seq_info->submit_type, seq_info->ring,
				&seq_info->index);

	err = PNSO_OK;
	PNSO_LOG_INFO(err, "service initialized!");

	return err;

out_status_desc:
	cpdc_release_sgl(svc_info->si_dst_sgl);
	cpdc_release_sgl(svc_info->si_src_sgl);

#if USE_DPOOL
	err = dpool_put_desc(cpdc_status_sha512_dpool, status_sha512);
	if (err)
		PNSO_LOG_ERROR(err, "failed to return status desc to pool!");
#else
	pnso_free(status_sha512);
#endif
out_cp_desc:
	err = dpool_put_desc(cpdc_dpool, cp_desc);
	if (err)
		PNSO_LOG_ERROR(err, "failed to return cp desc to pool!");
out:
	return err;
}

static pnso_error_t
compress_chain(struct chain_entry *centry)
{
	pnso_error_t err = PNSO_OK;

	PNSO_LOG_INFO(err, "enter ...");

	assert(centry);

	err = cpdc_common_chain(centry);
	if (err) {
		PNSO_LOG_INFO(err, "failed to chain");
		goto out;
	}

	/* TODO: anything more? */

out:
	PNSO_LOG_INFO(err, "exit!");
	return err;
}

static pnso_error_t
compress_schedule(const struct service_info *svc_info)
{
	struct sequencer_info *seq_info;
	bool ring_db;

	PNSO_LOG_INFO(PNSO_OK, "enter ... ");

	assert(svc_info);
	seq_info = &svc_info->si_seq_info;

	ring_db = (svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
	if (ring_db) {
		PNSO_LOG_INFO(PNSO_OK, "ring door bell <===");
		seq_ring_db(&seq_info->queue, seq_info->index);
	}

	PNSO_LOG_INFO(PNSO_OK, "exit!");
	return PNSO_OK;
}

static pnso_error_t
compress_poll(const struct service_info *svc_info)
{
	uint32_t i;
	struct cpdc_status_sha512 *status_sha512;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	assert(svc_info);

	status_sha512 = (struct cpdc_status_sha512 *) svc_info->si_status_buf;
	assert(status_sha512);

	for (i = 0; i < PNSO_UT_NUM_POLL; i++) {
		PNSO_LOG_INFO(PNSO_OK, "status updated (%d) status_sha512: 0x%jx", i + 1, status_sha512);

		if (status_sha512->css512_valid) {
			PNSO_LOG_INFO(PNSO_OK, "status updated (%d)", i + 1);
			break;
		}
		usleep(PNSO_UT_POLL_DELAY);
	}

	PNSO_LOG_INFO(PNSO_OK, "exit!");
	return PNSO_OK;
}

static pnso_error_t
compress_read_status(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_sha512 *status_sha512;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	assert(svc_info);

	cp_desc = svc_info->si_desc;
	assert(cp_desc);

	status_sha512 = (struct cpdc_status_sha512 *) svc_info->si_status_buf;
	assert(status_sha512);

	cpdc_pprint_status_sha512(status_sha512);

	if (!status_sha512->css512_valid) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "valid bit not set!");
		goto out;
	}

	if (status_sha512->css512_err) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "error reported! error: %d",
				status_sha512->css512_err);
		goto out;
	}

	if (status_sha512->css512_partial_data != cp_desc->cd_status_data) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "partial data mismatch, expected 0x%x received: 0x%x",
				cp_desc->cd_status_data,
				status_sha512->css512_partial_data);
		goto out;
	}

	if (cp_desc->u.cd_bits.cc_enabled &&
			cp_desc->u.cd_bits.cc_insert_header) {
		/*
		 * TODO: check
		 *	- header version
		 *	- zero checksum
		 *	- len
		 */
	}

	if (cp_desc->u.cd_bits.cc_hash_enabled) {
		/* TODO: check SHA */
	}

	if (status_sha512->css512_integrity_data == 0) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "integrity data is zero");
		goto out;
	}

	err = PNSO_OK;
	PNSO_LOG_ERROR(err, "status verification success!");

out:
	PNSO_LOG_INFO(PNSO_OK, "exit");
	return err;
}

static pnso_error_t
compress_write_result(struct service_info *svc_info)
{
	pnso_error_t err;
	struct pnso_service_status *svc_status;
	struct cpdc_status_sha512 *status_sha512;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	assert(svc_info);

	svc_status = svc_info->si_status;
	assert(svc_status->svc_type == svc_info->si_type);

	status_sha512 = (struct cpdc_status_sha512 *) svc_info->si_status_buf;
	assert(status_sha512);

	/* TODO: handle this better */
	svc_status->err = EINVAL;
	if (status_sha512->css512_valid) {
		cpdc_convert_desc_error(status_sha512->css512_err);
		svc_status->err = status_sha512->css512_err;
	}

	svc_status->u.dst.data_len = status_sha512->css512_output_data_len;

	cpdc_release_sgl(svc_info->si_src_sgl);
	cpdc_release_sgl(svc_info->si_dst_sgl);

	err = PNSO_OK;
	PNSO_LOG_ERROR(err, "status/result update success!");

	PNSO_LOG_INFO(PNSO_OK, "exit");
	return err;
}

static void
compress_teardown(const struct service_info *svc_info)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_sha512 *status_sha512;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	assert(svc_info);

	cp_desc = svc_info->si_desc;
	assert(cp_desc);

	status_sha512 = (struct cpdc_status_sha512 *) svc_info->si_status_buf;
	assert(status_sha512);

#if USE_DPOOL
	err = dpool_put_desc(cpdc_status_sha512_dpool, status_sha512);
	if (err) {
		PNSO_LOG_ERROR(EINVAL, "failed to return status desc to pool! status_sha512: %p", status_sha512);
		assert(0);
	}
#else
	pnso_free(status_sha512);
#endif
	err = dpool_put_desc(cpdc_dpool, cp_desc);
	if (err) {
		PNSO_LOG_ERROR(EINVAL, "failed to return cp desc to pool! cp_desc: %p", cp_desc);
		assert(0);
	}

	PNSO_LOG_INFO(PNSO_OK, "exit!");
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
