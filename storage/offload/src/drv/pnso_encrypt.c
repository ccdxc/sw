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

static pnso_error_t
encrypt_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
encrypt_chain(struct chain_entry *centry)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
encrypt_schedule(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
encrypt_poll(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
encrypt_read_status(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
encrypt_write_result(struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static void
encrypt_teardown(const struct service_info *svc_info)
{
	/* -EOPNOTSUPP */
}

struct service_ops encrypt_ops = {
	.setup = encrypt_setup,
	.chain = encrypt_chain,
	.schedule = encrypt_schedule,
	.poll = encrypt_poll,
	.read_status = encrypt_read_status,
	.write_result = encrypt_write_result,
	.teardown = encrypt_teardown,
};
