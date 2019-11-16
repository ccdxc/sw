/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __PAL_COMPAT_HPP__
#define __PAL_COMPAT_HPP__

#ifdef __x86_64__

#include "nic/sdk/lib/pal/pal.hpp"

static inline uint8_t *MEM_MAP(uint64_t pa, uint32_t sz, uint32_t flags)
{
    uint8_t *vaddr = (uint8_t *)malloc(sz);
    uint32_t remsize = sz;
    for (uint32_t i = 0; i < sz; i += 1024) {
        sdk::lib::pal_mem_read(pa + i, vaddr + i, remsize > 1024 ? 1024 : remsize);
        remsize -= 1024;
    }
    return vaddr;
}
#define MEM_UNMAP(va)            free(va)
#define MEM_PTOV(pa)

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
    sdk::lib::pal_reg_read(addr, &val);
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

#define MEM_CLR(pa, va, sz, skip) { \
    if (!skip) { \
        MEM_SET(pa, 0, sz, 0); \
    } \
}

// used by dpdk driver to get the lifid and and devcmd regions
static inline void WRITE_DEVINFO(const char *fname, uint64_t pa, uint32_t lif)
{
    char buf[256];
    FILE *fp;
    const char *cfg_path = std::getenv("HAL_CONFIG_PATH");
    static int devices = 0;

    if (cfg_path) {
        snprintf(buf, sizeof(buf), "%s/gen/device_info.txt", cfg_path);
        if ((fp = fopen(buf, devices == 0 ? "w" : "a+")) != NULL) {
            if (devices == 0) {
                fprintf(fp, "Name BarAddr LifNum\n");
            }
            fprintf(fp, "%s 0x%lx %u\n", fname, pa, lif);
            fclose(fp);
            devices++;
        }
    }
}

#else

#include "nic/sdk/platform/pal/include/pal.h"

#define MEM_MAP         pal_mem_map
#define MEM_UNMAP       pal_mem_unmap
#define MEM_PTOV        pal_mem_ptov

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

#define MEM_CLR(pa, va, sz, skip) { \
    if (!skip) { \
        MEM_SET(pa, 0, sz, 0); \
    } \
}
#define WRITE_DEVINFO(fname, pa, lif) { }

#endif

#endif  /* __PAL_COMPAT_HPP__ */
