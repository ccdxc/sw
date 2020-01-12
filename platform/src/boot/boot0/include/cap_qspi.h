
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CAP_QSPI_H__
#define __CAP_QSPI_H__

#define QSPI_BASE               0x2400ULL
#define QSPI_AHB_BASE           0x70000000ULL
#define QSPI_AHB_LEN            0x08000000ULL
#define QSPI_TRIGGER            0x7fff0000ULL
#define QSPI_TRIGGER_FAKE       (QSPI_BASE + 0xf0)      /* dac2 FastModel */

#define QSPI_CONFIG             0x00
#define QSPI_READ_IR            0x04
#define QSPI_WRITE_IR           0x08
#define QSPI_DEV_DELAY          0x0c
#define QSPI_READ_CAPTURE       0x10
#define QSPI_SIZE_CFG           0x14
#define QSPI_SRAM_PARTS_CFG     0x18
#define QSPI_AHB_TRIGADDR       0x1c
#define QSPI_DMA_CONFIG         0x20
#define QSPI_REMAP_ADDR         0x24
#define QSPI_MODE_BIT_CFG       0x28
#define QSPI_SRAM_FILL_LVL      0x2c
#define QSPI_TX_THRESH          0x30
#define QSPI_RX_THRESH          0x34
#define QSPI_WR_COMPL_CTL       0x38
#define QSPI_POLL_EXPIRE        0x3c
#define QSPI_INT_STATUS         0x40
#define QSPI_INT_MASK           0x44
#define QSPI_LOWER_WP           0x50
#define QSPI_UPPER_WP           0x54
#define QSPI_WRITE_PROTECT      0x58
#define QSPI_INDRD_CTL          0x60
#define QSPI_INDRD_WM           0x64
#define QSPI_INDRD_ADDR         0x68
#define QSPI_INDRD_NBYTES       0x6c
#define QSPI_INDWR_CTL          0x70
#define QSPI_INDWR_WM           0x74
#define QSPI_INDWR_ADDR         0x78
#define QSPI_INDWR_NBYTES       0x7c
#define QSPI_AHB_TRIG_RANGE     0x80
#define QSPI_STIG_MEMBANK       0x8c
#define QSPI_STIG_CMD           0x90
#define QSPI_STIG_ADDR          0x94
#define QSPI_STIG_RDDAT_LO      0xa0
#define QSPI_STIG_RDDAT_HI      0xa4
#define QSPI_STIG_WRDAT_LO      0xa8
#define QSPI_STIG_WRDAT_HI      0xac
#define QSPI_POLLING_STATUS     0xb0

#define QSPI_CONFIG_IDLE            (1 << 31)
#define QSPI_CONFIG_BAUDDIV(n)      ((((n) - 2) >> 1) << 19)
#define QSPI_CONFIG_BAUDDIV_MASK    (0xf << 19)
#define QSPI_CONFIG_ENABLE          (1 << 0)

#define QSPI_READ_CAPTURE_READDELAY(n)      (((n) & 0xf) << 1)
#define QSPI_READ_CAPTURE_READDELAY_MASK    (0xf << 1)

#define QSPI_IO_1BIT                0x0
#define QSPI_IO_2BIT                0x1
#define QSPI_IO_4BIT                0x2

#define QSPI_READ_IR_DUMMY(n)       ((n) << 24)
#define QSPI_READ_IR_DATA_TYPE(n)   ((n) << 16)
#define QSPI_READ_IR_ADDR_TYPE(n)   ((n) << 12)
#define QSPI_READ_IR_INST_TYPE(n)   ((n) << 8)
#define QSPI_READ_IR_OPCODE(n)      (n)

#define QSPI_SRAM_FILL_LVL_RX(x)    ((x) & 0xffff)

#define QSPI_INDRD_CTL_COMPLETE     (1 << 5)
#define QSPI_INDRD_CTL_START        (1 << 0)

#define QSPI_STIG_CMD_OP(n)         ((n) << 24)
#define QSPI_STIG_CMD_READ(n)       ((1 << 23) | (((n) - 1) << 20))
#define QSPI_STIG_CMD_ADDR(n)       ((1 << 19) | (((n) - 1) << 16))
#define QSPI_STIG_CMD_WRITE(n)      ((1 << 15) | (((n) - 1) << 12))
#define QSPI_STIG_CMD_BUSY          (1 << 1)
#define QSPI_STIG_CMD_START         (1 << 0)

#endif
