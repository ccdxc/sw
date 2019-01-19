/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CHAIN_PARAMS_H__
#define __PNSO_CHAIN_PARAMS_H__

#include "sonic_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct sequencer_desc: represents the descriptor of the sequencer.
 * @sd_desc_addr: specifies the accelerator descriptor address.
 * @sd_pndx_addr: specifies the accelerator producer index register address.
 * @sd_pndx_shadow_addr: specifies the accelerator producer index shadow
 * register address.
 * @sd_ring_address: specifies the accelerator ring address.
 * @sd_desc_size: specifies the size of the descriptor in
 * log2(accelerator descriptor size).
 * @sd_pndx_size: specifies the accelerator producer index size in
 * log2(accelerator producer index size).
 * @sd_ring_size: specifies the accelerator ring size in
 * log2(accelerator ring size).
 * @sd_batch_mode: when set to 1, this field specifies a batch of descriptors
 * to be processed, as specified in 'sd_batch_size' to process.  When set to 0,
 * 'sd_batch_size' field is ignored.
 * @sd_batch_size: field specifies total number of descriptors in the batch.
 * 'sd_desc_addr' is the 1st descriptor of the batch.
 * @sd_filler_1: specifies a filler field to yield an aligned structure.
 * @sd_filler_2: specifies a filler field to yield an aligned structure.
 *
 */
struct sequencer_desc {
	uint64_t sd_desc_addr;
	uint64_t sd_pndx_addr;
	uint64_t sd_pndx_shadow_addr;
	uint64_t sd_ring_addr;
	uint8_t sd_desc_size;
	uint8_t sd_pndx_size;
	uint8_t sd_ring_size;
	uint8_t sd_batch_mode;
	uint16_t sd_batch_size;
	uint16_t sd_filler_0;
	uint64_t sd_filler_1[3];
} __attribute__((packed));

struct next_db_spec {
	uint64_t nds_addr;
	uint64_t nds_data;
};

struct ring_spec {
	uint64_t rs_ring_addr;
	uint64_t rs_pndx_addr;
	uint64_t rs_pndx_shadow_addr;
	uint64_t rs_desc_addr;
	uint8_t rs_desc_size;
	uint8_t rs_pndx_size;
	uint8_t rs_ring_size;
	uint16_t rs_num_descs;
};

struct sequencer_spec {
	struct queue *sqs_seq_q;
	struct queue *sqs_seq_status_q;
	uint64_t sqs_seq_next_q;
	uint64_t sqs_seq_next_status_q;
	uint64_t sqs_ret_doorbell_addr;
	uint64_t sqs_ret_doorbell_data;
	uint16_t sqs_ret_seq_status_index;
};

struct cpdc_chain_params_command {
	uint32_t ccpc_data_len_from_desc:1;
	uint32_t ccpc_status_dma_en:1;
	uint32_t ccpc_next_doorbell_en:1;
	uint32_t ccpc_intr_en:1;
	uint32_t ccpc_next_db_action_ring_push:1;
	uint32_t ccpc_stop_chain_on_error:1;
	uint32_t ccpc_chain_alt_desc_on_error:1;
	uint32_t ccpc_aol_pad_en:1;
	uint32_t ccpc_sgl_pad_en:1;
	uint32_t ccpc_sgl_sparse_format_en:1;
	uint32_t ccpc_sgl_pdma_en:1;
	uint32_t ccpc_sgl_pdma_pad_only:1;
	uint32_t ccpc_sgl_pdma_alt_src_on_error:1;
	uint32_t ccpc_desc_vec_push_en:1;
	uint32_t integ_data0_wr_en:1;
	uint32_t integ_data_null_en:1;
	uint32_t desc_dlen_update_en:1;
};

struct cpdc_chain_params {
	struct sequencer_spec ccp_seq_spec;
	union {
		struct next_db_spec ccp_next_db_spec;
		struct ring_spec ccp_ring_spec;
	};
	uint64_t ccp_status_addr_0;
	uint64_t ccp_status_addr_1;

	uint64_t ccp_comp_buf_addr;
	uint64_t ccp_alt_buf_addr;
	uint64_t ccp_aol_src_vec_addr;
	uint64_t ccp_aol_dst_vec_addr;
	uint64_t ccp_sgl_vec_addr;
	uint64_t ccp_pad_buf_addr;

	uint64_t ccp_intr_addr;
	uint32_t ccp_intr_data;

	uint16_t ccp_status_len;
	uint16_t ccp_data_len;

	uint8_t ccp_status_offset_0;
	uint8_t ccp_pad_boundary_shift;

	struct cpdc_chain_params_command ccp_cmd;
};

struct crypto_chain_params_command {
	uint16_t ccpc_status_dma_en:1;
	uint16_t ccpc_next_doorbell_en:1;
	uint16_t ccpc_intr_en:1;
	uint16_t ccpc_next_db_action_ring_push:1;
	uint16_t ccpc_stop_chain_on_error:1;
	uint16_t ccpc_comp_len_update_en:1;
	uint16_t ccpc_comp_sgl_src_en:1;
	uint16_t ccpc_comp_sgl_src_vec_en:1;
	uint16_t ccpc_sgl_sparse_format_en:1;
	uint16_t ccpc_sgl_pdma_en:1;
	uint16_t ccpc_sgl_pdma_len_from_desc:1;
	uint16_t ccpc_desc_vec_push_en:1;
};

struct crypto_chain_params {
	struct sequencer_spec ccp_seq_spec;
	union {
		struct next_db_spec ccp_next_db_spec;
		struct ring_spec ccp_ring_spec;
	};
	uint64_t ccp_status_addr_0;
	uint64_t ccp_status_addr_1;

	uint64_t ccp_crypto_buf_addr;
	uint64_t ccp_comp_sgl_src_addr;
	uint64_t ccp_sgl_pdma_dst_addr;

	uint64_t ccp_intr_addr;
	uint32_t ccp_intr_data;

	uint16_t ccp_status_len;
	uint16_t ccp_data_len;

	uint8_t  ccp_status_offset_0;
	uint8_t  ccp_blk_boundary_shift;

	struct crypto_chain_params_command ccp_cmd;
};

/*
 * SGL PDMA pamameters
 */
#define SGL_PDMA_TUPLE_MAX_LEN	((1 << 14) - 1)

struct chain_sgl_pdma_tuple {
	uint64_t	addr;
	uint32_t	len;
} __attribute__((packed));

struct chain_sgl_pdma {
	struct chain_sgl_pdma_tuple tuple[4];
	uint64_t                    pad[2];
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_CHAIN_PARAMS_H__ */
