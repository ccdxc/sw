// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_MAC_HPP__
#define __SDK_PORT_MAC_HPP__

#include "include/sdk/base.hpp"
#include "linkmgr.hpp"

namespace sdk {
namespace linkmgr {

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
} mac_info_t;

typedef struct mac_fn_s_ {
    int (*mac_cfg) (mac_info_t *mac_info);
    int (*mac_enable) (uint32_t port_num, uint32_t speed,
                       uint32_t num_lanes, bool enable);
    int (*mac_soft_reset)  (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool reset);
    int (*mac_stats_reset) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool reset);
    int (*mac_intr_clear)  (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes);
    int (*mac_intr_enable) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool enable);
    bool (*mac_faults_get) (uint32_t port_num);
    bool (*mac_sync_get)   (uint32_t port_num);
    int  (*mac_flush_set)  (uint32_t port_num, bool enable);
} mac_fn_t;

extern mac_fn_t mac_fns;
extern mac_fn_t mac_mgmt_fns;

sdk_ret_t port_mac_fn_init(linkmgr_cfg_t *cfg);

// MAC CFG
int mac_cfg(uint32_t port_num, uint32_t speed, uint32_t num_lanes);

// mac enable or disable
int mac_enable(uint32_t port_num, uint32_t speed,
               uint32_t num_lanes, bool enable);

// mac software reset
int mac_soft_reset(uint32_t port_num, uint32_t speed,
                   uint32_t num_lanes, bool reset);

// mac stats reset
int mac_stats_reset(uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool reset);

// mac interrupt enable or disable
int mac_intr_enable(uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool enable);

// mac interrupt clear
int mac_intr_clear(uint32_t port_num, uint32_t speed, uint32_t num_lanes);

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_PORT_MAC_HPP__
