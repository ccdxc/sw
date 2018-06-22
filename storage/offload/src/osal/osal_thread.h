#ifndef OSAL_THREAD_H
#define OSAL_THREAD_H

#include "osal_atomic.h"
#include "osal_stdtypes.h"

#ifndef __KERNEL__
#include <pthread.h>
typedef pthread_t thread_t;
#else
#include <linux/kthread.h>
typedef struct task_struct thread_t;
#endif

typedef int (*osal_thread_fn_t)(void *);
typedef struct osal_thread_ {
	thread_t handle;
	osal_atomic_int_t running;
	osal_atomic_int_t should_stop;
	osal_thread_fn_t fn;
	void *arg;
} osal_thread_t;

int osal_thread_run(osal_thread_t *thread, osal_thread_fn_t thread_fn, void *arg);
int osal_thread_stop(osal_thread_t *osal_thread);
bool osal_thread_is_running(osal_thread_t* osal_thread);
bool osal_thread_should_stop(osal_thread_t* osal_thread);


#endif
