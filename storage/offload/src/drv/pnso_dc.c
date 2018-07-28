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
decompress_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_schedule(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_poll(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_read_status(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decompress_write_result(struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static void
decompress_teardown(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

struct service_ops dc_ops = {
	.setup = decompress_setup,
	.chain = decompress_chain,
	.schedule = decompress_schedule,
	.poll = decompress_poll,
	.read_status = decompress_read_status,
	.write_result = decompress_write_result,
	.teardown = decompress_teardown
};
