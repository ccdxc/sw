
#ifndef __OFFLOADER_SIM_H__
#define __OFFLOADER_SIM_H__

#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include/pnso_sim_api.h"

typedef uint16_t pnso_sim_req_id_t;

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


void pnso_sim_dequeue_req(uint32_t sess_id,
			  struct pnso_sim_q_request *entry);
void pnso_sim_start_worker_thread(uint32_t sess_id);
void pnso_sim_stop_worker_thread(uint32_t sess_id);

int pnso_sim_init(uint32_t sess_count, uint8_t * scratch,
		  uint32_t scratch_sz);
pnso_error_t pnso_sim_execute_request(uint32_t sess_id,
				      struct pnso_service_request
				      *svc_req,
				      struct pnso_service_result
				      *svc_res, completion_t cb,
				      void *cb_ctx);
pnso_error_t pnso_sim_submit_request(uint32_t sess_id,
				     enum pnso_batch_request
				     batch_req,
				     struct pnso_service_request
				     *svc_req,
				     struct pnso_service_result
				     *svc_res, completion_t cb,
				     void *cb_ctx,
				     void **poll_ctx);

/* Return number of requests processed since last poll */
bool pnso_sim_poll(uint32_t sess_id, void *poll_ctx);

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
