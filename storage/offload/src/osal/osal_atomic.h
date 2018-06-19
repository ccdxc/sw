#ifndef OSAL_ATOMIC_H
#define OSAL_ATOMIC_H

#ifndef __KERNEL__

#include <stdatomic.h>
#define osal_atomic_int_t atomic_int

#else

#define osal_atomic_int_t atomic_t

#endif

void osal_atomic_set(osal_atomic_int_t *addr, int val);
int osal_atomic_read(const osal_atomic_int_t *addr);
void osal_atomic_init(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_add(osal_atomic_int_t *addr, int val);
int osal_atomic_fetch_sub(osal_atomic_int_t *addr, int val);
int osal_atomic_exchange(osal_atomic_int_t *addr, int new_val);



#endif
