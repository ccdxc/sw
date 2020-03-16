/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __MARVELL_H__
#define __MARVELL_H__

#include <stdint.h>

// -------------------------------------------
// Marvell switch port connections on Naples25
// -------------------------------------------
// Port: Desc                    Interface
// -------------------------------------------
// P0:   Capri                   SGMII
// P1:   SGMII to MTP (Hardware) SGMII
// P2:   No connect
// P3:   RJ-45 (Naples mgmt)     PHY
// P4:   No connect
// P5:   ALOM (BMC/iLO)          RMII
// P6:   No connect

#define MARVELL_NPORTS      7       // number of ports in the marvell switch

#define MARVELL_PORT0       0x10    // ASIC (capri) connection
#define MARVELL_PORT1       0x11      
#define MARVELL_PORT2       0x12
#define MARVELL_PORT3       0x13    // oob_mnic0
#define MARVELL_PORT4       0x14
#define MARVELL_PORT5       0x15    // BMC
#define MARVELL_PORT6       0x16

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

#define MARVELL_PORT_UPDOWN_SHIFT   11          // Single bit
#define MARVELL_PORT_UPDOWN_MASK    0x1

#define MARVELL_PORT_DUPLEX_SHIFT   10          // Single bit
#define MARVELL_PORT_DUPLEX_MASK    0x1

#define MARVELL_PORT_SPEED_SHIFT    8           // Two bit data
#define MARVELL_PORT_SPEED_MASK     0x3

#define MARVELL_PORT_TXPAUSE_SHIFT  5           // Single bit
#define MARVELL_PORT_TXPAUSE_MASK   0x1

#define MARVELL_PORT_FCTRL_SHIFT    4           // Single bit
#define MARVELL_PORT_FCTRL_MASK     0x1

namespace sdk {
namespace marvell {

static inline const char *
marvell_get_descr (uint8_t port)
{
    switch (port) {
        case 0:
            return "Connected to Asic";
            break;
        case 1:
            return "SGMII to MTP";
            break;
        case 3:
            return "Out of band Management";
            break;
        case 5:
            return "BMC";
            break;
        // fall through
    }
    return "Unused";
}

static inline void
marvell_get_status_updown (uint16_t status, bool *up)
{
    *up = (status >> MARVELL_PORT_UPDOWN_SHIFT) & MARVELL_PORT_UPDOWN_MASK;
}

static inline void
marvell_get_status_duplex (uint16_t status, bool *fullduplex)
{
    *fullduplex = (status >> MARVELL_PORT_DUPLEX_SHIFT) &
                  MARVELL_PORT_DUPLEX_MASK;
}

static inline void
marvell_get_status_speed (uint16_t status, uint8_t *speed)
{
    *speed = (status >> MARVELL_PORT_SPEED_SHIFT) & MARVELL_PORT_SPEED_MASK;
}

static inline void
marvell_get_status_txpause (uint16_t status, bool *txpause)
{
    *txpause = (status >> MARVELL_PORT_TXPAUSE_SHIFT) &
               MARVELL_PORT_TXPAUSE_MASK;
}

static inline void
marvell_get_status_flowctrl (uint16_t status, bool *fctrl)
{
    *fctrl = (status >> MARVELL_PORT_FCTRL_SHIFT) & MARVELL_PORT_FCTRL_MASK;
}

// extern functions
void marvell_switch_init(void);
int marvell_get_port_status(uint8_t port_num, uint16_t *data);

}   // namespace marvell
}   // namespace sdk

#endif  /* __MARVELL_H__ */
