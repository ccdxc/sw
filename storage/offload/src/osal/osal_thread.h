/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_THREAD_H
#define OSAL_THREAD_H

#include "osal_atomic.h"
#include "osal_stdtypes.h"

#ifndef __KERNEL__
#include <pthread.h>
typedef pthread_t thread_t;
struct workqueue_struct;
#else
#include <linux/kthread.h>
#include <linux/workqueue.h>
typedef struct task_struct *thread_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*osal_thread_fn_t)(void *);
typedef struct osal_thread_ {
	thread_t handle;
	osal_atomic_int_t running;
	osal_atomic_int_t should_stop;
	osal_thread_fn_t fn;
	int core_id;
	void *arg;
} osal_thread_t;

int osal_thread_create(osal_thread_t *thread, osal_thread_fn_t thread_fn, void *arg);
int osal_thread_bind(osal_thread_t *thread, int core_id);
int osal_thread_start(osal_thread_t *thread);

/* Wait for the given thread to complete */
int osal_thread_stop(osal_thread_t *osal_thread);

bool osal_thread_is_running(osal_thread_t* osal_thread);
bool osal_thread_should_stop(osal_thread_t* osal_thread);

struct workqueue_struct *osal_create_workqueue_fast(char *name, int max_active);
#define osal_destroy_workqueue destroy_workqueue

#ifdef __cplusplus
}
#endif

#endif
