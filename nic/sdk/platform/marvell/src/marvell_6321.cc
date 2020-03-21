/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <unistd.h>
#include <marvell.hpp>
#include "marvell_6321.hpp"
extern "C" {
#include "platform/pal/include/pal_cpld.h"
}

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

// \@brief     set the power mode for copper phy
// \@param[in] phy Marvell PHY port
// \@param[in] powerup power up the PHY
// \@return    0 on success, -1 on failure
int
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

// \@brief      Given a port number return the status of that port
// \@param[in]  port 0 - 6
// \@param[out] data Status of the port.
// \@return    0 on success, -1 on failure
int
marvell_get_port_status (uint8_t port, uint16_t *data)
{
    int rc = -1;
    rc = cpld_mdio_rd(MARVELL_PORT_STATUS_REG, data,
                      MARVELL_PORT0 + port);
    return rc;
}

static uint32_t
marvell_get_port_cntr (uint8_t port, uint8_t counter)
{
    uint16_t data_lo, data_hi;
    uint32_t value;

    data_lo = data_hi = 0;
    port = port + 1;
    cpld_mdio_wr(
        MARVELL_STAT_OPT_REG,
        (MARVELL_STATS_OP_BUSY_READ | port << MARVELL_STATS_OP_PORT_SHIFT) +
        counter, MARVELL_GLOBAL1_PHY_ADDR);
    usleep(1000);
    cpld_mdio_rd(MARVELL_STAT_CNT_LO_REG, &data_lo, MARVELL_GLOBAL1_PHY_ADDR);
    cpld_mdio_rd(MARVELL_STAT_CNT_HI_REG, &data_hi, MARVELL_GLOBAL1_PHY_ADDR);
    value = ((uint32_t)data_hi) << 16 | data_lo;
    return value;
}

// \@brief      Given a port number return the statistics for that port
// \@param[in]  port 0 - 6
// \@param[out] data statistics for that port
// \@return    0 on success, -1 on failure
int
marvell_get_port_stats (uint8_t port, marvell_port_stats_t *stats)
{
    uint32_t low, hi;

    if (port >= MARVELL_NPORTS) {
        return -1;
    }
    // Rx Counters
    low = hi = 0;
    low = marvell_get_port_cntr(port, MARVELL_STATS_IN_GOOD_OCT_LO);
    hi = marvell_get_port_cntr(port, MARVELL_STATS_IN_GOOD_OCT_HI);
    stats->in_good_octets = ((uint64_t)hi) << 32 | low;
    stats->in_bad_octets = marvell_get_port_cntr(port, MARVELL_STATS_IN_BAD_OCT);
    stats->in_unicast = marvell_get_port_cntr(port, MARVELL_STATS_IN_UNICAST);
    stats->in_broadcast = marvell_get_port_cntr(port, MARVELL_STATS_IN_BROADCAST);
    stats->in_multicast = marvell_get_port_cntr(port, MARVELL_STATS_IN_MULTICAST);
    stats->in_pause = marvell_get_port_cntr(port, MARVELL_STATS_IN_PAUSE);
    stats->in_undersize = marvell_get_port_cntr(port, MARVELL_STATS_IN_UNDERSIZE);
    stats->in_fragments = marvell_get_port_cntr(port, MARVELL_STATS_IN_FRAGMENTS);
    stats->in_oversize = marvell_get_port_cntr(port, MARVELL_STATS_IN_OVERSIZE);
    stats->in_jabber = marvell_get_port_cntr(port, MARVELL_STATS_IN_JABBER);
    stats->in_rx_err = marvell_get_port_cntr(port, MARVELL_STATS_IN_RX_ERR);
    stats->in_fcs_err = marvell_get_port_cntr(port, MARVELL_STATS_IN_FCS_ERR);

    // Tx Counters
    low = hi = 0;
    low = marvell_get_port_cntr(port, MARVELL_STATS_OUT_OCTETS_LO);
    hi = marvell_get_port_cntr(port, MARVELL_STATS_OUT_OCTETS_HI);
    stats->out_octets = ((uint64_t)hi) << 32 | low;
    stats->out_unicast = marvell_get_port_cntr(port, MARVELL_STATS_OUT_UNICAST);
    stats->out_broadcast = marvell_get_port_cntr(port, MARVELL_STATS_OUT_BROADCAST);
    stats->out_multicast = marvell_get_port_cntr(port, MARVELL_STATS_OUT_MULTICAST);
    stats->out_fcs_err = marvell_get_port_cntr(port, MARVELL_STATS_OUT_FCS_ERR);
    stats->out_pause = marvell_get_port_cntr(port, MARVELL_STATS_OUT_PAUSE);
    stats->out_collisions = marvell_get_port_cntr(port, MARVELL_STATS_OUT_COLLISION);
    stats->out_deferred = marvell_get_port_cntr(port, MARVELL_STATS_OUT_DEFERRED);
    stats->out_single = marvell_get_port_cntr(port, MARVELL_STATS_OUT_SINGLE);
    stats->out_multiple = marvell_get_port_cntr(port, MARVELL_STATS_OUT_MULTIPLE);
    stats->out_excessive = marvell_get_port_cntr(port, MARVELL_STATS_OUT_EXCESSIVE);
    stats->out_late = marvell_get_port_cntr(port, MARVELL_STATS_OUT_LATE);

    return 0;
}

}   // namespace marvell
}   // namespace sdk
