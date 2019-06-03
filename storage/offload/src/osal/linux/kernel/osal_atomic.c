/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/atomic.h>
#include "osal_atomic.h"

void osal_atomic_set(osal_atomic_int_t* addr, int val) 
{
	return atomic_set(addr, val);
}

int osal_atomic_read(const osal_atomic_int_t* addr) 
{
	return atomic_read(addr);
}

void osal_atomic_init(osal_atomic_int_t* addr, int val) 
{
	return atomic_set(addr, val);
}

int osal_atomic_fetch_add(osal_atomic_int_t* addr, int val) 
{
	// Userspace and kernel space apis behave differently. In kernel, the returned value is after addition 
	// while in userspace it is before addition. Sticking with c11 behavior for now.
	return atomic_add_return(val, addr) - val;
}

int osal_atomic_fetch_sub(osal_atomic_int_t* addr, int val) 
{
	// Userspace and kernel space apis behave differently. In kernel, the returned value is after subtraction
	// while in userspace it is before subtraction. Sticking with c11 behavior for now.
	return atomic_sub_return(val, addr) + val;
}

#if defined(_KERNEL) || defined(atomic_fetch_or)

int osal_atomic_fetch_and(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_and(val, addr);
}

int osal_atomic_fetch_or(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_or(val, addr);
}

int osal_atomic_fetch_xor(osal_atomic_int_t* addr, int val) 
{
	return atomic_fetch_xor(val, addr);
}

#else

int osal_atomic_fetch_and(osal_atomic_int_t* addr, int val)
{
	int tmp, old;

	tmp = atomic_read(addr);
	while ((old = atomic_cmpxchg(addr, tmp, tmp & val)) != tmp)
		tmp = old;

	return tmp;
}

int osal_atomic_fetch_or(osal_atomic_int_t* addr, int val)
{
	int tmp, old;

	tmp = atomic_read(addr);
	while ((old = atomic_cmpxchg(addr, tmp, tmp | val)) != tmp)
		tmp = old;

	return tmp;
}

int osal_atomic_fetch_xor(osal_atomic_int_t* addr, int val)
{
	int tmp, old;

	tmp = atomic_read(addr);
	while ((old = atomic_cmpxchg(addr, tmp, tmp ^ val)) != tmp)
		tmp = old;

	return tmp;
}

#endif


int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val)
{
	return atomic_xchg(addr, new_val);
}

int osal_atomic_add_unless(osal_atomic_int_t *addr, int val, int cmp_val)
{
	int tmp;

	while (1) {
		tmp = atomic_read(addr);
		if (tmp == cmp_val)
			break;

		if (atomic_cmpxchg(addr, tmp, tmp + val) == tmp)
			break;
	}

	return tmp;
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
	spin_lock_init(lock);
}

void osal_spin_lock(osal_spinlock_t *lock)
{
	spin_lock(lock);
}

bool osal_spin_trylock(osal_spinlock_t *lock)
{
	return (spin_trylock(lock) != 0);
}

void osal_spin_unlock(osal_spinlock_t *lock)
{
	spin_unlock(lock);
}

void osal_rw_lock_init(osal_rwlock_t *lock)
{
	rwlock_init(lock);
}

void osal_rw_rlock(osal_rwlock_t *lock)
{
	read_lock(lock);
}

bool osal_rw_try_rlock(osal_rwlock_t *lock)
{
#ifdef _KERNEL
	return (rw_try_rlock(&lock->rw) != 0);
#else
	return (read_trylock(lock) != 0);
#endif
}

void osal_rw_runlock(osal_rwlock_t *lock)
{
	read_unlock(lock);
}

void osal_rw_wlock(osal_rwlock_t *lock)
{
	write_lock(lock);
}

bool osal_rw_try_wlock(osal_rwlock_t *lock)
{
#ifdef _KERNEL
	return (rw_try_wlock(&lock->rw) != 0);
#else
	return (write_trylock(lock) != 0);
#endif
}

void osal_rw_wunlock(osal_rwlock_t *lock)
{
	write_unlock(lock);
}
