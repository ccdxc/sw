// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "include/sdk/mem.hpp"
#include "port_mac.hpp"
#include "linkmgr_rw.hpp"
#include "linkmgr_types.hpp"
#include "linkmgr_internal.hpp"
#include "platform/drivers/xcvr.hpp"

using sdk::types::port_speed_t;

namespace sdk {
namespace linkmgr {

uint8_t mx_init[MAX_MAC];
uint8_t bx_init[MAX_MAC];

//---------------------------------------------------------------------------
// HAPS platform methods
//---------------------------------------------------------------------------

static int
mac_soft_reset_haps (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool reset)
                     __attribute__ ((unused));

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
    while (mdio_ready == 0) {
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
    while (mdio_ready == 0) {
        mac_temac_regrd_haps(chip, port_num, MDIO_CTRL_OFFSET_HAPS, &reg_data);
        mdio_ready = (reg_data >> 7) & 0x1;
        SDK_LINKMGR_TRACE_DEBUG("reg_data 0x%x mdio_ready %d\n",
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
mac_cfg_haps (const char *cfg_path) __attribute__ ((unused));

static int
mac_cfg_haps (const char *cfg_path)
{
    std::string haps_mac_cfg = "haps_mac_cfg";

    haps_mac_cfg = std::string(cfg_path) + "/" + haps_mac_cfg;

    FILE *fp = fopen(haps_mac_cfg.c_str(), "r");
    if (fp == NULL) {
        return -1;
    }

    uint32_t addrh = 0x0;
    uint32_t addrl = 0x0;
    uint32_t data  = 0x0;
    uint32_t chip  = 0;

    while (fscanf(fp, "%" PRIx32 "%" PRIx32 "%" PRIx32,
                  &addrh, &addrl, &data) != EOF) {
        WRITE_REG_BASE(chip, (uint64_t)addrl, data);
    }

    data = 0x1140;
    uint32_t port = 0;
    for (port = 0; port < 4; ++port) {
        mac_sgmii_regwr_haps(0, port, 0x0, data);
        mac_sgmii_regwr_haps(1, port, 0x0, data);
    }

    return 0x0;
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

int
mac_temac_stats_rd (uint32_t port_num, uint32_t size)
{
    int i = 0;
    uint32_t data[1];
    uint32_t chip = port_num / 4;
    port_num = port_num % 4;

    size = 1;

    mac_temac_regrd_words_haps(chip, port_num, 0x200, size, data);
    SDK_LINKMGR_TRACE_DEBUG("Received Bytes Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x208, size, data);
    SDK_LINKMGR_TRACE_DEBUG("Transmitted Bytes Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x210, size, data);
    SDK_LINKMGR_TRACE_DEBUG("Underside Frames Counter:                     %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x218, size, data);
    SDK_LINKMGR_TRACE_DEBUG("Fragment Frames Counter:                      %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x220, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 64-Byte Frames Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x228, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 65-127-Byte Frames Counter:                %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x230, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 128-255-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x238, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 256-511-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x240, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 512-1023-Byte Frames Counter:              %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x248, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX 1024-Max Frames Size Frames Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x250, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Oversize Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x258, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 64-Byte Frames Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x260, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 65-127-Byte Frames Counter:                %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x268, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 128-255-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x270, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 256-511-Byte Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x278, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 512-1023-Byte Frames Counter:              %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x280, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX 1024-Max Frames Size Frames Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x288, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Oversize Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x290, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good Frames Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x298, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Frame Check Sequence Errors Counter:       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good Broadcast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2a8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good Multicast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good Control Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2b8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Length/Type Out of Range Errrors Counter:  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good VLAN Tagged Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2c8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Good Pause Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Bad Opcode Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2d8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good Frames Counter:                       %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good Broadcast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2e8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good Multicast Frames Counter:             %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f0, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Underrun Errors Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x2f8, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good Control Frames Counter:               %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x300, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good VLAN Tagged Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x308, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Good Pause Frames Counter:                 %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x310, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Single Collision Frames Counter:           %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x318, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Multiple Collision Frames Counter:         %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x320, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Deferred Frames Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x328, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Late Collision Counter:                    %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x330, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Excess Collision Counter:                  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x338, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX Excess Deferral Counter:                   %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x340, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX Alignment Errors Counter:                  %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x348, size, data);
    SDK_LINKMGR_TRACE_DEBUG("RX PFC Frames Counter:                        %d", data[i]);

    mac_temac_regrd_words_haps(chip, port_num, 0x350, size, data);
    SDK_LINKMGR_TRACE_DEBUG("TX PFC Frames Counter:                        %d", data[i]);

    return 0;
}

//----------------------------------------------------------------------------
// HW methods
//----------------------------------------------------------------------------

static uint32_t
mac_get_ch_from_port(uint32_t port_num)
{
    return port_num;  // For elba, port.cc set port_num = mac_ch (see port::port_mac_port_num_calc)
}

static uint32_t
mac_get_tm_port_from_mac_ch(uint32_t mac_ch)
{
    // when 4 ports are enabled in mx, channels assigment is {0, 4, 2, 6} for port 0~3
    if (mac_ch == 0) return 0;
    else if (mac_ch == 4) return 1;
    else if (mac_ch == 2) return 2;
    else if (mac_ch == 6) return 3;
    // when 5 ports are enabled in mx, channels assigment is {0, 4, 5, 6, 7} for port 0~4
    else if (mac_ch == 5) return 2;
    else if (mac_ch == 7) return 4;

    SDK_LINKMGR_TRACE_DEBUG("ERROR: mac_get_tm_port_from_mac_ch unsupported mac_ch=%d", mac_ch);
    return 0;
}

static bool
mac_global_init(uint32_t inst_id)
{
    return mx_init[inst_id];
}

static int
mac_cfg_fec_hw (mac_info_t *mac_info)
{
    int          chip_id       = 0;
    uint32_t     inst_id       = 0;
    uint32_t     mac_ch        = mac_info->mac_ch;
    uint32_t     num_lanes     = mac_info->num_lanes;
    uint32_t     fec           = mac_info->fec;
    port_speed_t      port_speed = (port_speed_t) mac_info->speed;
    int          ch_speed      = 0;

    switch (port_speed) {
    case port_speed_t::PORT_SPEED_10G:
        ch_speed = 10;
        break;
    case port_speed_t::PORT_SPEED_25G:
        ch_speed = 25;
        break;
    case port_speed_t::PORT_SPEED_40G:
        ch_speed = 40;
        break;
    case port_speed_t::PORT_SPEED_50G:
        ch_speed = 50;
        break;
    case port_speed_t::PORT_SPEED_100G:
        ch_speed = 100;
        break;
    case port_speed_t::PORT_SPEED_200G:
        ch_speed = 200;
        break;
    default:
        break;
    }
    return elb_mx_set_ch_mode(chip_id, inst_id, mac_ch, ch_speed, num_lanes, fec);
}

static int
mac_cfg_hw (mac_info_t *mac_info)
{
    int          chip_id       = 0;
    int          mac_ch_en     = 0;
    uint32_t     inst_id       = 0;
    uint32_t     mac_ch        = mac_info->mac_ch;
    uint32_t     tm_port         = mac_get_tm_port_from_mac_ch(mac_ch);
    uint32_t     num_lanes     = mac_info->num_lanes;
    uint32_t     fec           = mac_info->fec;
    uint32_t     mx_api_speed  = 0;
    uint8_t      loopback      = mac_info->loopback;
    port_pause_type_t pause      = mac_info->pause;
    bool         tx_pause_enable = mac_info->tx_pause_enable;
    bool         rx_pause_enable = mac_info->rx_pause_enable;
    port_speed_t      port_speed = (port_speed_t) mac_info->speed;

    int port_ch_map_arr[5] = {0, 4, 2, 6, 0};
    int port_speed_arr[5]  = {0, 0, 0, 0, 0};
    int port_vec_arr[5]    = {0, 0, 0, 0, 0};
    int slot_port_arr[8];
    int slot_ch_arr[8];

    // For future 5-port configure
    //int port_ch_map_arr[5] = {0, 4, 5, 6, 7};
    //int port_vec_arr[5]    = {200, 50, 50, 50, 50} or {100, 25, 25, 25, 25};

    switch (port_speed) {
    case port_speed_t::PORT_SPEED_10G:
        mx_api_speed = 10;
        for (int i = 0; i < 4; i++) port_speed_arr[i] = 10;
        for (int i = 0; i < 4; i++) port_vec_arr[i] = 1;
        break;

    case port_speed_t::PORT_SPEED_25G:
        mx_api_speed = 25;
        for (int i = 0; i < 4; i++) port_speed_arr[i] = 25;
        for (int i = 0; i < 4; i++) port_vec_arr[i] = 1;
        break;

    case port_speed_t::PORT_SPEED_40G:
        mx_api_speed = 40;
        for (int i = 0; i < 4; i++) port_speed_arr[i] = 40;
        for (int i = 0; i < 4; i++) port_vec_arr[i] = 1;
        break;

    case port_speed_t::PORT_SPEED_50G:
        mx_api_speed = 50;
        for (int i = 0; i < 4; i++) port_speed_arr[i] = 50;
        for (int i = 0; i < 4; i++) port_vec_arr[i] = 1;
        break;

    case port_speed_t::PORT_SPEED_100G:
        mx_api_speed = 100;
        for (int i = 0; i < 4; i++) port_speed_arr[i] = 100;
        for (int i = 0; i < 4; i++) port_vec_arr[i] = 1;
        break;

    case port_speed_t::PORT_SPEED_200G:
        mx_api_speed = 200;
        for (int i = 0; i < 2; i++) port_speed_arr[i] = 200;
        for (int i = 0; i < 2; i++) port_vec_arr[i] = 1;
        break;

    default:
        break;
    }

    if (mac_global_init(inst_id) == 0 || mac_info->force_global_init == true) {
      // Set TDM slot cfg. Potentially, json file can provide slot_port and slot_ch directly.
      if (elb_mx_slot_gen(inst_id, port_vec_arr, port_ch_map_arr, port_speed_arr, slot_port_arr, slot_ch_arr) < 0) return -1;
      elb_mx_set_slot(chip_id, inst_id, slot_port_arr, slot_ch_arr);
    }

    mx_init[inst_id] = mx_init[inst_id] | mac_ch_en;

    // Only master lane
    SDK_LINKMGR_TRACE_DEBUG("mac_inst: %d, mac_ch: %d, tm_port: %d, mx_init: 0x%x",
                    inst_id, mac_ch, tm_port, mx_init[inst_id]);

    // set MAC serdes loopback if enabled
    elb_mx_pcs_lpbk_set(chip_id, inst_id, mac_ch, num_lanes, loopback);

    // Channel configuration
    elb_mx_set_ch_mode(chip_id, inst_id, mac_ch, mx_api_speed, num_lanes, fec);

    // Tx FIFO early-full and min-start thresholds
    if (mx_api_speed >= 200) {
       elb_mx_set_txthresh(chip_id, inst_id, mac_ch, 0x3f, 0x4);
     } else {
       elb_mx_set_txthresh(chip_id, inst_id, mac_ch, 0x3f, 0x2);
    }

    // MTU/Jabber
    elb_mx_set_mtu_jabber(chip_id, inst_id, mac_ch, mac_info->mtu, 4+mac_info->mtu);

    // By default enable the RX len error check
    elb_mx_disable_eth_len_err(chip_id, inst_id, mac_ch, 0);

    // Set Tx IFG to 12 (call this function to resolve conflict between ifglen and tx_ipg)
    elb_mx_set_ifglen(chip_id, inst_id, mac_ch, 12);

    elb_mx_set_tx_padding(chip_id, inst_id, mac_ch, mac_info->tx_pad_enable);

    // No more rx padding disable in elba. 
    // By disabling rx padding in capri, it may cause rx overflow in pb.
    //cap_mx_set_rx_padding(chip_id, inst_id, mac_ch, mac_info->rx_pad_enable);  // TODO: why sw disable capri rx padding?

    elb_mx_set_vlan_check(chip_id, inst_id, mac_ch,
                          3 /* num of tags to check */,
                          0x8100 /* tag1 */,
                          0x8100 /* tag2 */,
                          0x8100 /* tag3 */);

    // Rx eof timeout max
    elb_mx_set_fixer_timeout(chip_id, inst_id, tm_port, 256);

    switch (pause) {
    case port_pause_type_t::PORT_PAUSE_TYPE_LINK:
        elb_mx_set_pause(chip_id, inst_id, tm_port, mac_ch, 0x1, 1,
                         tx_pause_enable, rx_pause_enable);
        break;

    case port_pause_type_t::PORT_PAUSE_TYPE_PFC:
        elb_mx_set_pause(chip_id, inst_id, tm_port, mac_ch, 0xff, 0,
                         tx_pause_enable, rx_pause_enable);
        break;

    default:
        elb_mx_set_pause(chip_id, inst_id, tm_port, mac_ch, 0x0, 0,
                         tx_pause_enable, rx_pause_enable);
        break;
    }

    if ((mx_api_speed == 25 || mx_api_speed == 10) && (fec == 0)) {
       elb_mx_enable_false_linkup_detection(chip_id , inst_id, mac_ch, (9800/8));
    }


    return 0;
}

static int
mac_enable_hw (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = 0;
    uint32_t mac_ch     = mac_get_ch_from_port(port_num);

    if (enable == true) {
        value = 1;
    }

    elb_mx_set_ch_enable(chip_id, inst_id, mac_ch, value, value);

    return 0;
}

static int
mac_soft_reset_hw (uint32_t port_num, uint32_t speed,
                        uint32_t num_lanes, bool reset)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = 0;
    uint32_t mac_ch     = mac_get_ch_from_port(port_num);
    uint32_t tm_port    = mac_get_tm_port_from_mac_ch(mac_ch);

    if (reset == true) {
        value = 1;
    }

    elb_mx_set_soft_reset(chip_id, inst_id, tm_port, mac_ch, value);

    return 0;
}

static int
mac_stats_reset_hw (uint32_t mac_inst, uint32_t mac_ch, bool reset)
{
    int chip_id = 0;

    if (reset == true) {
        elb_mx_clear_mac_stat(chip_id, mac_inst, mac_ch, 1);
    } else {
        elb_mx_clear_mac_stat(chip_id, mac_inst, mac_ch, 0);
    }
    return 0;
}

static int
mac_intr_clear_hw (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    int chip_id = 0;
    int inst_id = 0;
    elb_mx_clear_int(chip_id, inst_id);
    return 0;
}

static int
mac_intr_enable_hw (uint32_t port_num, uint32_t speed,
                         uint32_t num_lanes, bool enable)
{
    int chip_id = 0;
    int inst_id = 0;
    elb_mx_enable_int(chip_id, inst_id);
    return 0;
}

static int
mac_faults_clear_hw (uint32_t inst_id, uint32_t mac_ch)
{
    int chip_id = 0;
    elb_mx_read_stats(chip_id, inst_id, mac_ch, 96, true);  // ber_err_cnt
    elb_mx_read_stats(chip_id, inst_id, mac_ch, 97, true);  // err_blocks_cnt
    return 0;
}

static bool
mac_faults_get_hw (uint32_t inst_id, uint32_t mac_ch)
{
    int chip_id = 0;
    uint64_t ber_err_cnt = elb_mx_read_stats(chip_id, inst_id, mac_ch, 96);
    uint64_t err_blocks_cnt = elb_mx_read_stats(chip_id, inst_id, mac_ch, 97);
    return (ber_err_cnt > 0) || (err_blocks_cnt > 0);
}

static bool
mac_sync_get_hw (uint32_t port_num)
{
    uint32_t chip_id    = 0;
    uint32_t inst_id    = 0;
    uint32_t mac_ch     = mac_get_ch_from_port(port_num);

    return elb_mx_check_ch_sync(chip_id, inst_id, mac_ch) == 1;
}

static int
mac_flush_set_hw (uint32_t port_num, bool enable)
{
    int      val        = 0x1;
    uint32_t chip_id    = 0;
    uint32_t inst_id    = 0;
    uint32_t mac_ch     = mac_get_ch_from_port(port_num);
    uint32_t tm_port    = mac_get_tm_port_from_mac_ch(mac_ch);

    if (enable == true) {
        val = 0x0;
    }

    //elb_mx_set_flush(chip_id, inst_id, tm_port, enable);
    elb_mx_set_rxsm_enable(chip_id, inst_id, tm_port, val);
    elb_mx_set_tx_drain(chip_id, inst_id, mac_ch, enable);

    return 0;
}

static int
mac_stats_get_hw (uint32_t mac_inst, uint32_t mac_ch,
                  uint64_t *stats_data)
{
    elb_mx_mac_stat(0 /*chip_id*/, mac_inst, mac_ch, stats_data);
    return 0;
}

static int
mac_pause_src_addr_hw (uint32_t mac_inst, uint32_t mac_ch, uint8_t *mac_addr)
{
    elb_mx_set_pause_src_addr(0, mac_inst, mac_ch, mac_addr);
    return 0;
}

// clear channel bit
static int
mac_deinit_hw (uint32_t mac_inst, uint32_t mac_ch)
{
    uint32_t mask = 1 << mac_ch;
    mx_init[mac_inst] = mx_init[mac_inst] & ~mask;
    SDK_LINKMGR_TRACE_DEBUG("mac_inst: %d, mac_ch: %d, mx_init: 0x%x",
                    mac_inst, mac_ch, mx_init[mac_inst]);
    return 0;
}

static int
mac_send_remote_faults_hw (uint32_t mac_inst, uint32_t mac_ch,
                           bool send)
{
    elb_mx_send_remote_faults(0, mac_inst, mac_ch, send);
    return 0;
}

static int
mac_tx_drain_hw (uint32_t mac_inst, uint32_t mac_ch, bool drain)
{
    elb_mx_set_tx_drain(0, mac_inst, mac_ch, drain);
    return 0;
}

static int
mac_an_start_hw (uint32_t mac_ch, uint32_t user_cap, bool fec_ability, uint32_t fec_request)
{
   int user_fec = 0;
   int user_pause = 0;
   int ignore_nonce = 1;
   if (fec_ability) {
      user_fec = user_fec | elb_mx_an_fec_capability_t::ELB_MX_AN_FEC_FEC_10G_ABI;
   }
   if ((fec_request & AN_FEC_REQ_25GB_RSFEC) == 1) {
      user_fec = user_fec | elb_mx_an_fec_capability_t::ELB_MX_AN_FEC_RSFEC_25G_REQ;
   }
   if ((fec_request & AN_FEC_REQ_25GB_FCFEC) == 1) {
      user_fec = user_fec | elb_mx_an_fec_capability_t::ELB_MX_AN_FEC_FCFEC_25G_REQ;
   }
   elb_mx_an_init(0, 0, mac_ch, user_cap, user_fec, user_pause, ignore_nonce);
   return 0;
}

static bool
mac_an_wait_hcd_hw (uint32_t mac_ch)
{
   return elb_mx_an_get_an_complete(0, 0, mac_ch);
}

static uint32_t
mac_an_hcd_read_hw (uint32_t mac_ch)
{
   return elb_mx_an_get_hcd(0, 0, mac_ch);
}

static int
mac_an_fec_enable_read_hw (uint32_t mac_ch)
{
   return elb_mx_an_get_25g_fcfec(0, 0, mac_ch);
}

static int
mac_an_rsfec_enable_read_hw (uint32_t mac_ch)
{
   return elb_mx_an_get_25g_rsfec(0, 0, mac_ch);
}

//----------------------------------------------------------------------------
// MGMT MAC methods
//----------------------------------------------------------------------------

static int
mac_mgmt_cfg_fec_hw (mac_info_t *mac_info)
{
    return 0;
}

static int
mac_mgmt_cfg_hw (mac_info_t *mac_info)
{
    int          chip_id       = 0;
    int          mac_ch_en     = 0;
    uint32_t     inst_id       = 0; // bx inst is 0
    uint32_t     start_lane    = mac_info->mac_ch;
    uint32_t     bx_api_speed  = 0;
    port_speed_t port_speed    = (port_speed_t) mac_info->speed;

    switch (port_speed) {
    case port_speed_t::PORT_SPEED_1G:
        bx[inst_id].mac_mode = MAC_MODE_4x1g;
        bx_api_speed = 1;
        break;

    case port_speed_t::PORT_SPEED_10G:
        bx[inst_id].mac_mode = MAC_MODE_4x10g;
        bx_api_speed = 10;
        break;

    case port_speed_t::PORT_SPEED_25G:
        bx[inst_id].mac_mode = MAC_MODE_4x25g;
        bx_api_speed = 25;
        break;

    case port_speed_t::PORT_SPEED_40G:
        bx[inst_id].mac_mode = MAC_MODE_1x40g;
        bx_api_speed = 40;
        break;

    case port_speed_t::PORT_SPEED_50G:
        bx[inst_id].mac_mode = MAC_MODE_2x50g;
        bx_api_speed = 50;
        break;

    case port_speed_t::PORT_SPEED_100G:
        bx[inst_id].mac_mode = MAC_MODE_1x100g;
        bx_api_speed = 100;
        break;

    default:
        break;
    }

    bx[inst_id].glbl_mode = glbl_mode_mgmt(bx[inst_id].mac_mode);

    // Only master lane
    mac_ch_en |= (1 << start_lane);

    bx[inst_id].ch_mode [start_lane] =
                           ch_mode_mgmt(bx[inst_id].mac_mode, start_lane);

    bx[inst_id].speed      [start_lane] = bx_api_speed;
    bx[inst_id].port_enable[start_lane] = 1;

    if (bx_init[inst_id] == 0) {
        // global mode
        cap_bx_set_glbl_mode(chip_id, inst_id, bx[inst_id].glbl_mode);

        // MAC Rx Configuration: bit4: Promiscuous Mode (1: disable MAC address check)
        cap_bx_apb_write(chip_id, inst_id, 0x2102, 0x10);

        // FIFO Control 1: 16'b0_000010_01000_0100;
        cap_bx_apb_write(chip_id, inst_id, 0x3f01, 0x484);

        // channel mode
        cap_bx_apb_write(chip_id, inst_id, 0x4010, bx[inst_id].ch_mode[start_lane]);

        // mtu
        cap_bx_set_mtu(chip_id, inst_id, mac_info->mtu, mac_info->mtu + 1);

        // channel enable
        cap_bx_set_ch_enable(chip_id, inst_id, mac_ch_en);

        bx_init[inst_id] |= mac_ch_en;
        SDK_LINKMGR_TRACE_DEBUG("mac_inst: %d, mac_ch: %d, bx_init: 0x%x",
                        inst_id, start_lane, bx_init[inst_id]);
    }

    return 0;
}

static int
mac_mgmt_enable_hw (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = 0;
    uint32_t start_lane = 0;
    uint32_t max_lanes  = start_lane + num_lanes;

    if (enable == true) {
        value = 1;
        // Enable only master lane
        max_lanes = start_lane + 1;
    }

    for (uint32_t lane = start_lane; lane < max_lanes; lane++) {
        cap_bx_set_ch_enable(chip_id, inst_id, value);
    }

    return 0;
}

static int
mac_mgmt_soft_reset_hw (uint32_t port_num, uint32_t speed,
                        uint32_t num_lanes, bool reset)
{
    uint32_t chip_id    = 0;
    int      value      = 0;
    uint32_t inst_id    = 0;
    uint32_t start_lane = 0;
    uint32_t max_lanes  = start_lane + 1;

    if (reset == true) {
        value = 1;
        // Reset all lanes
        max_lanes = start_lane + num_lanes;
    }

    for (uint32_t lane = start_lane; lane < max_lanes; lane++) {
        cap_bx_set_soft_reset(chip_id, inst_id, value);
    }

    return 0;
}

static int
mac_mgmt_faults_clear_hw (uint32_t inst_id, uint32_t mac_ch)
{
    return 0;
}

static bool
mac_mgmt_faults_get_hw (uint32_t inst_id, uint32_t mac_ch)
{
    return false;
}

static bool
mac_mgmt_sync_get_hw (uint32_t port_num)
{
    uint32_t chip_id    = 0;
    uint32_t inst_id    = 0;

    return cap_bx_check_sync(chip_id, inst_id) == 1;
}

static int
mac_mgmt_flush_set_hw (uint32_t port_num, bool enable)
{
    return 0;
}

static int
mac_mgmt_stats_get_hw (uint32_t mac_inst, uint32_t mac_ch,
                       uint64_t *stats_data)
{
    cap_bx_mac_stat(0 /*chip_id*/, 0 /* bx_inst */, mac_ch, 0, stats_data);
    return 0;
}

static int
mac_mgmt_deinit_hw (uint32_t mac_inst, uint32_t mac_ch)
{
    uint32_t mask = 1 << mac_ch;
    bx_init[mac_inst] = bx_init[mac_inst] & ~mask;
    SDK_LINKMGR_TRACE_DEBUG("mac_inst: %d, mac_ch: %d, bx_init: 0x%x",
                    mac_inst, mac_ch, bx_init[mac_inst]);
    return 0;
}

static int
mac_mgmt_tx_drain_hw (uint32_t mac_inst, uint32_t mac_ch, bool drain)
{
    cap_bx_tx_drain(0, 0, mac_ch, drain);
    return 0;
}

static int
mac_mgmt_stats_reset_hw (uint32_t mac_inst, uint32_t mac_ch, bool reset)
{
    int chip_id = 0;

    if (reset == true) {
        cap_bx_stats_reset(chip_id, 0, mac_ch, 1);
    } else {
        cap_bx_stats_reset(chip_id, 0, mac_ch, 0);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Mock methods
//----------------------------------------------------------------------------

static bool
mac_sync_get_mock (uint32_t port_num)
{
    uint32_t inst_id    = 0;
    uint32_t start_lane = mac_get_ch_from_port(port_num);

    static uint8_t *cnt = NULL;

    if (cnt == NULL) {
        // Total MAC = MAX_MAC + MGMT_MAC
        uint32_t size = (MAX_MAC+1) * MAX_CHANNEL * sizeof(uint8_t);
        cnt = (uint8_t*)SDK_MALLOC(SDK_PORT_DEBUG, size);
        if (cnt != NULL) {
            memset (cnt, 0, size);
        }
    }

    int index = (inst_id * MAX_CHANNEL) +  start_lane;

    // fail 5 times
    if (cnt[index] < 5) {
        cnt[index]++;
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
// Default methods
//----------------------------------------------------------------------------

static int
mac_cfg_default (mac_info_t *mac_info)
{
    return 0;
}

static int
mac_cfg_fec_default (mac_info_t *mac_info)
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
mac_stats_reset_default (uint32_t mac_inst, uint32_t mac_ch, bool reset)
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

static int
mac_faults_clear_default (uint32_t inst_id, uint32_t mac_ch)
{
    return 0;
}

static bool
mac_faults_get_default (uint32_t inst_id, uint32_t mac_ch)
{
    return false;
}

static bool
mac_sync_get_default (uint32_t port_num)
{
    return true;
}

static int
mac_flush_set_default (uint32_t port_num, bool enable)
{
    return 0;
}

static int
mac_stats_get_default (uint32_t mac_inst, uint32_t mac_ch,
                       uint64_t *stats_data)
{
    return 0;
}

static int
mac_pause_src_addr_default (uint32_t mac_inst, uint32_t mac_ch,
                            uint8_t *mac_addr)
{
    return 0;
}

static int
mac_deinit_default (uint32_t mac_inst, uint32_t mac_ch)
{
    return 0;
}

static int
mac_send_remote_faults_default (uint32_t mac_inst, uint32_t mac_ch,
                                bool send)
{
    return 0;
}

static int
mac_tx_drain_default (uint32_t mac_inst, uint32_t mac_ch, bool drain)
{
    return 0;
}

static int
mac_an_start_default (uint32_t mac_ch, uint32_t user_cap, bool fec_ability, uint32_t fec_request)
{
    return 0;
}

static bool
mac_an_wait_hcd_default (uint32_t mac_ch)
{
    return 0;
}

static uint32_t
mac_an_hcd_read_default (uint32_t mac_ch)
{   
    return 0;
}

static int
mac_an_fec_enable_read_default (uint32_t mac_ch)
{
    return 0;
}

static int
mac_an_rsfec_enable_read_default (uint32_t mac_ch)
{
    return 0;
}

sdk_ret_t
port_mac_fn_init(linkmgr_cfg_t *cfg)
{
    mac_fn_t        *mac_fn       = &mac_fns;
    mac_fn_t        *mac_mgmt_fn  = &mac_mgmt_fns;
    platform_type_t platform_type = cfg->platform_type;

    mac_fn->mac_cfg            = &mac_cfg_default;
    mac_fn->mac_cfg_fec        = &mac_cfg_fec_default;
    mac_fn->mac_enable         = &mac_enable_default;
    mac_fn->mac_soft_reset     = &mac_soft_reset_default;
    mac_fn->mac_stats_reset    = &mac_stats_reset_default;
    mac_fn->mac_intr_clear     = &mac_intr_clear_default;
    mac_fn->mac_intr_enable    = &mac_intr_enable_default;
    mac_fn->mac_faults_get     = &mac_faults_get_default;
    mac_fn->mac_faults_clear   = &mac_faults_clear_default;
    mac_fn->mac_sync_get       = &mac_sync_get_default;
    mac_fn->mac_flush_set      = &mac_flush_set_default;
    mac_fn->mac_stats_get      = &mac_stats_get_default;
    mac_fn->mac_pause_src_addr = &mac_pause_src_addr_default;
    mac_fn->mac_deinit         = &mac_deinit_default;
    mac_fn->mac_send_remote_faults = &mac_send_remote_faults_default;
    mac_fn->mac_tx_drain       = &mac_tx_drain_default;
    mac_fn->mac_an_start             = &mac_an_start_default;
    mac_fn->mac_an_wait_hcd          = &mac_an_wait_hcd_default;
    mac_fn->mac_an_hcd_read          = &mac_an_hcd_read_default;
    mac_fn->mac_an_fec_enable_read   = &mac_an_fec_enable_read_default;
    mac_fn->mac_an_rsfec_enable_read = &mac_an_rsfec_enable_read_default;

    mac_mgmt_fn->mac_cfg            = &mac_cfg_default;
    mac_mgmt_fn->mac_cfg_fec        = &mac_cfg_fec_default;
    mac_mgmt_fn->mac_enable         = &mac_enable_default;
    mac_mgmt_fn->mac_soft_reset     = &mac_soft_reset_default;
    mac_mgmt_fn->mac_stats_reset    = &mac_stats_reset_default;
    mac_mgmt_fn->mac_intr_clear     = &mac_intr_clear_default;
    mac_mgmt_fn->mac_intr_enable    = &mac_intr_enable_default;
    mac_mgmt_fn->mac_faults_get     = &mac_faults_get_default;
    mac_mgmt_fn->mac_faults_clear   = &mac_faults_clear_default;
    mac_mgmt_fn->mac_sync_get       = &mac_sync_get_default;
    mac_mgmt_fn->mac_flush_set      = &mac_flush_set_default;
    mac_mgmt_fn->mac_stats_get      = &mac_stats_get_default;
    mac_mgmt_fn->mac_pause_src_addr = &mac_pause_src_addr_default;
    mac_mgmt_fn->mac_deinit         = &mac_deinit_default;
    mac_mgmt_fn->mac_send_remote_faults = &mac_send_remote_faults_default;
    mac_mgmt_fn->mac_tx_drain       = &mac_tx_drain_default;
    mac_mgmt_fn->mac_an_start             = &mac_an_start_default;
    mac_mgmt_fn->mac_an_wait_hcd          = &mac_an_wait_hcd_default;
    mac_mgmt_fn->mac_an_hcd_read          = &mac_an_hcd_read_default;
    mac_mgmt_fn->mac_an_fec_enable_read   = &mac_an_fec_enable_read_default;
    mac_mgmt_fn->mac_an_rsfec_enable_read = &mac_an_rsfec_enable_read_default;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_HAPS:
        // TODO needs sequencing from hal
        // mac_cfg_haps(cfg->cfg_path);
        break;

    case platform_type_t::PLATFORM_TYPE_SIM:
    case platform_type_t::PLATFORM_TYPE_MOCK:
        // Faults and Sync is mocked
        mac_fn->mac_cfg            = &mac_cfg_hw;
        mac_fn->mac_cfg_fec        = &mac_cfg_fec_hw;
        mac_fn->mac_enable         = &mac_enable_hw;
        mac_fn->mac_soft_reset     = &mac_soft_reset_hw;
        mac_fn->mac_stats_reset    = &mac_stats_reset_hw;
        mac_fn->mac_intr_clear     = &mac_intr_clear_hw;
        mac_fn->mac_intr_enable    = &mac_intr_enable_hw;
        mac_fn->mac_sync_get       = &mac_sync_get_mock;
        mac_fn->mac_flush_set      = &mac_flush_set_hw;
        mac_fn->mac_stats_get      = &mac_stats_get_hw;
        mac_fn->mac_pause_src_addr = &mac_pause_src_addr_hw;
        mac_fn->mac_deinit         = &mac_deinit_hw;
        mac_fn->mac_tx_drain       = &mac_tx_drain_hw;
        mac_fn->mac_an_start             = &mac_an_start_hw;
        mac_fn->mac_an_wait_hcd          = &mac_an_wait_hcd_hw;
        mac_fn->mac_an_hcd_read          = &mac_an_hcd_read_hw;
        mac_fn->mac_an_fec_enable_read   = &mac_an_fec_enable_read_hw;
        mac_fn->mac_an_rsfec_enable_read = &mac_an_rsfec_enable_read_hw;

        mac_mgmt_fn->mac_cfg          = &mac_mgmt_cfg_hw;
        mac_mgmt_fn->mac_cfg_fec      = &mac_mgmt_cfg_fec_hw;
        mac_mgmt_fn->mac_enable       = &mac_mgmt_enable_hw;
        mac_mgmt_fn->mac_soft_reset   = &mac_mgmt_soft_reset_hw;
        mac_mgmt_fn->mac_faults_get   = &mac_mgmt_faults_get_hw;
        mac_mgmt_fn->mac_faults_clear = &mac_mgmt_faults_clear_hw;
        mac_mgmt_fn->mac_sync_get     = &mac_sync_get_mock;
        mac_mgmt_fn->mac_flush_set    = &mac_mgmt_flush_set_hw;
        mac_mgmt_fn->mac_stats_get    = &mac_mgmt_stats_get_hw;
        mac_mgmt_fn->mac_deinit       = &mac_mgmt_deinit_hw;
        mac_mgmt_fn->mac_tx_drain     = &mac_mgmt_tx_drain_hw;
        mac_mgmt_fn->mac_stats_reset  = &mac_mgmt_stats_reset_hw;
        break;

    case platform_type_t::PLATFORM_TYPE_ZEBU:
    case platform_type_t::PLATFORM_TYPE_HW:
        mac_fn->mac_cfg            = &mac_cfg_hw;
        mac_fn->mac_cfg_fec        = &mac_cfg_fec_hw;
        mac_fn->mac_enable         = &mac_enable_hw;
        mac_fn->mac_soft_reset     = &mac_soft_reset_hw;
        mac_fn->mac_stats_reset    = &mac_stats_reset_hw;
        mac_fn->mac_intr_clear     = &mac_intr_clear_hw;
        mac_fn->mac_intr_enable    = &mac_intr_enable_hw;
        mac_fn->mac_faults_get     = &mac_faults_get_hw;
        mac_fn->mac_faults_clear   = &mac_faults_clear_hw;
        mac_fn->mac_sync_get       = &mac_sync_get_hw;
        mac_fn->mac_flush_set      = &mac_flush_set_hw;
        mac_fn->mac_stats_get      = &mac_stats_get_hw;
        mac_fn->mac_pause_src_addr = &mac_pause_src_addr_hw;
        mac_fn->mac_deinit         = &mac_deinit_hw;
        mac_fn->mac_send_remote_faults = &mac_send_remote_faults_hw;
        mac_fn->mac_tx_drain       = &mac_tx_drain_hw;
        mac_fn->mac_an_start             = &mac_an_start_hw;
        mac_fn->mac_an_wait_hcd          = &mac_an_wait_hcd_hw;
        mac_fn->mac_an_hcd_read          = &mac_an_hcd_read_hw;
        mac_fn->mac_an_fec_enable_read   = &mac_an_fec_enable_read_hw;
        mac_fn->mac_an_rsfec_enable_read = &mac_an_rsfec_enable_read_hw;

        mac_mgmt_fn->mac_cfg         = &mac_mgmt_cfg_hw;
        mac_mgmt_fn->mac_cfg_fec     = &mac_mgmt_cfg_fec_hw;
        mac_mgmt_fn->mac_enable      = &mac_mgmt_enable_hw;
        mac_mgmt_fn->mac_soft_reset  = &mac_mgmt_soft_reset_hw;
        mac_mgmt_fn->mac_faults_get  = &mac_mgmt_faults_get_hw;
        mac_mgmt_fn->mac_faults_clear = &mac_mgmt_faults_clear_hw;
        mac_mgmt_fn->mac_sync_get    = &mac_mgmt_sync_get_hw;
        mac_mgmt_fn->mac_flush_set   = &mac_mgmt_flush_set_hw;
        mac_mgmt_fn->mac_stats_get   = &mac_mgmt_stats_get_hw;
        mac_mgmt_fn->mac_deinit      = &mac_mgmt_deinit_hw;
        mac_mgmt_fn->mac_tx_drain    = &mac_mgmt_tx_drain_hw;
        mac_mgmt_fn->mac_stats_reset = &mac_mgmt_stats_reset_hw;
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

}    // namespace linkmgr
}    // namespace sdk
