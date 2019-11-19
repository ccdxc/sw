/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_utils.h"
#include "pnso_chain_params.h"
#include "pnso_seq.h"
#include "pnso_seq_ops.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_init.h"

/**
 * TODO-seq:
 *	- although chaining can be done generically for compression
 *	related chains, focus for now is comp+hash bring-up.
 *	- revisit layer violations
 *	- storage_seq_p4pd. vs utils.h
 *
 */
#ifdef NDEBUG
#define PPRINT_SEQUENCER_DESC(d)
#define PPRINT_SEQUENCER_INFO(sqi)
#define PPRINT_SEQUENCER_ACCOUNTING(sqi)
#define PPRINT_CPDC_CHAIN_PARAMS(cp)
#define PPRINT_CRYPTO_CHAIN_PARAMS(cp)
#else
#define PPRINT_SEQUENCER_DESC(d)	pprint_seq_desc(d)
#define PPRINT_SEQUENCER_INFO(sqi)					\
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		pprint_seq_info(sqi);					\
	} while (0)
#define PPRINT_SEQUENCER_ACCOUNTING(sqi)				\
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		pprint_seq_accounting(sqi);				\
	} while (0)
#define PPRINT_CPDC_CHAIN_PARAMS(cp)					\
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		pprint_cpdc_chain_params(cp);				\
	} while (0)
#define PPRINT_CRYPTO_CHAIN_PARAMS(cp)					\
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		pprint_crypto_chain_params(cp);				\
	} while (0)
#endif

static void __attribute__((unused))
pprint_seq_desc(const struct sequencer_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "seq_desc", (uint64_t) desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "seq_desc_pa",
			osal_virt_to_phy((void *) desc));

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sd_desc_addr", desc->sd_desc_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sd_pndx_addr", desc->sd_pndx_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sd_pndx_shadow_addr",
			desc->sd_pndx_shadow_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sd_ring_addr", desc->sd_ring_addr);

	OSAL_LOG_DEBUG("%30s: %d", "sd_desc_size", desc->sd_desc_size);
	OSAL_LOG_DEBUG("%30s: %d", "sd_pndx_size", desc->sd_pndx_size);
	OSAL_LOG_DEBUG("%30s: %d", "sd_ring_size", desc->sd_ring_size);
	OSAL_LOG_DEBUG("%30s: %d", "sd_batch_mode", desc->sd_batch_mode);
	OSAL_LOG_DEBUG("%30s: %d", "sd_batch_size", desc->sd_batch_size);
	OSAL_LOG_DEBUG("%30s: %d", "sd_rate_limit_en", desc->sd_rate_limit_en);
}

static void __attribute__((unused))
pprint_seq_info(const struct sequencer_info *seq_info)
{
	if (!seq_info)
		return;

	OSAL_LOG_DEBUG("%30s: %s", "sqi_ring", seq_info->sqi_ring->name);
	OSAL_LOG_DEBUG("%30s: %d", "sqi_qtype", seq_info->sqi_qtype);
	OSAL_LOG_DEBUG("%30s: %d", "sqi_status_qtype",
			seq_info->sqi_status_qtype);
	OSAL_LOG_DEBUG("%30s: %d", "sqi_index", seq_info->sqi_index);
	OSAL_LOG_DEBUG("%30s: %d", "sqi_batch_mode", seq_info->sqi_batch_mode);
	OSAL_LOG_DEBUG("%30s: %d", "sqi_batch_size", seq_info->sqi_batch_size);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqi_sqi_desc",
			(uint64_t) seq_info->sqi_desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqi_status_desc",
			(uint64_t) seq_info->sqi_status_desc);
}

static void __attribute__((unused))
pprint_seq_accounting(const struct sequencer_info *seq_info)
{
	struct queue *q;

	if (!seq_info || !seq_info->sqi_seq_q)
		return;

	q = seq_info->sqi_seq_q;
	OSAL_LOG_DEBUG("%30s: %d", "seq qid", q->qid);
	OSAL_LOG_DEBUG("%30s: %d", "seq qgroup", q->qgroup);
	OSAL_LOG_DEBUG("%30s: %d", "seq descs_inuse",
			osal_atomic_read(&q->descs_inuse));
	OSAL_LOG_DEBUG("%30s: %d", "seq total_takes",
			seq_info->sqi_seq_total_takes);
	OSAL_LOG_DEBUG("%30s: %d", "seq status_total_takes",
			seq_info->sqi_status_total_takes);
}

static void __attribute__((unused))
pprint_cpdc_chain_params(const struct cpdc_chain_params *chain_params)
{
	const struct sequencer_spec *spec;
	const struct ring_spec *ring_spec;
	const struct cpdc_chain_params_command *cmd;

	if (!chain_params)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "cpdc_chain_params",
			(uint64_t) chain_params);

	spec = &chain_params->ccp_seq_spec;
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sequencer_spec", (uint64_t) spec);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_q",
			(uint64_t)spec->sqs_seq_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_status_q",
			(uint64_t)spec->sqs_seq_status_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_next_q",
			spec->sqs_seq_next_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_next_status_q",
			spec->sqs_seq_next_status_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_ret_doorbell_addr",
			spec->sqs_ret_doorbell_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_ret_doorbell_data",
			spec->sqs_ret_doorbell_data);
	OSAL_LOG_DEBUG("%30s: %d", "sqs_ret_seq_status_index",
			spec->sqs_ret_seq_status_index);

	ring_spec = &chain_params->ccp_ring_spec;
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ring_spec", (uint64_t) ring_spec);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_ring_addr",
			ring_spec->rs_ring_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_pndx_addr",
			ring_spec->rs_pndx_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_pndx_shadow_addr",
			ring_spec->rs_pndx_shadow_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_desc_addr",
			ring_spec->rs_desc_addr);
	OSAL_LOG_DEBUG("%30s: %d", "rs_desc_size",
			ring_spec->rs_desc_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_pndx_size",
			ring_spec->rs_pndx_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_ring_size",
			ring_spec->rs_ring_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_num_descs",
			ring_spec->rs_num_descs);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_status_addr_0",
			chain_params->ccp_status_addr_0);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_status_addr_1",
			chain_params->ccp_status_addr_1);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_comp_buf_addr",
			chain_params->ccp_comp_buf_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_alt_buf_addr",
			chain_params->ccp_alt_buf_addr);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_aol_src_vec_addr",
			chain_params->ccp_aol_src_vec_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_aol_dst_vec_addr",
			chain_params->ccp_aol_dst_vec_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_sgl_vec_addr",
			chain_params->ccp_sgl_vec_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_pad_buf_addr",
			chain_params->ccp_pad_buf_addr);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_intr_addr",
			chain_params->ccp_intr_addr);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_intr_data",
			chain_params->ccp_intr_data);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_status_len",
			chain_params->ccp_status_len);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_data_len",
			chain_params->ccp_data_len);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_alt_data_len",
			chain_params->ccp_alt_data_len);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_num_alt_descs",
			chain_params->ccp_num_alt_descs);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_hdr_version",
			chain_params->ccp_hdr_version);

	OSAL_LOG_DEBUG("%30s: %d", "ccp_status_offset_0",
			chain_params->ccp_status_offset_0);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_pad_boundary_shift",
			chain_params->ccp_pad_boundary_shift);

	cmd = &chain_params->ccp_cmd;
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_data_len_from_desc",
			cmd->ccpc_data_len_from_desc);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_status_dma_en",
			cmd->ccpc_status_dma_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_next_doorbell_en",
			cmd->ccpc_next_doorbell_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_intr_en",
			cmd->ccpc_intr_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_next_db_action_ring_push",
			cmd->ccpc_next_db_action_ring_push);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_stop_chain_on_error",
			cmd->ccpc_stop_chain_on_error);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_chain_alt_desc_on_error",
			cmd->ccpc_chain_alt_desc_on_error);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_aol_update_en",
			cmd->ccpc_aol_update_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_update_en",
			cmd->ccpc_sgl_update_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_sparse_format_en",
			cmd->ccpc_sgl_sparse_format_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_pdma_en",
			cmd->ccpc_sgl_pdma_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_pdma_pad_only",
			cmd->ccpc_sgl_pdma_pad_only);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_pdma_alt_src_on_error",
			cmd->ccpc_sgl_pdma_alt_src_on_error);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_desc_vec_push_en",
			cmd->ccpc_desc_vec_push_en);
	OSAL_LOG_DEBUG("%30s: %d", "integ_data0_wr_en",
			cmd->integ_data0_wr_en);
	OSAL_LOG_DEBUG("%30s: %d", "integ_data_null_en",
			cmd->integ_data_null_en);
	OSAL_LOG_DEBUG("%30s: %d", "desc_dlen_update_en",
			cmd->desc_dlen_update_en);
	OSAL_LOG_DEBUG("%30s: %d", "hdr_version_wr_en",
			cmd->hdr_version_wr_en);
	OSAL_LOG_DEBUG("%30s: %d", "cp_hdr_update_en",
			cmd->cp_hdr_update_en);
	OSAL_LOG_DEBUG("%30s: %d", "status_len_no_hdr",
			cmd->status_len_no_hdr);
	OSAL_LOG_DEBUG("%30s: %d", "padding_en",
			cmd->padding_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_src_en",
			chain_params->ccp_rl_control.rate_limit_src_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_dst_en",
			chain_params->ccp_rl_control.rate_limit_dst_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_en",
			chain_params->ccp_rl_control.rate_limit_en);
}

static void __attribute__((unused))
pprint_crypto_chain_params(const struct crypto_chain_params *chain_params)
{
	const struct sequencer_spec *spec;
	const struct ring_spec *ring_spec;
	const struct crypto_chain_params_command *cmd;

	if (!chain_params)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "crypto_chain_params",
			(uint64_t) chain_params);

	spec = &chain_params->ccp_seq_spec;
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sequencer_spec", (uint64_t) spec);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_q",
			(uint64_t)spec->sqs_seq_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_status_q",
			(uint64_t)spec->sqs_seq_status_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_next_q",
			spec->sqs_seq_next_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_seq_next_status_q",
			spec->sqs_seq_next_status_q);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_ret_doorbell_addr",
			spec->sqs_ret_doorbell_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "sqs_ret_doorbell_data",
			spec->sqs_ret_doorbell_data);
	OSAL_LOG_DEBUG("%30s: %d", "sqs_ret_seq_status_index",
			spec->sqs_ret_seq_status_index);

	ring_spec = &chain_params->ccp_ring_spec;
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ring_spec", (uint64_t) ring_spec);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_ring_addr",
			ring_spec->rs_ring_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_pndx_addr",
			ring_spec->rs_pndx_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_pndx_shadow_addr",
			ring_spec->rs_pndx_shadow_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "rs_desc_addr",
			ring_spec->rs_desc_addr);
	OSAL_LOG_DEBUG("%30s: %d", "rs_desc_size",
			ring_spec->rs_desc_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_pndx_size",
			ring_spec->rs_pndx_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_ring_size",
			ring_spec->rs_ring_size);
	OSAL_LOG_DEBUG("%30s: %d", "rs_num_descs",
			ring_spec->rs_num_descs);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_status_addr_0",
			chain_params->ccp_status_addr_0);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_status_addr_1",
			chain_params->ccp_status_addr_1);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_crypto_buf_addr",
			chain_params->ccp_crypto_buf_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_comp_sgl_src_addr",
			chain_params->ccp_comp_sgl_src_addr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_sgl_pdma_dst_addr",
			chain_params->ccp_sgl_pdma_dst_addr);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ccp_intr_addr",
			chain_params->ccp_intr_addr);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_intr_data",
			chain_params->ccp_intr_data);

	OSAL_LOG_DEBUG("%30s: %d", "ccp_status_offset_0",
			chain_params->ccp_status_offset_0);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_status_len",
			chain_params->ccp_status_len);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_blk_boundary_shift",
			chain_params->ccp_blk_boundary_shift);
	OSAL_LOG_DEBUG("%30s: %d", "ccp_data_len",
			chain_params->ccp_data_len);

	cmd = &chain_params->ccp_cmd;
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_status_dma_en",
			cmd->ccpc_status_dma_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_next_doorbell_en",
			cmd->ccpc_next_doorbell_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_intr_en",
			cmd->ccpc_intr_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_next_db_action_ring_push",
			cmd->ccpc_next_db_action_ring_push);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_stop_chain_on_error",
			cmd->ccpc_stop_chain_on_error);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_comp_len_update_en",
			cmd->ccpc_comp_len_update_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_comp_sgl_src_en",
			cmd->ccpc_comp_sgl_src_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_comp_sgl_src_vec_en",
			cmd->ccpc_comp_sgl_src_vec_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_sparse_format_en",
			cmd->ccpc_sgl_sparse_format_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_pdma_en",
			cmd->ccpc_sgl_pdma_en);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_sgl_pdma_len_from_desc",
			cmd->ccpc_sgl_pdma_len_from_desc);
	OSAL_LOG_DEBUG("%30s: %d", "ccpc_desc_vec_push_en",
			cmd->ccpc_desc_vec_push_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_src_en",
			chain_params->ccp_rl_control.rate_limit_src_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_dst_en",
			chain_params->ccp_rl_control.rate_limit_dst_en);
	OSAL_LOG_DEBUG("%30s: %d", "rate_limit_en",
			chain_params->ccp_rl_control.rate_limit_en);
}

static void
__write_bit_(uint8_t *p, unsigned int bit_off, bool val)
{
	unsigned int start_byte;
	uint8_t mask;

	/* Assumes p was already memset to zero, no need to clear bits */
	if (val) {
		start_byte = bit_off >> 3;
		mask = 1 << (7 - (bit_off & 7));
		p[start_byte] |= mask;
	}
}

/* Assumes bit_off is an even multiple of 8, and length is 8 */
static void
__write_8bits_(uint8_t *p, unsigned int bit_off, uint8_t val)
{
	unsigned int start_byte;

	if (val) {
		start_byte = bit_off >> 3;
		p[start_byte] = val;
	}
}

/* Assumes 1 <= size_in_bits <= 64 */
static void
write_bits(void *ptr, unsigned int start_bit_offset,
		unsigned int size_in_bits, uint64_t value)
{
	uint8_t *p = (uint8_t *)ptr;
	int bit_no = size_in_bits - 1;
	int off = start_bit_offset;

	/* First write non-byte-aligned prefix */
	for (; (off & 7) != 0 && bit_no >= 0; bit_no--, off++) {
		__write_bit_(p, off,
			     value & (1ull << bit_no));
	}

	/* Now write byte-aligned middle */
	for (; bit_no >= 7; bit_no -= 8, off += 8) {
		__write_8bits_(p, off,
			       (value >> (bit_no - 7)) & 0xff);
	}

	/* Finally write non-byte-aligned suffix */
	for (; bit_no >= 0; bit_no--, off++) {
		__write_bit_(p, off,
			     value & (1ull << bit_no));
	}
}

/* Assumes bit_offset is an even multiple of 64, and length is 64 */
static void
write_64bits(void *ptr, unsigned int start_bit_offset,
	     uint64_t value)
{
	uint64_t *p = (uint64_t *)(ptr + (start_bit_offset / 8));

	*p = cpu_to_be64(value);
}

#define write_bit_fields(p, off, sz, val) \
	if ((sz) == 64 && ((off) & 63) == 0) { \
		if (val) write_64bits(p, off, val); \
	} else if ((sz) == 1) { \
		__write_bit_((uint8_t *) (p), off, val); \
	} else { \
		if (val) write_bits(p, off, sz, val); \
	}


static void
fill_cpdc_seq_status_desc(struct cpdc_chain_params *chain_params,
		uint8_t *seq_status_desc)
{
	struct next_db_spec *next_db_spec;
	struct ring_spec *ring_spec;
	struct cpdc_chain_params_command *cmd;
	struct seq_cpdc_status_desc0 *desc0;
	struct seq_cpdc_status_desc1 *desc1;

	desc0 = (struct seq_cpdc_status_desc0 *)seq_status_desc;
	desc1 = (struct seq_cpdc_status_desc1 *)(seq_status_desc + sizeof(*desc0));

	OSAL_STATIC_ASSERT((sizeof(*desc0) == sizeof(*desc1)) && 
			   (sizeof(*desc1) == (SONIC_SEQ_STATUS_Q_DESC_SIZE/2)));
	ring_spec = &chain_params->ccp_ring_spec;
	next_db_spec = &chain_params->ccp_next_db_spec;
	cmd = &chain_params->ccp_cmd;

	memset(seq_status_desc, 0, SONIC_SEQ_STATUS_Q_DESC_SIZE);
	desc0->status_addr0 = cpu_to_be64(chain_params->ccp_status_addr_0);
	desc0->status_addr1 = cpu_to_be64(chain_params->ccp_status_addr_1);
	desc0->intr_addr = cpu_to_be64(chain_params->ccp_intr_addr);
	desc0->intr_data = htonl(chain_params->ccp_intr_data);
	desc0->status_len = htons(chain_params->ccp_status_len);
	desc0->status_offset = chain_params->ccp_status_offset_0;
	desc0->num_alt_descs = chain_params->ccp_num_alt_descs;

	if (cmd->ccpc_next_db_action_ring_push) {
		desc0->next_db.addr = cpu_to_be64(ring_spec->rs_ring_addr);
		desc0->next_db.data = cpu_to_be64(ring_spec->rs_desc_addr);
                desc0->push.num_descs = htons(ring_spec->rs_num_descs);

#ifdef HAVE_RING_TEMPLATES
		/*
		 * Use template here
		 */
#else
		write_bit_fields(seq_status_desc, 128, 34,
				ring_spec->rs_pndx_addr);
		write_bit_fields(seq_status_desc, 162, 34,
				ring_spec->rs_pndx_shadow_addr);
		write_bit_fields(seq_status_desc, 196, 4,
				ring_spec->rs_desc_size);
		write_bit_fields(seq_status_desc, 200, 3,
				ring_spec->rs_pndx_size);
		write_bit_fields(seq_status_desc, 203, 5,
				ring_spec->rs_ring_size);
#endif
	} else {
		desc0->next_db.addr = cpu_to_be64(next_db_spec->nds_addr);
		desc0->next_db.data = cpu_to_be64(next_db_spec->nds_data);
	}

	desc0->options.status_dma_en = cmd->ccpc_status_dma_en;
	desc0->options.next_db_en = cmd->ccpc_next_doorbell_en;
	desc0->options.intr_en = cmd->ccpc_intr_en;
	desc0->options.action_push = cmd->ccpc_next_db_action_ring_push;
	desc0->options.rate_limit_en = chain_params->ccp_rl_control.rate_limit_en;
	desc0->options.rate_limit_src_en = chain_params->ccp_rl_control.rate_limit_src_en;
	desc0->options.rate_limit_dst_en = chain_params->ccp_rl_control.rate_limit_dst_en;

	desc1->comp_buf_addr = cpu_to_be64(chain_params->ccp_comp_buf_addr);
	desc1->aol_src_vec_addr = cpu_to_be64(chain_params->ccp_aol_src_vec_addr);
	desc1->aol_dst_vec_addr = cpu_to_be64(chain_params->ccp_aol_dst_vec_addr);
	desc1->sgl_vec_addr = cpu_to_be64(chain_params->ccp_sgl_vec_addr);
	desc1->pad_buf_addr = cpu_to_be64(chain_params->ccp_pad_buf_addr);
	desc1->alt_buf_addr = cpu_to_be64(chain_params->ccp_alt_buf_addr);
	desc1->data_len = htonl(chain_params->ccp_data_len);
	desc1->alt_data_len = htonl(chain_params->ccp_alt_data_len);
	desc1->hdr_version = htons(chain_params->ccp_hdr_version);

	desc1->options.blk_boundary_shift = chain_params->ccp_pad_boundary_shift;
	desc1->options.stop_chain_on_error = cmd->ccpc_stop_chain_on_error;
	desc1->options.data_len_from_desc = cmd->ccpc_data_len_from_desc;
	desc1->options.aol_update_en = cmd->ccpc_aol_update_en;
	desc1->options.sgl_update_en = cmd->ccpc_sgl_update_en;
	desc1->options.sgl_sparse_format_en = cmd->ccpc_sgl_sparse_format_en;
	desc1->options.sgl_pdma_en = cmd->ccpc_sgl_pdma_en;
	desc1->options.sgl_pdma_pad_only = cmd->ccpc_sgl_pdma_pad_only;
	desc1->options.sgl_pdma_alt_src_on_error = cmd->ccpc_sgl_pdma_alt_src_on_error;
	desc1->options.desc_vec_push_en = cmd->ccpc_desc_vec_push_en;
	desc1->options.chain_alt_desc_on_error = cmd->ccpc_chain_alt_desc_on_error;
	desc1->options.integ_data0_wr_en = cmd->integ_data0_wr_en;
	desc1->options.integ_data_null_en = cmd->integ_data_null_en;
	desc1->options.desc_dlen_update_en = cmd->desc_dlen_update_en;
	desc1->options.hdr_version_wr_en = cmd->hdr_version_wr_en;
	desc1->options.cp_hdr_update_en = cmd->cp_hdr_update_en;
	desc1->options.status_len_no_hdr = cmd->status_len_no_hdr;
	desc1->options.padding_en = cmd->padding_en;
}

static void
fill_crypto_seq_status_desc(struct crypto_chain_params *chain_params,
		uint8_t *seq_status_desc)
{
	struct next_db_spec *next_db_spec;
	struct ring_spec *ring_spec;
	struct crypto_chain_params_command *cmd;
	struct seq_crypto_status_desc0 *desc0;
	struct seq_crypto_status_desc1 *desc1;

	desc0 = (struct seq_crypto_status_desc0 *)seq_status_desc;
	desc1 = (struct seq_crypto_status_desc1 *)(seq_status_desc + sizeof(*desc0));

	OSAL_STATIC_ASSERT((sizeof(*desc0) == sizeof(*desc1)) && 
			    (sizeof(*desc1) == (SONIC_SEQ_STATUS_Q_DESC_SIZE/2)));
	ring_spec = &chain_params->ccp_ring_spec;
	next_db_spec = &chain_params->ccp_next_db_spec;
	cmd = &chain_params->ccp_cmd;

	memset(seq_status_desc, 0, SONIC_SEQ_STATUS_Q_DESC_SIZE);
	desc0->status_addr0 = cpu_to_be64(chain_params->ccp_status_addr_0);
	desc0->status_addr1 = cpu_to_be64(chain_params->ccp_status_addr_1);
	desc0->intr_addr = cpu_to_be64(chain_params->ccp_intr_addr);
	desc0->intr_data = htonl(chain_params->ccp_intr_data);
	desc0->status_len = htons(chain_params->ccp_status_len);
	desc0->status_offset = chain_params->ccp_status_offset_0;

	if (cmd->ccpc_next_db_action_ring_push) {
		desc0->next_db.addr = cpu_to_be64(ring_spec->rs_ring_addr);
		desc0->next_db.data = cpu_to_be64(ring_spec->rs_desc_addr);
                desc0->push.num_descs = htons(ring_spec->rs_num_descs);

#ifdef HAVE_RING_TEMPLATES
		/*
		 * Use template here
		 */
#else
		write_bit_fields(seq_status_desc, 128, 34,
				ring_spec->rs_pndx_addr);
		write_bit_fields(seq_status_desc, 162, 34,
				ring_spec->rs_pndx_shadow_addr);
		write_bit_fields(seq_status_desc, 196, 4,
				ring_spec->rs_desc_size);
		write_bit_fields(seq_status_desc, 200, 3,
				ring_spec->rs_pndx_size);
		write_bit_fields(seq_status_desc, 203, 5,
				ring_spec->rs_ring_size);
#endif
	} else {
		desc0->next_db.addr = cpu_to_be64(next_db_spec->nds_addr);
		desc0->next_db.data = cpu_to_be64(next_db_spec->nds_data);
	}

	desc0->options.status_dma_en = cmd->ccpc_status_dma_en;
	desc0->options.next_db_en = cmd->ccpc_next_doorbell_en;
	desc0->options.intr_en = cmd->ccpc_intr_en;
	desc0->options.action_push = cmd->ccpc_next_db_action_ring_push;
	desc0->options.rate_limit_en = chain_params->ccp_rl_control.rate_limit_en;
	desc0->options.rate_limit_src_en = chain_params->ccp_rl_control.rate_limit_src_en;
	desc0->options.rate_limit_dst_en = chain_params->ccp_rl_control.rate_limit_dst_en;

	desc1->comp_sgl_src_addr = cpu_to_be64(chain_params->ccp_comp_sgl_src_addr);
	desc1->sgl_pdma_dst_addr = cpu_to_be64(chain_params->ccp_sgl_pdma_dst_addr);
	desc1->crypto_buf_addr = cpu_to_be64(chain_params->ccp_crypto_buf_addr);
	desc1->data_len = htonl(chain_params->ccp_data_len);

	desc1->options.blk_boundary_shift = chain_params->ccp_blk_boundary_shift;
	desc1->options.stop_chain_on_error = cmd->ccpc_stop_chain_on_error;
	desc1->options.comp_len_update_en = cmd->ccpc_comp_len_update_en;
	desc1->options.comp_sgl_src_en = cmd->ccpc_comp_sgl_src_en;
	desc1->options.comp_sgl_src_vec_en = cmd->ccpc_comp_sgl_src_vec_en;
	desc1->options.sgl_sparse_format_en = cmd->ccpc_sgl_sparse_format_en;
	desc1->options.sgl_pdma_en = cmd->ccpc_sgl_pdma_en;
	desc1->options.sgl_pdma_len_from_desc = cmd->ccpc_sgl_pdma_len_from_desc;
	desc1->options.desc_vec_push_en = cmd->ccpc_desc_vec_push_en;
}

static pnso_error_t
hw_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size, void **seq_desc_new)
{
	pnso_error_t err = EINVAL;
	struct sonic_accel_ring *ring;
	struct lif *lif;
	struct sequencer_desc *seq_desc;
	struct rate_limit_control rl;
	uint32_t index;
	uint16_t qtype;

	OSAL_LOG_DEBUG("enter ...");

	ring = svc_info->si_seq_info.sqi_ring;
	qtype = svc_info->si_seq_info.sqi_qtype;
	svc_info->si_seq_info.sqi_index = 0;

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(lif);
		goto out;
	}

	err = sonic_get_seq_sq(lif, qtype, &svc_info->si_seq_info.sqi_seq_q);
	if (err) {
		OSAL_ASSERT(!err);
		goto out;
	}

	seq_desc = sonic_q_consume_entry(svc_info->si_seq_info.sqi_seq_q,
			&index);
	if (!seq_desc) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("failed to obtain sequencer desc! err: %d", err);
		goto out;
	}
	svc_info->si_seq_info.sqi_index = index;
	svc_info->si_seq_info.sqi_seq_total_takes++;

	memset(seq_desc, 0, sizeof(*seq_desc));

	seq_desc->sd_desc_addr =
		cpu_to_be64(sonic_virt_to_phy((void *) src_desc));
	seq_desc->sd_pndx_addr = cpu_to_be64(ring->accel_ring.ring_pndx_pa);
	seq_desc->sd_pndx_shadow_addr =
		cpu_to_be64(ring->accel_ring.ring_shadow_pndx_pa);
	seq_desc->sd_ring_addr = cpu_to_be64(ring->accel_ring.ring_base_pa);

	seq_desc->sd_desc_size =
		(uint8_t) ilog2(ring->accel_ring.ring_desc_size);
	seq_desc->sd_pndx_size =
		(uint8_t) ilog2(ring->accel_ring.ring_pndx_size);
	seq_desc->sd_ring_size = (uint8_t) ilog2(ring->accel_ring.ring_size);
	seq_desc->sd_src_data_len = htonl(svc_info->si_seq_info.sqi_src_data_len);
	seq_desc->sd_dst_data_len = htonl(svc_info->si_seq_info.sqi_dst_data_len);
	svc_rate_limit_control_eval(svc_info, &rl);
	seq_desc->sd_rate_limit_src_en = rl.rate_limit_src_en;
	seq_desc->sd_rate_limit_dst_en = rl.rate_limit_dst_en;
	seq_desc->sd_rate_limit_en = rl.rate_limit_en;

	if (svc_info->si_seq_info.sqi_batch_mode) {
		seq_desc->sd_batch_mode = true;
		seq_desc->sd_batch_size =
			cpu_to_be16(svc_info->si_seq_info.sqi_batch_size);
	}

	OSAL_LOG_INFO("ring: %s index: %u src_desc: 0x" PRIx64 "  desc_size: %lu batch_mode: %d batch_size: %d",
			ring->name, index, (uint64_t) src_desc, desc_size,
			svc_info->si_seq_info.sqi_batch_mode,
			svc_info->si_seq_info.sqi_batch_size);

	PPRINT_SEQUENCER_ACCOUNTING(&svc_info->si_seq_info);
	PPRINT_SEQUENCER_DESC(seq_desc);

	*seq_desc_new = (void *) seq_desc;

	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static bool is_db_rung(struct service_info *svc_info)
{
	struct service_chain *sc = svc_info->si_centry->ce_chain_head;

	return (sc->sc_flags & CHAIN_CFLAG_RANG_DB) != 0;
}

static void
hw_cleanup_desc(struct service_info *svc_info)
{
	if (svc_info->si_seq_info.sqi_seq_q) {
		if (is_db_rung(svc_info)) {
			sonic_q_service(
				svc_info->si_seq_info.sqi_seq_q, NULL,
				svc_info->si_seq_info.sqi_seq_total_takes);
		} else {
			sonic_q_unconsume(
				svc_info->si_seq_info.sqi_seq_q,
				svc_info->si_seq_info.sqi_seq_total_takes);
		}
		svc_info->si_seq_info.sqi_seq_total_takes = 0;
	}
}

static pnso_error_t
hw_ring_db(struct service_info *svc_info)
{
	struct batch_page *page;
	struct queue *seq_q;
	struct sequencer_desc *seq_desc;
	pnso_error_t err = PNSO_OK;

	OSAL_LOG_DEBUG("enter ... ");

	seq_q = svc_info->si_seq_info.sqi_seq_q;
	if (!seq_q) {
		err = EINVAL;
		OSAL_LOG_ERROR("failed to get sequencer q!: err: %d", err);
		OSAL_ASSERT(seq_q);
		goto out;
	}

	/*
	 * If db is being rung for a batch page, update descriptor data size
	 * to the total data size represented by the page.
	 */
	page = svc_info->si_batch_info.sbi_page;
	if (page) {
		seq_desc = svc_info->si_seq_info.sqi_desc;
		OSAL_ASSERT(seq_desc);
		seq_desc->sd_src_data_len = htonl(page->bp_src_data_len);
		seq_desc->sd_dst_data_len = htonl(page->bp_dst_data_len);
		seq_desc->sd_rate_limit_src_en =
			page->bp_rl_control.rate_limit_src_en;
		seq_desc->sd_rate_limit_dst_en =
			page->bp_rl_control.rate_limit_dst_en;
		seq_desc->sd_rate_limit_en =
			page->bp_rl_control.rate_limit_en;
	}

	sonic_q_ringdb(seq_q, svc_info->si_seq_info.sqi_index);

out:
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static void
hw_setup_cp_hdr_update(struct service_info *svc_info,
		struct cpdc_desc *cp_desc,
		struct cpdc_chain_params *chain_params)
{
	uint32_t chksum_len;

	if (chn_service_is_cp_hdr_insert_applic(svc_info)) {
		chain_params->ccp_cmd.integ_data0_wr_en =
			cpdc_cp_hdr_chksum_info_get(svc_info, &chksum_len) &&
			(cpdc_desc_is_integ_data_wr_required(cp_desc) || (chksum_len == 0));
                chain_params->ccp_cmd.integ_data_null_en = (chksum_len == 0);

		chain_params->ccp_hdr_version = cpdc_cp_hdr_version_info_get(svc_info);
		chain_params->ccp_cmd.hdr_version_wr_en =
			cpdc_cp_hdr_version_wr_required(chain_params->ccp_hdr_version);

		chain_params->ccp_cmd.cp_hdr_update_en =
			chain_params->ccp_cmd.integ_data0_wr_en ||
			chain_params->ccp_cmd.hdr_version_wr_en;
	}
}

static pnso_error_t
hw_setup_cp_chain_params(struct service_info *svc_info,
		struct cpdc_desc *cp_desc,
		struct cpdc_status_desc *status_desc)
{
	pnso_error_t err = EINVAL;
	struct cpdc_chain_params *chain_params;
	struct sequencer_info *seq_info;
	struct sequencer_spec *seq_spec;
	uint32_t index;
	uint16_t qtype;
	uint8_t *seq_status_desc;
	struct sonic_accel_ring *ring = svc_info->si_seq_info.sqi_ring;

	OSAL_LOG_DEBUG("enter ...");

	chain_params = &svc_info->si_cpdc_chain;
	seq_spec = &chain_params->ccp_seq_spec;

	seq_info = &svc_info->si_seq_info;
	qtype = seq_info->sqi_qtype;

	err = seq_get_statusq(svc_info, &seq_spec->sqs_seq_status_q);
	if (err) {
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq err: %d",
				err);
		return err;
	}

	seq_status_desc = (uint8_t *) sonic_q_consume_entry(
			seq_spec->sqs_seq_status_q, &index);
	if (!seq_status_desc) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq desc! err: %d",
				err);
		goto out;
	}
	svc_info->si_seq_info.sqi_status_total_takes++;
	seq_info->sqi_status_desc = seq_status_desc;

	/* skip sqs_seq_next_q/sqs_seq_next_status_q not needed for comp+hash */

	cp_desc->cd_db_addr = sonic_get_lif_local_dbaddr();
	cp_desc->cd_db_data = sonic_q_ringdb_data(
			seq_spec->sqs_seq_status_q, index);
	cp_desc->u.cd_bits.cc_db_on = 1;

	if (chn_service_is_cp_padding_applic(svc_info)) {
		chain_params->ccp_pad_buf_addr = pad_buffer;
		chain_params->ccp_cmd.padding_en = 1;
	}

	/* set ccp_pad_boundary_shift whether or not padding is applicable */
	chain_params->ccp_pad_boundary_shift =
		(uint8_t) ilog2(PNSO_MEM_ALIGN_PAGE);

	chain_params->ccp_sgl_vec_addr = cp_desc->cd_dst;
	chain_params->ccp_comp_buf_addr = svc_info->si_dst_sgl.sgl->cs_addr_0;

	chain_params->ccp_data_len = svc_info->si_dst_blist.len;
	chain_params->ccp_alt_data_len = svc_info->si_bof_blist.len;
	if (chn_service_has_interm_blist(svc_info)) {
		if (chn_service_has_sgl_pdma(svc_info)) {
			chain_params->ccp_cmd.ccpc_sgl_pdma_en = 1;
			chain_params->ccp_aol_dst_vec_addr =
				sonic_virt_to_phy(svc_info->si_sgl_pdma);

			SGL_PDMA_PPRINT(chain_params->ccp_aol_dst_vec_addr);
		}
	} else  {
		chain_params->ccp_cmd.ccpc_sgl_pdma_pad_only = 1;
		chain_params->ccp_cmd.ccpc_sgl_pdma_en =
			chain_params->ccp_cmd.padding_en;
	}

	err = cpdc_setup_status_chain_dma(svc_info, chain_params);
	if (err)
		goto out;

	chain_params->ccp_cmd.ccpc_stop_chain_on_error = 1;

	if (svc_info->si_desc_flags & PNSO_CP_DFLAG_BYPASS_ONFAIL) {
		chain_params->ccp_cmd.ccpc_chain_alt_desc_on_error = 1;
		chain_params->ccp_cmd.ccpc_stop_chain_on_error = 0;
	}

	hw_setup_cp_hdr_update(svc_info, cp_desc, chain_params);

	OSAL_LOG_INFO("ring: %s index: %u src_desc: 0x" PRIx64 " status_desc: 0x" PRIx64 "",
			ring->name, index, (uint64_t) cp_desc,
			(uint64_t) status_desc);

	PPRINT_SEQUENCER_INFO(seq_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

/*
 * This function is similar to hw_setup_cp_chain_params(), except
 * it assumes the caller has already made all the necessary modifications
 * to the service's chain parameters template.
 */
static pnso_error_t
hw_setup_cpdc_chain(struct service_info *svc_info,
		    struct cpdc_desc *desc)
{
	struct sequencer_info *seq_info;
	struct sequencer_spec *seq_spec;
	uint32_t statusq_index;
	pnso_error_t err;

	seq_spec = &svc_info->si_cpdc_chain.ccp_seq_spec;
	seq_info = &svc_info->si_seq_info;
	err = seq_get_statusq(svc_info, &seq_spec->sqs_seq_status_q);
	if (err) {
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq err: %d",
				err);
		return err;
	}

	seq_info->sqi_status_desc =
		(uint8_t *)sonic_q_consume_entry(seq_spec->sqs_seq_status_q,
						 &statusq_index);
	if (!seq_info->sqi_status_desc) {
		OSAL_LOG_DEBUG("failed to obtain cpdc sequencer statusq desc");
		return EAGAIN;
	}
	svc_info->si_seq_info.sqi_status_total_takes++;
	desc->cd_db_addr = sonic_get_lif_local_dbaddr();
	desc->cd_db_data =
		sonic_q_ringdb_data(seq_spec->sqs_seq_status_q, statusq_index);
	desc->u.cd_bits.cc_db_on = 1;

	OSAL_LOG_DEBUG("ring: %s index: %u desc: 0x"PRIx64,
		       seq_info->sqi_ring->name, statusq_index, (uint64_t)desc);
	fill_cpdc_seq_status_desc(&svc_info->si_cpdc_chain,
				  seq_info->sqi_status_desc);

	PPRINT_SEQUENCER_INFO(seq_info);
	PPRINT_CPDC_CHAIN_PARAMS(&svc_info->si_cpdc_chain);

	return PNSO_OK;
}

static pnso_error_t
hw_setup_cp_pad_chain_params(struct service_info *svc_info,
		struct cpdc_desc *cp_desc,
		struct cpdc_status_desc *status_desc)
{
	pnso_error_t err = EINVAL;
	struct cpdc_chain_params *chain_params;
	struct sequencer_info *seq_info;
	struct sequencer_spec *seq_spec;
	uint32_t index;
	uint16_t qtype;
	uint8_t *seq_status_desc;
	struct sonic_accel_ring *ring = svc_info->si_seq_info.sqi_ring;

	OSAL_LOG_DEBUG("enter ...");

	chain_params = &svc_info->si_cpdc_chain;
	seq_spec = &chain_params->ccp_seq_spec;

	seq_info = &svc_info->si_seq_info;
	qtype = seq_info->sqi_qtype;

	err = seq_get_statusq(svc_info, &seq_spec->sqs_seq_status_q);
	if (err) {
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq err: %d",
				err);
		return err;
	}

	seq_status_desc = (uint8_t *) sonic_q_consume_entry(
			seq_spec->sqs_seq_status_q, &index);
	if (!seq_status_desc) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq desc! err: %d",
				err);
		goto out;
	}

	svc_info->si_seq_info.sqi_status_total_takes++;
	seq_info->sqi_status_desc = seq_status_desc;

	/* skip sqs_seq_next_q/sqs_seq_next_status_q not needed for cp+pad */

	chain_params->ccp_next_db_spec.nds_addr =
		sonic_virt_to_phy((void *)cpdc_cp_pad_cpl_addr_get(status_desc));
	chain_params->ccp_next_db_spec.nds_data =
		cpu_to_be64(CPDC_PAD_STATUS_DATA);

	cp_desc->cd_db_addr = sonic_get_lif_local_dbaddr();
	cp_desc->cd_db_data =
		sonic_q_ringdb_data(seq_spec->sqs_seq_status_q, index);
	cp_desc->u.cd_bits.cc_db_on = 1;

	chain_params->ccp_cmd.ccpc_next_doorbell_en = 1;
	chain_params->ccp_cmd.ccpc_next_db_action_ring_push = 0;

	chain_params->ccp_cmd.ccpc_stop_chain_on_error = 1;
	chain_params->ccp_cmd.ccpc_sgl_pdma_en = 1;
	chain_params->ccp_cmd.ccpc_sgl_update_en = 1;
	chain_params->ccp_cmd.ccpc_sgl_pdma_pad_only = 1;

	chain_params->ccp_pad_buf_addr = pad_buffer;
	chain_params->ccp_cmd.padding_en = 1;
	chain_params->ccp_pad_boundary_shift =
			(uint8_t) ilog2(PNSO_MEM_ALIGN_PAGE);
	chain_params->ccp_data_len = svc_info->si_dst_blist.len;
	chain_params->ccp_alt_data_len = svc_info->si_bof_blist.len;

	err = cpdc_setup_status_chain_dma(svc_info, chain_params);
	if (err)
		goto out;

	hw_setup_cp_hdr_update(svc_info, cp_desc, chain_params);

	chain_params->ccp_sgl_vec_addr = cp_desc->cd_dst;
	chain_params->ccp_comp_buf_addr = svc_info->si_dst_sgl.sgl->cs_addr_0;

	fill_cpdc_seq_status_desc(chain_params, seq_info->sqi_status_desc);
	PPRINT_CPDC_CHAIN_PARAMS(chain_params);

	OSAL_LOG_INFO("ring: %s index: %u src_desc: 0x" PRIx64 " status_desc: 0x" PRIx64 "",
			ring->name, index, (uint64_t) cp_desc,
			(uint64_t) status_desc);

	PPRINT_SEQUENCER_INFO(seq_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
hw_setup_hashorchksum_chain_params(struct cpdc_chain_params *chain_params,
		struct service_info *svc_info,
		struct cpdc_desc *desc, struct cpdc_sgl *sgl)
{
	pnso_error_t err = EINVAL;
	struct sonic_accel_ring *ring = svc_info->si_seq_info.sqi_ring;
	struct ring_spec *ring_spec;
	struct service_info *svc_prev = chn_service_prev_svc_get(svc_info);
	bool scratch_buf_in_use;

	OSAL_LOG_DEBUG("enter ...");

	ring_spec = &chain_params->ccp_ring_spec;

	ring_spec->rs_ring_addr = ring->accel_ring.ring_base_pa;
	ring_spec->rs_pndx_addr = ring->accel_ring.ring_pndx_pa;
	ring_spec->rs_pndx_shadow_addr = ring->accel_ring.ring_shadow_pndx_pa;
	ring_spec->rs_desc_addr = sonic_virt_to_phy((void *) desc);
	ring_spec->rs_desc_size =
		(uint8_t) ilog2(ring->accel_ring.ring_desc_size);
	ring_spec->rs_pndx_size =
		(uint8_t) ilog2(ring->accel_ring.ring_pndx_size);
	ring_spec->rs_ring_size = (uint8_t) ilog2(ring->accel_ring.ring_size);
	ring_spec->rs_num_descs = svc_info->si_num_tags;
	chain_params->ccp_num_alt_descs = svc_info->si_num_bof_tags;

	chain_params->ccp_cmd.ccpc_next_doorbell_en = 1;
	chain_params->ccp_cmd.ccpc_next_db_action_ring_push = 1;

	chain_params->ccp_sgl_vec_addr = sonic_virt_to_phy((void *) sgl);

	/*
	 * P4+ should not be updating sgl, header length, and vector push,
	 * if DC in the chain has SGLs prepadded with scratch buffer
	 *
	 */
	scratch_buf_in_use = (chn_service_type_is_dc(svc_prev) &&
				(svc_prev->si_dst_blist.len >=
				 CPDC_MIN_USER_BUFFER_LEN)) ? true : false;

	if (!scratch_buf_in_use)
		chain_params->ccp_cmd.ccpc_sgl_update_en = 1;

	if (scratch_buf_in_use)
		chain_params->ccp_cmd.ccpc_data_len_from_desc = 1;

	if (svc_info->si_flags & CHAIN_SFLAG_PER_BLOCK) {
		chain_params->ccp_cmd.ccpc_sgl_sparse_format_en = 1;

		if (chn_service_type_is_cp(svc_prev) &&
		    !chain_params->ccp_cmd.padding_en)
			chain_params->ccp_cmd.desc_dlen_update_en = 1;

		/*
		 * hash/chksum executes multiple requests, one per block; hence,
		 * indicate to P4+ to push a vector of descriptors
		 *
		 */
		if (!scratch_buf_in_use)
			chain_params->ccp_cmd.ccpc_desc_vec_push_en = 1;
	} else {
		if (!scratch_buf_in_use)
			chain_params->ccp_cmd.desc_dlen_update_en = 1;
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
hw_setup_cpdc_chain_status_desc(struct service_info *svc_info)
{
	pnso_error_t err = PNSO_OK;
	struct cpdc_chain_params *chain_params;
	struct sequencer_info *seq_info;

	OSAL_LOG_DEBUG("enter ...");

	chain_params = &svc_info->si_cpdc_chain;
	seq_info = &svc_info->si_seq_info;

	fill_cpdc_seq_status_desc(chain_params, seq_info->sqi_status_desc);
	PPRINT_CPDC_CHAIN_PARAMS(chain_params);
	PPRINT_SEQUENCER_INFO(seq_info);

	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

static void
hw_cleanup_cpdc_chain(struct service_info *svc_info)
{
	struct cpdc_chain_params *cpdc_chain = &svc_info->si_cpdc_chain;

	if (cpdc_chain->ccp_seq_spec.sqs_seq_status_q) {
		if (is_db_rung(svc_info)) {
			sonic_q_service(
				cpdc_chain->ccp_seq_spec.sqs_seq_status_q, NULL,
				svc_info->si_seq_info.sqi_status_total_takes);
		} else {
			sonic_q_unconsume(
				cpdc_chain->ccp_seq_spec.sqs_seq_status_q,
				svc_info->si_seq_info.sqi_status_total_takes);
		}
		svc_info->si_seq_info.sqi_status_total_takes = 0;

		sonic_put_seq_statusq(
				cpdc_chain->ccp_seq_spec.sqs_seq_status_q);

		cpdc_chain->ccp_seq_spec.sqs_seq_status_q = NULL;
	}
}

static pnso_error_t
hw_setup_crypto_chain(struct service_info *svc_info,
		      struct crypto_desc *desc)
{
	struct sequencer_info *seq_info;
	struct sequencer_spec *seq_spec;
	uint32_t statusq_index;
	pnso_error_t err;

	seq_spec = &svc_info->si_crypto_chain.ccp_seq_spec;
	seq_info = &svc_info->si_seq_info;
	err = seq_get_statusq(svc_info, &seq_spec->sqs_seq_status_q);
	if (err) {
		OSAL_LOG_DEBUG("failed to obtain sequencer statusq err: %d",
				err);
		return err;
	}

	seq_info->sqi_status_desc =
		(uint8_t *)sonic_q_consume_entry(seq_spec->sqs_seq_status_q,
						 &statusq_index);
	if (!seq_info->sqi_status_desc) {
		OSAL_LOG_DEBUG("failed to obtain crypto sequencer statusq desc");
		return EAGAIN;
	}
	svc_info->si_seq_info.sqi_status_total_takes++;
	desc->cd_db_addr = sonic_get_lif_local_dbaddr();
	desc->cd_db_data =
		sonic_q_ringdb_data(seq_spec->sqs_seq_status_q, statusq_index);

	OSAL_LOG_DEBUG("ring: %s index: %u desc: 0x"PRIx64,
		       seq_info->sqi_ring->name, statusq_index, (uint64_t)desc);
	fill_crypto_seq_status_desc(&svc_info->si_crypto_chain,
				    seq_info->sqi_status_desc);

	PPRINT_SEQUENCER_INFO(seq_info);
	PPRINT_CRYPTO_CHAIN_PARAMS(&svc_info->si_crypto_chain);

	return PNSO_OK;
}

static void
hw_cleanup_crypto_chain(struct service_info *svc_info)
{
	struct crypto_chain_params *crypto_chain =
		&svc_info->si_crypto_chain;

	if (crypto_chain->ccp_seq_spec.sqs_seq_status_q) {
		if (is_db_rung(svc_info)) {
			sonic_q_service(
				crypto_chain->ccp_seq_spec.sqs_seq_status_q,
				NULL,
				svc_info->si_seq_info.sqi_status_total_takes);
		} else {
			sonic_q_unconsume(
				crypto_chain->ccp_seq_spec.sqs_seq_status_q,
				svc_info->si_seq_info.sqi_status_total_takes);
		}
		svc_info->si_seq_info.sqi_status_total_takes = 0;

		sonic_put_seq_statusq(
				crypto_chain->ccp_seq_spec.sqs_seq_status_q);

		crypto_chain->ccp_seq_spec.sqs_seq_status_q = NULL;
	}
}

const struct sequencer_ops hw_seq_ops = {
	.setup_desc = hw_setup_desc,
	.cleanup_desc = hw_cleanup_desc,
	.ring_db = hw_ring_db,
	.setup_cp_chain_params = hw_setup_cp_chain_params,
	.setup_cpdc_chain = hw_setup_cpdc_chain,
	.setup_cp_pad_chain_params = hw_setup_cp_pad_chain_params,
	.setup_hash_chain_params = hw_setup_hashorchksum_chain_params,
	.setup_chksum_chain_params = hw_setup_hashorchksum_chain_params,
	.setup_cpdc_chain_status_desc = hw_setup_cpdc_chain_status_desc,
	.cleanup_cpdc_chain = hw_cleanup_cpdc_chain,
	.setup_crypto_chain = hw_setup_crypto_chain,
	.cleanup_crypto_chain = hw_cleanup_crypto_chain,
};
