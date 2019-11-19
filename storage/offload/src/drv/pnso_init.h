/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_INIT_H__
#define __PNSO_INIT_H__

#include "sonic_dev.h"
#include "pnso_cpdc.h"
#include "pnso_crypto.h"

#define INTERM_BUF_NOMINAL_PAGE_SIZE	4096
#define INTERM_BUF_MIN_SIZE		8192
#define INTERM_BUF_MAX_SIZE		65536

#define INTERM_BUF_NOMINAL_SIZE()	sonic_interm_buf_size_get()

#define INTERM_BUF_NOMINAL_NUM_BUFS()					\
	(INTERM_BUF_NOMINAL_SIZE() / INTERM_BUF_NOMINAL_PAGE_SIZE)
#define INTERM_BUF_MAX_NUM_NOMINAL_BUFS()				\
	(INTERM_BUF_MAX_SIZE / INTERM_BUF_NOMINAL_SIZE())
#define INTERM_BUF_MAX_NUM_BUFS						\
	(INTERM_BUF_MAX_SIZE / INTERM_BUF_MIN_SIZE)

/*
 * (3 for 32k buffers * 2 for src+dst buffers)
 */
#define MAX_SGLS_PER_REQ 6
#define MAX_CPDC_SGLS_PER_REQ MAX_SGLS_PER_REQ
#define MAX_CRYPTO_SGLS_PER_REQ MAX_SGLS_PER_REQ
/*
 * 2 (hash + chksum)
 */
#define MAX_CPDC_VEC_PER_REQ 2
#define MAX_CPDC_SGL_VEC_PER_REQ MAX_CPDC_VEC_PER_REQ
#define MAX_CPDC_DESC_VEC_PER_REQ MAX_CPDC_VEC_PER_REQ

#define MAX_CRYPTO_VEC_PER_REQ 1
#define MAX_CRYPTO_SGL_VEC_PER_REQ MAX_CRYPTO_VEC_PER_REQ
#define MAX_CRYPTO_DESC_VEC_PER_REQ MAX_CRYPTO_VEC_PER_REQ

/*
 * 3 for nominal number of chained services (or number of simultaneous chains)
 */
#define MAX_SHARED_STATUS_PER_REQ 3

/**
 * struct pc_res_init_params - used to initialize per core resources.
 *
 */
struct pc_res_init_params {
	struct pnso_init_params pnso_init;
	uint32_t rmem_page_size;
	uint32_t max_seq_sq_descs;
};

/**
 * struct shared_status_desc - allow for sharing of status descriptor space
 */
union shared_status_desc {
	struct cpdc_status_desc cpdc_status;
	struct crypto_status_desc crypto_status;
} __attribute__((__packed__));

extern uint64_t pad_buffer;

void pnso_deinit(void);
void pnso_lif_reset(struct lif *lif);
void pnso_set_log_level(int level);

#endif /* __PNSO_INIT_H__ */
