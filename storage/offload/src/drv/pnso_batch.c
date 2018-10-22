/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>

#include "sonic_dev.h"
#include "sonic_lif.h"

#include "osal.h"

#include "pnso_mpool.h"
#include "pnso_batch.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_crypto_cmn.h"

/*
 * NOTE/TODO-batch:
 *	- few assumptions on batching are ... same type of chain-services
 *	are added to batch; 1-entry batch not supported.
 *	- cache 1st service type from 1st request, and use it to reject
 *	batch-add process, if the assumption of same request is fed in the
 *	session; secondly, use this type info to determine the pool for
 *	bulk desc.
 *
 */

#ifdef NDEBUG
#define PPRINT_BATCH_INFO(bi)
#else
#define PPRINT_BATCH_INFO(bi)						\
	do {								\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
		pprint_batch_info(bi);					\
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

	OSAL_LOG_INFO("%30s: 0x%llx", "=== batch_info",  (uint64_t) batch_info);
	OSAL_LOG_INFO("%30s: %d", "bi_svc_type", batch_info->bi_svc_type);
	OSAL_LOG_INFO("%30s: 0x%llx", "bi_xxxx_desc",
			(uint64_t) batch_info->u.bi_cpdc_desc);
	OSAL_LOG_INFO("%30s: 0x%llx", "bi_pcr",
			(uint64_t) batch_info->bi_pcr);
	OSAL_LOG_INFO("%30s: %d", "bi_num_entries", batch_info->bi_num_entries);

	for (i = 0; i < batch_info->bi_num_entries;  i++) {
		batch_page = GET_PAGE(batch_info, i);
		page_entry = GET_PAGE_ENTRY(batch_page, i);
		OSAL_LOG_DEBUG("%30s: (%d)", "", i);
		OSAL_LOG_DEBUG("%30s: 0x%llx/0x%llx", "",
				(uint64_t) batch_page, (uint64_t) page_entry);
		OSAL_LOG_DEBUG("%30s: 0x%llx/0x%llx/0x%llx", "",
				(uint64_t) page_entry->bpe_req,
				(uint64_t) page_entry->bpe_res,
				(uint64_t) page_entry->bpe_chain);
	}

	OSAL_LOG_INFO("%30s: 0x%llx", "bi_req_cb",
			(uint64_t) batch_info->bi_req_cb);
	OSAL_LOG_INFO("%30s: 0x%llx", "bi_req_cb_ctx",
			(uint64_t) batch_info->bi_req_cb_ctx);
	OSAL_LOG_INFO("%30s: 0x%llx", "bi_req_poll_fn",
			(uint64_t) batch_info->bi_req_poll_fn);
	OSAL_LOG_INFO("%30s: 0x%llx", "bi_req_poll_ctx",
			(uint64_t) batch_info->bi_req_poll_ctx);
}

static void *
get_mpool_batch_object(enum mem_pool_type pool_type)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct mem_pool *batch_page_mpool, *batch_info_mpool;
	void *p = NULL;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT((pool_type == MPOOL_TYPE_BATCH_PAGE) ||
			(pool_type == MPOOL_TYPE_BATCH_INFO));

	batch_page_mpool = pcr->mpools[MPOOL_TYPE_BATCH_PAGE];
	batch_info_mpool = pcr->mpools[MPOOL_TYPE_BATCH_INFO];
	if (!batch_page_mpool || !batch_info_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	p =  (void *) ((pool_type == MPOOL_TYPE_BATCH_PAGE) ?
			mpool_get_object(batch_page_mpool) :
			mpool_get_object(batch_info_mpool));
	if (!p) {
		OSAL_ASSERT(0);
		goto out;
	}

	OSAL_LOG_DEBUG("obtained batch page/info object from mpool. pool_type: %d",
			pool_type);
	OSAL_LOG_DEBUG("exit!");
	return p;

out:
	OSAL_LOG_ERROR("exit! failed to obtain batch page/info object from mpool. pool_type: %d err: %d",
			pool_type, err);
	return p;
}

static pnso_error_t
put_mpool_batch_object(enum mem_pool_type pool_type, void *p)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct mem_pool *batch_page_mpool, *batch_info_mpool;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT((pool_type == MPOOL_TYPE_BATCH_PAGE) ||
			(pool_type == MPOOL_TYPE_BATCH_INFO));

	batch_page_mpool = pcr->mpools[MPOOL_TYPE_BATCH_PAGE];
	batch_info_mpool = pcr->mpools[MPOOL_TYPE_BATCH_INFO];
	if (!batch_page_mpool || !batch_info_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	err  = (pool_type == MPOOL_TYPE_BATCH_PAGE) ?
			mpool_put_object(batch_page_mpool, p) :
			mpool_put_object(batch_info_mpool, p);
	if (err) {
		OSAL_ASSERT(0);
		goto out;
	}

	OSAL_LOG_DEBUG("returned batch page/info object to mpool. pool_type: %d",
			pool_type);
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! failed to return batch page/info object from mpool. pool_type: %d err: %d",
			pool_type, err);
	return err;
}

static void
get_bulk_batch_desc(struct batch_info *batch_info)
{
	struct mem_pool *mpool;
	struct per_core_resource *pcr;

	pcr = batch_info->bi_pcr;
	switch (batch_info->bi_svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		// mpool = pcr->mpools[batch_info->bi_mpool_type];
		mpool = pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR];
		batch_info->u.bi_crypto_desc =
			crypto_get_batch_bulk_desc(mpool);

		OSAL_LOG_DEBUG("obtained desc pool_type: %d desc: 0x%llxd",
			batch_info->bi_mpool_type,
			(uint64_t) batch_info->u.bi_crypto_desc);
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		// mpool = pcr->mpools[batch_info->bi_mpool_type];
		mpool = pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR];
		batch_info->u.bi_cpdc_desc =
			cpdc_get_batch_bulk_desc(mpool);

		OSAL_LOG_DEBUG("obtained desc pool_type: %d desc: 0x%llxd",
			batch_info->bi_mpool_type,
			(uint64_t) batch_info->u.bi_cpdc_desc);
		break;
	default:
		OSAL_ASSERT(0);
		break;
	}
}

static void
put_bulk_batch_desc(struct batch_info *batch_info)
{
	struct mem_pool *mpool;
	struct per_core_resource *pcr;

	switch (batch_info->bi_svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		pcr = batch_info->bi_pcr;
#if 1
		mpool = pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR];
#else
		mpool = pcr->mpools[batch_info->bi_mpool_type];
#endif
		crypto_put_batch_bulk_desc(mpool, batch_info->u.bi_crypto_desc);

		OSAL_LOG_DEBUG("returned batch desc pool_type: %d desc: 0x%llxd",
			batch_info->bi_mpool_type,
			(uint64_t) batch_info->u.bi_crypto_desc);
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		pcr = batch_info->bi_pcr;
#if 1
		mpool = pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR];
#else
		mpool = pcr->mpools[batch_info->bi_mpool_type];
#endif
		cpdc_put_batch_bulk_desc(mpool, batch_info->u.bi_cpdc_desc);

		OSAL_LOG_DEBUG("returned batch desc pool_type: %d desc: 0x%llxd",
			batch_info->bi_mpool_type,
			(uint64_t) batch_info->u.bi_cpdc_desc);
		break;
	default:
		OSAL_ASSERT(0);
		break;
	}
}

static struct batch_info *
init_batch_info(struct pnso_service_request *req)
{
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct batch_info *batch_info = NULL;

	batch_info = (struct batch_info *)
		get_mpool_batch_object(MPOOL_TYPE_BATCH_INFO);
	if (!batch_info)
		goto out;

	memset(batch_info, 0, sizeof(struct batch_info));
	batch_info->bi_svc_type = req->svc[0].svc_type;
	// batch_info->bi_mpool_type = TODO
	batch_info->bi_pcr = pcr;
	get_bulk_batch_desc(batch_info);

	pcr->batch_info = batch_info;
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

		OSAL_LOG_DEBUG("destroy chain num_entries: %d idx: %d pge: 0x%llx chain: 0x%llx",
				num_entries, idx, (uint64_t) page_entry,
				(uint64_t) page_entry->bpe_chain);

		OSAL_ASSERT(page_entry->bpe_chain);
		chn_destroy_chain(page_entry->bpe_chain);
	}

	OSAL_LOG_DEBUG("exit!");
}

static void
deinit_batch(struct batch_info *batch_info)
{
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct batch_page *batch_page;
	uint32_t idx, num_pages;

	OSAL_LOG_DEBUG("release pages/batch batch_info: 0x%llx",
			(uint64_t) batch_info);

	put_bulk_batch_desc(batch_info);

	destroy_batch_chain(batch_info);

	num_pages = GET_NUM_PAGES_ACTIVE(batch_info->bi_num_entries);
	for (idx = 0; idx < num_pages; idx++) {
		batch_page = batch_info->bi_pages[idx];
		put_mpool_batch_object(MPOOL_TYPE_BATCH_PAGE, batch_page);
		OSAL_LOG_DEBUG("released page 0x%llx idx: %d",
				(uint64_t) batch_page, idx);
	}

	put_mpool_batch_object(MPOOL_TYPE_BATCH_INFO, batch_info);
	pcr->batch_info = NULL;

	return;
}

void
bat_destroy_batch(void)
{
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct batch_info *batch_info;

	OSAL_LOG_DEBUG("enter ...");

	batch_info = pcr->batch_info;
	if (!batch_info) {
		OSAL_LOG_DEBUG("batch not found! pcr: 0x%llx",
				(uint64_t) pcr);
		goto out;
	}
	deinit_batch(batch_info);

out:
	OSAL_LOG_DEBUG("exit!");
	return;
}

static pnso_error_t
add_page(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct batch_page *batch_page;
	uint32_t idx;

	batch_page = (struct batch_page *)
		get_mpool_batch_object(MPOOL_TYPE_BATCH_PAGE);
	if (!batch_page) {
		OSAL_LOG_DEBUG("failed to obtain batch page from pool! err: %d",
				err);
		goto out;
	}
	memset(batch_page, 0, sizeof(struct batch_page));

	idx = batch_info->bi_num_entries / MAX_PAGE_ENTRIES;
	OSAL_ASSERT(idx <= MAX_NUM_PAGES);
	batch_info->bi_pages[idx] = batch_page;

	OSAL_LOG_DEBUG("added new page 0x%llx idx: %d",
			(uint64_t) batch_page, idx);
	err = PNSO_OK;
out:
	return err;
}

pnso_error_t
build_batch(struct batch_info *batch_info,
		const completion_cb_t cb, void *cb_ctx,
		void *pnso_poll_fn, void **pnso_poll_ctx)
{
	pnso_error_t err = EINVAL;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	uint32_t idx, num_entries;

	OSAL_LOG_DEBUG("enter ...");

	PPRINT_BATCH_INFO(batch_info);
	num_entries = batch_info->bi_num_entries;
	for (idx = 0; idx < num_entries; idx++) {
		batch_page = GET_PAGE(batch_info, idx);
		page_entry = GET_PAGE_ENTRY(batch_page, idx);

		OSAL_LOG_DEBUG("add pge to batch num_entries: %d idx: %d pge: 0x%llx",
				num_entries, idx, (uint64_t) page_entry);

		err = chn_build_batch_chain(batch_info, page_entry,
				idx, cb, cb_ctx, pnso_poll_fn, pnso_poll_ctx);
		if (err) {
			OSAL_LOG_DEBUG("failed to build batch of chains! err: %d",
					err);
			goto out;
		}
	}
	OSAL_LOG_DEBUG("added all entries batch! num_entries: %d", num_entries);
	PPRINT_BATCH_INFO(batch_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
bat_add_to_batch(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct batch_info *batch_info;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;

	OSAL_LOG_DEBUG("enter ...");

	batch_info = pcr->batch_info;
	if (!batch_info) {
		batch_info = init_batch_info(svc_req);
		if (!batch_info) {
			OSAL_LOG_DEBUG("failed to init batch info! err: %d",
					err);
			goto out;
		}
	}

	if (batch_info->bi_num_entries >= MAX_NUM_BATCH_ENTRIES) {
		OSAL_LOG_DEBUG("batch limit reached! num_entries: %d err: %d",
				batch_info->bi_num_entries, err);
		goto out_batch;
	}

	if ((batch_info->bi_num_entries % MAX_PAGE_ENTRIES) == 0)
		add_page(batch_info);

	batch_page = GET_PAGE(batch_info, batch_info->bi_num_entries);
	page_entry = GET_PAGE_ENTRY(batch_page, batch_info->bi_num_entries);

	page_entry->bpe_req = svc_req;
	page_entry->bpe_res = svc_res;
	batch_info->bi_num_entries++;

	PPRINT_BATCH_INFO(batch_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out_batch:
	deinit_batch(batch_info);
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
poll_all_chains(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct batch_page *batch_page;
	struct batch_page_entry *page_entry;
	int i;

	for (i = 0; i < batch_info->bi_num_entries;  i++) {
		batch_page = GET_PAGE(batch_info, i);
		page_entry = GET_PAGE_ENTRY(batch_page, i);

		err = chn_poll_all_services(page_entry->bpe_chain);
		if (err)
			break;
	}

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

		/* TODO-batch: handle error cases */
		chn_read_write_result(page_entry->bpe_chain);

		/* update over all status of the chain */
		chn_update_overall_result(page_entry->bpe_chain);

		/* TODO-batch: handle sync/async */
		chn_notify_caller(page_entry->bpe_chain);
	}
}

static pnso_error_t
execute_batch(struct batch_info *batch_info)
{
	pnso_error_t err = EINVAL;
	struct service_chain *first_chain, *last_chain;
	struct chain_entry *first_ce, *last_ce;

	OSAL_LOG_DEBUG("enter ...");

	/* get first chain's first service */
	first_chain = chn_get_first_service_chain(batch_info);
	first_ce = chn_get_first_centry(first_chain);

	/* get last chain's last service */
	last_chain = chn_get_last_service_chain(batch_info);
	last_ce = chn_get_last_centry(last_chain);

	/* ring DB first chain's first service */
	err = first_ce->ce_svc_info.si_ops.schedule(&first_ce->ce_svc_info);
	if (err) {
		OSAL_LOG_DEBUG("failed to schedule the service svc_type: %d err: %d",
			       first_ce->ce_svc_info.si_type, err);
		goto out;
	}

	/* poll on last chain's last service */
	err = last_ce->ce_svc_info.si_ops.poll(&last_ce->ce_svc_info);
	if (err) {
		OSAL_LOG_ERROR("service failed to poll svc_type: %d err: %d",
			       last_ce->ce_svc_info.si_type, err);
		goto out;
	}

	/* on success, poll in loop on every chain's every service */
	err = poll_all_chains(batch_info);
	if (err) {
		OSAL_LOG_ERROR("failed to poll all services err: %d", err);
		goto out;
	}

	/*
	 * on success, read/write result from first to last chain's first
	 * to last service
	 *
	 */
	read_write_result_all_chains(batch_info);

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
bat_flush_batch(completion_cb_t cb, void *cb_ctx, pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr = putil_get_per_core_resource();
	struct batch_info *batch_info;

	OSAL_LOG_DEBUG("enter ...");

	batch_info = pcr->batch_info;
	if (!batch_info) {
		OSAL_LOG_DEBUG("invalid thread/request! err: %d", err);
		goto out;
	}

	batch_info->bi_req_cb = cb;
	batch_info->bi_req_cb_ctx = cb_ctx;
	batch_info->bi_req_poll_fn = pnso_poll_fn;
	batch_info->bi_req_poll_ctx = pnso_poll_ctx;

	err = build_batch(batch_info, cb, cb_ctx,
			pnso_poll_fn, pnso_poll_ctx);
	if (err) {
		OSAL_LOG_DEBUG("batch-build failed! err: %d", err);
		goto out_deinit;
	}

	err = execute_batch(batch_info);
	if (err) {
		OSAL_LOG_DEBUG("batch-execute failed! err: %d", err);
		goto out_deinit;
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit!");
	return err;

out_deinit:
	deinit_batch(batch_info);
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}
