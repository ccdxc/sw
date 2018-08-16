/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_xts.h"

pnso_error_t
xts_init_accelerator(const struct xts_init_params *init_params)
{
	return PNSO_OK;	/* EOPNOTSUPP */
}

void
xts_deinit_accelerator(void)
{
	/* EOPNOTSUPP */
}
