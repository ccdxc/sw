/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __OSAL_LOGGER_H__
#define __OSAL_LOGGER_H__

#ifndef __KERNEL__
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#define OSAL_OUT_FILE stdout
#define OSAL_ERR_FILE stderr
#define osal_logf(f, ...) 						\
	do {								\
		if (f)							\
			fprintf(f, __VA_ARGS__);			\
	} while (0)
#else
#include <linux/kernel.h>
#define OSAL_OUT_FILE NULL
#define OSAL_ERR_FILE NULL
#define osal_logf(f, ...) printk(__VA_ARGS__)
#endif

#define OSAL_LOG(...) osal_logf(OSAL_OUT_FILE, __VA_ARGS__)
#define OSAL_ERR(...) osal_logf(OSAL_ERR_FILE, __VA_ARGS__)

#include "osal_stdtypes.h"

#include "pnso_api.h"

#ifndef __KERNEL__
enum osal_log_level {
	OSAL_LOG_LEVEL_EMERGENCY = LOG_EMERG,	/* from syslog.h */
	OSAL_LOG_LEVEL_ALERT,
	OSAL_LOG_LEVEL_CRITICAL,
	OSAL_LOG_LEVEL_ERROR,
	OSAL_LOG_LEVEL_WARNING,
	OSAL_LOG_LEVEL_NOTICE,
	OSAL_LOG_LEVEL_INFO,
	OSAL_LOG_LEVEL_DEBUG,
};
#else
enum osal_log_level {
	OSAL_LOG_LEVEL_NONE = -1	/* for kernel use the macros directly */
};
#endif

extern enum osal_log_level g_osal_log_level;

#ifndef __KERNEL__
#define PNSO_LOG(level, format, ...)					\
	do {								\
		if ((enum osal_log_level) level <= g_osal_log_level)	\
			osal_log_msg(level, " %4d | %-30.30s | " format,\
				__LINE__, __func__,			\
				##__VA_ARGS__);				\
	} while (0)

#define OSAL_LOG_EMERG(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_EMERGENCY, fmt, ##__VA_ARGS__)
#define OSAL_LOG_ALERT(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_ALERT, fmt, ##__VA_ARGS__)
#define OSAL_LOG_CRITICAL(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define OSAL_LOG_ERROR(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define OSAL_LOG_WARN(fmt, ...)						\
	PNSO_LOG(OSAL_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define OSAL_LOG_NOTICE(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define OSAL_LOG_INFO(fmt, ...)						\
	PNSO_LOG(OSAL_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define OSAL_LOG_DEBUG(fmt, ...)					\
	PNSO_LOG(OSAL_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define OSAL_LOG_EMERG(fmt, ...)					\
	OSAL_LOG(KERN_EMERG fmt, ##__VA_ARGS__)
#define OSAL_LOG_ALERT(fmt, ...)					\
	OSAL_LOG(KERN_ALERT fmt, ##__VA_ARGS__)
#define OSAL_LOG_CRITICAL(fmt, ...)					\
	OSAL_LOG(KERN_CRIT fmt, ##__VA_ARGS__)
#define OSAL_LOG_ERROR(fmt, ...)					\
	OSAL_LOG(KERN_ERR fmt, ##__VA_ARGS__)
#define OSAL_LOG_WARN(fmt, ...)						\
	OSAL_LOG(KERN_WARNING fmt, ##__VA_ARGS__)
#define OSAL_LOG_NOTICE(fmt, ...)					\
	OSAL_LOG(KERN_NOTICE fmt, ##__VA_ARGS__)
#define OSAL_LOG_INFO(fmt, ...)						\
	OSAL_LOG(KERN_INFO fmt, ##__VA_ARGS__)
#define OSAL_LOG_DEBUG(fmt, ...)					\
	OSAL_LOG(KERN_DEBUG fmt, ##__VA_ARGS__)
#endif

/**
 * osal_log_init() - initializes the log library.
 * @log_console:	[in]	specifies a flag to log to file or console.
 * @level:		[in]	specifies one of the log levels.
 * 				(i.e. error or debug or emergency, etc.).
 * @base_fname:		[in]	specifies the name of the file to where log
 * 				messages are routed.
 *
 * Return Value:
 * 	PNSO_OK	- on success
 *	-EEXIST	- if log library is already initialized
 *	-EINVAL	- if invalid log level is specified
 *
 */
pnso_error_t osal_log_init(const bool log_console,
			 const enum osal_log_level level,
			 const char *base_fname);

/**
 * osal_log_deinit() - uninitializes the log library.
 *
 * Return Value:
 * 	PNSO_OK	- on success
 *	-EINVAL	- if log library is not initialized
 *
 */
pnso_error_t osal_log_deinit(void);

/**
 * osal_log_msg() - routes the log message to console or file.
 *
 * Typically, callers need not invoke this function directly, instead of using
 * the wrapper macros (ex: OSAL_LOG_ERROR(), etc.).
 *
 * Return Value:
 *	None
 *
 */
void osal_log_msg(enum osal_log_level level, const char *format, ...);

#endif	/* __OSAL_LOGGER_H__ */
