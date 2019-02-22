
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>

#define GPIOHANDLES_MAX 64

#define GPIOHANDLE_REQUEST_INPUT        (1UL << 0)
#define GPIOHANDLE_REQUEST_OUTPUT       (1UL << 1)
#define GPIOHANDLE_REQUEST_ACTIVE_LOW   (1UL << 2)
#define GPIOHANDLE_REQUEST_OPEN_DRAIN   (1UL << 3)
#define GPIOHANDLE_REQUEST_OPEN_SOURCE  (1UL << 4)

#define FLASH_TRANS_SIZE				16
#define CPLD_DATA_CACHE_START_ADDR		0x50
#define CPLD_DATA_CACHE_END_ADDR		0x5F
#define CFG_SIZE						(16*2175)

#define MDIO_CRTL_LO_REG		0x6
#define MDIO_CRTL_HI_REG		0x7
#define MDIO_DATA_LO_REG		0x8
#define MDIO_DATA_HI_REG		0x9

#define MDIO_ACC_ENA				0x1
#define MDIO_RD_ENA					0x2
#define MDIO_WR_ENA					0x4

#define SMI_CMD_REG					0x18
#define SMI_DATA_REG				0x19
#define SMI_PHY_ADDR				0x1C
#define SMI_BUSY					(1 << 15)
#define SMI_MODE					(1 << 12)
#define SMI_READ					(1 << 11)
#define SMI_WRITE					(1 << 10)
#define DEV_BITS					5

#define CNT_REG_PKT_OFFSET			0
#define CNT_REG_PORT_OFFSET			5
#define STAT_OPT_REG				0x1D
#define STAT_CNT_HI_REG				0x1E
#define STAT_CNT_LO_REG				0x1F

#define GLOBAL1_PHY_ADDR			0x1B

//#ifndef _GPIO_H_
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
//#endif

#define GPIOHANDLE_GET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x08, struct gpiohandle_data)
#define GPIOHANDLE_SET_LINE_VALUES_IOCTL _IOWR(0xB4, 0x09, struct gpiohandle_data)
#define GPIO_GET_LINEHANDLE_IOCTL _IOWR(0xB4, 0x03, struct gpiohandle_request)
#define GPIO_GET_LINEEVENT_IOCTL _IOWR(0xB4, 0x04, struct gpioevent_request)

static const char spidev_path0[] = "/dev/spidev0.0";
static const char spidev_path1[] = "/dev/spidev0.1";

//lsc commands definition
unsigned char lsc_idcode_cmd[]			= {0xE0, 0x0, 0x0, 0x0};
unsigned char lsc_enable_cmd[]			= {0x74, 0x8, 0x0, 0x0};
unsigned char lsc_erase_cmd[]			= {0x0E, 0x4, 0x0, 0x0};
unsigned char lsc_init_cmd[]			= {0x46, 0x0, 0x0, 0x0};
unsigned char lsc_disable_cmd[]			= {0x26, 0x0, 0x0};
unsigned char lsc_prog_done_cmd[]		= {0x5E, 0x0, 0x0, 0x0};
unsigned char lsc_cfg_add_cmd[]			= {0x46, 0x0, 0x0, 0x0};
//unsigned char lsc_read_cmd[]			= {0x73, 0x0, 0x0, 0x0};
unsigned char lsc_read_cmd[]			= {0x73, 0x0, 0x0, 0x0,
											0x0, 0x0, 0x0, 0x0,
											0x0, 0x0};
//											0x0, 0x0,
//											0x0, 0x0, 0x0, 0x0,
//											0x0, 0x0, 0x0, 0x0
//											};

unsigned char lsc_acc_mode_cmd[]		= {0xC6, 0x8, 0x0};
unsigned char lsc_refresh_cmd[]			= {0x79, 0x0, 0x0};
unsigned char lsc_no_op_cmd[]			= {0xFF, 0xFF, 0xFF, 0xFF};
unsigned char lsc_prog_incr_cmd[]		= {0x70, 0x0, 0x0, 0x0};
unsigned char lsc_status_cmd[]			= {0x3C, 0x0, 0x0, 0x0};

static int _e_ioctl(int fd, const char *name, unsigned long req, void *arg)
{
    int r = ioctl(fd, req, arg);
    if (r < 0) {
        perror(name);
        exit(1);
    }
    return r;
}
#define e_ioctl(fd, req, arg)       _e_ioctl(fd, #req, req, arg)

static int e_open(const char *path, int flags, int mode)
{
    int fd = open(path, flags, mode);
    if (fd < 0) {
        perror(path);
        exit(1);
    }
    return fd;
}

static int
read_gpios(int d, uint32_t mask)
{
    struct gpiochip_info ci;
    struct gpiohandle_request hr;
    struct gpiohandle_data hd;
    char buf[32];
    int r, fd, n, i;

    snprintf(buf, sizeof (buf), "/dev/gpiochip%d", d);
    fd = e_open(buf, O_RDWR, 0);
    e_ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci);
    memset(&hr, 0, sizeof (hr));
    n = 0;
    for (i = 0; i < ci.lines; i++) {
        if (mask & (1 << i)) {
            hr.lineoffsets[n++] = i;
        }
    }
    hr.flags = GPIOHANDLE_REQUEST_INPUT;
    hr.lines = n;
    e_ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &hr);
    close(fd);
    e_ioctl(hr.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &hd);
    close(hr.fd);
    r = 0;
    for (i = hr.lines - 1; i >= 0; i--) {
        r = (r << 1) | (hd.values[i] & 0x1);
    }
    return r;
}

int write_gpios(int gpio, uint32_t data)
{
    struct gpiochip_info ci;
    struct gpiohandle_request hr;
    struct gpiohandle_data hd;
//    char buf[32];
    int fd;

//    snprintf(buf, sizeof (buf), "/dev/gpiochip%d", d);
    memset(&hr, 0, sizeof (hr));
    //control only one gpio
    if(gpio > 7) {
    	fd = e_open("/dev/gpiochip1", O_RDWR, 0);
    	hr.lineoffsets[0] = gpio - 7;
    } else {
    	fd = e_open("/dev/gpiochip0", O_RDWR, 0);
    	hr.lineoffsets[0] = gpio;
    }
    e_ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &ci);


//    n = 0;
//    for (i = 0; i < ci.lines; i++) {
//        if (mask & (1 << i)) {
//            hr.lineoffsets[n++] = i;
//            hd.values[n++] = (data >> i) & 1;
//        }
//    }
    hr.flags = GPIOHANDLE_REQUEST_OUTPUT;
    hr.lines = 1;
    hd.values[0] = data;
    e_ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &hr);
    close(fd);
    e_ioctl(hr.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hd);
    close(hr.fd);

    return 0;
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
    msg[0].speed_hz = 12000000;
    msg[1].rx_buf = (intptr_t)rxbuf;
    msg[1].len = 1;

    fd = e_open(spidev_path0, O_RDWR, 0);
    e_ioctl(fd, SPI_IOC_MESSAGE(2), msg);
    close(fd);

    //printf("0x%02x\n", read_cpld_gpios());
    return read_cpld_gpios();
}

static int
cpld_write(uint8_t addr, uint8_t data)
{
    struct spi_ioc_transfer msg[1];
    uint8_t txbuf[3];
    int fd;//, status;

    txbuf[0] = 0x02;
    txbuf[1] = addr;
    txbuf[2] = data;

    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)txbuf;
    msg[0].len = 3;
    msg[0].speed_hz = 12000000;

    fd = e_open(spidev_path0, O_RDWR, 0);
    e_ioctl(fd, SPI_IOC_MESSAGE(1), msg);
//    fd = open(spidev_path, O_RDWR);
//    if (fd < 0) {
//	perror("open");
//	return 1;
//    }
//
//    status = ioctl(fd, SPI_IOC_MESSAGE(1), msg);
//    if (status < 0) {
//	perror("SPI_IOC_MESSAGE");
//	return -1;
//    }
    close(fd);
    return 0;
}

static int
cpld_write_field(uint8_t addr, uint8_t data, uint8_t offset, uint8_t mask)
{
    uint8_t tmp;
    tmp = cpld_read(addr);
    tmp &= ~(0xFF & (mask << offset));
    tmp |= (data & mask) << offset;
    cpld_write(addr, tmp);
    return 0;
}

static int
cpld_read_flash(uint32_t fd, uint8_t* buf, uint8_t size)
{
    struct spi_ioc_transfer msg[1];
    uint8_t txbuf[size];
//    uint8_t rxbuf[1];

    memset(txbuf, 0, sizeof(txbuf));
    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)txbuf;
    msg[0].len = size;
    msg[0].speed_hz = 12000000;
//    msg[1].rx_buf = (intptr_t)rxbuf;
//    msg[1].len = 1;

//    e_ioctl(fd, SPI_IOC_MESSAGE(1), msg);

    //printf("0x%02x\n", read_cpld_gpios());
    for(int i = 0; i < size; i++)
    {
    	buf[i] = cpld_read(CPLD_DATA_CACHE_END_ADDR - i);
    }
    return 0;
}

static int
cpld_write_flash(uint32_t fd, uint8_t* data, uint32_t size)
{
    struct spi_ioc_transfer msg[1];
//    uint8_t txbuf[size];
//    int status;

//    for(int i = 0; i < size; i++)
//    {
//    	txbuf[i] = data[i];
//    }

    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)data;
    msg[0].len = size;
    msg[0].speed_hz = 12000000;

    e_ioctl(fd, SPI_IOC_MESSAGE(1), msg);

    return 0;
}

static int
cpld_write_rb_flash(uint32_t fd, uint8_t* data, uint32_t size, uint32_t rb_size)
{
    struct spi_ioc_transfer msg[2];
    uint8_t rxbuf[rb_size];
    //int status;

    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)data;
    msg[0].len = size;
    msg[0].speed_hz = 12000000;
    msg[1].rx_buf = (intptr_t)rxbuf;
    msg[1].len = rb_size;

    e_ioctl(fd, SPI_IOC_MESSAGE(2), msg);

    return 0;
}

static int flash_enable(uint32_t fd)
{
	uint32_t status = 0;
	status = cpld_write_flash(fd, lsc_enable_cmd, sizeof(lsc_enable_cmd));
	usleep(1000);
	return status;
}

static int flash_disable(uint32_t fd)
{
	uint32_t status = 0;
	status = cpld_write_flash(fd, lsc_disable_cmd, sizeof(lsc_disable_cmd));
	sleep(1);
	status = cpld_write_flash(fd, lsc_no_op_cmd, sizeof(lsc_no_op_cmd));
	sleep(3);
	return status;
}

static int flash_init(uint32_t fd)
{
	uint32_t status = 0;
	status = cpld_write_flash(fd, lsc_init_cmd, sizeof(lsc_init_cmd));
	usleep(1000);
	return status;
}

static int flash_erase(uint32_t fd)
{
	uint32_t status = 0;
	status = cpld_write_flash(fd, lsc_erase_cmd, sizeof(lsc_erase_cmd));
	sleep(10);
	return status;
}

static int flash_program_done(uint32_t fd)
{
	uint32_t status = 0;
	status = cpld_write_flash(fd, lsc_prog_done_cmd, sizeof(lsc_prog_done_cmd));
	sleep(1);
	return status;
}

static int flash_read(uint32_t fd, uint8_t* buf, uint32_t size)
{
	uint32_t status = 0;
	uint32_t count = 0;

    do {
    	status = cpld_write_rb_flash(fd, lsc_read_cmd, sizeof(lsc_read_cmd), FLASH_TRANS_SIZE);
        if (status)
        {
            printf("Failure. cpld_flash_wr failed.\n");
            return status;
        }
		status = cpld_read_flash(fd, buf+count, FLASH_TRANS_SIZE);
	    usleep(1000);
		if (status)
		{
			printf("Failure. cpld_flash_rd failed.\n");
			return status;
		}
//		for(int i = 0; i < 16; i++)
//			printf("0x%x ", buf[count + i]);
//		printf("\n");

		count += FLASH_TRANS_SIZE;
    } while(count < size);

	return status;
}

static int flash_program(uint32_t fd, uint8_t* buf, uint32_t size)
{
	uint32_t status = 0;
	//int row = 0;

	status = cpld_write_flash(fd, lsc_cfg_add_cmd, sizeof(lsc_cfg_add_cmd));
    if (status)
    {
        printf("Failure.  %s returned %d.\n", __FUNCTION__, status);
        return status;
    }
    usleep(1000);
    uint8_t wr_buf[20] = {0x70, 0x0, 0x0, 0x0};
	do {
	    if(size/FLASH_TRANS_SIZE == 0)
	    {
	    	memset(wr_buf, 0, sizeof(wr_buf));
	    	wr_buf[0] = 0x70;
	    	memcpy(wr_buf+4, buf, size%FLASH_TRANS_SIZE);
	    }
	    else {
			memcpy(wr_buf+4, buf, FLASH_TRANS_SIZE);
	    }

		status = cpld_write_flash(fd, wr_buf, sizeof(wr_buf));
	    if (status)
	    {
	        printf("Failure.  %s returned %d.\n", __FUNCTION__, status);
	        return status;
	    }


//		status = cpld_write_flash(fd, lsc_prog_incr_cmd, sizeof(lsc_prog_incr_cmd));
//	    if (status)
//	    {
//	        printf("Failure.  %s returned %d.\n", __FUNCTION__, status);
//	        return status;
//	    }
//	    if(size/FLASH_TRANS_SIZE == 0)
//	    	status = cpld_write_flash(fd, buf, size%FLASH_TRANS_SIZE);
//	    else
//	    	status = cpld_write_flash(fd, buf, FLASH_TRANS_SIZE);
//	    if (status)
//	    {
//	        printf("Failure.  %s returned %d.\n", __FUNCTION__, status);
//	        return status;
//	    }
	    usleep(1000);
//	    printf("row %d\n", ++row);
	    buf = buf + FLASH_TRANS_SIZE;
	    size -= FLASH_TRANS_SIZE;
	} while(size > 0);

	return status;
}

static int flash_id(uint32_t fd) {
//	cpld_write_flash(fd, lsc_idcode_cmd, sizeof(lsc_idcode_cmd));
	struct spi_ioc_transfer msg[2];
	uint8_t rxbuf[4];
    memset(msg, 0, sizeof (msg));
    msg[0].tx_buf = (intptr_t)lsc_idcode_cmd;
    msg[0].len = 4;
    msg[0].speed_hz = 12000000;

    msg[1].rx_buf = (intptr_t)rxbuf;
    msg[1].len = 4;

    e_ioctl(fd, SPI_IOC_MESSAGE(2), msg);
//	uint8_t id[4];
////	cpld_read(fd, id, sizeof(id));
//	for(int i = 0; i < sizeof(id); i++) {
//
//		printf("%02x ", cpld_read(0x50+i));
//	}
//	printf("\n");

    return 0;
}

//static int
//cpld_write_flash(uint32_t fd, uint8_t* data, uint8_t size)
//{
//	for(int i = 0; i < size; i++)
//	{
//		cpld_write_op(fd, data[i]);
//	}
//}

static int mdio_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
	uint8_t data_lo, data_hi;
	cpld_write(MDIO_CRTL_HI_REG, addr);
	cpld_write(MDIO_CRTL_LO_REG, (phy << 3) | MDIO_RD_ENA | MDIO_ACC_ENA);
	usleep(1000);
	cpld_write(MDIO_CRTL_LO_REG, 0);
	usleep(1000);
	data_lo = cpld_read(MDIO_DATA_LO_REG);
	data_hi = cpld_read(MDIO_DATA_HI_REG);
	*data = (data_hi << 8) | data_lo;

	return 0;
}

static int mdio_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
	cpld_write(MDIO_CRTL_HI_REG, addr);
	cpld_write(MDIO_DATA_LO_REG, (data & 0xFF));
	cpld_write(MDIO_DATA_HI_REG, ((data >> 8) & 0xFF));
	cpld_write(MDIO_CRTL_LO_REG, (phy << 3) | MDIO_WR_ENA | MDIO_ACC_ENA);
	usleep(1000);
	cpld_write(MDIO_CRTL_LO_REG, 0);

	return 0;
}

static int mdio_smi_rd(uint8_t addr, uint16_t* data, uint8_t phy)
{
	uint16_t tmp;

	tmp = SMI_BUSY | SMI_MODE | SMI_READ | phy << DEV_BITS | addr;

	mdio_wr(SMI_CMD_REG, tmp, SMI_PHY_ADDR);
	usleep(1000);
	mdio_rd(SMI_DATA_REG, data, SMI_PHY_ADDR);

	return 0;
}

static int mdio_smi_wr(uint8_t addr, uint16_t data, uint8_t phy)
{
	uint16_t tmp;

	mdio_wr(SMI_DATA_REG, data, SMI_PHY_ADDR);
	usleep(1000);
	tmp = SMI_BUSY | SMI_MODE | SMI_WRITE | phy << DEV_BITS | addr;
	mdio_wr(SMI_CMD_REG, tmp, SMI_PHY_ADDR);

	return 0;
}

static void cap_counter(uint8_t port)
{
	uint16_t data_lo, data_hi;
	port = port + 1;
	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InGoodOctetsLo 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 1, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InGoodOctetsHi 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 2, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InBadOctets 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 4, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InUnicast 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 6, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InBroadcasts 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 7, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InMulticasts 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x16, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InPause 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x18, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InUndersize 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x19, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InFragments 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1A, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InOversize 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1B, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InJabber 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1C, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InRxErr 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1D, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("InFCSErr 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0xE, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("\nOutGoodOctetsLo 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0xF, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutGoodOctetsHi 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x10, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutUnicast 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x13, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutBroadcasts 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x12, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutMulticasts 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x3, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutFCSErr 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x15, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("OutPause 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1E, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Collisions 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x5, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Deferred 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x14, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Single 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x17, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Multiple 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x11, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Excessive 0x%x\n", (data_hi << 8 | data_lo));

	mdio_wr(STAT_OPT_REG, (0xC000 | port << CNT_REG_PORT_OFFSET) + 0x1F, GLOBAL1_PHY_ADDR);
	usleep(1000);
	mdio_rd(STAT_CNT_LO_REG, &data_lo, GLOBAL1_PHY_ADDR);
	mdio_rd(STAT_CNT_HI_REG, &data_hi, GLOBAL1_PHY_ADDR);
	printf("Late 0x%x\n", (data_hi << 8 | data_lo));

}

static void flash_refresh(uint32_t fd)
{
	cpld_write_flash(fd, lsc_refresh_cmd, sizeof(lsc_refresh_cmd));
}
static void
usage(void)
{
    printf("cpld (-r addr | -w addr data | -wf addr data offset mask)\n");
    printf("cpld (-prog input_file | -file output_file | -refresh | -erase | -id)\n");
    printf("cpld (-mdiord addr phy | -mdiowr addr phy | -smird addr phy | -smiwr addr phy data | -cnt port)\n");
    printf("cpld (-led [green|yellow] [on|off])\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    uint8_t addr, data, offset, mask;
 
    if (argc < 2) {
    	usage();
    }
    if (strcmp(argv[1], "-r") == 0) {
    	int iter = 1;
        addr = strtoul(argv[2], NULL, 0);
        if(argc == 5) {
        	if(strcmp(argv[3], "-c") == 0)
        		iter = strtoul(argv[4], NULL, 0);
        	else
        		usage();
        }
        for(int i = 0; i < iter; i++) {
			data = cpld_read(addr);
			printf("0x%x\n", data);
        }
    } else if (strcmp(argv[1], "-w") == 0) {
        if (argc < 4) {
            usage();
        }
        addr = strtoul(argv[2], NULL, 0);
        data = strtoul(argv[3], NULL, 0);
        cpld_write(addr, data);
    } else if (strcmp(argv[1], "-wf") == 0) {
        addr = strtoul(argv[2], NULL, 0);
        data = strtoul(argv[3], NULL, 0);
        offset = strtoul(argv[4], NULL, 0);
        mask = strtoul(argv[5], NULL, 0);
        cpld_write_field(addr, data, offset, mask);
    } else if (strcmp(argv[1], "-prog") == 0) {
    	unsigned char buf[2000000];
		memset(buf, 0, sizeof(buf));
		FILE* fptr = fopen(argv[2], "rb");
		if(fptr == NULL)
		{
			printf("Cannot open file %s\n", argv[2]);
			exit(1);
		}
		int read_byte = fread(buf, 1, sizeof(buf), fptr);
		printf("program size %d\n", read_byte);
		uint32_t fd = e_open(spidev_path1, O_RDWR, 0);
    	flash_enable(fd);
    	flash_init(fd);
    	flash_erase(fd);
    	flash_program(fd, buf, read_byte);
		flash_program_done(fd);
		flash_disable(fd);
		close(fd);
		fclose(fptr);
    } else if (strcmp(argv[1], "-erase") == 0) {
		uint32_t fd = e_open(spidev_path1, O_RDWR, 0);
    	flash_enable(fd);
    	flash_init(fd);
    	flash_erase(fd);
		flash_disable(fd);
		close(fd);
    } else if (strcmp(argv[1], "-file") == 0) {
    	FILE* fptr = fopen(argv[2], "wb");
		if(fptr == NULL)
		{
			printf("Cannot create file %s\n", argv[2]);
			exit(1);
		}
		uint32_t fd = e_open(spidev_path1, O_RDWR, 0);
    	flash_enable(fd);
    	flash_init(fd);
    	uint8_t buf[CFG_SIZE];
    	memset(buf, 0, sizeof(buf));
    	flash_read(fd, buf, sizeof(buf));
    	flash_disable(fd);
    	fwrite(buf, sizeof(buf), 1, fptr);
		close(fd);
		fclose(fptr);
    } else if (strcmp(argv[1], "-id") == 0) {
    	uint32_t fd = e_open(spidev_path1, O_RDWR, 0);
    	flash_id(fd);
    	close(fd);
    } else if (strcmp(argv[1], "-mdiord") == 0) {
    	addr = strtoul(argv[2], NULL, 0);
    	uint8_t phy = strtoul(argv[3], NULL, 0);
    	uint16_t data;
    	mdio_rd(addr, &data, phy);
    	printf("0x%x\n", data);
    } else if (strcmp(argv[1], "-mdiowr") == 0) {
    	addr = strtoul(argv[2], NULL, 0);
    	uint8_t phy = strtoul(argv[3], NULL, 0);
    	uint16_t data = strtoul(argv[4], NULL, 0);
    	mdio_wr(addr, data, phy);
    } else if (strcmp(argv[1], "-smird") == 0) {
    	addr = strtoul(argv[2], NULL, 0);
    	uint8_t phy = strtoul(argv[3], NULL, 0);
    	uint16_t data;
    	mdio_smi_rd(addr, &data, phy);
    	printf("0x%x\n", data);
    } else if (strcmp(argv[1], "-smiwr") == 0) {
    	addr = strtoul(argv[2], NULL, 0);
    	uint8_t phy = strtoul(argv[3], NULL, 0);
    	uint16_t data = strtoul(argv[4], NULL, 0);
    	mdio_smi_wr(addr, data, phy);
    } else if (strcmp(argv[1], "-cnt") == 0) {
    	uint8_t port = strtoul(argv[2], NULL, 0);
    	cap_counter(port);
    } else if (strcmp(argv[1], "-refresh") == 0) {
    	uint32_t fd = e_open(spidev_path1, O_RDWR, 0);
    	flash_refresh(fd);
    	close(fd);
    } else if (strcmp(argv[1], "-gpiowr") == 0) {
    	uint8_t gpio = strtoul(argv[2], NULL, 0);
    	uint8_t data = strtoul(argv[3], NULL, 0);
    	write_gpios(gpio, data);
    } else if (strcmp(argv[1], "-led") == 0) {
    	cpld_write(0x15, 0x12);
    	if (strcmp(argv[2], "green") == 0) {
    		if (strcmp(argv[3], "on") == 0) {
    			write_gpios(4, 1);
    		} else if (strcmp(argv[3], "off") == 0) {
    			write_gpios(4, 0);
    		} else {
    			usage();
    		}
    	} else if (strcmp(argv[2], "yellow") == 0) {
    		if (strcmp(argv[3], "on") == 0) {
    			write_gpios(5, 1);
    		} else if (strcmp(argv[3], "off") == 0) {
    			write_gpios(5, 0);
    		} else {
    			usage();
    		}
    	} else {
    		usage();
    	}
    } else {
        usage();
    }

    return 0;
}
