// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <sys/mman.h>
#include <dlfcn.h>

#include "pal.hpp"

#include "nic/include/base.h"

typedef struct pal_mmap_regions_s {
    uint64_t    pa_base;
    uint64_t    size;
    uint64_t    va_base;
} pal_mmap_regions_t;

extern pal_info_t   gl_pal_info;
#define NELEMS(_arr) sizeof(_arr)/sizeof(_arr[0])
pal_mmap_regions_t
pal_mmap_regions[] = {
    {
        // Registers
        .pa_base = 0x01000000,
        .size = 0x6effffff,
    },
    {
        // 3GB of HBM
        .pa_base = 0xC0000000,
        .size = 0xC0000000,
    }
};
        

static inline uint64_t
pal_hw_get_mmap_addr (uint64_t addr)
{
    uint32_t         i = 0;
    uint64_t    offset = 0;
    for (i = 0; i < NELEMS(pal_mmap_regions); i++) {
        if (addr >= pal_mmap_regions[i].pa_base &&
            addr < pal_mmap_regions[i].pa_base + pal_mmap_regions[i].size) {
            offset = pal_mmap_regions[i].pa_base - addr;
            return pal_mmap_regions[i].va_base + offset;
        }
    }
    return 0;
}

static inline pal_ret_t
pal_hw_read_common (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr = pal_hw_get_mmap_addr(addr);
    for (uint32_t i = 0; i < size; i++) {
        data[i] = ((uint8_t *)mmap_addr)[i];
    }
    return PAL_RET_OK;
}

static inline pal_ret_t
pal_hw_write_common (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr = pal_hw_get_mmap_addr(addr);
    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *)mmap_addr)[i] = data[i];
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_hw_reg_read (uint64_t addr, uint32_t *data)
{
    return pal_hw_read_common(addr, (uint8_t *)data, sizeof(uint32_t));
}

pal_ret_t
pal_hw_reg_write (uint64_t addr, uint32_t data)
{
    return pal_hw_write_common(addr, (uint8_t *)&data, sizeof(uint32_t));
}

pal_ret_t
pal_hw_mem_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    return pal_hw_read_common(addr, data, size);
}

pal_ret_t
pal_hw_mem_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    return pal_hw_write_common(addr, data, size);
}

pal_ret_t
pal_hw_ring_doorbell (uint64_t addr, uint64_t data)
{
    assert(0);
    return PAL_RET_OK;
}

static pal_ret_t
pal_hw_mmap_device ()
{
    uint32_t i = 0;
    int devfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (devfd < 0) {
        HAL_TRACE_ERR("Failed to open /dev/mem");
        return PAL_RET_NOK;
    }

    for (i = 0; i < NELEMS(pal_mmap_regions); i++) {
        pal_mmap_regions[i].va_base = 
                    (uint64_t)mmap((void *)pal_mmap_regions[i].pa_base,
                                    pal_mmap_regions[i].size,
                                    PROT_READ|PROT_WRITE,
                                    MAP_SHARED, devfd, 0);
        HAL_TRACE_DEBUG("MMAP Info: pa_base:{} size:{} va_base:{}",
                        pal_mmap_regions[i].pa_base,
                        pal_mmap_regions[i].size,
                        pal_mmap_regions[i].va_base);

        if (pal_mmap_regions[i].va_base == 0) {
            HAL_TRACE_ERR("Failed to mmap /dev/mem");
            return PAL_RET_NOK;
        }
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_init_rwvectors ()
{
    gl_pal_info.rwvecs.reg_read = pal_hw_reg_read;
    gl_pal_info.rwvecs.reg_write = pal_hw_reg_write;
    gl_pal_info.rwvecs.mem_read = pal_hw_mem_read;
    gl_pal_info.rwvecs.mem_write = pal_hw_mem_write;
    gl_pal_info.rwvecs.ring_doorbell = pal_hw_ring_doorbell;

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_init ()
{
    pal_ret_t   rv;

    rv = pal_hw_init_rwvectors();
    HAL_ASSERT(IS_PAL_API_SUCCESS(rv));

    rv = pal_hw_mmap_device();
    HAL_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}
