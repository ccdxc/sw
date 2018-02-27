// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

pal_info_t   gl_pal_info;

static pal_ret_t
pal_init_cfg (sdk::types::platform_type_t platform_type)
{
    memset(&gl_pal_info, 0, sizeof(gl_pal_info));
    gl_pal_info.platform_type = platform_type;
    return PAL_RET_OK;
}

pal_ret_t
pal_init (sdk::types::platform_type_t platform_type)
{
    pal_init_cfg(platform_type);

    switch(platform_type) {
    case sdk::types::platform_type_t::PLATFORM_TYPE_HW:
    case sdk::types::platform_type_t::PLATFORM_TYPE_HAPS:
        SDK_TRACE_DEBUG("Initializing PAL in HW mode\n");
        return pal_hw_init();

    case sdk::types::platform_type_t::PLATFORM_TYPE_SIM:
        SDK_TRACE_DEBUG("Initializing PAL in SIM mode\n");
        return pal_init_sim();

    case sdk::types::platform_type_t::PLATFORM_TYPE_MOCK:
        SDK_TRACE_DEBUG("Initializing PAL in MOCK mode\n");
        return pal_mock_init();

    default:
        break;
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    return gl_pal_info.rwvecs.reg_read(addr, data, num_words);
}

pal_ret_t
pal_reg_write(uint64_t addr, uint32_t *data, uint32_t num_words)
{
    return gl_pal_info.rwvecs.reg_write(addr, data, num_words);
}

pal_ret_t
pal_mem_read (uint64_t addr, uint8_t *data, uint32_t size)
{
    return gl_pal_info.rwvecs.mem_read(addr, data, size);
}

pal_ret_t
pal_mem_write (uint64_t addr, uint8_t *data, uint32_t size)
{
    return gl_pal_info.rwvecs.mem_write(addr, data, size);
}

pal_ret_t
pal_ring_doorbell (uint64_t addr, uint64_t data)
{
    return gl_pal_info.rwvecs.ring_doorbell(addr, data);
}

pal_ret_t
pal_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len)
{
    return gl_pal_info.rwvecs.step_cpu_pkt(pkt, pkt_len);
}

}    // namespace lib
}    // namespace sdk
