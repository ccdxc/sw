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

	PNSO_LOG_EMERG("Emergency");
	PNSO_LOG_ALERT("Alert");
	PNSO_LOG_CRITICAL("Critical");
	PNSO_LOG_ERROR("Error");
	PNSO_LOG_WARN("Warning");
	PNSO_LOG_NOTICE("Notice");
	PNSO_LOG_INFO("Info");
	PNSO_LOG_DEBUG("Debug");

	pnso_log_deinit();
}

int
main(void)
{
	exercise_logger_to_console();

	return 0;
}
