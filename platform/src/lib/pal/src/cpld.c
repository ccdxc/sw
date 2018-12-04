
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pal_cpld.h"
#include "pal_locks.h"

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

int
pal_qsfp_set_led(int port, pal_qsfp_led_color_t led) {
    return -1;
}

int
pal_program_marvell(uint8_t marvell_addr, uint32_t data) {
    return -1;
}

int
pal_get_cpld_rev() {
    return -1;
}

int
pal_get_cpld_id() {
    return -1;
}

#else
#include "internal.h"
#include "cpld_int.h"
#include <assert.h>

const  int CPLD_FAIL    = -1;
const  int CPLD_SUCCESS = 0;
static int cpld_rev     = -1;

static inline int
cpld_reg_rd(int reg) {
    int cpld_rd_data = 0;

    if (!pal_rd_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_rd_data = cpld_read(0x02);

    if (!pal_rd_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
    }

    return cpld_rd_data;
}

static inline int
cpld_reg_bit_set(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;

    if (!pal_wr_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_data = cpld_read(reg);
    assert(cpld_data >= 0);

    cpld_data |= mask;   
    cpld_write(reg, cpld_data); 

    if (!pal_wr_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
	return CPLD_FAIL;
    }

    return cpld_data;
}

static inline int
cpld_reg_bit_reset(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;

    if (!pal_wr_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_data = cpld_read(reg);
    cpld_data &= ~mask;   
    cpld_write(reg, cpld_data);


    if (!pal_wr_unlock(CPLDLOCK)) {
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

    return cpld_reg_bit_reset(0x02, bit);
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

    return cpld_reg_bit_set(0x02, bit);
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

    return cpld_reg_bit_reset(0x02, bit);
}

int
pal_qsfp_set_led(int port, pal_qsfp_led_color_t led) {
    switch(port) {
        case 1:
            switch(led) {
                case QSFP_LED_COLOR_GREEN:
                    cpld_reg_bit_reset(0x05, 1); 
                    return cpld_reg_bit_set(0x05, 0);
                case QSFP_LED_COLOR_YELLOW:
                    cpld_reg_bit_reset(0x05, 0);
                    return cpld_reg_bit_set(0x05, 1);
                case QSFP_LED_COLOR_NONE:
                    cpld_reg_bit_reset(0x05, 0);
                    cpld_reg_bit_reset(0x05, 1); 
                    return CPLD_SUCCESS;
                default:
                    return CPLD_FAIL;
            }

        case 2:
            switch(led) {
                case QSFP_LED_COLOR_GREEN:
                    cpld_reg_bit_reset(0x05, 3);
                    return cpld_reg_bit_set(0x05, 2);
                case QSFP_LED_COLOR_YELLOW:
                    cpld_reg_bit_reset(0x05, 2);
                    return cpld_reg_bit_set(0x05, 3);
                case QSFP_LED_COLOR_NONE:
                    cpld_reg_bit_reset(0x05, 2);
                    cpld_reg_bit_reset(0x05, 3);
                    return CPLD_SUCCESS;
                default:
                    return CPLD_FAIL;
            }
       default:
           return CPLD_FAIL;
    }
}

int
pal_program_marvell(uint8_t marvell_addr, uint32_t data) {
    if (!pal_wr_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    cpld_write(0x7, marvell_addr);
    cpld_write(0x8, (data >> 8) && 0xff);
    cpld_write(0x9, data && 0xff);
    cpld_write(0x6, (0xc << 3) | 0x4 | 0x1);
    cpld_write(0x6, 0);

    if (!pal_wr_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
    }

    return CPLD_SUCCESS;
}

int
pal_get_cpld_rev() {
    if (cpld_rev == -1) {
        cpld_rev = cpld_reg_rd(0x0);
    }

    return cpld_rev;
}

int
pal_get_cpld_id() {
    return cpld_reg_rd(0x80);
}

#endif

