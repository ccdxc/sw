#ifndef SONIC_API_INT_H
#define SONIC_API_INT_H

#include <linux/types.h>
#include "accel_ring.h"

#define SONIC_RMEM_ADDR_INVALID		0

static inline uint64_t
sonic_rmem_addr_invalid_def(void)
{
	return SONIC_RMEM_ADDR_INVALID;
}

static inline bool
sonic_rmem_addr_valid(uint64_t addr)
{
	return addr != SONIC_RMEM_ADDR_INVALID;
}

uint32_t sonic_rmem_total_pages_get(void);
uint32_t sonic_rmem_avail_pages_get(void);
uint32_t sonic_rmem_page_size_get(void);
uint64_t sonic_rmem_alloc(size_t size);
uint64_t sonic_rmem_calloc(size_t size);
void sonic_rmem_free(uint64_t pgaddr, size_t size);
void sonic_rmem_set(uint64_t pgaddr, uint8_t val, size_t size);
void sonic_rmem_read(void *dst, uint64_t pgaddr, size_t size);
void sonic_rmem_write(uint64_t pgaddr, const void *src, size_t size);

uint16_t sonic_get_lif_id(void);
uint64_t sonic_get_lif_local_dbaddr(void);
bool sonic_validate_crypto_key_idx(uint32_t user_key_idx, uint32_t *ret_keys_max);
uint32_t sonic_get_crypto_key_idx(uint32_t user_key_idx);
uint64_t sonic_get_intr_assert_addr(uint32_t intr_idx);
uint32_t sonic_get_intr_assert_data(void);
accel_ring_t *sonic_get_accel_ring(uint32_t accel_ring_id);

uint64_t sonic_hostpa_to_devpa(uint64_t hostpa);
uint64_t sonic_devpa_to_hostpa(uint64_t devpa);
void *sonic_phy_to_virt(uint64_t phy);

uint64_t sonic_virt_to_phy(void *ptr);
void *sonic_phy_to_virt(uint64_t phy);

#endif /* SONIC_API_INT_H */
