// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_SERDES_HPP__
#define __SDK_PORT_SERDES_HPP__

namespace sdk {
namespace linkmgr {

typedef struct serdes_fn_s_ {
    int (*serdes_cfg) (uint32_t sbus_addr, serdes_info_t *serdes_info);
    int (*serdes_tx_rx_enable) (uint32_t sbus_addr, bool enable);
    int (*serdes_output_enable) (uint32_t sbus_addr, bool enable);
    int (*serdes_reset) (uint32_t sbus_addr, bool reset);
    bool (*serdes_signal_detect) (uint32_t sbus_addr);
    bool (*serdes_rdy) (uint32_t sbus_addr);
} serdes_fn_t;

extern serdes_fn_t serdes_fns;

sdk_ret_t port_serdes_fn_init(linkmgr_cfg_t *cfg);

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_PORT_SERDES_HPP__
