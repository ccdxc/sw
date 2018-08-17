// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port.hpp"
#include "port_mac.hpp"
#include "port_serdes.hpp"
#include "linkmgr_periodic.hpp"
#include "linkmgr_internal.hpp"
#include "linkmgr_types.hpp"
#include "timer_cb.hpp"
#include "sdk/asic/capri/cap_mx_api.h"

namespace sdk {
namespace linkmgr {

mac_fn_t    mac_fns;
serdes_fn_t serdes_fns;

#define SDK_PORT_TRACE(port, state) { \
    SDK_TRACE_DEBUG("MAC_ID: %d, MAC_CH: %d, state: %s", \
                     port->mac_id_, port->mac_ch_, state); \
}

// Debounce timer expiration handler
sdk_ret_t
port::port_debounce_timer(void)
{
    // Notify if link is still down
    if (port_link_status() == false) {
        // TODO disable the port?
        port_event_notify(port_event_t::PORT_EVENT_LINK_DOWN);
    }

    return SDK_RET_OK;
}

bool
port::port_link_status(void)
{
    return port_mac_sync_get();
}

sdk_ret_t
port::port_mac_init(void)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_fifo_ctrl(void)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_global_mode_cfg(void)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_ch_enable(bool enable)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_generic_cfg(void)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_rx_tx_enable(bool rx_enable, bool tx_enable)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_ch_mode_cfg(void)
{
    return SDK_RET_OK;
}

uint32_t
port::port_mac_port_num_calc(void)
{
    return (this->mac_id_ * MAX_PORT_LANES) + this->mac_ch_;
}

sdk_ret_t
port::port_mac_cfg(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_cfg(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_enable(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_enable(
               mac_port_num,
               static_cast<uint32_t>(this->port_speed_),
               this->num_lanes_,
               enable);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_soft_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_soft_reset(
                   mac_port_num,
                   static_cast<uint32_t>(this->port_speed_),
                   this->num_lanes_,
                   reset);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_stats_reset(
                    mac_port_num,
                    static_cast<uint32_t>(this->port_speed_),
                    this->num_lanes_,
                    reset);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_intr_en(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_intr_enable(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_,
            enable);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_intr_clr(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns.mac_intr_clear(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return SDK_RET_OK;
}

bool
port::port_mac_faults_get(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return mac_fns.mac_faults_get(mac_port_num);
}

bool
port::port_mac_sync_get(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return mac_fns.mac_sync_get(mac_port_num);
}

uint32_t
port::port_sbus_addr(uint32_t lane)
{
    return this->sbus_addr_[lane];
}

sdk_ret_t
port::port_serdes_cfg(void)
{
    uint32_t      lane         = 0;
    uint32_t      sbus_addr    = 0;
    serdes_info_t *serdes_info = NULL;

    for (lane = 0; lane < num_lanes_; ++lane) {
        sbus_addr = port_sbus_addr(lane);

        serdes_info = serdes_info_get(
                                    sbus_addr,
                                    static_cast<uint32_t>(this->port_speed_),
                                    this->cable_type_);

        serdes_fns.serdes_cfg(sbus_addr, serdes_info);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_tx_rx_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns.serdes_tx_rx_enable(port_sbus_addr(lane),
                                            enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_output_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns.serdes_output_enable(port_sbus_addr(lane),
                                             enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_reset(bool reset)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns.serdes_reset(port_sbus_addr(lane),
                                     reset);
    }

    return SDK_RET_OK;
}

bool
port::port_serdes_signal_detect(void)
{
    uint32_t lane = 0;
    bool signal_detect = false;

    for (lane = 0; lane < num_lanes_; ++lane) {
        signal_detect = serdes_fns.serdes_signal_detect(
                                        port_sbus_addr(lane));
        if (signal_detect == false) {
            break;
        }
    }
    return signal_detect;
}

bool
port::port_serdes_rdy(void)
{
    uint32_t lane = 0;
    bool serdes_rdy = false;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_rdy =  serdes_fns.serdes_rdy(port_sbus_addr(lane));
        if (serdes_rdy == false) {
            break;
        }
    }

    return serdes_rdy;
}

bool
port::port_serdes_dfe_complete(void)
{
    return true;
}

sdk_ret_t
port::sbus_addr_set (uint32_t lane, uint32_t sbus_addr)
{
    this->sbus_addr_[lane] = sbus_addr;
    return SDK_RET_OK;
}

bool
port::port_dfe_tuning_enabled(void)
{
    return true;
}

bool
port::port_link_sm_dfe_process(void)
{
    bool dfe_complete = false;
    bool ret          = true;
    int  timeout      = 500; //msecs

    switch(this->link_dfe_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DFE_DISABLED:

            // transition to start ical
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL:

            SDK_PORT_TRACE(this, "start ICAL");

            // transition to wait for ical complete
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL:

            SDK_PORT_TRACE(this, "wait ICAL");

            dfe_complete = port_serdes_dfe_complete();

            if(dfe_complete == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);

                ret = false;
                break;
            }

            // transition to pcal one shot
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL:

            SDK_PORT_TRACE(this, "start PCAL");

            // transition to wait for pcal complete
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL:

            SDK_PORT_TRACE(this, "wait PCAL");

            dfe_complete = port_serdes_dfe_complete();

            if(dfe_complete == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);

                ret = false;
                break;
            }

            // transition to pcal continuous
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_PCAL_CONTINUOUS);

        case port_link_sm_t::PORT_LINK_SM_DFE_PCAL_CONTINUOUS:
            SDK_PORT_TRACE(this, "PCAL continuous");

            break;

        default:
            break;
    }

    return ret;
}

sdk_ret_t
port::port_link_sm_process(void)
{
    int timeout = 500; // msecs
    bool sig_detect = false;
    bool serdes_rdy = false;
    bool mac_faults = true;
    bool mac_sync   = true;

    switch (this->link_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DISABLED:

            // reset timers

            if (this->link_bring_up_timer_ != NULL) {
                linkmgr_timer_delete(this->link_bring_up_timer_);
                this->link_bring_up_timer_ = NULL;
            }

            if (this->link_debounce_timer_ != NULL) {
                linkmgr_timer_delete(this->link_debounce_timer_);
                this->link_debounce_timer_ = NULL;
            }

            this->bringup_timer_val_ = 0;

            // set operational status as down
            this->set_oper_status(port_oper_status_t::PORT_OPER_STATUS_DOWN);

            // disable and clear mac interrupts
            port_mac_intr_en(false);
            port_mac_intr_clr();

            // disable serdes
            port_serdes_output_enable(false);

            // disable and put mac in reset
            port_mac_soft_reset(true);
            port_mac_enable(false);
            port_mac_stats_reset(true);

            SDK_PORT_TRACE(this, "Disabled");

            break;

        case port_link_sm_t::PORT_LINK_SM_ENABLED:

            SDK_PORT_TRACE(this, "Enabled");

            // transition to serdes cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SERDES_CFG);

        case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:

            SDK_PORT_TRACE(this, "SerDes CFG");

            // configure the serdes
            port_serdes_cfg();

            // enable serdes tx and rx
            port_serdes_tx_rx_enable(true);

            // transition to wait for serdes rdy state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY);

        case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:

            SDK_PORT_TRACE(this, "Wait SerDes RDY");

            serdes_rdy = port_serdes_rdy();

            if(serdes_rdy == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to mac cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_CFG);

        case port_link_sm_t::PORT_LINK_SM_MAC_CFG:

            SDK_PORT_TRACE(this, "MAC CFG");

            // configure the mac
            port_mac_cfg();

            // bring MAC out of reset and enable
            port_mac_stats_reset(false);
            port_mac_enable(true);
            port_mac_soft_reset(false);

            // TODO wait for mac to initialize?

            // enable serdes
            port_serdes_output_enable(true);

            // transition to serdes signal detect state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT);

        case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:

            SDK_PORT_TRACE(this, "Wait for signal detect");

            sig_detect = port_serdes_signal_detect();

            if(sig_detect == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                            0, timeout, this,
                            (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                            false);
                break;
            }

            // transition to DFE tuning stage
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DFE_TUNING);

        case port_link_sm_t::PORT_LINK_SM_DFE_TUNING:
            if (port_dfe_tuning_enabled()) {
                if (port_link_sm_dfe_process() == false) {
                    // DFE tuning is pending
                    // Timer would have been already started. So just break.
                    break;
                }
            }

            // transition to wait for mac sync
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC:

            SDK_PORT_TRACE(this, "Wait MAC SYNC");

            mac_sync = port_mac_sync_get();

            if(mac_sync == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);

                break;
            }

            // transition to mac faults to be cleared
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:

            SDK_PORT_TRACE(this, "Wait MAC faults clear");

            mac_faults = port_mac_faults_get();

            if(mac_faults == true) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to link up state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_UP);

        case port_link_sm_t::PORT_LINK_SM_UP:

            SDK_PORT_TRACE(this, "Link UP");

            // enable mac interrupts
            port_mac_intr_en(true);

            // set operational status as up
            this->set_oper_status(port_oper_status_t::PORT_OPER_STATUS_UP);

            // enable pCal

            // add to link poll timer
            port_link_poll_timer_add(this);

            // notify others that link is up
            port_event_notify(port_event_t::PORT_EVENT_LINK_UP);
            break;

        default:
            break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_enable(void)
{
    // check if already enabled
    if (this->admin_state_ == port_admin_state_t::PORT_ADMIN_STATE_UP) {
        return SDK_RET_OK;
    }

    // enable the port
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);
    set_port_link_dfe_sm(port_link_sm_t::PORT_LINK_SM_DFE_DISABLED);

    port_link_sm_process();

    this->admin_state_ = port_admin_state_t::PORT_ADMIN_STATE_UP;

    return SDK_RET_OK;
}

sdk_ret_t
port::port_disable(void)
{
    // check if already disabled
    if (this->admin_state_ == port_admin_state_t::PORT_ADMIN_STATE_DOWN) {
        return SDK_RET_OK;
    }

    // disable the port
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DISABLED);

    port_link_sm_process();

    this->admin_state_ = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

    return SDK_RET_OK;;
}

sdk_ret_t
port::port_event_notify(port_event_t event)
{
    switch(event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        break;

    case port_event_t::PORT_EVENT_LINK_DOWN:
        port_link_poll_timer_delete(this);
        break;

    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
port::port_link_dn_handler(void)
{
    // start the debounce timer
    if (this->debounce_time_ != 0) {
        this->link_debounce_timer_ =
            linkmgr_timer_schedule(
                0, this->debounce_time_, this,
                (sdk::lib::twheel_cb_t)link_debounce_timer_cb,
                false);
        return SDK_RET_OK;
    }

    port_debounce_timer();

    return SDK_RET_OK;
}

bool
port::bringup_timer_expired(void)
{
    return (this->bringup_timer_val_ > MAX_LINK_BRINGUP_TIMEOUT);
}

// ----------------------------------------------------
// static methods
// ----------------------------------------------------

sdk_ret_t
port::port_init(linkmgr_cfg_t *cfg)
{
    sdk_ret_t rc = SDK_RET_OK;

    linkmgr_csr_init();

    rc = port_mac_fn_init(cfg);
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("port mac init failed");
    }

    rc = port_serdes_fn_init(cfg);
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("port mac init failed");
    }

    return rc;
}

sdk_ret_t
port::port_enable(port *port_p)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (is_linkmgr_ctrl_thread()) {
        ret = port_p->port_enable();
    } else {
        // wake up the linkmgr control thread to process port event

        linkmgr_entry_data_t data;
        data.ctxt  = port_p;
        data.timer = NULL;

        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_ENABLE, &data);

        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error notifying control-thread for port enable");
        }
    }

    return ret;
}

sdk_ret_t
port::port_disable(port *port_p)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (is_linkmgr_ctrl_thread()) {
        ret = port_p->port_disable();
    } else {
        // wake up the linkmgr control thread to process port event

        linkmgr_entry_data_t data;
        data.ctxt  = port_p;
        data.timer = NULL;

        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_DISABLE, &data);

        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error notifying control-thread for port disable");
        }
    }

    return ret;
}

}    // namespace linkmgr
}    // namespace sdk
