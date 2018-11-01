// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

extern pal_info_t   gl_pal_info;

pal_ret_t
pal_default_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    SDK_TRACE_DEBUG("PAL default register read called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_reg_write(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    SDK_TRACE_DEBUG("PAL default register write called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_mem_read (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    SDK_TRACE_DEBUG("PAL default memory read called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_mem_write (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    SDK_TRACE_DEBUG("PAL default memory write called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_mem_set (uint64_t addr, uint8_t c, uint32_t size, uint32_t flags)
{
    SDK_TRACE_DEBUG("PAL default memset called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_ring_doorbell (uint64_t addr, uint64_t data)
{
    SDK_TRACE_DEBUG("PAL default ring_doorbell called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_physical_addr_to_virtual_addr(uint64_t phy_addr,
                                       uint64_t *virtual_addr)
{
    SDK_TRACE_DEBUG("PAL default ptov called for physical address %lx", phy_addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_virtual_addr_to_physical_addr(uint64_t virtual_addr,
                                          uint64_t *phy_addr)
{
    SDK_TRACE_DEBUG("PAL default vtop called for virtual address %lx", virtual_addr);
    return PAL_RET_OK;
}

static bool
pal_default_is_qsfp_port_present(int port_no)
{
    
    SDK_TRACE_DEBUG("PAL default is_qsfp_port_present.");
    return false;
}

pal_ret_t
pal_default_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read      = pal_default_reg_read;
    gl_pal_info.rwvecs.reg_write     = pal_default_reg_write;
    gl_pal_info.rwvecs.mem_read      = pal_default_mem_read;
    gl_pal_info.rwvecs.mem_write     = pal_default_mem_write;
    gl_pal_info.rwvecs.mem_set       = pal_default_mem_set;
    gl_pal_info.rwvecs.ring_doorbell = pal_default_ring_doorbell;
    gl_pal_info.rwvecs.physical_addr_to_virtual_addr =
                            pal_default_physical_addr_to_virtual_addr;
    gl_pal_info.rwvecs.virtual_addr_to_physical_addr =
                            pal_default_virtual_addr_to_physical_addr;
    gl_pal_info.rwvecs.is_qsfp_port_present =
                            pal_default_is_qsfp_port_present;

    return PAL_RET_OK;
}

pal_ret_t
pal_default_init (void)
{
    pal_ret_t   rv;

    rv = pal_default_init_rwvectors();
    SDK_ASSERT(IS_PAL_API_SUCCESS(rv));

    return PAL_RET_OK;
}

}    // namespace lib
}    // namespace sdk
