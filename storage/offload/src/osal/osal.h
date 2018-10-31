/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __OSAL_H__
#define __OSAL_H__

#ifdef __KERNEL__
#include <linux/netdevice.h>
#include <linux/ktime.h>

#ifndef __FreeBSD__
#include <linux/net.h>
#endif
#endif

#include "osal_stdtypes.h"
#include "osal_errno.h"
#include "osal_assert.h"
#include "osal_logger.h"
#include "osal_atomic.h"
#include "osal_mem.h"
#include "osal_rmem.h"
#include "osal_thread.h"
#include "osal_setup.h"
#include "osal_sys.h"
#include "osal_random.h"

#ifdef __FreeBSD__
#define PRIx64	"%lx"
#define PRIu64	"%lu"
#else
#define PRIx64	"%llx"
#define PRIu64	"%llu"
#endif
#endif	/* __OSAL_H__ */
