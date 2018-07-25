/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_thread.h"
#include "osal_sys.h"
#include "osal_errno.h"

void* osal_thread_fn_wrapper(void* arg) 
{
	int rv;
	osal_thread_t *ot = (osal_thread_t *)arg;

	rv = ot->fn(ot->arg);
	osal_atomic_set(&ot->running, 0);
	return (void*)((uint64_t)rv);
}

int osal_thread_create(osal_thread_t *thread, osal_thread_fn_t thread_fn, void* arg) 
{
	if (thread == NULL) return EINVAL;
	thread->fn = thread_fn;
	thread->arg = arg;
	thread->handle = 0;
	thread->core_id = -1;
	osal_atomic_init(&thread->running, 0);
	osal_atomic_init(&thread->should_stop, 0);

	return 0;
}

int osal_thread_bind(osal_thread_t *thread, int core_id)
{
	if (thread == NULL) {
		return EINVAL;
	}
	if (core_id < 0 || core_id >= osal_get_core_count()) {
		return EINVAL;
	}
	thread->core_id = core_id;
	return 0;
}

int osal_thread_start(osal_thread_t *thread)
{
	int rv;
	pthread_attr_t attr;
#ifdef _GNU_SOURCE
	cpu_set_t cpuset;
#endif

	if (thread == NULL) {
		return EINVAL;
	}
	if (osal_atomic_exchange(&thread->running, 1) != 0) {
		/* Thread already started */
		return EINVAL;
	}

	pthread_attr_init(&attr);
#ifdef _GNU_SOURCE
	if (thread->core_id >= 0) {
		CPU_ZERO(&cpuset);
		CPU_SET(thread->core_id, &cpuset);
		pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);
	}
#endif

	rv = pthread_create(&thread->handle, &attr, &osal_thread_fn_wrapper, thread);
	if (rv != 0) {
		osal_atomic_set(&thread->running, 0);
	}

	pthread_attr_destroy(&attr);
	return rv;
}

int osal_thread_stop(osal_thread_t* osal_thread) 
{
	int running = osal_atomic_read(&osal_thread->running);

	if (running) {
		int rv;
		if (osal_atomic_exchange(&osal_thread->should_stop, 1)) {
			/* Some other caller is already stopping the thread */
			return EINVAL;
		}
		rv = pthread_join(osal_thread->handle, NULL);
		osal_atomic_set(&osal_thread->should_stop, 0);
		return rv;
	}

	return 0;
}

bool osal_thread_is_running(osal_thread_t* osal_thread)
{
	return (bool) osal_atomic_read(&osal_thread->running);
}

bool osal_thread_should_stop(osal_thread_t* osal_thread)
{
	return (bool) osal_atomic_read(&osal_thread->should_stop);
}
