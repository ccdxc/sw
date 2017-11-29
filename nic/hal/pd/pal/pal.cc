// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "sys/mman.h"

#include "pal.hpp"

#include "nic/include/base.h"
#include "nic/model_sim/include/lib_model_client.h"

pal_cfg_t   gl_pal_cfg;

static int
pal_init_cfg(hal_cfg_t *hal_cfg)
{
    bzero(&gl_pal_cfg, sizeof(gl_pal_cfg));
    gl_pal_cfg.sim = hal_cfg->sim;
    gl_pal_cfg.baseaddr = NULL;
    gl_pal_cfg.devfd = -1;
    return HAL_RET_OK;
}

static int
pal_mmap_device()
{
    gl_pal_cfg.devfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (gl_pal_cfg.devfd < 0) {
        HAL_TRACE_ERR("Failed to open /dev/mem");
        return HAL_RET_ERR;
    }
    
    // Non HBM Region to mmap()
    // 0x01000000 to 0x6fffffff
    gl_pal_cfg.baseaddr = mmap((void *)0x01000000, 0x6effffff,
                               PROT_READ|PROT_WRITE,
                               MAP_SHARED, gl_pal_cfg.devfd, 0);
    if (gl_pal_cfg.baseaddr == NULL) {
        HAL_TRACE_ERR("Failed to mmap /dev/mem");
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

int
pal_init(hal_cfg_t *hal_cfg)
{
    pal_init_cfg(hal_cfg);

    if (gl_pal_cfg.sim) {
        HAL_TRACE_DEBUG("Initializing PAL in SIM mode.");
        return HAL_RET_OK;
    } else {
        HAL_TRACE_DEBUG("Initializing PAL");
        return pal_mmap_device();
    }

    return HAL_RET_OK;
}

bool
pal_reg_read(uint64_t addr, uint32_t& data)
{
    return read_reg(addr, data);
}

bool
pal_reg_write(uint64_t addr, uint32_t  data)
{
    return write_reg(addr, data);
}

bool
pal_mem_read(uint64_t addr, uint8_t * data, uint32_t size)
{
    return read_mem(addr, data, size);
}

bool
pal_mem_write(uint64_t addr, uint8_t * data, uint32_t size)
{
    return write_mem(addr, data, size);
}
