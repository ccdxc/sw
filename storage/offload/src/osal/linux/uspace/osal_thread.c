#include "osal_thread.h"
#include "osal_errno.h"

void* osal_thread_fn_wrapper(void* arg) 
{
	int rv;
	osal_thread_t *ot = (osal_thread_t *)arg;

	rv = ot->fn(ot->arg);
	osal_atomic_set(&ot->running, 0);
	return (void*)((uint64_t)rv);
}

int osal_thread_run(osal_thread_t *thread, osal_thread_fn_t thread_fn, void* arg) 
{
	int rv;

	if (thread == NULL) return EINVAL;
	thread->fn = thread_fn;
	thread->arg = arg;
	osal_atomic_set(&thread->running, 0);
	rv = pthread_create(&thread->handle, NULL, &osal_thread_fn_wrapper, thread);
	if (rv == 0) {
		osal_atomic_set(&thread->running, 1);
	}
	return rv;
}

int osal_thread_stop(osal_thread_t* osal_thread) 
{
	int rv = 0;
	int running = osal_atomic_read(&osal_thread->running);

	if (running) {
		rv = pthread_cancel(osal_thread->handle);
	}
	return rv;
}
