// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PORT_HPP__
#define __PORT_HPP__

#include "nic/include/base.h"

namespace hal {
namespace pd {

// starting point for control thread
void *hal_control_start(void *ctxt);

enum class port_speed_t {
    PORT_SPEED_25G,
    PORT_SPEED_10G
};

enum class port_type_t {
    PORT_TYPE_ETH
};

enum class port_oper_status_t {
    PORT_OPER_STATUS_DOWN,
    PORT_OPER_STATUS_UP
};

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

typedef uint32_t port_num_t;

class port {
public:
    port(port_num_t port_num_) {
        this->port_num_ = port_num_;
    }

    port_num_t get_port_num() {
        return this->port_num_;
    }

    bool get_admin_state() {
        return this->admin_state_;
    }

    port_oper_status_t get_oper_status() {
        return this->oper_status_;
    }

    void set_oper_status(port_oper_status_t oper_status) {
        this->oper_status_ = oper_status;
    }

    port_speed_t get_port_speed() {
        return this->port_speed_;
    }

    void set_port_speed(port_speed_t port_speed) {
        this->port_speed_ = port_speed;
    }

    port_type_t get_port_type() {
        return this->port_type_;
    }

    void set_port_type(port_type_t port_type) {
        this->port_type_ = port_type;
    }

    port_link_sm_t get_port_link_sm() {
        return this->link_sm_;
    }

    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
    }

    hal_ret_t port_enable();
    hal_ret_t port_disable();
    hal_ret_t port_link_sm();
    hal_ret_t port_serdes_cfg();

    // MAC CFG
    hal_ret_t port_mac_cfg();
    hal_ret_t port_mac_init();
    hal_ret_t port_mac_soft_reset(bool);
    hal_ret_t port_mac_fifo_ctrl();
    hal_ret_t port_mac_global_mode_cfg();
    hal_ret_t port_mac_ch_enable(bool);
    hal_ret_t port_mac_generic_cfg();
    hal_ret_t port_mac_rx_tx_enable(bool, bool);
    hal_ret_t port_mac_ch_mode_cfg();
    hal_ret_t port_mac_stats_reset(bool);

    static hal_ret_t
        link_bring_up_timer_cb(uint32_t timer_id, void *ctxt);

    static hal_ret_t port_event_notify(void *ctxt);

private:
    port_num_t          port_num_;       // port number
    port_oper_status_t  oper_status_;    // port operational status
    bool                admin_state_;    // port admin state
    port_speed_t        port_speed_;     // port speed
    port_type_t         port_type_;      // port type
    port_link_sm_t      link_sm_;        // port link state machine
    void                *link_bring_up_timer;   // port link bring up timer
};

}    // namespace pd
}    // namespace hal

#endif  // __PORT_HPP__
