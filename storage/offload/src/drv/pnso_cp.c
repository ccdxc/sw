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
compress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return EOPNOTSUPP;
}

static pnso_error_t
compress_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

static pnso_error_t
compress_schedule(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
compress_poll(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
compress_read_status(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
compress_write_result(struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static void
compress_teardown(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

struct service_ops cp_ops = {
	.setup = compress_setup,
	.chain = compress_chain,
	.schedule = compress_schedule,
	.poll = compress_poll,
	.read_status = compress_read_status,
	.write_result = compress_write_result,
	.teardown = compress_teardown
};
