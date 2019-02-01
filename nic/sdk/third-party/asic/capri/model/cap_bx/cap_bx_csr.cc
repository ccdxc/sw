
#include "cap_bx_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_bx_csr_int_mac_int_enable_clear_t::cap_bx_csr_int_mac_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_int_mac_int_enable_clear_t::~cap_bx_csr_int_mac_int_enable_clear_t() { }

cap_bx_csr_intreg_t::cap_bx_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_intreg_t::~cap_bx_csr_intreg_t() { }

cap_bx_csr_intgrp_t::cap_bx_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_intgrp_t::~cap_bx_csr_intgrp_t() { }

cap_bx_csr_intreg_status_t::cap_bx_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_intreg_status_t::~cap_bx_csr_intreg_status_t() { }

cap_bx_csr_int_groups_int_enable_rw_reg_t::cap_bx_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_int_groups_int_enable_rw_reg_t::~cap_bx_csr_int_groups_int_enable_rw_reg_t() { }

cap_bx_csr_intgrp_status_t::cap_bx_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_intgrp_status_t::~cap_bx_csr_intgrp_status_t() { }

cap_bx_csr_dhs_apb_entry_t::cap_bx_csr_dhs_apb_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_dhs_apb_entry_t::~cap_bx_csr_dhs_apb_entry_t() { }

cap_bx_csr_dhs_apb_t::cap_bx_csr_dhs_apb_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_bx_csr_dhs_apb_t::~cap_bx_csr_dhs_apb_t() { }

cap_bx_csr_dhs_mac_stats_entry_t::cap_bx_csr_dhs_mac_stats_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_dhs_mac_stats_entry_t::~cap_bx_csr_dhs_mac_stats_entry_t() { }

cap_bx_csr_dhs_mac_stats_t::cap_bx_csr_dhs_mac_stats_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_bx_csr_dhs_mac_stats_t::~cap_bx_csr_dhs_mac_stats_t() { }

cap_bx_csr_sta_stats_mem_t::cap_bx_csr_sta_stats_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_stats_mem_t::~cap_bx_csr_sta_stats_mem_t() { }

cap_bx_csr_cfg_stats_mem_t::cap_bx_csr_cfg_stats_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_stats_mem_t::~cap_bx_csr_cfg_stats_mem_t() { }

cap_bx_csr_sta_ff_rxfifo_t::cap_bx_csr_sta_ff_rxfifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_ff_rxfifo_t::~cap_bx_csr_sta_ff_rxfifo_t() { }

cap_bx_csr_cfg_ff_rxfifo_t::cap_bx_csr_cfg_ff_rxfifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_ff_rxfifo_t::~cap_bx_csr_cfg_ff_rxfifo_t() { }

cap_bx_csr_sta_ff_txfifo_t::cap_bx_csr_sta_ff_txfifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_ff_txfifo_t::~cap_bx_csr_sta_ff_txfifo_t() { }

cap_bx_csr_cfg_ff_txfifo_t::cap_bx_csr_cfg_ff_txfifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_ff_txfifo_t::~cap_bx_csr_cfg_ff_txfifo_t() { }

cap_bx_csr_cfg_debug_port_t::cap_bx_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_debug_port_t::~cap_bx_csr_cfg_debug_port_t() { }

cap_bx_csr_cfg_fixer_t::cap_bx_csr_cfg_fixer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_fixer_t::~cap_bx_csr_cfg_fixer_t() { }

cap_bx_csr_csr_intr_t::cap_bx_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_csr_intr_t::~cap_bx_csr_csr_intr_t() { }

cap_bx_csr_sta_mac_sd_status_t::cap_bx_csr_sta_mac_sd_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_mac_sd_status_t::~cap_bx_csr_sta_mac_sd_status_t() { }

cap_bx_csr_sta_mac_sd_interrupt_status_t::cap_bx_csr_sta_mac_sd_interrupt_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_mac_sd_interrupt_status_t::~cap_bx_csr_sta_mac_sd_interrupt_status_t() { }

cap_bx_csr_cfg_mac_sd_core_to_cntl_t::cap_bx_csr_cfg_mac_sd_core_to_cntl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_mac_sd_core_to_cntl_t::~cap_bx_csr_cfg_mac_sd_core_to_cntl_t() { }

cap_bx_csr_cfg_mac_sd_interrupt_request_t::cap_bx_csr_cfg_mac_sd_interrupt_request_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_mac_sd_interrupt_request_t::~cap_bx_csr_cfg_mac_sd_interrupt_request_t() { }

cap_bx_csr_cfg_mac_sd_interrupt_t::cap_bx_csr_cfg_mac_sd_interrupt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_mac_sd_interrupt_t::~cap_bx_csr_cfg_mac_sd_interrupt_t() { }

cap_bx_csr_cfg_mac_xoff_t::cap_bx_csr_cfg_mac_xoff_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_mac_xoff_t::~cap_bx_csr_cfg_mac_xoff_t() { }

cap_bx_csr_sta_mac_t::cap_bx_csr_sta_mac_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_sta_mac_t::~cap_bx_csr_sta_mac_t() { }

cap_bx_csr_cfg_serdes_t::cap_bx_csr_cfg_serdes_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_serdes_t::~cap_bx_csr_cfg_serdes_t() { }

cap_bx_csr_cfg_mac_gbl_t::cap_bx_csr_cfg_mac_gbl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_cfg_mac_gbl_t::~cap_bx_csr_cfg_mac_gbl_t() { }

cap_bx_csr_base_t::cap_bx_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_bx_csr_base_t::~cap_bx_csr_base_t() { }

cap_bx_csr_t::cap_bx_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(524288);
        set_attributes(0,get_name(), 0);
        }
cap_bx_csr_t::~cap_bx_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_int_mac_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lane_sbe_enable: 0x" << int_var__lane_sbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane_dbe_enable: 0x" << int_var__lane_dbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".xdmac10_intr_enable: 0x" << int_var__xdmac10_intr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".xdmac10_pslverr_enable: 0x" << int_var__xdmac10_pslverr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bxpb_pbus_drdy_enable: 0x" << int_var__bxpb_pbus_drdy_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_missing_sof_enable: 0x" << int_var__rx_missing_sof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_missing_eof_enable: 0x" << int_var__rx_missing_eof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_timeout_err_enable: 0x" << int_var__rx_timeout_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_min_size_err_enable: 0x" << int_var__rx_min_size_err_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lane_sbe_interrupt: 0x" << int_var__lane_sbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane_dbe_interrupt: 0x" << int_var__lane_dbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".xdmac10_intr_interrupt: 0x" << int_var__xdmac10_intr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".xdmac10_pslverr_interrupt: 0x" << int_var__xdmac10_pslverr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bxpb_pbus_drdy_interrupt: 0x" << int_var__bxpb_pbus_drdy_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_missing_sof_interrupt: 0x" << int_var__rx_missing_sof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_missing_eof_interrupt: 0x" << int_var__rx_missing_eof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_timeout_err_interrupt: 0x" << int_var__rx_timeout_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_min_size_err_interrupt: 0x" << int_var__rx_min_size_err_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mac_interrupt: 0x" << int_var__int_mac_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mac_enable: 0x" << int_var__int_mac_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_dhs_apb_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_dhs_apb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_dhs_mac_stats_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_dhs_mac_stats_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_stats_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_stats_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_ff_rxfifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_ff_rxfifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_ff_txfifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_ff_txfifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_debug_port_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_fixer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".timeout: 0x" << int_var__timeout << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_mac_sd_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rx_idle_detect: 0x" << int_var__rx_idle_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_rdy: 0x" << int_var__rx_rdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tx_rdy: 0x" << int_var__tx_rdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".core_status: 0x" << int_var__core_status << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_mac_sd_interrupt_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".in_progress: 0x" << int_var__in_progress << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_out: 0x" << int_var__data_out << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_mac_sd_core_to_cntl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_mac_sd_interrupt_request_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lane: 0x" << int_var__lane << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_mac_sd_interrupt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".code: 0x" << int_var__code << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_mac_xoff_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txfcxoff_i: 0x" << int_var__ff_txfcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txpfcxoff_i: 0x" << int_var__ff_txpfcxoff_i << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_sta_mac_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txafull_o: 0x" << int_var__ff_txafull_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_serdessigok_o: 0x" << int_var__ff_serdessigok_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txidle_o: 0x" << int_var__ff_txidle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txgood_o: 0x" << int_var__ff_txgood_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txread_o: 0x" << int_var__ff_txread_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxsync_o: 0x" << int_var__ff_rxsync_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxpfcxoff_o: 0x" << int_var__ff_rxpfcxoff_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxpreamble_o: 0x" << int_var__ff_rxpreamble_o << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_serdes_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rxsigok_i_sel: 0x" << int_var__rxsigok_i_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxsigok_i: 0x" << int_var__rxsigok_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tx_override_in: 0x" << int_var__tx_override_in << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_cfg_mac_gbl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txdispad_i: 0x" << int_var__ff_txdispad_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_fcs_enable: 0x" << int_var__pkt_fcs_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx_ipg: 0x" << int_var__ff_tx_ipg << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txpreamble_i: 0x" << int_var__ff_txpreamble_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_err_gen_enable: 0x" << int_var__rx_err_gen_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_err_mask: 0x" << int_var__rx_err_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".umacreg_rxdisfcschk: 0x" << int_var__umacreg_rxdisfcschk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_bx_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_mac_gbl.show();
    cfg_serdes.show();
    sta_mac.show();
    cfg_mac_xoff.show();
    cfg_mac_sd_interrupt.show();
    cfg_mac_sd_interrupt_request.show();
    cfg_mac_sd_core_to_cntl.show();
    sta_mac_sd_interrupt_status.show();
    sta_mac_sd_status.show();
    csr_intr.show();
    cfg_fixer.show();
    cfg_debug_port.show();
    cfg_ff_txfifo.show();
    sta_ff_txfifo.show();
    cfg_ff_rxfifo.show();
    sta_ff_rxfifo.show();
    cfg_stats_mem.show();
    sta_stats_mem.show();
    dhs_mac_stats.show();
    dhs_apb.show();
    int_groups.show();
    int_mac.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_bx_csr_int_mac_int_enable_clear_t::get_width() const {
    return cap_bx_csr_int_mac_int_enable_clear_t::s_get_width();

}

int cap_bx_csr_intreg_t::get_width() const {
    return cap_bx_csr_intreg_t::s_get_width();

}

int cap_bx_csr_intgrp_t::get_width() const {
    return cap_bx_csr_intgrp_t::s_get_width();

}

int cap_bx_csr_intreg_status_t::get_width() const {
    return cap_bx_csr_intreg_status_t::s_get_width();

}

int cap_bx_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_bx_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_bx_csr_intgrp_status_t::get_width() const {
    return cap_bx_csr_intgrp_status_t::s_get_width();

}

int cap_bx_csr_dhs_apb_entry_t::get_width() const {
    return cap_bx_csr_dhs_apb_entry_t::s_get_width();

}

int cap_bx_csr_dhs_apb_t::get_width() const {
    return cap_bx_csr_dhs_apb_t::s_get_width();

}

int cap_bx_csr_dhs_mac_stats_entry_t::get_width() const {
    return cap_bx_csr_dhs_mac_stats_entry_t::s_get_width();

}

int cap_bx_csr_dhs_mac_stats_t::get_width() const {
    return cap_bx_csr_dhs_mac_stats_t::s_get_width();

}

int cap_bx_csr_sta_stats_mem_t::get_width() const {
    return cap_bx_csr_sta_stats_mem_t::s_get_width();

}

int cap_bx_csr_cfg_stats_mem_t::get_width() const {
    return cap_bx_csr_cfg_stats_mem_t::s_get_width();

}

int cap_bx_csr_sta_ff_rxfifo_t::get_width() const {
    return cap_bx_csr_sta_ff_rxfifo_t::s_get_width();

}

int cap_bx_csr_cfg_ff_rxfifo_t::get_width() const {
    return cap_bx_csr_cfg_ff_rxfifo_t::s_get_width();

}

int cap_bx_csr_sta_ff_txfifo_t::get_width() const {
    return cap_bx_csr_sta_ff_txfifo_t::s_get_width();

}

int cap_bx_csr_cfg_ff_txfifo_t::get_width() const {
    return cap_bx_csr_cfg_ff_txfifo_t::s_get_width();

}

int cap_bx_csr_cfg_debug_port_t::get_width() const {
    return cap_bx_csr_cfg_debug_port_t::s_get_width();

}

int cap_bx_csr_cfg_fixer_t::get_width() const {
    return cap_bx_csr_cfg_fixer_t::s_get_width();

}

int cap_bx_csr_csr_intr_t::get_width() const {
    return cap_bx_csr_csr_intr_t::s_get_width();

}

int cap_bx_csr_sta_mac_sd_status_t::get_width() const {
    return cap_bx_csr_sta_mac_sd_status_t::s_get_width();

}

int cap_bx_csr_sta_mac_sd_interrupt_status_t::get_width() const {
    return cap_bx_csr_sta_mac_sd_interrupt_status_t::s_get_width();

}

int cap_bx_csr_cfg_mac_sd_core_to_cntl_t::get_width() const {
    return cap_bx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width();

}

int cap_bx_csr_cfg_mac_sd_interrupt_request_t::get_width() const {
    return cap_bx_csr_cfg_mac_sd_interrupt_request_t::s_get_width();

}

int cap_bx_csr_cfg_mac_sd_interrupt_t::get_width() const {
    return cap_bx_csr_cfg_mac_sd_interrupt_t::s_get_width();

}

int cap_bx_csr_cfg_mac_xoff_t::get_width() const {
    return cap_bx_csr_cfg_mac_xoff_t::s_get_width();

}

int cap_bx_csr_sta_mac_t::get_width() const {
    return cap_bx_csr_sta_mac_t::s_get_width();

}

int cap_bx_csr_cfg_serdes_t::get_width() const {
    return cap_bx_csr_cfg_serdes_t::s_get_width();

}

int cap_bx_csr_cfg_mac_gbl_t::get_width() const {
    return cap_bx_csr_cfg_mac_gbl_t::s_get_width();

}

int cap_bx_csr_base_t::get_width() const {
    return cap_bx_csr_base_t::s_get_width();

}

int cap_bx_csr_t::get_width() const {
    return cap_bx_csr_t::s_get_width();

}

int cap_bx_csr_int_mac_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // lane_sbe_enable
    _count += 1; // lane_dbe_enable
    _count += 1; // xdmac10_intr_enable
    _count += 1; // xdmac10_pslverr_enable
    _count += 1; // bxpb_pbus_drdy_enable
    _count += 1; // rx_missing_sof_enable
    _count += 1; // rx_missing_eof_enable
    _count += 1; // rx_timeout_err_enable
    _count += 1; // rx_min_size_err_enable
    return _count;
}

int cap_bx_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // lane_sbe_interrupt
    _count += 1; // lane_dbe_interrupt
    _count += 1; // xdmac10_intr_interrupt
    _count += 1; // xdmac10_pslverr_interrupt
    _count += 1; // bxpb_pbus_drdy_interrupt
    _count += 1; // rx_missing_sof_interrupt
    _count += 1; // rx_missing_eof_interrupt
    _count += 1; // rx_timeout_err_interrupt
    _count += 1; // rx_min_size_err_interrupt
    return _count;
}

int cap_bx_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_bx_csr_intreg_t::s_get_width(); // intreg
    _count += cap_bx_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_bx_csr_int_mac_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_bx_csr_int_mac_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_bx_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mac_interrupt
    return _count;
}

int cap_bx_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mac_enable
    return _count;
}

int cap_bx_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_bx_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_bx_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_bx_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_bx_csr_dhs_apb_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_bx_csr_dhs_apb_t::s_get_width() {
    int _count = 0;

    _count += (cap_bx_csr_dhs_apb_entry_t::s_get_width() * 65536); // entry
    return _count;
}

int cap_bx_csr_dhs_mac_stats_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_bx_csr_dhs_mac_stats_t::s_get_width() {
    int _count = 0;

    _count += (cap_bx_csr_dhs_mac_stats_entry_t::s_get_width() * 32); // entry
    return _count;
}

int cap_bx_csr_sta_stats_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_bx_csr_cfg_stats_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_bx_csr_sta_ff_rxfifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_bx_csr_cfg_ff_rxfifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_bx_csr_sta_ff_txfifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_bx_csr_cfg_ff_txfifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_bx_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // select
    return _count;
}

int cap_bx_csr_cfg_fixer_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 16; // timeout
    return _count;
}

int cap_bx_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_bx_csr_sta_mac_sd_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // rx_idle_detect
    _count += 1; // rx_rdy
    _count += 1; // tx_rdy
    _count += 32; // core_status
    return _count;
}

int cap_bx_csr_sta_mac_sd_interrupt_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // in_progress
    _count += 16; // data_out
    return _count;
}

int cap_bx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width() {
    int _count = 0;

    _count += 16; // value
    return _count;
}

int cap_bx_csr_cfg_mac_sd_interrupt_request_t::s_get_width() {
    int _count = 0;

    _count += 1; // lane
    return _count;
}

int cap_bx_csr_cfg_mac_sd_interrupt_t::s_get_width() {
    int _count = 0;

    _count += 16; // code
    _count += 16; // data
    return _count;
}

int cap_bx_csr_cfg_mac_xoff_t::s_get_width() {
    int _count = 0;

    _count += 1; // ff_txfcxoff_i
    _count += 8; // ff_txpfcxoff_i
    return _count;
}

int cap_bx_csr_sta_mac_t::s_get_width() {
    int _count = 0;

    _count += 1; // ff_txafull_o
    _count += 1; // ff_serdessigok_o
    _count += 1; // ff_txidle_o
    _count += 1; // ff_txgood_o
    _count += 1; // ff_txread_o
    _count += 1; // ff_rxsync_o
    _count += 8; // ff_rxpfcxoff_o
    _count += 56; // ff_rxpreamble_o
    return _count;
}

int cap_bx_csr_cfg_serdes_t::s_get_width() {
    int _count = 0;

    _count += 1; // rxsigok_i_sel
    _count += 1; // rxsigok_i
    _count += 1; // tx_override_in
    return _count;
}

int cap_bx_csr_cfg_mac_gbl_t::s_get_width() {
    int _count = 0;

    _count += 1; // ff_txdispad_i
    _count += 1; // pkt_fcs_enable
    _count += 8; // ff_tx_ipg
    _count += 56; // ff_txpreamble_i
    _count += 1; // rx_err_gen_enable
    _count += 7; // rx_err_mask
    _count += 1; // umacreg_rxdisfcschk
    return _count;
}

int cap_bx_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_bx_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_bx_csr_base_t::s_get_width(); // base
    _count += cap_bx_csr_cfg_mac_gbl_t::s_get_width(); // cfg_mac_gbl
    _count += cap_bx_csr_cfg_serdes_t::s_get_width(); // cfg_serdes
    _count += cap_bx_csr_sta_mac_t::s_get_width(); // sta_mac
    _count += cap_bx_csr_cfg_mac_xoff_t::s_get_width(); // cfg_mac_xoff
    _count += cap_bx_csr_cfg_mac_sd_interrupt_t::s_get_width(); // cfg_mac_sd_interrupt
    _count += cap_bx_csr_cfg_mac_sd_interrupt_request_t::s_get_width(); // cfg_mac_sd_interrupt_request
    _count += cap_bx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width(); // cfg_mac_sd_core_to_cntl
    _count += cap_bx_csr_sta_mac_sd_interrupt_status_t::s_get_width(); // sta_mac_sd_interrupt_status
    _count += cap_bx_csr_sta_mac_sd_status_t::s_get_width(); // sta_mac_sd_status
    _count += cap_bx_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_bx_csr_cfg_fixer_t::s_get_width(); // cfg_fixer
    _count += cap_bx_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_bx_csr_cfg_ff_txfifo_t::s_get_width(); // cfg_ff_txfifo
    _count += cap_bx_csr_sta_ff_txfifo_t::s_get_width(); // sta_ff_txfifo
    _count += cap_bx_csr_cfg_ff_rxfifo_t::s_get_width(); // cfg_ff_rxfifo
    _count += cap_bx_csr_sta_ff_rxfifo_t::s_get_width(); // sta_ff_rxfifo
    _count += cap_bx_csr_cfg_stats_mem_t::s_get_width(); // cfg_stats_mem
    _count += cap_bx_csr_sta_stats_mem_t::s_get_width(); // sta_stats_mem
    _count += cap_bx_csr_dhs_mac_stats_t::s_get_width(); // dhs_mac_stats
    _count += cap_bx_csr_dhs_apb_t::s_get_width(); // dhs_apb
    _count += cap_bx_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_bx_csr_intgrp_t::s_get_width(); // int_mac
    return _count;
}

void cap_bx_csr_int_mac_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lane_sbe_enable = _val.convert_to< lane_sbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane_dbe_enable = _val.convert_to< lane_dbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__xdmac10_intr_enable = _val.convert_to< xdmac10_intr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__xdmac10_pslverr_enable = _val.convert_to< xdmac10_pslverr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bxpb_pbus_drdy_enable = _val.convert_to< bxpb_pbus_drdy_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_missing_sof_enable = _val.convert_to< rx_missing_sof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_missing_eof_enable = _val.convert_to< rx_missing_eof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_timeout_err_enable = _val.convert_to< rx_timeout_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_min_size_err_enable = _val.convert_to< rx_min_size_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lane_sbe_interrupt = _val.convert_to< lane_sbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane_dbe_interrupt = _val.convert_to< lane_dbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__xdmac10_intr_interrupt = _val.convert_to< xdmac10_intr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__xdmac10_pslverr_interrupt = _val.convert_to< xdmac10_pslverr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bxpb_pbus_drdy_interrupt = _val.convert_to< bxpb_pbus_drdy_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_missing_sof_interrupt = _val.convert_to< rx_missing_sof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_missing_eof_interrupt = _val.convert_to< rx_missing_eof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_timeout_err_interrupt = _val.convert_to< rx_timeout_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_min_size_err_interrupt = _val.convert_to< rx_min_size_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_intgrp_t::all(const cpp_int & in_val) {
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

void cap_bx_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mac_interrupt = _val.convert_to< int_mac_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mac_enable = _val.convert_to< int_mac_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_bx_csr_dhs_apb_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_bx_csr_dhs_apb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_bx_csr_dhs_mac_stats_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_bx_csr_dhs_mac_stats_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_bx_csr_sta_stats_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_stats_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_sta_ff_rxfifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_ff_rxfifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_sta_ff_txfifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_ff_txfifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_debug_port_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__select = _val.convert_to< select_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_fixer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_bx_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_sta_mac_sd_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__rx_idle_detect = _val.convert_to< rx_idle_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_rdy = _val.convert_to< rx_rdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tx_rdy = _val.convert_to< tx_rdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__core_status = _val.convert_to< core_status_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_bx_csr_sta_mac_sd_interrupt_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__in_progress = _val.convert_to< in_progress_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__data_out = _val.convert_to< data_out_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_bx_csr_cfg_mac_sd_core_to_cntl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_bx_csr_cfg_mac_sd_interrupt_request_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lane = _val.convert_to< lane_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_mac_sd_interrupt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__code = _val.convert_to< code_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_bx_csr_cfg_mac_xoff_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ff_txfcxoff_i = _val.convert_to< ff_txfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txpfcxoff_i = _val.convert_to< ff_txpfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_bx_csr_sta_mac_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ff_txafull_o = _val.convert_to< ff_txafull_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_serdessigok_o = _val.convert_to< ff_serdessigok_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txidle_o = _val.convert_to< ff_txidle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txgood_o = _val.convert_to< ff_txgood_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txread_o = _val.convert_to< ff_txread_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxsync_o = _val.convert_to< ff_rxsync_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxpfcxoff_o = _val.convert_to< ff_rxpfcxoff_o_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_rxpreamble_o = _val.convert_to< ff_rxpreamble_o_cpp_int_t >()  ;
    _val = _val >> 56;
    
}

void cap_bx_csr_cfg_serdes_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__rxsigok_i_sel = _val.convert_to< rxsigok_i_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rxsigok_i = _val.convert_to< rxsigok_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tx_override_in = _val.convert_to< tx_override_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_cfg_mac_gbl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ff_txdispad_i = _val.convert_to< ff_txdispad_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_fcs_enable = _val.convert_to< pkt_fcs_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx_ipg = _val.convert_to< ff_tx_ipg_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_txpreamble_i = _val.convert_to< ff_txpreamble_i_cpp_int_t >()  ;
    _val = _val >> 56;
    
    int_var__rx_err_gen_enable = _val.convert_to< rx_err_gen_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx_err_mask = _val.convert_to< rx_err_mask_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__umacreg_rxdisfcschk = _val.convert_to< umacreg_rxdisfcschk_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_bx_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_bx_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_mac_gbl.all( _val);
    _val = _val >> cfg_mac_gbl.get_width(); 
    cfg_serdes.all( _val);
    _val = _val >> cfg_serdes.get_width(); 
    sta_mac.all( _val);
    _val = _val >> sta_mac.get_width(); 
    cfg_mac_xoff.all( _val);
    _val = _val >> cfg_mac_xoff.get_width(); 
    cfg_mac_sd_interrupt.all( _val);
    _val = _val >> cfg_mac_sd_interrupt.get_width(); 
    cfg_mac_sd_interrupt_request.all( _val);
    _val = _val >> cfg_mac_sd_interrupt_request.get_width(); 
    cfg_mac_sd_core_to_cntl.all( _val);
    _val = _val >> cfg_mac_sd_core_to_cntl.get_width(); 
    sta_mac_sd_interrupt_status.all( _val);
    _val = _val >> sta_mac_sd_interrupt_status.get_width(); 
    sta_mac_sd_status.all( _val);
    _val = _val >> sta_mac_sd_status.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    cfg_fixer.all( _val);
    _val = _val >> cfg_fixer.get_width(); 
    cfg_debug_port.all( _val);
    _val = _val >> cfg_debug_port.get_width(); 
    cfg_ff_txfifo.all( _val);
    _val = _val >> cfg_ff_txfifo.get_width(); 
    sta_ff_txfifo.all( _val);
    _val = _val >> sta_ff_txfifo.get_width(); 
    cfg_ff_rxfifo.all( _val);
    _val = _val >> cfg_ff_rxfifo.get_width(); 
    sta_ff_rxfifo.all( _val);
    _val = _val >> sta_ff_rxfifo.get_width(); 
    cfg_stats_mem.all( _val);
    _val = _val >> cfg_stats_mem.get_width(); 
    sta_stats_mem.all( _val);
    _val = _val >> sta_stats_mem.get_width(); 
    dhs_mac_stats.all( _val);
    _val = _val >> dhs_mac_stats.get_width(); 
    dhs_apb.all( _val);
    _val = _val >> dhs_apb.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_mac.all( _val);
    _val = _val >> int_mac.get_width(); 
}

cpp_int cap_bx_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_mac.get_width(); ret_val = ret_val  | int_mac.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << dhs_apb.get_width(); ret_val = ret_val  | dhs_apb.all(); 
    ret_val = ret_val << dhs_mac_stats.get_width(); ret_val = ret_val  | dhs_mac_stats.all(); 
    ret_val = ret_val << sta_stats_mem.get_width(); ret_val = ret_val  | sta_stats_mem.all(); 
    ret_val = ret_val << cfg_stats_mem.get_width(); ret_val = ret_val  | cfg_stats_mem.all(); 
    ret_val = ret_val << sta_ff_rxfifo.get_width(); ret_val = ret_val  | sta_ff_rxfifo.all(); 
    ret_val = ret_val << cfg_ff_rxfifo.get_width(); ret_val = ret_val  | cfg_ff_rxfifo.all(); 
    ret_val = ret_val << sta_ff_txfifo.get_width(); ret_val = ret_val  | sta_ff_txfifo.all(); 
    ret_val = ret_val << cfg_ff_txfifo.get_width(); ret_val = ret_val  | cfg_ff_txfifo.all(); 
    ret_val = ret_val << cfg_debug_port.get_width(); ret_val = ret_val  | cfg_debug_port.all(); 
    ret_val = ret_val << cfg_fixer.get_width(); ret_val = ret_val  | cfg_fixer.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_mac_sd_status.get_width(); ret_val = ret_val  | sta_mac_sd_status.all(); 
    ret_val = ret_val << sta_mac_sd_interrupt_status.get_width(); ret_val = ret_val  | sta_mac_sd_interrupt_status.all(); 
    ret_val = ret_val << cfg_mac_sd_core_to_cntl.get_width(); ret_val = ret_val  | cfg_mac_sd_core_to_cntl.all(); 
    ret_val = ret_val << cfg_mac_sd_interrupt_request.get_width(); ret_val = ret_val  | cfg_mac_sd_interrupt_request.all(); 
    ret_val = ret_val << cfg_mac_sd_interrupt.get_width(); ret_val = ret_val  | cfg_mac_sd_interrupt.all(); 
    ret_val = ret_val << cfg_mac_xoff.get_width(); ret_val = ret_val  | cfg_mac_xoff.all(); 
    ret_val = ret_val << sta_mac.get_width(); ret_val = ret_val  | sta_mac.all(); 
    ret_val = ret_val << cfg_serdes.get_width(); ret_val = ret_val  | cfg_serdes.all(); 
    ret_val = ret_val << cfg_mac_gbl.get_width(); ret_val = ret_val  | cfg_mac_gbl.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_bx_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::all() const {
    cpp_int ret_val;

    // umacreg_rxdisfcschk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__umacreg_rxdisfcschk; 
    
    // rx_err_mask
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__rx_err_mask; 
    
    // rx_err_gen_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_err_gen_enable; 
    
    // ff_txpreamble_i
    ret_val = ret_val << 56; ret_val = ret_val  | int_var__ff_txpreamble_i; 
    
    // ff_tx_ipg
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx_ipg; 
    
    // pkt_fcs_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkt_fcs_enable; 
    
    // ff_txdispad_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txdispad_i; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_serdes_t::all() const {
    cpp_int ret_val;

    // tx_override_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tx_override_in; 
    
    // rxsigok_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxsigok_i; 
    
    // rxsigok_i_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rxsigok_i_sel; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_mac_t::all() const {
    cpp_int ret_val;

    // ff_rxpreamble_o
    ret_val = ret_val << 56; ret_val = ret_val  | int_var__ff_rxpreamble_o; 
    
    // ff_rxpfcxoff_o
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_rxpfcxoff_o; 
    
    // ff_rxsync_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rxsync_o; 
    
    // ff_txread_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txread_o; 
    
    // ff_txgood_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txgood_o; 
    
    // ff_txidle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txidle_o; 
    
    // ff_serdessigok_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_serdessigok_o; 
    
    // ff_txafull_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txafull_o; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_mac_xoff_t::all() const {
    cpp_int ret_val;

    // ff_txpfcxoff_i
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_txpfcxoff_i; 
    
    // ff_txfcxoff_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txfcxoff_i; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_mac_sd_interrupt_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data; 
    
    // code
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__code; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_mac_sd_interrupt_request_t::all() const {
    cpp_int ret_val;

    // lane
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_mac_sd_core_to_cntl_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_mac_sd_interrupt_status_t::all() const {
    cpp_int ret_val;

    // data_out
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_out; 
    
    // in_progress
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__in_progress; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_mac_sd_status_t::all() const {
    cpp_int ret_val;

    // core_status
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__core_status; 
    
    // tx_rdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tx_rdy; 
    
    // rx_rdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_rdy; 
    
    // rx_idle_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_idle_detect; 
    
    return ret_val;
}

cpp_int cap_bx_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_fixer_t::all() const {
    cpp_int ret_val;

    // timeout
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__timeout; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_debug_port_t::all() const {
    cpp_int ret_val;

    // select
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__select; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_ff_txfifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_ff_txfifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_ff_rxfifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_ff_rxfifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_bx_csr_cfg_stats_mem_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_bx_csr_sta_stats_mem_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_bx_csr_dhs_mac_stats_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 32-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_bx_csr_dhs_mac_stats_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_bx_csr_dhs_apb_t::all() const {
    cpp_int ret_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 65536-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_bx_csr_dhs_apb_entry_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_bx_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_bx_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_mac_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mac_interrupt; 
    
    return ret_val;
}

cpp_int cap_bx_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_mac_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mac_enable; 
    
    return ret_val;
}

cpp_int cap_bx_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_bx_csr_intreg_t::all() const {
    cpp_int ret_val;

    // rx_min_size_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_min_size_err_interrupt; 
    
    // rx_timeout_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_timeout_err_interrupt; 
    
    // rx_missing_eof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_missing_eof_interrupt; 
    
    // rx_missing_sof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_missing_sof_interrupt; 
    
    // bxpb_pbus_drdy_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bxpb_pbus_drdy_interrupt; 
    
    // xdmac10_pslverr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__xdmac10_pslverr_interrupt; 
    
    // xdmac10_intr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__xdmac10_intr_interrupt; 
    
    // lane_dbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane_dbe_interrupt; 
    
    // lane_sbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane_sbe_interrupt; 
    
    return ret_val;
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // rx_min_size_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_min_size_err_enable; 
    
    // rx_timeout_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_timeout_err_enable; 
    
    // rx_missing_eof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_missing_eof_enable; 
    
    // rx_missing_sof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx_missing_sof_enable; 
    
    // bxpb_pbus_drdy_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bxpb_pbus_drdy_enable; 
    
    // xdmac10_pslverr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__xdmac10_pslverr_enable; 
    
    // xdmac10_intr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__xdmac10_intr_enable; 
    
    // lane_dbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane_dbe_enable; 
    
    // lane_sbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane_sbe_enable; 
    
    return ret_val;
}

void cap_bx_csr_int_mac_int_enable_clear_t::clear() {

    int_var__lane_sbe_enable = 0; 
    
    int_var__lane_dbe_enable = 0; 
    
    int_var__xdmac10_intr_enable = 0; 
    
    int_var__xdmac10_pslverr_enable = 0; 
    
    int_var__bxpb_pbus_drdy_enable = 0; 
    
    int_var__rx_missing_sof_enable = 0; 
    
    int_var__rx_missing_eof_enable = 0; 
    
    int_var__rx_timeout_err_enable = 0; 
    
    int_var__rx_min_size_err_enable = 0; 
    
}

void cap_bx_csr_intreg_t::clear() {

    int_var__lane_sbe_interrupt = 0; 
    
    int_var__lane_dbe_interrupt = 0; 
    
    int_var__xdmac10_intr_interrupt = 0; 
    
    int_var__xdmac10_pslverr_interrupt = 0; 
    
    int_var__bxpb_pbus_drdy_interrupt = 0; 
    
    int_var__rx_missing_sof_interrupt = 0; 
    
    int_var__rx_missing_eof_interrupt = 0; 
    
    int_var__rx_timeout_err_interrupt = 0; 
    
    int_var__rx_min_size_err_interrupt = 0; 
    
}

void cap_bx_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_bx_csr_intreg_status_t::clear() {

    int_var__int_mac_interrupt = 0; 
    
}

void cap_bx_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_mac_enable = 0; 
    
}

void cap_bx_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_bx_csr_dhs_apb_entry_t::clear() {

    int_var__data = 0; 
    
}

void cap_bx_csr_dhs_apb_t::clear() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_bx_csr_dhs_mac_stats_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_bx_csr_dhs_mac_stats_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_bx_csr_sta_stats_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_bx_csr_cfg_stats_mem_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_bx_csr_sta_ff_rxfifo_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_bx_csr_cfg_ff_rxfifo_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_bx_csr_sta_ff_txfifo_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_bx_csr_cfg_ff_txfifo_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_bx_csr_cfg_debug_port_t::clear() {

    int_var__enable = 0; 
    
    int_var__select = 0; 
    
}

void cap_bx_csr_cfg_fixer_t::clear() {

    int_var__enable = 0; 
    
    int_var__timeout = 0; 
    
}

void cap_bx_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_bx_csr_sta_mac_sd_status_t::clear() {

    int_var__rx_idle_detect = 0; 
    
    int_var__rx_rdy = 0; 
    
    int_var__tx_rdy = 0; 
    
    int_var__core_status = 0; 
    
}

void cap_bx_csr_sta_mac_sd_interrupt_status_t::clear() {

    int_var__in_progress = 0; 
    
    int_var__data_out = 0; 
    
}

void cap_bx_csr_cfg_mac_sd_core_to_cntl_t::clear() {

    int_var__value = 0; 
    
}

void cap_bx_csr_cfg_mac_sd_interrupt_request_t::clear() {

    int_var__lane = 0; 
    
}

void cap_bx_csr_cfg_mac_sd_interrupt_t::clear() {

    int_var__code = 0; 
    
    int_var__data = 0; 
    
}

void cap_bx_csr_cfg_mac_xoff_t::clear() {

    int_var__ff_txfcxoff_i = 0; 
    
    int_var__ff_txpfcxoff_i = 0; 
    
}

void cap_bx_csr_sta_mac_t::clear() {

    int_var__ff_txafull_o = 0; 
    
    int_var__ff_serdessigok_o = 0; 
    
    int_var__ff_txidle_o = 0; 
    
    int_var__ff_txgood_o = 0; 
    
    int_var__ff_txread_o = 0; 
    
    int_var__ff_rxsync_o = 0; 
    
    int_var__ff_rxpfcxoff_o = 0; 
    
    int_var__ff_rxpreamble_o = 0; 
    
}

void cap_bx_csr_cfg_serdes_t::clear() {

    int_var__rxsigok_i_sel = 0; 
    
    int_var__rxsigok_i = 0; 
    
    int_var__tx_override_in = 0; 
    
}

void cap_bx_csr_cfg_mac_gbl_t::clear() {

    int_var__ff_txdispad_i = 0; 
    
    int_var__pkt_fcs_enable = 0; 
    
    int_var__ff_tx_ipg = 0; 
    
    int_var__ff_txpreamble_i = 0; 
    
    int_var__rx_err_gen_enable = 0; 
    
    int_var__rx_err_mask = 0; 
    
    int_var__umacreg_rxdisfcschk = 0; 
    
}

void cap_bx_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_bx_csr_t::clear() {

    base.clear();
    cfg_mac_gbl.clear();
    cfg_serdes.clear();
    sta_mac.clear();
    cfg_mac_xoff.clear();
    cfg_mac_sd_interrupt.clear();
    cfg_mac_sd_interrupt_request.clear();
    cfg_mac_sd_core_to_cntl.clear();
    sta_mac_sd_interrupt_status.clear();
    sta_mac_sd_status.clear();
    csr_intr.clear();
    cfg_fixer.clear();
    cfg_debug_port.clear();
    cfg_ff_txfifo.clear();
    sta_ff_txfifo.clear();
    cfg_ff_rxfifo.clear();
    sta_ff_rxfifo.clear();
    cfg_stats_mem.clear();
    sta_stats_mem.clear();
    dhs_mac_stats.clear();
    dhs_apb.clear();
    int_groups.clear();
    int_mac.clear();
}

void cap_bx_csr_int_mac_int_enable_clear_t::init() {

}

void cap_bx_csr_intreg_t::init() {

}

void cap_bx_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_bx_csr_intreg_status_t::init() {

}

void cap_bx_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_bx_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_bx_csr_dhs_apb_entry_t::init() {

}

void cap_bx_csr_dhs_apb_t::init() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 65536; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_bx_csr_dhs_mac_stats_entry_t::init() {

}

void cap_bx_csr_dhs_mac_stats_t::init() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_bx_csr_sta_stats_mem_t::init() {

}

void cap_bx_csr_cfg_stats_mem_t::init() {

}

void cap_bx_csr_sta_ff_rxfifo_t::init() {

}

void cap_bx_csr_cfg_ff_rxfifo_t::init() {

}

void cap_bx_csr_sta_ff_txfifo_t::init() {

}

void cap_bx_csr_cfg_ff_txfifo_t::init() {

}

void cap_bx_csr_cfg_debug_port_t::init() {

}

void cap_bx_csr_cfg_fixer_t::init() {

    set_reset_val(cpp_int("0x201"));
    all(get_reset_val());
}

void cap_bx_csr_csr_intr_t::init() {

}

void cap_bx_csr_sta_mac_sd_status_t::init() {

}

void cap_bx_csr_sta_mac_sd_interrupt_status_t::init() {

}

void cap_bx_csr_cfg_mac_sd_core_to_cntl_t::init() {

    set_reset_val(cpp_int("0xaaaa"));
    all(get_reset_val());
}

void cap_bx_csr_cfg_mac_sd_interrupt_request_t::init() {

}

void cap_bx_csr_cfg_mac_sd_interrupt_t::init() {

}

void cap_bx_csr_cfg_mac_xoff_t::init() {

    set_reset_val(cpp_int("0x1fe"));
    all(get_reset_val());
}

void cap_bx_csr_sta_mac_t::init() {

}

void cap_bx_csr_cfg_serdes_t::init() {

}

void cap_bx_csr_cfg_mac_gbl_t::init() {

    set_reset_val(cpp_int("0x1055555555555555432"));
    all(get_reset_val());
}

void cap_bx_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_bx_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_mac_gbl.set_attributes(this,"cfg_mac_gbl", 0x10 );
    cfg_serdes.set_attributes(this,"cfg_serdes", 0x20 );
    sta_mac.set_attributes(this,"sta_mac", 0x30 );
    cfg_mac_xoff.set_attributes(this,"cfg_mac_xoff", 0x200 );
    cfg_mac_sd_interrupt.set_attributes(this,"cfg_mac_sd_interrupt", 0x204 );
    cfg_mac_sd_interrupt_request.set_attributes(this,"cfg_mac_sd_interrupt_request", 0x208 );
    cfg_mac_sd_core_to_cntl.set_attributes(this,"cfg_mac_sd_core_to_cntl", 0x20c );
    sta_mac_sd_interrupt_status.set_attributes(this,"sta_mac_sd_interrupt_status", 0x210 );
    sta_mac_sd_status.set_attributes(this,"sta_mac_sd_status", 0x218 );
    csr_intr.set_attributes(this,"csr_intr", 0x220 );
    cfg_fixer.set_attributes(this,"cfg_fixer", 0x250 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x254 );
    cfg_ff_txfifo.set_attributes(this,"cfg_ff_txfifo", 0x258 );
    sta_ff_txfifo.set_attributes(this,"sta_ff_txfifo", 0x25c );
    cfg_ff_rxfifo.set_attributes(this,"cfg_ff_rxfifo", 0x260 );
    sta_ff_rxfifo.set_attributes(this,"sta_ff_rxfifo", 0x264 );
    cfg_stats_mem.set_attributes(this,"cfg_stats_mem", 0x268 );
    sta_stats_mem.set_attributes(this,"sta_stats_mem", 0x26c );
    dhs_mac_stats.set_attributes(this,"dhs_mac_stats", 0x100 );
    dhs_apb.set_attributes(this,"dhs_apb", 0x40000 );
    int_groups.set_attributes(this,"int_groups", 0x230 );
    int_mac.set_attributes(this,"int_mac", 0x240 );
}

void cap_bx_csr_int_mac_int_enable_clear_t::lane_sbe_enable(const cpp_int & _val) { 
    // lane_sbe_enable
    int_var__lane_sbe_enable = _val.convert_to< lane_sbe_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::lane_sbe_enable() const {
    return int_var__lane_sbe_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::lane_dbe_enable(const cpp_int & _val) { 
    // lane_dbe_enable
    int_var__lane_dbe_enable = _val.convert_to< lane_dbe_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::lane_dbe_enable() const {
    return int_var__lane_dbe_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::xdmac10_intr_enable(const cpp_int & _val) { 
    // xdmac10_intr_enable
    int_var__xdmac10_intr_enable = _val.convert_to< xdmac10_intr_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::xdmac10_intr_enable() const {
    return int_var__xdmac10_intr_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::xdmac10_pslverr_enable(const cpp_int & _val) { 
    // xdmac10_pslverr_enable
    int_var__xdmac10_pslverr_enable = _val.convert_to< xdmac10_pslverr_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::xdmac10_pslverr_enable() const {
    return int_var__xdmac10_pslverr_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::bxpb_pbus_drdy_enable(const cpp_int & _val) { 
    // bxpb_pbus_drdy_enable
    int_var__bxpb_pbus_drdy_enable = _val.convert_to< bxpb_pbus_drdy_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::bxpb_pbus_drdy_enable() const {
    return int_var__bxpb_pbus_drdy_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::rx_missing_sof_enable(const cpp_int & _val) { 
    // rx_missing_sof_enable
    int_var__rx_missing_sof_enable = _val.convert_to< rx_missing_sof_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::rx_missing_sof_enable() const {
    return int_var__rx_missing_sof_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::rx_missing_eof_enable(const cpp_int & _val) { 
    // rx_missing_eof_enable
    int_var__rx_missing_eof_enable = _val.convert_to< rx_missing_eof_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::rx_missing_eof_enable() const {
    return int_var__rx_missing_eof_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::rx_timeout_err_enable(const cpp_int & _val) { 
    // rx_timeout_err_enable
    int_var__rx_timeout_err_enable = _val.convert_to< rx_timeout_err_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::rx_timeout_err_enable() const {
    return int_var__rx_timeout_err_enable;
}
    
void cap_bx_csr_int_mac_int_enable_clear_t::rx_min_size_err_enable(const cpp_int & _val) { 
    // rx_min_size_err_enable
    int_var__rx_min_size_err_enable = _val.convert_to< rx_min_size_err_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_mac_int_enable_clear_t::rx_min_size_err_enable() const {
    return int_var__rx_min_size_err_enable;
}
    
void cap_bx_csr_intreg_t::lane_sbe_interrupt(const cpp_int & _val) { 
    // lane_sbe_interrupt
    int_var__lane_sbe_interrupt = _val.convert_to< lane_sbe_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::lane_sbe_interrupt() const {
    return int_var__lane_sbe_interrupt;
}
    
void cap_bx_csr_intreg_t::lane_dbe_interrupt(const cpp_int & _val) { 
    // lane_dbe_interrupt
    int_var__lane_dbe_interrupt = _val.convert_to< lane_dbe_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::lane_dbe_interrupt() const {
    return int_var__lane_dbe_interrupt;
}
    
void cap_bx_csr_intreg_t::xdmac10_intr_interrupt(const cpp_int & _val) { 
    // xdmac10_intr_interrupt
    int_var__xdmac10_intr_interrupt = _val.convert_to< xdmac10_intr_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::xdmac10_intr_interrupt() const {
    return int_var__xdmac10_intr_interrupt;
}
    
void cap_bx_csr_intreg_t::xdmac10_pslverr_interrupt(const cpp_int & _val) { 
    // xdmac10_pslverr_interrupt
    int_var__xdmac10_pslverr_interrupt = _val.convert_to< xdmac10_pslverr_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::xdmac10_pslverr_interrupt() const {
    return int_var__xdmac10_pslverr_interrupt;
}
    
void cap_bx_csr_intreg_t::bxpb_pbus_drdy_interrupt(const cpp_int & _val) { 
    // bxpb_pbus_drdy_interrupt
    int_var__bxpb_pbus_drdy_interrupt = _val.convert_to< bxpb_pbus_drdy_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::bxpb_pbus_drdy_interrupt() const {
    return int_var__bxpb_pbus_drdy_interrupt;
}
    
void cap_bx_csr_intreg_t::rx_missing_sof_interrupt(const cpp_int & _val) { 
    // rx_missing_sof_interrupt
    int_var__rx_missing_sof_interrupt = _val.convert_to< rx_missing_sof_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::rx_missing_sof_interrupt() const {
    return int_var__rx_missing_sof_interrupt;
}
    
void cap_bx_csr_intreg_t::rx_missing_eof_interrupt(const cpp_int & _val) { 
    // rx_missing_eof_interrupt
    int_var__rx_missing_eof_interrupt = _val.convert_to< rx_missing_eof_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::rx_missing_eof_interrupt() const {
    return int_var__rx_missing_eof_interrupt;
}
    
void cap_bx_csr_intreg_t::rx_timeout_err_interrupt(const cpp_int & _val) { 
    // rx_timeout_err_interrupt
    int_var__rx_timeout_err_interrupt = _val.convert_to< rx_timeout_err_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::rx_timeout_err_interrupt() const {
    return int_var__rx_timeout_err_interrupt;
}
    
void cap_bx_csr_intreg_t::rx_min_size_err_interrupt(const cpp_int & _val) { 
    // rx_min_size_err_interrupt
    int_var__rx_min_size_err_interrupt = _val.convert_to< rx_min_size_err_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_t::rx_min_size_err_interrupt() const {
    return int_var__rx_min_size_err_interrupt;
}
    
void cap_bx_csr_intreg_status_t::int_mac_interrupt(const cpp_int & _val) { 
    // int_mac_interrupt
    int_var__int_mac_interrupt = _val.convert_to< int_mac_interrupt_cpp_int_t >();
}

cpp_int cap_bx_csr_intreg_status_t::int_mac_interrupt() const {
    return int_var__int_mac_interrupt;
}
    
void cap_bx_csr_int_groups_int_enable_rw_reg_t::int_mac_enable(const cpp_int & _val) { 
    // int_mac_enable
    int_var__int_mac_enable = _val.convert_to< int_mac_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_int_groups_int_enable_rw_reg_t::int_mac_enable() const {
    return int_var__int_mac_enable;
}
    
void cap_bx_csr_dhs_apb_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_bx_csr_dhs_apb_entry_t::data() const {
    return int_var__data;
}
    
void cap_bx_csr_dhs_mac_stats_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_bx_csr_dhs_mac_stats_entry_t::value() const {
    return int_var__value;
}
    
void cap_bx_csr_sta_stats_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_stats_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_bx_csr_sta_stats_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_stats_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_bx_csr_cfg_stats_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_stats_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_bx_csr_sta_ff_rxfifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_ff_rxfifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_bx_csr_sta_ff_rxfifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_ff_rxfifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_bx_csr_cfg_ff_rxfifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_ff_rxfifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_bx_csr_sta_ff_txfifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_ff_txfifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_bx_csr_sta_ff_txfifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_ff_txfifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_bx_csr_cfg_ff_txfifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_ff_txfifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_bx_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_debug_port_t::enable() const {
    return int_var__enable;
}
    
void cap_bx_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_debug_port_t::select() const {
    return int_var__select;
}
    
void cap_bx_csr_cfg_fixer_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_fixer_t::enable() const {
    return int_var__enable;
}
    
void cap_bx_csr_cfg_fixer_t::timeout(const cpp_int & _val) { 
    // timeout
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_fixer_t::timeout() const {
    return int_var__timeout;
}
    
void cap_bx_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_bx_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_bx_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_bx_csr_sta_mac_sd_status_t::rx_idle_detect(const cpp_int & _val) { 
    // rx_idle_detect
    int_var__rx_idle_detect = _val.convert_to< rx_idle_detect_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_status_t::rx_idle_detect() const {
    return int_var__rx_idle_detect;
}
    
void cap_bx_csr_sta_mac_sd_status_t::rx_rdy(const cpp_int & _val) { 
    // rx_rdy
    int_var__rx_rdy = _val.convert_to< rx_rdy_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_status_t::rx_rdy() const {
    return int_var__rx_rdy;
}
    
void cap_bx_csr_sta_mac_sd_status_t::tx_rdy(const cpp_int & _val) { 
    // tx_rdy
    int_var__tx_rdy = _val.convert_to< tx_rdy_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_status_t::tx_rdy() const {
    return int_var__tx_rdy;
}
    
void cap_bx_csr_sta_mac_sd_status_t::core_status(const cpp_int & _val) { 
    // core_status
    int_var__core_status = _val.convert_to< core_status_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_status_t::core_status() const {
    return int_var__core_status;
}
    
void cap_bx_csr_sta_mac_sd_interrupt_status_t::in_progress(const cpp_int & _val) { 
    // in_progress
    int_var__in_progress = _val.convert_to< in_progress_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_interrupt_status_t::in_progress() const {
    return int_var__in_progress;
}
    
void cap_bx_csr_sta_mac_sd_interrupt_status_t::data_out(const cpp_int & _val) { 
    // data_out
    int_var__data_out = _val.convert_to< data_out_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_sd_interrupt_status_t::data_out() const {
    return int_var__data_out;
}
    
void cap_bx_csr_cfg_mac_sd_core_to_cntl_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_sd_core_to_cntl_t::value() const {
    return int_var__value;
}
    
void cap_bx_csr_cfg_mac_sd_interrupt_request_t::lane(const cpp_int & _val) { 
    // lane
    int_var__lane = _val.convert_to< lane_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_sd_interrupt_request_t::lane() const {
    return int_var__lane;
}
    
void cap_bx_csr_cfg_mac_sd_interrupt_t::code(const cpp_int & _val) { 
    // code
    int_var__code = _val.convert_to< code_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_sd_interrupt_t::code() const {
    return int_var__code;
}
    
void cap_bx_csr_cfg_mac_sd_interrupt_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_sd_interrupt_t::data() const {
    return int_var__data;
}
    
void cap_bx_csr_cfg_mac_xoff_t::ff_txfcxoff_i(const cpp_int & _val) { 
    // ff_txfcxoff_i
    int_var__ff_txfcxoff_i = _val.convert_to< ff_txfcxoff_i_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_xoff_t::ff_txfcxoff_i() const {
    return int_var__ff_txfcxoff_i;
}
    
void cap_bx_csr_cfg_mac_xoff_t::ff_txpfcxoff_i(const cpp_int & _val) { 
    // ff_txpfcxoff_i
    int_var__ff_txpfcxoff_i = _val.convert_to< ff_txpfcxoff_i_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_xoff_t::ff_txpfcxoff_i() const {
    return int_var__ff_txpfcxoff_i;
}
    
void cap_bx_csr_sta_mac_t::ff_txafull_o(const cpp_int & _val) { 
    // ff_txafull_o
    int_var__ff_txafull_o = _val.convert_to< ff_txafull_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_txafull_o() const {
    return int_var__ff_txafull_o;
}
    
void cap_bx_csr_sta_mac_t::ff_serdessigok_o(const cpp_int & _val) { 
    // ff_serdessigok_o
    int_var__ff_serdessigok_o = _val.convert_to< ff_serdessigok_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_serdessigok_o() const {
    return int_var__ff_serdessigok_o;
}
    
void cap_bx_csr_sta_mac_t::ff_txidle_o(const cpp_int & _val) { 
    // ff_txidle_o
    int_var__ff_txidle_o = _val.convert_to< ff_txidle_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_txidle_o() const {
    return int_var__ff_txidle_o;
}
    
void cap_bx_csr_sta_mac_t::ff_txgood_o(const cpp_int & _val) { 
    // ff_txgood_o
    int_var__ff_txgood_o = _val.convert_to< ff_txgood_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_txgood_o() const {
    return int_var__ff_txgood_o;
}
    
void cap_bx_csr_sta_mac_t::ff_txread_o(const cpp_int & _val) { 
    // ff_txread_o
    int_var__ff_txread_o = _val.convert_to< ff_txread_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_txread_o() const {
    return int_var__ff_txread_o;
}
    
void cap_bx_csr_sta_mac_t::ff_rxsync_o(const cpp_int & _val) { 
    // ff_rxsync_o
    int_var__ff_rxsync_o = _val.convert_to< ff_rxsync_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_rxsync_o() const {
    return int_var__ff_rxsync_o;
}
    
void cap_bx_csr_sta_mac_t::ff_rxpfcxoff_o(const cpp_int & _val) { 
    // ff_rxpfcxoff_o
    int_var__ff_rxpfcxoff_o = _val.convert_to< ff_rxpfcxoff_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_rxpfcxoff_o() const {
    return int_var__ff_rxpfcxoff_o;
}
    
void cap_bx_csr_sta_mac_t::ff_rxpreamble_o(const cpp_int & _val) { 
    // ff_rxpreamble_o
    int_var__ff_rxpreamble_o = _val.convert_to< ff_rxpreamble_o_cpp_int_t >();
}

cpp_int cap_bx_csr_sta_mac_t::ff_rxpreamble_o() const {
    return int_var__ff_rxpreamble_o;
}
    
void cap_bx_csr_cfg_serdes_t::rxsigok_i_sel(const cpp_int & _val) { 
    // rxsigok_i_sel
    int_var__rxsigok_i_sel = _val.convert_to< rxsigok_i_sel_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_serdes_t::rxsigok_i_sel() const {
    return int_var__rxsigok_i_sel;
}
    
void cap_bx_csr_cfg_serdes_t::rxsigok_i(const cpp_int & _val) { 
    // rxsigok_i
    int_var__rxsigok_i = _val.convert_to< rxsigok_i_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_serdes_t::rxsigok_i() const {
    return int_var__rxsigok_i;
}
    
void cap_bx_csr_cfg_serdes_t::tx_override_in(const cpp_int & _val) { 
    // tx_override_in
    int_var__tx_override_in = _val.convert_to< tx_override_in_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_serdes_t::tx_override_in() const {
    return int_var__tx_override_in;
}
    
void cap_bx_csr_cfg_mac_gbl_t::ff_txdispad_i(const cpp_int & _val) { 
    // ff_txdispad_i
    int_var__ff_txdispad_i = _val.convert_to< ff_txdispad_i_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::ff_txdispad_i() const {
    return int_var__ff_txdispad_i;
}
    
void cap_bx_csr_cfg_mac_gbl_t::pkt_fcs_enable(const cpp_int & _val) { 
    // pkt_fcs_enable
    int_var__pkt_fcs_enable = _val.convert_to< pkt_fcs_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::pkt_fcs_enable() const {
    return int_var__pkt_fcs_enable;
}
    
void cap_bx_csr_cfg_mac_gbl_t::ff_tx_ipg(const cpp_int & _val) { 
    // ff_tx_ipg
    int_var__ff_tx_ipg = _val.convert_to< ff_tx_ipg_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::ff_tx_ipg() const {
    return int_var__ff_tx_ipg;
}
    
void cap_bx_csr_cfg_mac_gbl_t::ff_txpreamble_i(const cpp_int & _val) { 
    // ff_txpreamble_i
    int_var__ff_txpreamble_i = _val.convert_to< ff_txpreamble_i_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::ff_txpreamble_i() const {
    return int_var__ff_txpreamble_i;
}
    
void cap_bx_csr_cfg_mac_gbl_t::rx_err_gen_enable(const cpp_int & _val) { 
    // rx_err_gen_enable
    int_var__rx_err_gen_enable = _val.convert_to< rx_err_gen_enable_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::rx_err_gen_enable() const {
    return int_var__rx_err_gen_enable;
}
    
void cap_bx_csr_cfg_mac_gbl_t::rx_err_mask(const cpp_int & _val) { 
    // rx_err_mask
    int_var__rx_err_mask = _val.convert_to< rx_err_mask_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::rx_err_mask() const {
    return int_var__rx_err_mask;
}
    
void cap_bx_csr_cfg_mac_gbl_t::umacreg_rxdisfcschk(const cpp_int & _val) { 
    // umacreg_rxdisfcschk
    int_var__umacreg_rxdisfcschk = _val.convert_to< umacreg_rxdisfcschk_cpp_int_t >();
}

cpp_int cap_bx_csr_cfg_mac_gbl_t::umacreg_rxdisfcschk() const {
    return int_var__umacreg_rxdisfcschk;
}
    
void cap_bx_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_bx_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_int_mac_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane_sbe_enable")) { field_val = lane_sbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane_dbe_enable")) { field_val = lane_dbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_intr_enable")) { field_val = xdmac10_intr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_pslverr_enable")) { field_val = xdmac10_pslverr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bxpb_pbus_drdy_enable")) { field_val = bxpb_pbus_drdy_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_sof_enable")) { field_val = rx_missing_sof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_eof_enable")) { field_val = rx_missing_eof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_timeout_err_enable")) { field_val = rx_timeout_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_min_size_err_enable")) { field_val = rx_min_size_err_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane_sbe_interrupt")) { field_val = lane_sbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane_dbe_interrupt")) { field_val = lane_dbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_intr_interrupt")) { field_val = xdmac10_intr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_pslverr_interrupt")) { field_val = xdmac10_pslverr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bxpb_pbus_drdy_interrupt")) { field_val = bxpb_pbus_drdy_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_sof_interrupt")) { field_val = rx_missing_sof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_eof_interrupt")) { field_val = rx_missing_eof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_timeout_err_interrupt")) { field_val = rx_timeout_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_min_size_err_interrupt")) { field_val = rx_min_size_err_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_bx_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_interrupt")) { field_val = int_mac_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_enable")) { field_val = int_mac_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_bx_csr_dhs_apb_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_apb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_mac_stats_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_mac_stats_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_stats_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_stats_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_ff_rxfifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_ff_rxfifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_ff_txfifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_ff_txfifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_debug_port_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { field_val = select(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_fixer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { field_val = timeout(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_sd_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rx_idle_detect")) { field_val = rx_idle_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_rdy")) { field_val = rx_rdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tx_rdy")) { field_val = tx_rdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_status")) { field_val = core_status(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_sd_interrupt_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_progress")) { field_val = in_progress(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_out")) { field_val = data_out(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_core_to_cntl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_interrupt_request_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane")) { field_val = lane(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_interrupt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "code")) { field_val = code(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_xoff_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txfcxoff_i")) { field_val = ff_txfcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpfcxoff_i")) { field_val = ff_txpfcxoff_i(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txafull_o")) { field_val = ff_txafull_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdessigok_o")) { field_val = ff_serdessigok_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txidle_o")) { field_val = ff_txidle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txgood_o")) { field_val = ff_txgood_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txread_o")) { field_val = ff_txread_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxsync_o")) { field_val = ff_rxsync_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxpfcxoff_o")) { field_val = ff_rxpfcxoff_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxpreamble_o")) { field_val = ff_rxpreamble_o(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_serdes_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxsigok_i_sel")) { field_val = rxsigok_i_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxsigok_i")) { field_val = rxsigok_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tx_override_in")) { field_val = tx_override_in(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_gbl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txdispad_i")) { field_val = ff_txdispad_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_fcs_enable")) { field_val = pkt_fcs_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx_ipg")) { field_val = ff_tx_ipg(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpreamble_i")) { field_val = ff_txpreamble_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_err_gen_enable")) { field_val = rx_err_gen_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_err_mask")) { field_val = rx_err_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "umacreg_rxdisfcschk")) { field_val = umacreg_rxdisfcschk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_gbl.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_serdes.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_xoff.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_interrupt.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_interrupt_request.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_core_to_cntl.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac_sd_interrupt_status.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac_sd_status.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fixer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ff_txfifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_txfifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ff_rxfifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_rxfifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_stats_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stats_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_mac_stats.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mac.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_int_mac_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane_sbe_enable")) { lane_sbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane_dbe_enable")) { lane_dbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_intr_enable")) { xdmac10_intr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_pslverr_enable")) { xdmac10_pslverr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bxpb_pbus_drdy_enable")) { bxpb_pbus_drdy_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_sof_enable")) { rx_missing_sof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_eof_enable")) { rx_missing_eof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_timeout_err_enable")) { rx_timeout_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_min_size_err_enable")) { rx_min_size_err_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane_sbe_interrupt")) { lane_sbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane_dbe_interrupt")) { lane_dbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_intr_interrupt")) { xdmac10_intr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "xdmac10_pslverr_interrupt")) { xdmac10_pslverr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bxpb_pbus_drdy_interrupt")) { bxpb_pbus_drdy_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_sof_interrupt")) { rx_missing_sof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_missing_eof_interrupt")) { rx_missing_eof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_timeout_err_interrupt")) { rx_timeout_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_min_size_err_interrupt")) { rx_min_size_err_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_bx_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_interrupt")) { int_mac_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_enable")) { int_mac_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_bx_csr_dhs_apb_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_apb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_mac_stats_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_dhs_mac_stats_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_stats_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_stats_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_ff_rxfifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_ff_rxfifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_ff_txfifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_ff_txfifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_debug_port_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { select(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_fixer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { timeout(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_sd_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rx_idle_detect")) { rx_idle_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_rdy")) { rx_rdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tx_rdy")) { tx_rdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_status")) { core_status(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_sd_interrupt_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_progress")) { in_progress(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_out")) { data_out(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_core_to_cntl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_interrupt_request_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane")) { lane(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_sd_interrupt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "code")) { code(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_xoff_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txfcxoff_i")) { ff_txfcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpfcxoff_i")) { ff_txpfcxoff_i(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_sta_mac_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txafull_o")) { ff_txafull_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdessigok_o")) { ff_serdessigok_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txidle_o")) { ff_txidle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txgood_o")) { ff_txgood_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txread_o")) { ff_txread_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxsync_o")) { ff_rxsync_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxpfcxoff_o")) { ff_rxpfcxoff_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxpreamble_o")) { ff_rxpreamble_o(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_serdes_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rxsigok_i_sel")) { rxsigok_i_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxsigok_i")) { rxsigok_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tx_override_in")) { tx_override_in(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_cfg_mac_gbl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txdispad_i")) { ff_txdispad_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_fcs_enable")) { pkt_fcs_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx_ipg")) { ff_tx_ipg(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpreamble_i")) { ff_txpreamble_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_err_gen_enable")) { rx_err_gen_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_err_mask")) { rx_err_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "umacreg_rxdisfcschk")) { umacreg_rxdisfcschk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_bx_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_gbl.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_serdes.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_xoff.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_interrupt.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_interrupt_request.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_sd_core_to_cntl.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac_sd_interrupt_status.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac_sd_status.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fixer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ff_txfifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_txfifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ff_rxfifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_rxfifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_stats_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stats_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_mac_stats.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mac.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_int_mac_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lane_sbe_enable");
    ret_vec.push_back("lane_dbe_enable");
    ret_vec.push_back("xdmac10_intr_enable");
    ret_vec.push_back("xdmac10_pslverr_enable");
    ret_vec.push_back("bxpb_pbus_drdy_enable");
    ret_vec.push_back("rx_missing_sof_enable");
    ret_vec.push_back("rx_missing_eof_enable");
    ret_vec.push_back("rx_timeout_err_enable");
    ret_vec.push_back("rx_min_size_err_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lane_sbe_interrupt");
    ret_vec.push_back("lane_dbe_interrupt");
    ret_vec.push_back("xdmac10_intr_interrupt");
    ret_vec.push_back("xdmac10_pslverr_interrupt");
    ret_vec.push_back("bxpb_pbus_drdy_interrupt");
    ret_vec.push_back("rx_missing_sof_interrupt");
    ret_vec.push_back("rx_missing_eof_interrupt");
    ret_vec.push_back("rx_timeout_err_interrupt");
    ret_vec.push_back("rx_min_size_err_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_intgrp_t::get_fields(int level) const { 
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
std::vector<string> cap_bx_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mac_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mac_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_bx_csr_dhs_apb_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_dhs_apb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_dhs_mac_stats_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_dhs_mac_stats_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_stats_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_stats_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_ff_rxfifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_ff_rxfifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_ff_txfifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_ff_txfifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_debug_port_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("select");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_fixer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("timeout");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_mac_sd_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("rx_idle_detect");
    ret_vec.push_back("rx_rdy");
    ret_vec.push_back("tx_rdy");
    ret_vec.push_back("core_status");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_mac_sd_interrupt_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("in_progress");
    ret_vec.push_back("data_out");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_mac_sd_core_to_cntl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_mac_sd_interrupt_request_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lane");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_mac_sd_interrupt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("code");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_mac_xoff_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ff_txfcxoff_i");
    ret_vec.push_back("ff_txpfcxoff_i");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_sta_mac_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ff_txafull_o");
    ret_vec.push_back("ff_serdessigok_o");
    ret_vec.push_back("ff_txidle_o");
    ret_vec.push_back("ff_txgood_o");
    ret_vec.push_back("ff_txread_o");
    ret_vec.push_back("ff_rxsync_o");
    ret_vec.push_back("ff_rxpfcxoff_o");
    ret_vec.push_back("ff_rxpreamble_o");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_serdes_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("rxsigok_i_sel");
    ret_vec.push_back("rxsigok_i");
    ret_vec.push_back("tx_override_in");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_cfg_mac_gbl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ff_txdispad_i");
    ret_vec.push_back("pkt_fcs_enable");
    ret_vec.push_back("ff_tx_ipg");
    ret_vec.push_back("ff_txpreamble_i");
    ret_vec.push_back("rx_err_gen_enable");
    ret_vec.push_back("rx_err_mask");
    ret_vec.push_back("umacreg_rxdisfcschk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_bx_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac_gbl.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_gbl." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_serdes.get_fields(level-1)) {
            ret_vec.push_back("cfg_serdes." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mac.get_fields(level-1)) {
            ret_vec.push_back("sta_mac." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac_xoff.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_xoff." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac_sd_interrupt.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_sd_interrupt." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac_sd_interrupt_request.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_sd_interrupt_request." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac_sd_core_to_cntl.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_sd_core_to_cntl." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mac_sd_interrupt_status.get_fields(level-1)) {
            ret_vec.push_back("sta_mac_sd_interrupt_status." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mac_sd_status.get_fields(level-1)) {
            ret_vec.push_back("sta_mac_sd_status." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_fixer.get_fields(level-1)) {
            ret_vec.push_back("cfg_fixer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_debug_port.get_fields(level-1)) {
            ret_vec.push_back("cfg_debug_port." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_ff_txfifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_ff_txfifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_txfifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_txfifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_ff_rxfifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_ff_rxfifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_rxfifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_rxfifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_stats_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_stats_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_stats_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_stats_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_mac_stats.get_fields(level-1)) {
            ret_vec.push_back("dhs_mac_stats." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_apb.get_fields(level-1)) {
            ret_vec.push_back("dhs_apb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_mac.get_fields(level-1)) {
            ret_vec.push_back("int_mac." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
