// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_MAC_HPP__
#define __SDK_PORT_MAC_HPP__

namespace sdk {
namespace linkmgr {

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

