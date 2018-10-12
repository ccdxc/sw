/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>

#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * TODO:
 *	- add additional UTs for read/write status/result, as needed
 *	- reuse/common code (write_result, read_status, etc.)
 *	- move validation routines from services to higher layer
 *
 * TODO-cp:
 *	- handle PNSO_CP_DFLAG_ZERO_PAD, PNSO_CP_DFLAG_BYPASS_ONFAIL fully
 *	- see embedded ones
 *
 * TODO-dc:
 *	- see embedded ones
 *
 * TODO-hash:
 * TODO-chksum:
 *	- per_block support assumes flat buf as input for now; revalidate
 *	when chaining logic kicks-in along with buffer list support
 *	- check for per-block iterator, move into common and then refine
 *	- handle input len for per-block/entire buffer in setup and fill
 *	routines
 *	- see embedded ones
 *
 */

pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	return PNSO_OK;	/* TODO-chain: EOPNOTSUPP */
}

void
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

	OSAL_LOG_DEBUG("%30s: 0x%llx ==> 0x%llx", "", (uint64_t) sgl, sgl_pa);
	while (sgl) {
		OSAL_LOG_DEBUG("%30s: 0x%llx/%d/%d 0x%llx/%d/%d 0x%llx/%d/%d",
				"",
				sgl->cs_addr_0, sgl->cs_len_0, sgl->cs_rsvd_0,
				sgl->cs_addr_1, sgl->cs_len_1, sgl->cs_rsvd_1,
				sgl->cs_addr_2, sgl->cs_len_2, sgl->cs_rsvd_2);
		OSAL_LOG_DEBUG("%30s: 0x%llx/0x%llx", "",
				sgl->cs_next, sgl->cs_rsvd_3);

		sgl = sgl->cs_next ? sonic_phy_to_virt(sgl->cs_next) : NULL;
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

	OSAL_LOG_DEBUG("%30s: 0x%llx", "cpdc_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_src", desc->cd_src);
	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_dst", desc->cd_dst);

	OSAL_LOG_DEBUG("%30s:", "=== cpdc_cmd");
	pprint_cpdc_cmd(&desc->u.cd_bits);

	OSAL_LOG_DEBUG("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_DEBUG("%30s: %d", "cd_threshold_len", desc->cd_threshold_len);

	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_status_addr", desc->cd_status_addr);

	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_db_data", desc->cd_db_data);

	OSAL_LOG_DEBUG("%30s: 0x%llx", "cd_otag_addr", desc->cd_otag_addr);
	OSAL_LOG_DEBUG("%30s: %d", "cd_otag_data", desc->cd_otag_data);
	OSAL_LOG_DEBUG("%30s: %d", "cd_status_data", desc->cd_status_data);

	if (desc->u.cd_bits.cc_src_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x%llx", "=== src_sgl", desc->cd_src);
		pprint_sgl(desc->cd_src);
	}
	if (desc->u.cd_bits.cc_dst_is_list) {
		OSAL_LOG_DEBUG("%30s: 0x%llx", "=== dst_sgl", desc->cd_dst);
		pprint_sgl(desc->cd_dst);
	}
}

void __attribute__((unused))
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	if (!status_desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x%llx", "=== status_desc",
			(uint64_t) status_desc);

	OSAL_LOG_DEBUG("%30s: %d", "csd_err", status_desc->csd_err);
	OSAL_LOG_DEBUG("%30s: %d", "csd_valid", status_desc->csd_valid);

	OSAL_LOG_DEBUG("%30s: %d", "csd_output_data_len",
			status_desc->csd_output_data_len);
	OSAL_LOG_DEBUG("%30s: %d", "csd_partial_data",
			status_desc->csd_partial_data);
	OSAL_LOG_DEBUG("%30s: 0x%llx", "csd_integrity_data",
			status_desc->csd_integrity_data);

	/* TODO-cpdc: print SHA */
}

void
cpdc_release_sgl(struct cpdc_sgl *sgl)
{
	pnso_error_t err;
	struct lif *lif;
	struct per_core_resource *pc_res;
	struct mem_pool *cpdc_sgl_mpool;
	struct cpdc_sgl *sgl_next;
	uint32_t iter;

	if (!sgl)
		return;

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(lif);
		return;
	}

	pc_res = sonic_get_per_core_res(lif);
	if (!pc_res) {
		OSAL_ASSERT(pc_res);
		return;
	}

	cpdc_sgl_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_SGL];
	if (!cpdc_sgl_mpool) {
		OSAL_ASSERT(cpdc_sgl_mpool);
		return;
	}

	iter = 0;
	while (sgl) {
		sgl_next = sgl->cs_next ?
			(struct cpdc_sgl *) sonic_phy_to_virt(sgl->cs_next) :
			NULL;
		iter++;

		err = mpool_put_object(cpdc_sgl_mpool, sgl);
		if (err) {
			OSAL_LOG_ERROR("failed to return cpdc sgl desc to pool! #: %2d sgl 0x%llx err: %d",
					iter, (uint64_t) sgl, err);
			OSAL_ASSERT(err);
		}

		sgl = sgl_next;
	}
}

static struct cpdc_sgl *
populate_sgl(const struct pnso_buffer_list *buf_list)
{
	pnso_error_t err = EINVAL;
	struct lif *lif;
	struct per_core_resource *pc_res;
	struct mem_pool *cpdc_sgl_mpool;
	struct cpdc_sgl *sgl_head = NULL;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl = NULL;
	uint32_t count;
	uint32_t i;

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(lif);
		goto out;
	}

	pc_res = sonic_get_per_core_res(lif);
	if (!pc_res) {
		OSAL_ASSERT(pc_res);
		goto out;
	}

	cpdc_sgl_mpool = pc_res->mpools[MPOOL_TYPE_CPDC_SGL];
	if (!cpdc_sgl_mpool) {
		OSAL_ASSERT(cpdc_sgl_mpool);
		goto out;
	}

	i = 0;
	count = buf_list->count;
	while (count) {
		sgl = (struct cpdc_sgl *) mpool_get_object(cpdc_sgl_mpool);
		if (!sgl) {
			err = ENOMEM;
			OSAL_LOG_ERROR("cannot obtain cpdc sgl desc from pool! err: %d",
					err);
			goto out;
		}
		memset(sgl, 0, sizeof(struct cpdc_sgl));

		if (!sgl_head)
			sgl_head = sgl;
		else
			sgl_prev->cs_next = (uint64_t) sonic_virt_to_phy(sgl);

		sgl->cs_addr_0 =
			sonic_hostpa_to_devpa(buf_list->buffers[i].buf);
		sgl->cs_len_0 = buf_list->buffers[i].len;
		i++;
		count--;

		if (count == 0) {
			sgl->cs_next = 0;
			break;
		}

		if (count && buf_list->buffers[i].len) {
			sgl->cs_addr_1 =
				sonic_hostpa_to_devpa(buf_list->buffers[i].buf);
			sgl->cs_len_1 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		if (count && buf_list->buffers[i].len) {
			sgl->cs_addr_2 =
				sonic_hostpa_to_devpa(buf_list->buffers[i].buf);
			sgl->cs_len_2 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		sgl_prev = sgl;
	}

	return sgl_head;

out:
	cpdc_release_sgl(sgl_head);
	return NULL;
}

static struct cpdc_sgl	*
convert_buffer_list_to_sgl(const struct pnso_buffer_list *buf_list)
{
	if (!buf_list || buf_list->count == 0)
		return NULL;

	return populate_sgl(buf_list);
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

struct cpdc_desc *
cpdc_get_desc(struct per_core_resource *pc_res, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_DESC];

	return (struct cpdc_desc *) mpool_get_object(mpool);
}

pnso_error_t
cpdc_put_desc(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_DESC];

	return mpool_put_object(mpool, desc);
}

struct cpdc_status_desc *
cpdc_get_status_desc(struct per_core_resource *pc_res, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];

	return (struct cpdc_status_desc *) mpool_get_object(mpool);
}

pnso_error_t
cpdc_put_status_desc(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_status_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC];

	return mpool_put_object(mpool, desc);
}

struct cpdc_sgl *
cpdc_get_sgl(struct per_core_resource *pc_res, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_SGL];

	return (struct cpdc_sgl *) mpool_get_object(mpool);
}

pnso_error_t
cpdc_put_sgl(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_sgl *sgl)
{
	struct mem_pool *mpool;

	mpool = per_block ? pc_res->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR] :
		pc_res->mpools[MPOOL_TYPE_CPDC_SGL];

	return mpool_put_object(mpool, sgl);
}

pnso_error_t
cpdc_update_service_info_sgl(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err = PNSO_OK;
	struct cpdc_sgl	*sgl;

	sgl = convert_buffer_list_to_sgl(svc_params->sp_src_blist);
	if (!sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("cannot obtain src sgl from pool! err: %d", err);
		goto out;
	}
	svc_info->si_src_sgl = sgl;

out:
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

	OSAL_LOG_INFO("block_cnt: %d block_size: %d src_buf_len: %d buf: 0x%llx desc: 0x%llx status_desc: 0x%llx",
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

		OSAL_LOG_INFO("blk_num: %d buf: 0x%llx, len: %d desc: 0x%llx status_desc: 0x%llx sgl: 0x%llx",
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
cpdc_update_service_info_sgls(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	pnso_error_t err = PNSO_OK;
	struct cpdc_sgl	*sgl;

	sgl = convert_buffer_list_to_sgl(svc_params->sp_src_blist);
	if (!sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("cannot obtain src sgl from pool! err: %d", err);
		goto out;
	}
	svc_info->si_src_sgl = sgl;

	sgl = convert_buffer_list_to_sgl(svc_params->sp_dst_blist);
	if (!sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("cannot obtain dst sgl from pool! err: %d", err);
		goto out_sgl;
	}
	svc_info->si_dst_sgl = sgl;
	svc_info->si_dst_blist.sbl_type = SERVICE_BUF_LIST_TYPE_DFLT;
	svc_info->si_dst_blist.sbl_blist = svc_params->sp_dst_blist;

	return err;

out_sgl:
	cpdc_release_sgl(svc_info->si_src_sgl);
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
