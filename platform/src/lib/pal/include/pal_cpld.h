#ifndef _PAL_CPLD_H_
#define _PAL_CPLD_H_
#include "pal_types.h"

/* CPLD APIs */
int pal_is_qsfp_port_psnt(int port_no);
int pal_qsfp_set_port(int port);
int pal_qsfp_reset_port(int port);
int pal_qsfp_set_low_power_mode(int port);
int pal_qsfp_reset_low_power_mode(int port);
int pal_qsfp_set_led(int port, pal_qsfp_led_color_t led);
int pal_program_marvell(uint8_t marvell_addr, uint32_t data);

#endif
