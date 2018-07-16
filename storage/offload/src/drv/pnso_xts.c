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
xts_start_accelerator(const struct xts_init_params *init_params)
{
	return -EOPNOTSUPP;
}

void
xts_stop_accelerator(void)
{
	/* -EOPNOTSUPP */
}
