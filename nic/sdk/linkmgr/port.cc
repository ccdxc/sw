// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port.hpp"
#include "port_mac.hpp"
#include "port_serdes.hpp"
#include "linkmgr_internal.hpp"
#include "linkmgr_types.hpp"
#include "timer_cb.hpp"
#include "include/sdk/periodic.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"
#include "spidev.h"

namespace sdk {
namespace linkmgr {

mac_fn_t    mac_fns;
mac_fn_t    mac_mgmt_fns;
serdes_fn_t serdes_fns;

// Debounce timer expiration handler
sdk_ret_t
port::port_debounce_timer(void)
{
    // Notify if link is still down
    if (port_link_status() == false) {
        // Disable notifies link down if link was UP before
        port_disable();

        // Enable the port
        port_enable();
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
    mac_info_t mac_info;
    memset(&mac_info, 0, sizeof(mac_info_t));

    mac_info.mac_id    = this->mac_id_;
    mac_info.mac_ch    = this->mac_ch_;
    mac_info.speed     = static_cast<uint32_t>(this->port_speed_);

    mac_info.mtu       = this->mtu_;
    mac_info.num_lanes = this->num_lanes_;

    // 0: disable fec, 1: enable fec
    // TODO current FEC type is determined by global mode in mx api
    mac_info.fec       = static_cast<uint32_t>(this->fec_type_);

    // Enable Tx padding. Disable Rx padding
    mac_info.tx_pad_enable = 1;
    mac_info.rx_pad_enable = 0;

    mac_info.force_global_init = false;

    mac_fns()->mac_cfg(&mac_info);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_enable(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns()->mac_enable(
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

    mac_fns()->mac_soft_reset(
                   mac_port_num,
                   static_cast<uint32_t>(this->port_speed_),
                   this->num_lanes_,
                   reset);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_reset(bool reset)
{
    mac_fns()->mac_stats_reset(this->mac_id_, this->mac_ch_, reset);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_intr_en(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns()->mac_intr_enable(
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

    mac_fns()->mac_intr_clear(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return SDK_RET_OK;
}

bool
port::port_mac_faults_get(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return mac_fns()->mac_faults_get(mac_port_num);
}

bool
port::port_mac_sync_get(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return mac_fns()->mac_sync_get(mac_port_num);
}

sdk_ret_t
port::port_flush_set (bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_fns()->mac_flush_set(mac_port_num, enable);

    return SDK_RET_OK;
}

uint32_t
port::port_sbus_addr(uint32_t lane)
{
    return this->sbus_addr_[lane];
}

port_speed_t
port::port_speed_to_serdes_speed(port_speed_t port_speed)
{
    switch(port_speed) {
    case port_speed_t::PORT_SPEED_100G:
    case port_speed_t::PORT_SPEED_50G:
    case port_speed_t::PORT_SPEED_25G:
        return port_speed_t::PORT_SPEED_25G;

    case port_speed_t::PORT_SPEED_40G:
    case port_speed_t::PORT_SPEED_10G:
        return port_speed_t::PORT_SPEED_10G;

    default:
        break;
    }

    return port_speed;
}

sdk_ret_t
port::port_serdes_cfg(void)
{
    uint32_t      lane         = 0;
    uint32_t      sbus_addr    = 0;
    serdes_info_t *serdes_info = NULL;

    port_speed_t serdes_speed =
                    port_speed_to_serdes_speed(this->port_speed_);

    for (lane = 0; lane < num_lanes_; ++lane) {
        sbus_addr = port_sbus_addr(lane);

        serdes_info = serdes_info_get(
                                    sbus_addr,
                                    static_cast<uint32_t>(serdes_speed),
                                    this->cable_type_);

        serdes_fns()->serdes_cfg(sbus_addr, serdes_info);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_tx_rx_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_tx_rx_enable(port_sbus_addr(lane),
                                            enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_output_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_output_enable(port_sbus_addr(lane),
                                             enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_reset(bool reset)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_reset(port_sbus_addr(lane),
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
        signal_detect = serdes_fns()->serdes_signal_detect(
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
        serdes_rdy =  serdes_fns()->serdes_rdy(port_sbus_addr(lane));
        if (serdes_rdy == false) {
            break;
        }
    }

    return serdes_rdy;
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
    switch (this->port_speed_) {
        case port_speed_t::PORT_SPEED_100G:
        case port_speed_t::PORT_SPEED_50G:
        case port_speed_t::PORT_SPEED_40G:
        case port_speed_t::PORT_SPEED_25G:
        case port_speed_t::PORT_SPEED_10G:
            return true;

        default:
            break;
    }

    return false;
}

int
port::port_serdes_ical_start(void)
{
    uint32_t lane = 0;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_ical_start(port_sbus_addr(lane));
    }

    return 0;
}

int
port::port_serdes_pcal_start(void)
{
    uint32_t lane = 0;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_pcal_start(port_sbus_addr(lane));
    }

    return 0;
}

int
port::port_serdes_pcal_continuous_start(void)
{
    uint32_t lane = 0;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_pcal_continuous_start(port_sbus_addr(lane));
    }

    return 0;
}

bool
port::port_serdes_dfe_complete(void)
{
    uint32_t lane = 0;

    for (lane = 0; lane < num_lanes_; ++lane) {
        if (serdes_fns()->serdes_dfe_status(port_sbus_addr(lane)) != 1) {
            return false;
        }
    }

    return true;
}

bool
port::port_link_sm_dfe_process(void)
{
    bool dfe_complete = false;
    bool ret          = true;
    int  timeout      = 40; //msecs

    switch(this->link_dfe_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DFE_DISABLED:

            // transition to start ical
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL:

            SDK_PORT_SM_DEBUG(this, "start ICAL");

            port_serdes_ical_start();

            // transition to wait for ical complete
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_ICAL:

            SDK_PORT_SM_DEBUG(this, "wait ICAL");

            dfe_complete = port_serdes_dfe_complete();

            if(dfe_complete == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
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

            SDK_PORT_SM_DEBUG(this, "start PCAL");

            port_serdes_pcal_start();

            // transition to wait for pcal complete
            set_port_link_dfe_sm(
                    port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL);

        case port_link_sm_t::PORT_LINK_SM_DFE_WAIT_PCAL:

            SDK_PORT_SM_DEBUG(this, "wait PCAL");

            dfe_complete = port_serdes_dfe_complete();

            if(dfe_complete == false) {
                timeout = 100; // 100 msec for pCal

                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);

                ret = false;
                break;
            }

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
                sdk::lib::timer_delete(this->link_bring_up_timer_);
                this->link_bring_up_timer_ = NULL;
            }

            if (this->link_debounce_timer_ != NULL) {
                sdk::lib::timer_delete(this->link_debounce_timer_);
                this->link_debounce_timer_ = NULL;
            }

            // remove from link poll timer if present
            port_link_poll_timer_delete(this);

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

            // reset number of link bringup retries
            set_num_retries(0);

            SDK_PORT_SM_DEBUG(this, "Disabled");

            break;

        case port_link_sm_t::PORT_LINK_SM_ENABLED:

            SDK_PORT_SM_DEBUG(this, "Enabled");

            // transition to serdes cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SERDES_CFG);

        case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:

            SDK_PORT_SM_DEBUG(this, "SerDes CFG");

            // configure the serdes
            port_serdes_cfg();

            // enable serdes tx and rx
            port_serdes_tx_rx_enable(true);

            // transition to wait for serdes rdy state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY);

        case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:

            SDK_PORT_SM_DEBUG(this, "Wait SerDes RDY");

            serdes_rdy = port_serdes_rdy();

            if(serdes_rdy == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to mac cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_CFG);

        case port_link_sm_t::PORT_LINK_SM_MAC_CFG:

            SDK_PORT_SM_DEBUG(this, "MAC CFG");

            // configure the mac
            port_mac_cfg();

            // put port in flush
            port_flush_set(true);

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

            SDK_PORT_SM_DEBUG(this, "Wait for signal detect");

            sig_detect = port_serdes_signal_detect();

            if(sig_detect == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
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

            SDK_PORT_SM_DEBUG(this, "Wait MAC SYNC");

            mac_sync = port_mac_sync_get();

            if(mac_sync == false) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);

                break;
            }

            // transition to mac faults to be cleared
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:

            SDK_PORT_SM_DEBUG(this, "Wait MAC faults clear");

            mac_faults = port_mac_faults_get();

            if(mac_faults == true) {
                this->bringup_timer_val_ += timeout;

                this->link_bring_up_timer_ =
                    sdk::lib::timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to link up state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_UP);

        case port_link_sm_t::PORT_LINK_SM_UP:

            SDK_PORT_SM_DEBUG(this, "PCAL continuous");

            port_serdes_pcal_continuous_start();

            SDK_PORT_SM_TRACE(this, "Link UP");

            // enable mac interrupts
            port_mac_intr_en(true);

            // bring port out of flush
            port_flush_set(false);

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

    // store the current link status
    port_oper_status_t prev_oper_status = oper_status();

    // disable the port
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DISABLED);

    port_link_sm_process();

    this->admin_state_ = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

    // Notfiy the link down if link was up before disable
    if (prev_oper_status == port_oper_status_t::PORT_OPER_STATUS_UP) {
        port_event_notify(port_event_t::PORT_EVENT_LINK_DOWN);
    }

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

    if (g_linkmgr_cfg.port_event_cb) {
        g_linkmgr_cfg.port_event_cb(port_num(), event);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_link_dn_handler(void)
{
    // start the debounce timer
    if (this->debounce_time_ != 0) {
        this->link_debounce_timer_ =
            sdk::lib::timer_schedule(
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

sdk_ret_t
port::port_mac_stats_get (uint64_t *stats_data)
{
    mac_fns()->mac_stats_get(this->mac_id_, this->mac_ch_, stats_data);
    return SDK_RET_OK;
}

sdk_ret_t
port::port_deinit (void)
{
    mac_fns()->mac_deinit (this->mac_id_, this->mac_ch_);
    return SDK_RET_OK;
}

// ----------------------------------------------------
// static methods
// ----------------------------------------------------

sdk_ret_t
port::port_init(linkmgr_cfg_t *cfg)
{
    sdk_ret_t   rc           = SDK_RET_OK;
    int         exp_build_id = serdes_build_id();
    int         exp_rev_id   = serdes_rev_id();
    char        *cfg_path    = std::getenv("HAL_CONFIG_PATH");
    std::string cfg_file     = "fw/" + serdes_fw_file();

    rc = port_mac_fn_init(cfg);
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("port mac init failed");
    }

    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    }

    rc = port_serdes_fn_init(cfg->platform_type,
                             jtag_id(),
                             num_sbus_rings(),
                             aacs_server_en(),
                             aacs_connect(),
                             aacs_server_port(),
                             aacs_server_ip());
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("port mac init failed");
    }

    // TODO move back to serdes_fn_init
    serdes_get_ip_info(1);

    serdes_sbm_set_sbus_clock_divider(sbm_clk_div());

    for (uint32_t asic_port = 0; asic_port < num_asic_ports(0); ++asic_port) {
        uint32_t sbus_addr = sbus_addr_asic_port(0, asic_port);

        if (sbus_addr == 0) {
            continue;
        }

        sdk::linkmgr::serdes_fns.serdes_spico_upload(sbus_addr, cfg_file.c_str());

        int build_id = sdk::linkmgr::serdes_fns.serdes_get_build_id(sbus_addr);
        int rev_id   = sdk::linkmgr::serdes_fns.serdes_get_rev(sbus_addr);

        if (build_id != exp_build_id || rev_id != exp_rev_id) {
            SDK_TRACE_DEBUG("sbus_addr: 0x%x,"
                            " build_id: 0x%x, exp_build_id: 0x%x,"
                            " rev_id: 0x%x, exp_rev_id: 0x%x",
                            sbus_addr, build_id, exp_build_id,
                            rev_id, exp_rev_id);
            // TODO fail if no match
        }

        sdk::linkmgr::serdes_fns.serdes_spico_status(sbus_addr);

        SDK_TRACE_DEBUG("sbus_addr: 0x%x, spico_crc: %d",
                        sbus_addr,
                        sdk::linkmgr::serdes_fns.serdes_spico_crc(sbus_addr));
    }

    spidev_init();

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
