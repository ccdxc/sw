/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_xts.h"
#include "pnso_xts_cmn.h"

pnso_error_t
xts_common_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

void
xts_common_teardown(void *desc)
{
	/* EOPNOTSUPP */
}

void
xts_pprint_desc(const struct xts_desc *desc)
{
	/* EOPNOTSUPP */
}
