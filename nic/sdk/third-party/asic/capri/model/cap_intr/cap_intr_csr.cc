
#include "cap_intr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_intr_csr_int_intr_ecc_int_enable_clear_t::cap_intr_csr_int_intr_ecc_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_int_intr_ecc_int_enable_clear_t::~cap_intr_csr_int_intr_ecc_int_enable_clear_t() { }

cap_intr_csr_intreg_t::cap_intr_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_intreg_t::~cap_intr_csr_intreg_t() { }

cap_intr_csr_intgrp_t::cap_intr_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_intgrp_t::~cap_intr_csr_intgrp_t() { }

cap_intr_csr_intreg_status_t::cap_intr_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_intreg_status_t::~cap_intr_csr_intreg_status_t() { }

cap_intr_csr_int_groups_int_enable_rw_reg_t::cap_intr_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_int_groups_int_enable_rw_reg_t::~cap_intr_csr_int_groups_int_enable_rw_reg_t() { }

cap_intr_csr_intgrp_status_t::cap_intr_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_intgrp_status_t::~cap_intr_csr_intgrp_status_t() { }

cap_intr_csr_dhs_intr_state_entry_t::cap_intr_csr_dhs_intr_state_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_state_entry_t::~cap_intr_csr_dhs_intr_state_entry_t() { }

cap_intr_csr_dhs_intr_state_t::cap_intr_csr_dhs_intr_state_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_state_t::~cap_intr_csr_dhs_intr_state_t() { }

cap_intr_csr_dhs_intr_coalesce_entry_t::cap_intr_csr_dhs_intr_coalesce_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_coalesce_entry_t::~cap_intr_csr_dhs_intr_coalesce_entry_t() { }

cap_intr_csr_dhs_intr_coalesce_t::cap_intr_csr_dhs_intr_coalesce_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_coalesce_t::~cap_intr_csr_dhs_intr_coalesce_t() { }

cap_intr_csr_dhs_intr_assert_entry_t::cap_intr_csr_dhs_intr_assert_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_assert_entry_t::~cap_intr_csr_dhs_intr_assert_entry_t() { }

cap_intr_csr_dhs_intr_assert_t::cap_intr_csr_dhs_intr_assert_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_assert_t::~cap_intr_csr_dhs_intr_assert_t() { }

cap_intr_csr_dhs_intr_pba_array_entry_t::cap_intr_csr_dhs_intr_pba_array_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_pba_array_entry_t::~cap_intr_csr_dhs_intr_pba_array_entry_t() { }

cap_intr_csr_dhs_intr_pba_array_t::cap_intr_csr_dhs_intr_pba_array_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_pba_array_t::~cap_intr_csr_dhs_intr_pba_array_t() { }

cap_intr_csr_dhs_intr_pba_cfg_entry_t::cap_intr_csr_dhs_intr_pba_cfg_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_pba_cfg_entry_t::~cap_intr_csr_dhs_intr_pba_cfg_entry_t() { }

cap_intr_csr_dhs_intr_pba_cfg_t::cap_intr_csr_dhs_intr_pba_cfg_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_pba_cfg_t::~cap_intr_csr_dhs_intr_pba_cfg_t() { }

cap_intr_csr_dhs_intr_drvcfg_entry_t::cap_intr_csr_dhs_intr_drvcfg_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_drvcfg_entry_t::~cap_intr_csr_dhs_intr_drvcfg_entry_t() { }

cap_intr_csr_dhs_intr_drvcfg_t::cap_intr_csr_dhs_intr_drvcfg_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_drvcfg_t::~cap_intr_csr_dhs_intr_drvcfg_t() { }

cap_intr_csr_dhs_intr_fwcfg_entry_t::cap_intr_csr_dhs_intr_fwcfg_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_fwcfg_entry_t::~cap_intr_csr_dhs_intr_fwcfg_entry_t() { }

cap_intr_csr_dhs_intr_fwcfg_t::cap_intr_csr_dhs_intr_fwcfg_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_fwcfg_t::~cap_intr_csr_dhs_intr_fwcfg_t() { }

cap_intr_csr_dhs_intr_msixcfg_entry_t::cap_intr_csr_dhs_intr_msixcfg_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_msixcfg_entry_t::~cap_intr_csr_dhs_intr_msixcfg_entry_t() { }

cap_intr_csr_dhs_intr_msixcfg_t::cap_intr_csr_dhs_intr_msixcfg_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_intr_csr_dhs_intr_msixcfg_t::~cap_intr_csr_dhs_intr_msixcfg_t() { }

cap_intr_csr_csr_intr_t::cap_intr_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_csr_intr_t::~cap_intr_csr_csr_intr_t() { }

cap_intr_csr_sta_sram_bist_t::cap_intr_csr_sta_sram_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_sram_bist_t::~cap_intr_csr_sta_sram_bist_t() { }

cap_intr_csr_cfg_sram_bist_t::cap_intr_csr_cfg_sram_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_sram_bist_t::~cap_intr_csr_cfg_sram_bist_t() { }

cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::~cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t() { }

cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::~cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t() { }

cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::~cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t() { }

cap_intr_csr_cfg_debug_max_credits_limit_t::cap_intr_csr_cfg_debug_max_credits_limit_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_debug_max_credits_limit_t::~cap_intr_csr_cfg_debug_max_credits_limit_t() { }

cap_intr_csr_cfg_debug_port_t::cap_intr_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_debug_port_t::~cap_intr_csr_cfg_debug_port_t() { }

cap_intr_csr_cfg_intr_axi_attr_t::cap_intr_csr_cfg_intr_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_intr_axi_attr_t::~cap_intr_csr_cfg_intr_axi_attr_t() { }

cap_intr_csr_sat_intr_event_counters_t::cap_intr_csr_sat_intr_event_counters_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sat_intr_event_counters_t::~cap_intr_csr_sat_intr_event_counters_t() { }

cap_intr_csr_cnt_intr_legacy_send_t::cap_intr_csr_cnt_intr_legacy_send_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cnt_intr_legacy_send_t::~cap_intr_csr_cnt_intr_legacy_send_t() { }

cap_intr_csr_cnt_intr_tot_axi_wr_t::cap_intr_csr_cnt_intr_tot_axi_wr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cnt_intr_tot_axi_wr_t::~cap_intr_csr_cnt_intr_tot_axi_wr_t() { }

cap_intr_csr_sta_intr_ecc_intr_coalesce_t::cap_intr_csr_sta_intr_ecc_intr_coalesce_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_ecc_intr_coalesce_t::~cap_intr_csr_sta_intr_ecc_intr_coalesce_t() { }

cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::~cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t() { }

cap_intr_csr_sta_intr_ecc_intr_state_t::cap_intr_csr_sta_intr_ecc_intr_state_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_ecc_intr_state_t::~cap_intr_csr_sta_intr_ecc_intr_state_t() { }

cap_intr_csr_cfg_intr_ecc_disable_t::cap_intr_csr_cfg_intr_ecc_disable_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_intr_ecc_disable_t::~cap_intr_csr_cfg_intr_ecc_disable_t() { }

cap_intr_csr_sta_intr_debug_t::cap_intr_csr_sta_intr_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_debug_t::~cap_intr_csr_sta_intr_debug_t() { }

cap_intr_csr_sta_intr_init_t::cap_intr_csr_sta_intr_init_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_init_t::~cap_intr_csr_sta_intr_init_t() { }

cap_intr_csr_sta_intr_axi_t::cap_intr_csr_sta_intr_axi_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_sta_intr_axi_t::~cap_intr_csr_sta_intr_axi_t() { }

cap_intr_csr_cfg_intr_axi_t::cap_intr_csr_cfg_intr_axi_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_intr_axi_t::~cap_intr_csr_cfg_intr_axi_t() { }

cap_intr_csr_cfg_intr_coalesce_t::cap_intr_csr_cfg_intr_coalesce_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_cfg_intr_coalesce_t::~cap_intr_csr_cfg_intr_coalesce_t() { }

cap_intr_csr_rdintr_t::cap_intr_csr_rdintr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_rdintr_t::~cap_intr_csr_rdintr_t() { }

cap_intr_csr_base_t::cap_intr_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_intr_csr_base_t::~cap_intr_csr_base_t() { }

cap_intr_csr_t::cap_intr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_intr_csr_t::~cap_intr_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_uncorrectable_enable: 0x" << int_var__intr_state_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_correctable_enable: 0x" << int_var__intr_state_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_uncorrectable_enable: 0x" << int_var__intr_pba_cfg_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_correctable_enable: 0x" << int_var__intr_pba_cfg_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_uncorrectable_enable: 0x" << int_var__intr_coalesce_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_correctable_enable: 0x" << int_var__intr_coalesce_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_legacy_fifo_overrun_enable: 0x" << int_var__intr_legacy_fifo_overrun_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_credit_positive_after_return_enable: 0x" << int_var__int_credit_positive_after_return_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_credits_max_debug_threshold_enable: 0x" << int_var__int_credits_max_debug_threshold_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_bresp_err_enable: 0x" << int_var__intr_bresp_err_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_uncorrectable_interrupt: 0x" << int_var__intr_state_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_correctable_interrupt: 0x" << int_var__intr_state_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_uncorrectable_interrupt: 0x" << int_var__intr_pba_cfg_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_correctable_interrupt: 0x" << int_var__intr_pba_cfg_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_uncorrectable_interrupt: 0x" << int_var__intr_coalesce_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_correctable_interrupt: 0x" << int_var__intr_coalesce_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_legacy_fifo_overrun_interrupt: 0x" << int_var__intr_legacy_fifo_overrun_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_credit_positive_after_return_interrupt: 0x" << int_var__int_credit_positive_after_return_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_credits_max_debug_threshold_interrupt: 0x" << int_var__int_credits_max_debug_threshold_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_bresp_err_interrupt: 0x" << int_var__intr_bresp_err_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_intr_ecc_interrupt: 0x" << int_var__int_intr_ecc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_intr_ecc_enable: 0x" << int_var__int_intr_ecc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_state_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msixcfg_msg_addr_51_2: 0x" << int_var__msixcfg_msg_addr_51_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".msixcfg_msg_data: 0x" << int_var__msixcfg_msg_data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".msixcfg_vector_ctrl: 0x" << int_var__msixcfg_vector_ctrl << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_function_mask: 0x" << int_var__fwcfg_function_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_lif: 0x" << int_var__fwcfg_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_local_int: 0x" << int_var__fwcfg_local_int << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_legacy_int: 0x" << int_var__fwcfg_legacy_int << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_legacy_pin: 0x" << int_var__fwcfg_legacy_pin << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drvcfg_mask: 0x" << int_var__drvcfg_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drvcfg_int_credits: 0x" << int_var__drvcfg_int_credits << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drvcfg_mask_on_assert: 0x" << int_var__drvcfg_mask_on_assert << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fwcfg_port_id: 0x" << int_var__fwcfg_port_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_state_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_coalesce_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".coal_init_value0: 0x" << int_var__coal_init_value0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_init_value1: 0x" << int_var__coal_init_value1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_init_value2: 0x" << int_var__coal_init_value2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_init_value3: 0x" << int_var__coal_init_value3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_curr_value0: 0x" << int_var__coal_curr_value0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_curr_value1: 0x" << int_var__coal_curr_value1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_curr_value2: 0x" << int_var__coal_curr_value2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_curr_value3: 0x" << int_var__coal_curr_value3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_use_rsvd: 0x" << int_var__hw_use_rsvd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_legacy_sent_state0: 0x" << int_var__hw_legacy_sent_state0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_legacy_sent_state1: 0x" << int_var__hw_legacy_sent_state1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_legacy_sent_state2: 0x" << int_var__hw_legacy_sent_state2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_legacy_sent_state3: 0x" << int_var__hw_legacy_sent_state3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_coalesce_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_assert_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rst_coalesce: 0x" << int_var__rst_coalesce << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_assert_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_pba_array_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".vec: 0x" << int_var__vec << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_pba_array_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_pba_cfg_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".start: 0x" << int_var__start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".count: 0x" << int_var__count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_pba_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_drvcfg_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".coal_init: 0x" << int_var__coal_init << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved1: 0x" << int_var__reserved1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask: 0x" << int_var__mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved2: 0x" << int_var__reserved2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_credits: 0x" << int_var__int_credits << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".unmask: 0x" << int_var__unmask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".restart_coal: 0x" << int_var__restart_coal << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved3: 0x" << int_var__reserved3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_on_assert: 0x" << int_var__mask_on_assert << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved4: 0x" << int_var__reserved4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_curr: 0x" << int_var__coal_curr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved5: 0x" << int_var__reserved5 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_drvcfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_fwcfg_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".function_mask: 0x" << int_var__function_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved1: 0x" << int_var__reserved1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".port_id: 0x" << int_var__port_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".local_int: 0x" << int_var__local_int << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".legacy: 0x" << int_var__legacy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intpin: 0x" << int_var__intpin << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved2: 0x" << int_var__reserved2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_fwcfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_msixcfg_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msg_addr: 0x" << int_var__msg_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".msg_data: 0x" << int_var__msg_data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".vector_ctrl: 0x" << int_var__vector_ctrl << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reserved1: 0x" << int_var__reserved1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_dhs_intr_msixcfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_sram_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".done_pass: 0x" << int_var__done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_fail: 0x" << int_var__done_fail << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_sram_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".run: 0x" << int_var__run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_readout: 0x" << int_var__debug_readout << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dw0: 0x" << int_var__dw0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dw1: 0x" << int_var__dw1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dw2: 0x" << int_var__dw2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dw3: 0x" << int_var__dw3 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".source: 0x" << int_var__source << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_debug_max_credits_limit_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_threshold: 0x" << int_var__int_threshold << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_debug_port_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_intr_axi_attr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".awcache: 0x" << int_var__awcache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sat_intr_event_counters_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_bresp_errs: 0x" << int_var__axi_bresp_errs << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".delayed_coal_scan: 0x" << int_var__delayed_coal_scan << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cnt_intr_legacy_send_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".req: 0x" << int_var__req << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cnt_intr_tot_axi_wr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".req: 0x" << int_var__req << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_ecc_intr_state_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_intr_ecc_disable_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_cor: 0x" << int_var__intr_state_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_det: 0x" << int_var__intr_state_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_state_dhs: 0x" << int_var__intr_state_dhs << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_cor: 0x" << int_var__intr_pba_cfg_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_det: 0x" << int_var__intr_pba_cfg_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_pba_cfg_dhs: 0x" << int_var__intr_pba_cfg_dhs << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_cor: 0x" << int_var__intr_coalesce_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_det: 0x" << int_var__intr_coalesce_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".intr_coalesce_dhs: 0x" << int_var__intr_coalesce_dhs << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_debug_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".legacy_fifodepth: 0x" << int_var__legacy_fifodepth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coalexp_fifodepth: 0x" << int_var__coalexp_fifodepth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wfifo_depth: 0x" << int_var__wfifo_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awfifo_depth: 0x" << int_var__awfifo_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wready: 0x" << int_var__wready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wvalid: 0x" << int_var__wvalid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awready: 0x" << int_var__awready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awvalid: 0x" << int_var__awvalid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rdy: 0x" << int_var__axi_rdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".coal_scan_active: 0x" << int_var__coal_scan_active << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".init_done: 0x" << int_var__init_done << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_init_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".done: 0x" << int_var__done << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_sta_intr_axi_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".num_ids: 0x" << int_var__num_ids << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_intr_axi_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".num_ids: 0x" << int_var__num_ids << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_cfg_intr_coalesce_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".resolution: 0x" << int_var__resolution << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_rdintr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ireg: 0x" << int_var__ireg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_intr_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    rdintr.show();
    cfg_intr_coalesce.show();
    cfg_intr_axi.show();
    sta_intr_axi.show();
    sta_intr_init.show();
    sta_intr_debug.show();
    cfg_intr_ecc_disable.show();
    sta_intr_ecc_intr_state.show();
    sta_intr_ecc_intr_pba_cfg.show();
    sta_intr_ecc_intr_coalesce.show();
    cnt_intr_tot_axi_wr.show();
    cnt_intr_legacy_send.show();
    sat_intr_event_counters.show();
    cfg_intr_axi_attr.show();
    cfg_debug_port.show();
    cfg_debug_max_credits_limit.show();
    cfg_todo_bits_legacy_cnt_read_debug.show();
    cfg_legacy_intx_pcie_msg_hdr.show();
    sta_todo_bits_legacy_cnt_read_debug.show();
    cfg_sram_bist.show();
    sta_sram_bist.show();
    csr_intr.show();
    dhs_intr_msixcfg.show();
    dhs_intr_fwcfg.show();
    dhs_intr_drvcfg.show();
    dhs_intr_pba_cfg.show();
    dhs_intr_pba_array.show();
    dhs_intr_assert.show();
    dhs_intr_coalesce.show();
    dhs_intr_state.show();
    int_groups.show();
    int_intr_ecc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_intr_csr_int_intr_ecc_int_enable_clear_t::get_width() const {
    return cap_intr_csr_int_intr_ecc_int_enable_clear_t::s_get_width();

}

int cap_intr_csr_intreg_t::get_width() const {
    return cap_intr_csr_intreg_t::s_get_width();

}

int cap_intr_csr_intgrp_t::get_width() const {
    return cap_intr_csr_intgrp_t::s_get_width();

}

int cap_intr_csr_intreg_status_t::get_width() const {
    return cap_intr_csr_intreg_status_t::s_get_width();

}

int cap_intr_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_intr_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_intr_csr_intgrp_status_t::get_width() const {
    return cap_intr_csr_intgrp_status_t::s_get_width();

}

int cap_intr_csr_dhs_intr_state_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_state_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_state_t::get_width() const {
    return cap_intr_csr_dhs_intr_state_t::s_get_width();

}

int cap_intr_csr_dhs_intr_coalesce_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_coalesce_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_coalesce_t::get_width() const {
    return cap_intr_csr_dhs_intr_coalesce_t::s_get_width();

}

int cap_intr_csr_dhs_intr_assert_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_assert_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_assert_t::get_width() const {
    return cap_intr_csr_dhs_intr_assert_t::s_get_width();

}

int cap_intr_csr_dhs_intr_pba_array_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_pba_array_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_pba_array_t::get_width() const {
    return cap_intr_csr_dhs_intr_pba_array_t::s_get_width();

}

int cap_intr_csr_dhs_intr_pba_cfg_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_pba_cfg_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_pba_cfg_t::get_width() const {
    return cap_intr_csr_dhs_intr_pba_cfg_t::s_get_width();

}

int cap_intr_csr_dhs_intr_drvcfg_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_drvcfg_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_drvcfg_t::get_width() const {
    return cap_intr_csr_dhs_intr_drvcfg_t::s_get_width();

}

int cap_intr_csr_dhs_intr_fwcfg_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_fwcfg_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_fwcfg_t::get_width() const {
    return cap_intr_csr_dhs_intr_fwcfg_t::s_get_width();

}

int cap_intr_csr_dhs_intr_msixcfg_entry_t::get_width() const {
    return cap_intr_csr_dhs_intr_msixcfg_entry_t::s_get_width();

}

int cap_intr_csr_dhs_intr_msixcfg_t::get_width() const {
    return cap_intr_csr_dhs_intr_msixcfg_t::s_get_width();

}

int cap_intr_csr_csr_intr_t::get_width() const {
    return cap_intr_csr_csr_intr_t::s_get_width();

}

int cap_intr_csr_sta_sram_bist_t::get_width() const {
    return cap_intr_csr_sta_sram_bist_t::s_get_width();

}

int cap_intr_csr_cfg_sram_bist_t::get_width() const {
    return cap_intr_csr_cfg_sram_bist_t::s_get_width();

}

int cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::get_width() const {
    return cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::s_get_width();

}

int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::get_width() const {
    return cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::s_get_width();

}

int cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::get_width() const {
    return cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::s_get_width();

}

int cap_intr_csr_cfg_debug_max_credits_limit_t::get_width() const {
    return cap_intr_csr_cfg_debug_max_credits_limit_t::s_get_width();

}

int cap_intr_csr_cfg_debug_port_t::get_width() const {
    return cap_intr_csr_cfg_debug_port_t::s_get_width();

}

int cap_intr_csr_cfg_intr_axi_attr_t::get_width() const {
    return cap_intr_csr_cfg_intr_axi_attr_t::s_get_width();

}

int cap_intr_csr_sat_intr_event_counters_t::get_width() const {
    return cap_intr_csr_sat_intr_event_counters_t::s_get_width();

}

int cap_intr_csr_cnt_intr_legacy_send_t::get_width() const {
    return cap_intr_csr_cnt_intr_legacy_send_t::s_get_width();

}

int cap_intr_csr_cnt_intr_tot_axi_wr_t::get_width() const {
    return cap_intr_csr_cnt_intr_tot_axi_wr_t::s_get_width();

}

int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::get_width() const {
    return cap_intr_csr_sta_intr_ecc_intr_coalesce_t::s_get_width();

}

int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::get_width() const {
    return cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::s_get_width();

}

int cap_intr_csr_sta_intr_ecc_intr_state_t::get_width() const {
    return cap_intr_csr_sta_intr_ecc_intr_state_t::s_get_width();

}

int cap_intr_csr_cfg_intr_ecc_disable_t::get_width() const {
    return cap_intr_csr_cfg_intr_ecc_disable_t::s_get_width();

}

int cap_intr_csr_sta_intr_debug_t::get_width() const {
    return cap_intr_csr_sta_intr_debug_t::s_get_width();

}

int cap_intr_csr_sta_intr_init_t::get_width() const {
    return cap_intr_csr_sta_intr_init_t::s_get_width();

}

int cap_intr_csr_sta_intr_axi_t::get_width() const {
    return cap_intr_csr_sta_intr_axi_t::s_get_width();

}

int cap_intr_csr_cfg_intr_axi_t::get_width() const {
    return cap_intr_csr_cfg_intr_axi_t::s_get_width();

}

int cap_intr_csr_cfg_intr_coalesce_t::get_width() const {
    return cap_intr_csr_cfg_intr_coalesce_t::s_get_width();

}

int cap_intr_csr_rdintr_t::get_width() const {
    return cap_intr_csr_rdintr_t::s_get_width();

}

int cap_intr_csr_base_t::get_width() const {
    return cap_intr_csr_base_t::s_get_width();

}

int cap_intr_csr_t::get_width() const {
    return cap_intr_csr_t::s_get_width();

}

int cap_intr_csr_int_intr_ecc_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // intr_state_uncorrectable_enable
    _count += 1; // intr_state_correctable_enable
    _count += 1; // intr_pba_cfg_uncorrectable_enable
    _count += 1; // intr_pba_cfg_correctable_enable
    _count += 1; // intr_coalesce_uncorrectable_enable
    _count += 1; // intr_coalesce_correctable_enable
    _count += 1; // intr_legacy_fifo_overrun_enable
    _count += 1; // int_credit_positive_after_return_enable
    _count += 1; // int_credits_max_debug_threshold_enable
    _count += 1; // intr_bresp_err_enable
    return _count;
}

int cap_intr_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // intr_state_uncorrectable_interrupt
    _count += 1; // intr_state_correctable_interrupt
    _count += 1; // intr_pba_cfg_uncorrectable_interrupt
    _count += 1; // intr_pba_cfg_correctable_interrupt
    _count += 1; // intr_coalesce_uncorrectable_interrupt
    _count += 1; // intr_coalesce_correctable_interrupt
    _count += 1; // intr_legacy_fifo_overrun_interrupt
    _count += 1; // int_credit_positive_after_return_interrupt
    _count += 1; // int_credits_max_debug_threshold_interrupt
    _count += 1; // intr_bresp_err_interrupt
    return _count;
}

int cap_intr_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_intr_csr_intreg_t::s_get_width(); // intreg
    _count += cap_intr_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_intr_csr_int_intr_ecc_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_intr_csr_int_intr_ecc_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_intr_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_intr_ecc_interrupt
    return _count;
}

int cap_intr_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_intr_ecc_enable
    return _count;
}

int cap_intr_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_intr_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_intr_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_intr_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_intr_csr_dhs_intr_state_entry_t::s_get_width() {
    int _count = 0;

    _count += 50; // msixcfg_msg_addr_51_2
    _count += 32; // msixcfg_msg_data
    _count += 1; // msixcfg_vector_ctrl
    _count += 1; // fwcfg_function_mask
    _count += 11; // fwcfg_lif
    _count += 1; // fwcfg_local_int
    _count += 1; // fwcfg_legacy_int
    _count += 2; // fwcfg_legacy_pin
    _count += 1; // drvcfg_mask
    _count += 16; // drvcfg_int_credits
    _count += 1; // drvcfg_mask_on_assert
    _count += 3; // fwcfg_port_id
    _count += 8; // ecc
    return _count;
}

int cap_intr_csr_dhs_intr_state_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_state_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_coalesce_entry_t::s_get_width() {
    int _count = 0;

    _count += 6; // coal_init_value0
    _count += 6; // coal_init_value1
    _count += 6; // coal_init_value2
    _count += 6; // coal_init_value3
    _count += 6; // coal_curr_value0
    _count += 6; // coal_curr_value1
    _count += 6; // coal_curr_value2
    _count += 6; // coal_curr_value3
    _count += 4; // hw_use_rsvd
    _count += 1; // hw_legacy_sent_state0
    _count += 1; // hw_legacy_sent_state1
    _count += 1; // hw_legacy_sent_state2
    _count += 1; // hw_legacy_sent_state3
    _count += 7; // ecc
    return _count;
}

int cap_intr_csr_dhs_intr_coalesce_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_coalesce_entry_t::s_get_width() * 1024); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_assert_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // data
    _count += 1; // rst_coalesce
    return _count;
}

int cap_intr_csr_dhs_intr_assert_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_assert_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_pba_array_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // vec
    return _count;
}

int cap_intr_csr_dhs_intr_pba_array_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_pba_array_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_pba_cfg_entry_t::s_get_width() {
    int _count = 0;

    _count += 12; // start
    _count += 6; // count
    _count += 6; // ecc
    return _count;
}

int cap_intr_csr_dhs_intr_pba_cfg_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_pba_cfg_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_drvcfg_entry_t::s_get_width() {
    int _count = 0;

    _count += 6; // coal_init
    _count += 26; // reserved1
    _count += 1; // mask
    _count += 31; // reserved2
    _count += 16; // int_credits
    _count += 1; // unmask
    _count += 1; // restart_coal
    _count += 14; // reserved3
    _count += 1; // mask_on_assert
    _count += 31; // reserved4
    _count += 6; // coal_curr
    _count += 26; // reserved5
    return _count;
}

int cap_intr_csr_dhs_intr_drvcfg_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_drvcfg_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_fwcfg_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // function_mask
    _count += 31; // reserved1
    _count += 11; // lif
    _count += 3; // port_id
    _count += 1; // local_int
    _count += 1; // legacy
    _count += 2; // intpin
    _count += 14; // reserved2
    return _count;
}

int cap_intr_csr_dhs_intr_fwcfg_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_fwcfg_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_intr_csr_dhs_intr_msixcfg_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // msg_addr
    _count += 32; // msg_data
    _count += 1; // vector_ctrl
    _count += 31; // reserved1
    return _count;
}

int cap_intr_csr_dhs_intr_msixcfg_t::s_get_width() {
    int _count = 0;

    _count += (cap_intr_csr_dhs_intr_msixcfg_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_intr_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_intr_csr_sta_sram_bist_t::s_get_width() {
    int _count = 0;

    _count += 3; // done_pass
    _count += 3; // done_fail
    return _count;
}

int cap_intr_csr_cfg_sram_bist_t::s_get_width() {
    int _count = 0;

    _count += 3; // run
    return _count;
}

int cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 16; // debug_readout
    return _count;
}

int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::s_get_width() {
    int _count = 0;

    _count += 32; // dw0
    _count += 32; // dw1
    _count += 32; // dw2
    _count += 32; // dw3
    return _count;
}

int cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::s_get_width() {
    int _count = 0;

    _count += 1; // source
    _count += 8; // addr
    return _count;
}

int cap_intr_csr_cfg_debug_max_credits_limit_t::s_get_width() {
    int _count = 0;

    _count += 16; // int_threshold
    return _count;
}

int cap_intr_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 2; // select
    _count += 1; // enable
    return _count;
}

int cap_intr_csr_cfg_intr_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // awcache
    _count += 3; // prot
    _count += 4; // qos
    _count += 1; // lock
    return _count;
}

int cap_intr_csr_sat_intr_event_counters_t::s_get_width() {
    int _count = 0;

    _count += 8; // axi_bresp_errs
    _count += 8; // delayed_coal_scan
    return _count;
}

int cap_intr_csr_cnt_intr_legacy_send_t::s_get_width() {
    int _count = 0;

    _count += 40; // req
    return _count;
}

int cap_intr_csr_cnt_intr_tot_axi_wr_t::s_get_width() {
    int _count = 0;

    _count += 40; // req
    return _count;
}

int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 7; // syndrome
    _count += 10; // addr
    return _count;
}

int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 6; // syndrome
    _count += 11; // addr
    return _count;
}

int cap_intr_csr_sta_intr_ecc_intr_state_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 12; // addr
    return _count;
}

int cap_intr_csr_cfg_intr_ecc_disable_t::s_get_width() {
    int _count = 0;

    _count += 1; // intr_state_cor
    _count += 1; // intr_state_det
    _count += 1; // intr_state_dhs
    _count += 1; // intr_pba_cfg_cor
    _count += 1; // intr_pba_cfg_det
    _count += 1; // intr_pba_cfg_dhs
    _count += 1; // intr_coalesce_cor
    _count += 1; // intr_coalesce_det
    _count += 1; // intr_coalesce_dhs
    return _count;
}

int cap_intr_csr_sta_intr_debug_t::s_get_width() {
    int _count = 0;

    _count += 7; // legacy_fifodepth
    _count += 4; // coalexp_fifodepth
    _count += 3; // wfifo_depth
    _count += 3; // awfifo_depth
    _count += 1; // wready
    _count += 1; // wvalid
    _count += 1; // awready
    _count += 1; // awvalid
    _count += 1; // axi_rdy
    _count += 1; // coal_scan_active
    _count += 1; // init_done
    return _count;
}

int cap_intr_csr_sta_intr_init_t::s_get_width() {
    int _count = 0;

    _count += 1; // done
    return _count;
}

int cap_intr_csr_sta_intr_axi_t::s_get_width() {
    int _count = 0;

    _count += 8; // num_ids
    return _count;
}

int cap_intr_csr_cfg_intr_axi_t::s_get_width() {
    int _count = 0;

    _count += 8; // num_ids
    return _count;
}

int cap_intr_csr_cfg_intr_coalesce_t::s_get_width() {
    int _count = 0;

    _count += 16; // resolution
    return _count;
}

int cap_intr_csr_rdintr_t::s_get_width() {
    int _count = 0;

    _count += 32; // ireg
    return _count;
}

int cap_intr_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_intr_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_intr_csr_base_t::s_get_width(); // base
    _count += cap_intr_csr_rdintr_t::s_get_width(); // rdintr
    _count += cap_intr_csr_cfg_intr_coalesce_t::s_get_width(); // cfg_intr_coalesce
    _count += cap_intr_csr_cfg_intr_axi_t::s_get_width(); // cfg_intr_axi
    _count += cap_intr_csr_sta_intr_axi_t::s_get_width(); // sta_intr_axi
    _count += cap_intr_csr_sta_intr_init_t::s_get_width(); // sta_intr_init
    _count += cap_intr_csr_sta_intr_debug_t::s_get_width(); // sta_intr_debug
    _count += cap_intr_csr_cfg_intr_ecc_disable_t::s_get_width(); // cfg_intr_ecc_disable
    _count += cap_intr_csr_sta_intr_ecc_intr_state_t::s_get_width(); // sta_intr_ecc_intr_state
    _count += cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::s_get_width(); // sta_intr_ecc_intr_pba_cfg
    _count += cap_intr_csr_sta_intr_ecc_intr_coalesce_t::s_get_width(); // sta_intr_ecc_intr_coalesce
    _count += cap_intr_csr_cnt_intr_tot_axi_wr_t::s_get_width(); // cnt_intr_tot_axi_wr
    _count += cap_intr_csr_cnt_intr_legacy_send_t::s_get_width(); // cnt_intr_legacy_send
    _count += cap_intr_csr_sat_intr_event_counters_t::s_get_width(); // sat_intr_event_counters
    _count += cap_intr_csr_cfg_intr_axi_attr_t::s_get_width(); // cfg_intr_axi_attr
    _count += cap_intr_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_intr_csr_cfg_debug_max_credits_limit_t::s_get_width(); // cfg_debug_max_credits_limit
    _count += cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::s_get_width(); // cfg_todo_bits_legacy_cnt_read_debug
    _count += cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::s_get_width(); // cfg_legacy_intx_pcie_msg_hdr
    _count += cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::s_get_width(); // sta_todo_bits_legacy_cnt_read_debug
    _count += cap_intr_csr_cfg_sram_bist_t::s_get_width(); // cfg_sram_bist
    _count += cap_intr_csr_sta_sram_bist_t::s_get_width(); // sta_sram_bist
    _count += cap_intr_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_intr_csr_dhs_intr_msixcfg_t::s_get_width(); // dhs_intr_msixcfg
    _count += cap_intr_csr_dhs_intr_fwcfg_t::s_get_width(); // dhs_intr_fwcfg
    _count += cap_intr_csr_dhs_intr_drvcfg_t::s_get_width(); // dhs_intr_drvcfg
    _count += cap_intr_csr_dhs_intr_pba_cfg_t::s_get_width(); // dhs_intr_pba_cfg
    _count += cap_intr_csr_dhs_intr_pba_array_t::s_get_width(); // dhs_intr_pba_array
    _count += cap_intr_csr_dhs_intr_assert_t::s_get_width(); // dhs_intr_assert
    _count += cap_intr_csr_dhs_intr_coalesce_t::s_get_width(); // dhs_intr_coalesce
    _count += cap_intr_csr_dhs_intr_state_t::s_get_width(); // dhs_intr_state
    _count += cap_intr_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_intr_csr_intgrp_t::s_get_width(); // int_intr_ecc
    return _count;
}

void cap_intr_csr_int_intr_ecc_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__intr_state_uncorrectable_enable = _val.convert_to< intr_state_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_state_correctable_enable = _val.convert_to< intr_state_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_uncorrectable_enable = _val.convert_to< intr_pba_cfg_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_correctable_enable = _val.convert_to< intr_pba_cfg_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_uncorrectable_enable = _val.convert_to< intr_coalesce_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_correctable_enable = _val.convert_to< intr_coalesce_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_legacy_fifo_overrun_enable = _val.convert_to< intr_legacy_fifo_overrun_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_credit_positive_after_return_enable = _val.convert_to< int_credit_positive_after_return_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_credits_max_debug_threshold_enable = _val.convert_to< int_credits_max_debug_threshold_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_bresp_err_enable = _val.convert_to< intr_bresp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__intr_state_uncorrectable_interrupt = _val.convert_to< intr_state_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_state_correctable_interrupt = _val.convert_to< intr_state_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_uncorrectable_interrupt = _val.convert_to< intr_pba_cfg_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_correctable_interrupt = _val.convert_to< intr_pba_cfg_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_uncorrectable_interrupt = _val.convert_to< intr_coalesce_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_correctable_interrupt = _val.convert_to< intr_coalesce_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_legacy_fifo_overrun_interrupt = _val.convert_to< intr_legacy_fifo_overrun_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_credit_positive_after_return_interrupt = _val.convert_to< int_credit_positive_after_return_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_credits_max_debug_threshold_interrupt = _val.convert_to< int_credits_max_debug_threshold_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_bresp_err_interrupt = _val.convert_to< intr_bresp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_intgrp_t::all(const cpp_int & in_val) {
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

void cap_intr_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_intr_ecc_interrupt = _val.convert_to< int_intr_ecc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_intr_ecc_enable = _val.convert_to< int_intr_ecc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_intr_csr_dhs_intr_state_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msixcfg_msg_addr_51_2 = _val.convert_to< msixcfg_msg_addr_51_2_cpp_int_t >()  ;
    _val = _val >> 50;
    
    int_var__msixcfg_msg_data = _val.convert_to< msixcfg_msg_data_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__msixcfg_vector_ctrl = _val.convert_to< msixcfg_vector_ctrl_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fwcfg_function_mask = _val.convert_to< fwcfg_function_mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fwcfg_lif = _val.convert_to< fwcfg_lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__fwcfg_local_int = _val.convert_to< fwcfg_local_int_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fwcfg_legacy_int = _val.convert_to< fwcfg_legacy_int_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fwcfg_legacy_pin = _val.convert_to< fwcfg_legacy_pin_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__drvcfg_mask = _val.convert_to< drvcfg_mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__drvcfg_int_credits = _val.convert_to< drvcfg_int_credits_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__drvcfg_mask_on_assert = _val.convert_to< drvcfg_mask_on_assert_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fwcfg_port_id = _val.convert_to< fwcfg_port_id_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_intr_csr_dhs_intr_state_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_coalesce_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__coal_init_value0 = _val.convert_to< coal_init_value0_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_init_value1 = _val.convert_to< coal_init_value1_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_init_value2 = _val.convert_to< coal_init_value2_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_init_value3 = _val.convert_to< coal_init_value3_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_curr_value0 = _val.convert_to< coal_curr_value0_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_curr_value1 = _val.convert_to< coal_curr_value1_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_curr_value2 = _val.convert_to< coal_curr_value2_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__coal_curr_value3 = _val.convert_to< coal_curr_value3_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__hw_use_rsvd = _val.convert_to< hw_use_rsvd_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__hw_legacy_sent_state0 = _val.convert_to< hw_legacy_sent_state0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hw_legacy_sent_state1 = _val.convert_to< hw_legacy_sent_state1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hw_legacy_sent_state2 = _val.convert_to< hw_legacy_sent_state2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hw_legacy_sent_state3 = _val.convert_to< hw_legacy_sent_state3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_intr_csr_dhs_intr_coalesce_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_assert_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rst_coalesce = _val.convert_to< rst_coalesce_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_dhs_intr_assert_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_array_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__vec = _val.convert_to< vec_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_intr_csr_dhs_intr_pba_array_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_cfg_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__start = _val.convert_to< start_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__count = _val.convert_to< count_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_intr_csr_dhs_intr_pba_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_drvcfg_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__coal_init = _val.convert_to< coal_init_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >()  ;
    _val = _val >> 26;
    
    int_var__mask = _val.convert_to< mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__reserved2 = _val.convert_to< reserved2_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__int_credits = _val.convert_to< int_credits_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__unmask = _val.convert_to< unmask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__restart_coal = _val.convert_to< restart_coal_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__reserved3 = _val.convert_to< reserved3_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__mask_on_assert = _val.convert_to< mask_on_assert_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__reserved4 = _val.convert_to< reserved4_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__coal_curr = _val.convert_to< coal_curr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__reserved5 = _val.convert_to< reserved5_cpp_int_t >()  ;
    _val = _val >> 26;
    
}

void cap_intr_csr_dhs_intr_drvcfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_fwcfg_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__function_mask = _val.convert_to< function_mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__port_id = _val.convert_to< port_id_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__local_int = _val.convert_to< local_int_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__legacy = _val.convert_to< legacy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intpin = _val.convert_to< intpin_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__reserved2 = _val.convert_to< reserved2_cpp_int_t >()  ;
    _val = _val >> 14;
    
}

void cap_intr_csr_dhs_intr_fwcfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_msixcfg_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msg_addr = _val.convert_to< msg_addr_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__msg_data = _val.convert_to< msg_data_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__vector_ctrl = _val.convert_to< vector_ctrl_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >()  ;
    _val = _val >> 31;
    
}

void cap_intr_csr_dhs_intr_msixcfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_intr_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_sta_sram_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__done_pass = _val.convert_to< done_pass_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__done_fail = _val.convert_to< done_fail_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_intr_csr_cfg_sram_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__run = _val.convert_to< run_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__debug_readout = _val.convert_to< debug_readout_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dw0 = _val.convert_to< dw0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__dw1 = _val.convert_to< dw1_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__dw2 = _val.convert_to< dw2_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__dw3 = _val.convert_to< dw3_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__source = _val.convert_to< source_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_intr_csr_cfg_debug_max_credits_limit_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_threshold = _val.convert_to< int_threshold_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_intr_csr_cfg_debug_port_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__select = _val.convert_to< select_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_cfg_intr_axi_attr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__awcache = _val.convert_to< awcache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_sat_intr_event_counters_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_bresp_errs = _val.convert_to< axi_bresp_errs_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__delayed_coal_scan = _val.convert_to< delayed_coal_scan_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_intr_csr_cnt_intr_legacy_send_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__req = _val.convert_to< req_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_intr_csr_cnt_intr_tot_axi_wr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__req = _val.convert_to< req_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 11;
    
}

void cap_intr_csr_sta_intr_ecc_intr_state_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_intr_csr_cfg_intr_ecc_disable_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__intr_state_cor = _val.convert_to< intr_state_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_state_det = _val.convert_to< intr_state_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_state_dhs = _val.convert_to< intr_state_dhs_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_cor = _val.convert_to< intr_pba_cfg_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_det = _val.convert_to< intr_pba_cfg_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_pba_cfg_dhs = _val.convert_to< intr_pba_cfg_dhs_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_cor = _val.convert_to< intr_coalesce_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_det = _val.convert_to< intr_coalesce_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__intr_coalesce_dhs = _val.convert_to< intr_coalesce_dhs_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_sta_intr_debug_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__legacy_fifodepth = _val.convert_to< legacy_fifodepth_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__coalexp_fifodepth = _val.convert_to< coalexp_fifodepth_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__wfifo_depth = _val.convert_to< wfifo_depth_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__awfifo_depth = _val.convert_to< awfifo_depth_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__wready = _val.convert_to< wready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wvalid = _val.convert_to< wvalid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__awready = _val.convert_to< awready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__awvalid = _val.convert_to< awvalid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_rdy = _val.convert_to< axi_rdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__coal_scan_active = _val.convert_to< coal_scan_active_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__init_done = _val.convert_to< init_done_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_sta_intr_init_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__done = _val.convert_to< done_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_intr_csr_sta_intr_axi_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_intr_csr_cfg_intr_axi_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_intr_csr_cfg_intr_coalesce_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__resolution = _val.convert_to< resolution_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_intr_csr_rdintr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ireg = _val.convert_to< ireg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_intr_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_intr_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    rdintr.all( _val);
    _val = _val >> rdintr.get_width(); 
    cfg_intr_coalesce.all( _val);
    _val = _val >> cfg_intr_coalesce.get_width(); 
    cfg_intr_axi.all( _val);
    _val = _val >> cfg_intr_axi.get_width(); 
    sta_intr_axi.all( _val);
    _val = _val >> sta_intr_axi.get_width(); 
    sta_intr_init.all( _val);
    _val = _val >> sta_intr_init.get_width(); 
    sta_intr_debug.all( _val);
    _val = _val >> sta_intr_debug.get_width(); 
    cfg_intr_ecc_disable.all( _val);
    _val = _val >> cfg_intr_ecc_disable.get_width(); 
    sta_intr_ecc_intr_state.all( _val);
    _val = _val >> sta_intr_ecc_intr_state.get_width(); 
    sta_intr_ecc_intr_pba_cfg.all( _val);
    _val = _val >> sta_intr_ecc_intr_pba_cfg.get_width(); 
    sta_intr_ecc_intr_coalesce.all( _val);
    _val = _val >> sta_intr_ecc_intr_coalesce.get_width(); 
    cnt_intr_tot_axi_wr.all( _val);
    _val = _val >> cnt_intr_tot_axi_wr.get_width(); 
    cnt_intr_legacy_send.all( _val);
    _val = _val >> cnt_intr_legacy_send.get_width(); 
    sat_intr_event_counters.all( _val);
    _val = _val >> sat_intr_event_counters.get_width(); 
    cfg_intr_axi_attr.all( _val);
    _val = _val >> cfg_intr_axi_attr.get_width(); 
    cfg_debug_port.all( _val);
    _val = _val >> cfg_debug_port.get_width(); 
    cfg_debug_max_credits_limit.all( _val);
    _val = _val >> cfg_debug_max_credits_limit.get_width(); 
    cfg_todo_bits_legacy_cnt_read_debug.all( _val);
    _val = _val >> cfg_todo_bits_legacy_cnt_read_debug.get_width(); 
    cfg_legacy_intx_pcie_msg_hdr.all( _val);
    _val = _val >> cfg_legacy_intx_pcie_msg_hdr.get_width(); 
    sta_todo_bits_legacy_cnt_read_debug.all( _val);
    _val = _val >> sta_todo_bits_legacy_cnt_read_debug.get_width(); 
    cfg_sram_bist.all( _val);
    _val = _val >> cfg_sram_bist.get_width(); 
    sta_sram_bist.all( _val);
    _val = _val >> sta_sram_bist.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    dhs_intr_msixcfg.all( _val);
    _val = _val >> dhs_intr_msixcfg.get_width(); 
    dhs_intr_fwcfg.all( _val);
    _val = _val >> dhs_intr_fwcfg.get_width(); 
    dhs_intr_drvcfg.all( _val);
    _val = _val >> dhs_intr_drvcfg.get_width(); 
    dhs_intr_pba_cfg.all( _val);
    _val = _val >> dhs_intr_pba_cfg.get_width(); 
    dhs_intr_pba_array.all( _val);
    _val = _val >> dhs_intr_pba_array.get_width(); 
    dhs_intr_assert.all( _val);
    _val = _val >> dhs_intr_assert.get_width(); 
    dhs_intr_coalesce.all( _val);
    _val = _val >> dhs_intr_coalesce.get_width(); 
    dhs_intr_state.all( _val);
    _val = _val >> dhs_intr_state.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_intr_ecc.all( _val);
    _val = _val >> int_intr_ecc.get_width(); 
}

cpp_int cap_intr_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_intr_ecc.get_width(); ret_val = ret_val  | int_intr_ecc.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << dhs_intr_state.get_width(); ret_val = ret_val  | dhs_intr_state.all(); 
    ret_val = ret_val << dhs_intr_coalesce.get_width(); ret_val = ret_val  | dhs_intr_coalesce.all(); 
    ret_val = ret_val << dhs_intr_assert.get_width(); ret_val = ret_val  | dhs_intr_assert.all(); 
    ret_val = ret_val << dhs_intr_pba_array.get_width(); ret_val = ret_val  | dhs_intr_pba_array.all(); 
    ret_val = ret_val << dhs_intr_pba_cfg.get_width(); ret_val = ret_val  | dhs_intr_pba_cfg.all(); 
    ret_val = ret_val << dhs_intr_drvcfg.get_width(); ret_val = ret_val  | dhs_intr_drvcfg.all(); 
    ret_val = ret_val << dhs_intr_fwcfg.get_width(); ret_val = ret_val  | dhs_intr_fwcfg.all(); 
    ret_val = ret_val << dhs_intr_msixcfg.get_width(); ret_val = ret_val  | dhs_intr_msixcfg.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_sram_bist.get_width(); ret_val = ret_val  | sta_sram_bist.all(); 
    ret_val = ret_val << cfg_sram_bist.get_width(); ret_val = ret_val  | cfg_sram_bist.all(); 
    ret_val = ret_val << sta_todo_bits_legacy_cnt_read_debug.get_width(); ret_val = ret_val  | sta_todo_bits_legacy_cnt_read_debug.all(); 
    ret_val = ret_val << cfg_legacy_intx_pcie_msg_hdr.get_width(); ret_val = ret_val  | cfg_legacy_intx_pcie_msg_hdr.all(); 
    ret_val = ret_val << cfg_todo_bits_legacy_cnt_read_debug.get_width(); ret_val = ret_val  | cfg_todo_bits_legacy_cnt_read_debug.all(); 
    ret_val = ret_val << cfg_debug_max_credits_limit.get_width(); ret_val = ret_val  | cfg_debug_max_credits_limit.all(); 
    ret_val = ret_val << cfg_debug_port.get_width(); ret_val = ret_val  | cfg_debug_port.all(); 
    ret_val = ret_val << cfg_intr_axi_attr.get_width(); ret_val = ret_val  | cfg_intr_axi_attr.all(); 
    ret_val = ret_val << sat_intr_event_counters.get_width(); ret_val = ret_val  | sat_intr_event_counters.all(); 
    ret_val = ret_val << cnt_intr_legacy_send.get_width(); ret_val = ret_val  | cnt_intr_legacy_send.all(); 
    ret_val = ret_val << cnt_intr_tot_axi_wr.get_width(); ret_val = ret_val  | cnt_intr_tot_axi_wr.all(); 
    ret_val = ret_val << sta_intr_ecc_intr_coalesce.get_width(); ret_val = ret_val  | sta_intr_ecc_intr_coalesce.all(); 
    ret_val = ret_val << sta_intr_ecc_intr_pba_cfg.get_width(); ret_val = ret_val  | sta_intr_ecc_intr_pba_cfg.all(); 
    ret_val = ret_val << sta_intr_ecc_intr_state.get_width(); ret_val = ret_val  | sta_intr_ecc_intr_state.all(); 
    ret_val = ret_val << cfg_intr_ecc_disable.get_width(); ret_val = ret_val  | cfg_intr_ecc_disable.all(); 
    ret_val = ret_val << sta_intr_debug.get_width(); ret_val = ret_val  | sta_intr_debug.all(); 
    ret_val = ret_val << sta_intr_init.get_width(); ret_val = ret_val  | sta_intr_init.all(); 
    ret_val = ret_val << sta_intr_axi.get_width(); ret_val = ret_val  | sta_intr_axi.all(); 
    ret_val = ret_val << cfg_intr_axi.get_width(); ret_val = ret_val  | cfg_intr_axi.all(); 
    ret_val = ret_val << cfg_intr_coalesce.get_width(); ret_val = ret_val  | cfg_intr_coalesce.all(); 
    ret_val = ret_val << rdintr.get_width(); ret_val = ret_val  | rdintr.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_intr_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_intr_csr_rdintr_t::all() const {
    cpp_int ret_val;

    // ireg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ireg; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_intr_coalesce_t::all() const {
    cpp_int ret_val;

    // resolution
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__resolution; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_intr_axi_t::all() const {
    cpp_int ret_val;

    // num_ids
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__num_ids; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_axi_t::all() const {
    cpp_int ret_val;

    // num_ids
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__num_ids; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_init_t::all() const {
    cpp_int ret_val;

    // done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_debug_t::all() const {
    cpp_int ret_val;

    // init_done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__init_done; 
    
    // coal_scan_active
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__coal_scan_active; 
    
    // axi_rdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_rdy; 
    
    // awvalid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__awvalid; 
    
    // awready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__awready; 
    
    // wvalid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wvalid; 
    
    // wready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wready; 
    
    // awfifo_depth
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__awfifo_depth; 
    
    // wfifo_depth
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__wfifo_depth; 
    
    // coalexp_fifodepth
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__coalexp_fifodepth; 
    
    // legacy_fifodepth
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__legacy_fifodepth; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::all() const {
    cpp_int ret_val;

    // intr_coalesce_dhs
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_dhs; 
    
    // intr_coalesce_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_det; 
    
    // intr_coalesce_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_cor; 
    
    // intr_pba_cfg_dhs
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_dhs; 
    
    // intr_pba_cfg_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_det; 
    
    // intr_pba_cfg_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_cor; 
    
    // intr_state_dhs
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_dhs; 
    
    // intr_state_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_det; 
    
    // intr_state_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_cor; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_state_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cnt_intr_tot_axi_wr_t::all() const {
    cpp_int ret_val;

    // req
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__req; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cnt_intr_legacy_send_t::all() const {
    cpp_int ret_val;

    // req
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__req; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sat_intr_event_counters_t::all() const {
    cpp_int ret_val;

    // delayed_coal_scan
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__delayed_coal_scan; 
    
    // axi_bresp_errs
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__axi_bresp_errs; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_intr_axi_attr_t::all() const {
    cpp_int ret_val;

    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // awcache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__awcache; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_debug_port_t::all() const {
    cpp_int ret_val;

    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    // select
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__select; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_debug_max_credits_limit_t::all() const {
    cpp_int ret_val;

    // int_threshold
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__int_threshold; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr; 
    
    // source
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__source; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::all() const {
    cpp_int ret_val;

    // dw3
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__dw3; 
    
    // dw2
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__dw2; 
    
    // dw1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__dw1; 
    
    // dw0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__dw0; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::all() const {
    cpp_int ret_val;

    // debug_readout
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__debug_readout; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    return ret_val;
}

cpp_int cap_intr_csr_cfg_sram_bist_t::all() const {
    cpp_int ret_val;

    // run
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__run; 
    
    return ret_val;
}

cpp_int cap_intr_csr_sta_sram_bist_t::all() const {
    cpp_int ret_val;

    // done_fail
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__done_fail; 
    
    // done_pass
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__done_pass; 
    
    return ret_val;
}

cpp_int cap_intr_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_entry_t::all() const {
    cpp_int ret_val;

    // reserved1
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__reserved1; 
    
    // vector_ctrl
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__vector_ctrl; 
    
    // msg_data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__msg_data; 
    
    // msg_addr
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__msg_addr; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::all() const {
    cpp_int ret_val;

    // reserved2
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__reserved2; 
    
    // intpin
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__intpin; 
    
    // legacy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__legacy; 
    
    // local_int
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__local_int; 
    
    // port_id
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__port_id; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    // reserved1
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__reserved1; 
    
    // function_mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__function_mask; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::all() const {
    cpp_int ret_val;

    // reserved5
    ret_val = ret_val << 26; ret_val = ret_val  | int_var__reserved5; 
    
    // coal_curr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_curr; 
    
    // reserved4
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__reserved4; 
    
    // mask_on_assert
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask_on_assert; 
    
    // reserved3
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__reserved3; 
    
    // restart_coal
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__restart_coal; 
    
    // unmask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__unmask; 
    
    // int_credits
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__int_credits; 
    
    // reserved2
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__reserved2; 
    
    // mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask; 
    
    // reserved1
    ret_val = ret_val << 26; ret_val = ret_val  | int_var__reserved1; 
    
    // coal_init
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_init; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_pba_cfg_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_pba_cfg_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ecc; 
    
    // count
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__count; 
    
    // start
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__start; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_pba_array_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_pba_array_entry_t::all() const {
    cpp_int ret_val;

    // vec
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__vec; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_assert_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_assert_entry_t::all() const {
    cpp_int ret_val;

    // rst_coalesce
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rst_coalesce; 
    
    // data
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_coalesce_t::all() const {
    cpp_int ret_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 1024-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ecc; 
    
    // hw_legacy_sent_state3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hw_legacy_sent_state3; 
    
    // hw_legacy_sent_state2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hw_legacy_sent_state2; 
    
    // hw_legacy_sent_state1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hw_legacy_sent_state1; 
    
    // hw_legacy_sent_state0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hw_legacy_sent_state0; 
    
    // hw_use_rsvd
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__hw_use_rsvd; 
    
    // coal_curr_value3
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_curr_value3; 
    
    // coal_curr_value2
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_curr_value2; 
    
    // coal_curr_value1
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_curr_value1; 
    
    // coal_curr_value0
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_curr_value0; 
    
    // coal_init_value3
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_init_value3; 
    
    // coal_init_value2
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_init_value2; 
    
    // coal_init_value1
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_init_value1; 
    
    // coal_init_value0
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__coal_init_value0; 
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_state_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ecc; 
    
    // fwcfg_port_id
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__fwcfg_port_id; 
    
    // drvcfg_mask_on_assert
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__drvcfg_mask_on_assert; 
    
    // drvcfg_int_credits
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__drvcfg_int_credits; 
    
    // drvcfg_mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__drvcfg_mask; 
    
    // fwcfg_legacy_pin
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__fwcfg_legacy_pin; 
    
    // fwcfg_legacy_int
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fwcfg_legacy_int; 
    
    // fwcfg_local_int
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fwcfg_local_int; 
    
    // fwcfg_lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__fwcfg_lif; 
    
    // fwcfg_function_mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fwcfg_function_mask; 
    
    // msixcfg_vector_ctrl
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__msixcfg_vector_ctrl; 
    
    // msixcfg_msg_data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__msixcfg_msg_data; 
    
    // msixcfg_msg_addr_51_2
    ret_val = ret_val << 50; ret_val = ret_val  | int_var__msixcfg_msg_addr_51_2; 
    
    return ret_val;
}

cpp_int cap_intr_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_intr_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_intr_ecc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_intr_ecc_interrupt; 
    
    return ret_val;
}

cpp_int cap_intr_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_intr_ecc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_intr_ecc_enable; 
    
    return ret_val;
}

cpp_int cap_intr_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_intr_csr_intreg_t::all() const {
    cpp_int ret_val;

    // intr_bresp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_bresp_err_interrupt; 
    
    // int_credits_max_debug_threshold_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_credits_max_debug_threshold_interrupt; 
    
    // int_credit_positive_after_return_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_credit_positive_after_return_interrupt; 
    
    // intr_legacy_fifo_overrun_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_legacy_fifo_overrun_interrupt; 
    
    // intr_coalesce_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_correctable_interrupt; 
    
    // intr_coalesce_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_uncorrectable_interrupt; 
    
    // intr_pba_cfg_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_correctable_interrupt; 
    
    // intr_pba_cfg_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_uncorrectable_interrupt; 
    
    // intr_state_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_correctable_interrupt; 
    
    // intr_state_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_uncorrectable_interrupt; 
    
    return ret_val;
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // intr_bresp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_bresp_err_enable; 
    
    // int_credits_max_debug_threshold_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_credits_max_debug_threshold_enable; 
    
    // int_credit_positive_after_return_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_credit_positive_after_return_enable; 
    
    // intr_legacy_fifo_overrun_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_legacy_fifo_overrun_enable; 
    
    // intr_coalesce_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_correctable_enable; 
    
    // intr_coalesce_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_coalesce_uncorrectable_enable; 
    
    // intr_pba_cfg_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_correctable_enable; 
    
    // intr_pba_cfg_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_pba_cfg_uncorrectable_enable; 
    
    // intr_state_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_correctable_enable; 
    
    // intr_state_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__intr_state_uncorrectable_enable; 
    
    return ret_val;
}

void cap_intr_csr_int_intr_ecc_int_enable_clear_t::clear() {

    int_var__intr_state_uncorrectable_enable = 0; 
    
    int_var__intr_state_correctable_enable = 0; 
    
    int_var__intr_pba_cfg_uncorrectable_enable = 0; 
    
    int_var__intr_pba_cfg_correctable_enable = 0; 
    
    int_var__intr_coalesce_uncorrectable_enable = 0; 
    
    int_var__intr_coalesce_correctable_enable = 0; 
    
    int_var__intr_legacy_fifo_overrun_enable = 0; 
    
    int_var__int_credit_positive_after_return_enable = 0; 
    
    int_var__int_credits_max_debug_threshold_enable = 0; 
    
    int_var__intr_bresp_err_enable = 0; 
    
}

void cap_intr_csr_intreg_t::clear() {

    int_var__intr_state_uncorrectable_interrupt = 0; 
    
    int_var__intr_state_correctable_interrupt = 0; 
    
    int_var__intr_pba_cfg_uncorrectable_interrupt = 0; 
    
    int_var__intr_pba_cfg_correctable_interrupt = 0; 
    
    int_var__intr_coalesce_uncorrectable_interrupt = 0; 
    
    int_var__intr_coalesce_correctable_interrupt = 0; 
    
    int_var__intr_legacy_fifo_overrun_interrupt = 0; 
    
    int_var__int_credit_positive_after_return_interrupt = 0; 
    
    int_var__int_credits_max_debug_threshold_interrupt = 0; 
    
    int_var__intr_bresp_err_interrupt = 0; 
    
}

void cap_intr_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_intr_csr_intreg_status_t::clear() {

    int_var__int_intr_ecc_interrupt = 0; 
    
}

void cap_intr_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_intr_ecc_enable = 0; 
    
}

void cap_intr_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_intr_csr_dhs_intr_state_entry_t::clear() {

    int_var__msixcfg_msg_addr_51_2 = 0; 
    
    int_var__msixcfg_msg_data = 0; 
    
    int_var__msixcfg_vector_ctrl = 0; 
    
    int_var__fwcfg_function_mask = 0; 
    
    int_var__fwcfg_lif = 0; 
    
    int_var__fwcfg_local_int = 0; 
    
    int_var__fwcfg_legacy_int = 0; 
    
    int_var__fwcfg_legacy_pin = 0; 
    
    int_var__drvcfg_mask = 0; 
    
    int_var__drvcfg_int_credits = 0; 
    
    int_var__drvcfg_mask_on_assert = 0; 
    
    int_var__fwcfg_port_id = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_intr_csr_dhs_intr_state_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_coalesce_entry_t::clear() {

    int_var__coal_init_value0 = 0; 
    
    int_var__coal_init_value1 = 0; 
    
    int_var__coal_init_value2 = 0; 
    
    int_var__coal_init_value3 = 0; 
    
    int_var__coal_curr_value0 = 0; 
    
    int_var__coal_curr_value1 = 0; 
    
    int_var__coal_curr_value2 = 0; 
    
    int_var__coal_curr_value3 = 0; 
    
    int_var__hw_use_rsvd = 0; 
    
    int_var__hw_legacy_sent_state0 = 0; 
    
    int_var__hw_legacy_sent_state1 = 0; 
    
    int_var__hw_legacy_sent_state2 = 0; 
    
    int_var__hw_legacy_sent_state3 = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_intr_csr_dhs_intr_coalesce_t::clear() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_assert_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__rst_coalesce = 0; 
    
}

void cap_intr_csr_dhs_intr_assert_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_array_entry_t::clear() {

    int_var__vec = 0; 
    
}

void cap_intr_csr_dhs_intr_pba_array_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_cfg_entry_t::clear() {

    int_var__start = 0; 
    
    int_var__count = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_intr_csr_dhs_intr_pba_cfg_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_drvcfg_entry_t::clear() {

    int_var__coal_init = 0; 
    
    int_var__reserved1 = 0; 
    
    int_var__mask = 0; 
    
    int_var__reserved2 = 0; 
    
    int_var__int_credits = 0; 
    
    int_var__unmask = 0; 
    
    int_var__restart_coal = 0; 
    
    int_var__reserved3 = 0; 
    
    int_var__mask_on_assert = 0; 
    
    int_var__reserved4 = 0; 
    
    int_var__coal_curr = 0; 
    
    int_var__reserved5 = 0; 
    
}

void cap_intr_csr_dhs_intr_drvcfg_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_fwcfg_entry_t::clear() {

    int_var__function_mask = 0; 
    
    int_var__reserved1 = 0; 
    
    int_var__lif = 0; 
    
    int_var__port_id = 0; 
    
    int_var__local_int = 0; 
    
    int_var__legacy = 0; 
    
    int_var__intpin = 0; 
    
    int_var__reserved2 = 0; 
    
}

void cap_intr_csr_dhs_intr_fwcfg_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_msixcfg_entry_t::clear() {

    int_var__msg_addr = 0; 
    
    int_var__msg_data = 0; 
    
    int_var__vector_ctrl = 0; 
    
    int_var__reserved1 = 0; 
    
}

void cap_intr_csr_dhs_intr_msixcfg_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_intr_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_intr_csr_sta_sram_bist_t::clear() {

    int_var__done_pass = 0; 
    
    int_var__done_fail = 0; 
    
}

void cap_intr_csr_cfg_sram_bist_t::clear() {

    int_var__run = 0; 
    
}

void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::clear() {

    int_var__valid = 0; 
    
    int_var__debug_readout = 0; 
    
}

void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::clear() {

    int_var__dw0 = 0; 
    
    int_var__dw1 = 0; 
    
    int_var__dw2 = 0; 
    
    int_var__dw3 = 0; 
    
}

void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::clear() {

    int_var__source = 0; 
    
    int_var__addr = 0; 
    
}

void cap_intr_csr_cfg_debug_max_credits_limit_t::clear() {

    int_var__int_threshold = 0; 
    
}

void cap_intr_csr_cfg_debug_port_t::clear() {

    int_var__select = 0; 
    
    int_var__enable = 0; 
    
}

void cap_intr_csr_cfg_intr_axi_attr_t::clear() {

    int_var__awcache = 0; 
    
    int_var__prot = 0; 
    
    int_var__qos = 0; 
    
    int_var__lock = 0; 
    
}

void cap_intr_csr_sat_intr_event_counters_t::clear() {

    int_var__axi_bresp_errs = 0; 
    
    int_var__delayed_coal_scan = 0; 
    
}

void cap_intr_csr_cnt_intr_legacy_send_t::clear() {

    int_var__req = 0; 
    
}

void cap_intr_csr_cnt_intr_tot_axi_wr_t::clear() {

    int_var__req = 0; 
    
}

void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_intr_csr_sta_intr_ecc_intr_state_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_intr_csr_cfg_intr_ecc_disable_t::clear() {

    int_var__intr_state_cor = 0; 
    
    int_var__intr_state_det = 0; 
    
    int_var__intr_state_dhs = 0; 
    
    int_var__intr_pba_cfg_cor = 0; 
    
    int_var__intr_pba_cfg_det = 0; 
    
    int_var__intr_pba_cfg_dhs = 0; 
    
    int_var__intr_coalesce_cor = 0; 
    
    int_var__intr_coalesce_det = 0; 
    
    int_var__intr_coalesce_dhs = 0; 
    
}

void cap_intr_csr_sta_intr_debug_t::clear() {

    int_var__legacy_fifodepth = 0; 
    
    int_var__coalexp_fifodepth = 0; 
    
    int_var__wfifo_depth = 0; 
    
    int_var__awfifo_depth = 0; 
    
    int_var__wready = 0; 
    
    int_var__wvalid = 0; 
    
    int_var__awready = 0; 
    
    int_var__awvalid = 0; 
    
    int_var__axi_rdy = 0; 
    
    int_var__coal_scan_active = 0; 
    
    int_var__init_done = 0; 
    
}

void cap_intr_csr_sta_intr_init_t::clear() {

    int_var__done = 0; 
    
}

void cap_intr_csr_sta_intr_axi_t::clear() {

    int_var__num_ids = 0; 
    
}

void cap_intr_csr_cfg_intr_axi_t::clear() {

    int_var__num_ids = 0; 
    
}

void cap_intr_csr_cfg_intr_coalesce_t::clear() {

    int_var__resolution = 0; 
    
}

void cap_intr_csr_rdintr_t::clear() {

    int_var__ireg = 0; 
    
}

void cap_intr_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_intr_csr_t::clear() {

    base.clear();
    rdintr.clear();
    cfg_intr_coalesce.clear();
    cfg_intr_axi.clear();
    sta_intr_axi.clear();
    sta_intr_init.clear();
    sta_intr_debug.clear();
    cfg_intr_ecc_disable.clear();
    sta_intr_ecc_intr_state.clear();
    sta_intr_ecc_intr_pba_cfg.clear();
    sta_intr_ecc_intr_coalesce.clear();
    cnt_intr_tot_axi_wr.clear();
    cnt_intr_legacy_send.clear();
    sat_intr_event_counters.clear();
    cfg_intr_axi_attr.clear();
    cfg_debug_port.clear();
    cfg_debug_max_credits_limit.clear();
    cfg_todo_bits_legacy_cnt_read_debug.clear();
    cfg_legacy_intx_pcie_msg_hdr.clear();
    sta_todo_bits_legacy_cnt_read_debug.clear();
    cfg_sram_bist.clear();
    sta_sram_bist.clear();
    csr_intr.clear();
    dhs_intr_msixcfg.clear();
    dhs_intr_fwcfg.clear();
    dhs_intr_drvcfg.clear();
    dhs_intr_pba_cfg.clear();
    dhs_intr_pba_array.clear();
    dhs_intr_assert.clear();
    dhs_intr_coalesce.clear();
    dhs_intr_state.clear();
    int_groups.clear();
    int_intr_ecc.clear();
}

void cap_intr_csr_int_intr_ecc_int_enable_clear_t::init() {

}

void cap_intr_csr_intreg_t::init() {

}

void cap_intr_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_intr_csr_intreg_status_t::init() {

}

void cap_intr_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_intr_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_intr_csr_dhs_intr_state_entry_t::init() {

}

void cap_intr_csr_dhs_intr_state_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_coalesce_entry_t::init() {

}

void cap_intr_csr_dhs_intr_coalesce_t::init() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 1024; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_assert_entry_t::init() {

}

void cap_intr_csr_dhs_intr_assert_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_array_entry_t::init() {

}

void cap_intr_csr_dhs_intr_pba_array_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_pba_cfg_entry_t::init() {

}

void cap_intr_csr_dhs_intr_pba_cfg_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_drvcfg_entry_t::init() {

}

void cap_intr_csr_dhs_intr_drvcfg_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_fwcfg_entry_t::init() {

}

void cap_intr_csr_dhs_intr_fwcfg_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_dhs_intr_msixcfg_entry_t::init() {

}

void cap_intr_csr_dhs_intr_msixcfg_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_intr_csr_csr_intr_t::init() {

}

void cap_intr_csr_sta_sram_bist_t::init() {

}

void cap_intr_csr_cfg_sram_bist_t::init() {

}

void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::init() {

}

void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::init() {

    set_reset_val(cpp_int("0x20000000000000c4"));
    all(get_reset_val());
}

void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::init() {

}

void cap_intr_csr_cfg_debug_max_credits_limit_t::init() {

    set_reset_val(cpp_int("0x7fff"));
    all(get_reset_val());
}

void cap_intr_csr_cfg_debug_port_t::init() {

}

void cap_intr_csr_cfg_intr_axi_attr_t::init() {

    set_reset_val(cpp_int("0x20"));
    all(get_reset_val());
}

void cap_intr_csr_sat_intr_event_counters_t::init() {

}

void cap_intr_csr_cnt_intr_legacy_send_t::init() {

}

void cap_intr_csr_cnt_intr_tot_axi_wr_t::init() {

}

void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::init() {

}

void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::init() {

}

void cap_intr_csr_sta_intr_ecc_intr_state_t::init() {

}

void cap_intr_csr_cfg_intr_ecc_disable_t::init() {

}

void cap_intr_csr_sta_intr_debug_t::init() {

}

void cap_intr_csr_sta_intr_init_t::init() {

}

void cap_intr_csr_sta_intr_axi_t::init() {

}

void cap_intr_csr_cfg_intr_axi_t::init() {

    set_reset_val(cpp_int("0x10"));
    all(get_reset_val());
}

void cap_intr_csr_cfg_intr_coalesce_t::init() {

    set_reset_val(cpp_int("0x206c"));
    all(get_reset_val());
}

void cap_intr_csr_rdintr_t::init() {

}

void cap_intr_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_intr_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    rdintr.set_attributes(this,"rdintr", 0x4 );
    cfg_intr_coalesce.set_attributes(this,"cfg_intr_coalesce", 0x80000 );
    cfg_intr_axi.set_attributes(this,"cfg_intr_axi", 0x80004 );
    sta_intr_axi.set_attributes(this,"sta_intr_axi", 0x80008 );
    sta_intr_init.set_attributes(this,"sta_intr_init", 0x8000c );
    sta_intr_debug.set_attributes(this,"sta_intr_debug", 0x80010 );
    cfg_intr_ecc_disable.set_attributes(this,"cfg_intr_ecc_disable", 0x80014 );
    sta_intr_ecc_intr_state.set_attributes(this,"sta_intr_ecc_intr_state", 0x80018 );
    sta_intr_ecc_intr_pba_cfg.set_attributes(this,"sta_intr_ecc_intr_pba_cfg", 0x8001c );
    sta_intr_ecc_intr_coalesce.set_attributes(this,"sta_intr_ecc_intr_coalesce", 0x80020 );
    cnt_intr_tot_axi_wr.set_attributes(this,"cnt_intr_tot_axi_wr", 0x80028 );
    cnt_intr_legacy_send.set_attributes(this,"cnt_intr_legacy_send", 0x80030 );
    sat_intr_event_counters.set_attributes(this,"sat_intr_event_counters", 0x80038 );
    cfg_intr_axi_attr.set_attributes(this,"cfg_intr_axi_attr", 0x8003c );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x80040 );
    cfg_debug_max_credits_limit.set_attributes(this,"cfg_debug_max_credits_limit", 0x80044 );
    cfg_todo_bits_legacy_cnt_read_debug.set_attributes(this,"cfg_todo_bits_legacy_cnt_read_debug", 0x80048 );
    cfg_legacy_intx_pcie_msg_hdr.set_attributes(this,"cfg_legacy_intx_pcie_msg_hdr", 0x80050 );
    sta_todo_bits_legacy_cnt_read_debug.set_attributes(this,"sta_todo_bits_legacy_cnt_read_debug", 0x80060 );
    cfg_sram_bist.set_attributes(this,"cfg_sram_bist", 0x80064 );
    sta_sram_bist.set_attributes(this,"sta_sram_bist", 0x80068 );
    csr_intr.set_attributes(this,"csr_intr", 0x8006c );
    dhs_intr_msixcfg.set_attributes(this,"dhs_intr_msixcfg", 0x10000 );
    dhs_intr_fwcfg.set_attributes(this,"dhs_intr_fwcfg", 0x20000 );
    dhs_intr_drvcfg.set_attributes(this,"dhs_intr_drvcfg", 0x40000 );
    dhs_intr_pba_cfg.set_attributes(this,"dhs_intr_pba_cfg", 0x60000 );
    dhs_intr_pba_array.set_attributes(this,"dhs_intr_pba_array", 0x64000 );
    dhs_intr_assert.set_attributes(this,"dhs_intr_assert", 0x68000 );
    dhs_intr_coalesce.set_attributes(this,"dhs_intr_coalesce", 0x6c000 );
    dhs_intr_state.set_attributes(this,"dhs_intr_state", 0x70000 );
    int_groups.set_attributes(this,"int_groups", 0x80070 );
    int_intr_ecc.set_attributes(this,"int_intr_ecc", 0x80080 );
}

void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_state_uncorrectable_enable(const cpp_int & _val) { 
    // intr_state_uncorrectable_enable
    int_var__intr_state_uncorrectable_enable = _val.convert_to< intr_state_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_state_uncorrectable_enable() const {
    return int_var__intr_state_uncorrectable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_state_correctable_enable(const cpp_int & _val) { 
    // intr_state_correctable_enable
    int_var__intr_state_correctable_enable = _val.convert_to< intr_state_correctable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_state_correctable_enable() const {
    return int_var__intr_state_correctable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_pba_cfg_uncorrectable_enable(const cpp_int & _val) { 
    // intr_pba_cfg_uncorrectable_enable
    int_var__intr_pba_cfg_uncorrectable_enable = _val.convert_to< intr_pba_cfg_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_pba_cfg_uncorrectable_enable() const {
    return int_var__intr_pba_cfg_uncorrectable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_pba_cfg_correctable_enable(const cpp_int & _val) { 
    // intr_pba_cfg_correctable_enable
    int_var__intr_pba_cfg_correctable_enable = _val.convert_to< intr_pba_cfg_correctable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_pba_cfg_correctable_enable() const {
    return int_var__intr_pba_cfg_correctable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_coalesce_uncorrectable_enable(const cpp_int & _val) { 
    // intr_coalesce_uncorrectable_enable
    int_var__intr_coalesce_uncorrectable_enable = _val.convert_to< intr_coalesce_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_coalesce_uncorrectable_enable() const {
    return int_var__intr_coalesce_uncorrectable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_coalesce_correctable_enable(const cpp_int & _val) { 
    // intr_coalesce_correctable_enable
    int_var__intr_coalesce_correctable_enable = _val.convert_to< intr_coalesce_correctable_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_coalesce_correctable_enable() const {
    return int_var__intr_coalesce_correctable_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_legacy_fifo_overrun_enable(const cpp_int & _val) { 
    // intr_legacy_fifo_overrun_enable
    int_var__intr_legacy_fifo_overrun_enable = _val.convert_to< intr_legacy_fifo_overrun_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_legacy_fifo_overrun_enable() const {
    return int_var__intr_legacy_fifo_overrun_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::int_credit_positive_after_return_enable(const cpp_int & _val) { 
    // int_credit_positive_after_return_enable
    int_var__int_credit_positive_after_return_enable = _val.convert_to< int_credit_positive_after_return_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::int_credit_positive_after_return_enable() const {
    return int_var__int_credit_positive_after_return_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::int_credits_max_debug_threshold_enable(const cpp_int & _val) { 
    // int_credits_max_debug_threshold_enable
    int_var__int_credits_max_debug_threshold_enable = _val.convert_to< int_credits_max_debug_threshold_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::int_credits_max_debug_threshold_enable() const {
    return int_var__int_credits_max_debug_threshold_enable;
}
    
void cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_bresp_err_enable(const cpp_int & _val) { 
    // intr_bresp_err_enable
    int_var__intr_bresp_err_enable = _val.convert_to< intr_bresp_err_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_intr_ecc_int_enable_clear_t::intr_bresp_err_enable() const {
    return int_var__intr_bresp_err_enable;
}
    
void cap_intr_csr_intreg_t::intr_state_uncorrectable_interrupt(const cpp_int & _val) { 
    // intr_state_uncorrectable_interrupt
    int_var__intr_state_uncorrectable_interrupt = _val.convert_to< intr_state_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_state_uncorrectable_interrupt() const {
    return int_var__intr_state_uncorrectable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_state_correctable_interrupt(const cpp_int & _val) { 
    // intr_state_correctable_interrupt
    int_var__intr_state_correctable_interrupt = _val.convert_to< intr_state_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_state_correctable_interrupt() const {
    return int_var__intr_state_correctable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_pba_cfg_uncorrectable_interrupt(const cpp_int & _val) { 
    // intr_pba_cfg_uncorrectable_interrupt
    int_var__intr_pba_cfg_uncorrectable_interrupt = _val.convert_to< intr_pba_cfg_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_pba_cfg_uncorrectable_interrupt() const {
    return int_var__intr_pba_cfg_uncorrectable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_pba_cfg_correctable_interrupt(const cpp_int & _val) { 
    // intr_pba_cfg_correctable_interrupt
    int_var__intr_pba_cfg_correctable_interrupt = _val.convert_to< intr_pba_cfg_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_pba_cfg_correctable_interrupt() const {
    return int_var__intr_pba_cfg_correctable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_coalesce_uncorrectable_interrupt(const cpp_int & _val) { 
    // intr_coalesce_uncorrectable_interrupt
    int_var__intr_coalesce_uncorrectable_interrupt = _val.convert_to< intr_coalesce_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_coalesce_uncorrectable_interrupt() const {
    return int_var__intr_coalesce_uncorrectable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_coalesce_correctable_interrupt(const cpp_int & _val) { 
    // intr_coalesce_correctable_interrupt
    int_var__intr_coalesce_correctable_interrupt = _val.convert_to< intr_coalesce_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_coalesce_correctable_interrupt() const {
    return int_var__intr_coalesce_correctable_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_legacy_fifo_overrun_interrupt(const cpp_int & _val) { 
    // intr_legacy_fifo_overrun_interrupt
    int_var__intr_legacy_fifo_overrun_interrupt = _val.convert_to< intr_legacy_fifo_overrun_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_legacy_fifo_overrun_interrupt() const {
    return int_var__intr_legacy_fifo_overrun_interrupt;
}
    
void cap_intr_csr_intreg_t::int_credit_positive_after_return_interrupt(const cpp_int & _val) { 
    // int_credit_positive_after_return_interrupt
    int_var__int_credit_positive_after_return_interrupt = _val.convert_to< int_credit_positive_after_return_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::int_credit_positive_after_return_interrupt() const {
    return int_var__int_credit_positive_after_return_interrupt;
}
    
void cap_intr_csr_intreg_t::int_credits_max_debug_threshold_interrupt(const cpp_int & _val) { 
    // int_credits_max_debug_threshold_interrupt
    int_var__int_credits_max_debug_threshold_interrupt = _val.convert_to< int_credits_max_debug_threshold_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::int_credits_max_debug_threshold_interrupt() const {
    return int_var__int_credits_max_debug_threshold_interrupt;
}
    
void cap_intr_csr_intreg_t::intr_bresp_err_interrupt(const cpp_int & _val) { 
    // intr_bresp_err_interrupt
    int_var__intr_bresp_err_interrupt = _val.convert_to< intr_bresp_err_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_t::intr_bresp_err_interrupt() const {
    return int_var__intr_bresp_err_interrupt;
}
    
void cap_intr_csr_intreg_status_t::int_intr_ecc_interrupt(const cpp_int & _val) { 
    // int_intr_ecc_interrupt
    int_var__int_intr_ecc_interrupt = _val.convert_to< int_intr_ecc_interrupt_cpp_int_t >();
}

cpp_int cap_intr_csr_intreg_status_t::int_intr_ecc_interrupt() const {
    return int_var__int_intr_ecc_interrupt;
}
    
void cap_intr_csr_int_groups_int_enable_rw_reg_t::int_intr_ecc_enable(const cpp_int & _val) { 
    // int_intr_ecc_enable
    int_var__int_intr_ecc_enable = _val.convert_to< int_intr_ecc_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_int_groups_int_enable_rw_reg_t::int_intr_ecc_enable() const {
    return int_var__int_intr_ecc_enable;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::msixcfg_msg_addr_51_2(const cpp_int & _val) { 
    // msixcfg_msg_addr_51_2
    int_var__msixcfg_msg_addr_51_2 = _val.convert_to< msixcfg_msg_addr_51_2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::msixcfg_msg_addr_51_2() const {
    return int_var__msixcfg_msg_addr_51_2;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::msixcfg_msg_data(const cpp_int & _val) { 
    // msixcfg_msg_data
    int_var__msixcfg_msg_data = _val.convert_to< msixcfg_msg_data_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::msixcfg_msg_data() const {
    return int_var__msixcfg_msg_data;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::msixcfg_vector_ctrl(const cpp_int & _val) { 
    // msixcfg_vector_ctrl
    int_var__msixcfg_vector_ctrl = _val.convert_to< msixcfg_vector_ctrl_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::msixcfg_vector_ctrl() const {
    return int_var__msixcfg_vector_ctrl;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_function_mask(const cpp_int & _val) { 
    // fwcfg_function_mask
    int_var__fwcfg_function_mask = _val.convert_to< fwcfg_function_mask_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_function_mask() const {
    return int_var__fwcfg_function_mask;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_lif(const cpp_int & _val) { 
    // fwcfg_lif
    int_var__fwcfg_lif = _val.convert_to< fwcfg_lif_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_lif() const {
    return int_var__fwcfg_lif;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_local_int(const cpp_int & _val) { 
    // fwcfg_local_int
    int_var__fwcfg_local_int = _val.convert_to< fwcfg_local_int_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_local_int() const {
    return int_var__fwcfg_local_int;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_legacy_int(const cpp_int & _val) { 
    // fwcfg_legacy_int
    int_var__fwcfg_legacy_int = _val.convert_to< fwcfg_legacy_int_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_legacy_int() const {
    return int_var__fwcfg_legacy_int;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_legacy_pin(const cpp_int & _val) { 
    // fwcfg_legacy_pin
    int_var__fwcfg_legacy_pin = _val.convert_to< fwcfg_legacy_pin_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_legacy_pin() const {
    return int_var__fwcfg_legacy_pin;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::drvcfg_mask(const cpp_int & _val) { 
    // drvcfg_mask
    int_var__drvcfg_mask = _val.convert_to< drvcfg_mask_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::drvcfg_mask() const {
    return int_var__drvcfg_mask;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::drvcfg_int_credits(const cpp_int & _val) { 
    // drvcfg_int_credits
    int_var__drvcfg_int_credits = _val.convert_to< drvcfg_int_credits_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::drvcfg_int_credits() const {
    return int_var__drvcfg_int_credits;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::drvcfg_mask_on_assert(const cpp_int & _val) { 
    // drvcfg_mask_on_assert
    int_var__drvcfg_mask_on_assert = _val.convert_to< drvcfg_mask_on_assert_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::drvcfg_mask_on_assert() const {
    return int_var__drvcfg_mask_on_assert;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::fwcfg_port_id(const cpp_int & _val) { 
    // fwcfg_port_id
    int_var__fwcfg_port_id = _val.convert_to< fwcfg_port_id_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::fwcfg_port_id() const {
    return int_var__fwcfg_port_id;
}
    
void cap_intr_csr_dhs_intr_state_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_state_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value0(const cpp_int & _val) { 
    // coal_init_value0
    int_var__coal_init_value0 = _val.convert_to< coal_init_value0_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value0() const {
    return int_var__coal_init_value0;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value1(const cpp_int & _val) { 
    // coal_init_value1
    int_var__coal_init_value1 = _val.convert_to< coal_init_value1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value1() const {
    return int_var__coal_init_value1;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value2(const cpp_int & _val) { 
    // coal_init_value2
    int_var__coal_init_value2 = _val.convert_to< coal_init_value2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value2() const {
    return int_var__coal_init_value2;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value3(const cpp_int & _val) { 
    // coal_init_value3
    int_var__coal_init_value3 = _val.convert_to< coal_init_value3_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_init_value3() const {
    return int_var__coal_init_value3;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value0(const cpp_int & _val) { 
    // coal_curr_value0
    int_var__coal_curr_value0 = _val.convert_to< coal_curr_value0_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value0() const {
    return int_var__coal_curr_value0;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value1(const cpp_int & _val) { 
    // coal_curr_value1
    int_var__coal_curr_value1 = _val.convert_to< coal_curr_value1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value1() const {
    return int_var__coal_curr_value1;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value2(const cpp_int & _val) { 
    // coal_curr_value2
    int_var__coal_curr_value2 = _val.convert_to< coal_curr_value2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value2() const {
    return int_var__coal_curr_value2;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value3(const cpp_int & _val) { 
    // coal_curr_value3
    int_var__coal_curr_value3 = _val.convert_to< coal_curr_value3_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::coal_curr_value3() const {
    return int_var__coal_curr_value3;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::hw_use_rsvd(const cpp_int & _val) { 
    // hw_use_rsvd
    int_var__hw_use_rsvd = _val.convert_to< hw_use_rsvd_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::hw_use_rsvd() const {
    return int_var__hw_use_rsvd;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state0(const cpp_int & _val) { 
    // hw_legacy_sent_state0
    int_var__hw_legacy_sent_state0 = _val.convert_to< hw_legacy_sent_state0_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state0() const {
    return int_var__hw_legacy_sent_state0;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state1(const cpp_int & _val) { 
    // hw_legacy_sent_state1
    int_var__hw_legacy_sent_state1 = _val.convert_to< hw_legacy_sent_state1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state1() const {
    return int_var__hw_legacy_sent_state1;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state2(const cpp_int & _val) { 
    // hw_legacy_sent_state2
    int_var__hw_legacy_sent_state2 = _val.convert_to< hw_legacy_sent_state2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state2() const {
    return int_var__hw_legacy_sent_state2;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state3(const cpp_int & _val) { 
    // hw_legacy_sent_state3
    int_var__hw_legacy_sent_state3 = _val.convert_to< hw_legacy_sent_state3_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::hw_legacy_sent_state3() const {
    return int_var__hw_legacy_sent_state3;
}
    
void cap_intr_csr_dhs_intr_coalesce_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_coalesce_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_intr_csr_dhs_intr_assert_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_assert_entry_t::data() const {
    return int_var__data;
}
    
void cap_intr_csr_dhs_intr_assert_entry_t::rst_coalesce(const cpp_int & _val) { 
    // rst_coalesce
    int_var__rst_coalesce = _val.convert_to< rst_coalesce_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_assert_entry_t::rst_coalesce() const {
    return int_var__rst_coalesce;
}
    
void cap_intr_csr_dhs_intr_pba_array_entry_t::vec(const cpp_int & _val) { 
    // vec
    int_var__vec = _val.convert_to< vec_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_pba_array_entry_t::vec() const {
    return int_var__vec;
}
    
void cap_intr_csr_dhs_intr_pba_cfg_entry_t::start(const cpp_int & _val) { 
    // start
    int_var__start = _val.convert_to< start_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_pba_cfg_entry_t::start() const {
    return int_var__start;
}
    
void cap_intr_csr_dhs_intr_pba_cfg_entry_t::count(const cpp_int & _val) { 
    // count
    int_var__count = _val.convert_to< count_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_pba_cfg_entry_t::count() const {
    return int_var__count;
}
    
void cap_intr_csr_dhs_intr_pba_cfg_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_pba_cfg_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::coal_init(const cpp_int & _val) { 
    // coal_init
    int_var__coal_init = _val.convert_to< coal_init_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::coal_init() const {
    return int_var__coal_init;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved1(const cpp_int & _val) { 
    // reserved1
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved1() const {
    return int_var__reserved1;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::mask(const cpp_int & _val) { 
    // mask
    int_var__mask = _val.convert_to< mask_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::mask() const {
    return int_var__mask;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved2(const cpp_int & _val) { 
    // reserved2
    int_var__reserved2 = _val.convert_to< reserved2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved2() const {
    return int_var__reserved2;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::int_credits(const cpp_int & _val) { 
    // int_credits
    int_var__int_credits = _val.convert_to< int_credits_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::int_credits() const {
    return int_var__int_credits;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::unmask(const cpp_int & _val) { 
    // unmask
    int_var__unmask = _val.convert_to< unmask_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::unmask() const {
    return int_var__unmask;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::restart_coal(const cpp_int & _val) { 
    // restart_coal
    int_var__restart_coal = _val.convert_to< restart_coal_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::restart_coal() const {
    return int_var__restart_coal;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved3(const cpp_int & _val) { 
    // reserved3
    int_var__reserved3 = _val.convert_to< reserved3_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved3() const {
    return int_var__reserved3;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::mask_on_assert(const cpp_int & _val) { 
    // mask_on_assert
    int_var__mask_on_assert = _val.convert_to< mask_on_assert_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::mask_on_assert() const {
    return int_var__mask_on_assert;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved4(const cpp_int & _val) { 
    // reserved4
    int_var__reserved4 = _val.convert_to< reserved4_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved4() const {
    return int_var__reserved4;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::coal_curr(const cpp_int & _val) { 
    // coal_curr
    int_var__coal_curr = _val.convert_to< coal_curr_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::coal_curr() const {
    return int_var__coal_curr;
}
    
void cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved5(const cpp_int & _val) { 
    // reserved5
    int_var__reserved5 = _val.convert_to< reserved5_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_drvcfg_entry_t::reserved5() const {
    return int_var__reserved5;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::function_mask(const cpp_int & _val) { 
    // function_mask
    int_var__function_mask = _val.convert_to< function_mask_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::function_mask() const {
    return int_var__function_mask;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::reserved1(const cpp_int & _val) { 
    // reserved1
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::reserved1() const {
    return int_var__reserved1;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::lif() const {
    return int_var__lif;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::port_id(const cpp_int & _val) { 
    // port_id
    int_var__port_id = _val.convert_to< port_id_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::port_id() const {
    return int_var__port_id;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::local_int(const cpp_int & _val) { 
    // local_int
    int_var__local_int = _val.convert_to< local_int_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::local_int() const {
    return int_var__local_int;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::legacy(const cpp_int & _val) { 
    // legacy
    int_var__legacy = _val.convert_to< legacy_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::legacy() const {
    return int_var__legacy;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::intpin(const cpp_int & _val) { 
    // intpin
    int_var__intpin = _val.convert_to< intpin_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::intpin() const {
    return int_var__intpin;
}
    
void cap_intr_csr_dhs_intr_fwcfg_entry_t::reserved2(const cpp_int & _val) { 
    // reserved2
    int_var__reserved2 = _val.convert_to< reserved2_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_fwcfg_entry_t::reserved2() const {
    return int_var__reserved2;
}
    
void cap_intr_csr_dhs_intr_msixcfg_entry_t::msg_addr(const cpp_int & _val) { 
    // msg_addr
    int_var__msg_addr = _val.convert_to< msg_addr_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_entry_t::msg_addr() const {
    return int_var__msg_addr;
}
    
void cap_intr_csr_dhs_intr_msixcfg_entry_t::msg_data(const cpp_int & _val) { 
    // msg_data
    int_var__msg_data = _val.convert_to< msg_data_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_entry_t::msg_data() const {
    return int_var__msg_data;
}
    
void cap_intr_csr_dhs_intr_msixcfg_entry_t::vector_ctrl(const cpp_int & _val) { 
    // vector_ctrl
    int_var__vector_ctrl = _val.convert_to< vector_ctrl_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_entry_t::vector_ctrl() const {
    return int_var__vector_ctrl;
}
    
void cap_intr_csr_dhs_intr_msixcfg_entry_t::reserved1(const cpp_int & _val) { 
    // reserved1
    int_var__reserved1 = _val.convert_to< reserved1_cpp_int_t >();
}

cpp_int cap_intr_csr_dhs_intr_msixcfg_entry_t::reserved1() const {
    return int_var__reserved1;
}
    
void cap_intr_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_intr_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_intr_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_intr_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_intr_csr_sta_sram_bist_t::done_pass(const cpp_int & _val) { 
    // done_pass
    int_var__done_pass = _val.convert_to< done_pass_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_sram_bist_t::done_pass() const {
    return int_var__done_pass;
}
    
void cap_intr_csr_sta_sram_bist_t::done_fail(const cpp_int & _val) { 
    // done_fail
    int_var__done_fail = _val.convert_to< done_fail_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_sram_bist_t::done_fail() const {
    return int_var__done_fail;
}
    
void cap_intr_csr_cfg_sram_bist_t::run(const cpp_int & _val) { 
    // run
    int_var__run = _val.convert_to< run_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_sram_bist_t::run() const {
    return int_var__run;
}
    
void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::valid() const {
    return int_var__valid;
}
    
void cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::debug_readout(const cpp_int & _val) { 
    // debug_readout
    int_var__debug_readout = _val.convert_to< debug_readout_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::debug_readout() const {
    return int_var__debug_readout;
}
    
void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw0(const cpp_int & _val) { 
    // dw0
    int_var__dw0 = _val.convert_to< dw0_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw0() const {
    return int_var__dw0;
}
    
void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw1(const cpp_int & _val) { 
    // dw1
    int_var__dw1 = _val.convert_to< dw1_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw1() const {
    return int_var__dw1;
}
    
void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw2(const cpp_int & _val) { 
    // dw2
    int_var__dw2 = _val.convert_to< dw2_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw2() const {
    return int_var__dw2;
}
    
void cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw3(const cpp_int & _val) { 
    // dw3
    int_var__dw3 = _val.convert_to< dw3_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::dw3() const {
    return int_var__dw3;
}
    
void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::source(const cpp_int & _val) { 
    // source
    int_var__source = _val.convert_to< source_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::source() const {
    return int_var__source;
}
    
void cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::addr() const {
    return int_var__addr;
}
    
void cap_intr_csr_cfg_debug_max_credits_limit_t::int_threshold(const cpp_int & _val) { 
    // int_threshold
    int_var__int_threshold = _val.convert_to< int_threshold_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_debug_max_credits_limit_t::int_threshold() const {
    return int_var__int_threshold;
}
    
void cap_intr_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_debug_port_t::select() const {
    return int_var__select;
}
    
void cap_intr_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_debug_port_t::enable() const {
    return int_var__enable;
}
    
void cap_intr_csr_cfg_intr_axi_attr_t::awcache(const cpp_int & _val) { 
    // awcache
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_axi_attr_t::awcache() const {
    return int_var__awcache;
}
    
void cap_intr_csr_cfg_intr_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_axi_attr_t::prot() const {
    return int_var__prot;
}
    
void cap_intr_csr_cfg_intr_axi_attr_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_axi_attr_t::qos() const {
    return int_var__qos;
}
    
void cap_intr_csr_cfg_intr_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_axi_attr_t::lock() const {
    return int_var__lock;
}
    
void cap_intr_csr_sat_intr_event_counters_t::axi_bresp_errs(const cpp_int & _val) { 
    // axi_bresp_errs
    int_var__axi_bresp_errs = _val.convert_to< axi_bresp_errs_cpp_int_t >();
}

cpp_int cap_intr_csr_sat_intr_event_counters_t::axi_bresp_errs() const {
    return int_var__axi_bresp_errs;
}
    
void cap_intr_csr_sat_intr_event_counters_t::delayed_coal_scan(const cpp_int & _val) { 
    // delayed_coal_scan
    int_var__delayed_coal_scan = _val.convert_to< delayed_coal_scan_cpp_int_t >();
}

cpp_int cap_intr_csr_sat_intr_event_counters_t::delayed_coal_scan() const {
    return int_var__delayed_coal_scan;
}
    
void cap_intr_csr_cnt_intr_legacy_send_t::req(const cpp_int & _val) { 
    // req
    int_var__req = _val.convert_to< req_cpp_int_t >();
}

cpp_int cap_intr_csr_cnt_intr_legacy_send_t::req() const {
    return int_var__req;
}
    
void cap_intr_csr_cnt_intr_tot_axi_wr_t::req(const cpp_int & _val) { 
    // req
    int_var__req = _val.convert_to< req_cpp_int_t >();
}

cpp_int cap_intr_csr_cnt_intr_tot_axi_wr_t::req() const {
    return int_var__req;
}
    
void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::correctable() const {
    return int_var__correctable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_intr_csr_sta_intr_ecc_intr_coalesce_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_coalesce_t::addr() const {
    return int_var__addr;
}
    
void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::correctable() const {
    return int_var__correctable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::addr() const {
    return int_var__addr;
}
    
void cap_intr_csr_sta_intr_ecc_intr_state_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_state_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_state_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_state_t::correctable() const {
    return int_var__correctable;
}
    
void cap_intr_csr_sta_intr_ecc_intr_state_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_state_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_intr_csr_sta_intr_ecc_intr_state_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_ecc_intr_state_t::addr() const {
    return int_var__addr;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_cor(const cpp_int & _val) { 
    // intr_state_cor
    int_var__intr_state_cor = _val.convert_to< intr_state_cor_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_cor() const {
    return int_var__intr_state_cor;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_det(const cpp_int & _val) { 
    // intr_state_det
    int_var__intr_state_det = _val.convert_to< intr_state_det_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_det() const {
    return int_var__intr_state_det;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_dhs(const cpp_int & _val) { 
    // intr_state_dhs
    int_var__intr_state_dhs = _val.convert_to< intr_state_dhs_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_state_dhs() const {
    return int_var__intr_state_dhs;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_cor(const cpp_int & _val) { 
    // intr_pba_cfg_cor
    int_var__intr_pba_cfg_cor = _val.convert_to< intr_pba_cfg_cor_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_cor() const {
    return int_var__intr_pba_cfg_cor;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_det(const cpp_int & _val) { 
    // intr_pba_cfg_det
    int_var__intr_pba_cfg_det = _val.convert_to< intr_pba_cfg_det_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_det() const {
    return int_var__intr_pba_cfg_det;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_dhs(const cpp_int & _val) { 
    // intr_pba_cfg_dhs
    int_var__intr_pba_cfg_dhs = _val.convert_to< intr_pba_cfg_dhs_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_pba_cfg_dhs() const {
    return int_var__intr_pba_cfg_dhs;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_cor(const cpp_int & _val) { 
    // intr_coalesce_cor
    int_var__intr_coalesce_cor = _val.convert_to< intr_coalesce_cor_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_cor() const {
    return int_var__intr_coalesce_cor;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_det(const cpp_int & _val) { 
    // intr_coalesce_det
    int_var__intr_coalesce_det = _val.convert_to< intr_coalesce_det_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_det() const {
    return int_var__intr_coalesce_det;
}
    
void cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_dhs(const cpp_int & _val) { 
    // intr_coalesce_dhs
    int_var__intr_coalesce_dhs = _val.convert_to< intr_coalesce_dhs_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_ecc_disable_t::intr_coalesce_dhs() const {
    return int_var__intr_coalesce_dhs;
}
    
void cap_intr_csr_sta_intr_debug_t::legacy_fifodepth(const cpp_int & _val) { 
    // legacy_fifodepth
    int_var__legacy_fifodepth = _val.convert_to< legacy_fifodepth_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::legacy_fifodepth() const {
    return int_var__legacy_fifodepth;
}
    
void cap_intr_csr_sta_intr_debug_t::coalexp_fifodepth(const cpp_int & _val) { 
    // coalexp_fifodepth
    int_var__coalexp_fifodepth = _val.convert_to< coalexp_fifodepth_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::coalexp_fifodepth() const {
    return int_var__coalexp_fifodepth;
}
    
void cap_intr_csr_sta_intr_debug_t::wfifo_depth(const cpp_int & _val) { 
    // wfifo_depth
    int_var__wfifo_depth = _val.convert_to< wfifo_depth_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::wfifo_depth() const {
    return int_var__wfifo_depth;
}
    
void cap_intr_csr_sta_intr_debug_t::awfifo_depth(const cpp_int & _val) { 
    // awfifo_depth
    int_var__awfifo_depth = _val.convert_to< awfifo_depth_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::awfifo_depth() const {
    return int_var__awfifo_depth;
}
    
void cap_intr_csr_sta_intr_debug_t::wready(const cpp_int & _val) { 
    // wready
    int_var__wready = _val.convert_to< wready_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::wready() const {
    return int_var__wready;
}
    
void cap_intr_csr_sta_intr_debug_t::wvalid(const cpp_int & _val) { 
    // wvalid
    int_var__wvalid = _val.convert_to< wvalid_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::wvalid() const {
    return int_var__wvalid;
}
    
void cap_intr_csr_sta_intr_debug_t::awready(const cpp_int & _val) { 
    // awready
    int_var__awready = _val.convert_to< awready_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::awready() const {
    return int_var__awready;
}
    
void cap_intr_csr_sta_intr_debug_t::awvalid(const cpp_int & _val) { 
    // awvalid
    int_var__awvalid = _val.convert_to< awvalid_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::awvalid() const {
    return int_var__awvalid;
}
    
void cap_intr_csr_sta_intr_debug_t::axi_rdy(const cpp_int & _val) { 
    // axi_rdy
    int_var__axi_rdy = _val.convert_to< axi_rdy_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::axi_rdy() const {
    return int_var__axi_rdy;
}
    
void cap_intr_csr_sta_intr_debug_t::coal_scan_active(const cpp_int & _val) { 
    // coal_scan_active
    int_var__coal_scan_active = _val.convert_to< coal_scan_active_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::coal_scan_active() const {
    return int_var__coal_scan_active;
}
    
void cap_intr_csr_sta_intr_debug_t::init_done(const cpp_int & _val) { 
    // init_done
    int_var__init_done = _val.convert_to< init_done_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_debug_t::init_done() const {
    return int_var__init_done;
}
    
void cap_intr_csr_sta_intr_init_t::done(const cpp_int & _val) { 
    // done
    int_var__done = _val.convert_to< done_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_init_t::done() const {
    return int_var__done;
}
    
void cap_intr_csr_sta_intr_axi_t::num_ids(const cpp_int & _val) { 
    // num_ids
    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >();
}

cpp_int cap_intr_csr_sta_intr_axi_t::num_ids() const {
    return int_var__num_ids;
}
    
void cap_intr_csr_cfg_intr_axi_t::num_ids(const cpp_int & _val) { 
    // num_ids
    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_axi_t::num_ids() const {
    return int_var__num_ids;
}
    
void cap_intr_csr_cfg_intr_coalesce_t::resolution(const cpp_int & _val) { 
    // resolution
    int_var__resolution = _val.convert_to< resolution_cpp_int_t >();
}

cpp_int cap_intr_csr_cfg_intr_coalesce_t::resolution() const {
    return int_var__resolution;
}
    
void cap_intr_csr_rdintr_t::ireg(const cpp_int & _val) { 
    // ireg
    int_var__ireg = _val.convert_to< ireg_cpp_int_t >();
}

cpp_int cap_intr_csr_rdintr_t::ireg() const {
    return int_var__ireg;
}
    
void cap_intr_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_intr_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_int_intr_ecc_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_uncorrectable_enable")) { field_val = intr_state_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_correctable_enable")) { field_val = intr_state_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_uncorrectable_enable")) { field_val = intr_pba_cfg_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_correctable_enable")) { field_val = intr_pba_cfg_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_uncorrectable_enable")) { field_val = intr_coalesce_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_correctable_enable")) { field_val = intr_coalesce_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_legacy_fifo_overrun_enable")) { field_val = intr_legacy_fifo_overrun_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credit_positive_after_return_enable")) { field_val = int_credit_positive_after_return_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits_max_debug_threshold_enable")) { field_val = int_credits_max_debug_threshold_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_bresp_err_enable")) { field_val = intr_bresp_err_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_uncorrectable_interrupt")) { field_val = intr_state_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_correctable_interrupt")) { field_val = intr_state_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_uncorrectable_interrupt")) { field_val = intr_pba_cfg_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_correctable_interrupt")) { field_val = intr_pba_cfg_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_uncorrectable_interrupt")) { field_val = intr_coalesce_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_correctable_interrupt")) { field_val = intr_coalesce_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_legacy_fifo_overrun_interrupt")) { field_val = intr_legacy_fifo_overrun_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credit_positive_after_return_interrupt")) { field_val = int_credit_positive_after_return_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits_max_debug_threshold_interrupt")) { field_val = int_credits_max_debug_threshold_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_bresp_err_interrupt")) { field_val = intr_bresp_err_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_intr_ecc_interrupt")) { field_val = int_intr_ecc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_intr_ecc_enable")) { field_val = int_intr_ecc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_dhs_intr_state_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msixcfg_msg_addr_51_2")) { field_val = msixcfg_msg_addr_51_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "msixcfg_msg_data")) { field_val = msixcfg_msg_data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "msixcfg_vector_ctrl")) { field_val = msixcfg_vector_ctrl(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_function_mask")) { field_val = fwcfg_function_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_lif")) { field_val = fwcfg_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_local_int")) { field_val = fwcfg_local_int(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_legacy_int")) { field_val = fwcfg_legacy_int(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_legacy_pin")) { field_val = fwcfg_legacy_pin(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_mask")) { field_val = drvcfg_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_int_credits")) { field_val = drvcfg_int_credits(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_mask_on_assert")) { field_val = drvcfg_mask_on_assert(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_port_id")) { field_val = fwcfg_port_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_state_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_coalesce_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "coal_init_value0")) { field_val = coal_init_value0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value1")) { field_val = coal_init_value1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value2")) { field_val = coal_init_value2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value3")) { field_val = coal_init_value3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value0")) { field_val = coal_curr_value0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value1")) { field_val = coal_curr_value1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value2")) { field_val = coal_curr_value2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value3")) { field_val = coal_curr_value3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_use_rsvd")) { field_val = hw_use_rsvd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state0")) { field_val = hw_legacy_sent_state0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state1")) { field_val = hw_legacy_sent_state1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state2")) { field_val = hw_legacy_sent_state2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state3")) { field_val = hw_legacy_sent_state3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_coalesce_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_assert_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rst_coalesce")) { field_val = rst_coalesce(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_assert_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_array_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vec")) { field_val = vec(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_array_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_cfg_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start")) { field_val = start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "count")) { field_val = count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_drvcfg_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "coal_init")) { field_val = coal_init(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { field_val = reserved1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { field_val = mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved2")) { field_val = reserved2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits")) { field_val = int_credits(); field_found=1; }
    if(!field_found && !strcmp(field_name, "unmask")) { field_val = unmask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "restart_coal")) { field_val = restart_coal(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved3")) { field_val = reserved3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_on_assert")) { field_val = mask_on_assert(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved4")) { field_val = reserved4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr")) { field_val = coal_curr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved5")) { field_val = reserved5(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_drvcfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_fwcfg_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "function_mask")) { field_val = function_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { field_val = reserved1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "port_id")) { field_val = port_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "local_int")) { field_val = local_int(); field_found=1; }
    if(!field_found && !strcmp(field_name, "legacy")) { field_val = legacy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intpin")) { field_val = intpin(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved2")) { field_val = reserved2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_fwcfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_msixcfg_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msg_addr")) { field_val = msg_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "msg_data")) { field_val = msg_data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "vector_ctrl")) { field_val = vector_ctrl(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { field_val = reserved1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_msixcfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_sram_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_pass")) { field_val = done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail")) { field_val = done_fail(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_sram_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { field_val = run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_readout")) { field_val = debug_readout(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dw0")) { field_val = dw0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw1")) { field_val = dw1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw2")) { field_val = dw2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw3")) { field_val = dw3(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "source")) { field_val = source(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_debug_max_credits_limit_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_threshold")) { field_val = int_threshold(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_debug_port_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "select")) { field_val = select(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_axi_attr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "awcache")) { field_val = awcache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sat_intr_event_counters_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_bresp_errs")) { field_val = axi_bresp_errs(); field_found=1; }
    if(!field_found && !strcmp(field_name, "delayed_coal_scan")) { field_val = delayed_coal_scan(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cnt_intr_legacy_send_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "req")) { field_val = req(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cnt_intr_tot_axi_wr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "req")) { field_val = req(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_ecc_intr_coalesce_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_sta_intr_ecc_intr_state_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_cfg_intr_ecc_disable_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_cor")) { field_val = intr_state_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_det")) { field_val = intr_state_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_dhs")) { field_val = intr_state_dhs(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_cor")) { field_val = intr_pba_cfg_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_det")) { field_val = intr_pba_cfg_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_dhs")) { field_val = intr_pba_cfg_dhs(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_cor")) { field_val = intr_coalesce_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_det")) { field_val = intr_coalesce_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_dhs")) { field_val = intr_coalesce_dhs(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_debug_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "legacy_fifodepth")) { field_val = legacy_fifodepth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coalexp_fifodepth")) { field_val = coalexp_fifodepth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wfifo_depth")) { field_val = wfifo_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awfifo_depth")) { field_val = awfifo_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wready")) { field_val = wready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wvalid")) { field_val = wvalid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awready")) { field_val = awready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awvalid")) { field_val = awvalid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdy")) { field_val = axi_rdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_scan_active")) { field_val = coal_scan_active(); field_found=1; }
    if(!field_found && !strcmp(field_name, "init_done")) { field_val = init_done(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_init_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done")) { field_val = done(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_axi_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "num_ids")) { field_val = num_ids(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_axi_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "num_ids")) { field_val = num_ids(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_coalesce_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "resolution")) { field_val = resolution(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_rdintr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ireg")) { field_val = ireg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = rdintr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_coalesce.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_axi.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_axi.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_init.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_debug.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_ecc_disable.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_state.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_pba_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_coalesce.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_intr_tot_axi_wr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_intr_legacy_send.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_intr_event_counters.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_axi_attr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_max_credits_limit.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_todo_bits_legacy_cnt_read_debug.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_legacy_intx_pcie_msg_hdr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_todo_bits_legacy_cnt_read_debug.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sram_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sram_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_msixcfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_fwcfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_drvcfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_pba_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_pba_array.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_assert.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_coalesce.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_state.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_intr_ecc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_int_intr_ecc_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_uncorrectable_enable")) { intr_state_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_correctable_enable")) { intr_state_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_uncorrectable_enable")) { intr_pba_cfg_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_correctable_enable")) { intr_pba_cfg_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_uncorrectable_enable")) { intr_coalesce_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_correctable_enable")) { intr_coalesce_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_legacy_fifo_overrun_enable")) { intr_legacy_fifo_overrun_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credit_positive_after_return_enable")) { int_credit_positive_after_return_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits_max_debug_threshold_enable")) { int_credits_max_debug_threshold_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_bresp_err_enable")) { intr_bresp_err_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_uncorrectable_interrupt")) { intr_state_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_correctable_interrupt")) { intr_state_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_uncorrectable_interrupt")) { intr_pba_cfg_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_correctable_interrupt")) { intr_pba_cfg_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_uncorrectable_interrupt")) { intr_coalesce_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_correctable_interrupt")) { intr_coalesce_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_legacy_fifo_overrun_interrupt")) { intr_legacy_fifo_overrun_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credit_positive_after_return_interrupt")) { int_credit_positive_after_return_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits_max_debug_threshold_interrupt")) { int_credits_max_debug_threshold_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_bresp_err_interrupt")) { intr_bresp_err_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_intr_ecc_interrupt")) { int_intr_ecc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_intr_ecc_enable")) { int_intr_ecc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_dhs_intr_state_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msixcfg_msg_addr_51_2")) { msixcfg_msg_addr_51_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "msixcfg_msg_data")) { msixcfg_msg_data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "msixcfg_vector_ctrl")) { msixcfg_vector_ctrl(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_function_mask")) { fwcfg_function_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_lif")) { fwcfg_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_local_int")) { fwcfg_local_int(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_legacy_int")) { fwcfg_legacy_int(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_legacy_pin")) { fwcfg_legacy_pin(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_mask")) { drvcfg_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_int_credits")) { drvcfg_int_credits(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drvcfg_mask_on_assert")) { drvcfg_mask_on_assert(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fwcfg_port_id")) { fwcfg_port_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_state_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_coalesce_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "coal_init_value0")) { coal_init_value0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value1")) { coal_init_value1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value2")) { coal_init_value2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_init_value3")) { coal_init_value3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value0")) { coal_curr_value0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value1")) { coal_curr_value1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value2")) { coal_curr_value2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr_value3")) { coal_curr_value3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_use_rsvd")) { hw_use_rsvd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state0")) { hw_legacy_sent_state0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state1")) { hw_legacy_sent_state1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state2")) { hw_legacy_sent_state2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_legacy_sent_state3")) { hw_legacy_sent_state3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_coalesce_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_assert_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rst_coalesce")) { rst_coalesce(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_assert_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_array_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vec")) { vec(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_array_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_cfg_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start")) { start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "count")) { count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_pba_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_drvcfg_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "coal_init")) { coal_init(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { reserved1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved2")) { reserved2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_credits")) { int_credits(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "unmask")) { unmask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "restart_coal")) { restart_coal(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved3")) { reserved3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_on_assert")) { mask_on_assert(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved4")) { reserved4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_curr")) { coal_curr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved5")) { reserved5(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_drvcfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_fwcfg_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "function_mask")) { function_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { reserved1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "port_id")) { port_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "local_int")) { local_int(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "legacy")) { legacy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intpin")) { intpin(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved2")) { reserved2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_fwcfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_msixcfg_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msg_addr")) { msg_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "msg_data")) { msg_data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "vector_ctrl")) { vector_ctrl(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reserved1")) { reserved1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_dhs_intr_msixcfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_sram_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_pass")) { done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail")) { done_fail(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_sram_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_readout")) { debug_readout(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dw0")) { dw0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw1")) { dw1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw2")) { dw2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dw3")) { dw3(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "source")) { source(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_debug_max_credits_limit_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_threshold")) { int_threshold(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_debug_port_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "select")) { select(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_axi_attr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "awcache")) { awcache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sat_intr_event_counters_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_bresp_errs")) { axi_bresp_errs(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "delayed_coal_scan")) { delayed_coal_scan(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cnt_intr_legacy_send_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "req")) { req(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cnt_intr_tot_axi_wr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "req")) { req(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_ecc_intr_coalesce_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_sta_intr_ecc_intr_state_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_intr_csr_cfg_intr_ecc_disable_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "intr_state_cor")) { intr_state_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_det")) { intr_state_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_state_dhs")) { intr_state_dhs(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_cor")) { intr_pba_cfg_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_det")) { intr_pba_cfg_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_pba_cfg_dhs")) { intr_pba_cfg_dhs(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_cor")) { intr_coalesce_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_det")) { intr_coalesce_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "intr_coalesce_dhs")) { intr_coalesce_dhs(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_debug_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "legacy_fifodepth")) { legacy_fifodepth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coalexp_fifodepth")) { coalexp_fifodepth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wfifo_depth")) { wfifo_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awfifo_depth")) { awfifo_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wready")) { wready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wvalid")) { wvalid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awready")) { awready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awvalid")) { awvalid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdy")) { axi_rdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "coal_scan_active")) { coal_scan_active(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "init_done")) { init_done(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_init_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done")) { done(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_sta_intr_axi_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "num_ids")) { num_ids(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_axi_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "num_ids")) { num_ids(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_cfg_intr_coalesce_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "resolution")) { resolution(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_rdintr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ireg")) { ireg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_intr_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = rdintr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_coalesce.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_axi.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_axi.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_init.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_debug.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_ecc_disable.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_state.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_pba_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_intr_ecc_intr_coalesce.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_intr_tot_axi_wr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_intr_legacy_send.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sat_intr_event_counters.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_intr_axi_attr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_max_credits_limit.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_todo_bits_legacy_cnt_read_debug.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_legacy_intx_pcie_msg_hdr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_todo_bits_legacy_cnt_read_debug.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sram_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sram_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_msixcfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_fwcfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_drvcfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_pba_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_pba_array.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_assert.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_coalesce.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_intr_state.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_intr_ecc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_int_intr_ecc_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("intr_state_uncorrectable_enable");
    ret_vec.push_back("intr_state_correctable_enable");
    ret_vec.push_back("intr_pba_cfg_uncorrectable_enable");
    ret_vec.push_back("intr_pba_cfg_correctable_enable");
    ret_vec.push_back("intr_coalesce_uncorrectable_enable");
    ret_vec.push_back("intr_coalesce_correctable_enable");
    ret_vec.push_back("intr_legacy_fifo_overrun_enable");
    ret_vec.push_back("int_credit_positive_after_return_enable");
    ret_vec.push_back("int_credits_max_debug_threshold_enable");
    ret_vec.push_back("intr_bresp_err_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("intr_state_uncorrectable_interrupt");
    ret_vec.push_back("intr_state_correctable_interrupt");
    ret_vec.push_back("intr_pba_cfg_uncorrectable_interrupt");
    ret_vec.push_back("intr_pba_cfg_correctable_interrupt");
    ret_vec.push_back("intr_coalesce_uncorrectable_interrupt");
    ret_vec.push_back("intr_coalesce_correctable_interrupt");
    ret_vec.push_back("intr_legacy_fifo_overrun_interrupt");
    ret_vec.push_back("int_credit_positive_after_return_interrupt");
    ret_vec.push_back("int_credits_max_debug_threshold_interrupt");
    ret_vec.push_back("intr_bresp_err_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_intgrp_t::get_fields(int level) const { 
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
std::vector<string> cap_intr_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_intr_ecc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_intr_ecc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_intr_csr_dhs_intr_state_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msixcfg_msg_addr_51_2");
    ret_vec.push_back("msixcfg_msg_data");
    ret_vec.push_back("msixcfg_vector_ctrl");
    ret_vec.push_back("fwcfg_function_mask");
    ret_vec.push_back("fwcfg_lif");
    ret_vec.push_back("fwcfg_local_int");
    ret_vec.push_back("fwcfg_legacy_int");
    ret_vec.push_back("fwcfg_legacy_pin");
    ret_vec.push_back("drvcfg_mask");
    ret_vec.push_back("drvcfg_int_credits");
    ret_vec.push_back("drvcfg_mask_on_assert");
    ret_vec.push_back("fwcfg_port_id");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_state_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_coalesce_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("coal_init_value0");
    ret_vec.push_back("coal_init_value1");
    ret_vec.push_back("coal_init_value2");
    ret_vec.push_back("coal_init_value3");
    ret_vec.push_back("coal_curr_value0");
    ret_vec.push_back("coal_curr_value1");
    ret_vec.push_back("coal_curr_value2");
    ret_vec.push_back("coal_curr_value3");
    ret_vec.push_back("hw_use_rsvd");
    ret_vec.push_back("hw_legacy_sent_state0");
    ret_vec.push_back("hw_legacy_sent_state1");
    ret_vec.push_back("hw_legacy_sent_state2");
    ret_vec.push_back("hw_legacy_sent_state3");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_coalesce_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_assert_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("rst_coalesce");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_assert_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_pba_array_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("vec");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_pba_array_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_pba_cfg_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("start");
    ret_vec.push_back("count");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_pba_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_drvcfg_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("coal_init");
    ret_vec.push_back("reserved1");
    ret_vec.push_back("mask");
    ret_vec.push_back("reserved2");
    ret_vec.push_back("int_credits");
    ret_vec.push_back("unmask");
    ret_vec.push_back("restart_coal");
    ret_vec.push_back("reserved3");
    ret_vec.push_back("mask_on_assert");
    ret_vec.push_back("reserved4");
    ret_vec.push_back("coal_curr");
    ret_vec.push_back("reserved5");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_drvcfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_fwcfg_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("function_mask");
    ret_vec.push_back("reserved1");
    ret_vec.push_back("lif");
    ret_vec.push_back("port_id");
    ret_vec.push_back("local_int");
    ret_vec.push_back("legacy");
    ret_vec.push_back("intpin");
    ret_vec.push_back("reserved2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_fwcfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_msixcfg_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msg_addr");
    ret_vec.push_back("msg_data");
    ret_vec.push_back("vector_ctrl");
    ret_vec.push_back("reserved1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_dhs_intr_msixcfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_sram_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("done_pass");
    ret_vec.push_back("done_fail");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_sram_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("valid");
    ret_vec.push_back("debug_readout");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dw0");
    ret_vec.push_back("dw1");
    ret_vec.push_back("dw2");
    ret_vec.push_back("dw3");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("source");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_debug_max_credits_limit_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_threshold");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_debug_port_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("select");
    ret_vec.push_back("enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_intr_axi_attr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("awcache");
    ret_vec.push_back("prot");
    ret_vec.push_back("qos");
    ret_vec.push_back("lock");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sat_intr_event_counters_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_bresp_errs");
    ret_vec.push_back("delayed_coal_scan");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cnt_intr_legacy_send_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("req");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cnt_intr_tot_axi_wr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("req");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_intr_ecc_intr_coalesce_t::get_fields(int level) const { 
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
std::vector<string> cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t::get_fields(int level) const { 
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
std::vector<string> cap_intr_csr_sta_intr_ecc_intr_state_t::get_fields(int level) const { 
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
std::vector<string> cap_intr_csr_cfg_intr_ecc_disable_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("intr_state_cor");
    ret_vec.push_back("intr_state_det");
    ret_vec.push_back("intr_state_dhs");
    ret_vec.push_back("intr_pba_cfg_cor");
    ret_vec.push_back("intr_pba_cfg_det");
    ret_vec.push_back("intr_pba_cfg_dhs");
    ret_vec.push_back("intr_coalesce_cor");
    ret_vec.push_back("intr_coalesce_det");
    ret_vec.push_back("intr_coalesce_dhs");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_intr_debug_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("legacy_fifodepth");
    ret_vec.push_back("coalexp_fifodepth");
    ret_vec.push_back("wfifo_depth");
    ret_vec.push_back("awfifo_depth");
    ret_vec.push_back("wready");
    ret_vec.push_back("wvalid");
    ret_vec.push_back("awready");
    ret_vec.push_back("awvalid");
    ret_vec.push_back("axi_rdy");
    ret_vec.push_back("coal_scan_active");
    ret_vec.push_back("init_done");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_intr_init_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("done");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_sta_intr_axi_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("num_ids");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_intr_axi_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("num_ids");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_cfg_intr_coalesce_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("resolution");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_rdintr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ireg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_intr_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : rdintr.get_fields(level-1)) {
            ret_vec.push_back("rdintr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_intr_coalesce.get_fields(level-1)) {
            ret_vec.push_back("cfg_intr_coalesce." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_intr_axi.get_fields(level-1)) {
            ret_vec.push_back("cfg_intr_axi." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_axi.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_axi." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_init.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_init." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_debug.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_debug." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_intr_ecc_disable.get_fields(level-1)) {
            ret_vec.push_back("cfg_intr_ecc_disable." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_ecc_intr_state.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_ecc_intr_state." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_ecc_intr_pba_cfg.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_ecc_intr_pba_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_intr_ecc_intr_coalesce.get_fields(level-1)) {
            ret_vec.push_back("sta_intr_ecc_intr_coalesce." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_intr_tot_axi_wr.get_fields(level-1)) {
            ret_vec.push_back("cnt_intr_tot_axi_wr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_intr_legacy_send.get_fields(level-1)) {
            ret_vec.push_back("cnt_intr_legacy_send." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sat_intr_event_counters.get_fields(level-1)) {
            ret_vec.push_back("sat_intr_event_counters." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_intr_axi_attr.get_fields(level-1)) {
            ret_vec.push_back("cfg_intr_axi_attr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_debug_port.get_fields(level-1)) {
            ret_vec.push_back("cfg_debug_port." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_debug_max_credits_limit.get_fields(level-1)) {
            ret_vec.push_back("cfg_debug_max_credits_limit." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_todo_bits_legacy_cnt_read_debug.get_fields(level-1)) {
            ret_vec.push_back("cfg_todo_bits_legacy_cnt_read_debug." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_legacy_intx_pcie_msg_hdr.get_fields(level-1)) {
            ret_vec.push_back("cfg_legacy_intx_pcie_msg_hdr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_todo_bits_legacy_cnt_read_debug.get_fields(level-1)) {
            ret_vec.push_back("sta_todo_bits_legacy_cnt_read_debug." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sram_bist.get_fields(level-1)) {
            ret_vec.push_back("cfg_sram_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sram_bist.get_fields(level-1)) {
            ret_vec.push_back("sta_sram_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_msixcfg.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_msixcfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_fwcfg.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_fwcfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_drvcfg.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_drvcfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_pba_cfg.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_pba_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_pba_array.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_pba_array." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_assert.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_assert." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_coalesce.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_coalesce." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_intr_state.get_fields(level-1)) {
            ret_vec.push_back("dhs_intr_state." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_intr_ecc.get_fields(level-1)) {
            ret_vec.push_back("int_intr_ecc." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
