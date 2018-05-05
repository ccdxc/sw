
#ifndef __PNSO_SIM_OSAL_H__
#define __PNSO_SIM_OSAL_H__

#ifndef PNSO_ASSERT
#include "assert.h"
#define PNSO_ASSERT(x)  assert(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct pnso_sim_worker_ctx {
	struct pnso_sim_q *req_q;
	struct pnso_sim_session *sess;
};

pnso_error_t pnso_sim_sq_enqueue(enum pnso_batch_request batch_req,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_t cb,
				 void *cb_ctx, void **poll_ctx);

/* Poll for completion of a particular request */
pnso_error_t pnso_sim_poll(void *poll_ctx);

/* Same as pnso_sim_poll, but keep looping until request is done. */
pnso_error_t pnso_sim_poll_wait(void *poll_ctx);

/* Alloc and free */
void *pnso_sim_alloc(size_t sz);
void pnso_sim_free(void *ptr);

struct pnso_sim_worker_ctx *pnso_sim_get_worker_ctx();

pnso_error_t pnso_sim_start_worker_thread();
void pnso_sim_stop_worker_thread();
bool pnso_sim_is_worker_running();

void pnso_sim_init_req_pool();

#ifdef __cplusplus
}
#endif

#endif  /* __PNSO_SIM_OSAL_H__ */
