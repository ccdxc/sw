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

int osal_atomic_fetch_sub_post(osal_atomic_int_t* addr, int val) 
{
	// return the value post subtraction
	return atomic_sub_return(val, addr);
}

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

int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val)
{
	return atomic_xchg(addr, new_val);
}

int osal_atomic_add_unless(osal_atomic_int_t* addr, int a, int u)
{
	return atomic_add_unless(addr, a, u);
}

int osal_atomic_dec_if_positive(osal_atomic_int_t* addr)
{
#ifdef atomic_dec_if_positive
	return atomic_dec_if_positive(addr);
#else
	/*
	 * atomic_dec_if_positive() is missing in some server hosts'
	 * build environment. Code fragment below is temporarily taken
	 * from linux/include/linux/atomic.h to prevent compilation issues.
	 */
	int c, old, dec;
	c = atomic_read(addr);
	for (;;) {
		dec = c - 1;
		if (unlikely(dec < 0))
			break;
		old = atomic_cmpxchg(addr, c, dec);
		if (likely(old == c))
			break;
		c = old;
	}
	return dec;
#endif
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
