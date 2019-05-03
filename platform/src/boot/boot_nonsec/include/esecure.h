
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __ESECURE_H__
#define __ESECURE_H__

/*
 * eSecure mailbox registers
 */
#define ESEC_MB_REG_FIFO_DATA        0x00700000 
#define ESEC_MB_REG_RX_STATUS        0x00700044
#define ESEC_MB_REG_TX_HEADER        0x00700050
#define ESEC_MB_REG_RX_HEADER        0x00700054

#define ESEC_MB_REG_RX_STATUS_RXEMPTY   (1 << 21)

/*
 * Boot Success Command ID
 */
#define ESEC_CMD_BOOT_SUCCESS        0x07040000

#endif

