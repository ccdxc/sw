// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/mman.h>
#include <string.h>
#include "lib/pal/pal.hpp"
#include "lib/pal/pal_internal.hpp"

namespace sdk {
namespace lib {

pal_info_t   gl_pal_info;

static pal_ret_t
pal_init_cfg (bool sim)
{
    memset(&gl_pal_info, 0, sizeof(gl_pal_info));
    gl_pal_info.sim = sim;
    return PAL_RET_OK;
}

pal_ret_t
pal_init (bool sim)
{
    pal_init_cfg(sim);

    if (sim) {
        SDK_TRACE_DEBUG("Initializing PAL in SIM mode");
        return pal_init_sim();
    } else {
        SDK_TRACE_DEBUG("Initializing PAL");
        return pal_hw_init();
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_reg_read (uint64_t addr, uint32_t *data)
{
    return gl_pal_info.rwvecs.reg_read(addr, data);
}

pal_ret_t
pal_reg_write (uint64_t addr, uint32_t data)
{
    return gl_pal_info.rwvecs.reg_write(addr, data);
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
