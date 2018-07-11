/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_LOGGER_H__
#define __PNSO_LOGGER_H__

#ifndef __KERNEL__
#include <syslog.h>
#endif

#include "pnso_global.h"
#include "osal_log.h"

#ifndef __KERNEL__
enum pnso_log_level {
	PNSO_LOG_LEVEL_EMERGENCY = LOG_EMERG,
	PNSO_LOG_LEVEL_ALERT,
	PNSO_LOG_LEVEL_CRITICAL,
	PNSO_LOG_LEVEL_ERROR,
	PNSO_LOG_LEVEL_WARNING,
	PNSO_LOG_LEVEL_NOTICE,
	PNSO_LOG_LEVEL_INFO,
	PNSO_LOG_LEVEL_DEBUG,
};
#endif

extern enum pnso_log_level g_pnso_log_level;

#ifndef __KERNEL__
#define PNSO_LOG(level, format, ...)					\
	do {								\
		if ((enum pnso_log_level) level <= g_pnso_log_level)	\
			pnso_log_msg(level, " %4d | %-30.30s | " format,\
				__LINE__, __func__,			\
				##__VA_ARGS__);				\
	} while (0)

#define PNSO_LOG_EMERG(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_EMERGENCY, fmt, ##__VA_ARGS__)
#define PNSO_LOG_ALERT(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_ALERT, fmt, ##__VA_ARGS__)
#define PNSO_LOG_CRITICAL(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define PNSO_LOG_ERROR(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define PNSO_LOG_WARN(fmt, ...)						\
	PNSO_LOG(PNSO_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define PNSO_LOG_NOTICE(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define PNSO_LOG_INFO(fmt, ...)						\
	PNSO_LOG(PNSO_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define PNSO_LOG_DEBUG(fmt, ...)					\
	PNSO_LOG(PNSO_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define PNSO_LOG_EMERG(fmt, ...)					\
	osal_log(KERN_EMERG fmt, ##__VA_ARGS__)
#define PNSO_LOG_ALERT(fmt, ...)					\
	osal_log(KERN_ALERT fmt, ##__VA_ARGS__)
#define PNSO_LOG_CRITICAL(fmt, ...)					\
	osal_log(KERN_CRIT fmt, ##__VA_ARGS__)
#define PNSO_LOG_ERROR(fmt, ...)					\
	osal_log(KERN_ERR fmt, ##__VA_ARGS__)
#define PNSO_LOG_WARN(fmt, ...)						\
	osal_log(KERN_WARNING fmt, ##__VA_ARGS__)
#define PNSO_LOG_NOTICE(fmt, ...)					\
	osal_log(KERN_NOTICE fmt, ##__VA_ARGS__)
#define PNSO_LOG_INFO(fmt, ...)						\
	osal_log(KERN_INFO fmt, ##__VA_ARGS__)
#define PNSO_LOG_DEBUG(fmt, ...)					\
	osal_log(KERN_DEBUG fmt, ##__VA_ARGS__)
#endif

/**
 * pnso_log_init() - initializes the log library.
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
pnso_error_t pnso_log_init(const bool log_console,
			 const enum pnso_log_level level,
			 const char *base_fname);

/**
 * pnso_log_deinit() - uninitializes the log library.
 *
 * Return Value:
 * 	PNSO_OK	- on success
 *	-EINVAL	- if log library is not initialized
 *
 */
pnso_error_t pnso_log_deinit(void);

/**
 * pnso_log_msg() - routes the log message to console or file.
 *
 * Typically, callers need not invoke this function directly, instead to use
 * the wrapper macros (ex: PNSO_LOG_ERROR(), etc.).
 *
 * Return Value:
 *	None
 *
 */
void pnso_log_msg(enum pnso_log_level level, const char *format, ...);

#endif	/* __PNSO_LOGGER_H__ */
