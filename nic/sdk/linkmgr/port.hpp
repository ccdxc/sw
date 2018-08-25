// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_PORT_HPP__
#define __SDK_LINKMGR_PORT_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "linkmgr_types.hpp"
#include "linkmgr.hpp"
#include "port_mac.hpp"
#include "port_serdes.hpp"

namespace sdk {
namespace linkmgr {

class port {
public:
    port_oper_status_t oper_status(void) const {
        return this->oper_status_;
    }

    void set_oper_status(port_oper_status_t oper_status) {
        this->oper_status_ = oper_status;
    }

    port_speed_t port_speed(void) const {
        return this->port_speed_;
    }

    void set_port_speed(port_speed_t port_speed) {
        this->port_speed_ = port_speed;
    }

    port_type_t port_type(void) const {
        return this->port_type_;
    }

    void set_port_type(port_type_t port_type) {
        this->port_type_ = port_type;
    }

    uint32_t port_num(void) { return this->port_num_; }
    void set_port_num(uint32_t port_num) {
        this->port_num_ = port_num;
    }

    void set_admin_state(port_admin_state_t admin_state) {
        this->admin_state_ = admin_state;
    }

    port_admin_state_t admin_state(void) const {
        return this->admin_state_;
    }

    uint32_t mac_ch(void) const { return this->mac_ch_; }
    void set_mac_ch(uint32_t mac_ch) { this->mac_ch_ = mac_ch; }

    uint32_t mac_id(void) const { return this->mac_id_; }
    void set_mac_id(uint32_t mac_id) { this->mac_id_ = mac_id; }

    uint32_t num_lanes(void) const { return this->num_lanes_; }
    void set_num_lanes(uint32_t num_lanes) { this->num_lanes_ = num_lanes; }

    port_fec_type_t fec_type(void) const { return this->fec_type_; }
    void set_fec_type(port_fec_type_t fec_type) { this->fec_type_ = fec_type; }

    bool auto_neg_enable(void) const { return this->auto_neg_enable_; }
    void set_auto_neg_enable(bool auto_neg_enable) {
        this->auto_neg_enable_ = auto_neg_enable;
    }

    uint32_t debounce_time(void) const { return this->debounce_time_; }
    void set_debounce_time(uint32_t debounce_time) {
        this->debounce_time_ = debounce_time;
    }

    port_link_sm_t port_link_sm(void) const { return this->link_sm_; }
    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
    }

    void set_port_link_dfe_sm(port_link_sm_t link_sm) {
        this->link_dfe_sm_ = link_sm;
    }

    void *link_bring_up_timer(void) const {
        return this->link_bring_up_timer_;
    }

    void set_link_bring_up_timer(void *link_bring_up_timer) {
        this->link_bring_up_timer_ = link_bring_up_timer;
    }

    void *link_debounce_timer(void) const {
        return this->link_debounce_timer_;
    }

    void set_link_debounce_timer(void *link_debounce_timer) {
        this->link_debounce_timer_ = link_debounce_timer;
    }

    void set_mtu(uint32_t mtu) { this->mtu_ = mtu; }

    void set_mac_fns(mac_fn_t *mac_fns) { this->mac_fns_ = mac_fns; }

    void set_serdes_fns(serdes_fn_t *serdes_fns) {
        this->serdes_fns_ = serdes_fns;
    }

    sdk_ret_t port_enable(void);
    sdk_ret_t port_disable(void);
    sdk_ret_t port_link_sm_process(void);
    bool      port_link_sm_dfe_process(void);
    bool      port_dfe_tuning_enabled(void);
    bool      port_link_status(void);

    // Link down event handler
    sdk_ret_t port_link_dn_handler(void);

    // Notify link events to others
    sdk_ret_t port_event_notify(port_event_t event);

    // debounce timer expiration handler
    sdk_ret_t port_debounce_timer(void);

    bool bringup_timer_expired(void);

    // ----------------------------------------------------
    // mac methods
    // ----------------------------------------------------

    // MAC CFG
    sdk_ret_t port_mac_cfg(void);

    // mac enable or disable
    sdk_ret_t port_mac_enable(bool);

    // mac software reset
    sdk_ret_t port_mac_soft_reset(bool);

    // mac stats reset
    sdk_ret_t port_mac_stats_reset(bool);

    // mac interrupt enable or disable
    sdk_ret_t port_mac_intr_en(bool enable);

    // mac interrupt clear
    sdk_ret_t port_mac_intr_clr(void);

    // tdm config and any other mac init
    sdk_ret_t port_mac_init(void);

    // mac fifo ctrl 1 configure
    sdk_ret_t port_mac_fifo_ctrl(void);

    // set the mac global mode
    sdk_ret_t port_mac_global_mode_cfg(void);

    // mac channel enable
    sdk_ret_t port_mac_ch_enable(bool);

    // mac generic cfg
    // app fifo portmap
    // rx fifo control
    // channel mapping
    sdk_ret_t port_mac_generic_cfg(void);

    // mac rx and tx enable
    sdk_ret_t port_mac_rx_tx_enable(bool, bool);

    //mac channel mode config
    sdk_ret_t port_mac_ch_mode_cfg(void);

    // mac faults
    bool port_mac_faults_get(void);

    // mac sync
    bool port_mac_sync_get(void);

    // ----------------------------------------------------
    // serdes methods
    // ----------------------------------------------------

    // serdes config
    sdk_ret_t port_serdes_cfg(void);

    // enable serdes tx and rx
    sdk_ret_t port_serdes_tx_rx_enable(bool enable);

    // enable serdes output
    sdk_ret_t port_serdes_output_enable(bool enable);

    // serdes spico reset
    sdk_ret_t port_serdes_reset(bool reset);

    // serdes signal detect
    bool port_serdes_signal_detect(void);

    // check for serdes ready
    bool port_serdes_rdy(void);
    bool port_serdes_dfe_complete(void);

    // set the sbus addr for each serdes
    sdk_ret_t sbus_addr_set (uint32_t lane, uint32_t sbus_addr);

    int port_serdes_ical_start(void);
    int port_serdes_pcal_start(void);
    int port_serdes_pcal_continuous_start(void);

    // ----------------------------------------------------
    // static methods
    // ----------------------------------------------------

    static sdk_ret_t port_init(linkmgr_cfg_t *cfg);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_enable(port *port_p);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_disable(port *port_p);

private:
    uint32_t              port_num_;                  // uplink port number
    port_oper_status_t    oper_status_;               // port operational status
    port_speed_t          port_speed_;                // port speed
    port_type_t           port_type_;                 // port type
    port_admin_state_t    admin_state_;               // port admin state
    port_link_sm_t        link_sm_;                   // port link state machine
    port_link_sm_t        link_dfe_sm_;               // port link DFE state machine
    port_fec_type_t       fec_type_;                  // FEC type
    bool                  auto_neg_enable_;           // Enable AutoNeg
    void                  *link_bring_up_timer_;      // port link bring up timer
    void                  *link_debounce_timer_;      // port link debounce timer
    uint32_t              mac_id_;                    // mac instance for this port
    uint32_t              mac_ch_;                    // mac channel within mac instance
    uint32_t              num_lanes_;                 // number of lanes for this port
    uint32_t              mtu_;                       // number of lanes for this port
    uint32_t              debounce_time_;             // Debounce time in ms
    uint32_t              bringup_timer_val_;         // current bringup timer value
    uint32_t              sbus_addr_[MAX_PORT_LANES]; // sbus addr for each serdes

    mac_fn_t              *mac_fns_;                  // mac functions
    serdes_fn_t           *serdes_fns_;               // serdes functions

    sdk::types::cable_type_t cable_type_;                // cable type

    // MAC port num calculation based on mac instance and mac channel
    uint32_t  port_mac_port_num_calc(void);

    // Get serdes sbus address for a port lane
    uint32_t port_sbus_addr(uint32_t lane);

    mac_fn_t*    mac_fns(void)    { return this->mac_fns_; }     // mac functions
    serdes_fn_t* serdes_fns(void) { return this->serdes_fns_; }  // serdes functions
};

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_LINKMGR_PORT_HPP__
