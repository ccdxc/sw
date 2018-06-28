/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdio.h>
#include <string.h>

#include "storage/offload/src/common/pnso_logger.h"

static void
exercise_logger_to_console(void)
{
	pnso_log_init(true, 7, "");

	PNSO_LOG_EMERG(PNSO_OK, "Emergency");
	PNSO_LOG_ALERT(PNSO_OK, "Alert");
	PNSO_LOG_CRITICAL(PNSO_OK, "Critical");
	PNSO_LOG_ERROR(PNSO_OK, "Error");
	PNSO_LOG_WARN(PNSO_OK, "Warning");
	PNSO_LOG_NOTICE(PNSO_OK, "Notice");
	PNSO_LOG_INFO(PNSO_OK, "Info");
	PNSO_LOG_DEBUG(PNSO_OK, "Debug");

	pnso_log_deinit();
}

int
main(void)
{
	exercise_logger_to_console();

	return 0;
}
