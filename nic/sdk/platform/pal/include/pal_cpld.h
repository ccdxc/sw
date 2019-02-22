#ifndef _PAL_CPLD_H_
#define _PAL_CPLD_H_
#include <stdint.h>
#include "pal_types.h"

#define CPLD_REGISTER_REVISION    0x00
#define CPLD_REGISTER_QSFP_CTRL   0x02
#define CPLD_REGISTER_QSFP_LED    0x05
#define CPLD_REGISTER_SYSTEM_LED  0x15
#define CPLD_REGISTER_ID          0x80

#define GPIOHANDLES_MAX 64

#define GPIOHANDLE_REQUEST_INPUT        (1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT       (1UL << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW   (1UL << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN   (1UL << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE  (1UL << 4)

#define GPIOHANDLE_GET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x08, struct gpiohandle_data)
#define GPIOHANDLE_SET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x09, struct gpiohandle_data)
#define GPIO_GET_LINEHANDLE_IOCTL _IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIO_GET_LINEEVENT_IOCTL _IOWR(0xB4, 0x04, struct gpioevent_request)

int cpld_read(uint8_t addr);
int cpld_write(uint8_t addr, uint8_t data);

/* CPLD APIs */
int pal_is_qsfp_port_psnt(int port_no);
int pal_qsfp_set_port(int port);
int pal_qsfp_reset_port(int port);
int pal_qsfp_set_low_power_mode(int port);
int pal_qsfp_reset_low_power_mode(int port);
int pal_qsfp_set_led(int port, pal_led_color_t led);
int pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency);
int pal_program_marvell(uint8_t marvell_addr, uint32_t data);
int pal_get_cpld_rev();
int pal_get_cpld_id();

#endif
