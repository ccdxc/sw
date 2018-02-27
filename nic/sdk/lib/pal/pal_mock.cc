// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

extern pal_info_t   gl_pal_info;

inline int
pal_virtual_addr_from_physical_addr(uint64_t phy_addr,
                                    uint64_t *mmap_addr)
{
    return -1;
}

pal_ret_t
pal_mock_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_reg_write(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_mem_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_mem_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_ring_doorbell (uint64_t addr, uint64_t data)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read      = pal_mock_reg_read;
    gl_pal_info.rwvecs.reg_write     = pal_mock_reg_write;
    gl_pal_info.rwvecs.mem_read      = pal_mock_mem_read;
    gl_pal_info.rwvecs.mem_write     = pal_mock_mem_write;
    gl_pal_info.rwvecs.ring_doorbell = pal_mock_ring_doorbell;

    return PAL_RET_OK;
}

pal_ret_t
pal_mock_init (void)
{
    pal_ret_t   rv;

    rv = pal_mock_init_rwvectors();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk
