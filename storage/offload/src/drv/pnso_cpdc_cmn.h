/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_cpdc.h"
#include "pnso_chain.h"

#define CPDC_COMPRESSION_TYPE_DFLT      PNSO_COMPRESSION_TYPE_LZRW1A

struct cp_header_format {
	uint32_t fmt_idx;
	uint32_t total_hdr_sz;
	uint16_t flags;
	uint16_t type_mask;
	uint16_t chksum_len;
	enum pnso_compression_type pnso_algo;
	uint8_t *static_hdr;
	struct pnso_compression_header_format fmt;
};

struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

#ifdef NDEBUG
#define CPDC_PPRINT_DESC(d)
#define CPDC_PPRINT_STATUS_DESC(d)
#else
#define CPDC_PPRINT_DESC(d)						       \
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		cpdc_pprint_desc(d);					\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
	} while (0)
#define CPDC_PPRINT_STATUS_DESC(d)	cpdc_pprint_status_desc(d)
#endif

/* CPDC common/utility functions */
pnso_error_t cpdc_poll_all(struct service_info *svc_info);

pnso_error_t cpdc_poll(struct service_info *svc_info,
		volatile struct cpdc_status_desc *status_desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

void cpdc_report_suspect_desc(const struct cpdc_desc *desc);
void cpdc_report_suspect_desc_ex(const struct cpdc_desc *desc);

pnso_error_t cpdc_update_service_info_src_sgl(struct service_info *svc_info);
pnso_error_t cpdc_update_service_info_dst_sgl(struct service_info *svc_info);
pnso_error_t cpdc_update_service_info_sgls(struct service_info *svc_info);

struct cpdc_status_desc *cpdc_get_status_desc(struct per_core_resource *pcr,
		bool per_block);

pnso_error_t cpdc_setup_status_desc(struct service_info *svc_info,
		bool per_block);

void cpdc_teardown_status_desc(struct service_info *svc_info);

pnso_error_t
cpdc_setup_rmem_status_desc(struct service_info *svc_info,
			    bool per_block);
pnso_error_t
cpdc_setup_status_chain_dma(struct service_info *svc_info,
			    struct cpdc_chain_params *chain_params);

void cpdc_teardown_rmem_status_desc(struct service_info *svc_info);

struct cpdc_sgl *cpdc_get_sgl(struct per_core_resource *pcr, bool per_block);

void cpdc_put_sgl(struct per_core_resource *pcr, bool per_block,
		struct cpdc_sgl *sgl);

struct cpdc_desc *cpdc_get_batch_bulk_desc(struct mem_pool *mpool);

void cpdc_put_batch_bulk_desc(struct mem_pool *mpool, struct cpdc_desc *desc);

struct cpdc_desc *cpdc_get_desc(struct service_info *svc_info, bool per_block);

void cpdc_put_desc(const struct service_info *svc_info,
		bool per_block, struct cpdc_desc *desc);

uint32_t cpdc_get_desc_size(void);

uint32_t cpdc_get_status_desc_size(void);

uint32_t cpdc_get_sgl_size(void);

uint32_t cpdc_sgl_total_len_get(const struct service_cpdc_sgl *svc_sgl);

struct cpdc_status_desc *cpdc_get_next_status_desc(
		struct cpdc_status_desc *desc, uint32_t object_size);

typedef int (*fill_desc_fn_t) (struct service_info *svc_info,
		uint32_t algo_type,
		uint32_t buf_len, void *src_buf,
		struct cpdc_desc *desc, uint32_t block_no);

uint32_t cpdc_fill_per_block_desc(struct service_info *svc_info,
		uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct service_buf_list *svc_src_blist,
		struct cpdc_sgl *sgl, struct cpdc_desc *desc,
		fill_desc_fn_t fill_desc_fn);

pnso_error_t
cpdc_setup_desc_blocks(struct service_info *svc_info, uint32_t algo_type,
		fill_desc_fn_t fill_desc_fn);

pnso_error_t cpdc_setup_interrupt_params(struct service_info *svc_info,
		void *poll_ctx);

void cpdc_update_bof_interrupt_params(struct service_info *svc_info);

void cpdc_cleanup_interrupt_params(struct service_info *svc_info);
void cpdc_update_interrupt_params(struct service_info *svc_info);

void cpdc_update_batch_tags(struct service_info *svc_info,
		uint32_t num_tags);

void cpdc_update_seq_batch_size(const struct service_info *svc_info);

pnso_error_t cpdc_convert_desc_error(int error);

void cpdc_report_mpools(struct per_core_resource *pcr);

pnso_error_t cpdc_setup_rmem_dst_blist(struct service_info *svc_info,
		const struct service_params *svc_params);

void cpdc_teardown_rmem_dst_blist(struct service_info *svc_info);

void cpdc_update_tags(struct service_info *svc_info);

struct cp_header_format *lookup_hdr_format(uint32_t hdr_fmt_idx, bool alloc);

static inline uint32_t
cpdc_desc_data_len_set_eval(enum pnso_service_type svc_type,
			    uint32_t data_len)
{
	switch (svc_type) {
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		if (data_len >= MAX_CPDC_DST_BUF_LEN)
			data_len = 0;
		break;
	default:
		break;
	}

	return data_len;
}

static inline uint32_t
cpdc_desc_data_len_get_eval(enum pnso_service_type svc_type,
			    uint32_t data_len)
{
	switch (svc_type) {
	case PNSO_SVC_TYPE_DECOMPRESS:
		if (data_len == 0)
			data_len = MAX_CPDC_DST_BUF_LEN;
		break;
	default:
		break;
	}

	return data_len;
}

static inline volatile uint64_t *
cpdc_cp_pad_cpl_addr_get(volatile struct cpdc_status_desc *status_desc)
{
	OSAL_ASSERT(sizeof(status_desc->csd_sha) >= sizeof(uint64_t));
	return (uint64_t *)&status_desc->csd_sha[0];
}

static inline bool
cpdc_cp_hdr_chksum_info_get(const struct service_info *svc_info,
			    uint32_t *ret_chksum_len)
{
	struct cp_header_format *hdr_fmt;

	*ret_chksum_len = sizeof(((struct pnso_compression_header *)0)->chksum);
	hdr_fmt = lookup_hdr_format(svc_info->hdr_fmt_idx, false);
	if (hdr_fmt) {
		*ret_chksum_len = hdr_fmt->chksum_len;
		return true;
	}
	return false;
}

static inline uint32_t
cpdc_cp_hdr_version_info_get(const struct service_info *svc_info)
{
	struct cp_header_format *hdr_fmt;

	hdr_fmt = lookup_hdr_format(svc_info->hdr_fmt_idx, false);
	return hdr_fmt ?  hdr_fmt->pnso_algo : CPDC_COMPRESSION_TYPE_DFLT;
}

static inline bool
cpdc_cp_hdr_version_wr_required(uint32_t hdr_version)
{
	return (hdr_version != CPDC_COMPRESSION_TYPE_DFLT);
}

static inline bool
cpdc_desc_is_integ_data_wr_required(const struct cpdc_desc *desc)
{
	return desc->u.cd_bits.cc_integrity_type == CPDC_INTEGRITY_M_ADLER32;
}

#endif /* __PNSO_CPDC_CMN_H__ */
