// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "port.hpp"
#include "port_mac.hpp"
#include "linkmgr_rw.hpp"
#include "linkmgr_internal.hpp"
#include "sdk/mem.hpp"
#include "sdk/asic/capri/csrlite/cap_mx_csr_helper.hpp"
#include "sdk/asic/capri/cap_mx_api.h"
#include <sdk/types.hpp>

using sdk::types::port_speed_t;

namespace sdk {
namespace linkmgr {

using namespace sdk::lib::csrlite;

//---------------------------------------------------------------------------
// HAPS platform methods
//---------------------------------------------------------------------------

static int
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

static int
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

static int
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
    while ((mdio_ready == 0) && (g_linkmgr_cfg.hw_mock == false)) {
        mac_temac_regrd_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, &reg_data);
        mdio_ready = (reg_data >> 7) & 0x1;
    }

    // read mdio-data register
    mac_temac_regrd_haps(chip, port_num, MDIO_DATA_RD_OFFSET_HAPS, data);

    return 0;
}

static int
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
    while ((mdio_ready == 0) && (g_linkmgr_cfg.hw_mock == false)) {
        mac_temac_regrd_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, &reg_data);
        mdio_ready = (reg_data >> 7) & 0x1;
        SDK_TRACE_DEBUG("reg_data 0x%x mdio_ready %d\n",
                        reg_data, mdio_ready);
    }

    return 0;
}

static int
mac_sgmii_regwr_haps (uint32_t chip, uint32_t port_num,
                      uint64_t addr, uint32_t data)
{
    return mac_temac_mdio_wr_haps(chip, 1, 4 + port_num, addr, data);
}

static int
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

static int
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

static int
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

static int
mac_cfg_haps (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0x0;
}

static int
mac_enable_haps (uint32_t port_num, uint32_t speed,
                 uint32_t num_lanes, bool enable)
{
    return 0;
}

static int
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

static int
mac_stats_reset_haps (uint32_t port_num, uint32_t speed,
                      uint32_t num_lanes, bool reset)
{
    return 0;
}

static int
mac_intr_clear_haps (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

static int
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
    SDK_TRACE_DEBUG("Received Bytes Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x208, size, data);
    SDK_TRACE_DEBUG("Transmitted Bytes Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x210, size, data);
    SDK_TRACE_DEBUG("Underside Frames Counter:                     %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x218, size, data);
    SDK_TRACE_DEBUG("Fragment Frames Counter:                      %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x220, size, data);
    SDK_TRACE_DEBUG("RX 64-Byte Frames Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x228, size, data);
    SDK_TRACE_DEBUG("RX 65-127-Byte Frames Counter:                %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x230, size, data);
    SDK_TRACE_DEBUG("RX 128-255-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x238, size, data);
    SDK_TRACE_DEBUG("RX 256-511-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x240, size, data);
    SDK_TRACE_DEBUG("RX 512-1023-Byte Frames Counter:              %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x248, size, data);
    SDK_TRACE_DEBUG("RX 1024-Max Frames Size Frames Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x250, size, data);
    SDK_TRACE_DEBUG("RX Oversize Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x258, size, data);
    SDK_TRACE_DEBUG("TX 64-Byte Frames Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x260, size, data);
    SDK_TRACE_DEBUG("TX 65-127-Byte Frames Counter:                %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x268, size, data);
    SDK_TRACE_DEBUG("TX 128-255-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x270, size, data);
    SDK_TRACE_DEBUG("TX 256-511-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x278, size, data);
    SDK_TRACE_DEBUG("TX 512-1023-Byte Frames Counter:              %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x280, size, data);
    SDK_TRACE_DEBUG("TX 1024-Max Frames Size Frames Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x288, size, data);
    SDK_TRACE_DEBUG("TX Oversize Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x290, size, data);
    SDK_TRACE_DEBUG("RX Good Frames Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x298, size, data);
    SDK_TRACE_DEBUG("RX Frame Check Sequence Errors Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a0, size, data);
    SDK_TRACE_DEBUG("RX Good Broadcast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a8, size, data);
    SDK_TRACE_DEBUG("RX Good Multicast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b0, size, data);
    SDK_TRACE_DEBUG("RX Good Control Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b8, size, data);
    SDK_TRACE_DEBUG("RX Length/Type Out of Range Errrors Counter:  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c0, size, data);
    SDK_TRACE_DEBUG("RX Good VLAN Tagged Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c8, size, data);
    SDK_TRACE_DEBUG("RX Good Pause Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d0, size, data);
    SDK_TRACE_DEBUG("RX Bad Opcode Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d8, size, data);
    SDK_TRACE_DEBUG("TX Good Frames Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e0, size, data);
    SDK_TRACE_DEBUG("TX Good Broadcast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e8, size, data);
    SDK_TRACE_DEBUG("TX Good Multicast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f0, size, data);
    SDK_TRACE_DEBUG("TX Underrun Errors Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f8, size, data);
    SDK_TRACE_DEBUG("TX Good Control Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x300, size, data);
    SDK_TRACE_DEBUG("TX Good VLAN Tagged Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x308, size, data);
    SDK_TRACE_DEBUG("TX Good Pause Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x310, size, data);
    SDK_TRACE_DEBUG("TX Single Collision Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x318, size, data);
    SDK_TRACE_DEBUG("TX Multiple Collision Frames Counter:         %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x320, size, data);
    SDK_TRACE_DEBUG("TX Deferred Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x328, size, data);
    SDK_TRACE_DEBUG("TX Late Collision Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x330, size, data);
    SDK_TRACE_DEBUG("TX Excess Collision Counter:                  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x338, size, data);
    SDK_TRACE_DEBUG("TX Excess Deferral Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x340, size, data);
    SDK_TRACE_DEBUG("RX Alignment Errors Counter:                  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x348, size, data);
    SDK_TRACE_DEBUG("RX PFC Frames Counter:                        %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x350, size, data);
    SDK_TRACE_DEBUG("TX PFC Frames Counter:                        %d", data[i]);

    return 0;
}

//----------------------------------------------------------------------------
// HW methods
//----------------------------------------------------------------------------

static uint32_t
mac_get_inst_from_port(uint32_t port_num)
{
    return (port_num / PORT_LANES_MAX);
}

static uint32_t
mac_get_lane_from_port(uint32_t port_num)
{
    return (port_num % PORT_LANES_MAX);
}

bool mx_init[MAX_MAC];

static bool
mac_global_init(uint32_t inst_id)
{
    return mx_init[inst_id];
}

static int
mac_cfg_hw (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{

    int chip_id = 0;
    uint32_t mac_num_lanes = num_lanes;

    uint32_t inst_id    = mac_get_inst_from_port(port_num);
    uint32_t start_lane = mac_get_lane_from_port(port_num);;

    port_speed_t port_speed = (port_speed_t) speed;
    uint32_t mx_api_speed = 0;

    switch (port_speed) {
    case port_speed_t::PORT_SPEED_10G:
        mx[inst_id].mac_mode = MAC_MODE_4x10g;
        mx_api_speed = 10;
        break;

    case port_speed_t::PORT_SPEED_25G:
        mx[inst_id].mac_mode = MAC_MODE_4x25g;
        mx_api_speed = 25;
        break;

    case port_speed_t::PORT_SPEED_40G:
        mx[inst_id].mac_mode = MAC_MODE_1x40g;
        mac_num_lanes = 1;
        mx_api_speed = 40;
        break;

    case port_speed_t::PORT_SPEED_50G:
        mx[inst_id].mac_mode = MAC_MODE_1x50g;
        mx_api_speed = 50;
        break;

    case port_speed_t::PORT_SPEED_100G:
        mx[inst_id].mac_mode = MAC_MODE_1x100g;
        mac_num_lanes = 1;
        mx_api_speed = 100;
        break;

    default:
        break;
    }

    int ch_enable_vec = 0;

    mx[inst_id].glbl_mode = glbl_mode(mx[inst_id].mac_mode);

    for (uint32_t ch = start_lane; ch < mac_num_lanes; ch++) {
        mx[inst_id].ch_mode[ch]     = ch_mode(mx[inst_id].mac_mode, ch);
        mx[inst_id].speed[ch]       = mx_api_speed;
        mx[inst_id].port_enable[ch] = 1;
    }

    if (mac_global_init(inst_id) != 1) {
        cap_mx_load_from_cfg_glbl1(chip_id, inst_id, &ch_enable_vec);
        mx_init[inst_id] = 1;
    }

    for (uint32_t ch = start_lane; ch < num_lanes; ch++) {
        cap_mx_load_from_cfg_channel(chip_id, inst_id, ch, ch_enable_vec);
    }

    // cap_mx_load_from_cfg_glbl2(chip_id, inst_id, ch_enable_vec);

    return 0;
}

static int
mac_enable_hw (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = mac_get_inst_from_port(port_num);
    uint32_t start_lane = mac_get_lane_from_port(port_num);;
    uint32_t max_lanes  = start_lane + num_lanes;

    if (enable == true) {
        value = 1;
        // Enable only master lane
        max_lanes = start_lane + 1;
    }

    for (uint32_t lane = start_lane; lane < max_lanes; lane++) {
        cap_mx_set_ch_enable(chip_id, inst_id, lane, value);
    }

    return 0;
}

static int
mac_soft_reset_hw (uint32_t port_num, uint32_t speed,
                        uint32_t num_lanes, bool reset)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = mac_get_inst_from_port(port_num);
    uint32_t start_lane = mac_get_lane_from_port(port_num);;
    uint32_t max_lanes  = start_lane + 1;

    if (reset == true) {
        value = 1;
        // Reset all lanes
        max_lanes = start_lane + num_lanes;
    }

    for (uint32_t lane = start_lane; lane < max_lanes; lane++) {
        cap_mx_set_soft_reset(chip_id, inst_id, lane, value);
    }

    return 0;
}

static int
mac_stats_reset_hw (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool reset)
{
    return 0;
}

static int
mac_intr_clear_hw (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

static int
mac_intr_enable_hw (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool enable)
{
    return 0;
}

static bool
mac_faults_get_hw (uint32_t port_num)
{
    return false;
}

static bool
mac_sync_get_hw (uint32_t port_num)
{
    uint32_t chip_id    = 0;
    uint32_t inst_id    = mac_get_inst_from_port(port_num);
    uint32_t start_lane = mac_get_lane_from_port(port_num);;

    return cap_mx_check_ch_sync(chip_id, inst_id, start_lane) == 1;
}

//----------------------------------------------------------------------------
// Default methods
//----------------------------------------------------------------------------

static int
mac_cfg_default (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

static int
mac_enable_default (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable)
{
    return 0;
}

static int
mac_soft_reset_default (uint32_t port_num, uint32_t speed,
                        uint32_t num_lanes, bool reset)
{
    return 0;
}

static int
mac_stats_reset_default (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool reset)
{
    return 0;
}

static int
mac_intr_clear_default (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return 0;
}

static int
mac_intr_enable_default (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool enable)
{
    return 0;
}

static bool
mac_faults_get_default (uint32_t port_num)
{
    return false;
}

static bool
mac_sync_get_default (uint32_t port_num)
{
    return true;
}

sdk_ret_t
port::port_mac_fn_init(linkmgr_cfg_t *cfg)
{
    mac_fn_t           *mac_fn = &port::mac_fn;
    platform_type_t    platform_type = cfg->platform_type;

    // TODO test code
    cap_mx_csr_helper_t *mx_csr = NULL;
    LINKMGR_CALLOC(mx_csr, SDK_MEM_ALLOC_ID_LINKMGR, cap_mx_csr_helper_t);
    SDK_TRACE_DEBUG("%p", mx_csr);
    mx_csr->int_ecc.init(0x0);

    mac_fn->mac_cfg         = &mac_cfg_default;
    mac_fn->mac_enable      = &mac_enable_default;
    mac_fn->mac_soft_reset  = &mac_soft_reset_default;
    mac_fn->mac_stats_reset = &mac_stats_reset_default;
    mac_fn->mac_intr_clear  = &mac_intr_clear_default;
    mac_fn->mac_intr_enable = &mac_intr_enable_default;
    mac_fn->mac_faults_get  = &mac_faults_get_default;
    mac_fn->mac_sync_get    = &mac_sync_get_default;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_HAPS:
        mac_fn->mac_cfg         = &mac_cfg_haps;
        mac_fn->mac_enable      = &mac_enable_haps;
        mac_fn->mac_soft_reset  = &mac_soft_reset_haps;
        mac_fn->mac_stats_reset = &mac_stats_reset_haps;
        mac_fn->mac_intr_clear  = &mac_intr_clear_haps;
        mac_fn->mac_intr_enable = &mac_intr_enable_haps;
        break;

    case platform_type_t::PLATFORM_TYPE_SIM:
    case platform_type_t::PLATFORM_TYPE_MOCK:
        mac_fn->mac_cfg         = &mac_cfg_hw;
        mac_fn->mac_enable      = &mac_enable_hw;
        mac_fn->mac_soft_reset  = &mac_soft_reset_hw;
        mac_fn->mac_stats_reset = &mac_stats_reset_hw;
        mac_fn->mac_intr_clear  = &mac_intr_clear_hw;
        mac_fn->mac_intr_enable = &mac_intr_enable_hw;

        break;

    case platform_type_t::PLATFORM_TYPE_HW:
        mac_fn->mac_cfg         = &mac_cfg_hw;
        mac_fn->mac_enable      = &mac_enable_hw;
        mac_fn->mac_soft_reset  = &mac_soft_reset_hw;
        mac_fn->mac_stats_reset = &mac_stats_reset_hw;
        mac_fn->mac_intr_clear  = &mac_intr_clear_hw;
        mac_fn->mac_intr_enable = &mac_intr_enable_hw;
        mac_fn->mac_faults_get  = &mac_faults_get_hw;
        mac_fn->mac_sync_get    = &mac_sync_get_hw;

        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

}    // namespace linkmgr
}    // namespace sdk
