/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include "../pnso_logger.h"	/* TODO: use BUILD to fix the path! */
#else
#include "pnso_logger.h"
#endif


static void
exercise_logger_to_console(void)
{
#ifndef __KERNEL__
	pnso_log_init(true, 7, "");
#endif

	PNSO_LOG_EMERG("Emergency");
	PNSO_LOG_ALERT("Alert");
	PNSO_LOG_CRITICAL("Critical");
	PNSO_LOG_ERROR("Error");
	PNSO_LOG_WARN("Warning");
	PNSO_LOG_NOTICE("Notice");
	PNSO_LOG_INFO("Info");
	PNSO_LOG_DEBUG("Debug");

#ifndef __KERNEL__
	pnso_log_deinit();
#endif
}

int
main(void)
{
	exercise_logger_to_console();

	return 0;
}
