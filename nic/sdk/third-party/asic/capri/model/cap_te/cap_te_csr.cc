
#include "cap_te_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_te_csr_int_info_int_enable_clear_t::cap_te_csr_int_info_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_info_int_enable_clear_t::~cap_te_csr_int_info_int_enable_clear_t() { }

cap_te_csr_int_info_int_test_set_t::cap_te_csr_int_info_int_test_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_info_int_test_set_t::~cap_te_csr_int_info_int_test_set_t() { }

cap_te_csr_int_info_t::cap_te_csr_int_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_info_t::~cap_te_csr_int_info_t() { }

cap_te_csr_int_err_int_enable_clear_t::cap_te_csr_int_err_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_err_int_enable_clear_t::~cap_te_csr_int_err_int_enable_clear_t() { }

cap_te_csr_int_err_intreg_t::cap_te_csr_int_err_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_err_intreg_t::~cap_te_csr_int_err_intreg_t() { }

cap_te_csr_int_err_t::cap_te_csr_int_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_err_t::~cap_te_csr_int_err_t() { }

cap_te_csr_intreg_status_t::cap_te_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_intreg_status_t::~cap_te_csr_intreg_status_t() { }

cap_te_csr_int_groups_int_enable_rw_reg_t::cap_te_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_int_groups_int_enable_rw_reg_t::~cap_te_csr_int_groups_int_enable_rw_reg_t() { }

cap_te_csr_intgrp_status_t::cap_te_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_intgrp_status_t::~cap_te_csr_intgrp_status_t() { }

cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::cap_te_csr_dhs_table_profile_ctrl_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::~cap_te_csr_dhs_table_profile_ctrl_sram_entry_t() { }

cap_te_csr_dhs_table_profile_ctrl_sram_t::cap_te_csr_dhs_table_profile_ctrl_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_te_csr_dhs_table_profile_ctrl_sram_t::~cap_te_csr_dhs_table_profile_ctrl_sram_t() { }

cap_te_csr_CNT_stall_t::cap_te_csr_CNT_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_CNT_stall_t::~cap_te_csr_CNT_stall_t() { }

cap_te_csr_cfg_hbm_addr_align_t::cap_te_csr_cfg_hbm_addr_align_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_hbm_addr_align_t::~cap_te_csr_cfg_hbm_addr_align_t() { }

cap_te_csr_cfg_bist_t::cap_te_csr_cfg_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_bist_t::~cap_te_csr_cfg_bist_t() { }

cap_te_csr_sta_bist_done_t::cap_te_csr_sta_bist_done_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_bist_done_t::~cap_te_csr_sta_bist_done_t() { }

cap_te_csr_sta_peek_t::cap_te_csr_sta_peek_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_peek_t::~cap_te_csr_sta_peek_t() { }

cap_te_csr_csr_intr_t::cap_te_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_csr_intr_t::~cap_te_csr_csr_intr_t() { }

cap_te_csr_sta_bad_axi_read_t::cap_te_csr_sta_bad_axi_read_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_bad_axi_read_t::~cap_te_csr_sta_bad_axi_read_t() { }

cap_te_csr_sta_axi_err_t::cap_te_csr_sta_axi_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_axi_err_t::~cap_te_csr_sta_axi_err_t() { }

cap_te_csr_sta_wait_t::cap_te_csr_sta_wait_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_wait_t::~cap_te_csr_sta_wait_t() { }

cap_te_csr_sta_wait_fsm_states_t::cap_te_csr_sta_wait_fsm_states_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_wait_fsm_states_t::~cap_te_csr_sta_wait_fsm_states_t() { }

cap_te_csr_sta_wait_entry_t::cap_te_csr_sta_wait_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_wait_entry_t::~cap_te_csr_sta_wait_entry_t() { }

cap_te_csr_cfg_timeout_t::cap_te_csr_cfg_timeout_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_timeout_t::~cap_te_csr_cfg_timeout_t() { }

cap_te_csr_cfg_page_size_t::cap_te_csr_cfg_page_size_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_page_size_t::~cap_te_csr_cfg_page_size_t() { }

cap_te_csr_cfg_read_wait_entry_t::cap_te_csr_cfg_read_wait_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_read_wait_entry_t::~cap_te_csr_cfg_read_wait_entry_t() { }

cap_te_csr_cfg_addr_target_t::cap_te_csr_cfg_addr_target_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_addr_target_t::~cap_te_csr_cfg_addr_target_t() { }

cap_te_csr_sta_debug_bus_t::cap_te_csr_sta_debug_bus_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_debug_bus_t::~cap_te_csr_sta_debug_bus_t() { }

cap_te_csr_cfg_debug_bus_t::cap_te_csr_cfg_debug_bus_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_debug_bus_t::~cap_te_csr_cfg_debug_bus_t() { }

cap_te_csr_cnt_mpu_out_t::cap_te_csr_cnt_mpu_out_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_mpu_out_t::~cap_te_csr_cnt_mpu_out_t() { }

cap_te_csr_cnt_tcam_rsp_t::cap_te_csr_cnt_tcam_rsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_tcam_rsp_t::~cap_te_csr_cnt_tcam_rsp_t() { }

cap_te_csr_cnt_tcam_req_t::cap_te_csr_cnt_tcam_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_tcam_req_t::~cap_te_csr_cnt_tcam_req_t() { }

cap_te_csr_cnt_axi_rdrsp_t::cap_te_csr_cnt_axi_rdrsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_axi_rdrsp_t::~cap_te_csr_cnt_axi_rdrsp_t() { }

cap_te_csr_cnt_axi_rdreq_t::cap_te_csr_cnt_axi_rdreq_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_axi_rdreq_t::~cap_te_csr_cnt_axi_rdreq_t() { }

cap_te_csr_cnt_phv_out_eop_t::cap_te_csr_cnt_phv_out_eop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_phv_out_eop_t::~cap_te_csr_cnt_phv_out_eop_t() { }

cap_te_csr_cnt_phv_out_sop_t::cap_te_csr_cnt_phv_out_sop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_phv_out_sop_t::~cap_te_csr_cnt_phv_out_sop_t() { }

cap_te_csr_cnt_phv_in_eop_t::cap_te_csr_cnt_phv_in_eop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_phv_in_eop_t::~cap_te_csr_cnt_phv_in_eop_t() { }

cap_te_csr_cnt_phv_in_sop_t::cap_te_csr_cnt_phv_in_sop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_phv_in_sop_t::~cap_te_csr_cnt_phv_in_sop_t() { }

cap_te_csr_dhs_single_step_t::cap_te_csr_dhs_single_step_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_dhs_single_step_t::~cap_te_csr_dhs_single_step_t() { }

cap_te_csr_cfg_km_profile_bit_loc_t::cap_te_csr_cfg_km_profile_bit_loc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_km_profile_bit_loc_t::~cap_te_csr_cfg_km_profile_bit_loc_t() { }

cap_te_csr_cfg_km_profile_bit_sel_t::cap_te_csr_cfg_km_profile_bit_sel_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_km_profile_bit_sel_t::~cap_te_csr_cfg_km_profile_bit_sel_t() { }

cap_te_csr_cfg_km_profile_byte_sel_t::cap_te_csr_cfg_km_profile_byte_sel_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_km_profile_byte_sel_t::~cap_te_csr_cfg_km_profile_byte_sel_t() { }

cap_te_csr_cfg_table_mpu_const_t::cap_te_csr_cfg_table_mpu_const_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_mpu_const_t::~cap_te_csr_cfg_table_mpu_const_t() { }

cap_te_csr_cfg_table_property_t::cap_te_csr_cfg_table_property_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_property_t::~cap_te_csr_cfg_table_property_t() { }

cap_te_csr_cfg_table_profile_t::cap_te_csr_cfg_table_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_profile_t::~cap_te_csr_cfg_table_profile_t() { }

cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::cap_te_csr_cfg_table_profile_ctrl_sram_ext_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::~cap_te_csr_cfg_table_profile_ctrl_sram_ext_t() { }

cap_te_csr_cfg_table_profile_cam_t::cap_te_csr_cfg_table_profile_cam_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_profile_cam_t::~cap_te_csr_cfg_table_profile_cam_t() { }

cap_te_csr_cfg_table_profile_key_t::cap_te_csr_cfg_table_profile_key_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_table_profile_key_t::~cap_te_csr_cfg_table_profile_key_t() { }

cap_te_csr_cfg_global_t::cap_te_csr_cfg_global_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_global_t::~cap_te_csr_cfg_global_t() { }

cap_te_csr_base_t::cap_te_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_base_t::~cap_te_csr_base_t() { }

cap_te_csr_t::cap_te_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(16384);
        set_attributes(0,get_name(), 0);
        }
cap_te_csr_t::~cap_te_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_info_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit0_enable: 0x" << int_var__profile_cam_hit0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit1_enable: 0x" << int_var__profile_cam_hit1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit2_enable: 0x" << int_var__profile_cam_hit2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit3_enable: 0x" << int_var__profile_cam_hit3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit4_enable: 0x" << int_var__profile_cam_hit4_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit5_enable: 0x" << int_var__profile_cam_hit5_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit6_enable: 0x" << int_var__profile_cam_hit6_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit7_enable: 0x" << int_var__profile_cam_hit7_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit8_enable: 0x" << int_var__profile_cam_hit8_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit9_enable: 0x" << int_var__profile_cam_hit9_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit10_enable: 0x" << int_var__profile_cam_hit10_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit11_enable: 0x" << int_var__profile_cam_hit11_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit12_enable: 0x" << int_var__profile_cam_hit12_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit13_enable: 0x" << int_var__profile_cam_hit13_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit14_enable: 0x" << int_var__profile_cam_hit14_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit15_enable: 0x" << int_var__profile_cam_hit15_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_miss_enable: 0x" << int_var__profile_cam_miss_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_info_int_test_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit0_interrupt: 0x" << int_var__profile_cam_hit0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit1_interrupt: 0x" << int_var__profile_cam_hit1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit2_interrupt: 0x" << int_var__profile_cam_hit2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit3_interrupt: 0x" << int_var__profile_cam_hit3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit4_interrupt: 0x" << int_var__profile_cam_hit4_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit5_interrupt: 0x" << int_var__profile_cam_hit5_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit6_interrupt: 0x" << int_var__profile_cam_hit6_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit7_interrupt: 0x" << int_var__profile_cam_hit7_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit8_interrupt: 0x" << int_var__profile_cam_hit8_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit9_interrupt: 0x" << int_var__profile_cam_hit9_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit10_interrupt: 0x" << int_var__profile_cam_hit10_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit11_interrupt: 0x" << int_var__profile_cam_hit11_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit12_interrupt: 0x" << int_var__profile_cam_hit12_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit13_interrupt: 0x" << int_var__profile_cam_hit13_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit14_interrupt: 0x" << int_var__profile_cam_hit14_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_hit15_interrupt: 0x" << int_var__profile_cam_hit15_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".profile_cam_miss_interrupt: 0x" << int_var__profile_cam_miss_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_err_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".miss_sop_enable: 0x" << int_var__miss_sop_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".miss_eop_enable: 0x" << int_var__miss_eop_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_max_size_enable: 0x" << int_var__phv_max_size_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spurious_axi_rsp_enable: 0x" << int_var__spurious_axi_rsp_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spurious_tcam_rsp_enable: 0x" << int_var__spurious_tcam_rsp_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".te2mpu_timeout_enable: 0x" << int_var__te2mpu_timeout_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rdrsp_err_enable: 0x" << int_var__axi_rdrsp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_bad_read_enable: 0x" << int_var__axi_bad_read_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_req_idx_fifo_enable: 0x" << int_var__tcam_req_idx_fifo_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_rsp_idx_fifo_enable: 0x" << int_var__tcam_rsp_idx_fifo_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_req_idx_fifo_enable: 0x" << int_var__mpu_req_idx_fifo_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_req_idx_fifo_enable: 0x" << int_var__axi_req_idx_fifo_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".proc_tbl_vld_wo_proc_enable: 0x" << int_var__proc_tbl_vld_wo_proc_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend_wo_wb_enable: 0x" << int_var__pend_wo_wb_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend1_wo_pend0_enable: 0x" << int_var__pend1_wo_pend0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".both_pend_down_enable: 0x" << int_var__both_pend_down_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend_wo_proc_down_enable: 0x" << int_var__pend_wo_proc_down_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".both_pend_went_up_enable: 0x" << int_var__both_pend_went_up_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loaded_but_no_proc_enable: 0x" << int_var__loaded_but_no_proc_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loaded_but_no_proc_tbl_vld_enable: 0x" << int_var__loaded_but_no_proc_tbl_vld_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_err_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".miss_sop_interrupt: 0x" << int_var__miss_sop_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".miss_eop_interrupt: 0x" << int_var__miss_eop_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_max_size_interrupt: 0x" << int_var__phv_max_size_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spurious_axi_rsp_interrupt: 0x" << int_var__spurious_axi_rsp_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spurious_tcam_rsp_interrupt: 0x" << int_var__spurious_tcam_rsp_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".te2mpu_timeout_interrupt: 0x" << int_var__te2mpu_timeout_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rdrsp_err_interrupt: 0x" << int_var__axi_rdrsp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_bad_read_interrupt: 0x" << int_var__axi_bad_read_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_req_idx_fifo_interrupt: 0x" << int_var__tcam_req_idx_fifo_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_rsp_idx_fifo_interrupt: 0x" << int_var__tcam_rsp_idx_fifo_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_req_idx_fifo_interrupt: 0x" << int_var__mpu_req_idx_fifo_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_req_idx_fifo_interrupt: 0x" << int_var__axi_req_idx_fifo_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".proc_tbl_vld_wo_proc_interrupt: 0x" << int_var__proc_tbl_vld_wo_proc_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend_wo_wb_interrupt: 0x" << int_var__pend_wo_wb_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend1_wo_pend0_interrupt: 0x" << int_var__pend1_wo_pend0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".both_pend_down_interrupt: 0x" << int_var__both_pend_down_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pend_wo_proc_down_interrupt: 0x" << int_var__pend_wo_proc_down_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".both_pend_went_up_interrupt: 0x" << int_var__both_pend_went_up_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loaded_but_no_proc_interrupt: 0x" << int_var__loaded_but_no_proc_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loaded_but_no_proc_tbl_vld_interrupt: 0x" << int_var__loaded_but_no_proc_tbl_vld_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_err_interrupt: 0x" << int_var__int_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_info_interrupt: 0x" << int_var__int_info_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_err_enable: 0x" << int_var__int_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_info_enable: 0x" << int_var__int_info_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode0: 0x" << int_var__km_mode0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode1: 0x" << int_var__km_mode1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode2: 0x" << int_var__km_mode2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode3: 0x" << int_var__km_mode3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile0: 0x" << int_var__km_profile0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile1: 0x" << int_var__km_profile1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile2: 0x" << int_var__km_profile2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile3: 0x" << int_var__km_profile3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key0: 0x" << int_var__km_new_key0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key1: 0x" << int_var__km_new_key1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key2: 0x" << int_var__km_new_key2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key3: 0x" << int_var__km_new_key3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lkup: 0x" << int_var__lkup << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tableid: 0x" << int_var__tableid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_sel: 0x" << int_var__hash_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_chain: 0x" << int_var__hash_chain << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_store: 0x" << int_var__hash_store << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_dhs_table_profile_ctrl_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 192; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_CNT_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".reason0: 0x" << int_var__reason0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reason1: 0x" << int_var__reason1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reason2: 0x" << int_var__reason2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".reason3: 0x" << int_var__reason3 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_hbm_addr_align_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".run: 0x" << int_var__run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_bist_done_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pass: 0x" << int_var__pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fail: 0x" << int_var__fail << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_peek_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".signals: 0x" << int_var__signals << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_bad_axi_read_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_axi_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_wait_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".entry_dat: 0x" << int_var__entry_dat << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_wait_fsm_states_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".entry0: 0x" << int_var__entry0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry1: 0x" << int_var__entry1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry2: 0x" << int_var__entry2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry3: 0x" << int_var__entry3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry4: 0x" << int_var__entry4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry5: 0x" << int_var__entry5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry6: 0x" << int_var__entry6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry7: 0x" << int_var__entry7 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry8: 0x" << int_var__entry8 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry9: 0x" << int_var__entry9 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry10: 0x" << int_var__entry10 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry11: 0x" << int_var__entry11 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry12: 0x" << int_var__entry12 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry13: 0x" << int_var__entry13 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry14: 0x" << int_var__entry14 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".entry15: 0x" << int_var__entry15 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_wait_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cam: 0x" << int_var__cam << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".haz: 0x" << int_var__haz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fsm: 0x" << int_var__fsm << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_timeout_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".te2mpu: 0x" << int_var__te2mpu << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_page_size_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_read_wait_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".idx: 0x" << int_var__idx << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_addr_target_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".host: 0x" << int_var__host << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram: 0x" << int_var__sram << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_sta_debug_bus_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".vec: 0x" << int_var__vec << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_debug_bus_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rdata_sel: 0x" << int_var__axi_rdata_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_key_sel: 0x" << int_var__tcam_key_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_key_sel: 0x" << int_var__hash_key_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_mpu_out_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_tcam_rsp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_tcam_req_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_axi_rdrsp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_axi_rdreq_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_phv_out_eop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_phv_out_sop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_phv_in_eop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cnt_phv_in_sop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_dhs_single_step_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lkup: 0x" << int_var__lkup << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu: 0x" << int_var__mpu << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_km_profile_bit_loc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bit_loc: 0x" << int_var__bit_loc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_km_profile_bit_sel_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bit_sel: 0x" << int_var__bit_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_km_profile_byte_sel_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".byte_sel: 0x" << int_var__byte_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_mpu_const_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_property_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi: 0x" << int_var__axi << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".key_mask_hi: 0x" << int_var__key_mask_hi << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".key_mask_lo: 0x" << int_var__key_mask_lo << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fullkey_km_sel0: 0x" << int_var__fullkey_km_sel0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fullkey_km_sel1: 0x" << int_var__fullkey_km_sel1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock_en: 0x" << int_var__lock_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tbl_entry_sz_raw: 0x" << int_var__tbl_entry_sz_raw << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_dyn: 0x" << int_var__mpu_pc_dyn << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc: 0x" << int_var__mpu_pc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_raw: 0x" << int_var__mpu_pc_raw << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_ofst_err: 0x" << int_var__mpu_pc_ofst_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_vec: 0x" << int_var__mpu_vec << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_base: 0x" << int_var__addr_base << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_vf_id_en: 0x" << int_var__addr_vf_id_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_vf_id_loc: 0x" << int_var__addr_vf_id_loc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_shift: 0x" << int_var__addr_shift << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".chain_shift: 0x" << int_var__chain_shift << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_sz: 0x" << int_var__addr_sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lg2_entry_size: 0x" << int_var__lg2_entry_size << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_bypass_cnt: 0x" << int_var__max_bypass_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock_en_raw: 0x" << int_var__lock_en_raw << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_lb: 0x" << int_var__mpu_lb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_loc: 0x" << int_var__mpu_pc_loc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_ofst_shift: 0x" << int_var__mpu_pc_ofst_shift << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".oflow_base_idx: 0x" << int_var__oflow_base_idx << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".en_pc_axi_err: 0x" << int_var__en_pc_axi_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".en_pc_phv_err: 0x" << int_var__en_pc_phv_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".memory_only: 0x" << int_var__memory_only << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_results: 0x" << int_var__mpu_results << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".seq_base: 0x" << int_var__seq_base << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".adv_phv_flit: 0x" << int_var__adv_phv_flit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done: 0x" << int_var__done << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_profile_cam_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask: 0x" << int_var__mask << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_table_profile_key_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel0: 0x" << int_var__sel0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel1: 0x" << int_var__sel1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel2: 0x" << int_var__sel2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel3: 0x" << int_var__sel3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel4: 0x" << int_var__sel4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel5: 0x" << int_var__sel5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel6: 0x" << int_var__sel6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel7: 0x" << int_var__sel7 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_cfg_global_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sw_rst: 0x" << int_var__sw_rst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".step_key: 0x" << int_var__step_key << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".step_lkup: 0x" << int_var__step_lkup << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".step_mpu: 0x" << int_var__step_mpu << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_global.show();
    cfg_table_profile_key.show();
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile_cam.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile_cam[ii].show();
    }
    #endif
    
    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile_ctrl_sram_ext.show(); // large_array
    #else
    for(int ii = 0; ii < 192; ii++) {
        cfg_table_profile_ctrl_sram_ext[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_property.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_property[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_mpu_const.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_mpu_const[ii].show();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_byte_sel.show(); // large_array
    #else
    for(int ii = 0; ii < 256; ii++) {
        cfg_km_profile_byte_sel[ii].show();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_bit_sel.show(); // large_array
    #else
    for(int ii = 0; ii < 128; ii++) {
        cfg_km_profile_bit_sel[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_bit_loc.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_km_profile_bit_loc[ii].show();
    }
    #endif
    
    dhs_single_step.show();
    cnt_phv_in_sop.show();
    cnt_phv_in_eop.show();
    cnt_phv_out_sop.show();
    cnt_phv_out_eop.show();
    cnt_axi_rdreq.show();
    cnt_axi_rdrsp.show();
    cnt_tcam_req.show();
    cnt_tcam_rsp.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_mpu_out.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cnt_mpu_out[ii].show();
    }
    #endif
    
    cfg_debug_bus.show();
    sta_debug_bus.show();
    cfg_addr_target.show();
    cfg_read_wait_entry.show();
    cfg_page_size.show();
    cfg_timeout.show();
    sta_wait_entry.show();
    sta_wait_fsm_states.show();
    sta_wait.show();
    sta_axi_err.show();
    sta_bad_axi_read.show();
    csr_intr.show();
    sta_peek.show();
    sta_bist_done.show();
    cfg_bist.show();
    cfg_hbm_addr_align.show();
    CNT_stall.show();
    dhs_table_profile_ctrl_sram.show();
    int_groups.show();
    int_err.show();
    int_info.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_te_csr_int_info_int_enable_clear_t::get_width() const {
    return cap_te_csr_int_info_int_enable_clear_t::s_get_width();

}

int cap_te_csr_int_info_int_test_set_t::get_width() const {
    return cap_te_csr_int_info_int_test_set_t::s_get_width();

}

int cap_te_csr_int_info_t::get_width() const {
    return cap_te_csr_int_info_t::s_get_width();

}

int cap_te_csr_int_err_int_enable_clear_t::get_width() const {
    return cap_te_csr_int_err_int_enable_clear_t::s_get_width();

}

int cap_te_csr_int_err_intreg_t::get_width() const {
    return cap_te_csr_int_err_intreg_t::s_get_width();

}

int cap_te_csr_int_err_t::get_width() const {
    return cap_te_csr_int_err_t::s_get_width();

}

int cap_te_csr_intreg_status_t::get_width() const {
    return cap_te_csr_intreg_status_t::s_get_width();

}

int cap_te_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_te_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_te_csr_intgrp_status_t::get_width() const {
    return cap_te_csr_intgrp_status_t::s_get_width();

}

int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::get_width() const {
    return cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::s_get_width();

}

int cap_te_csr_dhs_table_profile_ctrl_sram_t::get_width() const {
    return cap_te_csr_dhs_table_profile_ctrl_sram_t::s_get_width();

}

int cap_te_csr_CNT_stall_t::get_width() const {
    return cap_te_csr_CNT_stall_t::s_get_width();

}

int cap_te_csr_cfg_hbm_addr_align_t::get_width() const {
    return cap_te_csr_cfg_hbm_addr_align_t::s_get_width();

}

int cap_te_csr_cfg_bist_t::get_width() const {
    return cap_te_csr_cfg_bist_t::s_get_width();

}

int cap_te_csr_sta_bist_done_t::get_width() const {
    return cap_te_csr_sta_bist_done_t::s_get_width();

}

int cap_te_csr_sta_peek_t::get_width() const {
    return cap_te_csr_sta_peek_t::s_get_width();

}

int cap_te_csr_csr_intr_t::get_width() const {
    return cap_te_csr_csr_intr_t::s_get_width();

}

int cap_te_csr_sta_bad_axi_read_t::get_width() const {
    return cap_te_csr_sta_bad_axi_read_t::s_get_width();

}

int cap_te_csr_sta_axi_err_t::get_width() const {
    return cap_te_csr_sta_axi_err_t::s_get_width();

}

int cap_te_csr_sta_wait_t::get_width() const {
    return cap_te_csr_sta_wait_t::s_get_width();

}

int cap_te_csr_sta_wait_fsm_states_t::get_width() const {
    return cap_te_csr_sta_wait_fsm_states_t::s_get_width();

}

int cap_te_csr_sta_wait_entry_t::get_width() const {
    return cap_te_csr_sta_wait_entry_t::s_get_width();

}

int cap_te_csr_cfg_timeout_t::get_width() const {
    return cap_te_csr_cfg_timeout_t::s_get_width();

}

int cap_te_csr_cfg_page_size_t::get_width() const {
    return cap_te_csr_cfg_page_size_t::s_get_width();

}

int cap_te_csr_cfg_read_wait_entry_t::get_width() const {
    return cap_te_csr_cfg_read_wait_entry_t::s_get_width();

}

int cap_te_csr_cfg_addr_target_t::get_width() const {
    return cap_te_csr_cfg_addr_target_t::s_get_width();

}

int cap_te_csr_sta_debug_bus_t::get_width() const {
    return cap_te_csr_sta_debug_bus_t::s_get_width();

}

int cap_te_csr_cfg_debug_bus_t::get_width() const {
    return cap_te_csr_cfg_debug_bus_t::s_get_width();

}

int cap_te_csr_cnt_mpu_out_t::get_width() const {
    return cap_te_csr_cnt_mpu_out_t::s_get_width();

}

int cap_te_csr_cnt_tcam_rsp_t::get_width() const {
    return cap_te_csr_cnt_tcam_rsp_t::s_get_width();

}

int cap_te_csr_cnt_tcam_req_t::get_width() const {
    return cap_te_csr_cnt_tcam_req_t::s_get_width();

}

int cap_te_csr_cnt_axi_rdrsp_t::get_width() const {
    return cap_te_csr_cnt_axi_rdrsp_t::s_get_width();

}

int cap_te_csr_cnt_axi_rdreq_t::get_width() const {
    return cap_te_csr_cnt_axi_rdreq_t::s_get_width();

}

int cap_te_csr_cnt_phv_out_eop_t::get_width() const {
    return cap_te_csr_cnt_phv_out_eop_t::s_get_width();

}

int cap_te_csr_cnt_phv_out_sop_t::get_width() const {
    return cap_te_csr_cnt_phv_out_sop_t::s_get_width();

}

int cap_te_csr_cnt_phv_in_eop_t::get_width() const {
    return cap_te_csr_cnt_phv_in_eop_t::s_get_width();

}

int cap_te_csr_cnt_phv_in_sop_t::get_width() const {
    return cap_te_csr_cnt_phv_in_sop_t::s_get_width();

}

int cap_te_csr_dhs_single_step_t::get_width() const {
    return cap_te_csr_dhs_single_step_t::s_get_width();

}

int cap_te_csr_cfg_km_profile_bit_loc_t::get_width() const {
    return cap_te_csr_cfg_km_profile_bit_loc_t::s_get_width();

}

int cap_te_csr_cfg_km_profile_bit_sel_t::get_width() const {
    return cap_te_csr_cfg_km_profile_bit_sel_t::s_get_width();

}

int cap_te_csr_cfg_km_profile_byte_sel_t::get_width() const {
    return cap_te_csr_cfg_km_profile_byte_sel_t::s_get_width();

}

int cap_te_csr_cfg_table_mpu_const_t::get_width() const {
    return cap_te_csr_cfg_table_mpu_const_t::s_get_width();

}

int cap_te_csr_cfg_table_property_t::get_width() const {
    return cap_te_csr_cfg_table_property_t::s_get_width();

}

int cap_te_csr_cfg_table_profile_t::get_width() const {
    return cap_te_csr_cfg_table_profile_t::s_get_width();

}

int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::get_width() const {
    return cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::s_get_width();

}

int cap_te_csr_cfg_table_profile_cam_t::get_width() const {
    return cap_te_csr_cfg_table_profile_cam_t::s_get_width();

}

int cap_te_csr_cfg_table_profile_key_t::get_width() const {
    return cap_te_csr_cfg_table_profile_key_t::s_get_width();

}

int cap_te_csr_cfg_global_t::get_width() const {
    return cap_te_csr_cfg_global_t::s_get_width();

}

int cap_te_csr_base_t::get_width() const {
    return cap_te_csr_base_t::s_get_width();

}

int cap_te_csr_t::get_width() const {
    return cap_te_csr_t::s_get_width();

}

int cap_te_csr_int_info_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // profile_cam_hit0_enable
    _count += 1; // profile_cam_hit1_enable
    _count += 1; // profile_cam_hit2_enable
    _count += 1; // profile_cam_hit3_enable
    _count += 1; // profile_cam_hit4_enable
    _count += 1; // profile_cam_hit5_enable
    _count += 1; // profile_cam_hit6_enable
    _count += 1; // profile_cam_hit7_enable
    _count += 1; // profile_cam_hit8_enable
    _count += 1; // profile_cam_hit9_enable
    _count += 1; // profile_cam_hit10_enable
    _count += 1; // profile_cam_hit11_enable
    _count += 1; // profile_cam_hit12_enable
    _count += 1; // profile_cam_hit13_enable
    _count += 1; // profile_cam_hit14_enable
    _count += 1; // profile_cam_hit15_enable
    _count += 1; // profile_cam_miss_enable
    return _count;
}

int cap_te_csr_int_info_int_test_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // profile_cam_hit0_interrupt
    _count += 1; // profile_cam_hit1_interrupt
    _count += 1; // profile_cam_hit2_interrupt
    _count += 1; // profile_cam_hit3_interrupt
    _count += 1; // profile_cam_hit4_interrupt
    _count += 1; // profile_cam_hit5_interrupt
    _count += 1; // profile_cam_hit6_interrupt
    _count += 1; // profile_cam_hit7_interrupt
    _count += 1; // profile_cam_hit8_interrupt
    _count += 1; // profile_cam_hit9_interrupt
    _count += 1; // profile_cam_hit10_interrupt
    _count += 1; // profile_cam_hit11_interrupt
    _count += 1; // profile_cam_hit12_interrupt
    _count += 1; // profile_cam_hit13_interrupt
    _count += 1; // profile_cam_hit14_interrupt
    _count += 1; // profile_cam_hit15_interrupt
    _count += 1; // profile_cam_miss_interrupt
    return _count;
}

int cap_te_csr_int_info_t::s_get_width() {
    int _count = 0;

    _count += cap_te_csr_int_info_int_test_set_t::s_get_width(); // intreg
    _count += cap_te_csr_int_info_int_test_set_t::s_get_width(); // int_test_set
    _count += cap_te_csr_int_info_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_te_csr_int_info_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_te_csr_int_err_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // miss_sop_enable
    _count += 1; // miss_eop_enable
    _count += 1; // phv_max_size_enable
    _count += 1; // spurious_axi_rsp_enable
    _count += 1; // spurious_tcam_rsp_enable
    _count += 1; // te2mpu_timeout_enable
    _count += 1; // axi_rdrsp_err_enable
    _count += 1; // axi_bad_read_enable
    _count += 1; // tcam_req_idx_fifo_enable
    _count += 1; // tcam_rsp_idx_fifo_enable
    _count += 1; // mpu_req_idx_fifo_enable
    _count += 1; // axi_req_idx_fifo_enable
    _count += 1; // proc_tbl_vld_wo_proc_enable
    _count += 1; // pend_wo_wb_enable
    _count += 1; // pend1_wo_pend0_enable
    _count += 1; // both_pend_down_enable
    _count += 1; // pend_wo_proc_down_enable
    _count += 1; // both_pend_went_up_enable
    _count += 1; // loaded_but_no_proc_enable
    _count += 1; // loaded_but_no_proc_tbl_vld_enable
    return _count;
}

int cap_te_csr_int_err_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // miss_sop_interrupt
    _count += 1; // miss_eop_interrupt
    _count += 1; // phv_max_size_interrupt
    _count += 1; // spurious_axi_rsp_interrupt
    _count += 1; // spurious_tcam_rsp_interrupt
    _count += 1; // te2mpu_timeout_interrupt
    _count += 1; // axi_rdrsp_err_interrupt
    _count += 1; // axi_bad_read_interrupt
    _count += 1; // tcam_req_idx_fifo_interrupt
    _count += 1; // tcam_rsp_idx_fifo_interrupt
    _count += 1; // mpu_req_idx_fifo_interrupt
    _count += 1; // axi_req_idx_fifo_interrupt
    _count += 1; // proc_tbl_vld_wo_proc_interrupt
    _count += 1; // pend_wo_wb_interrupt
    _count += 1; // pend1_wo_pend0_interrupt
    _count += 1; // both_pend_down_interrupt
    _count += 1; // pend_wo_proc_down_interrupt
    _count += 1; // both_pend_went_up_interrupt
    _count += 1; // loaded_but_no_proc_interrupt
    _count += 1; // loaded_but_no_proc_tbl_vld_interrupt
    return _count;
}

int cap_te_csr_int_err_t::s_get_width() {
    int _count = 0;

    _count += cap_te_csr_int_err_intreg_t::s_get_width(); // intreg
    _count += cap_te_csr_int_err_intreg_t::s_get_width(); // int_test_set
    _count += cap_te_csr_int_err_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_te_csr_int_err_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_te_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_err_interrupt
    _count += 1; // int_info_interrupt
    return _count;
}

int cap_te_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_err_enable
    _count += 1; // int_info_enable
    return _count;
}

int cap_te_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_te_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_te_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_te_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 2; // km_mode0
    _count += 2; // km_mode1
    _count += 2; // km_mode2
    _count += 2; // km_mode3
    _count += 3; // km_profile0
    _count += 3; // km_profile1
    _count += 3; // km_profile2
    _count += 3; // km_profile3
    _count += 1; // km_new_key0
    _count += 1; // km_new_key1
    _count += 1; // km_new_key2
    _count += 1; // km_new_key3
    _count += 3; // lkup
    _count += 4; // tableid
    _count += 3; // hash_sel
    _count += 2; // hash_chain
    _count += 2; // hash_store
    return _count;
}

int cap_te_csr_dhs_table_profile_ctrl_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::s_get_width() * 192); // entry
    return _count;
}

int cap_te_csr_CNT_stall_t::s_get_width() {
    int _count = 0;

    _count += 16; // reason0
    _count += 16; // reason1
    _count += 16; // reason2
    _count += 16; // reason3
    return _count;
}

int cap_te_csr_cfg_hbm_addr_align_t::s_get_width() {
    int _count = 0;

    _count += 1; // en
    return _count;
}

int cap_te_csr_cfg_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // run
    return _count;
}

int cap_te_csr_sta_bist_done_t::s_get_width() {
    int _count = 0;

    _count += 1; // pass
    _count += 1; // fail
    return _count;
}

int cap_te_csr_sta_peek_t::s_get_width() {
    int _count = 0;

    _count += 64; // signals
    return _count;
}

int cap_te_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_te_csr_sta_bad_axi_read_t::s_get_width() {
    int _count = 0;

    _count += 65; // addr
    _count += 7; // id
    _count += 4; // len
    _count += 3; // sz
    return _count;
}

int cap_te_csr_sta_axi_err_t::s_get_width() {
    int _count = 0;

    _count += 2; // resp
    _count += 7; // id
    return _count;
}

int cap_te_csr_sta_wait_t::s_get_width() {
    int _count = 0;

    _count += 512; // entry_dat
    return _count;
}

int cap_te_csr_sta_wait_fsm_states_t::s_get_width() {
    int _count = 0;

    _count += 3; // entry0
    _count += 3; // entry1
    _count += 3; // entry2
    _count += 3; // entry3
    _count += 3; // entry4
    _count += 3; // entry5
    _count += 3; // entry6
    _count += 3; // entry7
    _count += 3; // entry8
    _count += 3; // entry9
    _count += 3; // entry10
    _count += 3; // entry11
    _count += 3; // entry12
    _count += 3; // entry13
    _count += 3; // entry14
    _count += 3; // entry15
    return _count;
}

int cap_te_csr_sta_wait_entry_t::s_get_width() {
    int _count = 0;

    _count += 590; // key
    _count += 108; // cam
    _count += 17; // haz
    _count += 3; // fsm
    return _count;
}

int cap_te_csr_cfg_timeout_t::s_get_width() {
    int _count = 0;

    _count += 24; // te2mpu
    return _count;
}

int cap_te_csr_cfg_page_size_t::s_get_width() {
    int _count = 0;

    _count += 4; // sz
    return _count;
}

int cap_te_csr_cfg_read_wait_entry_t::s_get_width() {
    int _count = 0;

    _count += 4; // idx
    return _count;
}

int cap_te_csr_cfg_addr_target_t::s_get_width() {
    int _count = 0;

    _count += 6; // host
    _count += 7; // sram
    return _count;
}

int cap_te_csr_sta_debug_bus_t::s_get_width() {
    int _count = 0;

    _count += 256; // vec
    return _count;
}

int cap_te_csr_cfg_debug_bus_t::s_get_width() {
    int _count = 0;

    _count += 3; // axi_rdata_sel
    _count += 3; // tcam_key_sel
    _count += 3; // hash_key_sel
    _count += 4; // sel
    return _count;
}

int cap_te_csr_cnt_mpu_out_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_tcam_rsp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_tcam_req_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_axi_rdrsp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_axi_rdreq_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_phv_out_eop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_phv_out_sop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_phv_in_eop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_cnt_phv_in_sop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_te_csr_dhs_single_step_t::s_get_width() {
    int _count = 0;

    _count += 1; // key
    _count += 1; // lkup
    _count += 1; // mpu
    return _count;
}

int cap_te_csr_cfg_km_profile_bit_loc_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 5; // bit_loc
    return _count;
}

int cap_te_csr_cfg_km_profile_bit_sel_t::s_get_width() {
    int _count = 0;

    _count += 13; // bit_sel
    return _count;
}

int cap_te_csr_cfg_km_profile_byte_sel_t::s_get_width() {
    int _count = 0;

    _count += 10; // byte_sel
    return _count;
}

int cap_te_csr_cfg_table_mpu_const_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_te_csr_cfg_table_property_t::s_get_width() {
    int _count = 0;

    _count += 1; // axi
    _count += 10; // key_mask_hi
    _count += 9; // key_mask_lo
    _count += 2; // fullkey_km_sel0
    _count += 2; // fullkey_km_sel1
    _count += 1; // lock_en
    _count += 1; // tbl_entry_sz_raw
    _count += 1; // mpu_pc_dyn
    _count += 28; // mpu_pc
    _count += 1; // mpu_pc_raw
    _count += 9; // mpu_pc_ofst_err
    _count += 4; // mpu_vec
    _count += 64; // addr_base
    _count += 1; // addr_vf_id_en
    _count += 6; // addr_vf_id_loc
    _count += 5; // addr_shift
    _count += 6; // chain_shift
    _count += 7; // addr_sz
    _count += 3; // lg2_entry_size
    _count += 10; // max_bypass_cnt
    _count += 1; // lock_en_raw
    _count += 1; // mpu_lb
    _count += 6; // mpu_pc_loc
    _count += 4; // mpu_pc_ofst_shift
    _count += 20; // oflow_base_idx
    _count += 1; // en_pc_axi_err
    _count += 1; // en_pc_phv_err
    _count += 1; // memory_only
    return _count;
}

int cap_te_csr_cfg_table_profile_t::s_get_width() {
    int _count = 0;

    _count += 4; // mpu_results
    _count += 8; // seq_base
    return _count;
}

int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::s_get_width() {
    int _count = 0;

    _count += 1; // adv_phv_flit
    _count += 1; // done
    return _count;
}

int cap_te_csr_cfg_table_profile_cam_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 8; // value
    _count += 8; // mask
    return _count;
}

int cap_te_csr_cfg_table_profile_key_t::s_get_width() {
    int _count = 0;

    _count += 9; // sel0
    _count += 9; // sel1
    _count += 9; // sel2
    _count += 9; // sel3
    _count += 9; // sel4
    _count += 9; // sel5
    _count += 9; // sel6
    _count += 9; // sel7
    return _count;
}

int cap_te_csr_cfg_global_t::s_get_width() {
    int _count = 0;

    _count += 1; // sw_rst
    _count += 1; // step_key
    _count += 1; // step_lkup
    _count += 1; // step_mpu
    return _count;
}

int cap_te_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_te_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_te_csr_base_t::s_get_width(); // base
    _count += cap_te_csr_cfg_global_t::s_get_width(); // cfg_global
    _count += cap_te_csr_cfg_table_profile_key_t::s_get_width(); // cfg_table_profile_key
    _count += (cap_te_csr_cfg_table_profile_cam_t::s_get_width() * 16); // cfg_table_profile_cam
    _count += (cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::s_get_width() * 192); // cfg_table_profile_ctrl_sram_ext
    _count += (cap_te_csr_cfg_table_profile_t::s_get_width() * 16); // cfg_table_profile
    _count += (cap_te_csr_cfg_table_property_t::s_get_width() * 16); // cfg_table_property
    _count += (cap_te_csr_cfg_table_mpu_const_t::s_get_width() * 16); // cfg_table_mpu_const
    _count += (cap_te_csr_cfg_km_profile_byte_sel_t::s_get_width() * 256); // cfg_km_profile_byte_sel
    _count += (cap_te_csr_cfg_km_profile_bit_sel_t::s_get_width() * 128); // cfg_km_profile_bit_sel
    _count += (cap_te_csr_cfg_km_profile_bit_loc_t::s_get_width() * 16); // cfg_km_profile_bit_loc
    _count += cap_te_csr_dhs_single_step_t::s_get_width(); // dhs_single_step
    _count += cap_te_csr_cnt_phv_in_sop_t::s_get_width(); // cnt_phv_in_sop
    _count += cap_te_csr_cnt_phv_in_eop_t::s_get_width(); // cnt_phv_in_eop
    _count += cap_te_csr_cnt_phv_out_sop_t::s_get_width(); // cnt_phv_out_sop
    _count += cap_te_csr_cnt_phv_out_eop_t::s_get_width(); // cnt_phv_out_eop
    _count += cap_te_csr_cnt_axi_rdreq_t::s_get_width(); // cnt_axi_rdreq
    _count += cap_te_csr_cnt_axi_rdrsp_t::s_get_width(); // cnt_axi_rdrsp
    _count += cap_te_csr_cnt_tcam_req_t::s_get_width(); // cnt_tcam_req
    _count += cap_te_csr_cnt_tcam_rsp_t::s_get_width(); // cnt_tcam_rsp
    _count += (cap_te_csr_cnt_mpu_out_t::s_get_width() * 4); // cnt_mpu_out
    _count += cap_te_csr_cfg_debug_bus_t::s_get_width(); // cfg_debug_bus
    _count += cap_te_csr_sta_debug_bus_t::s_get_width(); // sta_debug_bus
    _count += cap_te_csr_cfg_addr_target_t::s_get_width(); // cfg_addr_target
    _count += cap_te_csr_cfg_read_wait_entry_t::s_get_width(); // cfg_read_wait_entry
    _count += cap_te_csr_cfg_page_size_t::s_get_width(); // cfg_page_size
    _count += cap_te_csr_cfg_timeout_t::s_get_width(); // cfg_timeout
    _count += cap_te_csr_sta_wait_entry_t::s_get_width(); // sta_wait_entry
    _count += cap_te_csr_sta_wait_fsm_states_t::s_get_width(); // sta_wait_fsm_states
    _count += cap_te_csr_sta_wait_t::s_get_width(); // sta_wait
    _count += cap_te_csr_sta_axi_err_t::s_get_width(); // sta_axi_err
    _count += cap_te_csr_sta_bad_axi_read_t::s_get_width(); // sta_bad_axi_read
    _count += cap_te_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_te_csr_sta_peek_t::s_get_width(); // sta_peek
    _count += cap_te_csr_sta_bist_done_t::s_get_width(); // sta_bist_done
    _count += cap_te_csr_cfg_bist_t::s_get_width(); // cfg_bist
    _count += cap_te_csr_cfg_hbm_addr_align_t::s_get_width(); // cfg_hbm_addr_align
    _count += cap_te_csr_CNT_stall_t::s_get_width(); // CNT_stall
    _count += cap_te_csr_dhs_table_profile_ctrl_sram_t::s_get_width(); // dhs_table_profile_ctrl_sram
    _count += cap_te_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_te_csr_int_err_t::s_get_width(); // int_err
    _count += cap_te_csr_int_info_t::s_get_width(); // int_info
    return _count;
}

void cap_te_csr_int_info_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__profile_cam_hit0_enable = _val.convert_to< profile_cam_hit0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit1_enable = _val.convert_to< profile_cam_hit1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit2_enable = _val.convert_to< profile_cam_hit2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit3_enable = _val.convert_to< profile_cam_hit3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit4_enable = _val.convert_to< profile_cam_hit4_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit5_enable = _val.convert_to< profile_cam_hit5_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit6_enable = _val.convert_to< profile_cam_hit6_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit7_enable = _val.convert_to< profile_cam_hit7_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit8_enable = _val.convert_to< profile_cam_hit8_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit9_enable = _val.convert_to< profile_cam_hit9_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit10_enable = _val.convert_to< profile_cam_hit10_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit11_enable = _val.convert_to< profile_cam_hit11_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit12_enable = _val.convert_to< profile_cam_hit12_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit13_enable = _val.convert_to< profile_cam_hit13_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit14_enable = _val.convert_to< profile_cam_hit14_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit15_enable = _val.convert_to< profile_cam_hit15_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_miss_enable = _val.convert_to< profile_cam_miss_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_int_info_int_test_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__profile_cam_hit0_interrupt = _val.convert_to< profile_cam_hit0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit1_interrupt = _val.convert_to< profile_cam_hit1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit2_interrupt = _val.convert_to< profile_cam_hit2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit3_interrupt = _val.convert_to< profile_cam_hit3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit4_interrupt = _val.convert_to< profile_cam_hit4_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit5_interrupt = _val.convert_to< profile_cam_hit5_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit6_interrupt = _val.convert_to< profile_cam_hit6_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit7_interrupt = _val.convert_to< profile_cam_hit7_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit8_interrupt = _val.convert_to< profile_cam_hit8_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit9_interrupt = _val.convert_to< profile_cam_hit9_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit10_interrupt = _val.convert_to< profile_cam_hit10_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit11_interrupt = _val.convert_to< profile_cam_hit11_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit12_interrupt = _val.convert_to< profile_cam_hit12_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit13_interrupt = _val.convert_to< profile_cam_hit13_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit14_interrupt = _val.convert_to< profile_cam_hit14_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_hit15_interrupt = _val.convert_to< profile_cam_hit15_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__profile_cam_miss_interrupt = _val.convert_to< profile_cam_miss_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_int_info_t::all(const cpp_int & in_val) {
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

void cap_te_csr_int_err_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__miss_sop_enable = _val.convert_to< miss_sop_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__miss_eop_enable = _val.convert_to< miss_eop_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_max_size_enable = _val.convert_to< phv_max_size_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spurious_axi_rsp_enable = _val.convert_to< spurious_axi_rsp_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spurious_tcam_rsp_enable = _val.convert_to< spurious_tcam_rsp_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__te2mpu_timeout_enable = _val.convert_to< te2mpu_timeout_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_rdrsp_err_enable = _val.convert_to< axi_rdrsp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_bad_read_enable = _val.convert_to< axi_bad_read_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tcam_req_idx_fifo_enable = _val.convert_to< tcam_req_idx_fifo_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tcam_rsp_idx_fifo_enable = _val.convert_to< tcam_rsp_idx_fifo_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_req_idx_fifo_enable = _val.convert_to< mpu_req_idx_fifo_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_req_idx_fifo_enable = _val.convert_to< axi_req_idx_fifo_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__proc_tbl_vld_wo_proc_enable = _val.convert_to< proc_tbl_vld_wo_proc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend_wo_wb_enable = _val.convert_to< pend_wo_wb_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend1_wo_pend0_enable = _val.convert_to< pend1_wo_pend0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__both_pend_down_enable = _val.convert_to< both_pend_down_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend_wo_proc_down_enable = _val.convert_to< pend_wo_proc_down_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__both_pend_went_up_enable = _val.convert_to< both_pend_went_up_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__loaded_but_no_proc_enable = _val.convert_to< loaded_but_no_proc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__loaded_but_no_proc_tbl_vld_enable = _val.convert_to< loaded_but_no_proc_tbl_vld_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_int_err_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__miss_sop_interrupt = _val.convert_to< miss_sop_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__miss_eop_interrupt = _val.convert_to< miss_eop_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_max_size_interrupt = _val.convert_to< phv_max_size_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spurious_axi_rsp_interrupt = _val.convert_to< spurious_axi_rsp_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spurious_tcam_rsp_interrupt = _val.convert_to< spurious_tcam_rsp_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__te2mpu_timeout_interrupt = _val.convert_to< te2mpu_timeout_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_rdrsp_err_interrupt = _val.convert_to< axi_rdrsp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_bad_read_interrupt = _val.convert_to< axi_bad_read_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tcam_req_idx_fifo_interrupt = _val.convert_to< tcam_req_idx_fifo_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tcam_rsp_idx_fifo_interrupt = _val.convert_to< tcam_rsp_idx_fifo_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_req_idx_fifo_interrupt = _val.convert_to< mpu_req_idx_fifo_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_req_idx_fifo_interrupt = _val.convert_to< axi_req_idx_fifo_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__proc_tbl_vld_wo_proc_interrupt = _val.convert_to< proc_tbl_vld_wo_proc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend_wo_wb_interrupt = _val.convert_to< pend_wo_wb_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend1_wo_pend0_interrupt = _val.convert_to< pend1_wo_pend0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__both_pend_down_interrupt = _val.convert_to< both_pend_down_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pend_wo_proc_down_interrupt = _val.convert_to< pend_wo_proc_down_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__both_pend_went_up_interrupt = _val.convert_to< both_pend_went_up_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__loaded_but_no_proc_interrupt = _val.convert_to< loaded_but_no_proc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__loaded_but_no_proc_tbl_vld_interrupt = _val.convert_to< loaded_but_no_proc_tbl_vld_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_int_err_t::all(const cpp_int & in_val) {
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

void cap_te_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_err_interrupt = _val.convert_to< int_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_info_interrupt = _val.convert_to< int_info_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_err_enable = _val.convert_to< int_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_info_enable = _val.convert_to< int_info_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__km_mode0 = _val.convert_to< km_mode0_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__km_mode1 = _val.convert_to< km_mode1_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__km_mode2 = _val.convert_to< km_mode2_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__km_mode3 = _val.convert_to< km_mode3_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__km_profile0 = _val.convert_to< km_profile0_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__km_profile1 = _val.convert_to< km_profile1_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__km_profile2 = _val.convert_to< km_profile2_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__km_profile3 = _val.convert_to< km_profile3_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__km_new_key0 = _val.convert_to< km_new_key0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__km_new_key1 = _val.convert_to< km_new_key1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__km_new_key2 = _val.convert_to< km_new_key2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__km_new_key3 = _val.convert_to< km_new_key3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__tableid = _val.convert_to< tableid_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__hash_chain = _val.convert_to< hash_chain_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__hash_store = _val.convert_to< hash_store_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 192; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_te_csr_CNT_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__reason0 = _val.convert_to< reason0_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__reason1 = _val.convert_to< reason1_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__reason2 = _val.convert_to< reason2_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__reason3 = _val.convert_to< reason3_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_te_csr_cfg_hbm_addr_align_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__en = _val.convert_to< en_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_cfg_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__run = _val.convert_to< run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_sta_bist_done_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pass = _val.convert_to< pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fail = _val.convert_to< fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_sta_peek_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__signals = _val.convert_to< signals_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_te_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_sta_bad_axi_read_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 65;
    
    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_te_csr_sta_axi_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_te_csr_sta_wait_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__entry_dat = _val.convert_to< entry_dat_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_te_csr_sta_wait_fsm_states_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__entry0 = _val.convert_to< entry0_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry1 = _val.convert_to< entry1_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry2 = _val.convert_to< entry2_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry3 = _val.convert_to< entry3_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry4 = _val.convert_to< entry4_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry5 = _val.convert_to< entry5_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry6 = _val.convert_to< entry6_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry7 = _val.convert_to< entry7_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry8 = _val.convert_to< entry8_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry9 = _val.convert_to< entry9_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry10 = _val.convert_to< entry10_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry11 = _val.convert_to< entry11_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry12 = _val.convert_to< entry12_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry13 = _val.convert_to< entry13_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry14 = _val.convert_to< entry14_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__entry15 = _val.convert_to< entry15_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_te_csr_sta_wait_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__key = _val.convert_to< key_cpp_int_t >()  ;
    _val = _val >> 590;
    
    int_var__cam = _val.convert_to< cam_cpp_int_t >()  ;
    _val = _val >> 108;
    
    int_var__haz = _val.convert_to< haz_cpp_int_t >()  ;
    _val = _val >> 17;
    
    int_var__fsm = _val.convert_to< fsm_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_te_csr_cfg_timeout_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__te2mpu = _val.convert_to< te2mpu_cpp_int_t >()  ;
    _val = _val >> 24;
    
}

void cap_te_csr_cfg_page_size_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_te_csr_cfg_read_wait_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__idx = _val.convert_to< idx_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_te_csr_cfg_addr_target_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__host = _val.convert_to< host_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__sram = _val.convert_to< sram_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_te_csr_sta_debug_bus_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__vec = _val.convert_to< vec_cpp_int_t >()  ;
    _val = _val >> 256;
    
}

void cap_te_csr_cfg_debug_bus_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_rdata_sel = _val.convert_to< axi_rdata_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__tcam_key_sel = _val.convert_to< tcam_key_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__hash_key_sel = _val.convert_to< hash_key_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_te_csr_cnt_mpu_out_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_tcam_rsp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_tcam_req_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_axi_rdrsp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_axi_rdreq_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_phv_out_eop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_phv_out_sop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_phv_in_eop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_cnt_phv_in_sop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_dhs_single_step_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__key = _val.convert_to< key_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu = _val.convert_to< mpu_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_cfg_km_profile_bit_loc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bit_loc = _val.convert_to< bit_loc_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_te_csr_cfg_km_profile_bit_sel_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bit_sel = _val.convert_to< bit_sel_cpp_int_t >()  ;
    _val = _val >> 13;
    
}

void cap_te_csr_cfg_km_profile_byte_sel_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__byte_sel = _val.convert_to< byte_sel_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_te_csr_cfg_table_mpu_const_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_te_csr_cfg_table_property_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi = _val.convert_to< axi_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__key_mask_hi = _val.convert_to< key_mask_hi_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__key_mask_lo = _val.convert_to< key_mask_lo_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__fullkey_km_sel0 = _val.convert_to< fullkey_km_sel0_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__fullkey_km_sel1 = _val.convert_to< fullkey_km_sel1_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__lock_en = _val.convert_to< lock_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tbl_entry_sz_raw = _val.convert_to< tbl_entry_sz_raw_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_pc_dyn = _val.convert_to< mpu_pc_dyn_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_pc = _val.convert_to< mpu_pc_cpp_int_t >()  ;
    _val = _val >> 28;
    
    int_var__mpu_pc_raw = _val.convert_to< mpu_pc_raw_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_pc_ofst_err = _val.convert_to< mpu_pc_ofst_err_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__mpu_vec = _val.convert_to< mpu_vec_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__addr_base = _val.convert_to< addr_base_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__addr_vf_id_en = _val.convert_to< addr_vf_id_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr_vf_id_loc = _val.convert_to< addr_vf_id_loc_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addr_shift = _val.convert_to< addr_shift_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__chain_shift = _val.convert_to< chain_shift_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addr_sz = _val.convert_to< addr_sz_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__lg2_entry_size = _val.convert_to< lg2_entry_size_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__max_bypass_cnt = _val.convert_to< max_bypass_cnt_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__lock_en_raw = _val.convert_to< lock_en_raw_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_lb = _val.convert_to< mpu_lb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_pc_loc = _val.convert_to< mpu_pc_loc_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__mpu_pc_ofst_shift = _val.convert_to< mpu_pc_ofst_shift_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__oflow_base_idx = _val.convert_to< oflow_base_idx_cpp_int_t >()  ;
    _val = _val >> 20;
    
    int_var__en_pc_axi_err = _val.convert_to< en_pc_axi_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__en_pc_phv_err = _val.convert_to< en_pc_phv_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__memory_only = _val.convert_to< memory_only_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_cfg_table_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mpu_results = _val.convert_to< mpu_results_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__seq_base = _val.convert_to< seq_base_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__adv_phv_flit = _val.convert_to< adv_phv_flit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done = _val.convert_to< done_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_cfg_table_profile_cam_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__mask = _val.convert_to< mask_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_te_csr_cfg_table_profile_key_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel0 = _val.convert_to< sel0_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel1 = _val.convert_to< sel1_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel2 = _val.convert_to< sel2_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel3 = _val.convert_to< sel3_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel4 = _val.convert_to< sel4_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel5 = _val.convert_to< sel5_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel6 = _val.convert_to< sel6_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sel7 = _val.convert_to< sel7_cpp_int_t >()  ;
    _val = _val >> 9;
    
}

void cap_te_csr_cfg_global_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sw_rst = _val.convert_to< sw_rst_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__step_key = _val.convert_to< step_key_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__step_lkup = _val.convert_to< step_lkup_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__step_mpu = _val.convert_to< step_mpu_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_te_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_global.all( _val);
    _val = _val >> cfg_global.get_width(); 
    cfg_table_profile_key.all( _val);
    _val = _val >> cfg_table_profile_key.get_width(); 
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_cam
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile_cam[ii].all(_val);
        _val = _val >> cfg_table_profile_cam[ii].get_width();
    }
    #endif
    
    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_ctrl_sram_ext
    for(int ii = 0; ii < 192; ii++) {
        cfg_table_profile_ctrl_sram_ext[ii].all(_val);
        _val = _val >> cfg_table_profile_ctrl_sram_ext[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile[ii].all(_val);
        _val = _val >> cfg_table_profile[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_property
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_property[ii].all(_val);
        _val = _val >> cfg_table_property[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_mpu_const
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_mpu_const[ii].all(_val);
        _val = _val >> cfg_table_mpu_const[ii].get_width();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_byte_sel
    for(int ii = 0; ii < 256; ii++) {
        cfg_km_profile_byte_sel[ii].all(_val);
        _val = _val >> cfg_km_profile_byte_sel[ii].get_width();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_sel
    for(int ii = 0; ii < 128; ii++) {
        cfg_km_profile_bit_sel[ii].all(_val);
        _val = _val >> cfg_km_profile_bit_sel[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_loc
    for(int ii = 0; ii < 16; ii++) {
        cfg_km_profile_bit_loc[ii].all(_val);
        _val = _val >> cfg_km_profile_bit_loc[ii].get_width();
    }
    #endif
    
    dhs_single_step.all( _val);
    _val = _val >> dhs_single_step.get_width(); 
    cnt_phv_in_sop.all( _val);
    _val = _val >> cnt_phv_in_sop.get_width(); 
    cnt_phv_in_eop.all( _val);
    _val = _val >> cnt_phv_in_eop.get_width(); 
    cnt_phv_out_sop.all( _val);
    _val = _val >> cnt_phv_out_sop.get_width(); 
    cnt_phv_out_eop.all( _val);
    _val = _val >> cnt_phv_out_eop.get_width(); 
    cnt_axi_rdreq.all( _val);
    _val = _val >> cnt_axi_rdreq.get_width(); 
    cnt_axi_rdrsp.all( _val);
    _val = _val >> cnt_axi_rdrsp.get_width(); 
    cnt_tcam_req.all( _val);
    _val = _val >> cnt_tcam_req.get_width(); 
    cnt_tcam_rsp.all( _val);
    _val = _val >> cnt_tcam_rsp.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_mpu_out
    for(int ii = 0; ii < 4; ii++) {
        cnt_mpu_out[ii].all(_val);
        _val = _val >> cnt_mpu_out[ii].get_width();
    }
    #endif
    
    cfg_debug_bus.all( _val);
    _val = _val >> cfg_debug_bus.get_width(); 
    sta_debug_bus.all( _val);
    _val = _val >> sta_debug_bus.get_width(); 
    cfg_addr_target.all( _val);
    _val = _val >> cfg_addr_target.get_width(); 
    cfg_read_wait_entry.all( _val);
    _val = _val >> cfg_read_wait_entry.get_width(); 
    cfg_page_size.all( _val);
    _val = _val >> cfg_page_size.get_width(); 
    cfg_timeout.all( _val);
    _val = _val >> cfg_timeout.get_width(); 
    sta_wait_entry.all( _val);
    _val = _val >> sta_wait_entry.get_width(); 
    sta_wait_fsm_states.all( _val);
    _val = _val >> sta_wait_fsm_states.get_width(); 
    sta_wait.all( _val);
    _val = _val >> sta_wait.get_width(); 
    sta_axi_err.all( _val);
    _val = _val >> sta_axi_err.get_width(); 
    sta_bad_axi_read.all( _val);
    _val = _val >> sta_bad_axi_read.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    sta_peek.all( _val);
    _val = _val >> sta_peek.get_width(); 
    sta_bist_done.all( _val);
    _val = _val >> sta_bist_done.get_width(); 
    cfg_bist.all( _val);
    _val = _val >> cfg_bist.get_width(); 
    cfg_hbm_addr_align.all( _val);
    _val = _val >> cfg_hbm_addr_align.get_width(); 
    CNT_stall.all( _val);
    _val = _val >> CNT_stall.get_width(); 
    dhs_table_profile_ctrl_sram.all( _val);
    _val = _val >> dhs_table_profile_ctrl_sram.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_err.all( _val);
    _val = _val >> int_err.get_width(); 
    int_info.all( _val);
    _val = _val >> int_info.get_width(); 
}

cpp_int cap_te_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_info.get_width(); ret_val = ret_val  | int_info.all(); 
    ret_val = ret_val << int_err.get_width(); ret_val = ret_val  | int_err.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << dhs_table_profile_ctrl_sram.get_width(); ret_val = ret_val  | dhs_table_profile_ctrl_sram.all(); 
    ret_val = ret_val << CNT_stall.get_width(); ret_val = ret_val  | CNT_stall.all(); 
    ret_val = ret_val << cfg_hbm_addr_align.get_width(); ret_val = ret_val  | cfg_hbm_addr_align.all(); 
    ret_val = ret_val << cfg_bist.get_width(); ret_val = ret_val  | cfg_bist.all(); 
    ret_val = ret_val << sta_bist_done.get_width(); ret_val = ret_val  | sta_bist_done.all(); 
    ret_val = ret_val << sta_peek.get_width(); ret_val = ret_val  | sta_peek.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_bad_axi_read.get_width(); ret_val = ret_val  | sta_bad_axi_read.all(); 
    ret_val = ret_val << sta_axi_err.get_width(); ret_val = ret_val  | sta_axi_err.all(); 
    ret_val = ret_val << sta_wait.get_width(); ret_val = ret_val  | sta_wait.all(); 
    ret_val = ret_val << sta_wait_fsm_states.get_width(); ret_val = ret_val  | sta_wait_fsm_states.all(); 
    ret_val = ret_val << sta_wait_entry.get_width(); ret_val = ret_val  | sta_wait_entry.all(); 
    ret_val = ret_val << cfg_timeout.get_width(); ret_val = ret_val  | cfg_timeout.all(); 
    ret_val = ret_val << cfg_page_size.get_width(); ret_val = ret_val  | cfg_page_size.all(); 
    ret_val = ret_val << cfg_read_wait_entry.get_width(); ret_val = ret_val  | cfg_read_wait_entry.all(); 
    ret_val = ret_val << cfg_addr_target.get_width(); ret_val = ret_val  | cfg_addr_target.all(); 
    ret_val = ret_val << sta_debug_bus.get_width(); ret_val = ret_val  | sta_debug_bus.all(); 
    ret_val = ret_val << cfg_debug_bus.get_width(); ret_val = ret_val  | cfg_debug_bus.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_mpu_out
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << cnt_mpu_out[ii].get_width(); ret_val = ret_val  | cnt_mpu_out[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << cnt_tcam_rsp.get_width(); ret_val = ret_val  | cnt_tcam_rsp.all(); 
    ret_val = ret_val << cnt_tcam_req.get_width(); ret_val = ret_val  | cnt_tcam_req.all(); 
    ret_val = ret_val << cnt_axi_rdrsp.get_width(); ret_val = ret_val  | cnt_axi_rdrsp.all(); 
    ret_val = ret_val << cnt_axi_rdreq.get_width(); ret_val = ret_val  | cnt_axi_rdreq.all(); 
    ret_val = ret_val << cnt_phv_out_eop.get_width(); ret_val = ret_val  | cnt_phv_out_eop.all(); 
    ret_val = ret_val << cnt_phv_out_sop.get_width(); ret_val = ret_val  | cnt_phv_out_sop.all(); 
    ret_val = ret_val << cnt_phv_in_eop.get_width(); ret_val = ret_val  | cnt_phv_in_eop.all(); 
    ret_val = ret_val << cnt_phv_in_sop.get_width(); ret_val = ret_val  | cnt_phv_in_sop.all(); 
    ret_val = ret_val << dhs_single_step.get_width(); ret_val = ret_val  | dhs_single_step.all(); 
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_loc
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_km_profile_bit_loc[ii].get_width(); ret_val = ret_val  | cfg_km_profile_bit_loc[ii].all(); 
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_sel
    for(int ii = 128-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_km_profile_bit_sel[ii].get_width(); ret_val = ret_val  | cfg_km_profile_bit_sel[ii].all(); 
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_byte_sel
    for(int ii = 256-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_km_profile_byte_sel[ii].get_width(); ret_val = ret_val  | cfg_km_profile_byte_sel[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_mpu_const
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_table_mpu_const[ii].get_width(); ret_val = ret_val  | cfg_table_mpu_const[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_property
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_table_property[ii].get_width(); ret_val = ret_val  | cfg_table_property[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_table_profile[ii].get_width(); ret_val = ret_val  | cfg_table_profile[ii].all(); 
    }
    #endif
    
    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_ctrl_sram_ext
    for(int ii = 192-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_table_profile_ctrl_sram_ext[ii].get_width(); ret_val = ret_val  | cfg_table_profile_ctrl_sram_ext[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_cam
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_table_profile_cam[ii].get_width(); ret_val = ret_val  | cfg_table_profile_cam[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << cfg_table_profile_key.get_width(); ret_val = ret_val  | cfg_table_profile_key.all(); 
    ret_val = ret_val << cfg_global.get_width(); ret_val = ret_val  | cfg_global.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_te_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_global_t::all() const {
    cpp_int ret_val;

    // step_mpu
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__step_mpu; 
    
    // step_lkup
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__step_lkup; 
    
    // step_key
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__step_key; 
    
    // sw_rst
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sw_rst; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_key_t::all() const {
    cpp_int ret_val;

    // sel7
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel7; 
    
    // sel6
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel6; 
    
    // sel5
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel5; 
    
    // sel4
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel4; 
    
    // sel3
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel3; 
    
    // sel2
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel2; 
    
    // sel1
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel1; 
    
    // sel0
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sel0; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::all() const {
    cpp_int ret_val;

    // mask
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__mask; 
    
    // value
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__value; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::all() const {
    cpp_int ret_val;

    // done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done; 
    
    // adv_phv_flit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__adv_phv_flit; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_t::all() const {
    cpp_int ret_val;

    // seq_base
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__seq_base; 
    
    // mpu_results
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mpu_results; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_property_t::all() const {
    cpp_int ret_val;

    // memory_only
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__memory_only; 
    
    // en_pc_phv_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en_pc_phv_err; 
    
    // en_pc_axi_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en_pc_axi_err; 
    
    // oflow_base_idx
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__oflow_base_idx; 
    
    // mpu_pc_ofst_shift
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mpu_pc_ofst_shift; 
    
    // mpu_pc_loc
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__mpu_pc_loc; 
    
    // mpu_lb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_lb; 
    
    // lock_en_raw
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock_en_raw; 
    
    // max_bypass_cnt
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__max_bypass_cnt; 
    
    // lg2_entry_size
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__lg2_entry_size; 
    
    // addr_sz
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__addr_sz; 
    
    // chain_shift
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__chain_shift; 
    
    // addr_shift
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__addr_shift; 
    
    // addr_vf_id_loc
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__addr_vf_id_loc; 
    
    // addr_vf_id_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addr_vf_id_en; 
    
    // addr_base
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__addr_base; 
    
    // mpu_vec
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mpu_vec; 
    
    // mpu_pc_ofst_err
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__mpu_pc_ofst_err; 
    
    // mpu_pc_raw
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_pc_raw; 
    
    // mpu_pc
    ret_val = ret_val << 28; ret_val = ret_val  | int_var__mpu_pc; 
    
    // mpu_pc_dyn
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_pc_dyn; 
    
    // tbl_entry_sz_raw
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tbl_entry_sz_raw; 
    
    // lock_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock_en; 
    
    // fullkey_km_sel1
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__fullkey_km_sel1; 
    
    // fullkey_km_sel0
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__fullkey_km_sel0; 
    
    // key_mask_lo
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__key_mask_lo; 
    
    // key_mask_hi
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__key_mask_hi; 
    
    // axi
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_mpu_const_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_byte_sel_t::all() const {
    cpp_int ret_val;

    // byte_sel
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__byte_sel; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_bit_sel_t::all() const {
    cpp_int ret_val;

    // bit_sel
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__bit_sel; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::all() const {
    cpp_int ret_val;

    // bit_loc
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__bit_loc; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    return ret_val;
}

cpp_int cap_te_csr_dhs_single_step_t::all() const {
    cpp_int ret_val;

    // mpu
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu; 
    
    // lkup
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lkup; 
    
    // key
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__key; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_in_sop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_in_eop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_out_sop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_out_eop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_axi_rdreq_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_axi_rdrsp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_tcam_req_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_tcam_rsp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cnt_mpu_out_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_debug_bus_t::all() const {
    cpp_int ret_val;

    // sel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sel; 
    
    // hash_key_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__hash_key_sel; 
    
    // tcam_key_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__tcam_key_sel; 
    
    // axi_rdata_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__axi_rdata_sel; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_debug_bus_t::all() const {
    cpp_int ret_val;

    // vec
    ret_val = ret_val << 256; ret_val = ret_val  | int_var__vec; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_addr_target_t::all() const {
    cpp_int ret_val;

    // sram
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__sram; 
    
    // host
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__host; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_read_wait_entry_t::all() const {
    cpp_int ret_val;

    // idx
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__idx; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_page_size_t::all() const {
    cpp_int ret_val;

    // sz
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sz; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_timeout_t::all() const {
    cpp_int ret_val;

    // te2mpu
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__te2mpu; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_wait_entry_t::all() const {
    cpp_int ret_val;

    // fsm
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__fsm; 
    
    // haz
    ret_val = ret_val << 17; ret_val = ret_val  | int_var__haz; 
    
    // cam
    ret_val = ret_val << 108; ret_val = ret_val  | int_var__cam; 
    
    // key
    ret_val = ret_val << 590; ret_val = ret_val  | int_var__key; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::all() const {
    cpp_int ret_val;

    // entry15
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry15; 
    
    // entry14
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry14; 
    
    // entry13
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry13; 
    
    // entry12
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry12; 
    
    // entry11
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry11; 
    
    // entry10
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry10; 
    
    // entry9
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry9; 
    
    // entry8
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry8; 
    
    // entry7
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry7; 
    
    // entry6
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry6; 
    
    // entry5
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry5; 
    
    // entry4
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry4; 
    
    // entry3
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry3; 
    
    // entry2
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry2; 
    
    // entry1
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry1; 
    
    // entry0
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__entry0; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_wait_t::all() const {
    cpp_int ret_val;

    // entry_dat
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__entry_dat; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_axi_err_t::all() const {
    cpp_int ret_val;

    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_bad_axi_read_t::all() const {
    cpp_int ret_val;

    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    // addr
    ret_val = ret_val << 65; ret_val = ret_val  | int_var__addr; 
    
    return ret_val;
}

cpp_int cap_te_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_peek_t::all() const {
    cpp_int ret_val;

    // signals
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__signals; 
    
    return ret_val;
}

cpp_int cap_te_csr_sta_bist_done_t::all() const {
    cpp_int ret_val;

    // fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fail; 
    
    // pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pass; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_bist_t::all() const {
    cpp_int ret_val;

    // run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__run; 
    
    return ret_val;
}

cpp_int cap_te_csr_cfg_hbm_addr_align_t::all() const {
    cpp_int ret_val;

    // en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en; 
    
    return ret_val;
}

cpp_int cap_te_csr_CNT_stall_t::all() const {
    cpp_int ret_val;

    // reason3
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__reason3; 
    
    // reason2
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__reason2; 
    
    // reason1
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__reason1; 
    
    // reason0
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__reason0; 
    
    return ret_val;
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_t::all() const {
    cpp_int ret_val;

    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 192-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::all() const {
    cpp_int ret_val;

    // hash_store
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__hash_store; 
    
    // hash_chain
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__hash_chain; 
    
    // hash_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__hash_sel; 
    
    // tableid
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__tableid; 
    
    // lkup
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__lkup; 
    
    // km_new_key3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__km_new_key3; 
    
    // km_new_key2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__km_new_key2; 
    
    // km_new_key1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__km_new_key1; 
    
    // km_new_key0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__km_new_key0; 
    
    // km_profile3
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__km_profile3; 
    
    // km_profile2
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__km_profile2; 
    
    // km_profile1
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__km_profile1; 
    
    // km_profile0
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__km_profile0; 
    
    // km_mode3
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__km_mode3; 
    
    // km_mode2
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__km_mode2; 
    
    // km_mode1
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__km_mode1; 
    
    // km_mode0
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__km_mode0; 
    
    return ret_val;
}

cpp_int cap_te_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_te_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_info_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_info_interrupt; 
    
    // int_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_err_interrupt; 
    
    return ret_val;
}

cpp_int cap_te_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_info_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_info_enable; 
    
    // int_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_err_enable; 
    
    return ret_val;
}

cpp_int cap_te_csr_int_err_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_te_csr_int_err_intreg_t::all() const {
    cpp_int ret_val;

    // loaded_but_no_proc_tbl_vld_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__loaded_but_no_proc_tbl_vld_interrupt; 
    
    // loaded_but_no_proc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__loaded_but_no_proc_interrupt; 
    
    // both_pend_went_up_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__both_pend_went_up_interrupt; 
    
    // pend_wo_proc_down_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend_wo_proc_down_interrupt; 
    
    // both_pend_down_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__both_pend_down_interrupt; 
    
    // pend1_wo_pend0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend1_wo_pend0_interrupt; 
    
    // pend_wo_wb_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend_wo_wb_interrupt; 
    
    // proc_tbl_vld_wo_proc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__proc_tbl_vld_wo_proc_interrupt; 
    
    // axi_req_idx_fifo_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_req_idx_fifo_interrupt; 
    
    // mpu_req_idx_fifo_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_req_idx_fifo_interrupt; 
    
    // tcam_rsp_idx_fifo_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_rsp_idx_fifo_interrupt; 
    
    // tcam_req_idx_fifo_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_req_idx_fifo_interrupt; 
    
    // axi_bad_read_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_bad_read_interrupt; 
    
    // axi_rdrsp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_rdrsp_err_interrupt; 
    
    // te2mpu_timeout_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__te2mpu_timeout_interrupt; 
    
    // spurious_tcam_rsp_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__spurious_tcam_rsp_interrupt; 
    
    // spurious_axi_rsp_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__spurious_axi_rsp_interrupt; 
    
    // phv_max_size_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phv_max_size_interrupt; 
    
    // miss_eop_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__miss_eop_interrupt; 
    
    // miss_sop_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__miss_sop_interrupt; 
    
    return ret_val;
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // loaded_but_no_proc_tbl_vld_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__loaded_but_no_proc_tbl_vld_enable; 
    
    // loaded_but_no_proc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__loaded_but_no_proc_enable; 
    
    // both_pend_went_up_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__both_pend_went_up_enable; 
    
    // pend_wo_proc_down_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend_wo_proc_down_enable; 
    
    // both_pend_down_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__both_pend_down_enable; 
    
    // pend1_wo_pend0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend1_wo_pend0_enable; 
    
    // pend_wo_wb_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pend_wo_wb_enable; 
    
    // proc_tbl_vld_wo_proc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__proc_tbl_vld_wo_proc_enable; 
    
    // axi_req_idx_fifo_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_req_idx_fifo_enable; 
    
    // mpu_req_idx_fifo_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_req_idx_fifo_enable; 
    
    // tcam_rsp_idx_fifo_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_rsp_idx_fifo_enable; 
    
    // tcam_req_idx_fifo_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_req_idx_fifo_enable; 
    
    // axi_bad_read_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_bad_read_enable; 
    
    // axi_rdrsp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_rdrsp_err_enable; 
    
    // te2mpu_timeout_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__te2mpu_timeout_enable; 
    
    // spurious_tcam_rsp_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__spurious_tcam_rsp_enable; 
    
    // spurious_axi_rsp_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__spurious_axi_rsp_enable; 
    
    // phv_max_size_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phv_max_size_enable; 
    
    // miss_eop_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__miss_eop_enable; 
    
    // miss_sop_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__miss_sop_enable; 
    
    return ret_val;
}

cpp_int cap_te_csr_int_info_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_te_csr_int_info_int_test_set_t::all() const {
    cpp_int ret_val;

    // profile_cam_miss_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_miss_interrupt; 
    
    // profile_cam_hit15_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit15_interrupt; 
    
    // profile_cam_hit14_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit14_interrupt; 
    
    // profile_cam_hit13_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit13_interrupt; 
    
    // profile_cam_hit12_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit12_interrupt; 
    
    // profile_cam_hit11_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit11_interrupt; 
    
    // profile_cam_hit10_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit10_interrupt; 
    
    // profile_cam_hit9_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit9_interrupt; 
    
    // profile_cam_hit8_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit8_interrupt; 
    
    // profile_cam_hit7_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit7_interrupt; 
    
    // profile_cam_hit6_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit6_interrupt; 
    
    // profile_cam_hit5_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit5_interrupt; 
    
    // profile_cam_hit4_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit4_interrupt; 
    
    // profile_cam_hit3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit3_interrupt; 
    
    // profile_cam_hit2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit2_interrupt; 
    
    // profile_cam_hit1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit1_interrupt; 
    
    // profile_cam_hit0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit0_interrupt; 
    
    return ret_val;
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // profile_cam_miss_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_miss_enable; 
    
    // profile_cam_hit15_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit15_enable; 
    
    // profile_cam_hit14_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit14_enable; 
    
    // profile_cam_hit13_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit13_enable; 
    
    // profile_cam_hit12_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit12_enable; 
    
    // profile_cam_hit11_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit11_enable; 
    
    // profile_cam_hit10_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit10_enable; 
    
    // profile_cam_hit9_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit9_enable; 
    
    // profile_cam_hit8_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit8_enable; 
    
    // profile_cam_hit7_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit7_enable; 
    
    // profile_cam_hit6_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit6_enable; 
    
    // profile_cam_hit5_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit5_enable; 
    
    // profile_cam_hit4_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit4_enable; 
    
    // profile_cam_hit3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit3_enable; 
    
    // profile_cam_hit2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit2_enable; 
    
    // profile_cam_hit1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit1_enable; 
    
    // profile_cam_hit0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__profile_cam_hit0_enable; 
    
    return ret_val;
}

void cap_te_csr_int_info_int_enable_clear_t::clear() {

    int_var__profile_cam_hit0_enable = 0; 
    
    int_var__profile_cam_hit1_enable = 0; 
    
    int_var__profile_cam_hit2_enable = 0; 
    
    int_var__profile_cam_hit3_enable = 0; 
    
    int_var__profile_cam_hit4_enable = 0; 
    
    int_var__profile_cam_hit5_enable = 0; 
    
    int_var__profile_cam_hit6_enable = 0; 
    
    int_var__profile_cam_hit7_enable = 0; 
    
    int_var__profile_cam_hit8_enable = 0; 
    
    int_var__profile_cam_hit9_enable = 0; 
    
    int_var__profile_cam_hit10_enable = 0; 
    
    int_var__profile_cam_hit11_enable = 0; 
    
    int_var__profile_cam_hit12_enable = 0; 
    
    int_var__profile_cam_hit13_enable = 0; 
    
    int_var__profile_cam_hit14_enable = 0; 
    
    int_var__profile_cam_hit15_enable = 0; 
    
    int_var__profile_cam_miss_enable = 0; 
    
}

void cap_te_csr_int_info_int_test_set_t::clear() {

    int_var__profile_cam_hit0_interrupt = 0; 
    
    int_var__profile_cam_hit1_interrupt = 0; 
    
    int_var__profile_cam_hit2_interrupt = 0; 
    
    int_var__profile_cam_hit3_interrupt = 0; 
    
    int_var__profile_cam_hit4_interrupt = 0; 
    
    int_var__profile_cam_hit5_interrupt = 0; 
    
    int_var__profile_cam_hit6_interrupt = 0; 
    
    int_var__profile_cam_hit7_interrupt = 0; 
    
    int_var__profile_cam_hit8_interrupt = 0; 
    
    int_var__profile_cam_hit9_interrupt = 0; 
    
    int_var__profile_cam_hit10_interrupt = 0; 
    
    int_var__profile_cam_hit11_interrupt = 0; 
    
    int_var__profile_cam_hit12_interrupt = 0; 
    
    int_var__profile_cam_hit13_interrupt = 0; 
    
    int_var__profile_cam_hit14_interrupt = 0; 
    
    int_var__profile_cam_hit15_interrupt = 0; 
    
    int_var__profile_cam_miss_interrupt = 0; 
    
}

void cap_te_csr_int_info_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_te_csr_int_err_int_enable_clear_t::clear() {

    int_var__miss_sop_enable = 0; 
    
    int_var__miss_eop_enable = 0; 
    
    int_var__phv_max_size_enable = 0; 
    
    int_var__spurious_axi_rsp_enable = 0; 
    
    int_var__spurious_tcam_rsp_enable = 0; 
    
    int_var__te2mpu_timeout_enable = 0; 
    
    int_var__axi_rdrsp_err_enable = 0; 
    
    int_var__axi_bad_read_enable = 0; 
    
    int_var__tcam_req_idx_fifo_enable = 0; 
    
    int_var__tcam_rsp_idx_fifo_enable = 0; 
    
    int_var__mpu_req_idx_fifo_enable = 0; 
    
    int_var__axi_req_idx_fifo_enable = 0; 
    
    int_var__proc_tbl_vld_wo_proc_enable = 0; 
    
    int_var__pend_wo_wb_enable = 0; 
    
    int_var__pend1_wo_pend0_enable = 0; 
    
    int_var__both_pend_down_enable = 0; 
    
    int_var__pend_wo_proc_down_enable = 0; 
    
    int_var__both_pend_went_up_enable = 0; 
    
    int_var__loaded_but_no_proc_enable = 0; 
    
    int_var__loaded_but_no_proc_tbl_vld_enable = 0; 
    
}

void cap_te_csr_int_err_intreg_t::clear() {

    int_var__miss_sop_interrupt = 0; 
    
    int_var__miss_eop_interrupt = 0; 
    
    int_var__phv_max_size_interrupt = 0; 
    
    int_var__spurious_axi_rsp_interrupt = 0; 
    
    int_var__spurious_tcam_rsp_interrupt = 0; 
    
    int_var__te2mpu_timeout_interrupt = 0; 
    
    int_var__axi_rdrsp_err_interrupt = 0; 
    
    int_var__axi_bad_read_interrupt = 0; 
    
    int_var__tcam_req_idx_fifo_interrupt = 0; 
    
    int_var__tcam_rsp_idx_fifo_interrupt = 0; 
    
    int_var__mpu_req_idx_fifo_interrupt = 0; 
    
    int_var__axi_req_idx_fifo_interrupt = 0; 
    
    int_var__proc_tbl_vld_wo_proc_interrupt = 0; 
    
    int_var__pend_wo_wb_interrupt = 0; 
    
    int_var__pend1_wo_pend0_interrupt = 0; 
    
    int_var__both_pend_down_interrupt = 0; 
    
    int_var__pend_wo_proc_down_interrupt = 0; 
    
    int_var__both_pend_went_up_interrupt = 0; 
    
    int_var__loaded_but_no_proc_interrupt = 0; 
    
    int_var__loaded_but_no_proc_tbl_vld_interrupt = 0; 
    
}

void cap_te_csr_int_err_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_te_csr_intreg_status_t::clear() {

    int_var__int_err_interrupt = 0; 
    
    int_var__int_info_interrupt = 0; 
    
}

void cap_te_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_err_enable = 0; 
    
    int_var__int_info_enable = 0; 
    
}

void cap_te_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::clear() {

    int_var__km_mode0 = 0; 
    
    int_var__km_mode1 = 0; 
    
    int_var__km_mode2 = 0; 
    
    int_var__km_mode3 = 0; 
    
    int_var__km_profile0 = 0; 
    
    int_var__km_profile1 = 0; 
    
    int_var__km_profile2 = 0; 
    
    int_var__km_profile3 = 0; 
    
    int_var__km_new_key0 = 0; 
    
    int_var__km_new_key1 = 0; 
    
    int_var__km_new_key2 = 0; 
    
    int_var__km_new_key3 = 0; 
    
    int_var__lkup = 0; 
    
    int_var__tableid = 0; 
    
    int_var__hash_sel = 0; 
    
    int_var__hash_chain = 0; 
    
    int_var__hash_store = 0; 
    
}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::clear() {

    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 192; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_te_csr_CNT_stall_t::clear() {

    int_var__reason0 = 0; 
    
    int_var__reason1 = 0; 
    
    int_var__reason2 = 0; 
    
    int_var__reason3 = 0; 
    
}

void cap_te_csr_cfg_hbm_addr_align_t::clear() {

    int_var__en = 0; 
    
}

void cap_te_csr_cfg_bist_t::clear() {

    int_var__run = 0; 
    
}

void cap_te_csr_sta_bist_done_t::clear() {

    int_var__pass = 0; 
    
    int_var__fail = 0; 
    
}

void cap_te_csr_sta_peek_t::clear() {

    int_var__signals = 0; 
    
}

void cap_te_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_te_csr_sta_bad_axi_read_t::clear() {

    int_var__addr = 0; 
    
    int_var__id = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
}

void cap_te_csr_sta_axi_err_t::clear() {

    int_var__resp = 0; 
    
    int_var__id = 0; 
    
}

void cap_te_csr_sta_wait_t::clear() {

    int_var__entry_dat = 0; 
    
}

void cap_te_csr_sta_wait_fsm_states_t::clear() {

    int_var__entry0 = 0; 
    
    int_var__entry1 = 0; 
    
    int_var__entry2 = 0; 
    
    int_var__entry3 = 0; 
    
    int_var__entry4 = 0; 
    
    int_var__entry5 = 0; 
    
    int_var__entry6 = 0; 
    
    int_var__entry7 = 0; 
    
    int_var__entry8 = 0; 
    
    int_var__entry9 = 0; 
    
    int_var__entry10 = 0; 
    
    int_var__entry11 = 0; 
    
    int_var__entry12 = 0; 
    
    int_var__entry13 = 0; 
    
    int_var__entry14 = 0; 
    
    int_var__entry15 = 0; 
    
}

void cap_te_csr_sta_wait_entry_t::clear() {

    int_var__key = 0; 
    
    int_var__cam = 0; 
    
    int_var__haz = 0; 
    
    int_var__fsm = 0; 
    
}

void cap_te_csr_cfg_timeout_t::clear() {

    int_var__te2mpu = 0; 
    
}

void cap_te_csr_cfg_page_size_t::clear() {

    int_var__sz = 0; 
    
}

void cap_te_csr_cfg_read_wait_entry_t::clear() {

    int_var__idx = 0; 
    
}

void cap_te_csr_cfg_addr_target_t::clear() {

    int_var__host = 0; 
    
    int_var__sram = 0; 
    
}

void cap_te_csr_sta_debug_bus_t::clear() {

    int_var__vec = 0; 
    
}

void cap_te_csr_cfg_debug_bus_t::clear() {

    int_var__axi_rdata_sel = 0; 
    
    int_var__tcam_key_sel = 0; 
    
    int_var__hash_key_sel = 0; 
    
    int_var__sel = 0; 
    
}

void cap_te_csr_cnt_mpu_out_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_tcam_rsp_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_tcam_req_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_axi_rdrsp_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_axi_rdreq_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_phv_out_eop_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_phv_out_sop_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_phv_in_eop_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_cnt_phv_in_sop_t::clear() {

    int_var__val = 0; 
    
}

void cap_te_csr_dhs_single_step_t::clear() {

    int_var__key = 0; 
    
    int_var__lkup = 0; 
    
    int_var__mpu = 0; 
    
}

void cap_te_csr_cfg_km_profile_bit_loc_t::clear() {

    int_var__valid = 0; 
    
    int_var__bit_loc = 0; 
    
}

void cap_te_csr_cfg_km_profile_bit_sel_t::clear() {

    int_var__bit_sel = 0; 
    
}

void cap_te_csr_cfg_km_profile_byte_sel_t::clear() {

    int_var__byte_sel = 0; 
    
}

void cap_te_csr_cfg_table_mpu_const_t::clear() {

    int_var__value = 0; 
    
}

void cap_te_csr_cfg_table_property_t::clear() {

    int_var__axi = 0; 
    
    int_var__key_mask_hi = 0; 
    
    int_var__key_mask_lo = 0; 
    
    int_var__fullkey_km_sel0 = 0; 
    
    int_var__fullkey_km_sel1 = 0; 
    
    int_var__lock_en = 0; 
    
    int_var__tbl_entry_sz_raw = 0; 
    
    int_var__mpu_pc_dyn = 0; 
    
    int_var__mpu_pc = 0; 
    
    int_var__mpu_pc_raw = 0; 
    
    int_var__mpu_pc_ofst_err = 0; 
    
    int_var__mpu_vec = 0; 
    
    int_var__addr_base = 0; 
    
    int_var__addr_vf_id_en = 0; 
    
    int_var__addr_vf_id_loc = 0; 
    
    int_var__addr_shift = 0; 
    
    int_var__chain_shift = 0; 
    
    int_var__addr_sz = 0; 
    
    int_var__lg2_entry_size = 0; 
    
    int_var__max_bypass_cnt = 0; 
    
    int_var__lock_en_raw = 0; 
    
    int_var__mpu_lb = 0; 
    
    int_var__mpu_pc_loc = 0; 
    
    int_var__mpu_pc_ofst_shift = 0; 
    
    int_var__oflow_base_idx = 0; 
    
    int_var__en_pc_axi_err = 0; 
    
    int_var__en_pc_phv_err = 0; 
    
    int_var__memory_only = 0; 
    
}

void cap_te_csr_cfg_table_profile_t::clear() {

    int_var__mpu_results = 0; 
    
    int_var__seq_base = 0; 
    
}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::clear() {

    int_var__adv_phv_flit = 0; 
    
    int_var__done = 0; 
    
}

void cap_te_csr_cfg_table_profile_cam_t::clear() {

    int_var__valid = 0; 
    
    int_var__value = 0; 
    
    int_var__mask = 0; 
    
}

void cap_te_csr_cfg_table_profile_key_t::clear() {

    int_var__sel0 = 0; 
    
    int_var__sel1 = 0; 
    
    int_var__sel2 = 0; 
    
    int_var__sel3 = 0; 
    
    int_var__sel4 = 0; 
    
    int_var__sel5 = 0; 
    
    int_var__sel6 = 0; 
    
    int_var__sel7 = 0; 
    
}

void cap_te_csr_cfg_global_t::clear() {

    int_var__sw_rst = 0; 
    
    int_var__step_key = 0; 
    
    int_var__step_lkup = 0; 
    
    int_var__step_mpu = 0; 
    
}

void cap_te_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_te_csr_t::clear() {

    base.clear();
    cfg_global.clear();
    cfg_table_profile_key.clear();
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_cam
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile_cam[ii].clear();
    }
    #endif
    
    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_table_profile_ctrl_sram_ext
    for(int ii = 0; ii < 192; ii++) {
        cfg_table_profile_ctrl_sram_ext[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_table_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_table_property
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_property[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_table_mpu_const
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_mpu_const[ii].clear();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_byte_sel
    for(int ii = 0; ii < 256; ii++) {
        cfg_km_profile_byte_sel[ii].clear();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_sel
    for(int ii = 0; ii < 128; ii++) {
        cfg_km_profile_bit_sel[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_km_profile_bit_loc
    for(int ii = 0; ii < 16; ii++) {
        cfg_km_profile_bit_loc[ii].clear();
    }
    #endif
    
    dhs_single_step.clear();
    cnt_phv_in_sop.clear();
    cnt_phv_in_eop.clear();
    cnt_phv_out_sop.clear();
    cnt_phv_out_eop.clear();
    cnt_axi_rdreq.clear();
    cnt_axi_rdrsp.clear();
    cnt_tcam_req.clear();
    cnt_tcam_rsp.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cnt_mpu_out
    for(int ii = 0; ii < 4; ii++) {
        cnt_mpu_out[ii].clear();
    }
    #endif
    
    cfg_debug_bus.clear();
    sta_debug_bus.clear();
    cfg_addr_target.clear();
    cfg_read_wait_entry.clear();
    cfg_page_size.clear();
    cfg_timeout.clear();
    sta_wait_entry.clear();
    sta_wait_fsm_states.clear();
    sta_wait.clear();
    sta_axi_err.clear();
    sta_bad_axi_read.clear();
    csr_intr.clear();
    sta_peek.clear();
    sta_bist_done.clear();
    cfg_bist.clear();
    cfg_hbm_addr_align.clear();
    CNT_stall.clear();
    dhs_table_profile_ctrl_sram.clear();
    int_groups.clear();
    int_err.clear();
    int_info.clear();
}

void cap_te_csr_int_info_int_enable_clear_t::init() {

}

void cap_te_csr_int_info_int_test_set_t::init() {

}

void cap_te_csr_int_info_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_te_csr_int_err_int_enable_clear_t::init() {

}

void cap_te_csr_int_err_intreg_t::init() {

}

void cap_te_csr_int_err_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_te_csr_intreg_status_t::init() {

}

void cap_te_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_te_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::init() {

}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::init() {

    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 192; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_te_csr_CNT_stall_t::init() {

}

void cap_te_csr_cfg_hbm_addr_align_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_te_csr_cfg_bist_t::init() {

}

void cap_te_csr_sta_bist_done_t::init() {

}

void cap_te_csr_sta_peek_t::init() {

}

void cap_te_csr_csr_intr_t::init() {

}

void cap_te_csr_sta_bad_axi_read_t::init() {

}

void cap_te_csr_sta_axi_err_t::init() {

}

void cap_te_csr_sta_wait_t::init() {

}

void cap_te_csr_sta_wait_fsm_states_t::init() {

}

void cap_te_csr_sta_wait_entry_t::init() {

}

void cap_te_csr_cfg_timeout_t::init() {

    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_te_csr_cfg_page_size_t::init() {

    set_reset_val(cpp_int("0x8"));
    all(get_reset_val());
}

void cap_te_csr_cfg_read_wait_entry_t::init() {

}

void cap_te_csr_cfg_addr_target_t::init() {

    set_reset_val(cpp_int("0x103f"));
    all(get_reset_val());
}

void cap_te_csr_sta_debug_bus_t::init() {

}

void cap_te_csr_cfg_debug_bus_t::init() {

}

void cap_te_csr_cnt_mpu_out_t::init() {

}

void cap_te_csr_cnt_tcam_rsp_t::init() {

}

void cap_te_csr_cnt_tcam_req_t::init() {

}

void cap_te_csr_cnt_axi_rdrsp_t::init() {

}

void cap_te_csr_cnt_axi_rdreq_t::init() {

}

void cap_te_csr_cnt_phv_out_eop_t::init() {

}

void cap_te_csr_cnt_phv_out_sop_t::init() {

}

void cap_te_csr_cnt_phv_in_eop_t::init() {

}

void cap_te_csr_cnt_phv_in_sop_t::init() {

}

void cap_te_csr_dhs_single_step_t::init() {

}

void cap_te_csr_cfg_km_profile_bit_loc_t::init() {

}

void cap_te_csr_cfg_km_profile_bit_sel_t::init() {

}

void cap_te_csr_cfg_km_profile_byte_sel_t::init() {

}

void cap_te_csr_cfg_table_mpu_const_t::init() {

}

void cap_te_csr_cfg_table_property_t::init() {

    set_reset_val(cpp_int("0x1800000001ff90000000000000000000001e0000000000000000"));
    all(get_reset_val());
}

void cap_te_csr_cfg_table_profile_t::init() {

}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::init() {

    set_reset_val(cpp_int("0x3"));
    all(get_reset_val());
}

void cap_te_csr_cfg_table_profile_cam_t::init() {

}

void cap_te_csr_cfg_table_profile_key_t::init() {

}

void cap_te_csr_cfg_global_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_te_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_te_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_global.set_attributes(this,"cfg_global", 0x4 );
    cfg_table_profile_key.set_attributes(this,"cfg_table_profile_key", 0x10 );
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile_cam.set_attributes(this, "cfg_table_profile_cam", 0x40);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_profile_cam[ii].set_field_init_done(true, true);
        cfg_table_profile_cam[ii].set_attributes(this,"cfg_table_profile_cam["+to_string(ii)+"]",  0x40 + (cfg_table_profile_cam[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile_ctrl_sram_ext.set_attributes(this, "cfg_table_profile_ctrl_sram_ext", 0x1000);
    #else
    for(int ii = 0; ii < 192; ii++) {
        if(ii != 0) cfg_table_profile_ctrl_sram_ext[ii].set_field_init_done(true, true);
        cfg_table_profile_ctrl_sram_ext[ii].set_attributes(this,"cfg_table_profile_ctrl_sram_ext["+to_string(ii)+"]",  0x1000 + (cfg_table_profile_ctrl_sram_ext[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_profile.set_attributes(this, "cfg_table_profile", 0x1400);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_profile[ii].set_field_init_done(true, true);
        cfg_table_profile[ii].set_attributes(this,"cfg_table_profile["+to_string(ii)+"]",  0x1400 + (cfg_table_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_property.set_attributes(this, "cfg_table_property", 0x1600);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_property[ii].set_field_init_done(true, true);
        cfg_table_property[ii].set_attributes(this,"cfg_table_property["+to_string(ii)+"]",  0x1600 + (cfg_table_property[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_table_mpu_const.set_attributes(this, "cfg_table_mpu_const", 0x1800);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_mpu_const[ii].set_field_init_done(true, true);
        cfg_table_mpu_const[ii].set_attributes(this,"cfg_table_mpu_const["+to_string(ii)+"]",  0x1800 + (cfg_table_mpu_const[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_byte_sel.set_attributes(this, "cfg_km_profile_byte_sel", 0x1c00);
    #else
    for(int ii = 0; ii < 256; ii++) {
        if(ii != 0) cfg_km_profile_byte_sel[ii].set_field_init_done(true, true);
        cfg_km_profile_byte_sel[ii].set_attributes(this,"cfg_km_profile_byte_sel["+to_string(ii)+"]",  0x1c00 + (cfg_km_profile_byte_sel[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_bit_sel.set_attributes(this, "cfg_km_profile_bit_sel", 0x2000);
    #else
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_km_profile_bit_sel[ii].set_field_init_done(true, true);
        cfg_km_profile_bit_sel[ii].set_attributes(this,"cfg_km_profile_bit_sel["+to_string(ii)+"]",  0x2000 + (cfg_km_profile_bit_sel[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_km_profile_bit_loc.set_attributes(this, "cfg_km_profile_bit_loc", 0x2200);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_km_profile_bit_loc[ii].set_field_init_done(true, true);
        cfg_km_profile_bit_loc[ii].set_attributes(this,"cfg_km_profile_bit_loc["+to_string(ii)+"]",  0x2200 + (cfg_km_profile_bit_loc[ii].get_byte_size()*ii));
    }
    #endif
    
    dhs_single_step.set_attributes(this,"dhs_single_step", 0x2240 );
    cnt_phv_in_sop.set_attributes(this,"cnt_phv_in_sop", 0x2244 );
    cnt_phv_in_eop.set_attributes(this,"cnt_phv_in_eop", 0x2248 );
    cnt_phv_out_sop.set_attributes(this,"cnt_phv_out_sop", 0x224c );
    cnt_phv_out_eop.set_attributes(this,"cnt_phv_out_eop", 0x2250 );
    cnt_axi_rdreq.set_attributes(this,"cnt_axi_rdreq", 0x2254 );
    cnt_axi_rdrsp.set_attributes(this,"cnt_axi_rdrsp", 0x2258 );
    cnt_tcam_req.set_attributes(this,"cnt_tcam_req", 0x225c );
    cnt_tcam_rsp.set_attributes(this,"cnt_tcam_rsp", 0x2260 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_mpu_out.set_attributes(this, "cnt_mpu_out", 0x2270);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cnt_mpu_out[ii].set_field_init_done(true, true);
        cnt_mpu_out[ii].set_attributes(this,"cnt_mpu_out["+to_string(ii)+"]",  0x2270 + (cnt_mpu_out[ii].get_byte_size()*ii));
    }
    #endif
    
    cfg_debug_bus.set_attributes(this,"cfg_debug_bus", 0x2280 );
    sta_debug_bus.set_attributes(this,"sta_debug_bus", 0x22a0 );
    cfg_addr_target.set_attributes(this,"cfg_addr_target", 0x22c0 );
    cfg_read_wait_entry.set_attributes(this,"cfg_read_wait_entry", 0x22c4 );
    cfg_page_size.set_attributes(this,"cfg_page_size", 0x22c8 );
    cfg_timeout.set_attributes(this,"cfg_timeout", 0x22cc );
    sta_wait_entry.set_attributes(this,"sta_wait_entry", 0x2300 );
    sta_wait_fsm_states.set_attributes(this,"sta_wait_fsm_states", 0x2380 );
    sta_wait.set_attributes(this,"sta_wait", 0x23c0 );
    sta_axi_err.set_attributes(this,"sta_axi_err", 0x2400 );
    sta_bad_axi_read.set_attributes(this,"sta_bad_axi_read", 0x2410 );
    csr_intr.set_attributes(this,"csr_intr", 0x2420 );
    sta_peek.set_attributes(this,"sta_peek", 0x2460 );
    sta_bist_done.set_attributes(this,"sta_bist_done", 0x2468 );
    cfg_bist.set_attributes(this,"cfg_bist", 0x246c );
    cfg_hbm_addr_align.set_attributes(this,"cfg_hbm_addr_align", 0x2470 );
    CNT_stall.set_attributes(this,"CNT_stall", 0x2478 );
    dhs_table_profile_ctrl_sram.set_attributes(this,"dhs_table_profile_ctrl_sram", 0x800 );
    int_groups.set_attributes(this,"int_groups", 0x2430 );
    int_err.set_attributes(this,"int_err", 0x2440 );
    int_info.set_attributes(this,"int_info", 0x2450 );
}

void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit0_enable(const cpp_int & _val) { 
    // profile_cam_hit0_enable
    int_var__profile_cam_hit0_enable = _val.convert_to< profile_cam_hit0_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit0_enable() const {
    return int_var__profile_cam_hit0_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit1_enable(const cpp_int & _val) { 
    // profile_cam_hit1_enable
    int_var__profile_cam_hit1_enable = _val.convert_to< profile_cam_hit1_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit1_enable() const {
    return int_var__profile_cam_hit1_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit2_enable(const cpp_int & _val) { 
    // profile_cam_hit2_enable
    int_var__profile_cam_hit2_enable = _val.convert_to< profile_cam_hit2_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit2_enable() const {
    return int_var__profile_cam_hit2_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit3_enable(const cpp_int & _val) { 
    // profile_cam_hit3_enable
    int_var__profile_cam_hit3_enable = _val.convert_to< profile_cam_hit3_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit3_enable() const {
    return int_var__profile_cam_hit3_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit4_enable(const cpp_int & _val) { 
    // profile_cam_hit4_enable
    int_var__profile_cam_hit4_enable = _val.convert_to< profile_cam_hit4_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit4_enable() const {
    return int_var__profile_cam_hit4_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit5_enable(const cpp_int & _val) { 
    // profile_cam_hit5_enable
    int_var__profile_cam_hit5_enable = _val.convert_to< profile_cam_hit5_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit5_enable() const {
    return int_var__profile_cam_hit5_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit6_enable(const cpp_int & _val) { 
    // profile_cam_hit6_enable
    int_var__profile_cam_hit6_enable = _val.convert_to< profile_cam_hit6_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit6_enable() const {
    return int_var__profile_cam_hit6_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit7_enable(const cpp_int & _val) { 
    // profile_cam_hit7_enable
    int_var__profile_cam_hit7_enable = _val.convert_to< profile_cam_hit7_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit7_enable() const {
    return int_var__profile_cam_hit7_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit8_enable(const cpp_int & _val) { 
    // profile_cam_hit8_enable
    int_var__profile_cam_hit8_enable = _val.convert_to< profile_cam_hit8_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit8_enable() const {
    return int_var__profile_cam_hit8_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit9_enable(const cpp_int & _val) { 
    // profile_cam_hit9_enable
    int_var__profile_cam_hit9_enable = _val.convert_to< profile_cam_hit9_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit9_enable() const {
    return int_var__profile_cam_hit9_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit10_enable(const cpp_int & _val) { 
    // profile_cam_hit10_enable
    int_var__profile_cam_hit10_enable = _val.convert_to< profile_cam_hit10_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit10_enable() const {
    return int_var__profile_cam_hit10_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit11_enable(const cpp_int & _val) { 
    // profile_cam_hit11_enable
    int_var__profile_cam_hit11_enable = _val.convert_to< profile_cam_hit11_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit11_enable() const {
    return int_var__profile_cam_hit11_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit12_enable(const cpp_int & _val) { 
    // profile_cam_hit12_enable
    int_var__profile_cam_hit12_enable = _val.convert_to< profile_cam_hit12_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit12_enable() const {
    return int_var__profile_cam_hit12_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit13_enable(const cpp_int & _val) { 
    // profile_cam_hit13_enable
    int_var__profile_cam_hit13_enable = _val.convert_to< profile_cam_hit13_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit13_enable() const {
    return int_var__profile_cam_hit13_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit14_enable(const cpp_int & _val) { 
    // profile_cam_hit14_enable
    int_var__profile_cam_hit14_enable = _val.convert_to< profile_cam_hit14_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit14_enable() const {
    return int_var__profile_cam_hit14_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit15_enable(const cpp_int & _val) { 
    // profile_cam_hit15_enable
    int_var__profile_cam_hit15_enable = _val.convert_to< profile_cam_hit15_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_hit15_enable() const {
    return int_var__profile_cam_hit15_enable;
}
    
void cap_te_csr_int_info_int_enable_clear_t::profile_cam_miss_enable(const cpp_int & _val) { 
    // profile_cam_miss_enable
    int_var__profile_cam_miss_enable = _val.convert_to< profile_cam_miss_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_enable_clear_t::profile_cam_miss_enable() const {
    return int_var__profile_cam_miss_enable;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit0_interrupt(const cpp_int & _val) { 
    // profile_cam_hit0_interrupt
    int_var__profile_cam_hit0_interrupt = _val.convert_to< profile_cam_hit0_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit0_interrupt() const {
    return int_var__profile_cam_hit0_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit1_interrupt(const cpp_int & _val) { 
    // profile_cam_hit1_interrupt
    int_var__profile_cam_hit1_interrupt = _val.convert_to< profile_cam_hit1_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit1_interrupt() const {
    return int_var__profile_cam_hit1_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit2_interrupt(const cpp_int & _val) { 
    // profile_cam_hit2_interrupt
    int_var__profile_cam_hit2_interrupt = _val.convert_to< profile_cam_hit2_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit2_interrupt() const {
    return int_var__profile_cam_hit2_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit3_interrupt(const cpp_int & _val) { 
    // profile_cam_hit3_interrupt
    int_var__profile_cam_hit3_interrupt = _val.convert_to< profile_cam_hit3_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit3_interrupt() const {
    return int_var__profile_cam_hit3_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit4_interrupt(const cpp_int & _val) { 
    // profile_cam_hit4_interrupt
    int_var__profile_cam_hit4_interrupt = _val.convert_to< profile_cam_hit4_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit4_interrupt() const {
    return int_var__profile_cam_hit4_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit5_interrupt(const cpp_int & _val) { 
    // profile_cam_hit5_interrupt
    int_var__profile_cam_hit5_interrupt = _val.convert_to< profile_cam_hit5_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit5_interrupt() const {
    return int_var__profile_cam_hit5_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit6_interrupt(const cpp_int & _val) { 
    // profile_cam_hit6_interrupt
    int_var__profile_cam_hit6_interrupt = _val.convert_to< profile_cam_hit6_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit6_interrupt() const {
    return int_var__profile_cam_hit6_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit7_interrupt(const cpp_int & _val) { 
    // profile_cam_hit7_interrupt
    int_var__profile_cam_hit7_interrupt = _val.convert_to< profile_cam_hit7_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit7_interrupt() const {
    return int_var__profile_cam_hit7_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit8_interrupt(const cpp_int & _val) { 
    // profile_cam_hit8_interrupt
    int_var__profile_cam_hit8_interrupt = _val.convert_to< profile_cam_hit8_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit8_interrupt() const {
    return int_var__profile_cam_hit8_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit9_interrupt(const cpp_int & _val) { 
    // profile_cam_hit9_interrupt
    int_var__profile_cam_hit9_interrupt = _val.convert_to< profile_cam_hit9_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit9_interrupt() const {
    return int_var__profile_cam_hit9_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit10_interrupt(const cpp_int & _val) { 
    // profile_cam_hit10_interrupt
    int_var__profile_cam_hit10_interrupt = _val.convert_to< profile_cam_hit10_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit10_interrupt() const {
    return int_var__profile_cam_hit10_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit11_interrupt(const cpp_int & _val) { 
    // profile_cam_hit11_interrupt
    int_var__profile_cam_hit11_interrupt = _val.convert_to< profile_cam_hit11_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit11_interrupt() const {
    return int_var__profile_cam_hit11_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit12_interrupt(const cpp_int & _val) { 
    // profile_cam_hit12_interrupt
    int_var__profile_cam_hit12_interrupt = _val.convert_to< profile_cam_hit12_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit12_interrupt() const {
    return int_var__profile_cam_hit12_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit13_interrupt(const cpp_int & _val) { 
    // profile_cam_hit13_interrupt
    int_var__profile_cam_hit13_interrupt = _val.convert_to< profile_cam_hit13_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit13_interrupt() const {
    return int_var__profile_cam_hit13_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit14_interrupt(const cpp_int & _val) { 
    // profile_cam_hit14_interrupt
    int_var__profile_cam_hit14_interrupt = _val.convert_to< profile_cam_hit14_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit14_interrupt() const {
    return int_var__profile_cam_hit14_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_hit15_interrupt(const cpp_int & _val) { 
    // profile_cam_hit15_interrupt
    int_var__profile_cam_hit15_interrupt = _val.convert_to< profile_cam_hit15_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_hit15_interrupt() const {
    return int_var__profile_cam_hit15_interrupt;
}
    
void cap_te_csr_int_info_int_test_set_t::profile_cam_miss_interrupt(const cpp_int & _val) { 
    // profile_cam_miss_interrupt
    int_var__profile_cam_miss_interrupt = _val.convert_to< profile_cam_miss_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_info_int_test_set_t::profile_cam_miss_interrupt() const {
    return int_var__profile_cam_miss_interrupt;
}
    
void cap_te_csr_int_err_int_enable_clear_t::miss_sop_enable(const cpp_int & _val) { 
    // miss_sop_enable
    int_var__miss_sop_enable = _val.convert_to< miss_sop_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::miss_sop_enable() const {
    return int_var__miss_sop_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::miss_eop_enable(const cpp_int & _val) { 
    // miss_eop_enable
    int_var__miss_eop_enable = _val.convert_to< miss_eop_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::miss_eop_enable() const {
    return int_var__miss_eop_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::phv_max_size_enable(const cpp_int & _val) { 
    // phv_max_size_enable
    int_var__phv_max_size_enable = _val.convert_to< phv_max_size_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::phv_max_size_enable() const {
    return int_var__phv_max_size_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::spurious_axi_rsp_enable(const cpp_int & _val) { 
    // spurious_axi_rsp_enable
    int_var__spurious_axi_rsp_enable = _val.convert_to< spurious_axi_rsp_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::spurious_axi_rsp_enable() const {
    return int_var__spurious_axi_rsp_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::spurious_tcam_rsp_enable(const cpp_int & _val) { 
    // spurious_tcam_rsp_enable
    int_var__spurious_tcam_rsp_enable = _val.convert_to< spurious_tcam_rsp_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::spurious_tcam_rsp_enable() const {
    return int_var__spurious_tcam_rsp_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::te2mpu_timeout_enable(const cpp_int & _val) { 
    // te2mpu_timeout_enable
    int_var__te2mpu_timeout_enable = _val.convert_to< te2mpu_timeout_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::te2mpu_timeout_enable() const {
    return int_var__te2mpu_timeout_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::axi_rdrsp_err_enable(const cpp_int & _val) { 
    // axi_rdrsp_err_enable
    int_var__axi_rdrsp_err_enable = _val.convert_to< axi_rdrsp_err_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::axi_rdrsp_err_enable() const {
    return int_var__axi_rdrsp_err_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::axi_bad_read_enable(const cpp_int & _val) { 
    // axi_bad_read_enable
    int_var__axi_bad_read_enable = _val.convert_to< axi_bad_read_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::axi_bad_read_enable() const {
    return int_var__axi_bad_read_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::tcam_req_idx_fifo_enable(const cpp_int & _val) { 
    // tcam_req_idx_fifo_enable
    int_var__tcam_req_idx_fifo_enable = _val.convert_to< tcam_req_idx_fifo_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::tcam_req_idx_fifo_enable() const {
    return int_var__tcam_req_idx_fifo_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::tcam_rsp_idx_fifo_enable(const cpp_int & _val) { 
    // tcam_rsp_idx_fifo_enable
    int_var__tcam_rsp_idx_fifo_enable = _val.convert_to< tcam_rsp_idx_fifo_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::tcam_rsp_idx_fifo_enable() const {
    return int_var__tcam_rsp_idx_fifo_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::mpu_req_idx_fifo_enable(const cpp_int & _val) { 
    // mpu_req_idx_fifo_enable
    int_var__mpu_req_idx_fifo_enable = _val.convert_to< mpu_req_idx_fifo_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::mpu_req_idx_fifo_enable() const {
    return int_var__mpu_req_idx_fifo_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::axi_req_idx_fifo_enable(const cpp_int & _val) { 
    // axi_req_idx_fifo_enable
    int_var__axi_req_idx_fifo_enable = _val.convert_to< axi_req_idx_fifo_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::axi_req_idx_fifo_enable() const {
    return int_var__axi_req_idx_fifo_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::proc_tbl_vld_wo_proc_enable(const cpp_int & _val) { 
    // proc_tbl_vld_wo_proc_enable
    int_var__proc_tbl_vld_wo_proc_enable = _val.convert_to< proc_tbl_vld_wo_proc_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::proc_tbl_vld_wo_proc_enable() const {
    return int_var__proc_tbl_vld_wo_proc_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::pend_wo_wb_enable(const cpp_int & _val) { 
    // pend_wo_wb_enable
    int_var__pend_wo_wb_enable = _val.convert_to< pend_wo_wb_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::pend_wo_wb_enable() const {
    return int_var__pend_wo_wb_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::pend1_wo_pend0_enable(const cpp_int & _val) { 
    // pend1_wo_pend0_enable
    int_var__pend1_wo_pend0_enable = _val.convert_to< pend1_wo_pend0_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::pend1_wo_pend0_enable() const {
    return int_var__pend1_wo_pend0_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::both_pend_down_enable(const cpp_int & _val) { 
    // both_pend_down_enable
    int_var__both_pend_down_enable = _val.convert_to< both_pend_down_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::both_pend_down_enable() const {
    return int_var__both_pend_down_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::pend_wo_proc_down_enable(const cpp_int & _val) { 
    // pend_wo_proc_down_enable
    int_var__pend_wo_proc_down_enable = _val.convert_to< pend_wo_proc_down_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::pend_wo_proc_down_enable() const {
    return int_var__pend_wo_proc_down_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::both_pend_went_up_enable(const cpp_int & _val) { 
    // both_pend_went_up_enable
    int_var__both_pend_went_up_enable = _val.convert_to< both_pend_went_up_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::both_pend_went_up_enable() const {
    return int_var__both_pend_went_up_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::loaded_but_no_proc_enable(const cpp_int & _val) { 
    // loaded_but_no_proc_enable
    int_var__loaded_but_no_proc_enable = _val.convert_to< loaded_but_no_proc_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::loaded_but_no_proc_enable() const {
    return int_var__loaded_but_no_proc_enable;
}
    
void cap_te_csr_int_err_int_enable_clear_t::loaded_but_no_proc_tbl_vld_enable(const cpp_int & _val) { 
    // loaded_but_no_proc_tbl_vld_enable
    int_var__loaded_but_no_proc_tbl_vld_enable = _val.convert_to< loaded_but_no_proc_tbl_vld_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_int_enable_clear_t::loaded_but_no_proc_tbl_vld_enable() const {
    return int_var__loaded_but_no_proc_tbl_vld_enable;
}
    
void cap_te_csr_int_err_intreg_t::miss_sop_interrupt(const cpp_int & _val) { 
    // miss_sop_interrupt
    int_var__miss_sop_interrupt = _val.convert_to< miss_sop_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::miss_sop_interrupt() const {
    return int_var__miss_sop_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::miss_eop_interrupt(const cpp_int & _val) { 
    // miss_eop_interrupt
    int_var__miss_eop_interrupt = _val.convert_to< miss_eop_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::miss_eop_interrupt() const {
    return int_var__miss_eop_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::phv_max_size_interrupt(const cpp_int & _val) { 
    // phv_max_size_interrupt
    int_var__phv_max_size_interrupt = _val.convert_to< phv_max_size_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::phv_max_size_interrupt() const {
    return int_var__phv_max_size_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::spurious_axi_rsp_interrupt(const cpp_int & _val) { 
    // spurious_axi_rsp_interrupt
    int_var__spurious_axi_rsp_interrupt = _val.convert_to< spurious_axi_rsp_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::spurious_axi_rsp_interrupt() const {
    return int_var__spurious_axi_rsp_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::spurious_tcam_rsp_interrupt(const cpp_int & _val) { 
    // spurious_tcam_rsp_interrupt
    int_var__spurious_tcam_rsp_interrupt = _val.convert_to< spurious_tcam_rsp_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::spurious_tcam_rsp_interrupt() const {
    return int_var__spurious_tcam_rsp_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::te2mpu_timeout_interrupt(const cpp_int & _val) { 
    // te2mpu_timeout_interrupt
    int_var__te2mpu_timeout_interrupt = _val.convert_to< te2mpu_timeout_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::te2mpu_timeout_interrupt() const {
    return int_var__te2mpu_timeout_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::axi_rdrsp_err_interrupt(const cpp_int & _val) { 
    // axi_rdrsp_err_interrupt
    int_var__axi_rdrsp_err_interrupt = _val.convert_to< axi_rdrsp_err_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::axi_rdrsp_err_interrupt() const {
    return int_var__axi_rdrsp_err_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::axi_bad_read_interrupt(const cpp_int & _val) { 
    // axi_bad_read_interrupt
    int_var__axi_bad_read_interrupt = _val.convert_to< axi_bad_read_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::axi_bad_read_interrupt() const {
    return int_var__axi_bad_read_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::tcam_req_idx_fifo_interrupt(const cpp_int & _val) { 
    // tcam_req_idx_fifo_interrupt
    int_var__tcam_req_idx_fifo_interrupt = _val.convert_to< tcam_req_idx_fifo_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::tcam_req_idx_fifo_interrupt() const {
    return int_var__tcam_req_idx_fifo_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::tcam_rsp_idx_fifo_interrupt(const cpp_int & _val) { 
    // tcam_rsp_idx_fifo_interrupt
    int_var__tcam_rsp_idx_fifo_interrupt = _val.convert_to< tcam_rsp_idx_fifo_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::tcam_rsp_idx_fifo_interrupt() const {
    return int_var__tcam_rsp_idx_fifo_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::mpu_req_idx_fifo_interrupt(const cpp_int & _val) { 
    // mpu_req_idx_fifo_interrupt
    int_var__mpu_req_idx_fifo_interrupt = _val.convert_to< mpu_req_idx_fifo_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::mpu_req_idx_fifo_interrupt() const {
    return int_var__mpu_req_idx_fifo_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::axi_req_idx_fifo_interrupt(const cpp_int & _val) { 
    // axi_req_idx_fifo_interrupt
    int_var__axi_req_idx_fifo_interrupt = _val.convert_to< axi_req_idx_fifo_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::axi_req_idx_fifo_interrupt() const {
    return int_var__axi_req_idx_fifo_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::proc_tbl_vld_wo_proc_interrupt(const cpp_int & _val) { 
    // proc_tbl_vld_wo_proc_interrupt
    int_var__proc_tbl_vld_wo_proc_interrupt = _val.convert_to< proc_tbl_vld_wo_proc_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::proc_tbl_vld_wo_proc_interrupt() const {
    return int_var__proc_tbl_vld_wo_proc_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::pend_wo_wb_interrupt(const cpp_int & _val) { 
    // pend_wo_wb_interrupt
    int_var__pend_wo_wb_interrupt = _val.convert_to< pend_wo_wb_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::pend_wo_wb_interrupt() const {
    return int_var__pend_wo_wb_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::pend1_wo_pend0_interrupt(const cpp_int & _val) { 
    // pend1_wo_pend0_interrupt
    int_var__pend1_wo_pend0_interrupt = _val.convert_to< pend1_wo_pend0_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::pend1_wo_pend0_interrupt() const {
    return int_var__pend1_wo_pend0_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::both_pend_down_interrupt(const cpp_int & _val) { 
    // both_pend_down_interrupt
    int_var__both_pend_down_interrupt = _val.convert_to< both_pend_down_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::both_pend_down_interrupt() const {
    return int_var__both_pend_down_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::pend_wo_proc_down_interrupt(const cpp_int & _val) { 
    // pend_wo_proc_down_interrupt
    int_var__pend_wo_proc_down_interrupt = _val.convert_to< pend_wo_proc_down_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::pend_wo_proc_down_interrupt() const {
    return int_var__pend_wo_proc_down_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::both_pend_went_up_interrupt(const cpp_int & _val) { 
    // both_pend_went_up_interrupt
    int_var__both_pend_went_up_interrupt = _val.convert_to< both_pend_went_up_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::both_pend_went_up_interrupt() const {
    return int_var__both_pend_went_up_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::loaded_but_no_proc_interrupt(const cpp_int & _val) { 
    // loaded_but_no_proc_interrupt
    int_var__loaded_but_no_proc_interrupt = _val.convert_to< loaded_but_no_proc_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::loaded_but_no_proc_interrupt() const {
    return int_var__loaded_but_no_proc_interrupt;
}
    
void cap_te_csr_int_err_intreg_t::loaded_but_no_proc_tbl_vld_interrupt(const cpp_int & _val) { 
    // loaded_but_no_proc_tbl_vld_interrupt
    int_var__loaded_but_no_proc_tbl_vld_interrupt = _val.convert_to< loaded_but_no_proc_tbl_vld_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_int_err_intreg_t::loaded_but_no_proc_tbl_vld_interrupt() const {
    return int_var__loaded_but_no_proc_tbl_vld_interrupt;
}
    
void cap_te_csr_intreg_status_t::int_err_interrupt(const cpp_int & _val) { 
    // int_err_interrupt
    int_var__int_err_interrupt = _val.convert_to< int_err_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_intreg_status_t::int_err_interrupt() const {
    return int_var__int_err_interrupt;
}
    
void cap_te_csr_intreg_status_t::int_info_interrupt(const cpp_int & _val) { 
    // int_info_interrupt
    int_var__int_info_interrupt = _val.convert_to< int_info_interrupt_cpp_int_t >();
}

cpp_int cap_te_csr_intreg_status_t::int_info_interrupt() const {
    return int_var__int_info_interrupt;
}
    
void cap_te_csr_int_groups_int_enable_rw_reg_t::int_err_enable(const cpp_int & _val) { 
    // int_err_enable
    int_var__int_err_enable = _val.convert_to< int_err_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_groups_int_enable_rw_reg_t::int_err_enable() const {
    return int_var__int_err_enable;
}
    
void cap_te_csr_int_groups_int_enable_rw_reg_t::int_info_enable(const cpp_int & _val) { 
    // int_info_enable
    int_var__int_info_enable = _val.convert_to< int_info_enable_cpp_int_t >();
}

cpp_int cap_te_csr_int_groups_int_enable_rw_reg_t::int_info_enable() const {
    return int_var__int_info_enable;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0(const cpp_int & _val) { 
    // km_mode0
    int_var__km_mode0 = _val.convert_to< km_mode0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0() const {
    return int_var__km_mode0;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1(const cpp_int & _val) { 
    // km_mode1
    int_var__km_mode1 = _val.convert_to< km_mode1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1() const {
    return int_var__km_mode1;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2(const cpp_int & _val) { 
    // km_mode2
    int_var__km_mode2 = _val.convert_to< km_mode2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2() const {
    return int_var__km_mode2;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3(const cpp_int & _val) { 
    // km_mode3
    int_var__km_mode3 = _val.convert_to< km_mode3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3() const {
    return int_var__km_mode3;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0(const cpp_int & _val) { 
    // km_profile0
    int_var__km_profile0 = _val.convert_to< km_profile0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0() const {
    return int_var__km_profile0;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1(const cpp_int & _val) { 
    // km_profile1
    int_var__km_profile1 = _val.convert_to< km_profile1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1() const {
    return int_var__km_profile1;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2(const cpp_int & _val) { 
    // km_profile2
    int_var__km_profile2 = _val.convert_to< km_profile2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2() const {
    return int_var__km_profile2;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3(const cpp_int & _val) { 
    // km_profile3
    int_var__km_profile3 = _val.convert_to< km_profile3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3() const {
    return int_var__km_profile3;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0(const cpp_int & _val) { 
    // km_new_key0
    int_var__km_new_key0 = _val.convert_to< km_new_key0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0() const {
    return int_var__km_new_key0;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1(const cpp_int & _val) { 
    // km_new_key1
    int_var__km_new_key1 = _val.convert_to< km_new_key1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1() const {
    return int_var__km_new_key1;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2(const cpp_int & _val) { 
    // km_new_key2
    int_var__km_new_key2 = _val.convert_to< km_new_key2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2() const {
    return int_var__km_new_key2;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3(const cpp_int & _val) { 
    // km_new_key3
    int_var__km_new_key3 = _val.convert_to< km_new_key3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3() const {
    return int_var__km_new_key3;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup(const cpp_int & _val) { 
    // lkup
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup() const {
    return int_var__lkup;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid(const cpp_int & _val) { 
    // tableid
    int_var__tableid = _val.convert_to< tableid_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid() const {
    return int_var__tableid;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel(const cpp_int & _val) { 
    // hash_sel
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel() const {
    return int_var__hash_sel;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain(const cpp_int & _val) { 
    // hash_chain
    int_var__hash_chain = _val.convert_to< hash_chain_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain() const {
    return int_var__hash_chain;
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store(const cpp_int & _val) { 
    // hash_store
    int_var__hash_store = _val.convert_to< hash_store_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store() const {
    return int_var__hash_store;
}
    
void cap_te_csr_CNT_stall_t::reason0(const cpp_int & _val) { 
    // reason0
    int_var__reason0 = _val.convert_to< reason0_cpp_int_t >();
}

cpp_int cap_te_csr_CNT_stall_t::reason0() const {
    return int_var__reason0;
}
    
void cap_te_csr_CNT_stall_t::reason1(const cpp_int & _val) { 
    // reason1
    int_var__reason1 = _val.convert_to< reason1_cpp_int_t >();
}

cpp_int cap_te_csr_CNT_stall_t::reason1() const {
    return int_var__reason1;
}
    
void cap_te_csr_CNT_stall_t::reason2(const cpp_int & _val) { 
    // reason2
    int_var__reason2 = _val.convert_to< reason2_cpp_int_t >();
}

cpp_int cap_te_csr_CNT_stall_t::reason2() const {
    return int_var__reason2;
}
    
void cap_te_csr_CNT_stall_t::reason3(const cpp_int & _val) { 
    // reason3
    int_var__reason3 = _val.convert_to< reason3_cpp_int_t >();
}

cpp_int cap_te_csr_CNT_stall_t::reason3() const {
    return int_var__reason3;
}
    
void cap_te_csr_cfg_hbm_addr_align_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_hbm_addr_align_t::en() const {
    return int_var__en;
}
    
void cap_te_csr_cfg_bist_t::run(const cpp_int & _val) { 
    // run
    int_var__run = _val.convert_to< run_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_bist_t::run() const {
    return int_var__run;
}
    
void cap_te_csr_sta_bist_done_t::pass(const cpp_int & _val) { 
    // pass
    int_var__pass = _val.convert_to< pass_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bist_done_t::pass() const {
    return int_var__pass;
}
    
void cap_te_csr_sta_bist_done_t::fail(const cpp_int & _val) { 
    // fail
    int_var__fail = _val.convert_to< fail_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bist_done_t::fail() const {
    return int_var__fail;
}
    
void cap_te_csr_sta_peek_t::signals(const cpp_int & _val) { 
    // signals
    int_var__signals = _val.convert_to< signals_cpp_int_t >();
}

cpp_int cap_te_csr_sta_peek_t::signals() const {
    return int_var__signals;
}
    
void cap_te_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_te_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_te_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_te_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_te_csr_sta_bad_axi_read_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bad_axi_read_t::addr() const {
    return int_var__addr;
}
    
void cap_te_csr_sta_bad_axi_read_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bad_axi_read_t::id() const {
    return int_var__id;
}
    
void cap_te_csr_sta_bad_axi_read_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bad_axi_read_t::len() const {
    return int_var__len;
}
    
void cap_te_csr_sta_bad_axi_read_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_te_csr_sta_bad_axi_read_t::sz() const {
    return int_var__sz;
}
    
void cap_te_csr_sta_axi_err_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_te_csr_sta_axi_err_t::resp() const {
    return int_var__resp;
}
    
void cap_te_csr_sta_axi_err_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_csr_sta_axi_err_t::id() const {
    return int_var__id;
}
    
void cap_te_csr_sta_wait_t::entry_dat(const cpp_int & _val) { 
    // entry_dat
    int_var__entry_dat = _val.convert_to< entry_dat_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_t::entry_dat() const {
    return int_var__entry_dat;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry0(const cpp_int & _val) { 
    // entry0
    int_var__entry0 = _val.convert_to< entry0_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry0() const {
    return int_var__entry0;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry1(const cpp_int & _val) { 
    // entry1
    int_var__entry1 = _val.convert_to< entry1_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry1() const {
    return int_var__entry1;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry2(const cpp_int & _val) { 
    // entry2
    int_var__entry2 = _val.convert_to< entry2_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry2() const {
    return int_var__entry2;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry3(const cpp_int & _val) { 
    // entry3
    int_var__entry3 = _val.convert_to< entry3_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry3() const {
    return int_var__entry3;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry4(const cpp_int & _val) { 
    // entry4
    int_var__entry4 = _val.convert_to< entry4_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry4() const {
    return int_var__entry4;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry5(const cpp_int & _val) { 
    // entry5
    int_var__entry5 = _val.convert_to< entry5_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry5() const {
    return int_var__entry5;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry6(const cpp_int & _val) { 
    // entry6
    int_var__entry6 = _val.convert_to< entry6_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry6() const {
    return int_var__entry6;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry7(const cpp_int & _val) { 
    // entry7
    int_var__entry7 = _val.convert_to< entry7_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry7() const {
    return int_var__entry7;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry8(const cpp_int & _val) { 
    // entry8
    int_var__entry8 = _val.convert_to< entry8_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry8() const {
    return int_var__entry8;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry9(const cpp_int & _val) { 
    // entry9
    int_var__entry9 = _val.convert_to< entry9_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry9() const {
    return int_var__entry9;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry10(const cpp_int & _val) { 
    // entry10
    int_var__entry10 = _val.convert_to< entry10_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry10() const {
    return int_var__entry10;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry11(const cpp_int & _val) { 
    // entry11
    int_var__entry11 = _val.convert_to< entry11_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry11() const {
    return int_var__entry11;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry12(const cpp_int & _val) { 
    // entry12
    int_var__entry12 = _val.convert_to< entry12_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry12() const {
    return int_var__entry12;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry13(const cpp_int & _val) { 
    // entry13
    int_var__entry13 = _val.convert_to< entry13_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry13() const {
    return int_var__entry13;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry14(const cpp_int & _val) { 
    // entry14
    int_var__entry14 = _val.convert_to< entry14_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry14() const {
    return int_var__entry14;
}
    
void cap_te_csr_sta_wait_fsm_states_t::entry15(const cpp_int & _val) { 
    // entry15
    int_var__entry15 = _val.convert_to< entry15_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_fsm_states_t::entry15() const {
    return int_var__entry15;
}
    
void cap_te_csr_sta_wait_entry_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::key() const {
    return int_var__key;
}
    
void cap_te_csr_sta_wait_entry_t::cam(const cpp_int & _val) { 
    // cam
    int_var__cam = _val.convert_to< cam_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::cam() const {
    return int_var__cam;
}
    
void cap_te_csr_sta_wait_entry_t::haz(const cpp_int & _val) { 
    // haz
    int_var__haz = _val.convert_to< haz_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::haz() const {
    return int_var__haz;
}
    
void cap_te_csr_sta_wait_entry_t::fsm(const cpp_int & _val) { 
    // fsm
    int_var__fsm = _val.convert_to< fsm_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::fsm() const {
    return int_var__fsm;
}
    
void cap_te_csr_cfg_timeout_t::te2mpu(const cpp_int & _val) { 
    // te2mpu
    int_var__te2mpu = _val.convert_to< te2mpu_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_timeout_t::te2mpu() const {
    return int_var__te2mpu;
}
    
void cap_te_csr_cfg_page_size_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_page_size_t::sz() const {
    return int_var__sz;
}
    
void cap_te_csr_cfg_read_wait_entry_t::idx(const cpp_int & _val) { 
    // idx
    int_var__idx = _val.convert_to< idx_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_read_wait_entry_t::idx() const {
    return int_var__idx;
}
    
void cap_te_csr_cfg_addr_target_t::host(const cpp_int & _val) { 
    // host
    int_var__host = _val.convert_to< host_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_addr_target_t::host() const {
    return int_var__host;
}
    
void cap_te_csr_cfg_addr_target_t::sram(const cpp_int & _val) { 
    // sram
    int_var__sram = _val.convert_to< sram_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_addr_target_t::sram() const {
    return int_var__sram;
}
    
void cap_te_csr_sta_debug_bus_t::vec(const cpp_int & _val) { 
    // vec
    int_var__vec = _val.convert_to< vec_cpp_int_t >();
}

cpp_int cap_te_csr_sta_debug_bus_t::vec() const {
    return int_var__vec;
}
    
void cap_te_csr_cfg_debug_bus_t::axi_rdata_sel(const cpp_int & _val) { 
    // axi_rdata_sel
    int_var__axi_rdata_sel = _val.convert_to< axi_rdata_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::axi_rdata_sel() const {
    return int_var__axi_rdata_sel;
}
    
void cap_te_csr_cfg_debug_bus_t::tcam_key_sel(const cpp_int & _val) { 
    // tcam_key_sel
    int_var__tcam_key_sel = _val.convert_to< tcam_key_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::tcam_key_sel() const {
    return int_var__tcam_key_sel;
}
    
void cap_te_csr_cfg_debug_bus_t::hash_key_sel(const cpp_int & _val) { 
    // hash_key_sel
    int_var__hash_key_sel = _val.convert_to< hash_key_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::hash_key_sel() const {
    return int_var__hash_key_sel;
}
    
void cap_te_csr_cfg_debug_bus_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::sel() const {
    return int_var__sel;
}
    
void cap_te_csr_cnt_mpu_out_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_mpu_out_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_tcam_rsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_tcam_rsp_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_tcam_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_tcam_req_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_axi_rdrsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_axi_rdrsp_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_axi_rdreq_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_axi_rdreq_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_phv_out_eop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_out_eop_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_phv_out_sop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_out_sop_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_phv_in_eop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_in_eop_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_cnt_phv_in_sop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_in_sop_t::val() const {
    return int_var__val;
}
    
void cap_te_csr_dhs_single_step_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::key() const {
    return int_var__key;
}
    
void cap_te_csr_dhs_single_step_t::lkup(const cpp_int & _val) { 
    // lkup
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::lkup() const {
    return int_var__lkup;
}
    
void cap_te_csr_dhs_single_step_t::mpu(const cpp_int & _val) { 
    // mpu
    int_var__mpu = _val.convert_to< mpu_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::mpu() const {
    return int_var__mpu;
}
    
void cap_te_csr_cfg_km_profile_bit_loc_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::valid() const {
    return int_var__valid;
}
    
void cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc(const cpp_int & _val) { 
    // bit_loc
    int_var__bit_loc = _val.convert_to< bit_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc() const {
    return int_var__bit_loc;
}
    
void cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel(const cpp_int & _val) { 
    // bit_sel
    int_var__bit_sel = _val.convert_to< bit_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel() const {
    return int_var__bit_sel;
}
    
void cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel(const cpp_int & _val) { 
    // byte_sel
    int_var__byte_sel = _val.convert_to< byte_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel() const {
    return int_var__byte_sel;
}
    
void cap_te_csr_cfg_table_mpu_const_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_mpu_const_t::value() const {
    return int_var__value;
}
    
void cap_te_csr_cfg_table_property_t::axi(const cpp_int & _val) { 
    // axi
    int_var__axi = _val.convert_to< axi_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::axi() const {
    return int_var__axi;
}
    
void cap_te_csr_cfg_table_property_t::key_mask_hi(const cpp_int & _val) { 
    // key_mask_hi
    int_var__key_mask_hi = _val.convert_to< key_mask_hi_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::key_mask_hi() const {
    return int_var__key_mask_hi;
}
    
void cap_te_csr_cfg_table_property_t::key_mask_lo(const cpp_int & _val) { 
    // key_mask_lo
    int_var__key_mask_lo = _val.convert_to< key_mask_lo_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::key_mask_lo() const {
    return int_var__key_mask_lo;
}
    
void cap_te_csr_cfg_table_property_t::fullkey_km_sel0(const cpp_int & _val) { 
    // fullkey_km_sel0
    int_var__fullkey_km_sel0 = _val.convert_to< fullkey_km_sel0_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::fullkey_km_sel0() const {
    return int_var__fullkey_km_sel0;
}
    
void cap_te_csr_cfg_table_property_t::fullkey_km_sel1(const cpp_int & _val) { 
    // fullkey_km_sel1
    int_var__fullkey_km_sel1 = _val.convert_to< fullkey_km_sel1_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::fullkey_km_sel1() const {
    return int_var__fullkey_km_sel1;
}
    
void cap_te_csr_cfg_table_property_t::lock_en(const cpp_int & _val) { 
    // lock_en
    int_var__lock_en = _val.convert_to< lock_en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lock_en() const {
    return int_var__lock_en;
}
    
void cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw(const cpp_int & _val) { 
    // tbl_entry_sz_raw
    int_var__tbl_entry_sz_raw = _val.convert_to< tbl_entry_sz_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw() const {
    return int_var__tbl_entry_sz_raw;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_dyn(const cpp_int & _val) { 
    // mpu_pc_dyn
    int_var__mpu_pc_dyn = _val.convert_to< mpu_pc_dyn_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_dyn() const {
    return int_var__mpu_pc_dyn;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc(const cpp_int & _val) { 
    // mpu_pc
    int_var__mpu_pc = _val.convert_to< mpu_pc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc() const {
    return int_var__mpu_pc;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_raw(const cpp_int & _val) { 
    // mpu_pc_raw
    int_var__mpu_pc_raw = _val.convert_to< mpu_pc_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_raw() const {
    return int_var__mpu_pc_raw;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err(const cpp_int & _val) { 
    // mpu_pc_ofst_err
    int_var__mpu_pc_ofst_err = _val.convert_to< mpu_pc_ofst_err_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err() const {
    return int_var__mpu_pc_ofst_err;
}
    
void cap_te_csr_cfg_table_property_t::mpu_vec(const cpp_int & _val) { 
    // mpu_vec
    int_var__mpu_vec = _val.convert_to< mpu_vec_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_vec() const {
    return int_var__mpu_vec;
}
    
void cap_te_csr_cfg_table_property_t::addr_base(const cpp_int & _val) { 
    // addr_base
    int_var__addr_base = _val.convert_to< addr_base_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_base() const {
    return int_var__addr_base;
}
    
void cap_te_csr_cfg_table_property_t::addr_vf_id_en(const cpp_int & _val) { 
    // addr_vf_id_en
    int_var__addr_vf_id_en = _val.convert_to< addr_vf_id_en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_vf_id_en() const {
    return int_var__addr_vf_id_en;
}
    
void cap_te_csr_cfg_table_property_t::addr_vf_id_loc(const cpp_int & _val) { 
    // addr_vf_id_loc
    int_var__addr_vf_id_loc = _val.convert_to< addr_vf_id_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_vf_id_loc() const {
    return int_var__addr_vf_id_loc;
}
    
void cap_te_csr_cfg_table_property_t::addr_shift(const cpp_int & _val) { 
    // addr_shift
    int_var__addr_shift = _val.convert_to< addr_shift_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_shift() const {
    return int_var__addr_shift;
}
    
void cap_te_csr_cfg_table_property_t::chain_shift(const cpp_int & _val) { 
    // chain_shift
    int_var__chain_shift = _val.convert_to< chain_shift_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::chain_shift() const {
    return int_var__chain_shift;
}
    
void cap_te_csr_cfg_table_property_t::addr_sz(const cpp_int & _val) { 
    // addr_sz
    int_var__addr_sz = _val.convert_to< addr_sz_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_sz() const {
    return int_var__addr_sz;
}
    
void cap_te_csr_cfg_table_property_t::lg2_entry_size(const cpp_int & _val) { 
    // lg2_entry_size
    int_var__lg2_entry_size = _val.convert_to< lg2_entry_size_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lg2_entry_size() const {
    return int_var__lg2_entry_size;
}
    
void cap_te_csr_cfg_table_property_t::max_bypass_cnt(const cpp_int & _val) { 
    // max_bypass_cnt
    int_var__max_bypass_cnt = _val.convert_to< max_bypass_cnt_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::max_bypass_cnt() const {
    return int_var__max_bypass_cnt;
}
    
void cap_te_csr_cfg_table_property_t::lock_en_raw(const cpp_int & _val) { 
    // lock_en_raw
    int_var__lock_en_raw = _val.convert_to< lock_en_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lock_en_raw() const {
    return int_var__lock_en_raw;
}
    
void cap_te_csr_cfg_table_property_t::mpu_lb(const cpp_int & _val) { 
    // mpu_lb
    int_var__mpu_lb = _val.convert_to< mpu_lb_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_lb() const {
    return int_var__mpu_lb;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_loc(const cpp_int & _val) { 
    // mpu_pc_loc
    int_var__mpu_pc_loc = _val.convert_to< mpu_pc_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_loc() const {
    return int_var__mpu_pc_loc;
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_ofst_shift(const cpp_int & _val) { 
    // mpu_pc_ofst_shift
    int_var__mpu_pc_ofst_shift = _val.convert_to< mpu_pc_ofst_shift_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_ofst_shift() const {
    return int_var__mpu_pc_ofst_shift;
}
    
void cap_te_csr_cfg_table_property_t::oflow_base_idx(const cpp_int & _val) { 
    // oflow_base_idx
    int_var__oflow_base_idx = _val.convert_to< oflow_base_idx_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::oflow_base_idx() const {
    return int_var__oflow_base_idx;
}
    
void cap_te_csr_cfg_table_property_t::en_pc_axi_err(const cpp_int & _val) { 
    // en_pc_axi_err
    int_var__en_pc_axi_err = _val.convert_to< en_pc_axi_err_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::en_pc_axi_err() const {
    return int_var__en_pc_axi_err;
}
    
void cap_te_csr_cfg_table_property_t::en_pc_phv_err(const cpp_int & _val) { 
    // en_pc_phv_err
    int_var__en_pc_phv_err = _val.convert_to< en_pc_phv_err_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::en_pc_phv_err() const {
    return int_var__en_pc_phv_err;
}
    
void cap_te_csr_cfg_table_property_t::memory_only(const cpp_int & _val) { 
    // memory_only
    int_var__memory_only = _val.convert_to< memory_only_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::memory_only() const {
    return int_var__memory_only;
}
    
void cap_te_csr_cfg_table_profile_t::mpu_results(const cpp_int & _val) { 
    // mpu_results
    int_var__mpu_results = _val.convert_to< mpu_results_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_t::mpu_results() const {
    return int_var__mpu_results;
}
    
void cap_te_csr_cfg_table_profile_t::seq_base(const cpp_int & _val) { 
    // seq_base
    int_var__seq_base = _val.convert_to< seq_base_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_t::seq_base() const {
    return int_var__seq_base;
}
    
void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit(const cpp_int & _val) { 
    // adv_phv_flit
    int_var__adv_phv_flit = _val.convert_to< adv_phv_flit_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit() const {
    return int_var__adv_phv_flit;
}
    
void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done(const cpp_int & _val) { 
    // done
    int_var__done = _val.convert_to< done_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done() const {
    return int_var__done;
}
    
void cap_te_csr_cfg_table_profile_cam_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::valid() const {
    return int_var__valid;
}
    
void cap_te_csr_cfg_table_profile_cam_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::value() const {
    return int_var__value;
}
    
void cap_te_csr_cfg_table_profile_cam_t::mask(const cpp_int & _val) { 
    // mask
    int_var__mask = _val.convert_to< mask_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::mask() const {
    return int_var__mask;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel0(const cpp_int & _val) { 
    // sel0
    int_var__sel0 = _val.convert_to< sel0_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel0() const {
    return int_var__sel0;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel1(const cpp_int & _val) { 
    // sel1
    int_var__sel1 = _val.convert_to< sel1_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel1() const {
    return int_var__sel1;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel2(const cpp_int & _val) { 
    // sel2
    int_var__sel2 = _val.convert_to< sel2_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel2() const {
    return int_var__sel2;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel3(const cpp_int & _val) { 
    // sel3
    int_var__sel3 = _val.convert_to< sel3_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel3() const {
    return int_var__sel3;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel4(const cpp_int & _val) { 
    // sel4
    int_var__sel4 = _val.convert_to< sel4_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel4() const {
    return int_var__sel4;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel5(const cpp_int & _val) { 
    // sel5
    int_var__sel5 = _val.convert_to< sel5_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel5() const {
    return int_var__sel5;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel6(const cpp_int & _val) { 
    // sel6
    int_var__sel6 = _val.convert_to< sel6_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel6() const {
    return int_var__sel6;
}
    
void cap_te_csr_cfg_table_profile_key_t::sel7(const cpp_int & _val) { 
    // sel7
    int_var__sel7 = _val.convert_to< sel7_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel7() const {
    return int_var__sel7;
}
    
void cap_te_csr_cfg_global_t::sw_rst(const cpp_int & _val) { 
    // sw_rst
    int_var__sw_rst = _val.convert_to< sw_rst_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::sw_rst() const {
    return int_var__sw_rst;
}
    
void cap_te_csr_cfg_global_t::step_key(const cpp_int & _val) { 
    // step_key
    int_var__step_key = _val.convert_to< step_key_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_key() const {
    return int_var__step_key;
}
    
void cap_te_csr_cfg_global_t::step_lkup(const cpp_int & _val) { 
    // step_lkup
    int_var__step_lkup = _val.convert_to< step_lkup_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_lkup() const {
    return int_var__step_lkup;
}
    
void cap_te_csr_cfg_global_t::step_mpu(const cpp_int & _val) { 
    // step_mpu
    int_var__step_mpu = _val.convert_to< step_mpu_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_mpu() const {
    return int_var__step_mpu;
}
    
void cap_te_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_te_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "profile_cam_hit0_enable")) { field_val = profile_cam_hit0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit1_enable")) { field_val = profile_cam_hit1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit2_enable")) { field_val = profile_cam_hit2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit3_enable")) { field_val = profile_cam_hit3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit4_enable")) { field_val = profile_cam_hit4_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit5_enable")) { field_val = profile_cam_hit5_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit6_enable")) { field_val = profile_cam_hit6_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit7_enable")) { field_val = profile_cam_hit7_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit8_enable")) { field_val = profile_cam_hit8_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit9_enable")) { field_val = profile_cam_hit9_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit10_enable")) { field_val = profile_cam_hit10_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit11_enable")) { field_val = profile_cam_hit11_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit12_enable")) { field_val = profile_cam_hit12_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit13_enable")) { field_val = profile_cam_hit13_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit14_enable")) { field_val = profile_cam_hit14_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit15_enable")) { field_val = profile_cam_hit15_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_miss_enable")) { field_val = profile_cam_miss_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_int_test_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "profile_cam_hit0_interrupt")) { field_val = profile_cam_hit0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit1_interrupt")) { field_val = profile_cam_hit1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit2_interrupt")) { field_val = profile_cam_hit2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit3_interrupt")) { field_val = profile_cam_hit3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit4_interrupt")) { field_val = profile_cam_hit4_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit5_interrupt")) { field_val = profile_cam_hit5_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit6_interrupt")) { field_val = profile_cam_hit6_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit7_interrupt")) { field_val = profile_cam_hit7_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit8_interrupt")) { field_val = profile_cam_hit8_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit9_interrupt")) { field_val = profile_cam_hit9_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit10_interrupt")) { field_val = profile_cam_hit10_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit11_interrupt")) { field_val = profile_cam_hit11_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit12_interrupt")) { field_val = profile_cam_hit12_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit13_interrupt")) { field_val = profile_cam_hit13_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit14_interrupt")) { field_val = profile_cam_hit14_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit15_interrupt")) { field_val = profile_cam_hit15_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_miss_interrupt")) { field_val = profile_cam_miss_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_int_err_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "miss_sop_enable")) { field_val = miss_sop_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "miss_eop_enable")) { field_val = miss_eop_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_max_size_enable")) { field_val = phv_max_size_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_axi_rsp_enable")) { field_val = spurious_axi_rsp_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_tcam_rsp_enable")) { field_val = spurious_tcam_rsp_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "te2mpu_timeout_enable")) { field_val = te2mpu_timeout_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdrsp_err_enable")) { field_val = axi_rdrsp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_bad_read_enable")) { field_val = axi_bad_read_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_req_idx_fifo_enable")) { field_val = tcam_req_idx_fifo_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_rsp_idx_fifo_enable")) { field_val = tcam_rsp_idx_fifo_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_req_idx_fifo_enable")) { field_val = mpu_req_idx_fifo_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_req_idx_fifo_enable")) { field_val = axi_req_idx_fifo_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "proc_tbl_vld_wo_proc_enable")) { field_val = proc_tbl_vld_wo_proc_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_wb_enable")) { field_val = pend_wo_wb_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend1_wo_pend0_enable")) { field_val = pend1_wo_pend0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_down_enable")) { field_val = both_pend_down_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_proc_down_enable")) { field_val = pend_wo_proc_down_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_went_up_enable")) { field_val = both_pend_went_up_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_enable")) { field_val = loaded_but_no_proc_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_tbl_vld_enable")) { field_val = loaded_but_no_proc_tbl_vld_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_err_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "miss_sop_interrupt")) { field_val = miss_sop_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "miss_eop_interrupt")) { field_val = miss_eop_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_max_size_interrupt")) { field_val = phv_max_size_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_axi_rsp_interrupt")) { field_val = spurious_axi_rsp_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_tcam_rsp_interrupt")) { field_val = spurious_tcam_rsp_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "te2mpu_timeout_interrupt")) { field_val = te2mpu_timeout_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdrsp_err_interrupt")) { field_val = axi_rdrsp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_bad_read_interrupt")) { field_val = axi_bad_read_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_req_idx_fifo_interrupt")) { field_val = tcam_req_idx_fifo_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_rsp_idx_fifo_interrupt")) { field_val = tcam_rsp_idx_fifo_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_req_idx_fifo_interrupt")) { field_val = mpu_req_idx_fifo_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_req_idx_fifo_interrupt")) { field_val = axi_req_idx_fifo_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "proc_tbl_vld_wo_proc_interrupt")) { field_val = proc_tbl_vld_wo_proc_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_wb_interrupt")) { field_val = pend_wo_wb_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend1_wo_pend0_interrupt")) { field_val = pend1_wo_pend0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_down_interrupt")) { field_val = both_pend_down_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_proc_down_interrupt")) { field_val = pend_wo_proc_down_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_went_up_interrupt")) { field_val = both_pend_went_up_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_interrupt")) { field_val = loaded_but_no_proc_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_tbl_vld_interrupt")) { field_val = loaded_but_no_proc_tbl_vld_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_interrupt")) { field_val = int_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_interrupt")) { field_val = int_info_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_enable")) { field_val = int_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_enable")) { field_val = int_info_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "km_mode0")) { field_val = km_mode0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode1")) { field_val = km_mode1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode2")) { field_val = km_mode2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode3")) { field_val = km_mode3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile0")) { field_val = km_profile0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile1")) { field_val = km_profile1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile2")) { field_val = km_profile2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile3")) { field_val = km_profile3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key0")) { field_val = km_new_key0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key1")) { field_val = km_new_key1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key2")) { field_val = km_new_key2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key3")) { field_val = km_new_key3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkup")) { field_val = lkup(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tableid")) { field_val = tableid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { field_val = hash_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_chain")) { field_val = hash_chain(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_store")) { field_val = hash_store(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_dhs_table_profile_ctrl_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_CNT_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reason0")) { field_val = reason0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason1")) { field_val = reason1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason2")) { field_val = reason2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason3")) { field_val = reason3(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_hbm_addr_align_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { field_val = en(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { field_val = run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_bist_done_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pass")) { field_val = pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fail")) { field_val = fail(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_peek_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "signals")) { field_val = signals(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_bad_axi_read_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_axi_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "entry_dat")) { field_val = entry_dat(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_fsm_states_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "entry0")) { field_val = entry0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry1")) { field_val = entry1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry2")) { field_val = entry2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry3")) { field_val = entry3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry4")) { field_val = entry4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry5")) { field_val = entry5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry6")) { field_val = entry6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry7")) { field_val = entry7(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry8")) { field_val = entry8(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry9")) { field_val = entry9(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry10")) { field_val = entry10(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry11")) { field_val = entry11(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry12")) { field_val = entry12(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry13")) { field_val = entry13(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry14")) { field_val = entry14(); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry15")) { field_val = entry15(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { field_val = key(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam")) { field_val = cam(); field_found=1; }
    if(!field_found && !strcmp(field_name, "haz")) { field_val = haz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fsm")) { field_val = fsm(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_timeout_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "te2mpu")) { field_val = te2mpu(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_page_size_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_read_wait_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "idx")) { field_val = idx(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_addr_target_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "host")) { field_val = host(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram")) { field_val = sram(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_debug_bus_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vec")) { field_val = vec(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_debug_bus_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_rdata_sel")) { field_val = axi_rdata_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_key_sel")) { field_val = tcam_key_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_key_sel")) { field_val = hash_key_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_mpu_out_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_tcam_rsp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_tcam_req_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_axi_rdrsp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_axi_rdreq_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_out_eop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_out_sop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_in_eop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_in_sop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_dhs_single_step_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { field_val = key(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkup")) { field_val = lkup(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu")) { field_val = mpu(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_bit_loc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bit_loc")) { field_val = bit_loc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_bit_sel_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bit_sel")) { field_val = bit_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_byte_sel_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "byte_sel")) { field_val = byte_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_mpu_const_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_property_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi")) { field_val = axi(); field_found=1; }
    if(!field_found && !strcmp(field_name, "key_mask_hi")) { field_val = key_mask_hi(); field_found=1; }
    if(!field_found && !strcmp(field_name, "key_mask_lo")) { field_val = key_mask_lo(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fullkey_km_sel0")) { field_val = fullkey_km_sel0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fullkey_km_sel1")) { field_val = fullkey_km_sel1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock_en")) { field_val = lock_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_entry_sz_raw")) { field_val = tbl_entry_sz_raw(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_dyn")) { field_val = mpu_pc_dyn(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc")) { field_val = mpu_pc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_raw")) { field_val = mpu_pc_raw(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_ofst_err")) { field_val = mpu_pc_ofst_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_vec")) { field_val = mpu_vec(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_base")) { field_val = addr_base(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_vf_id_en")) { field_val = addr_vf_id_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_vf_id_loc")) { field_val = addr_vf_id_loc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_shift")) { field_val = addr_shift(); field_found=1; }
    if(!field_found && !strcmp(field_name, "chain_shift")) { field_val = chain_shift(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_sz")) { field_val = addr_sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lg2_entry_size")) { field_val = lg2_entry_size(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_bypass_cnt")) { field_val = max_bypass_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock_en_raw")) { field_val = lock_en_raw(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_lb")) { field_val = mpu_lb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_loc")) { field_val = mpu_pc_loc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_ofst_shift")) { field_val = mpu_pc_ofst_shift(); field_found=1; }
    if(!field_found && !strcmp(field_name, "oflow_base_idx")) { field_val = oflow_base_idx(); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_pc_axi_err")) { field_val = en_pc_axi_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_pc_phv_err")) { field_val = en_pc_phv_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "memory_only")) { field_val = memory_only(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mpu_results")) { field_val = mpu_results(); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_base")) { field_val = seq_base(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "adv_phv_flit")) { field_val = adv_phv_flit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done")) { field_val = done(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_cam_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { field_val = mask(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_key_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel0")) { field_val = sel0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel1")) { field_val = sel1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel2")) { field_val = sel2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel3")) { field_val = sel3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel4")) { field_val = sel4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel5")) { field_val = sel5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel6")) { field_val = sel6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel7")) { field_val = sel7(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_global_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sw_rst")) { field_val = sw_rst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_key")) { field_val = step_key(); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_lkup")) { field_val = step_lkup(); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_mpu")) { field_val = step_mpu(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_global.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_table_profile_key.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_single_step.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_in_sop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_in_eop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_out_sop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_out_eop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_axi_rdreq.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_axi_rdrsp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tcam_req.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tcam_rsp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_bus.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_debug_bus.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_addr_target.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_read_wait_entry.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_page_size.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timeout.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait_entry.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait_fsm_states.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_axi_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bad_axi_read.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_peek.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist_done.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_addr_align.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = CNT_stall.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_table_profile_ctrl_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_info.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "profile_cam_hit0_enable")) { profile_cam_hit0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit1_enable")) { profile_cam_hit1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit2_enable")) { profile_cam_hit2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit3_enable")) { profile_cam_hit3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit4_enable")) { profile_cam_hit4_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit5_enable")) { profile_cam_hit5_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit6_enable")) { profile_cam_hit6_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit7_enable")) { profile_cam_hit7_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit8_enable")) { profile_cam_hit8_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit9_enable")) { profile_cam_hit9_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit10_enable")) { profile_cam_hit10_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit11_enable")) { profile_cam_hit11_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit12_enable")) { profile_cam_hit12_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit13_enable")) { profile_cam_hit13_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit14_enable")) { profile_cam_hit14_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit15_enable")) { profile_cam_hit15_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_miss_enable")) { profile_cam_miss_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_int_test_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "profile_cam_hit0_interrupt")) { profile_cam_hit0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit1_interrupt")) { profile_cam_hit1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit2_interrupt")) { profile_cam_hit2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit3_interrupt")) { profile_cam_hit3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit4_interrupt")) { profile_cam_hit4_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit5_interrupt")) { profile_cam_hit5_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit6_interrupt")) { profile_cam_hit6_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit7_interrupt")) { profile_cam_hit7_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit8_interrupt")) { profile_cam_hit8_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit9_interrupt")) { profile_cam_hit9_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit10_interrupt")) { profile_cam_hit10_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit11_interrupt")) { profile_cam_hit11_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit12_interrupt")) { profile_cam_hit12_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit13_interrupt")) { profile_cam_hit13_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit14_interrupt")) { profile_cam_hit14_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_hit15_interrupt")) { profile_cam_hit15_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "profile_cam_miss_interrupt")) { profile_cam_miss_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_int_err_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "miss_sop_enable")) { miss_sop_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "miss_eop_enable")) { miss_eop_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_max_size_enable")) { phv_max_size_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_axi_rsp_enable")) { spurious_axi_rsp_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_tcam_rsp_enable")) { spurious_tcam_rsp_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "te2mpu_timeout_enable")) { te2mpu_timeout_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdrsp_err_enable")) { axi_rdrsp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_bad_read_enable")) { axi_bad_read_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_req_idx_fifo_enable")) { tcam_req_idx_fifo_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_rsp_idx_fifo_enable")) { tcam_rsp_idx_fifo_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_req_idx_fifo_enable")) { mpu_req_idx_fifo_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_req_idx_fifo_enable")) { axi_req_idx_fifo_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "proc_tbl_vld_wo_proc_enable")) { proc_tbl_vld_wo_proc_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_wb_enable")) { pend_wo_wb_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend1_wo_pend0_enable")) { pend1_wo_pend0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_down_enable")) { both_pend_down_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_proc_down_enable")) { pend_wo_proc_down_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_went_up_enable")) { both_pend_went_up_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_enable")) { loaded_but_no_proc_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_tbl_vld_enable")) { loaded_but_no_proc_tbl_vld_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_err_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "miss_sop_interrupt")) { miss_sop_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "miss_eop_interrupt")) { miss_eop_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_max_size_interrupt")) { phv_max_size_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_axi_rsp_interrupt")) { spurious_axi_rsp_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spurious_tcam_rsp_interrupt")) { spurious_tcam_rsp_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "te2mpu_timeout_interrupt")) { te2mpu_timeout_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rdrsp_err_interrupt")) { axi_rdrsp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_bad_read_interrupt")) { axi_bad_read_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_req_idx_fifo_interrupt")) { tcam_req_idx_fifo_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_rsp_idx_fifo_interrupt")) { tcam_rsp_idx_fifo_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_req_idx_fifo_interrupt")) { mpu_req_idx_fifo_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_req_idx_fifo_interrupt")) { axi_req_idx_fifo_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "proc_tbl_vld_wo_proc_interrupt")) { proc_tbl_vld_wo_proc_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_wb_interrupt")) { pend_wo_wb_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend1_wo_pend0_interrupt")) { pend1_wo_pend0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_down_interrupt")) { both_pend_down_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pend_wo_proc_down_interrupt")) { pend_wo_proc_down_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "both_pend_went_up_interrupt")) { both_pend_went_up_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_interrupt")) { loaded_but_no_proc_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loaded_but_no_proc_tbl_vld_interrupt")) { loaded_but_no_proc_tbl_vld_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_interrupt")) { int_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_interrupt")) { int_info_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_enable")) { int_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_enable")) { int_info_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "km_mode0")) { km_mode0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode1")) { km_mode1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode2")) { km_mode2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_mode3")) { km_mode3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile0")) { km_profile0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile1")) { km_profile1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile2")) { km_profile2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_profile3")) { km_profile3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key0")) { km_new_key0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key1")) { km_new_key1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key2")) { km_new_key2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "km_new_key3")) { km_new_key3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkup")) { lkup(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tableid")) { tableid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { hash_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_chain")) { hash_chain(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_store")) { hash_store(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_dhs_table_profile_ctrl_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_CNT_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reason0")) { reason0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason1")) { reason1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason2")) { reason2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "reason3")) { reason3(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_hbm_addr_align_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { en(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run")) { run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_bist_done_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pass")) { pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fail")) { fail(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_peek_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "signals")) { signals(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_bad_axi_read_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_axi_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "entry_dat")) { entry_dat(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_fsm_states_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "entry0")) { entry0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry1")) { entry1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry2")) { entry2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry3")) { entry3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry4")) { entry4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry5")) { entry5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry6")) { entry6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry7")) { entry7(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry8")) { entry8(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry9")) { entry9(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry10")) { entry10(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry11")) { entry11(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry12")) { entry12(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry13")) { entry13(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry14")) { entry14(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "entry15")) { entry15(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_wait_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { key(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam")) { cam(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "haz")) { haz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fsm")) { fsm(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_timeout_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "te2mpu")) { te2mpu(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_page_size_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_read_wait_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "idx")) { idx(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_addr_target_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "host")) { host(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram")) { sram(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_sta_debug_bus_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vec")) { vec(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_debug_bus_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_rdata_sel")) { axi_rdata_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_key_sel")) { tcam_key_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_key_sel")) { hash_key_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_mpu_out_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_tcam_rsp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_tcam_req_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_axi_rdrsp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_axi_rdreq_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_out_eop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_out_sop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_in_eop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cnt_phv_in_sop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_dhs_single_step_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { key(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkup")) { lkup(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu")) { mpu(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_bit_loc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bit_loc")) { bit_loc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_bit_sel_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bit_sel")) { bit_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_km_profile_byte_sel_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "byte_sel")) { byte_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_mpu_const_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_property_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi")) { axi(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "key_mask_hi")) { key_mask_hi(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "key_mask_lo")) { key_mask_lo(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fullkey_km_sel0")) { fullkey_km_sel0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fullkey_km_sel1")) { fullkey_km_sel1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock_en")) { lock_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_entry_sz_raw")) { tbl_entry_sz_raw(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_dyn")) { mpu_pc_dyn(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc")) { mpu_pc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_raw")) { mpu_pc_raw(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_ofst_err")) { mpu_pc_ofst_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_vec")) { mpu_vec(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_base")) { addr_base(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_vf_id_en")) { addr_vf_id_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_vf_id_loc")) { addr_vf_id_loc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_shift")) { addr_shift(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "chain_shift")) { chain_shift(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_sz")) { addr_sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lg2_entry_size")) { lg2_entry_size(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_bypass_cnt")) { max_bypass_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock_en_raw")) { lock_en_raw(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_lb")) { mpu_lb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_loc")) { mpu_pc_loc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_pc_ofst_shift")) { mpu_pc_ofst_shift(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "oflow_base_idx")) { oflow_base_idx(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_pc_axi_err")) { en_pc_axi_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_pc_phv_err")) { en_pc_phv_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "memory_only")) { memory_only(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mpu_results")) { mpu_results(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_base")) { seq_base(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "adv_phv_flit")) { adv_phv_flit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done")) { done(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_cam_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { mask(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_table_profile_key_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel0")) { sel0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel1")) { sel1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel2")) { sel2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel3")) { sel3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel4")) { sel4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel5")) { sel5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel6")) { sel6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel7")) { sel7(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_cfg_global_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sw_rst")) { sw_rst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_key")) { step_key(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_lkup")) { step_lkup(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_mpu")) { step_mpu(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_global.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_table_profile_key.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_single_step.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_in_sop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_in_eop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_out_sop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_phv_out_eop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_axi_rdreq.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_axi_rdrsp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tcam_req.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tcam_rsp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_bus.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_debug_bus.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_addr_target.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_read_wait_entry.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_page_size.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timeout.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait_entry.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait_fsm_states.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_wait.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_axi_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bad_axi_read.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_peek.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist_done.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_addr_align.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = CNT_stall.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_table_profile_ctrl_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_info.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_info_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("profile_cam_hit0_enable");
    ret_vec.push_back("profile_cam_hit1_enable");
    ret_vec.push_back("profile_cam_hit2_enable");
    ret_vec.push_back("profile_cam_hit3_enable");
    ret_vec.push_back("profile_cam_hit4_enable");
    ret_vec.push_back("profile_cam_hit5_enable");
    ret_vec.push_back("profile_cam_hit6_enable");
    ret_vec.push_back("profile_cam_hit7_enable");
    ret_vec.push_back("profile_cam_hit8_enable");
    ret_vec.push_back("profile_cam_hit9_enable");
    ret_vec.push_back("profile_cam_hit10_enable");
    ret_vec.push_back("profile_cam_hit11_enable");
    ret_vec.push_back("profile_cam_hit12_enable");
    ret_vec.push_back("profile_cam_hit13_enable");
    ret_vec.push_back("profile_cam_hit14_enable");
    ret_vec.push_back("profile_cam_hit15_enable");
    ret_vec.push_back("profile_cam_miss_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_info_int_test_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("profile_cam_hit0_interrupt");
    ret_vec.push_back("profile_cam_hit1_interrupt");
    ret_vec.push_back("profile_cam_hit2_interrupt");
    ret_vec.push_back("profile_cam_hit3_interrupt");
    ret_vec.push_back("profile_cam_hit4_interrupt");
    ret_vec.push_back("profile_cam_hit5_interrupt");
    ret_vec.push_back("profile_cam_hit6_interrupt");
    ret_vec.push_back("profile_cam_hit7_interrupt");
    ret_vec.push_back("profile_cam_hit8_interrupt");
    ret_vec.push_back("profile_cam_hit9_interrupt");
    ret_vec.push_back("profile_cam_hit10_interrupt");
    ret_vec.push_back("profile_cam_hit11_interrupt");
    ret_vec.push_back("profile_cam_hit12_interrupt");
    ret_vec.push_back("profile_cam_hit13_interrupt");
    ret_vec.push_back("profile_cam_hit14_interrupt");
    ret_vec.push_back("profile_cam_hit15_interrupt");
    ret_vec.push_back("profile_cam_miss_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_info_t::get_fields(int level) const { 
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
std::vector<string> cap_te_csr_int_err_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("miss_sop_enable");
    ret_vec.push_back("miss_eop_enable");
    ret_vec.push_back("phv_max_size_enable");
    ret_vec.push_back("spurious_axi_rsp_enable");
    ret_vec.push_back("spurious_tcam_rsp_enable");
    ret_vec.push_back("te2mpu_timeout_enable");
    ret_vec.push_back("axi_rdrsp_err_enable");
    ret_vec.push_back("axi_bad_read_enable");
    ret_vec.push_back("tcam_req_idx_fifo_enable");
    ret_vec.push_back("tcam_rsp_idx_fifo_enable");
    ret_vec.push_back("mpu_req_idx_fifo_enable");
    ret_vec.push_back("axi_req_idx_fifo_enable");
    ret_vec.push_back("proc_tbl_vld_wo_proc_enable");
    ret_vec.push_back("pend_wo_wb_enable");
    ret_vec.push_back("pend1_wo_pend0_enable");
    ret_vec.push_back("both_pend_down_enable");
    ret_vec.push_back("pend_wo_proc_down_enable");
    ret_vec.push_back("both_pend_went_up_enable");
    ret_vec.push_back("loaded_but_no_proc_enable");
    ret_vec.push_back("loaded_but_no_proc_tbl_vld_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_err_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("miss_sop_interrupt");
    ret_vec.push_back("miss_eop_interrupt");
    ret_vec.push_back("phv_max_size_interrupt");
    ret_vec.push_back("spurious_axi_rsp_interrupt");
    ret_vec.push_back("spurious_tcam_rsp_interrupt");
    ret_vec.push_back("te2mpu_timeout_interrupt");
    ret_vec.push_back("axi_rdrsp_err_interrupt");
    ret_vec.push_back("axi_bad_read_interrupt");
    ret_vec.push_back("tcam_req_idx_fifo_interrupt");
    ret_vec.push_back("tcam_rsp_idx_fifo_interrupt");
    ret_vec.push_back("mpu_req_idx_fifo_interrupt");
    ret_vec.push_back("axi_req_idx_fifo_interrupt");
    ret_vec.push_back("proc_tbl_vld_wo_proc_interrupt");
    ret_vec.push_back("pend_wo_wb_interrupt");
    ret_vec.push_back("pend1_wo_pend0_interrupt");
    ret_vec.push_back("both_pend_down_interrupt");
    ret_vec.push_back("pend_wo_proc_down_interrupt");
    ret_vec.push_back("both_pend_went_up_interrupt");
    ret_vec.push_back("loaded_but_no_proc_interrupt");
    ret_vec.push_back("loaded_but_no_proc_tbl_vld_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_err_t::get_fields(int level) const { 
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
std::vector<string> cap_te_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_err_interrupt");
    ret_vec.push_back("int_info_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_err_enable");
    ret_vec.push_back("int_info_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("km_mode0");
    ret_vec.push_back("km_mode1");
    ret_vec.push_back("km_mode2");
    ret_vec.push_back("km_mode3");
    ret_vec.push_back("km_profile0");
    ret_vec.push_back("km_profile1");
    ret_vec.push_back("km_profile2");
    ret_vec.push_back("km_profile3");
    ret_vec.push_back("km_new_key0");
    ret_vec.push_back("km_new_key1");
    ret_vec.push_back("km_new_key2");
    ret_vec.push_back("km_new_key3");
    ret_vec.push_back("lkup");
    ret_vec.push_back("tableid");
    ret_vec.push_back("hash_sel");
    ret_vec.push_back("hash_chain");
    ret_vec.push_back("hash_store");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_dhs_table_profile_ctrl_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_CNT_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("reason0");
    ret_vec.push_back("reason1");
    ret_vec.push_back("reason2");
    ret_vec.push_back("reason3");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_hbm_addr_align_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("en");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_bist_done_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pass");
    ret_vec.push_back("fail");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_peek_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("signals");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_bad_axi_read_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addr");
    ret_vec.push_back("id");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_axi_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("resp");
    ret_vec.push_back("id");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_wait_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("entry_dat");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_wait_fsm_states_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("entry0");
    ret_vec.push_back("entry1");
    ret_vec.push_back("entry2");
    ret_vec.push_back("entry3");
    ret_vec.push_back("entry4");
    ret_vec.push_back("entry5");
    ret_vec.push_back("entry6");
    ret_vec.push_back("entry7");
    ret_vec.push_back("entry8");
    ret_vec.push_back("entry9");
    ret_vec.push_back("entry10");
    ret_vec.push_back("entry11");
    ret_vec.push_back("entry12");
    ret_vec.push_back("entry13");
    ret_vec.push_back("entry14");
    ret_vec.push_back("entry15");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_wait_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("key");
    ret_vec.push_back("cam");
    ret_vec.push_back("haz");
    ret_vec.push_back("fsm");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_timeout_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("te2mpu");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_page_size_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sz");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_read_wait_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("idx");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_addr_target_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("host");
    ret_vec.push_back("sram");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_sta_debug_bus_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("vec");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_debug_bus_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_rdata_sel");
    ret_vec.push_back("tcam_key_sel");
    ret_vec.push_back("hash_key_sel");
    ret_vec.push_back("sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_mpu_out_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_tcam_rsp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_tcam_req_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_axi_rdrsp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_axi_rdreq_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_phv_out_eop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_phv_out_sop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_phv_in_eop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cnt_phv_in_sop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_dhs_single_step_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("key");
    ret_vec.push_back("lkup");
    ret_vec.push_back("mpu");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_km_profile_bit_loc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("valid");
    ret_vec.push_back("bit_loc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_km_profile_bit_sel_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bit_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_km_profile_byte_sel_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("byte_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_mpu_const_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_property_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi");
    ret_vec.push_back("key_mask_hi");
    ret_vec.push_back("key_mask_lo");
    ret_vec.push_back("fullkey_km_sel0");
    ret_vec.push_back("fullkey_km_sel1");
    ret_vec.push_back("lock_en");
    ret_vec.push_back("tbl_entry_sz_raw");
    ret_vec.push_back("mpu_pc_dyn");
    ret_vec.push_back("mpu_pc");
    ret_vec.push_back("mpu_pc_raw");
    ret_vec.push_back("mpu_pc_ofst_err");
    ret_vec.push_back("mpu_vec");
    ret_vec.push_back("addr_base");
    ret_vec.push_back("addr_vf_id_en");
    ret_vec.push_back("addr_vf_id_loc");
    ret_vec.push_back("addr_shift");
    ret_vec.push_back("chain_shift");
    ret_vec.push_back("addr_sz");
    ret_vec.push_back("lg2_entry_size");
    ret_vec.push_back("max_bypass_cnt");
    ret_vec.push_back("lock_en_raw");
    ret_vec.push_back("mpu_lb");
    ret_vec.push_back("mpu_pc_loc");
    ret_vec.push_back("mpu_pc_ofst_shift");
    ret_vec.push_back("oflow_base_idx");
    ret_vec.push_back("en_pc_axi_err");
    ret_vec.push_back("en_pc_phv_err");
    ret_vec.push_back("memory_only");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mpu_results");
    ret_vec.push_back("seq_base");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("adv_phv_flit");
    ret_vec.push_back("done");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_profile_cam_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("valid");
    ret_vec.push_back("value");
    ret_vec.push_back("mask");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_table_profile_key_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel0");
    ret_vec.push_back("sel1");
    ret_vec.push_back("sel2");
    ret_vec.push_back("sel3");
    ret_vec.push_back("sel4");
    ret_vec.push_back("sel5");
    ret_vec.push_back("sel6");
    ret_vec.push_back("sel7");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_cfg_global_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sw_rst");
    ret_vec.push_back("step_key");
    ret_vec.push_back("step_lkup");
    ret_vec.push_back("step_mpu");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_global.get_fields(level-1)) {
            ret_vec.push_back("cfg_global." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_table_profile_key.get_fields(level-1)) {
            ret_vec.push_back("cfg_table_profile_key." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_single_step.get_fields(level-1)) {
            ret_vec.push_back("dhs_single_step." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_phv_in_sop.get_fields(level-1)) {
            ret_vec.push_back("cnt_phv_in_sop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_phv_in_eop.get_fields(level-1)) {
            ret_vec.push_back("cnt_phv_in_eop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_phv_out_sop.get_fields(level-1)) {
            ret_vec.push_back("cnt_phv_out_sop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_phv_out_eop.get_fields(level-1)) {
            ret_vec.push_back("cnt_phv_out_eop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_axi_rdreq.get_fields(level-1)) {
            ret_vec.push_back("cnt_axi_rdreq." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_axi_rdrsp.get_fields(level-1)) {
            ret_vec.push_back("cnt_axi_rdrsp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tcam_req.get_fields(level-1)) {
            ret_vec.push_back("cnt_tcam_req." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tcam_rsp.get_fields(level-1)) {
            ret_vec.push_back("cnt_tcam_rsp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_debug_bus.get_fields(level-1)) {
            ret_vec.push_back("cfg_debug_bus." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_debug_bus.get_fields(level-1)) {
            ret_vec.push_back("sta_debug_bus." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_addr_target.get_fields(level-1)) {
            ret_vec.push_back("cfg_addr_target." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_read_wait_entry.get_fields(level-1)) {
            ret_vec.push_back("cfg_read_wait_entry." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_page_size.get_fields(level-1)) {
            ret_vec.push_back("cfg_page_size." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_timeout.get_fields(level-1)) {
            ret_vec.push_back("cfg_timeout." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_wait_entry.get_fields(level-1)) {
            ret_vec.push_back("sta_wait_entry." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_wait_fsm_states.get_fields(level-1)) {
            ret_vec.push_back("sta_wait_fsm_states." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_wait.get_fields(level-1)) {
            ret_vec.push_back("sta_wait." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_axi_err.get_fields(level-1)) {
            ret_vec.push_back("sta_axi_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_bad_axi_read.get_fields(level-1)) {
            ret_vec.push_back("sta_bad_axi_read." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_peek.get_fields(level-1)) {
            ret_vec.push_back("sta_peek." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_bist_done.get_fields(level-1)) {
            ret_vec.push_back("sta_bist_done." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_bist.get_fields(level-1)) {
            ret_vec.push_back("cfg_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_addr_align.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_addr_align." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : CNT_stall.get_fields(level-1)) {
            ret_vec.push_back("CNT_stall." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_table_profile_ctrl_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_table_profile_ctrl_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_err.get_fields(level-1)) {
            ret_vec.push_back("int_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_info.get_fields(level-1)) {
            ret_vec.push_back("int_info." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
