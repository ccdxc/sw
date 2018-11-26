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

enum osal_log_level g_osal_log_level = OSAL_LOG_LEVEL_WARNING;
char g_osal_log_prefix[PREFIX_STR_LEN] = "";

pnso_error_t
osal_log_init(const enum osal_log_level level, const char* prefix)
{
	if ((level < OSAL_LOG_LEVEL_EMERGENCY) ||
			(level > OSAL_LOG_LEVEL_DEBUG))
		return -EINVAL;

	g_osal_log_level = level;
	strncpy(g_osal_log_prefix, prefix, PREFIX_STR_LEN);
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
