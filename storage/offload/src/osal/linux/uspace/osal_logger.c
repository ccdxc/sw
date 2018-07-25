/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <unistd.h>
#include <stdarg.h>
#endif
#include "osal_assert.h"
#include "osal_thread.h"
#include "osal_logger.h"
#include "osal_sys.h"
#include "osal_errno.h"

#include "pnso_api.h"

#define MAX_LOG_BUF_LEN 1024
#define MAX_LOG_TIME_STR 128
#define MAX_LOG_FNAME_STR 256

enum osal_log_level g_osal_log_level = OSAL_LOG_LEVEL_ERROR;
bool g_osal_log_enabled;
static pid_t prog_id;

static const char * const level_name[] = {
	[OSAL_LOG_LEVEL_EMERGENCY] = "EMERG",
	[OSAL_LOG_LEVEL_ALERT] = "ALERT",
	[OSAL_LOG_LEVEL_CRITICAL] = "CRITICAL",
	[OSAL_LOG_LEVEL_ERROR] = "ERROR",
	[OSAL_LOG_LEVEL_WARNING] = "WARN ",	/* extra space for formatting */
	[OSAL_LOG_LEVEL_NOTICE] = "NOTICE",
	[OSAL_LOG_LEVEL_INFO] = "INFO ",	/*      -- ditto --           */
	[OSAL_LOG_LEVEL_DEBUG] = "DEBUG"
};

static inline const char *
get_level_name(enum osal_log_level level)
{
	assert((level >= OSAL_LOG_LEVEL_EMERGENCY) &&
			(level <= OSAL_LOG_LEVEL_DEBUG));

	return level_name[level];
}

pnso_error_t
osal_log_init(const enum osal_log_level level)
{
	pnso_error_t err = PNSO_OK;

	if (g_osal_log_enabled)
		return -EEXIST;

	if ((level < OSAL_LOG_LEVEL_EMERGENCY) ||
			(level > OSAL_LOG_LEVEL_DEBUG))
		return -EINVAL;

	g_osal_log_enabled = true;
	g_osal_log_level = level;
	prog_id = getpid();

	return err;
}

pnso_error_t
osal_log_deinit(void)
{
	pnso_error_t err = -EINVAL;

	if (g_osal_log_enabled) {
		g_osal_log_enabled = false;

		return PNSO_OK;
	}

	return err;
}

void
osal_log_msg(const void *fp, enum osal_log_level level,
		const char *format, ...)
{
	char time_buf[MAX_LOG_TIME_STR];
	char hdr_buf[MAX_LOG_BUF_LEN];
	struct tm time_stamp;
	time_t time_now;
	va_list args;
	FILE *log_fp = (FILE *) fp;

	if (!g_osal_log_enabled)
		return;

	time_now = time(NULL);
	localtime_r(&time_now, &time_stamp);

	snprintf(time_buf, sizeof(time_buf),
		 "%4d-%02d-%02d %02d:%02d:%02d",
		 1900 + time_stamp.tm_year,
		 (time_stamp.tm_mon + 1),
		 time_stamp.tm_mday,
		 time_stamp.tm_hour, time_stamp.tm_min, time_stamp.tm_sec);

	snprintf(hdr_buf, sizeof(hdr_buf),
			 "%u | %u | %.5s", prog_id, osal_get_coreid(),
			 get_level_name(level));

	fprintf(log_fp, "%s | %s |", time_buf, hdr_buf);
	va_start(args, format);
	vfprintf(log_fp, format, args);
	va_end(args);
	fprintf(log_fp, "\n");
	fflush(log_fp);
}
