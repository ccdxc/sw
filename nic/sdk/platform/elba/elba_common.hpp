// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_COMMON_HPP__
#define __ELBA_COMMON_HPP__

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace elba {
#endif

// Timers
#define ELBA_MEM_TIMER_START           0x52000000
#define ELBA_MEM_FAST_TIMER_START      (ELBA_MEM_TIMER_START + 0x4000)
#define ELBA_MEM_SLOW_TIMER_START      (ELBA_MEM_TIMER_START + 0x10000)
#define ELBA_HBM_CPU_RX_DPR_RING_SIZE  1024

// Hardware fifo enums
#define HFF_INVALID_FIFO_NUM        -1

#define HFF_RNMDPR_FIFO_NUM         0
#define HFF_TNMDPR_FIFO_NUM         1

#define HFF_BASE_ADDRESS            0x600000

#define HFF_ADDRESS(fifo_num)       (((fifo_num) << 6) + HFF_BASE_ADDRESS)

#ifdef __cplusplus
}    // namespace elba
}    // namespace platform
}    // namespace sdk
#endif    // __cplusplus

#endif    // __ELBA_COMMON_HPP__
