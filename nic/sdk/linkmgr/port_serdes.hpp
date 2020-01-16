// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PORT_SERDES_HPP__
#define __SDK_PORT_SERDES_HPP__

#define MAX_SERDES_EYE_HEIGHTS 4
#define BIN_ENCODE_ASSERT_LINK_STATUS_SHIFT 15

namespace sdk {
namespace linkmgr {

typedef struct serdes_fn_s_ {
    int (*serdes_basic_cfg)    (uint32_t sbus_addr, serdes_info_t *serdes_info);
    int (*serdes_cfg)          (uint32_t sbus_addr, serdes_info_t *serdes_info);
    int (*serdes_tx_rx_enable)     (uint32_t sbus_addr, bool enable);
    int (*serdes_output_enable)    (uint32_t sbus_addr, bool enable);
    int (*serdes_reset)            (uint32_t sbus_addr, bool reset);
    bool (*serdes_signal_detect)   (uint32_t sbus_addr);
    bool (*serdes_rdy)             (uint32_t sbus_addr);
    int (*serdes_an_init)          (uint32_t sbus_addr,
                                    serdes_info_t *serdes_info);
    int (*serdes_an_start)         (uint32_t sbus_addr,
                                    serdes_info_t *serdes_info,
                                    uint32_t user_cap,
                                    bool fec_ability,
                                    uint32_t fec_request);
    bool (*serdes_an_wait_hcd)     (uint32_t sbus_addr);
    int (*serdes_an_hcd_read)      (uint32_t sbus_addr);
    int (*serdes_an_core_status)   (uint32_t sbus_addr);
    int (*serdes_an_hcd_cfg)       (uint32_t sbus_addr,
                                    uint32_t *sbus_addr_arr);
    int (*serdes_invert_cfg)       (uint32_t sbus_addr,
                                    serdes_info_t *serdes_info);
    int (*serdes_ical_start)       (uint32_t sbus_addr);
    int (*serdes_pcal_start)       (uint32_t sbus_addr);
    int (*serdes_an_pcal_start)    (uint32_t sbus_addr);
    int (*serdes_dfe_status)       (uint32_t sbus_addr);
    int (*serdes_eye_get)          (uint32_t sbus_addr, int eye_type);
    int (*serdes_rx_lpbk)          (uint32_t sbus_addr, bool enable);
    int (*serdes_spico_reset)      (uint32_t sbus_addr);
    int (*serdes_sbus_reset)       (uint32_t sbus_addr, int hard);
    int (*serdes_spico_upload)     (uint32_t sbus_addr, const char*);
    int (*serdes_spico_status)     (uint32_t sbus_addr);
    int (*serdes_get_rev)          (uint32_t sbus_addr);
    int (*serdes_get_build_id)     (uint32_t sbus_addr);
    bool (*serdes_spico_crc)       (uint32_t sbus_addr);
    uint32_t (*serdes_get_eng_id)  (uint32_t sbus_addr);
    void (*serdes_aacs_start)      (int port);
    int (*serdes_pcal_continuous_start) (uint32_t sbus_addr);
    int (*serdes_set_tx_eq)        (uint32_t sbus_addr, uint32_t amp,
                                    uint32_t pre, uint32_t post);
    int (*serdes_set_rx_term)      (uint32_t sbus_addr, uint8_t val);
    bool (*serdes_spico_int)       (uint32_t sbus_addr,
                                    int int_code, int int_data);
    uint32_t (*serdes_get_errors)  (uint32_t sbus_addr, bool clear);
    int (*serdes_prbs_start)       (uint32_t sbus_addr, serdes_info_t *serdes_info);

    int (*serdes_an_fec_enable_read)   (uint32_t sbus_addr);
    int (*serdes_an_rsfec_enable_read) (uint32_t sbus_addr);
    int (*serdes_eye_check)        (uint32_t sbus_addr, uint32_t *values);
} serdes_fn_t;

extern serdes_fn_t serdes_fns;

sdk_ret_t
port_serdes_fn_init(platform_type_t platform_type,
                    uint32_t        jtag_id,
                    int             num_sbus_rings,
                    bool            aacs_server_en,
                    bool            aacs_connect,
                    int             port,
                    std::string     ip);

void serdes_get_ip_info(int chip_reset);
int serdes_sbm_set_sbus_clock_divider (int val);

uint32_t
sbus_access (uint32_t sbus_addr,
             unsigned char reg_addr,
             unsigned char command,
             uint *sbus_data);

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_PORT_SERDES_HPP__
