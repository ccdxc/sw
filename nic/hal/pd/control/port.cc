#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "nic/include/port.hpp"
#include "nic/utils/twheel/twheel.hpp"
#include "nic/utils/thread/thread.hpp"
#include "hal_control.hpp"

namespace hal {

extern thread_local hal::utils::thread *t_curr_thread;

namespace periodic {
void* periodic_timer_schedule (uint32_t timer_id,
                               uint64_t timeout,
                               void *ctxt,
                               hal::utils::twheel_cb_t cb,
                               bool periodic);
}    // namespace periodic

namespace pd {

// Invoked in control thread context
hal_ret_t
port::port_event_notify(void *ctxt)
{
    port *port_p = (port*)ctxt;

    port_p->port_link_sm();

    return HAL_RET_OK;
}

// Invoked by the periodic thread when timer expires
hal_ret_t
port::link_bring_up_timer_cb(uint32_t timer_id, void *ctxt)
{
    // wake up the hal control thread to process port event
    uint16_t           pindx;
    uint32_t           curr_tid = t_curr_thread->thread_id();
    hal_ctrl_entry_t   *rw_entry;

    if (g_hal_ctrl_workq[curr_tid].nentries >= HAL_CONTROL_Q_SIZE) {
        HAL_TRACE_ERR("port control operation for thread {}, tid {} full",
                t_curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }
    pindx = g_hal_ctrl_workq[curr_tid].pindx;
    rw_entry = &g_hal_ctrl_workq[curr_tid].entries[pindx];
    rw_entry->opn = HAL_CONTROL_OPERATION_PORT;
    rw_entry->done = FALSE;
    rw_entry->status = HAL_RET_ERR;
    rw_entry->data = ctxt;
    g_hal_ctrl_workq[curr_tid].nentries++;
    while (!rw_entry->done) {
        if (t_curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    return rw_entry->status;
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
port::port_mac_soft_reset(bool reset)
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

hal_ret_t
port::port_mac_stats_reset(bool reset)
{
    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_cfg()
{
    port_mac_init();

    port_mac_soft_reset(true);

    port_mac_fifo_ctrl();

    port_mac_global_mode_cfg();

    port_mac_ch_enable(false);

    port_mac_generic_cfg();

    port_mac_rx_tx_enable(false, false);

    port_mac_ch_mode_cfg();

    port_mac_stats_reset(true);
    port_mac_stats_reset(false);

    return HAL_RET_OK;
}

hal_ret_t
port::port_link_sm()
{
    int timeout = 0;
    bool sig_detect = false;
    bool serdes_rdy = false;
    bool mac_faults = true;

    switch (this->link_sm_) {
        case port_link_sm_t::PORT_LINK_SM_DISABLED:
            // stop timers
            // disable serdes
            // disable mac interrupts
            // mac reset
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
                hal::periodic::periodic_timer_schedule(
                    0, timeout, this,
                    (hal::utils::twheel_cb_t)port::link_bring_up_timer_cb,
                    false);
                break;
            }

            // transition to mac enable state
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_MAC_ENABLE);

        case port_link_sm_t::PORT_LINK_SM_MAC_ENABLE:

            port_mac_soft_reset(false);
            port_mac_ch_enable(true);

            // transition to wait for clear mac faults
            this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_CLEAR_FAULTS);

        case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_CLEAR_FAULTS:

            mac_faults = false;

            if(mac_faults == true) {
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
            // enable pCal
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
    if (this->admin_state_ == true) {
        return HAL_RET_OK;
    }

    /* enable the port */
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);

    port_link_sm();

    this->admin_state_ = true;

    return HAL_RET_OK;
}

hal_ret_t
port::port_disable()
{
    /* check if already disabled */
    if (this->admin_state_ == false) {
        return HAL_RET_OK;
    }

    /* disable the port */

    this->admin_state_ = false;

    return HAL_RET_OK;;
}

}    // namespace pd
}    // namespace hal
