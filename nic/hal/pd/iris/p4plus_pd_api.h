/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __P4PLUS_PD_API_H__
#define __P4PLUS_PD_API_H__

#include <stdint.h>
#include "nic/include/asic_pd.hpp"

/* PC address offset coming out of stage 0 CB is shifted left by 6 bits as stage0 pc
 * start is assumed to be 64B cacheline aligned
 */
#define MPU_PC_ADDR_SHIFT 6

#ifndef P4PD_API_UT
#include "nic/include/hal_pd_error.hpp"
#else
typedef int p4pd_error_t;
#endif

static inline bool
p4plus_hbm_write(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    hal_ret_t rv = hal::pd::asic_mem_write(addr, data, size_in_bytes);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
p4plus_hbm_read(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    hal_ret_t rv = hal::pd::asic_mem_read(addr, data, size_in_bytes);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
p4plus_reg_read(uint64_t addr, uint32_t& data)
{
    hal_ret_t rv = hal::pd::asic_reg_read(addr, &data);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
p4plus_reg_write(uint64_t addr, uint32_t data)
{
    hal_ret_t rv = hal::pd::asic_reg_write(addr, data);
    return rv == HAL_RET_OK ? true : false;
}

#endif
