
#include "cap_pp_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pp_csr_dhs_sbus_indir_entry_t::cap_pp_csr_dhs_sbus_indir_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_dhs_sbus_indir_entry_t::~cap_pp_csr_dhs_sbus_indir_entry_t() { }

cap_pp_csr_dhs_sbus_indir_t::cap_pp_csr_dhs_sbus_indir_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pp_csr_dhs_sbus_indir_t::~cap_pp_csr_dhs_sbus_indir_t() { }

cap_pp_csr_cfg_debug_port_t::cap_pp_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_debug_port_t::~cap_pp_csr_cfg_debug_port_t() { }

cap_pp_csr_cfg_pp_sd_core_to_cntl_t::cap_pp_csr_cfg_pp_sd_core_to_cntl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sd_core_to_cntl_t::~cap_pp_csr_cfg_pp_sd_core_to_cntl_t() { }

cap_pp_csr_cfg_pp_pcsd_control_t::cap_pp_csr_cfg_pp_pcsd_control_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcsd_control_t::~cap_pp_csr_cfg_pp_pcsd_control_t() { }

cap_pp_csr_sta_pp_sbus_master_bist_t::cap_pp_csr_sta_pp_sbus_master_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_sbus_master_bist_t::~cap_pp_csr_sta_pp_sbus_master_bist_t() { }

cap_pp_csr_cfg_pp_sbus_master_bist_t::cap_pp_csr_cfg_pp_sbus_master_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sbus_master_bist_t::~cap_pp_csr_cfg_pp_sbus_master_bist_t() { }

cap_pp_csr_sta_pp_sd_rdy_t::cap_pp_csr_sta_pp_sd_rdy_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_sd_rdy_t::~cap_pp_csr_sta_pp_sd_rdy_t() { }

cap_pp_csr_sta_pp_sd_core_status_t::cap_pp_csr_sta_pp_sd_core_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_sd_core_status_t::~cap_pp_csr_sta_pp_sd_core_status_t() { }

cap_pp_csr_sta_pp_sd_spico_gp_t::cap_pp_csr_sta_pp_sd_spico_gp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_sd_spico_gp_t::~cap_pp_csr_sta_pp_sd_spico_gp_t() { }

cap_pp_csr_cfg_pp_sd_spico_gp_t::cap_pp_csr_cfg_pp_sd_spico_gp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sd_spico_gp_t::~cap_pp_csr_cfg_pp_sd_spico_gp_t() { }

cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::~cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t() { }

cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::~cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t() { }

cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::cap_pp_csr_cfg_pp_pcsd_interrupt_request_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::~cap_pp_csr_cfg_pp_pcsd_interrupt_request_t() { }

cap_pp_csr_cfg_pp_pcsd_interrupt_t::cap_pp_csr_cfg_pp_pcsd_interrupt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcsd_interrupt_t::~cap_pp_csr_cfg_pp_pcsd_interrupt_t() { }

cap_pp_csr_sta_pp_pcie_pll_t::cap_pp_csr_sta_pp_pcie_pll_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_pp_pcie_pll_t::~cap_pp_csr_sta_pp_pcie_pll_t() { }

cap_pp_csr_cfg_pp_pcie_pll_1_t::cap_pp_csr_cfg_pp_pcie_pll_1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcie_pll_1_t::~cap_pp_csr_cfg_pp_pcie_pll_1_t() { }

cap_pp_csr_cfg_pp_pcie_pll_0_t::cap_pp_csr_cfg_pp_pcie_pll_0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcie_pll_0_t::~cap_pp_csr_cfg_pp_pcie_pll_0_t() { }

cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::~cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t() { }

cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::cap_pp_csr_cfg_pp_pcie_pll_rst_n_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::~cap_pp_csr_cfg_pp_pcie_pll_rst_n_t() { }

cap_pp_csr_cfg_pp_rc_perstn_t::cap_pp_csr_cfg_pp_rc_perstn_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_rc_perstn_t::~cap_pp_csr_cfg_pp_rc_perstn_t() { }

cap_pp_csr_cfg_pp_sbus_t::cap_pp_csr_cfg_pp_sbus_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sbus_t::~cap_pp_csr_cfg_pp_sbus_t() { }

cap_pp_csr_cfg_pp_pcs_reset_n_t::cap_pp_csr_cfg_pp_pcs_reset_n_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcs_reset_n_t::~cap_pp_csr_cfg_pp_pcs_reset_n_t() { }

cap_pp_csr_cfg_pp_sd_async_reset_n_t::cap_pp_csr_cfg_pp_sd_async_reset_n_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sd_async_reset_n_t::~cap_pp_csr_cfg_pp_sd_async_reset_n_t() { }

cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::cap_pp_csr_cfg_pp_pcs_interrupt_disable_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::~cap_pp_csr_cfg_pp_pcs_interrupt_disable_t() { }

cap_pp_csr_sta_sbus_indir_t::cap_pp_csr_sta_sbus_indir_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_sta_sbus_indir_t::~cap_pp_csr_sta_sbus_indir_t() { }

cap_pp_csr_cfg_sbus_indir_t::cap_pp_csr_cfg_sbus_indir_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_sbus_indir_t::~cap_pp_csr_cfg_sbus_indir_t() { }

cap_pp_csr_cfg_sbus_result_t::cap_pp_csr_cfg_sbus_result_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_sbus_result_t::~cap_pp_csr_cfg_sbus_result_t() { }

cap_pp_csr_cfg_pp_sw_reset_t::cap_pp_csr_cfg_pp_sw_reset_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_sw_reset_t::~cap_pp_csr_cfg_pp_sw_reset_t() { }

cap_pp_csr_cfg_pp_linkwidth_t::cap_pp_csr_cfg_pp_linkwidth_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_cfg_pp_linkwidth_t::~cap_pp_csr_cfg_pp_linkwidth_t() { }

cap_pp_csr_base_t::cap_pp_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pp_csr_base_t::~cap_pp_csr_base_t() { }

cap_pp_csr_t::cap_pp_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(262144);
        set_attributes(0,get_name(), 0);
        }
cap_pp_csr_t::~cap_pp_csr_t() { }

void cap_pp_csr_dhs_sbus_indir_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_pp_csr_dhs_sbus_indir_t::show() {

    entry.show();
}

void cap_pp_csr_cfg_debug_port_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
}

void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".port0: 0x" << int_var__port0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port1: 0x" << int_var__port1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port2: 0x" << int_var__port2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port3: 0x" << int_var__port3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port4: 0x" << int_var__port4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port5: 0x" << int_var__port5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port6: 0x" << int_var__port6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port7: 0x" << int_var__port7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port8: 0x" << int_var__port8 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port9: 0x" << int_var__port9 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port10: 0x" << int_var__port10 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port11: 0x" << int_var__port11 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port12: 0x" << int_var__port12 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port13: 0x" << int_var__port13 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port14: 0x" << int_var__port14 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".port15: 0x" << int_var__port15 << dec << endl)
}

void cap_pp_csr_cfg_pp_pcsd_control_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_0: 0x" << int_var__fts_align_grp_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_0: 0x" << int_var__sris_en_grp_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_0: 0x" << int_var__rx8b10b_realign_grp_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_1: 0x" << int_var__fts_align_grp_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_1: 0x" << int_var__sris_en_grp_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_1: 0x" << int_var__rx8b10b_realign_grp_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_2: 0x" << int_var__fts_align_grp_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_2: 0x" << int_var__sris_en_grp_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_2: 0x" << int_var__rx8b10b_realign_grp_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_3: 0x" << int_var__fts_align_grp_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_3: 0x" << int_var__sris_en_grp_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_3: 0x" << int_var__rx8b10b_realign_grp_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_4: 0x" << int_var__fts_align_grp_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_4: 0x" << int_var__sris_en_grp_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_4: 0x" << int_var__rx8b10b_realign_grp_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_5: 0x" << int_var__fts_align_grp_5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_5: 0x" << int_var__sris_en_grp_5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_5: 0x" << int_var__rx8b10b_realign_grp_5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_6: 0x" << int_var__fts_align_grp_6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_6: 0x" << int_var__sris_en_grp_6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_6: 0x" << int_var__rx8b10b_realign_grp_6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fts_align_grp_7: 0x" << int_var__fts_align_grp_7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sris_en_grp_7: 0x" << int_var__sris_en_grp_7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rx8b10b_realign_grp_7: 0x" << int_var__rx8b10b_realign_grp_7 << dec << endl)
}

void cap_pp_csr_sta_pp_sbus_master_bist_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".fail: 0x" << int_var__fail << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pass: 0x" << int_var__pass << dec << endl)
}

void cap_pp_csr_cfg_pp_sbus_master_bist_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".mode: 0x" << int_var__mode << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rst: 0x" << int_var__rst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".run: 0x" << int_var__run << dec << endl)
}

void cap_pp_csr_sta_pp_sd_rdy_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".rx_rdy: 0x" << int_var__rx_rdy << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tx_rdy: 0x" << int_var__tx_rdy << dec << endl)
}

void cap_pp_csr_sta_pp_sd_core_status_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lane0: 0x" << int_var__lane0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1: 0x" << int_var__lane1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2: 0x" << int_var__lane2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3: 0x" << int_var__lane3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane4: 0x" << int_var__lane4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane5: 0x" << int_var__lane5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane6: 0x" << int_var__lane6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane7: 0x" << int_var__lane7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane8: 0x" << int_var__lane8 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane9: 0x" << int_var__lane9 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane10: 0x" << int_var__lane10 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane11: 0x" << int_var__lane11 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane12: 0x" << int_var__lane12 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane13: 0x" << int_var__lane13 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane14: 0x" << int_var__lane14 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane15: 0x" << int_var__lane15 << dec << endl)
}

void cap_pp_csr_sta_pp_sd_spico_gp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".out: 0x" << int_var__out << dec << endl)
}

void cap_pp_csr_cfg_pp_sd_spico_gp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".in: 0x" << int_var__in << dec << endl)
}

void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lane0: 0x" << int_var__lane0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane1: 0x" << int_var__lane1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane2: 0x" << int_var__lane2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane3: 0x" << int_var__lane3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane4: 0x" << int_var__lane4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane5: 0x" << int_var__lane5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane6: 0x" << int_var__lane6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane7: 0x" << int_var__lane7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane8: 0x" << int_var__lane8 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane9: 0x" << int_var__lane9 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane10: 0x" << int_var__lane10 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane11: 0x" << int_var__lane11 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane12: 0x" << int_var__lane12 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane13: 0x" << int_var__lane13 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane14: 0x" << int_var__lane14 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lane15: 0x" << int_var__lane15 << dec << endl)
}

void cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".per_lane: 0x" << int_var__per_lane << dec << endl)
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lanemask: 0x" << int_var__lanemask << dec << endl)
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".code: 0x" << int_var__code << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_pp_csr_sta_pp_pcie_pll_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_0_dll_lock: 0x" << int_var__pcie_0_dll_lock << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_0_pll_lock: 0x" << int_var__pcie_0_pll_lock << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_1_dll_lock: 0x" << int_var__pcie_1_dll_lock << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_1_pll_lock: 0x" << int_var__pcie_1_pll_lock << dec << endl)
}

void cap_pp_csr_cfg_pp_pcie_pll_1_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".dll_disable_output_clk: 0x" << int_var__dll_disable_output_clk << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_fbcnt: 0x" << int_var__dll_fbcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_out_divcnt: 0x" << int_var__dll_out_divcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_refcnt: 0x" << int_var__dll_refcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_disable_output_clk: 0x" << int_var__pll_disable_output_clk << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_out_divcnt: 0x" << int_var__pll_out_divcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_refcnt: 0x" << int_var__pll_refcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prog_fbdiv255: 0x" << int_var__prog_fbdiv255 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prog_fbdiv_23: 0x" << int_var__prog_fbdiv_23 << dec << endl)
}

void cap_pp_csr_cfg_pp_pcie_pll_0_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".dll_disable_output_clk: 0x" << int_var__dll_disable_output_clk << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_fbcnt: 0x" << int_var__dll_fbcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_out_divcnt: 0x" << int_var__dll_out_divcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dll_refcnt: 0x" << int_var__dll_refcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_disable_output_clk: 0x" << int_var__pll_disable_output_clk << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_out_divcnt: 0x" << int_var__pll_out_divcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_refcnt: 0x" << int_var__pll_refcnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prog_fbdiv255: 0x" << int_var__prog_fbdiv255 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prog_fbdiv_23: 0x" << int_var__prog_fbdiv_23 << dec << endl)
}

void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".p0: 0x" << int_var__p0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1: 0x" << int_var__p1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2: 0x" << int_var__p2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3: 0x" << int_var__p3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4: 0x" << int_var__p4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5: 0x" << int_var__p5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6: 0x" << int_var__p6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7: 0x" << int_var__p7 << dec << endl)
}

void cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".pll_0: 0x" << int_var__pll_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pll_1: 0x" << int_var__pll_1 << dec << endl)
}

void cap_pp_csr_cfg_pp_rc_perstn_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".p0_out_en: 0x" << int_var__p0_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p0_out_data: 0x" << int_var__p0_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1_out_en: 0x" << int_var__p1_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1_out_data: 0x" << int_var__p1_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2_out_en: 0x" << int_var__p2_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2_out_data: 0x" << int_var__p2_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3_out_en: 0x" << int_var__p3_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3_out_data: 0x" << int_var__p3_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4_out_en: 0x" << int_var__p4_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4_out_data: 0x" << int_var__p4_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5_out_en: 0x" << int_var__p5_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5_out_data: 0x" << int_var__p5_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6_out_en: 0x" << int_var__p6_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6_out_data: 0x" << int_var__p6_out_data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7_out_en: 0x" << int_var__p7_out_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7_out_data: 0x" << int_var__p7_out_data << dec << endl)
}

void cap_pp_csr_cfg_pp_sbus_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".rom_en: 0x" << int_var__rom_en << dec << endl)
}

void cap_pp_csr_cfg_pp_pcs_reset_n_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lanemask: 0x" << int_var__lanemask << dec << endl)
}

void cap_pp_csr_cfg_pp_sd_async_reset_n_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lanemask: 0x" << int_var__lanemask << dec << endl)
}

void cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lanemask: 0x" << int_var__lanemask << dec << endl)
}

void cap_pp_csr_sta_sbus_indir_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".execute: 0x" << int_var__execute << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".done: 0x" << int_var__done << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rcv_data_vld: 0x" << int_var__rcv_data_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".result_code: 0x" << int_var__result_code << dec << endl)
}

void cap_pp_csr_cfg_sbus_indir_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".rcvr_addr: 0x" << int_var__rcvr_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".data_addr: 0x" << int_var__data_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".command: 0x" << int_var__command << dec << endl)
}

void cap_pp_csr_cfg_sbus_result_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".mode: 0x" << int_var__mode << dec << endl)
}

void cap_pp_csr_cfg_pp_sw_reset_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".p0_hrst: 0x" << int_var__p0_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p0_srst: 0x" << int_var__p0_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1_hrst: 0x" << int_var__p1_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1_srst: 0x" << int_var__p1_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2_hrst: 0x" << int_var__p2_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2_srst: 0x" << int_var__p2_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3_hrst: 0x" << int_var__p3_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3_srst: 0x" << int_var__p3_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4_hrst: 0x" << int_var__p4_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4_srst: 0x" << int_var__p4_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5_hrst: 0x" << int_var__p5_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5_srst: 0x" << int_var__p5_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6_hrst: 0x" << int_var__p6_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6_srst: 0x" << int_var__p6_srst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7_hrst: 0x" << int_var__p7_hrst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7_srst: 0x" << int_var__p7_srst << dec << endl)
}

void cap_pp_csr_cfg_pp_linkwidth_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".p0_lw: 0x" << int_var__p0_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p1_lw: 0x" << int_var__p1_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p2_lw: 0x" << int_var__p2_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p3_lw: 0x" << int_var__p3_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p4_lw: 0x" << int_var__p4_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p5_lw: 0x" << int_var__p5_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p6_lw: 0x" << int_var__p6_lw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".p7_lw: 0x" << int_var__p7_lw << dec << endl)
}

void cap_pp_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_pp_csr_t::show() {

    base.show();
    cfg_pp_linkwidth.show();
    cfg_pp_sw_reset.show();
    cfg_sbus_result.show();
    cfg_sbus_indir.show();
    sta_sbus_indir.show();
    cfg_pp_pcs_interrupt_disable.show();
    cfg_pp_sd_async_reset_n.show();
    cfg_pp_pcs_reset_n.show();
    cfg_pp_sbus.show();
    cfg_pp_rc_perstn.show();
    cfg_pp_pcie_pll_rst_n.show();
    cfg_pp_pcie_pll_refclk_sel.show();
    cfg_pp_pcie_pll_0.show();
    cfg_pp_pcie_pll_1.show();
    sta_pp_pcie_pll.show();
    for(int ii = 0; ii < 16; ii++) {
        cfg_pp_pcsd_interrupt[ii].show();
    }
    cfg_pp_pcsd_interrupt_request.show();
    sta_pp_pcsd_interrupt_in_progress.show();
    sta_pp_pcsd_interrupt_data_out.show();
    cfg_pp_sd_spico_gp.show();
    sta_pp_sd_spico_gp.show();
    sta_pp_sd_core_status.show();
    sta_pp_sd_rdy.show();
    cfg_pp_sbus_master_bist.show();
    sta_pp_sbus_master_bist.show();
    cfg_pp_pcsd_control.show();
    cfg_pp_sd_core_to_cntl.show();
    cfg_debug_port.show();
    dhs_sbus_indir.show();
    for(int ii = 0; ii < 8; ii++) {
        port_p[ii].show();
    }
    for(int ii = 0; ii < 8; ii++) {
        port_c[ii].show();
    }
}

int cap_pp_csr_dhs_sbus_indir_entry_t::get_width() const {
    return cap_pp_csr_dhs_sbus_indir_entry_t::s_get_width();

}

int cap_pp_csr_dhs_sbus_indir_t::get_width() const {
    return cap_pp_csr_dhs_sbus_indir_t::s_get_width();

}

int cap_pp_csr_cfg_debug_port_t::get_width() const {
    return cap_pp_csr_cfg_debug_port_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::get_width() const {
    return cap_pp_csr_cfg_pp_sd_core_to_cntl_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcsd_control_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcsd_control_t::s_get_width();

}

int cap_pp_csr_sta_pp_sbus_master_bist_t::get_width() const {
    return cap_pp_csr_sta_pp_sbus_master_bist_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sbus_master_bist_t::get_width() const {
    return cap_pp_csr_cfg_pp_sbus_master_bist_t::s_get_width();

}

int cap_pp_csr_sta_pp_sd_rdy_t::get_width() const {
    return cap_pp_csr_sta_pp_sd_rdy_t::s_get_width();

}

int cap_pp_csr_sta_pp_sd_core_status_t::get_width() const {
    return cap_pp_csr_sta_pp_sd_core_status_t::s_get_width();

}

int cap_pp_csr_sta_pp_sd_spico_gp_t::get_width() const {
    return cap_pp_csr_sta_pp_sd_spico_gp_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sd_spico_gp_t::get_width() const {
    return cap_pp_csr_cfg_pp_sd_spico_gp_t::s_get_width();

}

int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::get_width() const {
    return cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::s_get_width();

}

int cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::get_width() const {
    return cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcsd_interrupt_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcsd_interrupt_t::s_get_width();

}

int cap_pp_csr_sta_pp_pcie_pll_t::get_width() const {
    return cap_pp_csr_sta_pp_pcie_pll_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcie_pll_1_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcie_pll_1_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcie_pll_0_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcie_pll_0_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::s_get_width();

}

int cap_pp_csr_cfg_pp_rc_perstn_t::get_width() const {
    return cap_pp_csr_cfg_pp_rc_perstn_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sbus_t::get_width() const {
    return cap_pp_csr_cfg_pp_sbus_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcs_reset_n_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcs_reset_n_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sd_async_reset_n_t::get_width() const {
    return cap_pp_csr_cfg_pp_sd_async_reset_n_t::s_get_width();

}

int cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::get_width() const {
    return cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::s_get_width();

}

int cap_pp_csr_sta_sbus_indir_t::get_width() const {
    return cap_pp_csr_sta_sbus_indir_t::s_get_width();

}

int cap_pp_csr_cfg_sbus_indir_t::get_width() const {
    return cap_pp_csr_cfg_sbus_indir_t::s_get_width();

}

int cap_pp_csr_cfg_sbus_result_t::get_width() const {
    return cap_pp_csr_cfg_sbus_result_t::s_get_width();

}

int cap_pp_csr_cfg_pp_sw_reset_t::get_width() const {
    return cap_pp_csr_cfg_pp_sw_reset_t::s_get_width();

}

int cap_pp_csr_cfg_pp_linkwidth_t::get_width() const {
    return cap_pp_csr_cfg_pp_linkwidth_t::s_get_width();

}

int cap_pp_csr_base_t::get_width() const {
    return cap_pp_csr_base_t::s_get_width();

}

int cap_pp_csr_t::get_width() const {
    return cap_pp_csr_t::s_get_width();

}

int cap_pp_csr_dhs_sbus_indir_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_pp_csr_dhs_sbus_indir_t::s_get_width() {
    int _count = 0;

    _count += cap_pp_csr_dhs_sbus_indir_entry_t::s_get_width(); // entry
    return _count;
}

int cap_pp_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 4; // select
    _count += 1; // enable
    return _count;
}

int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::s_get_width() {
    int _count = 0;

    _count += 16; // port0
    _count += 16; // port1
    _count += 16; // port2
    _count += 16; // port3
    _count += 16; // port4
    _count += 16; // port5
    _count += 16; // port6
    _count += 16; // port7
    _count += 16; // port8
    _count += 16; // port9
    _count += 16; // port10
    _count += 16; // port11
    _count += 16; // port12
    _count += 16; // port13
    _count += 16; // port14
    _count += 16; // port15
    return _count;
}

int cap_pp_csr_cfg_pp_pcsd_control_t::s_get_width() {
    int _count = 0;

    _count += 2; // fts_align_grp_0
    _count += 2; // sris_en_grp_0
    _count += 2; // rx8b10b_realign_grp_0
    _count += 2; // fts_align_grp_1
    _count += 2; // sris_en_grp_1
    _count += 2; // rx8b10b_realign_grp_1
    _count += 2; // fts_align_grp_2
    _count += 2; // sris_en_grp_2
    _count += 2; // rx8b10b_realign_grp_2
    _count += 2; // fts_align_grp_3
    _count += 2; // sris_en_grp_3
    _count += 2; // rx8b10b_realign_grp_3
    _count += 2; // fts_align_grp_4
    _count += 2; // sris_en_grp_4
    _count += 2; // rx8b10b_realign_grp_4
    _count += 2; // fts_align_grp_5
    _count += 2; // sris_en_grp_5
    _count += 2; // rx8b10b_realign_grp_5
    _count += 2; // fts_align_grp_6
    _count += 2; // sris_en_grp_6
    _count += 2; // rx8b10b_realign_grp_6
    _count += 2; // fts_align_grp_7
    _count += 2; // sris_en_grp_7
    _count += 2; // rx8b10b_realign_grp_7
    return _count;
}

int cap_pp_csr_sta_pp_sbus_master_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // fail
    _count += 1; // pass
    return _count;
}

int cap_pp_csr_cfg_pp_sbus_master_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // mode
    _count += 1; // rst
    _count += 1; // run
    return _count;
}

int cap_pp_csr_sta_pp_sd_rdy_t::s_get_width() {
    int _count = 0;

    _count += 16; // rx_rdy
    _count += 16; // tx_rdy
    return _count;
}

int cap_pp_csr_sta_pp_sd_core_status_t::s_get_width() {
    int _count = 0;

    _count += 16; // lane0
    _count += 16; // lane1
    _count += 16; // lane2
    _count += 16; // lane3
    _count += 16; // lane4
    _count += 16; // lane5
    _count += 16; // lane6
    _count += 16; // lane7
    _count += 16; // lane8
    _count += 16; // lane9
    _count += 16; // lane10
    _count += 16; // lane11
    _count += 16; // lane12
    _count += 16; // lane13
    _count += 16; // lane14
    _count += 16; // lane15
    return _count;
}

int cap_pp_csr_sta_pp_sd_spico_gp_t::s_get_width() {
    int _count = 0;

    _count += 16; // out
    return _count;
}

int cap_pp_csr_cfg_pp_sd_spico_gp_t::s_get_width() {
    int _count = 0;

    _count += 16; // in
    return _count;
}

int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::s_get_width() {
    int _count = 0;

    _count += 16; // lane0
    _count += 16; // lane1
    _count += 16; // lane2
    _count += 16; // lane3
    _count += 16; // lane4
    _count += 16; // lane5
    _count += 16; // lane6
    _count += 16; // lane7
    _count += 16; // lane8
    _count += 16; // lane9
    _count += 16; // lane10
    _count += 16; // lane11
    _count += 16; // lane12
    _count += 16; // lane13
    _count += 16; // lane14
    _count += 16; // lane15
    return _count;
}

int cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::s_get_width() {
    int _count = 0;

    _count += 16; // per_lane
    return _count;
}

int cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::s_get_width() {
    int _count = 0;

    _count += 16; // lanemask
    return _count;
}

int cap_pp_csr_cfg_pp_pcsd_interrupt_t::s_get_width() {
    int _count = 0;

    _count += 16; // code
    _count += 16; // data
    return _count;
}

int cap_pp_csr_sta_pp_pcie_pll_t::s_get_width() {
    int _count = 0;

    _count += 1; // pcie_0_dll_lock
    _count += 1; // pcie_0_pll_lock
    _count += 1; // pcie_1_dll_lock
    _count += 1; // pcie_1_pll_lock
    return _count;
}

int cap_pp_csr_cfg_pp_pcie_pll_1_t::s_get_width() {
    int _count = 0;

    _count += 4; // dll_disable_output_clk
    _count += 6; // dll_fbcnt
    _count += 6; // dll_out_divcnt
    _count += 6; // dll_refcnt
    _count += 4; // pll_disable_output_clk
    _count += 6; // pll_out_divcnt
    _count += 6; // pll_refcnt
    _count += 8; // prog_fbdiv255
    _count += 1; // prog_fbdiv_23
    return _count;
}

int cap_pp_csr_cfg_pp_pcie_pll_0_t::s_get_width() {
    int _count = 0;

    _count += 4; // dll_disable_output_clk
    _count += 6; // dll_fbcnt
    _count += 6; // dll_out_divcnt
    _count += 6; // dll_refcnt
    _count += 4; // pll_disable_output_clk
    _count += 6; // pll_out_divcnt
    _count += 6; // pll_refcnt
    _count += 8; // prog_fbdiv255
    _count += 1; // prog_fbdiv_23
    return _count;
}

int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::s_get_width() {
    int _count = 0;

    _count += 1; // p0
    _count += 1; // p1
    _count += 1; // p2
    _count += 1; // p3
    _count += 1; // p4
    _count += 1; // p5
    _count += 1; // p6
    _count += 1; // p7
    return _count;
}

int cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::s_get_width() {
    int _count = 0;

    _count += 1; // pll_0
    _count += 1; // pll_1
    return _count;
}

int cap_pp_csr_cfg_pp_rc_perstn_t::s_get_width() {
    int _count = 0;

    _count += 1; // p0_out_en
    _count += 1; // p0_out_data
    _count += 1; // p1_out_en
    _count += 1; // p1_out_data
    _count += 1; // p2_out_en
    _count += 1; // p2_out_data
    _count += 1; // p3_out_en
    _count += 1; // p3_out_data
    _count += 1; // p4_out_en
    _count += 1; // p4_out_data
    _count += 1; // p5_out_en
    _count += 1; // p5_out_data
    _count += 1; // p6_out_en
    _count += 1; // p6_out_data
    _count += 1; // p7_out_en
    _count += 1; // p7_out_data
    return _count;
}

int cap_pp_csr_cfg_pp_sbus_t::s_get_width() {
    int _count = 0;

    _count += 1; // rom_en
    return _count;
}

int cap_pp_csr_cfg_pp_pcs_reset_n_t::s_get_width() {
    int _count = 0;

    _count += 16; // lanemask
    return _count;
}

int cap_pp_csr_cfg_pp_sd_async_reset_n_t::s_get_width() {
    int _count = 0;

    _count += 16; // lanemask
    return _count;
}

int cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::s_get_width() {
    int _count = 0;

    _count += 16; // lanemask
    return _count;
}

int cap_pp_csr_sta_sbus_indir_t::s_get_width() {
    int _count = 0;

    _count += 1; // execute
    _count += 1; // done
    _count += 1; // rcv_data_vld
    _count += 3; // result_code
    return _count;
}

int cap_pp_csr_cfg_sbus_indir_t::s_get_width() {
    int _count = 0;

    _count += 8; // rcvr_addr
    _count += 8; // data_addr
    _count += 8; // command
    return _count;
}

int cap_pp_csr_cfg_sbus_result_t::s_get_width() {
    int _count = 0;

    _count += 1; // mode
    return _count;
}

int cap_pp_csr_cfg_pp_sw_reset_t::s_get_width() {
    int _count = 0;

    _count += 1; // p0_hrst
    _count += 1; // p0_srst
    _count += 1; // p1_hrst
    _count += 1; // p1_srst
    _count += 1; // p2_hrst
    _count += 1; // p2_srst
    _count += 1; // p3_hrst
    _count += 1; // p3_srst
    _count += 1; // p4_hrst
    _count += 1; // p4_srst
    _count += 1; // p5_hrst
    _count += 1; // p5_srst
    _count += 1; // p6_hrst
    _count += 1; // p6_srst
    _count += 1; // p7_hrst
    _count += 1; // p7_srst
    return _count;
}

int cap_pp_csr_cfg_pp_linkwidth_t::s_get_width() {
    int _count = 0;

    _count += 2; // p0_lw
    _count += 2; // p1_lw
    _count += 2; // p2_lw
    _count += 2; // p3_lw
    _count += 2; // p4_lw
    _count += 2; // p5_lw
    _count += 2; // p6_lw
    _count += 2; // p7_lw
    return _count;
}

int cap_pp_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pp_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pp_csr_base_t::s_get_width(); // base
    _count += cap_pp_csr_cfg_pp_linkwidth_t::s_get_width(); // cfg_pp_linkwidth
    _count += cap_pp_csr_cfg_pp_sw_reset_t::s_get_width(); // cfg_pp_sw_reset
    _count += cap_pp_csr_cfg_sbus_result_t::s_get_width(); // cfg_sbus_result
    _count += cap_pp_csr_cfg_sbus_indir_t::s_get_width(); // cfg_sbus_indir
    _count += cap_pp_csr_sta_sbus_indir_t::s_get_width(); // sta_sbus_indir
    _count += cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::s_get_width(); // cfg_pp_pcs_interrupt_disable
    _count += cap_pp_csr_cfg_pp_sd_async_reset_n_t::s_get_width(); // cfg_pp_sd_async_reset_n
    _count += cap_pp_csr_cfg_pp_pcs_reset_n_t::s_get_width(); // cfg_pp_pcs_reset_n
    _count += cap_pp_csr_cfg_pp_sbus_t::s_get_width(); // cfg_pp_sbus
    _count += cap_pp_csr_cfg_pp_rc_perstn_t::s_get_width(); // cfg_pp_rc_perstn
    _count += cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::s_get_width(); // cfg_pp_pcie_pll_rst_n
    _count += cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::s_get_width(); // cfg_pp_pcie_pll_refclk_sel
    _count += cap_pp_csr_cfg_pp_pcie_pll_0_t::s_get_width(); // cfg_pp_pcie_pll_0
    _count += cap_pp_csr_cfg_pp_pcie_pll_1_t::s_get_width(); // cfg_pp_pcie_pll_1
    _count += cap_pp_csr_sta_pp_pcie_pll_t::s_get_width(); // sta_pp_pcie_pll
    _count += (cap_pp_csr_cfg_pp_pcsd_interrupt_t::s_get_width() * 16); // cfg_pp_pcsd_interrupt
    _count += cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::s_get_width(); // cfg_pp_pcsd_interrupt_request
    _count += cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::s_get_width(); // sta_pp_pcsd_interrupt_in_progress
    _count += cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::s_get_width(); // sta_pp_pcsd_interrupt_data_out
    _count += cap_pp_csr_cfg_pp_sd_spico_gp_t::s_get_width(); // cfg_pp_sd_spico_gp
    _count += cap_pp_csr_sta_pp_sd_spico_gp_t::s_get_width(); // sta_pp_sd_spico_gp
    _count += cap_pp_csr_sta_pp_sd_core_status_t::s_get_width(); // sta_pp_sd_core_status
    _count += cap_pp_csr_sta_pp_sd_rdy_t::s_get_width(); // sta_pp_sd_rdy
    _count += cap_pp_csr_cfg_pp_sbus_master_bist_t::s_get_width(); // cfg_pp_sbus_master_bist
    _count += cap_pp_csr_sta_pp_sbus_master_bist_t::s_get_width(); // sta_pp_sbus_master_bist
    _count += cap_pp_csr_cfg_pp_pcsd_control_t::s_get_width(); // cfg_pp_pcsd_control
    _count += cap_pp_csr_cfg_pp_sd_core_to_cntl_t::s_get_width(); // cfg_pp_sd_core_to_cntl
    _count += cap_pp_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_pp_csr_dhs_sbus_indir_t::s_get_width(); // dhs_sbus_indir
    _count += (cap_pxp_csr_t::s_get_width() * 8); // port_p
    _count += (cap_pxc_csr_t::s_get_width() * 8); // port_c
    return _count;
}

void cap_pp_csr_dhs_sbus_indir_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< data_cpp_int_t >()  ;
    _count += 32;
}

void cap_pp_csr_dhs_sbus_indir_t::all(const cpp_int & _val) {
    int _count = 0;

    entry.all( hlp.get_slc(_val, _count, _count -1 + entry.get_width() )); // entry
    _count += entry.get_width();
}

void cap_pp_csr_cfg_debug_port_t::all(const cpp_int & _val) {
    int _count = 0;

    // select
    int_var__select = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< select_cpp_int_t >()  ;
    _count += 4;
    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::all(const cpp_int & _val) {
    int _count = 0;

    // port0
    int_var__port0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port0_cpp_int_t >()  ;
    _count += 16;
    // port1
    int_var__port1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port1_cpp_int_t >()  ;
    _count += 16;
    // port2
    int_var__port2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port2_cpp_int_t >()  ;
    _count += 16;
    // port3
    int_var__port3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port3_cpp_int_t >()  ;
    _count += 16;
    // port4
    int_var__port4 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port4_cpp_int_t >()  ;
    _count += 16;
    // port5
    int_var__port5 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port5_cpp_int_t >()  ;
    _count += 16;
    // port6
    int_var__port6 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port6_cpp_int_t >()  ;
    _count += 16;
    // port7
    int_var__port7 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port7_cpp_int_t >()  ;
    _count += 16;
    // port8
    int_var__port8 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port8_cpp_int_t >()  ;
    _count += 16;
    // port9
    int_var__port9 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port9_cpp_int_t >()  ;
    _count += 16;
    // port10
    int_var__port10 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port10_cpp_int_t >()  ;
    _count += 16;
    // port11
    int_var__port11 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port11_cpp_int_t >()  ;
    _count += 16;
    // port12
    int_var__port12 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port12_cpp_int_t >()  ;
    _count += 16;
    // port13
    int_var__port13 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port13_cpp_int_t >()  ;
    _count += 16;
    // port14
    int_var__port14 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port14_cpp_int_t >()  ;
    _count += 16;
    // port15
    int_var__port15 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< port15_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_pcsd_control_t::all(const cpp_int & _val) {
    int _count = 0;

    // fts_align_grp_0
    int_var__fts_align_grp_0 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_0_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_0
    int_var__sris_en_grp_0 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_0_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_0
    int_var__rx8b10b_realign_grp_0 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_0_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_1
    int_var__fts_align_grp_1 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_1_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_1
    int_var__sris_en_grp_1 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_1_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_1
    int_var__rx8b10b_realign_grp_1 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_1_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_2
    int_var__fts_align_grp_2 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_2_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_2
    int_var__sris_en_grp_2 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_2_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_2
    int_var__rx8b10b_realign_grp_2 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_2_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_3
    int_var__fts_align_grp_3 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_3_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_3
    int_var__sris_en_grp_3 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_3_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_3
    int_var__rx8b10b_realign_grp_3 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_3_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_4
    int_var__fts_align_grp_4 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_4_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_4
    int_var__sris_en_grp_4 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_4_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_4
    int_var__rx8b10b_realign_grp_4 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_4_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_5
    int_var__fts_align_grp_5 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_5_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_5
    int_var__sris_en_grp_5 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_5_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_5
    int_var__rx8b10b_realign_grp_5 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_5_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_6
    int_var__fts_align_grp_6 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_6_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_6
    int_var__sris_en_grp_6 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_6_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_6
    int_var__rx8b10b_realign_grp_6 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_6_cpp_int_t >()  ;
    _count += 2;
    // fts_align_grp_7
    int_var__fts_align_grp_7 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fts_align_grp_7_cpp_int_t >()  ;
    _count += 2;
    // sris_en_grp_7
    int_var__sris_en_grp_7 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< sris_en_grp_7_cpp_int_t >()  ;
    _count += 2;
    // rx8b10b_realign_grp_7
    int_var__rx8b10b_realign_grp_7 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< rx8b10b_realign_grp_7_cpp_int_t >()  ;
    _count += 2;
}

void cap_pp_csr_sta_pp_sbus_master_bist_t::all(const cpp_int & _val) {
    int _count = 0;

    // fail
    int_var__fail = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fail_cpp_int_t >()  ;
    _count += 1;
    // pass
    int_var__pass = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pass_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_sbus_master_bist_t::all(const cpp_int & _val) {
    int _count = 0;

    // mode
    int_var__mode = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mode_cpp_int_t >()  ;
    _count += 1;
    // rst
    int_var__rst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< rst_cpp_int_t >()  ;
    _count += 1;
    // run
    int_var__run = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< run_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_sta_pp_sd_rdy_t::all(const cpp_int & _val) {
    int _count = 0;

    // rx_rdy
    int_var__rx_rdy = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< rx_rdy_cpp_int_t >()  ;
    _count += 16;
    // tx_rdy
    int_var__tx_rdy = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< tx_rdy_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_pp_sd_core_status_t::all(const cpp_int & _val) {
    int _count = 0;

    // lane0
    int_var__lane0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane0_cpp_int_t >()  ;
    _count += 16;
    // lane1
    int_var__lane1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane1_cpp_int_t >()  ;
    _count += 16;
    // lane2
    int_var__lane2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane2_cpp_int_t >()  ;
    _count += 16;
    // lane3
    int_var__lane3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane3_cpp_int_t >()  ;
    _count += 16;
    // lane4
    int_var__lane4 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane4_cpp_int_t >()  ;
    _count += 16;
    // lane5
    int_var__lane5 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane5_cpp_int_t >()  ;
    _count += 16;
    // lane6
    int_var__lane6 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane6_cpp_int_t >()  ;
    _count += 16;
    // lane7
    int_var__lane7 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane7_cpp_int_t >()  ;
    _count += 16;
    // lane8
    int_var__lane8 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane8_cpp_int_t >()  ;
    _count += 16;
    // lane9
    int_var__lane9 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane9_cpp_int_t >()  ;
    _count += 16;
    // lane10
    int_var__lane10 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane10_cpp_int_t >()  ;
    _count += 16;
    // lane11
    int_var__lane11 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane11_cpp_int_t >()  ;
    _count += 16;
    // lane12
    int_var__lane12 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane12_cpp_int_t >()  ;
    _count += 16;
    // lane13
    int_var__lane13 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane13_cpp_int_t >()  ;
    _count += 16;
    // lane14
    int_var__lane14 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane14_cpp_int_t >()  ;
    _count += 16;
    // lane15
    int_var__lane15 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane15_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_pp_sd_spico_gp_t::all(const cpp_int & _val) {
    int _count = 0;

    // out
    int_var__out = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< out_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_sd_spico_gp_t::all(const cpp_int & _val) {
    int _count = 0;

    // in
    int_var__in = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< in_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::all(const cpp_int & _val) {
    int _count = 0;

    // lane0
    int_var__lane0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane0_cpp_int_t >()  ;
    _count += 16;
    // lane1
    int_var__lane1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane1_cpp_int_t >()  ;
    _count += 16;
    // lane2
    int_var__lane2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane2_cpp_int_t >()  ;
    _count += 16;
    // lane3
    int_var__lane3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane3_cpp_int_t >()  ;
    _count += 16;
    // lane4
    int_var__lane4 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane4_cpp_int_t >()  ;
    _count += 16;
    // lane5
    int_var__lane5 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane5_cpp_int_t >()  ;
    _count += 16;
    // lane6
    int_var__lane6 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane6_cpp_int_t >()  ;
    _count += 16;
    // lane7
    int_var__lane7 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane7_cpp_int_t >()  ;
    _count += 16;
    // lane8
    int_var__lane8 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane8_cpp_int_t >()  ;
    _count += 16;
    // lane9
    int_var__lane9 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane9_cpp_int_t >()  ;
    _count += 16;
    // lane10
    int_var__lane10 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane10_cpp_int_t >()  ;
    _count += 16;
    // lane11
    int_var__lane11 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane11_cpp_int_t >()  ;
    _count += 16;
    // lane12
    int_var__lane12 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane12_cpp_int_t >()  ;
    _count += 16;
    // lane13
    int_var__lane13 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane13_cpp_int_t >()  ;
    _count += 16;
    // lane14
    int_var__lane14 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane14_cpp_int_t >()  ;
    _count += 16;
    // lane15
    int_var__lane15 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lane15_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::all(const cpp_int & _val) {
    int _count = 0;

    // per_lane
    int_var__per_lane = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< per_lane_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::all(const cpp_int & _val) {
    int _count = 0;

    // lanemask
    int_var__lanemask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lanemask_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_t::all(const cpp_int & _val) {
    int _count = 0;

    // code
    int_var__code = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< code_cpp_int_t >()  ;
    _count += 16;
    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< data_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_pp_pcie_pll_t::all(const cpp_int & _val) {
    int _count = 0;

    // pcie_0_dll_lock
    int_var__pcie_0_dll_lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pcie_0_dll_lock_cpp_int_t >()  ;
    _count += 1;
    // pcie_0_pll_lock
    int_var__pcie_0_pll_lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pcie_0_pll_lock_cpp_int_t >()  ;
    _count += 1;
    // pcie_1_dll_lock
    int_var__pcie_1_dll_lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pcie_1_dll_lock_cpp_int_t >()  ;
    _count += 1;
    // pcie_1_pll_lock
    int_var__pcie_1_pll_lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pcie_1_pll_lock_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_pcie_pll_1_t::all(const cpp_int & _val) {
    int _count = 0;

    // dll_disable_output_clk
    int_var__dll_disable_output_clk = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< dll_disable_output_clk_cpp_int_t >()  ;
    _count += 4;
    // dll_fbcnt
    int_var__dll_fbcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_fbcnt_cpp_int_t >()  ;
    _count += 6;
    // dll_out_divcnt
    int_var__dll_out_divcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_out_divcnt_cpp_int_t >()  ;
    _count += 6;
    // dll_refcnt
    int_var__dll_refcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_refcnt_cpp_int_t >()  ;
    _count += 6;
    // pll_disable_output_clk
    int_var__pll_disable_output_clk = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< pll_disable_output_clk_cpp_int_t >()  ;
    _count += 4;
    // pll_out_divcnt
    int_var__pll_out_divcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< pll_out_divcnt_cpp_int_t >()  ;
    _count += 6;
    // pll_refcnt
    int_var__pll_refcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< pll_refcnt_cpp_int_t >()  ;
    _count += 6;
    // prog_fbdiv255
    int_var__prog_fbdiv255 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< prog_fbdiv255_cpp_int_t >()  ;
    _count += 8;
    // prog_fbdiv_23
    int_var__prog_fbdiv_23 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< prog_fbdiv_23_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_pcie_pll_0_t::all(const cpp_int & _val) {
    int _count = 0;

    // dll_disable_output_clk
    int_var__dll_disable_output_clk = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< dll_disable_output_clk_cpp_int_t >()  ;
    _count += 4;
    // dll_fbcnt
    int_var__dll_fbcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_fbcnt_cpp_int_t >()  ;
    _count += 6;
    // dll_out_divcnt
    int_var__dll_out_divcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_out_divcnt_cpp_int_t >()  ;
    _count += 6;
    // dll_refcnt
    int_var__dll_refcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< dll_refcnt_cpp_int_t >()  ;
    _count += 6;
    // pll_disable_output_clk
    int_var__pll_disable_output_clk = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< pll_disable_output_clk_cpp_int_t >()  ;
    _count += 4;
    // pll_out_divcnt
    int_var__pll_out_divcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< pll_out_divcnt_cpp_int_t >()  ;
    _count += 6;
    // pll_refcnt
    int_var__pll_refcnt = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< pll_refcnt_cpp_int_t >()  ;
    _count += 6;
    // prog_fbdiv255
    int_var__prog_fbdiv255 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< prog_fbdiv255_cpp_int_t >()  ;
    _count += 8;
    // prog_fbdiv_23
    int_var__prog_fbdiv_23 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< prog_fbdiv_23_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::all(const cpp_int & _val) {
    int _count = 0;

    // p0
    int_var__p0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p0_cpp_int_t >()  ;
    _count += 1;
    // p1
    int_var__p1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p1_cpp_int_t >()  ;
    _count += 1;
    // p2
    int_var__p2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p2_cpp_int_t >()  ;
    _count += 1;
    // p3
    int_var__p3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p3_cpp_int_t >()  ;
    _count += 1;
    // p4
    int_var__p4 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p4_cpp_int_t >()  ;
    _count += 1;
    // p5
    int_var__p5 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p5_cpp_int_t >()  ;
    _count += 1;
    // p6
    int_var__p6 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p6_cpp_int_t >()  ;
    _count += 1;
    // p7
    int_var__p7 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p7_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::all(const cpp_int & _val) {
    int _count = 0;

    // pll_0
    int_var__pll_0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pll_0_cpp_int_t >()  ;
    _count += 1;
    // pll_1
    int_var__pll_1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pll_1_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_rc_perstn_t::all(const cpp_int & _val) {
    int _count = 0;

    // p0_out_en
    int_var__p0_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p0_out_en_cpp_int_t >()  ;
    _count += 1;
    // p0_out_data
    int_var__p0_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p0_out_data_cpp_int_t >()  ;
    _count += 1;
    // p1_out_en
    int_var__p1_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p1_out_en_cpp_int_t >()  ;
    _count += 1;
    // p1_out_data
    int_var__p1_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p1_out_data_cpp_int_t >()  ;
    _count += 1;
    // p2_out_en
    int_var__p2_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p2_out_en_cpp_int_t >()  ;
    _count += 1;
    // p2_out_data
    int_var__p2_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p2_out_data_cpp_int_t >()  ;
    _count += 1;
    // p3_out_en
    int_var__p3_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p3_out_en_cpp_int_t >()  ;
    _count += 1;
    // p3_out_data
    int_var__p3_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p3_out_data_cpp_int_t >()  ;
    _count += 1;
    // p4_out_en
    int_var__p4_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p4_out_en_cpp_int_t >()  ;
    _count += 1;
    // p4_out_data
    int_var__p4_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p4_out_data_cpp_int_t >()  ;
    _count += 1;
    // p5_out_en
    int_var__p5_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p5_out_en_cpp_int_t >()  ;
    _count += 1;
    // p5_out_data
    int_var__p5_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p5_out_data_cpp_int_t >()  ;
    _count += 1;
    // p6_out_en
    int_var__p6_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p6_out_en_cpp_int_t >()  ;
    _count += 1;
    // p6_out_data
    int_var__p6_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p6_out_data_cpp_int_t >()  ;
    _count += 1;
    // p7_out_en
    int_var__p7_out_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p7_out_en_cpp_int_t >()  ;
    _count += 1;
    // p7_out_data
    int_var__p7_out_data = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p7_out_data_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_sbus_t::all(const cpp_int & _val) {
    int _count = 0;

    // rom_en
    int_var__rom_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< rom_en_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_pcs_reset_n_t::all(const cpp_int & _val) {
    int _count = 0;

    // lanemask
    int_var__lanemask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lanemask_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_sd_async_reset_n_t::all(const cpp_int & _val) {
    int _count = 0;

    // lanemask
    int_var__lanemask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lanemask_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::all(const cpp_int & _val) {
    int _count = 0;

    // lanemask
    int_var__lanemask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< lanemask_cpp_int_t >()  ;
    _count += 16;
}

void cap_pp_csr_sta_sbus_indir_t::all(const cpp_int & _val) {
    int _count = 0;

    // execute
    int_var__execute = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< execute_cpp_int_t >()  ;
    _count += 1;
    // done
    int_var__done = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< done_cpp_int_t >()  ;
    _count += 1;
    // rcv_data_vld
    int_var__rcv_data_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< rcv_data_vld_cpp_int_t >()  ;
    _count += 1;
    // result_code
    int_var__result_code = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< result_code_cpp_int_t >()  ;
    _count += 3;
}

void cap_pp_csr_cfg_sbus_indir_t::all(const cpp_int & _val) {
    int _count = 0;

    // rcvr_addr
    int_var__rcvr_addr = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< rcvr_addr_cpp_int_t >()  ;
    _count += 8;
    // data_addr
    int_var__data_addr = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< data_addr_cpp_int_t >()  ;
    _count += 8;
    // command
    int_var__command = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< command_cpp_int_t >()  ;
    _count += 8;
}

void cap_pp_csr_cfg_sbus_result_t::all(const cpp_int & _val) {
    int _count = 0;

    // mode
    int_var__mode = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mode_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_sw_reset_t::all(const cpp_int & _val) {
    int _count = 0;

    // p0_hrst
    int_var__p0_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p0_hrst_cpp_int_t >()  ;
    _count += 1;
    // p0_srst
    int_var__p0_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p0_srst_cpp_int_t >()  ;
    _count += 1;
    // p1_hrst
    int_var__p1_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p1_hrst_cpp_int_t >()  ;
    _count += 1;
    // p1_srst
    int_var__p1_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p1_srst_cpp_int_t >()  ;
    _count += 1;
    // p2_hrst
    int_var__p2_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p2_hrst_cpp_int_t >()  ;
    _count += 1;
    // p2_srst
    int_var__p2_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p2_srst_cpp_int_t >()  ;
    _count += 1;
    // p3_hrst
    int_var__p3_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p3_hrst_cpp_int_t >()  ;
    _count += 1;
    // p3_srst
    int_var__p3_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p3_srst_cpp_int_t >()  ;
    _count += 1;
    // p4_hrst
    int_var__p4_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p4_hrst_cpp_int_t >()  ;
    _count += 1;
    // p4_srst
    int_var__p4_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p4_srst_cpp_int_t >()  ;
    _count += 1;
    // p5_hrst
    int_var__p5_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p5_hrst_cpp_int_t >()  ;
    _count += 1;
    // p5_srst
    int_var__p5_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p5_srst_cpp_int_t >()  ;
    _count += 1;
    // p6_hrst
    int_var__p6_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p6_hrst_cpp_int_t >()  ;
    _count += 1;
    // p6_srst
    int_var__p6_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p6_srst_cpp_int_t >()  ;
    _count += 1;
    // p7_hrst
    int_var__p7_hrst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p7_hrst_cpp_int_t >()  ;
    _count += 1;
    // p7_srst
    int_var__p7_srst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< p7_srst_cpp_int_t >()  ;
    _count += 1;
}

void cap_pp_csr_cfg_pp_linkwidth_t::all(const cpp_int & _val) {
    int _count = 0;

    // p0_lw
    int_var__p0_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p0_lw_cpp_int_t >()  ;
    _count += 2;
    // p1_lw
    int_var__p1_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p1_lw_cpp_int_t >()  ;
    _count += 2;
    // p2_lw
    int_var__p2_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p2_lw_cpp_int_t >()  ;
    _count += 2;
    // p3_lw
    int_var__p3_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p3_lw_cpp_int_t >()  ;
    _count += 2;
    // p4_lw
    int_var__p4_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p4_lw_cpp_int_t >()  ;
    _count += 2;
    // p5_lw
    int_var__p5_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p5_lw_cpp_int_t >()  ;
    _count += 2;
    // p6_lw
    int_var__p6_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p6_lw_cpp_int_t >()  ;
    _count += 2;
    // p7_lw
    int_var__p7_lw = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< p7_lw_cpp_int_t >()  ;
    _count += 2;
}

void cap_pp_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_pp_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_pp_linkwidth.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_linkwidth.get_width() )); // cfg_pp_linkwidth
    _count += cfg_pp_linkwidth.get_width();
    cfg_pp_sw_reset.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sw_reset.get_width() )); // cfg_pp_sw_reset
    _count += cfg_pp_sw_reset.get_width();
    cfg_sbus_result.all( hlp.get_slc(_val, _count, _count -1 + cfg_sbus_result.get_width() )); // cfg_sbus_result
    _count += cfg_sbus_result.get_width();
    cfg_sbus_indir.all( hlp.get_slc(_val, _count, _count -1 + cfg_sbus_indir.get_width() )); // cfg_sbus_indir
    _count += cfg_sbus_indir.get_width();
    sta_sbus_indir.all( hlp.get_slc(_val, _count, _count -1 + sta_sbus_indir.get_width() )); // sta_sbus_indir
    _count += sta_sbus_indir.get_width();
    cfg_pp_pcs_interrupt_disable.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcs_interrupt_disable.get_width() )); // cfg_pp_pcs_interrupt_disable
    _count += cfg_pp_pcs_interrupt_disable.get_width();
    cfg_pp_sd_async_reset_n.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sd_async_reset_n.get_width() )); // cfg_pp_sd_async_reset_n
    _count += cfg_pp_sd_async_reset_n.get_width();
    cfg_pp_pcs_reset_n.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcs_reset_n.get_width() )); // cfg_pp_pcs_reset_n
    _count += cfg_pp_pcs_reset_n.get_width();
    cfg_pp_sbus.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sbus.get_width() )); // cfg_pp_sbus
    _count += cfg_pp_sbus.get_width();
    cfg_pp_rc_perstn.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_rc_perstn.get_width() )); // cfg_pp_rc_perstn
    _count += cfg_pp_rc_perstn.get_width();
    cfg_pp_pcie_pll_rst_n.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcie_pll_rst_n.get_width() )); // cfg_pp_pcie_pll_rst_n
    _count += cfg_pp_pcie_pll_rst_n.get_width();
    cfg_pp_pcie_pll_refclk_sel.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcie_pll_refclk_sel.get_width() )); // cfg_pp_pcie_pll_refclk_sel
    _count += cfg_pp_pcie_pll_refclk_sel.get_width();
    cfg_pp_pcie_pll_0.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcie_pll_0.get_width() )); // cfg_pp_pcie_pll_0
    _count += cfg_pp_pcie_pll_0.get_width();
    cfg_pp_pcie_pll_1.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcie_pll_1.get_width() )); // cfg_pp_pcie_pll_1
    _count += cfg_pp_pcie_pll_1.get_width();
    sta_pp_pcie_pll.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_pcie_pll.get_width() )); // sta_pp_pcie_pll
    _count += sta_pp_pcie_pll.get_width();
    // cfg_pp_pcsd_interrupt
    for(int ii = 0; ii < 16; ii++) {
        cfg_pp_pcsd_interrupt[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcsd_interrupt[ii].get_width()));
        _count += cfg_pp_pcsd_interrupt[ii].get_width();
    }
    cfg_pp_pcsd_interrupt_request.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcsd_interrupt_request.get_width() )); // cfg_pp_pcsd_interrupt_request
    _count += cfg_pp_pcsd_interrupt_request.get_width();
    sta_pp_pcsd_interrupt_in_progress.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_pcsd_interrupt_in_progress.get_width() )); // sta_pp_pcsd_interrupt_in_progress
    _count += sta_pp_pcsd_interrupt_in_progress.get_width();
    sta_pp_pcsd_interrupt_data_out.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_pcsd_interrupt_data_out.get_width() )); // sta_pp_pcsd_interrupt_data_out
    _count += sta_pp_pcsd_interrupt_data_out.get_width();
    cfg_pp_sd_spico_gp.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sd_spico_gp.get_width() )); // cfg_pp_sd_spico_gp
    _count += cfg_pp_sd_spico_gp.get_width();
    sta_pp_sd_spico_gp.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_sd_spico_gp.get_width() )); // sta_pp_sd_spico_gp
    _count += sta_pp_sd_spico_gp.get_width();
    sta_pp_sd_core_status.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_sd_core_status.get_width() )); // sta_pp_sd_core_status
    _count += sta_pp_sd_core_status.get_width();
    sta_pp_sd_rdy.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_sd_rdy.get_width() )); // sta_pp_sd_rdy
    _count += sta_pp_sd_rdy.get_width();
    cfg_pp_sbus_master_bist.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sbus_master_bist.get_width() )); // cfg_pp_sbus_master_bist
    _count += cfg_pp_sbus_master_bist.get_width();
    sta_pp_sbus_master_bist.all( hlp.get_slc(_val, _count, _count -1 + sta_pp_sbus_master_bist.get_width() )); // sta_pp_sbus_master_bist
    _count += sta_pp_sbus_master_bist.get_width();
    cfg_pp_pcsd_control.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_pcsd_control.get_width() )); // cfg_pp_pcsd_control
    _count += cfg_pp_pcsd_control.get_width();
    cfg_pp_sd_core_to_cntl.all( hlp.get_slc(_val, _count, _count -1 + cfg_pp_sd_core_to_cntl.get_width() )); // cfg_pp_sd_core_to_cntl
    _count += cfg_pp_sd_core_to_cntl.get_width();
    cfg_debug_port.all( hlp.get_slc(_val, _count, _count -1 + cfg_debug_port.get_width() )); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    dhs_sbus_indir.all( hlp.get_slc(_val, _count, _count -1 + dhs_sbus_indir.get_width() )); // dhs_sbus_indir
    _count += dhs_sbus_indir.get_width();
    // port_p
    for(int ii = 0; ii < 8; ii++) {
        port_p[ii].all( hlp.get_slc(_val, _count, _count -1 + port_p[ii].get_width()));
        _count += port_p[ii].get_width();
    }
    // port_c
    for(int ii = 0; ii < 8; ii++) {
        port_c[ii].all( hlp.get_slc(_val, _count, _count -1 + port_c[ii].get_width()));
        _count += port_c[ii].get_width();
    }
}

cpp_int cap_pp_csr_dhs_sbus_indir_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pp_csr_dhs_sbus_indir_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, entry.all() , _count, _count -1 + entry.get_width() ); // entry
    _count += entry.get_width();
    return ret_val;
}

cpp_int cap_pp_csr_cfg_debug_port_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // select
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__select) , _count, _count -1 + 4 );
    _count += 4;
    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // port0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port0) , _count, _count -1 + 16 );
    _count += 16;
    // port1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port1) , _count, _count -1 + 16 );
    _count += 16;
    // port2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port2) , _count, _count -1 + 16 );
    _count += 16;
    // port3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port3) , _count, _count -1 + 16 );
    _count += 16;
    // port4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port4) , _count, _count -1 + 16 );
    _count += 16;
    // port5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port5) , _count, _count -1 + 16 );
    _count += 16;
    // port6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port6) , _count, _count -1 + 16 );
    _count += 16;
    // port7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port7) , _count, _count -1 + 16 );
    _count += 16;
    // port8
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port8) , _count, _count -1 + 16 );
    _count += 16;
    // port9
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port9) , _count, _count -1 + 16 );
    _count += 16;
    // port10
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port10) , _count, _count -1 + 16 );
    _count += 16;
    // port11
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port11) , _count, _count -1 + 16 );
    _count += 16;
    // port12
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port12) , _count, _count -1 + 16 );
    _count += 16;
    // port13
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port13) , _count, _count -1 + 16 );
    _count += 16;
    // port14
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port14) , _count, _count -1 + 16 );
    _count += 16;
    // port15
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__port15) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // fts_align_grp_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_0) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_0) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_0) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_1) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_1) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_1) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_2) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_2) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_2) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_3) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_3) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_3) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_4) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_4) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_4) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_5) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_5) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_5) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_6) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_6) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_6) , _count, _count -1 + 2 );
    _count += 2;
    // fts_align_grp_7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fts_align_grp_7) , _count, _count -1 + 2 );
    _count += 2;
    // sris_en_grp_7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sris_en_grp_7) , _count, _count -1 + 2 );
    _count += 2;
    // rx8b10b_realign_grp_7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx8b10b_realign_grp_7) , _count, _count -1 + 2 );
    _count += 2;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_sbus_master_bist_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // fail
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fail) , _count, _count -1 + 1 );
    _count += 1;
    // pass
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pass) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sbus_master_bist_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // mode
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mode) , _count, _count -1 + 1 );
    _count += 1;
    // rst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rst) , _count, _count -1 + 1 );
    _count += 1;
    // run
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__run) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_sd_rdy_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // rx_rdy
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rx_rdy) , _count, _count -1 + 16 );
    _count += 16;
    // tx_rdy
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tx_rdy) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lane0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane0) , _count, _count -1 + 16 );
    _count += 16;
    // lane1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane1) , _count, _count -1 + 16 );
    _count += 16;
    // lane2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane2) , _count, _count -1 + 16 );
    _count += 16;
    // lane3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane3) , _count, _count -1 + 16 );
    _count += 16;
    // lane4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane4) , _count, _count -1 + 16 );
    _count += 16;
    // lane5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane5) , _count, _count -1 + 16 );
    _count += 16;
    // lane6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane6) , _count, _count -1 + 16 );
    _count += 16;
    // lane7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane7) , _count, _count -1 + 16 );
    _count += 16;
    // lane8
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane8) , _count, _count -1 + 16 );
    _count += 16;
    // lane9
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane9) , _count, _count -1 + 16 );
    _count += 16;
    // lane10
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane10) , _count, _count -1 + 16 );
    _count += 16;
    // lane11
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane11) , _count, _count -1 + 16 );
    _count += 16;
    // lane12
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane12) , _count, _count -1 + 16 );
    _count += 16;
    // lane13
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane13) , _count, _count -1 + 16 );
    _count += 16;
    // lane14
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane14) , _count, _count -1 + 16 );
    _count += 16;
    // lane15
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane15) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_sd_spico_gp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // out
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__out) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sd_spico_gp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // in
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__in) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lane0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane0) , _count, _count -1 + 16 );
    _count += 16;
    // lane1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane1) , _count, _count -1 + 16 );
    _count += 16;
    // lane2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane2) , _count, _count -1 + 16 );
    _count += 16;
    // lane3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane3) , _count, _count -1 + 16 );
    _count += 16;
    // lane4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane4) , _count, _count -1 + 16 );
    _count += 16;
    // lane5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane5) , _count, _count -1 + 16 );
    _count += 16;
    // lane6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane6) , _count, _count -1 + 16 );
    _count += 16;
    // lane7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane7) , _count, _count -1 + 16 );
    _count += 16;
    // lane8
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane8) , _count, _count -1 + 16 );
    _count += 16;
    // lane9
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane9) , _count, _count -1 + 16 );
    _count += 16;
    // lane10
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane10) , _count, _count -1 + 16 );
    _count += 16;
    // lane11
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane11) , _count, _count -1 + 16 );
    _count += 16;
    // lane12
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane12) , _count, _count -1 + 16 );
    _count += 16;
    // lane13
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane13) , _count, _count -1 + 16 );
    _count += 16;
    // lane14
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane14) , _count, _count -1 + 16 );
    _count += 16;
    // lane15
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lane15) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // per_lane
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__per_lane) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lanemask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lanemask) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcsd_interrupt_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // code
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__code) , _count, _count -1 + 16 );
    _count += 16;
    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_pp_pcie_pll_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // pcie_0_dll_lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pcie_0_dll_lock) , _count, _count -1 + 1 );
    _count += 1;
    // pcie_0_pll_lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pcie_0_pll_lock) , _count, _count -1 + 1 );
    _count += 1;
    // pcie_1_dll_lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pcie_1_dll_lock) , _count, _count -1 + 1 );
    _count += 1;
    // pcie_1_pll_lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pcie_1_pll_lock) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // dll_disable_output_clk
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_disable_output_clk) , _count, _count -1 + 4 );
    _count += 4;
    // dll_fbcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_fbcnt) , _count, _count -1 + 6 );
    _count += 6;
    // dll_out_divcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_out_divcnt) , _count, _count -1 + 6 );
    _count += 6;
    // dll_refcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_refcnt) , _count, _count -1 + 6 );
    _count += 6;
    // pll_disable_output_clk
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_disable_output_clk) , _count, _count -1 + 4 );
    _count += 4;
    // pll_out_divcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_out_divcnt) , _count, _count -1 + 6 );
    _count += 6;
    // pll_refcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_refcnt) , _count, _count -1 + 6 );
    _count += 6;
    // prog_fbdiv255
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prog_fbdiv255) , _count, _count -1 + 8 );
    _count += 8;
    // prog_fbdiv_23
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prog_fbdiv_23) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // dll_disable_output_clk
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_disable_output_clk) , _count, _count -1 + 4 );
    _count += 4;
    // dll_fbcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_fbcnt) , _count, _count -1 + 6 );
    _count += 6;
    // dll_out_divcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_out_divcnt) , _count, _count -1 + 6 );
    _count += 6;
    // dll_refcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dll_refcnt) , _count, _count -1 + 6 );
    _count += 6;
    // pll_disable_output_clk
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_disable_output_clk) , _count, _count -1 + 4 );
    _count += 4;
    // pll_out_divcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_out_divcnt) , _count, _count -1 + 6 );
    _count += 6;
    // pll_refcnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_refcnt) , _count, _count -1 + 6 );
    _count += 6;
    // prog_fbdiv255
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prog_fbdiv255) , _count, _count -1 + 8 );
    _count += 8;
    // prog_fbdiv_23
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prog_fbdiv_23) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // p0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0) , _count, _count -1 + 1 );
    _count += 1;
    // p1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1) , _count, _count -1 + 1 );
    _count += 1;
    // p2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2) , _count, _count -1 + 1 );
    _count += 1;
    // p3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3) , _count, _count -1 + 1 );
    _count += 1;
    // p4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4) , _count, _count -1 + 1 );
    _count += 1;
    // p5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5) , _count, _count -1 + 1 );
    _count += 1;
    // p6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6) , _count, _count -1 + 1 );
    _count += 1;
    // p7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // pll_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_0) , _count, _count -1 + 1 );
    _count += 1;
    // pll_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pll_1) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // p0_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p0_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p1_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p1_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p2_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p2_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p3_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p3_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p4_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p4_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p5_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p5_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p6_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p6_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6_out_data) , _count, _count -1 + 1 );
    _count += 1;
    // p7_out_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7_out_en) , _count, _count -1 + 1 );
    _count += 1;
    // p7_out_data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7_out_data) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sbus_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // rom_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rom_en) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcs_reset_n_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lanemask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lanemask) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sd_async_reset_n_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lanemask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lanemask) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lanemask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lanemask) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_pp_csr_sta_sbus_indir_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // execute
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__execute) , _count, _count -1 + 1 );
    _count += 1;
    // done
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__done) , _count, _count -1 + 1 );
    _count += 1;
    // rcv_data_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rcv_data_vld) , _count, _count -1 + 1 );
    _count += 1;
    // result_code
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__result_code) , _count, _count -1 + 3 );
    _count += 3;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_sbus_indir_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // rcvr_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rcvr_addr) , _count, _count -1 + 8 );
    _count += 8;
    // data_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data_addr) , _count, _count -1 + 8 );
    _count += 8;
    // command
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__command) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_sbus_result_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // mode
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mode) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // p0_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p0_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p1_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p1_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p2_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p2_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p3_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p3_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p4_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p4_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p5_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p5_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p6_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p6_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6_srst) , _count, _count -1 + 1 );
    _count += 1;
    // p7_hrst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7_hrst) , _count, _count -1 + 1 );
    _count += 1;
    // p7_srst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7_srst) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // p0_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p0_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p1_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p1_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p2_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p2_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p3_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p3_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p4_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p4_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p5_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p5_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p6_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p6_lw) , _count, _count -1 + 2 );
    _count += 2;
    // p7_lw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__p7_lw) , _count, _count -1 + 2 );
    _count += 2;
    return ret_val;
}

cpp_int cap_pp_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pp_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_linkwidth.all() , _count, _count -1 + cfg_pp_linkwidth.get_width() ); // cfg_pp_linkwidth
    _count += cfg_pp_linkwidth.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sw_reset.all() , _count, _count -1 + cfg_pp_sw_reset.get_width() ); // cfg_pp_sw_reset
    _count += cfg_pp_sw_reset.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_sbus_result.all() , _count, _count -1 + cfg_sbus_result.get_width() ); // cfg_sbus_result
    _count += cfg_sbus_result.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_sbus_indir.all() , _count, _count -1 + cfg_sbus_indir.get_width() ); // cfg_sbus_indir
    _count += cfg_sbus_indir.get_width();
    ret_val = hlp.set_slc(ret_val, sta_sbus_indir.all() , _count, _count -1 + sta_sbus_indir.get_width() ); // sta_sbus_indir
    _count += sta_sbus_indir.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcs_interrupt_disable.all() , _count, _count -1 + cfg_pp_pcs_interrupt_disable.get_width() ); // cfg_pp_pcs_interrupt_disable
    _count += cfg_pp_pcs_interrupt_disable.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sd_async_reset_n.all() , _count, _count -1 + cfg_pp_sd_async_reset_n.get_width() ); // cfg_pp_sd_async_reset_n
    _count += cfg_pp_sd_async_reset_n.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcs_reset_n.all() , _count, _count -1 + cfg_pp_pcs_reset_n.get_width() ); // cfg_pp_pcs_reset_n
    _count += cfg_pp_pcs_reset_n.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sbus.all() , _count, _count -1 + cfg_pp_sbus.get_width() ); // cfg_pp_sbus
    _count += cfg_pp_sbus.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_rc_perstn.all() , _count, _count -1 + cfg_pp_rc_perstn.get_width() ); // cfg_pp_rc_perstn
    _count += cfg_pp_rc_perstn.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcie_pll_rst_n.all() , _count, _count -1 + cfg_pp_pcie_pll_rst_n.get_width() ); // cfg_pp_pcie_pll_rst_n
    _count += cfg_pp_pcie_pll_rst_n.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcie_pll_refclk_sel.all() , _count, _count -1 + cfg_pp_pcie_pll_refclk_sel.get_width() ); // cfg_pp_pcie_pll_refclk_sel
    _count += cfg_pp_pcie_pll_refclk_sel.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcie_pll_0.all() , _count, _count -1 + cfg_pp_pcie_pll_0.get_width() ); // cfg_pp_pcie_pll_0
    _count += cfg_pp_pcie_pll_0.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcie_pll_1.all() , _count, _count -1 + cfg_pp_pcie_pll_1.get_width() ); // cfg_pp_pcie_pll_1
    _count += cfg_pp_pcie_pll_1.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_pcie_pll.all() , _count, _count -1 + sta_pp_pcie_pll.get_width() ); // sta_pp_pcie_pll
    _count += sta_pp_pcie_pll.get_width();
    // cfg_pp_pcsd_interrupt
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_pp_pcsd_interrupt[ii].all() , _count, _count -1 + cfg_pp_pcsd_interrupt[ii].get_width() );
        _count += cfg_pp_pcsd_interrupt[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcsd_interrupt_request.all() , _count, _count -1 + cfg_pp_pcsd_interrupt_request.get_width() ); // cfg_pp_pcsd_interrupt_request
    _count += cfg_pp_pcsd_interrupt_request.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_pcsd_interrupt_in_progress.all() , _count, _count -1 + sta_pp_pcsd_interrupt_in_progress.get_width() ); // sta_pp_pcsd_interrupt_in_progress
    _count += sta_pp_pcsd_interrupt_in_progress.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_pcsd_interrupt_data_out.all() , _count, _count -1 + sta_pp_pcsd_interrupt_data_out.get_width() ); // sta_pp_pcsd_interrupt_data_out
    _count += sta_pp_pcsd_interrupt_data_out.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sd_spico_gp.all() , _count, _count -1 + cfg_pp_sd_spico_gp.get_width() ); // cfg_pp_sd_spico_gp
    _count += cfg_pp_sd_spico_gp.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_sd_spico_gp.all() , _count, _count -1 + sta_pp_sd_spico_gp.get_width() ); // sta_pp_sd_spico_gp
    _count += sta_pp_sd_spico_gp.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_sd_core_status.all() , _count, _count -1 + sta_pp_sd_core_status.get_width() ); // sta_pp_sd_core_status
    _count += sta_pp_sd_core_status.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_sd_rdy.all() , _count, _count -1 + sta_pp_sd_rdy.get_width() ); // sta_pp_sd_rdy
    _count += sta_pp_sd_rdy.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sbus_master_bist.all() , _count, _count -1 + cfg_pp_sbus_master_bist.get_width() ); // cfg_pp_sbus_master_bist
    _count += cfg_pp_sbus_master_bist.get_width();
    ret_val = hlp.set_slc(ret_val, sta_pp_sbus_master_bist.all() , _count, _count -1 + sta_pp_sbus_master_bist.get_width() ); // sta_pp_sbus_master_bist
    _count += sta_pp_sbus_master_bist.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_pcsd_control.all() , _count, _count -1 + cfg_pp_pcsd_control.get_width() ); // cfg_pp_pcsd_control
    _count += cfg_pp_pcsd_control.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_pp_sd_core_to_cntl.all() , _count, _count -1 + cfg_pp_sd_core_to_cntl.get_width() ); // cfg_pp_sd_core_to_cntl
    _count += cfg_pp_sd_core_to_cntl.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_debug_port.all() , _count, _count -1 + cfg_debug_port.get_width() ); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_sbus_indir.all() , _count, _count -1 + dhs_sbus_indir.get_width() ); // dhs_sbus_indir
    _count += dhs_sbus_indir.get_width();
    // port_p
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, port_p[ii].all() , _count, _count -1 + port_p[ii].get_width() );
        _count += port_p[ii].get_width();
    }
    // port_c
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, port_c[ii].all() , _count, _count -1 + port_c[ii].get_width() );
        _count += port_c[ii].get_width();
    }
    return ret_val;
}

void cap_pp_csr_dhs_sbus_indir_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_pp_csr_dhs_sbus_indir_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_pp_csr_dhs_sbus_indir_entry_t::data);
        }
        #endif
    
}

void cap_pp_csr_dhs_sbus_indir_t::init() {

    entry.set_attributes(this,"entry", 0x0 );
}

void cap_pp_csr_cfg_debug_port_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("select", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_debug_port_t::select);
            register_get_func("select", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_debug_port_t::select);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_debug_port_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_debug_port_t::enable);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port0);
            register_get_func("port0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port1);
            register_get_func("port1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port2", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port2);
            register_get_func("port2", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port3", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port3);
            register_get_func("port3", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port4", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port4);
            register_get_func("port4", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port5", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port5);
            register_get_func("port5", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port6", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port6);
            register_get_func("port6", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port7", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port7);
            register_get_func("port7", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port8", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port8);
            register_get_func("port8", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port8);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port9", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port9);
            register_get_func("port9", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port9);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port10", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port10);
            register_get_func("port10", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port10);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port11", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port11);
            register_get_func("port11", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port11);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port12", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port12);
            register_get_func("port12", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port12);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port13", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port13);
            register_get_func("port13", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port13);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port14", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port14);
            register_get_func("port14", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port14);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("port15", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port15);
            register_get_func("port15", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port15);
        }
        #endif
    
    set_reset_val(cpp_int("0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    all(get_reset_val());
}

void cap_pp_csr_cfg_pp_pcsd_control_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_0);
            register_get_func("fts_align_grp_0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_0);
            register_get_func("sris_en_grp_0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_0);
            register_get_func("rx8b10b_realign_grp_0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_1);
            register_get_func("fts_align_grp_1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_1);
            register_get_func("sris_en_grp_1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_1);
            register_get_func("rx8b10b_realign_grp_1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_2", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_2);
            register_get_func("fts_align_grp_2", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_2", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_2);
            register_get_func("sris_en_grp_2", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_2", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_2);
            register_get_func("rx8b10b_realign_grp_2", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_3", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_3);
            register_get_func("fts_align_grp_3", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_3", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_3);
            register_get_func("sris_en_grp_3", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_3", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_3);
            register_get_func("rx8b10b_realign_grp_3", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_4", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_4);
            register_get_func("fts_align_grp_4", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_4", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_4);
            register_get_func("sris_en_grp_4", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_4", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_4);
            register_get_func("rx8b10b_realign_grp_4", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_5", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_5);
            register_get_func("fts_align_grp_5", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_5", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_5);
            register_get_func("sris_en_grp_5", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_5", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_5);
            register_get_func("rx8b10b_realign_grp_5", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_6", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_6);
            register_get_func("fts_align_grp_6", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_6", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_6);
            register_get_func("sris_en_grp_6", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_6", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_6);
            register_get_func("rx8b10b_realign_grp_6", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fts_align_grp_7", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_7);
            register_get_func("fts_align_grp_7", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sris_en_grp_7", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_7);
            register_get_func("sris_en_grp_7", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx8b10b_realign_grp_7", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_7);
            register_get_func("rx8b10b_realign_grp_7", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_7);
        }
        #endif
    
    set_reset_val(cpp_int("0xcf3cf3cf3cf3"));
    all(get_reset_val());
}

void cap_pp_csr_sta_pp_sbus_master_bist_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fail", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sbus_master_bist_t::fail);
            register_get_func("fail", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sbus_master_bist_t::fail);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pass", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sbus_master_bist_t::pass);
            register_get_func("pass", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sbus_master_bist_t::pass);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_sbus_master_bist_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mode", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::mode);
            register_get_func("mode", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::mode);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::rst);
            register_get_func("rst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::rst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("run", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::run);
            register_get_func("run", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sbus_master_bist_t::run);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_sd_rdy_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rx_rdy", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_rdy_t::rx_rdy);
            register_get_func("rx_rdy", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_rdy_t::rx_rdy);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tx_rdy", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_rdy_t::tx_rdy);
            register_get_func("tx_rdy", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_rdy_t::tx_rdy);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_sd_core_status_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane0", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane0);
            register_get_func("lane0", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane1", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane1);
            register_get_func("lane1", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane2", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane2);
            register_get_func("lane2", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane3", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane3);
            register_get_func("lane3", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane4", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane4);
            register_get_func("lane4", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane5", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane5);
            register_get_func("lane5", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane6", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane6);
            register_get_func("lane6", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane7", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane7);
            register_get_func("lane7", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane8", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane8);
            register_get_func("lane8", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane8);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane9", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane9);
            register_get_func("lane9", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane9);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane10", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane10);
            register_get_func("lane10", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane10);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane11", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane11);
            register_get_func("lane11", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane11);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane12", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane12);
            register_get_func("lane12", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane12);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane13", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane13);
            register_get_func("lane13", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane13);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane14", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane14);
            register_get_func("lane14", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane14);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane15", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane15);
            register_get_func("lane15", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_core_status_t::lane15);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_sd_spico_gp_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("out", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_sd_spico_gp_t::out);
            register_get_func("out", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_sd_spico_gp_t::out);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_sd_spico_gp_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("in", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_spico_gp_t::in);
            register_get_func("in", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_spico_gp_t::in);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane0", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane0);
            register_get_func("lane0", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane1", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane1);
            register_get_func("lane1", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane2", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane2);
            register_get_func("lane2", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane3", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane3);
            register_get_func("lane3", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane4", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane4);
            register_get_func("lane4", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane5", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane5);
            register_get_func("lane5", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane6", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane6);
            register_get_func("lane6", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane7", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane7);
            register_get_func("lane7", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane8", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane8);
            register_get_func("lane8", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane8);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane9", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane9);
            register_get_func("lane9", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane9);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane10", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane10);
            register_get_func("lane10", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane10);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane11", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane11);
            register_get_func("lane11", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane11);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane12", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane12);
            register_get_func("lane12", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane12);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane13", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane13);
            register_get_func("lane13", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane13);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane14", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane14);
            register_get_func("lane14", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane14);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lane15", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane15);
            register_get_func("lane15", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane15);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("per_lane", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::per_lane);
            register_get_func("per_lane", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::per_lane);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lanemask", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::lanemask);
            register_get_func("lanemask", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::lanemask);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcsd_interrupt_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("code", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_t::code);
            register_get_func("code", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_t::code);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcsd_interrupt_t::data);
        }
        #endif
    
}

void cap_pp_csr_sta_pp_pcie_pll_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pcie_0_dll_lock", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_dll_lock);
            register_get_func("pcie_0_dll_lock", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_dll_lock);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pcie_0_pll_lock", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_pll_lock);
            register_get_func("pcie_0_pll_lock", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_pll_lock);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pcie_1_dll_lock", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_dll_lock);
            register_get_func("pcie_1_dll_lock", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_dll_lock);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pcie_1_pll_lock", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_pll_lock);
            register_get_func("pcie_1_pll_lock", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_pll_lock);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcie_pll_1_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_disable_output_clk", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_disable_output_clk);
            register_get_func("dll_disable_output_clk", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_disable_output_clk);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_fbcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_fbcnt);
            register_get_func("dll_fbcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_fbcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_out_divcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_out_divcnt);
            register_get_func("dll_out_divcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_out_divcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_refcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_refcnt);
            register_get_func("dll_refcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_refcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_disable_output_clk", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_disable_output_clk);
            register_get_func("pll_disable_output_clk", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_disable_output_clk);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_out_divcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_out_divcnt);
            register_get_func("pll_out_divcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_out_divcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_refcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_refcnt);
            register_get_func("pll_refcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_refcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("prog_fbdiv255", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv255);
            register_get_func("prog_fbdiv255", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv255);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("prog_fbdiv_23", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv_23);
            register_get_func("prog_fbdiv_23", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv_23);
        }
        #endif
    
    set_reset_val(cpp_int("0x641140114a0"));
    all(get_reset_val());
}

void cap_pp_csr_cfg_pp_pcie_pll_0_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_disable_output_clk", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_disable_output_clk);
            register_get_func("dll_disable_output_clk", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_disable_output_clk);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_fbcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_fbcnt);
            register_get_func("dll_fbcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_fbcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_out_divcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_out_divcnt);
            register_get_func("dll_out_divcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_out_divcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dll_refcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_refcnt);
            register_get_func("dll_refcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_refcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_disable_output_clk", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_disable_output_clk);
            register_get_func("pll_disable_output_clk", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_disable_output_clk);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_out_divcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_out_divcnt);
            register_get_func("pll_out_divcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_out_divcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_refcnt", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_refcnt);
            register_get_func("pll_refcnt", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_refcnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("prog_fbdiv255", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv255);
            register_get_func("prog_fbdiv255", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv255);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("prog_fbdiv_23", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv_23);
            register_get_func("prog_fbdiv_23", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv_23);
        }
        #endif
    
    set_reset_val(cpp_int("0x641140114a0"));
    all(get_reset_val());
}

void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p0);
            register_get_func("p0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p1);
            register_get_func("p1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p2);
            register_get_func("p2", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p3);
            register_get_func("p3", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p4);
            register_get_func("p4", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p5);
            register_get_func("p5", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p6);
            register_get_func("p6", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p7);
            register_get_func("p7", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p7);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_0", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_0);
            register_get_func("pll_0", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pll_1", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_1);
            register_get_func("pll_1", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_1);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_rc_perstn_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_en);
            register_get_func("p0_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_data);
            register_get_func("p0_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_en);
            register_get_func("p1_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_data);
            register_get_func("p1_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_en);
            register_get_func("p2_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_data);
            register_get_func("p2_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_en);
            register_get_func("p3_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_data);
            register_get_func("p3_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_en);
            register_get_func("p4_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_data);
            register_get_func("p4_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_en);
            register_get_func("p5_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_data);
            register_get_func("p5_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_en);
            register_get_func("p6_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_data);
            register_get_func("p6_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7_out_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_en);
            register_get_func("p7_out_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7_out_data", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_data);
            register_get_func("p7_out_data", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_data);
        }
        #endif
    
    set_reset_val(cpp_int("0xaaaa"));
    all(get_reset_val());
}

void cap_pp_csr_cfg_pp_sbus_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rom_en", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sbus_t::rom_en);
            register_get_func("rom_en", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sbus_t::rom_en);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcs_reset_n_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lanemask", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcs_reset_n_t::lanemask);
            register_get_func("lanemask", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcs_reset_n_t::lanemask);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_sd_async_reset_n_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lanemask", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sd_async_reset_n_t::lanemask);
            register_get_func("lanemask", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sd_async_reset_n_t::lanemask);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lanemask", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::lanemask);
            register_get_func("lanemask", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::lanemask);
        }
        #endif
    
    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_pp_csr_sta_sbus_indir_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("execute", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_sbus_indir_t::execute);
            register_get_func("execute", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_sbus_indir_t::execute);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("done", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_sbus_indir_t::done);
            register_get_func("done", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_sbus_indir_t::done);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rcv_data_vld", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_sbus_indir_t::rcv_data_vld);
            register_get_func("rcv_data_vld", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_sbus_indir_t::rcv_data_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("result_code", (cap_csr_base::set_function_type_t)&cap_pp_csr_sta_sbus_indir_t::result_code);
            register_get_func("result_code", (cap_csr_base::get_function_type_t)&cap_pp_csr_sta_sbus_indir_t::result_code);
        }
        #endif
    
}

void cap_pp_csr_cfg_sbus_indir_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rcvr_addr", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::rcvr_addr);
            register_get_func("rcvr_addr", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::rcvr_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data_addr", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::data_addr);
            register_get_func("data_addr", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::data_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("command", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::command);
            register_get_func("command", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_sbus_indir_t::command);
        }
        #endif
    
}

void cap_pp_csr_cfg_sbus_result_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mode", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_sbus_result_t::mode);
            register_get_func("mode", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_sbus_result_t::mode);
        }
        #endif
    
}

void cap_pp_csr_cfg_pp_sw_reset_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p0_hrst);
            register_get_func("p0_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p0_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p0_srst);
            register_get_func("p0_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p0_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p1_hrst);
            register_get_func("p1_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p1_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p1_srst);
            register_get_func("p1_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p1_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p2_hrst);
            register_get_func("p2_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p2_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p2_srst);
            register_get_func("p2_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p2_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p3_hrst);
            register_get_func("p3_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p3_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p3_srst);
            register_get_func("p3_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p3_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p4_hrst);
            register_get_func("p4_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p4_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p4_srst);
            register_get_func("p4_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p4_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p5_hrst);
            register_get_func("p5_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p5_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p5_srst);
            register_get_func("p5_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p5_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p6_hrst);
            register_get_func("p6_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p6_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p6_srst);
            register_get_func("p6_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p6_srst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7_hrst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p7_hrst);
            register_get_func("p7_hrst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p7_hrst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7_srst", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p7_srst);
            register_get_func("p7_srst", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_sw_reset_t::p7_srst);
        }
        #endif
    
    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_pp_csr_cfg_pp_linkwidth_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p0_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p0_lw);
            register_get_func("p0_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p0_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p1_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p1_lw);
            register_get_func("p1_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p1_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p2_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p2_lw);
            register_get_func("p2_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p2_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p3_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p3_lw);
            register_get_func("p3_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p3_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p4_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p4_lw);
            register_get_func("p4_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p4_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p5_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p5_lw);
            register_get_func("p5_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p5_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p6_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p6_lw);
            register_get_func("p6_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p6_lw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("p7_lw", (cap_csr_base::set_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p7_lw);
            register_get_func("p7_lw", (cap_csr_base::get_function_type_t)&cap_pp_csr_cfg_pp_linkwidth_t::p7_lw);
        }
        #endif
    
}

void cap_pp_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_pp_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_pp_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pp_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_pp_linkwidth.set_attributes(this,"cfg_pp_linkwidth", 0x20000 );
    cfg_pp_sw_reset.set_attributes(this,"cfg_pp_sw_reset", 0x20004 );
    cfg_sbus_result.set_attributes(this,"cfg_sbus_result", 0x20008 );
    cfg_sbus_indir.set_attributes(this,"cfg_sbus_indir", 0x2000c );
    sta_sbus_indir.set_attributes(this,"sta_sbus_indir", 0x20014 );
    cfg_pp_pcs_interrupt_disable.set_attributes(this,"cfg_pp_pcs_interrupt_disable", 0x20018 );
    cfg_pp_sd_async_reset_n.set_attributes(this,"cfg_pp_sd_async_reset_n", 0x2001c );
    cfg_pp_pcs_reset_n.set_attributes(this,"cfg_pp_pcs_reset_n", 0x20020 );
    cfg_pp_sbus.set_attributes(this,"cfg_pp_sbus", 0x20024 );
    cfg_pp_rc_perstn.set_attributes(this,"cfg_pp_rc_perstn", 0x20028 );
    cfg_pp_pcie_pll_rst_n.set_attributes(this,"cfg_pp_pcie_pll_rst_n", 0x2002c );
    cfg_pp_pcie_pll_refclk_sel.set_attributes(this,"cfg_pp_pcie_pll_refclk_sel", 0x20030 );
    cfg_pp_pcie_pll_0.set_attributes(this,"cfg_pp_pcie_pll_0", 0x20038 );
    cfg_pp_pcie_pll_1.set_attributes(this,"cfg_pp_pcie_pll_1", 0x20040 );
    sta_pp_pcie_pll.set_attributes(this,"sta_pp_pcie_pll", 0x20048 );
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_pp_pcsd_interrupt[ii].set_field_init_done(true);
        cfg_pp_pcsd_interrupt[ii].set_attributes(this,"cfg_pp_pcsd_interrupt["+to_string(ii)+"]",  0x20080 + (cfg_pp_pcsd_interrupt[ii].get_byte_size()*ii));
    }
    cfg_pp_pcsd_interrupt_request.set_attributes(this,"cfg_pp_pcsd_interrupt_request", 0x200c0 );
    sta_pp_pcsd_interrupt_in_progress.set_attributes(this,"sta_pp_pcsd_interrupt_in_progress", 0x200c4 );
    sta_pp_pcsd_interrupt_data_out.set_attributes(this,"sta_pp_pcsd_interrupt_data_out", 0x200e0 );
    cfg_pp_sd_spico_gp.set_attributes(this,"cfg_pp_sd_spico_gp", 0x20100 );
    sta_pp_sd_spico_gp.set_attributes(this,"sta_pp_sd_spico_gp", 0x20104 );
    sta_pp_sd_core_status.set_attributes(this,"sta_pp_sd_core_status", 0x20120 );
    sta_pp_sd_rdy.set_attributes(this,"sta_pp_sd_rdy", 0x20140 );
    cfg_pp_sbus_master_bist.set_attributes(this,"cfg_pp_sbus_master_bist", 0x20144 );
    sta_pp_sbus_master_bist.set_attributes(this,"sta_pp_sbus_master_bist", 0x20148 );
    cfg_pp_pcsd_control.set_attributes(this,"cfg_pp_pcsd_control", 0x20150 );
    cfg_pp_sd_core_to_cntl.set_attributes(this,"cfg_pp_sd_core_to_cntl", 0x20160 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x20180 );
    dhs_sbus_indir.set_attributes(this,"dhs_sbus_indir", 0x20010 );
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) port_p[ii].set_field_init_done(true);
        port_p[ii].set_attributes(this,"port_p["+to_string(ii)+"]",  0x400 + (port_p[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) port_c[ii].set_field_init_done(true);
        port_c[ii].set_attributes(this,"port_c["+to_string(ii)+"]",  0x10000 + (port_c[ii].get_byte_size()*ii));
    }
}

void cap_pp_csr_dhs_sbus_indir_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_pp_csr_dhs_sbus_indir_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_debug_port_t::select() const {
    return int_var__select.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_debug_port_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port0(const cpp_int & _val) { 
    // port0
    int_var__port0 = _val.convert_to< port0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port0() const {
    return int_var__port0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port1(const cpp_int & _val) { 
    // port1
    int_var__port1 = _val.convert_to< port1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port1() const {
    return int_var__port1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port2(const cpp_int & _val) { 
    // port2
    int_var__port2 = _val.convert_to< port2_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port2() const {
    return int_var__port2.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port3(const cpp_int & _val) { 
    // port3
    int_var__port3 = _val.convert_to< port3_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port3() const {
    return int_var__port3.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port4(const cpp_int & _val) { 
    // port4
    int_var__port4 = _val.convert_to< port4_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port4() const {
    return int_var__port4.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port5(const cpp_int & _val) { 
    // port5
    int_var__port5 = _val.convert_to< port5_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port5() const {
    return int_var__port5.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port6(const cpp_int & _val) { 
    // port6
    int_var__port6 = _val.convert_to< port6_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port6() const {
    return int_var__port6.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port7(const cpp_int & _val) { 
    // port7
    int_var__port7 = _val.convert_to< port7_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port7() const {
    return int_var__port7.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port8(const cpp_int & _val) { 
    // port8
    int_var__port8 = _val.convert_to< port8_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port8() const {
    return int_var__port8.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port9(const cpp_int & _val) { 
    // port9
    int_var__port9 = _val.convert_to< port9_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port9() const {
    return int_var__port9.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port10(const cpp_int & _val) { 
    // port10
    int_var__port10 = _val.convert_to< port10_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port10() const {
    return int_var__port10.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port11(const cpp_int & _val) { 
    // port11
    int_var__port11 = _val.convert_to< port11_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port11() const {
    return int_var__port11.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port12(const cpp_int & _val) { 
    // port12
    int_var__port12 = _val.convert_to< port12_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port12() const {
    return int_var__port12.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port13(const cpp_int & _val) { 
    // port13
    int_var__port13 = _val.convert_to< port13_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port13() const {
    return int_var__port13.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port14(const cpp_int & _val) { 
    // port14
    int_var__port14 = _val.convert_to< port14_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port14() const {
    return int_var__port14.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port15(const cpp_int & _val) { 
    // port15
    int_var__port15 = _val.convert_to< port15_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_core_to_cntl_t::port15() const {
    return int_var__port15.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_0(const cpp_int & _val) { 
    // fts_align_grp_0
    int_var__fts_align_grp_0 = _val.convert_to< fts_align_grp_0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_0() const {
    return int_var__fts_align_grp_0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_0(const cpp_int & _val) { 
    // sris_en_grp_0
    int_var__sris_en_grp_0 = _val.convert_to< sris_en_grp_0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_0() const {
    return int_var__sris_en_grp_0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_0(const cpp_int & _val) { 
    // rx8b10b_realign_grp_0
    int_var__rx8b10b_realign_grp_0 = _val.convert_to< rx8b10b_realign_grp_0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_0() const {
    return int_var__rx8b10b_realign_grp_0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_1(const cpp_int & _val) { 
    // fts_align_grp_1
    int_var__fts_align_grp_1 = _val.convert_to< fts_align_grp_1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_1() const {
    return int_var__fts_align_grp_1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_1(const cpp_int & _val) { 
    // sris_en_grp_1
    int_var__sris_en_grp_1 = _val.convert_to< sris_en_grp_1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_1() const {
    return int_var__sris_en_grp_1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_1(const cpp_int & _val) { 
    // rx8b10b_realign_grp_1
    int_var__rx8b10b_realign_grp_1 = _val.convert_to< rx8b10b_realign_grp_1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_1() const {
    return int_var__rx8b10b_realign_grp_1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_2(const cpp_int & _val) { 
    // fts_align_grp_2
    int_var__fts_align_grp_2 = _val.convert_to< fts_align_grp_2_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_2() const {
    return int_var__fts_align_grp_2.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_2(const cpp_int & _val) { 
    // sris_en_grp_2
    int_var__sris_en_grp_2 = _val.convert_to< sris_en_grp_2_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_2() const {
    return int_var__sris_en_grp_2.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_2(const cpp_int & _val) { 
    // rx8b10b_realign_grp_2
    int_var__rx8b10b_realign_grp_2 = _val.convert_to< rx8b10b_realign_grp_2_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_2() const {
    return int_var__rx8b10b_realign_grp_2.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_3(const cpp_int & _val) { 
    // fts_align_grp_3
    int_var__fts_align_grp_3 = _val.convert_to< fts_align_grp_3_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_3() const {
    return int_var__fts_align_grp_3.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_3(const cpp_int & _val) { 
    // sris_en_grp_3
    int_var__sris_en_grp_3 = _val.convert_to< sris_en_grp_3_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_3() const {
    return int_var__sris_en_grp_3.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_3(const cpp_int & _val) { 
    // rx8b10b_realign_grp_3
    int_var__rx8b10b_realign_grp_3 = _val.convert_to< rx8b10b_realign_grp_3_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_3() const {
    return int_var__rx8b10b_realign_grp_3.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_4(const cpp_int & _val) { 
    // fts_align_grp_4
    int_var__fts_align_grp_4 = _val.convert_to< fts_align_grp_4_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_4() const {
    return int_var__fts_align_grp_4.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_4(const cpp_int & _val) { 
    // sris_en_grp_4
    int_var__sris_en_grp_4 = _val.convert_to< sris_en_grp_4_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_4() const {
    return int_var__sris_en_grp_4.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_4(const cpp_int & _val) { 
    // rx8b10b_realign_grp_4
    int_var__rx8b10b_realign_grp_4 = _val.convert_to< rx8b10b_realign_grp_4_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_4() const {
    return int_var__rx8b10b_realign_grp_4.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_5(const cpp_int & _val) { 
    // fts_align_grp_5
    int_var__fts_align_grp_5 = _val.convert_to< fts_align_grp_5_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_5() const {
    return int_var__fts_align_grp_5.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_5(const cpp_int & _val) { 
    // sris_en_grp_5
    int_var__sris_en_grp_5 = _val.convert_to< sris_en_grp_5_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_5() const {
    return int_var__sris_en_grp_5.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_5(const cpp_int & _val) { 
    // rx8b10b_realign_grp_5
    int_var__rx8b10b_realign_grp_5 = _val.convert_to< rx8b10b_realign_grp_5_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_5() const {
    return int_var__rx8b10b_realign_grp_5.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_6(const cpp_int & _val) { 
    // fts_align_grp_6
    int_var__fts_align_grp_6 = _val.convert_to< fts_align_grp_6_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_6() const {
    return int_var__fts_align_grp_6.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_6(const cpp_int & _val) { 
    // sris_en_grp_6
    int_var__sris_en_grp_6 = _val.convert_to< sris_en_grp_6_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_6() const {
    return int_var__sris_en_grp_6.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_6(const cpp_int & _val) { 
    // rx8b10b_realign_grp_6
    int_var__rx8b10b_realign_grp_6 = _val.convert_to< rx8b10b_realign_grp_6_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_6() const {
    return int_var__rx8b10b_realign_grp_6.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_7(const cpp_int & _val) { 
    // fts_align_grp_7
    int_var__fts_align_grp_7 = _val.convert_to< fts_align_grp_7_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::fts_align_grp_7() const {
    return int_var__fts_align_grp_7.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_7(const cpp_int & _val) { 
    // sris_en_grp_7
    int_var__sris_en_grp_7 = _val.convert_to< sris_en_grp_7_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::sris_en_grp_7() const {
    return int_var__sris_en_grp_7.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_7(const cpp_int & _val) { 
    // rx8b10b_realign_grp_7
    int_var__rx8b10b_realign_grp_7 = _val.convert_to< rx8b10b_realign_grp_7_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_control_t::rx8b10b_realign_grp_7() const {
    return int_var__rx8b10b_realign_grp_7.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sbus_master_bist_t::fail(const cpp_int & _val) { 
    // fail
    int_var__fail = _val.convert_to< fail_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sbus_master_bist_t::fail() const {
    return int_var__fail.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sbus_master_bist_t::pass(const cpp_int & _val) { 
    // pass
    int_var__pass = _val.convert_to< pass_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sbus_master_bist_t::pass() const {
    return int_var__pass.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sbus_master_bist_t::mode(const cpp_int & _val) { 
    // mode
    int_var__mode = _val.convert_to< mode_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sbus_master_bist_t::mode() const {
    return int_var__mode.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sbus_master_bist_t::rst(const cpp_int & _val) { 
    // rst
    int_var__rst = _val.convert_to< rst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sbus_master_bist_t::rst() const {
    return int_var__rst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sbus_master_bist_t::run(const cpp_int & _val) { 
    // run
    int_var__run = _val.convert_to< run_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sbus_master_bist_t::run() const {
    return int_var__run.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_rdy_t::rx_rdy(const cpp_int & _val) { 
    // rx_rdy
    int_var__rx_rdy = _val.convert_to< rx_rdy_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_rdy_t::rx_rdy() const {
    return int_var__rx_rdy.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_rdy_t::tx_rdy(const cpp_int & _val) { 
    // tx_rdy
    int_var__tx_rdy = _val.convert_to< tx_rdy_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_rdy_t::tx_rdy() const {
    return int_var__tx_rdy.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane0(const cpp_int & _val) { 
    // lane0
    int_var__lane0 = _val.convert_to< lane0_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane0() const {
    return int_var__lane0.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane1(const cpp_int & _val) { 
    // lane1
    int_var__lane1 = _val.convert_to< lane1_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane1() const {
    return int_var__lane1.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane2(const cpp_int & _val) { 
    // lane2
    int_var__lane2 = _val.convert_to< lane2_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane2() const {
    return int_var__lane2.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane3(const cpp_int & _val) { 
    // lane3
    int_var__lane3 = _val.convert_to< lane3_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane3() const {
    return int_var__lane3.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane4(const cpp_int & _val) { 
    // lane4
    int_var__lane4 = _val.convert_to< lane4_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane4() const {
    return int_var__lane4.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane5(const cpp_int & _val) { 
    // lane5
    int_var__lane5 = _val.convert_to< lane5_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane5() const {
    return int_var__lane5.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane6(const cpp_int & _val) { 
    // lane6
    int_var__lane6 = _val.convert_to< lane6_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane6() const {
    return int_var__lane6.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane7(const cpp_int & _val) { 
    // lane7
    int_var__lane7 = _val.convert_to< lane7_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane7() const {
    return int_var__lane7.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane8(const cpp_int & _val) { 
    // lane8
    int_var__lane8 = _val.convert_to< lane8_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane8() const {
    return int_var__lane8.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane9(const cpp_int & _val) { 
    // lane9
    int_var__lane9 = _val.convert_to< lane9_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane9() const {
    return int_var__lane9.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane10(const cpp_int & _val) { 
    // lane10
    int_var__lane10 = _val.convert_to< lane10_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane10() const {
    return int_var__lane10.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane11(const cpp_int & _val) { 
    // lane11
    int_var__lane11 = _val.convert_to< lane11_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane11() const {
    return int_var__lane11.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane12(const cpp_int & _val) { 
    // lane12
    int_var__lane12 = _val.convert_to< lane12_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane12() const {
    return int_var__lane12.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane13(const cpp_int & _val) { 
    // lane13
    int_var__lane13 = _val.convert_to< lane13_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane13() const {
    return int_var__lane13.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane14(const cpp_int & _val) { 
    // lane14
    int_var__lane14 = _val.convert_to< lane14_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane14() const {
    return int_var__lane14.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_core_status_t::lane15(const cpp_int & _val) { 
    // lane15
    int_var__lane15 = _val.convert_to< lane15_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_core_status_t::lane15() const {
    return int_var__lane15.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_sd_spico_gp_t::out(const cpp_int & _val) { 
    // out
    int_var__out = _val.convert_to< out_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_sd_spico_gp_t::out() const {
    return int_var__out.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_spico_gp_t::in(const cpp_int & _val) { 
    // in
    int_var__in = _val.convert_to< in_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_spico_gp_t::in() const {
    return int_var__in.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane0(const cpp_int & _val) { 
    // lane0
    int_var__lane0 = _val.convert_to< lane0_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane0() const {
    return int_var__lane0.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane1(const cpp_int & _val) { 
    // lane1
    int_var__lane1 = _val.convert_to< lane1_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane1() const {
    return int_var__lane1.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane2(const cpp_int & _val) { 
    // lane2
    int_var__lane2 = _val.convert_to< lane2_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane2() const {
    return int_var__lane2.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane3(const cpp_int & _val) { 
    // lane3
    int_var__lane3 = _val.convert_to< lane3_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane3() const {
    return int_var__lane3.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane4(const cpp_int & _val) { 
    // lane4
    int_var__lane4 = _val.convert_to< lane4_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane4() const {
    return int_var__lane4.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane5(const cpp_int & _val) { 
    // lane5
    int_var__lane5 = _val.convert_to< lane5_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane5() const {
    return int_var__lane5.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane6(const cpp_int & _val) { 
    // lane6
    int_var__lane6 = _val.convert_to< lane6_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane6() const {
    return int_var__lane6.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane7(const cpp_int & _val) { 
    // lane7
    int_var__lane7 = _val.convert_to< lane7_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane7() const {
    return int_var__lane7.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane8(const cpp_int & _val) { 
    // lane8
    int_var__lane8 = _val.convert_to< lane8_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane8() const {
    return int_var__lane8.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane9(const cpp_int & _val) { 
    // lane9
    int_var__lane9 = _val.convert_to< lane9_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane9() const {
    return int_var__lane9.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane10(const cpp_int & _val) { 
    // lane10
    int_var__lane10 = _val.convert_to< lane10_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane10() const {
    return int_var__lane10.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane11(const cpp_int & _val) { 
    // lane11
    int_var__lane11 = _val.convert_to< lane11_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane11() const {
    return int_var__lane11.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane12(const cpp_int & _val) { 
    // lane12
    int_var__lane12 = _val.convert_to< lane12_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane12() const {
    return int_var__lane12.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane13(const cpp_int & _val) { 
    // lane13
    int_var__lane13 = _val.convert_to< lane13_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane13() const {
    return int_var__lane13.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane14(const cpp_int & _val) { 
    // lane14
    int_var__lane14 = _val.convert_to< lane14_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane14() const {
    return int_var__lane14.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane15(const cpp_int & _val) { 
    // lane15
    int_var__lane15 = _val.convert_to< lane15_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t::lane15() const {
    return int_var__lane15.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::per_lane(const cpp_int & _val) { 
    // per_lane
    int_var__per_lane = _val.convert_to< per_lane_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t::per_lane() const {
    return int_var__per_lane.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::lanemask(const cpp_int & _val) { 
    // lanemask
    int_var__lanemask = _val.convert_to< lanemask_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_interrupt_request_t::lanemask() const {
    return int_var__lanemask.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_interrupt_t::code(const cpp_int & _val) { 
    // code
    int_var__code = _val.convert_to< code_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_interrupt_t::code() const {
    return int_var__code.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcsd_interrupt_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcsd_interrupt_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_dll_lock(const cpp_int & _val) { 
    // pcie_0_dll_lock
    int_var__pcie_0_dll_lock = _val.convert_to< pcie_0_dll_lock_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_dll_lock() const {
    return int_var__pcie_0_dll_lock.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_pll_lock(const cpp_int & _val) { 
    // pcie_0_pll_lock
    int_var__pcie_0_pll_lock = _val.convert_to< pcie_0_pll_lock_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcie_pll_t::pcie_0_pll_lock() const {
    return int_var__pcie_0_pll_lock.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_dll_lock(const cpp_int & _val) { 
    // pcie_1_dll_lock
    int_var__pcie_1_dll_lock = _val.convert_to< pcie_1_dll_lock_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_dll_lock() const {
    return int_var__pcie_1_dll_lock.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_pll_lock(const cpp_int & _val) { 
    // pcie_1_pll_lock
    int_var__pcie_1_pll_lock = _val.convert_to< pcie_1_pll_lock_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_pp_pcie_pll_t::pcie_1_pll_lock() const {
    return int_var__pcie_1_pll_lock.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_disable_output_clk(const cpp_int & _val) { 
    // dll_disable_output_clk
    int_var__dll_disable_output_clk = _val.convert_to< dll_disable_output_clk_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_disable_output_clk() const {
    return int_var__dll_disable_output_clk.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_fbcnt(const cpp_int & _val) { 
    // dll_fbcnt
    int_var__dll_fbcnt = _val.convert_to< dll_fbcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_fbcnt() const {
    return int_var__dll_fbcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_out_divcnt(const cpp_int & _val) { 
    // dll_out_divcnt
    int_var__dll_out_divcnt = _val.convert_to< dll_out_divcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_out_divcnt() const {
    return int_var__dll_out_divcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_refcnt(const cpp_int & _val) { 
    // dll_refcnt
    int_var__dll_refcnt = _val.convert_to< dll_refcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::dll_refcnt() const {
    return int_var__dll_refcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_disable_output_clk(const cpp_int & _val) { 
    // pll_disable_output_clk
    int_var__pll_disable_output_clk = _val.convert_to< pll_disable_output_clk_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_disable_output_clk() const {
    return int_var__pll_disable_output_clk.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_out_divcnt(const cpp_int & _val) { 
    // pll_out_divcnt
    int_var__pll_out_divcnt = _val.convert_to< pll_out_divcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_out_divcnt() const {
    return int_var__pll_out_divcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_refcnt(const cpp_int & _val) { 
    // pll_refcnt
    int_var__pll_refcnt = _val.convert_to< pll_refcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::pll_refcnt() const {
    return int_var__pll_refcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv255(const cpp_int & _val) { 
    // prog_fbdiv255
    int_var__prog_fbdiv255 = _val.convert_to< prog_fbdiv255_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv255() const {
    return int_var__prog_fbdiv255.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv_23(const cpp_int & _val) { 
    // prog_fbdiv_23
    int_var__prog_fbdiv_23 = _val.convert_to< prog_fbdiv_23_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_1_t::prog_fbdiv_23() const {
    return int_var__prog_fbdiv_23.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_disable_output_clk(const cpp_int & _val) { 
    // dll_disable_output_clk
    int_var__dll_disable_output_clk = _val.convert_to< dll_disable_output_clk_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_disable_output_clk() const {
    return int_var__dll_disable_output_clk.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_fbcnt(const cpp_int & _val) { 
    // dll_fbcnt
    int_var__dll_fbcnt = _val.convert_to< dll_fbcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_fbcnt() const {
    return int_var__dll_fbcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_out_divcnt(const cpp_int & _val) { 
    // dll_out_divcnt
    int_var__dll_out_divcnt = _val.convert_to< dll_out_divcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_out_divcnt() const {
    return int_var__dll_out_divcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_refcnt(const cpp_int & _val) { 
    // dll_refcnt
    int_var__dll_refcnt = _val.convert_to< dll_refcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::dll_refcnt() const {
    return int_var__dll_refcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_disable_output_clk(const cpp_int & _val) { 
    // pll_disable_output_clk
    int_var__pll_disable_output_clk = _val.convert_to< pll_disable_output_clk_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_disable_output_clk() const {
    return int_var__pll_disable_output_clk.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_out_divcnt(const cpp_int & _val) { 
    // pll_out_divcnt
    int_var__pll_out_divcnt = _val.convert_to< pll_out_divcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_out_divcnt() const {
    return int_var__pll_out_divcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_refcnt(const cpp_int & _val) { 
    // pll_refcnt
    int_var__pll_refcnt = _val.convert_to< pll_refcnt_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::pll_refcnt() const {
    return int_var__pll_refcnt.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv255(const cpp_int & _val) { 
    // prog_fbdiv255
    int_var__prog_fbdiv255 = _val.convert_to< prog_fbdiv255_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv255() const {
    return int_var__prog_fbdiv255.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv_23(const cpp_int & _val) { 
    // prog_fbdiv_23
    int_var__prog_fbdiv_23 = _val.convert_to< prog_fbdiv_23_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_0_t::prog_fbdiv_23() const {
    return int_var__prog_fbdiv_23.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p0(const cpp_int & _val) { 
    // p0
    int_var__p0 = _val.convert_to< p0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p0() const {
    return int_var__p0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p1(const cpp_int & _val) { 
    // p1
    int_var__p1 = _val.convert_to< p1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p1() const {
    return int_var__p1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p2(const cpp_int & _val) { 
    // p2
    int_var__p2 = _val.convert_to< p2_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p2() const {
    return int_var__p2.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p3(const cpp_int & _val) { 
    // p3
    int_var__p3 = _val.convert_to< p3_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p3() const {
    return int_var__p3.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p4(const cpp_int & _val) { 
    // p4
    int_var__p4 = _val.convert_to< p4_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p4() const {
    return int_var__p4.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p5(const cpp_int & _val) { 
    // p5
    int_var__p5 = _val.convert_to< p5_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p5() const {
    return int_var__p5.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p6(const cpp_int & _val) { 
    // p6
    int_var__p6 = _val.convert_to< p6_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p6() const {
    return int_var__p6.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p7(const cpp_int & _val) { 
    // p7
    int_var__p7 = _val.convert_to< p7_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t::p7() const {
    return int_var__p7.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_0(const cpp_int & _val) { 
    // pll_0
    int_var__pll_0 = _val.convert_to< pll_0_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_0() const {
    return int_var__pll_0.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_1(const cpp_int & _val) { 
    // pll_1
    int_var__pll_1 = _val.convert_to< pll_1_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcie_pll_rst_n_t::pll_1() const {
    return int_var__pll_1.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_en(const cpp_int & _val) { 
    // p0_out_en
    int_var__p0_out_en = _val.convert_to< p0_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_en() const {
    return int_var__p0_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_data(const cpp_int & _val) { 
    // p0_out_data
    int_var__p0_out_data = _val.convert_to< p0_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p0_out_data() const {
    return int_var__p0_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_en(const cpp_int & _val) { 
    // p1_out_en
    int_var__p1_out_en = _val.convert_to< p1_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_en() const {
    return int_var__p1_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_data(const cpp_int & _val) { 
    // p1_out_data
    int_var__p1_out_data = _val.convert_to< p1_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p1_out_data() const {
    return int_var__p1_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_en(const cpp_int & _val) { 
    // p2_out_en
    int_var__p2_out_en = _val.convert_to< p2_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_en() const {
    return int_var__p2_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_data(const cpp_int & _val) { 
    // p2_out_data
    int_var__p2_out_data = _val.convert_to< p2_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p2_out_data() const {
    return int_var__p2_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_en(const cpp_int & _val) { 
    // p3_out_en
    int_var__p3_out_en = _val.convert_to< p3_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_en() const {
    return int_var__p3_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_data(const cpp_int & _val) { 
    // p3_out_data
    int_var__p3_out_data = _val.convert_to< p3_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p3_out_data() const {
    return int_var__p3_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_en(const cpp_int & _val) { 
    // p4_out_en
    int_var__p4_out_en = _val.convert_to< p4_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_en() const {
    return int_var__p4_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_data(const cpp_int & _val) { 
    // p4_out_data
    int_var__p4_out_data = _val.convert_to< p4_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p4_out_data() const {
    return int_var__p4_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_en(const cpp_int & _val) { 
    // p5_out_en
    int_var__p5_out_en = _val.convert_to< p5_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_en() const {
    return int_var__p5_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_data(const cpp_int & _val) { 
    // p5_out_data
    int_var__p5_out_data = _val.convert_to< p5_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p5_out_data() const {
    return int_var__p5_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_en(const cpp_int & _val) { 
    // p6_out_en
    int_var__p6_out_en = _val.convert_to< p6_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_en() const {
    return int_var__p6_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_data(const cpp_int & _val) { 
    // p6_out_data
    int_var__p6_out_data = _val.convert_to< p6_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p6_out_data() const {
    return int_var__p6_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_en(const cpp_int & _val) { 
    // p7_out_en
    int_var__p7_out_en = _val.convert_to< p7_out_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_en() const {
    return int_var__p7_out_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_data(const cpp_int & _val) { 
    // p7_out_data
    int_var__p7_out_data = _val.convert_to< p7_out_data_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_rc_perstn_t::p7_out_data() const {
    return int_var__p7_out_data.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sbus_t::rom_en(const cpp_int & _val) { 
    // rom_en
    int_var__rom_en = _val.convert_to< rom_en_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sbus_t::rom_en() const {
    return int_var__rom_en.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcs_reset_n_t::lanemask(const cpp_int & _val) { 
    // lanemask
    int_var__lanemask = _val.convert_to< lanemask_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcs_reset_n_t::lanemask() const {
    return int_var__lanemask.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sd_async_reset_n_t::lanemask(const cpp_int & _val) { 
    // lanemask
    int_var__lanemask = _val.convert_to< lanemask_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sd_async_reset_n_t::lanemask() const {
    return int_var__lanemask.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::lanemask(const cpp_int & _val) { 
    // lanemask
    int_var__lanemask = _val.convert_to< lanemask_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_pcs_interrupt_disable_t::lanemask() const {
    return int_var__lanemask.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_sbus_indir_t::execute(const cpp_int & _val) { 
    // execute
    int_var__execute = _val.convert_to< execute_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_sbus_indir_t::execute() const {
    return int_var__execute.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_sbus_indir_t::done(const cpp_int & _val) { 
    // done
    int_var__done = _val.convert_to< done_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_sbus_indir_t::done() const {
    return int_var__done.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_sbus_indir_t::rcv_data_vld(const cpp_int & _val) { 
    // rcv_data_vld
    int_var__rcv_data_vld = _val.convert_to< rcv_data_vld_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_sbus_indir_t::rcv_data_vld() const {
    return int_var__rcv_data_vld.convert_to< cpp_int >();
}
    
void cap_pp_csr_sta_sbus_indir_t::result_code(const cpp_int & _val) { 
    // result_code
    int_var__result_code = _val.convert_to< result_code_cpp_int_t >();
}

cpp_int cap_pp_csr_sta_sbus_indir_t::result_code() const {
    return int_var__result_code.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_sbus_indir_t::rcvr_addr(const cpp_int & _val) { 
    // rcvr_addr
    int_var__rcvr_addr = _val.convert_to< rcvr_addr_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_sbus_indir_t::rcvr_addr() const {
    return int_var__rcvr_addr.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_sbus_indir_t::data_addr(const cpp_int & _val) { 
    // data_addr
    int_var__data_addr = _val.convert_to< data_addr_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_sbus_indir_t::data_addr() const {
    return int_var__data_addr.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_sbus_indir_t::command(const cpp_int & _val) { 
    // command
    int_var__command = _val.convert_to< command_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_sbus_indir_t::command() const {
    return int_var__command.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_sbus_result_t::mode(const cpp_int & _val) { 
    // mode
    int_var__mode = _val.convert_to< mode_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_sbus_result_t::mode() const {
    return int_var__mode.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p0_hrst(const cpp_int & _val) { 
    // p0_hrst
    int_var__p0_hrst = _val.convert_to< p0_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p0_hrst() const {
    return int_var__p0_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p0_srst(const cpp_int & _val) { 
    // p0_srst
    int_var__p0_srst = _val.convert_to< p0_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p0_srst() const {
    return int_var__p0_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p1_hrst(const cpp_int & _val) { 
    // p1_hrst
    int_var__p1_hrst = _val.convert_to< p1_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p1_hrst() const {
    return int_var__p1_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p1_srst(const cpp_int & _val) { 
    // p1_srst
    int_var__p1_srst = _val.convert_to< p1_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p1_srst() const {
    return int_var__p1_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p2_hrst(const cpp_int & _val) { 
    // p2_hrst
    int_var__p2_hrst = _val.convert_to< p2_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p2_hrst() const {
    return int_var__p2_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p2_srst(const cpp_int & _val) { 
    // p2_srst
    int_var__p2_srst = _val.convert_to< p2_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p2_srst() const {
    return int_var__p2_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p3_hrst(const cpp_int & _val) { 
    // p3_hrst
    int_var__p3_hrst = _val.convert_to< p3_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p3_hrst() const {
    return int_var__p3_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p3_srst(const cpp_int & _val) { 
    // p3_srst
    int_var__p3_srst = _val.convert_to< p3_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p3_srst() const {
    return int_var__p3_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p4_hrst(const cpp_int & _val) { 
    // p4_hrst
    int_var__p4_hrst = _val.convert_to< p4_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p4_hrst() const {
    return int_var__p4_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p4_srst(const cpp_int & _val) { 
    // p4_srst
    int_var__p4_srst = _val.convert_to< p4_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p4_srst() const {
    return int_var__p4_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p5_hrst(const cpp_int & _val) { 
    // p5_hrst
    int_var__p5_hrst = _val.convert_to< p5_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p5_hrst() const {
    return int_var__p5_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p5_srst(const cpp_int & _val) { 
    // p5_srst
    int_var__p5_srst = _val.convert_to< p5_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p5_srst() const {
    return int_var__p5_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p6_hrst(const cpp_int & _val) { 
    // p6_hrst
    int_var__p6_hrst = _val.convert_to< p6_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p6_hrst() const {
    return int_var__p6_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p6_srst(const cpp_int & _val) { 
    // p6_srst
    int_var__p6_srst = _val.convert_to< p6_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p6_srst() const {
    return int_var__p6_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p7_hrst(const cpp_int & _val) { 
    // p7_hrst
    int_var__p7_hrst = _val.convert_to< p7_hrst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p7_hrst() const {
    return int_var__p7_hrst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_sw_reset_t::p7_srst(const cpp_int & _val) { 
    // p7_srst
    int_var__p7_srst = _val.convert_to< p7_srst_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_sw_reset_t::p7_srst() const {
    return int_var__p7_srst.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p0_lw(const cpp_int & _val) { 
    // p0_lw
    int_var__p0_lw = _val.convert_to< p0_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p0_lw() const {
    return int_var__p0_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p1_lw(const cpp_int & _val) { 
    // p1_lw
    int_var__p1_lw = _val.convert_to< p1_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p1_lw() const {
    return int_var__p1_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p2_lw(const cpp_int & _val) { 
    // p2_lw
    int_var__p2_lw = _val.convert_to< p2_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p2_lw() const {
    return int_var__p2_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p3_lw(const cpp_int & _val) { 
    // p3_lw
    int_var__p3_lw = _val.convert_to< p3_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p3_lw() const {
    return int_var__p3_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p4_lw(const cpp_int & _val) { 
    // p4_lw
    int_var__p4_lw = _val.convert_to< p4_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p4_lw() const {
    return int_var__p4_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p5_lw(const cpp_int & _val) { 
    // p5_lw
    int_var__p5_lw = _val.convert_to< p5_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p5_lw() const {
    return int_var__p5_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p6_lw(const cpp_int & _val) { 
    // p6_lw
    int_var__p6_lw = _val.convert_to< p6_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p6_lw() const {
    return int_var__p6_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_cfg_pp_linkwidth_t::p7_lw(const cpp_int & _val) { 
    // p7_lw
    int_var__p7_lw = _val.convert_to< p7_lw_cpp_int_t >();
}

cpp_int cap_pp_csr_cfg_pp_linkwidth_t::p7_lw() const {
    return int_var__p7_lw.convert_to< cpp_int >();
}
    
void cap_pp_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pp_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    