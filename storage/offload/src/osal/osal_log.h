/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __OSAL_LOG_H__
#define __OSAL_LOG_H__

#ifndef __KERNEL__
#include <stdio.h>
#define OSAL_OUT_FILE stdout
#define OSAL_ERR_FILE stderr
#define osal_logf(f, ...) do { if (f) fprintf(f, __VA_ARGS__); } while (0)
#else
#include <linux/kernel.h>
#define OSAL_OUT_FILE NULL
#define OSAL_ERR_FILE NULL
#define osal_logf(f, ...) printk(__VA_ARGS__)
#endif

#define OSAL_LOG(...) osal_logf(OSAL_OUT_FILE, __VA_ARGS__)
#define OSAL_ERR(...) osal_logf(OSAL_ERR_FILE, __VA_ARGS__)

#endif /* __OSAL_LOG_H__ */
