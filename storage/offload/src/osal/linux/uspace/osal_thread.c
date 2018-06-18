#include <assert.h>
#include "osal_thread.h"
#include "osal_errno.h"

void* osal_thread_fn_wrapper(void* arg) 
{
	void *rv;
	osal_thread_t *ot = (osal_thread_t *)arg;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	rv = ot->fn(ot->arg);
	osal_atomic_set(&ot->running, 0);
	return rv;
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

#define MAX_NUM_THREADS 16
static osal_atomic_int_t num_threads;
static int thread_map[MAX_NUM_THREADS];
int osal_get_coreid() 
{
	int i;
	int nthreads = osal_atomic_read(&num_threads);
	pthread_t tid = pthread_self();

	for (i = 0; i < nthreads && i < MAX_NUM_THREADS; i++) {
		if (thread_map[i] == tid)
			return i;
	}

	if (nthreads < MAX_NUM_THREADS) {
		nthreads = osal_atomic_fetch_add(&num_threads, 1);
		if (nthreads > MAX_NUM_THREADS) {
			//asserting for now
			assert(0);
		}
		thread_map[nthreads - 1] = (int)pthread_self();
		return nthreads - 1;
	}

	assert(0);
	return 0;
}
