// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "port.hpp"
#include "port_mac.hpp"
#include "linkmgr_rw.hpp"

// TODO: Avinash, please add static to all methods that are local to this file

namespace sdk {
namespace linkmgr {

//---------------------------------------------------------------------------
// HAPS platform methods
//---------------------------------------------------------------------------

int
mac_temac_regrd_haps (uint32_t chip, uint32_t port_num,
                      uint64_t offset, uint32_t *data)
{
    uint64_t addr = 0x0;

    addr = MXP_BASE_HAPS +
           (chip     * MXP_INST_STRIDE_HAPS) +
           (port_num * MXP_PORT_STRIDE_HAPS) +
           TEMAC_BASE_OFFSET_HAPS +
           offset;

    READ_REG_BASE(chip, addr, data);

    return 0;
}

int
mac_temac_regwr_haps (uint32_t chip, uint32_t port_num,
                      uint64_t offset, uint32_t data)
{
    uint64_t addr = 0x0;

    addr = MXP_BASE_HAPS +
           (chip     * MXP_INST_STRIDE_HAPS) +
           (port_num * MXP_PORT_STRIDE_HAPS) +
           TEMAC_BASE_OFFSET_HAPS +
           offset;

    WRITE_REG_BASE(chip, addr, data);

    return 0;
}

int
mac_temac_regrd_words_haps (uint32_t chip, uint32_t port_num,
                            uint32_t offset, uint32_t size,
                            uint32_t *data)
{
    uint32_t i = 0;

    for (i = 0; i < size; ++i) {
        mac_temac_regrd_haps(chip, port_num, offset + (i * 0x4), &data[i]);
    }

    return 0;
}

int
mac_temac_mdio_rd_haps (uint32_t chip, uint32_t port_num, uint32_t phy_addr,
                        uint64_t addr, uint32_t *data)
{
    uint32_t reg_data = 0x0;
    uint32_t mdio_ready = 0x0;

    // TODO use bit offset #defs
    reg_data = (phy_addr << 24) + (addr << 16) + (2 << 14) + (1 << 11);

    // write phy_addr and addr to control register. Initiate read operation
    mac_temac_regwr_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, reg_data);

    //TODO add MDIO timeouts
    reg_data = 0x0;
    while ((mdio_ready == 0) && (g_linkmgr_cfg.mock_hw_access == false)) {
        mac_temac_regrd_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, &reg_data);
        mdio_ready = (reg_data >> 7) & 0x1;
    }

    // read mdio-data register
    mac_temac_regrd_haps(chip, port_num, MDIO_DATA_RD_OFFSET_HAPS, data);

    return 0;
}

int
mac_temac_mdio_wr_haps (uint32_t chip, uint32_t port_num, uint32_t phy_addr,
                        uint64_t addr, uint32_t data)
{
    uint32_t reg_data = 0x0;
    uint32_t mdio_ready = 0x0;

    // write data to mdio-data register
    mac_temac_regwr_haps(chip, port_num, MDIO_DATA_WR_OFFSET_HAPS, data);

    // TODO use bit offset #defs
    reg_data = (phy_addr << 24) + (addr << 16) + (1 << 14) + (1 << 11);

    // write phy_addr and addr to control register. Initiate write operation
    mac_temac_regwr_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, reg_data);

    //TODO add MDIO timeouts
    reg_data = 0x0;
    while ((mdio_ready == 0) && (g_linkmgr_cfg.mock_hw_access == false)) {
        mac_temac_regrd_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, &reg_data);
        mdio_ready = (reg_data >> 7) & 0x1;
        SDK_TRACE_DEBUG("{0:s} reg_data {1:x} mdio_ready {2:d}",
                        __FUNCTION__, reg_data, mdio_ready);
    }

    return 0;
}

int
mac_sgmii_regwr_haps (uint32_t chip, uint32_t port_num,
                      uint64_t addr, uint32_t data)
{
    return mac_temac_mdio_wr_haps(chip, 1, 4 + port_num, addr, data);
}

int
mac_sgmii_reset_haps (uint32_t chip, uint32_t port_num, bool reset)
{
    uint64_t addr = 0x0;
    uint32_t data = 0x0;

    if (reset == true) {
        data = 0x1;
    }

    addr = MXP_BASE_HAPS +
           (chip     * MXP_INST_STRIDE_HAPS) +
           (port_num * MXP_PORT_STRIDE_HAPS) +
           SGMII_RESET_OFFSET_HAPS;

    WRITE_REG_BASE(chip, addr, data);

    return 0;
}

int
mac_temac_reset_haps (uint32_t chip, uint32_t port_num, bool reset)
{
    uint64_t addr = 0x0;
    uint32_t data = 0x0;

    if (reset == true) {
        data = 0x1;
    }

    addr = MXP_BASE_HAPS +
           (chip     * MXP_INST_STRIDE_HAPS) +
           (port_num * MXP_PORT_STRIDE_HAPS) +
           TEMAC_RESET_OFFSET_HAPS;

    WRITE_REG_BASE(chip, addr, data);

    return 0;
}

int
mac_datapath_reset_haps (uint32_t chip, uint32_t port_num, bool reset)
{
    uint64_t addr = 0x0;
    uint32_t data = 0x0;

    if (reset == true) {
        data = 0x1;
    }

    addr = MXP_BASE_HAPS +
           (chip     * MXP_INST_STRIDE_HAPS) +
           (port_num * MXP_PORT_STRIDE_HAPS) +
           DATAPATH_RESET_OFFSET;

    WRITE_REG_BASE(chip, addr, data);

    return 0;
}

int
mac_cfg_haps (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0x0;
}

int
mac_enable_haps (uint32_t port_num, uint32_t speed,
                 uint32_t num_lanes, bool enable)
{
    return 0;
}

int
mac_soft_reset_haps (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool reset)
{
    // MAC driver interfaces are agnostic of instance since the
    // asic-lib handles only interms of mac port numbers.
    // However, HAPS platform deals with instances as well.
    // Repurpose chip as mac instance for HAPS

    uint32_t chip = port_num / 4;
    port_num = port_num % 4;

    uint32_t data = 0x0;

    if (reset == true) {
        // data path reset
        mac_datapath_reset_haps(chip, port_num, reset);

        // TEMAC reset
        mac_temac_reset_haps(chip, port_num, reset);

        // SGMII reset
        mac_sgmii_reset_haps(chip, port_num, reset);
    } else {
        // SGMII un-reset
        mac_sgmii_reset_haps(chip, port_num, reset);

        // TEMAC un-reset
        mac_temac_reset_haps(chip, port_num, reset);

        // MDIO init
        data = (1 << 6) + (0x20 << 0);
        mac_temac_regwr_haps(chip, port_num, MDIO_SETUP_OFFSET_HAPS, data);

        // data path un-reset
        mac_datapath_reset_haps (chip, port_num, reset);

        // TODO what does this do?
        if (port_num == 3 || port_num == 7) {
            data = 0x1140;
            uint32_t port = 0;
            for (port = port_num - 3; port < port_num + 1; ++port) {
                mac_sgmii_regwr_haps(chip, port, 0x0, data);
            }
        }
    }

    return 0;
}

int
mac_stats_reset_haps (uint32_t port_num, uint32_t speed,
                      uint32_t num_lanes, bool reset)
{
    return 0;
}

int
mac_intr_clear_haps (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

int
mac_intr_enable_haps (uint32_t port_num, uint32_t speed,
                      uint32_t num_lanes, bool enable)
{
    return 0;
}

int
mac_temac_stats_rd (uint32_t port_num, uint32_t size)
{
    int i = 0;
    uint32_t data[1];
    uint32_t chip = port_num / 4;
    port_num = port_num % 4;

    size = 1;

    mac_temac_regrd_words_haps(chip, port_num, 0x200, size, data);
    SDK_TRACE_DEBUG("{}: Received Bytes Counter:                       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x208, size, data);
    SDK_TRACE_DEBUG("{}: Transmitted Bytes Counter:                    {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x210, size, data);
    SDK_TRACE_DEBUG("{}: Underside Frames Counter:                     {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x218, size, data);
    SDK_TRACE_DEBUG("{}: Fragment Frames Counter:                      {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x220, size, data);
    SDK_TRACE_DEBUG("{}: RX 64-Byte Frames Counter:                    {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x228, size, data);
    SDK_TRACE_DEBUG("{}: RX 65-127-Byte Frames Counter:                {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x230, size, data);
    SDK_TRACE_DEBUG("{}: RX 128-255-Byte Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x238, size, data);
    SDK_TRACE_DEBUG("{}: RX 256-511-Byte Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x240, size, data);
    SDK_TRACE_DEBUG("{}: RX 512-1023-Byte Frames Counter:              {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x248, size, data);
    SDK_TRACE_DEBUG("{}: RX 1024-Max Frames Size Frames Counter:       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x250, size, data);
    SDK_TRACE_DEBUG("{}: RX Oversize Frames Counter:                   {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x258, size, data);
    SDK_TRACE_DEBUG("{}: TX 64-Byte Frames Counter:                    {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x260, size, data);
    SDK_TRACE_DEBUG("{}: TX 65-127-Byte Frames Counter:                {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x268, size, data);
    SDK_TRACE_DEBUG("{}: TX 128-255-Byte Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x270, size, data);
    SDK_TRACE_DEBUG("{}: TX 256-511-Byte Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x278, size, data);
    SDK_TRACE_DEBUG("{}: TX 512-1023-Byte Frames Counter:              {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x280, size, data);
    SDK_TRACE_DEBUG("{}: TX 1024-Max Frames Size Frames Counter:       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x288, size, data);
    SDK_TRACE_DEBUG("{}: TX Oversize Frames Counter:                   {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x290, size, data);
    SDK_TRACE_DEBUG("{}: RX Good Frames Counter:                       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x298, size, data);
    SDK_TRACE_DEBUG("{}: RX Frame Check Sequence Errors Counter:       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a0, size, data);
    SDK_TRACE_DEBUG("{}: RX Good Broadcast Frames Counter:             {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a8, size, data);
    SDK_TRACE_DEBUG("{}: RX Good Multicast Frames Counter:             {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b0, size, data);
    SDK_TRACE_DEBUG("{}: RX Good Control Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b8, size, data);
    SDK_TRACE_DEBUG("{}: RX Length/Type Out of Range Errrors Counter:  {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c0, size, data);
    SDK_TRACE_DEBUG("{}: RX Good VLAN Tagged Frames Counter:           {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c8, size, data);
    SDK_TRACE_DEBUG("{}: RX Good Pause Frames Counter:                 {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d0, size, data);
    SDK_TRACE_DEBUG("{}: RX Bad Opcode Frames Counter:                 {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d8, size, data);
    SDK_TRACE_DEBUG("{}: TX Good Frames Counter:                       {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e0, size, data);
    SDK_TRACE_DEBUG("{}: TX Good Broadcast Frames Counter:             {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e8, size, data);
    SDK_TRACE_DEBUG("{}: TX Good Multicast Frames Counter:             {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f0, size, data);
    SDK_TRACE_DEBUG("{}: TX Underrun Errors Counter:                   {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f8, size, data);
    SDK_TRACE_DEBUG("{}: TX Good Control Frames Counter:               {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x300, size, data);
    SDK_TRACE_DEBUG("{}: TX Good VLAN Tagged Frames Counter:           {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x308, size, data);
    SDK_TRACE_DEBUG("{}: TX Good Pause Frames Counter:                 {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x310, size, data);
    SDK_TRACE_DEBUG("{}: TX Single Collision Frames Counter:           {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x318, size, data);
    SDK_TRACE_DEBUG("{}: TX Multiple Collision Frames Counter:         {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x320, size, data);
    SDK_TRACE_DEBUG("{}: TX Deferred Frames Counter:                   {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x328, size, data);
    SDK_TRACE_DEBUG("{}: TX Late Collision Counter:                    {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x330, size, data);
    SDK_TRACE_DEBUG("{}: TX Excess Collision Counter:                  {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x338, size, data);
    SDK_TRACE_DEBUG("{}: TX Excess Deferral Counter:                   {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x340, size, data);
    SDK_TRACE_DEBUG("{}: RX Alignment Errors Counter:                  {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x348, size, data);
    SDK_TRACE_DEBUG("{}: RX PFC Frames Counter:                        {}", __FUNCTION__, data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x350, size, data);
    SDK_TRACE_DEBUG("{}: TX PFC Frames Counter:                        {}", __FUNCTION__, data[i]);

    return 0;
}

//---------------------------------------------------------------------------
// SIM platform methods
//---------------------------------------------------------------------------

sdk_ret_t
asic_port_cfg (uint32_t port_num,
               uint32_t speed,
               uint32_t type,
               uint32_t num_lanes,
               uint32_t val)
{
    lib_model_mac_msg_send(port_num,
                           speed,
                           type,
                           num_lanes,
                           val);
    return SDK_RET_OK;
}

int
mac_cfg_sim (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_CFG, num_lanes, 0);
}

int
mac_enable_sim (uint32_t port_num, uint32_t speed,
                uint32_t num_lanes, bool enable)
{
    if (enable == false) {
        return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_EN, num_lanes, 0);
    }

    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_EN, num_lanes, 1);
}

int
mac_soft_reset_sim (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool reset)
{
    if (reset == false) {
        return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
                             num_lanes, 0);
    }

    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
            num_lanes, 1);
}

int
mac_stats_reset_sim (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool reset)
{
    if (reset == false) {
        return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_STATS_RESET,
                             num_lanes, 0);
    }

    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_STATS_RESET,
                         num_lanes, 1);
}

int
mac_intr_clear_sim (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_CLR,
                         num_lanes, 0);
}

int
mac_intr_enable_sim (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool enable)
{
    if (enable == false) {
        return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
                             num_lanes, 0);
    }

    return asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
                         num_lanes, 1);
}

//----------------------------------------------------------------------------
// Default methods
//----------------------------------------------------------------------------

int
mac_cfg_default (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

int
mac_enable_default (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable)
{
    return 0;
}

int
mac_soft_reset_default (uint32_t port_num, uint32_t speed,
                        uint32_t num_lanes, bool reset)
{
    return 0;
}

int
mac_stats_reset_default (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool reset)
{
    return 0;
}

int
mac_intr_clear_default (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

int
mac_intr_enable_default (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool enable)
{
    return 0;
}

bool
mac_faults_get_default (uint32_t port_num)
{
    return false;
}

sdk_ret_t
port::port_mac_fn_init(linkmgr_cfg_t *cfg)
{
    mac_fn_t           *mac_fn = &port::mac_fn;
    platform_type_t    platform_type = cfg->platform_type;

    mac_fn->mac_cfg         = &mac_cfg_default;
    mac_fn->mac_enable      = &mac_enable_default;
    mac_fn->mac_soft_reset  = &mac_soft_reset_default;
    mac_fn->mac_stats_reset = &mac_stats_reset_default;
    mac_fn->mac_intr_clear  = &mac_intr_clear_default;
    mac_fn->mac_intr_enable = &mac_intr_enable_default;
    mac_fn->mac_faults_get  = &mac_faults_get_default;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_HAPS:
        mac_fn->mac_cfg         = &mac_cfg_haps;
        mac_fn->mac_enable      = &mac_enable_haps;
        mac_fn->mac_soft_reset  = &mac_soft_reset_haps;
        mac_fn->mac_stats_reset = &mac_stats_reset_haps;
        mac_fn->mac_intr_clear  = &mac_intr_clear_haps;
        mac_fn->mac_intr_enable = &mac_intr_enable_haps;
        break;

    default:
        mac_fn->mac_cfg         = &mac_cfg_sim;
        mac_fn->mac_enable      = &mac_enable_sim;
        mac_fn->mac_soft_reset  = &mac_soft_reset_sim;
        mac_fn->mac_stats_reset = &mac_stats_reset_sim;
        mac_fn->mac_intr_clear  = &mac_intr_clear_sim;
        mac_fn->mac_intr_enable = &mac_intr_enable_sim;
        break;
    }

    return SDK_RET_OK;
}

}    // namespace linkmgr
}    // namespace sdk
