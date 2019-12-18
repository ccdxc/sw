// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"
#include "platform/pal/include/pal_types.h"

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
pal_default_ring_db16 (uint64_t addr, uint16_t data)
{
    SDK_TRACE_DEBUG("PAL default ring_doorbell 16 called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_ring_db32 (uint64_t addr, uint32_t data)
{
    SDK_TRACE_DEBUG("PAL default ring_doorbell 32 called for %lx", addr);
    return PAL_RET_OK;
}

pal_ret_t
pal_default_ring_db64 (uint64_t addr, uint64_t data)
{
    SDK_TRACE_DEBUG("PAL default ring_doorbell 64 called for %lx", addr);
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

static pal_ret_t
pal_default_qsfp_set_port(int port_no)
{

    SDK_TRACE_DEBUG("PAL default qsfp_set_port %lx", port_no);
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_qsfp_reset_port(int port_no)
{

    SDK_TRACE_DEBUG("PAL default qsfp_reset_port %lx", port_no);
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_qsfp_set_low_power_mode(int port_no)
{

    SDK_TRACE_DEBUG("PAL default qsfp_set_low_power_mode %lx", port_no);
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_qsfp_reset_low_power_mode(int port_no)
{

    SDK_TRACE_DEBUG("PAL default qsfp_set_low_power_mode %lx", port_no);
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_qsfp_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
          qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_default_qsfp_read");
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_qsfp_write(const uint8_t *buffer, uint32_t size, uint32_t offset,
           qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    SDK_TRACE_DEBUG("PAL default pal_default_qsfp_write");
    return PAL_RET_OK;
}

static void*
pal_default_mem_map(const uint64_t pa, const uint32_t sz)
{
    SDK_TRACE_DEBUG("PAL default pal_default_mem_map called for PA=%lx", pa);
    return NULL;
}

static void
pal_default_mem_unmap(void  *va)
{
    SDK_TRACE_DEBUG("PAL default pal_default_mem_unmap called.");
}

static pal_ret_t
pal_default_qsfp_set_led(int port, pal_led_color_t led, pal_led_frequency_t frequency = LED_FREQUENCY_0HZ)
{
    SDK_TRACE_DEBUG("PAL default pal_default_qsfp_set_led");
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_program_marvell(uint8_t addr, uint32_t data, uint8_t phy)
{
    SDK_TRACE_DEBUG("PAL default program marvell called.");
    return PAL_RET_OK;
}

static pal_ret_t
pal_default_marvell_link_status(uint8_t addr, uint16_t *data, uint8_t phy)
{
    SDK_TRACE_DEBUG("PAL default marvell link status called.");
    return PAL_RET_OK;
}

static int
pal_default_get_cpld_rev(void)
{
    SDK_TRACE_DEBUG("PAL default get cpld_rev");
    return -1;
}

static int
pal_default_get_cpld_id(void)
{
    SDK_TRACE_DEBUG("PAL default get cpld_id");
    return -1;
}

static inline int
pal_default_cpld_write_qsfp_temp(uint32_t temperature, uint32_t port) {
    SDK_TRACE_DEBUG("PAL default write cpld temperature");
    return -1;
}

pal_ret_t
pal_default_init_rwvectors (void)
{
    gl_pal_info.rwvecs.reg_read      = pal_default_reg_read;
    gl_pal_info.rwvecs.reg_write     = pal_default_reg_write;
    gl_pal_info.rwvecs.mem_read      = pal_default_mem_read;
    gl_pal_info.rwvecs.mem_write     = pal_default_mem_write;
    gl_pal_info.rwvecs.mem_set       = pal_default_mem_set;
    gl_pal_info.rwvecs.ring_db16 = pal_default_ring_db16;
    gl_pal_info.rwvecs.ring_db32 = pal_default_ring_db32;
    gl_pal_info.rwvecs.ring_db64 = pal_default_ring_db64;
    gl_pal_info.rwvecs.physical_addr_to_virtual_addr =
                            pal_default_physical_addr_to_virtual_addr;
    gl_pal_info.rwvecs.virtual_addr_to_physical_addr =
                            pal_default_virtual_addr_to_physical_addr;
    gl_pal_info.rwvecs.is_qsfp_port_present =
                            pal_default_is_qsfp_port_present;
    gl_pal_info.rwvecs.qsfp_set_port = pal_default_qsfp_set_port;
    gl_pal_info.rwvecs.qsfp_reset_port = pal_default_qsfp_reset_port;
    gl_pal_info.rwvecs.qsfp_set_low_power_mode =
                                         pal_default_qsfp_set_low_power_mode;
    gl_pal_info.rwvecs.qsfp_reset_low_power_mode =
                                         pal_default_qsfp_reset_low_power_mode;
    gl_pal_info.rwvecs.qsfp_read = pal_default_qsfp_read;
    gl_pal_info.rwvecs.qsfp_write = pal_default_qsfp_write;
    gl_pal_info.rwvecs.mem_map = pal_default_mem_map;
    gl_pal_info.rwvecs.mem_unmap = pal_default_mem_unmap;
    gl_pal_info.rwvecs.qsfp_set_led = pal_default_qsfp_set_led;
    gl_pal_info.rwvecs.program_marvell = pal_default_program_marvell;
    gl_pal_info.rwvecs.marvell_link_status = pal_default_marvell_link_status;
    gl_pal_info.rwvecs.get_cpld_id = pal_default_get_cpld_id;
    gl_pal_info.rwvecs.get_cpld_rev = pal_default_get_cpld_rev;
    gl_pal_info.rwvecs.cpld_write_qsfp_temp = pal_default_cpld_write_qsfp_temp;

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
