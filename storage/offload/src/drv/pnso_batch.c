/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "sonic_dev.h"
#include "sonic_lif.h"

#include "osal.h"

#include "pnso_mpool.h"
#include "pnso_batch.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_crypto_cmn.h"
#include "pnso_stats.h"
#include "pnso_utils.h"

#ifdef NDEBUG
#define PPRINT_BATCH_INFO(bi)
#define PPRINT_SERVICE_RESULT(sr)
#else
#define PPRINT_BATCH_INFO(bi)						\
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		pprint_batch_info(bi);					\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
	} while (0)
#define PPRINT_SERVICE_RESULT(sr)				\
	do {								\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
		pprint_service_result(sr);					\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
	} while (0)
#endif

static void __attribute__((unused))
pprint_batch_info(struct batch_info *batch_info)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	int i;

	if (!batch_info)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== batch_info",
			(uint64_t) batch_info);
	OSAL_LOG_DEBUG("%30s: %d", "bi_flags", batch_info->bi_flags);
	OSAL_LOG_DEBUG("%30s: %d", "bi_gen_id", batch_info->bi_gen_id);
	OSAL_LOG_DEBUG("%30s: %d", "bi_svc_type", batch_info->bi_svc_type);
	OSAL_LOG_DEBUG("%30s: %d", "bi_mpool_type", batch_info->bi_mpool_type);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "bi_pcr",
			(uint64_t) batch_info->bi_pcr);

	OSAL_LOG_DEBUG("%30s: %d", "bi_polled_idx", batch_info->bi_polled_idx);
	OSAL_LOG_DEBUG("%30s: %d", "bi_num_entries",
			batch_info->bi_num_entries);

	for (i = 0; i < batch_info->bi_num_entries;  i++) {
		batch_page = GET_PAGE(batch_info, i);
		page_entry = GET_PAGE_ENTRY(batch_page, i);
		OSAL_LOG_DEBUG("%30s: (%d)", "", i);
		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/0x" PRIx64,
				"",
				(uint64_t) batch_page, (uint64_t) page_entry);

		OSAL_LOG_DEBUG("%30s: 0x" PRIx64 "/0x" PRIx64 "/0x" PRIx64 "/%u/%u",
				"",
				(uint64_t) page_entry->bpe_req,
				(uint64_t) page_entry->bpe_res,
				(uint64_t) page_entry->bpe_chain,
				batch_page->bp_tags.bpt_num_hashes,
				batch_page->bp_tags.bpt_num_chksums);
	}

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "bi_req_cb",
			(uint64_t) batch_info->bi_req_cb);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "bi_req_cb_ctx",
			(uint64_t) batch_info->bi_req_cb_ctx);
}

static void __attribute__((unused))
pprint_service_result(struct pnso_service_result *res)
{
	uint32_t i;

	if (!res)
		return;

	OSAL_LOG_DEBUG("%30s: %p", "=== pnso_service_result", res);

	OSAL_LOG_DEBUG("%30s: %d", "err", res->err);

	OSAL_LOG_DEBUG("%30s: %d", "num_services", res->num_services);
	for (i = 0; i < res->num_services; i++) {
		OSAL_LOG_DEBUG("%30s: %d", "service #", i+1);

		OSAL_LOG_DEBUG("%30s: %d", "err", res->svc[i].err);
		OSAL_LOG_DEBUG("%30s: %d", "svc_type", res->svc[i].svc_type);
		OSAL_LOG_DEBUG("%30s: %d", "rsvd_1", res->svc[i].rsvd_1);

		switch (res->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
		case PNSO_SVC_TYPE_DECOMPACT:
			OSAL_LOG_DEBUG("%30s: %d", "data_len",
					res->svc[i].u.dst.data_len);
			break;
		case PNSO_SVC_TYPE_HASH:
			OSAL_LOG_DEBUG("%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			OSAL_LOG_DEBUG("%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			OSAL_LOG_DEBUG("%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			OSAL_LOG_DEBUG("%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

static void *batch_to_poll_ctx(struct batch_info *batch);
static struct batch_info *poll_ctx_to_batch(void *poll_ctx);

static void *
get_mpool_batch_object(struct per_core_resource *pcr,
		enum mem_pool_type pool_type)
{
	pnso_error_t err = EINVAL;
	struct mem_pool *batch_page_mpool, *batch_info_mpool;
	void *p = NULL;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT((pool_type == MPOOL_TYPE_BATCH_PAGE) ||
			(pool_type == MPOOL_TYPE_BATCH_INFO));

	batch_page_mpool = pcr->mpools[MPOOL_TYPE_BATCH_PAGE];
	batch_info_mpool = pcr->mpools[MPOOL_TYPE_BATCH_INFO];
	if (!batch_page_mpool || !batch_info_mpool)
		goto out;

	p =  (void *) ((pool_type == MPOOL_TYPE_BATCH_PAGE) ?
			mpool_get_object(batch_page_mpool) :
			mpool_get_object(batch_info_mpool));
	if (!p) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("cannot obtain batch page/info object from pool! err: %d",
				err);
		goto out;
	}

	OSAL_LOG_DEBUG("obtained batch page/info object from mpool. pcr: 0x " PRIx64 " pool_type: %d",
			(uint64_t) pcr, pool_type);
	OSAL_LOG_DEBUG("exit!");
	return p;

out:
	OSAL_LOG_DEBUG("exit! failed to obtain batch page/info object from mpool. pcr: 0x" PRIx64 " pool_type: %d err: %d",
			(uint64_t) pcr, pool_type, err);
	return p;
}

static pnso_error_t
put_mpool_batch_object(struct per_core_resource *pcr,
		enum mem_pool_type pool_type, void *p)
{
	pnso_error_t err = EINVAL;
	struct mem_pool *batch_page_mpool, *batch_info_mpool;

	OSAL_ASSERT((pool_type == MPOOL_TYPE_BATCH_PAGE) ||
			(pool_type == MPOOL_TYPE_BATCH_INFO));

	batch_page_mpool = pcr->mpools[MPOOL_TYPE_BATCH_PAGE];
	batch_info_mpool = pcr->mpools[MPOOL_TYPE_BATCH_INFO];
	if (!batch_page_mpool || !batch_info_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	if (pool_type == MPOOL_TYPE_BATCH_PAGE)
		err = mpool_put_object(batch_page_mpool, p);
	else
		err = mpool_put_object(batch_info_mpool, p);

	OSAL_LOG_DEBUG("returned batch page/info object to mpool. pcr: 0x " PRIx64 " pool_type: %d",
			(uint64_t) pcr, pool_type);
	return err;

out:
	OSAL_LOG_ERROR("exit! failed to return batch page/info object from mpool. pcr: 0x " PRIx64 " pool_type: %d err: %d",
			(uint64_t) pcr, pool_type, err);
	return err;
}

static pnso_error_t
get_bulk_batch_desc(struct batch_info *batch_info, uint32_t page_idx)
{
	pnso_error_t err;
	struct mem_pool *mpool;
	struct per_core_resource *pcr;

	pcr = batch_info->bi_pcr;
	if (batch_info->bi_mpool_type == MPOOL_TYPE_CRYPTO_DESC_VECTOR) {
		mpool = pcr->mpools[batch_info->bi_mpool_type];
		batch_info->bi_bulk_desc[page_idx] =
			crypto_get_batch_bulk_desc(mpool);
	} else {
		mpool = pcr->mpools[batch_info->bi_mpool_type];
		batch_info->bi_bulk_desc[page_idx] =
			cpdc_get_batch_bulk_desc(mpool);
	}

	if (!batch_info->bi_bulk_desc[page_idx]) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("failed to obtain batch bulk desc. pcr: 0x " PRIx64 " pool_type: %d page_idx: %d err: %d",
			(uint64_t) pcr, batch_info->bi_mpool_type, page_idx, err);
		goto out;
	}

	err = PNSO_OK;

	OSAL_LOG_DEBUG("obtained batch desc. pcr: 0x" PRIx64 " pool_type: %d page_idx: %d desc: 0x" PRIx64,
			(uint64_t) pcr, batch_info->bi_mpool_type, page_idx,
			(uint64_t) batch_info->bi_bulk_desc[page_idx]);
out:
	return err;
}

static void
put_bulk_batch_desc(struct batch_info *batch_info, uint32_t page_idx)
{
	struct mem_pool *mpool;
	struct per_core_resource *pcr;

	pcr = batch_info->bi_pcr;
	if (batch_info->bi_mpool_type == MPOOL_TYPE_CRYPTO_DESC_VECTOR) {
		mpool = pcr->mpools[batch_info->bi_mpool_type];
		crypto_put_batch_bulk_desc(mpool,
				batch_info->bi_bulk_desc[page_idx]);
	} else {
		/* both cpdc desc and pb vector as well covered */
		mpool = pcr->mpools[batch_info->bi_mpool_type];
		cpdc_put_batch_bulk_desc(mpool,
				batch_info->bi_bulk_desc[page_idx]);
	}

	OSAL_LOG_DEBUG("returned batch desc. pcr: 0x" PRIx64 " pool_type: %d page_idx: %d desc: 0x" PRIx64,
			(uint64_t) pcr, batch_info->bi_mpool_type, page_idx,
			(uint64_t) batch_info->bi_bulk_desc[page_idx]);
}

static inline enum mem_pool_type
get_batch_mpool_type(struct pnso_service *svc)
{
	enum mem_pool_type mpool_type;
	struct pnso_hash_desc *hash_desc;
	struct pnso_checksum_desc *chksum_desc;

	switch (svc->svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		mpool_type = MPOOL_TYPE_CRYPTO_DESC_VECTOR;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
		mpool_type = MPOOL_TYPE_CPDC_DESC_VECTOR;
		break;
	case PNSO_SVC_TYPE_HASH:
		hash_desc = &svc->u.hash_desc;
		mpool_type = (hash_desc->flags & PNSO_HASH_DFLAG_PER_BLOCK) ?
			MPOOL_TYPE_CPDC_DESC_PB_VECTOR :
			MPOOL_TYPE_CPDC_DESC_VECTOR;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		chksum_desc = &svc->u.chksum_desc;
		mpool_type = (chksum_desc->flags &
				PNSO_CHKSUM_DFLAG_PER_BLOCK) ?
			MPOOL_TYPE_CPDC_DESC_PB_VECTOR :
			MPOOL_TYPE_CPDC_DESC_VECTOR;
		break;
	default:
		mpool_type = MPOOL_TYPE_NONE;
		OSAL_LOG_DEBUG("invalid pool type! mpool_type: %d",
				mpool_type);
		OSAL_ASSERT(0);
		break;
	}

	return mpool_type;
}

static pnso_error_t
poll_all_chains(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	uint32_t idx;

	OSAL_LOG_DEBUG("poll batch/chain bi_polled_idx: %d",
			batch_info->bi_polled_idx);

	for (idx = batch_info->bi_polled_idx;
			idx < batch_info->bi_num_entries; idx++) {
		batch_page = GET_PAGE(batch_info, idx);
		page_entry = GET_PAGE_ENTRY(batch_page, idx);

		OSAL_LOG_DEBUG("polling batch/chain idx: %d", idx);

		err = chn_poll_all_services(page_entry->bpe_chain);
		if (err)
			break;
	}
	batch_info->bi_polled_idx = idx;

	return err;
}

static void
report_suspect_chains(struct batch_info *batch_info, bool report_internal)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	uint32_t idx;

	for (idx = 0; idx < batch_info->bi_num_entries; idx++) {
		batch_page = GET_PAGE(batch_info, idx);
		page_entry = GET_PAGE_ENTRY(batch_page, idx);

		chn_report_suspect_chain(page_entry->bpe_chain,
				report_internal);
	}
}

static struct batch_info *
init_batch_info(struct per_core_resource *pcr,
		struct pnso_service_request *req)
{
	enum mem_pool_type mpool_type;
	struct batch_info *batch_info = NULL;
	uint16_t gen_id;

	mpool_type = get_batch_mpool_type(&req->svc[0]);
	if (mpool_type == MPOOL_TYPE_NONE)
		goto out;

	batch_info = (struct batch_info *)
		get_mpool_batch_object(pcr, MPOOL_TYPE_BATCH_INFO);
	if (!batch_info)
		goto out;

	gen_id = batch_info->bi_gen_id;
	memset(batch_info, 0, sizeof(struct batch_info));
	batch_info->bi_gen_id = gen_id + 1;

	batch_info->bi_svc_type = req->svc[0].svc_type;
	batch_info->bi_mpool_type = mpool_type;
	batch_info->bi_pcr = pcr;
	batch_info->bi_polled_idx = 0;

	pcr->batch_ctx = batch_to_poll_ctx(batch_info);

out:
	return batch_info;
}

static void
destroy_batch_chain(struct batch_info *batch_info)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	uint32_t idx, num_entries;

	OSAL_LOG_DEBUG("enter ...");

	PPRINT_BATCH_INFO(batch_info);
	num_entries = batch_info->bi_num_entries;
	for (idx = 0; idx < num_entries; idx++) {
		batch_page = GET_PAGE(batch_info, idx);
		page_entry = GET_PAGE_ENTRY(batch_page, idx);

		OSAL_LOG_DEBUG("destroy chain num_entries: %d idx: %d pge: 0x" PRIx64 " chain: 0x" PRIx64,
				num_entries, idx, (uint64_t) page_entry,
				(uint64_t) page_entry->bpe_chain);

		if (page_entry->bpe_chain)
			chn_destroy_chain(page_entry->bpe_chain);
	}

	OSAL_LOG_DEBUG("exit!");
}

static void
deinit_batch(struct batch_info *batch_info)
{
	struct per_core_resource *pcr;
	struct batch_page *batch_page;
	uint32_t idx, num_pages;

	OSAL_LOG_DEBUG("release pages/batch batch_info: 0x" PRIx64,
			(uint64_t) batch_info);

	if (batch_info->bi_flags & BATCH_BFLAG_CHAIN_PRESENT)
		destroy_batch_chain(batch_info);

	pcr = batch_info->bi_pcr;
	num_pages = GET_NUM_PAGES_ACTIVE(batch_info->bi_num_entries);
	for (idx = 0; idx < num_pages; idx++) {
		put_bulk_batch_desc(batch_info, idx);

		batch_page = batch_info->bi_pages[idx];
		put_mpool_batch_object(pcr, MPOOL_TYPE_BATCH_PAGE, batch_page);

		OSAL_LOG_DEBUG("released page pcr: 0x" PRIx64 " page: 0x" PRIx64 " idx: %d",
				(uint64_t) pcr, (uint64_t) batch_page, idx);
	}

	batch_info->bi_gen_id++;
	put_mpool_batch_object(pcr, MPOOL_TYPE_BATCH_INFO, batch_info);
}

void
bat_destroy_batch(struct per_core_resource *pcr)
{
	struct batch_info *batch_info;

	OSAL_LOG_DEBUG("enter ...");

	batch_info = poll_ctx_to_batch(pcr->batch_ctx);
	if (!batch_info) {
		OSAL_LOG_DEBUG("batch not found! pcr: 0x" PRIx64 " poll_ctx: 0x" PRIx64,
				(uint64_t) pcr, (uint64_t) pcr->batch_ctx);
		goto out;
	}
	pcr->batch_ctx = NULL;
	deinit_batch(batch_info);

out:
	OSAL_LOG_DEBUG("exit!");
}

void
bat_clear_batch(struct per_core_resource *pcr)
{
	if (pcr && pcr->batch_ctx) {
		OSAL_LOG_DEBUG("clear batch pcr: 0x" PRIx64 " poll_ctx: 0x" PRIx64,
				(uint64_t) pcr, (uint64_t) pcr->batch_ctx);
		pcr->batch_ctx = NULL;
	}
}

static pnso_error_t
add_page(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct batch_page *batch_page;
	uint32_t page_idx;

	batch_page = (struct batch_page *)
		get_mpool_batch_object(batch_info->bi_pcr,
				MPOOL_TYPE_BATCH_PAGE);
	if (!batch_page) {
		err = EAGAIN;
		OSAL_LOG_DEBUG("failed to obtain batch page from pool! err: %d",
				err);
		goto out;
	}
	memset(batch_page, 0, sizeof(struct batch_page));

	page_idx = batch_info->bi_num_entries / MAX_PAGE_ENTRIES;
	OSAL_ASSERT(page_idx < MAX_NUM_PAGES);
	batch_info->bi_pages[page_idx] = batch_page;

	/* get a vector of either CPDC or Crypto bulk desc */
	err = get_bulk_batch_desc(batch_info, page_idx);
	if (err)
		goto out;

	OSAL_LOG_DEBUG("added new page 0x" PRIx64 " page_idx: %d",
			(uint64_t) batch_page, page_idx);
	err = PNSO_OK;
out:
	return err;
}

static void read_write_result_all_chains(struct batch_info *batch_info);
static void read_write_error_result_all_chains(struct batch_info *batch_info,
					       pnso_error_t err);

static void *batch_to_poll_ctx(struct batch_info *batch)
{
	return req_obj_to_poll_ctx(batch, MPOOL_TYPE_BATCH_INFO,
				   batch->bi_gen_id, batch->bi_pcr);
}

static inline struct batch_info *
poll_ctx_to_batch_params(void *poll_ctx, uint16_t *gen_id,
			 struct per_core_resource **pcr)
{
	struct batch_info *batch;
	uint8_t mpool_type;

	batch = (struct batch_info *) poll_ctx_to_req_obj(poll_ctx, &mpool_type,
							  gen_id, pcr);
	if (!batch || mpool_type != MPOOL_TYPE_BATCH_INFO) {
		OSAL_LOG_DEBUG("invalid batch poll ctx 0x" PRIx64,
			       (uint64_t) poll_ctx);
		return NULL;
	}
	return batch;
}

static struct batch_info *poll_ctx_to_batch(void *poll_ctx)
{
	struct batch_info *batch;
	uint16_t gen_id;
	struct per_core_resource *pcr;

	batch = poll_ctx_to_batch_params(poll_ctx, &gen_id, &pcr);
	if (!batch)
		return NULL;

	if (batch->bi_gen_id != gen_id) {
		OSAL_LOG_ERROR("new batch gen_id %d found, expected %d",
			       batch->bi_gen_id, gen_id);
		return NULL;
	}

	return batch;
}

pnso_error_t
bat_poller(struct batch_info *batch_info, uint16_t gen_id, bool is_timeout)
{
	pnso_error_t err = EINVAL;
	completion_cb_t	cb = NULL;
	void *cb_ctx = NULL;
	bool skip_destroy = false;

	PAS_DECL_SW_PERF();
	PAS_DECL_HW_PERF();

	OSAL_LOG_DEBUG("enter ...");

	OSAL_LOG_DEBUG("core_id: %u", osal_get_coreid());

	OSAL_ASSERT(batch_info);

	PPRINT_BATCH_INFO(batch_info);

	if (batch_info->bi_gen_id != gen_id) {
		OSAL_LOG_ERROR("new batch gen_id %d found, expected %d",
			       batch_info->bi_gen_id, gen_id);
		goto out;
	}

	PAS_SET_SW_PERF(batch_info->bi_sw_latency_start);
	PAS_SET_HW_PERF(batch_info->bi_hw_latency_start);

	if (is_timeout) {
		if (pnso_lif_reset_ctl_pending()) {
			report_suspect_chains(batch_info, false);
			report_suspect_chains(batch_info, true);
		}

		err = ETIMEDOUT;
	} else {
		err = poll_all_chains(batch_info);
		if (err == ETIMEDOUT && pnso_lif_reset_ctl_pending()) {
			skip_destroy = true;
			OSAL_LOG_DEBUG("skip destroying batch 0x" PRIx64,
				       (uint64_t) batch_info);
		}
	}
	if (err) {
		OSAL_LOG_DEBUG("poll failed! batch_info: 0x" PRIx64 "err: %d",
			(uint64_t) batch_info, err);

		if (err == EBUSY)
			goto out;

		PAS_END_HW_PERF(batch_info->bi_pcr);
		PAS_END_SW_PERF(batch_info->bi_pcr);

		read_write_error_result_all_chains(batch_info, err);
	} else {
		PAS_END_HW_PERF(batch_info->bi_pcr);
		PAS_END_SW_PERF(batch_info->bi_pcr);

		/*
		 * on success, read/write result from first to last chain's - first
		 * to last service - of the entire batch
		 *
		 */
		read_write_result_all_chains(batch_info);
	}

	/* save caller's cb and context ahead of destroy */
	cb = batch_info->bi_req_cb;
	cb_ctx = batch_info->bi_req_cb_ctx;
	batch_info->bi_req_cb_ctx = NULL;

	if (!skip_destroy)
		deinit_batch(batch_info);

	if (cb && cb_ctx) {
		OSAL_LOG_DEBUG("invoking caller's cb ctx: 0x" PRIx64 "err: %d",
				(uint64_t) cb_ctx, err);
		cb(cb_ctx, NULL);
	}

	return err;

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
pnso_batch_poller(void *poll_ctx)
{
	pnso_error_t err;
	struct per_core_resource *pcr;
	struct batch_info *batch_info;
	uint16_t gen_id;

	batch_info = poll_ctx_to_batch_params(poll_ctx, &gen_id, &pcr);
	if (!batch_info)
		return EINVAL;

	if (!sonic_try_reserve_per_core_res(pcr))
		return PNSO_LIF_IO_ERROR;

	err = bat_poller(batch_info, gen_id, false);

	sonic_unreserve_per_core_res(pcr);

	return err;
}

/* Note: assumes this is called only while pcr is exclusively reserved */
void
bat_poll_timeout_all(struct per_core_resource *pcr)
{
	struct mem_pool *batch_info_mpool;
	void *obj = NULL;
	struct batch_info *batch_info;
	uint32_t count = 0;

	OSAL_LOG_DEBUG("enter ...");

	batch_info_mpool = pcr->mpools[MPOOL_TYPE_BATCH_INFO];
	if (!batch_info_mpool) {
		goto out;
	}

	obj = mpool_get_first_inuse_object(batch_info_mpool);
	while (obj) {
		batch_info = (struct batch_info *) obj;
		bat_poller(batch_info, batch_info->bi_gen_id, true);

		obj = mpool_get_next_inuse_object(batch_info_mpool, obj);
		count++;
	}

	if (count)
		OSAL_LOG_DEBUG("timed out %u batch entries", count);

out:
	OSAL_LOG_DEBUG("exit!");
}

static inline void
set_batch_mode(uint16_t mode_flags, uint16_t *flags)
{
	if (mode_flags & REQUEST_RFLAG_MODE_SYNC)
		*flags |= BATCH_BFLAG_MODE_SYNC;
	else if (mode_flags & REQUEST_RFLAG_MODE_POLL)
		*flags |= BATCH_BFLAG_MODE_POLL;
	else if (mode_flags & REQUEST_RFLAG_MODE_ASYNC)
		*flags |= BATCH_BFLAG_MODE_ASYNC;
}

static pnso_error_t
set_interrupt_params(struct batch_info *batch_info, struct service_chain *chain,
		     void *poll_ctx)
{
	struct service_chain *last_chain;
	struct chain_entry *last_ce;
	struct service_info *svc_info;

	last_chain = chn_get_last_service_chain(batch_info);
	last_ce = chn_get_last_centry(last_chain);
	svc_info = &last_ce->ce_svc_info;

	return svc_info->si_ops.enable_interrupt(svc_info, poll_ctx);
}

static pnso_error_t
build_batch(struct batch_info *batch_info, struct request_params *req_params)
{
	pnso_error_t err;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	struct service_chain *chain = NULL;
	uint32_t idx, num_entries;
	void *poll_ctx = NULL;

	OSAL_LOG_DEBUG("enter ...");

	set_batch_mode(req_params->rp_flags, &batch_info->bi_flags);

	if ((req_params->rp_flags & REQUEST_RFLAG_MODE_ASYNC) ||
			(req_params->rp_flags & REQUEST_RFLAG_MODE_POLL)) {
		batch_info->bi_sw_latency_start = req_params->rp_sw_latency_ts;

		batch_info->bi_req_cb = req_params->rp_cb;
		batch_info->bi_req_cb_ctx = req_params->rp_cb_ctx;

		poll_ctx = batch_to_poll_ctx(batch_info);
		if (req_params->rp_flags & REQUEST_RFLAG_MODE_POLL) {
			/* for caller to poll */
			*req_params->rp_poll_fn = pnso_batch_poller;
			*req_params->rp_poll_ctx = poll_ctx;
		}
	}
	PPRINT_BATCH_INFO(batch_info);

	req_params->rp_batch_info = batch_info;
	num_entries = batch_info->bi_num_entries;
	for (idx = 0; idx < num_entries; idx++) {
		batch_page = GET_PAGE(batch_info, idx);
		page_entry = GET_PAGE_ENTRY(batch_page, idx);

		OSAL_LOG_DEBUG("use pge to batch num_entries: %d idx: %d page: 0x" PRIx64 " pge: 0x" PRIx64,
				num_entries, idx, (uint64_t) batch_page,
				(uint64_t) page_entry);

		req_params->rp_svc_req = page_entry->bpe_req;
		req_params->rp_svc_res = page_entry->bpe_res;
		req_params->rp_page = batch_page;
		req_params->rp_batch_index = idx;

		err = chn_create_chain(req_params, &chain);
		if (err) {
			OSAL_LOG_DEBUG("failed to build batch of chains! idx: %d err: %d",
					idx, err);
			PAS_INC_NUM_CHAIN_FAILURES(batch_info->bi_pcr);
			if (idx)
				batch_info->bi_flags |=
					BATCH_BFLAG_CHAIN_PRESENT;
			goto out;
		}
		page_entry->bpe_chain = chain;
		PAS_INC_NUM_CHAINS(batch_info->bi_pcr);
	}
	batch_info->bi_flags |= BATCH_BFLAG_CHAIN_PRESENT;

	/* setup interrupt params in last chain's last service */
	if ((req_params->rp_flags & REQUEST_RFLAG_MODE_ASYNC) && chain) {
		err = set_interrupt_params(batch_info, chain, poll_ctx);
		if (err)
			goto out;
	}

	OSAL_LOG_DEBUG("added all entries to batch! num_entries: %d",
			num_entries);
	PPRINT_BATCH_INFO(batch_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
bat_add_to_batch(struct per_core_resource *pcr,
		struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res)
{
	pnso_error_t err = EINVAL;
	struct batch_info *batch_info = NULL;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	uint16_t req_svc_type;
	bool new_batch = false;

	OSAL_LOG_DEBUG("enter ...");

	if (pcr->batch_ctx)
		batch_info = poll_ctx_to_batch(pcr->batch_ctx);
	if (!batch_info) {
		batch_info = init_batch_info(pcr, svc_req);
		if (!batch_info) {
			err = EAGAIN;
			OSAL_LOG_DEBUG("failed to init batch! err: %d",
					err);
			goto out;
		}
		new_batch = true;
	}

	if (batch_info->bi_num_entries >= MAX_NUM_BATCH_ENTRIES) {
		OSAL_LOG_DEBUG("batch limit reached! num_entries: %d err: %d",
				batch_info->bi_num_entries, err);
		goto out_batch;
	}

	if (!new_batch) {
		req_svc_type = svc_req->svc[0].svc_type;
		if (batch_info->bi_svc_type != req_svc_type) {
			err = EINVAL;
			OSAL_LOG_WARN("batch service type mismatch! batch_svc_type: %d req_svc_type: %d err: %d",
					batch_info->bi_svc_type,
					req_svc_type, err);
			goto out_batch;
		}
	}

	if ((batch_info->bi_num_entries % MAX_PAGE_ENTRIES) == 0) {
		err = add_page(batch_info);
		if (err)
			goto out_batch;
	}

	batch_page = GET_PAGE(batch_info, batch_info->bi_num_entries);
	page_entry = GET_PAGE_ENTRY(batch_page, batch_info->bi_num_entries);

	page_entry->bpe_req = svc_req;
	page_entry->bpe_res = svc_res;
	batch_info->bi_num_entries++;

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out_batch:
	pcr->batch_ctx = NULL;
	deinit_batch(batch_info);
out:
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}

static void
read_write_result_all_chains(struct batch_info *batch_info)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	int i;

	for (i = 0; i < batch_info->bi_num_entries;  i++) {
		batch_page = GET_PAGE(batch_info, i);
		page_entry = GET_PAGE_ENTRY(batch_page, i);

		chn_read_write_result(page_entry->bpe_chain);

		chn_update_overall_result(page_entry->bpe_chain);

		chn_notify_caller(page_entry->bpe_chain);

		PPRINT_SERVICE_RESULT(page_entry->bpe_res);
	}
}

static void
read_write_error_result_all_chains(struct batch_info *batch_info, pnso_error_t err)
{
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	int i;

	for (i = 0; i < batch_info->bi_num_entries;  i++) {
		batch_page = GET_PAGE(batch_info, i);
		page_entry = GET_PAGE_ENTRY(batch_page, i);

		if (page_entry->bpe_chain) {
			page_entry->bpe_chain->sc_res->err = err;
			chn_notify_caller(page_entry->bpe_chain);
		}
	}
}

static void
set_batch_page_cflag(struct batch_page *bp, uint16_t cflag)
{
	struct batch_page_entry *bpe;
	uint32_t i;

	for (i = 0; i < MAX_PAGE_ENTRIES; i++) {
		bpe = GET_PAGE_ENTRY(bp, i);
		if (!bpe->bpe_chain)
			break;
		bpe->bpe_chain->sc_flags |= cflag;
	}
}

static void
clear_batch_page_cflag(struct batch_page *bp, uint16_t cflag)
{
	struct batch_page_entry *bpe;
	uint32_t i;

	for (i = 0; i < MAX_PAGE_ENTRIES; i++) {
		bpe = GET_PAGE_ENTRY(bp, i);
		if (!bpe->bpe_chain)
			break;
		bpe->bpe_chain->sc_flags &= ~cflag;
	}
}

static pnso_error_t
execute_batch(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr;
	struct service_chain *first_chain, *last_chain;
	struct chain_entry *first_ce, *last_ce;
	uint32_t idx;
	uint32_t num_entries;
	uint16_t async_evid;
	bool is_sync_mode = (batch_info->bi_flags & BATCH_BFLAG_MODE_SYNC) != 0;

	PAS_DECL_HW_PERF();

	OSAL_LOG_DEBUG("enter ...");

	/* get last chain's last service of the batch */
	last_chain = chn_get_last_service_chain(batch_info);
	last_ce = chn_get_last_centry(last_chain);

	async_evid = last_chain->sc_async_evid;
	pcr = batch_info->bi_pcr;

	num_entries = batch_info->bi_num_entries;
	for (idx = 0; idx < num_entries; idx += MAX_PAGE_ENTRIES) {
		/* get first chain's first service within the mini-batch */
		first_chain = chn_get_first_service_chain(batch_info, idx);
		first_ce = chn_get_first_centry(first_chain);
		OSAL_LOG_DEBUG("ring DB batch idx: %d", idx);

		/* ring DB first chain's first service within the mini-batch  */
		set_batch_page_cflag(GET_PAGE(batch_info, idx),
				CHAIN_CFLAG_RANG_DB);

		/*
		 * record the perf-event just for the 1st page; similar to
		 * chain non-batch case, record the perf-event ahead of ringing
		 * the db.
		 *
		 */
		if (!batch_info->bi_hw_latency_start)
			PAS_START_HW_PERF(batch_info->bi_hw_latency_start);

		err = first_ce->ce_svc_info.si_ops.ring_db(
				&first_ce->ce_svc_info);
		if (err) {
			PAS_END_HW_PERF(pcr);

			clear_batch_page_cflag(GET_PAGE(batch_info, idx),
					CHAIN_CFLAG_RANG_DB);
			OSAL_LOG_DEBUG("failed to ring service door bell! svc_type: %d err: %d",
				       first_ce->ce_svc_info.si_type, err);
			goto out;
		}
	}

	if (!is_sync_mode) {
		OSAL_LOG_DEBUG("in non-sync mode ...");
		if (async_evid)
			sonic_intr_touch_ev_id(pcr, async_evid);

		goto done;
	}

	/* poll on last chain's last service of the batch */
	err = last_ce->ce_svc_info.si_ops.poll(&last_ce->ce_svc_info);
	if (err) {
		PAS_END_HW_PERF(pcr);

		/* in sync-mode, poll() will return either OK or ETIMEDOUT */
		OSAL_LOG_ERROR("service failed to poll svc_type: %d err: %d",
			       last_ce->ce_svc_info.si_type, err);
		goto out;
	}

	/* on success, loop n' poll on every chain's - every service(s) */
	err = poll_all_chains(batch_info);
	PAS_END_HW_PERF(pcr);
	if (err) {
		OSAL_LOG_ERROR("failed to poll all services err: %d", err);
		goto out;
	}

	/*
	 * on success, read/write result from first to last chain's - first
	 * to last service - of the entire batch
	 *
	 */
	read_write_result_all_chains(batch_info);

done:
	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
bat_flush_batch(struct request_params *req_params)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = req_params->rp_pcr;
	struct batch_info *batch_info;
	bool is_sync_mode;
	uint32_t num_entries = 0;

	OSAL_LOG_DEBUG("enter ...");

	batch_info = poll_ctx_to_batch(pcr->batch_ctx);
	if (!batch_info) {
		OSAL_LOG_DEBUG("invalid thread/request! err: %d", err);
		goto out;
	}
	num_entries = batch_info->bi_num_entries;
	PAS_INC_NUM_BATCH_REQUESTS(pcr, num_entries);

	err = build_batch(batch_info, req_params);
	if (err) {
		OSAL_LOG_DEBUG("batch/build failed! err: %d", err);
		goto out;
	}

	is_sync_mode = (batch_info->bi_flags & BATCH_BFLAG_MODE_SYNC) != 0;
	err = execute_batch(batch_info);
	if (err) {
		OSAL_LOG_DEBUG("batch/execute failed! err: %d", err);
		goto out;
	}
	/* do not read or write batch_info after this point */

	if (!is_sync_mode)
		pcr->batch_ctx = NULL;

	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	PAS_INC_NUM_BATCH_REQUEST_FAILURES(pcr, num_entries);
	OSAL_LOG_SPECIAL_ERROR("exit! err: %d", err);
	return err;
}
