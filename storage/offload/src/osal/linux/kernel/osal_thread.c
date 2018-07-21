/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_thread.h"
#include "osal_sys.h"
#include "osal_errno.h"

#ifdef _KERNEL
#include <sys/mutex.h>
#endif

static int  osal_thread_fn_wrapper(void *arg)
{
	int rv;
	osal_thread_t *ot = (osal_thread_t *)arg;

	/* Bind the thread to the current core */
#ifdef _KERNEL
	if (ot->core_id >= 0) {
		thread_lock(curthread);
		sched_bind(curthread, ot->core_id);
		thread_unlock(curthread);
		osal_yield();
	}
#endif

	rv = (int)ot->fn(ot->arg);
	osal_atomic_set(&ot->running, 0);
	return rv;
}

int osal_thread_create(osal_thread_t *thread, osal_thread_fn_t thread_fn, void *arg)
{
	void *td = NULL;

	if (thread == NULL) return EINVAL;
	thread->fn = thread_fn;
	thread->arg = arg;
	thread->handle = NULL;
	thread->core_id = -1;
	osal_atomic_init(&thread->running, 0);

	return 0;
}

int osal_thread_bind(osal_thread_t *thread, int core_id)
{
	if (thread == NULL || thread->handle == NULL) {
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
	thread_t t;

	if (thread == NULL || thread->handle == NULL) {
		return EINVAL;
	}
	if (osal_atomic_exchange(&thread->running, 1) != 0) {
		/* Thread already started */
		return EINVAL;
	}
#ifdef _KERNEL
	t = kthread_run(&osal_thread_fn_wrapper, thread, "None");
	/* FreeBSD allows late binding */
#else
	if (thread->core_id >= 0) {
		t = kthread_run_on_cpu(&osal_thread_fn_wrapper,
				       thread, "None", thread->core_id);
	} else {
		t = kthread_run(&osal_thread_fn_wrapper,
				thread, "None");
	}
#endif
	if (IS_ERR(t)) {
		osal_atomic_set(&thread->running, 0);
		return PTR_ERR(t);
	}
	thread->handle = t;
	return 0;
}

int osal_thread_stop(osal_thread_t *osal_thread)
{
	int rv = 0;
	int running = osal_atomic_read(&osal_thread->running);

	if (running)
	{
		rv = kthread_stop(osal_thread->handle);
	}
	return rv;
}

bool osal_thread_is_running(osal_thread_t* osal_thread)
{
	return (bool) osal_atomic_read(&osal_thread->running);
}

bool osal_thread_should_stop(osal_thread_t* osal_thread)
{
	/* TODO: should not ignore input param */
	return (bool) kthread_should_stop();
}
