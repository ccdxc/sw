// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __NIC_HAL_PD_CONTROL_PORT_HPP__
#define __NIC_HAL_PD_CONTROL_PORT_HPP__

#include "nic/include/base.h"
#include "nic/gen/proto/hal/port.pb.h"
#include "nic/hal/src/port.hpp"

namespace hal {
namespace pd {

#define PORT_LANES_MAX 4

enum class port_link_sm_t {
    PORT_LINK_SM_DISABLED,
    PORT_LINK_SM_ENABLED,
    PORT_LINK_SM_SERDES_CFG,
    PORT_LINK_SM_MAC_CFG,
    PORT_LINK_SM_SIGNAL_DETECT,
    PORT_LINK_SM_WAIT_SERDES_RDY,
    PORT_LINK_SM_MAC_ENABLE,
    PORT_LINK_SM_WAIT_MAC_CLEAR_FAULTS,
    PORT_LINK_SM_UP
};

typedef struct mac_fn_s_ {
    int (*mac_cfg) (uint32_t port_num, uint32_t speed, uint32_t num_lanes);

    int (*mac_enable) (uint32_t port_num, uint32_t speed,
                       uint32_t num_lanes, bool enable);

    int (*mac_soft_reset) (uint32_t port_num, uint32_t speed,
                           uint32_t num_lanes, bool reset);

    int (*mac_stats_reset) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool reset);

    int (*mac_intr_clear) (uint32_t port_num, uint32_t speed,
                           uint32_t num_lanes);

    int (*mac_intr_enable) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool enable);
} mac_fn_t;

class port {
public:
    ::port::PortOperStatus oper_status() {
        return this->oper_status_;
    }

    void set_oper_status(::port::PortOperStatus oper_status) {
        this->oper_status_ = oper_status;
    }

    ::port::PortSpeed port_speed() {
        return this->port_speed_;
    }

    void set_port_speed(::port::PortSpeed port_speed) {
        this->port_speed_ = port_speed;
    }

    ::port::PortType port_type() {
        return this->port_type_;
    }

    void set_port_type(::port::PortType port_type) {
        this->port_type_ = port_type;
    }

    ::port::PortAdminState admin_state() {
        return this->admin_state_;
    }

    uint32_t mac_ch() { return this->mac_ch_; }
    void set_mac_ch(uint32_t mac_ch) { this->mac_ch_ = mac_ch; }

    uint32_t mac_id() { return this->mac_id_; }
    void set_mac_id(uint32_t mac_id) { this->mac_id_ = mac_id; }

    uint32_t num_lanes() { return this->num_lanes_; }
    void set_num_lanes(uint32_t num_lanes) { this->num_lanes_ = num_lanes; }

    void set_pi_p(hal::port_t *pi_p) { this->pi_p_ = pi_p; }

    port_link_sm_t port_link_sm() {
        return this->link_sm_;
    }

    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
    }

    hal_ret_t port_enable();
    hal_ret_t port_disable();
    hal_ret_t port_link_sm_process();
    hal_ret_t port_serdes_cfg();

    // MAC CFG
    hal_ret_t port_mac_cfg();

    // mac enable or disable
    hal_ret_t port_mac_enable(bool);

    // mac software reset
    hal_ret_t port_mac_soft_reset(bool);

    // mac stats reset
    hal_ret_t port_mac_stats_reset(bool);

    // mac interrupt enable or disable
    hal_ret_t port_mac_intr_en(bool enable);

    // mac interrupt clear
    hal_ret_t port_mac_intr_clr();

    // tdm config and any other mac init
    hal_ret_t port_mac_init();

    // mac fifo ctrl 1 configure
    hal_ret_t port_mac_fifo_ctrl();

    // set the mac global mode
    hal_ret_t port_mac_global_mode_cfg();

    // mac channel enable
    hal_ret_t port_mac_ch_enable(bool);

    // mac generic cfg
    // app fifo portmap
    // rx fifo control
    // channel mapping
    hal_ret_t port_mac_generic_cfg();

    // mac rx and tx enable
    hal_ret_t port_mac_rx_tx_enable(bool, bool);

    //mac channel mode config
    hal_ret_t port_mac_ch_mode_cfg();

    static hal_ret_t
        link_bring_up_timer_cb(uint32_t timer_id, void *ctxt);

    static hal_ret_t port_event_notify(uint8_t opn, void *ctxt);

    static hal_ret_t port_init(bool is_sim);

    static hal_ret_t port_mac_init(bool is_sim);

    static mac_fn_t mac_fn;

private:
    hal::port_t             *pi_p_;          // PI structure

    ::port::PortOperStatus  oper_status_;    // port operational status
    ::port::PortSpeed       port_speed_;     // port speed
    ::port::PortType        port_type_;      // port type
    ::port::PortAdminState  admin_state_;    // port admin state

    port_link_sm_t        link_sm_;        // port link state machine
    void                  *link_bring_up_timer_;   // port link bring up timer

    uint32_t              mac_id_;          // mac instance for this port
    uint32_t              mac_ch_;          // mac channel within mac instance
    uint32_t              num_lanes_;       // number of lanes for this port

    // MAC port num calculation based on mac instance and mac channel
    uint32_t  port_mac_port_num_calc();
};

}    // namespace pd
}    // namespace hal

#endif  // __NIC_HAL_PD_CONTROL_PORT_HPP__
