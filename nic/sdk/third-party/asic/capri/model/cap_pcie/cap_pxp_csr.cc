
#include "cap_pxp_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pxp_csr_int_p_ecc_int_enable_clear_t::cap_pxp_csr_int_p_ecc_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_int_p_ecc_int_enable_clear_t::~cap_pxp_csr_int_p_ecc_int_enable_clear_t() { }

cap_pxp_csr_intreg_t::cap_pxp_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_intreg_t::~cap_pxp_csr_intreg_t() { }

cap_pxp_csr_intgrp_t::cap_pxp_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_intgrp_t::~cap_pxp_csr_intgrp_t() { }

cap_pxp_csr_intreg_status_t::cap_pxp_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_intreg_status_t::~cap_pxp_csr_intreg_status_t() { }

cap_pxp_csr_int_groups_int_enable_rw_reg_t::cap_pxp_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_int_groups_int_enable_rw_reg_t::~cap_pxp_csr_int_groups_int_enable_rw_reg_t() { }

cap_pxp_csr_intgrp_status_t::cap_pxp_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_intgrp_status_t::~cap_pxp_csr_intgrp_status_t() { }

cap_pxp_csr_csr_intr_t::cap_pxp_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_csr_intr_t::~cap_pxp_csr_csr_intr_t() { }

cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::~cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t() { }

cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::cap_pxp_csr_sat_p_port_cnt_phystatus_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::~cap_pxp_csr_sat_p_port_cnt_phystatus_err_t() { }

cap_pxp_csr_sat_p_port_cnt_deskew_err_t::cap_pxp_csr_sat_p_port_cnt_deskew_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_deskew_err_t::~cap_pxp_csr_sat_p_port_cnt_deskew_err_t() { }

cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::~cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t() { }

cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::~cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t() { }

cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::~cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t() { }

cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::~cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t() { }

cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::cap_pxp_csr_sat_p_port_cnt_replay_num_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::~cap_pxp_csr_sat_p_port_cnt_replay_num_err_t() { }

cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::~cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t() { }

cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::cap_pxp_csr_sat_p_port_cnt_fc_timeout_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::~cap_pxp_csr_sat_p_port_cnt_fc_timeout_t() { }

cap_pxp_csr_sat_p_port_cnt_fcpe_t::cap_pxp_csr_sat_p_port_cnt_fcpe_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_fcpe_t::~cap_pxp_csr_sat_p_port_cnt_fcpe_t() { }

cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::cap_pxp_csr_sat_p_port_cnt_rx_nullified_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::~cap_pxp_csr_sat_p_port_cnt_rx_nullified_t() { }

cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::~cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t() { }

cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::~cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t() { }

cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::~cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t() { }

cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::~cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t() { }

cap_pxp_csr_sta_p_ecc_txbuf_3_t::cap_pxp_csr_sta_p_ecc_txbuf_3_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_ecc_txbuf_3_t::~cap_pxp_csr_sta_p_ecc_txbuf_3_t() { }

cap_pxp_csr_sta_p_ecc_txbuf_2_t::cap_pxp_csr_sta_p_ecc_txbuf_2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_ecc_txbuf_2_t::~cap_pxp_csr_sta_p_ecc_txbuf_2_t() { }

cap_pxp_csr_sta_p_ecc_txbuf_1_t::cap_pxp_csr_sta_p_ecc_txbuf_1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_ecc_txbuf_1_t::~cap_pxp_csr_sta_p_ecc_txbuf_1_t() { }

cap_pxp_csr_sta_p_ecc_txbuf_0_t::cap_pxp_csr_sta_p_ecc_txbuf_0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_ecc_txbuf_0_t::~cap_pxp_csr_sta_p_ecc_txbuf_0_t() { }

cap_pxp_csr_sta_p_sram_bist_t::cap_pxp_csr_sta_p_sram_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_sram_bist_t::~cap_pxp_csr_sta_p_sram_bist_t() { }

cap_pxp_csr_cfg_p_mac_test_in_t::cap_pxp_csr_cfg_p_mac_test_in_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_cfg_p_mac_test_in_t::~cap_pxp_csr_cfg_p_mac_test_in_t() { }

cap_pxp_csr_cfg_p_sram_bist_t::cap_pxp_csr_cfg_p_sram_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_cfg_p_sram_bist_t::~cap_pxp_csr_cfg_p_sram_bist_t() { }

cap_pxp_csr_cfg_p_ecc_disable_t::cap_pxp_csr_cfg_p_ecc_disable_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_cfg_p_ecc_disable_t::~cap_pxp_csr_cfg_p_ecc_disable_t() { }

cap_pxp_csr_sta_p_port_lanes_7_0_t::cap_pxp_csr_sta_p_port_lanes_7_0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_port_lanes_7_0_t::~cap_pxp_csr_sta_p_port_lanes_7_0_t() { }

cap_pxp_csr_sta_p_port_mac_t::cap_pxp_csr_sta_p_port_mac_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_sta_p_port_mac_t::~cap_pxp_csr_sta_p_port_mac_t() { }

cap_pxp_csr_cfg_p_mac_k_eq_t::cap_pxp_csr_cfg_p_mac_k_eq_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pxp_csr_cfg_p_mac_k_eq_t::~cap_pxp_csr_cfg_p_mac_k_eq_t() { }

cap_pxp_csr_t::cap_pxp_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(256);
        set_attributes(0,get_name(), 0);
        }
cap_pxp_csr_t::~cap_pxp_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rxbfr_overflow_enable: 0x" << int_var__rxbfr_overflow_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".replay_bfr_overflow_enable: 0x" << int_var__replay_bfr_overflow_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxtlp_err_enable: 0x" << int_var__rxtlp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_0_uncorrectable_enable: 0x" << int_var__txbuf_0_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_1_uncorrectable_enable: 0x" << int_var__txbuf_1_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_2_uncorrectable_enable: 0x" << int_var__txbuf_2_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_3_uncorrectable_enable: 0x" << int_var__txbuf_3_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_0_correctable_enable: 0x" << int_var__txbuf_0_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_1_correctable_enable: 0x" << int_var__txbuf_1_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_2_correctable_enable: 0x" << int_var__txbuf_2_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_3_correctable_enable: 0x" << int_var__txbuf_3_correctable_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rxbfr_overflow_interrupt: 0x" << int_var__rxbfr_overflow_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".replay_bfr_overflow_interrupt: 0x" << int_var__replay_bfr_overflow_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxtlp_err_interrupt: 0x" << int_var__rxtlp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_0_uncorrectable_interrupt: 0x" << int_var__txbuf_0_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_1_uncorrectable_interrupt: 0x" << int_var__txbuf_1_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_2_uncorrectable_interrupt: 0x" << int_var__txbuf_2_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_3_uncorrectable_interrupt: 0x" << int_var__txbuf_3_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_0_correctable_interrupt: 0x" << int_var__txbuf_0_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_1_correctable_interrupt: 0x" << int_var__txbuf_1_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_2_correctable_interrupt: 0x" << int_var__txbuf_2_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_3_correctable_interrupt: 0x" << int_var__txbuf_3_correctable_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_p_ecc_interrupt: 0x" << int_var__int_p_ecc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_p_ecc_enable: 0x" << int_var__int_p_ecc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_deskew_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_fcpe_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".event: 0x" << int_var__event << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_ecc_txbuf_3_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_ecc_txbuf_2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_ecc_txbuf_1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_ecc_txbuf_0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_sram_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".done_pass: 0x" << int_var__done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_fail: 0x" << int_var__done_fail << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_cfg_p_mac_test_in_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bit_23_22: 0x" << int_var__bit_23_22 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_cfg_p_sram_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".run: 0x" << int_var__run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_cfg_p_ecc_disable_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_cor: 0x" << int_var__txbuf_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txbuf_det: 0x" << int_var__txbuf_det << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_port_lanes_7_0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".detected: 0x" << int_var__detected << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".active: 0x" << int_var__active << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_sta_p_port_mac_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pl_ltssm: 0x" << int_var__pl_ltssm << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pl_equ_phase: 0x" << int_var__pl_equ_phase << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rate: 0x" << int_var__rate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxbfr_fill_level: 0x" << int_var__rxbfr_fill_level << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxl0s_substate: 0x" << int_var__rxl0s_substate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txl0s_substate: 0x" << int_var__txl0s_substate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lanes_reversed: 0x" << int_var__lanes_reversed << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_cfg_p_mac_k_eq_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".k_finetune_max_8gt: 0x" << int_var__k_finetune_max_8gt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_preset_to_use_8gt: 0x" << int_var__k_preset_to_use_8gt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_finetune_max_16gt: 0x" << int_var__k_finetune_max_16gt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_preset_to_use_16gt: 0x" << int_var__k_preset_to_use_16gt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_finetune_err: 0x" << int_var__k_finetune_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_phyparam_query: 0x" << int_var__k_phyparam_query << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".k_query_timeout: 0x" << int_var__k_query_timeout << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pxp_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    cfg_p_mac_k_eq.show();
    sta_p_port_mac.show();
    sta_p_port_lanes_7_0.show();
    cfg_p_ecc_disable.show();
    cfg_p_sram_bist.show();
    cfg_p_mac_test_in.show();
    sta_p_sram_bist.show();
    sta_p_ecc_txbuf_0.show();
    sta_p_ecc_txbuf_1.show();
    sta_p_ecc_txbuf_2.show();
    sta_p_ecc_txbuf_3.show();
    sat_p_port_cnt_rx_bad_tlp.show();
    sat_p_port_cnt_rx_bad_dllp.show();
    sat_p_port_cnt_rx_nak_received.show();
    sat_p_port_cnt_tx_nak_sent.show();
    sat_p_port_cnt_rx_nullified.show();
    sat_p_port_cnt_fcpe.show();
    sat_p_port_cnt_fc_timeout.show();
    sat_p_port_cnt_txbuf_ecc_err.show();
    sat_p_port_cnt_replay_num_err.show();
    sat_p_port_cnt_replay_timer_err.show();
    sat_p_port_cnt_core_initiated_recovery.show();
    sat_p_port_cnt_ltssm_state_changed.show();
    sat_p_port_cnt_8b10b_128b130b_skp_os_err.show();
    sat_p_port_cnt_deskew_err.show();
    sat_p_port_cnt_phystatus_err.show();
    sat_p_port_cnt_rxbfr_overflow.show();
    csr_intr.show();
    int_groups.show();
    int_p_ecc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_pxp_csr_int_p_ecc_int_enable_clear_t::get_width() const {
    return cap_pxp_csr_int_p_ecc_int_enable_clear_t::s_get_width();

}

int cap_pxp_csr_intreg_t::get_width() const {
    return cap_pxp_csr_intreg_t::s_get_width();

}

int cap_pxp_csr_intgrp_t::get_width() const {
    return cap_pxp_csr_intgrp_t::s_get_width();

}

int cap_pxp_csr_intreg_status_t::get_width() const {
    return cap_pxp_csr_intreg_status_t::s_get_width();

}

int cap_pxp_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_pxp_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_pxp_csr_intgrp_status_t::get_width() const {
    return cap_pxp_csr_intgrp_status_t::s_get_width();

}

int cap_pxp_csr_csr_intr_t::get_width() const {
    return cap_pxp_csr_csr_intr_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_deskew_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_deskew_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_fcpe_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_fcpe_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::s_get_width();

}

int cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::get_width() const {
    return cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::s_get_width();

}

int cap_pxp_csr_sta_p_ecc_txbuf_3_t::get_width() const {
    return cap_pxp_csr_sta_p_ecc_txbuf_3_t::s_get_width();

}

int cap_pxp_csr_sta_p_ecc_txbuf_2_t::get_width() const {
    return cap_pxp_csr_sta_p_ecc_txbuf_2_t::s_get_width();

}

int cap_pxp_csr_sta_p_ecc_txbuf_1_t::get_width() const {
    return cap_pxp_csr_sta_p_ecc_txbuf_1_t::s_get_width();

}

int cap_pxp_csr_sta_p_ecc_txbuf_0_t::get_width() const {
    return cap_pxp_csr_sta_p_ecc_txbuf_0_t::s_get_width();

}

int cap_pxp_csr_sta_p_sram_bist_t::get_width() const {
    return cap_pxp_csr_sta_p_sram_bist_t::s_get_width();

}

int cap_pxp_csr_cfg_p_mac_test_in_t::get_width() const {
    return cap_pxp_csr_cfg_p_mac_test_in_t::s_get_width();

}

int cap_pxp_csr_cfg_p_sram_bist_t::get_width() const {
    return cap_pxp_csr_cfg_p_sram_bist_t::s_get_width();

}

int cap_pxp_csr_cfg_p_ecc_disable_t::get_width() const {
    return cap_pxp_csr_cfg_p_ecc_disable_t::s_get_width();

}

int cap_pxp_csr_sta_p_port_lanes_7_0_t::get_width() const {
    return cap_pxp_csr_sta_p_port_lanes_7_0_t::s_get_width();

}

int cap_pxp_csr_sta_p_port_mac_t::get_width() const {
    return cap_pxp_csr_sta_p_port_mac_t::s_get_width();

}

int cap_pxp_csr_cfg_p_mac_k_eq_t::get_width() const {
    return cap_pxp_csr_cfg_p_mac_k_eq_t::s_get_width();

}

int cap_pxp_csr_t::get_width() const {
    return cap_pxp_csr_t::s_get_width();

}

int cap_pxp_csr_int_p_ecc_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // rxbfr_overflow_enable
    _count += 1; // replay_bfr_overflow_enable
    _count += 1; // rxtlp_err_enable
    _count += 1; // txbuf_0_uncorrectable_enable
    _count += 1; // txbuf_1_uncorrectable_enable
    _count += 1; // txbuf_2_uncorrectable_enable
    _count += 1; // txbuf_3_uncorrectable_enable
    _count += 1; // txbuf_0_correctable_enable
    _count += 1; // txbuf_1_correctable_enable
    _count += 1; // txbuf_2_correctable_enable
    _count += 1; // txbuf_3_correctable_enable
    return _count;
}

int cap_pxp_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // rxbfr_overflow_interrupt
    _count += 1; // replay_bfr_overflow_interrupt
    _count += 1; // rxtlp_err_interrupt
    _count += 1; // txbuf_0_uncorrectable_interrupt
    _count += 1; // txbuf_1_uncorrectable_interrupt
    _count += 1; // txbuf_2_uncorrectable_interrupt
    _count += 1; // txbuf_3_uncorrectable_interrupt
    _count += 1; // txbuf_0_correctable_interrupt
    _count += 1; // txbuf_1_correctable_interrupt
    _count += 1; // txbuf_2_correctable_interrupt
    _count += 1; // txbuf_3_correctable_interrupt
    return _count;
}

int cap_pxp_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_pxp_csr_intreg_t::s_get_width(); // intreg
    _count += cap_pxp_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_pxp_csr_int_p_ecc_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_pxp_csr_int_p_ecc_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_pxp_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_p_ecc_interrupt
    return _count;
}

int cap_pxp_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_p_ecc_enable
    return _count;
}

int cap_pxp_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_pxp_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_pxp_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_pxp_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_pxp_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_deskew_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_fcpe_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::s_get_width() {
    int _count = 0;

    _count += 8; // event
    return _count;
}

int cap_pxp_csr_sta_p_ecc_txbuf_3_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 8; // addr
    return _count;
}

int cap_pxp_csr_sta_p_ecc_txbuf_2_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 8; // addr
    return _count;
}

int cap_pxp_csr_sta_p_ecc_txbuf_1_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 8; // addr
    return _count;
}

int cap_pxp_csr_sta_p_ecc_txbuf_0_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 8; // addr
    return _count;
}

int cap_pxp_csr_sta_p_sram_bist_t::s_get_width() {
    int _count = 0;

    _count += 8; // done_pass
    _count += 8; // done_fail
    return _count;
}

int cap_pxp_csr_cfg_p_mac_test_in_t::s_get_width() {
    int _count = 0;

    _count += 2; // bit_23_22
    return _count;
}

int cap_pxp_csr_cfg_p_sram_bist_t::s_get_width() {
    int _count = 0;

    _count += 8; // run
    return _count;
}

int cap_pxp_csr_cfg_p_ecc_disable_t::s_get_width() {
    int _count = 0;

    _count += 1; // txbuf_cor
    _count += 1; // txbuf_det
    return _count;
}

int cap_pxp_csr_sta_p_port_lanes_7_0_t::s_get_width() {
    int _count = 0;

    _count += 8; // detected
    _count += 8; // active
    return _count;
}

int cap_pxp_csr_sta_p_port_mac_t::s_get_width() {
    int _count = 0;

    _count += 5; // pl_ltssm
    _count += 2; // pl_equ_phase
    _count += 2; // rate
    _count += 5; // rxbfr_fill_level
    _count += 2; // rxl0s_substate
    _count += 2; // txl0s_substate
    _count += 1; // lanes_reversed
    return _count;
}

int cap_pxp_csr_cfg_p_mac_k_eq_t::s_get_width() {
    int _count = 0;

    _count += 6; // k_finetune_max_8gt
    _count += 11; // k_preset_to_use_8gt
    _count += 6; // k_finetune_max_16gt
    _count += 11; // k_preset_to_use_16gt
    _count += 2; // k_finetune_err
    _count += 1; // k_phyparam_query
    _count += 1; // k_query_timeout
    return _count;
}

int cap_pxp_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pxp_csr_cfg_p_mac_k_eq_t::s_get_width(); // cfg_p_mac_k_eq
    _count += cap_pxp_csr_sta_p_port_mac_t::s_get_width(); // sta_p_port_mac
    _count += cap_pxp_csr_sta_p_port_lanes_7_0_t::s_get_width(); // sta_p_port_lanes_7_0
    _count += cap_pxp_csr_cfg_p_ecc_disable_t::s_get_width(); // cfg_p_ecc_disable
    _count += cap_pxp_csr_cfg_p_sram_bist_t::s_get_width(); // cfg_p_sram_bist
    _count += cap_pxp_csr_cfg_p_mac_test_in_t::s_get_width(); // cfg_p_mac_test_in
    _count += cap_pxp_csr_sta_p_sram_bist_t::s_get_width(); // sta_p_sram_bist
    _count += cap_pxp_csr_sta_p_ecc_txbuf_0_t::s_get_width(); // sta_p_ecc_txbuf_0
    _count += cap_pxp_csr_sta_p_ecc_txbuf_1_t::s_get_width(); // sta_p_ecc_txbuf_1
    _count += cap_pxp_csr_sta_p_ecc_txbuf_2_t::s_get_width(); // sta_p_ecc_txbuf_2
    _count += cap_pxp_csr_sta_p_ecc_txbuf_3_t::s_get_width(); // sta_p_ecc_txbuf_3
    _count += cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::s_get_width(); // sat_p_port_cnt_rx_bad_tlp
    _count += cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::s_get_width(); // sat_p_port_cnt_rx_bad_dllp
    _count += cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::s_get_width(); // sat_p_port_cnt_rx_nak_received
    _count += cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::s_get_width(); // sat_p_port_cnt_tx_nak_sent
    _count += cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::s_get_width(); // sat_p_port_cnt_rx_nullified
    _count += cap_pxp_csr_sat_p_port_cnt_fcpe_t::s_get_width(); // sat_p_port_cnt_fcpe
    _count += cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::s_get_width(); // sat_p_port_cnt_fc_timeout
    _count += cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::s_get_width(); // sat_p_port_cnt_txbuf_ecc_err
    _count += cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::s_get_width(); // sat_p_port_cnt_replay_num_err
    _count += cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::s_get_width(); // sat_p_port_cnt_replay_timer_err
    _count += cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::s_get_width(); // sat_p_port_cnt_core_initiated_recovery
    _count += cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::s_get_width(); // sat_p_port_cnt_ltssm_state_changed
    _count += cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::s_get_width(); // sat_p_port_cnt_8b10b_128b130b_skp_os_err
    _count += cap_pxp_csr_sat_p_port_cnt_deskew_err_t::s_get_width(); // sat_p_port_cnt_deskew_err
    _count += cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::s_get_width(); // sat_p_port_cnt_phystatus_err
    _count += cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::s_get_width(); // sat_p_port_cnt_rxbfr_overflow
    _count += cap_pxp_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_pxp_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_pxp_csr_intgrp_t::s_get_width(); // int_p_ecc
    return _count;
}

void cap_pxp_csr_int_p_ecc_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__rxbfr_overflow_enable = _val.convert_to< rxbfr_overflow_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__replay_bfr_overflow_enable = _val.convert_to< replay_bfr_overflow_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rxtlp_err_enable = _val.convert_to< rxtlp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_0_uncorrectable_enable = _val.convert_to< txbuf_0_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_1_uncorrectable_enable = _val.convert_to< txbuf_1_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_2_uncorrectable_enable = _val.convert_to< txbuf_2_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_3_uncorrectable_enable = _val.convert_to< txbuf_3_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_0_correctable_enable = _val.convert_to< txbuf_0_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_1_correctable_enable = _val.convert_to< txbuf_1_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_2_correctable_enable = _val.convert_to< txbuf_2_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_3_correctable_enable = _val.convert_to< txbuf_3_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__rxbfr_overflow_interrupt = _val.convert_to< rxbfr_overflow_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__replay_bfr_overflow_interrupt = _val.convert_to< replay_bfr_overflow_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rxtlp_err_interrupt = _val.convert_to< rxtlp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_0_uncorrectable_interrupt = _val.convert_to< txbuf_0_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_1_uncorrectable_interrupt = _val.convert_to< txbuf_1_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_2_uncorrectable_interrupt = _val.convert_to< txbuf_2_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_3_uncorrectable_interrupt = _val.convert_to< txbuf_3_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_0_correctable_interrupt = _val.convert_to< txbuf_0_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_1_correctable_interrupt = _val.convert_to< txbuf_1_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_2_correctable_interrupt = _val.convert_to< txbuf_2_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_3_correctable_interrupt = _val.convert_to< txbuf_3_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_intgrp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_test_set.all( _val);
    _val = _val >> int_test_set.get_width(); 
    int_enable_set.all( _val);
    _val = _val >> int_enable_set.get_width(); 
    int_enable_clear.all( _val);
    _val = _val >> int_enable_clear.get_width(); 
}

void cap_pxp_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_p_ecc_interrupt = _val.convert_to< int_p_ecc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_p_ecc_enable = _val.convert_to< int_p_ecc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_pxp_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_deskew_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_fcpe_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__event = _val.convert_to< event_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_3_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_sram_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__done_pass = _val.convert_to< done_pass_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__done_fail = _val.convert_to< done_fail_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_cfg_p_mac_test_in_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bit_23_22 = _val.convert_to< bit_23_22_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_pxp_csr_cfg_p_sram_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__run = _val.convert_to< run_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_cfg_p_ecc_disable_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__txbuf_cor = _val.convert_to< txbuf_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txbuf_det = _val.convert_to< txbuf_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_sta_p_port_lanes_7_0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__detected = _val.convert_to< detected_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__active = _val.convert_to< active_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pxp_csr_sta_p_port_mac_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pl_ltssm = _val.convert_to< pl_ltssm_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__pl_equ_phase = _val.convert_to< pl_equ_phase_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__rate = _val.convert_to< rate_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__rxbfr_fill_level = _val.convert_to< rxbfr_fill_level_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__rxl0s_substate = _val.convert_to< rxl0s_substate_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__txl0s_substate = _val.convert_to< txl0s_substate_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__lanes_reversed = _val.convert_to< lanes_reversed_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_cfg_p_mac_k_eq_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__k_finetune_max_8gt = _val.convert_to< k_finetune_max_8gt_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__k_preset_to_use_8gt = _val.convert_to< k_preset_to_use_8gt_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__k_finetune_max_16gt = _val.convert_to< k_finetune_max_16gt_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__k_preset_to_use_16gt = _val.convert_to< k_preset_to_use_16gt_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__k_finetune_err = _val.convert_to< k_finetune_err_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__k_phyparam_query = _val.convert_to< k_phyparam_query_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__k_query_timeout = _val.convert_to< k_query_timeout_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pxp_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    cfg_p_mac_k_eq.all( _val);
    _val = _val >> cfg_p_mac_k_eq.get_width(); 
    sta_p_port_mac.all( _val);
    _val = _val >> sta_p_port_mac.get_width(); 
    sta_p_port_lanes_7_0.all( _val);
    _val = _val >> sta_p_port_lanes_7_0.get_width(); 
    cfg_p_ecc_disable.all( _val);
    _val = _val >> cfg_p_ecc_disable.get_width(); 
    cfg_p_sram_bist.all( _val);
    _val = _val >> cfg_p_sram_bist.get_width(); 
    cfg_p_mac_test_in.all( _val);
    _val = _val >> cfg_p_mac_test_in.get_width(); 
    sta_p_sram_bist.all( _val);
    _val = _val >> sta_p_sram_bist.get_width(); 
    sta_p_ecc_txbuf_0.all( _val);
    _val = _val >> sta_p_ecc_txbuf_0.get_width(); 
    sta_p_ecc_txbuf_1.all( _val);
    _val = _val >> sta_p_ecc_txbuf_1.get_width(); 
    sta_p_ecc_txbuf_2.all( _val);
    _val = _val >> sta_p_ecc_txbuf_2.get_width(); 
    sta_p_ecc_txbuf_3.all( _val);
    _val = _val >> sta_p_ecc_txbuf_3.get_width(); 
    sat_p_port_cnt_rx_bad_tlp.all( _val);
    _val = _val >> sat_p_port_cnt_rx_bad_tlp.get_width(); 
    sat_p_port_cnt_rx_bad_dllp.all( _val);
    _val = _val >> sat_p_port_cnt_rx_bad_dllp.get_width(); 
    sat_p_port_cnt_rx_nak_received.all( _val);
    _val = _val >> sat_p_port_cnt_rx_nak_received.get_width(); 
    sat_p_port_cnt_tx_nak_sent.all( _val);
    _val = _val >> sat_p_port_cnt_tx_nak_sent.get_width(); 
    sat_p_port_cnt_rx_nullified.all( _val);
    _val = _val >> sat_p_port_cnt_rx_nullified.get_width(); 
    sat_p_port_cnt_fcpe.all( _val);
    _val = _val >> sat_p_port_cnt_fcpe.get_width(); 
    sat_p_port_cnt_fc_timeout.all( _val);
    _val = _val >> sat_p_port_cnt_fc_timeout.get_width(); 
    sat_p_port_cnt_txbuf_ecc_err.all( _val);
    _val = _val >> sat_p_port_cnt_txbuf_ecc_err.get_width(); 
    sat_p_port_cnt_replay_num_err.all( _val);
    _val = _val >> sat_p_port_cnt_replay_num_err.get_width(); 
    sat_p_port_cnt_replay_timer_err.all( _val);
    _val = _val >> sat_p_port_cnt_replay_timer_err.get_width(); 
    sat_p_port_cnt_core_initiated_recovery.all( _val);
    _val = _val >> sat_p_port_cnt_core_initiated_recovery.get_width(); 
    sat_p_port_cnt_ltssm_state_changed.all( _val);
    _val = _val >> sat_p_port_cnt_ltssm_state_changed.get_width(); 
    sat_p_port_cnt_8b10b_128b130b_skp_os_err.all( _val);
    _val = _val >> sat_p_port_cnt_8b10b_128b130b_skp_os_err.get_width(); 
    sat_p_port_cnt_deskew_err.all( _val);
    _val = _val >> sat_p_port_cnt_deskew_err.get_width(); 
    sat_p_port_cnt_phystatus_err.all( _val);
    _val = _val >> sat_p_port_cnt_phystatus_err.get_width(); 
    sat_p_port_cnt_rxbfr_overflow.all( _val);
    _val = _val >> sat_p_port_cnt_rxbfr_overflow.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_p_ecc.all( _val);
    _val = _val >> int_p_ecc.get_width(); 
}

cpp_int cap_pxp_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_p_ecc.get_width(); ret_val = ret_val  | int_p_ecc.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sat_p_port_cnt_rxbfr_overflow.get_width(); ret_val = ret_val  | sat_p_port_cnt_rxbfr_overflow.all(); 
    ret_val = ret_val << sat_p_port_cnt_phystatus_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_phystatus_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_deskew_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_deskew_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_8b10b_128b130b_skp_os_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_8b10b_128b130b_skp_os_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_ltssm_state_changed.get_width(); ret_val = ret_val  | sat_p_port_cnt_ltssm_state_changed.all(); 
    ret_val = ret_val << sat_p_port_cnt_core_initiated_recovery.get_width(); ret_val = ret_val  | sat_p_port_cnt_core_initiated_recovery.all(); 
    ret_val = ret_val << sat_p_port_cnt_replay_timer_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_replay_timer_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_replay_num_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_replay_num_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_txbuf_ecc_err.get_width(); ret_val = ret_val  | sat_p_port_cnt_txbuf_ecc_err.all(); 
    ret_val = ret_val << sat_p_port_cnt_fc_timeout.get_width(); ret_val = ret_val  | sat_p_port_cnt_fc_timeout.all(); 
    ret_val = ret_val << sat_p_port_cnt_fcpe.get_width(); ret_val = ret_val  | sat_p_port_cnt_fcpe.all(); 
    ret_val = ret_val << sat_p_port_cnt_rx_nullified.get_width(); ret_val = ret_val  | sat_p_port_cnt_rx_nullified.all(); 
    ret_val = ret_val << sat_p_port_cnt_tx_nak_sent.get_width(); ret_val = ret_val  | sat_p_port_cnt_tx_nak_sent.all(); 
    ret_val = ret_val << sat_p_port_cnt_rx_nak_received.get_width(); ret_val = ret_val  | sat_p_port_cnt_rx_nak_received.all(); 
    ret_val = ret_val << sat_p_port_cnt_rx_bad_dllp.get_width(); ret_val = ret_val  | sat_p_port_cnt_rx_bad_dllp.all(); 
    ret_val = ret_val << sat_p_port_cnt_rx_bad_tlp.get_width(); ret_val = ret_val  | sat_p_port_cnt_rx_bad_tlp.all(); 
    ret_val = ret_val << sta_p_ecc_txbuf_3.get_width(); ret_val = ret_val  | sta_p_ecc_txbuf_3.all(); 
    ret_val = ret_val << sta_p_ecc_txbuf_2.get_width(); ret_val = ret_val  | sta_p_ecc_txbuf_2.all(); 
    ret_val = ret_val << sta_p_ecc_txbuf_1.get_width(); ret_val = ret_val  | sta_p_ecc_txbuf_1.all(); 
    ret_val = ret_val << sta_p_ecc_txbuf_0.get_width(); ret_val = ret_val  | sta_p_ecc_txbuf_0.all(); 
    ret_val = ret_val << sta_p_sram_bist.get_width(); ret_val = ret_val  | sta_p_sram_bist.all(); 
    ret_val = ret_val << cfg_p_mac_test_in.get_width(); ret_val = ret_val  | cfg_p_mac_test_in.all(); 
    ret_val = ret_val << cfg_p_sram_bist.get_width(); ret_val = ret_val  | cfg_p_sram_bist.all(); 
    ret_val = ret_val << cfg_p_ecc_disable.get_width(); ret_val = ret_val  | cfg_p_ecc_disable.all(); 
    ret_val = ret_val << sta_p_port_lanes_7_0.get_width(); ret_val = ret_val  | sta_p_port_lanes_7_0.all(); 
    ret_val = ret_val << sta_p_port_mac.get_width(); ret_val = ret_val  | sta_p_port_mac.all(); 
    ret_val = ret_val << cfg_p_mac_k_eq.get_width(); ret_val = ret_val  | cfg_p_mac_k_eq.all(); 
    return ret_val;
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::all() const {
    cpp_int ret_val;

    // k_query_timeout
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__k_query_timeout; 
    
    // k_phyparam_query
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__k_phyparam_query; 
    
    // k_finetune_err
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__k_finetune_err; 
    
    // k_preset_to_use_16gt
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__k_preset_to_use_16gt; 
    
    // k_finetune_max_16gt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__k_finetune_max_16gt; 
    
    // k_preset_to_use_8gt
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__k_preset_to_use_8gt; 
    
    // k_finetune_max_8gt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__k_finetune_max_8gt; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::all() const {
    cpp_int ret_val;

    // lanes_reversed
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lanes_reversed; 
    
    // txl0s_substate
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__txl0s_substate; 
    
    // rxl0s_substate
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__rxl0s_substate; 
    
    // rxbfr_fill_level
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__rxbfr_fill_level; 
    
    // rate
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__rate; 
    
    // pl_equ_phase
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__pl_equ_phase; 
    
    // pl_ltssm
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__pl_ltssm; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_port_lanes_7_0_t::all() const {
    cpp_int ret_val;

    // active
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__active; 
    
    // detected
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__detected; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_cfg_p_ecc_disable_t::all() const {
    cpp_int ret_val;

    // txbuf_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_det; 
    
    // txbuf_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_cor; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_cfg_p_sram_bist_t::all() const {
    cpp_int ret_val;

    // run
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__run; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_cfg_p_mac_test_in_t::all() const {
    cpp_int ret_val;

    // bit_23_22
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__bit_23_22; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_sram_bist_t::all() const {
    cpp_int ret_val;

    // done_fail
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__done_fail; 
    
    // done_pass
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__done_pass; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_0_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_1_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_2_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_3_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_fcpe_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_deskew_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::all() const {
    cpp_int ret_val;

    // event
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__event; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_pxp_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_p_ecc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_p_ecc_interrupt; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_p_ecc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_p_ecc_enable; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_pxp_csr_intreg_t::all() const {
    cpp_int ret_val;

    // txbuf_3_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_3_correctable_interrupt; 
    
    // txbuf_2_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_2_correctable_interrupt; 
    
    // txbuf_1_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_1_correctable_interrupt; 
    
    // txbuf_0_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_0_correctable_interrupt; 
    
    // txbuf_3_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_3_uncorrectable_interrupt; 
    
    // txbuf_2_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_2_uncorrectable_interrupt; 
    
    // txbuf_1_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_1_uncorrectable_interrupt; 
    
    // txbuf_0_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_0_uncorrectable_interrupt; 
    
    // rxtlp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxtlp_err_interrupt; 
    
    // replay_bfr_overflow_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__replay_bfr_overflow_interrupt; 
    
    // rxbfr_overflow_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxbfr_overflow_interrupt; 
    
    return ret_val;
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // txbuf_3_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_3_correctable_enable; 
    
    // txbuf_2_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_2_correctable_enable; 
    
    // txbuf_1_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_1_correctable_enable; 
    
    // txbuf_0_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_0_correctable_enable; 
    
    // txbuf_3_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_3_uncorrectable_enable; 
    
    // txbuf_2_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_2_uncorrectable_enable; 
    
    // txbuf_1_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_1_uncorrectable_enable; 
    
    // txbuf_0_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txbuf_0_uncorrectable_enable; 
    
    // rxtlp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxtlp_err_enable; 
    
    // replay_bfr_overflow_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__replay_bfr_overflow_enable; 
    
    // rxbfr_overflow_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxbfr_overflow_enable; 
    
    return ret_val;
}

void cap_pxp_csr_int_p_ecc_int_enable_clear_t::clear() {

    int_var__rxbfr_overflow_enable = 0; 
    
    int_var__replay_bfr_overflow_enable = 0; 
    
    int_var__rxtlp_err_enable = 0; 
    
    int_var__txbuf_0_uncorrectable_enable = 0; 
    
    int_var__txbuf_1_uncorrectable_enable = 0; 
    
    int_var__txbuf_2_uncorrectable_enable = 0; 
    
    int_var__txbuf_3_uncorrectable_enable = 0; 
    
    int_var__txbuf_0_correctable_enable = 0; 
    
    int_var__txbuf_1_correctable_enable = 0; 
    
    int_var__txbuf_2_correctable_enable = 0; 
    
    int_var__txbuf_3_correctable_enable = 0; 
    
}

void cap_pxp_csr_intreg_t::clear() {

    int_var__rxbfr_overflow_interrupt = 0; 
    
    int_var__replay_bfr_overflow_interrupt = 0; 
    
    int_var__rxtlp_err_interrupt = 0; 
    
    int_var__txbuf_0_uncorrectable_interrupt = 0; 
    
    int_var__txbuf_1_uncorrectable_interrupt = 0; 
    
    int_var__txbuf_2_uncorrectable_interrupt = 0; 
    
    int_var__txbuf_3_uncorrectable_interrupt = 0; 
    
    int_var__txbuf_0_correctable_interrupt = 0; 
    
    int_var__txbuf_1_correctable_interrupt = 0; 
    
    int_var__txbuf_2_correctable_interrupt = 0; 
    
    int_var__txbuf_3_correctable_interrupt = 0; 
    
}

void cap_pxp_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_pxp_csr_intreg_status_t::clear() {

    int_var__int_p_ecc_interrupt = 0; 
    
}

void cap_pxp_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_p_ecc_enable = 0; 
    
}

void cap_pxp_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_pxp_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_deskew_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_fcpe_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::clear() {

    int_var__event = 0; 
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_3_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_2_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_1_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_pxp_csr_sta_p_ecc_txbuf_0_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_pxp_csr_sta_p_sram_bist_t::clear() {

    int_var__done_pass = 0; 
    
    int_var__done_fail = 0; 
    
}

void cap_pxp_csr_cfg_p_mac_test_in_t::clear() {

    int_var__bit_23_22 = 0; 
    
}

void cap_pxp_csr_cfg_p_sram_bist_t::clear() {

    int_var__run = 0; 
    
}

void cap_pxp_csr_cfg_p_ecc_disable_t::clear() {

    int_var__txbuf_cor = 0; 
    
    int_var__txbuf_det = 0; 
    
}

void cap_pxp_csr_sta_p_port_lanes_7_0_t::clear() {

    int_var__detected = 0; 
    
    int_var__active = 0; 
    
}

void cap_pxp_csr_sta_p_port_mac_t::clear() {

    int_var__pl_ltssm = 0; 
    
    int_var__pl_equ_phase = 0; 
    
    int_var__rate = 0; 
    
    int_var__rxbfr_fill_level = 0; 
    
    int_var__rxl0s_substate = 0; 
    
    int_var__txl0s_substate = 0; 
    
    int_var__lanes_reversed = 0; 
    
}

void cap_pxp_csr_cfg_p_mac_k_eq_t::clear() {

    int_var__k_finetune_max_8gt = 0; 
    
    int_var__k_preset_to_use_8gt = 0; 
    
    int_var__k_finetune_max_16gt = 0; 
    
    int_var__k_preset_to_use_16gt = 0; 
    
    int_var__k_finetune_err = 0; 
    
    int_var__k_phyparam_query = 0; 
    
    int_var__k_query_timeout = 0; 
    
}

void cap_pxp_csr_t::clear() {

    cfg_p_mac_k_eq.clear();
    sta_p_port_mac.clear();
    sta_p_port_lanes_7_0.clear();
    cfg_p_ecc_disable.clear();
    cfg_p_sram_bist.clear();
    cfg_p_mac_test_in.clear();
    sta_p_sram_bist.clear();
    sta_p_ecc_txbuf_0.clear();
    sta_p_ecc_txbuf_1.clear();
    sta_p_ecc_txbuf_2.clear();
    sta_p_ecc_txbuf_3.clear();
    sat_p_port_cnt_rx_bad_tlp.clear();
    sat_p_port_cnt_rx_bad_dllp.clear();
    sat_p_port_cnt_rx_nak_received.clear();
    sat_p_port_cnt_tx_nak_sent.clear();
    sat_p_port_cnt_rx_nullified.clear();
    sat_p_port_cnt_fcpe.clear();
    sat_p_port_cnt_fc_timeout.clear();
    sat_p_port_cnt_txbuf_ecc_err.clear();
    sat_p_port_cnt_replay_num_err.clear();
    sat_p_port_cnt_replay_timer_err.clear();
    sat_p_port_cnt_core_initiated_recovery.clear();
    sat_p_port_cnt_ltssm_state_changed.clear();
    sat_p_port_cnt_8b10b_128b130b_skp_os_err.clear();
    sat_p_port_cnt_deskew_err.clear();
    sat_p_port_cnt_phystatus_err.clear();
    sat_p_port_cnt_rxbfr_overflow.clear();
    csr_intr.clear();
    int_groups.clear();
    int_p_ecc.clear();
}

void cap_pxp_csr_int_p_ecc_int_enable_clear_t::init() {

}

void cap_pxp_csr_intreg_t::init() {

}

void cap_pxp_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_pxp_csr_intreg_status_t::init() {

}

void cap_pxp_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_pxp_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_pxp_csr_csr_intr_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_deskew_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_fcpe_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::init() {

}

void cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::init() {

}

void cap_pxp_csr_sta_p_ecc_txbuf_3_t::init() {

}

void cap_pxp_csr_sta_p_ecc_txbuf_2_t::init() {

}

void cap_pxp_csr_sta_p_ecc_txbuf_1_t::init() {

}

void cap_pxp_csr_sta_p_ecc_txbuf_0_t::init() {

}

void cap_pxp_csr_sta_p_sram_bist_t::init() {

}

void cap_pxp_csr_cfg_p_mac_test_in_t::init() {

}

void cap_pxp_csr_cfg_p_sram_bist_t::init() {

}

void cap_pxp_csr_cfg_p_ecc_disable_t::init() {

}

void cap_pxp_csr_sta_p_port_lanes_7_0_t::init() {

}

void cap_pxp_csr_sta_p_port_mac_t::init() {

}

void cap_pxp_csr_cfg_p_mac_k_eq_t::init() {

    set_reset_val(cpp_int("0x14080a0405"));
    all(get_reset_val());
}

void cap_pxp_csr_t::init() {

    cfg_p_mac_k_eq.set_attributes(this,"cfg_p_mac_k_eq", 0x0 );
    sta_p_port_mac.set_attributes(this,"sta_p_port_mac", 0x8 );
    sta_p_port_lanes_7_0.set_attributes(this,"sta_p_port_lanes_7_0", 0xc );
    cfg_p_ecc_disable.set_attributes(this,"cfg_p_ecc_disable", 0x10 );
    cfg_p_sram_bist.set_attributes(this,"cfg_p_sram_bist", 0x14 );
    cfg_p_mac_test_in.set_attributes(this,"cfg_p_mac_test_in", 0x18 );
    sta_p_sram_bist.set_attributes(this,"sta_p_sram_bist", 0x1c );
    sta_p_ecc_txbuf_0.set_attributes(this,"sta_p_ecc_txbuf_0", 0x20 );
    sta_p_ecc_txbuf_1.set_attributes(this,"sta_p_ecc_txbuf_1", 0x24 );
    sta_p_ecc_txbuf_2.set_attributes(this,"sta_p_ecc_txbuf_2", 0x28 );
    sta_p_ecc_txbuf_3.set_attributes(this,"sta_p_ecc_txbuf_3", 0x2c );
    sat_p_port_cnt_rx_bad_tlp.set_attributes(this,"sat_p_port_cnt_rx_bad_tlp", 0x30 );
    sat_p_port_cnt_rx_bad_dllp.set_attributes(this,"sat_p_port_cnt_rx_bad_dllp", 0x34 );
    sat_p_port_cnt_rx_nak_received.set_attributes(this,"sat_p_port_cnt_rx_nak_received", 0x38 );
    sat_p_port_cnt_tx_nak_sent.set_attributes(this,"sat_p_port_cnt_tx_nak_sent", 0x3c );
    sat_p_port_cnt_rx_nullified.set_attributes(this,"sat_p_port_cnt_rx_nullified", 0x40 );
    sat_p_port_cnt_fcpe.set_attributes(this,"sat_p_port_cnt_fcpe", 0x44 );
    sat_p_port_cnt_fc_timeout.set_attributes(this,"sat_p_port_cnt_fc_timeout", 0x48 );
    sat_p_port_cnt_txbuf_ecc_err.set_attributes(this,"sat_p_port_cnt_txbuf_ecc_err", 0x4c );
    sat_p_port_cnt_replay_num_err.set_attributes(this,"sat_p_port_cnt_replay_num_err", 0x50 );
    sat_p_port_cnt_replay_timer_err.set_attributes(this,"sat_p_port_cnt_replay_timer_err", 0x54 );
    sat_p_port_cnt_core_initiated_recovery.set_attributes(this,"sat_p_port_cnt_core_initiated_recovery", 0x58 );
    sat_p_port_cnt_ltssm_state_changed.set_attributes(this,"sat_p_port_cnt_ltssm_state_changed", 0x5c );
    sat_p_port_cnt_8b10b_128b130b_skp_os_err.set_attributes(this,"sat_p_port_cnt_8b10b_128b130b_skp_os_err", 0x60 );
    sat_p_port_cnt_deskew_err.set_attributes(this,"sat_p_port_cnt_deskew_err", 0x64 );
    sat_p_port_cnt_phystatus_err.set_attributes(this,"sat_p_port_cnt_phystatus_err", 0x68 );
    sat_p_port_cnt_rxbfr_overflow.set_attributes(this,"sat_p_port_cnt_rxbfr_overflow", 0x6c );
    csr_intr.set_attributes(this,"csr_intr", 0x70 );
    int_groups.set_attributes(this,"int_groups", 0x80 );
    int_p_ecc.set_attributes(this,"int_p_ecc", 0x90 );
}

void cap_pxp_csr_int_p_ecc_int_enable_clear_t::rxbfr_overflow_enable(const cpp_int & _val) { 
    // rxbfr_overflow_enable
    int_var__rxbfr_overflow_enable = _val.convert_to< rxbfr_overflow_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::rxbfr_overflow_enable() const {
    return int_var__rxbfr_overflow_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::replay_bfr_overflow_enable(const cpp_int & _val) { 
    // replay_bfr_overflow_enable
    int_var__replay_bfr_overflow_enable = _val.convert_to< replay_bfr_overflow_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::replay_bfr_overflow_enable() const {
    return int_var__replay_bfr_overflow_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::rxtlp_err_enable(const cpp_int & _val) { 
    // rxtlp_err_enable
    int_var__rxtlp_err_enable = _val.convert_to< rxtlp_err_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::rxtlp_err_enable() const {
    return int_var__rxtlp_err_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_0_uncorrectable_enable(const cpp_int & _val) { 
    // txbuf_0_uncorrectable_enable
    int_var__txbuf_0_uncorrectable_enable = _val.convert_to< txbuf_0_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_0_uncorrectable_enable() const {
    return int_var__txbuf_0_uncorrectable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_1_uncorrectable_enable(const cpp_int & _val) { 
    // txbuf_1_uncorrectable_enable
    int_var__txbuf_1_uncorrectable_enable = _val.convert_to< txbuf_1_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_1_uncorrectable_enable() const {
    return int_var__txbuf_1_uncorrectable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_2_uncorrectable_enable(const cpp_int & _val) { 
    // txbuf_2_uncorrectable_enable
    int_var__txbuf_2_uncorrectable_enable = _val.convert_to< txbuf_2_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_2_uncorrectable_enable() const {
    return int_var__txbuf_2_uncorrectable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_3_uncorrectable_enable(const cpp_int & _val) { 
    // txbuf_3_uncorrectable_enable
    int_var__txbuf_3_uncorrectable_enable = _val.convert_to< txbuf_3_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_3_uncorrectable_enable() const {
    return int_var__txbuf_3_uncorrectable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_0_correctable_enable(const cpp_int & _val) { 
    // txbuf_0_correctable_enable
    int_var__txbuf_0_correctable_enable = _val.convert_to< txbuf_0_correctable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_0_correctable_enable() const {
    return int_var__txbuf_0_correctable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_1_correctable_enable(const cpp_int & _val) { 
    // txbuf_1_correctable_enable
    int_var__txbuf_1_correctable_enable = _val.convert_to< txbuf_1_correctable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_1_correctable_enable() const {
    return int_var__txbuf_1_correctable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_2_correctable_enable(const cpp_int & _val) { 
    // txbuf_2_correctable_enable
    int_var__txbuf_2_correctable_enable = _val.convert_to< txbuf_2_correctable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_2_correctable_enable() const {
    return int_var__txbuf_2_correctable_enable;
}
    
void cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_3_correctable_enable(const cpp_int & _val) { 
    // txbuf_3_correctable_enable
    int_var__txbuf_3_correctable_enable = _val.convert_to< txbuf_3_correctable_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_p_ecc_int_enable_clear_t::txbuf_3_correctable_enable() const {
    return int_var__txbuf_3_correctable_enable;
}
    
void cap_pxp_csr_intreg_t::rxbfr_overflow_interrupt(const cpp_int & _val) { 
    // rxbfr_overflow_interrupt
    int_var__rxbfr_overflow_interrupt = _val.convert_to< rxbfr_overflow_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::rxbfr_overflow_interrupt() const {
    return int_var__rxbfr_overflow_interrupt;
}
    
void cap_pxp_csr_intreg_t::replay_bfr_overflow_interrupt(const cpp_int & _val) { 
    // replay_bfr_overflow_interrupt
    int_var__replay_bfr_overflow_interrupt = _val.convert_to< replay_bfr_overflow_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::replay_bfr_overflow_interrupt() const {
    return int_var__replay_bfr_overflow_interrupt;
}
    
void cap_pxp_csr_intreg_t::rxtlp_err_interrupt(const cpp_int & _val) { 
    // rxtlp_err_interrupt
    int_var__rxtlp_err_interrupt = _val.convert_to< rxtlp_err_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::rxtlp_err_interrupt() const {
    return int_var__rxtlp_err_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_0_uncorrectable_interrupt(const cpp_int & _val) { 
    // txbuf_0_uncorrectable_interrupt
    int_var__txbuf_0_uncorrectable_interrupt = _val.convert_to< txbuf_0_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_0_uncorrectable_interrupt() const {
    return int_var__txbuf_0_uncorrectable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_1_uncorrectable_interrupt(const cpp_int & _val) { 
    // txbuf_1_uncorrectable_interrupt
    int_var__txbuf_1_uncorrectable_interrupt = _val.convert_to< txbuf_1_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_1_uncorrectable_interrupt() const {
    return int_var__txbuf_1_uncorrectable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_2_uncorrectable_interrupt(const cpp_int & _val) { 
    // txbuf_2_uncorrectable_interrupt
    int_var__txbuf_2_uncorrectable_interrupt = _val.convert_to< txbuf_2_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_2_uncorrectable_interrupt() const {
    return int_var__txbuf_2_uncorrectable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_3_uncorrectable_interrupt(const cpp_int & _val) { 
    // txbuf_3_uncorrectable_interrupt
    int_var__txbuf_3_uncorrectable_interrupt = _val.convert_to< txbuf_3_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_3_uncorrectable_interrupt() const {
    return int_var__txbuf_3_uncorrectable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_0_correctable_interrupt(const cpp_int & _val) { 
    // txbuf_0_correctable_interrupt
    int_var__txbuf_0_correctable_interrupt = _val.convert_to< txbuf_0_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_0_correctable_interrupt() const {
    return int_var__txbuf_0_correctable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_1_correctable_interrupt(const cpp_int & _val) { 
    // txbuf_1_correctable_interrupt
    int_var__txbuf_1_correctable_interrupt = _val.convert_to< txbuf_1_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_1_correctable_interrupt() const {
    return int_var__txbuf_1_correctable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_2_correctable_interrupt(const cpp_int & _val) { 
    // txbuf_2_correctable_interrupt
    int_var__txbuf_2_correctable_interrupt = _val.convert_to< txbuf_2_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_2_correctable_interrupt() const {
    return int_var__txbuf_2_correctable_interrupt;
}
    
void cap_pxp_csr_intreg_t::txbuf_3_correctable_interrupt(const cpp_int & _val) { 
    // txbuf_3_correctable_interrupt
    int_var__txbuf_3_correctable_interrupt = _val.convert_to< txbuf_3_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_t::txbuf_3_correctable_interrupt() const {
    return int_var__txbuf_3_correctable_interrupt;
}
    
void cap_pxp_csr_intreg_status_t::int_p_ecc_interrupt(const cpp_int & _val) { 
    // int_p_ecc_interrupt
    int_var__int_p_ecc_interrupt = _val.convert_to< int_p_ecc_interrupt_cpp_int_t >();
}

cpp_int cap_pxp_csr_intreg_status_t::int_p_ecc_interrupt() const {
    return int_var__int_p_ecc_interrupt;
}
    
void cap_pxp_csr_int_groups_int_enable_rw_reg_t::int_p_ecc_enable(const cpp_int & _val) { 
    // int_p_ecc_enable
    int_var__int_p_ecc_enable = _val.convert_to< int_p_ecc_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_int_groups_int_enable_rw_reg_t::int_p_ecc_enable() const {
    return int_var__int_p_ecc_enable;
}
    
void cap_pxp_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_pxp_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_pxp_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_pxp_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_deskew_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_deskew_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_fcpe_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_fcpe_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::event(const cpp_int & _val) { 
    // event
    int_var__event = _val.convert_to< event_cpp_int_t >();
}

cpp_int cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::event() const {
    return int_var__event;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_3_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_3_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_3_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_3_t::correctable() const {
    return int_var__correctable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_3_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_3_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_3_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_3_t::addr() const {
    return int_var__addr;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_2_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_2_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_2_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_2_t::correctable() const {
    return int_var__correctable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_2_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_2_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_2_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_2_t::addr() const {
    return int_var__addr;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_1_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_1_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_1_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_1_t::correctable() const {
    return int_var__correctable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_1_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_1_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_1_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_1_t::addr() const {
    return int_var__addr;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_0_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_0_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_0_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_0_t::correctable() const {
    return int_var__correctable;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_0_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_0_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_pxp_csr_sta_p_ecc_txbuf_0_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_ecc_txbuf_0_t::addr() const {
    return int_var__addr;
}
    
void cap_pxp_csr_sta_p_sram_bist_t::done_pass(const cpp_int & _val) { 
    // done_pass
    int_var__done_pass = _val.convert_to< done_pass_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_sram_bist_t::done_pass() const {
    return int_var__done_pass;
}
    
void cap_pxp_csr_sta_p_sram_bist_t::done_fail(const cpp_int & _val) { 
    // done_fail
    int_var__done_fail = _val.convert_to< done_fail_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_sram_bist_t::done_fail() const {
    return int_var__done_fail;
}
    
void cap_pxp_csr_cfg_p_mac_test_in_t::bit_23_22(const cpp_int & _val) { 
    // bit_23_22
    int_var__bit_23_22 = _val.convert_to< bit_23_22_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_test_in_t::bit_23_22() const {
    return int_var__bit_23_22;
}
    
void cap_pxp_csr_cfg_p_sram_bist_t::run(const cpp_int & _val) { 
    // run
    int_var__run = _val.convert_to< run_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_sram_bist_t::run() const {
    return int_var__run;
}
    
void cap_pxp_csr_cfg_p_ecc_disable_t::txbuf_cor(const cpp_int & _val) { 
    // txbuf_cor
    int_var__txbuf_cor = _val.convert_to< txbuf_cor_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_ecc_disable_t::txbuf_cor() const {
    return int_var__txbuf_cor;
}
    
void cap_pxp_csr_cfg_p_ecc_disable_t::txbuf_det(const cpp_int & _val) { 
    // txbuf_det
    int_var__txbuf_det = _val.convert_to< txbuf_det_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_ecc_disable_t::txbuf_det() const {
    return int_var__txbuf_det;
}
    
void cap_pxp_csr_sta_p_port_lanes_7_0_t::detected(const cpp_int & _val) { 
    // detected
    int_var__detected = _val.convert_to< detected_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_lanes_7_0_t::detected() const {
    return int_var__detected;
}
    
void cap_pxp_csr_sta_p_port_lanes_7_0_t::active(const cpp_int & _val) { 
    // active
    int_var__active = _val.convert_to< active_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_lanes_7_0_t::active() const {
    return int_var__active;
}
    
void cap_pxp_csr_sta_p_port_mac_t::pl_ltssm(const cpp_int & _val) { 
    // pl_ltssm
    int_var__pl_ltssm = _val.convert_to< pl_ltssm_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::pl_ltssm() const {
    return int_var__pl_ltssm;
}
    
void cap_pxp_csr_sta_p_port_mac_t::pl_equ_phase(const cpp_int & _val) { 
    // pl_equ_phase
    int_var__pl_equ_phase = _val.convert_to< pl_equ_phase_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::pl_equ_phase() const {
    return int_var__pl_equ_phase;
}
    
void cap_pxp_csr_sta_p_port_mac_t::rate(const cpp_int & _val) { 
    // rate
    int_var__rate = _val.convert_to< rate_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::rate() const {
    return int_var__rate;
}
    
void cap_pxp_csr_sta_p_port_mac_t::rxbfr_fill_level(const cpp_int & _val) { 
    // rxbfr_fill_level
    int_var__rxbfr_fill_level = _val.convert_to< rxbfr_fill_level_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::rxbfr_fill_level() const {
    return int_var__rxbfr_fill_level;
}
    
void cap_pxp_csr_sta_p_port_mac_t::rxl0s_substate(const cpp_int & _val) { 
    // rxl0s_substate
    int_var__rxl0s_substate = _val.convert_to< rxl0s_substate_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::rxl0s_substate() const {
    return int_var__rxl0s_substate;
}
    
void cap_pxp_csr_sta_p_port_mac_t::txl0s_substate(const cpp_int & _val) { 
    // txl0s_substate
    int_var__txl0s_substate = _val.convert_to< txl0s_substate_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::txl0s_substate() const {
    return int_var__txl0s_substate;
}
    
void cap_pxp_csr_sta_p_port_mac_t::lanes_reversed(const cpp_int & _val) { 
    // lanes_reversed
    int_var__lanes_reversed = _val.convert_to< lanes_reversed_cpp_int_t >();
}

cpp_int cap_pxp_csr_sta_p_port_mac_t::lanes_reversed() const {
    return int_var__lanes_reversed;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_max_8gt(const cpp_int & _val) { 
    // k_finetune_max_8gt
    int_var__k_finetune_max_8gt = _val.convert_to< k_finetune_max_8gt_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_max_8gt() const {
    return int_var__k_finetune_max_8gt;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_preset_to_use_8gt(const cpp_int & _val) { 
    // k_preset_to_use_8gt
    int_var__k_preset_to_use_8gt = _val.convert_to< k_preset_to_use_8gt_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_preset_to_use_8gt() const {
    return int_var__k_preset_to_use_8gt;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_max_16gt(const cpp_int & _val) { 
    // k_finetune_max_16gt
    int_var__k_finetune_max_16gt = _val.convert_to< k_finetune_max_16gt_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_max_16gt() const {
    return int_var__k_finetune_max_16gt;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_preset_to_use_16gt(const cpp_int & _val) { 
    // k_preset_to_use_16gt
    int_var__k_preset_to_use_16gt = _val.convert_to< k_preset_to_use_16gt_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_preset_to_use_16gt() const {
    return int_var__k_preset_to_use_16gt;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_err(const cpp_int & _val) { 
    // k_finetune_err
    int_var__k_finetune_err = _val.convert_to< k_finetune_err_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_finetune_err() const {
    return int_var__k_finetune_err;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_phyparam_query(const cpp_int & _val) { 
    // k_phyparam_query
    int_var__k_phyparam_query = _val.convert_to< k_phyparam_query_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_phyparam_query() const {
    return int_var__k_phyparam_query;
}
    
void cap_pxp_csr_cfg_p_mac_k_eq_t::k_query_timeout(const cpp_int & _val) { 
    // k_query_timeout
    int_var__k_query_timeout = _val.convert_to< k_query_timeout_cpp_int_t >();
}

cpp_int cap_pxp_csr_cfg_p_mac_k_eq_t::k_query_timeout() const {
    return int_var__k_query_timeout;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_int_p_ecc_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxbfr_overflow_enable")) { field_val = rxbfr_overflow_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "replay_bfr_overflow_enable")) { field_val = replay_bfr_overflow_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxtlp_err_enable")) { field_val = rxtlp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_uncorrectable_enable")) { field_val = txbuf_0_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_uncorrectable_enable")) { field_val = txbuf_1_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_uncorrectable_enable")) { field_val = txbuf_2_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_uncorrectable_enable")) { field_val = txbuf_3_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_correctable_enable")) { field_val = txbuf_0_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_correctable_enable")) { field_val = txbuf_1_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_correctable_enable")) { field_val = txbuf_2_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_correctable_enable")) { field_val = txbuf_3_correctable_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxbfr_overflow_interrupt")) { field_val = rxbfr_overflow_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "replay_bfr_overflow_interrupt")) { field_val = replay_bfr_overflow_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxtlp_err_interrupt")) { field_val = rxtlp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_uncorrectable_interrupt")) { field_val = txbuf_0_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_uncorrectable_interrupt")) { field_val = txbuf_1_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_uncorrectable_interrupt")) { field_val = txbuf_2_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_uncorrectable_interrupt")) { field_val = txbuf_3_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_correctable_interrupt")) { field_val = txbuf_0_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_correctable_interrupt")) { field_val = txbuf_1_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_correctable_interrupt")) { field_val = txbuf_2_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_correctable_interrupt")) { field_val = txbuf_3_correctable_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_p_ecc_interrupt")) { field_val = int_p_ecc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_p_ecc_enable")) { field_val = int_p_ecc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_deskew_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_fcpe_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { field_val = event(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_3_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_sram_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_pass")) { field_val = done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail")) { field_val = done_fail(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_mac_test_in_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bit_23_22")) { field_val = bit_23_22(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_sram_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { field_val = run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_ecc_disable_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "txbuf_cor")) { field_val = txbuf_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_det")) { field_val = txbuf_det(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_port_lanes_7_0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "detected")) { field_val = detected(); field_found=1; }
    if(!field_found && !strcmp(field_name, "active")) { field_val = active(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_port_mac_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pl_ltssm")) { field_val = pl_ltssm(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pl_equ_phase")) { field_val = pl_equ_phase(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rate")) { field_val = rate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxbfr_fill_level")) { field_val = rxbfr_fill_level(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxl0s_substate")) { field_val = rxl0s_substate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txl0s_substate")) { field_val = txl0s_substate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lanes_reversed")) { field_val = lanes_reversed(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_mac_k_eq_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "k_finetune_max_8gt")) { field_val = k_finetune_max_8gt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_preset_to_use_8gt")) { field_val = k_preset_to_use_8gt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_finetune_max_16gt")) { field_val = k_finetune_max_16gt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_preset_to_use_16gt")) { field_val = k_preset_to_use_16gt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_finetune_err")) { field_val = k_finetune_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_phyparam_query")) { field_val = k_phyparam_query(); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_query_timeout")) { field_val = k_query_timeout(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = cfg_p_mac_k_eq.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_port_mac.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_port_lanes_7_0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_ecc_disable.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_sram_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_mac_test_in.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_sram_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_3.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_bad_tlp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_bad_dllp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_nak_received.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_tx_nak_sent.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_nullified.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_fcpe.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_fc_timeout.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_txbuf_ecc_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_replay_num_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_replay_timer_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_core_initiated_recovery.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_ltssm_state_changed.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_8b10b_128b130b_skp_os_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_deskew_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_phystatus_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rxbfr_overflow.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_p_ecc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_int_p_ecc_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxbfr_overflow_enable")) { rxbfr_overflow_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "replay_bfr_overflow_enable")) { replay_bfr_overflow_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxtlp_err_enable")) { rxtlp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_uncorrectable_enable")) { txbuf_0_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_uncorrectable_enable")) { txbuf_1_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_uncorrectable_enable")) { txbuf_2_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_uncorrectable_enable")) { txbuf_3_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_correctable_enable")) { txbuf_0_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_correctable_enable")) { txbuf_1_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_correctable_enable")) { txbuf_2_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_correctable_enable")) { txbuf_3_correctable_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxbfr_overflow_interrupt")) { rxbfr_overflow_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "replay_bfr_overflow_interrupt")) { replay_bfr_overflow_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxtlp_err_interrupt")) { rxtlp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_uncorrectable_interrupt")) { txbuf_0_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_uncorrectable_interrupt")) { txbuf_1_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_uncorrectable_interrupt")) { txbuf_2_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_uncorrectable_interrupt")) { txbuf_3_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_0_correctable_interrupt")) { txbuf_0_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_1_correctable_interrupt")) { txbuf_1_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_2_correctable_interrupt")) { txbuf_2_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_3_correctable_interrupt")) { txbuf_3_correctable_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_p_ecc_interrupt")) { int_p_ecc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_p_ecc_enable")) { int_p_ecc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_deskew_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_fcpe_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "event")) { event(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_3_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_ecc_txbuf_0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_sram_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_pass")) { done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail")) { done_fail(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_mac_test_in_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bit_23_22")) { bit_23_22(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_sram_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_ecc_disable_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "txbuf_cor")) { txbuf_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txbuf_det")) { txbuf_det(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_port_lanes_7_0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "detected")) { detected(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "active")) { active(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_sta_p_port_mac_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pl_ltssm")) { pl_ltssm(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pl_equ_phase")) { pl_equ_phase(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rate")) { rate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxbfr_fill_level")) { rxbfr_fill_level(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxl0s_substate")) { rxl0s_substate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txl0s_substate")) { txl0s_substate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lanes_reversed")) { lanes_reversed(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_cfg_p_mac_k_eq_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "k_finetune_max_8gt")) { k_finetune_max_8gt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_preset_to_use_8gt")) { k_preset_to_use_8gt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_finetune_max_16gt")) { k_finetune_max_16gt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_preset_to_use_16gt")) { k_preset_to_use_16gt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_finetune_err")) { k_finetune_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_phyparam_query")) { k_phyparam_query(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "k_query_timeout")) { k_query_timeout(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pxp_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = cfg_p_mac_k_eq.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_port_mac.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_port_lanes_7_0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_ecc_disable.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_sram_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_p_mac_test_in.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_sram_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_p_ecc_txbuf_3.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_bad_tlp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_bad_dllp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_nak_received.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_tx_nak_sent.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rx_nullified.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_fcpe.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_fc_timeout.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_txbuf_ecc_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_replay_num_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_replay_timer_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_core_initiated_recovery.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_ltssm_state_changed.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_8b10b_128b130b_skp_os_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_deskew_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_phystatus_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_p_port_cnt_rxbfr_overflow.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_p_ecc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_int_p_ecc_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("rxbfr_overflow_enable");
    ret_vec.push_back("replay_bfr_overflow_enable");
    ret_vec.push_back("rxtlp_err_enable");
    ret_vec.push_back("txbuf_0_uncorrectable_enable");
    ret_vec.push_back("txbuf_1_uncorrectable_enable");
    ret_vec.push_back("txbuf_2_uncorrectable_enable");
    ret_vec.push_back("txbuf_3_uncorrectable_enable");
    ret_vec.push_back("txbuf_0_correctable_enable");
    ret_vec.push_back("txbuf_1_correctable_enable");
    ret_vec.push_back("txbuf_2_correctable_enable");
    ret_vec.push_back("txbuf_3_correctable_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("rxbfr_overflow_interrupt");
    ret_vec.push_back("replay_bfr_overflow_interrupt");
    ret_vec.push_back("rxtlp_err_interrupt");
    ret_vec.push_back("txbuf_0_uncorrectable_interrupt");
    ret_vec.push_back("txbuf_1_uncorrectable_interrupt");
    ret_vec.push_back("txbuf_2_uncorrectable_interrupt");
    ret_vec.push_back("txbuf_3_uncorrectable_interrupt");
    ret_vec.push_back("txbuf_0_correctable_interrupt");
    ret_vec.push_back("txbuf_1_correctable_interrupt");
    ret_vec.push_back("txbuf_2_correctable_interrupt");
    ret_vec.push_back("txbuf_3_correctable_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_intgrp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_test_set.get_fields(level-1)) {
            ret_vec.push_back("int_test_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_set.get_fields(level-1)) {
            ret_vec.push_back("int_enable_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_clear.get_fields(level-1)) {
            ret_vec.push_back("int_enable_clear." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_p_ecc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_p_ecc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_intgrp_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_enable_rw_reg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_rw_reg." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_phystatus_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_deskew_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_replay_num_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_fc_timeout_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_fcpe_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_rx_nullified_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("event");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_ecc_txbuf_3_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_ecc_txbuf_2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_ecc_txbuf_1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_ecc_txbuf_0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_sram_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("done_pass");
    ret_vec.push_back("done_fail");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_cfg_p_mac_test_in_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bit_23_22");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_cfg_p_sram_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_cfg_p_ecc_disable_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("txbuf_cor");
    ret_vec.push_back("txbuf_det");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_port_lanes_7_0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("detected");
    ret_vec.push_back("active");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_sta_p_port_mac_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pl_ltssm");
    ret_vec.push_back("pl_equ_phase");
    ret_vec.push_back("rate");
    ret_vec.push_back("rxbfr_fill_level");
    ret_vec.push_back("rxl0s_substate");
    ret_vec.push_back("txl0s_substate");
    ret_vec.push_back("lanes_reversed");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_cfg_p_mac_k_eq_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("k_finetune_max_8gt");
    ret_vec.push_back("k_preset_to_use_8gt");
    ret_vec.push_back("k_finetune_max_16gt");
    ret_vec.push_back("k_preset_to_use_16gt");
    ret_vec.push_back("k_finetune_err");
    ret_vec.push_back("k_phyparam_query");
    ret_vec.push_back("k_query_timeout");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pxp_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : cfg_p_mac_k_eq.get_fields(level-1)) {
            ret_vec.push_back("cfg_p_mac_k_eq." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_port_mac.get_fields(level-1)) {
            ret_vec.push_back("sta_p_port_mac." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_port_lanes_7_0.get_fields(level-1)) {
            ret_vec.push_back("sta_p_port_lanes_7_0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_p_ecc_disable.get_fields(level-1)) {
            ret_vec.push_back("cfg_p_ecc_disable." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_p_sram_bist.get_fields(level-1)) {
            ret_vec.push_back("cfg_p_sram_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_p_mac_test_in.get_fields(level-1)) {
            ret_vec.push_back("cfg_p_mac_test_in." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_sram_bist.get_fields(level-1)) {
            ret_vec.push_back("sta_p_sram_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_ecc_txbuf_0.get_fields(level-1)) {
            ret_vec.push_back("sta_p_ecc_txbuf_0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_ecc_txbuf_1.get_fields(level-1)) {
            ret_vec.push_back("sta_p_ecc_txbuf_1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_ecc_txbuf_2.get_fields(level-1)) {
            ret_vec.push_back("sta_p_ecc_txbuf_2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_p_ecc_txbuf_3.get_fields(level-1)) {
            ret_vec.push_back("sta_p_ecc_txbuf_3." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_rx_bad_tlp.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_rx_bad_tlp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_rx_bad_dllp.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_rx_bad_dllp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_rx_nak_received.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_rx_nak_received." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_tx_nak_sent.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_tx_nak_sent." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_rx_nullified.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_rx_nullified." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_fcpe.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_fcpe." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_fc_timeout.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_fc_timeout." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_txbuf_ecc_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_txbuf_ecc_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_replay_num_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_replay_num_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_replay_timer_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_replay_timer_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_core_initiated_recovery.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_core_initiated_recovery." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_ltssm_state_changed.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_ltssm_state_changed." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_8b10b_128b130b_skp_os_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_8b10b_128b130b_skp_os_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_deskew_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_deskew_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_phystatus_err.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_phystatus_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_p_port_cnt_rxbfr_overflow.get_fields(level-1)) {
            ret_vec.push_back("sat_p_port_cnt_rxbfr_overflow." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_p_ecc.get_fields(level-1)) {
            ret_vec.push_back("int_p_ecc." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
