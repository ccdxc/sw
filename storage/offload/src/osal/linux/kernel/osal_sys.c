/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include "osal_sys.h"

int osal_get_coreid(void)
{
	return smp_processor_id();
}

int osal_get_core_count(void)
{
	return num_online_cpus();
}

int osal_yield(void)
{
	yield();
	return 0;
}

int osal_sched_yield(void)
{
#ifdef _KERNEL
	maybe_yield();
#else
	cond_resched();
#endif
	return 0;
}
