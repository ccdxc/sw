// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "sys/mman.h"

#include "pal.hpp"

pal_info_t   gl_pal_info;

static pal_ret_t
pal_init_cfg (bool sim)
{
    bzero(&gl_pal_info, sizeof(gl_pal_info));
    gl_pal_info.sim = sim;
    gl_pal_info.baseaddr = NULL;
    gl_pal_info.devfd = -1;
    return PAL_RET_OK;
}

static pal_ret_t
pal_mmap_device ()
{
    gl_pal_info.devfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (gl_pal_info.devfd < 0) {
        HAL_TRACE_ERR("Failed to open /dev/mem");
        return PAL_RET_NOK;
    }
    
    // Non HBM Region to mmap()
    // 0x01000000 to 0x6fffffff
    gl_pal_info.baseaddr = mmap((void *)0x01000000, 0x6effffff,
                                PROT_READ|PROT_WRITE,
                                MAP_SHARED, gl_pal_info.devfd, 0);
    if (gl_pal_info.baseaddr == NULL) {
        HAL_TRACE_ERR("Failed to mmap /dev/mem");
        return PAL_RET_NOK;
    }

    return PAL_RET_OK;
}

pal_ret_t
pal_init (bool sim)
{
    pal_init_cfg(sim);

    if (sim) {
        HAL_TRACE_DEBUG("Initializing PAL in SIM mode.");
        return pal_init_sim();
    } else {
        HAL_TRACE_DEBUG("Initializing PAL");
        return pal_mmap_device();
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
