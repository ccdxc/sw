// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_PORT_HPP__
#define __SDK_LINKMGR_PORT_HPP__

#include "sdk/base.hpp"
#include "sdk/types.hpp"
#include "linkmgr.hpp"

namespace sdk {
namespace linkmgr {

#define PORT_LANES_MAX 4

#define MXP_BASE_HAPS           0x01d00000
#define MXP_INST_STRIDE_HAPS    0x00100000
#define MXP_PORT_STRIDE_HAPS    0x2000
#define TEMAC_BASE_OFFSET_HAPS  0x1000

#define PHY_RESET_OFFSET_HAPS    0x4
#define SGMII_RESET_OFFSET_HAPS  0x2000
#define TEMAC_RESET_OFFSET_HAPS  0x2008
#define DATAPATH_RESET_OFFSET    0x200c

#define MDIO_SETUP_OFFSET_HAPS   0x500
#define MDIO_CTRL_OFFSET_HAPS    0x504
#define MDIO_DATA_WR_OFFSET_HAPS 0x508
#define MDIO_DATA_RD_OFFSET_HAPS 0x50c

enum class port_link_sm_t {
    PORT_LINK_SM_DISABLED,
    PORT_LINK_SM_ENABLED,
    PORT_LINK_SM_SERDES_CFG,
    PORT_LINK_SM_WAIT_SERDES_RDY,
    PORT_LINK_SM_MAC_CFG,
    PORT_LINK_SM_SIGNAL_DETECT,
    PORT_LINK_SM_WAIT_MAC_FAULTS_CLEAR,
    PORT_LINK_SM_UP
};

typedef struct mac_fn_s_ {
    int (*mac_cfg) (uint32_t port_num, uint32_t speed, uint32_t num_lanes);
    int (*mac_enable) (uint32_t port_num, uint32_t speed,
                       uint32_t num_lanes, bool enable);
    int (*mac_soft_reset) (uint32_t port_num, uint32_t speed,
                           uint32_t num_lanes, bool reset);
    int (*mac_stats_reset) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool reset);
    int (*mac_intr_clear) (uint32_t port_num, uint32_t speed,
                           uint32_t num_lanes);
    int (*mac_intr_enable) (uint32_t port_num, uint32_t speed,
                            uint32_t num_lanes, bool enable);
    bool (*mac_faults_get) (uint32_t port_num);

} mac_fn_t;

typedef struct serdes_fn_s_ {
    int (*serdes_cfg) (uint32_t sbus_addr);
    int (*serdes_tx_rx_enable) (uint32_t sbus_addr, bool enable);
    int (*serdes_output_enable) (uint32_t sbus_addr, bool enable);
    int (*serdes_reset) (uint32_t sbus_addr, bool reset);
    bool (*serdes_signal_detect) (uint32_t sbus_addr);
    bool (*serdes_rdy) (uint32_t sbus_addr);
} serdes_fn_t;

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

    port_link_sm_t port_link_sm(void) const { return this->link_sm_; }
    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
    }

    void *link_bring_up_timer(void) const {
        return this->link_bring_up_timer_;
    }

    void set_link_bring_up_timer(void *link_bring_up_timer) {
        this->link_bring_up_timer_ = link_bring_up_timer;
    }

    sdk_ret_t port_enable(void);
    sdk_ret_t port_disable(void);
    sdk_ret_t port_link_sm_process(void);

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

    // ----------------------------------------------------
    // static methods
    // ----------------------------------------------------

    static sdk_ret_t
        link_bring_up_timer_cb(void *timer, uint32_t timer_id, void *ctxt);

    static sdk_ret_t port_init(linkmgr_cfg_t *cfg);

    static sdk_ret_t port_mac_fn_init(linkmgr_cfg_t *cfg);
    static sdk_ret_t port_serdes_fn_init(linkmgr_cfg_t *cfg);

    static mac_fn_t mac_fn;

    static serdes_fn_t serdes_fn;

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_enable(port *port_p);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static sdk_ret_t port_disable(port *port_p);

private:
    port_oper_status_t    oper_status_;    // port operational status
    port_speed_t          port_speed_;     // port speed
    port_type_t           port_type_;      // port type
    port_admin_state_t    admin_state_;    // port admin state
    port_link_sm_t        link_sm_;          // port link state machine
    void                  *link_bring_up_timer_;   // port link bring up timer
    uint32_t              mac_id_;          // mac instance for this port
    uint32_t              mac_ch_;          // mac channel within mac instance
    uint32_t              num_lanes_;       // number of lanes for this port

    // MAC port num calculation based on mac instance and mac channel
    uint32_t  port_mac_port_num_calc(void);

    // Get serdes sbus address for a port lane
    uint32_t port_sbus_addr(uint32_t lane);
};

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_LINKMGR_PORT_HPP__
