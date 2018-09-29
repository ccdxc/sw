/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CAP_SW_GLUE_H__
#define __CAP_SW_GLUE_H__

/*
 * Glue file between cap_xxx.c ASIC API files and our application.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "nic/sdk/include/sdk/pal.hpp"

#define SW_PRINT printf
#define PLOG_API_MSG(f, ...) do {} while (0)
#define PLOG_MSG(f, ...) do {} while (0)
#define PLOG_ERR(f, ...) do {} while (0)
#define SLEEP(t) usleep(t)
#define sleep(t) usleep(t)

static inline void cap_sw_writereg(uint64_t addr, uint32_t data)
{
    sdk::lib::pal_reg_write(addr, &data, 1);
}

static inline uint32_t cap_sw_readreg(uint64_t addr)
{
    uint32_t data = 0x0;
    sdk::lib::pal_reg_read(addr, &data, 1);
    return data;
}

#endif /* __CAP_SW_GLUE_H__ */
