/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_SYS_H
#define OSAL_SYS_H

#ifdef _KERNEL
#define OSAL_MAX_CORES MAXCPU
#elif defined(__KERNEL__)
#include <linux/threads.h>
#define OSAL_MAX_CORES NR_CPUS
#else
#define OSAL_MAX_CORES 128
#endif

#ifdef __cplusplus
extern "C" {
#endif

int osal_get_coreid(void);
int osal_get_core_count(void);
int osal_yield(void);
int osal_sched_yield(void);

#ifdef __cplusplus
}
#endif

#endif
