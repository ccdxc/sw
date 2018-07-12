/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal_logger.h"

static void
exercise_logger_to_console(void)
{
	osal_log_init(true, OSAL_LOG_LEVEL_DEBUG, "");

	OSAL_LOG_EMERG("Emergency");
	OSAL_LOG_ALERT("Alert");
	OSAL_LOG_CRITICAL("Critical");
	OSAL_LOG_ERROR("Error");
	OSAL_LOG_WARN("Warning");
	OSAL_LOG_NOTICE("Notice");
	OSAL_LOG_INFO("Info");
	OSAL_LOG_DEBUG("Debug");

	osal_log_deinit();
}

int
main(void)
{
	exercise_logger_to_console();

	return 0;
}
