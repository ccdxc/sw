/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_cpdc.h"
#include "pnso_chain.h"

struct cp_header_format {
	uint32_t fmt_idx;
	uint32_t total_hdr_sz;
	uint16_t flags;
	uint16_t type_mask;
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
pnso_error_t cpdc_common_chain(struct chain_entry *centry);

pnso_error_t cpdc_poll(const struct service_info *svc_info,
		struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_common_read_status(struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_update_service_info_sgl(struct service_info *svc_info);

pnso_error_t cpdc_update_service_info_sgls(struct service_info *svc_info);

pnso_error_t cpdc_update_service_info_bof_sgl(struct service_info *svc_info);

struct cpdc_status_desc *cpdc_get_status_desc(struct per_core_resource *pcr,
		bool per_block);

void cpdc_put_status_desc(struct per_core_resource *pcr,
		bool per_block, struct cpdc_status_desc *desc);

pnso_error_t cpdc_setup_status_desc(struct service_info *svc_info,
		bool per_block);

struct cpdc_status_desc *cpdc_get_rmem_status_desc(
		struct per_core_resource *pcr, bool per_block);

void cpdc_put_rmem_status_desc(struct per_core_resource *pcr, bool per_block,
		struct cpdc_status_desc *desc);

pnso_error_t cpdc_setup_rmem_status_desc(struct service_info *svc_info,
		bool per_block);

void cpdc_teardown_rmem_status_desc(struct service_info *svc_info,
		bool per_block);

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

typedef void (*fill_desc_fn_t) (uint32_t algo_type, uint32_t buf_len,
		void *src_buf, struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc);

uint32_t cpdc_fill_per_block_desc(uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct service_buf_list *svc_src_blist,
		struct cpdc_sgl *sgl, struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc,
		fill_desc_fn_t fill_desc_fn);

pnso_error_t
cpdc_setup_desc_blocks(struct service_info *svc_info, uint32_t algo_type,
		fill_desc_fn_t fill_desc_fn);

pnso_error_t cpdc_setup_batch_desc(struct service_info *svc_info,
		struct cpdc_desc *desc);

pnso_error_t cpdc_setup_seq_desc(struct service_info *svc_info,
		struct cpdc_desc *desc, uint32_t num_tags);

pnso_error_t cpdc_setup_interrupt_params(struct service_info *svc_info,
		void *poll_ctx);

void cpdc_update_bof_interrupt_params(struct service_info *svc_info);

void cpdc_cleanup_interrupt_params(struct service_info *svc_info);

void cpdc_update_batch_tags(struct service_info *svc_info,
		uint32_t num_tags);

void cpdc_update_seq_batch_size(const struct service_info *svc_info);

pnso_error_t cpdc_convert_desc_error(int error);

void cpdc_pprint_mpools(struct per_core_resource *pcr);

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
		if (data_len == MAX_CPDC_DST_BUF_LEN)
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

#endif /* __PNSO_CPDC_CMN_H__ */
