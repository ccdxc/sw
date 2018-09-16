/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>

#include "sonic_dev.h"
#include "sonic_lif.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * TODO:
 *	- revisit chain() when handling multiple services in one request
 *	- add additional UTs for read/write status/result, as needed
 *	- reuse/common code
 *	- skip partial/status data mismatch in cp/chksum/hash and its UTs
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
#ifdef NDEBUG
#define CPDC_PPRINT_STATUS_DESC(d)
#else
#define CPDC_PPRINT_STATUS_DESC(d)	cpdc_pprint_status_desc(d)
#endif

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

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void __attribute__((unused))
pprint_sgl(uint64_t sgl_pa)
{
	const struct cpdc_sgl *sgl;

	sgl = (const struct cpdc_sgl *) osal_phy_to_virt(sgl_pa);
	if (!sgl)
		return;

	OSAL_LOG_INFO("%30s: %llx", "cs_addr_0", sgl->cs_addr_0);
	OSAL_LOG_INFO("%30s: %d", "cs_len_0", sgl->cs_len_0);

	OSAL_LOG_INFO("%30s: %llx", "cs_addr_1", sgl->cs_addr_1);
	OSAL_LOG_INFO("%30s: %d", "cs_len_1", sgl->cs_len_1);

	OSAL_LOG_INFO("%30s: %llx", "cs_addr_2", sgl->cs_addr_2);
	OSAL_LOG_INFO("%30s: %d", "cs_len_2", sgl->cs_len_2);

	OSAL_LOG_INFO("%30s: %llx", "cs_next", sgl->cs_next);
}

static void __attribute__((unused))
pprint_cpdc_cmd(const struct cpdc_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_INFO("%30s: %d", "cc_enabled", cmd->cc_enabled);

	OSAL_LOG_INFO("%30s: %d", "cc_header_present", cmd->cc_header_present);
	OSAL_LOG_INFO("%30s: %d", "cc_insert_header", cmd->cc_insert_header);

	OSAL_LOG_INFO("%30s: %d", "cc_db_on", cmd->cc_db_on);
	OSAL_LOG_INFO("%30s: %d", "cc_otag_on", cmd->cc_otag_on);

	OSAL_LOG_INFO("%30s: %d", "cc_src_is_list", cmd->cc_src_is_list);
	OSAL_LOG_INFO("%30s: %d", "cc_dst_is_list", cmd->cc_dst_is_list);

	OSAL_LOG_INFO("%30s: %d", "cc_chksum_verify_enabled",
			cmd->cc_chksum_verify_enabled);
	OSAL_LOG_INFO("%30s: %d", "cc_chksum_adler", cmd->cc_chksum_adler);

	OSAL_LOG_INFO("%30s: %d", "cc_hash_enabled", cmd->cc_hash_enabled);
	OSAL_LOG_INFO("%30s: %d", "cc_hash_type", cmd->cc_hash_type);

	OSAL_LOG_INFO("%30s: %d", "cc_integrity_src", cmd->cc_integrity_src);
	OSAL_LOG_INFO("%30s: %d", "cc_integrity_type", cmd->cc_integrity_type);
}

void __attribute__((unused))
cpdc_pprint_desc(const struct cpdc_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%.*s", 30, "=========================================");
	OSAL_LOG_INFO("%30s: 0x%llx", "cpdc_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %llx", "cd_src", desc->cd_src);
	OSAL_LOG_INFO("%30s: %llx", "cd_dst", desc->cd_dst);

	OSAL_LOG_INFO("%30s:", "=== cpdc_cmd");
	pprint_cpdc_cmd(&desc->u.cd_bits);

	OSAL_LOG_INFO("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_INFO("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_INFO("%30s: %d", "cd_threshold_len", desc->cd_threshold_len);

	OSAL_LOG_INFO("%30s: %llx", "cd_status_addr", desc->cd_status_addr);

	OSAL_LOG_INFO("%30s: %llx", "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_INFO("%30s: %llx", "cd_db_data", desc->cd_db_data);

	OSAL_LOG_INFO("%30s: %llx", "cd_otag_addr", desc->cd_otag_addr);
	OSAL_LOG_INFO("%30s: %d", "cd_otag_data", desc->cd_otag_data);

	OSAL_LOG_INFO("%32s: %d", "cd_status_data", desc->cd_status_data);

	if (desc->u.cd_bits.cc_src_is_list) {
		OSAL_LOG_INFO("%30s: %llx", "=== src_sgl", desc->cd_src);
		pprint_sgl(desc->cd_src);
	}
	if (desc->u.cd_bits.cc_dst_is_list) {
		OSAL_LOG_INFO("%30s: %llx", "=== dst_sgl", desc->cd_dst);
		pprint_sgl(desc->cd_dst);
	}
	OSAL_LOG_INFO("%.*s", 30, "=========================================");
}

void __attribute__((unused))
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	if (!status_desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== status_desc", (u64) status_desc);

	OSAL_LOG_INFO("%30s: %d", "csd_err", status_desc->csd_err);
	OSAL_LOG_INFO("%30s: %d", "csd_valid", status_desc->csd_valid);

	OSAL_LOG_INFO("%30s: %d", "csd_output_data_len",
			status_desc->csd_output_data_len);
	OSAL_LOG_INFO("%30s: %d", "csd_partial_data",
			status_desc->csd_partial_data);
	OSAL_LOG_INFO("%30s: 0x%llx", "csd_integrity_data",
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
			(struct cpdc_sgl *) osal_phy_to_virt(sgl->cs_next) :
			NULL;
		sgl->cs_next = 0;
		iter++;

		err = mpool_put_object(cpdc_sgl_mpool, sgl);
		if (err) {
			OSAL_LOG_ERROR("failed to return cpdc sgl desc to pool! #: %2d sgl 0x%llx err: %d",
					iter, (u64) sgl, err);
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

	count = buf_list->count;
	while (count) {
		sgl = (struct cpdc_sgl *) mpool_get_object(cpdc_sgl_mpool);
		if (!sgl) {
			err = ENOMEM;
			OSAL_LOG_ERROR("cannot obtain cpdc sgl desc from pool! err: %d", err);
			goto out;
		}
		memset(sgl, 0, sizeof(struct cpdc_sgl));

		if (!sgl_head)
			sgl_head = sgl;
		else
			sgl_prev->cs_next = (uint64_t) osal_virt_to_phy(sgl);

		i = 0;
		sgl->cs_addr_0 = buf_list->buffers[i].buf;
		sgl->cs_len_0 = buf_list->buffers[i].len;
		i++;
		count--;

		if (count == 0) {
			sgl->cs_next = 0;
			break;
		}

		if (count && buf_list->buffers[i].len) {
			sgl->cs_addr_1 = buf_list->buffers[i].buf;
			sgl->cs_len_1 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		if (count && buf_list->buffers[i].len) {
			sgl->cs_addr_2 = buf_list->buffers[i].buf;
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

	return err;

out_sgl:
	cpdc_release_sgl(svc_info->si_src_sgl);
out:
	return err;
}

void
cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list)
{
	/* TODO-chain: EOPNOTSUPP */
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
