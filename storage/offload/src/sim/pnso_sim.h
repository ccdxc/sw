
#ifndef __PNSO_SIM_H__
#define __PNSO_SIM_H__

#include <string.h>
#include <stdbool.h>
#include "pnso_sim_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PNSO_SIM_MAX_SESSIONS 24
#define PNSO_MAX_SERVICE_CHAIN 8
#define PNSO_DEFAULT_BLOCK_SZ (4 * 1024)
#define PNSO_MAX_BUFFER_LEN (64 * 1024)
#define PNSO_SIM_DEFAULT_SQ_DEPTH 16

typedef uint16_t pnso_sim_req_id_t;

extern struct pnso_init_params g_init_params;

/* defined in pnso_sim_osal.h */
struct pnso_sim_worker_ctx;

/* Request queue structures */
struct pnso_sim_q_request {
	struct pnso_service_request *svc_req;
	struct pnso_service_result *svc_res;
	completion_t cb;
	void *cb_ctx;
	void *poll_ctx;
	pnso_sim_req_id_t id;
	bool end_of_batch;
	bool poll_mode;
	volatile bool is_proc_done;
};


void pnso_sim_finit();
pnso_error_t pnso_sim_thread_init();
void pnso_sim_thread_finit();

pnso_error_t pnso_sim_execute_request(struct pnso_sim_worker_ctx *worker_ctx,
				      struct pnso_service_request *svc_req,
				      struct pnso_service_result *svc_res,
				      completion_t cb, void *cb_ctx);
pnso_error_t pnso_sim_submit_request(uint32_t sess_id,
				     enum pnso_batch_request
				     batch_req,
				     struct pnso_service_request
				     *svc_req,
				     struct pnso_service_result
				     *svc_res, completion_t cb,
				     void *cb_ctx,
				     void **poll_ctx);

/* Key store */
pnso_error_t pnso_sim_get_key_desc_idx(void **key1,
				       void **key2,
				       uint32_t * key_size,
				       uint32_t key_idx);
pnso_error_t pnso_sim_key_store_init(uint8_t * slab,
				     uint32_t slab_size);

struct slab_desc;
struct slab_desc *pnso_sim_slab_init(uint8_t * data,
				     uint32_t size);
void *pnso_sim_slab_alloc(struct slab_desc *slab, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif				/* __OFFLOADER_SIM_H__ */
