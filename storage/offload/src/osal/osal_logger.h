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
#else
#include <linux/kernel.h>
#endif

#include "osal_stdtypes.h"
#include "pnso_api.h"

#ifdef __cplusplus
extern "C" {
#endif

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
	OSAL_LOG_LEVEL_NONE = -1,	/* levels handled in macros */
};
#endif
extern enum osal_log_level g_osal_log_level;

#ifndef __KERNEL__
#define USPACE_LOG(log_fp, level, format, ...)				\
	do {								\
		if ((enum osal_log_level) level <= g_osal_log_level)	\
			osal_log_msg(log_fp, level,			\
					" %4d | %-30.30s | " format,	\
					__LINE__, __func__,		\
					##__VA_ARGS__);			\
	} while (0)

#define OSAL_LOG_EMERG(fmt, ...)					\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_EMERGENCY, fmt, ##__VA_ARGS__)
#define OSAL_LOG_ALERT(fmt, ...)					\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_ALERT, fmt, ##__VA_ARGS__)
#define OSAL_LOG_CRITICAL(fmt, ...)					\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define OSAL_LOG_ERROR(fmt, ...)					\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define OSAL_LOG_WARN(fmt, ...)						\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define OSAL_LOG_NOTICE(fmt, ...)					\
	USPACE_LOG(stderr, OSAL_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define OSAL_LOG_INFO(fmt, ...)						\
	USPACE_LOG(stdout, OSAL_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define OSAL_LOG_DEBUG(fmt, ...)					\
	USPACE_LOG(stdout, OSAL_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define OSAL_LOG printf
#else
#define KSPACE_LOG(...) printk(__VA_ARGS__)

#define OSAL_LOG_EMERG(fmt, ...)					\
	KSPACE_LOG(KERN_EMERG "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_ALERT(fmt, ...)					\
	KSPACE_LOG(KERN_ALERT "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_CRITICAL(fmt, ...)					\
	KSPACE_LOG(KERN_CRIT "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_ERROR(fmt, ...)					\
	KSPACE_LOG(KERN_ERR "%30s:%d:> " fmt, __func__, __LINE__,		\
			##__VA_ARGS__)
#define OSAL_LOG_WARN(fmt, ...)						\
	KSPACE_LOG(KERN_WARNING "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_NOTICE(fmt, ...)					\
	KSPACE_LOG(KERN_NOTICE "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_INFO(fmt, ...)						\
	KSPACE_LOG(KERN_INFO "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG_DEBUG(fmt, ...)					\
	KSPACE_LOG(KERN_DEBUG "%30s:%d:> " fmt, __func__, __LINE__,	\
			##__VA_ARGS__)
#define OSAL_LOG printk
#endif

/**
 * osal_log_init() - initializes the log library.
 * @level:	[in]	specifies one of the log levels.
 *			(i.e. error or debug or emergency, etc.).
 *
 * Invocation of This function for kernel target is a no-op.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EEXIST	- if log library is already initialized
 *	-EINVAL	- if invalid log level is specified
 *
 */
pnso_error_t osal_log_init(const enum osal_log_level level);

/**
 * osal_log_deinit() - uninitializes the log library.
 *
 * Invocation of This function for kernel target is a no-op.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- if log library is not initialized
 *
 */
pnso_error_t osal_log_deinit(void);

/**
 * osal_log_msg() - routes the log message to the speficied stream.
 * @fp:		[in]	specifies stderr/stdout stream.
 * @level:	[in]	specifies the log level.
 * @format:	[in]	specifies the format specifier for the variadic args.
 *
 * Invocation of This function for kernel target is a no-op.
 *
 * Typically, callers need not invoke this function directly, instead of using
 * the wrapper macros (ex: OSAL_LOG_ERROR(), etc.).
 *
 * Return Value:
 *	None
 *
 */
void osal_log_msg(const void *fp, enum osal_log_level level,
		const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif	/* __OSAL_LOGGER_H__ */
