/*
 * tcp_proxy_rxdma_api.hpp
 * Saurabh Jain (Pensando Systems)
 */
#ifndef __P4PLUS_PD_API_H__
#define __P4PLUS_PD_API_H__

#include <stdint.h>
#include <../../../model_sim/include/lib_model_client.h>

#ifndef P4PD_API_UT
#include <hal_pd_error.hpp>
#else
typedef int p4pd_error_t;
#endif

static inline bool
p4plus_hbm_write(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    return write_mem(addr, data, size_in_bytes);
}

static inline bool
p4plus_hbm_read(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    return read_mem(addr, data, size_in_bytes);
}

#endif
