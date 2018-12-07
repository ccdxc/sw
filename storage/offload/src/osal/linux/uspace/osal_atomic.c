/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_atomic.h"

void osal_atomic_set(osal_atomic_int_t* addr, int val) 
{
	return atomic_store(addr, val);
}

int osal_atomic_read(const osal_atomic_int_t* addr) 
{
	return atomic_load(addr);
}

void osal_atomic_init(osal_atomic_int_t* addr, int val) 
{
	return atomic_init(addr, val);
}

int osal_atomic_fetch_add(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_add(addr, val);
}

int osal_atomic_fetch_sub(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_sub(addr, val);
}

int osal_atomic_fetch_and(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_and(addr, val);
}

int osal_atomic_fetch_or(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_or(addr, val);
}

int osal_atomic_fetch_xor(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_xor(addr, val);
}

int osal_atomic_exchange(osal_atomic_int_t* addr, int new_val)
{
	return atomic_exchange(addr, new_val);
}

void osal_atomic_lock(osal_atomic_int_t *addr)
{
	int tmp;

	while (1) {
		tmp = osal_atomic_exchange(addr, 1);
		if (tmp == 0) {
			break;
		}
	}
}

void osal_atomic_unlock(osal_atomic_int_t *addr)
{
	osal_atomic_set(addr, 0);
}

void osal_spin_lock_init(osal_spinlock_t *lock)
{
	osal_atomic_init(lock, 0);
}

void osal_spin_lock(osal_spinlock_t *lock)
{
	osal_atomic_lock(lock);
}

void osal_spin_unlock(osal_spinlock_t *lock)
{
	osal_atomic_unlock(lock);
}
