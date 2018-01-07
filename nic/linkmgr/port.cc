// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "sdk/twheel.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/include/periodic.hpp"
#include "port_mac.hpp"
#include "linkmgr.hpp"
#include "port.hpp"

namespace linkmgr {
namespace pd {

mac_fn_t port::mac_fn;
serdes_fn_t port::serdes_fn;

// Invoked by the periodic thread when timer expires
hal_ret_t
port::link_bring_up_timer_cb(uint32_t timer_id, void *ctxt)
{
    hal_ret_t ret = HAL_RET_OK;

    // wake up the hal control thread to process port event
    ret = linkmgr_notify (LINKMGR_OPERATION_PORT_TIMER, ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error notifying control-thread for port timer");
    }

    return ret;
}

hal_ret_t
port::port_mac_init()
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_fifo_ctrl()
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_global_mode_cfg()
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_ch_enable(bool enable)
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_generic_cfg()
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_rx_tx_enable(bool rx_enable, bool tx_enable)
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_ch_mode_cfg()
{
    return HAL_RET_OK;
}

uint32_t
port::port_mac_port_num_calc()
{
    return (this->mac_id_ * PORT_LANES_MAX) + this->mac_ch_;
}

hal_ret_t
port::port_mac_cfg()
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_cfg(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_enable(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_enable(
               mac_port_num,
               static_cast<uint32_t>(this->port_speed_),
               this->num_lanes_,
               enable);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_soft_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_soft_reset(
                   mac_port_num,
                   static_cast<uint32_t>(this->port_speed_),
                   this->num_lanes_,
                   reset);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_stats_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_stats_reset(
                    mac_port_num,
                    static_cast<uint32_t>(this->port_speed_),
                    this->num_lanes_,
                    reset);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_intr_en(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_intr_enable(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_,
            enable);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_intr_clr()
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    port::mac_fn.mac_intr_clear(
            mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes_);

    return HAL_RET_OK;
}

bool
port::port_mac_faults_get()
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    return port::mac_fn.mac_faults_get(mac_port_num);
}

uint32_t
port::port_sbus_addr(uint32_t lane)
{
    return sbus_addr(mac_id_, mac_ch_, lane);
}

hal_ret_t
port::port_serdes_cfg()
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_cfg(port_sbus_addr(lane));
    }

    return HAL_RET_OK;
}

hal_ret_t
port::port_serdes_tx_rx_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_tx_rx_enable(port_sbus_addr(lane),
                                            enable);
    }

    return HAL_RET_OK;
}

hal_ret_t
port::port_serdes_output_enable(bool enable)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_output_enable(port_sbus_addr(lane),
                                             enable);
    }

    return HAL_RET_OK;
}

hal_ret_t
port::port_serdes_reset(bool reset)
{
    uint32_t lane = 0;
    for (lane = 0; lane < num_lanes_; ++lane) {
        port::serdes_fn.serdes_reset(port_sbus_addr(lane),
                                     reset);
    }

    return HAL_RET_OK;
}

bool
port::port_serdes_signal_detect()
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
port::port_serdes_rdy()
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

hal_ret_t
port::port_link_sm_process()
{
    int timeout = 0;
    bool sig_detect = false;
    bool serdes_rdy = false;
    bool mac_faults = true;

    switch (this->link_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DISABLED:
            // stop link bring up timer
            hal::periodic::periodic_timer_delete (this->link_bring_up_timer_);
            this->link_bring_up_timer_ = NULL;  // sanity

            // set operational status as down
            this->set_oper_status(::port::PORT_OPER_STATUS_DOWN);

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
                    hal::periodic::periodic_timer_schedule(
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
                    hal::periodic::periodic_timer_schedule(
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
                    hal::periodic::periodic_timer_schedule(
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
            this->set_oper_status(::port::PORT_OPER_STATUS_UP);

            // enable pCal
            // notify others that link is up
            break;

        default:
            break;
    }

    return HAL_RET_OK;
}

hal_ret_t
port::port_enable()
{
    /* check if already enabled */
    if (this->admin_state_ == ::port::PORT_ADMIN_STATE_UP) {
        return HAL_RET_OK;
    }

    /* enable the port */
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);

    port_link_sm_process();

    this->admin_state_ = ::port::PORT_ADMIN_STATE_UP;

    return HAL_RET_OK;
}

hal_ret_t
port::port_disable()
{
    /* check if already disabled */
    if (this->admin_state_ == ::port::PORT_ADMIN_STATE_DOWN) {
        return HAL_RET_OK;
    }

    /* disable the port */
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DISABLED);

    port_link_sm_process();

    this->admin_state_ = ::port::PORT_ADMIN_STATE_DOWN;

    return HAL_RET_OK;;
}

hal_ret_t
port::port_init()
{
    hal_ret_t rc = HAL_RET_OK;

    rc = port::port_mac_fn_init();
    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("port mac init failed");
    }

    rc = port::port_serdes_fn_init();
    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("port mac init failed");
    }

    return rc;
}

hal_ret_t
port::port_enable(port *pd_p)
{
    hal_ret_t ret = HAL_RET_OK;

    if (1) {
        ret = pd_p->port_enable();
    } else {
        // wake up the hal control thread to process port event
        ret = linkmgr_notify (LINKMGR_OPERATION_PORT_ENABLE, pd_p);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}: Error notifying control-thread for port enable",
                          __FUNCTION__);
        }
    }

    return ret;
}

hal_ret_t
port::port_disable(port *pd_p)
{
    hal_ret_t ret = HAL_RET_OK;

    if (1) {
        ret = pd_p->port_disable();
    } else {
        // wake up the hal control thread to process port event
        ret = linkmgr_notify (LINKMGR_OPERATION_PORT_DISABLE, pd_p);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{}: Error notifying control-thread for port disable",
                          __FUNCTION__);
        }
    }

    return ret;
}

}    // namespace pd
}    // namespace linkmgr
