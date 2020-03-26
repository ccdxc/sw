// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __ELBA_COMMON_HPP_
#define __ELBA_COMMON_HPP_

#include "asic/cmn/asic_common.hpp"

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace elba {
#endif
/*
 * #defines shared by ASM and C code
 */

// Timers
#define ELBA_MEM_TIMER_START           0x52400000
#define ELBA_MEM_FAST_TIMER_START      (ELBA_MEM_TIMER_START + 0x4000)
#define ELBA_MEM_SLOW_TIMER_START      (ELBA_MEM_TIMER_START + 0x10000)
#define ELBA_HBM_CPU_RX_DPR_RING_SIZE  1024

#ifdef __cplusplus
}    // namespace elba
}    // namespace platform
}    // namespace sdk
#endif    // __cplusplus
#endif    // __ELBA_COMMON_HPP_
