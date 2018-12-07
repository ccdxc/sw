/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_ATOMIC_H
#define OSAL_ATOMIC_H

#ifndef __KERNEL__

#include <stdatomic.h>
#define osal_atomic_int_t atomic_int
#define osal_spinlock_t atomic_int

#else
#include <linux/kernel.h>
#include <linux/spinlock.h>
#define osal_atomic_int_t atomic_t
#define osal_spinlock_t spinlock_t

#endif

#ifdef __cplusplus
extern "C" {
#endif

void osal_atomic_set(osal_atomic_int_t *addr, int val);
int osal_atomic_read(const osal_atomic_int_t *addr);
void osal_atomic_init(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_add(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_sub(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_sub_post(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_and(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_or(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_xor(osal_atomic_int_t *addr, int val);
int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val);
int osal_atomic_add_unless(osal_atomic_int_t *addr, int a, int u);
int osal_atomic_dec_if_positive(osal_atomic_int_t *addr);

void osal_atomic_lock(osal_atomic_int_t *addr);
void osal_atomic_unlock(osal_atomic_int_t *addr);

void osal_spin_lock_init(osal_spinlock_t *lock);
void osal_spin_lock(osal_spinlock_t *lock);
void osal_spin_unlock(osal_spinlock_t *lock);

#ifdef __cplusplus
}
#endif

#endif
