
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __DEFS_H__
#define __DEFS_H__

/*
 * Capri Clocks
 */
#define REFCLK_FREQ_ASIC            156250000

/*
 * UART configuration
 */
#define UART_CLK                    REFCLK_FREQ_ASIC
#define UART_BAUD                   115200

/*
 * Boot addresses in flash.
 */
#define ADDR_FLASH_BOOT             0x70100000

/*
 * Secure if any eFuse bits [511:508] are set.
 */
#define EFUSE_SECURE_BOOT_BIT       508
#define EFUSE_SECURE_BOOT_MASK      0xf

#endif
