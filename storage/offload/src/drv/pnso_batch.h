/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_BATCH_H__
#define __PNSO_BATCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAGE_ENTRIES_SHIFT_BITS	(4)	/* 16 entries per page */
#define MAX_PAGE_ENTRIES		(1 << MAX_PAGE_ENTRIES_SHIFT_BITS)
#define MAX_NUM_PAGES			(32)	/* 512 entries per batch */
#define MAX_NUM_BATCH_ENTRIES		(MAX_NUM_PAGES * MAX_PAGE_ENTRIES)

#define GET_NUM_PAGES_ACTIVE(n)	\
	((n + MAX_PAGE_ENTRIES - 1) / MAX_PAGE_ENTRIES)
#define GET_PAGE(bi, idx)	\
	bi->bi_pages[idx >> MAX_PAGE_ENTRIES_SHIFT_BITS]
#define GET_PAGE_ENTRY(p, idx)	\
	(&p->bp_entries[idx & ((1 << MAX_PAGE_ENTRIES_SHIFT_BITS) - 1)])

struct batch_page {
	struct batch_page_entry {
		struct pnso_service_request *bpe_req;
		struct pnso_service_result *bpe_res;
		struct service_chain *bpe_chain;
	} bp_entries[MAX_PAGE_ENTRIES];
};

struct batch_info {
	uint8_t bi_svc_type;		/* type of 1st service in chain */
	uint8_t bi_mpool_type;		/* type of pool type */
	union {
		struct cpdc_desc *bi_cpdc_desc;
		struct crypto_desc *bi_crypto_desc;
	} u;

	struct per_core_resource *bi_pc_res;	/* TODO-batch: why?! */

	completion_cb_t	bi_req_cb;	/* caller supplied call-back */
	void *bi_req_cb_ctx;		/* caller supplied cb context */
	pnso_poll_fn_t *bi_req_poll_fn;	/* poller to run in caller's thread */
	void *bi_req_poll_ctx;		/* request context for poller */

	uint32_t bi_num_entries;
	struct batch_page *bi_pages[MAX_NUM_PAGES];
};

pnso_error_t bat_add_to_batch(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res);

pnso_error_t bat_flush_batch(completion_cb_t cb, void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn, void **pnso_poll_ctx);

void bat_destroy_batch(void);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_BATCH_H__ */
