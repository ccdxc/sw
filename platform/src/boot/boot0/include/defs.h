
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __DEFS_H__
#define __DEFS_H__
    
/*
 * Capri Clocks
 */
#define REF_CLK_FREQ_ASIC           156250000
#define QSPI_CLK_FREQ_ASIC          400000000

/*
 * UART configuration
 */
#define UART_CLK                    REF_CLK_FREQ_ASIC
#define UART_BAUD                   115200

/*
 * Chip type
 */
#define CHIP_TYPE_ASIC  0
#define CHIP_TYPE_HAPS  1
#define CHIP_TYPE_ZEBU  2

/*
 * The active-high force-golden image GPIO locks us in to only trying to boot
 * the golden u-boot, and then golden firmware.
 */
#define GPIO_PIN_FORCE_GOLDEN       2

/*
 * GPIOs for SPI CPLD data
 */
#define CAP_GPIO_CPLD_DATA(x)       (((x) >> 6) & 0xff)

/*
 * U-Boot Magic
 */
#define UBOOT_SIZE_MAGIC            0xfb89090a
#define UBOOT_CRC32_MAGIC           0xd8569817
#define UBOOT_PART_SIZE             (4 << 20)

#define BOOT0_MAGIC                 0x30f29e8b

#ifndef __ASSEMBLY__
struct uboot_header {
    uint32_t    inst;
    uint32_t    size_magic;
    uint32_t    size;
    uint32_t    reserved;
    uint32_t    crc_magic;
    uint32_t    crc;
};
#endif

#endif
