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

#define P4PLUS_MAX_DATA_LEN_IN_BYTES    512

static inline void
p4plus_swizzle_bytes(uint8_t* data, uint32_t size_in_bytes)
{
    size_in_bytes += (size_in_bytes & 1) ? 1 : 0;

    for(uint32_t i = 0; i < (size_in_bytes >> 1); i++) {
        uint8_t temp = data[i];
        data[i] = data[size_in_bytes - i - 1];
        data[size_in_bytes -i -1] = temp;
    }
}

static inline bool
p4plus_hbm_write(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
#if 0
    if(size_in_bytes > P4PLUS_MAX_DATA_LEN_IN_BYTES) {
        printf("Cannot write more than 64 byte data\n");
        return false;    
    } 

    uint8_t buffer[P4PLUS_MAX_DATA_LEN_IN_BYTES] = {0}; 
    memcpy(buffer, data, size_in_bytes);
    
    // Swizzle entry 
    p4plus_swizzle_bytes(buffer, size_in_bytes);
    size_in_bytes += (size_in_bytes & 1) ? 1 : 0;
#endif
    return write_mem(addr, data, size_in_bytes);            
}

static inline bool
p4plus_hbm_read(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    if(size_in_bytes > P4PLUS_MAX_DATA_LEN_IN_BYTES) {
        printf("Cannot read more than 64 byte data\n");
        return false;    
    } 

    uint8_t buffer[P4PLUS_MAX_DATA_LEN_IN_BYTES] = {0}; 
    int readlen_in_bytes = (size_in_bytes & 1) ? size_in_bytes + 1 : size_in_bytes;

    if(!read_mem(addr, buffer, readlen_in_bytes)) {
        return false;    
    }

    p4plus_swizzle_bytes(buffer, readlen_in_bytes);

    memcpy(data, buffer, size_in_bytes);
    return true;
}

#endif
