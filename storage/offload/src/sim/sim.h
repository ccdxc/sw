/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_SIM_H__
#define __PNSO_SIM_H__

#include "osal_assert.h"
#include "osal_stdtypes.h"
#include "pnso_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PNSO_MAX_BUFFER_LEN (64 * 1024)

#define SIM_MAX_SESSIONS 24
#define SIM_MAX_Q_DEPTH 1024
#define SIM_DEFAULT_SQ_DEPTH 16
#define SIM_KEY_STORE_SZ (64 * 1024)
#define SIM_DEFAULT_REQ_COUNT (16 * 1024)
#define SIM_MAX_CP_HEADER_FMTS 8

typedef uint16_t sim_req_id_t;

void pnso_sim_finit(void);
pnso_error_t pnso_sim_thread_init(int core_id);
void pnso_sim_thread_finit(int core_id);

/* Key store */
pnso_error_t sim_get_key_desc_idx(void **key1,
				  void **key2,
				  uint32_t *key_size,
				  uint32_t key_idx);
pnso_error_t sim_key_store_init(uint32_t size);
void sim_key_store_finit(void);

/* pnso_api functions */
#ifdef PNSO_DUAL_MODE
#define SIM_API(fn) pnso_sim_##fn
pnso_error_t pnso_sim_init(struct pnso_init_params *init_params);
pnso_error_t pnso_sim_set_key_desc_idx(const void *key1,
				       const void *key2,
				       uint32_t key_size, uint32_t key_idx);
pnso_error_t pnso_sim_register_compression_header_format(
		struct pnso_compression_header_format *cp_hdr_fmt,
		uint16_t hdr_fmt_idx);
pnso_error_t pnso_sim_add_compression_algo_mapping(
		enum pnso_compression_type pnso_algo,
		uint32_t header_algo);
pnso_error_t pnso_sim_add_to_batch(struct pnso_service_request *svc_req,
				   struct pnso_service_result *svc_res);
pnso_error_t pnso_sim_flush_batch(completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx);
pnso_error_t pnso_sim_submit_request(struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *poll_fn,
				 void **poll_ctx);
#else
#define SIM_API(fn) pnso_##fn
#endif

#ifdef __cplusplus
}
#endif

#endif				/* __PNSO_SIM_H__ */
