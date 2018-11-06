// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PAL_HPP__
#define __PAL_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

namespace sdk {
namespace lib {

typedef enum pal_ret_s {
    PAL_RET_OK,
    PAL_RET_NOK,
} pal_ret_t;

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
    pal_ret_t   (*ring_doorbell)(uint64_t addr, uint64_t data);
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
} __PACK__ pal_rwvectors_t;

typedef struct pal_info_s {
    sdk::types::platform_type_t platform_type;
    pal_rwvectors_t             rwvecs;
} __PACK__ pal_info_t;
extern pal_info_t   gl_pal_info;

pal_ret_t pal_init(sdk::types::platform_type_t platform_type);

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
pal_ring_doorbell (uint64_t addr, uint64_t data)
{
    return gl_pal_info.rwvecs.ring_doorbell(addr, data);
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

}    // namespace lib
}    // namespace sdk

#endif // __PAL_HPP__

