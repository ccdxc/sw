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
#define SIM_DEFAULT_REQ_COUNT (32 * 1024)
#define SIM_MAX_CP_HEADER_FMTS 8

typedef uint16_t sim_req_id_t;

extern struct pnso_init_params g_init_params;

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


#ifdef __cplusplus
}
#endif

#endif				/* __PNSO_SIM_H__ */
