/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_INIT_H__
#define __PNSO_INIT_H__

#include "sonic_dev.h"

#define PNSO_NOMINAL_PAGE_SIZE	4096
#define PNSO_NOMINAL_NUM_BUFS	8
#define PNSO_NOMINAL_BUF_SIZE	(PNSO_NOMINAL_NUM_BUFS * PNSO_NOMINAL_PAGE_SIZE)
#define PNSO_MAX_BUF_SIZE	PNSO_NOMINAL_BUF_SIZE


/**
 * struct pc_res_init_params - used to initialize per core resources.
 *
 */
struct pc_res_init_params {
	struct pnso_init_params pnso_init;
	uint32_t rmem_total_pages;
	uint32_t rmem_page_size;
	uint32_t max_seq_sq_descs;
};


void pnso_deinit(void);
pnso_error_t pnso_pc_res_init(struct pc_res_init_params *pc_init,
			      struct per_core_resource *pc_res);
void pnso_pc_res_deinit(struct per_core_resource *pc_res);

#endif /* __PNSO_INIT_H__ */
