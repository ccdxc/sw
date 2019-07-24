
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pal_cpld.h"
#include "pal_locks.h"
#include "pal.h"

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
pal_program_marvell(uint8_t marvell_addr, uint32_t data)
{
    return -1;
}

int
pal_get_cpld_rev(void)
{
    return -1;
}

int
pal_marvell_link_status(uint8_t marvell_addr, uint16_t *data, uint8_t phy)
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
    return -1;
}

void
pal_write_hbm_temp(int data)
{
    return -1;
}

void
pal_write_board_temp(int data)
{
    return -1;
}
#else
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
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

#define GPIOHANDLE_GET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x08, struct gpiohandle_data)
#define GPIOHANDLE_SET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x09, struct gpiohandle_data)
#define GPIO_GET_LINEHANDLE_IOCTL _IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIO_GET_LINEEVENT_IOCTL _IOWR(0xB4, 0x04, struct gpioevent_request)

const  int CPLD_FAIL    = -1;
const  int CPLD_SUCCESS = 0;
static int cpld_rev     = -1;
static int cpld_id     = -1;
static const char spidev_path[] = "/dev/spidev0.0";

static int
write_gpios(int gpio, uint32_t data)
{
    struct gpiochip_info ci;
    struct gpiohandle_request hr;
    struct gpiohandle_data hd;
    int fd;

    memset(&hr, 0, sizeof (hr));
    //control only one gpio
    if (gpio > 7) {
        if ((fd = open("/dev/gpiochip1", O_RDWR, 0)) < 0) {
            return CPLD_FAIL;
        }
        hr.lineoffsets[0] = gpio - 7;
    } else {
        if ((fd = open("/dev/gpiochip0", O_RDWR, 0)) < 0) {
            return CPLD_FAIL;
        }
        hr.lineoffsets[0] = gpio;
    }
    if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci) < 0) {
        close(fd);
        return CPLD_FAIL;
    }
    hr.flags = GPIOHANDLE_REQUEST_OUTPUT;
    hr.lines = 1;
    hd.values[0] = data;
    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &hr) < 0) {
        close(fd);
        return CPLD_FAIL;
    }
    close(fd);
    if (ioctl(hr.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hd) < 0) {
        close(hr.fd);
        return CPLD_FAIL;
    }
    close(hr.fd);
    return CPLD_SUCCESS;
}

static int
read_gpios(int d, uint32_t mask)
{
    struct gpiochip_info ci;
    struct gpiohandle_request hr;
    struct gpiohandle_data hd;
    char buf[32];
    int value, fd, n, i;

    snprintf(buf, sizeof (buf), "/dev/gpiochip%d", d);
    if ((fd = open(buf, O_RDWR, 0)) <  0) {
        return CPLD_FAIL;
    }
    if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci) < 0) {
        close(fd);
        return CPLD_FAIL;
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
        return CPLD_FAIL;
    }
    close(fd);
    if (ioctl(hr.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &hd) < 0) {
        close(hr.fd);
        return CPLD_FAIL;
    }
    close(hr.fd);
    value = 0;
    for (i = hr.lines - 1; i >= 0; i--) {
        value = (value << 1) | (hd.values[i] & 0x1);
    }
    return value;
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
        return CPLD_FAIL;
    }
    if (ioctl(fd, SPI_IOC_MESSAGE(1), msg) < 0) {
        close(fd);
        return CPLD_FAIL;
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
        return CPLD_FAIL;
    }

    if (ioctl(fd, SPI_IOC_MESSAGE(1), msg) < 0) {
        close(fd);
        return CPLD_FAIL;
    }
    close(fd);
    return CPLD_SUCCESS;
}

static int
cpld_reg_bit_set(int reg, int bit)
{
    int cpld_data = 0;
    int mask = 0x01 << bit;
    int rc = CPLD_FAIL;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    cpld_data = cpld_read(reg);
    if (cpld_data == -1) {
        return cpld_data;
    }
    cpld_data |= mask;
    rc = cpld_write(reg, cpld_data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return rc;
}

static int
cpld_reg_bit_reset(int reg, int bit)
{
    int cpld_data = 0;
    int mask = 0x01 << bit;
    int rc = CPLD_FAIL;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    cpld_data = cpld_read(reg);
    if (cpld_data == -1) {
        return cpld_data;
    }
    cpld_data &= ~mask;
    rc = cpld_write(reg, cpld_data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return rc;
}

/* Public APIs */
int
cpld_reg_rd(uint8_t reg)
{
    int value = 0;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    value = cpld_read(reg);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return value;
}

int
cpld_reg_wr(uint8_t reg, uint8_t data)
{
    int rc = CPLD_FAIL;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    rc = cpld_write(reg, data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return rc;
}

int cpld_mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
    uint8_t data_lo, data_hi;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    cpld_write(MDIO_CRTL_HI_REG, addr);
    cpld_write(MDIO_CRTL_LO_REG, (phy << 3) | MDIO_RD_ENA | MDIO_ACC_ENA);
    usleep(1000);
    cpld_write(MDIO_CRTL_LO_REG, 0);
    usleep(1000);
    data_lo = cpld_read(MDIO_DATA_LO_REG);
    data_hi = cpld_read(MDIO_DATA_HI_REG);
    *data = (data_hi << 8) | data_lo;
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
    }
    return CPLD_SUCCESS;
}

int cpld_mdio_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    cpld_write(MDIO_CRTL_HI_REG, addr);
    cpld_write(MDIO_DATA_LO_REG, (data & 0xFF));
    cpld_write(MDIO_DATA_HI_REG, ((data >> 8) & 0xFF));
    cpld_write(MDIO_CRTL_LO_REG, (phy << 3) | MDIO_WR_ENA | MDIO_ACC_ENA);
    usleep(1000);
    cpld_write(MDIO_CRTL_LO_REG, 0);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return CPLD_SUCCESS;
}

int
write_cpld_gpios(int gpio, uint32_t data)
{
    return (write_gpios(gpio, data));
}

int
pal_is_qsfp_port_psnt(int port_no)
{
    int cpld_rd_data = cpld_reg_rd(CPLD_REGISTER_QSFP_CTRL);

    if (port_no == 1) {
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

    if (port == 1) {
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

    if (port == 1) {
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

    if (port == 1) {
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

    if (port == 1) {
        bit = 2;
    } else if (port == 2) {
        bit = 3;
    } else {
        return CPLD_FAIL;
    }
    return cpld_reg_bit_reset(CPLD_REGISTER_QSFP_CTRL, bit);
}

static int
pal_change_qsfp_frequency(uint8_t mask, uint8_t frequency)
{
    uint8_t frequency_orig;
    frequency_orig = cpld_reg_rd(CPLD_REGISTER_QSFP_LED_FREQUENCY);
    frequency_orig = frequency_orig & mask;
    frequency_orig = frequency_orig | frequency;
    return cpld_reg_wr(CPLD_REGISTER_QSFP_LED_FREQUENCY, frequency_orig);
}

int
pal_qsfp_set_led(int port, pal_led_color_t led,
                 pal_led_frequency_t frequency)
{
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
pal_program_marvell(uint8_t marvell_addr, uint32_t data)
{
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return CPLD_FAIL;
    }
    cpld_write(0x7, marvell_addr);
    cpld_write(0x8, (data >> 8) && 0xff);
    cpld_write(0x9, data && 0xff);
    cpld_write(0x6, (0xc << 3) | 0x4 | 0x1);
    cpld_write(0x6, 0);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return CPLD_FAIL;
    }
    return CPLD_SUCCESS;
}

int
pal_marvell_link_status(uint8_t marvell_addr, uint16_t *data, uint8_t phy)
{
    int rc = CPLD_FAIL;
    rc = cpld_mdio_rd(marvell_addr, data, phy);
    *data =  (*data >> 2) & 0x1;
    return rc;
}

int
pal_get_cpld_rev(void)
{
    if (cpld_rev == -1) {
        cpld_rev = cpld_reg_rd(CPLD_REGISTER_REVISION);
    }
    return cpld_rev;
}

int
pal_get_cpld_id(void)
{
    if (cpld_id == -1) {
        cpld_id = cpld_reg_rd(CPLD_REGISTER_ID);
    }
    return cpld_id;
}

int
pal_system_set_led(pal_led_color_t led, pal_led_frequency_t frequency)
{
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

void
pal_write_core_temp(int data)
{
    data = (data > 127) ? 127 : data;
    cpld_reg_wr(CPLD_REGISTER_CORE_TEMP, data);
}

void
pal_write_hbm_temp(int data)
{
    data = (data > 127) ? 127 : data;
    cpld_reg_wr(CPLD_REGISTER_HBM_TEMP, data);
}

void
pal_write_board_temp(int data)
{
    data = (data > 127) ? 127 : data;
    cpld_reg_wr(CPLD_REGISTER_BOARD_TEMP, data);
}
#endif
