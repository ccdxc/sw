/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal_thread.h"
#include "osal_logger.h"
#include "osal_sys.h"
#include "osal_errno.h"

#include "pnso_api.h"

pnso_error_t
osal_log_init(const enum osal_log_level level)
{
	/* a dummy stub */
	return PNSO_OK;
}

pnso_error_t
osal_log_deinit(void)
{
	/* a dummy stub */
	return PNSO_OK;
}

void
osal_log_msg(const void *fp, enum osal_log_level level,
		const char *format, ...)
{
	/* a dummy stub */
}
