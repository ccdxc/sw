/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_REQ_H__
#define __PNSO_REQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#define REQ_SZ_TO_NUM_BLKS(sz, blk_sz)			\
	(((sz) + (blk_sz) - 1) / (blk_sz))

#define REQ_SZ_ROUND_UP_TO_BLK_SZ(sz, blk_sz)		\
	(REQ_SZ_TO_NUM_BLKS(sz, blk_sz) * (blk_sz))

union request_poll_context {
	uint64_t val;
	struct {
		uint32_t obj_id;
		uint16_t gen_id;
		uint8_t pcr_id;
		uint8_t mpool_type;
	} s;
};

struct request_params {
	uint16_t rp_flags;	/* mode/type flags (rflags) */

	struct per_core_resource *rp_pcr;	/* to access pool/etc. */

	struct batch_info *rp_batch_info;
	struct batch_page *rp_page;
	uint16_t rp_batch_index;

	uint64_t rp_sw_latency_ts;

	struct pnso_service_request *rp_svc_req;
	struct pnso_service_result *rp_svc_res;

	completion_cb_t rp_cb;
	void *rp_cb_ctx;

	pnso_poll_fn_t *rp_poll_fn;
	void **rp_poll_ctx;
};

#define REQUEST_RFLAG_MODE_SYNC		(1 << 0)
#define REQUEST_RFLAG_MODE_POLL		(1 << 1)
#define REQUEST_RFLAG_MODE_ASYNC	(1 << 2)
#define REQUEST_RFLAG_TYPE_CHAIN	(1 << 3)
#define REQUEST_RFLAG_TYPE_BATCH	(1 << 4)

void req_pprint_request(const struct pnso_service_request *req);

void req_pprint_result(const struct pnso_service_result *res);

pnso_error_t pnso_request_poller(void *poll_ctx);
pnso_error_t pnso_request_poll_timeout(void *poll_ctx);

#ifdef __cplusplus
}
#endif

#endif	/* __PNSO_REQ_H__ */
