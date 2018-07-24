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
chksum_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_schedule(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_poll(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_read_status(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
chksum_write_result(struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static void
chksum_teardown(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

struct service_ops chksum_ops = {
	.setup = chksum_setup,
	.chain = chksum_chain,
	.schedule = chksum_schedule,
	.poll = chksum_poll,
	.read_status = chksum_read_status,
	.write_result = chksum_write_result,
	.teardown = chksum_teardown
};
