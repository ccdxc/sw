#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "nic/include/port.hpp"
#include "nic/utils/twheel/twheel.hpp"
#include "nic/utils/thread/thread.hpp"
#include "hal_control.hpp"
#include "nic/include/periodic.hpp"
#include "port_mac.hpp"
#include "hal_control.hpp"

namespace hal {

namespace pd {

mac_fn_t port::mac_fn;

// Invoked in control thread context
hal_ret_t
port::port_event_notify(uint8_t opn, void *ctxt)
{
    port *pd_p = (port*)ctxt;

    switch (opn) {
        case HAL_CONTROL_OPERATION_PORT_TIMER:
            return pd_p->port_link_sm_process();

        case HAL_CONTROL_OPERATION_PORT_ENABLE:
            return pd_p->port_enable();

        case HAL_CONTROL_OPERATION_PORT_DISABLE:
            return pd_p->port_disable();

        default:
            break;
    }

    return HAL_RET_OK;
}

// Invoked by the periodic thread when timer expires
hal_ret_t
port::link_bring_up_timer_cb(uint32_t timer_id, void *ctxt)
{
    hal_ret_t ret = HAL_RET_OK;

    // wake up the hal control thread to process port event
    ret = hal_control_notify (HAL_CONTROL_OPERATION_PORT_TIMER, ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error notifying control-thread for port timer");
    }

    return ret;
}

hal_ret_t
port::port_serdes_cfg()
{
    return HAL_RET_OK;
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

            // disable and claer mac interrupts
            port_mac_intr_en(false);
            port_mac_intr_clr();

            // disable serdes

            // mac reset
            port_mac_soft_reset(true);

            // mac disable
            port_mac_enable(false);

            // mac stats reset
            port_mac_stats_reset(true);

            break;

        case port_link_sm_t::PORT_LINK_SM_ENABLED:

            // transition to serdes cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SERDES_CFG);

        case port_link_sm_t::PORT_LINK_SM_SERDES_CFG:

            // transition to mac cfg state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_CFG);

        case port_link_sm_t::PORT_LINK_SM_MAC_CFG:

            // configure the mac
            port_mac_cfg();

            // transition to serdes signal detect state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT);

        case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:

            sig_detect = true;

            if(sig_detect == false) {
                this->link_bring_up_timer_ =
                    hal::periodic::periodic_timer_schedule(
                            0, timeout, this,
                            (hal::utils::twheel_cb_t)port::link_bring_up_timer_cb,
                            false);
                break;
            }

            // transition to wait for serdes rdy state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY);

        case port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY:

            serdes_rdy = true;

            if(serdes_rdy == false) {
                this->link_bring_up_timer_ =
                    hal::periodic::periodic_timer_schedule(
                        0, timeout, this,
                        (hal::utils::twheel_cb_t)port::link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to mac enable state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_ENABLE);

        case port_link_sm_t::PORT_LINK_SM_MAC_ENABLE:

            port_mac_stats_reset(false);
            port_mac_enable(true);
            port_mac_soft_reset(false);

            // transition to wait for clear mac faults
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_CLEAR_FAULTS);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_CLEAR_FAULTS:

            mac_faults = false;

            if(mac_faults == true) {
                this->link_bring_up_timer_ =
                    hal::periodic::periodic_timer_schedule(
                        0, timeout, this,
                        (hal::utils::twheel_cb_t)port::link_bring_up_timer_cb,
                        false);
                break;
            }

            // transition to link up state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_UP);

        case port_link_sm_t::PORT_LINK_SM_UP:
            // enable mac interrupts
            port_mac_intr_en(true);

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
port::port_init(bool is_sim)
{
    hal_ret_t rc = HAL_RET_OK;

    rc = port::port_mac_init(is_sim);
    if (rc != HAL_RET_OK) {
        HAL_TRACE_ERR("port mac init failed");
    }

    return rc;
}

}    // namespace pd
}    // namespace hal
