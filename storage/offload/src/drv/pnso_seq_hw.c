/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>
#include <net.h>
#include <kernel.h>

#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"

#include "osal.h"
#include "pnso_api.h"

#include "pnso_seq_ops.h"

/**
 * struct sequencer_desc: represents the descriptor of the sequencer
 * @sd_desc_addr: specifies the accelerator descriptor address
 * @sd_pndx_addr: specifies the accelerator producer index register address
 * @sd_pndx_shadow_addr: specifies the accelerator producer index shadow
 * register address
 * @sd_ring_address: specifies the accelerator ring address
 * @sd_desc_size: specifies the size of the descriptor in
 * log2(accelerator descriptor size)
 * @sd_pndx_size: specifies the accelerator producer index size in
 * log2(accelerator producer index size)
 * @sd_ring_size: specifies the accelerator ring size in
 * log2(accelerator ring size)
 * @sd_batch_mode: specifies the 1st descriptor (i.e. sd_desc_addr) of a batch
 * @sd_batch_size: number of batch descriptors
 * @sd_filler_1: TODO-seq
 * @sd_filler_2:
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

#ifdef NDEBUG
#define PPRINT_SEQUENCER_DESC(d)
#else
#define PPRINT_SEQUENCER_DESC(d)	pprint_seq_desc(d)
#endif

static void __attribute__((unused))
pprint_seq_desc(const struct sequencer_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "seq_desc", (uint64_t) desc);
	OSAL_LOG_INFO("%30s: 0x%llx", "seq_desc_pa",
			osal_virt_to_phy((void *) desc));

	OSAL_LOG_INFO("%30s: 0x%llx", "sd_desc_addr", desc->sd_desc_addr);
	OSAL_LOG_INFO("%30s: 0x%llx", "sd_pndx_addr", desc->sd_pndx_addr);
	OSAL_LOG_INFO("%30s: 0x%llx", "sd_pndx_shadow_addr",
			desc->sd_pndx_shadow_addr);
	OSAL_LOG_INFO("%30s: 0x%llx", "sd_ring_addr", desc->sd_ring_addr);

	OSAL_LOG_INFO("%30s: %d", "sd_desc_size", desc->sd_desc_size);
	OSAL_LOG_INFO("%30s: %d", "sd_pndx_size", desc->sd_pndx_size);
	OSAL_LOG_INFO("%30s: %d", "sd_ring_size", desc->sd_ring_size);
	OSAL_LOG_INFO("%30s: %d", "sd_batch_mode", desc->sd_batch_mode);
	OSAL_LOG_INFO("%30s: %d", "sd_batch_size", desc->sd_batch_size);
}

static void *
hw_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size)
{
	pnso_error_t err = EINVAL;
	struct accel_ring *ring;
	struct lif *lif;
	struct queue *q;
	struct sequencer_desc *seq_desc;
	uint32_t ring_id, index;
	uint16_t qtype;

	OSAL_LOG_DEBUG("enter ...");

	ring_id = svc_info->si_seq_info.sqi_ring_id;
	qtype = svc_info->si_seq_info.sqi_qtype;
	svc_info->si_seq_info.sqi_index = 0;

	ring = sonic_get_accel_ring(ring_id);
	if (!ring) {
		OSAL_ASSERT(ring);
		goto out;
	}

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(lif);
		goto out;
	}

	err = sonic_get_seq_sq(lif, qtype, &q);
	if (err) {
		OSAL_ASSERT(err);
		goto out;
	}

	seq_desc = (struct sequencer_desc *) sonic_q_consume_entry(q, &index);
	if (!seq_desc) {
		err = EINVAL;
		OSAL_LOG_ERROR("failed to obtain sequencer desc! err: %d", err);
		OSAL_ASSERT(seq_desc);
		goto out;
	}
	svc_info->si_seq_info.sqi_index = index;

	memset(seq_desc, 0, sizeof(*seq_desc));
	seq_desc->sd_desc_addr =
		cpu_to_be64(osal_virt_to_phy((void *) src_desc));
	seq_desc->sd_pndx_addr = cpu_to_be64(ring->ring_pndx_pa);
	seq_desc->sd_pndx_shadow_addr = cpu_to_be64(ring->ring_shadow_pndx_pa);
	seq_desc->sd_ring_addr = cpu_to_be64(ring->ring_base_pa);
	seq_desc->sd_desc_size = (uint8_t) ilog2(ring->ring_desc_size);
	seq_desc->sd_pndx_size = (uint8_t) ilog2(ring->ring_pndx_size);
	seq_desc->sd_ring_size = (uint8_t) ilog2(ring->ring_size);

	OSAL_LOG_INFO("ring_id: %u index: %u src_desc: 0x%llx  desc_size: %lu",
			ring_id, index, (u64) src_desc, desc_size);
	PPRINT_SEQUENCER_DESC(seq_desc);

	OSAL_LOG_DEBUG("exit!");
	return seq_desc;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return NULL;
}

static struct queue *
get_seq_q(const struct service_info *svc_info, bool status_q)
{
	struct queue *q = NULL;
	struct per_core_resource *pc_res;

	/* TODO-seq: remove using hard-coded 0th status queue */
	pc_res = svc_info->si_pc_res;

	switch (svc_info->si_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		q = status_q ? &pc_res->crypto_seq_status_qs[0] :
			&pc_res->crypto_enc_seq_q;
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		q = status_q ? &pc_res->crypto_seq_status_qs[0] :
			&pc_res->crypto_dec_seq_q;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		q = status_q ? &pc_res->cpdc_seq_status_qs[0] :
			&pc_res->cp_seq_q;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		q = status_q ? &pc_res->cpdc_seq_status_qs[0] :
			&pc_res->dc_seq_q;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	default:
		OSAL_ASSERT(0);
		break;
	}

	return q;
}

static void
hw_ring_db(const struct service_info *svc_info, uint16_t index)
{
	struct queue *seq_q;

	OSAL_LOG_DEBUG("enter ... ");

	seq_q = get_seq_q(svc_info, false);
	if (!seq_q) {
		OSAL_LOG_ERROR("failed to get sequencer q!");
		OSAL_ASSERT(seq_q);
		goto out;
	}

	sonic_q_ringdb(seq_q, index);

out:
	OSAL_LOG_DEBUG("exit!");
}

const struct sequencer_ops hw_seq_ops = {
	.setup_desc = hw_setup_desc,
	.ring_db = hw_ring_db,
};
