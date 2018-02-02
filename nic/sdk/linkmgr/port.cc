// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "sdk/twheel.hpp"
#include "sdk/thread.hpp"
#include "sdk/catalog.hpp"
#include "linkmgr.hpp"
#include "port_mac.hpp"
#include "port.hpp"
#include "linkmgr_periodic.hpp"
#include "linkmgr_internal.hpp"

namespace sdk {
namespace linkmgr {

mac_fn_t port::mac_fn;
serdes_fn_t port::serdes_fn;

// invoked by the periodic thread when timer expires
sdk_ret_t
port::link_bring_up_timer_cb(void *timer, uint32_t timer_id, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify(LINKMGR_OPERATION_PORT_TIMER, ctxt);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error notifying control-thread for port timer");
    }

    return ret;
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
    return (this->mac_id_ * PORT_LANES_MAX) + this->mac_ch_;
}

sdk_ret_t
port::port_mac_cfg(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_cfg(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_enable(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_enable(
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

    port::mac_fn.mac_soft_reset(
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

    port::mac_fn.mac_stats_reset(
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

    port::mac_fn.mac_intr_enable(
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

    port::mac_fn.mac_intr_clear(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return SDK_RET_OK;
}

bool
port::port_mac_faults_get(void)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return port::mac_fn.mac_faults_get(mac_port_num);
}

uint32_t
port::port_sbus_addr(uint32_t lane)
{
    return this->sbus_addr_[lane];
}

sdk_ret_t
port::port_serdes_cfg(void)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_cfg(port_sbus_addr(lane));
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_tx_rx_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_tx_rx_enable(port_sbus_addr(lane),
                                            enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_output_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_output_enable(port_sbus_addr(lane),
                                             enable);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_reset(bool reset)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_reset(port_sbus_addr(lane),
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
        signal_detect = port::serdes_fn.serdes_signal_detect(
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
        serdes_rdy =  port::serdes_fn.serdes_rdy(port_sbus_addr(lane));
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

sdk_ret_t
port::port_link_sm_process(void)
{
    int timeout = 0;
    bool sig_detect = false;
    bool serdes_rdy = false;
    bool mac_faults = true;

    switch (this->link_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DISABLED:
            // stop link bring up timer
            linkmgr_timer_delete(this->link_bring_up_timer_);
            this->link_bring_up_timer_ = NULL;  // sanity

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

            break;

        case port_link_sm_t::PORT_LINK_SM_ENABLED:

            // transition to serdes cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SERDES_CFG);

        case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:

            // configure the serdes
            port_serdes_cfg();

            // enable serdes tx and rx
            port_serdes_tx_rx_enable(true);

            // transition to wait for serdes rdy state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY);

        case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:

            serdes_rdy = port_serdes_rdy();

            if(serdes_rdy == false) {
                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)port::link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to mac cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_CFG);

        case port_link_sm_t::PORT_LINK_SM_MAC_CFG:

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

            sig_detect = port_serdes_signal_detect();

            if(sig_detect == false) {
                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                            0, timeout, this,
                            (sdk::lib::twheel_cb_t)port::link_bring_up_timer_cb,
                            false);
                break;
            }

            // transition to wait for serdes rdy state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:

            mac_faults = port_mac_faults_get();

            if(mac_faults == true) {
                this->link_bring_up_timer_ =
                    linkmgr_timer_schedule(
                        0, timeout, this,
                        (sdk::lib::twheel_cb_t)port::link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to link up state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_UP);

        case port_link_sm_t::PORT_LINK_SM_UP:
            // enable mac interrupts
            port_mac_intr_en(true);

            // set operational status as up
            this->set_oper_status(port_oper_status_t::PORT_OPER_STATUS_UP);

            // enable pCal
            // notify others that link is up
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
port::port_init(linkmgr_cfg_t *cfg)
{
    sdk_ret_t rc = SDK_RET_OK;

    rc = port::port_mac_fn_init(cfg);
    if (rc != SDK_RET_OK) {
        SDK_TRACE_ERR("port mac init failed");
    }

    rc = port::port_serdes_fn_init(cfg);
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
        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_ENABLE, port_p);

        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("%s: Error notifying control-thread for port enable",
                          __FUNCTION__);
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
        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_DISABLE, port_p);

        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("%s: Error notifying control-thread for port disable",
                          __FUNCTION__);
        }
    }

    return ret;
}

}    // namespace linkmgr
}    // namespace sdk
