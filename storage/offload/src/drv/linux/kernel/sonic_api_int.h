#ifndef SONIC_API_INT_H
#define SONIC_API_INT_H

#include <linux/types.h>

uint64_t sonic_rmem_alloc(size_t size);
void sonic_rmem_free(uint64_t ptr, size_t size);

#endif /* SONIC_API_INT_H */
