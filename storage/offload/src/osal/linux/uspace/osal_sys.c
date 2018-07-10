/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <assert.h>
#include <sys/sysinfo.h>
#include <sched.h>
#include "osal_atomic.h"
#include "osal_sys.h"
#include "osal_errno.h"
#include "osal_thread.h"

#define MAX_NUM_THREADS 128
static osal_atomic_int_t num_threads;
static pthread_t thread_map[MAX_NUM_THREADS];
static _Thread_local int core_id = -1;
int osal_get_coreid() 
{
	int nthreads = osal_atomic_read(&num_threads);

	if(core_id != -1) return core_id;

	if (nthreads < MAX_NUM_THREADS) 
	{
		nthreads = osal_atomic_fetch_add(&num_threads, 1);
		if (nthreads > MAX_NUM_THREADS) 
		{
			//asserting for now
			assert(0);
		}
		core_id = nthreads;
		thread_map[nthreads] = pthread_self();
		return nthreads;
	}

	assert(0);
	return 0;
}

int osal_get_core_count()
{
	return get_nprocs();
}

int osal_yield(void)
{
	return sched_yield();
}
