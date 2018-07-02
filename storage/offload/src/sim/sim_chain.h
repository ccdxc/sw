/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_SIM_CHAIN_H__
#define __PNSO_SIM_CHAIN_H__

#include "osal_stdtypes.h"
#include "pnso_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sim_scratch_region {
	uint8_t *cmd;		/* scratch for commands */
	uint8_t *data[2];	/* for bank switching between input/output */
	uint32_t data_sz;	/* size of each data region */
};

struct sim_svc_ctx;
typedef pnso_error_t(*svc_exec_func_t) (struct sim_svc_ctx *ctx,
					void *opaque);

struct sim_session {
	bool is_valid;
	uint32_t block_sz;
	uint32_t q_depth;
	struct sim_scratch_region scratch;
	svc_exec_func_t funcs[PNSO_SVC_TYPE_MAX];
};

struct sim_svc_ctx {
	struct sim_session *sess;
	struct sim_svc_ctx *prev_ctx;
	struct pnso_service cmd;
	struct pnso_service_status status;
	uint32_t is_first:1,
		 is_last:1,
		 is_chksum_present:1,
		 rsvrd:29;
	struct pnso_flat_buffer input;
	struct pnso_flat_buffer output;
};

struct sim_cp_header_format {
	uint32_t fmt_idx;
	uint32_t total_hdr_sz;
	uint16_t flags;
	uint16_t type_mask;
	enum pnso_compression_type pnso_algo;
	uint8_t *static_hdr;
	struct pnso_compression_header_format fmt;
};

/* Full definition in sim_worker.h */
struct sim_worker_ctx;

void sim_init_globals(void);
pnso_error_t sim_init_session(int core_id);
void sim_finit_session(int core_id);

void sim_set_algo_mapping(enum pnso_compression_type pnso_algo,
			  uint32_t hdr_algo);
struct sim_cp_header_format *sim_lookup_hdr_format(uint32_t hdr_fmt_idx,
						   bool alloc);

pnso_error_t sim_execute_request(struct sim_worker_ctx *worker_ctx,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb, void *cb_ctx);

#ifdef __cplusplus
}
#endif

#endif				/* __PNSO_SIM_CHAIN_H__ */
