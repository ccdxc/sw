#ifndef SONIC_API_INT_H
#define SONIC_API_INT_H

#include <linux/types.h>
#include "accel_ring.h"


uint64_t sonic_rmem_alloc(size_t size);
void sonic_rmem_free(uint64_t ptr, size_t size);

accel_ring_t* sonic_get_accel_ring(uint32_t accel_ring_id);
int sonic_get_accel_ring_base_pa(uint32_t accel_ring_id, uint64_t *ring_base);
int sonic_get_accel_ring_pndx_pa(uint32_t accel_ring_id, uint64_t *ring_pndx);
int sonic_get_accel_ring_shadow_pndx_pa(uint32_t accel_ring_id, uint64_t *shadow_pndx);
int sonic_get_accel_ring_size(uint32_t accel_ring_id, uint32_t *ring_size);

#endif /* SONIC_API_INT_H */
