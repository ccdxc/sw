#ifndef __PAL_H_
#define __PAL_H_

#include <stddef.h>

#include "nic/include/hal_cfg.hpp"
using namespace hal;

typedef enum pal_ret_s {
    PAL_RET_OK,
    PAL_RET_NOK,
} pal_ret_t;

#define IS_PAL_API_SUCCESS(_ret) ((_ret) == PAL_RET_OK)
#define IS_PAL_API_FAILURE(_ret) ((_ret) != PAL_RET_OK)

typedef struct pal_cfg_s {
    bool    sim;
    void    *baseaddr;
    int     devfd;
} pal_cfg_t;

pal_ret_t
pal_init(hal_cfg_t *hal_cfg);

pal_ret_t
pal_reg_read(uint64_t addr, uint32_t *data);

pal_ret_t
pal_reg_write(uint64_t addr, uint32_t data);

pal_ret_t
pal_mem_read(uint64_t addr, uint8_t *data, uint32_t size);

pal_ret_t
pal_mem_write(uint64_t addr, uint8_t *data, uint32_t size);

pal_ret_t
pal_ring_doorbell(uint64_t addr, uint64_t data);

#endif // __PAL_H_
