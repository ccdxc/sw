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
	return atomic_add_return(val, addr) - val;
}

int osal_atomic_fetch_sub(osal_atomic_int_t* addr, int val) 
{
	return atomic_sub_return(val, addr) + val;
}

int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val)
{
	return atomic_xchg(addr, new_val);
}
