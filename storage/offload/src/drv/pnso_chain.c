/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_dev.h"
#include "sonic_lif.h"

#include "osal.h"
#include "accel_ring.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_stats.h"
#include "pnso_utils.h"

/*
 * TODO-chain:
 *	- pass/emit req_id across
 *	- update return values in header template
 *	- once all services are in, use union for si_desc/si_status_desc
 *	as needed
 *	- investigate SONIC_QTYPE_DC_SQ vs SONIC_QTYPE_CP_SQ for checksum
 *
 */
#ifdef NDEBUG
#define PPRINT_SERVICE_INFO(s)
#define PPRINT_CHAIN_ENTRY(ce)
#else
#define PPRINT_SERVICE_INFO(s)		pprint_service_info(s)
#define PPRINT_CHAIN_ENTRY(ce)		pprint_chain_entry(ce)
#endif

osal_atomic_int_t g_req_id;

static void __attribute__((unused))
pprint_service_info(const struct service_info *svc_info)
{
	if (!svc_info)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== svc_info",  (uint64_t) svc_info);
	OSAL_LOG_DEBUG("%30s: %d", "si_type", svc_info->si_type);
	OSAL_LOG_DEBUG("%30s: %d", "si_flags", svc_info->si_flags);

	OSAL_LOG_DEBUG("%30s: %d", "si_block_size", svc_info->si_block_size);
	OSAL_LOG_DEBUG("%30s: %d", "si_desc_flags", svc_info->si_desc_flags);
	OSAL_LOG_DEBUG("%30s: %d", "si_num_tags", svc_info->si_num_tags);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_desc",  (uint64_t) svc_info->si_desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_status_desc",
			(uint64_t) svc_info->si_status_desc);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "si_istatus_desc",
			(uint64_t) svc_info->si_istatus_desc);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_src_sgl",
			(uint64_t) svc_info->si_src_sgl.sgl);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_dst_sgl",
			(uint64_t) svc_info->si_dst_sgl.sgl);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_src_aol",
			(uint64_t) svc_info->si_src_aol.aol);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_dst_aol",
			(uint64_t) svc_info->si_dst_aol.aol);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_p4_sgl",
			(uint64_t) svc_info->si_p4_sgl);
	OSAL_LOG_INFO("%30s: %u", "=== si_iblist count",
			svc_info->si_iblist.blist.count);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_src_blist",
			(uint64_t) svc_info->si_src_blist.blist);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_src_blist.len",
			(uint64_t) svc_info->si_src_blist.len);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_dst_blist",
			(uint64_t) svc_info->si_dst_blist.blist);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_dst_blist.len",
			(uint64_t) svc_info->si_dst_blist.len);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_sgl_pdma",
			(uint64_t) svc_info->si_sgl_pdma);

	OSAL_LOG_DEBUG("%30s: %d", "sbi_num_entries",
			svc_info->si_batch_info.sbi_num_entries);
	OSAL_LOG_DEBUG("%30s: %d", "sbi_bulk_desc_idx",
			svc_info->si_batch_info.sbi_bulk_desc_idx);
	OSAL_LOG_DEBUG("%30s: %d", "sbi_desc_idx",
			svc_info->si_batch_info.sbi_desc_idx);

	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_pcr",
			(uint64_t) svc_info->si_pcr);
	OSAL_LOG_INFO("%30s: 0x" PRIx64, "=== si_centry",
			(uint64_t) svc_info->si_centry);

	/* TODO-chain: include service status and other members */
}

static void __attribute__((unused))
pprint_chain_entry(const struct chain_entry *centry)
{
	if (!centry)
		return;

	/* chain entry */
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "centry", (uint64_t) centry);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "centry->ce_chain_head",
			      (uint64_t) centry->ce_chain_head);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "centry->ce_next",
			      (uint64_t) centry->ce_next);

	/* basic service info */
	OSAL_LOG_DEBUG("%30s: %d", "ce_svc_info->si_type",
		      centry->ce_svc_info.si_type);
	OSAL_LOG_DEBUG("%30s: %d", "ce_svc_info->si_flags",
		      centry->ce_svc_info.si_flags);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ce_svc_info->si_ops",
		      (uint64_t) &centry->ce_svc_info.si_ops);
}

void __attribute__((unused))
chn_pprint_chain(const struct service_chain *chain)
{
	uint32_t i;
	struct chain_entry *sc_entry;

	if (!chain)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain", (uint64_t) chain);
	OSAL_LOG_DEBUG("%30s: %d", "chain->sc_req_id", chain->sc_req_id);
	OSAL_LOG_DEBUG("%30s: %d", "chain->sc_num_services",
			chain->sc_num_services);
	OSAL_LOG_DEBUG("%30s: %d", "chain->sc_flags", chain->sc_flags);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_pcr",
			(uint64_t) chain->sc_pcr);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		OSAL_LOG_DEBUG("%30s: %d", "service: #", ++i);

		/* chain entry */
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_entry",
				(uint64_t) sc_entry);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_entry->ce_chain_head",
			      (uint64_t) sc_entry->ce_chain_head);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_entry->ce_next",
			      (uint64_t) sc_entry->ce_next);

		/* service info */
		OSAL_LOG_DEBUG("%30s: %d", "ce_svc_info->si_type",
			      sc_entry->ce_svc_info.si_type);
		OSAL_LOG_DEBUG("%30s: %d", "ce_svc_info->si_flags",
			      sc_entry->ce_svc_info.si_flags);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "ce_svc_info->si_ops",
			      (uint64_t) &sc_entry->ce_svc_info.si_ops);

		sc_entry = sc_entry->ce_next;
	}

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_req_cb",
			(uint64_t) chain->sc_req_cb);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_req_cb_ctx",
			(uint64_t) chain->sc_req_cb_ctx);
}

struct service_chain *
chn_get_first_service_chain(struct batch_info *batch_info, uint32_t idx)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;

	batch_page = GET_PAGE(batch_info, idx);
	page_entry = GET_PAGE_ENTRY(batch_page, idx);
	return page_entry->bpe_chain;
}

struct service_chain *
chn_get_last_service_chain(struct batch_info *batch_info)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;

	batch_page = GET_PAGE(batch_info, (batch_info->bi_num_entries-1));
	page_entry = GET_PAGE_ENTRY(batch_page, (batch_info->bi_num_entries-1));
	return page_entry->bpe_chain;
}

struct chain_entry *
chn_get_first_centry(struct service_chain *chain)
{
	OSAL_ASSERT(chain);
	OSAL_ASSERT(chain->sc_entry);
	return chain->sc_entry;
}

struct chain_entry *
chn_get_last_centry(struct service_chain *chain)
{
	struct chain_entry *sc_entry, *ce_last = NULL;

	OSAL_ASSERT(chain);
	OSAL_ASSERT(chain->sc_entry);

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		ce_last = sc_entry;
		sc_entry = sc_entry->ce_next;
	}

	return ce_last;
}

bool
chn_is_poll_done(struct service_chain *chain)
{
	return (chain->sc_flags & CHAIN_CFLAG_POLLED) ? true : false;
}

pnso_error_t
chn_poll_all_services(struct service_chain *chain)
{
	pnso_error_t err = EINVAL;
	struct chain_entry *sc_entry;
	struct service_ops *svc_ops;
	struct service_info *svc_info;

	OSAL_ASSERT(chain);
	OSAL_ASSERT(chain->sc_entry);

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		svc_info = &sc_entry->ce_svc_info;
		svc_ops = &svc_info->si_ops;
		err = svc_ops->poll(svc_info);
		if (err) {
			OSAL_LOG_DEBUG("poll failed! svc_type: %d desc 0x" PRIx64 " status_desc: 0x" PRIx64 " err: %d",
					svc_info->si_type,
					(uint64_t) svc_info->si_desc,
					(uint64_t) svc_info->si_status_desc,
					err);
			break;
		}
		sc_entry = sc_entry->ce_next;
	}

	if (!err)
		chain->sc_flags |= CHAIN_CFLAG_POLLED;

	return err;
}

pnso_error_t chn_poller(void *pnso_poll_ctx);

pnso_error_t
chn_poller(void *poll_ctx)
{
	pnso_error_t err = EINVAL;
	struct service_chain *chain = (struct service_chain *) poll_ctx;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_LOG_DEBUG("core_id: %u", osal_get_coreid());

	if (!poll_ctx) {
		OSAL_LOG_ERROR("invalid poll context! err: %d", err);
		goto out;
	}
	PPRINT_CHAIN(chain);

	err = chn_poll_all_services(chain);
	if (err) {
		/* TODO-poll: retry then give-up after a limit to clean-up */
		OSAL_LOG_DEBUG("poll failed! chain: 0x" PRIx64 " err: %d",
				(uint64_t) chain, err);
		goto out;
	}

	chn_read_write_result(chain);

	chn_update_overall_result(chain);

	if (chain->sc_req_cb) {
		OSAL_LOG_DEBUG("invoking caller's cb ctx: 0x" PRIx64 " res: 0x" PRIx64 " err: %d",
				(uint64_t) chain->sc_req_cb_ctx,
				(uint64_t) chain->sc_res, err);

		chain->sc_req_cb(chain->sc_req_cb_ctx, chain->sc_res);
	}

	chn_destroy_chain(chain);

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

void
chn_read_write_result(struct service_chain *chain)
{
	pnso_error_t err;
	struct chain_entry *sc_entry;
	struct service_ops *svc_ops;

	OSAL_ASSERT(chain);
	OSAL_ASSERT(chain->sc_entry);

	/* update status of individual service(s) */
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		svc_ops = &sc_entry->ce_svc_info.si_ops;
		err = svc_ops->read_status(&sc_entry->ce_svc_info);
		if (err)
			OSAL_LOG_ERROR("read status failed svc_type: %d err: %d",
				       sc_entry->ce_svc_info.si_type, err);

		err = svc_ops->write_result(&sc_entry->ce_svc_info);
		if (err)
			OSAL_LOG_ERROR("write result failed svc_type: %d err: %d",
				       sc_entry->ce_svc_info.si_type, err);

		sc_entry = sc_entry->ce_next;
	}
}

static void
init_service_batch_params(struct batch_info *batch_info,
		uint16_t batch_request_idx, struct service_info *svc_info)
{
	struct service_batch_info *svc_batch_info = &svc_info->si_batch_info;
	uint16_t bulk_desc_idx, desc_idx;

	if (!((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)))
		return;

	bulk_desc_idx = batch_request_idx / MAX_PAGE_ENTRIES;
	desc_idx = batch_request_idx % MAX_PAGE_ENTRIES;

	svc_batch_info->sbi_num_entries = batch_info->bi_num_entries;
	svc_batch_info->sbi_bulk_desc_idx = bulk_desc_idx;
	svc_batch_info->sbi_desc_idx = desc_idx;

	switch (batch_info->bi_svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc_batch_info->u.sbi_crypto_desc = (struct crypto_desc *)
			batch_info->bi_bulk_desc[bulk_desc_idx];
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		svc_batch_info->u.sbi_cpdc_desc = (struct cpdc_desc *)
			batch_info->bi_bulk_desc[bulk_desc_idx];
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
	}

	OSAL_LOG_DEBUG("num_entries: %d batch_request_idx: %d bulk_desc_idx: %d, desc_idx: %d bulk_desc: 0x" PRIx64,
			batch_info->bi_num_entries,
			batch_request_idx, bulk_desc_idx, desc_idx,
			(uint64_t) batch_info->bi_bulk_desc[bulk_desc_idx]);
}

static pnso_error_t
setup_service_param_buffers(struct pnso_service *pnso_svc,
		struct service_params *svc_params,
		struct service_info *svc_info,
		struct service_buf_list *interm_blist)
{
	switch (pnso_svc->svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		if (chn_service_is_in_chain(svc_info) &&
				!chn_service_is_first(svc_info))
			svc_info->si_src_blist = *interm_blist;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
		return EINVAL;
	}

	return PNSO_OK;
}

static pnso_error_t
init_service_params(const struct pnso_service_request *svc_req,
		const struct pnso_service_status *svc_status,
		struct pnso_service *pnso_svc,
		struct service_params *svc_params)
{
	OSAL_ASSERT(pnso_svc->svc_type == svc_status->svc_type);

	svc_params->sp_src_blist = svc_req->sgl;
	svc_params->sp_dst_blist = svc_status->u.dst.sgl;

	switch (pnso_svc->svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc_params->u.sp_crypto_desc = &pnso_svc->u.crypto_desc;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_params->u.sp_cp_desc = &pnso_svc->u.cp_desc;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_params->u.sp_dc_desc = &pnso_svc->u.dc_desc;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_params->u.sp_hash_desc = &pnso_svc->u.hash_desc;
		svc_params->sp_dst_blist = NULL;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_params->u.sp_chksum_desc = &pnso_svc->u.chksum_desc;
		svc_params->sp_dst_blist = NULL;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
		return EINVAL;
	}

	return PNSO_OK;
}

static pnso_error_t
init_service_info(enum pnso_service_type svc_type,
		struct pnso_service_status *svc_status,
		struct service_params *svc_params,
		struct service_info *svc_info)
{
	accel_ring_id_t ring_id = ACCEL_RING_ID_MAX;

	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		svc_info->si_ops = encrypt_ops;
		ring_id = ACCEL_RING_XTS0;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_ENC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CRYPTO_STATUS;
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		svc_info->si_ops = decrypt_ops;
		ring_id = ACCEL_RING_XTS1;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_DEC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CRYPTO_STATUS;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_info->si_ops = cp_ops;
		ring_id = ACCEL_RING_CP;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_info->si_ops = dc_ops;
		ring_id = ACCEL_RING_DC;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_DC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_info->si_ops = hash_ops;
		ring_id = ACCEL_RING_CP_HOT;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_info->si_ops = chksum_ops;
		ring_id = ACCEL_RING_CP;
		/* TODO-chain: rolling back to previous PR setting DCq failed */
		// svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_DC_SQ;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
		return EINVAL;
	}

	/* Set reasonable default here and allow service op to modify if necessary */
	chn_service_hw_ring_take_set(svc_info, 1);
	svc_info->si_seq_info.sqi_ring = sonic_get_accel_ring(ring_id);
	if (!svc_info->si_seq_info.sqi_ring)
		return EINVAL;

	/* svc_info->si_flags updated outside TODO-chain: leave this here?? */
	svc_info->si_block_size = 4096;	/* TODO-chain: get via init params??  */
	svc_info->si_svc_status = svc_status;

	svc_info->si_src_blist.type = SERVICE_BUF_LIST_TYPE_DFLT;
	svc_info->si_src_blist.blist = svc_params->sp_src_blist;
	svc_info->si_src_blist.len =
		pbuf_get_buffer_list_len(svc_params->sp_src_blist);

	svc_info->si_dst_blist.type = SERVICE_BUF_LIST_TYPE_DFLT;
	svc_info->si_dst_blist.blist = svc_params->sp_dst_blist;
	svc_info->si_dst_blist.len =
		pbuf_get_buffer_list_len(svc_params->sp_dst_blist);

	return PNSO_OK;
}

void
chn_destroy_chain(struct service_chain *chain)
{
	struct per_core_resource *pcr;
	struct mem_pool *svc_chain_mpool;
	struct mem_pool *svc_chain_entry_mpool;
	struct chain_entry *sc_entry;
	struct chain_entry *sc_next;
	struct service_info *svc_info;
	uint32_t i;

	if (!chain)
		return;

	OSAL_LOG_DEBUG("enter ...");

	PPRINT_CHAIN(chain);

	pcr = chain->sc_pcr;
	OSAL_ASSERT(pcr);

	svc_chain_mpool = pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN];
	OSAL_ASSERT(svc_chain_mpool);

	svc_chain_entry_mpool = pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY];
	OSAL_ASSERT(svc_chain_entry_mpool);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		i++;

		svc_info = &sc_entry->ce_svc_info;
		svc_info->si_ops.teardown(svc_info);

		chn_service_hw_ring_give(svc_info);
		sc_next = sc_entry->ce_next;

		mpool_put_object(svc_chain_entry_mpool, sc_entry);
		sc_entry = sc_next;
	}
	OSAL_ASSERT(i == chain->sc_num_services);

	mpool_put_object(svc_chain_mpool, chain);
	OSAL_LOG_DEBUG("exit!");
}

/* TODO-poll/async: Revisit to collapse flags after async in-place */
static inline void
set_chain_mode(uint16_t mode_flags, uint16_t *flags)
{
	if (mode_flags & REQUEST_RFLAG_MODE_SYNC)
		*flags |= CHAIN_CFLAG_MODE_SYNC;
	else if (mode_flags & REQUEST_RFLAG_MODE_POLL)
		*flags |= CHAIN_CFLAG_MODE_POLL;
	else if (mode_flags & REQUEST_RFLAG_MODE_ASYNC)
		*flags |= CHAIN_CFLAG_MODE_ASYNC;
}

static inline void
set_service_mode(uint16_t mode_flags, uint8_t *flags)
{
	if (mode_flags & REQUEST_RFLAG_MODE_SYNC)
		*flags |= CHAIN_SFLAG_MODE_SYNC;
	else if (mode_flags & REQUEST_RFLAG_MODE_POLL)
		*flags |= CHAIN_SFLAG_MODE_POLL;
	else if (mode_flags & REQUEST_RFLAG_MODE_ASYNC)
		*flags |= CHAIN_SFLAG_MODE_ASYNC;
}

struct service_chain *
chn_create_chain(struct request_params *req_params)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct mem_pool *svc_chain_mpool;
	struct mem_pool *svc_chain_entry_mpool;
	struct pnso_service_request *req;
	struct pnso_service_result *res;
	struct service_chain *chain = NULL;
	struct chain_entry *centry = NULL;
	struct chain_entry *last_ce;
	struct chain_entry *centry_prev = NULL;
	struct service_info *svc_info;
	struct service_params svc_params;
	struct service_buf_list interm_blist;
	uint32_t i;

	OSAL_LOG_DEBUG("enter ...");

	svc_chain_mpool = pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN];
	if (!svc_chain_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	svc_chain_entry_mpool = pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY];
	if (!svc_chain_entry_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	chain = (struct service_chain *) mpool_get_object(svc_chain_mpool);
	if (!chain) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain service chain object from pool! err: %d",
				err);
		goto out;
	}
	memset(chain, 0, sizeof(struct service_chain));

	req = req_params->rp_svc_req;
	res = req_params->rp_svc_res;

	chain->sc_num_services = req->num_services;
	chain->sc_entry = NULL;
	chain->sc_res = res;

	chain->sc_pcr = pcr;

	set_chain_mode(req_params->rp_flags, &chain->sc_flags);

	if ((req_params->rp_flags & REQUEST_RFLAG_TYPE_CHAIN) &&
			((chain->sc_flags & CHAIN_CFLAG_MODE_ASYNC) ||
			(chain->sc_flags & CHAIN_CFLAG_MODE_POLL))) {
		chain->sc_req_cb = req_params->rp_cb;
		chain->sc_req_cb_ctx = req_params->rp_cb_ctx;

		if (chain->sc_flags & CHAIN_CFLAG_MODE_POLL) {
			/* for caller to poll */
			*req_params->rp_poll_fn = chn_poller;
			*req_params->rp_poll_ctx = (void *) chain;
		}
	}

	/* init services in the chain  */
	for (i = 0; i < chain->sc_num_services; i++) {
		centry = (struct chain_entry *)
			mpool_get_object(svc_chain_entry_mpool);
		if (!centry) {
			err = ENOMEM;
			OSAL_LOG_ERROR("cannot obtain service chain entry object from pool! err: %d",
				err);
			goto out_chain;
		}
		memset(centry, 0, sizeof(struct chain_entry));

		centry->ce_chain_head = chain;
		centry->ce_next = NULL;
		svc_info = &centry->ce_svc_info;

		if (i == 0) {
			svc_info->si_flags = (chain->sc_num_services == 1) ?
				CHAIN_SFLAG_LONE_SERVICE :
				CHAIN_SFLAG_FIRST_SERVICE;
			chain->sc_entry = centry;
		} else
			centry_prev->ce_next = centry;
		centry_prev = centry;

		set_service_mode(req_params->rp_flags, &svc_info->si_flags);

		if (!(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE))
			if (i+1 == chain->sc_num_services)
				svc_info->si_flags |= CHAIN_SFLAG_LAST_SERVICE;

		if (req_params->rp_flags & REQUEST_RFLAG_TYPE_BATCH) {
			svc_info->si_flags |= CHAIN_SFLAG_IN_BATCH;

			init_service_batch_params(req_params->rp_batch_info,
					req_params->rp_batch_index, svc_info);
		}

		init_service_params(req, &res->svc[i],
				&req->svc[i], &svc_params);

		init_service_info(req->svc[i].svc_type, &res->svc[i],
				  &svc_params, svc_info);

		svc_info->si_pcr = chain->sc_pcr;
		svc_info->si_centry = centry;

		setup_service_param_buffers(&req->svc[i], &svc_params,
					svc_info, &interm_blist);

		err = svc_info->si_ops.setup(svc_info, &svc_params);
		if (err)
			goto out_chain;

		err = chn_service_hw_ring_take(svc_info);
		if (err)
			goto out_chain;

		PPRINT_SERVICE_INFO(svc_info);

		interm_blist = svc_info->si_dst_blist;
		PAS_INC_NUM_SERVICES(pcr);
	}
	chain->sc_req_id = osal_atomic_fetch_add(&g_req_id, 1);

	/* chain the services  */
	centry = chain->sc_entry;
	svc_info = &centry->ce_svc_info;

	err = svc_info->si_ops.chain(centry);
	if (err)
		goto out_chain;

	/* setup interrupt params in last chain's last service */
	if (!(req_params->rp_flags & REQUEST_RFLAG_TYPE_BATCH) &&
		(req_params->rp_flags & REQUEST_RFLAG_MODE_ASYNC) && chain) {
		last_ce = chn_get_last_centry(chain);
		svc_info = &last_ce->ce_svc_info;
		svc_info->si_ops.enable_interrupt(svc_info, chain);
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return chain;

out_chain:
	chn_destroy_chain(chain);
	chain = NULL;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return chain;
}

void
chn_notify_caller(struct service_chain *chain)
{
	if (chain->sc_req_cb)
		chain->sc_req_cb(chain->sc_req_cb_ctx, chain->sc_res);
}

void
chn_update_overall_result(struct service_chain *chain)
{
	struct pnso_service_result *res;
	uint32_t i;

	OSAL_ASSERT(chain);

	res = chain->sc_res;
	OSAL_ASSERT(res);

	res->err = PNSO_OK;
	for (i = 0; i < res->num_services; i++) {
		if (res->svc[i].err) {
			res->err = res->svc[i].err;
			break;
		}
	}
}

pnso_error_t
chn_execute_chain(struct service_chain *chain)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct chain_entry *sc_entry;
	struct chain_entry *ce_first, *ce_last;

	PAS_DECL_HW_PERF();

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(chain);

	ce_first = ce_last = chain->sc_entry;
	if (!ce_first)
		goto out;

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		ce_last = sc_entry;
		sc_entry = sc_entry->ce_next;
	}

	/* ring 'first' service door bell */
	err = ce_first->ce_svc_info.si_ops.ring_db(&ce_first->ce_svc_info);
	PAS_START_HW_PERF();
	if (err) {
		OSAL_LOG_ERROR("failed to ring service door bell! svc_type: %d err: %d",
			       ce_first->ce_svc_info.si_type, err);
		PAS_END_HW_PERF(pcr);
		goto out;
	}

	if ((chain->sc_flags & CHAIN_CFLAG_MODE_POLL) ||
		(chain->sc_flags & CHAIN_CFLAG_MODE_ASYNC)) {
		OSAL_LOG_DEBUG("in non-sync mode ... sc_flags: %d",
				chain->sc_flags);
		PAS_END_HW_PERF(pcr);
		goto out;
	}
	PAS_START_HW_PERF();

	if ((chain->sc_flags & CHAIN_CFLAG_MODE_POLL) ||
		(chain->sc_flags & CHAIN_CFLAG_MODE_ASYNC)) {
		OSAL_LOG_DEBUG("in non-sync mode ... sc_flags: %d",
				chain->sc_flags);
		PAS_END_HW_PERF(pcr);
		goto out;
	}

	/* wait for 'last' service completion */
	err = ce_last->ce_svc_info.si_ops.poll(&ce_last->ce_svc_info);
	PAS_END_HW_PERF(pcr);
	if (err) {
		/* in sync-mode, poll() will return either OK or ETIMEDOUT */
		OSAL_LOG_ERROR("service failed to poll svc_type: %d err: %d",
			       ce_last->ce_svc_info.si_type, err);
		goto out;
	}

	/* update status of individual service(s) */
	chn_read_write_result(chain);

	/* update over all status of the chain */
	chn_update_overall_result(chain);

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
chn_service_hw_ring_take(struct service_info *svc_info)
{
	pnso_error_t err;

	err = sonic_accel_ring_take(svc_info->si_seq_info.sqi_ring,
				    svc_info->si_seq_info.sqi_hw_dflt_takes);
	if (err == PNSO_OK) {
		svc_info->si_seq_info.sqi_hw_total_takes +=
			svc_info->si_seq_info.sqi_hw_dflt_takes;
		svc_info->si_seq_info.sqi_hw_dflt_takes = 0;
	} else
		OSAL_LOG_ERROR("HW ring %s potential overflow",
			       svc_info->si_seq_info.sqi_ring->name);

	return err;
}

pnso_error_t
chn_service_hw_ring_give(struct service_info *svc_info)
{
	pnso_error_t err;

	err = sonic_accel_ring_give(svc_info->si_seq_info.sqi_ring,
				    svc_info->si_seq_info.sqi_hw_total_takes);
	svc_info->si_seq_info.sqi_hw_total_takes = 0;
	if (err != PNSO_OK)
		OSAL_LOG_ERROR("HW ring %s potential underflow",
			       svc_info->si_seq_info.sqi_ring->name);

	return err;
}

