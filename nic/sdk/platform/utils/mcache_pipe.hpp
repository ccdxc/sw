/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mcache_pipe.hpp
 *
 * @brief   This module defines memory partition cache pipe enums
 */
#ifndef __SDK_PLATFORM_MEM_REGION_CACHE_PIPE_HPP__
#define __SDK_PLATFORM_MEM_REGION_CACHE_PIPE_HPP__

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace utils {
#endif
/**
 * @brief Memory mpartition cache pipes
 */
typedef enum cache_pipe_e {
    // Note: Values are used in bitmap
    MEM_REGION_CACHE_PIPE_NONE           = 0,    /**< None */
    MEM_REGION_CACHE_PIPE_P4IG           = 1,    /**< P4 ingress */
    MEM_REGION_CACHE_PIPE_P4EG           = 2,    /**< P4 egress */
    MEM_REGION_CACHE_PIPE_P4IG_P4EG      = 3,    /**< P4 ingress and egress */
    MEM_REGION_CACHE_PIPE_P4PLUS_TXDMA   = 4,    /**< P4plus txdma */
    MEM_REGION_CACHE_PIPE_P4PLUS_RXDMA   = 8,    /**< P4plus rxdma */
    MEM_REGION_CACHE_PIPE_P4PLUS_PCIE_DB = 16,   /**< P4plus pcie */
    MEM_REGION_CACHE_PIPE_P4PLUS_ALL     = 28,   /**< TxDMA + RxDMA + PCIE + DB blocks */
} cache_pipe_t;

#ifdef __cplusplus
}   // namespace utils
}   // namespace platform
}   // namespace sdk
using sdk::platform::utils::cache_pipe_t;
#endif
#endif    // __SDK_PLATFORM_MEM_REGION_CACHE_PIPE_HPP__
