/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "pal_cpld.h"
#include "pal_locks.h"
#include "pal.h"
#ifdef __x86_64__
void
cpld_reset(void)
{
}

bool cpld_verify_idcode(void)
{
    return false;
}

int cpld_erase(void)
{
    return -1;
}

int cpld_read_flash(uint8_t *buf, uint32_t len)
{
    return -1;
}

int
cpld_write_flash(const uint8_t *buf, uint32_t len, cpld_upgrade_status_cb_t cpld_upgrade_status_cb, void *arg)
{
    return -1;
}

int cpld_reg_bit_set(int reg, int bit)
{
    return -1;
}

int cpld_reg_bit_reset(int reg, int bit)
{
    return -1;
}

int cpld_reg_rd(uint8_t addr)
{
    return -1;
}

int cpld_reg_wr(uint8_t addr, uint8_t data)
{
    return -1;
}

int
pal_write_gpios(int gpio, uint32_t data)
{
    return -1;
}
#else
#include <string.h>
#include <errno.h>
#include <fcntl.h>
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

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

static const char spidev0_path[] = "/dev/spidev0.0";
static const char spidev1_path[] = "/dev/spidev0.1";
void
cpld_upgrade_status_cb_default(pal_cpld_status_t status, int percentage, void *ctxt)
{
    return;
}

static cpld_upgrade_status_cb_t g_cpld_upgrade_status_cb = cpld_upgrade_status_cb_default;

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
            return -1;
        }
        hr.lineoffsets[0] = gpio - 7;
    } else {
        if ((fd = open("/dev/gpiochip0", O_RDWR, 0)) < 0) {
            return -1;
        }
        hr.lineoffsets[0] = gpio;
    }
    if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci) < 0) {
        close(fd);
        return -1;
    }
    hr.flags = GPIOHANDLE_REQUEST_OUTPUT;
    hr.lines = 1;
    hd.values[0] = data;
    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &hr) < 0) {
        close(fd);
        return -1;
    }
    close(fd);
    if (ioctl(hr.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hd) < 0) {
        close(hr.fd);
        return -1;
    }
    close(hr.fd);
    return 0;
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

    if ((fd = open(spidev0_path, O_RDWR, 0)) < 0) {
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

    if ((fd = open(spidev0_path, O_RDWR, 0)) < 0) {
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
cpld_send_cmd_spi(int fd, const uint8_t* data, uint32_t size)
{
    struct spi_ioc_transfer msg[1];
    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)data;
    msg[0].len = size;
    msg[0].speed_hz = 12000000;
    int rc = 0;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_MESSAGE(1), msg) < 0) {
        rc = -1;
    }
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return rc;
}

static int
cpld_send_cmd_spi_rx(int fd, const uint8_t* data, uint32_t size,
                     uint8_t *rx_data, uint32_t rx_size)
{
    int rc = 0;
    struct spi_ioc_transfer msg[2];
    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)data;
    msg[0].len = size;
    msg[0].speed_hz = 12000000;
    msg[1].rx_buf = (intptr_t)rx_data;
    msg[1].len = rx_size;

    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_MESSAGE(2), msg) < 0) {
        rc = -1;
    }
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return rc;
}
/* Public APIs */
int
cpld_reg_bit_set(int reg, int bit)
{
    int cpld_data = 0;
    int mask = 0x01 << bit;
    int rc = -1;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    cpld_data = cpld_read(reg);
    if (cpld_data == -1) {
        if (!pal_wr_unlock(CPLDLOCK)) {
            pal_mem_trace("Failed to unlock.\n");
            return -1;
        }
        return cpld_data;
    }
    cpld_data |= mask;
    rc = cpld_write(reg, cpld_data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return rc;
}

int
cpld_reg_bit_reset(int reg, int bit)
{
    int cpld_data = 0;
    int mask = 0x01 << bit;
    int rc = -1;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    cpld_data = cpld_read(reg);
    if (cpld_data == -1) {
        if (!pal_wr_unlock(CPLDLOCK)) {
            pal_mem_trace("Failed to unlock.\n");
            return -1;
        }
        return cpld_data;
    }
    cpld_data &= ~mask;
    rc = cpld_write(reg, cpld_data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return rc;
}

int
cpld_reg_rd(uint8_t reg)
{
    int value = 0;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    value = cpld_read(reg);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return value;
}

int
cpld_reg_wr(uint8_t reg, uint8_t data)
{
    int rc = -1;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    rc = cpld_write(reg, data);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return rc;
}

int cpld_mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
    uint8_t data_lo, data_hi;
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
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
    return 0;
}

int cpld_mdio_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
    if (!pal_wr_lock(CPLDLOCK)) {
        pal_mem_trace("Could not lock pal.lck\n");
        return -1;
    }
    cpld_write(MDIO_CRTL_HI_REG, addr);
    cpld_write(MDIO_DATA_LO_REG, (data & 0xFF));
    cpld_write(MDIO_DATA_HI_REG, ((data >> 8) & 0xFF));
    cpld_write(MDIO_CRTL_LO_REG, (phy << 3) | MDIO_WR_ENA | MDIO_ACC_ENA);
    usleep(1000);
    cpld_write(MDIO_CRTL_LO_REG, 0);
    if (!pal_wr_unlock(CPLDLOCK)) {
        pal_mem_trace("Failed to unlock.\n");
        return -1;
    }
    return 0;
}

int
pal_write_gpios(int gpio, uint32_t data)
{
    return (write_gpios(gpio, data));
}

void
cpld_reload_reset(void)
{
    int fd;
    static const uint8_t lsc_refresh_cmd[] = {0x79, 0x0, 0x0};

    // Open the spi device.
    if ((fd = open(spidev1_path, O_RDWR, 0)) < 0) {
        return;
    }
    cpld_send_cmd_spi(fd, lsc_refresh_cmd, sizeof(lsc_refresh_cmd));
    close(fd);
}

static int
busy_check_bit(int fd)
{
    uint8_t completion_status[4];
    unsigned char lsc_completion_status[] = {0xF0, 0x0, 0x0, 0x0};

    for (int counter = 0; counter < CPLD_MAX_RETRY; counter++) {
        if (cpld_send_cmd_spi(fd, lsc_completion_status, sizeof(lsc_completion_status)) == 0) {
            // Delay tempw(50ms)
            usleep(50000);
            for (int i = 0; i < ARRAY_SIZE(completion_status); i++) {
                completion_status[i] = cpld_reg_rd(CPLD_CONF_FLASH_READ_BYTE + ARRAY_SIZE(completion_status) - (i + 1));
            }
            if ((completion_status[0] & CPLD_COMPLETION_STATUS_BIT1) == 0) {
                // command completed
                return 0;
            }
        }
    }
    return -1;
}

bool
cpld_verify_idcode(void)
{
    int fd;
    int idcode[4];
    unsigned char lsc_idcode_cmd[] = {0xE0, 0x0, 0x0, 0x0};

    // Open the spi device.
    if ((fd = open(spidev1_path, O_RDWR, 0)) < 0) {
        return false;
    }

    if (cpld_send_cmd_spi(fd, lsc_idcode_cmd, sizeof(lsc_idcode_cmd)) == 0) {
        for (int i = 0; i < ARRAY_SIZE(idcode); i++) {
            idcode[i] = cpld_reg_rd(CPLD_CONF_FLASH_READ_BYTE + ARRAY_SIZE(idcode) - (i + 1));
        }
        if (idcode[3] == 0x43 &&
            (idcode[2] == 0xb0 || idcode[2] == 0xc0)) {
            pal_mem_trace("Id verified\n");
            close(fd);
            return true;
        }
    }
    pal_mem_trace("Unable to verify id\n");
    close(fd);
    return false;
}

static int
cpld_erase_flash_cmd(int fd)
{
    uint8_t read_status[4];
    static const uint8_t lsc_erase_cmd[]  = {0x0E, 0x4, 0x0, 0x0};
    static const uint8_t lsc_status_cmd[] = {0x3C, 0x0, 0x0, 0x0};

    // Erase the flash
    if (cpld_send_cmd_spi(fd, lsc_erase_cmd, sizeof(lsc_erase_cmd)) == 0) {
        // Delay 1ms
        usleep(1000);
        // Check for busy bit.
        if (busy_check_bit(fd) == -1) {
            pal_mem_trace("cpld_erase_flash_cmd::busy bit check failed\n");
            return -1;
        }
        // Verify if erase actually happened
        if (cpld_send_cmd_spi(fd, lsc_status_cmd, sizeof(lsc_status_cmd)) == 0) {
            for (int i = 0; i < ARRAY_SIZE(read_status); i++) {
                read_status[i] = cpld_reg_rd(CPLD_CONF_FLASH_READ_BYTE + ARRAY_SIZE(read_status) - (i + 1));
            }
            if ((read_status[1] & CPLD_READ_STATUS_MASK) == 0) {
                pal_mem_trace("cpld_erase_flash_cmd::erase done\n");
                return 0;
            }
        }
    }
    pal_mem_trace("cpld_erase_flash_cmd::erase failed\n");
    return -1;
}

int
cpld_erase(void)
{
    int fd;
    static const uint8_t lsc_enable_cmd[]    = {0x74, 0x8, 0x0, 0x0};
    static const uint8_t lsc_disable_cmd[]   = {0x26, 0x0, 0x0};
    static const uint8_t lsc_no_op_cmd[]     = {0xFF, 0xFF, 0xFF, 0xFF};

    //verify device id
    if (cpld_verify_idcode() == false) {
        pal_mem_trace("cpld_erase::verify idcode failed\n");
        return -1;
    }

    // Open the spi device.
    if ((fd = open(spidev1_path, O_RDWR, 0)) < 0) {
        pal_mem_trace("cpld_erase::unable to open the spi dev\n");
        return -1;
    }
    pal_mem_trace("Trying to initialize the device\n");
    // Put the device in ISC_ACCESSED mode
    if (cpld_send_cmd_spi(fd, lsc_enable_cmd, sizeof(lsc_enable_cmd)) == -1) {
        goto error;
    }
    // Delay 1ms
    usleep(1000);
    pal_mem_trace("Device initialized\n");

    // Erase the flash
    if (cpld_erase_flash_cmd(fd) == -1) {
        pal_mem_trace("Error erasing flash\n");
        goto error;
    }

    // Leaving the ISC_Accessed mode
    if (cpld_send_cmd_spi(fd, lsc_disable_cmd, sizeof(lsc_disable_cmd)) == 0) {
        // Delay 2ms
        usleep(2000);
        cpld_send_cmd_spi(fd, lsc_no_op_cmd, sizeof(lsc_no_op_cmd));
        close(fd);
        pal_mem_trace("Device uninitialized\n");
        return 0;
    }

error:
    close(fd);
    return -1;
}

static int
cpld_read_flash_cmd(int fd, uint8_t *buf, uint32_t len)
{
    static const uint8_t lsc_init_cmd[] = {0x46, 0x0, 0x0, 0x0};
    static const uint8_t lsc_read_cmd[] = {0x73, 0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0, 0x0};
    uint32_t count = 0;
    uint8_t rx_data[CPLD_FLASH_TRANS_SIZE];

    memset(rx_data, 0, sizeof(rx_data));

    // Set the initial address for the flash
    if (cpld_send_cmd_spi(fd, lsc_init_cmd, sizeof(lsc_init_cmd)) == 0) {
        // Do the read
        do {
            if (cpld_send_cmd_spi_rx(fd, lsc_read_cmd, sizeof(lsc_read_cmd), rx_data, sizeof(rx_data)) == 0) {
                for (int i = 0; i < CPLD_FLASH_TRANS_SIZE; i++) {
                    buf[i + count] = cpld_reg_rd(CPLD_DATA_CACHE_END_ADDR - i);
                }
            } else {
                // error reading file
                return -1;
            }
            usleep(1000);
            count += CPLD_FLASH_TRANS_SIZE;
        } while(count < len);
        return 0;
    }
    pal_mem_trace("cpld_read_flash_cmd::read flash failed\n");
    return -1;
}

int
cpld_read_flash(uint8_t *buf, uint32_t len)
{
    int fd;
    static const uint8_t lsc_enable_cmd[]    = {0x74, 0x8, 0x0, 0x0};
    static const uint8_t lsc_disable_cmd[]   = {0x26, 0x0, 0x0};
    static const uint8_t lsc_no_op_cmd[]     = {0xFF, 0xFF, 0xFF, 0xFF};

    //verify device id
    if (cpld_verify_idcode() == false) {
        pal_mem_trace("cpld_read_flash::cannot verify id\n");
        return -1;
    }

    // Open the spi device.
    if ((fd = open(spidev1_path, O_RDWR, 0)) < 0) {
        pal_mem_trace("cpld_read_flash::failed to open the spi dev\n");
        return -1;
    }
    // Put the device in ISC_ACCESSED mode
    if (cpld_send_cmd_spi(fd, lsc_enable_cmd, sizeof(lsc_enable_cmd)) == -1) {
        pal_mem_trace("cpld_read_flash::cannot put it in the isc accessed mode\n");
        goto error;
    }
    // Delay 1ms
    usleep(1000);
    pal_mem_trace("Device initialized\n");

    // Read CFG Flash area
    if (cpld_read_flash_cmd(fd, buf, len) == -1) {
        pal_mem_trace("Error reading flash read command failed\n");
        goto error;
    }

    // Leaving the ISC_Accessed mode
    if (cpld_send_cmd_spi(fd, lsc_disable_cmd, sizeof(lsc_disable_cmd)) == 0) {
        // Delay 2ms
        usleep(2000);
        cpld_send_cmd_spi(fd, lsc_no_op_cmd, sizeof(lsc_no_op_cmd));
        close(fd);
        pal_mem_trace("Device uninitialized\n");
        return 0;
    }

error:
    close(fd);
    return -1;
}

static int
cpld_write_flash_cmd(int fd, const uint8_t *buf, uint32_t len, void *arg)
{
    static const uint8_t lsc_init_cmd[] = {0x46, 0x0, 0x0, 0x0};
    uint8_t wr_buf[20] = {0x70, 0x0, 0x0, 0x0, 0x0};
    const uint8_t *buffer;
    uint32_t count = len;
    uint32_t tot_percent;
    uint32_t cur_percent = 0;

    if (len % CPLD_FLASH_TRANS_SIZE == 0) {
        tot_percent = len / CPLD_FLASH_TRANS_SIZE;
    } else {
        tot_percent = len / CPLD_FLASH_TRANS_SIZE + 1;
    }
    buffer = buf;
    // Set the initial address for the flash
    if (cpld_send_cmd_spi(fd, lsc_init_cmd, sizeof(lsc_init_cmd)) == 0) {
        // Do the write
        do {
            if (count / CPLD_FLASH_TRANS_SIZE == 0) {
                if (count) {
                    memset(wr_buf, 0, sizeof(wr_buf));
                    wr_buf[0] = 0x70;
                    memcpy(wr_buf + 4, buffer, count);
                    buffer = buffer + count;
                }
            } else {
                memcpy(wr_buf + 4, buffer, CPLD_FLASH_TRANS_SIZE);
                buffer = buffer + CPLD_FLASH_TRANS_SIZE;
            }
            if (cpld_send_cmd_spi(fd, wr_buf, sizeof(wr_buf)) != 0) {
                // error writing to the cpld
                goto error;
            }
            // Check for busy bit.
            if (busy_check_bit(fd) == -1) {
                goto error;
            }
            count -= CPLD_FLASH_TRANS_SIZE;
            if ((cur_percent * 100)/tot_percent) {
                g_cpld_upgrade_status_cb(PAL_UPGRADE_WRITE_DONE, (cur_percent * 100)/tot_percent, arg);
            }
            cur_percent++;
        } while(count > 0);
        return 0;
    }
error:
    return -1;
}

static int
cpld_program_done_bit_cmd(int fd)
{
    uint8_t read_status[4];
    static const uint8_t lsc_prog_done_cmd[]  = {0x5E, 0x0, 0x0, 0x0};
    static const uint8_t lsc_status_cmd[] = {0x3C, 0x0, 0x0, 0x0};

    // Send program done bit
    if (cpld_send_cmd_spi(fd, lsc_prog_done_cmd, sizeof(lsc_prog_done_cmd)) == 0) {
        // Delay 1ms
        usleep(1000);
        // Check for busy bit.
        if (busy_check_bit(fd) == -1) {
            pal_mem_trace("cpld_program_done_bit_cmd::busy bit check failed\n");
            return -1;
        }
        // Verify if programmed happened actually happened
        if (cpld_send_cmd_spi(fd, lsc_status_cmd, sizeof(lsc_status_cmd)) == 0) {
            for (int i = 0; i < ARRAY_SIZE(read_status); i++) {
                read_status[i] = cpld_reg_rd(CPLD_CONF_FLASH_READ_BYTE + ARRAY_SIZE(read_status) - (i + 1));
            }
            if ((read_status[1] & CPLD_READ_STATUS_MASK) == 0) {
                pal_mem_trace("cpld_program_done_bit_cmd::program done\n");
                return 0;
            }
        }
    }
    return -1;
}

int
cpld_write_flash(const uint8_t *buf, uint32_t len, cpld_upgrade_status_cb_t cpld_upgrade_status_cb, void *arg)
{
    int fd;
    uint8_t *readbuf;
    static const uint8_t lsc_enable_cmd[]    = {0x74, 0x8, 0x0, 0x0};
    static const uint8_t lsc_disable_cmd[]   = {0x26, 0x0, 0x0};
    static const uint8_t lsc_no_op_cmd[]     = {0xFF, 0xFF, 0xFF, 0xFF};

    if (cpld_upgrade_status_cb != NULL) {
        // since no callback provided init the cb to dummy function
        g_cpld_upgrade_status_cb = cpld_upgrade_status_cb;
    }

    //verify device id
    if (cpld_verify_idcode() == false) {
        pal_mem_trace("cpld_write_flash::verify id failed\n");
        return -1;
    }

    // Open the spi device.
    if ((fd = open(spidev1_path, O_RDWR, 0)) < 0) {
        pal_mem_trace("cpld_write_flash::failed to open the spi dev\n");
        return -1;
    }
    // Put the device in ISC_ACCESSED mode
    if (cpld_send_cmd_spi(fd, lsc_enable_cmd, sizeof(lsc_enable_cmd)) == -1) {
        pal_mem_trace("cpld_write_flash::unable to set isc accessed mode\n");
        goto error;
    }
    // Delay 1ms
    usleep(1000);
    pal_mem_trace("Device initialized\n");

    // Erase the flash
    if (cpld_erase_flash_cmd(fd) == -1) {
        pal_mem_trace("Error erasing flash\n");
        goto error;
    }
    pal_mem_trace("Device Erased\n");
    g_cpld_upgrade_status_cb(PAL_UPGRADE_ERASED_DONE, 100, arg);

    // Write the cpld
    if (cpld_write_flash_cmd(fd, buf, len, arg) == -1) {
        pal_mem_trace("Error writing flash\n");
        goto error;
    }
    pal_mem_trace("Device written\n");
    g_cpld_upgrade_status_cb(PAL_UPGRADE_WRITE_DONE, 100, arg);

    // Program done bit
    if (cpld_program_done_bit_cmd(fd) == -1) {
        pal_mem_trace("Error writing program done bit\n");
        goto error;
    }
    pal_mem_trace("Programmed done bit written\n");

    // verify flash
    readbuf = (uint8_t*)calloc(len, sizeof(uint8_t));
    // Read CFG Flash area
    if (cpld_read_flash_cmd(fd, readbuf, len) == -1) {
        pal_mem_trace("Error reading flash read command failed\n");
        goto error;
    }
    if (memcmp(buf, readbuf, len) != 0) {
        pal_mem_trace("Error verifying flash\n");
        goto error;
    }
    pal_mem_trace("flash verified\n");
    g_cpld_upgrade_status_cb(PAL_UPGRADE_VERIFY_DONE, 100, arg);

    // Leaving the ISC_Accessed mode
    if (cpld_send_cmd_spi(fd, lsc_disable_cmd, sizeof(lsc_disable_cmd)) == 0) {
        // Delay 2ms
        usleep(2000);
        cpld_send_cmd_spi(fd, lsc_no_op_cmd, sizeof(lsc_no_op_cmd));
        close(fd);
        pal_mem_trace("Device uninitialized\n");
        g_cpld_upgrade_status_cb(PAL_UPGRADE_COMPLETED, 100, arg);
        return 0;
    }

error:
    close(fd);
    return -1;
}
#endif
