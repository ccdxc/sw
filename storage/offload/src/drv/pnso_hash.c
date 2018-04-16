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

static pnso_error_t
hash_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
hash_chain(struct chain_entry *centry)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
hash_schedule(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
hash_poll(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
hash_read_status(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
hash_write_result(struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static void
hash_teardown(const struct service_info *svc_info)
{
	/* -EOPNOTSUPP */
}

struct service_ops hash_ops = {
	.setup = hash_setup,
	.chain = hash_chain,
	.schedule = hash_schedule,
	.poll = hash_poll,
	.read_status = hash_read_status,
	.write_result = hash_write_result,
	.teardown = hash_teardown
};
