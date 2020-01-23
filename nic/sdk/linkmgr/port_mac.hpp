// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_MAC_HPP__
#define __SDK_PORT_MAC_HPP__

#include "include/sdk/base.hpp"
#include "linkmgr.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"

namespace sdk {
namespace linkmgr {

// MAC stats offsets in stats_data that need errata correction
#define PORT_FRAMES_RX_PAUSE      9
#define PORT_FRAMES_RX_PRIPAUSE  15
#define PORT_FRAMES_RX_PRI_0     63
#define PORT_FRAMES_RX_PRI_1     64
#define PORT_FRAMES_RX_PRI_2     65
#define PORT_FRAMES_RX_PRI_3     66
#define PORT_FRAMES_RX_PRI_4     67
#define PORT_FRAMES_RX_PRI_5     68
#define PORT_FRAMES_RX_PRI_6     69
#define PORT_FRAMES_RX_PRI_7     70

#define MAX_MGMT_MAC_STATS 32

typedef struct mac_info_s {
    uint32_t mac_id;
    uint32_t mac_ch;
    uint32_t speed;
    uint32_t num_lanes;
    uint32_t fec;
    uint8_t  tx_pad_enable;
    uint8_t  rx_pad_enable;
    uint32_t mtu;
    bool     force_global_init;
    uint8_t  loopback;
    port_pause_type_t    pause;
    bool     tx_pause_enable;
    bool     rx_pause_enable;
} mac_info_t;

typedef struct mac_fn_s_ {
    int (*mac_cfg)(mac_info_t *mac_info);
    int (*mac_enable)(uint32_t port_num, uint32_t speed,
                      uint32_t num_lanes, bool enable);
    int (*mac_soft_reset)(uint32_t port_num, uint32_t speed,
                          uint32_t num_lanes, bool reset);
    int (*mac_stats_reset)(uint32_t mac_inst, uint32_t mac_ch, bool reset);
    int (*mac_intr_clear)(uint32_t port_num, uint32_t speed,
                          uint32_t num_lanes);
    int (*mac_intr_enable)(uint32_t port_num, uint32_t speed,
                           uint32_t num_lanes, bool enable);
    bool (*mac_faults_get)(uint32_t inst_id, uint32_t mac_ch);
    int  (*mac_faults_clear)(uint32_t inst_id, uint32_t mac_ch);
    bool (*mac_sync_get)(uint32_t port_num);
    int  (*mac_flush_set)(uint32_t port_num, bool enable);
    int  (*mac_stats_get)(uint32_t mac_inst, uint32_t mac_ch,
                          uint64_t *stats_data);
    int  (*mac_pause_src_addr)(uint32_t mac_inst, uint32_t mac_ch,
                               uint8_t *mac_addr);
    int  (*mac_deinit)(uint32_t mac_inst, uint32_t mac_ch);
    int  (*mac_send_remote_faults)(uint32_t mac_inst, uint32_t mac_ch,
                                   bool send);
    int  (*mac_tx_drain)(uint32_t mac_inst, uint32_t mac_ch,
                         bool drain);
} mac_fn_t;

extern mac_fn_t mac_fns;
extern mac_fn_t mac_mgmt_fns;

sdk_ret_t port_mac_fn_init(linkmgr_cfg_t *cfg);

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_PORT_MAC_HPP__
