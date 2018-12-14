/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifdef __FreeBSD__
#include <linux/netdevice.h>
#else
#include <netdevice.h>
#endif

#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_svc.h"
#include "pnso_mpool.h"
#include "pnso_batch.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_utils.h"
#include "pnso_seq.h"

/*
 * TODO:
 *	- add additional UTs for read/write status/result, as needed
 *	- handle PNSO_CP_DFLAG_BYPASS_ONFAIL fully
 *	- reuse/common code (write_result, read_status, cpdc_setup_batch_desc)
 *	- address cpdc_fill_per_block_desc_ex()
 *
 */
pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	return PNSO_OK;	/* TODO-chain: EOPNOTSUPP */
}

pnso_error_t
cpdc_poll(const struct service_info *svc_info)
{
	pnso_error_t err;

	volatile struct cpdc_status_desc *status_desc;
	uint64_t elapsed_ts, start_ts;

	OSAL_LOG_DEBUG("enter ...");

	status_desc = (struct cpdc_status_desc *) svc_info->si_status_desc;

	if ((svc_info->si_flags & CHAIN_SFLAG_MODE_POLL) ||
		(svc_info->si_flags & CHAIN_SFLAG_MODE_ASYNC)) {
		err = status_desc->csd_valid ? PNSO_OK : EBUSY;
		goto out;
	}

	/* sync-mode ... */
	start_ts = osal_get_clock_nsec();
	while (1) {
		err = status_desc->csd_valid ? PNSO_OK : EBUSY;
		if (!err)
			break;

		elapsed_ts = osal_get_clock_nsec() - start_ts;
		if (elapsed_ts >= CPDC_POLL_LOOP_TIMEOUT) {
			err = ETIMEDOUT;
			OSAL_LOG_ERROR("poll-time limit reached! service: %s status_desc: 0x" PRIx64 " err: %d",
					svc_get_type_str(svc_info->si_type),
					(uint64_t) status_desc, err);
			break;
		}

		osal_yield();
	}

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

static void __attribute__((unused))
cpdc_common_teardown(void *desc)
{
	/* TODO-chain: EOPNOTSUPP */
}

pnso_error_t
cpdc_common_read_status(struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc)
{
	pnso_error_t err = EINVAL;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(desc);
	OSAL_ASSERT(status_desc);

	if (!status_desc) {
		OSAL_LOG_ERROR("invalid status desc! err: %d", err);
		goto out;
	}
	CPDC_PPRINT_STATUS_DESC(status_desc);

	if (!status_desc->csd_valid) {
		OSAL_LOG_ERROR("valid bit not set! err: %d", err);
		goto out;
	}

	if (!desc) {
		OSAL_LOG_ERROR("invalid desc! err: %d", err);
		goto out;
	}

	if (status_desc->csd_partial_data != desc->cd_status_data) {
		OSAL_LOG_ERROR("partial data mismatch, expected %u received: %u err: %d",
				desc->cd_status_data,
				status_desc->csd_partial_data, err);
	}

	if (status_desc->csd_err) {
		err = status_desc->csd_err;
		OSAL_LOG_ERROR("hw error reported! csd_err: %d err: %d",
				status_desc->csd_err, err);
		goto out;
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void __attribute__((unused))
pprint_sgl(uint64_t sgl_pa)
{
	const struct cpdc_sgl *sgl;

	sgl = (const struct cpdc_sgl *) sonic_phy_to_virt(sgl_pa);
	if (!sgl)
		return;

	while (sgl) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 " ==> 0x" PRIx64,
			"",
			(uint64_t) sgl, sgl_pa);

		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d 0x" PRIx64 "/%d/%d",
				"",
				sgl->cs_addr_0, sgl->cs_len_0, sgl->cs_rsvd_0,
				sgl->cs_addr_1, sgl->cs_len_1, sgl->cs_rsvd_1,
				sgl->cs_addr_2, sgl->cs_len_2, sgl->cs_rsvd_2);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/0x" PRIx64,
				"",
				sgl->cs_next, sgl->cs_rsvd_swlink);

		CPDC_SGL_SWLINK_GET(sgl, sgl);

		if (sgl)
			sgl_pa = sonic_virt_to_phy((void *) sgl);
	}
}

static void __attribute__((unused))
pprint_cpdc_cmd(const struct cpdc_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_DEBUG("%30s: %d", "cc_enabled", cmd->cc_enabled);

	OSAL_LOG_DEBUG("%30s: %d", "cc_header_present", cmd->cc_header_present);
	OSAL_LOG_DEBUG("%30s: %d", "cc_insert_header", cmd->cc_insert_header);

	OSAL_LOG_DEBUG("%30s: %d", "cc_db_on", cmd->cc_db_on);
	OSAL_LOG_DEBUG("%30s: %d", "cc_otag_on", cmd->cc_otag_on);

	OSAL_LOG_DEBUG("%30s: %d", "cc_src_is_list", cmd->cc_src_is_list);
	OSAL_LOG_DEBUG("%30s: %d", "cc_dst_is_list", cmd->cc_dst_is_list);

	OSAL_LOG_DEBUG("%30s: %d", "cc_chksum_verify_enabled",
			cmd->cc_chksum_verify_enabled);
	OSAL_LOG_DEBUG("%30s: %d", "cc_chksum_adler", cmd->cc_chksum_adler);

	OSAL_LOG_DEBUG("%30s: %d", "cc_hash_enabled", cmd->cc_hash_enabled);
	OSAL_LOG_DEBUG("%30s: %d", "cc_hash_type", cmd->cc_hash_type);

	OSAL_LOG_DEBUG("%30s: %d", "cc_integrity_src", cmd->cc_integrity_src);
	OSAL_LOG_DEBUG("%30s: %d", "cc_integrity_type", cmd->cc_integrity_type);
}

void __attribute__((unused))
cpdc_pprint_desc(const struct cpdc_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cpdc_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_src", desc->cd_src);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_dst", desc->cd_dst);

	OSAL_LOG_DEBUG("%30s:", "=== cpdc_cmd");
	pprint_cpdc_cmd(&desc->u.cd_bits);

	OSAL_LOG_DEBUG("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_threshold_len", desc->cd_threshold_len);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_status_addr",
			desc->cd_status_addr);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_db_data", desc->cd_db_data);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cd_otag_addr", desc->cd_otag_addr);
	OSAL_LOG_DEBUG("%30s: %d", "cd_otag_data", desc->cd_otag_data);
	OSAL_LOG_DEBUG("%30s: %d", "cd_status_data", desc->cd_status_data);

	if (desc->u.cd_bits.cc_src_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== src_sgl", desc->cd_src);
		pprint_sgl(desc->cd_src);
	}
	if (desc->u.cd_bits.cc_dst_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== dst_sgl", desc->cd_dst);
		pprint_sgl(desc->cd_dst);
	}
}

void __attribute__((unused))
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	if (!status_desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== status_desc",
			(uint64_t) status_desc);

	OSAL_LOG_DEBUG("%30s: %d", "csd_err", status_desc->csd_err);
	OSAL_LOG_DEBUG("%30s: %d", "csd_valid", status_desc->csd_valid);

	OSAL_LOG_DEBUG("%30s: %d", "csd_output_data_len",
			status_desc->csd_output_data_len);
	OSAL_LOG_DEBUG("%30s: %d", "csd_partial_data",
			status_desc->csd_partial_data);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "csd_integrity_data",
			status_desc->csd_integrity_data);

	/* TODO-cpdc: print SHA */
}

uint32_t
cpdc_get_desc_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_desc) + pad_size;
}

uint32_t
cpdc_get_status_desc_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_status_desc) + pad_size;
}

uint32_t
cpdc_get_sgl_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_sgl),
			PNSO_MEM_ALIGN_DESC);
	return sizeof(struct cpdc_sgl) + pad_size;
}

static struct cpdc_desc *
get_next_desc(struct cpdc_desc *desc, uint32_t object_size)
{
	char *obj;

	obj = (char *) desc;
	obj += object_size;
	desc = (struct cpdc_desc *) obj;

	return desc;
}

struct cpdc_status_desc *
cpdc_get_next_status_desc(struct cpdc_status_desc *desc, uint32_t object_size)
{
	char *obj;

	obj = (char *) desc;
	obj += object_size;
	desc = (struct cpdc_status_desc *) obj;

	return desc;
}

static struct cpdc_sgl *
get_next_sgl(struct cpdc_sgl *sgl, uint32_t object_size)
{
	char *obj;

	obj = (char *) sgl;
	obj += object_size;
	sgl = (struct cpdc_sgl *) obj;

	return sgl;
}

static struct cpdc_desc *
get_desc(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC];

	return (struct cpdc_desc *) mpool_get_object(mpool);
}

static struct cpdc_desc *
get_batch_desc(struct service_info *svc_info)
{
	struct service_batch_info *svc_batch_info;
	struct cpdc_desc *desc;

	svc_batch_info = &svc_info->si_batch_info;
	desc = &svc_batch_info->u.sbi_cpdc_desc[svc_batch_info->sbi_desc_idx];

	OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64,
			svc_batch_info->sbi_num_entries,
			svc_batch_info->sbi_desc_idx,
			(uint64_t) svc_batch_info->u.sbi_cpdc_desc,
			(uint64_t) desc);
	return desc;
}

struct cpdc_desc *__attribute__((unused))
cpdc_get_desc(struct service_info *svc_info, bool per_block)
{
	struct cpdc_desc *desc;
	bool in_batch = false;

	if (cpdc_is_service_in_batch(svc_info->si_flags))
		in_batch = true;

	desc = in_batch ? get_batch_desc(svc_info) :
		get_desc(svc_info->si_pcr, per_block);

	OSAL_ASSERT(desc);
	return desc;
}

/* 'batch' is the caller, not the services ... */
struct cpdc_desc *
cpdc_get_batch_bulk_desc(struct mem_pool *mpool)
{
	struct cpdc_desc *desc;

	desc = (struct cpdc_desc *) mpool_get_object(mpool);
	if (!desc) {
		OSAL_LOG_ERROR("cannot obtain cpdc bulk object from pool!");
		return NULL;
	}

	return desc;
}

static void
put_desc(struct per_core_resource *pcr, bool per_block, struct cpdc_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_DESC];

	mpool_put_object(mpool, desc);
}

static void
put_batch_desc(const struct service_info *svc_info, struct cpdc_desc *desc)
{
	struct service_batch_info *svc_batch_info;

	svc_batch_info = (struct service_batch_info *) &svc_info->si_batch_info;

	/* do nothing */

	OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64,
			svc_batch_info->sbi_num_entries,
			svc_batch_info->sbi_desc_idx,
			(uint64_t) svc_batch_info->u.sbi_cpdc_desc,
			(uint64_t) desc);
}

void
cpdc_put_desc(const struct service_info *svc_info, bool per_block,
		struct cpdc_desc *desc)
{
	bool in_batch = false;

	if (cpdc_is_service_in_batch(svc_info->si_flags))
		in_batch = true;

	if (in_batch)
		put_batch_desc(svc_info, desc);
	else
		put_desc(svc_info->si_pcr, per_block, desc);
}

/* 'batch' is the caller, not the services ... */
void
cpdc_put_batch_bulk_desc(struct mem_pool *mpool, struct cpdc_desc *desc)
{
	mpool_put_object(mpool, desc);
}

struct cpdc_status_desc *
cpdc_get_status_desc(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];

	return (struct cpdc_status_desc *) mpool_get_object(mpool);
}

void
cpdc_put_status_desc(struct per_core_resource *pcr, bool per_block,
		struct cpdc_status_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];

	mpool_put_object(mpool, desc);
}

struct cpdc_sgl *
cpdc_get_sgl(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_SGL];

	return (struct cpdc_sgl *) mpool_get_object(mpool);
}

void
cpdc_put_sgl(struct per_core_resource *pcr, bool per_block,
		struct cpdc_sgl *sgl)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CPDC_SGL];

	mpool_put_object(mpool, sgl);
}

pnso_error_t
cpdc_update_service_info_sgl(struct service_info *svc_info)
{
	pnso_error_t err;

	err = pc_res_sgl_packed_get(svc_info->si_pcr, &svc_info->si_src_blist,
			CPDC_SGL_TUPLE_LEN_MAX, MPOOL_TYPE_CPDC_SGL,
			&svc_info->si_src_sgl);
	if (err)
		OSAL_LOG_ERROR("cannot obtain src sgl from pool! err: %d", err);

	return err;
}

uint32_t
cpdc_fill_per_block_desc(uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct pnso_buffer_list *src_blist,
		struct cpdc_sgl *sgl, struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc,
		fill_desc_fn_t fill_desc_fn)
{
	struct cpdc_desc *pb_desc;
	struct cpdc_status_desc *pb_status_desc;
	struct cpdc_sgl *pb_sgl;
	struct pnso_flat_buffer flat_buf;
	uint32_t desc_object_size, status_object_size, sgl_object_size;
	uint32_t len, block_cnt, buf_len, i = 0;
	char *buf;

	/*
	 * per-block support assumes the input buffer will be mapped to a
	 * flat buffer and for 8-blocks max.  Memory for this flat buffer
	 * should come from HBM memory.
	 *
	 */
	flat_buf.len = src_buf_len;
	flat_buf.buf = (uint64_t) osal_phy_to_virt(src_blist->buffers[0].buf);

	block_cnt = pbuf_get_flat_buffer_block_count(&flat_buf, block_size);
	pb_desc = desc;
	pb_status_desc = status_desc;
	pb_sgl = sgl;

	OSAL_LOG_INFO("block_cnt: %d block_size: %d src_buf_len: %d buf: 0x" PRIx64 " desc: 0x" PRIx64 " status_desc: 0x" PRIx64,
			block_cnt, block_size, src_buf_len, flat_buf.buf,
			(uint64_t) desc, (uint64_t) status_desc);

	desc_object_size = cpdc_get_desc_size();
	status_object_size = cpdc_get_status_desc_size();
	sgl_object_size = cpdc_get_sgl_size();

	buf_len = src_buf_len;
	for (i = 0; buf_len && (i < block_cnt); i++) {
		buf = (char *) flat_buf.buf + (i * block_size);
		len = buf_len > block_size ? block_size : buf_len;

		memset(pb_sgl, 0, sizeof(struct cpdc_sgl));
		pb_sgl->cs_addr_0 = sonic_virt_to_phy(buf);
		pb_sgl->cs_len_0 = len;

		OSAL_LOG_INFO("blk_num: %d buf: 0x" PRIx64 ", len: %d desc: 0x" PRIx64 " status_desc: 0x" PRIx64 " sgl: 0x" PRIx64,
			i, (uint64_t) buf, len, (uint64_t) pb_desc,
			(uint64_t) pb_status_desc, (uint64_t) pb_sgl);

		fill_desc_fn(algo_type, len, false,
				pb_sgl, pb_desc, pb_status_desc);
		buf_len -= len;

		pb_desc = get_next_desc(pb_desc, desc_object_size);

		pb_status_desc = cpdc_get_next_status_desc(pb_status_desc,
				status_object_size);

		pb_sgl = get_next_sgl(pb_sgl, sgl_object_size);
	}

	return i;
}

pnso_error_t
cpdc_update_service_info_sgls(struct service_info *svc_info)
{
	pnso_error_t err;

	err = pc_res_sgl_packed_get(svc_info->si_pcr, &svc_info->si_src_blist,
			CPDC_SGL_TUPLE_LEN_MAX, MPOOL_TYPE_CPDC_SGL,
			&svc_info->si_src_sgl);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain src sgl from pool! err: %d", err);
		goto out;
	}

	err = pc_res_sgl_packed_get(svc_info->si_pcr, &svc_info->si_dst_blist,
			CPDC_SGL_TUPLE_LEN_MAX, MPOOL_TYPE_CPDC_SGL,
			&svc_info->si_dst_sgl);
	if (err) {
		OSAL_LOG_ERROR("cannot obtain dst sgl from pool! err: %d", err);
		goto out_sgl;
	}

	return err;

out_sgl:
	pc_res_sgl_put(svc_info->si_pcr, &svc_info->si_src_sgl);
out:
	return err;
}

pnso_error_t
cpdc_convert_desc_error(int error)
{
	switch (error) {
	case CP_STATUS_SUCCESS:
		return PNSO_OK;
	case CP_STATUS_AXI_TIMEOUT:
		return PNSO_ERR_CPDC_AXI_TIMEOUT;
	case CP_STATUS_AXI_DATA_ERROR:
		return PNSO_ERR_CPDC_AXI_DATA_ERROR;
	case CP_STATUS_AXI_ADDR_ERROR:
		return PNSO_ERR_CPDC_AXI_ADDR_ERROR;
	case CP_STATUS_COMPRESSION_FAILED:
		return PNSO_ERR_CPDC_COMPRESSION_FAILED;
	case CP_STATUS_DATA_TOO_LONG:
		return PNSO_ERR_CPDC_DATA_TOO_LONG;
	case CP_STATUS_CHECKSUM_FAILED:
		return PNSO_ERR_CPDC_CHECKSUM_FAILED;
	case CP_STATUS_SGL_DESC_ERROR:
		return PNSO_ERR_CPDC_SGL_DESC_ERROR;
	/*
	 * TODO-cpdc: handle other errors
	 *	return PNSO_ERR_CPDC_HDR_IDX_INVALID
	 *	return PNSO_ERR_CPDC_ALGO_INVALID
	 *
	 */
	default:
		OSAL_ASSERT(0);	/* unreachable code */
		return EOPNOTSUPP;
	}
}

struct service_deps *
cpdc_get_service_deps(const struct service_info *svc_info)
{
	struct service_chain *chead;
	struct chain_entry *centry;
	struct service_deps *svc_deps;

	if (!svc_info->si_centry)
		return NULL;

	centry = svc_info->si_centry;
	chead = centry->ce_chain_head;
	svc_deps = &chead->sc_svc_deps;

	return svc_deps;
}

bool
cpdc_is_service_in_batch(uint8_t flags)
{
	return ((flags & CHAIN_SFLAG_IN_BATCH) &&
			((flags & CHAIN_SFLAG_LONE_SERVICE) ||
			 (flags & CHAIN_SFLAG_FIRST_SERVICE))) ? true : false;
}

pnso_error_t
cpdc_setup_batch_desc(struct service_info *svc_info, struct cpdc_desc *desc)
{
	struct service_batch_info *svc_batch_info;
	uint32_t batch_size, remaining;

	svc_batch_info = &svc_info->si_batch_info;
	OSAL_ASSERT(svc_batch_info->sbi_num_entries);

	if (svc_batch_info->sbi_desc_idx != 0) {
		OSAL_LOG_DEBUG("sequencer setup not needed!");
		return PNSO_OK;
	}

	remaining = svc_batch_info->sbi_num_entries -
		(svc_batch_info->sbi_bulk_desc_idx * MAX_PAGE_ENTRIES);
	batch_size = (remaining / MAX_PAGE_ENTRIES) ? MAX_PAGE_ENTRIES :
		remaining;

	/* indicate batch processing only for 1st entry in the batch */
	svc_info->si_seq_info.sqi_batch_mode = true;
	svc_info->si_seq_info.sqi_batch_size = batch_size;

	svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
			desc, sizeof(*desc));
	if (!svc_info->si_seq_info.sqi_desc) {
		OSAL_LOG_ERROR("failed to setup sequencer desc!");
		return EINVAL;
	}

	return PNSO_OK;
}

pnso_error_t
cpdc_setup_seq_desc(struct service_info *svc_info, struct cpdc_desc *desc,
		uint32_t num_tags)
{
	pnso_error_t err = PNSO_OK;
	uint8_t	flags;

	if (cpdc_is_service_in_batch(svc_info->si_flags)) {
		err = cpdc_setup_batch_desc(svc_info, desc);
		if (err)
			OSAL_LOG_DEBUG("failed to setup batch sequencer desc! err: %d",
					err);
		goto out;
	}

	flags = svc_info->si_flags;
	if ((flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		if (num_tags > 1) {
			svc_info->si_seq_info.sqi_batch_mode = true;
			svc_info->si_seq_info.sqi_batch_size = num_tags;
		}

		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				desc, sizeof(*desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_DEBUG("failed to setup sequencer desc! num_tags: %d flags: %d err: %d",
						num_tags, flags, err);
			goto out;
		}
	}

out:
	return err;
}

pnso_error_t
cpdc_setup_interrupt_params(const struct service_info *svc_info, void *poll_ctx)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;
	struct per_core_resource *pcr;

	cp_desc = (struct cpdc_desc *) svc_info->si_desc;
	pcr = svc_info->si_pcr;

	OSAL_LOG_DEBUG("cp_desc: 0x" PRIx64 " pcr: 0x" PRIx64 " poll_ctx: 0x" PRIx64,
			(uint64_t) cp_desc, (uint64_t) pcr,
			(uint64_t) poll_ctx);

	cp_desc->u.cd_bits.cc_otag_on = 1;

	if ((svc_info->si_type == PNSO_SVC_TYPE_COMPRESS) &&
		(svc_info->si_desc_flags & PNSO_CP_DFLAG_ZERO_PAD)) {
		cp_desc->cd_otag_addr =
			sonic_intr_get_db_addr(pcr, (uint64_t) poll_ctx);
		if (!cp_desc->cd_otag_addr) {
			err = EINVAL;
			OSAL_LOG_DEBUG("cp/pad failed to get db addr err: %d",
					err);
			goto out;
		}

		cp_desc->cd_otag_data = sonic_intr_get_fire_data32();
	} else {
		cp_desc->cd_db_addr = (uint64_t) sonic_intr_get_db_addr(pcr,
				(uint64_t) poll_ctx);
		if (!cp_desc->cd_db_addr) {
			err = EINVAL;
			OSAL_LOG_DEBUG("failed to get db addr err: %d", err);
			goto out;
		}

		cp_desc->u.cd_bits.cc_db_on = 1;
		cp_desc->cd_db_data = sonic_intr_get_fire_data64();

		cp_desc->cd_otag_addr =
			sonic_get_per_core_intr_assert_addr(pcr);
		cp_desc->cd_otag_data = sonic_get_intr_assert_data();
	}

	err = PNSO_OK;
out:
	return  err;
}
