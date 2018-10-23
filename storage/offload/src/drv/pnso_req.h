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

// #include "pnso_api.h"

struct request_params {
	uint16_t rp_flags;	/* mode/type/state flags (rflags) */

	struct batch_info *rp_batch_info;
	uint16_t rp_batch_index;

	struct pnso_service_request *rp_svc_req;
	struct pnso_service_result *rp_svc_res;

	completion_cb_t rp_cb;
	void *rp_cb_ctx;
	pnso_poll_fn_t *rp_poll_fn;
	void *rp_poll_ctx;
};

#define REQUEST_RFLAG_MODE_SYNC		(1 << 0)
#define REQUEST_RFLAG_MODE_POLL		(1 << 1)
#define REQUEST_RFLAG_MODE_ASYNC	(1 << 2)
#define REQUEST_RFLAG_TYPE_CHAIN	(1 << 3)
#define REQUEST_RFLAG_TYPE_BATCH	(1 << 4)
#define REQUEST_RFLAG_STATE_CHAINED	(1 << 5)
#define REQUEST_RFLAG_STATE_BATCHED	(1 << 6)

void req_pprint_request(const struct pnso_service_request *req);

void req_pprint_result(const struct pnso_service_result *res);

#ifdef __cplusplus
}
#endif

#endif	/* __PNSO_REQ_H__ */
