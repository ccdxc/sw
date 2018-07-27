/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <assert.h>
#define PNSO_ASSERT(x) assert(x)
#else
#define PNSO_ASSERT(x)
#endif

#include "osal.h"
#include "pnso_api.h"

#include "../common/pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

void
cpdc_common_teardown(void *desc)
{
	/* EOPNOTSUPP */
}

static void __attribute__((unused))
pprint_cp_cmd(const struct cpdc_cmd *cmd)
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

	OSAL_LOG_INFO("%30s: %p", "cpdc_desc", desc);

	OSAL_LOG_INFO("%30s: %llx", "cd_src", desc->cd_src);
	OSAL_LOG_INFO("%30s: %llx", "cd_dst", desc->cd_dst);

	OSAL_LOG_INFO("%30s: %p", "=== cpdc_cmd", &desc->u.cd_bits);
	pprint_cp_cmd(&desc->u.cd_bits);

	OSAL_LOG_INFO("%30s: %d", "cd_datain_len", desc->cd_datain_len);
	OSAL_LOG_INFO("%30s: %d", "cd_extended_len", desc->cd_extended_len);
	OSAL_LOG_INFO("%30s: %d", "cd_threshold_len", desc->cd_threshold_len);

	OSAL_LOG_INFO("%30s: %llx", "cd_status_addr", desc->cd_status_addr);

	OSAL_LOG_INFO("%30s: %llx", "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_INFO("%30s: %llx", "cd_db_data", desc->cd_db_data);

	OSAL_LOG_INFO("%30s: %llx", "cd_otag_addr", desc->cd_otag_addr);
	OSAL_LOG_INFO("%30s: %d", "cd_otag_data", desc->cd_otag_data);

	OSAL_LOG_INFO("%30s: %d", "cd_status_data", desc->cd_status_data);
}

void __attribute__((unused))
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	if (!status_desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== status_desc", status_desc);

	OSAL_LOG_INFO("%30s: %d", "csd_err", status_desc->csd_err);
	OSAL_LOG_INFO("%30s: %d", "csd_valid", status_desc->csd_valid);

	OSAL_LOG_INFO("%30s: %d", "csd_output_data_len",
			status_desc->csd_output_data_len);
	OSAL_LOG_INFO("%30s: %d", "csd_partial_data",
			status_desc->csd_partial_data);
	OSAL_LOG_INFO("%30s: %llu", "csd_integrity_data",
			status_desc->csd_integrity_data);

	/* TODO-cpdc: print SHA */
}

void
cpdc_release_sgl(struct cpdc_sgl *sgl)
{
	pnso_error_t err;
	struct cpdc_sgl *sgl_next;
	uint32_t iter;

	if (!sgl)
		return;

	iter = 0;
	while (sgl) {
		sgl_next = (struct cpdc_sgl *) sgl->cs_next;
		sgl->cs_next = 0;
		iter++;

		err = mpool_put_object(cpdc_sgl_mpool, sgl);
		if (err) {
			/* TODO-cpdc: ensure error recovery going way-up */
			OSAL_LOG_ERROR("failed to return cpdc sgl desc to pool! #: %2d sgl %p err: %d",
					iter, sgl, err);
			PNSO_ASSERT(0);
		}

		sgl = sgl_next;
	}
}

static struct cpdc_sgl *
populate_sgl(const struct pnso_buffer_list *buf_list)
{
	pnso_error_t err;
	struct cpdc_sgl *sgl_head = NULL;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl = NULL;
	uint32_t count;
	void *ptr;
	uint32_t i;

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
			sgl_prev->cs_next = (uint64_t) sgl;

		i = 0;
		ptr = (void *) buf_list->buffers[i].buf;
		sgl->cs_addr_0 = (uint64_t) osal_virt_to_phy(ptr);
		sgl->cs_len_0 = buf_list->buffers[i].len;
		i++;
		count--;

		if (count == 0) {
			sgl->cs_next = 0;
			break;
		}

		if (count) {
			ptr = (void *) buf_list->buffers[i].buf;
			sgl->cs_addr_1 = (uint64_t) osal_virt_to_phy(ptr);
			sgl->cs_len_1 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		if (count) {
			ptr = (void *) buf_list->buffers[i].buf;
			sgl->cs_addr_2 = (uint64_t) osal_virt_to_phy(ptr);
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

pnso_error_t
cpdc_convert_buffer_list_to_sgl(struct service_info *svc_info,
		const struct pnso_buffer_list *src_buf,
		const struct pnso_buffer_list *dst_buf)
{
#if 0	/* TODO-cpdc: do not check for sgl */
	bool is_sgl;

	is_sgl = pbuf_is_buffer_list_sgl(src_buf);
	if (!is_sgl)
		return EINVAL;

	is_sgl = pbuf_is_buffer_list_sgl(dst_buf);
	if (!is_sgl)
		return EINVAL;
#else
	if (!src_buf || src_buf->count == 0)
		return false;
	if (!dst_buf || dst_buf->count == 0)
		return false;
#endif
	svc_info->si_src_sgl = populate_sgl(src_buf);
	if (!svc_info->si_src_sgl)
		return EINVAL;

	svc_info->si_dst_sgl = populate_sgl(dst_buf);
	if (!svc_info->si_dst_sgl) {
		cpdc_release_sgl(svc_info->si_src_sgl);
		return EINVAL;
	}

	return PNSO_OK;
}

struct cpdc_sgl	*
cpdc_convert_buffer_list_to_sgl_ex(const struct pnso_buffer_list *buf_list)
{
	if (!buf_list || buf_list->count == 0)
		return NULL;

	return populate_sgl(buf_list);
}

void
cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list)
{
	/* EOPNOTSUPP */
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
		PNSO_ASSERT(0);	/* unreachable code */
		return EOPNOTSUPP;
	}
}
