

void pnso_sim_init_queues();

pnso_error_t pnso_sim_sq_enqueue(uint32_t sess_id,
				 enum pnso_batch_request batch_req,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_t cb,
				 void *cb_ctx, void **poll_ctx);
