
#include "cap_mx_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_mx_csr_int_ecc_int_enable_clear_t::cap_mx_csr_int_ecc_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_ecc_int_enable_clear_t::~cap_mx_csr_int_ecc_int_enable_clear_t() { }

cap_mx_csr_int_ecc_int_test_set_t::cap_mx_csr_int_ecc_int_test_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_ecc_int_test_set_t::~cap_mx_csr_int_ecc_int_test_set_t() { }

cap_mx_csr_int_ecc_t::cap_mx_csr_int_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_ecc_t::~cap_mx_csr_int_ecc_t() { }

cap_mx_csr_int_mac_int_enable_clear_t::cap_mx_csr_int_mac_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_mac_int_enable_clear_t::~cap_mx_csr_int_mac_int_enable_clear_t() { }

cap_mx_csr_int_mac_intreg_t::cap_mx_csr_int_mac_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_mac_intreg_t::~cap_mx_csr_int_mac_intreg_t() { }

cap_mx_csr_int_mac_t::cap_mx_csr_int_mac_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_mac_t::~cap_mx_csr_int_mac_t() { }

cap_mx_csr_intreg_status_t::cap_mx_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_intreg_status_t::~cap_mx_csr_intreg_status_t() { }

cap_mx_csr_int_groups_int_enable_rw_reg_t::cap_mx_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_int_groups_int_enable_rw_reg_t::~cap_mx_csr_int_groups_int_enable_rw_reg_t() { }

cap_mx_csr_intgrp_status_t::cap_mx_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_intgrp_status_t::~cap_mx_csr_intgrp_status_t() { }

cap_mx_csr_dhs_mac_stats_entry_t::cap_mx_csr_dhs_mac_stats_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_dhs_mac_stats_entry_t::~cap_mx_csr_dhs_mac_stats_entry_t() { }

cap_mx_csr_dhs_mac_stats_t::cap_mx_csr_dhs_mac_stats_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mx_csr_dhs_mac_stats_t::~cap_mx_csr_dhs_mac_stats_t() { }

cap_mx_csr_dhs_apb_entry_t::cap_mx_csr_dhs_apb_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_dhs_apb_entry_t::~cap_mx_csr_dhs_apb_entry_t() { }

cap_mx_csr_dhs_apb_t::cap_mx_csr_dhs_apb_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mx_csr_dhs_apb_t::~cap_mx_csr_dhs_apb_t() { }

cap_mx_csr_sta_ecc_t::cap_mx_csr_sta_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_ecc_t::~cap_mx_csr_sta_ecc_t() { }

cap_mx_csr_sta_fec_mem_t::cap_mx_csr_sta_fec_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_fec_mem_t::~cap_mx_csr_sta_fec_mem_t() { }

cap_mx_csr_cfg_fec_mem_t::cap_mx_csr_cfg_fec_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_fec_mem_t::~cap_mx_csr_cfg_fec_mem_t() { }

cap_mx_csr_sta_stats_mem_t::cap_mx_csr_sta_stats_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_stats_mem_t::~cap_mx_csr_sta_stats_mem_t() { }

cap_mx_csr_cfg_stats_mem_t::cap_mx_csr_cfg_stats_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_stats_mem_t::~cap_mx_csr_cfg_stats_mem_t() { }

cap_mx_csr_sta_rxfifo_mem_t::cap_mx_csr_sta_rxfifo_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_rxfifo_mem_t::~cap_mx_csr_sta_rxfifo_mem_t() { }

cap_mx_csr_cfg_rxfifo_mem_t::cap_mx_csr_cfg_rxfifo_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_rxfifo_mem_t::~cap_mx_csr_cfg_rxfifo_mem_t() { }

cap_mx_csr_sta_txfifo_mem_t::cap_mx_csr_sta_txfifo_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_txfifo_mem_t::~cap_mx_csr_sta_txfifo_mem_t() { }

cap_mx_csr_cfg_txfifo_mem_t::cap_mx_csr_cfg_txfifo_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_txfifo_mem_t::~cap_mx_csr_cfg_txfifo_mem_t() { }

cap_mx_csr_cfg_debug_port_t::cap_mx_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_debug_port_t::~cap_mx_csr_cfg_debug_port_t() { }

cap_mx_csr_cfg_fixer_t::cap_mx_csr_cfg_fixer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_fixer_t::~cap_mx_csr_cfg_fixer_t() { }

cap_mx_csr_csr_intr_t::cap_mx_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_csr_intr_t::~cap_mx_csr_csr_intr_t() { }

cap_mx_csr_sta_mac_sd_status_t::cap_mx_csr_sta_mac_sd_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_mac_sd_status_t::~cap_mx_csr_sta_mac_sd_status_t() { }

cap_mx_csr_sta_mac_sd_interrupt_status_t::cap_mx_csr_sta_mac_sd_interrupt_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_mac_sd_interrupt_status_t::~cap_mx_csr_sta_mac_sd_interrupt_status_t() { }

cap_mx_csr_cfg_mac_sd_core_to_cntl_t::cap_mx_csr_cfg_mac_sd_core_to_cntl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_sd_core_to_cntl_t::~cap_mx_csr_cfg_mac_sd_core_to_cntl_t() { }

cap_mx_csr_cfg_mac_sd_interrupt_request_t::cap_mx_csr_cfg_mac_sd_interrupt_request_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_sd_interrupt_request_t::~cap_mx_csr_cfg_mac_sd_interrupt_request_t() { }

cap_mx_csr_cfg_mac_sd_interrupt_t::cap_mx_csr_cfg_mac_sd_interrupt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_sd_interrupt_t::~cap_mx_csr_cfg_mac_sd_interrupt_t() { }

cap_mx_csr_cfg_mac_xoff_t::cap_mx_csr_cfg_mac_xoff_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_xoff_t::~cap_mx_csr_cfg_mac_xoff_t() { }

cap_mx_csr_sta_mac_t::cap_mx_csr_sta_mac_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_sta_mac_t::~cap_mx_csr_sta_mac_t() { }

cap_mx_csr_cfg_mac_t::cap_mx_csr_cfg_mac_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_t::~cap_mx_csr_cfg_mac_t() { }

cap_mx_csr_cfg_mac_tdm_t::cap_mx_csr_cfg_mac_tdm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_tdm_t::~cap_mx_csr_cfg_mac_tdm_t() { }

cap_mx_csr_cfg_mac_gbl_t::cap_mx_csr_cfg_mac_gbl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_cfg_mac_gbl_t::~cap_mx_csr_cfg_mac_gbl_t() { }

cap_mx_csr_base_t::cap_mx_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mx_csr_base_t::~cap_mx_csr_base_t() { }

cap_mx_csr_t::cap_mx_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_mx_csr_t::~cap_mx_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_ecc_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_enable: 0x" << int_var__uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_enable: 0x" << int_var__correctable_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_ecc_int_test_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_interrupt: 0x" << int_var__uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_interrupt: 0x" << int_var__correctable_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_mac_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lane0_sbe_enable: 0x" << int_var__lane0_sbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane0_dbe_enable: 0x" << int_var__lane0_dbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1_sbe_enable: 0x" << int_var__lane1_sbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1_dbe_enable: 0x" << int_var__lane1_dbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2_sbe_enable: 0x" << int_var__lane2_sbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2_dbe_enable: 0x" << int_var__lane2_dbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3_sbe_enable: 0x" << int_var__lane3_sbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3_dbe_enable: 0x" << int_var__lane3_dbe_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".umac_CFG3_intr_enable: 0x" << int_var__umac_CFG3_intr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".umac_CFG3_pslverr_enable: 0x" << int_var__umac_CFG3_pslverr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m0pb_pbus_drdy_enable: 0x" << int_var__m0pb_pbus_drdy_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m1pb_pbus_drdy_enable: 0x" << int_var__m1pb_pbus_drdy_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m2pb_pbus_drdy_enable: 0x" << int_var__m2pb_pbus_drdy_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m3pb_pbus_drdy_enable: 0x" << int_var__m3pb_pbus_drdy_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_missing_sof_enable: 0x" << int_var__rx0_missing_sof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_missing_eof_enable: 0x" << int_var__rx0_missing_eof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_timeout_err_enable: 0x" << int_var__rx0_timeout_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_min_size_err_enable: 0x" << int_var__rx0_min_size_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_missing_sof_enable: 0x" << int_var__rx1_missing_sof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_missing_eof_enable: 0x" << int_var__rx1_missing_eof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_timeout_err_enable: 0x" << int_var__rx1_timeout_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_min_size_err_enable: 0x" << int_var__rx1_min_size_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_missing_sof_enable: 0x" << int_var__rx2_missing_sof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_missing_eof_enable: 0x" << int_var__rx2_missing_eof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_timeout_err_enable: 0x" << int_var__rx2_timeout_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_min_size_err_enable: 0x" << int_var__rx2_min_size_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_missing_sof_enable: 0x" << int_var__rx3_missing_sof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_missing_eof_enable: 0x" << int_var__rx3_missing_eof_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_timeout_err_enable: 0x" << int_var__rx3_timeout_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_min_size_err_enable: 0x" << int_var__rx3_min_size_err_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_mac_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lane0_sbe_interrupt: 0x" << int_var__lane0_sbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane0_dbe_interrupt: 0x" << int_var__lane0_dbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1_sbe_interrupt: 0x" << int_var__lane1_sbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1_dbe_interrupt: 0x" << int_var__lane1_dbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2_sbe_interrupt: 0x" << int_var__lane2_sbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2_dbe_interrupt: 0x" << int_var__lane2_dbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3_sbe_interrupt: 0x" << int_var__lane3_sbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3_dbe_interrupt: 0x" << int_var__lane3_dbe_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".umac_CFG3_intr_interrupt: 0x" << int_var__umac_CFG3_intr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".umac_CFG3_pslverr_interrupt: 0x" << int_var__umac_CFG3_pslverr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m0pb_pbus_drdy_interrupt: 0x" << int_var__m0pb_pbus_drdy_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m1pb_pbus_drdy_interrupt: 0x" << int_var__m1pb_pbus_drdy_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m2pb_pbus_drdy_interrupt: 0x" << int_var__m2pb_pbus_drdy_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".m3pb_pbus_drdy_interrupt: 0x" << int_var__m3pb_pbus_drdy_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_missing_sof_interrupt: 0x" << int_var__rx0_missing_sof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_missing_eof_interrupt: 0x" << int_var__rx0_missing_eof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_timeout_err_interrupt: 0x" << int_var__rx0_timeout_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx0_min_size_err_interrupt: 0x" << int_var__rx0_min_size_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_missing_sof_interrupt: 0x" << int_var__rx1_missing_sof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_missing_eof_interrupt: 0x" << int_var__rx1_missing_eof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_timeout_err_interrupt: 0x" << int_var__rx1_timeout_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx1_min_size_err_interrupt: 0x" << int_var__rx1_min_size_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_missing_sof_interrupt: 0x" << int_var__rx2_missing_sof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_missing_eof_interrupt: 0x" << int_var__rx2_missing_eof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_timeout_err_interrupt: 0x" << int_var__rx2_timeout_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx2_min_size_err_interrupt: 0x" << int_var__rx2_min_size_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_missing_sof_interrupt: 0x" << int_var__rx3_missing_sof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_missing_eof_interrupt: 0x" << int_var__rx3_missing_eof_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_timeout_err_interrupt: 0x" << int_var__rx3_timeout_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx3_min_size_err_interrupt: 0x" << int_var__rx3_min_size_err_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_mac_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mac_interrupt: 0x" << int_var__int_mac_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_ecc_interrupt: 0x" << int_var__int_ecc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mac_enable: 0x" << int_var__int_mac_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_ecc_enable: 0x" << int_var__int_ecc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_dhs_mac_stats_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_dhs_mac_stats_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_dhs_apb_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_dhs_apb_t::show() {
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
void cap_mx_csr_sta_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_fec_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_fec_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_stats_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_stats_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_rxfifo_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_rxfifo_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_txfifo_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_txfifo_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_debug_port_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_fixer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".timeout: 0x" << int_var__timeout << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_mac_sd_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rx_idle_detect: 0x" << int_var__rx_idle_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_rdy: 0x" << int_var__rx_rdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tx_rdy: 0x" << int_var__tx_rdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".core_status: 0x" << int_var__core_status << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_mac_sd_interrupt_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".in_progress: 0x" << int_var__in_progress << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_out: 0x" << int_var__data_out << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_sd_core_to_cntl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_sd_interrupt_request_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_sd_interrupt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".code: 0x" << int_var__code << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_xoff_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx0fcxoff_i: 0x" << int_var__ff_tx0fcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx0pfcxoff_i: 0x" << int_var__ff_tx0pfcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx1fcxoff_i: 0x" << int_var__ff_tx1fcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx1pfcxoff_i: 0x" << int_var__ff_tx1pfcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx2fcxoff_i: 0x" << int_var__ff_tx2fcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx2pfcxoff_i: 0x" << int_var__ff_tx2pfcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx3fcxoff_i: 0x" << int_var__ff_tx3fcxoff_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx3pfcxoff_i: 0x" << int_var__ff_tx3pfcxoff_i << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_sta_mac_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorecontrol0_o: 0x" << int_var__serdes_rxcorecontrol0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorecontrol0_o: 0x" << int_var__serdes_txcorecontrol0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txafull0_o: 0x" << int_var__ff_txafull0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_serdes0sigok_o: 0x" << int_var__ff_serdes0sigok_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx0sync_o: 0x" << int_var__ff_rx0sync_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx0idle_o: 0x" << int_var__ff_tx0idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx0idle_o: 0x" << int_var__ff_rx0idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx0good_o: 0x" << int_var__ff_tx0good_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txquiet_ts_0_o: 0x" << int_var__serdes_txquiet_ts_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxquiet_rs_0_o: 0x" << int_var__serdes_rxquiet_rs_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx0read_o: 0x" << int_var__ff_tx0read_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_0_o: 0x" << int_var__ff_txlpi_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxlpi_0_o: 0x" << int_var__ff_rxlpi_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxlpiactive_rs_0_o: 0x" << int_var__serdes_rxlpiactive_rs_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txalert_ts_0_o: 0x" << int_var__serdes_txalert_ts_0_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorecontrol1_o: 0x" << int_var__serdes_rxcorecontrol1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorecontrol1_o: 0x" << int_var__serdes_txcorecontrol1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txafull1_o: 0x" << int_var__ff_txafull1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_serdes1sigok_o: 0x" << int_var__ff_serdes1sigok_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx1sync_o: 0x" << int_var__ff_rx1sync_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx1idle_o: 0x" << int_var__ff_tx1idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx1idle_o: 0x" << int_var__ff_rx1idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx1good_o: 0x" << int_var__ff_tx1good_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txquiet_ts_1_o: 0x" << int_var__serdes_txquiet_ts_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxquiet_rs_1_o: 0x" << int_var__serdes_rxquiet_rs_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx1read_o: 0x" << int_var__ff_tx1read_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_1_o: 0x" << int_var__ff_txlpi_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxlpi_1_o: 0x" << int_var__ff_rxlpi_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxlpiactive_rs_1_o: 0x" << int_var__serdes_rxlpiactive_rs_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txalert_ts_1_o: 0x" << int_var__serdes_txalert_ts_1_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorecontrol2_o: 0x" << int_var__serdes_rxcorecontrol2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorecontrol2_o: 0x" << int_var__serdes_txcorecontrol2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txafull2_o: 0x" << int_var__ff_txafull2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_serdes2sigok_o: 0x" << int_var__ff_serdes2sigok_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx2sync_o: 0x" << int_var__ff_rx2sync_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx2idle_o: 0x" << int_var__ff_tx2idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx2idle_o: 0x" << int_var__ff_rx2idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx2good_o: 0x" << int_var__ff_tx2good_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txquiet_ts_2_o: 0x" << int_var__serdes_txquiet_ts_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxquiet_rs_2_o: 0x" << int_var__serdes_rxquiet_rs_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx2read_o: 0x" << int_var__ff_tx2read_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_2_o: 0x" << int_var__ff_txlpi_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxlpi_2_o: 0x" << int_var__ff_rxlpi_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxlpiactive_rs_2_o: 0x" << int_var__serdes_rxlpiactive_rs_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txalert_ts_2_o: 0x" << int_var__serdes_txalert_ts_2_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorecontrol3_o: 0x" << int_var__serdes_rxcorecontrol3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorecontrol3_o: 0x" << int_var__serdes_txcorecontrol3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txafull3_o: 0x" << int_var__ff_txafull3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_serdes3sigok_o: 0x" << int_var__ff_serdes3sigok_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx3sync_o: 0x" << int_var__ff_rx3sync_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx3idle_o: 0x" << int_var__ff_tx3idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rx3idle_o: 0x" << int_var__ff_rx3idle_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx3good_o: 0x" << int_var__ff_tx3good_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txquiet_ts_3_o: 0x" << int_var__serdes_txquiet_ts_3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxquiet_rs_3_o: 0x" << int_var__serdes_rxquiet_rs_3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx3read_o: 0x" << int_var__ff_tx3read_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_3_o: 0x" << int_var__ff_txlpi_3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_rxlpi_3_o: 0x" << int_var__ff_rxlpi_3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxlpiactive_rs_3_o: 0x" << int_var__serdes_rxlpiactive_rs_3_o << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txalert_ts_3_o: 0x" << int_var__serdes_txalert_ts_3_o << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mac_rxsigok_i_sel: 0x" << int_var__mac_rxsigok_i_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mac_rxenergydetect_rs_i_sel: 0x" << int_var__mac_rxenergydetect_rs_i_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mac_tx_corestatus_i_sel: 0x" << int_var__mac_tx_corestatus_i_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mac_rx_corestatus_i_sel: 0x" << int_var__mac_rx_corestatus_i_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".core_to_cntl_7_5_sel: 0x" << int_var__core_to_cntl_7_5_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".core_to_cntl_9_8_sel: 0x" << int_var__core_to_cntl_9_8_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".core_to_cntl_15_10_sel: 0x" << int_var__core_to_cntl_15_10_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes0_rxsigok_i: 0x" << int_var__serdes0_rxsigok_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes0_tx_override_in: 0x" << int_var__serdes0_tx_override_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxenergydetect_rs_0_i: 0x" << int_var__serdes_rxenergydetect_rs_0_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorestatus_0_i: 0x" << int_var__serdes_txcorestatus_0_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorestatus_0_i: 0x" << int_var__serdes_rxcorestatus_0_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes1_rxsigok_i: 0x" << int_var__serdes1_rxsigok_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes1_tx_override_in: 0x" << int_var__serdes1_tx_override_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxenergydetect_rs_1_i: 0x" << int_var__serdes_rxenergydetect_rs_1_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorestatus_1_i: 0x" << int_var__serdes_txcorestatus_1_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorestatus_1_i: 0x" << int_var__serdes_rxcorestatus_1_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes2_rxsigok_i: 0x" << int_var__serdes2_rxsigok_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes2_tx_override_in: 0x" << int_var__serdes2_tx_override_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxenergydetect_rs_2_i: 0x" << int_var__serdes_rxenergydetect_rs_2_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorestatus_2_i: 0x" << int_var__serdes_txcorestatus_2_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorestatus_2_i: 0x" << int_var__serdes_rxcorestatus_2_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes3_rxsigok_i: 0x" << int_var__serdes3_rxsigok_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes3_tx_override_in: 0x" << int_var__serdes3_tx_override_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxenergydetect_rs_3_i: 0x" << int_var__serdes_rxenergydetect_rs_3_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_txcorestatus_3_i: 0x" << int_var__serdes_txcorestatus_3_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".serdes_rxcorestatus_3_i: 0x" << int_var__serdes_rxcorestatus_3_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_0_i: 0x" << int_var__ff_txlpi_0_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_1_i: 0x" << int_var__ff_txlpi_1_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_2_i: 0x" << int_var__ff_txlpi_2_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txlpi_3_i: 0x" << int_var__ff_txlpi_3_i << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_tdm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".slot0: 0x" << int_var__slot0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slot1: 0x" << int_var__slot1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slot2: 0x" << int_var__slot2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slot3: 0x" << int_var__slot3 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_cfg_mac_gbl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txdispad_i: 0x" << int_var__ff_txdispad_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_fcs_enable: 0x" << int_var__pkt_fcs_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_tx_ipg: 0x" << int_var__ff_tx_ipg << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txtimestampena_i: 0x" << int_var__ff_txtimestampena_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txtimestampid_i: 0x" << int_var__ff_txtimestampid_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cg_fec_enable_i: 0x" << int_var__cg_fec_enable_i << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ff_txpreamble_i: 0x" << int_var__ff_txpreamble_i << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mx_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_mac_gbl.show();
    cfg_mac_tdm.show();
    cfg_mac.show();
    sta_mac.show();
    cfg_mac_xoff.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_interrupt.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_interrupt_request.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt_request[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_core_to_cntl.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_core_to_cntl[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mac_sd_interrupt_status.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_interrupt_status[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mac_sd_status.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_status[ii].show();
    }
    #endif
    
    csr_intr.show();
    cfg_fixer.show();
    cfg_debug_port.show();
    cfg_txfifo_mem.show();
    sta_txfifo_mem.show();
    cfg_rxfifo_mem.show();
    sta_rxfifo_mem.show();
    cfg_stats_mem.show();
    sta_stats_mem.show();
    cfg_fec_mem.show();
    sta_fec_mem.show();
    sta_ecc.show();
    dhs_apb.show();
    dhs_mac_stats.show();
    int_groups.show();
    int_mac.show();
    int_ecc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_mx_csr_int_ecc_int_enable_clear_t::get_width() const {
    return cap_mx_csr_int_ecc_int_enable_clear_t::s_get_width();

}

int cap_mx_csr_int_ecc_int_test_set_t::get_width() const {
    return cap_mx_csr_int_ecc_int_test_set_t::s_get_width();

}

int cap_mx_csr_int_ecc_t::get_width() const {
    return cap_mx_csr_int_ecc_t::s_get_width();

}

int cap_mx_csr_int_mac_int_enable_clear_t::get_width() const {
    return cap_mx_csr_int_mac_int_enable_clear_t::s_get_width();

}

int cap_mx_csr_int_mac_intreg_t::get_width() const {
    return cap_mx_csr_int_mac_intreg_t::s_get_width();

}

int cap_mx_csr_int_mac_t::get_width() const {
    return cap_mx_csr_int_mac_t::s_get_width();

}

int cap_mx_csr_intreg_status_t::get_width() const {
    return cap_mx_csr_intreg_status_t::s_get_width();

}

int cap_mx_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_mx_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_mx_csr_intgrp_status_t::get_width() const {
    return cap_mx_csr_intgrp_status_t::s_get_width();

}

int cap_mx_csr_dhs_mac_stats_entry_t::get_width() const {
    return cap_mx_csr_dhs_mac_stats_entry_t::s_get_width();

}

int cap_mx_csr_dhs_mac_stats_t::get_width() const {
    return cap_mx_csr_dhs_mac_stats_t::s_get_width();

}

int cap_mx_csr_dhs_apb_entry_t::get_width() const {
    return cap_mx_csr_dhs_apb_entry_t::s_get_width();

}

int cap_mx_csr_dhs_apb_t::get_width() const {
    return cap_mx_csr_dhs_apb_t::s_get_width();

}

int cap_mx_csr_sta_ecc_t::get_width() const {
    return cap_mx_csr_sta_ecc_t::s_get_width();

}

int cap_mx_csr_sta_fec_mem_t::get_width() const {
    return cap_mx_csr_sta_fec_mem_t::s_get_width();

}

int cap_mx_csr_cfg_fec_mem_t::get_width() const {
    return cap_mx_csr_cfg_fec_mem_t::s_get_width();

}

int cap_mx_csr_sta_stats_mem_t::get_width() const {
    return cap_mx_csr_sta_stats_mem_t::s_get_width();

}

int cap_mx_csr_cfg_stats_mem_t::get_width() const {
    return cap_mx_csr_cfg_stats_mem_t::s_get_width();

}

int cap_mx_csr_sta_rxfifo_mem_t::get_width() const {
    return cap_mx_csr_sta_rxfifo_mem_t::s_get_width();

}

int cap_mx_csr_cfg_rxfifo_mem_t::get_width() const {
    return cap_mx_csr_cfg_rxfifo_mem_t::s_get_width();

}

int cap_mx_csr_sta_txfifo_mem_t::get_width() const {
    return cap_mx_csr_sta_txfifo_mem_t::s_get_width();

}

int cap_mx_csr_cfg_txfifo_mem_t::get_width() const {
    return cap_mx_csr_cfg_txfifo_mem_t::s_get_width();

}

int cap_mx_csr_cfg_debug_port_t::get_width() const {
    return cap_mx_csr_cfg_debug_port_t::s_get_width();

}

int cap_mx_csr_cfg_fixer_t::get_width() const {
    return cap_mx_csr_cfg_fixer_t::s_get_width();

}

int cap_mx_csr_csr_intr_t::get_width() const {
    return cap_mx_csr_csr_intr_t::s_get_width();

}

int cap_mx_csr_sta_mac_sd_status_t::get_width() const {
    return cap_mx_csr_sta_mac_sd_status_t::s_get_width();

}

int cap_mx_csr_sta_mac_sd_interrupt_status_t::get_width() const {
    return cap_mx_csr_sta_mac_sd_interrupt_status_t::s_get_width();

}

int cap_mx_csr_cfg_mac_sd_core_to_cntl_t::get_width() const {
    return cap_mx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width();

}

int cap_mx_csr_cfg_mac_sd_interrupt_request_t::get_width() const {
    return cap_mx_csr_cfg_mac_sd_interrupt_request_t::s_get_width();

}

int cap_mx_csr_cfg_mac_sd_interrupt_t::get_width() const {
    return cap_mx_csr_cfg_mac_sd_interrupt_t::s_get_width();

}

int cap_mx_csr_cfg_mac_xoff_t::get_width() const {
    return cap_mx_csr_cfg_mac_xoff_t::s_get_width();

}

int cap_mx_csr_sta_mac_t::get_width() const {
    return cap_mx_csr_sta_mac_t::s_get_width();

}

int cap_mx_csr_cfg_mac_t::get_width() const {
    return cap_mx_csr_cfg_mac_t::s_get_width();

}

int cap_mx_csr_cfg_mac_tdm_t::get_width() const {
    return cap_mx_csr_cfg_mac_tdm_t::s_get_width();

}

int cap_mx_csr_cfg_mac_gbl_t::get_width() const {
    return cap_mx_csr_cfg_mac_gbl_t::s_get_width();

}

int cap_mx_csr_base_t::get_width() const {
    return cap_mx_csr_base_t::s_get_width();

}

int cap_mx_csr_t::get_width() const {
    return cap_mx_csr_t::s_get_width();

}

int cap_mx_csr_int_ecc_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable_enable
    _count += 1; // correctable_enable
    return _count;
}

int cap_mx_csr_int_ecc_int_test_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable_interrupt
    _count += 1; // correctable_interrupt
    return _count;
}

int cap_mx_csr_int_ecc_t::s_get_width() {
    int _count = 0;

    _count += cap_mx_csr_int_ecc_int_test_set_t::s_get_width(); // intreg
    _count += cap_mx_csr_int_ecc_int_test_set_t::s_get_width(); // int_test_set
    _count += cap_mx_csr_int_ecc_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mx_csr_int_ecc_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mx_csr_int_mac_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // lane0_sbe_enable
    _count += 1; // lane0_dbe_enable
    _count += 1; // lane1_sbe_enable
    _count += 1; // lane1_dbe_enable
    _count += 1; // lane2_sbe_enable
    _count += 1; // lane2_dbe_enable
    _count += 1; // lane3_sbe_enable
    _count += 1; // lane3_dbe_enable
    _count += 1; // umac_CFG3_intr_enable
    _count += 1; // umac_CFG3_pslverr_enable
    _count += 1; // m0pb_pbus_drdy_enable
    _count += 1; // m1pb_pbus_drdy_enable
    _count += 1; // m2pb_pbus_drdy_enable
    _count += 1; // m3pb_pbus_drdy_enable
    _count += 1; // rx0_missing_sof_enable
    _count += 1; // rx0_missing_eof_enable
    _count += 1; // rx0_timeout_err_enable
    _count += 1; // rx0_min_size_err_enable
    _count += 1; // rx1_missing_sof_enable
    _count += 1; // rx1_missing_eof_enable
    _count += 1; // rx1_timeout_err_enable
    _count += 1; // rx1_min_size_err_enable
    _count += 1; // rx2_missing_sof_enable
    _count += 1; // rx2_missing_eof_enable
    _count += 1; // rx2_timeout_err_enable
    _count += 1; // rx2_min_size_err_enable
    _count += 1; // rx3_missing_sof_enable
    _count += 1; // rx3_missing_eof_enable
    _count += 1; // rx3_timeout_err_enable
    _count += 1; // rx3_min_size_err_enable
    return _count;
}

int cap_mx_csr_int_mac_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // lane0_sbe_interrupt
    _count += 1; // lane0_dbe_interrupt
    _count += 1; // lane1_sbe_interrupt
    _count += 1; // lane1_dbe_interrupt
    _count += 1; // lane2_sbe_interrupt
    _count += 1; // lane2_dbe_interrupt
    _count += 1; // lane3_sbe_interrupt
    _count += 1; // lane3_dbe_interrupt
    _count += 1; // umac_CFG3_intr_interrupt
    _count += 1; // umac_CFG3_pslverr_interrupt
    _count += 1; // m0pb_pbus_drdy_interrupt
    _count += 1; // m1pb_pbus_drdy_interrupt
    _count += 1; // m2pb_pbus_drdy_interrupt
    _count += 1; // m3pb_pbus_drdy_interrupt
    _count += 1; // rx0_missing_sof_interrupt
    _count += 1; // rx0_missing_eof_interrupt
    _count += 1; // rx0_timeout_err_interrupt
    _count += 1; // rx0_min_size_err_interrupt
    _count += 1; // rx1_missing_sof_interrupt
    _count += 1; // rx1_missing_eof_interrupt
    _count += 1; // rx1_timeout_err_interrupt
    _count += 1; // rx1_min_size_err_interrupt
    _count += 1; // rx2_missing_sof_interrupt
    _count += 1; // rx2_missing_eof_interrupt
    _count += 1; // rx2_timeout_err_interrupt
    _count += 1; // rx2_min_size_err_interrupt
    _count += 1; // rx3_missing_sof_interrupt
    _count += 1; // rx3_missing_eof_interrupt
    _count += 1; // rx3_timeout_err_interrupt
    _count += 1; // rx3_min_size_err_interrupt
    return _count;
}

int cap_mx_csr_int_mac_t::s_get_width() {
    int _count = 0;

    _count += cap_mx_csr_int_mac_intreg_t::s_get_width(); // intreg
    _count += cap_mx_csr_int_mac_intreg_t::s_get_width(); // int_test_set
    _count += cap_mx_csr_int_mac_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mx_csr_int_mac_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mx_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mac_interrupt
    _count += 1; // int_ecc_interrupt
    return _count;
}

int cap_mx_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mac_enable
    _count += 1; // int_ecc_enable
    return _count;
}

int cap_mx_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_mx_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_mx_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_mx_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_mx_csr_dhs_mac_stats_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mx_csr_dhs_mac_stats_t::s_get_width() {
    int _count = 0;

    _count += (cap_mx_csr_dhs_mac_stats_entry_t::s_get_width() * 512); // entry
    return _count;
}

int cap_mx_csr_dhs_apb_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_mx_csr_dhs_apb_t::s_get_width() {
    int _count = 0;

    _count += (cap_mx_csr_dhs_apb_entry_t::s_get_width() * 65536); // entry
    return _count;
}

int cap_mx_csr_sta_ecc_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 9; // syndrome
    _count += 10; // addr
    return _count;
}

int cap_mx_csr_sta_fec_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mx_csr_cfg_fec_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    _count += 1; // ecc_disable_det
    _count += 1; // ecc_disable_cor
    return _count;
}

int cap_mx_csr_sta_stats_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mx_csr_cfg_stats_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    _count += 1; // ecc_disable_det
    _count += 1; // ecc_disable_cor
    return _count;
}

int cap_mx_csr_sta_rxfifo_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mx_csr_cfg_rxfifo_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mx_csr_sta_txfifo_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mx_csr_cfg_txfifo_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mx_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // select
    return _count;
}

int cap_mx_csr_cfg_fixer_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 16; // timeout
    return _count;
}

int cap_mx_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_mx_csr_sta_mac_sd_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // rx_idle_detect
    _count += 1; // rx_rdy
    _count += 1; // tx_rdy
    _count += 32; // core_status
    return _count;
}

int cap_mx_csr_sta_mac_sd_interrupt_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // in_progress
    _count += 16; // data_out
    return _count;
}

int cap_mx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width() {
    int _count = 0;

    _count += 16; // value
    return _count;
}

int cap_mx_csr_cfg_mac_sd_interrupt_request_t::s_get_width() {
    int _count = 0;

    _count += 1; // val
    return _count;
}

int cap_mx_csr_cfg_mac_sd_interrupt_t::s_get_width() {
    int _count = 0;

    _count += 16; // code
    _count += 16; // data
    return _count;
}

int cap_mx_csr_cfg_mac_xoff_t::s_get_width() {
    int _count = 0;

    _count += 1; // ff_tx0fcxoff_i
    _count += 8; // ff_tx0pfcxoff_i
    _count += 1; // ff_tx1fcxoff_i
    _count += 8; // ff_tx1pfcxoff_i
    _count += 1; // ff_tx2fcxoff_i
    _count += 8; // ff_tx2pfcxoff_i
    _count += 1; // ff_tx3fcxoff_i
    _count += 8; // ff_tx3pfcxoff_i
    return _count;
}

int cap_mx_csr_sta_mac_t::s_get_width() {
    int _count = 0;

    _count += 2; // serdes_rxcorecontrol0_o
    _count += 6; // serdes_txcorecontrol0_o
    _count += 1; // ff_txafull0_o
    _count += 1; // ff_serdes0sigok_o
    _count += 1; // ff_rx0sync_o
    _count += 1; // ff_tx0idle_o
    _count += 1; // ff_rx0idle_o
    _count += 1; // ff_tx0good_o
    _count += 1; // serdes_txquiet_ts_0_o
    _count += 1; // serdes_rxquiet_rs_0_o
    _count += 1; // ff_tx0read_o
    _count += 1; // ff_txlpi_0_o
    _count += 1; // ff_rxlpi_0_o
    _count += 1; // serdes_rxlpiactive_rs_0_o
    _count += 1; // serdes_txalert_ts_0_o
    _count += 2; // serdes_rxcorecontrol1_o
    _count += 6; // serdes_txcorecontrol1_o
    _count += 1; // ff_txafull1_o
    _count += 1; // ff_serdes1sigok_o
    _count += 1; // ff_rx1sync_o
    _count += 1; // ff_tx1idle_o
    _count += 1; // ff_rx1idle_o
    _count += 1; // ff_tx1good_o
    _count += 1; // serdes_txquiet_ts_1_o
    _count += 1; // serdes_rxquiet_rs_1_o
    _count += 1; // ff_tx1read_o
    _count += 1; // ff_txlpi_1_o
    _count += 1; // ff_rxlpi_1_o
    _count += 1; // serdes_rxlpiactive_rs_1_o
    _count += 1; // serdes_txalert_ts_1_o
    _count += 2; // serdes_rxcorecontrol2_o
    _count += 6; // serdes_txcorecontrol2_o
    _count += 1; // ff_txafull2_o
    _count += 1; // ff_serdes2sigok_o
    _count += 1; // ff_rx2sync_o
    _count += 1; // ff_tx2idle_o
    _count += 1; // ff_rx2idle_o
    _count += 1; // ff_tx2good_o
    _count += 1; // serdes_txquiet_ts_2_o
    _count += 1; // serdes_rxquiet_rs_2_o
    _count += 1; // ff_tx2read_o
    _count += 1; // ff_txlpi_2_o
    _count += 1; // ff_rxlpi_2_o
    _count += 1; // serdes_rxlpiactive_rs_2_o
    _count += 1; // serdes_txalert_ts_2_o
    _count += 2; // serdes_rxcorecontrol3_o
    _count += 6; // serdes_txcorecontrol3_o
    _count += 1; // ff_txafull3_o
    _count += 1; // ff_serdes3sigok_o
    _count += 1; // ff_rx3sync_o
    _count += 1; // ff_tx3idle_o
    _count += 1; // ff_rx3idle_o
    _count += 1; // ff_tx3good_o
    _count += 1; // serdes_txquiet_ts_3_o
    _count += 1; // serdes_rxquiet_rs_3_o
    _count += 1; // ff_tx3read_o
    _count += 1; // ff_txlpi_3_o
    _count += 1; // ff_rxlpi_3_o
    _count += 1; // serdes_rxlpiactive_rs_3_o
    _count += 1; // serdes_txalert_ts_3_o
    return _count;
}

int cap_mx_csr_cfg_mac_t::s_get_width() {
    int _count = 0;

    _count += 1; // mac_rxsigok_i_sel
    _count += 1; // mac_rxenergydetect_rs_i_sel
    _count += 1; // mac_tx_corestatus_i_sel
    _count += 1; // mac_rx_corestatus_i_sel
    _count += 1; // core_to_cntl_7_5_sel
    _count += 1; // core_to_cntl_9_8_sel
    _count += 1; // core_to_cntl_15_10_sel
    _count += 1; // serdes0_rxsigok_i
    _count += 1; // serdes0_tx_override_in
    _count += 1; // serdes_rxenergydetect_rs_0_i
    _count += 2; // serdes_txcorestatus_0_i
    _count += 6; // serdes_rxcorestatus_0_i
    _count += 1; // serdes1_rxsigok_i
    _count += 1; // serdes1_tx_override_in
    _count += 1; // serdes_rxenergydetect_rs_1_i
    _count += 2; // serdes_txcorestatus_1_i
    _count += 6; // serdes_rxcorestatus_1_i
    _count += 1; // serdes2_rxsigok_i
    _count += 1; // serdes2_tx_override_in
    _count += 1; // serdes_rxenergydetect_rs_2_i
    _count += 2; // serdes_txcorestatus_2_i
    _count += 6; // serdes_rxcorestatus_2_i
    _count += 1; // serdes3_rxsigok_i
    _count += 1; // serdes3_tx_override_in
    _count += 1; // serdes_rxenergydetect_rs_3_i
    _count += 2; // serdes_txcorestatus_3_i
    _count += 6; // serdes_rxcorestatus_3_i
    _count += 1; // ff_txlpi_0_i
    _count += 1; // ff_txlpi_1_i
    _count += 1; // ff_txlpi_2_i
    _count += 1; // ff_txlpi_3_i
    return _count;
}

int cap_mx_csr_cfg_mac_tdm_t::s_get_width() {
    int _count = 0;

    _count += 3; // slot0
    _count += 3; // slot1
    _count += 3; // slot2
    _count += 3; // slot3
    return _count;
}

int cap_mx_csr_cfg_mac_gbl_t::s_get_width() {
    int _count = 0;

    _count += 1; // ff_txdispad_i
    _count += 1; // pkt_fcs_enable
    _count += 8; // ff_tx_ipg
    _count += 1; // ff_txtimestampena_i
    _count += 2; // ff_txtimestampid_i
    _count += 1; // cg_fec_enable_i
    _count += 56; // ff_txpreamble_i
    return _count;
}

int cap_mx_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_mx_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_mx_csr_base_t::s_get_width(); // base
    _count += cap_mx_csr_cfg_mac_gbl_t::s_get_width(); // cfg_mac_gbl
    _count += cap_mx_csr_cfg_mac_tdm_t::s_get_width(); // cfg_mac_tdm
    _count += cap_mx_csr_cfg_mac_t::s_get_width(); // cfg_mac
    _count += cap_mx_csr_sta_mac_t::s_get_width(); // sta_mac
    _count += cap_mx_csr_cfg_mac_xoff_t::s_get_width(); // cfg_mac_xoff
    _count += (cap_mx_csr_cfg_mac_sd_interrupt_t::s_get_width() * 4); // cfg_mac_sd_interrupt
    _count += (cap_mx_csr_cfg_mac_sd_interrupt_request_t::s_get_width() * 4); // cfg_mac_sd_interrupt_request
    _count += (cap_mx_csr_cfg_mac_sd_core_to_cntl_t::s_get_width() * 4); // cfg_mac_sd_core_to_cntl
    _count += (cap_mx_csr_sta_mac_sd_interrupt_status_t::s_get_width() * 4); // sta_mac_sd_interrupt_status
    _count += (cap_mx_csr_sta_mac_sd_status_t::s_get_width() * 4); // sta_mac_sd_status
    _count += cap_mx_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_mx_csr_cfg_fixer_t::s_get_width(); // cfg_fixer
    _count += cap_mx_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_mx_csr_cfg_txfifo_mem_t::s_get_width(); // cfg_txfifo_mem
    _count += cap_mx_csr_sta_txfifo_mem_t::s_get_width(); // sta_txfifo_mem
    _count += cap_mx_csr_cfg_rxfifo_mem_t::s_get_width(); // cfg_rxfifo_mem
    _count += cap_mx_csr_sta_rxfifo_mem_t::s_get_width(); // sta_rxfifo_mem
    _count += cap_mx_csr_cfg_stats_mem_t::s_get_width(); // cfg_stats_mem
    _count += cap_mx_csr_sta_stats_mem_t::s_get_width(); // sta_stats_mem
    _count += cap_mx_csr_cfg_fec_mem_t::s_get_width(); // cfg_fec_mem
    _count += cap_mx_csr_sta_fec_mem_t::s_get_width(); // sta_fec_mem
    _count += cap_mx_csr_sta_ecc_t::s_get_width(); // sta_ecc
    _count += cap_mx_csr_dhs_apb_t::s_get_width(); // dhs_apb
    _count += cap_mx_csr_dhs_mac_stats_t::s_get_width(); // dhs_mac_stats
    _count += cap_mx_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_mx_csr_int_mac_t::s_get_width(); // int_mac
    _count += cap_mx_csr_int_ecc_t::s_get_width(); // int_ecc
    return _count;
}

void cap_mx_csr_int_ecc_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable_enable = _val.convert_to< uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable_enable = _val.convert_to< correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_int_ecc_int_test_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable_interrupt = _val.convert_to< uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable_interrupt = _val.convert_to< correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_int_ecc_t::all(const cpp_int & in_val) {
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

void cap_mx_csr_int_mac_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lane0_sbe_enable = _val.convert_to< lane0_sbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane0_dbe_enable = _val.convert_to< lane0_dbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane1_sbe_enable = _val.convert_to< lane1_sbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane1_dbe_enable = _val.convert_to< lane1_dbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane2_sbe_enable = _val.convert_to< lane2_sbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane2_dbe_enable = _val.convert_to< lane2_dbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane3_sbe_enable = _val.convert_to< lane3_sbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane3_dbe_enable = _val.convert_to< lane3_dbe_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__umac_CFG3_intr_enable = _val.convert_to< umac_CFG3_intr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__umac_CFG3_pslverr_enable = _val.convert_to< umac_CFG3_pslverr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m0pb_pbus_drdy_enable = _val.convert_to< m0pb_pbus_drdy_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m1pb_pbus_drdy_enable = _val.convert_to< m1pb_pbus_drdy_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m2pb_pbus_drdy_enable = _val.convert_to< m2pb_pbus_drdy_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m3pb_pbus_drdy_enable = _val.convert_to< m3pb_pbus_drdy_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_missing_sof_enable = _val.convert_to< rx0_missing_sof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_missing_eof_enable = _val.convert_to< rx0_missing_eof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_timeout_err_enable = _val.convert_to< rx0_timeout_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_min_size_err_enable = _val.convert_to< rx0_min_size_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_missing_sof_enable = _val.convert_to< rx1_missing_sof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_missing_eof_enable = _val.convert_to< rx1_missing_eof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_timeout_err_enable = _val.convert_to< rx1_timeout_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_min_size_err_enable = _val.convert_to< rx1_min_size_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_missing_sof_enable = _val.convert_to< rx2_missing_sof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_missing_eof_enable = _val.convert_to< rx2_missing_eof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_timeout_err_enable = _val.convert_to< rx2_timeout_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_min_size_err_enable = _val.convert_to< rx2_min_size_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_missing_sof_enable = _val.convert_to< rx3_missing_sof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_missing_eof_enable = _val.convert_to< rx3_missing_eof_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_timeout_err_enable = _val.convert_to< rx3_timeout_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_min_size_err_enable = _val.convert_to< rx3_min_size_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_int_mac_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lane0_sbe_interrupt = _val.convert_to< lane0_sbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane0_dbe_interrupt = _val.convert_to< lane0_dbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane1_sbe_interrupt = _val.convert_to< lane1_sbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane1_dbe_interrupt = _val.convert_to< lane1_dbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane2_sbe_interrupt = _val.convert_to< lane2_sbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane2_dbe_interrupt = _val.convert_to< lane2_dbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane3_sbe_interrupt = _val.convert_to< lane3_sbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lane3_dbe_interrupt = _val.convert_to< lane3_dbe_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__umac_CFG3_intr_interrupt = _val.convert_to< umac_CFG3_intr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__umac_CFG3_pslverr_interrupt = _val.convert_to< umac_CFG3_pslverr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m0pb_pbus_drdy_interrupt = _val.convert_to< m0pb_pbus_drdy_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m1pb_pbus_drdy_interrupt = _val.convert_to< m1pb_pbus_drdy_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m2pb_pbus_drdy_interrupt = _val.convert_to< m2pb_pbus_drdy_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__m3pb_pbus_drdy_interrupt = _val.convert_to< m3pb_pbus_drdy_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_missing_sof_interrupt = _val.convert_to< rx0_missing_sof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_missing_eof_interrupt = _val.convert_to< rx0_missing_eof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_timeout_err_interrupt = _val.convert_to< rx0_timeout_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx0_min_size_err_interrupt = _val.convert_to< rx0_min_size_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_missing_sof_interrupt = _val.convert_to< rx1_missing_sof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_missing_eof_interrupt = _val.convert_to< rx1_missing_eof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_timeout_err_interrupt = _val.convert_to< rx1_timeout_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx1_min_size_err_interrupt = _val.convert_to< rx1_min_size_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_missing_sof_interrupt = _val.convert_to< rx2_missing_sof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_missing_eof_interrupt = _val.convert_to< rx2_missing_eof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_timeout_err_interrupt = _val.convert_to< rx2_timeout_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx2_min_size_err_interrupt = _val.convert_to< rx2_min_size_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_missing_sof_interrupt = _val.convert_to< rx3_missing_sof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_missing_eof_interrupt = _val.convert_to< rx3_missing_eof_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_timeout_err_interrupt = _val.convert_to< rx3_timeout_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rx3_min_size_err_interrupt = _val.convert_to< rx3_min_size_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_int_mac_t::all(const cpp_int & in_val) {
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

void cap_mx_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mac_interrupt = _val.convert_to< int_mac_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_ecc_interrupt = _val.convert_to< int_ecc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mac_enable = _val.convert_to< int_mac_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_ecc_enable = _val.convert_to< int_ecc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_mx_csr_dhs_mac_stats_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mx_csr_dhs_mac_stats_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_mx_csr_dhs_apb_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mx_csr_dhs_apb_t::all(const cpp_int & in_val) {
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

void cap_mx_csr_sta_ecc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_mx_csr_sta_fec_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_fec_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_sta_stats_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_stats_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_sta_rxfifo_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_rxfifo_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_sta_txfifo_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_txfifo_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_debug_port_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__select = _val.convert_to< select_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_fixer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mx_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_sta_mac_sd_status_t::all(const cpp_int & in_val) {
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

void cap_mx_csr_sta_mac_sd_interrupt_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__in_progress = _val.convert_to< in_progress_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__data_out = _val.convert_to< data_out_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mx_csr_cfg_mac_sd_core_to_cntl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mx_csr_cfg_mac_sd_interrupt_request_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_mac_sd_interrupt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__code = _val.convert_to< code_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mx_csr_cfg_mac_xoff_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ff_tx0fcxoff_i = _val.convert_to< ff_tx0fcxoff_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx0pfcxoff_i = _val.convert_to< ff_tx0pfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_tx1fcxoff_i = _val.convert_to< ff_tx1fcxoff_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx1pfcxoff_i = _val.convert_to< ff_tx1pfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_tx2fcxoff_i = _val.convert_to< ff_tx2fcxoff_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx2pfcxoff_i = _val.convert_to< ff_tx2pfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_tx3fcxoff_i = _val.convert_to< ff_tx3fcxoff_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx3pfcxoff_i = _val.convert_to< ff_tx3pfcxoff_i_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mx_csr_sta_mac_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__serdes_rxcorecontrol0_o = _val.convert_to< serdes_rxcorecontrol0_o_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_txcorecontrol0_o = _val.convert_to< serdes_txcorecontrol0_o_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ff_txafull0_o = _val.convert_to< ff_txafull0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_serdes0sigok_o = _val.convert_to< ff_serdes0sigok_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx0sync_o = _val.convert_to< ff_rx0sync_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx0idle_o = _val.convert_to< ff_tx0idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx0idle_o = _val.convert_to< ff_rx0idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx0good_o = _val.convert_to< ff_tx0good_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txquiet_ts_0_o = _val.convert_to< serdes_txquiet_ts_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxquiet_rs_0_o = _val.convert_to< serdes_rxquiet_rs_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx0read_o = _val.convert_to< ff_tx0read_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_0_o = _val.convert_to< ff_txlpi_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxlpi_0_o = _val.convert_to< ff_rxlpi_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxlpiactive_rs_0_o = _val.convert_to< serdes_rxlpiactive_rs_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txalert_ts_0_o = _val.convert_to< serdes_txalert_ts_0_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxcorecontrol1_o = _val.convert_to< serdes_rxcorecontrol1_o_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_txcorecontrol1_o = _val.convert_to< serdes_txcorecontrol1_o_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ff_txafull1_o = _val.convert_to< ff_txafull1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_serdes1sigok_o = _val.convert_to< ff_serdes1sigok_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx1sync_o = _val.convert_to< ff_rx1sync_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx1idle_o = _val.convert_to< ff_tx1idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx1idle_o = _val.convert_to< ff_rx1idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx1good_o = _val.convert_to< ff_tx1good_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txquiet_ts_1_o = _val.convert_to< serdes_txquiet_ts_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxquiet_rs_1_o = _val.convert_to< serdes_rxquiet_rs_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx1read_o = _val.convert_to< ff_tx1read_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_1_o = _val.convert_to< ff_txlpi_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxlpi_1_o = _val.convert_to< ff_rxlpi_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxlpiactive_rs_1_o = _val.convert_to< serdes_rxlpiactive_rs_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txalert_ts_1_o = _val.convert_to< serdes_txalert_ts_1_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxcorecontrol2_o = _val.convert_to< serdes_rxcorecontrol2_o_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_txcorecontrol2_o = _val.convert_to< serdes_txcorecontrol2_o_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ff_txafull2_o = _val.convert_to< ff_txafull2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_serdes2sigok_o = _val.convert_to< ff_serdes2sigok_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx2sync_o = _val.convert_to< ff_rx2sync_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx2idle_o = _val.convert_to< ff_tx2idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx2idle_o = _val.convert_to< ff_rx2idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx2good_o = _val.convert_to< ff_tx2good_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txquiet_ts_2_o = _val.convert_to< serdes_txquiet_ts_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxquiet_rs_2_o = _val.convert_to< serdes_rxquiet_rs_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx2read_o = _val.convert_to< ff_tx2read_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_2_o = _val.convert_to< ff_txlpi_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxlpi_2_o = _val.convert_to< ff_rxlpi_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxlpiactive_rs_2_o = _val.convert_to< serdes_rxlpiactive_rs_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txalert_ts_2_o = _val.convert_to< serdes_txalert_ts_2_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxcorecontrol3_o = _val.convert_to< serdes_rxcorecontrol3_o_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_txcorecontrol3_o = _val.convert_to< serdes_txcorecontrol3_o_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ff_txafull3_o = _val.convert_to< ff_txafull3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_serdes3sigok_o = _val.convert_to< ff_serdes3sigok_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx3sync_o = _val.convert_to< ff_rx3sync_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx3idle_o = _val.convert_to< ff_tx3idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rx3idle_o = _val.convert_to< ff_rx3idle_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx3good_o = _val.convert_to< ff_tx3good_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txquiet_ts_3_o = _val.convert_to< serdes_txquiet_ts_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxquiet_rs_3_o = _val.convert_to< serdes_rxquiet_rs_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx3read_o = _val.convert_to< ff_tx3read_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_3_o = _val.convert_to< ff_txlpi_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_rxlpi_3_o = _val.convert_to< ff_rxlpi_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxlpiactive_rs_3_o = _val.convert_to< serdes_rxlpiactive_rs_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txalert_ts_3_o = _val.convert_to< serdes_txalert_ts_3_o_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_mac_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mac_rxsigok_i_sel = _val.convert_to< mac_rxsigok_i_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mac_rxenergydetect_rs_i_sel = _val.convert_to< mac_rxenergydetect_rs_i_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mac_tx_corestatus_i_sel = _val.convert_to< mac_tx_corestatus_i_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mac_rx_corestatus_i_sel = _val.convert_to< mac_rx_corestatus_i_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__core_to_cntl_7_5_sel = _val.convert_to< core_to_cntl_7_5_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__core_to_cntl_9_8_sel = _val.convert_to< core_to_cntl_9_8_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__core_to_cntl_15_10_sel = _val.convert_to< core_to_cntl_15_10_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes0_rxsigok_i = _val.convert_to< serdes0_rxsigok_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes0_tx_override_in = _val.convert_to< serdes0_tx_override_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxenergydetect_rs_0_i = _val.convert_to< serdes_rxenergydetect_rs_0_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txcorestatus_0_i = _val.convert_to< serdes_txcorestatus_0_i_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_rxcorestatus_0_i = _val.convert_to< serdes_rxcorestatus_0_i_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__serdes1_rxsigok_i = _val.convert_to< serdes1_rxsigok_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes1_tx_override_in = _val.convert_to< serdes1_tx_override_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxenergydetect_rs_1_i = _val.convert_to< serdes_rxenergydetect_rs_1_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txcorestatus_1_i = _val.convert_to< serdes_txcorestatus_1_i_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_rxcorestatus_1_i = _val.convert_to< serdes_rxcorestatus_1_i_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__serdes2_rxsigok_i = _val.convert_to< serdes2_rxsigok_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes2_tx_override_in = _val.convert_to< serdes2_tx_override_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxenergydetect_rs_2_i = _val.convert_to< serdes_rxenergydetect_rs_2_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txcorestatus_2_i = _val.convert_to< serdes_txcorestatus_2_i_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_rxcorestatus_2_i = _val.convert_to< serdes_rxcorestatus_2_i_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__serdes3_rxsigok_i = _val.convert_to< serdes3_rxsigok_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes3_tx_override_in = _val.convert_to< serdes3_tx_override_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_rxenergydetect_rs_3_i = _val.convert_to< serdes_rxenergydetect_rs_3_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__serdes_txcorestatus_3_i = _val.convert_to< serdes_txcorestatus_3_i_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__serdes_rxcorestatus_3_i = _val.convert_to< serdes_rxcorestatus_3_i_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ff_txlpi_0_i = _val.convert_to< ff_txlpi_0_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_1_i = _val.convert_to< ff_txlpi_1_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_2_i = _val.convert_to< ff_txlpi_2_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txlpi_3_i = _val.convert_to< ff_txlpi_3_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mx_csr_cfg_mac_tdm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__slot0 = _val.convert_to< slot0_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__slot1 = _val.convert_to< slot1_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__slot2 = _val.convert_to< slot2_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__slot3 = _val.convert_to< slot3_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_mx_csr_cfg_mac_gbl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ff_txdispad_i = _val.convert_to< ff_txdispad_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_fcs_enable = _val.convert_to< pkt_fcs_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_tx_ipg = _val.convert_to< ff_tx_ipg_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ff_txtimestampena_i = _val.convert_to< ff_txtimestampena_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txtimestampid_i = _val.convert_to< ff_txtimestampid_i_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__cg_fec_enable_i = _val.convert_to< cg_fec_enable_i_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ff_txpreamble_i = _val.convert_to< ff_txpreamble_i_cpp_int_t >()  ;
    _val = _val >> 56;
    
}

void cap_mx_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mx_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_mac_gbl.all( _val);
    _val = _val >> cfg_mac_gbl.get_width(); 
    cfg_mac_tdm.all( _val);
    _val = _val >> cfg_mac_tdm.get_width(); 
    cfg_mac.all( _val);
    _val = _val >> cfg_mac.get_width(); 
    sta_mac.all( _val);
    _val = _val >> sta_mac.get_width(); 
    cfg_mac_xoff.all( _val);
    _val = _val >> cfg_mac_xoff.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt[ii].all(_val);
        _val = _val >> cfg_mac_sd_interrupt[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt_request
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt_request[ii].all(_val);
        _val = _val >> cfg_mac_sd_interrupt_request[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_core_to_cntl
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_core_to_cntl[ii].all(_val);
        _val = _val >> cfg_mac_sd_core_to_cntl[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_interrupt_status
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_interrupt_status[ii].all(_val);
        _val = _val >> sta_mac_sd_interrupt_status[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_status
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_status[ii].all(_val);
        _val = _val >> sta_mac_sd_status[ii].get_width();
    }
    #endif
    
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    cfg_fixer.all( _val);
    _val = _val >> cfg_fixer.get_width(); 
    cfg_debug_port.all( _val);
    _val = _val >> cfg_debug_port.get_width(); 
    cfg_txfifo_mem.all( _val);
    _val = _val >> cfg_txfifo_mem.get_width(); 
    sta_txfifo_mem.all( _val);
    _val = _val >> sta_txfifo_mem.get_width(); 
    cfg_rxfifo_mem.all( _val);
    _val = _val >> cfg_rxfifo_mem.get_width(); 
    sta_rxfifo_mem.all( _val);
    _val = _val >> sta_rxfifo_mem.get_width(); 
    cfg_stats_mem.all( _val);
    _val = _val >> cfg_stats_mem.get_width(); 
    sta_stats_mem.all( _val);
    _val = _val >> sta_stats_mem.get_width(); 
    cfg_fec_mem.all( _val);
    _val = _val >> cfg_fec_mem.get_width(); 
    sta_fec_mem.all( _val);
    _val = _val >> sta_fec_mem.get_width(); 
    sta_ecc.all( _val);
    _val = _val >> sta_ecc.get_width(); 
    dhs_apb.all( _val);
    _val = _val >> dhs_apb.get_width(); 
    dhs_mac_stats.all( _val);
    _val = _val >> dhs_mac_stats.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_mac.all( _val);
    _val = _val >> int_mac.get_width(); 
    int_ecc.all( _val);
    _val = _val >> int_ecc.get_width(); 
}

cpp_int cap_mx_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_ecc.get_width(); ret_val = ret_val  | int_ecc.all(); 
    ret_val = ret_val << int_mac.get_width(); ret_val = ret_val  | int_mac.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << dhs_mac_stats.get_width(); ret_val = ret_val  | dhs_mac_stats.all(); 
    ret_val = ret_val << dhs_apb.get_width(); ret_val = ret_val  | dhs_apb.all(); 
    ret_val = ret_val << sta_ecc.get_width(); ret_val = ret_val  | sta_ecc.all(); 
    ret_val = ret_val << sta_fec_mem.get_width(); ret_val = ret_val  | sta_fec_mem.all(); 
    ret_val = ret_val << cfg_fec_mem.get_width(); ret_val = ret_val  | cfg_fec_mem.all(); 
    ret_val = ret_val << sta_stats_mem.get_width(); ret_val = ret_val  | sta_stats_mem.all(); 
    ret_val = ret_val << cfg_stats_mem.get_width(); ret_val = ret_val  | cfg_stats_mem.all(); 
    ret_val = ret_val << sta_rxfifo_mem.get_width(); ret_val = ret_val  | sta_rxfifo_mem.all(); 
    ret_val = ret_val << cfg_rxfifo_mem.get_width(); ret_val = ret_val  | cfg_rxfifo_mem.all(); 
    ret_val = ret_val << sta_txfifo_mem.get_width(); ret_val = ret_val  | sta_txfifo_mem.all(); 
    ret_val = ret_val << cfg_txfifo_mem.get_width(); ret_val = ret_val  | cfg_txfifo_mem.all(); 
    ret_val = ret_val << cfg_debug_port.get_width(); ret_val = ret_val  | cfg_debug_port.all(); 
    ret_val = ret_val << cfg_fixer.get_width(); ret_val = ret_val  | cfg_fixer.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_status
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_mac_sd_status[ii].get_width(); ret_val = ret_val  | sta_mac_sd_status[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_interrupt_status
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_mac_sd_interrupt_status[ii].get_width(); ret_val = ret_val  | sta_mac_sd_interrupt_status[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_core_to_cntl
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_mac_sd_core_to_cntl[ii].get_width(); ret_val = ret_val  | cfg_mac_sd_core_to_cntl[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt_request
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_mac_sd_interrupt_request[ii].get_width(); ret_val = ret_val  | cfg_mac_sd_interrupt_request[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_mac_sd_interrupt[ii].get_width(); ret_val = ret_val  | cfg_mac_sd_interrupt[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << cfg_mac_xoff.get_width(); ret_val = ret_val  | cfg_mac_xoff.all(); 
    ret_val = ret_val << sta_mac.get_width(); ret_val = ret_val  | sta_mac.all(); 
    ret_val = ret_val << cfg_mac.get_width(); ret_val = ret_val  | cfg_mac.all(); 
    ret_val = ret_val << cfg_mac_tdm.get_width(); ret_val = ret_val  | cfg_mac_tdm.all(); 
    ret_val = ret_val << cfg_mac_gbl.get_width(); ret_val = ret_val  | cfg_mac_gbl.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_mx_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::all() const {
    cpp_int ret_val;

    // ff_txpreamble_i
    ret_val = ret_val << 56; ret_val = ret_val  | int_var__ff_txpreamble_i; 
    
    // cg_fec_enable_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cg_fec_enable_i; 
    
    // ff_txtimestampid_i
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ff_txtimestampid_i; 
    
    // ff_txtimestampena_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txtimestampena_i; 
    
    // ff_tx_ipg
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx_ipg; 
    
    // pkt_fcs_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkt_fcs_enable; 
    
    // ff_txdispad_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txdispad_i; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_tdm_t::all() const {
    cpp_int ret_val;

    // slot3
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__slot3; 
    
    // slot2
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__slot2; 
    
    // slot1
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__slot1; 
    
    // slot0
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__slot0; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_t::all() const {
    cpp_int ret_val;

    // ff_txlpi_3_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_3_i; 
    
    // ff_txlpi_2_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_2_i; 
    
    // ff_txlpi_1_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_1_i; 
    
    // ff_txlpi_0_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_0_i; 
    
    // serdes_rxcorestatus_3_i
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_rxcorestatus_3_i; 
    
    // serdes_txcorestatus_3_i
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_txcorestatus_3_i; 
    
    // serdes_rxenergydetect_rs_3_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxenergydetect_rs_3_i; 
    
    // serdes3_tx_override_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes3_tx_override_in; 
    
    // serdes3_rxsigok_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes3_rxsigok_i; 
    
    // serdes_rxcorestatus_2_i
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_rxcorestatus_2_i; 
    
    // serdes_txcorestatus_2_i
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_txcorestatus_2_i; 
    
    // serdes_rxenergydetect_rs_2_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxenergydetect_rs_2_i; 
    
    // serdes2_tx_override_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes2_tx_override_in; 
    
    // serdes2_rxsigok_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes2_rxsigok_i; 
    
    // serdes_rxcorestatus_1_i
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_rxcorestatus_1_i; 
    
    // serdes_txcorestatus_1_i
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_txcorestatus_1_i; 
    
    // serdes_rxenergydetect_rs_1_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxenergydetect_rs_1_i; 
    
    // serdes1_tx_override_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes1_tx_override_in; 
    
    // serdes1_rxsigok_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes1_rxsigok_i; 
    
    // serdes_rxcorestatus_0_i
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_rxcorestatus_0_i; 
    
    // serdes_txcorestatus_0_i
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_txcorestatus_0_i; 
    
    // serdes_rxenergydetect_rs_0_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxenergydetect_rs_0_i; 
    
    // serdes0_tx_override_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes0_tx_override_in; 
    
    // serdes0_rxsigok_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes0_rxsigok_i; 
    
    // core_to_cntl_15_10_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__core_to_cntl_15_10_sel; 
    
    // core_to_cntl_9_8_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__core_to_cntl_9_8_sel; 
    
    // core_to_cntl_7_5_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__core_to_cntl_7_5_sel; 
    
    // mac_rx_corestatus_i_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mac_rx_corestatus_i_sel; 
    
    // mac_tx_corestatus_i_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mac_tx_corestatus_i_sel; 
    
    // mac_rxenergydetect_rs_i_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mac_rxenergydetect_rs_i_sel; 
    
    // mac_rxsigok_i_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mac_rxsigok_i_sel; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_mac_t::all() const {
    cpp_int ret_val;

    // serdes_txalert_ts_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txalert_ts_3_o; 
    
    // serdes_rxlpiactive_rs_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxlpiactive_rs_3_o; 
    
    // ff_rxlpi_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rxlpi_3_o; 
    
    // ff_txlpi_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_3_o; 
    
    // ff_tx3read_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx3read_o; 
    
    // serdes_rxquiet_rs_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxquiet_rs_3_o; 
    
    // serdes_txquiet_ts_3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txquiet_ts_3_o; 
    
    // ff_tx3good_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx3good_o; 
    
    // ff_rx3idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx3idle_o; 
    
    // ff_tx3idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx3idle_o; 
    
    // ff_rx3sync_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx3sync_o; 
    
    // ff_serdes3sigok_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_serdes3sigok_o; 
    
    // ff_txafull3_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txafull3_o; 
    
    // serdes_txcorecontrol3_o
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_txcorecontrol3_o; 
    
    // serdes_rxcorecontrol3_o
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_rxcorecontrol3_o; 
    
    // serdes_txalert_ts_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txalert_ts_2_o; 
    
    // serdes_rxlpiactive_rs_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxlpiactive_rs_2_o; 
    
    // ff_rxlpi_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rxlpi_2_o; 
    
    // ff_txlpi_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_2_o; 
    
    // ff_tx2read_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx2read_o; 
    
    // serdes_rxquiet_rs_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxquiet_rs_2_o; 
    
    // serdes_txquiet_ts_2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txquiet_ts_2_o; 
    
    // ff_tx2good_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx2good_o; 
    
    // ff_rx2idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx2idle_o; 
    
    // ff_tx2idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx2idle_o; 
    
    // ff_rx2sync_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx2sync_o; 
    
    // ff_serdes2sigok_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_serdes2sigok_o; 
    
    // ff_txafull2_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txafull2_o; 
    
    // serdes_txcorecontrol2_o
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_txcorecontrol2_o; 
    
    // serdes_rxcorecontrol2_o
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_rxcorecontrol2_o; 
    
    // serdes_txalert_ts_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txalert_ts_1_o; 
    
    // serdes_rxlpiactive_rs_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxlpiactive_rs_1_o; 
    
    // ff_rxlpi_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rxlpi_1_o; 
    
    // ff_txlpi_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_1_o; 
    
    // ff_tx1read_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx1read_o; 
    
    // serdes_rxquiet_rs_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxquiet_rs_1_o; 
    
    // serdes_txquiet_ts_1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txquiet_ts_1_o; 
    
    // ff_tx1good_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx1good_o; 
    
    // ff_rx1idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx1idle_o; 
    
    // ff_tx1idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx1idle_o; 
    
    // ff_rx1sync_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx1sync_o; 
    
    // ff_serdes1sigok_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_serdes1sigok_o; 
    
    // ff_txafull1_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txafull1_o; 
    
    // serdes_txcorecontrol1_o
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_txcorecontrol1_o; 
    
    // serdes_rxcorecontrol1_o
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_rxcorecontrol1_o; 
    
    // serdes_txalert_ts_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txalert_ts_0_o; 
    
    // serdes_rxlpiactive_rs_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxlpiactive_rs_0_o; 
    
    // ff_rxlpi_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rxlpi_0_o; 
    
    // ff_txlpi_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txlpi_0_o; 
    
    // ff_tx0read_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx0read_o; 
    
    // serdes_rxquiet_rs_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_rxquiet_rs_0_o; 
    
    // serdes_txquiet_ts_0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__serdes_txquiet_ts_0_o; 
    
    // ff_tx0good_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx0good_o; 
    
    // ff_rx0idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx0idle_o; 
    
    // ff_tx0idle_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx0idle_o; 
    
    // ff_rx0sync_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_rx0sync_o; 
    
    // ff_serdes0sigok_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_serdes0sigok_o; 
    
    // ff_txafull0_o
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_txafull0_o; 
    
    // serdes_txcorecontrol0_o
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__serdes_txcorecontrol0_o; 
    
    // serdes_rxcorecontrol0_o
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__serdes_rxcorecontrol0_o; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::all() const {
    cpp_int ret_val;

    // ff_tx3pfcxoff_i
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx3pfcxoff_i; 
    
    // ff_tx3fcxoff_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx3fcxoff_i; 
    
    // ff_tx2pfcxoff_i
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx2pfcxoff_i; 
    
    // ff_tx2fcxoff_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx2fcxoff_i; 
    
    // ff_tx1pfcxoff_i
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx1pfcxoff_i; 
    
    // ff_tx1fcxoff_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx1fcxoff_i; 
    
    // ff_tx0pfcxoff_i
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ff_tx0pfcxoff_i; 
    
    // ff_tx0fcxoff_i
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ff_tx0fcxoff_i; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_sd_interrupt_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data; 
    
    // code
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__code; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_sd_interrupt_request_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_mac_sd_core_to_cntl_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_mac_sd_interrupt_status_t::all() const {
    cpp_int ret_val;

    // data_out
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_out; 
    
    // in_progress
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__in_progress; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_mac_sd_status_t::all() const {
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

cpp_int cap_mx_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_fixer_t::all() const {
    cpp_int ret_val;

    // timeout
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__timeout; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_debug_port_t::all() const {
    cpp_int ret_val;

    // select
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__select; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_txfifo_mem_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_txfifo_mem_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_rxfifo_mem_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_rxfifo_mem_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_stats_mem_t::all() const {
    cpp_int ret_val;

    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_stats_mem_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mx_csr_cfg_fec_mem_t::all() const {
    cpp_int ret_val;

    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_fec_mem_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mx_csr_sta_ecc_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_mx_csr_dhs_apb_t::all() const {
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

cpp_int cap_mx_csr_dhs_apb_entry_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_mx_csr_dhs_mac_stats_t::all() const {
    cpp_int ret_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 512-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_mx_csr_dhs_mac_stats_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mx_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mx_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_ecc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_ecc_interrupt; 
    
    // int_mac_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mac_interrupt; 
    
    return ret_val;
}

cpp_int cap_mx_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_ecc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_ecc_enable; 
    
    // int_mac_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mac_enable; 
    
    return ret_val;
}

cpp_int cap_mx_csr_int_mac_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mx_csr_int_mac_intreg_t::all() const {
    cpp_int ret_val;

    // rx3_min_size_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_min_size_err_interrupt; 
    
    // rx3_timeout_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_timeout_err_interrupt; 
    
    // rx3_missing_eof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_missing_eof_interrupt; 
    
    // rx3_missing_sof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_missing_sof_interrupt; 
    
    // rx2_min_size_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_min_size_err_interrupt; 
    
    // rx2_timeout_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_timeout_err_interrupt; 
    
    // rx2_missing_eof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_missing_eof_interrupt; 
    
    // rx2_missing_sof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_missing_sof_interrupt; 
    
    // rx1_min_size_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_min_size_err_interrupt; 
    
    // rx1_timeout_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_timeout_err_interrupt; 
    
    // rx1_missing_eof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_missing_eof_interrupt; 
    
    // rx1_missing_sof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_missing_sof_interrupt; 
    
    // rx0_min_size_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_min_size_err_interrupt; 
    
    // rx0_timeout_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_timeout_err_interrupt; 
    
    // rx0_missing_eof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_missing_eof_interrupt; 
    
    // rx0_missing_sof_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_missing_sof_interrupt; 
    
    // m3pb_pbus_drdy_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m3pb_pbus_drdy_interrupt; 
    
    // m2pb_pbus_drdy_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m2pb_pbus_drdy_interrupt; 
    
    // m1pb_pbus_drdy_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m1pb_pbus_drdy_interrupt; 
    
    // m0pb_pbus_drdy_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m0pb_pbus_drdy_interrupt; 
    
    // umac_CFG3_pslverr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__umac_CFG3_pslverr_interrupt; 
    
    // umac_CFG3_intr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__umac_CFG3_intr_interrupt; 
    
    // lane3_dbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane3_dbe_interrupt; 
    
    // lane3_sbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane3_sbe_interrupt; 
    
    // lane2_dbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane2_dbe_interrupt; 
    
    // lane2_sbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane2_sbe_interrupt; 
    
    // lane1_dbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane1_dbe_interrupt; 
    
    // lane1_sbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane1_sbe_interrupt; 
    
    // lane0_dbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane0_dbe_interrupt; 
    
    // lane0_sbe_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane0_sbe_interrupt; 
    
    return ret_val;
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // rx3_min_size_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_min_size_err_enable; 
    
    // rx3_timeout_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_timeout_err_enable; 
    
    // rx3_missing_eof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_missing_eof_enable; 
    
    // rx3_missing_sof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx3_missing_sof_enable; 
    
    // rx2_min_size_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_min_size_err_enable; 
    
    // rx2_timeout_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_timeout_err_enable; 
    
    // rx2_missing_eof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_missing_eof_enable; 
    
    // rx2_missing_sof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx2_missing_sof_enable; 
    
    // rx1_min_size_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_min_size_err_enable; 
    
    // rx1_timeout_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_timeout_err_enable; 
    
    // rx1_missing_eof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_missing_eof_enable; 
    
    // rx1_missing_sof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx1_missing_sof_enable; 
    
    // rx0_min_size_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_min_size_err_enable; 
    
    // rx0_timeout_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_timeout_err_enable; 
    
    // rx0_missing_eof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_missing_eof_enable; 
    
    // rx0_missing_sof_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rx0_missing_sof_enable; 
    
    // m3pb_pbus_drdy_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m3pb_pbus_drdy_enable; 
    
    // m2pb_pbus_drdy_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m2pb_pbus_drdy_enable; 
    
    // m1pb_pbus_drdy_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m1pb_pbus_drdy_enable; 
    
    // m0pb_pbus_drdy_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__m0pb_pbus_drdy_enable; 
    
    // umac_CFG3_pslverr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__umac_CFG3_pslverr_enable; 
    
    // umac_CFG3_intr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__umac_CFG3_intr_enable; 
    
    // lane3_dbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane3_dbe_enable; 
    
    // lane3_sbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane3_sbe_enable; 
    
    // lane2_dbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane2_dbe_enable; 
    
    // lane2_sbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane2_sbe_enable; 
    
    // lane1_dbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane1_dbe_enable; 
    
    // lane1_sbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane1_sbe_enable; 
    
    // lane0_dbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane0_dbe_enable; 
    
    // lane0_sbe_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lane0_sbe_enable; 
    
    return ret_val;
}

cpp_int cap_mx_csr_int_ecc_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mx_csr_int_ecc_int_test_set_t::all() const {
    cpp_int ret_val;

    // correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_interrupt; 
    
    // uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_interrupt; 
    
    return ret_val;
}

cpp_int cap_mx_csr_int_ecc_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_enable; 
    
    // uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_enable; 
    
    return ret_val;
}

void cap_mx_csr_int_ecc_int_enable_clear_t::clear() {

    int_var__uncorrectable_enable = 0; 
    
    int_var__correctable_enable = 0; 
    
}

void cap_mx_csr_int_ecc_int_test_set_t::clear() {

    int_var__uncorrectable_interrupt = 0; 
    
    int_var__correctable_interrupt = 0; 
    
}

void cap_mx_csr_int_ecc_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mx_csr_int_mac_int_enable_clear_t::clear() {

    int_var__lane0_sbe_enable = 0; 
    
    int_var__lane0_dbe_enable = 0; 
    
    int_var__lane1_sbe_enable = 0; 
    
    int_var__lane1_dbe_enable = 0; 
    
    int_var__lane2_sbe_enable = 0; 
    
    int_var__lane2_dbe_enable = 0; 
    
    int_var__lane3_sbe_enable = 0; 
    
    int_var__lane3_dbe_enable = 0; 
    
    int_var__umac_CFG3_intr_enable = 0; 
    
    int_var__umac_CFG3_pslverr_enable = 0; 
    
    int_var__m0pb_pbus_drdy_enable = 0; 
    
    int_var__m1pb_pbus_drdy_enable = 0; 
    
    int_var__m2pb_pbus_drdy_enable = 0; 
    
    int_var__m3pb_pbus_drdy_enable = 0; 
    
    int_var__rx0_missing_sof_enable = 0; 
    
    int_var__rx0_missing_eof_enable = 0; 
    
    int_var__rx0_timeout_err_enable = 0; 
    
    int_var__rx0_min_size_err_enable = 0; 
    
    int_var__rx1_missing_sof_enable = 0; 
    
    int_var__rx1_missing_eof_enable = 0; 
    
    int_var__rx1_timeout_err_enable = 0; 
    
    int_var__rx1_min_size_err_enable = 0; 
    
    int_var__rx2_missing_sof_enable = 0; 
    
    int_var__rx2_missing_eof_enable = 0; 
    
    int_var__rx2_timeout_err_enable = 0; 
    
    int_var__rx2_min_size_err_enable = 0; 
    
    int_var__rx3_missing_sof_enable = 0; 
    
    int_var__rx3_missing_eof_enable = 0; 
    
    int_var__rx3_timeout_err_enable = 0; 
    
    int_var__rx3_min_size_err_enable = 0; 
    
}

void cap_mx_csr_int_mac_intreg_t::clear() {

    int_var__lane0_sbe_interrupt = 0; 
    
    int_var__lane0_dbe_interrupt = 0; 
    
    int_var__lane1_sbe_interrupt = 0; 
    
    int_var__lane1_dbe_interrupt = 0; 
    
    int_var__lane2_sbe_interrupt = 0; 
    
    int_var__lane2_dbe_interrupt = 0; 
    
    int_var__lane3_sbe_interrupt = 0; 
    
    int_var__lane3_dbe_interrupt = 0; 
    
    int_var__umac_CFG3_intr_interrupt = 0; 
    
    int_var__umac_CFG3_pslverr_interrupt = 0; 
    
    int_var__m0pb_pbus_drdy_interrupt = 0; 
    
    int_var__m1pb_pbus_drdy_interrupt = 0; 
    
    int_var__m2pb_pbus_drdy_interrupt = 0; 
    
    int_var__m3pb_pbus_drdy_interrupt = 0; 
    
    int_var__rx0_missing_sof_interrupt = 0; 
    
    int_var__rx0_missing_eof_interrupt = 0; 
    
    int_var__rx0_timeout_err_interrupt = 0; 
    
    int_var__rx0_min_size_err_interrupt = 0; 
    
    int_var__rx1_missing_sof_interrupt = 0; 
    
    int_var__rx1_missing_eof_interrupt = 0; 
    
    int_var__rx1_timeout_err_interrupt = 0; 
    
    int_var__rx1_min_size_err_interrupt = 0; 
    
    int_var__rx2_missing_sof_interrupt = 0; 
    
    int_var__rx2_missing_eof_interrupt = 0; 
    
    int_var__rx2_timeout_err_interrupt = 0; 
    
    int_var__rx2_min_size_err_interrupt = 0; 
    
    int_var__rx3_missing_sof_interrupt = 0; 
    
    int_var__rx3_missing_eof_interrupt = 0; 
    
    int_var__rx3_timeout_err_interrupt = 0; 
    
    int_var__rx3_min_size_err_interrupt = 0; 
    
}

void cap_mx_csr_int_mac_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mx_csr_intreg_status_t::clear() {

    int_var__int_mac_interrupt = 0; 
    
    int_var__int_ecc_interrupt = 0; 
    
}

void cap_mx_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_mac_enable = 0; 
    
    int_var__int_ecc_enable = 0; 
    
}

void cap_mx_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_mx_csr_dhs_mac_stats_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_mx_csr_dhs_mac_stats_t::clear() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_mx_csr_dhs_apb_entry_t::clear() {

    int_var__data = 0; 
    
}

void cap_mx_csr_dhs_apb_t::clear() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_mx_csr_sta_ecc_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_mx_csr_sta_fec_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mx_csr_cfg_fec_mem_t::clear() {

    int_var__bist_run = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__ecc_disable_cor = 0; 
    
}

void cap_mx_csr_sta_stats_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mx_csr_cfg_stats_mem_t::clear() {

    int_var__bist_run = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__ecc_disable_cor = 0; 
    
}

void cap_mx_csr_sta_rxfifo_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mx_csr_cfg_rxfifo_mem_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mx_csr_sta_txfifo_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mx_csr_cfg_txfifo_mem_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mx_csr_cfg_debug_port_t::clear() {

    int_var__enable = 0; 
    
    int_var__select = 0; 
    
}

void cap_mx_csr_cfg_fixer_t::clear() {

    int_var__enable = 0; 
    
    int_var__timeout = 0; 
    
}

void cap_mx_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_mx_csr_sta_mac_sd_status_t::clear() {

    int_var__rx_idle_detect = 0; 
    
    int_var__rx_rdy = 0; 
    
    int_var__tx_rdy = 0; 
    
    int_var__core_status = 0; 
    
}

void cap_mx_csr_sta_mac_sd_interrupt_status_t::clear() {

    int_var__in_progress = 0; 
    
    int_var__data_out = 0; 
    
}

void cap_mx_csr_cfg_mac_sd_core_to_cntl_t::clear() {

    int_var__value = 0; 
    
}

void cap_mx_csr_cfg_mac_sd_interrupt_request_t::clear() {

    int_var__val = 0; 
    
}

void cap_mx_csr_cfg_mac_sd_interrupt_t::clear() {

    int_var__code = 0; 
    
    int_var__data = 0; 
    
}

void cap_mx_csr_cfg_mac_xoff_t::clear() {

    int_var__ff_tx0fcxoff_i = 0; 
    
    int_var__ff_tx0pfcxoff_i = 0; 
    
    int_var__ff_tx1fcxoff_i = 0; 
    
    int_var__ff_tx1pfcxoff_i = 0; 
    
    int_var__ff_tx2fcxoff_i = 0; 
    
    int_var__ff_tx2pfcxoff_i = 0; 
    
    int_var__ff_tx3fcxoff_i = 0; 
    
    int_var__ff_tx3pfcxoff_i = 0; 
    
}

void cap_mx_csr_sta_mac_t::clear() {

    int_var__serdes_rxcorecontrol0_o = 0; 
    
    int_var__serdes_txcorecontrol0_o = 0; 
    
    int_var__ff_txafull0_o = 0; 
    
    int_var__ff_serdes0sigok_o = 0; 
    
    int_var__ff_rx0sync_o = 0; 
    
    int_var__ff_tx0idle_o = 0; 
    
    int_var__ff_rx0idle_o = 0; 
    
    int_var__ff_tx0good_o = 0; 
    
    int_var__serdes_txquiet_ts_0_o = 0; 
    
    int_var__serdes_rxquiet_rs_0_o = 0; 
    
    int_var__ff_tx0read_o = 0; 
    
    int_var__ff_txlpi_0_o = 0; 
    
    int_var__ff_rxlpi_0_o = 0; 
    
    int_var__serdes_rxlpiactive_rs_0_o = 0; 
    
    int_var__serdes_txalert_ts_0_o = 0; 
    
    int_var__serdes_rxcorecontrol1_o = 0; 
    
    int_var__serdes_txcorecontrol1_o = 0; 
    
    int_var__ff_txafull1_o = 0; 
    
    int_var__ff_serdes1sigok_o = 0; 
    
    int_var__ff_rx1sync_o = 0; 
    
    int_var__ff_tx1idle_o = 0; 
    
    int_var__ff_rx1idle_o = 0; 
    
    int_var__ff_tx1good_o = 0; 
    
    int_var__serdes_txquiet_ts_1_o = 0; 
    
    int_var__serdes_rxquiet_rs_1_o = 0; 
    
    int_var__ff_tx1read_o = 0; 
    
    int_var__ff_txlpi_1_o = 0; 
    
    int_var__ff_rxlpi_1_o = 0; 
    
    int_var__serdes_rxlpiactive_rs_1_o = 0; 
    
    int_var__serdes_txalert_ts_1_o = 0; 
    
    int_var__serdes_rxcorecontrol2_o = 0; 
    
    int_var__serdes_txcorecontrol2_o = 0; 
    
    int_var__ff_txafull2_o = 0; 
    
    int_var__ff_serdes2sigok_o = 0; 
    
    int_var__ff_rx2sync_o = 0; 
    
    int_var__ff_tx2idle_o = 0; 
    
    int_var__ff_rx2idle_o = 0; 
    
    int_var__ff_tx2good_o = 0; 
    
    int_var__serdes_txquiet_ts_2_o = 0; 
    
    int_var__serdes_rxquiet_rs_2_o = 0; 
    
    int_var__ff_tx2read_o = 0; 
    
    int_var__ff_txlpi_2_o = 0; 
    
    int_var__ff_rxlpi_2_o = 0; 
    
    int_var__serdes_rxlpiactive_rs_2_o = 0; 
    
    int_var__serdes_txalert_ts_2_o = 0; 
    
    int_var__serdes_rxcorecontrol3_o = 0; 
    
    int_var__serdes_txcorecontrol3_o = 0; 
    
    int_var__ff_txafull3_o = 0; 
    
    int_var__ff_serdes3sigok_o = 0; 
    
    int_var__ff_rx3sync_o = 0; 
    
    int_var__ff_tx3idle_o = 0; 
    
    int_var__ff_rx3idle_o = 0; 
    
    int_var__ff_tx3good_o = 0; 
    
    int_var__serdes_txquiet_ts_3_o = 0; 
    
    int_var__serdes_rxquiet_rs_3_o = 0; 
    
    int_var__ff_tx3read_o = 0; 
    
    int_var__ff_txlpi_3_o = 0; 
    
    int_var__ff_rxlpi_3_o = 0; 
    
    int_var__serdes_rxlpiactive_rs_3_o = 0; 
    
    int_var__serdes_txalert_ts_3_o = 0; 
    
}

void cap_mx_csr_cfg_mac_t::clear() {

    int_var__mac_rxsigok_i_sel = 0; 
    
    int_var__mac_rxenergydetect_rs_i_sel = 0; 
    
    int_var__mac_tx_corestatus_i_sel = 0; 
    
    int_var__mac_rx_corestatus_i_sel = 0; 
    
    int_var__core_to_cntl_7_5_sel = 0; 
    
    int_var__core_to_cntl_9_8_sel = 0; 
    
    int_var__core_to_cntl_15_10_sel = 0; 
    
    int_var__serdes0_rxsigok_i = 0; 
    
    int_var__serdes0_tx_override_in = 0; 
    
    int_var__serdes_rxenergydetect_rs_0_i = 0; 
    
    int_var__serdes_txcorestatus_0_i = 0; 
    
    int_var__serdes_rxcorestatus_0_i = 0; 
    
    int_var__serdes1_rxsigok_i = 0; 
    
    int_var__serdes1_tx_override_in = 0; 
    
    int_var__serdes_rxenergydetect_rs_1_i = 0; 
    
    int_var__serdes_txcorestatus_1_i = 0; 
    
    int_var__serdes_rxcorestatus_1_i = 0; 
    
    int_var__serdes2_rxsigok_i = 0; 
    
    int_var__serdes2_tx_override_in = 0; 
    
    int_var__serdes_rxenergydetect_rs_2_i = 0; 
    
    int_var__serdes_txcorestatus_2_i = 0; 
    
    int_var__serdes_rxcorestatus_2_i = 0; 
    
    int_var__serdes3_rxsigok_i = 0; 
    
    int_var__serdes3_tx_override_in = 0; 
    
    int_var__serdes_rxenergydetect_rs_3_i = 0; 
    
    int_var__serdes_txcorestatus_3_i = 0; 
    
    int_var__serdes_rxcorestatus_3_i = 0; 
    
    int_var__ff_txlpi_0_i = 0; 
    
    int_var__ff_txlpi_1_i = 0; 
    
    int_var__ff_txlpi_2_i = 0; 
    
    int_var__ff_txlpi_3_i = 0; 
    
}

void cap_mx_csr_cfg_mac_tdm_t::clear() {

    int_var__slot0 = 0; 
    
    int_var__slot1 = 0; 
    
    int_var__slot2 = 0; 
    
    int_var__slot3 = 0; 
    
}

void cap_mx_csr_cfg_mac_gbl_t::clear() {

    int_var__ff_txdispad_i = 0; 
    
    int_var__pkt_fcs_enable = 0; 
    
    int_var__ff_tx_ipg = 0; 
    
    int_var__ff_txtimestampena_i = 0; 
    
    int_var__ff_txtimestampid_i = 0; 
    
    int_var__cg_fec_enable_i = 0; 
    
    int_var__ff_txpreamble_i = 0; 
    
}

void cap_mx_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_mx_csr_t::clear() {

    base.clear();
    cfg_mac_gbl.clear();
    cfg_mac_tdm.clear();
    cfg_mac.clear();
    sta_mac.clear();
    cfg_mac_xoff.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_interrupt_request
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_interrupt_request[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_mac_sd_core_to_cntl
    for(int ii = 0; ii < 4; ii++) {
        cfg_mac_sd_core_to_cntl[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_interrupt_status
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_interrupt_status[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_mac_sd_status
    for(int ii = 0; ii < 4; ii++) {
        sta_mac_sd_status[ii].clear();
    }
    #endif
    
    csr_intr.clear();
    cfg_fixer.clear();
    cfg_debug_port.clear();
    cfg_txfifo_mem.clear();
    sta_txfifo_mem.clear();
    cfg_rxfifo_mem.clear();
    sta_rxfifo_mem.clear();
    cfg_stats_mem.clear();
    sta_stats_mem.clear();
    cfg_fec_mem.clear();
    sta_fec_mem.clear();
    sta_ecc.clear();
    dhs_apb.clear();
    dhs_mac_stats.clear();
    int_groups.clear();
    int_mac.clear();
    int_ecc.clear();
}

void cap_mx_csr_int_ecc_int_enable_clear_t::init() {

}

void cap_mx_csr_int_ecc_int_test_set_t::init() {

}

void cap_mx_csr_int_ecc_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mx_csr_int_mac_int_enable_clear_t::init() {

}

void cap_mx_csr_int_mac_intreg_t::init() {

}

void cap_mx_csr_int_mac_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mx_csr_intreg_status_t::init() {

}

void cap_mx_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_mx_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_mx_csr_dhs_mac_stats_entry_t::init() {

}

void cap_mx_csr_dhs_mac_stats_t::init() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 512; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_mx_csr_dhs_apb_entry_t::init() {

}

void cap_mx_csr_dhs_apb_t::init() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 65536; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_mx_csr_sta_ecc_t::init() {

}

void cap_mx_csr_sta_fec_mem_t::init() {

}

void cap_mx_csr_cfg_fec_mem_t::init() {

}

void cap_mx_csr_sta_stats_mem_t::init() {

}

void cap_mx_csr_cfg_stats_mem_t::init() {

}

void cap_mx_csr_sta_rxfifo_mem_t::init() {

}

void cap_mx_csr_cfg_rxfifo_mem_t::init() {

}

void cap_mx_csr_sta_txfifo_mem_t::init() {

}

void cap_mx_csr_cfg_txfifo_mem_t::init() {

}

void cap_mx_csr_cfg_debug_port_t::init() {

}

void cap_mx_csr_cfg_fixer_t::init() {

    set_reset_val(cpp_int("0x201"));
    all(get_reset_val());
}

void cap_mx_csr_csr_intr_t::init() {

}

void cap_mx_csr_sta_mac_sd_status_t::init() {

}

void cap_mx_csr_sta_mac_sd_interrupt_status_t::init() {

}

void cap_mx_csr_cfg_mac_sd_core_to_cntl_t::init() {

    set_reset_val(cpp_int("0xaaaa"));
    all(get_reset_val());
}

void cap_mx_csr_cfg_mac_sd_interrupt_request_t::init() {

}

void cap_mx_csr_cfg_mac_sd_interrupt_t::init() {

}

void cap_mx_csr_cfg_mac_xoff_t::init() {

    set_reset_val(cpp_int("0xff7fbfdfe"));
    all(get_reset_val());
}

void cap_mx_csr_sta_mac_t::init() {

}

void cap_mx_csr_cfg_mac_t::init() {

}

void cap_mx_csr_cfg_mac_tdm_t::init() {

}

void cap_mx_csr_cfg_mac_gbl_t::init() {

    set_reset_val(cpp_int("0x15555555555555404a"));
    all(get_reset_val());
}

void cap_mx_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mx_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_mac_gbl.set_attributes(this,"cfg_mac_gbl", 0x80000 );
    cfg_mac_tdm.set_attributes(this,"cfg_mac_tdm", 0x80010 );
    cfg_mac.set_attributes(this,"cfg_mac", 0x80018 );
    sta_mac.set_attributes(this,"sta_mac", 0x80020 );
    cfg_mac_xoff.set_attributes(this,"cfg_mac_xoff", 0x80030 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_interrupt.set_attributes(this, "cfg_mac_sd_interrupt", 0x82000);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_mac_sd_interrupt[ii].set_field_init_done(true, true);
        cfg_mac_sd_interrupt[ii].set_attributes(this,"cfg_mac_sd_interrupt["+to_string(ii)+"]",  0x82000 + (cfg_mac_sd_interrupt[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_interrupt_request.set_attributes(this, "cfg_mac_sd_interrupt_request", 0x82010);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_mac_sd_interrupt_request[ii].set_field_init_done(true, true);
        cfg_mac_sd_interrupt_request[ii].set_attributes(this,"cfg_mac_sd_interrupt_request["+to_string(ii)+"]",  0x82010 + (cfg_mac_sd_interrupt_request[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_mac_sd_core_to_cntl.set_attributes(this, "cfg_mac_sd_core_to_cntl", 0x82020);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_mac_sd_core_to_cntl[ii].set_field_init_done(true, true);
        cfg_mac_sd_core_to_cntl[ii].set_attributes(this,"cfg_mac_sd_core_to_cntl["+to_string(ii)+"]",  0x82020 + (cfg_mac_sd_core_to_cntl[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mac_sd_interrupt_status.set_attributes(this, "sta_mac_sd_interrupt_status", 0x82030);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_mac_sd_interrupt_status[ii].set_field_init_done(true, true);
        sta_mac_sd_interrupt_status[ii].set_attributes(this,"sta_mac_sd_interrupt_status["+to_string(ii)+"]",  0x82030 + (sta_mac_sd_interrupt_status[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mac_sd_status.set_attributes(this, "sta_mac_sd_status", 0x82040);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_mac_sd_status[ii].set_field_init_done(true, true);
        sta_mac_sd_status[ii].set_attributes(this,"sta_mac_sd_status["+to_string(ii)+"]",  0x82040 + (sta_mac_sd_status[ii].get_byte_size()*ii));
    }
    #endif
    
    csr_intr.set_attributes(this,"csr_intr", 0x82060 );
    cfg_fixer.set_attributes(this,"cfg_fixer", 0x82090 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x82094 );
    cfg_txfifo_mem.set_attributes(this,"cfg_txfifo_mem", 0x82098 );
    sta_txfifo_mem.set_attributes(this,"sta_txfifo_mem", 0x8209c );
    cfg_rxfifo_mem.set_attributes(this,"cfg_rxfifo_mem", 0x820a0 );
    sta_rxfifo_mem.set_attributes(this,"sta_rxfifo_mem", 0x820a4 );
    cfg_stats_mem.set_attributes(this,"cfg_stats_mem", 0x820a8 );
    sta_stats_mem.set_attributes(this,"sta_stats_mem", 0x820ac );
    cfg_fec_mem.set_attributes(this,"cfg_fec_mem", 0x820b0 );
    sta_fec_mem.set_attributes(this,"sta_fec_mem", 0x820b4 );
    sta_ecc.set_attributes(this,"sta_ecc", 0x820d0 );
    dhs_apb.set_attributes(this,"dhs_apb", 0x40000 );
    dhs_mac_stats.set_attributes(this,"dhs_mac_stats", 0x81000 );
    int_groups.set_attributes(this,"int_groups", 0x82070 );
    int_mac.set_attributes(this,"int_mac", 0x82080 );
    int_ecc.set_attributes(this,"int_ecc", 0x820c0 );
}

void cap_mx_csr_int_ecc_int_enable_clear_t::uncorrectable_enable(const cpp_int & _val) { 
    // uncorrectable_enable
    int_var__uncorrectable_enable = _val.convert_to< uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_ecc_int_enable_clear_t::uncorrectable_enable() const {
    return int_var__uncorrectable_enable;
}
    
void cap_mx_csr_int_ecc_int_enable_clear_t::correctable_enable(const cpp_int & _val) { 
    // correctable_enable
    int_var__correctable_enable = _val.convert_to< correctable_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_ecc_int_enable_clear_t::correctable_enable() const {
    return int_var__correctable_enable;
}
    
void cap_mx_csr_int_ecc_int_test_set_t::uncorrectable_interrupt(const cpp_int & _val) { 
    // uncorrectable_interrupt
    int_var__uncorrectable_interrupt = _val.convert_to< uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_ecc_int_test_set_t::uncorrectable_interrupt() const {
    return int_var__uncorrectable_interrupt;
}
    
void cap_mx_csr_int_ecc_int_test_set_t::correctable_interrupt(const cpp_int & _val) { 
    // correctable_interrupt
    int_var__correctable_interrupt = _val.convert_to< correctable_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_ecc_int_test_set_t::correctable_interrupt() const {
    return int_var__correctable_interrupt;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane0_sbe_enable(const cpp_int & _val) { 
    // lane0_sbe_enable
    int_var__lane0_sbe_enable = _val.convert_to< lane0_sbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane0_sbe_enable() const {
    return int_var__lane0_sbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane0_dbe_enable(const cpp_int & _val) { 
    // lane0_dbe_enable
    int_var__lane0_dbe_enable = _val.convert_to< lane0_dbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane0_dbe_enable() const {
    return int_var__lane0_dbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane1_sbe_enable(const cpp_int & _val) { 
    // lane1_sbe_enable
    int_var__lane1_sbe_enable = _val.convert_to< lane1_sbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane1_sbe_enable() const {
    return int_var__lane1_sbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane1_dbe_enable(const cpp_int & _val) { 
    // lane1_dbe_enable
    int_var__lane1_dbe_enable = _val.convert_to< lane1_dbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane1_dbe_enable() const {
    return int_var__lane1_dbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane2_sbe_enable(const cpp_int & _val) { 
    // lane2_sbe_enable
    int_var__lane2_sbe_enable = _val.convert_to< lane2_sbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane2_sbe_enable() const {
    return int_var__lane2_sbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane2_dbe_enable(const cpp_int & _val) { 
    // lane2_dbe_enable
    int_var__lane2_dbe_enable = _val.convert_to< lane2_dbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane2_dbe_enable() const {
    return int_var__lane2_dbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane3_sbe_enable(const cpp_int & _val) { 
    // lane3_sbe_enable
    int_var__lane3_sbe_enable = _val.convert_to< lane3_sbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane3_sbe_enable() const {
    return int_var__lane3_sbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::lane3_dbe_enable(const cpp_int & _val) { 
    // lane3_dbe_enable
    int_var__lane3_dbe_enable = _val.convert_to< lane3_dbe_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::lane3_dbe_enable() const {
    return int_var__lane3_dbe_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::umac_CFG3_intr_enable(const cpp_int & _val) { 
    // umac_CFG3_intr_enable
    int_var__umac_CFG3_intr_enable = _val.convert_to< umac_CFG3_intr_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::umac_CFG3_intr_enable() const {
    return int_var__umac_CFG3_intr_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::umac_CFG3_pslverr_enable(const cpp_int & _val) { 
    // umac_CFG3_pslverr_enable
    int_var__umac_CFG3_pslverr_enable = _val.convert_to< umac_CFG3_pslverr_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::umac_CFG3_pslverr_enable() const {
    return int_var__umac_CFG3_pslverr_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::m0pb_pbus_drdy_enable(const cpp_int & _val) { 
    // m0pb_pbus_drdy_enable
    int_var__m0pb_pbus_drdy_enable = _val.convert_to< m0pb_pbus_drdy_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::m0pb_pbus_drdy_enable() const {
    return int_var__m0pb_pbus_drdy_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::m1pb_pbus_drdy_enable(const cpp_int & _val) { 
    // m1pb_pbus_drdy_enable
    int_var__m1pb_pbus_drdy_enable = _val.convert_to< m1pb_pbus_drdy_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::m1pb_pbus_drdy_enable() const {
    return int_var__m1pb_pbus_drdy_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::m2pb_pbus_drdy_enable(const cpp_int & _val) { 
    // m2pb_pbus_drdy_enable
    int_var__m2pb_pbus_drdy_enable = _val.convert_to< m2pb_pbus_drdy_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::m2pb_pbus_drdy_enable() const {
    return int_var__m2pb_pbus_drdy_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::m3pb_pbus_drdy_enable(const cpp_int & _val) { 
    // m3pb_pbus_drdy_enable
    int_var__m3pb_pbus_drdy_enable = _val.convert_to< m3pb_pbus_drdy_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::m3pb_pbus_drdy_enable() const {
    return int_var__m3pb_pbus_drdy_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx0_missing_sof_enable(const cpp_int & _val) { 
    // rx0_missing_sof_enable
    int_var__rx0_missing_sof_enable = _val.convert_to< rx0_missing_sof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx0_missing_sof_enable() const {
    return int_var__rx0_missing_sof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx0_missing_eof_enable(const cpp_int & _val) { 
    // rx0_missing_eof_enable
    int_var__rx0_missing_eof_enable = _val.convert_to< rx0_missing_eof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx0_missing_eof_enable() const {
    return int_var__rx0_missing_eof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx0_timeout_err_enable(const cpp_int & _val) { 
    // rx0_timeout_err_enable
    int_var__rx0_timeout_err_enable = _val.convert_to< rx0_timeout_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx0_timeout_err_enable() const {
    return int_var__rx0_timeout_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx0_min_size_err_enable(const cpp_int & _val) { 
    // rx0_min_size_err_enable
    int_var__rx0_min_size_err_enable = _val.convert_to< rx0_min_size_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx0_min_size_err_enable() const {
    return int_var__rx0_min_size_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx1_missing_sof_enable(const cpp_int & _val) { 
    // rx1_missing_sof_enable
    int_var__rx1_missing_sof_enable = _val.convert_to< rx1_missing_sof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx1_missing_sof_enable() const {
    return int_var__rx1_missing_sof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx1_missing_eof_enable(const cpp_int & _val) { 
    // rx1_missing_eof_enable
    int_var__rx1_missing_eof_enable = _val.convert_to< rx1_missing_eof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx1_missing_eof_enable() const {
    return int_var__rx1_missing_eof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx1_timeout_err_enable(const cpp_int & _val) { 
    // rx1_timeout_err_enable
    int_var__rx1_timeout_err_enable = _val.convert_to< rx1_timeout_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx1_timeout_err_enable() const {
    return int_var__rx1_timeout_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx1_min_size_err_enable(const cpp_int & _val) { 
    // rx1_min_size_err_enable
    int_var__rx1_min_size_err_enable = _val.convert_to< rx1_min_size_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx1_min_size_err_enable() const {
    return int_var__rx1_min_size_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx2_missing_sof_enable(const cpp_int & _val) { 
    // rx2_missing_sof_enable
    int_var__rx2_missing_sof_enable = _val.convert_to< rx2_missing_sof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx2_missing_sof_enable() const {
    return int_var__rx2_missing_sof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx2_missing_eof_enable(const cpp_int & _val) { 
    // rx2_missing_eof_enable
    int_var__rx2_missing_eof_enable = _val.convert_to< rx2_missing_eof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx2_missing_eof_enable() const {
    return int_var__rx2_missing_eof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx2_timeout_err_enable(const cpp_int & _val) { 
    // rx2_timeout_err_enable
    int_var__rx2_timeout_err_enable = _val.convert_to< rx2_timeout_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx2_timeout_err_enable() const {
    return int_var__rx2_timeout_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx2_min_size_err_enable(const cpp_int & _val) { 
    // rx2_min_size_err_enable
    int_var__rx2_min_size_err_enable = _val.convert_to< rx2_min_size_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx2_min_size_err_enable() const {
    return int_var__rx2_min_size_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx3_missing_sof_enable(const cpp_int & _val) { 
    // rx3_missing_sof_enable
    int_var__rx3_missing_sof_enable = _val.convert_to< rx3_missing_sof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx3_missing_sof_enable() const {
    return int_var__rx3_missing_sof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx3_missing_eof_enable(const cpp_int & _val) { 
    // rx3_missing_eof_enable
    int_var__rx3_missing_eof_enable = _val.convert_to< rx3_missing_eof_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx3_missing_eof_enable() const {
    return int_var__rx3_missing_eof_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx3_timeout_err_enable(const cpp_int & _val) { 
    // rx3_timeout_err_enable
    int_var__rx3_timeout_err_enable = _val.convert_to< rx3_timeout_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx3_timeout_err_enable() const {
    return int_var__rx3_timeout_err_enable;
}
    
void cap_mx_csr_int_mac_int_enable_clear_t::rx3_min_size_err_enable(const cpp_int & _val) { 
    // rx3_min_size_err_enable
    int_var__rx3_min_size_err_enable = _val.convert_to< rx3_min_size_err_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_int_enable_clear_t::rx3_min_size_err_enable() const {
    return int_var__rx3_min_size_err_enable;
}
    
void cap_mx_csr_int_mac_intreg_t::lane0_sbe_interrupt(const cpp_int & _val) { 
    // lane0_sbe_interrupt
    int_var__lane0_sbe_interrupt = _val.convert_to< lane0_sbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane0_sbe_interrupt() const {
    return int_var__lane0_sbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane0_dbe_interrupt(const cpp_int & _val) { 
    // lane0_dbe_interrupt
    int_var__lane0_dbe_interrupt = _val.convert_to< lane0_dbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane0_dbe_interrupt() const {
    return int_var__lane0_dbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane1_sbe_interrupt(const cpp_int & _val) { 
    // lane1_sbe_interrupt
    int_var__lane1_sbe_interrupt = _val.convert_to< lane1_sbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane1_sbe_interrupt() const {
    return int_var__lane1_sbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane1_dbe_interrupt(const cpp_int & _val) { 
    // lane1_dbe_interrupt
    int_var__lane1_dbe_interrupt = _val.convert_to< lane1_dbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane1_dbe_interrupt() const {
    return int_var__lane1_dbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane2_sbe_interrupt(const cpp_int & _val) { 
    // lane2_sbe_interrupt
    int_var__lane2_sbe_interrupt = _val.convert_to< lane2_sbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane2_sbe_interrupt() const {
    return int_var__lane2_sbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane2_dbe_interrupt(const cpp_int & _val) { 
    // lane2_dbe_interrupt
    int_var__lane2_dbe_interrupt = _val.convert_to< lane2_dbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane2_dbe_interrupt() const {
    return int_var__lane2_dbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane3_sbe_interrupt(const cpp_int & _val) { 
    // lane3_sbe_interrupt
    int_var__lane3_sbe_interrupt = _val.convert_to< lane3_sbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane3_sbe_interrupt() const {
    return int_var__lane3_sbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::lane3_dbe_interrupt(const cpp_int & _val) { 
    // lane3_dbe_interrupt
    int_var__lane3_dbe_interrupt = _val.convert_to< lane3_dbe_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::lane3_dbe_interrupt() const {
    return int_var__lane3_dbe_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::umac_CFG3_intr_interrupt(const cpp_int & _val) { 
    // umac_CFG3_intr_interrupt
    int_var__umac_CFG3_intr_interrupt = _val.convert_to< umac_CFG3_intr_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::umac_CFG3_intr_interrupt() const {
    return int_var__umac_CFG3_intr_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::umac_CFG3_pslverr_interrupt(const cpp_int & _val) { 
    // umac_CFG3_pslverr_interrupt
    int_var__umac_CFG3_pslverr_interrupt = _val.convert_to< umac_CFG3_pslverr_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::umac_CFG3_pslverr_interrupt() const {
    return int_var__umac_CFG3_pslverr_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::m0pb_pbus_drdy_interrupt(const cpp_int & _val) { 
    // m0pb_pbus_drdy_interrupt
    int_var__m0pb_pbus_drdy_interrupt = _val.convert_to< m0pb_pbus_drdy_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::m0pb_pbus_drdy_interrupt() const {
    return int_var__m0pb_pbus_drdy_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::m1pb_pbus_drdy_interrupt(const cpp_int & _val) { 
    // m1pb_pbus_drdy_interrupt
    int_var__m1pb_pbus_drdy_interrupt = _val.convert_to< m1pb_pbus_drdy_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::m1pb_pbus_drdy_interrupt() const {
    return int_var__m1pb_pbus_drdy_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::m2pb_pbus_drdy_interrupt(const cpp_int & _val) { 
    // m2pb_pbus_drdy_interrupt
    int_var__m2pb_pbus_drdy_interrupt = _val.convert_to< m2pb_pbus_drdy_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::m2pb_pbus_drdy_interrupt() const {
    return int_var__m2pb_pbus_drdy_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::m3pb_pbus_drdy_interrupt(const cpp_int & _val) { 
    // m3pb_pbus_drdy_interrupt
    int_var__m3pb_pbus_drdy_interrupt = _val.convert_to< m3pb_pbus_drdy_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::m3pb_pbus_drdy_interrupt() const {
    return int_var__m3pb_pbus_drdy_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx0_missing_sof_interrupt(const cpp_int & _val) { 
    // rx0_missing_sof_interrupt
    int_var__rx0_missing_sof_interrupt = _val.convert_to< rx0_missing_sof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx0_missing_sof_interrupt() const {
    return int_var__rx0_missing_sof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx0_missing_eof_interrupt(const cpp_int & _val) { 
    // rx0_missing_eof_interrupt
    int_var__rx0_missing_eof_interrupt = _val.convert_to< rx0_missing_eof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx0_missing_eof_interrupt() const {
    return int_var__rx0_missing_eof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx0_timeout_err_interrupt(const cpp_int & _val) { 
    // rx0_timeout_err_interrupt
    int_var__rx0_timeout_err_interrupt = _val.convert_to< rx0_timeout_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx0_timeout_err_interrupt() const {
    return int_var__rx0_timeout_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx0_min_size_err_interrupt(const cpp_int & _val) { 
    // rx0_min_size_err_interrupt
    int_var__rx0_min_size_err_interrupt = _val.convert_to< rx0_min_size_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx0_min_size_err_interrupt() const {
    return int_var__rx0_min_size_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx1_missing_sof_interrupt(const cpp_int & _val) { 
    // rx1_missing_sof_interrupt
    int_var__rx1_missing_sof_interrupt = _val.convert_to< rx1_missing_sof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx1_missing_sof_interrupt() const {
    return int_var__rx1_missing_sof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx1_missing_eof_interrupt(const cpp_int & _val) { 
    // rx1_missing_eof_interrupt
    int_var__rx1_missing_eof_interrupt = _val.convert_to< rx1_missing_eof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx1_missing_eof_interrupt() const {
    return int_var__rx1_missing_eof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx1_timeout_err_interrupt(const cpp_int & _val) { 
    // rx1_timeout_err_interrupt
    int_var__rx1_timeout_err_interrupt = _val.convert_to< rx1_timeout_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx1_timeout_err_interrupt() const {
    return int_var__rx1_timeout_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx1_min_size_err_interrupt(const cpp_int & _val) { 
    // rx1_min_size_err_interrupt
    int_var__rx1_min_size_err_interrupt = _val.convert_to< rx1_min_size_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx1_min_size_err_interrupt() const {
    return int_var__rx1_min_size_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx2_missing_sof_interrupt(const cpp_int & _val) { 
    // rx2_missing_sof_interrupt
    int_var__rx2_missing_sof_interrupt = _val.convert_to< rx2_missing_sof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx2_missing_sof_interrupt() const {
    return int_var__rx2_missing_sof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx2_missing_eof_interrupt(const cpp_int & _val) { 
    // rx2_missing_eof_interrupt
    int_var__rx2_missing_eof_interrupt = _val.convert_to< rx2_missing_eof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx2_missing_eof_interrupt() const {
    return int_var__rx2_missing_eof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx2_timeout_err_interrupt(const cpp_int & _val) { 
    // rx2_timeout_err_interrupt
    int_var__rx2_timeout_err_interrupt = _val.convert_to< rx2_timeout_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx2_timeout_err_interrupt() const {
    return int_var__rx2_timeout_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx2_min_size_err_interrupt(const cpp_int & _val) { 
    // rx2_min_size_err_interrupt
    int_var__rx2_min_size_err_interrupt = _val.convert_to< rx2_min_size_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx2_min_size_err_interrupt() const {
    return int_var__rx2_min_size_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx3_missing_sof_interrupt(const cpp_int & _val) { 
    // rx3_missing_sof_interrupt
    int_var__rx3_missing_sof_interrupt = _val.convert_to< rx3_missing_sof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx3_missing_sof_interrupt() const {
    return int_var__rx3_missing_sof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx3_missing_eof_interrupt(const cpp_int & _val) { 
    // rx3_missing_eof_interrupt
    int_var__rx3_missing_eof_interrupt = _val.convert_to< rx3_missing_eof_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx3_missing_eof_interrupt() const {
    return int_var__rx3_missing_eof_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx3_timeout_err_interrupt(const cpp_int & _val) { 
    // rx3_timeout_err_interrupt
    int_var__rx3_timeout_err_interrupt = _val.convert_to< rx3_timeout_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx3_timeout_err_interrupt() const {
    return int_var__rx3_timeout_err_interrupt;
}
    
void cap_mx_csr_int_mac_intreg_t::rx3_min_size_err_interrupt(const cpp_int & _val) { 
    // rx3_min_size_err_interrupt
    int_var__rx3_min_size_err_interrupt = _val.convert_to< rx3_min_size_err_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_int_mac_intreg_t::rx3_min_size_err_interrupt() const {
    return int_var__rx3_min_size_err_interrupt;
}
    
void cap_mx_csr_intreg_status_t::int_mac_interrupt(const cpp_int & _val) { 
    // int_mac_interrupt
    int_var__int_mac_interrupt = _val.convert_to< int_mac_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_intreg_status_t::int_mac_interrupt() const {
    return int_var__int_mac_interrupt;
}
    
void cap_mx_csr_intreg_status_t::int_ecc_interrupt(const cpp_int & _val) { 
    // int_ecc_interrupt
    int_var__int_ecc_interrupt = _val.convert_to< int_ecc_interrupt_cpp_int_t >();
}

cpp_int cap_mx_csr_intreg_status_t::int_ecc_interrupt() const {
    return int_var__int_ecc_interrupt;
}
    
void cap_mx_csr_int_groups_int_enable_rw_reg_t::int_mac_enable(const cpp_int & _val) { 
    // int_mac_enable
    int_var__int_mac_enable = _val.convert_to< int_mac_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_groups_int_enable_rw_reg_t::int_mac_enable() const {
    return int_var__int_mac_enable;
}
    
void cap_mx_csr_int_groups_int_enable_rw_reg_t::int_ecc_enable(const cpp_int & _val) { 
    // int_ecc_enable
    int_var__int_ecc_enable = _val.convert_to< int_ecc_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_int_groups_int_enable_rw_reg_t::int_ecc_enable() const {
    return int_var__int_ecc_enable;
}
    
void cap_mx_csr_dhs_mac_stats_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mx_csr_dhs_mac_stats_entry_t::value() const {
    return int_var__value;
}
    
void cap_mx_csr_dhs_apb_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_mx_csr_dhs_apb_entry_t::data() const {
    return int_var__data;
}
    
void cap_mx_csr_sta_ecc_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_ecc_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_mx_csr_sta_ecc_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_ecc_t::correctable() const {
    return int_var__correctable;
}
    
void cap_mx_csr_sta_ecc_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_ecc_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_mx_csr_sta_ecc_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_ecc_t::addr() const {
    return int_var__addr;
}
    
void cap_mx_csr_sta_fec_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_fec_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mx_csr_sta_fec_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_fec_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mx_csr_cfg_fec_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_fec_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mx_csr_cfg_fec_mem_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_fec_mem_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_mx_csr_cfg_fec_mem_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_fec_mem_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_mx_csr_sta_stats_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_stats_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mx_csr_sta_stats_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_stats_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mx_csr_cfg_stats_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_stats_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mx_csr_cfg_stats_mem_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_stats_mem_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_mx_csr_cfg_stats_mem_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_stats_mem_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_mx_csr_sta_rxfifo_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_rxfifo_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mx_csr_sta_rxfifo_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_rxfifo_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mx_csr_cfg_rxfifo_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_rxfifo_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mx_csr_sta_txfifo_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_txfifo_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mx_csr_sta_txfifo_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_txfifo_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mx_csr_cfg_txfifo_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_txfifo_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mx_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_debug_port_t::enable() const {
    return int_var__enable;
}
    
void cap_mx_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_debug_port_t::select() const {
    return int_var__select;
}
    
void cap_mx_csr_cfg_fixer_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_fixer_t::enable() const {
    return int_var__enable;
}
    
void cap_mx_csr_cfg_fixer_t::timeout(const cpp_int & _val) { 
    // timeout
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_fixer_t::timeout() const {
    return int_var__timeout;
}
    
void cap_mx_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_mx_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_mx_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_mx_csr_sta_mac_sd_status_t::rx_idle_detect(const cpp_int & _val) { 
    // rx_idle_detect
    int_var__rx_idle_detect = _val.convert_to< rx_idle_detect_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_status_t::rx_idle_detect() const {
    return int_var__rx_idle_detect;
}
    
void cap_mx_csr_sta_mac_sd_status_t::rx_rdy(const cpp_int & _val) { 
    // rx_rdy
    int_var__rx_rdy = _val.convert_to< rx_rdy_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_status_t::rx_rdy() const {
    return int_var__rx_rdy;
}
    
void cap_mx_csr_sta_mac_sd_status_t::tx_rdy(const cpp_int & _val) { 
    // tx_rdy
    int_var__tx_rdy = _val.convert_to< tx_rdy_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_status_t::tx_rdy() const {
    return int_var__tx_rdy;
}
    
void cap_mx_csr_sta_mac_sd_status_t::core_status(const cpp_int & _val) { 
    // core_status
    int_var__core_status = _val.convert_to< core_status_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_status_t::core_status() const {
    return int_var__core_status;
}
    
void cap_mx_csr_sta_mac_sd_interrupt_status_t::in_progress(const cpp_int & _val) { 
    // in_progress
    int_var__in_progress = _val.convert_to< in_progress_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_interrupt_status_t::in_progress() const {
    return int_var__in_progress;
}
    
void cap_mx_csr_sta_mac_sd_interrupt_status_t::data_out(const cpp_int & _val) { 
    // data_out
    int_var__data_out = _val.convert_to< data_out_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_sd_interrupt_status_t::data_out() const {
    return int_var__data_out;
}
    
void cap_mx_csr_cfg_mac_sd_core_to_cntl_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_sd_core_to_cntl_t::value() const {
    return int_var__value;
}
    
void cap_mx_csr_cfg_mac_sd_interrupt_request_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_sd_interrupt_request_t::val() const {
    return int_var__val;
}
    
void cap_mx_csr_cfg_mac_sd_interrupt_t::code(const cpp_int & _val) { 
    // code
    int_var__code = _val.convert_to< code_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_sd_interrupt_t::code() const {
    return int_var__code;
}
    
void cap_mx_csr_cfg_mac_sd_interrupt_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_sd_interrupt_t::data() const {
    return int_var__data;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx0fcxoff_i(const cpp_int & _val) { 
    // ff_tx0fcxoff_i
    int_var__ff_tx0fcxoff_i = _val.convert_to< ff_tx0fcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx0fcxoff_i() const {
    return int_var__ff_tx0fcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx0pfcxoff_i(const cpp_int & _val) { 
    // ff_tx0pfcxoff_i
    int_var__ff_tx0pfcxoff_i = _val.convert_to< ff_tx0pfcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx0pfcxoff_i() const {
    return int_var__ff_tx0pfcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx1fcxoff_i(const cpp_int & _val) { 
    // ff_tx1fcxoff_i
    int_var__ff_tx1fcxoff_i = _val.convert_to< ff_tx1fcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx1fcxoff_i() const {
    return int_var__ff_tx1fcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx1pfcxoff_i(const cpp_int & _val) { 
    // ff_tx1pfcxoff_i
    int_var__ff_tx1pfcxoff_i = _val.convert_to< ff_tx1pfcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx1pfcxoff_i() const {
    return int_var__ff_tx1pfcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx2fcxoff_i(const cpp_int & _val) { 
    // ff_tx2fcxoff_i
    int_var__ff_tx2fcxoff_i = _val.convert_to< ff_tx2fcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx2fcxoff_i() const {
    return int_var__ff_tx2fcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx2pfcxoff_i(const cpp_int & _val) { 
    // ff_tx2pfcxoff_i
    int_var__ff_tx2pfcxoff_i = _val.convert_to< ff_tx2pfcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx2pfcxoff_i() const {
    return int_var__ff_tx2pfcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx3fcxoff_i(const cpp_int & _val) { 
    // ff_tx3fcxoff_i
    int_var__ff_tx3fcxoff_i = _val.convert_to< ff_tx3fcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx3fcxoff_i() const {
    return int_var__ff_tx3fcxoff_i;
}
    
void cap_mx_csr_cfg_mac_xoff_t::ff_tx3pfcxoff_i(const cpp_int & _val) { 
    // ff_tx3pfcxoff_i
    int_var__ff_tx3pfcxoff_i = _val.convert_to< ff_tx3pfcxoff_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_xoff_t::ff_tx3pfcxoff_i() const {
    return int_var__ff_tx3pfcxoff_i;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxcorecontrol0_o(const cpp_int & _val) { 
    // serdes_rxcorecontrol0_o
    int_var__serdes_rxcorecontrol0_o = _val.convert_to< serdes_rxcorecontrol0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxcorecontrol0_o() const {
    return int_var__serdes_rxcorecontrol0_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txcorecontrol0_o(const cpp_int & _val) { 
    // serdes_txcorecontrol0_o
    int_var__serdes_txcorecontrol0_o = _val.convert_to< serdes_txcorecontrol0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txcorecontrol0_o() const {
    return int_var__serdes_txcorecontrol0_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txafull0_o(const cpp_int & _val) { 
    // ff_txafull0_o
    int_var__ff_txafull0_o = _val.convert_to< ff_txafull0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txafull0_o() const {
    return int_var__ff_txafull0_o;
}
    
void cap_mx_csr_sta_mac_t::ff_serdes0sigok_o(const cpp_int & _val) { 
    // ff_serdes0sigok_o
    int_var__ff_serdes0sigok_o = _val.convert_to< ff_serdes0sigok_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_serdes0sigok_o() const {
    return int_var__ff_serdes0sigok_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx0sync_o(const cpp_int & _val) { 
    // ff_rx0sync_o
    int_var__ff_rx0sync_o = _val.convert_to< ff_rx0sync_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx0sync_o() const {
    return int_var__ff_rx0sync_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx0idle_o(const cpp_int & _val) { 
    // ff_tx0idle_o
    int_var__ff_tx0idle_o = _val.convert_to< ff_tx0idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx0idle_o() const {
    return int_var__ff_tx0idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx0idle_o(const cpp_int & _val) { 
    // ff_rx0idle_o
    int_var__ff_rx0idle_o = _val.convert_to< ff_rx0idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx0idle_o() const {
    return int_var__ff_rx0idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx0good_o(const cpp_int & _val) { 
    // ff_tx0good_o
    int_var__ff_tx0good_o = _val.convert_to< ff_tx0good_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx0good_o() const {
    return int_var__ff_tx0good_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txquiet_ts_0_o(const cpp_int & _val) { 
    // serdes_txquiet_ts_0_o
    int_var__serdes_txquiet_ts_0_o = _val.convert_to< serdes_txquiet_ts_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txquiet_ts_0_o() const {
    return int_var__serdes_txquiet_ts_0_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_0_o(const cpp_int & _val) { 
    // serdes_rxquiet_rs_0_o
    int_var__serdes_rxquiet_rs_0_o = _val.convert_to< serdes_rxquiet_rs_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_0_o() const {
    return int_var__serdes_rxquiet_rs_0_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx0read_o(const cpp_int & _val) { 
    // ff_tx0read_o
    int_var__ff_tx0read_o = _val.convert_to< ff_tx0read_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx0read_o() const {
    return int_var__ff_tx0read_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txlpi_0_o(const cpp_int & _val) { 
    // ff_txlpi_0_o
    int_var__ff_txlpi_0_o = _val.convert_to< ff_txlpi_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txlpi_0_o() const {
    return int_var__ff_txlpi_0_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rxlpi_0_o(const cpp_int & _val) { 
    // ff_rxlpi_0_o
    int_var__ff_rxlpi_0_o = _val.convert_to< ff_rxlpi_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rxlpi_0_o() const {
    return int_var__ff_rxlpi_0_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_0_o(const cpp_int & _val) { 
    // serdes_rxlpiactive_rs_0_o
    int_var__serdes_rxlpiactive_rs_0_o = _val.convert_to< serdes_rxlpiactive_rs_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_0_o() const {
    return int_var__serdes_rxlpiactive_rs_0_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txalert_ts_0_o(const cpp_int & _val) { 
    // serdes_txalert_ts_0_o
    int_var__serdes_txalert_ts_0_o = _val.convert_to< serdes_txalert_ts_0_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txalert_ts_0_o() const {
    return int_var__serdes_txalert_ts_0_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxcorecontrol1_o(const cpp_int & _val) { 
    // serdes_rxcorecontrol1_o
    int_var__serdes_rxcorecontrol1_o = _val.convert_to< serdes_rxcorecontrol1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxcorecontrol1_o() const {
    return int_var__serdes_rxcorecontrol1_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txcorecontrol1_o(const cpp_int & _val) { 
    // serdes_txcorecontrol1_o
    int_var__serdes_txcorecontrol1_o = _val.convert_to< serdes_txcorecontrol1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txcorecontrol1_o() const {
    return int_var__serdes_txcorecontrol1_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txafull1_o(const cpp_int & _val) { 
    // ff_txafull1_o
    int_var__ff_txafull1_o = _val.convert_to< ff_txafull1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txafull1_o() const {
    return int_var__ff_txafull1_o;
}
    
void cap_mx_csr_sta_mac_t::ff_serdes1sigok_o(const cpp_int & _val) { 
    // ff_serdes1sigok_o
    int_var__ff_serdes1sigok_o = _val.convert_to< ff_serdes1sigok_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_serdes1sigok_o() const {
    return int_var__ff_serdes1sigok_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx1sync_o(const cpp_int & _val) { 
    // ff_rx1sync_o
    int_var__ff_rx1sync_o = _val.convert_to< ff_rx1sync_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx1sync_o() const {
    return int_var__ff_rx1sync_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx1idle_o(const cpp_int & _val) { 
    // ff_tx1idle_o
    int_var__ff_tx1idle_o = _val.convert_to< ff_tx1idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx1idle_o() const {
    return int_var__ff_tx1idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx1idle_o(const cpp_int & _val) { 
    // ff_rx1idle_o
    int_var__ff_rx1idle_o = _val.convert_to< ff_rx1idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx1idle_o() const {
    return int_var__ff_rx1idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx1good_o(const cpp_int & _val) { 
    // ff_tx1good_o
    int_var__ff_tx1good_o = _val.convert_to< ff_tx1good_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx1good_o() const {
    return int_var__ff_tx1good_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txquiet_ts_1_o(const cpp_int & _val) { 
    // serdes_txquiet_ts_1_o
    int_var__serdes_txquiet_ts_1_o = _val.convert_to< serdes_txquiet_ts_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txquiet_ts_1_o() const {
    return int_var__serdes_txquiet_ts_1_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_1_o(const cpp_int & _val) { 
    // serdes_rxquiet_rs_1_o
    int_var__serdes_rxquiet_rs_1_o = _val.convert_to< serdes_rxquiet_rs_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_1_o() const {
    return int_var__serdes_rxquiet_rs_1_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx1read_o(const cpp_int & _val) { 
    // ff_tx1read_o
    int_var__ff_tx1read_o = _val.convert_to< ff_tx1read_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx1read_o() const {
    return int_var__ff_tx1read_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txlpi_1_o(const cpp_int & _val) { 
    // ff_txlpi_1_o
    int_var__ff_txlpi_1_o = _val.convert_to< ff_txlpi_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txlpi_1_o() const {
    return int_var__ff_txlpi_1_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rxlpi_1_o(const cpp_int & _val) { 
    // ff_rxlpi_1_o
    int_var__ff_rxlpi_1_o = _val.convert_to< ff_rxlpi_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rxlpi_1_o() const {
    return int_var__ff_rxlpi_1_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_1_o(const cpp_int & _val) { 
    // serdes_rxlpiactive_rs_1_o
    int_var__serdes_rxlpiactive_rs_1_o = _val.convert_to< serdes_rxlpiactive_rs_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_1_o() const {
    return int_var__serdes_rxlpiactive_rs_1_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txalert_ts_1_o(const cpp_int & _val) { 
    // serdes_txalert_ts_1_o
    int_var__serdes_txalert_ts_1_o = _val.convert_to< serdes_txalert_ts_1_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txalert_ts_1_o() const {
    return int_var__serdes_txalert_ts_1_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxcorecontrol2_o(const cpp_int & _val) { 
    // serdes_rxcorecontrol2_o
    int_var__serdes_rxcorecontrol2_o = _val.convert_to< serdes_rxcorecontrol2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxcorecontrol2_o() const {
    return int_var__serdes_rxcorecontrol2_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txcorecontrol2_o(const cpp_int & _val) { 
    // serdes_txcorecontrol2_o
    int_var__serdes_txcorecontrol2_o = _val.convert_to< serdes_txcorecontrol2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txcorecontrol2_o() const {
    return int_var__serdes_txcorecontrol2_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txafull2_o(const cpp_int & _val) { 
    // ff_txafull2_o
    int_var__ff_txafull2_o = _val.convert_to< ff_txafull2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txafull2_o() const {
    return int_var__ff_txafull2_o;
}
    
void cap_mx_csr_sta_mac_t::ff_serdes2sigok_o(const cpp_int & _val) { 
    // ff_serdes2sigok_o
    int_var__ff_serdes2sigok_o = _val.convert_to< ff_serdes2sigok_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_serdes2sigok_o() const {
    return int_var__ff_serdes2sigok_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx2sync_o(const cpp_int & _val) { 
    // ff_rx2sync_o
    int_var__ff_rx2sync_o = _val.convert_to< ff_rx2sync_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx2sync_o() const {
    return int_var__ff_rx2sync_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx2idle_o(const cpp_int & _val) { 
    // ff_tx2idle_o
    int_var__ff_tx2idle_o = _val.convert_to< ff_tx2idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx2idle_o() const {
    return int_var__ff_tx2idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx2idle_o(const cpp_int & _val) { 
    // ff_rx2idle_o
    int_var__ff_rx2idle_o = _val.convert_to< ff_rx2idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx2idle_o() const {
    return int_var__ff_rx2idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx2good_o(const cpp_int & _val) { 
    // ff_tx2good_o
    int_var__ff_tx2good_o = _val.convert_to< ff_tx2good_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx2good_o() const {
    return int_var__ff_tx2good_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txquiet_ts_2_o(const cpp_int & _val) { 
    // serdes_txquiet_ts_2_o
    int_var__serdes_txquiet_ts_2_o = _val.convert_to< serdes_txquiet_ts_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txquiet_ts_2_o() const {
    return int_var__serdes_txquiet_ts_2_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_2_o(const cpp_int & _val) { 
    // serdes_rxquiet_rs_2_o
    int_var__serdes_rxquiet_rs_2_o = _val.convert_to< serdes_rxquiet_rs_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_2_o() const {
    return int_var__serdes_rxquiet_rs_2_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx2read_o(const cpp_int & _val) { 
    // ff_tx2read_o
    int_var__ff_tx2read_o = _val.convert_to< ff_tx2read_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx2read_o() const {
    return int_var__ff_tx2read_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txlpi_2_o(const cpp_int & _val) { 
    // ff_txlpi_2_o
    int_var__ff_txlpi_2_o = _val.convert_to< ff_txlpi_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txlpi_2_o() const {
    return int_var__ff_txlpi_2_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rxlpi_2_o(const cpp_int & _val) { 
    // ff_rxlpi_2_o
    int_var__ff_rxlpi_2_o = _val.convert_to< ff_rxlpi_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rxlpi_2_o() const {
    return int_var__ff_rxlpi_2_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_2_o(const cpp_int & _val) { 
    // serdes_rxlpiactive_rs_2_o
    int_var__serdes_rxlpiactive_rs_2_o = _val.convert_to< serdes_rxlpiactive_rs_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_2_o() const {
    return int_var__serdes_rxlpiactive_rs_2_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txalert_ts_2_o(const cpp_int & _val) { 
    // serdes_txalert_ts_2_o
    int_var__serdes_txalert_ts_2_o = _val.convert_to< serdes_txalert_ts_2_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txalert_ts_2_o() const {
    return int_var__serdes_txalert_ts_2_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxcorecontrol3_o(const cpp_int & _val) { 
    // serdes_rxcorecontrol3_o
    int_var__serdes_rxcorecontrol3_o = _val.convert_to< serdes_rxcorecontrol3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxcorecontrol3_o() const {
    return int_var__serdes_rxcorecontrol3_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txcorecontrol3_o(const cpp_int & _val) { 
    // serdes_txcorecontrol3_o
    int_var__serdes_txcorecontrol3_o = _val.convert_to< serdes_txcorecontrol3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txcorecontrol3_o() const {
    return int_var__serdes_txcorecontrol3_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txafull3_o(const cpp_int & _val) { 
    // ff_txafull3_o
    int_var__ff_txafull3_o = _val.convert_to< ff_txafull3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txafull3_o() const {
    return int_var__ff_txafull3_o;
}
    
void cap_mx_csr_sta_mac_t::ff_serdes3sigok_o(const cpp_int & _val) { 
    // ff_serdes3sigok_o
    int_var__ff_serdes3sigok_o = _val.convert_to< ff_serdes3sigok_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_serdes3sigok_o() const {
    return int_var__ff_serdes3sigok_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx3sync_o(const cpp_int & _val) { 
    // ff_rx3sync_o
    int_var__ff_rx3sync_o = _val.convert_to< ff_rx3sync_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx3sync_o() const {
    return int_var__ff_rx3sync_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx3idle_o(const cpp_int & _val) { 
    // ff_tx3idle_o
    int_var__ff_tx3idle_o = _val.convert_to< ff_tx3idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx3idle_o() const {
    return int_var__ff_tx3idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rx3idle_o(const cpp_int & _val) { 
    // ff_rx3idle_o
    int_var__ff_rx3idle_o = _val.convert_to< ff_rx3idle_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rx3idle_o() const {
    return int_var__ff_rx3idle_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx3good_o(const cpp_int & _val) { 
    // ff_tx3good_o
    int_var__ff_tx3good_o = _val.convert_to< ff_tx3good_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx3good_o() const {
    return int_var__ff_tx3good_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txquiet_ts_3_o(const cpp_int & _val) { 
    // serdes_txquiet_ts_3_o
    int_var__serdes_txquiet_ts_3_o = _val.convert_to< serdes_txquiet_ts_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txquiet_ts_3_o() const {
    return int_var__serdes_txquiet_ts_3_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_3_o(const cpp_int & _val) { 
    // serdes_rxquiet_rs_3_o
    int_var__serdes_rxquiet_rs_3_o = _val.convert_to< serdes_rxquiet_rs_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxquiet_rs_3_o() const {
    return int_var__serdes_rxquiet_rs_3_o;
}
    
void cap_mx_csr_sta_mac_t::ff_tx3read_o(const cpp_int & _val) { 
    // ff_tx3read_o
    int_var__ff_tx3read_o = _val.convert_to< ff_tx3read_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_tx3read_o() const {
    return int_var__ff_tx3read_o;
}
    
void cap_mx_csr_sta_mac_t::ff_txlpi_3_o(const cpp_int & _val) { 
    // ff_txlpi_3_o
    int_var__ff_txlpi_3_o = _val.convert_to< ff_txlpi_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_txlpi_3_o() const {
    return int_var__ff_txlpi_3_o;
}
    
void cap_mx_csr_sta_mac_t::ff_rxlpi_3_o(const cpp_int & _val) { 
    // ff_rxlpi_3_o
    int_var__ff_rxlpi_3_o = _val.convert_to< ff_rxlpi_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::ff_rxlpi_3_o() const {
    return int_var__ff_rxlpi_3_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_3_o(const cpp_int & _val) { 
    // serdes_rxlpiactive_rs_3_o
    int_var__serdes_rxlpiactive_rs_3_o = _val.convert_to< serdes_rxlpiactive_rs_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_rxlpiactive_rs_3_o() const {
    return int_var__serdes_rxlpiactive_rs_3_o;
}
    
void cap_mx_csr_sta_mac_t::serdes_txalert_ts_3_o(const cpp_int & _val) { 
    // serdes_txalert_ts_3_o
    int_var__serdes_txalert_ts_3_o = _val.convert_to< serdes_txalert_ts_3_o_cpp_int_t >();
}

cpp_int cap_mx_csr_sta_mac_t::serdes_txalert_ts_3_o() const {
    return int_var__serdes_txalert_ts_3_o;
}
    
void cap_mx_csr_cfg_mac_t::mac_rxsigok_i_sel(const cpp_int & _val) { 
    // mac_rxsigok_i_sel
    int_var__mac_rxsigok_i_sel = _val.convert_to< mac_rxsigok_i_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::mac_rxsigok_i_sel() const {
    return int_var__mac_rxsigok_i_sel;
}
    
void cap_mx_csr_cfg_mac_t::mac_rxenergydetect_rs_i_sel(const cpp_int & _val) { 
    // mac_rxenergydetect_rs_i_sel
    int_var__mac_rxenergydetect_rs_i_sel = _val.convert_to< mac_rxenergydetect_rs_i_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::mac_rxenergydetect_rs_i_sel() const {
    return int_var__mac_rxenergydetect_rs_i_sel;
}
    
void cap_mx_csr_cfg_mac_t::mac_tx_corestatus_i_sel(const cpp_int & _val) { 
    // mac_tx_corestatus_i_sel
    int_var__mac_tx_corestatus_i_sel = _val.convert_to< mac_tx_corestatus_i_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::mac_tx_corestatus_i_sel() const {
    return int_var__mac_tx_corestatus_i_sel;
}
    
void cap_mx_csr_cfg_mac_t::mac_rx_corestatus_i_sel(const cpp_int & _val) { 
    // mac_rx_corestatus_i_sel
    int_var__mac_rx_corestatus_i_sel = _val.convert_to< mac_rx_corestatus_i_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::mac_rx_corestatus_i_sel() const {
    return int_var__mac_rx_corestatus_i_sel;
}
    
void cap_mx_csr_cfg_mac_t::core_to_cntl_7_5_sel(const cpp_int & _val) { 
    // core_to_cntl_7_5_sel
    int_var__core_to_cntl_7_5_sel = _val.convert_to< core_to_cntl_7_5_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::core_to_cntl_7_5_sel() const {
    return int_var__core_to_cntl_7_5_sel;
}
    
void cap_mx_csr_cfg_mac_t::core_to_cntl_9_8_sel(const cpp_int & _val) { 
    // core_to_cntl_9_8_sel
    int_var__core_to_cntl_9_8_sel = _val.convert_to< core_to_cntl_9_8_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::core_to_cntl_9_8_sel() const {
    return int_var__core_to_cntl_9_8_sel;
}
    
void cap_mx_csr_cfg_mac_t::core_to_cntl_15_10_sel(const cpp_int & _val) { 
    // core_to_cntl_15_10_sel
    int_var__core_to_cntl_15_10_sel = _val.convert_to< core_to_cntl_15_10_sel_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::core_to_cntl_15_10_sel() const {
    return int_var__core_to_cntl_15_10_sel;
}
    
void cap_mx_csr_cfg_mac_t::serdes0_rxsigok_i(const cpp_int & _val) { 
    // serdes0_rxsigok_i
    int_var__serdes0_rxsigok_i = _val.convert_to< serdes0_rxsigok_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes0_rxsigok_i() const {
    return int_var__serdes0_rxsigok_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes0_tx_override_in(const cpp_int & _val) { 
    // serdes0_tx_override_in
    int_var__serdes0_tx_override_in = _val.convert_to< serdes0_tx_override_in_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes0_tx_override_in() const {
    return int_var__serdes0_tx_override_in;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_0_i(const cpp_int & _val) { 
    // serdes_rxenergydetect_rs_0_i
    int_var__serdes_rxenergydetect_rs_0_i = _val.convert_to< serdes_rxenergydetect_rs_0_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_0_i() const {
    return int_var__serdes_rxenergydetect_rs_0_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_txcorestatus_0_i(const cpp_int & _val) { 
    // serdes_txcorestatus_0_i
    int_var__serdes_txcorestatus_0_i = _val.convert_to< serdes_txcorestatus_0_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_txcorestatus_0_i() const {
    return int_var__serdes_txcorestatus_0_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_0_i(const cpp_int & _val) { 
    // serdes_rxcorestatus_0_i
    int_var__serdes_rxcorestatus_0_i = _val.convert_to< serdes_rxcorestatus_0_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_0_i() const {
    return int_var__serdes_rxcorestatus_0_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes1_rxsigok_i(const cpp_int & _val) { 
    // serdes1_rxsigok_i
    int_var__serdes1_rxsigok_i = _val.convert_to< serdes1_rxsigok_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes1_rxsigok_i() const {
    return int_var__serdes1_rxsigok_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes1_tx_override_in(const cpp_int & _val) { 
    // serdes1_tx_override_in
    int_var__serdes1_tx_override_in = _val.convert_to< serdes1_tx_override_in_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes1_tx_override_in() const {
    return int_var__serdes1_tx_override_in;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_1_i(const cpp_int & _val) { 
    // serdes_rxenergydetect_rs_1_i
    int_var__serdes_rxenergydetect_rs_1_i = _val.convert_to< serdes_rxenergydetect_rs_1_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_1_i() const {
    return int_var__serdes_rxenergydetect_rs_1_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_txcorestatus_1_i(const cpp_int & _val) { 
    // serdes_txcorestatus_1_i
    int_var__serdes_txcorestatus_1_i = _val.convert_to< serdes_txcorestatus_1_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_txcorestatus_1_i() const {
    return int_var__serdes_txcorestatus_1_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_1_i(const cpp_int & _val) { 
    // serdes_rxcorestatus_1_i
    int_var__serdes_rxcorestatus_1_i = _val.convert_to< serdes_rxcorestatus_1_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_1_i() const {
    return int_var__serdes_rxcorestatus_1_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes2_rxsigok_i(const cpp_int & _val) { 
    // serdes2_rxsigok_i
    int_var__serdes2_rxsigok_i = _val.convert_to< serdes2_rxsigok_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes2_rxsigok_i() const {
    return int_var__serdes2_rxsigok_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes2_tx_override_in(const cpp_int & _val) { 
    // serdes2_tx_override_in
    int_var__serdes2_tx_override_in = _val.convert_to< serdes2_tx_override_in_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes2_tx_override_in() const {
    return int_var__serdes2_tx_override_in;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_2_i(const cpp_int & _val) { 
    // serdes_rxenergydetect_rs_2_i
    int_var__serdes_rxenergydetect_rs_2_i = _val.convert_to< serdes_rxenergydetect_rs_2_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_2_i() const {
    return int_var__serdes_rxenergydetect_rs_2_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_txcorestatus_2_i(const cpp_int & _val) { 
    // serdes_txcorestatus_2_i
    int_var__serdes_txcorestatus_2_i = _val.convert_to< serdes_txcorestatus_2_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_txcorestatus_2_i() const {
    return int_var__serdes_txcorestatus_2_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_2_i(const cpp_int & _val) { 
    // serdes_rxcorestatus_2_i
    int_var__serdes_rxcorestatus_2_i = _val.convert_to< serdes_rxcorestatus_2_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_2_i() const {
    return int_var__serdes_rxcorestatus_2_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes3_rxsigok_i(const cpp_int & _val) { 
    // serdes3_rxsigok_i
    int_var__serdes3_rxsigok_i = _val.convert_to< serdes3_rxsigok_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes3_rxsigok_i() const {
    return int_var__serdes3_rxsigok_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes3_tx_override_in(const cpp_int & _val) { 
    // serdes3_tx_override_in
    int_var__serdes3_tx_override_in = _val.convert_to< serdes3_tx_override_in_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes3_tx_override_in() const {
    return int_var__serdes3_tx_override_in;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_3_i(const cpp_int & _val) { 
    // serdes_rxenergydetect_rs_3_i
    int_var__serdes_rxenergydetect_rs_3_i = _val.convert_to< serdes_rxenergydetect_rs_3_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxenergydetect_rs_3_i() const {
    return int_var__serdes_rxenergydetect_rs_3_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_txcorestatus_3_i(const cpp_int & _val) { 
    // serdes_txcorestatus_3_i
    int_var__serdes_txcorestatus_3_i = _val.convert_to< serdes_txcorestatus_3_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_txcorestatus_3_i() const {
    return int_var__serdes_txcorestatus_3_i;
}
    
void cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_3_i(const cpp_int & _val) { 
    // serdes_rxcorestatus_3_i
    int_var__serdes_rxcorestatus_3_i = _val.convert_to< serdes_rxcorestatus_3_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::serdes_rxcorestatus_3_i() const {
    return int_var__serdes_rxcorestatus_3_i;
}
    
void cap_mx_csr_cfg_mac_t::ff_txlpi_0_i(const cpp_int & _val) { 
    // ff_txlpi_0_i
    int_var__ff_txlpi_0_i = _val.convert_to< ff_txlpi_0_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::ff_txlpi_0_i() const {
    return int_var__ff_txlpi_0_i;
}
    
void cap_mx_csr_cfg_mac_t::ff_txlpi_1_i(const cpp_int & _val) { 
    // ff_txlpi_1_i
    int_var__ff_txlpi_1_i = _val.convert_to< ff_txlpi_1_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::ff_txlpi_1_i() const {
    return int_var__ff_txlpi_1_i;
}
    
void cap_mx_csr_cfg_mac_t::ff_txlpi_2_i(const cpp_int & _val) { 
    // ff_txlpi_2_i
    int_var__ff_txlpi_2_i = _val.convert_to< ff_txlpi_2_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::ff_txlpi_2_i() const {
    return int_var__ff_txlpi_2_i;
}
    
void cap_mx_csr_cfg_mac_t::ff_txlpi_3_i(const cpp_int & _val) { 
    // ff_txlpi_3_i
    int_var__ff_txlpi_3_i = _val.convert_to< ff_txlpi_3_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_t::ff_txlpi_3_i() const {
    return int_var__ff_txlpi_3_i;
}
    
void cap_mx_csr_cfg_mac_tdm_t::slot0(const cpp_int & _val) { 
    // slot0
    int_var__slot0 = _val.convert_to< slot0_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_tdm_t::slot0() const {
    return int_var__slot0;
}
    
void cap_mx_csr_cfg_mac_tdm_t::slot1(const cpp_int & _val) { 
    // slot1
    int_var__slot1 = _val.convert_to< slot1_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_tdm_t::slot1() const {
    return int_var__slot1;
}
    
void cap_mx_csr_cfg_mac_tdm_t::slot2(const cpp_int & _val) { 
    // slot2
    int_var__slot2 = _val.convert_to< slot2_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_tdm_t::slot2() const {
    return int_var__slot2;
}
    
void cap_mx_csr_cfg_mac_tdm_t::slot3(const cpp_int & _val) { 
    // slot3
    int_var__slot3 = _val.convert_to< slot3_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_tdm_t::slot3() const {
    return int_var__slot3;
}
    
void cap_mx_csr_cfg_mac_gbl_t::ff_txdispad_i(const cpp_int & _val) { 
    // ff_txdispad_i
    int_var__ff_txdispad_i = _val.convert_to< ff_txdispad_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::ff_txdispad_i() const {
    return int_var__ff_txdispad_i;
}
    
void cap_mx_csr_cfg_mac_gbl_t::pkt_fcs_enable(const cpp_int & _val) { 
    // pkt_fcs_enable
    int_var__pkt_fcs_enable = _val.convert_to< pkt_fcs_enable_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::pkt_fcs_enable() const {
    return int_var__pkt_fcs_enable;
}
    
void cap_mx_csr_cfg_mac_gbl_t::ff_tx_ipg(const cpp_int & _val) { 
    // ff_tx_ipg
    int_var__ff_tx_ipg = _val.convert_to< ff_tx_ipg_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::ff_tx_ipg() const {
    return int_var__ff_tx_ipg;
}
    
void cap_mx_csr_cfg_mac_gbl_t::ff_txtimestampena_i(const cpp_int & _val) { 
    // ff_txtimestampena_i
    int_var__ff_txtimestampena_i = _val.convert_to< ff_txtimestampena_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::ff_txtimestampena_i() const {
    return int_var__ff_txtimestampena_i;
}
    
void cap_mx_csr_cfg_mac_gbl_t::ff_txtimestampid_i(const cpp_int & _val) { 
    // ff_txtimestampid_i
    int_var__ff_txtimestampid_i = _val.convert_to< ff_txtimestampid_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::ff_txtimestampid_i() const {
    return int_var__ff_txtimestampid_i;
}
    
void cap_mx_csr_cfg_mac_gbl_t::cg_fec_enable_i(const cpp_int & _val) { 
    // cg_fec_enable_i
    int_var__cg_fec_enable_i = _val.convert_to< cg_fec_enable_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::cg_fec_enable_i() const {
    return int_var__cg_fec_enable_i;
}
    
void cap_mx_csr_cfg_mac_gbl_t::ff_txpreamble_i(const cpp_int & _val) { 
    // ff_txpreamble_i
    int_var__ff_txpreamble_i = _val.convert_to< ff_txpreamble_i_cpp_int_t >();
}

cpp_int cap_mx_csr_cfg_mac_gbl_t::ff_txpreamble_i() const {
    return int_var__ff_txpreamble_i;
}
    
void cap_mx_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_mx_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable_enable")) { field_val = uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_enable")) { field_val = correctable_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_int_test_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable_interrupt")) { field_val = uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_interrupt")) { field_val = correctable_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_int_mac_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane0_sbe_enable")) { field_val = lane0_sbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane0_dbe_enable")) { field_val = lane0_dbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_sbe_enable")) { field_val = lane1_sbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_dbe_enable")) { field_val = lane1_dbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_sbe_enable")) { field_val = lane2_sbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_dbe_enable")) { field_val = lane2_dbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_sbe_enable")) { field_val = lane3_sbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_dbe_enable")) { field_val = lane3_dbe_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_intr_enable")) { field_val = umac_CFG3_intr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_pslverr_enable")) { field_val = umac_CFG3_pslverr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m0pb_pbus_drdy_enable")) { field_val = m0pb_pbus_drdy_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m1pb_pbus_drdy_enable")) { field_val = m1pb_pbus_drdy_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m2pb_pbus_drdy_enable")) { field_val = m2pb_pbus_drdy_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m3pb_pbus_drdy_enable")) { field_val = m3pb_pbus_drdy_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_sof_enable")) { field_val = rx0_missing_sof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_eof_enable")) { field_val = rx0_missing_eof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_timeout_err_enable")) { field_val = rx0_timeout_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_min_size_err_enable")) { field_val = rx0_min_size_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_sof_enable")) { field_val = rx1_missing_sof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_eof_enable")) { field_val = rx1_missing_eof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_timeout_err_enable")) { field_val = rx1_timeout_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_min_size_err_enable")) { field_val = rx1_min_size_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_sof_enable")) { field_val = rx2_missing_sof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_eof_enable")) { field_val = rx2_missing_eof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_timeout_err_enable")) { field_val = rx2_timeout_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_min_size_err_enable")) { field_val = rx2_min_size_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_sof_enable")) { field_val = rx3_missing_sof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_eof_enable")) { field_val = rx3_missing_eof_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_timeout_err_enable")) { field_val = rx3_timeout_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_min_size_err_enable")) { field_val = rx3_min_size_err_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_mac_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane0_sbe_interrupt")) { field_val = lane0_sbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane0_dbe_interrupt")) { field_val = lane0_dbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_sbe_interrupt")) { field_val = lane1_sbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_dbe_interrupt")) { field_val = lane1_dbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_sbe_interrupt")) { field_val = lane2_sbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_dbe_interrupt")) { field_val = lane2_dbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_sbe_interrupt")) { field_val = lane3_sbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_dbe_interrupt")) { field_val = lane3_dbe_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_intr_interrupt")) { field_val = umac_CFG3_intr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_pslverr_interrupt")) { field_val = umac_CFG3_pslverr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m0pb_pbus_drdy_interrupt")) { field_val = m0pb_pbus_drdy_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m1pb_pbus_drdy_interrupt")) { field_val = m1pb_pbus_drdy_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m2pb_pbus_drdy_interrupt")) { field_val = m2pb_pbus_drdy_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "m3pb_pbus_drdy_interrupt")) { field_val = m3pb_pbus_drdy_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_sof_interrupt")) { field_val = rx0_missing_sof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_eof_interrupt")) { field_val = rx0_missing_eof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_timeout_err_interrupt")) { field_val = rx0_timeout_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_min_size_err_interrupt")) { field_val = rx0_min_size_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_sof_interrupt")) { field_val = rx1_missing_sof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_eof_interrupt")) { field_val = rx1_missing_eof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_timeout_err_interrupt")) { field_val = rx1_timeout_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_min_size_err_interrupt")) { field_val = rx1_min_size_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_sof_interrupt")) { field_val = rx2_missing_sof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_eof_interrupt")) { field_val = rx2_missing_eof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_timeout_err_interrupt")) { field_val = rx2_timeout_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_min_size_err_interrupt")) { field_val = rx2_min_size_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_sof_interrupt")) { field_val = rx3_missing_sof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_eof_interrupt")) { field_val = rx3_missing_eof_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_timeout_err_interrupt")) { field_val = rx3_timeout_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_min_size_err_interrupt")) { field_val = rx3_min_size_err_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_mac_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_interrupt")) { field_val = int_mac_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_ecc_interrupt")) { field_val = int_ecc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_enable")) { field_val = int_mac_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_ecc_enable")) { field_val = int_ecc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_dhs_mac_stats_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_mac_stats_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_apb_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_apb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_sta_fec_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_fec_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_stats_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_stats_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_rxfifo_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_rxfifo_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_txfifo_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_txfifo_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_debug_port_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { field_val = select(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_fixer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { field_val = timeout(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_mac_sd_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_sta_mac_sd_interrupt_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_progress")) { field_val = in_progress(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_out")) { field_val = data_out(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_core_to_cntl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_interrupt_request_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_interrupt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "code")) { field_val = code(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_xoff_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_tx0fcxoff_i")) { field_val = ff_tx0fcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0pfcxoff_i")) { field_val = ff_tx0pfcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1fcxoff_i")) { field_val = ff_tx1fcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1pfcxoff_i")) { field_val = ff_tx1pfcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2fcxoff_i")) { field_val = ff_tx2fcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2pfcxoff_i")) { field_val = ff_tx2pfcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3fcxoff_i")) { field_val = ff_tx3fcxoff_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3pfcxoff_i")) { field_val = ff_tx3pfcxoff_i(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_mac_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol0_o")) { field_val = serdes_rxcorecontrol0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol0_o")) { field_val = serdes_txcorecontrol0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull0_o")) { field_val = ff_txafull0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes0sigok_o")) { field_val = ff_serdes0sigok_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx0sync_o")) { field_val = ff_rx0sync_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0idle_o")) { field_val = ff_tx0idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx0idle_o")) { field_val = ff_rx0idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0good_o")) { field_val = ff_tx0good_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_0_o")) { field_val = serdes_txquiet_ts_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_0_o")) { field_val = serdes_rxquiet_rs_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0read_o")) { field_val = ff_tx0read_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_0_o")) { field_val = ff_txlpi_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_0_o")) { field_val = ff_rxlpi_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_0_o")) { field_val = serdes_rxlpiactive_rs_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_0_o")) { field_val = serdes_txalert_ts_0_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol1_o")) { field_val = serdes_rxcorecontrol1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol1_o")) { field_val = serdes_txcorecontrol1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull1_o")) { field_val = ff_txafull1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes1sigok_o")) { field_val = ff_serdes1sigok_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx1sync_o")) { field_val = ff_rx1sync_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1idle_o")) { field_val = ff_tx1idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx1idle_o")) { field_val = ff_rx1idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1good_o")) { field_val = ff_tx1good_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_1_o")) { field_val = serdes_txquiet_ts_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_1_o")) { field_val = serdes_rxquiet_rs_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1read_o")) { field_val = ff_tx1read_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_1_o")) { field_val = ff_txlpi_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_1_o")) { field_val = ff_rxlpi_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_1_o")) { field_val = serdes_rxlpiactive_rs_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_1_o")) { field_val = serdes_txalert_ts_1_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol2_o")) { field_val = serdes_rxcorecontrol2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol2_o")) { field_val = serdes_txcorecontrol2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull2_o")) { field_val = ff_txafull2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes2sigok_o")) { field_val = ff_serdes2sigok_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx2sync_o")) { field_val = ff_rx2sync_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2idle_o")) { field_val = ff_tx2idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx2idle_o")) { field_val = ff_rx2idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2good_o")) { field_val = ff_tx2good_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_2_o")) { field_val = serdes_txquiet_ts_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_2_o")) { field_val = serdes_rxquiet_rs_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2read_o")) { field_val = ff_tx2read_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_2_o")) { field_val = ff_txlpi_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_2_o")) { field_val = ff_rxlpi_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_2_o")) { field_val = serdes_rxlpiactive_rs_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_2_o")) { field_val = serdes_txalert_ts_2_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol3_o")) { field_val = serdes_rxcorecontrol3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol3_o")) { field_val = serdes_txcorecontrol3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull3_o")) { field_val = ff_txafull3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes3sigok_o")) { field_val = ff_serdes3sigok_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx3sync_o")) { field_val = ff_rx3sync_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3idle_o")) { field_val = ff_tx3idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx3idle_o")) { field_val = ff_rx3idle_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3good_o")) { field_val = ff_tx3good_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_3_o")) { field_val = serdes_txquiet_ts_3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_3_o")) { field_val = serdes_rxquiet_rs_3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3read_o")) { field_val = ff_tx3read_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_3_o")) { field_val = ff_txlpi_3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_3_o")) { field_val = ff_rxlpi_3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_3_o")) { field_val = serdes_rxlpiactive_rs_3_o(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_3_o")) { field_val = serdes_txalert_ts_3_o(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mac_rxsigok_i_sel")) { field_val = mac_rxsigok_i_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_rxenergydetect_rs_i_sel")) { field_val = mac_rxenergydetect_rs_i_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_tx_corestatus_i_sel")) { field_val = mac_tx_corestatus_i_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_rx_corestatus_i_sel")) { field_val = mac_rx_corestatus_i_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_7_5_sel")) { field_val = core_to_cntl_7_5_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_9_8_sel")) { field_val = core_to_cntl_9_8_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_15_10_sel")) { field_val = core_to_cntl_15_10_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes0_rxsigok_i")) { field_val = serdes0_rxsigok_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes0_tx_override_in")) { field_val = serdes0_tx_override_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_0_i")) { field_val = serdes_rxenergydetect_rs_0_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_0_i")) { field_val = serdes_txcorestatus_0_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_0_i")) { field_val = serdes_rxcorestatus_0_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes1_rxsigok_i")) { field_val = serdes1_rxsigok_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes1_tx_override_in")) { field_val = serdes1_tx_override_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_1_i")) { field_val = serdes_rxenergydetect_rs_1_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_1_i")) { field_val = serdes_txcorestatus_1_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_1_i")) { field_val = serdes_rxcorestatus_1_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes2_rxsigok_i")) { field_val = serdes2_rxsigok_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes2_tx_override_in")) { field_val = serdes2_tx_override_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_2_i")) { field_val = serdes_rxenergydetect_rs_2_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_2_i")) { field_val = serdes_txcorestatus_2_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_2_i")) { field_val = serdes_rxcorestatus_2_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes3_rxsigok_i")) { field_val = serdes3_rxsigok_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes3_tx_override_in")) { field_val = serdes3_tx_override_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_3_i")) { field_val = serdes_rxenergydetect_rs_3_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_3_i")) { field_val = serdes_txcorestatus_3_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_3_i")) { field_val = serdes_rxcorestatus_3_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_0_i")) { field_val = ff_txlpi_0_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_1_i")) { field_val = ff_txlpi_1_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_2_i")) { field_val = ff_txlpi_2_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_3_i")) { field_val = ff_txlpi_3_i(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_tdm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slot0")) { field_val = slot0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot1")) { field_val = slot1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot2")) { field_val = slot2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot3")) { field_val = slot3(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_gbl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txdispad_i")) { field_val = ff_txdispad_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_fcs_enable")) { field_val = pkt_fcs_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx_ipg")) { field_val = ff_tx_ipg(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txtimestampena_i")) { field_val = ff_txtimestampena_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txtimestampid_i")) { field_val = ff_txtimestampid_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cg_fec_enable_i")) { field_val = cg_fec_enable_i(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpreamble_i")) { field_val = ff_txpreamble_i(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_gbl.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_tdm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_xoff.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fixer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_txfifo_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_txfifo_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_rxfifo_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_rxfifo_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_stats_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stats_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fec_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_fec_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ecc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_mac_stats.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mac.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_ecc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable_enable")) { uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_enable")) { correctable_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_int_test_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable_interrupt")) { uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_interrupt")) { correctable_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_int_mac_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane0_sbe_enable")) { lane0_sbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane0_dbe_enable")) { lane0_dbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_sbe_enable")) { lane1_sbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_dbe_enable")) { lane1_dbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_sbe_enable")) { lane2_sbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_dbe_enable")) { lane2_dbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_sbe_enable")) { lane3_sbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_dbe_enable")) { lane3_dbe_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_intr_enable")) { umac_CFG3_intr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_pslverr_enable")) { umac_CFG3_pslverr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m0pb_pbus_drdy_enable")) { m0pb_pbus_drdy_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m1pb_pbus_drdy_enable")) { m1pb_pbus_drdy_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m2pb_pbus_drdy_enable")) { m2pb_pbus_drdy_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m3pb_pbus_drdy_enable")) { m3pb_pbus_drdy_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_sof_enable")) { rx0_missing_sof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_eof_enable")) { rx0_missing_eof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_timeout_err_enable")) { rx0_timeout_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_min_size_err_enable")) { rx0_min_size_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_sof_enable")) { rx1_missing_sof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_eof_enable")) { rx1_missing_eof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_timeout_err_enable")) { rx1_timeout_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_min_size_err_enable")) { rx1_min_size_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_sof_enable")) { rx2_missing_sof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_eof_enable")) { rx2_missing_eof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_timeout_err_enable")) { rx2_timeout_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_min_size_err_enable")) { rx2_min_size_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_sof_enable")) { rx3_missing_sof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_eof_enable")) { rx3_missing_eof_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_timeout_err_enable")) { rx3_timeout_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_min_size_err_enable")) { rx3_min_size_err_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_mac_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lane0_sbe_interrupt")) { lane0_sbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane0_dbe_interrupt")) { lane0_dbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_sbe_interrupt")) { lane1_sbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane1_dbe_interrupt")) { lane1_dbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_sbe_interrupt")) { lane2_sbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane2_dbe_interrupt")) { lane2_dbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_sbe_interrupt")) { lane3_sbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lane3_dbe_interrupt")) { lane3_dbe_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_intr_interrupt")) { umac_CFG3_intr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "umac_CFG3_pslverr_interrupt")) { umac_CFG3_pslverr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m0pb_pbus_drdy_interrupt")) { m0pb_pbus_drdy_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m1pb_pbus_drdy_interrupt")) { m1pb_pbus_drdy_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m2pb_pbus_drdy_interrupt")) { m2pb_pbus_drdy_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "m3pb_pbus_drdy_interrupt")) { m3pb_pbus_drdy_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_sof_interrupt")) { rx0_missing_sof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_missing_eof_interrupt")) { rx0_missing_eof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_timeout_err_interrupt")) { rx0_timeout_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx0_min_size_err_interrupt")) { rx0_min_size_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_sof_interrupt")) { rx1_missing_sof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_missing_eof_interrupt")) { rx1_missing_eof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_timeout_err_interrupt")) { rx1_timeout_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx1_min_size_err_interrupt")) { rx1_min_size_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_sof_interrupt")) { rx2_missing_sof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_missing_eof_interrupt")) { rx2_missing_eof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_timeout_err_interrupt")) { rx2_timeout_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx2_min_size_err_interrupt")) { rx2_min_size_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_sof_interrupt")) { rx3_missing_sof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_missing_eof_interrupt")) { rx3_missing_eof_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_timeout_err_interrupt")) { rx3_timeout_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx3_min_size_err_interrupt")) { rx3_min_size_err_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_mac_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_interrupt")) { int_mac_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_ecc_interrupt")) { int_ecc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mac_enable")) { int_mac_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_ecc_enable")) { int_ecc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_dhs_mac_stats_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_mac_stats_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_apb_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_dhs_apb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_sta_fec_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_fec_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_stats_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_stats_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_rxfifo_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_rxfifo_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_txfifo_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_txfifo_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_debug_port_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { select(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_fixer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { timeout(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_mac_sd_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mx_csr_sta_mac_sd_interrupt_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_progress")) { in_progress(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_out")) { data_out(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_core_to_cntl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_interrupt_request_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_sd_interrupt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "code")) { code(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_xoff_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_tx0fcxoff_i")) { ff_tx0fcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0pfcxoff_i")) { ff_tx0pfcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1fcxoff_i")) { ff_tx1fcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1pfcxoff_i")) { ff_tx1pfcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2fcxoff_i")) { ff_tx2fcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2pfcxoff_i")) { ff_tx2pfcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3fcxoff_i")) { ff_tx3fcxoff_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3pfcxoff_i")) { ff_tx3pfcxoff_i(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_sta_mac_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol0_o")) { serdes_rxcorecontrol0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol0_o")) { serdes_txcorecontrol0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull0_o")) { ff_txafull0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes0sigok_o")) { ff_serdes0sigok_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx0sync_o")) { ff_rx0sync_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0idle_o")) { ff_tx0idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx0idle_o")) { ff_rx0idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0good_o")) { ff_tx0good_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_0_o")) { serdes_txquiet_ts_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_0_o")) { serdes_rxquiet_rs_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx0read_o")) { ff_tx0read_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_0_o")) { ff_txlpi_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_0_o")) { ff_rxlpi_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_0_o")) { serdes_rxlpiactive_rs_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_0_o")) { serdes_txalert_ts_0_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol1_o")) { serdes_rxcorecontrol1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol1_o")) { serdes_txcorecontrol1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull1_o")) { ff_txafull1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes1sigok_o")) { ff_serdes1sigok_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx1sync_o")) { ff_rx1sync_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1idle_o")) { ff_tx1idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx1idle_o")) { ff_rx1idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1good_o")) { ff_tx1good_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_1_o")) { serdes_txquiet_ts_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_1_o")) { serdes_rxquiet_rs_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx1read_o")) { ff_tx1read_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_1_o")) { ff_txlpi_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_1_o")) { ff_rxlpi_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_1_o")) { serdes_rxlpiactive_rs_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_1_o")) { serdes_txalert_ts_1_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol2_o")) { serdes_rxcorecontrol2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol2_o")) { serdes_txcorecontrol2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull2_o")) { ff_txafull2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes2sigok_o")) { ff_serdes2sigok_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx2sync_o")) { ff_rx2sync_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2idle_o")) { ff_tx2idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx2idle_o")) { ff_rx2idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2good_o")) { ff_tx2good_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_2_o")) { serdes_txquiet_ts_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_2_o")) { serdes_rxquiet_rs_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx2read_o")) { ff_tx2read_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_2_o")) { ff_txlpi_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_2_o")) { ff_rxlpi_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_2_o")) { serdes_rxlpiactive_rs_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_2_o")) { serdes_txalert_ts_2_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorecontrol3_o")) { serdes_rxcorecontrol3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorecontrol3_o")) { serdes_txcorecontrol3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txafull3_o")) { ff_txafull3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_serdes3sigok_o")) { ff_serdes3sigok_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx3sync_o")) { ff_rx3sync_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3idle_o")) { ff_tx3idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rx3idle_o")) { ff_rx3idle_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3good_o")) { ff_tx3good_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txquiet_ts_3_o")) { serdes_txquiet_ts_3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxquiet_rs_3_o")) { serdes_rxquiet_rs_3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx3read_o")) { ff_tx3read_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_3_o")) { ff_txlpi_3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_rxlpi_3_o")) { ff_rxlpi_3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxlpiactive_rs_3_o")) { serdes_rxlpiactive_rs_3_o(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txalert_ts_3_o")) { serdes_txalert_ts_3_o(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mac_rxsigok_i_sel")) { mac_rxsigok_i_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_rxenergydetect_rs_i_sel")) { mac_rxenergydetect_rs_i_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_tx_corestatus_i_sel")) { mac_tx_corestatus_i_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mac_rx_corestatus_i_sel")) { mac_rx_corestatus_i_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_7_5_sel")) { core_to_cntl_7_5_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_9_8_sel")) { core_to_cntl_9_8_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "core_to_cntl_15_10_sel")) { core_to_cntl_15_10_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes0_rxsigok_i")) { serdes0_rxsigok_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes0_tx_override_in")) { serdes0_tx_override_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_0_i")) { serdes_rxenergydetect_rs_0_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_0_i")) { serdes_txcorestatus_0_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_0_i")) { serdes_rxcorestatus_0_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes1_rxsigok_i")) { serdes1_rxsigok_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes1_tx_override_in")) { serdes1_tx_override_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_1_i")) { serdes_rxenergydetect_rs_1_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_1_i")) { serdes_txcorestatus_1_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_1_i")) { serdes_rxcorestatus_1_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes2_rxsigok_i")) { serdes2_rxsigok_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes2_tx_override_in")) { serdes2_tx_override_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_2_i")) { serdes_rxenergydetect_rs_2_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_2_i")) { serdes_txcorestatus_2_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_2_i")) { serdes_rxcorestatus_2_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes3_rxsigok_i")) { serdes3_rxsigok_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes3_tx_override_in")) { serdes3_tx_override_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxenergydetect_rs_3_i")) { serdes_rxenergydetect_rs_3_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_txcorestatus_3_i")) { serdes_txcorestatus_3_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "serdes_rxcorestatus_3_i")) { serdes_rxcorestatus_3_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_0_i")) { ff_txlpi_0_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_1_i")) { ff_txlpi_1_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_2_i")) { ff_txlpi_2_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txlpi_3_i")) { ff_txlpi_3_i(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_tdm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slot0")) { slot0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot1")) { slot1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot2")) { slot2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot3")) { slot3(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_cfg_mac_gbl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ff_txdispad_i")) { ff_txdispad_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_fcs_enable")) { pkt_fcs_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_tx_ipg")) { ff_tx_ipg(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txtimestampena_i")) { ff_txtimestampena_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txtimestampid_i")) { ff_txtimestampid_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cg_fec_enable_i")) { cg_fec_enable_i(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ff_txpreamble_i")) { ff_txpreamble_i(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mx_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_gbl.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_tdm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mac.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mac_xoff.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fixer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_txfifo_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_txfifo_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_rxfifo_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_rxfifo_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_stats_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stats_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fec_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_fec_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ecc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_mac_stats.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mac.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_ecc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_ecc_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable_enable");
    ret_vec.push_back("correctable_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_ecc_int_test_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable_interrupt");
    ret_vec.push_back("correctable_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_ecc_t::get_fields(int level) const { 
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
std::vector<string> cap_mx_csr_int_mac_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lane0_sbe_enable");
    ret_vec.push_back("lane0_dbe_enable");
    ret_vec.push_back("lane1_sbe_enable");
    ret_vec.push_back("lane1_dbe_enable");
    ret_vec.push_back("lane2_sbe_enable");
    ret_vec.push_back("lane2_dbe_enable");
    ret_vec.push_back("lane3_sbe_enable");
    ret_vec.push_back("lane3_dbe_enable");
    ret_vec.push_back("umac_CFG3_intr_enable");
    ret_vec.push_back("umac_CFG3_pslverr_enable");
    ret_vec.push_back("m0pb_pbus_drdy_enable");
    ret_vec.push_back("m1pb_pbus_drdy_enable");
    ret_vec.push_back("m2pb_pbus_drdy_enable");
    ret_vec.push_back("m3pb_pbus_drdy_enable");
    ret_vec.push_back("rx0_missing_sof_enable");
    ret_vec.push_back("rx0_missing_eof_enable");
    ret_vec.push_back("rx0_timeout_err_enable");
    ret_vec.push_back("rx0_min_size_err_enable");
    ret_vec.push_back("rx1_missing_sof_enable");
    ret_vec.push_back("rx1_missing_eof_enable");
    ret_vec.push_back("rx1_timeout_err_enable");
    ret_vec.push_back("rx1_min_size_err_enable");
    ret_vec.push_back("rx2_missing_sof_enable");
    ret_vec.push_back("rx2_missing_eof_enable");
    ret_vec.push_back("rx2_timeout_err_enable");
    ret_vec.push_back("rx2_min_size_err_enable");
    ret_vec.push_back("rx3_missing_sof_enable");
    ret_vec.push_back("rx3_missing_eof_enable");
    ret_vec.push_back("rx3_timeout_err_enable");
    ret_vec.push_back("rx3_min_size_err_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_mac_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lane0_sbe_interrupt");
    ret_vec.push_back("lane0_dbe_interrupt");
    ret_vec.push_back("lane1_sbe_interrupt");
    ret_vec.push_back("lane1_dbe_interrupt");
    ret_vec.push_back("lane2_sbe_interrupt");
    ret_vec.push_back("lane2_dbe_interrupt");
    ret_vec.push_back("lane3_sbe_interrupt");
    ret_vec.push_back("lane3_dbe_interrupt");
    ret_vec.push_back("umac_CFG3_intr_interrupt");
    ret_vec.push_back("umac_CFG3_pslverr_interrupt");
    ret_vec.push_back("m0pb_pbus_drdy_interrupt");
    ret_vec.push_back("m1pb_pbus_drdy_interrupt");
    ret_vec.push_back("m2pb_pbus_drdy_interrupt");
    ret_vec.push_back("m3pb_pbus_drdy_interrupt");
    ret_vec.push_back("rx0_missing_sof_interrupt");
    ret_vec.push_back("rx0_missing_eof_interrupt");
    ret_vec.push_back("rx0_timeout_err_interrupt");
    ret_vec.push_back("rx0_min_size_err_interrupt");
    ret_vec.push_back("rx1_missing_sof_interrupt");
    ret_vec.push_back("rx1_missing_eof_interrupt");
    ret_vec.push_back("rx1_timeout_err_interrupt");
    ret_vec.push_back("rx1_min_size_err_interrupt");
    ret_vec.push_back("rx2_missing_sof_interrupt");
    ret_vec.push_back("rx2_missing_eof_interrupt");
    ret_vec.push_back("rx2_timeout_err_interrupt");
    ret_vec.push_back("rx2_min_size_err_interrupt");
    ret_vec.push_back("rx3_missing_sof_interrupt");
    ret_vec.push_back("rx3_missing_eof_interrupt");
    ret_vec.push_back("rx3_timeout_err_interrupt");
    ret_vec.push_back("rx3_min_size_err_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_mac_t::get_fields(int level) const { 
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
std::vector<string> cap_mx_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mac_interrupt");
    ret_vec.push_back("int_ecc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mac_enable");
    ret_vec.push_back("int_ecc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_mx_csr_dhs_mac_stats_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_dhs_mac_stats_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_dhs_apb_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_dhs_apb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_ecc_t::get_fields(int level) const { 
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
std::vector<string> cap_mx_csr_sta_fec_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_fec_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("ecc_disable_cor");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_stats_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_stats_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("ecc_disable_cor");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_rxfifo_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_rxfifo_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_txfifo_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_txfifo_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_debug_port_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("select");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_fixer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("timeout");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_mac_sd_status_t::get_fields(int level) const { 
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
std::vector<string> cap_mx_csr_sta_mac_sd_interrupt_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("in_progress");
    ret_vec.push_back("data_out");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_sd_core_to_cntl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_sd_interrupt_request_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_sd_interrupt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("code");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_xoff_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ff_tx0fcxoff_i");
    ret_vec.push_back("ff_tx0pfcxoff_i");
    ret_vec.push_back("ff_tx1fcxoff_i");
    ret_vec.push_back("ff_tx1pfcxoff_i");
    ret_vec.push_back("ff_tx2fcxoff_i");
    ret_vec.push_back("ff_tx2pfcxoff_i");
    ret_vec.push_back("ff_tx3fcxoff_i");
    ret_vec.push_back("ff_tx3pfcxoff_i");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_sta_mac_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("serdes_rxcorecontrol0_o");
    ret_vec.push_back("serdes_txcorecontrol0_o");
    ret_vec.push_back("ff_txafull0_o");
    ret_vec.push_back("ff_serdes0sigok_o");
    ret_vec.push_back("ff_rx0sync_o");
    ret_vec.push_back("ff_tx0idle_o");
    ret_vec.push_back("ff_rx0idle_o");
    ret_vec.push_back("ff_tx0good_o");
    ret_vec.push_back("serdes_txquiet_ts_0_o");
    ret_vec.push_back("serdes_rxquiet_rs_0_o");
    ret_vec.push_back("ff_tx0read_o");
    ret_vec.push_back("ff_txlpi_0_o");
    ret_vec.push_back("ff_rxlpi_0_o");
    ret_vec.push_back("serdes_rxlpiactive_rs_0_o");
    ret_vec.push_back("serdes_txalert_ts_0_o");
    ret_vec.push_back("serdes_rxcorecontrol1_o");
    ret_vec.push_back("serdes_txcorecontrol1_o");
    ret_vec.push_back("ff_txafull1_o");
    ret_vec.push_back("ff_serdes1sigok_o");
    ret_vec.push_back("ff_rx1sync_o");
    ret_vec.push_back("ff_tx1idle_o");
    ret_vec.push_back("ff_rx1idle_o");
    ret_vec.push_back("ff_tx1good_o");
    ret_vec.push_back("serdes_txquiet_ts_1_o");
    ret_vec.push_back("serdes_rxquiet_rs_1_o");
    ret_vec.push_back("ff_tx1read_o");
    ret_vec.push_back("ff_txlpi_1_o");
    ret_vec.push_back("ff_rxlpi_1_o");
    ret_vec.push_back("serdes_rxlpiactive_rs_1_o");
    ret_vec.push_back("serdes_txalert_ts_1_o");
    ret_vec.push_back("serdes_rxcorecontrol2_o");
    ret_vec.push_back("serdes_txcorecontrol2_o");
    ret_vec.push_back("ff_txafull2_o");
    ret_vec.push_back("ff_serdes2sigok_o");
    ret_vec.push_back("ff_rx2sync_o");
    ret_vec.push_back("ff_tx2idle_o");
    ret_vec.push_back("ff_rx2idle_o");
    ret_vec.push_back("ff_tx2good_o");
    ret_vec.push_back("serdes_txquiet_ts_2_o");
    ret_vec.push_back("serdes_rxquiet_rs_2_o");
    ret_vec.push_back("ff_tx2read_o");
    ret_vec.push_back("ff_txlpi_2_o");
    ret_vec.push_back("ff_rxlpi_2_o");
    ret_vec.push_back("serdes_rxlpiactive_rs_2_o");
    ret_vec.push_back("serdes_txalert_ts_2_o");
    ret_vec.push_back("serdes_rxcorecontrol3_o");
    ret_vec.push_back("serdes_txcorecontrol3_o");
    ret_vec.push_back("ff_txafull3_o");
    ret_vec.push_back("ff_serdes3sigok_o");
    ret_vec.push_back("ff_rx3sync_o");
    ret_vec.push_back("ff_tx3idle_o");
    ret_vec.push_back("ff_rx3idle_o");
    ret_vec.push_back("ff_tx3good_o");
    ret_vec.push_back("serdes_txquiet_ts_3_o");
    ret_vec.push_back("serdes_rxquiet_rs_3_o");
    ret_vec.push_back("ff_tx3read_o");
    ret_vec.push_back("ff_txlpi_3_o");
    ret_vec.push_back("ff_rxlpi_3_o");
    ret_vec.push_back("serdes_rxlpiactive_rs_3_o");
    ret_vec.push_back("serdes_txalert_ts_3_o");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mac_rxsigok_i_sel");
    ret_vec.push_back("mac_rxenergydetect_rs_i_sel");
    ret_vec.push_back("mac_tx_corestatus_i_sel");
    ret_vec.push_back("mac_rx_corestatus_i_sel");
    ret_vec.push_back("core_to_cntl_7_5_sel");
    ret_vec.push_back("core_to_cntl_9_8_sel");
    ret_vec.push_back("core_to_cntl_15_10_sel");
    ret_vec.push_back("serdes0_rxsigok_i");
    ret_vec.push_back("serdes0_tx_override_in");
    ret_vec.push_back("serdes_rxenergydetect_rs_0_i");
    ret_vec.push_back("serdes_txcorestatus_0_i");
    ret_vec.push_back("serdes_rxcorestatus_0_i");
    ret_vec.push_back("serdes1_rxsigok_i");
    ret_vec.push_back("serdes1_tx_override_in");
    ret_vec.push_back("serdes_rxenergydetect_rs_1_i");
    ret_vec.push_back("serdes_txcorestatus_1_i");
    ret_vec.push_back("serdes_rxcorestatus_1_i");
    ret_vec.push_back("serdes2_rxsigok_i");
    ret_vec.push_back("serdes2_tx_override_in");
    ret_vec.push_back("serdes_rxenergydetect_rs_2_i");
    ret_vec.push_back("serdes_txcorestatus_2_i");
    ret_vec.push_back("serdes_rxcorestatus_2_i");
    ret_vec.push_back("serdes3_rxsigok_i");
    ret_vec.push_back("serdes3_tx_override_in");
    ret_vec.push_back("serdes_rxenergydetect_rs_3_i");
    ret_vec.push_back("serdes_txcorestatus_3_i");
    ret_vec.push_back("serdes_rxcorestatus_3_i");
    ret_vec.push_back("ff_txlpi_0_i");
    ret_vec.push_back("ff_txlpi_1_i");
    ret_vec.push_back("ff_txlpi_2_i");
    ret_vec.push_back("ff_txlpi_3_i");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_tdm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("slot0");
    ret_vec.push_back("slot1");
    ret_vec.push_back("slot2");
    ret_vec.push_back("slot3");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_cfg_mac_gbl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ff_txdispad_i");
    ret_vec.push_back("pkt_fcs_enable");
    ret_vec.push_back("ff_tx_ipg");
    ret_vec.push_back("ff_txtimestampena_i");
    ret_vec.push_back("ff_txtimestampid_i");
    ret_vec.push_back("cg_fec_enable_i");
    ret_vec.push_back("ff_txpreamble_i");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mx_csr_t::get_fields(int level) const { 
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
        for(auto tmp_vec : cfg_mac_tdm.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac_tdm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mac.get_fields(level-1)) {
            ret_vec.push_back("cfg_mac." + tmp_vec);
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
        for(auto tmp_vec : cfg_txfifo_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_txfifo_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_txfifo_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_txfifo_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_rxfifo_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_rxfifo_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_rxfifo_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_rxfifo_mem." + tmp_vec);
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
        for(auto tmp_vec : cfg_fec_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_fec_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_fec_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_fec_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ecc.get_fields(level-1)) {
            ret_vec.push_back("sta_ecc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_apb.get_fields(level-1)) {
            ret_vec.push_back("dhs_apb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_mac_stats.get_fields(level-1)) {
            ret_vec.push_back("dhs_mac_stats." + tmp_vec);
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
    {
        for(auto tmp_vec : int_ecc.get_fields(level-1)) {
            ret_vec.push_back("int_ecc." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
