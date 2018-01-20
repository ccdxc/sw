// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_PORT_HPP__
#define __LINKMGR_PORT_HPP__

#include "nic/include/base.h"
#include "sdk/types.hpp"
#include "nic/gen/proto/hal/port.pb.h"

extern uint32_t read_reg_base (uint32_t chip, uint64_t addr);
extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t  data);

#define WRITE_REG_BASE(chip, addr, data) {                                  \
    HAL_TRACE_DEBUG("PORT: {0:s}: chip {1:d} addr 0x{2:x} data 0x{3:x}",    \
                    __FUNCTION__, chip, addr, data);                        \
    if (linkmgr::hw_access_mock_mode() == false) {  \
        write_reg_base(chip, addr, data);                                   \
    }                                                                       \
}

#define READ_REG_BASE(chip, addr, data) {                       \
    HAL_TRACE_DEBUG("PORT: {0:s}: chip {1:d} addr 0x{2:x}",     \
                    __FUNCTION__, chip, addr);                  \
    if (linkmgr::hw_access_mock_mode() == false) {  \
        *data = read_reg_base(chip, addr);                      \
    }                                                           \
}

namespace linkmgr {

extern bool hw_access_mock_mode(void);
extern platform_type_t platform_type(void);
extern uint32_t sbus_addr(uint32_t asic, uint32_t asic_port, uint32_t lane);

namespace pd {

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
    ::port::PortOperStatus oper_status() {
        return this->oper_status_;
    }

    void set_oper_status(::port::PortOperStatus oper_status) {
        this->oper_status_ = oper_status;
    }

    ::port::PortSpeed port_speed() {
        return this->port_speed_;
    }

    void set_port_speed(::port::PortSpeed port_speed) {
        this->port_speed_ = port_speed;
    }

    ::port::PortType port_type() {
        return this->port_type_;
    }

    void set_port_type(::port::PortType port_type) {
        this->port_type_ = port_type;
    }

    void set_admin_state(::port::PortAdminState admin_state) {
        this->admin_state_ = admin_state;
    }

    ::port::PortAdminState admin_state() {
        return this->admin_state_;
    }

    uint32_t mac_ch() { return this->mac_ch_; }
    void set_mac_ch(uint32_t mac_ch) { this->mac_ch_ = mac_ch; }

    uint32_t mac_id() { return this->mac_id_; }
    void set_mac_id(uint32_t mac_id) { this->mac_id_ = mac_id; }

    uint32_t num_lanes() { return this->num_lanes_; }
    void set_num_lanes(uint32_t num_lanes) { this->num_lanes_ = num_lanes; }

    port_link_sm_t port_link_sm() {
        return this->link_sm_;
    }

    void set_port_link_sm(port_link_sm_t link_sm) {
        this->link_sm_ = link_sm;
    }

    void *link_bring_up_timer() {
        return this->link_bring_up_timer_;
    }

    void set_link_bring_up_timer(void *link_bring_up_timer) {
        this->link_bring_up_timer_ = link_bring_up_timer;
    }

    hal_ret_t port_enable();
    hal_ret_t port_disable();
    hal_ret_t port_link_sm_process();

    // ----------------------------------------------------
    // mac methods
    // ----------------------------------------------------

    // MAC CFG
    hal_ret_t port_mac_cfg();

    // mac enable or disable
    hal_ret_t port_mac_enable(bool);

    // mac software reset
    hal_ret_t port_mac_soft_reset(bool);

    // mac stats reset
    hal_ret_t port_mac_stats_reset(bool);

    // mac interrupt enable or disable
    hal_ret_t port_mac_intr_en(bool enable);

    // mac interrupt clear
    hal_ret_t port_mac_intr_clr();

    // tdm config and any other mac init
    hal_ret_t port_mac_init();

    // mac fifo ctrl 1 configure
    hal_ret_t port_mac_fifo_ctrl();

    // set the mac global mode
    hal_ret_t port_mac_global_mode_cfg();

    // mac channel enable
    hal_ret_t port_mac_ch_enable(bool);

    // mac generic cfg
    // app fifo portmap
    // rx fifo control
    // channel mapping
    hal_ret_t port_mac_generic_cfg();

    // mac rx and tx enable
    hal_ret_t port_mac_rx_tx_enable(bool, bool);

    //mac channel mode config
    hal_ret_t port_mac_ch_mode_cfg();

    // mac faults
    bool port_mac_faults_get();

    // ----------------------------------------------------
    // serdes methods
    // ----------------------------------------------------

    // serdes config
    hal_ret_t port_serdes_cfg();

    // enable serdes tx and rx
    hal_ret_t port_serdes_tx_rx_enable(bool enable);

    // enable serdes output
    hal_ret_t port_serdes_output_enable(bool enable);

    // serdes spico reset
    hal_ret_t port_serdes_reset(bool reset);

    // serdes signal detect
    bool port_serdes_signal_detect();

    // check for serdes ready
    bool port_serdes_rdy();

    // ----------------------------------------------------
    // static methods
    // ----------------------------------------------------

    static hal_ret_t
        link_bring_up_timer_cb(void *timer, uint32_t timer_id, void *ctxt);

    static hal_ret_t port_init();

    static hal_ret_t port_mac_fn_init();
    static hal_ret_t port_serdes_fn_init();

    static mac_fn_t mac_fn;

    static serdes_fn_t serdes_fn;

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static hal_ret_t port_enable(port *pd_p);

    // If current_thread is hal-control thread, invoke method directly
    // Else trigger hal-control thread to invoke method
    static hal_ret_t port_disable(port *pd_p);

private:
    ::port::PortOperStatus  oper_status_;    // port operational status
    ::port::PortSpeed       port_speed_;     // port speed
    ::port::PortType        port_type_;      // port type
    ::port::PortAdminState  admin_state_;    // port admin state

    port_link_sm_t        link_sm_;        // port link state machine
    void                  *link_bring_up_timer_;   // port link bring up timer

    uint32_t              mac_id_;          // mac instance for this port
    uint32_t              mac_ch_;          // mac channel within mac instance
    uint32_t              num_lanes_;       // number of lanes for this port

    // MAC port num calculation based on mac instance and mac channel
    uint32_t  port_mac_port_num_calc();

    // Get serdes sbus address for a port lane
    uint32_t port_sbus_addr(uint32_t lane);
};

}    // namespace pd
}    // namespace linkmgr

#endif  // __LINKMGR_PORT_HPP__
