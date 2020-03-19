
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include "pal_cpld.h"
#include "pal_locks.h"
#include "pal.h"

#define MDIO_CRTL_LO_REG        0x6
#define MDIO_CRTL_HI_REG        0x7
#define MDIO_DATA_LO_REG        0x8
#define MDIO_DATA_HI_REG        0x9

#define MDIO_ACC_ENA            0x1
#define MDIO_RD_ENA             0x2
#define MDIO_WR_ENA             0x4

#ifdef __x86_64__
int pal_is_qsfp_port_psnt(int port_no)
{
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
pal_qsfp_set_low_power_mode(int port)
{
    return -1;
}

int
pal_qsfp_reset_low_power_mode(int port)
{
    return -1;
}

int
pal_qsfp_set_led(int port, pal_led_color_t led,
                 pal_led_frequency_t frequency)
{
    return -1;
}

int
pal_get_cpld_rev(void)
{
    return -1;
}

int
pal_get_cpld_id(void)
{
    return -1;
}

int
pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency)
{
    return -1;
}

void
pal_write_core_temp(int data)
{
}

void
pal_write_hbm_temp(int data)
{
}

void
pal_write_board_temp(int data)
{
}

void
pal_cpld_reload_reset(void)
{
}

bool
pal_cpld_verify_idcode(void)
{
    return false;
}

int
pal_cpld_erase(void)
{
    return -1;
}

int
pal_cpld_read_flash(uint8_t *buf, uint32_t size)
{
    return -1;
}

int
pal_cpld_write_flash(const uint8_t *buf, uint32_t size, cpld_upgrade_status_cb_t cpld_upgrade_status_cb, void *arg)
{
    return -1;
}

void
pal_power_cycle(void)
{
}

bool
pal_cpld_hwlock_enabled(void)
{
    return false;
}

int
pal_write_qsfp_temp(int data, int port)
{
    return -1;
}

int
pal_write_qsfp_alarm_temp(int data, int port)
{
    return -1;
}

int
pal_write_qsfp_warning_temp(int data, int port)
{
    return -1;
}

void
pal_write_hbmwarning_temp(int data)
{
}

void
pal_write_hbmcritical_temp(int data)
{
}

void
pal_write_hbmfatal_temp(int data)
{
}

int
cpld_reg_rd(uint8_t reg)
{
    return -1;
}

int
cpld_reg_wr(uint8_t reg, uint8_t data)
{
    return -1;
}

int cpld_mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
    return -1;
}

int cpld_mdio_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
    return -1;
}

void
pal_cpld_increment_liveness(void)
{
}

void
pal_cpld_set_port0_link_status(uint8_t status)
{
}

void
pal_cpld_set_port1_link_status(uint8_t status)
{
}

void
pal_cpld_set_card_status(uint8_t status)
{
}
#else
#include <string.h>
/* Public APIs */
int
pal_is_qsfp_port_psnt(int port_no)
{
    return -1;
}

/* Register: 0x2, bit 0 is qsfp port 1, bit 1 is qsfp port 2 */
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

/* Register: 0x2, bit 2 is qsfp port 1, bit 3 is qsfp port 2 */
int
pal_qsfp_set_low_power_mode(int port)
{
    return -1;
}

int
pal_qsfp_reset_low_power_mode(int port)
{
    return -1;
}

int
pal_qsfp_set_led(int port, pal_led_color_t led,
                 pal_led_frequency_t frequency)
{
    return -1;
}

int
pal_get_cpld_rev(void)
{
    return -1;
}

int
pal_get_cpld_id(void)
{
    return -1;
}

bool
pal_cpld_hwlock_enabled(void)
{
    return -1;
}

int
pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency)
{
    return -1;
}

void
pal_write_core_temp(int data)
{
    return;
}

void
pal_write_hbm_temp(int data)
{
    return;
}

void
pal_write_board_temp(int data)
{
    return;
}

void
pal_write_hbmwarning_temp(int data)
{
    return;
}

void
pal_write_hbmcritical_temp(int data)
{
    return;
}

void
pal_write_hbmfatal_temp(int data)
{
    return;
}

int
pal_write_qsfp_temp(int data, int port)
{
    return -1;
}

int
pal_write_qsfp_alarm_temp(int data, int port)
{
    return -1;
}

int
pal_write_qsfp_warning_temp(int data, int port)
{
    return -1;
}

void
pal_cpld_reload_reset(void)
{
    return;
}

bool
pal_cpld_verify_idcode(void)
{
    return -1;
}

int
pal_cpld_erase(void)
{
    return -1;
}

int
pal_cpld_read_flash(uint8_t *buf, uint32_t size)
{
    return -1;
}

int
pal_cpld_write_flash(const uint8_t *buf, uint32_t size, cpld_upgrade_status_cb_t cpld_upgrade_status_cb, void *arg)
{
    return -1;
}

void
pal_power_cycle(void)
{
    return;
}

int
cpld_reg_rd(uint8_t reg)
{
    return -1;
}

int
cpld_reg_wr(uint8_t reg, uint8_t data)
{
    return -1;
}

int cpld_mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
    return -1;
}

int cpld_mdio_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
    return -1;
}

void
pal_cpld_increment_liveness(void)
{
}

void
pal_cpld_set_port0_link_status(uint8_t status)
{
}

void
pal_cpld_set_port1_link_status(uint8_t status)
{
}

void
pal_cpld_set_card_status(uint8_t status)
{
}
#endif
