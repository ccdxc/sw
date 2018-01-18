// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

extern pal_info_t   gl_pal_info;

#define NELEMS(_arr) sizeof(_arr)/sizeof(_arr[0])

pal_mmap_regions_t
pal_mmap_regions[] = {
    {
        // Registers
        .phy_addr_base = 0x01000000,
        .size          = 0x6effffff,
    },
    {
        // 3GB of HBM
        .phy_addr_base = 0xC0000000,
        .size          = 0xC0000000,
    }
};

inline int
pal_virtual_addr_from_physical_addr(uint64_t phy_addr,
                                    uint64_t *mmap_addr)
{
    uint32_t i      = 0;
    uint64_t offset = 0;

    for (i = 0; i < NELEMS(pal_mmap_regions); i++) {
        if (phy_addr >= pal_mmap_regions[i].phy_addr_base &&
            phy_addr < pal_mmap_regions[i].phy_addr_base +
                       pal_mmap_regions[i].size) {

            offset = phy_addr - pal_mmap_regions[i].phy_addr_base;
            *mmap_addr = pal_mmap_regions[i].virtual_addr_base + offset;

            return 0;
        }
    }

    return -1;
}

pal_ret_t
pal_hw_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    uint64_t mmap_addr = 0x0;

    int valid = pal_virtual_addr_from_physical_addr(addr, &mmap_addr);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. phy_addr: 0x%x\n",
                        __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    uint32_t *mmap_addr_word = (uint32_t *)mmap_addr;

    for (uint32_t word = 0; word < num_words; ++word, ++mmap_addr_word) {
        data[word] = *mmap_addr_word;
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_reg_write(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    uint64_t mmap_addr = 0x0;

    int valid = pal_virtual_addr_from_physical_addr(addr, &mmap_addr);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. phy_addr: 0x%x\n",
                        __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    uint32_t *mmap_addr_word = (uint32_t *)mmap_addr;

    for (uint32_t word = 0; word < num_words; ++word, ++mmap_addr_word) {
        *mmap_addr_word = data[word];
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_mem_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr = 0x0;

    int valid = pal_virtual_addr_from_physical_addr(addr, &mmap_addr);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. phy_addr: 0x%x\n",
                        __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    for (uint32_t i = 0; i < size; i++) {
        data[i] = ((uint8_t *)mmap_addr)[i];
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_hw_mem_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    uint64_t mmap_addr = 0x0;

    int valid = pal_virtual_addr_from_physical_addr(addr, &mmap_addr);

    if (valid == -1) {
        SDK_TRACE_DEBUG("%s Invalid access. phy_addr: 0x%x\n",
                        __FUNCTION__, addr);
        return PAL_RET_NOK;
    }

    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *)mmap_addr)[i] = data[i];
    }

    return PAL_RET_OK;
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
        pal_mmap_regions[i].virtual_addr_base =
                    (uint64_t)mmap(NULL,
                                   pal_mmap_regions[i].size,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED,
                                   devfd,
                                   pal_mmap_regions[i].phy_addr_base);
        SDK_TRACE_DEBUG("MMAP Info fixed: phy_addr_base:0x%x size:%u virtual_addr_base:0x%x",
                        pal_mmap_regions[i].phy_addr_base,
                        pal_mmap_regions[i].size,
                        pal_mmap_regions[i].virtual_addr_base);

        if (pal_mmap_regions[i].virtual_addr_base == 0) {
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
