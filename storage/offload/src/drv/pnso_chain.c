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
#include "pnso_svc.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
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
	OSAL_LOG_DEBUG("%30s: %d", "si_num_bof_tags", svc_info->si_num_bof_tags);
	OSAL_LOG_DEBUG("%30s: %d", "si_num_bytes", svc_info->si_num_bytes);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_desc",
			(uint64_t) svc_info->si_desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_status_desc",
			(uint64_t) svc_info->si_status_desc.desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_istatus_desc",
			(uint64_t) svc_info->si_istatus_desc.desc);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_bof_desc",
			(uint64_t) svc_info->si_bof_desc);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_src_sgl",
			(uint64_t) svc_info->si_src_sgl.sgl);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_dst_sgl",
			(uint64_t) svc_info->si_dst_sgl.sgl);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_bof_sgl",
			(uint64_t) svc_info->si_bof_sgl.sgl);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_src_aol",
			(uint64_t) svc_info->si_src_aol.aol);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_dst_aol",
			(uint64_t) svc_info->si_dst_aol.aol);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_pb_sgl",
			(uint64_t) svc_info->si_pb_sgl);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_pb_bof_sgl",
			(uint64_t) svc_info->si_pb_bof_sgl);

	OSAL_LOG_DEBUG("%30s: %d", "=== sbi_num_entries",
			svc_info->si_batch_info.sbi_num_entries);
	OSAL_LOG_DEBUG("%30s: %d", "sbi_bulk_desc_idx",
			svc_info->si_batch_info.sbi_bulk_desc_idx);
	OSAL_LOG_DEBUG("%30s: %d", "sbi_desc_idx",
			svc_info->si_batch_info.sbi_desc_idx);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_pcr",
			(uint64_t) svc_info->si_pcr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_centry",
			(uint64_t) svc_info->si_centry);

	OSAL_LOG_DEBUG("%30s: %u", "si_iblist count",
			svc_info->si_iblist.blist.count);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_src_blist.type",
			(uint64_t) svc_info->si_src_blist.type);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_src_blist.len",
			(uint64_t) svc_info->si_src_blist.len);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_src_blist",
			(uint64_t) svc_info->si_src_blist.blist);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_dst_blist.type",
			(uint64_t) svc_info->si_dst_blist.type);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_dst_blist.len",
			(uint64_t) svc_info->si_dst_blist.len);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_dst_blist",
			(uint64_t) svc_info->si_dst_blist.blist);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_bof_blist.type",
			(uint64_t) svc_info->si_bof_blist.type);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_bof_blist.len",
			(uint64_t) svc_info->si_bof_blist.len);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "si_bof_blist",
			(uint64_t) svc_info->si_bof_blist.blist);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== si_sgl_pdma",
			(uint64_t) svc_info->si_sgl_pdma);
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
	OSAL_LOG_DEBUG("%30s: %d", "chain->sc_gen_id", chain->sc_gen_id);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_req",
			(uint64_t) chain->sc_req);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_res",
			(uint64_t) chain->sc_res);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_pcr",
			(uint64_t) chain->sc_pcr);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_batch_info",
			(uint64_t) chain->sc_batch_info);

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
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_last_entry",
			(uint64_t) chain->sc_last_entry);

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_req_cb",
			(uint64_t) chain->sc_req_cb);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "chain->sc_req_cb_ctx",
			(uint64_t) atomic64_read((atomic64_t *)
					&chain->sc_req_cb_ctx));
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
	return chain->sc_entry;
}

struct chain_entry *
chn_get_last_centry(struct service_chain *chain)
{
	OSAL_ASSERT(chain);
	return chain->sc_last_entry;
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

	if ((chain->sc_flags & CHAIN_CFLAG_RANG_DB) == 0) {
		/* This race can happen during async timeout */
		err = EBUSY;
		goto out;
	}

	if (chain->sc_num_services != chain->sc_res->num_services) {
		OSAL_LOG_ERROR("service count mismatch! chain_num_svcs: %d res_num_svcs: %d err %d",
			       chain->sc_num_services,
			       chain->sc_res->num_services, err);
		goto out;
	}

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		svc_info = &sc_entry->ce_svc_info;
		svc_ops = &svc_info->si_ops;
		err = svc_ops->poll(svc_info);
		if (err) {
			OSAL_LOG_DEBUG("poll failed! svc_type: %d desc 0x" PRIx64 " status_desc: 0x" PRIx64 " err: %d",
				svc_info->si_type,
				(uint64_t) svc_info->si_desc,
				(uint64_t) svc_info->si_status_desc.desc,
				err);
			break;
		}
		sc_entry = sc_entry->ce_next;
	}

	if (!err)
		chain->sc_flags |= CHAIN_CFLAG_POLLED;

out:
	return err;
}

static void *chain_to_poll_ctx(struct service_chain *chain)
{
	return req_obj_to_poll_ctx(chain, MPOOL_TYPE_SERVICE_CHAIN,
				   chain->sc_gen_id, chain->sc_pcr);
}

static inline struct service_chain *
poll_ctx_to_chain_params(void *poll_ctx, uint16_t *gen_id,
			 struct per_core_resource **pcr)
{
	struct service_chain *chain;
	uint8_t mpool_type;

	chain = (struct service_chain *) poll_ctx_to_req_obj(poll_ctx, &mpool_type,
							     gen_id, pcr);
	if (!chain || mpool_type != MPOOL_TYPE_SERVICE_CHAIN) {
		OSAL_LOG_DEBUG("invalid chain poll ctx 0x" PRIx64,
			       (uint64_t) poll_ctx);
		return NULL;
	}
	return chain;
}

pnso_error_t
chn_poller(struct service_chain *chain, uint16_t gen_id, bool is_timeout)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_result *res = NULL;
	completion_cb_t	cb = NULL;
	void *cb_ctx = NULL;

	PAS_DECL_SW_PERF();
	PAS_DECL_HW_PERF();

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(chain);

	OSAL_LOG_DEBUG("core_id: %u", osal_get_coreid());

	PPRINT_CHAIN(chain);

	if (chain->sc_gen_id != gen_id) {
		OSAL_LOG_ERROR("new chain gen_id %d found, expected %d",
			       chain->sc_gen_id, gen_id);
		goto out;
	}

	res = chain->sc_res;
	PAS_SET_SW_PERF(chain->sc_sw_latency_start);
	PAS_SET_HW_PERF(chain->sc_hw_latency_start);

	if (is_timeout) {
		err = ETIMEDOUT;
	} else {
		err = chn_poll_all_services(chain);
	}
	if (err) {
		OSAL_LOG_DEBUG("poll failed! chain: 0x" PRIx64 " err: %d",
				(uint64_t) chain, err);
		if (err == EBUSY)
			goto out;

		PAS_END_HW_PERF(chain->sc_pcr);
		PAS_END_SW_PERF(chain->sc_pcr);

		res->err = err; /* ETIMEDOUT most likely */
	} else {
		PAS_END_HW_PERF(chain->sc_pcr);
		PAS_END_SW_PERF(chain->sc_pcr);

		chn_read_write_result(chain);
		chn_update_overall_result(chain);
	}

	/* save caller's cb and context ahead of destroy */
	cb = chain->sc_req_cb;
	cb_ctx = (void *) atomic64_xchg(&chain->sc_req_cb_ctx, 0);

	chn_destroy_chain(chain);

	if (cb && cb_ctx) {
		OSAL_LOG_DEBUG("invoking caller's cb ctx: 0x" PRIx64 " res: 0x" PRIx64 " err: %d",
				(uint64_t) cb_ctx, (uint64_t) res, err);

		cb(cb_ctx, res);
	}

	return err;

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
pnso_chain_poller(void *poll_ctx)
{
	pnso_error_t err;
	struct per_core_resource *pcr;
	struct service_chain *chain;
	uint16_t gen_id;

	chain = poll_ctx_to_chain_params(poll_ctx, &gen_id, &pcr);
	if (!chain)
		return EINVAL;

	if (!sonic_try_reserve_per_core_res(pcr))
		return PNSO_LIF_IO_ERROR;

	err = chn_poller(chain, gen_id, false);

	sonic_unreserve_per_core_res(pcr);

	return err;
}

/* Note: assumes this is called only while pcr is exclusively reserved */
void
chn_poll_timeout_all(struct per_core_resource *pcr)
{
	struct mem_pool *mpool;
	void *obj = NULL;
	struct service_chain *chain;

	OSAL_LOG_DEBUG("enter ...");

	mpool = pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN];
	if (!mpool) {
		goto out;
	}

	obj = mpool_get_first_inuse_object(mpool);
	while (obj) {
		chain = (struct service_chain *) obj;
		chn_poller(chain, chain->sc_gen_id, true);

		obj = mpool_get_next_inuse_object(mpool, obj);
	}

out:
	OSAL_LOG_DEBUG("exit!");
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

		err = svc_ops->write_result(&sc_entry->ce_svc_info);
		if (err)
			OSAL_LOG_DEBUG("write result failed svc_type: %d err: %d",
				       sc_entry->ce_svc_info.si_type, err);

		sc_entry = sc_entry->ce_next;
	}
}

static void
init_service_batch_params(struct batch_info *batch_info,
		struct batch_page *page,
		uint16_t batch_request_idx, struct service_info *svc_info)
{
	struct service_batch_info *svc_batch_info = &svc_info->si_batch_info;
	uint16_t bulk_desc_idx, desc_idx;

	svc_batch_info->sbi_page = page;
	if (!((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)))
		return;

	bulk_desc_idx = batch_request_idx / MAX_PAGE_ENTRIES;
	desc_idx = batch_request_idx % MAX_PAGE_ENTRIES;

	svc_batch_info->sbi_num_entries = batch_info->bi_num_entries;
	svc_batch_info->sbi_bulk_desc_idx = bulk_desc_idx;
	svc_batch_info->sbi_desc_idx = desc_idx;
	svc_batch_info->sbi_batch_info = batch_info;

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
setup_service_param_buffers(struct service_info *svc_info,
		struct service_info *svc_prev)
{
	if (svc_prev) {
		switch (svc_prev->si_type) {

		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
			svc_info->si_src_blist = svc_prev->si_dst_blist;
			break;

		case PNSO_SVC_TYPE_HASH:
		case PNSO_SVC_TYPE_CHKSUM:
			svc_info->si_src_blist = svc_prev->si_src_blist;
			break;

		case PNSO_SVC_TYPE_DECOMPACT:
		case PNSO_SVC_TYPE_NONE:
		default:
			OSAL_ASSERT(0);
			return EINVAL;
		}
	}

	svc_info->si_seq_info.sqi_src_data_len = svc_info->si_src_blist.len;
	svc_info->si_seq_info.sqi_dst_data_len = svc_info->si_dst_blist.len;

	/*
	 * Pre-set the "service dependency" data length...As HW operation
	 * progresses for the service, this data length may later get updated.
	 */
	chn_service_deps_data_len_set(svc_info, svc_info->si_src_blist.len);
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
		if (svc_params->u.sp_cp_desc->flags &
				PNSO_CP_DFLAG_BYPASS_ONFAIL)
			svc_params->sp_bof_blist = svc_req->sgl;
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

static inline void
get_seq_info_by_path(struct service_info *svc_info, accel_ring_id_t *ring_id,
		uint16_t *qtype)
{
	if ((svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE) ||
		(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE)) {
		if (svc_info->si_flags & CHAIN_SFLAG_TYPE_READ) {
			*ring_id = ACCEL_RING_DC;
			*qtype = SONIC_QTYPE_DC_SQ;
		} else {
			*ring_id = ACCEL_RING_CP;
			*qtype = SONIC_QTYPE_CP_SQ;
		}
	} else {
		if (svc_info->si_flags & CHAIN_SFLAG_TYPE_READ) {
			*ring_id = ACCEL_RING_DC_HOT;
			*qtype = SONIC_QTYPE_DC_SQ;
		} else {
			*ring_id = ACCEL_RING_CP_HOT;
			*qtype = SONIC_QTYPE_CP_SQ;
		}
	}
}

static pnso_error_t
init_service_info(enum pnso_service_type svc_type,
		struct pnso_service_status *svc_status,
		struct service_params *svc_params,
		struct service_info *svc_info)
{
	accel_ring_id_t ring_id = ACCEL_RING_ID_MAX;

	svc_info->si_type = svc_type;
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		svc_info->si_ops = encrypt_ops;
		ring_id = ACCEL_RING_XTS0;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_ENC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CRYPTO_STATUS;
		svc_info->algo_type = svc_params->u.sp_crypto_desc->algo_type;
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		svc_info->si_ops = decrypt_ops;
		ring_id = ACCEL_RING_XTS1;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_DEC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CRYPTO_STATUS;
		svc_info->algo_type = svc_params->u.sp_crypto_desc->algo_type;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_info->si_ops = cp_ops;
		get_seq_info_by_path(svc_info, &ring_id,
				&svc_info->si_seq_info.sqi_qtype);
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		svc_info->si_desc_flags = svc_params->u.sp_cp_desc->flags;
		svc_info->algo_type = svc_params->u.sp_cp_desc->algo_type;
		svc_info->hdr_fmt_idx = svc_params->u.sp_cp_desc->hdr_fmt_idx;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_info->si_ops = dc_ops;
		get_seq_info_by_path(svc_info, &ring_id,
				&svc_info->si_seq_info.sqi_qtype);
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_DC_SQ;
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		svc_info->si_desc_flags = svc_params->u.sp_dc_desc->flags;
		svc_info->algo_type = svc_params->u.sp_dc_desc->algo_type;
		svc_info->hdr_fmt_idx = svc_params->u.sp_dc_desc->hdr_fmt_idx;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_info->si_ops = hash_ops;
		get_seq_info_by_path(svc_info, &ring_id,
				&svc_info->si_seq_info.sqi_qtype);
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		svc_info->si_desc_flags = svc_params->u.sp_hash_desc->flags;
		svc_info->algo_type = svc_params->u.sp_hash_desc->algo_type;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_info->si_ops = chksum_ops;
		get_seq_info_by_path(svc_info, &ring_id,
				&svc_info->si_seq_info.sqi_qtype);
		svc_info->si_seq_info.sqi_status_qtype =
			SONIC_QTYPE_CPDC_STATUS;
		svc_info->si_desc_flags = svc_params->u.sp_chksum_desc->flags;
		svc_info->algo_type = svc_params->u.sp_chksum_desc->algo_type;
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

	if (svc_params->sp_bof_blist)
		svc_info->si_bof_blist = svc_info->si_src_blist;

	return PNSO_OK;
}

static struct ring_tracker *
get_ring_tracker(struct service_chain *chain, accel_ring_id_t ring_id)
{
	return ring_id >= ARRAY_SIZE(chain->sc_ring_tracker) ?
		NULL : &chain->sc_ring_tracker[ring_id];
}

static inline pnso_error_t
_hw_ring_take(accel_ring_id_t ring_id, struct service_chain *chain)
{
	pnso_error_t err;
	struct ring_tracker *ring_tracker;

	ring_tracker = get_ring_tracker(chain, ring_id);
	if (!ring_tracker)
		return EINVAL;

	if (ring_tracker->rt_max_dflt_takes) {
		err = sonic_accel_ring_take(sonic_get_accel_ring(ring_id),
				ring_tracker->rt_max_dflt_takes);
		if (err == PNSO_OK){
			ring_tracker->rt_max_total_takes +=
				ring_tracker->rt_max_dflt_takes;

			ring_tracker->rt_max_dflt_takes =  0;
		}
			
		OSAL_LOG_DEBUG("ring: %s total: %u default: %u err: %d",
				sonic_accel_ring_name_get(ring_id),
				ring_tracker->rt_max_total_takes,
				ring_tracker->rt_max_dflt_takes, err);

		return err;
	}

	return	PNSO_OK;
}

static inline pnso_error_t
hw_ring_take(struct service_chain *chain)
{
	pnso_error_t err;
	accel_ring_id_t ring_id;

	ring_id = ACCEL_RING_CP;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	ring_id = ACCEL_RING_CP_HOT;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	ring_id = ACCEL_RING_DC;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	ring_id = ACCEL_RING_DC_HOT;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	ring_id = ACCEL_RING_XTS0;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	ring_id = ACCEL_RING_XTS1;
	err = _hw_ring_take(ring_id, chain);
	if (err)
		goto out;

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("HW ring %s potential overflow! err: %d",
			sonic_accel_ring_name_get(ring_id), err);
	return err;
}

static void
hw_ring_evaluate_take(struct service_info *svc_info)
{
	struct service_chain *chain = svc_info->si_centry->ce_chain_head;
	struct ring_tracker *ring_tracker;
	accel_ring_id_t ring_id;

	ring_id = sonic_get_accel_ring_id(svc_info->si_seq_info.sqi_ring);
	ring_tracker = get_ring_tracker(chain, ring_id);
	if (!ring_tracker) {
		OSAL_LOG_ERROR("invalid ring tracker for ring %s",
				svc_info->si_seq_info.sqi_ring->name);
		return;
	}

	ring_tracker->rt_max_dflt_takes =
		max((uint8_t) ring_tracker->rt_max_dflt_takes,
			(uint8_t) svc_info->si_seq_info.sqi_hw_dflt_takes);

	svc_info->si_seq_info.sqi_hw_dflt_takes = 0;
}

static pnso_error_t
hw_ring_give(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct service_chain *chain = svc_info->si_centry->ce_chain_head;
	struct ring_tracker *ring_tracker;
	accel_ring_id_t ring_id;

	ring_id = sonic_get_accel_ring_id(svc_info->si_seq_info.sqi_ring);
	ring_tracker = get_ring_tracker(chain, ring_id);
	if (!ring_tracker) {
		OSAL_LOG_ERROR("invalid ring tracker for ring %s! err: %d",
				svc_info->si_seq_info.sqi_ring->name, err);
		return err;
	}

	err = sonic_accel_ring_give(svc_info->si_seq_info.sqi_ring,
			ring_tracker->rt_max_total_takes);
	
	if (err != PNSO_OK)
		OSAL_LOG_ERROR("HW ring %s potential underflow! total: %u err: %d",
			       svc_info->si_seq_info.sqi_ring->name,
			       ring_tracker->rt_max_total_takes, err);

	ring_tracker->rt_max_total_takes = 0;

	return err;
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

	if (!(chain->sc_flags & CHAIN_CFLAG_RANG_DB) &&
	    chain->sc_async_evid) {
		/* cleanup for async mode submission failure */
		sonic_intr_put_ev_id(pcr, chain->sc_async_evid);
		chain->sc_async_evid = 0;
	}

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		i++;

		svc_info = &sc_entry->ce_svc_info;
		svc_info->si_ops.teardown(svc_info);

		hw_ring_give(svc_info);
		sc_next = sc_entry->ce_next;

		mpool_put_object(svc_chain_entry_mpool, sc_entry);
		sc_entry = sc_next;
	}

	chain->sc_gen_id++;
	mpool_put_object(svc_chain_mpool, chain);
	OSAL_LOG_DEBUG("exit!");
}

static inline void
set_chain_type(const struct pnso_service_request *svc_req, uint16_t *flags)
{
	uint32_t i;

	for (i = 0; i < svc_req->num_services; i++) {
		if ((svc_req->svc[i].svc_type == PNSO_SVC_TYPE_DECOMPRESS) ||
			(svc_req->svc[i].svc_type == PNSO_SVC_TYPE_DECRYPT)) {
			*flags |= CHAIN_CFLAG_TYPE_READ;
			return;
		}
	}
}

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
set_service_mode(uint16_t mode_flags, uint16_t *flags)
{
	if (mode_flags & REQUEST_RFLAG_MODE_SYNC)
		*flags |= CHAIN_SFLAG_MODE_SYNC;
	else if (mode_flags & REQUEST_RFLAG_MODE_POLL)
		*flags |= CHAIN_SFLAG_MODE_POLL;
	else if (mode_flags & REQUEST_RFLAG_MODE_ASYNC)
		*flags |= CHAIN_SFLAG_MODE_ASYNC;
}

pnso_error_t
chn_create_chain(struct request_params *req_params,
		struct service_chain **ret_chain)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = req_params->rp_pcr;
	struct mem_pool *svc_chain_mpool;
	struct mem_pool *svc_chain_entry_mpool;
	struct pnso_service_request *req;
	struct pnso_service_result *res;
	struct service_chain *chain = NULL;
	struct chain_entry *centry = NULL;
	struct chain_entry *centry_prev = NULL;
	struct service_info *svc_info = NULL;
	struct service_params svc_params = { 0 };
	struct service_info *svc_prev = NULL;
	void *poll_ctx = NULL;
	uint32_t i;
	uint16_t gen_id;

	OSAL_LOG_DEBUG("enter ...");

	*ret_chain = NULL;

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
	gen_id = chain->sc_gen_id;
	memset(chain, 0, sizeof(struct service_chain));
	chain->sc_gen_id = gen_id + 1;

	req = req_params->rp_svc_req;
	res = req_params->rp_svc_res;

	chain->sc_num_services = req->num_services;
	chain->sc_entry = NULL;

	chain->sc_req = req;
	chain->sc_res = res;

	chain->sc_pcr = pcr;

	if (req_params->rp_flags & REQUEST_RFLAG_TYPE_BATCH)
		chain->sc_batch_info = req_params->rp_batch_info;

	set_chain_mode(req_params->rp_flags, &chain->sc_flags);

	set_chain_type(chain->sc_req, &chain->sc_flags);

	if ((req_params->rp_flags & REQUEST_RFLAG_TYPE_CHAIN) &&
			((chain->sc_flags & CHAIN_CFLAG_MODE_ASYNC) ||
			(chain->sc_flags & CHAIN_CFLAG_MODE_POLL))) {
		chain->sc_sw_latency_start = req_params->rp_sw_latency_ts;

		chain->sc_req_cb = req_params->rp_cb;
		atomic64_set(&chain->sc_req_cb_ctx,
				(uint64_t) req_params->rp_cb_ctx);

		poll_ctx = chain_to_poll_ctx(chain);
		if (chain->sc_flags & CHAIN_CFLAG_MODE_POLL) {
			/* for caller to poll */
			*req_params->rp_poll_fn = pnso_chain_poller;
			*req_params->rp_poll_ctx = poll_ctx;
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
		centry->ce_prev = centry_prev;
		svc_info = &centry->ce_svc_info;

		if (i == 0) {
			svc_info->si_flags = (chain->sc_num_services == 1) ?
				CHAIN_SFLAG_LONE_SERVICE :
				CHAIN_SFLAG_FIRST_SERVICE;
			chain->sc_entry = centry;
		} else
			centry_prev->ce_next = centry;
		chain->sc_last_entry = centry;
		centry_prev = centry;

		set_service_mode(req_params->rp_flags, &svc_info->si_flags);

		if (!(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE))
			if (i+1 == chain->sc_num_services)
				svc_info->si_flags |= CHAIN_SFLAG_LAST_SERVICE;

		if (chain->sc_flags & CHAIN_CFLAG_TYPE_READ)
			svc_info->si_flags |= CHAIN_SFLAG_TYPE_READ;

		if (req_params->rp_flags & REQUEST_RFLAG_TYPE_BATCH) {
			svc_info->si_flags |= CHAIN_SFLAG_IN_BATCH;

			init_service_batch_params(req_params->rp_batch_info,
					req_params->rp_page,
					req_params->rp_batch_index, svc_info);
		}

		init_service_params(req, &res->svc[i],
				&req->svc[i], &svc_params);

		init_service_info(req->svc[i].svc_type, &res->svc[i],
				  &svc_params, svc_info);

		svc_info->si_pcr = chain->sc_pcr;
		svc_info->si_centry = centry;

		setup_service_param_buffers(svc_info, svc_prev);

		err = svc_info->si_ops.setup(svc_info, &svc_params);
		if (err)
			goto out_chain;

		hw_ring_evaluate_take(svc_info);

		PPRINT_SERVICE_INFO(svc_info);

		svc_prev = svc_info;
		PAS_INC_NUM_SERVICES(pcr);
	}
	chain->sc_req_id = osal_atomic_fetch_add(&g_req_id, 1);

	/*
	 * After completing service linkages and setups above, execute
	 * chain() for the services. This is done for every service in the
	 * chain, which helps facilitate multi-level chaining.
	 *
	 * Note also that chain() has to be performed before enable_interrupt()
	 * since the latter is designed to have the last say in programming
	 * some of the chain paramaters.
	 */
	centry = chain->sc_entry;
	while (centry) {
		svc_info = &centry->ce_svc_info;
		err = svc_info->si_ops.chain(centry);
		if (err)
			goto out_chain;
		centry = centry->ce_next;
	}

	/* setup interrupt params in last chain's last service */
	if (svc_info && !(req_params->rp_flags & REQUEST_RFLAG_TYPE_BATCH) &&
		(req_params->rp_flags & REQUEST_RFLAG_MODE_ASYNC)) {
		err = svc_info->si_ops.enable_interrupt(svc_info, poll_ctx);
		if (err)
			goto out_chain;
	}

	err = hw_ring_take(chain);
	if (err)
		goto out_chain;

	*ret_chain = chain;
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out_chain:
	chn_destroy_chain(chain);
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
chn_notify_caller(struct service_chain *chain)
{
	void *cb_ctx;

	if (chain->sc_req_cb) {
		cb_ctx = (void *) atomic64_xchg(&chain->sc_req_cb_ctx, 0);
		if (cb_ctx)
			chain->sc_req_cb(cb_ctx, chain->sc_res);
	}
}

void
chn_update_overall_result(struct service_chain *chain)
{
	struct pnso_service_request *req;
	struct pnso_service_result *res;
	uint32_t i;

	OSAL_ASSERT(chain);

	req = chain->sc_req;
	res = chain->sc_res;

	res->err = PNSO_OK;
	for (i = 0; i < res->num_services; i++) {
		if (res->svc[i].err) {
			if (res->num_services > 1 &&
				svc_is_bypass_onfail_enabled(&req->svc[i])) {
				OSAL_LOG_DEBUG("skip updating overall error for cp svc_type: %d num_services: %d err: %d",
						req->svc[i].svc_type,
						res->num_services,
						res->svc[i].err);
				continue;
			}

			res->err = res->svc[i].err;
			break;
		}
	}
}

pnso_error_t
chn_execute_chain(struct service_chain *chain)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr;
	struct chain_entry *ce_first, *ce_last;
	uint16_t async_evid;
	bool is_sync_mode = (chain->sc_flags & CHAIN_CFLAG_MODE_SYNC) != 0;

	PAS_DECL_HW_PERF();

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(chain);

	ce_first = chain->sc_entry;
	if (!ce_first)
		goto out;
	ce_last = chain->sc_last_entry;

	async_evid = chain->sc_async_evid;
	pcr = chain->sc_pcr;

	/* ring 'first' service door bell */
	chain->sc_flags |= CHAIN_CFLAG_RANG_DB;

	/*
	 * in poll or async mode, the request may finish before recording this
	 * perf-event, so record it ahead of ringing the db to cover the case
	 * of a race condition.
	 *
	 */
	PAS_START_HW_PERF(chain->sc_hw_latency_start);

	err = ce_first->ce_svc_info.si_ops.ring_db(&ce_first->ce_svc_info);
	if (err) {
		PAS_END_HW_PERF(pcr);

		chain->sc_flags &= ~((uint16_t) CHAIN_CFLAG_RANG_DB);
		OSAL_LOG_ERROR("failed to ring service door bell! svc_type: %d err: %d",
			       ce_first->ce_svc_info.si_type, err);
		goto out;
	}

	if (!is_sync_mode) {
		OSAL_LOG_DEBUG("in non-sync mode ... sc_flags: %d",
				chain->sc_flags);
		if (async_evid)
			sonic_intr_touch_ev_id(pcr, async_evid);

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

uint32_t
chn_service_deps_data_len_get(struct service_info *svc_info)
{
	struct cpdc_status_desc *status_desc;
	uint32_t len;

	if (chn_service_type_is_cpdc(svc_info)) {
		status_desc = svc_info->si_status_desc.desc;
		if (status_desc->csd_err) {
			/* to handle the case where BOF is enabled in a chain */
			OSAL_LOG_DEBUG("bail out to rely on original len for bof");
			goto out;
		}

		len = cpdc_desc_data_len_get_eval(svc_info->si_type,
				status_desc->csd_output_data_len);
		chn_service_deps_data_len_set(svc_info, len);
	}

out:
	return svc_info->si_svc_deps.sd_data_len;
}
