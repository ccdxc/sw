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

