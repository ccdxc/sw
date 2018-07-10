/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <asm/atomic.h>
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

int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val)
{
	return atomic_xchg(addr, new_val);
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
