/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <unistd.h>
#include <marvell.hpp>
extern "C" {
#include "platform/pal/include/pal_cpld.h"
}

#define MARVELL_SMI_CMD_REG             0x18
#define MARVELL_SMI_DATA_REG            0x19
#define MARVELL_SMI_PHY_ADDR            0x1C
#define MARVELL_SMI_BUSY                (1 << 15)
#define MARVELL_SMI_MODE                (1 << 12)
#define MARVELL_SMI_READ                (1 << 11)
#define MARVELL_SMI_WRITE               (1 << 10)
#define MARVELL_DEV_BITS                5

namespace sdk {
namespace marvell {

// Get the value to be set in the config register
// for 1G, Full Duplex
static inline uint16_t
marvell_port_cfg_1g (void) {
    return ((0x1 << MARVELL_PORT_CTRL_FORCEDFC_OFFSET)  |
            (0x1 << MARVELL_PORT_CTRL_DPX_OFFSET)       |
            (0x1 << MARVELL_PORT_CTRL_FORCEDDPX_OFFSET) |
            (0x2 << MARVELL_PORT_CTRL_SPEED_OFFSET));
}

static int
marvell_smi_rd (uint8_t addr, uint16_t* data, uint8_t phy)
{
    uint16_t tmp;

    tmp = MARVELL_SMI_BUSY | MARVELL_SMI_MODE | MARVELL_SMI_READ |
          phy << MARVELL_DEV_BITS | addr;

    cpld_mdio_wr(MARVELL_SMI_CMD_REG, tmp, MARVELL_SMI_PHY_ADDR);
    usleep(1000);
    cpld_mdio_rd(MARVELL_SMI_DATA_REG, data, MARVELL_SMI_PHY_ADDR);

    return 0;
}

static int
marvell_smi_wr (uint8_t addr, uint16_t data, uint8_t phy)
{
    uint16_t tmp;

    cpld_mdio_wr(MARVELL_SMI_DATA_REG, data, MARVELL_SMI_PHY_ADDR);
    usleep(1000);
    tmp = MARVELL_SMI_BUSY | MARVELL_SMI_MODE | MARVELL_SMI_WRITE |
          phy << MARVELL_DEV_BITS | addr;
    cpld_mdio_wr(MARVELL_SMI_CMD_REG, tmp, MARVELL_SMI_PHY_ADDR);
    return 0;
}

// \@brief     set the power mode for serdes
// \@param[in] serdes_port Marvell serdes port
// \@param[in] powerup power up the serdes
// \@return    0 on success, -1 on failure
static int
marvell_serdes_enable (uint8_t serdes_port, bool powerup)
{
    uint16_t data;
    uint8_t addr;

    data = 0x0;
    // read the fiber control register
    addr = MARVELL_FIBER_CTRL_REG;
    marvell_smi_rd(addr, &data, serdes_port);
    if (powerup) {
        // reset for normal operation
        data = data & ~(1 << MARVELL_FIBER_POWERDN_BIT);
    } else {
        // set for power down state
        data = data | (1 << MARVELL_FIBER_POWERDN_BIT);
    }
    // write back data
    marvell_smi_wr(addr, data, serdes_port);
    return 0;
}

// This function is not currently being used, but is here
// so that we can enable it if required in future
#if 0
// \@brief     set the power mode for copper phy
// \@param[in] phy Marvell PHY port
// \@param[in] powerup power up the PHY
// \@return    0 on success, -1 on failure
static int
marvell_phy_enable (uint8_t phy, bool powerup)
{
    uint16_t data;
    uint8_t addr;

    data = 0x0;
    // read the copper control register
    addr = MARVELL_PHY_CU_CTRL_REG;
    marvell_smi_rd(addr, &data, phy);
    if (powerup) {
        // reset for normal operation
        data = data & ~(1 << MARVELL_PHY_POWERDN_BIT);
    } else {
        // set for power down state
        data = data | (1 << MARVELL_PHY_POWERDN_BIT);
    }
    // write back data
    marvell_smi_wr(addr, data, phy);
    return 0;
}
#endif

// \@brief     set the forwarding state of Marvell ports
// \@param[in] src_port 0x10-0x16
// \@param[in] port_state disabled/blocking/learning/forwarding
// \@return    0 on success, -1 on failure
static int
marvell_port_enable (uint8_t src_port, uint32_t port_state)
{
    uint8_t marvell_addr;
    uint16_t data_;

    // set PortState in switch port control register
    marvell_addr = MARVELL_SWITCH_PORT_CTRL_REG;

    // read data
    cpld_mdio_rd(marvell_addr, &data_, src_port);

    // reset bits
    data_ = data_ & ~(MARVELL_PORT_STATE_MASK << MARVELL_PORT_STATE_BITSHIFT);

    // set the bits
    data_ = data_ | (port_state << MARVELL_PORT_STATE_BITSHIFT);

    // write data
    cpld_mdio_wr(marvell_addr, data_, src_port);
    return 0;
}

// \@brief     Block traffic from one Marvell port to another.
//             MGMT frames are not blocked.
// \@param[in] src_port 0x10-0x16
// \@param[in] dst_port 0 based port numbers: 0-6
// \@param[in] enable   enable or disable the traffic
// \@return    0 on success, -1 on failure
static int
marvell_set_pvlan (uint8_t src_port, uint8_t dst_port, bool enable)
{
    uint8_t marvell_addr;
    uint16_t data_;

    // port based VLAN map - switch port register, bits 6:0
    marvell_addr = MARVELL_PORT_VLAN_MAP_REG;

    // read data
    cpld_mdio_rd(marvell_addr, &data_, src_port);

    if (enable) {
        data_ = data_ | ((1 << dst_port) & 0xffff);
    } else {
        data_ = data_ & (~(1 << dst_port) & 0xffff);
    }

    // write data
    cpld_mdio_wr(marvell_addr, data_, src_port);
    return 0;
}

// \@brief  Enable the forwarding state of all Marvell ports except
//          the OOB connected port
// \@return 0 on success, -1 on failure
static int
marvell_ports_enable (void)
{
    uint32_t port;

    for (port = MARVELL_PORT0; port <= MARVELL_PORT6; port++) {
        if (port == MARVELL_PORT3) {
            // keep port 3 connected to OOB port in disabled state
            continue;
        }
        marvell_port_enable(port, MARVELL_PORT_STATE_FORWARDING);
    }
    return 0;
}

void
marvell_switch_init (void)
{
    // Force the port that connects to the ASIC to 1G
    cpld_mdio_wr(MARVELL_PORT_CTRL_REG, marvell_port_cfg_1g(), MARVELL_PORT0);

    // If ALOM is present:
    //     - Enable forwarding state of all ports except the RJ45 port
    //     - Power up the serdes on the serdes ports
    if (cpld_reg_rd(CPLD_REGISTER_CTRL) & CPLD_ALOM_PRESENT_BIT) {
        marvell_ports_enable();
        marvell_serdes_enable(MARVELL_SERDES_PORT0, true);
        marvell_serdes_enable(MARVELL_SERDES_PORT1, true);
    }

    // block traffic from port 3 to port 5
    marvell_set_pvlan(MARVELL_PORT3, 5, false);

    // block traffic from port 5 to port 3
    marvell_set_pvlan(MARVELL_PORT5, 3, false);
}

int
marvell_get_port_status (uint8_t port_num, uint16_t *data)
{
    int rc = -1;
    rc = cpld_mdio_rd(MARVELL_PORT_STATUS_REG, data, port_num);
    return rc;
}

}   // namespace marvell
}   // namespace sdk
