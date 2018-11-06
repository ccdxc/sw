
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pal.h"

#ifdef __x86_64__
int pal_is_qsfp_port_psnt(int port_no) {
    return 0;
}

int
pal_qsfp_set_port(int port)
{
    return -1;
}

int
pal_qsfp_reset_port(int port)
{
    return -1;
}

int
pal_qsfp_set_low_power_mode(int port) {
    return -1;
}

int
pal_qsfp_reset_low_power_mode(int port) {
    return -1;
}

#else
#include "pal_impl.h"
#include "cpld.h"
#include <assert.h>

const int CPLD_FAIL = -1;
const int CPLD_SUCCESS = 0;

static inline int
cpld_reg_rd(int reg) {
    int cpld_rd_data = 0;
    pal_data_t *pd = pal_get_data();

    if (pd->cpldlckfd <= 0) {
        pd->cpldlckfd = pal_lock_init(CPLDLOCKFILE);
    }

    if (!pal_rd_lock_int(pd->cpldlckfd)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_rd_data = cpld_read(0x02);

    if (!pal_rd_unlock_int(pd->cpldlckfd)) {
        printf("Failed to unlock.\n");
    }

    return cpld_rd_data;
}

static inline int
cpld_reg_bit_set(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;
    pal_data_t *pd = pal_get_data();

    if (pd->cpldlckfd <= 0) {
        pd->cpldlckfd = pal_lock_init(CPLDLOCKFILE);
    }

    if (!pal_wr_lock_int(pd->cpldlckfd)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_data = cpld_read(reg);
    assert(cpld_data >= 0);

    cpld_data |= mask;   
    cpld_write(reg, cpld_data); 


    if (!pal_wr_unlock_int(pd->cpldlckfd)) {
        printf("Failed to unlock.\n");
	return CPLD_FAIL;
    }

    return cpld_data;
}

static inline int
cpld_reg_bit_reset(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;
    pal_data_t *pd = pal_get_data();

    if (pd->cpldlckfd <= 0) {
        pd->cpldlckfd = pal_lock_init(CPLDLOCKFILE);
    }

    if (!pal_wr_lock_int(pd->cpldlckfd)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_data = cpld_read(reg);
    cpld_data &= mask;   
    cpld_write(reg, cpld_data);


    if (!pal_wr_unlock_int(pd->cpldlckfd)) {
        printf("Failed to unlock.\n");
    }

    return cpld_data;
}


/* Public APIs */
int
pal_is_qsfp_port_psnt(int port_no) {
    int cpld_rd_data = cpld_reg_rd(0x02);

    if(port_no == 1) { 
        return ((cpld_rd_data & 0x10) != 0);
    } else if (port_no == 2) {
        return ((cpld_rd_data & 0x20) != 0);
    }

    return CPLD_FAIL;
}

/* Register: 0x2, bit 0 is qsfp port 1, bit 1 is qsfp port 2 */
int
pal_qsfp_set_port(int port)
{
    int bit = 0;

    if(port == 1) {
	bit = 0;
    } else if (port == 2) {
	bit = 1;
    } else {
	return CPLD_FAIL;
    }

    return cpld_reg_bit_set(0x02, bit);
}

int
pal_qsfp_reset_port(int port)
{
    int bit = 0;

    if(port == 1) {
        bit = 0;
    } else if (port == 2) {
        bit = 1;
    } else {
        return CPLD_FAIL;
    }

    return cpld_reg_bit_reset(0x02, bit);
}

/* Register: 0x2, bit 2 is qsfp port 1, bit 3 is qsfp port 2 */
int
pal_qsfp_set_low_power_mode(int port)
{
    int bit = 0;

    if(port == 1) {
        bit = 2;
    } else if (port == 2) {
        bit = 3;
    } else {
        return CPLD_FAIL;
    }

    return cpld_reg_bit_set(0x02, bit);
}

int
pal_qsfp_reset_low_power_mode(int port)
{
    int bit = 0;

    if(port == 1) {
        bit = 2;
    } else if (port == 2) {
        bit = 3;
    } else {
        return CPLD_FAIL;
    }

    return cpld_reg_bit_set(0x02, bit);
}
#endif
