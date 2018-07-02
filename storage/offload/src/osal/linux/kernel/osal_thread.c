#include "osal_thread.h"
#include "osal_errno.h"

static int  osal_thread_fn_wrapper(void *arg)
{
	int rv;
	osal_thread_t *ot = (osal_thread_t *)arg;

	rv = (int)ot->fn(ot->arg);
	osal_atomic_set(&ot->running, 0);
	return rv;
}

int osal_thread_run(osal_thread_t *thread, osal_thread_fn_t thread_fn, void *arg)
{
	if (thread == NULL) return EINVAL;
	thread->fn = thread_fn;
	thread->arg = arg;
	osal_atomic_set(&thread->running, 0);
	thread->handle = kthread_run(&osal_thread_fn_wrapper, thread, "None");
	if (IS_ERR(thread->handle))
	{
		return PTR_ERR(thread->handle);
	}
	osal_atomic_set(&thread->running, 1);
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
