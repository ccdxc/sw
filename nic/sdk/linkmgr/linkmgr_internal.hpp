// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_INTERNAL_HPP__
#define __SDK_LINKMGR_INTERNAL_HPP__

#include <atomic>
#include "lib/logger/logger.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "port.hpp"

namespace sdk {
namespace linkmgr {

#define PORT_TIMER_INIT_TIME    0.1 // secs
#define MAX_PORT_LINKUP_RETRIES 100
#define XCVR_POLL_TIME          1.0 // secs
#define MAX_LOG_SIZE            1024

// max link training fail count before starting timer
#define MAX_LINK_TRAIN_FAIL_COUNT 5

// HBM region for port_stats - need to create same thing here,
// as we cannot reference any declare outside "nic/sdk" domain
#define ASIC_HBM_REG_PORT_STATS         "port_stats"

extern linkmgr_cfg_t g_linkmgr_cfg;
extern char log_buf[];
extern bool hal_cfg;

#define SDK_PORT_SM_DEBUG(port, fmt, ...)                                 \
    SDK_LINK_TRACE_DEBUG("port: %u, MAC_ID: %u, MAC_CH: %u, " fmt,        \
                         port->port_num(), port->mac_id_, port->mac_ch_,  \
                         ##__VA_ARGS__);

#define SDK_PORT_SM_TRACE(port, fmt, ...)                                 \
    SDK_LINK_TRACE_DEBUG("port: %u, MAC_ID: %u, MAC_CH: %u, " fmt,        \
                         port->port_num(), port->mac_id_, port->mac_ch_,  \
                         ##__VA_ARGS__);

#define SDK_LINKMGR_TRACE_DEBUG(fmt, ...)  {                              \
    SDK_LINK_TRACE_DEBUG(fmt, ##__VA_ARGS__);                             \
}

#define SDK_LINKMGR_TRACE_ERR(fmt, ...)  {                                \
    SDK_LINK_TRACE_ERR(fmt, ##__VA_ARGS__);                               \
}

#define SDK_LINKMGR_TRACE_DEBUG_SIZE(logsize, fmt, ...)  {                \
    snprintf(log_buf, logsize, fmt, ##__VA_ARGS__);                       \
    SDK_LINK_TRACE_DEBUG("%s", log_buf);                                  \
}

#define SDK_LINKMGR_TRACE_ERR_SIZE(logsize, fmt, ...)  {                  \
    snprintf(log_buf, logsize, fmt, ##__VA_ARGS__);                       \
    SDK_LINK_TRACE_ERR("%s", log_buf);                                    \
}

typedef enum sdk_timer_id_e {
    SDK_TIMER_ID_LINK_BRINGUP = 1000,   // TODO global unique across hal
    SDK_TIMER_ID_XCVR_POLL,
    SDK_TIMER_ID_LINK_POLL,
    SDK_TIMER_ID_LINK_DEBOUNCE,
    SDK_TIMER_ID_PORT_EVENT
} sdk_timer_id_t;

sdk_ret_t linkmgr_notify(uint8_t operation, linkmgr_entry_data_t *data,
                         q_notify_mode_t mode);
sdk_ret_t port_link_poll_timer_add(port *port);
sdk_ret_t port_link_poll_timer_delete(port *port);
bool is_linkmgr_ctrl_thread(void);
bool is_linkmgr_ctrl_thread_ready(void);
sdk::lib::thread *current_thread(void);
uint32_t glbl_mode_mgmt(mac_mode_t mac_mode);
uint32_t ch_mode_mgmt(mac_mode_t mac_mode, uint32_t ch);
uint32_t glbl_mode(mac_mode_t mac_mode);
uint32_t ch_mode(mac_mode_t mac_mode, uint32_t ch);
uint32_t       num_asic_ports(uint32_t asic);
uint32_t       sbus_addr_asic_port(uint32_t asic, uint32_t asic_port);
uint32_t       jtag_id(void);
uint8_t        num_sbus_rings(void);
uint8_t        sbm_clk_div(void);
bool           aacs_server_en(void);
bool           aacs_connect(void);
uint32_t       aacs_server_port(void);
int            serdes_build_id (void);
int            serdes_rev_id(void);
std::string    serdes_fw_file(void);
std::string    aacs_server_ip(void);
serdes_info_t* serdes_info_get(uint32_t sbus_addr,
                               uint32_t port_speed,
                               uint32_t cable_type);
uint32_t logical_port_to_tm_port(uint32_t logical_port);
void port_bringup_timer_cb(sdk::event_thread::timer_t *timer);
void port_debounce_timer_cb(sdk::event_thread::timer_t *timer);

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_INTERNAL_HPP__
