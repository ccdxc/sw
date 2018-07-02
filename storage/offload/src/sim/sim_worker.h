/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_SIM_WORKER_H__
#define __PNSO_SIM_WORKER_H__

#include "pnso_api.h"
#include "osal_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Request queue structures */
struct sim_q_request {
	struct pnso_service_request *svc_req;
	struct pnso_service_result *svc_res;
	completion_cb_t cb;
	void *cb_ctx;
	void *poll_ctx;
	sim_req_id_t id;
	bool end_of_batch;
	bool poll_mode;
	volatile bool is_proc_done;
};

struct sim_worker_ctx {
	int core_id;
	struct sim_q *req_q;
	osal_thread_t *worker;
	struct sim_session *sess;
};

pnso_error_t sim_sq_enqueue(int core_id,
			    struct pnso_service_request *svc_req,
			    struct pnso_service_result *svc_res,
			    completion_cb_t cb,
			    void *cb_ctx, void **poll_ctx,
			    bool flush);
pnso_error_t sim_sq_flush(int core_id,
			  completion_cb_t cb,
			  void *cb_ctx, void **poll_ctx);

/* Poll for completion of a particular request */
pnso_error_t pnso_sim_poll(void *poll_ctx);

/* Same as pnso_sim_poll, but keep looping until request is done. */
pnso_error_t pnso_sim_poll_wait(void *poll_ctx, int core_id);

/* Worker thread */
struct sim_worker_ctx *sim_get_worker_ctx(int core_id);
pnso_error_t sim_start_worker_thread(int core_id);
pnso_error_t sim_stop_worker_thread(int core_id);
bool sim_is_worker_running(int core_id);

pnso_error_t sim_init_req_pool(uint32_t max_reqs);
pnso_error_t sim_init_worker_pool(uint32_t max_q_depth);

int pnso_sim_run_worker_loop(void *opaque);

#ifdef __cplusplus
}
#endif

#endif  /* __PNSO_SIM_WORKER_H__ */
