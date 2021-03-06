/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef SONIC_API_INT_H
#define SONIC_API_INT_H

#include <linux/types.h>
#include "accel_ring.h"
#include "osal_atomic.h"

struct per_core_resource;
extern bool rate_limit_src_en;
extern bool rate_limit_dst_en;
extern bool chain_rate_limit_src_en;
extern bool chain_rate_limit_dst_en;
extern unsigned int interm_buf_size;
extern bool error_reset_recovery_en;

struct sonic;
struct sonic_dev;

#define SONIC_INTR_FIRE_DATA32		0x95969798
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

static inline bool
sonic_rate_limit_src_en_get(void)
{
	return rate_limit_src_en;
}

static inline bool
sonic_rate_limit_dst_en_get(void)
{
	return rate_limit_dst_en;
}

static inline bool
sonic_chain_rate_limit_src_en_get(void)
{
	return chain_rate_limit_src_en;
}

static inline bool
sonic_chain_rate_limit_dst_en_get(void)
{
	return chain_rate_limit_dst_en;
}

static inline unsigned int
sonic_interm_buf_size_get(void)
{
	return interm_buf_size;
}

static inline bool
sonic_error_reset_recovery_en_get(void)
{
	return error_reset_recovery_en;
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

unsigned int sonic_get_lif_id(struct sonic *sonic, uint32_t idx);
uint64_t sonic_get_lif_local_dbaddr(void);
bool sonic_validate_crypto_key_idx(uint32_t user_key_idx, uint32_t *ret_keys_max);
uint32_t sonic_get_crypto_key_idx(uint32_t user_key_idx);
uint32_t sonic_get_crypto_suspect_key_idx(uint32_t user_key_idx);
uint64_t sonic_get_intr_assert_addr(uint32_t intr_idx);
uint32_t sonic_get_intr_assert_data(void);
struct sonic_accel_ring *sonic_get_accel_ring(uint32_t accel_ring_id);
accel_ring_id_t sonic_get_accel_ring_id(struct sonic_accel_ring *ring);
const char *sonic_accel_ring_name_get(uint32_t accel_ring_id);
void sonic_accel_rings_reinit(struct sonic_dev *idev);
int sonic_accounting_atomic_take(osal_atomic_int_t *atomic_c,
				 uint32_t count,
				 uint32_t high_water);
int sonic_accounting_atomic_give(osal_atomic_int_t *atomic_c,
				 uint32_t count);
int sonic_accel_ring_take(struct sonic_accel_ring *ring, uint32_t count);
int sonic_accel_ring_give(struct sonic_accel_ring *ring, uint32_t count);
int sonic_accel_rings_sanity_check(void);

bool sonic_pnso_async_poll(uint64_t data);
uint16_t sonic_intr_get_ev_id(struct per_core_resource *pc_res,
			      uint64_t usr_data, uint64_t *paddr);
void sonic_intr_put_ev_id(struct per_core_resource *pc_res, uint16_t id);
void sonic_intr_touch_ev_id(struct per_core_resource *pc_res, uint16_t id);
uint64_t sonic_get_per_core_intr_assert_addr(struct per_core_resource *pc_res);

static inline uint32_t
sonic_intr_get_fire_data32(void)
{
	return SONIC_INTR_FIRE_DATA32;
}

static inline uint64_t
sonic_intr_get_fire_data64(void)
{
	/* 64-bit data must be the same 32-bit data replicated twice */
	return ((uint64_t)SONIC_INTR_FIRE_DATA32 << 32) | SONIC_INTR_FIRE_DATA32;
}

uint64_t sonic_hostpa_to_devpa(uint64_t hostpa);
uint64_t sonic_devpa_to_hostpa(uint64_t devpa);
void *sonic_phy_to_virt(uint64_t phy);

uint64_t sonic_virt_to_phy(void *ptr);
void *sonic_phy_to_virt(uint64_t phy);

bool sonic_is_accel_dev_ready(void);
void sonic_report_seq_bmps(struct per_core_resource *pcr);
void sonic_report_pcr_ev_list(struct per_core_resource *pcr);
void sonic_report_ev_lists(void);

void sonic_report_pcr_counters(const struct per_core_resource *pcr);

#define REPORT_PCR_COUNTERS(pcr)					\
	do {								\
		if (OSAL_LOG_ON(OSAL_LOG_LEVEL_NOTICE)) 		\
			sonic_report_pcr_counters(pcr);			\
	} while (0)

#endif /* SONIC_API_INT_H */
