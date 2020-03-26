/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __P4PLUS_PD_API_H__
#define __P4PLUS_PD_API_H__

#include <stdint.h>
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "asic/asic.hpp"
#include "asic/pd/pd.hpp"

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
p4plus_hbm_write (uint64_t addr, uint8_t* data, uint32_t size_in_bytes,
                  p4plus_cache_action_t action)
{
    sdk_ret_t rv = sdk::asic::asic_mem_write(addr, data, size_in_bytes);
    if (action != P4PLUS_CACHE_ACTION_NONE) {
        sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr, size_in_bytes, action);
    }
    return rv == SDK_RET_OK ? true : false;
}

#endif    // __P4PLUS_PD_API_H__
