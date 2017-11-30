// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "sys/mman.h"

#include "pal.hpp"

#include "nic/include/base.h"
#include "nic/model_sim/include/lib_model_client.h"

pal_cfg_t   gl_pal_cfg;

static pal_ret_t
pal_init_cfg (hal_cfg_t *hal_cfg)
{
    bzero(&gl_pal_cfg, sizeof(gl_pal_cfg));
    gl_pal_cfg.sim = hal_cfg->sim;
    gl_pal_cfg.baseaddr = NULL;
    gl_pal_cfg.devfd = -1;
    return PAL_RET_OK;
}

static pal_ret_t
pal_mmap_device ()
{
    gl_pal_cfg.devfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (gl_pal_cfg.devfd < 0) {
        HAL_TRACE_ERR("Failed to open /dev/mem");
        return PAL_RET_NOK;
    }
    
    // Non HBM Region to mmap()
    // 0x01000000 to 0x6fffffff
    gl_pal_cfg.baseaddr = mmap((void *)0x01000000, 0x6effffff,
                               PROT_READ|PROT_WRITE,
                               MAP_SHARED, gl_pal_cfg.devfd, 0);
    if (gl_pal_cfg.baseaddr == NULL) {
        HAL_TRACE_ERR("Failed to mmap /dev/mem");
        return PAL_RET_NOK;
    }

    return PAL_RET_OK;
}

static pal_ret_t
pal_init_sim ()
{
    int    rc;
    HAL_TRACE_DEBUG("Connecting to ASIC SIM");
    do {
        rc = lib_model_connect();
        if (rc != -1) {
            HAL_TRACE_DEBUG("Connected to the ASIC model...");
            break;
        }
        HAL_TRACE_WARN("Failed to connect to asic, retrying in 1 sec ...");
        sleep(1);
    } while (1);

    return PAL_RET_OK;
}

pal_ret_t
pal_init (hal_cfg_t *hal_cfg)
{
    pal_init_cfg(hal_cfg);

    if (gl_pal_cfg.sim) {
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
    if (!read_reg(addr, *data)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_reg_write (uint64_t addr, uint32_t data)
{
    if (!write_reg(addr, data)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_mem_read (uint64_t addr, uint8_t * data, uint32_t size)
{
    if (!read_mem(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_mem_write (uint64_t addr, uint8_t * data, uint32_t size)
{
    if (!write_mem(addr, data, size)) {
        return PAL_RET_NOK;
    }
    return PAL_RET_OK;
}

pal_ret_t
pal_ring_doorbell (uint64_t addr, uint64_t data)
{
    step_doorbell(addr, data);
    return PAL_RET_OK;
}
