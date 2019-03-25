
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
pal_qsfp_set_led(int port, pal_led_color_t led,
                 pal_led_frequency_t frequency) {
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

int
pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency) {
    return -1;
}

#else
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <linux/spi/spidev.h>

struct gpiohandle_request {
        __u32 lineoffsets[GPIOHANDLES_MAX];
        __u32 flags;
        __u8 default_values[GPIOHANDLES_MAX];
        char consumer_label[32];
        __u32 lines;
        int fd;
};
struct gpiohandle_data {
        __u8 values[GPIOHANDLES_MAX];
};

const  int CPLD_FAIL    = -1;
const  int CPLD_SUCCESS = 0;
static int cpld_rev     = -1;
static const char spidev_path[] = "/dev/spidev0.0";

static int
read_gpios(int d, uint32_t mask)
{
    struct gpiochip_info ci;
    struct gpiohandle_request hr;
    struct gpiohandle_data hd;
    char buf[32];
    int r, fd, n, i;

    snprintf(buf, sizeof (buf), "/dev/gpiochip%d", d);
    if ((fd = open(buf, O_RDWR, 0)) <  0) {
        return -1;
    }
    if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci) < 0) {
        close(fd);
        return -1;
    }
    memset(&hr, 0, sizeof (hr));
    n = 0;
    for (i = 0; i < ci.lines; i++) {
        if (mask & (1 << i)) {
            hr.lineoffsets[n++] = i;
        }
    }
    hr.flags = GPIOHANDLE_REQUEST_INPUT;
    hr.lines = n;
    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &hr) < 0) {
        close(fd);
        return -1;
    }
    close(fd);
    if (ioctl(hr.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &hd) < 0) {
        close(hr.fd);
        return -1;
    }
    close(hr.fd);
    r = 0;
    for (i = hr.lines - 1; i >= 0; i--) {
        r = (r << 1) | (hd.values[i] & 0x1);
    }
    return r;
}

static int
read_cpld_gpios(void)
{
    return (read_gpios(1, 0x3f) << 2) | read_gpios(0, 0xc0);
}

static int
cpld_read(uint8_t addr)
{
    struct spi_ioc_transfer msg[2];
    uint8_t txbuf[4];
    uint8_t rxbuf[1];
    int fd;

    txbuf[0] = 0x0b;
    txbuf[1] = addr;
    txbuf[2] = 0x00;
    rxbuf[0] = 0x00;

    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)txbuf;
    msg[0].len = 3;
    msg[1].rx_buf = (intptr_t)rxbuf;
    msg[1].len = 1;

    if ((fd = open(spidev_path, O_RDWR, 0)) < 0) {
        return -1;
    }
    if (ioctl(fd, SPI_IOC_MESSAGE(1), msg) < 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return read_cpld_gpios();
}

static int
cpld_write(uint8_t addr, uint8_t data)
{
    struct spi_ioc_transfer msg[1];
    uint8_t txbuf[3];
    int fd;

    txbuf[0] = 0x02;
    txbuf[1] = addr;
    txbuf[2] = data;

    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)txbuf;
    msg[0].len = 3;

    if ((fd = open(spidev_path, O_RDWR, 0)) < 0) {
        return -1;
    }

    if (ioctl(fd, SPI_IOC_MESSAGE(1), msg) < 0) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int
cpld_reg_bit_set(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;

    cpld_data = cpld_reg_rd(reg);
    if (cpld_data == -1) {
        return cpld_data;
    }

    cpld_data |= mask;
    return cpld_reg_wr(reg, cpld_data); 

}

static int
cpld_reg_bit_reset(int reg, int bit) {
    int cpld_data = 0;
    int mask = 0x01 << bit;

    cpld_data = cpld_reg_rd(reg);
    if (cpld_data == -1) {
        return cpld_data;
    }

    cpld_data &= ~mask;
    return cpld_reg_wr(reg, cpld_data);

}

int
cpld_reg_rd(uint8_t reg) {
    int rc = 0;

    if (!pal_wr_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    rc = cpld_read(reg);

    if (!pal_wr_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
    }

    return rc;
}

int
cpld_reg_wr(uint8_t reg, uint8_t data) {
    int rc = 0;

    if (!pal_wr_lock(CPLDLOCK)) {
        printf("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }

    rc = cpld_write(reg, data);

    if (!pal_wr_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
    }

    return rc;
}

/* Public APIs */
int
pal_is_qsfp_port_psnt(int port_no) {
    int cpld_rd_data = cpld_reg_rd(CPLD_REGISTER_QSFP_CTRL);

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

    return cpld_reg_bit_reset(CPLD_REGISTER_QSFP_CTRL, bit);
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

    return cpld_reg_bit_set(CPLD_REGISTER_QSFP_CTRL, bit);
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

    return cpld_reg_bit_set(CPLD_REGISTER_QSFP_CTRL, bit);
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

    return cpld_reg_bit_reset(CPLD_REGISTER_QSFP_CTRL, bit);
}

static int
pal_change_qsfp_frequency(uint8_t mask, uint8_t frequency) {
    uint8_t frequency_orig;

    if (frequency < 0 || frequency >= 4) {
        return CPLD_FAIL;
    }

    frequency_orig = cpld_reg_rd(CPLD_REGISTER_QSFP_LED_FREQUENCY);
    frequency_orig = frequency_orig & mask;
    frequency_orig = frequency_orig | frequency;
    return cpld_reg_wr(CPLD_REGISTER_QSFP_LED_FREQUENCY, frequency_orig);
}

int
pal_qsfp_set_led(int port, pal_led_color_t led,
                 pal_led_frequency_t frequency) {
    static uint8_t qsfp_port1_led_color;
    static uint8_t qsfp_port2_led_color;
    static uint8_t qsfp_port1_green_led_frequency;
    static uint8_t qsfp_port1_yellow_led_frequency;
    static uint8_t qsfp_port2_green_led_frequency;
    static uint8_t qsfp_port2_yellow_led_frequency;

    switch(port) {
        case 1:
            //check if the port1 LED has changed color
            if (qsfp_port1_led_color == led) {

                //check if frequency needs to be changed.
                if (led == LED_COLOR_GREEN &&
                    qsfp_port1_green_led_frequency != frequency) {
                        qsfp_port1_green_led_frequency =  frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT1_GREEN_BLINK_MASK,
                                                         frequency << QSFP_PORT1_GREEN_BLINK_SHIFT);
                } else if (led == LED_COLOR_YELLOW &&
                           qsfp_port1_yellow_led_frequency != frequency) {
                        qsfp_port1_yellow_led_frequency =  frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT1_YELLOW_BLINK_MASK,
                                                         frequency << QSFP_PORT1_YELLOW_BLINK_SHIFT);
                } else {
                    return CPLD_FAIL;
                }

            } else {
                qsfp_port1_led_color = led;
                switch(led) {
                    case LED_COLOR_GREEN:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_YELLOW_ON);
                        cpld_reg_bit_set(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_GREEN_ON);
                        qsfp_port1_green_led_frequency = frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT1_GREEN_BLINK_MASK,
                                                         frequency << QSFP_PORT1_GREEN_BLINK_SHIFT);
                    case LED_COLOR_YELLOW:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_GREEN_ON);
                        cpld_reg_bit_set(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_YELLOW_ON);
                        qsfp_port1_yellow_led_frequency = frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT1_YELLOW_BLINK_MASK,
                                                         frequency << QSFP_PORT1_YELLOW_BLINK_SHIFT);
                    case LED_COLOR_NONE:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_GREEN_ON);
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT1_YELLOW_ON);
                        return CPLD_SUCCESS;
                    default:
                        return CPLD_FAIL;
                }
            }
        case 2:
            //check if the port2 LED has changed color
            if (qsfp_port2_led_color == led) {

                //check if frequency needs to be changed.
                if (led == LED_COLOR_GREEN &&
                    qsfp_port2_green_led_frequency != frequency) {
                        qsfp_port2_green_led_frequency =  frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT2_GREEN_BLINK_MASK,
                                                         frequency << QSFP_PORT2_GREEN_BLINK_SHIFT);
                } else if (led == LED_COLOR_YELLOW &&
                           qsfp_port2_yellow_led_frequency != frequency) {
                        qsfp_port2_yellow_led_frequency =  frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT2_YELLOW_BLINK_MASK,
                                                         frequency << QSFP_PORT2_YELLOW_BLINK_SHIFT);
                } else {
                    return CPLD_FAIL;
                }

            } else {
                qsfp_port2_led_color = led;
                switch(led) {
                    case LED_COLOR_GREEN:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_YELLOW_ON);
                        cpld_reg_bit_set(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_GREEN_ON);
                        qsfp_port2_green_led_frequency = frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT2_GREEN_BLINK_MASK,
                                                         frequency << QSFP_PORT2_GREEN_BLINK_SHIFT);
                    case LED_COLOR_YELLOW:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_GREEN_ON);
                        cpld_reg_bit_set(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_YELLOW_ON);
                        qsfp_port2_yellow_led_frequency = frequency;
                        return pal_change_qsfp_frequency((uint8_t)QSFP_PORT2_YELLOW_BLINK_MASK,
                                                         frequency << QSFP_PORT2_YELLOW_BLINK_SHIFT);
                    case LED_COLOR_NONE:
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_GREEN_ON);
                        cpld_reg_bit_reset(CPLD_REGISTER_QSFP_LED, QSFP_LED_PORT2_YELLOW_ON);
                        return CPLD_SUCCESS;
                    default:
                        return CPLD_FAIL;
                }
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

    cpld_reg_wr(0x7, marvell_addr);
    cpld_reg_wr(0x8, (data >> 8) && 0xff);
    cpld_reg_wr(0x9, data && 0xff);
    cpld_reg_wr(0x6, (0xc << 3) | 0x4 | 0x1);
    cpld_reg_wr(0x6, 0);

    if (!pal_wr_unlock(CPLDLOCK)) {
        printf("Failed to unlock.\n");
    }

    return CPLD_SUCCESS;
}

int
pal_get_cpld_rev() {
    if (cpld_rev == -1) {
        cpld_rev = cpld_reg_rd(CPLD_REGISTER_REVISION);
    }

    return cpld_rev;
}

int
pal_get_cpld_id() {
    return cpld_reg_rd(CPLD_REGISTER_ID);
}

int
pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency) {
    switch(led) {
        case LED_COLOR_GREEN:
            switch(frequency) {
                case LED_FREQUENCY_0HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x14);
                case LED_FREQUENCY_2HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x15);
                case LED_FREQUENCY_1HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x16);
                case LED_FREQUENCY_05HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x17);
                default:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x00);
            }

        case LED_COLOR_YELLOW:
            switch(frequency) {
                case LED_FREQUENCY_0HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x20);
                case LED_FREQUENCY_2HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x28);
                case LED_FREQUENCY_1HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x30);
                case LED_FREQUENCY_05HZ:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x38);
                default:
                    return cpld_reg_wr(CPLD_REGISTER_SYSTEM_LED, 0x00);
            }
       default:
           return CPLD_FAIL;
    }
}
#endif
