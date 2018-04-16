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
decrypt_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
decrypt_chain(struct chain_entry *centry)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
decrypt_schedule(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
decrypt_poll(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
decrypt_read_status(const struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static pnso_error_t
decrypt_write_result(struct service_info *svc_info)
{
	return -EOPNOTSUPP;
}

static void
decrypt_teardown(const struct service_info *svc_info)
{
	/* -EOPNOTSUPP */
}

struct service_ops decrypt_ops = {
	.setup = decrypt_setup,
	.chain = decrypt_chain,
	.schedule = decrypt_schedule,
	.poll = decrypt_poll,
	.read_status = decrypt_read_status,
	.write_result = decrypt_write_result,
	.teardown = decrypt_teardown
};
