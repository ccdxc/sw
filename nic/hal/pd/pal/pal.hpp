#ifndef __PAL_H_
#define __PAL_H_

#include <stddef.h>

#include "nic/include/hal_cfg.hpp"
using namespace hal;

typedef struct pal_cfg_s {
    bool    sim;
    void    *baseaddr;
    int     devfd;
} pal_cfg_t;

int 
pal_init(hal_cfg_t *hal_cfg);

bool
pal_reg_read(uint64_t addr, uint32_t& data);

bool
pal_reg_write(uint64_t addr, uint32_t  data);

bool
pal_mem_read(uint64_t addr, uint8_t * data, uint32_t size);

bool
pal_mem_write(uint64_t addr, uint8_t * data, uint32_t size);

#endif // __PAL_H_
