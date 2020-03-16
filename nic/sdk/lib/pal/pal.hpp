// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PAL_HPP__
#define __PAL_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/platform.hpp"
#include "platform/pal/include/pal_types.h"

namespace sdk {
namespace lib {

typedef enum pal_ret_s {
    PAL_RET_OK,
    PAL_RET_NOK,
} pal_ret_t;

typedef enum qsfp_page {
    QSFP_PAGE_HIGH0,
    QSFP_PAGE_HIGH1,
    QSFP_PAGE_HIGH2,
    QSFP_PAGE_HIGH3,
    QSFP_PAGE_LOW,
} qsfp_page_t;

#define MAX_CPLD_REV_OLD  0x6
#define QSFP_PAGE_OFFSET  127
#define MAX_QSFP_RETRIES  5

#define IS_PAL_API_SUCCESS(_ret) ((_ret) == sdk::lib::PAL_RET_OK)
#define IS_PAL_API_FAILURE(_ret) ((_ret) != sdk::lib::PAL_RET_OK)

typedef struct pal_rwvectors_s {
    pal_ret_t   (*reg_read)(uint64_t addr,
                            uint32_t *data,
                            uint32_t num_words);
    pal_ret_t   (*reg_write)(uint64_t addr,
                             uint32_t *data,
                             uint32_t num_words);
    pal_ret_t   (*mem_read)(uint64_t addr, uint8_t *data,
                            uint32_t size, uint32_t flags);
    pal_ret_t   (*mem_write)(uint64_t addr, uint8_t *data,
                             uint32_t size, uint32_t flags);
    pal_ret_t   (*ring_db16)(uint64_t addr, uint16_t data);
    pal_ret_t   (*ring_db32)(uint64_t addr, uint32_t data);
    pal_ret_t   (*ring_db64)(uint64_t addr, uint64_t data);
    pal_ret_t   (*step_cpu_pkt)(const uint8_t* pkt, size_t pkt_len);
    pal_ret_t   (*physical_addr_to_virtual_addr)(uint64_t phy_addr,
                                                 uint64_t *virtual_addr);
    pal_ret_t   (*virtual_addr_to_physical_addr)(uint64_t virtual_addr,
                                                 uint64_t *phy_addr);
    pal_ret_t   (*mem_set)(const uint64_t pa, uint8_t c,
                           const uint32_t sz, uint32_t flags);
    bool        (*is_qsfp_port_present)(int port_no);
    pal_ret_t   (*qsfp_set_port)(int port_no);
    pal_ret_t   (*qsfp_reset_port)(int port_no);
    pal_ret_t   (*qsfp_set_low_power_mode)(int port_no);
    pal_ret_t   (*qsfp_reset_low_power_mode)(int port_no);
    pal_ret_t   (*qsfp_read)(const uint8_t *buffer, uint32_t size,
                             uint32_t offset, qsfp_page_t page,
                             uint32_t nretry, uint32_t port);
    pal_ret_t   (*qsfp_write)(const uint8_t *buffer, uint32_t size,
                             uint32_t offset, qsfp_page_t page,
                             uint32_t nretry, uint32_t port);
    void*       (*mem_map)(uint64_t pa, uint32_t sz);
    void        (*mem_unmap)(void *va);
    pal_ret_t   (*qsfp_set_led)(int port, pal_led_color_t led, pal_led_frequency_t frequency);
    int         (*get_cpld_rev)(void);
    int         (*get_cpld_id)(void);
    int         (*cpld_write_qsfp_temp)(uint32_t temperature, uint32_t port);
    pal_ret_t   (*qsfp_dom_read)(const uint8_t *buffer, uint32_t size,
                                 uint32_t offset, uint32_t nretry, uint32_t port);
} __PACK__ pal_rwvectors_t;

typedef struct pal_info_s {
    platform_type_t platform_type;
    pal_rwvectors_t rwvecs;
} __PACK__ pal_info_t;
extern pal_info_t   gl_pal_info;

pal_ret_t pal_init(platform_type_t platform_type);
pal_ret_t pal_teardown(platform_type_t platform_type);

static inline pal_ret_t
pal_reg_read (uint64_t addr, uint32_t *data, uint32_t num_words = 1)
{
    return gl_pal_info.rwvecs.reg_read(addr, data, num_words);
}

static inline pal_ret_t
pal_reg_write (uint64_t addr, uint32_t *data, uint32_t num_words = 1)
{
    return gl_pal_info.rwvecs.reg_write(addr, data, num_words);
}

static inline pal_ret_t
pal_mem_read (uint64_t addr, uint8_t *data, uint32_t size, uint32_t flags = 0)
{
    return gl_pal_info.rwvecs.mem_read(addr, data, size, flags);
}

static inline pal_ret_t
pal_mem_write (uint64_t addr, uint8_t *data, uint32_t size, uint32_t flags = 0)
{
    return gl_pal_info.rwvecs.mem_write(addr, data, size, flags);
}

static inline pal_ret_t
pal_mem_set (uint64_t paddr, uint8_t data, uint32_t size, uint32_t flags = 0)
{
    return gl_pal_info.rwvecs.mem_set(paddr, data, size, flags);
}

static inline pal_ret_t
pal_ring_db16 (uint64_t addr, uint16_t data)
{
    return gl_pal_info.rwvecs.ring_db16(addr, data);
}

static inline pal_ret_t
pal_ring_db32 (uint64_t addr, uint32_t data)
{
    return gl_pal_info.rwvecs.ring_db32(addr, data);
}

static inline pal_ret_t
pal_ring_db64 (uint64_t addr, uint64_t data)
{
    return gl_pal_info.rwvecs.ring_db64(addr, data);
}

static inline pal_ret_t pal_step_cpu_pkt (const uint8_t* pkt, size_t pkt_len)
{
    return gl_pal_info.rwvecs.step_cpu_pkt(pkt, pkt_len);
}

static inline pal_ret_t
pal_physical_addr_to_virtual_addr (uint64_t  phy_addr, uint64_t  *virtual_addr)
{
    return gl_pal_info.rwvecs.physical_addr_to_virtual_addr(phy_addr,
                                                            virtual_addr);
}

static inline pal_ret_t
pal_virtual_addr_to_physical_addr (uint64_t  virtual_addr, uint64_t  *phy_addr)
{
    return gl_pal_info.rwvecs.virtual_addr_to_physical_addr(virtual_addr, phy_addr);
}

static inline bool
pal_is_qsfp_port_present(int port_no)
{
    return gl_pal_info.rwvecs.is_qsfp_port_present(port_no);
}

static inline pal_ret_t
pal_qsfp_set_port(int port_no)
{
    return gl_pal_info.rwvecs.qsfp_set_port(port_no);
}

static inline pal_ret_t
pal_qsfp_reset_port(int port_no)
{
    return gl_pal_info.rwvecs.qsfp_reset_port(port_no);
}

static inline pal_ret_t
pal_qsfp_set_low_power_mode(int port_no)
{
    return gl_pal_info.rwvecs.qsfp_set_low_power_mode(port_no);
}

static inline pal_ret_t
pal_qsfp_reset_low_power_mode(int port_no)
{
    return gl_pal_info.rwvecs.qsfp_reset_low_power_mode(port_no);
}

static inline pal_ret_t
pal_qsfp_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
              qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    return gl_pal_info.rwvecs.qsfp_read(buffer, size, offset,
                                            page, nretry, port);
}

static inline pal_ret_t
pal_qsfp_write(const uint8_t *buffer, uint32_t size, uint32_t offset,
               qsfp_page_t page, uint32_t nretry, uint32_t port)
{
    return gl_pal_info.rwvecs.qsfp_write(buffer, size, offset,
                                            page, nretry, port);
}

static inline void* 
pal_mem_map(uint64_t pa, uint32_t sz) {
    return gl_pal_info.rwvecs.mem_map(pa, sz);
}

// Since, PAL manages memory in units of logical blocks,
// it is expected that the programmer uses pal_mem_map 
// to map a block of memory she is interested in and
// get the base address of the block. This base
// address is then provided to unmap the memory meant
// for the block.
//
// NOTE : Unmapping any valid VA within a block using
// this API will result in the entire block being unmapped.
static inline void
pal_mem_unmap(void* va) {
    return gl_pal_info.rwvecs.mem_unmap(va);
}

static inline pal_ret_t
pal_qsfp_set_led(int port, pal_led_color_t led, pal_led_frequency_t frequency = LED_FREQUENCY_0HZ)
{
    return gl_pal_info.rwvecs.qsfp_set_led(port, led, frequency);
}

static inline int
pal_get_cpld_rev(void) {
    return gl_pal_info.rwvecs.get_cpld_rev();
}

static inline int
pal_get_cpld_id(void) {
    return gl_pal_info.rwvecs.get_cpld_id();
}

inline bool
pal_cpld_rev_old (void)
{
    return pal_get_cpld_rev() <= MAX_CPLD_REV_OLD? true : false;
}

static inline int
pal_cpld_write_qsfp_temp(uint32_t temperature, uint32_t port) {
    return gl_pal_info.rwvecs.cpld_write_qsfp_temp(temperature, port);
}

static inline pal_ret_t
qsfp_dom_read(const uint8_t *buffer, uint32_t size, uint32_t offset,
              uint32_t nretry, uint32_t port)
{
    return gl_pal_info.rwvecs.qsfp_dom_read(buffer, size, offset,
                                        nretry, port);
}

}    // namespace lib
}    // namespace sdk

using sdk::lib::pal_ret_t;

#endif // __PAL_HPP__

