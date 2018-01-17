// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

extern pal_info_t   gl_pal_info;
typedef struct pal_mmap_regions_s {
    uint64_t    pa_base;
    uint64_t    size;
    uint8_t     csr_access;
    uint64_t    va_base;
} __PACK__ pal_mmap_regions_t;

#define NELEMS(_arr) sizeof(_arr)/sizeof(_arr[0])

pal_mmap_regions_t
pal_mmap_regions[] = {
    {
        // Registers
        .pa_base = 0x01000000,
        .size = 0x6effffff,
        .csr_access = 1
    },
    {
        // 3GB of HBM
        .pa_base = 0xC0000000,
        .size = 0xC0000000,
        .csr_access = 0
    }
};

static inline bool
pal_hw_mpu_addr_range_check(uint64_t addr)
{
    if (addr >= 0x11ed37000 &&
        addr <  0x11ED4F000) {
        return true;
    }
    return false;
}

static inline int
pal_hw_virtual_addr_get (uint64_t phy_addr,
                         uint64_t *mmap_addr,
                         uint8_t  *csr_access)
{
    uint32_t    i      = 0;
    uint64_t    offset = 0;
    for (i = 0; i < NELEMS(pal_mmap_regions); i++) {
        if (phy_addr >= pal_mmap_regions[i].pa_base &&
            phy_addr < pal_mmap_regions[i].pa_base + pal_mmap_regions[i].size) {

            if (pal_hw_mpu_addr_range_check(phy_addr) == true) {
                SDK_TRACE_DEBUG("%s MPU_TRACE_ADDR 0x%lx",
                                __FUNCTION__, phy_addr);
            }

            offset = phy_addr - pal_mmap_regions[i].pa_base;
            *mmap_addr = pal_mmap_regions[i].va_base + offset;

            *csr_access = pal_mmap_regions[i].csr_access;
            return 0;
        }
    }
    return -1;
}

static inline pal_ret_t
pal_hw_read_common (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr  = 0x0;
    uint8_t  csr_access = false;
    int valid = pal_hw_virtual_addr_get (addr, &mmap_addr, &csr_access);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. addr: 0x%x\n", __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    if (csr_access == 1) {
        //SDK_TRACE_DEBUG("%s reading addr 0x%x, mmap 0x%x",
          //              __FUNCTION__, addr, mmap_addr);
        uint32_t *read_data = (uint32_t*) data;
        *read_data = *(uint32_t *)mmap_addr;
    } else {
        for (uint32_t i = 0; i < size; i++) {
            data[i] = ((uint8_t *)mmap_addr)[i];
        }
    }

    return PAL_RET_OK;
}

static inline pal_ret_t
pal_hw_write_common (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr  = 0x0;
    uint8_t  csr_access = false;
    int valid = pal_hw_virtual_addr_get (addr, &mmap_addr, &csr_access);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. addr: 0x%x\n", __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    if (csr_access == 1) {
        uint32_t *write_data = (uint32_t*) data;

        //SDK_TRACE_DEBUG("%s writing addr 0x%x, mmap 0x%x, data 0x%x",
         //               __FUNCTION__, addr, mmap_addr, *write_data);

        *(uint32_t *)mmap_addr = *write_data;
    } else {
        for (uint32_t i = 0; i < size; i++) {
            ((uint8_t *)mmap_addr)[i] = data[i];
        }
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
pal_hw_mmap_device (void)
{
    uint32_t i = 0;
    int devfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (devfd < 0) {
        SDK_TRACE_ERR("Failed to open /dev/mem");
        return PAL_RET_NOK;
    }

    for (i = 0; i < NELEMS(pal_mmap_regions); i++) {
        pal_mmap_regions[i].va_base =
                    (uint64_t)mmap(NULL,
                                   pal_mmap_regions[i].size,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED,
                                   devfd,
                                   pal_mmap_regions[i].pa_base);
        SDK_TRACE_DEBUG("MMAP Info fixed: pa_base:0x%x size:%u va_base:0x%x",
                        pal_mmap_regions[i].pa_base,
                        pal_mmap_regions[i].size,
                        pal_mmap_regions[i].va_base);

        if (pal_mmap_regions[i].va_base == 0) {
            SDK_TRACE_ERR("Failed to mmap /dev/mem");
            return PAL_RET_NOK;
        }
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read = pal_hw_reg_read;
    gl_pal_info.rwvecs.reg_write = pal_hw_reg_write;
    gl_pal_info.rwvecs.mem_read = pal_hw_mem_read;
    gl_pal_info.rwvecs.mem_write = pal_hw_mem_write;
    gl_pal_info.rwvecs.ring_doorbell = pal_hw_ring_doorbell;

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_init (void)
{
    pal_ret_t   rv;

    rv = pal_hw_init_rwvectors();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    rv = pal_hw_mmap_device();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk
