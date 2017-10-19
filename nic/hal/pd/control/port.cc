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

namespace hal {

namespace pd {

// Invoked in control thread context
hal_ret_t
port::port_event_notify(void *ctxt)
{
    port *port_p = (port*)ctxt;

    port_p->port_link_sm_process();

    return HAL_RET_OK;
}

// Invoked by the periodic thread when timer expires
hal_ret_t
port::link_bring_up_timer_cb(uint32_t timer_id, void *ctxt)
{
    // wake up the hal control thread to process port event
    uint16_t           pindx;
    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();
    uint32_t           curr_tid = curr_thread->thread_id();
    hal_ctrl_entry_t   *rw_entry;

    if (g_hal_ctrl_workq[curr_tid].nentries >= HAL_CONTROL_Q_SIZE) {
        HAL_TRACE_ERR("port control operation for thread {}, tid {} full",
                curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }
    pindx = g_hal_ctrl_workq[curr_tid].pindx;

    rw_entry = &g_hal_ctrl_workq[curr_tid].entries[pindx];
    rw_entry->opn = HAL_CONTROL_OPERATION_PORT;
    rw_entry->status = HAL_RET_ERR;
    rw_entry->data = ctxt;
    rw_entry->done.store(false);

    g_hal_ctrl_workq[curr_tid].nentries++;

    while (rw_entry->done.load() == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    g_hal_ctrl_workq[curr_tid].pindx++;
    if (g_hal_ctrl_workq[curr_tid].pindx >= HAL_CONTROL_Q_SIZE) {
        g_hal_ctrl_workq[curr_tid].pindx = 0;
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
    return (this->mac_id * PORT_LANES_MAX) + this->mac_ch;
}

hal_ret_t
port::port_mac_cfg()
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_cfg(mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_enable(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_enable(mac_port_num,
               static_cast<uint32_t>(this->port_speed_),
               this->num_lanes,
               enable);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_soft_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_soft_reset(mac_port_num,
                   static_cast<uint32_t>(this->port_speed_),
                   this->num_lanes,
                   reset);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_stats_reset(bool reset)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_stats_reset(mac_port_num,
                    static_cast<uint32_t>(this->port_speed_),
                    this->num_lanes,
                    reset);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_intr_en(bool enable)
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_intr_enable(mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes,
            enable);

    return HAL_RET_OK;
}

hal_ret_t
port::port_mac_intr_clr()
{
    uint32_t mac_port_num = port_mac_port_num_calc();

    mac_intr_clear(mac_port_num,
            static_cast<uint32_t>(this->port_speed_),
            this->num_lanes);

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
    if (this->admin_state_ == true) {
        return HAL_RET_OK;
    }

    /* enable the port */
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);

    port_link_sm_process();

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
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DISABLED);

    port_link_sm_process();

    this->admin_state_ = false;

    return HAL_RET_OK;;
}

}    // namespace pd
}    // namespace hal
