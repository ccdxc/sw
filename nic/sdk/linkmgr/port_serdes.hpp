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
    int (*serdes_ical_start)(uint32_t sbus_addr);
    int (*serdes_pcal_start)(uint32_t sbus_addr);
    int (*serdes_pcal_continuous_start)(uint32_t sbus_addr);
    int (*serdes_dfe_status)(uint32_t sbus_addr);
    int (*serdes_eye_get) (uint32_t sbus_addr, int eye_type);
    int (*serdes_rx_lpbk) (uint32_t sbus_addr, bool enable);
    int (*serdes_spico_reset) (uint32_t sbus_addr);
    int (*serdes_sbus_reset) (uint32_t sbus_addr, int hard);
    int (*serdes_spico_upload) (uint32_t sbus_addr, const char*);
} serdes_fn_t;

extern serdes_fn_t serdes_fns;

sdk_ret_t port_serdes_fn_init(linkmgr_cfg_t *cfg);

uint32_t
sbus_access (uint32_t sbus_addr,
             unsigned char reg_addr,
             unsigned char command,
             uint *sbus_data);

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_PORT_SERDES_HPP__
