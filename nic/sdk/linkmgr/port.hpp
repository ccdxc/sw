// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_PORT_HPP__
#define __SDK_LINKMGR_PORT_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/eth.hpp"
#include "include/sdk/timestamp.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "lib/utils/in_mem_fsm_logger.hpp"
#include "linkmgr_types.hpp"
#include "linkmgr.hpp"
#include "port_mac.hpp"
#include "port_serdes.hpp"

#define MAX_PORT_SERDES_READY_RETRIES               10
#define MAX_PORT_SERDES_SIG_DETECT_RETRIES          10
#define MAX_PORT_QSFP_AN_HCD_RETRIES                10
#define MAX_PORT_SFP_AN_HCD_RETRIES                 4
#define MAX_PORT_MAC_SYNC_RETRIES                   10
#define MAX_PORT_MAC_SYNC_RETRIES_10G               20
#define MAX_PORT_SERDES_DFE_ICAL_CMPLT_RETRIES      10             // DFE ICAL complete
#define MAX_PORT_SERDES_DFE_ICAL_EYE_RETRIES        3              // DFE ICAL eye values
#define MAX_PORT_MAC_FAULTS_CHECK                   3
#define MAX_PORT_MAC_NOFAULTS_CHECK                 3
#define PORT_MAC_STAT_REPORT_SIZE                   1024
#define MIN_PORT_TIMER_INTERVAL                     100            // msecs
#define MAX_LINK_AN_WAIT_TIME                       100 * 1000000  // in nanosecs
#define MAX_PORT_FEC_RETRIES                        3              // FC/RS/None FEC

using sdk::utils::in_mem_fsm_logger;

namespace sdk {
namespace linkmgr {

typedef enum an_ret_e {
    AN_RESET,
    AN_WAIT,
    AN_DONE,
    AN_SKIP,
} an_ret_t;

typedef enum dfe_ret_e {
    DFE_RESET,
    DFE_WAIT,
    DFE_DONE,
} dfe_ret_t;

typedef enum neg_mode_e {
    FIXED_NEG = 0,
    AUTO_NEG = 1,
} neg_mode_t;

class port {
public:
    port(size_t sm_logger_capacity=64) {
        sm_logger_ = in_mem_fsm_logger::factory(sm_logger_capacity,
                                                sizeof(port_link_sm_t));
    }

    ~port() {
        in_mem_fsm_logger::destroy(sm_logger_);
    }

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

    uint32_t cable_type(void) { return this->cable_type_; }
    void set_cable_type(cable_type_t cable_type) {
        this->cable_type_ = cable_type;
    }

    port_loopback_mode_t loopback_mode(void) { return this->loopback_mode_; }
    void set_loopback_mode(port_loopback_mode_t loopback_mode) {
        this->loopback_mode_ = loopback_mode;
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

    void set_user_admin_state(port_admin_state_t user_admin_state) {
        this->user_admin_state_ = user_admin_state;
    }

    port_admin_state_t user_admin_state(void) const {
        return this->user_admin_state_;
    }

    uint32_t mac_ch(void) const { return this->mac_ch_; }
    void set_mac_ch(uint32_t mac_ch) { this->mac_ch_ = mac_ch; }

    uint32_t mac_id(void) const { return this->mac_id_; }
    void set_mac_id(uint32_t mac_id) { this->mac_id_ = mac_id; }

    uint32_t num_lanes(void) const { return this->num_lanes_; }
    void set_num_lanes(uint32_t num_lanes) { this->num_lanes_ = num_lanes; }

    uint32_t num_lanes_cfg(void) const { return this->num_lanes_cfg_; }
    void set_num_lanes_cfg(uint32_t num_lanes_cfg) {
        this->num_lanes_cfg_ = num_lanes_cfg;
    }

    port_fec_type_t fec_type(void) const { return this->fec_type_; }
    void set_fec_type(port_fec_type_t fec_type) { this->fec_type_ = fec_type; }

    port_fec_type_t user_fec_type(void) const { return this->user_fec_type_; }
    void set_user_fec_type(port_fec_type_t user_fec_type) {
        this->user_fec_type_ = user_fec_type;
    }

    port_fec_type_t derived_fec_type(void) const {
        return this->derived_fec_type_;
    }
    void set_derived_fec_type(port_fec_type_t derived_fec_type) {
        this->derived_fec_type_ = derived_fec_type;
    }

    uint32_t fec_retries(void) const { return this->fec_retries_; }
    void set_fec_retries(uint32_t count) {
        this->fec_retries_ = count;
    }

    bool auto_neg_enable(void) const { return this->auto_neg_enable_; }
    void set_auto_neg_enable(bool auto_neg_enable) {
        this->auto_neg_enable_ = auto_neg_enable;
    }

    bool auto_neg_cfg(void) const { return this->auto_neg_cfg_; }
    void set_auto_neg_cfg(bool auto_neg_cfg) {
        this->auto_neg_cfg_ = auto_neg_cfg;
    }

    bool toggle_neg_mode(void) const { return this->toggle_neg_mode_; }
    void set_toggle_neg_mode(bool mode) {
        this->toggle_neg_mode_ = mode;
    }

    bool skip_toggle_neg_mode(void) const { return this->skip_toggle_neg_mode_; }
    void set_skip_toggle_neg_mode(bool mode) {
        this->skip_toggle_neg_mode_ = mode;
    }

    bool toggle_fec_mode(void) const { return this->toggle_fec_mode_; }
    void set_toggle_fec_mode(bool toggle_fec_mode) {
        this->toggle_fec_mode_ = toggle_fec_mode;
    }

    port_pause_type_t pause(void) const { return this->pause_; }
    void set_pause(port_pause_type_t pause) {
        this->pause_ = pause;
    }

    bool tx_pause_enable(void) const { return this->tx_pause_enable_; }
    void set_tx_pause_enable(bool tx_pause_enable) {
        this->tx_pause_enable_ = tx_pause_enable;
    }

    bool rx_pause_enable(void) const { return this->rx_pause_enable_; }
    void set_rx_pause_enable(bool rx_pause_enable) {
        this->rx_pause_enable_ = rx_pause_enable;
    }

    uint32_t debounce_time(void) const { return this->debounce_time_; }
    void set_debounce_time(uint32_t debounce_time) {
        this->debounce_time_ = debounce_time;
    }

    port_link_sm_t port_link_sm(void) const { return this->link_sm_; }
    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
        report_sm_(link_sm);
    }

    port_link_sm_t port_link_dfe_sm(void) { return this->link_dfe_sm_; }
    void set_port_link_dfe_sm(port_link_sm_t link_sm) {
        this->link_dfe_sm_ = link_sm;
        report_sm_(link_sm);
    }

    port_link_sm_t port_link_an_sm(void) { return this->link_an_sm_; }
    void set_port_link_an_sm(port_link_sm_t link_sm) {
        this->link_an_sm_ = link_sm;
        report_sm_(link_sm);
    }

    in_mem_fsm_logger *sm_logger(void) {
        return this->sm_logger_;
    }

    sdk::event_thread::timer_t *link_bringup_timer(void) {
        return &this->link_bringup_timer_;
    }

    sdk::event_thread::timer_t *link_debounce_timer(void) {
        return &this->link_debounce_timer_;
    }

    uint32_t mtu(void) { return this->mtu_; }

    void set_mtu(uint32_t mtu) { this->mtu_ = mtu; }

    uint32_t user_cap(void) { return this->user_cap_; }
    void set_user_cap(uint32_t user_cap) { this->user_cap_ = user_cap; }

    bool fec_ability(void) { return this->fec_ability_; }
    void set_fec_ability(bool fec_ability) { this->fec_ability_ = fec_ability; }

    uint32_t fec_request(void) { return this->fec_request_; }
    void set_fec_request(uint32_t fec_request) { this->fec_request_ = fec_request; }

    void set_mac_fns(mac_fn_t *mac_fns) { this->mac_fns_ = mac_fns; }

    void set_serdes_fns(serdes_fn_t *serdes_fns) {
        this->serdes_fns_ = serdes_fns;
    }

    uint32_t num_retries(void) { return this->num_retries_; }
    void set_num_retries(uint32_t retries) { this->num_retries_ = retries; }

    uint32_t num_link_down(void) { return this->num_link_down_; }
    void set_num_link_down(uint32_t link_down) {
        this->num_link_down_ = link_down;
    }

    const char *last_down_timestamp(void) { return this->last_down_timestamp_; }

    uint64_t bringup_duration_sec(void) { return this->bringup_duration_.tv_sec; }
    uint64_t bringup_duration_nsec(void) { return this->bringup_duration_.tv_nsec; }

    uint32_t mac_faults(void) { return this->mac_faults_; }
    void set_mac_faults(uint32_t faults) { this->mac_faults_ = faults; }

    uint32_t num_mac_nofaults(void) { return this->num_mac_nofaults_; }
    void set_num_mac_nofaults(uint32_t faults) { this->num_mac_nofaults_ = faults; }

    uint32_t num_dfe_ical_cmplt_retries(void) {
        return this->num_dfe_ical_cmplt_retries_;
    }
    void set_num_dfe_ical_cmplt_retries(uint32_t num_dfe) {
        this->num_dfe_ical_cmplt_retries_ = num_dfe;
    }

    uint32_t num_dfe_ical_eye_retries(void) {
        return this->num_dfe_ical_eye_retries_;
    }
    void set_num_dfe_ical_eye_retries(uint32_t num_dfe) {
        this->num_dfe_ical_eye_retries_ = num_dfe;
    }

    uint32_t num_an_hcd_retries(void) { return this->num_an_hcd_retries_; }
    void set_num_an_hcd_retries(uint32_t num_an_hcd) {
        this->num_an_hcd_retries_ = num_an_hcd;
    }

    uint32_t num_mac_sync_retries(void) { return this->num_mac_sync_retries_; }
    void set_num_mac_sync_retries(uint32_t num_mac_sync) {
        this->num_mac_sync_retries_ = num_mac_sync;
    }

    uint32_t num_link_train_retries(void) { return this->num_link_train_retries_; }
    void set_num_link_train_retries(uint32_t num_link_train) {
        this->num_link_train_retries_ = num_link_train;
    }

    uint32_t num_serdes_ready_retries(void) { return this->num_serdes_ready_retries_; }
    void set_num_serdes_ready_retries(uint32_t num_serdes_ready) {
        this->num_serdes_ready_retries_ = num_serdes_ready;
    }

    uint32_t num_sig_detect_retries(void) { return this->num_sig_detect_retries_; }
    void set_num_sig_detect_retries(uint32_t num) {
        this->num_sig_detect_retries_ = num;
    }

    sdk_ret_t port_enable(bool start_en_timer = false);
    sdk_ret_t port_disable(void);
    sdk_ret_t port_link_sm_process(bool start_en_timer = false);
    dfe_ret_t port_link_sm_dfe_process(void);
    an_ret_t  port_link_sm_an_process(void);
    bool      port_dfe_tuning_enabled(void);
    bool      port_link_status(void);

    // Link down event handler
    sdk_ret_t port_link_dn_handler(void);

    // Notify link events to others
    sdk_ret_t port_event_notify(port_event_t event);

    // debounce timer expiration handler
    sdk_ret_t port_debounce_timer_cb(void);

    bool bringup_timer_expired(void);

    sdk_ret_t port_flush_set(bool);
    sdk_ret_t port_deinit (void);
    sdk_ret_t port_pb_enable(bool enable);
    sdk_ret_t port_pb_drain(void);
    sdk_ret_t port_pb_write_control(bool enable);
    sdk_ret_t port_pb_flush(bool enable);

    // ----------------------------------------------------
    // mac methods
    // ----------------------------------------------------

    // MAC CFG
    sdk_ret_t port_mac_cfg(void);

    // MAC FEC CFG
    sdk_ret_t port_mac_cfg_fec(port_fec_type_t fec_type);

    // mac enable or disable
    sdk_ret_t port_mac_enable(bool);

    // mac software reset
    sdk_ret_t port_mac_soft_reset(bool);

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
    sdk_ret_t port_mac_faults_clear(void);

    // mac sync
    bool port_mac_sync_get(void);

    // mac stats
    sdk_ret_t port_mac_stats_init(void);
    sdk_ret_t port_mac_stats_reset(bool);
    sdk_ret_t port_mac_stats_get(uint64_t *stats_data);
    sdk_ret_t port_mac_stats_publish(uint64_t *stats_data);

    // mac pause src addr
    sdk_ret_t port_mac_set_pause_src_addr(uint8_t *mac_addr);

    // send/clear sending remote faults
    sdk_ret_t port_mac_send_remote_faults(bool send);

    // mac TX drain
    sdk_ret_t port_mac_tx_drain(bool drain);

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
    int  port_serdes_an_start(void);
    bool port_serdes_an_wait_hcd(void);
    int  port_serdes_an_hcd_cfg(void);
    bool port_serdes_an_link_train_check(void);

    // set the sbus addr for each serdes
    sdk_ret_t sbus_addr_set (uint32_t lane, uint32_t sbus_addr);

    int port_serdes_ical_start(void);
    int port_serdes_pcal_start(void);
    int port_serdes_pcal_continuous_start(void);
    bool port_serdes_eye_check(void);
    int port_serdes_eye_get(void);

    // ----------------------------------------------------
    // static methods
    // ----------------------------------------------------
    static sdk_ret_t port_init(linkmgr_cfg_t *cfg);
    static sdk_ret_t phy_port_mac_addr(uint32_t phy_port, mac_addr_t mac_addr);
    static uint32_t port_num_to_asic_port(uint32_t port_num) {
        return port_num - 1;
    }
    static uint32_t port_num_to_asic_num(uint32_t port_num) {
        return 0;
    }

    static sdk_ret_t port_mac_stats_init(port *port_p);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_enable(port *port_p);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_disable(port *port_p);

    // set and start the timer
    static sdk_ret_t port_timer_start(sdk::event_thread::timer_t *timer,
                                      double timeout);
    // init the bringup and debounce timers
    sdk_ret_t timers_init(void);

private:
    uint32_t                  port_num_;                  // uplink port number
    port_oper_status_t        oper_status_;               // port operational status
    port_speed_t              port_speed_;                // port speed
    port_type_t               port_type_;                 // port type
    port_admin_state_t        admin_state_;               // port admin state
    port_admin_state_t        user_admin_state_;          // port user configured admin state
    port_link_sm_t            link_sm_;                   // port link state machine
    port_link_sm_t            link_dfe_sm_;               // port link DFE state machine
    port_link_sm_t            link_an_sm_;                // port link AN state machine
    port_fec_type_t           fec_type_;                  // operational FEC type
    port_fec_type_t           user_fec_type_;             // configured FEC type
    port_fec_type_t           derived_fec_type_;          // FEC derived based on AN or user config
    uint32_t                  fec_retries_;               // FEC retry count
    port_pause_type_t         pause_;                     // Enable MAC pause
    bool                      tx_pause_enable_;           // Enable MAC Tx Pause
    bool                      rx_pause_enable_;           // Enable MAC Rx Pause
    bool                      auto_neg_enable_;           // Enable AutoNeg
    bool                      auto_neg_cfg_;              // user configured AutoNeg
    sdk::event_thread::timer_t link_bringup_timer_;       // port link bring up timer
    sdk::event_thread::timer_t link_debounce_timer_;      // port link debounce timer
    bool                      toggle_neg_mode_;           // for SFP+ toggle between auto_neg/force modes until link-up
    bool                      skip_toggle_neg_mode_;      // skip toggle between auto_neg/forced. Overrides toggle_neg_mode_
    neg_mode_t                last_neg_mode_;             // last toggle was auto_neg or force mode
    bool                      toggle_fec_mode_;           // for SFP+ toggle between RS/FC/None fec modes
    uint32_t                  mac_id_;                    // mac instance for this port
    uint32_t                  mac_ch_;                    // mac channel within mac instance
    uint32_t                  num_lanes_;                 // number of lanes for this port
    uint32_t                  num_lanes_cfg_;             // user configured number of lanes for this port
    uint32_t                  mtu_;                       // number of lanes for this port
    uint32_t                  debounce_time_;             // Debounce time in ms
    uint32_t                  bringup_timer_val_;         // current bringup timer value
    char                      last_down_timestamp_[TIME_STR_SIZE]; // last down time in string format
    timespec_t                last_down_ts_;              // ts at which link went down last
    timespec_t                last_up_ts_;                // ts at which link went up last
    timespec_t                bringup_duration_;          // time taken for link to come up: last_up_ts_ - last_down_ts_
    uint32_t                  user_cap_;                  //  AN user_cap
    bool                      fec_ability_;               //  AN fec_ability
    uint32_t                  fec_request_;               //  AN fec_request
    uint32_t                  sbus_addr_[MAX_PORT_LANES]; // sbus addr for each serdes
    mac_fn_t                  *mac_fns_;                  // mac functions
    serdes_fn_t               *serdes_fns_;               // serdes functions
    uint32_t                  num_retries_;               // max linkup retries
    uint32_t                  num_link_down_;             // number of link down
    cable_type_t              cable_type_;                // cable type
    port_loopback_mode_t      loopback_mode_;             // port loopback mode - MAC/PHY
    uint32_t                  mac_faults_;                // number of times MAC faults detected
    uint32_t                  num_mac_nofaults_;          // number of times no MAC faults were detected (applicable for 10G/25G-no-fec)
    uint32_t                  num_dfe_ical_cmplt_retries_; // number of times ical complete is retried in one pass of SM
    uint32_t                  num_dfe_ical_eye_retries_;  // number of times ical eye check is retried in one pass of SM
    uint32_t                  num_an_hcd_retries_;        // number of times AN HCD was retried in one pass of SM
    uint32_t                  num_mac_sync_retries_;      // number of times MAC sync is retried in one pass of SM
    uint32_t                  num_link_train_retries_;    // number of times link training failed in one pass of SM
    uint32_t                  num_serdes_ready_retries_;  // number of times serdes ready was retried in one pass of SM
    uint32_t                  num_sig_detect_retries_;    // number of times serdes signal detect was retried in one pass of SM
    bool                      persist_stats_collect_;     // set after initial link-up state; never reset (? TBD)
    uint64_t                  persist_stats_data_[MAX_MAC_STATS]; // saved stats before link flap or mac resets;
    sdk::types::mem_addr_t    port_stats_base_addr_;      // Base address for this port's stats
    in_mem_fsm_logger         *sm_logger_;                // Logs Port state transitions w.r.t. port state machine.

    uint32_t port_max_an_retries(void);
    uint32_t port_max_serdes_ready_retries(void);
    neg_mode_t is_auto_neg(void);
    void toggle_negotiation_mode(void);
    void toggle_fec_type(void);

    // MAC port num calculation based on mac instance and mac channel
    uint32_t  port_mac_port_num_calc(void);

    // Get serdes sbus address for a port lane
    uint32_t port_sbus_addr(uint32_t lane);

    sdk_ret_t port_link_sm_reset (void);

    int port_set_an_resolved_params(int an_hcd,
                                    int fec_enable,
                                    int rsfec_enable);

    int port_set_an_resolved_params_internal(port_speed_t speed,
                                             int num_lanes,
                                             port_fec_type_t fec_type);

    port_speed_t port_speed_to_serdes_speed(port_speed_t port_speed);

    mac_fn_t*    mac_fns(void)    { return this->mac_fns_; }     // mac functions
    serdes_fn_t* serdes_fns(void) { return this->serdes_fns_; }  // serdes functions

    sdk_ret_t port_mac_state_reset(void);
    sdk_ret_t port_mac_stats_errata_correct(uint64_t *stats_data);
    sdk_ret_t port_serdes_state_reset(void);
    void port_link_sm_counters_reset(void);
    sdk_ret_t port_link_sm_retry_enabled(bool serdes_reset = true);
    uint32_t port_max_mac_sync_retries(void);
    uint32_t port_mac_sync_ical_timeout(void);
    uint32_t port_max_ical_cmplt_retries(void);

    // methods to support persist mac stats across link flaps
    // note: hw mac stats get cleared upon link flap; we maintain prev in sw
    sdk_ret_t port_mac_stats_persist_collect_enable(void);
    sdk_ret_t port_mac_stats_persist_collect_disable(void);
    sdk_ret_t port_mac_stats_persist_collate(uint64_t *stats_data);
    sdk_ret_t port_mac_stats_persist_update(void);
    sdk_ret_t port_mac_stats_persist_clear(bool reset);

    // methods to update last link down and up times
    sdk_ret_t set_last_down_ts(void);
    sdk_ret_t set_last_up_ts(void);
    sdk_ret_t set_bringup_duration(void);
    void report_sm_(port_link_sm_t link_sm);
};

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_LINKMGR_PORT_HPP__
