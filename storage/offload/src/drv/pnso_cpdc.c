/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	return -EOPNOTSUPP;
}

void
cpdc_common_teardown(void *desc)
{
	/* -EOPNOTSUPP */
}

pnso_error_t
cpdc_start_accelerator(const struct cpdc_init_params *init_params)
{
	return -EOPNOTSUPP;
}

void
cpdc_stop_accelerator(void)
{
	/* -EOPNOTSUPP */
}
