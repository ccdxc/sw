/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __PAL_COMPAT_HPP__
#define __PAL_COMPAT_HPP__

#ifdef __x86_64__

#include "nic/sdk/lib/pal/pal.hpp"

#define READ_MEM        sdk::lib::pal_mem_read
#define WRITE_MEM       sdk::lib::pal_mem_write
#define MEM_SET(pa, val, sz, flags) { \
    uint8_t v = val; \
    for (size_t i = 0; i < sz; i += sizeof(v)) { \
        sdk::lib::pal_mem_write(pa + i, &v, sizeof(v)); \
    } \
}

#define READ_REG        sdk::lib::pal_reg_read
#define WRITE_REG       sdk::lib::pal_reg_write
static inline uint32_t READ_REG32(uint64_t addr)
{
    uint32_t val;
    sdk::lib::pal_reg_write(addr, &val);
    return val;
}
#define WRITE_REG32(addr, val) { \
    uint32_t v = val; \
    sdk::lib::pal_reg_write(addr, &v); \
}
static inline uint64_t READ_REG64(uint64_t addr)
{
    uint64_t val;
    sdk::lib::pal_reg_read(addr, (uint32_t *)&val, 2);
    return val;
}
#define WRITE_REG64(addr, val) { \
    uint64_t v = val; \
    sdk::lib::pal_reg_write(addr, (uint32_t *)&v, 2); \
}

#define WRITE_DB64      sdk::lib::pal_ring_db64

#define PAL_barrier()   do {} while (0)

#else

#include "nic/sdk/platform/pal/include/pal.h"

#define READ_MEM        pal_mem_rd
#define WRITE_MEM       pal_mem_wr
#define MEM_SET         pal_memset

#define READ_REG        pal_reg_rd32w
#define WRITE_REG       pal_reg_wr32w
#define READ_REG32      pal_reg_rd32
#define WRITE_REG32     pal_reg_wr32
#define READ_REG64      pal_reg_rd64
#define WRITE_REG64     pal_reg_wr64

#define WRITE_DB64      pal_reg_wr64

#endif

#endif  /* __PAL_COMPAT_HPP__ */
