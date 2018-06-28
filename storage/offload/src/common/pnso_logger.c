/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include "osal_sys.h"
#include "pnso_logger.h"

#define MAX_LOG_BUF_LEN 1024
#define MAX_LOG_TIME_STR 128
#define MAX_LOG_FNAME_STR 256

enum pnso_log_level g_pnso_log_level = PNSO_LOG_LEVEL_ERROR;
bool g_pnso_log_enabled;
bool g_pnso_log_console;
FILE *g_pnso_log_fp;

static const char *pnso_log_path = "/var/log";
static char g_pnso_log_file[MAX_LOG_FNAME_STR];
static pid_t prog_id;

static const char * const level_name[] = {
	[PNSO_LOG_LEVEL_EMERGENCY] = "EMERG",
	[PNSO_LOG_LEVEL_ALERT] = "ALERT",
	[PNSO_LOG_LEVEL_CRITICAL] = "CRITICAL",
	[PNSO_LOG_LEVEL_ERROR] = "ERROR",
	[PNSO_LOG_LEVEL_WARNING] = "WARN ",	/* extra space for formatting */
	[PNSO_LOG_LEVEL_NOTICE] = "NOTICE",
	[PNSO_LOG_LEVEL_INFO] = "INFO ",	/*      -- ditto --           */
	[PNSO_LOG_LEVEL_DEBUG] = "DEBUG"
};

static inline const char *
get_level_name(enum pnso_log_level level)
{
	assert((level >= PNSO_LOG_LEVEL_EMERGENCY) &&
			(level <= PNSO_LOG_LEVEL_DEBUG));

	return level_name[level];
}

pnso_error_t
pnso_log_init(const bool log_console, const enum pnso_log_level level,
	      const char *base_fname)
{
	pnso_error_t err = PNSO_OK;

	if (g_pnso_log_enabled)
		return -EEXIST;

	if ((level < PNSO_LOG_LEVEL_EMERGENCY) ||
			(level > PNSO_LOG_LEVEL_DEBUG))
		return -EINVAL;

	g_pnso_log_enabled = true;
	g_pnso_log_level = level;
	prog_id = getpid();
	g_pnso_log_console = log_console;

	if (log_console) {
		g_pnso_log_fp = stdout;
	} else {
		snprintf(g_pnso_log_file, sizeof(g_pnso_log_file), "%s/%s.log",
			 pnso_log_path, base_fname);

		g_pnso_log_fp = fopen(g_pnso_log_file, "a");
		if (!g_pnso_log_fp)
			err = errno;
		else
			setvbuf(g_pnso_log_fp, NULL, _IONBF, 0);
	}

	return err;
}

pnso_error_t
pnso_log_deinit(void)
{
	pnso_error_t err = -EINVAL;

	if (g_pnso_log_enabled) {
		if (stdout != g_pnso_log_fp)
			fclose(g_pnso_log_fp);

		g_pnso_log_enabled = false;

		return PNSO_OK;
	}

	return err;
}

void
pnso_log_msg(enum pnso_log_level level, pnso_error_t err,
	     const char *format, ...)
{
	char time_buf[MAX_LOG_TIME_STR];
	char hdr_buf[MAX_LOG_BUF_LEN];
	struct tm time_stamp;
	time_t time_now;
	va_list args;

	assert(g_pnso_log_fp);

	time_now = time(NULL);
	localtime_r(&time_now, &time_stamp);

	snprintf(time_buf, sizeof(time_buf),
		 "%4d-%02d-%02d %02d:%02d:%02d",
		 1900 + time_stamp.tm_year,
		 (time_stamp.tm_mon + 1),
		 time_stamp.tm_mday,
		 time_stamp.tm_hour, time_stamp.tm_min, time_stamp.tm_sec);

	if (err)
		snprintf(hdr_buf, sizeof(hdr_buf), "%u | %u | %.5s | %3d",
			 prog_id, osal_get_coreid(),
			 get_level_name(level), err);
	else
		snprintf(hdr_buf, sizeof(hdr_buf),
			 "%u | %u | %.5s | ---", prog_id, osal_get_coreid(),
			 get_level_name(level));

	fprintf(g_pnso_log_fp, "%s | %s |", time_buf, hdr_buf);
	va_start(args, format);
	vfprintf(g_pnso_log_fp, format, args);
	va_end(args);
	fprintf(g_pnso_log_fp, "\n");
}
