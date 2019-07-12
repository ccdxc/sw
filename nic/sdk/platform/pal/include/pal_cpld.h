#ifndef _PAL_CPLD_H_
#define _PAL_CPLD_H_
#include <stdint.h>
#include "pal_types.h"

#define CPLD_REGISTER_REVISION              0x00
#define CPLD_REGISTER_QSFP_CTRL             0x02
#define CPLD_REGISTER_QSFP_LED              0x05
#define CPLD_REGISTER_QSFP_LED_FREQUENCY    0x0F
#define CPLD_REGISTER_SYSTEM_LED            0x15
#define CPLD_REGISTER_CORE_TEMP             0x16
#define CPLD_REGISTER_HBM_TEMP              0x17
#define CPLD_REGISTER_BOARD_TEMP            0x18
#define CPLD_REGISTER_ID                    0x80

//CPLD ID and REV
#define CPLD_NAPLES_100_ID    0x12
#define CPLD_NAPLES_25_ID     0x13
#define CPLD_NAPLES_FORIO_ID  0x14
#define CPLD_NAPLES_VOMERO_ID 0x15
#define CPLD_REV_0            0x0
#define CPLD_REV_8            0x8

//CPLD bits for QSFP LED.
#define QSFP_LED_PORT1_GREEN_ON   0
#define QSFP_LED_PORT1_YELLOW_ON  1
#define QSFP_LED_PORT2_GREEN_ON   2
#define QSFP_LED_PORT2_YELLOW_ON  3

//Defines used for QSFP Frequency.
#define QSFP_PORT1_GREEN_BLINK_SHIFT             0
#define QSFP_PORT1_YELLOW_BLINK_SHIFT            2
#define QSFP_PORT2_GREEN_BLINK_SHIFT             4
#define QSFP_PORT2_YELLOW_BLINK_SHIFT            6

#define QSFP_PORT1_GREEN_BLINK_MASK         ~(0x3 << QSFP_PORT1_GREEN_BLINK_SHIFT)
#define QSFP_PORT1_YELLOW_BLINK_MASK        ~(0x3 << QSFP_PORT1_YELLOW_BLINK_SHIFT)
#define QSFP_PORT2_GREEN_BLINK_MASK         ~(0x3 << QSFP_PORT2_GREEN_BLINK_SHIFT)
#define QSFP_PORT2_YELLOW_BLINK_MASK        ~(0x3 << QSFP_PORT2_YELLOW_BLINK_SHIFT)

//Defines used for MDIO IO operations
#define MDIO_CRTL_LO_REG        0x6
#define MDIO_CRTL_HI_REG        0x7
#define MDIO_DATA_LO_REG        0x8
#define MDIO_DATA_HI_REG        0x9

#define MDIO_ACC_ENA            0x1
#define MDIO_RD_ENA             0x2
#define MDIO_WR_ENA             0x4

//Others.
#define GPIOHANDLES_MAX 64

#define GPIOHANDLE_REQUEST_INPUT        (1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT       (1UL << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW   (1UL << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN   (1UL << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE  (1UL << 4)

int cpld_reg_rd(uint8_t addr);
int cpld_reg_wr(uint8_t addr, uint8_t data);
int cpld_mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy);
int cpld_mdio_wr(uint8_t addr, uint16_t data, uint8_t phy);
int cpld_mdio_smi_rd(uint8_t addr, uint16_t* data, uint8_t phy);
int cpld_mdio_smi_wr(uint8_t addr, uint16_t data, uint8_t phy);
int write_cpld_gpios(int gpio, uint32_t data);
/* CPLD APIs */
int pal_is_qsfp_port_psnt(int port_no);
int pal_qsfp_set_port(int port);
int pal_qsfp_reset_port(int port);
int pal_qsfp_set_low_power_mode(int port);
int pal_qsfp_reset_low_power_mode(int port);
int pal_qsfp_set_led(int port, pal_led_color_t led, pal_led_frequency_t frequency);
int pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency);
int pal_program_marvell(uint8_t marvell_addr, uint32_t data);
int pal_marvell_link_status(uint8_t marvell_addr, uint16_t *data, uint8_t phy);
int pal_get_cpld_rev(void);
int pal_get_cpld_id(void);
void pal_write_core_temp(int data);
void pal_write_hbm_temp(int data);
void pal_write_board_temp(int data);
#endif
