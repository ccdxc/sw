/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */
#ifndef __MARVELL_6321_H__
#define __MARVELL_6321_H__

#define MARVELL_PORT0       0x10    // ASIC (capri) connection
#define MARVELL_PORT1       0x11    // Diag/MTP connection
#define MARVELL_PORT2       0x12    // Unused
#define MARVELL_PORT3       0x13    // oob_mnic0, RJ45-1
#define MARVELL_PORT4       0x14    // oob_mnic0, RJ45-2 (only on Naples 100)
#define MARVELL_PORT5       0x15    // BMC
#define MARVELL_PORT6       0x16    // Unused

#define MARVELL_PORT_STATE_DISABLED   0x0
#define MARVELL_PORT_STATE_BLOCKING   0x1
#define MARVELL_PORT_STATE_LEARNING   0x2
#define MARVELL_PORT_STATE_FORWARDING 0x3
#define MARVELL_PORT_STATE_MASK       0x3       // bits 1:0
#define MARVELL_PORT_STATE_BITSHIFT   0x0

#define MARVELL_SWITCH_PORT_CTRL_REG  0x4
#define MARVELL_PORT_VLAN_MAP_REG     0x6

// For accessing SERDES registers
#define MARVELL_SERDES_PORT0          0xC   // port 0
#define MARVELL_SERDES_PORT1          0XD   // port 1

// For accessing PHY registers
#define MARVELL_PHY_PORT0             0x3   // port 3
#define MARVELL_PHY_PORT1             0X4   // port 4

#define MARVELL_PHY_CU_CTRL_REG       0x0
#define MARVELL_PHY_POWERDN_BIT       11

#define MARVELL_FIBER_CTRL_REG        0x0
#define MARVELL_FIBER_POWERDN_BIT     11

#define MARVELL_PORT_STATUS_REG       0x0
#define MARVELL_PORT_CTRL_REG         0x1
#define MARVELL_STAT_OPT_REG          0x1D
#define MARVELL_STAT_CNT_HI_REG       0x1E
#define MARVELL_STAT_CNT_LO_REG       0x1F

// Offsets for information in the port status register
#define MARVELL_PORT_CTRL_SPEED_OFFSET     0x0   // force speed
#define MARVELL_PORT_CTRL_FORCEDDPX_OFFSET 0x2   // force duplex
#define MARVELL_PORT_CTRL_DPX_OFFSET       0x3   // duplex value
#define MARVELL_PORT_CTRL_FORCEDFC_OFFSET  0x6   // force flow contrl
#define MARVELL_PORT_CTRL_FC_OFFSET        0x7   // flow control value

#define MARVELL_SMI_CMD_REG             0x18
#define MARVELL_SMI_DATA_REG            0x19
#define MARVELL_SMI_PHY_ADDR            0x1C
#define MARVELL_SMI_BUSY                (1 << 15)
#define MARVELL_SMI_MODE                (1 << 12)
#define MARVELL_SMI_READ                (1 << 11)
#define MARVELL_SMI_WRITE               (1 << 10)
#define MARVELL_DEV_BITS                5

// Statistics related defines
#define MARVELL_GLOBAL1_PHY_ADDR        0x1B
#define MARVELL_MARVELL_STAT_OPT_REG    0x1D
#define MARVELL_STAT_CNT_HI_REG         0x1E
#define MARVELL_STAT_CNT_LO_REG         0x1F
#define MARVELL_STATS_OP_BUSY           (1 << 15)
#define MARVELL_STATS_OP_READ           (4 << 12)

#define MARVELL_STATS_OP_BUSY_READ      (MARVELL_STATS_OP_BUSY | MARVELL_STATS_OP_READ)
#define MARVELL_STATS_OP_PORT_SHIFT     5
#define MARVELL_STATS_IN_GOOD_OCT_LO    0x0
#define MARVELL_STATS_IN_GOOD_OCT_HI    0x1
#define MARVELL_STATS_IN_BAD_OCT        0x2
#define MARVELL_STATS_IN_UNICAST        0x4
#define MARVELL_STATS_IN_BROADCAST      0x6
#define MARVELL_STATS_IN_MULTICAST      0x7
#define MARVELL_STATS_IN_PAUSE          0x16
#define MARVELL_STATS_IN_UNDERSIZE      0x18
#define MARVELL_STATS_IN_FRAGMENTS      0x19
#define MARVELL_STATS_IN_OVERSIZE       0x1A
#define MARVELL_STATS_IN_JABBER         0x1B
#define MARVELL_STATS_IN_RX_ERR         0x1C
#define MARVELL_STATS_IN_FCS_ERR        0x1D

#define MARVELL_STATS_OUT_FCS_ERR       0x3
#define MARVELL_STATS_OUT_DEFERRED      0x5
#define MARVELL_STATS_OUT_OCTETS_LO     0xE
#define MARVELL_STATS_OUT_OCTETS_HI     0xF
#define MARVELL_STATS_OUT_UNICAST       0x10
#define MARVELL_STATS_OUT_EXCESSIVE     0x11
#define MARVELL_STATS_OUT_MULTICAST     0x12
#define MARVELL_STATS_OUT_BROADCAST     0x13
#define MARVELL_STATS_OUT_SINGLE        0x14
#define MARVELL_STATS_OUT_PAUSE         0x15
#define MARVELL_STATS_OUT_MULTIPLE      0x17
#define MARVELL_STATS_OUT_COLLISION     0x1E
#define MARVELL_STATS_OUT_LATE          0x1F


#endif  /* __MARVELL_6321_H__ */


