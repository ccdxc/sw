// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

extern pal_info_t   gl_pal_info;

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
pal_mock_mem_read (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_mem_write (uint64_t addr, uint8_t * data, uint32_t size, uint32_t flags)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_mem_set (uint64_t addr, uint8_t c, uint32_t size, uint32_t flags)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_ring_db16 (uint64_t addr, uint16_t data)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_ring_db32 (uint64_t addr, uint32_t data)
{
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_ring_db64 (uint64_t addr, uint64_t data)
{
    return PAL_RET_OK;
}

inline pal_ret_t
pal_mock_physical_addr_to_virtual_addr(uint64_t phy_addr,
                                       uint64_t *virtual_addr)
{
    *virtual_addr = phy_addr;
    return PAL_RET_OK;;
}

inline pal_ret_t
pal_mock_virtual_addr_to_physical_addr(uint64_t virtual_addr,
                                       uint64_t *phy_addr)
{
    *phy_addr = virtual_addr;
    return PAL_RET_OK;
}

inline bool
pal_mock_is_qsfp_port_present(int port_no)
{
    return true;
}

inline pal_ret_t
pal_mock_qsfp_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
          qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_mock_qsfp_read");
    return PAL_RET_OK;
}

inline pal_ret_t
pal_mock_qsfp_write(const uint8_t *buffer, uint32_t size, uint32_t offset,
           qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_mock_qsfp_write");
    return PAL_RET_OK;
}

inline pal_ret_t
pal_mock_qsfp_dom_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
                       uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_mock_qsfp_dom_read");
    return PAL_RET_OK;
}

pal_ret_t
pal_mock_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read      = pal_mock_reg_read;
    gl_pal_info.rwvecs.reg_write     = pal_mock_reg_write;
    gl_pal_info.rwvecs.mem_read      = pal_mock_mem_read;
    gl_pal_info.rwvecs.mem_write     = pal_mock_mem_write;
    gl_pal_info.rwvecs.mem_set     = pal_mock_mem_set;
    gl_pal_info.rwvecs.ring_db16 = pal_mock_ring_db16;
    gl_pal_info.rwvecs.ring_db32 = pal_mock_ring_db32;
    gl_pal_info.rwvecs.ring_db64 = pal_mock_ring_db64;
    gl_pal_info.rwvecs.physical_addr_to_virtual_addr =
                            pal_mock_physical_addr_to_virtual_addr;
    gl_pal_info.rwvecs.virtual_addr_to_physical_addr =
                            pal_mock_virtual_addr_to_physical_addr;
    gl_pal_info.rwvecs.is_qsfp_port_present = pal_mock_is_qsfp_port_present;
    gl_pal_info.rwvecs.qsfp_read = pal_mock_qsfp_read;
    gl_pal_info.rwvecs.qsfp_write = pal_mock_qsfp_write;
    gl_pal_info.rwvecs.qsfp_dom_read = pal_mock_qsfp_dom_read;
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
