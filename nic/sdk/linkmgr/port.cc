// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "port.hpp"
#include "port_mac.hpp"
#include "port_serdes.hpp"
#include "linkmgr_internal.hpp"
#include "linkmgr_types.hpp"
#include "timer_cb.hpp"
#include "lib/periodic/periodic.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"
#include "platform/drivers/xcvr.hpp"
#include "platform/fru/fru.hpp"
#include "lib/pal/pal.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "linkmgr_periodic.hpp"
#include "asic/rw/asicrw.hpp"

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
    // check for MAC sync and MAC faults
    if (port_mac_sync_get() == false) {
        return false;
    }
    if (port_mac_faults_get() == true) {
        SDK_LINKMGR_TRACE_DEBUG("port %u, MAC %u, MAC faults detected",
                                port_num(), mac_id());
        return false;
    }
    return true;
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
    mac_info.pause     = this->pause_;
    mac_info.tx_pause_enable = this->tx_pause_enable_;
    mac_info.rx_pause_enable = this->rx_pause_enable_;
    mac_info.num_lanes = this->num_lanes_;

    mac_info.fec       = static_cast<uint32_t>(this->fec_type_);
    mac_info.loopback  =
        loopback_mode() == port_loopback_mode_t::PORT_LOOPBACK_MODE_MAC? 1 : 0;

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
    port_mac_stats_persist_clear(reset);

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

sdk_ret_t
port::port_mac_faults_clear(void)
{
    mac_fns()->mac_faults_clear(this->mac_id(), this->mac_ch());
    return SDK_RET_OK;
}

bool
port::port_mac_faults_get(void)
{
    return mac_fns()->mac_faults_get(this->mac_id(), this->mac_ch());
}

sdk_ret_t
port::port_mac_send_remote_faults(bool send) {
    mac_fns()->mac_send_remote_faults(
                this->mac_id(), this->mac_ch(), send);
    return SDK_RET_OK;
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
                                    cable_type());

        serdes_fns()->serdes_basic_cfg(sbus_addr, serdes_info);
        serdes_fns()->serdes_cfg(sbus_addr, serdes_info);
        serdes_fns()->serdes_rx_lpbk(sbus_addr,
                loopback_mode() == port_loopback_mode_t::PORT_LOOPBACK_MODE_PHY?
                true : false);
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
    uint32_t lane         = 0;
    uint8_t  xcvr_en_mask = 0x0;
    sdk_ret_t sdk_ret = SDK_RET_OK;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_output_enable(port_sbus_addr(lane),
                                           enable);
        xcvr_en_mask |= (1 << lane);
    }

    int phy_port = sdk::lib::catalog::logical_port_to_phy_port(port_num());

    if (phy_port != -1 &&
        cable_type() == sdk::types::cable_type_t::CABLE_TYPE_FIBER) {
        sdk_ret = sdk::platform::xcvr_enable(phy_port-1, enable, xcvr_en_mask);
        if (sdk_ret != SDK_RET_OK) {
            SDK_LINKMGR_TRACE_ERR ("Failed to %s phy_port: %d",
                                   enable == true? "enable" : "disable",
                                   phy_port);
        }
    }

    return sdk_ret;
}

sdk_ret_t
port::port_serdes_reset(bool reset)
{
    uint32_t lane = 0;

    for (lane = 0; lane < num_lanes_; ++lane) {
        serdes_fns()->serdes_spico_reset(port_sbus_addr(lane));
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
        if (auto_neg_enable() == true) {
            serdes_fns()->serdes_an_pcal_start(port_sbus_addr(lane));
        } else {
            serdes_fns()->serdes_pcal_start(port_sbus_addr(lane));
        }
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

int
port::port_serdes_an_start (void)
{
    uint32_t      sbus_addr    = 0;
    serdes_info_t *serdes_info = NULL;
    port_speed_t  serdes_speed = port_speed_t::PORT_SPEED_25G;
    serdes_info_t serdes_info_an;

    memset(&serdes_info_an, 0, sizeof(serdes_info_t));

    // AN on lane 0
    sbus_addr = port_sbus_addr(0);

    serdes_info = serdes_info_get(
                                sbus_addr,
                                static_cast<uint32_t>(serdes_speed),
                                sdk::types::cable_type_t::CABLE_TYPE_CU);

    memcpy (&serdes_info_an, serdes_info, sizeof(serdes_info_t));

    // 1G AN settings
    serdes_info_an.width        = 20;
    serdes_info_an.sbus_divider = 8;

    // Init serdes in 1G
    serdes_fns()->serdes_an_init(port_sbus_addr(0), &serdes_info_an);

    // Configure Tx/Rx slip, Rx termination, Tx EQ for 25G serdes.
    // This is to avoid setting these values if negotiated speed
    // is 25G for each serdes
    // Do not configure lane 0 since AN is run on that lane
    for (uint32_t lane = 1; lane < num_lanes_; ++lane) {
        sbus_addr = port_sbus_addr(lane);

        serdes_info = serdes_info_get(
                                sbus_addr,
                                static_cast<uint32_t>(serdes_speed),
                                sdk::types::cable_type_t::CABLE_TYPE_CU);

        serdes_fns()->serdes_cfg(sbus_addr, serdes_info);
    }

    // start AN
    return serdes_fns()->serdes_an_start(port_sbus_addr(0), &serdes_info_an,
                                         user_cap(), fec_ability(),
                                         fec_request());
}

bool
port::port_serdes_an_wait_hcd (void)
{
    return serdes_fns()->serdes_an_wait_hcd(port_sbus_addr(0));
}

bool
port::port_serdes_an_link_train_check (void)
{
    uint32_t lane           = 0;
    uint32_t sbus_addr      = 0;
    int      o_core_status  = 0x0;
    bool     training_fail  = false;
    int      training_done  = 0x0;
    int      all_lanes_mask = 0x0;
    int      o_core_status_arr[4] = {0};
    int      tries = 0;
    int      max_retries = 10000;   // timeout after 100usec * 10000 = 1 sec

    while (true) {
        for (lane = 0; lane < num_lanes_; ++lane) {

            all_lanes_mask = all_lanes_mask | (1 << lane);

            if ( (training_done & (1 << lane)) != 0) {
                continue;
            }

            sbus_addr = port_sbus_addr(lane);

            o_core_status = serdes_fns()->serdes_an_core_status(sbus_addr);

            if ( (o_core_status & 0x2) == 0x0) {    // training completed
                if (o_core_status != 0x34) {        // training not yet passed
                    if ( (o_core_status & 0x1) == 0x1) {
                        // training failed
                        training_fail = true;
                    } else {
                        // training has completed, but not yet failed/passed
                        continue;
                    }
                }

                o_core_status_arr[lane] = o_core_status;
                training_done = training_done | (1 << lane);
            }
        }

        if ((training_done == all_lanes_mask) ||
            (tries++ == max_retries)) {
            break;
        }

        usleep(100);
    }

    for (lane = 0; lane < num_lanes_; ++lane) {
        sbus_addr = port_sbus_addr(lane);

        SDK_LINKMGR_TRACE_DEBUG("sbus_addr: %u, o_core_status: 0x%x, "
                                "training_fail: %d",
                                sbus_addr,
                                o_core_status_arr[lane],
                                training_fail);
    }

    return training_fail == false? true:false;
}

int
port::port_serdes_an_hcd_cfg (void)
{
    bool          skip         = false;
    uint32_t      an_hcd       = 0;
    uint32_t      lane         = 0;
    uint32_t      sbus_addr    = 0;
    int           fec_enable   = 0;
    int           rsfec_enable = 0;
    serdes_info_t *serdes_info = NULL;
    uint32_t      num_lanes    = 0;

    an_hcd       = serdes_fns()->serdes_an_hcd_read(port_sbus_addr(0));
    fec_enable   = serdes_fns()->serdes_an_fec_enable_read(port_sbus_addr(0));
    rsfec_enable = serdes_fns()->serdes_an_rsfec_enable_read(port_sbus_addr(0));

    SDK_LINKMGR_TRACE_DEBUG("port: %d, an_hcd: %d, fec_enable: %d, "
                            "rsfec_enable: %d",
                            port_num(), an_hcd, fec_enable, rsfec_enable);

    // set the negotiated port params
    if (port_set_an_resolved_params(an_hcd, fec_enable, rsfec_enable) == -1) {
        SDK_PORT_SM_TRACE(this, "Resolved params set failed");
        return -1;
    }

    // skip the settings on lanes 1-3 for 25G serdes since its already set
    // before AN start
    switch(an_hcd) {
    case 0x08: /* 100GBASE-KR4 */
    case 0x09: /* 100GBASE-CR4 */
    case 0x0a: /* 25GBASE-KRCR-S */
    case 0x0b: /* 25GBASE-KRCR */
        skip = true;
    default:
        break;
    }

    if (skip == true) {
        num_lanes = 1;
    } else {
        num_lanes = num_lanes_;
    }

    port_speed_t serdes_speed =
                    port_speed_to_serdes_speed(this->port_speed_);

    for (lane = 0; lane < num_lanes; ++lane) {
        sbus_addr = port_sbus_addr(lane);

        serdes_info = serdes_info_get(
                                sbus_addr,
                                static_cast<uint32_t>(serdes_speed),
                                cable_type());

        // configure Tx/Rx slip, Rx termination, Tx EQ
        serdes_fns()->serdes_cfg(sbus_addr, serdes_info);
    }

    // configure divider, width, start link training
    return serdes_fns()->serdes_an_hcd_cfg(port_sbus_addr(0), this->sbus_addr_);
}

int
port::port_set_an_resolved_params_internal (port_speed_t speed, int num_lanes,
                                            port_fec_type_t fec_type)
{
    this->set_port_speed(speed);
    this->set_num_lanes(num_lanes);
    this->set_fec_type(fec_type);

    return 0;
}

int
port::port_set_an_resolved_params (int an_hcd, int fec_enable, int rsfec_enable)
{
    int ret = 0;
    port_fec_type_t fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;

    switch (an_hcd) {
    case 0x08: /* 100GBASE-KR4 */
    case 0x09: /* 100GBASE-CR4 */
        fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
        port_set_an_resolved_params_internal(
                port_speed_t::PORT_SPEED_100G, 4, fec_type);
        break;

    case 0x03: /* 40GBASE-KR4 */
    case 0x04: /* 40GBASE-CR4 */
        port_set_an_resolved_params_internal(
                port_speed_t::PORT_SPEED_40G, 4, fec_type);
        break;

    case 0x0a: /* 25GBASE-KRCR-S */
    case 0x0b: /* 25GBASE-KRCR */
        if (fec_enable == 1) {
            fec_type = port_fec_type_t::PORT_FEC_TYPE_FC;
        } else if(rsfec_enable == 1) {
            fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
        }

        port_set_an_resolved_params_internal(
                port_speed_t::PORT_SPEED_25G, 1, fec_type);
        break;

    case 0x02:  /* 10GBASE-KR */
        port_set_an_resolved_params_internal(
                port_speed_t::PORT_SPEED_10G, 1, fec_type);
        break;

    // unsupported modes
    case 0x00: /* 1000BASE-KX */
    case 0x01: /* 10GBASE-KX4 */
    case 0x0c: /* 2.5GBASE-KX */
    case 0x0d: /* 5GBASE-KR */
    case 0x05: /* 100GBASE-CR10 */
    case 0x06: /* 100GBASE-KP4 */
    case 0x0e: /* 50GBASE-KR/CR */
    case 0x10: /* 100GBASE-KR2/CR2 */
    case 0x11: /* 200GBASE-KR4/CR4 */
    default:
    case 0x07:
    case 0x0f:
    case 0x1f:
        SDK_LINKMGR_TRACE_ERR("port: %d, unsupported an_hcd: %d",
                              port_num(), an_hcd);
        ret = -1;
        break;
    }

    return ret;
}

an_ret_t
port::port_link_sm_an_process(void)
{
    bool an_good = false;
    int  timeout = MIN_PORT_TIMER_INTERVAL;
    an_ret_t an_ret = AN_DONE;

    switch(this->link_an_sm_) {
    case port_link_sm_t::PORT_LINK_SM_AN_DISABLED:

        // transition to start AN
        set_port_link_an_sm(
                port_link_sm_t::PORT_LINK_SM_AN_START);

    case port_link_sm_t::PORT_LINK_SM_AN_START:

        SDK_PORT_SM_TRACE(this, "start AN");

        port_serdes_an_start();

        // transition to wait for AN HCD
        set_port_link_an_sm(
                port_link_sm_t::PORT_LINK_SM_AN_WAIT_HCD);

    case port_link_sm_t::PORT_LINK_SM_AN_WAIT_HCD:

        SDK_PORT_SM_TRACE(this, "wait AN HCD");

        // 5000 * 10 usecs = 50 msecs
        for (int i = 0; i < 5000; ++i) {
            an_good = port_serdes_an_wait_hcd();
            if (an_good == true) {
                break;
            }
            usleep(10);
        }

        // If AN HCD doesn't resolve:
        //   If retry count < MAX_PORT_AN_HCD_RETRIES:
        //     start bringup timer and return with AN_WAIT
        //  Else return with AN_RESET
        if(an_good == false) {
            set_num_an_hcd_retries(num_an_hcd_retries() + 1);
            if (num_an_hcd_retries() >= MAX_PORT_AN_HCD_RETRIES) {
                an_ret = AN_RESET;
                break;
            }
            this->bringup_timer_val_ += timeout;
            this->link_bring_up_timer_ =
                sdk::linkmgr::timer_schedule(
                        SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                        false);
            an_ret = AN_WAIT;
            break;
        }

        // transition to AN complete
        set_port_link_an_sm(
                port_link_sm_t::PORT_LINK_SM_AN_COMPLETE);

    case port_link_sm_t::PORT_LINK_SM_AN_COMPLETE:

        SDK_PORT_SM_TRACE(this, "AN HCD configure");

        port_serdes_an_hcd_cfg();

    default:
        break;
    }
    return an_ret;
}

bool
port::port_serdes_eye_check(void) {
    uint32_t lane = 0;
    uint32_t values[2 * MAX_SERDES_EYE_HEIGHTS];

    memset(values, 0, sizeof(uint32_t) * 2 * MAX_SERDES_EYE_HEIGHTS);
    for (lane = 0; lane < num_lanes_; ++lane) {
        if (serdes_fns()->serdes_eye_check(port_sbus_addr(lane), values) != 0) {
            return false;
        }
        SDK_LINKMGR_TRACE_DEBUG("port: %u, %3x %3x %3x %3x %3x %3x %3x %3x",
                                port_num(),
                                values[0], values[1], values[2], values[3],
                                values[4], values[5], values[6], values[7]);
        for (int i = 0; i < (2*MAX_SERDES_EYE_HEIGHTS); ++i) {
            if (values[i] == 0 || values[i] == 1 ||
                values[i] == 2 || values[i] >= 0xff) {
                return false;
            }
        }
    }
    return true;
}

dfe_ret_t
port::port_link_sm_dfe_process(void)
{
    bool dfe_complete = false;
    dfe_ret_t ret     = DFE_DONE;
    int  timeout      = MIN_PORT_TIMER_INTERVAL;
    bool retry_sm     = false;

    while (true) {
        retry_sm = false;

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
                        sdk::linkmgr::timer_schedule(
                            SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                            (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                            false);

                    ret = DFE_WAIT;
                    break;
                }

                // If eye check fails
                //   if count < MAX_PORT_SERDES_DFE_RETRIES
                //     retry ICAL
                //   else
                //     return with DFE_RESET
                if (port_serdes_eye_check() == false) {
                    set_port_link_dfe_sm(
                            port_link_sm_t::PORT_LINK_SM_DFE_START_ICAL);
                    set_num_dfe_retries(num_dfe_retries() + 1);
                    if (num_dfe_retries() < MAX_PORT_SERDES_DFE_RETRIES) {
                        retry_sm = true;
                        break;
                    } else {
                        ret = DFE_RESET;
                        break;
                    }
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
                    timeout = MIN_PORT_TIMER_INTERVAL;
                    this->bringup_timer_val_ += timeout;
                    this->link_bring_up_timer_ =
                        sdk::linkmgr::timer_schedule(
                            SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                            (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                            false);

                    ret = DFE_WAIT;
                    break;
                }

            default:
                break;
        }

        if (retry_sm == false) {
            break;
        }
    }

    return ret;
}

sdk_ret_t
port::port_link_sm_process(bool start_en_timer)
{
    int  timeout    = 500; // msecs
    bool sig_detect = false;
    bool serdes_rdy = false;
    bool mac_faults = true;
    bool mac_sync   = true;
    bool retry_sm   = false;
    an_ret_t an_ret = AN_DONE;
    dfe_ret_t dfe_ret = DFE_DONE;

    while (true) {
        retry_sm = false;

        switch (this->link_sm_) {
            case port_link_sm_t::PORT_LINK_SM_DISABLED:
                // set the link down ts
                set_last_down_ts();

                // Enable MAC TX drain and set Tx/Rx=0x0
                port_mac_tx_drain(true);

                // TODO Disable and drain PB
                port_pb_flush(true);
                port_pb_drain();
                port_pb_write_control(false);

                // reset timers

                if (this->link_bring_up_timer_ != NULL) {
                    sdk::linkmgr::timer_delete(this->link_bring_up_timer_);
                    this->link_bring_up_timer_ = NULL;
                }

                if (this->link_debounce_timer_ != NULL) {
                    sdk::linkmgr::timer_delete(this->link_debounce_timer_);
                    this->link_debounce_timer_ = NULL;
                }

                // pull the stats and save in persist storage before bringing down
                port_mac_stats_persist_update();

                // reset MAC
                port_mac_state_reset();

                // reset SERDES
                port_serdes_state_reset();

                // reset counters
                port_link_sm_counters_reset();

                // reset state

                // remove from link poll timer if present
                port_link_poll_timer_delete(this);

                // set operational status as down
                this->set_oper_status(port_oper_status_t::PORT_OPER_STATUS_DOWN);

                SDK_PORT_SM_DEBUG(this, "Disabled");

                break;

            case port_link_sm_t::PORT_LINK_SM_ENABLED:

                if ((start_en_timer == true) ||
                    (SDK_ATOMIC_LOAD_BOOL(&hal_cfg) == true)) {
                    // if this is invoked by cfg thread, start the timer so that
                    // the call can return
                    this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);
                    timeout = MIN_PORT_TIMER_INTERVAL;
                    this->bringup_timer_val_ += timeout;
                    this->link_bring_up_timer_ =
                        sdk::linkmgr::timer_schedule(
                            SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                            (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                            false);
                    break;
                }

                SDK_PORT_SM_DEBUG(this, "Enabled");

                // transition to AN cfg state
                this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_AN_CFG);

             case port_link_sm_t::PORT_LINK_SM_AN_CFG:

                if (auto_neg_enable() == true) {
                    an_ret = port_link_sm_an_process();

                    if (an_ret == AN_RESET) {
                        // AN HCD resolution failed. Restart AN
                        retry_sm = true;
                        set_num_an_hcd_retries(0);
                        // skip the serdes reset since it is done during AN_INIT
                        port_link_sm_retry_enabled(false);
                        break;
                    } else if (an_ret == AN_WAIT) {
                        // Timers started by AN SM. Just exit.
                        break;
                    } else if (an_ret == AN_SKIP) {
                        // only for debugging
                        break;
                    } else {
                        // AN SM configures serdes
                        // transition to wait serdes rdy state
                        this->set_port_link_sm(
                                port_link_sm_t::PORT_LINK_SM_WAIT_SERDES_RDY);
                        retry_sm = true;
                        break;
                    }
                }

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
                        sdk::linkmgr::timer_schedule(
                            SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
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

                // send MAC remote faults
                port_mac_send_remote_faults(true);

                // bring MAC out of reset and enable
                port_mac_enable(true);
                port_mac_soft_reset(false);

                if (auto_neg_enable() == true) {
                    // Restart AN if link training failed.
                    if (port_serdes_an_link_train_check() == false ||
                            port_serdes_eye_check() == false ) {
                        set_num_link_train_retries(num_link_train_retries() + 1);
                        if (num_link_train_retries() == MAX_LINK_TRAIN_FAIL_COUNT) {
                            set_num_link_train_retries(0);
                            timeout = rand() % 2 == 0? 500 : MIN_PORT_TIMER_INTERVAL;
                            this->bringup_timer_val_ += timeout;
                            this->link_bring_up_timer_ =
                                sdk::linkmgr::timer_schedule(
                                        SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                                        (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                                        false);
                        } else {
                            retry_sm = true;
                        }
                        set_num_an_hcd_retries(0);
                        // skip the serdes reset since it is done during AN_INIT
                        port_link_sm_retry_enabled(false);
                        break;
                    }

                    // Link training would have completed ICAL.
                    // Transition DFE SM to pcal one shot for AN
                    set_port_link_dfe_sm(
                            port_link_sm_t::PORT_LINK_SM_DFE_START_PCAL);

                    // transition link SM to AN DFE tuning
                    this->set_port_link_sm(
                                    port_link_sm_t::PORT_LINK_SM_AN_DFE_TUNING);
                    retry_sm = true;
                    break;
                }

                // transition to serdes signal detect state
                this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT);

            case port_link_sm_t::PORT_LINK_SM_AN_DFE_TUNING:
                dfe_ret = port_link_sm_dfe_process();
                if (dfe_ret == DFE_WAIT) {
                    // DFE tuning is pending
                    // Timer would have been already started. So just break.
                    break;
                }

                // transition to clear MAC remote faults
                this->set_port_link_sm(
                    port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS);
                retry_sm = true;
                break;

            case port_link_sm_t::PORT_LINK_SM_SIGNAL_DETECT:

                // enable serdes
                port_serdes_output_enable(true);

                SDK_PORT_SM_DEBUG(this, "Wait for signal detect");

                sig_detect = port_serdes_signal_detect();

                if(sig_detect == false) {

                    this->bringup_timer_val_ += timeout;

                    this->link_bring_up_timer_ =
                        sdk::linkmgr::timer_schedule(
                                SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                                (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                                false);
                    break;
                }

                // transition to DFE tuning stage
                this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DFE_TUNING);

            case port_link_sm_t::PORT_LINK_SM_DFE_TUNING:

                if (port_dfe_tuning_enabled()) {
                    dfe_ret = port_link_sm_dfe_process();
                    if (dfe_ret == DFE_WAIT) {
                        // DFE tuning is pending
                        // Timer would have been already started. So just break.
                        break;
                    } else if (dfe_ret == DFE_RESET) {
                        retry_sm = true;
                        set_num_dfe_retries(0);
                        port_link_sm_retry_enabled(false);
                        break;
                    }
                }

                // transition to clear MAC remote faults
                this->set_port_link_sm(
                        port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS);

            case port_link_sm_t::PORT_LINK_SM_CLEAR_MAC_REMOTE_FAULTS:
                SDK_PORT_SM_DEBUG(this, "Clear MAC remote faults");

                // clear MAC remote faults
                port_mac_send_remote_faults(false);

                // transition to wait for mac sync
                this->set_port_link_sm(
                                port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC);

            case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_SYNC:
                SDK_PORT_SM_DEBUG(this, "Wait MAC SYNC");

                mac_sync = port_mac_sync_get();

                if(mac_sync == false) {
                    set_num_mac_sync_retries(num_mac_sync_retries() + 1);
                    if (num_mac_sync_retries() >= MAX_PORT_MAC_SYNC_RETRIES) {
                        retry_sm = true;
                        set_num_mac_sync_retries(0);
                        port_link_sm_retry_enabled();
                        break;
                    }

                    this->bringup_timer_val_ += timeout;

                    this->link_bring_up_timer_ =
                        sdk::linkmgr::timer_schedule(
                            SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                            (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                            false);

                    break;
                }

                // transition to mac faults to be cleared
                this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR);

            case port_link_sm_t::PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR:

                SDK_PORT_SM_DEBUG(this, "Wait MAC faults clear");

                mac_faults = port_mac_faults_get();
                if (mac_faults == true) {
                    set_mac_faults(this->mac_faults() + 1);
                } else {
                    set_mac_faults(0);
                }

                // If the PCS faults/errors after MAC sync is persistent, restart SM
                if (this->mac_faults() > 0) {
                    // if MAC errors are detected, reset the MAC no-faults counter
                    set_num_mac_nofaults(0);

                    if (this->mac_faults() < MAX_PORT_MAC_FAULTS_CHECK) {
                        SDK_PORT_SM_DEBUG(this, "MAC faults detected");
                        timeout = MIN_PORT_TIMER_INTERVAL;
                        this->bringup_timer_val_ += timeout;
                        this->link_bring_up_timer_ =
                            sdk::linkmgr::timer_schedule(
                                SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                                (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                                false);
                        break;
                    } else {
                        SDK_PORT_SM_DEBUG(this,
                                            "MAC faults persistent, retry SM");
                        retry_sm = true;
                        set_mac_faults(0);
                        port_link_sm_retry_enabled();
                        break;
                    }
                } else if (fec_type() == port_fec_type_t::PORT_FEC_TYPE_NONE) {
                    // For 10G/25G-no-fec scenario, comira MAC has an issue
                    // where MAC gets sync and PCS error counter could still
                    // be 0 though peer is shutdown or not connected.
                    // To workaround that, check the error counter
                    // multiple times
                    if (port_speed() == port_speed_t::PORT_SPEED_10G ||
                        port_speed() == port_speed_t::PORT_SPEED_25G) {
                        set_num_mac_nofaults(num_mac_nofaults() + 1);
                        if (num_mac_nofaults() < MAX_PORT_MAC_NOFAULTS_CHECK) {
                            SDK_PORT_SM_DEBUG(this, "MAC faults check retry");
                            timeout = MIN_PORT_TIMER_INTERVAL;
                            this->bringup_timer_val_ += timeout;
                            this->link_bring_up_timer_ =
                                sdk::linkmgr::timer_schedule(
                                    SDK_TIMER_ID_LINK_BRINGUP, timeout, this,
                                    (sdk::lib::twheel_cb_t)link_bring_up_timer_cb,
                                    false);
                            break;
                        }
                    }
                }

                // transition to link up state
                this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_UP);

            case port_link_sm_t::PORT_LINK_SM_UP:

                SDK_PORT_SM_DEBUG(this, "PCAL continuous");
                port_serdes_pcal_continuous_start();

                set_last_up_ts();

                SDK_PORT_SM_TRACE(this, "Link UP");

                // start persist stats collection - relevant on first link-up and remains set
                port_mac_stats_persist_collect_enable();

                // TODO Enable PB
                port_pb_drain();
                port_pb_flush(false);
                port_pb_write_control(true);

                // Disable MAC TX drain and set Tx/Rx=0x1
                port_mac_tx_drain(false);

                // enable mac interrupts
                port_mac_intr_en(true);

                // set operational status as up
                this->set_oper_status(port_oper_status_t::PORT_OPER_STATUS_UP);
                set_bringup_duration();

                // enable pCal

                // add to link poll timer
                port_link_poll_timer_add(this);

                // notify others that link is up
                port_event_notify(port_event_t::PORT_EVENT_LINK_UP);
                break;

            default:
                break;
        }

        if (retry_sm == false) {
            break;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_enable(bool start_en_timer)
{
    // check if already enabled
    if (this->admin_state_ == port_admin_state_t::PORT_ADMIN_STATE_UP) {
        return SDK_RET_OK;
    }

    // enable the port
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);
    set_port_link_dfe_sm(port_link_sm_t::PORT_LINK_SM_DFE_DISABLED);
    set_port_link_an_sm(port_link_sm_t::PORT_LINK_SM_AN_DISABLED);

    port_link_sm_process(start_en_timer);

    this->admin_state_ = port_admin_state_t::PORT_ADMIN_STATE_UP;

    return SDK_RET_OK;
}

sdk_ret_t
port::port_link_sm_reset (void)
{
    set_port_link_sm(port_link_sm_t::PORT_LINK_SM_DISABLED);
    set_port_link_dfe_sm(port_link_sm_t::PORT_LINK_SM_DFE_DISABLED);
    set_port_link_an_sm(port_link_sm_t::PORT_LINK_SM_AN_DISABLED);
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

    port_link_sm_reset();

    port_link_sm_process();

    this->admin_state_ = port_admin_state_t::PORT_ADMIN_STATE_DOWN;

    // Notfiy the link down if link was up before disable
    if (prev_oper_status == port_oper_status_t::PORT_OPER_STATUS_UP) {
        port_event_notify(port_event_t::PORT_EVENT_LINK_DOWN);
    }

    return SDK_RET_OK;;
}

sdk_ret_t
port::port_event_notify(port_event_t port_event)
{
    port_event_info_t port_event_info;

    switch(port_event) {
    case port_event_t::PORT_EVENT_LINK_UP:
        break;

    case port_event_t::PORT_EVENT_LINK_DOWN:
        // increment the link down counter
        set_num_link_down(num_link_down() + 1);

        port_link_poll_timer_delete(this);
        break;

    default:
        break;
    }

    port_event_info.logical_port = port_num();
    port_event_info.event = port_event;
    port_event_info.speed = port_speed();
    port_event_info.type = port_type();
    port_event_info.num_lanes = num_lanes(); 
    port_event_info.oper_status = oper_status();
    port_event_info.auto_neg_enable = auto_neg_enable();

    if (g_linkmgr_cfg.port_event_cb) {
        g_linkmgr_cfg.port_event_cb(&port_event_info);
    }

    return SDK_RET_OK;
}

sdk_ret_t
port::port_link_dn_handler(void)
{
    // start the debounce timer
    if (this->debounce_time_ != 0) {
        this->link_debounce_timer_ =
            sdk::linkmgr::timer_schedule(
                SDK_TIMER_ID_LINK_DEBOUNCE, this->debounce_time_, this,
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

// for absoulte time diff, use CLOCK_MONOTONIC_RAW
sdk_ret_t
port::set_last_down_ts(void)
{
    time_t now = time(NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &this->last_down_ts_);
    strftime(this->last_down_timestamp_, TIME_STR_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&now));

    return SDK_RET_OK;
}

sdk_ret_t
port::set_last_up_ts(void)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &this->last_up_ts_);
    return SDK_RET_OK;
}

// If curent oper_status is UP, then, update bringup_duration_
sdk_ret_t
port::set_bringup_duration(void)
{
    if (this->oper_status_ == port_oper_status_t::PORT_OPER_STATUS_UP) {
        this->bringup_duration_ = sdk::timestamp_diff(&this->last_up_ts_, &this->last_down_ts_);
        SDK_LINKMGR_TRACE_DEBUG("port: %u Link down count %d last bringup duration: %lus.%luns\n", port_num(),
                                num_link_down(),
                                this->bringup_duration_.tv_sec,
                                this->bringup_duration_.tv_nsec);

    }
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_get (uint64_t *stats_data)
{
    mac_fns()->mac_stats_get(this->mac_id_, this->mac_ch_, stats_data);
    // add persist stats to this current stats_data and return
    this->port_mac_stats_persist_collate(stats_data);
    // Publish collated stats to HBM
    this->port_mac_stats_publish(stats_data);
    return SDK_RET_OK;
}

sdk_ret_t
port::port_deinit (void)
{
    mac_fns()->mac_deinit (this->mac_id_, this->mac_ch_);
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_set_pause_src_addr(uint8_t *mac_addr) {
    mac_fns()->mac_pause_src_addr(mac_id(), mac_ch(), mac_addr);
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_state_reset(void)
{
    // disable and clear mac interrupts
    port_mac_intr_en(false);
    port_mac_intr_clr();

    // disable and put mac in reset
    port_mac_soft_reset(true);
    port_mac_enable(false);

    // reset MAC global programming since serdes config
    // is changed to run AN
    if (auto_neg_enable() == true) {
        port_deinit();
    }
    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_state_reset(void)
{
    // disable serdes
    port_serdes_output_enable(false);

    // soft spico reset
    port_serdes_reset(true);
    return SDK_RET_OK;
}

void
port::port_link_sm_counters_reset(void)
{
    // reset bringup timer
    this->bringup_timer_val_ = 0;

    // reset MAC faults counter
    set_mac_faults(0);

    // reset MAC no-faults counter
    set_num_mac_nofaults(0);

    // reset number of MAC sync retries
    set_num_mac_sync_retries(0);

    // reset AN HCD retry counter
    set_num_an_hcd_retries(0);

    // reset AN link training retry counter
    set_num_link_train_retries(0);

    // reset dfe retry count
    set_num_dfe_retries(0);

    // reset number of link bringup retries
    set_num_retries(0);
}

sdk_ret_t
port::port_link_sm_retry_enabled(bool serdes_reset)
{
    // reset all SM states
    port_link_sm_reset();
    if (serdes_reset == true) {
        port_serdes_state_reset();
    }
    this->set_port_link_sm(port_link_sm_t::PORT_LINK_SM_ENABLED);
    return SDK_RET_OK;
}

// Function to add stats before last reset to current stats and return
sdk_ret_t
port::port_mac_stats_persist_collect_enable(void)
{
    if (this->persist_stats_collect_ == false) {
        SDK_PORT_SM_TRACE(this, "Enabling persistent stats collection");
    }
    this->persist_stats_collect_ = true;
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_persist_collect_disable(void)
{
    this->persist_stats_collect_ = false;
    SDK_PORT_SM_TRACE(this, "Disabling persistent stats collection");
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_persist_collate(uint64_t *stats_data)
{
    int iter;

    for (iter = 0; iter < MAX_MAC_STATS; iter++) {
        stats_data[iter] += this->persist_stats_data_[iter];
    }
    return SDK_RET_OK;
}

// Init mac stats reporting to HBM (json needs entry)
sdk_ret_t
port::port_mac_stats_init(void)
{
    sdk::types::mem_addr_t port_stats_base = INVALID_MEM_ADDRESS;

    this->port_stats_base_addr_ = INVALID_MEM_ADDRESS;
    if (g_linkmgr_cfg.mempartition == NULL) {
        SDK_TRACE_ERR("port %u stats_init NULL mempartition port stats not supported",
            this->port_num_);
        return SDK_RET_OK; // legacy
    }
    port_stats_base = g_linkmgr_cfg.mempartition->start_addr(CAPRI_HBM_REG_PORT_STATS);
    SDK_TRACE_DEBUG("port %u stats_init mpartition 0x%x, port_stats_base 0x%llx",
            this->port_num_, g_linkmgr_cfg.mempartition, port_stats_base);

    if ((port_stats_base == 0) || (port_stats_base== INVALID_MEM_ADDRESS)) {
        // legacy json; old stats model; log and return ok
        SDK_TRACE_ERR("port %u stats_init port_stats_base 0x%llx port stats not supported",
            this->port_num_, port_stats_base);
        return SDK_RET_OK;
    }

    if ((this->port_num_ >= 1) && (this->port_num_ <= 4)) {
        this->port_stats_base_addr_ = port_stats_base;
    } else if((this->port_num_ >= 5) && (this->port_num_ <= 8)) {
        this->port_stats_base_addr_ = port_stats_base + PORT_MAC_STAT_REPORT_SIZE;
    } else if(this->port_num_ == 9) {
        this->port_stats_base_addr_ = port_stats_base + (PORT_MAC_STAT_REPORT_SIZE * 2);
    } else {
        SDK_TRACE_ERR("port %u port stats not supported", this->port_num_);
        this->port_stats_base_addr_ = INVALID_MEM_ADDRESS;
    }

    SDK_TRACE_DEBUG("port %u stats_init port_stats_base = 0x%llx, port_stats_base_addr_ = 0x%llx",
                     this->port_num_, port_stats_base, this->port_stats_base_addr_);

    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_init(port *port_p)
{
    return port_p->port_mac_stats_init();
}

sdk_ret_t
port::port_mac_stats_publish(uint64_t *stats_data)
{
    uint32_t len;
    sdk_ret_t ret = SDK_RET_OK;

    if (this->port_stats_base_addr_ == INVALID_MEM_ADDRESS) {
        // We don't publish to HBM when we don't have the port_stats region available
        return SDK_RET_OK; // return ok. legacy stats collection
    }

    len = MAX_MAC_STATS * sizeof(uint64_t); // byte granularity

    ret = sdk::asic::asic_mem_write(this->port_stats_base_addr_, (uint8_t *) stats_data, len);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_DEBUG("port %u stats_publish port_stats_base_addr_ = 0x%llx asic_mem_write failed ret = 0x%x",
                         this->port_num_, this->port_stats_base_addr_, ret);
    }
    return ret;
}

sdk_ret_t
port::port_mac_stats_persist_update(void)
{
    int iter;
    uint64_t stats_data[MAX_MAC_STATS];

    if (this->persist_stats_collect_ == true) {
        mac_fns()->mac_stats_get(this->mac_id_, this->mac_ch_, stats_data);

        SDK_PORT_SM_TRACE(this, "Updating persistent stats");
        for (iter = 0; iter < MAX_MAC_STATS; iter++) {
            this->persist_stats_data_[iter] += stats_data[iter];
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
port::port_mac_stats_persist_clear(bool reset)
{
    if (reset) {
        SDK_PORT_SM_TRACE(this, "Clearing persistent stats");
        memset(this->persist_stats_data_, 0, sizeof(this->persist_stats_data_));
    }
    return SDK_RET_OK;
}

sdk_ret_t
port::port_pb_drain(void) {
    uint32_t tm_port = logical_port_to_tm_port(port_num());

    // TODO remove capri reference
    return sdk::platform::capri::capri_tm_drain_uplink_port(tm_port);
}

sdk_ret_t
port::port_pb_write_control(bool enable) {
    uint32_t tm_port = logical_port_to_tm_port(port_num());

    // TODO remove capri reference
    return sdk::platform::capri::capri_tm_write_control_uplink_port(
                                                        tm_port, enable);
}

sdk_ret_t
port::port_pb_flush(bool enable) {
    uint32_t tm_port = logical_port_to_tm_port(port_num());

    // TODO remove capri reference
    return sdk::platform::capri::capri_tm_flush_uplink_port(
                                                        tm_port, enable);
}

sdk_ret_t
port::port_pb_enable(bool enable) {
    uint32_t tm_port = logical_port_to_tm_port(port_num());

    // TODO remove capri reference
    return sdk::platform::capri::capri_tm_enable_disable_uplink_port(
                                                        tm_port, enable);
}

sdk_ret_t
port::port_mac_tx_drain(bool drain) {
    mac_fns()->mac_tx_drain(mac_id(), mac_ch(), drain);
    return SDK_RET_OK;
}

//----------------------------------------------------
// static methods
//----------------------------------------------------
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

    srand(time(NULL));
    return rc;
}

sdk_ret_t
port::phy_port_mac_addr(uint32_t phy_port, mac_addr_t mac_addr) {
    std::string   mac_addr_str;

    if (sdk::platform::readFruKey(MACADDRESS_KEY, mac_addr_str) == -1) {
        return SDK_RET_ERR;
    }

    mac_str_to_addr((char *)mac_addr_str.c_str(), mac_addr);

    // base mac addr is for first xcvr port, increment the last byte for
    // subsequent ports
    mac_addr[5] += (uint8_t)(phy_port - 1);
    return SDK_RET_OK;
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

        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_ENABLE, &data,
                             q_notify_mode_t::Q_NOTIFY_MODE_BLOCKING);

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

        ret = linkmgr_notify(LINKMGR_OPERATION_PORT_DISABLE, &data,
                             q_notify_mode_t::Q_NOTIFY_MODE_BLOCKING);

        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error notifying control-thread for port disable");
        }
    }

    return ret;
}

}    // namespace linkmgr
}    // namespace sdk
