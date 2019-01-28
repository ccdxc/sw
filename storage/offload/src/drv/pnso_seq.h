/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SEQ_H__
#define __PNSO_SEQ_H__

#include "pnso_crypto.h"
#include "pnso_chain_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Big endian layout for all sequencer structures below
 */
struct seq_next_db_spec {
	uint64_t		addr;
	uint64_t		data;
}  __attribute__((__packed__));

struct seq_cpdc_push_spec {
	uint64_t		pndx_addr	: 34;
	uint64_t		pndx_shadow_addr: 34;
	uint64_t		desc_size	: 4;
	uint64_t		pndx_size	: 3;
	uint64_t		ring_size	: 5;
	uint64_t		num_descs	: 16;
}  __attribute__((__packed__));

struct seq_cpdc_options0 {
	uint8_t			rsvd0		: 4,
				action_push	: 1,
				intr_en		: 1,
				next_db_en	: 1,
				status_dma_en	: 1;
	uint8_t			rsvd1;
	uint8_t			rsvd2;
	uint8_t			rsvd3;
} __attribute__((__packed__));

struct seq_cpdc_status_desc0 {
	struct seq_next_db_spec	next_db;
	struct seq_cpdc_push_spec push;
	uint64_t		status_addr0;
	uint64_t		status_addr1;
	uint64_t		intr_addr;
	uint32_t		intr_data;
	uint16_t		status_len;
	uint8_t			status_offset;
	struct seq_cpdc_options0 options;
	uint8_t			rsvd0;
}  __attribute__((__packed__));

static_assert(sizeof(struct seq_cpdc_status_desc0) == 64)

struct seq_cpdc_options1 {
	uint8_t			aol_pad_en		: 1,
				data_len_from_desc	: 1,
				stop_chain_on_error	: 1,
				blk_boundary_shift	: 5;
	uint8_t			integ_data0_wr_en	: 1,
				chain_alt_desc_on_error	: 1,
				desc_vec_push_en	: 1,
				sgl_pdma_alt_src_on_error:1,
				sgl_pdma_pad_only	: 1,
				sgl_pdma_en		: 1,
				sgl_sparse_format_en	: 1,
				sgl_pad_en		: 1;
	uint8_t			rsvd0                   : 4,
				cp_hdr_update_en	: 1,
				hdr_version_wr_en	: 1,
				desc_dlen_update_en	: 1,
				integ_data_null_en	: 1;
	uint8_t			rsvd1;
} __attribute__((__packed__));

struct seq_cpdc_status_desc1 {
	uint64_t		rsvd1;
	uint64_t		comp_buf_addr;
	uint64_t		aol_src_vec_addr;
	uint64_t		aol_dst_vec_addr;
	uint64_t		sgl_vec_addr;
	uint64_t		pad_buf_addr;
	uint64_t		alt_buf_addr;
	uint16_t		data_len;
	uint16_t		hdr_version;
	struct seq_cpdc_options1 options;
}  __attribute__((__packed__));

static_assert(sizeof(struct seq_cpdc_status_desc1) == 64)

struct seq_crypto_push_spec {
	uint64_t		pndx_addr	: 34;
	uint64_t		pndx_shadow_addr: 34;
	uint64_t		desc_size	: 4;
	uint64_t		pndx_size	: 3;
	uint64_t		ring_size	: 5;
	uint64_t		num_descs	: 16;
}  __attribute__((__packed__));

struct seq_crypto_options0 {
	uint8_t			rsvd0           : 4,
				action_push	: 1,
				intr_en		: 1,
				next_db_en	: 1,
				status_dma_en	: 1;
	uint8_t			rsvd1;
	uint8_t			rsvd2;
	uint8_t			rsvd3;
} __attribute__((__packed__));

struct seq_crypto_status_desc0 {
	struct seq_next_db_spec	next_db;
	struct seq_crypto_push_spec push;
	uint64_t		status_addr0;
	uint64_t		status_addr1;
	uint64_t		intr_addr;
	uint32_t		intr_data;
	uint16_t		status_len;
	uint8_t			status_offset;
	struct seq_crypto_options0 options;
	uint8_t			rsvd0;
}  __attribute__((__packed__));

static_assert(sizeof(struct seq_crypto_status_desc0) == 64)

struct seq_crypto_options1 {
	uint8_t		        comp_sgl_src_en		: 1,
				comp_len_update_en	: 1,
				stop_chain_on_error	: 1,
				blk_boundary_shift	: 5;
	uint8_t			rsvd0			: 3,
				desc_vec_push_en	: 1,
				sgl_pdma_len_from_desc	: 1,
				sgl_pdma_en		: 1,
				sgl_sparse_format_en	: 1,
				comp_sgl_src_vec_en	: 1;
	uint8_t			rsvd1;
	uint8_t			rsvd2;
} __attribute__((__packed__));

struct seq_crypto_status_desc1 {
	uint64_t		comp_sgl_src_addr;
	uint64_t		sgl_pdma_dst_addr;
	uint64_t		crypto_buf_addr;
	uint16_t		data_len;
	struct seq_crypto_options1 options;
	uint16_t		rsvd0;
	uint64_t		rsvd1;
	uint64_t		rsvd2;
	uint64_t		rsvd3;
	uint64_t		rsvd4;
}  __attribute__((__packed__));

static_assert(sizeof(struct seq_crypto_status_desc1) == 64)

void *seq_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size);
void seq_cleanup_desc(struct service_info *svc_info);

void seq_ring_db(struct service_info *svc_info);

pnso_error_t seq_setup_cp_chain_params(struct service_info *svc_info,
	struct cpdc_desc *cp_desc, struct cpdc_status_desc *status_desc);

pnso_error_t seq_setup_cpdc_chain(struct service_info *svc_info,
		   struct cpdc_desc *cp_desc);

pnso_error_t seq_setup_cp_pad_chain_params(struct service_info *svc_info,
	struct cpdc_desc *cp_desc, struct cpdc_status_desc *status_desc);

pnso_error_t seq_setup_hash_chain_params(struct cpdc_chain_params *chain_params,
		struct service_info *svc_info,
		struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl,
		uint32_t num_hash_blks);

pnso_error_t seq_setup_chksum_chain_params(
		struct cpdc_chain_params *chain_params,
		struct service_info *svc_info,
		struct cpdc_desc *chksum_desc, struct cpdc_sgl *sgl,
		uint32_t num_chksum_blks);

pnso_error_t seq_setup_cpdc_chain_status_desc(struct service_info *svc_info);

void seq_cleanup_cpdc_chain(struct service_info *svc_info);

pnso_error_t seq_setup_crypto_chain(struct service_info *svc_info,
			struct crypto_desc *desc);
void seq_cleanup_crypto_chain(struct service_info *svc_info);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_SEQ_H__ */
