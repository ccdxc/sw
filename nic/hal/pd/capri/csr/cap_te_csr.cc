
#include "cap_te_csr.h"
#include "LogMsg.h"        
using namespace std;
        
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

cap_te_csr_sta_spare_t::cap_te_csr_sta_spare_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_spare_t::~cap_te_csr_sta_spare_t() { }

cap_te_csr_sta_wait_entry_t::cap_te_csr_sta_wait_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_wait_entry_t::~cap_te_csr_sta_wait_entry_t() { }

cap_te_csr_cfg_read_wait_entry_t::cap_te_csr_cfg_read_wait_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_read_wait_entry_t::~cap_te_csr_cfg_read_wait_entry_t() { }

cap_te_csr_cfg_spare_t::cap_te_csr_cfg_spare_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_spare_t::~cap_te_csr_cfg_spare_t() { }

cap_te_csr_sta_debug_bus_t::cap_te_csr_sta_debug_bus_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_sta_debug_bus_t::~cap_te_csr_sta_debug_bus_t() { }

cap_te_csr_cfg_lock_timeout_t::cap_te_csr_cfg_lock_timeout_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_lock_timeout_t::~cap_te_csr_cfg_lock_timeout_t() { }

cap_te_csr_cfg_debug_bus_t::cap_te_csr_cfg_debug_bus_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_debug_bus_t::~cap_te_csr_cfg_debug_bus_t() { }

cap_te_csr_cfg_cnt_debug_t::cap_te_csr_cfg_cnt_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cfg_cnt_debug_t::~cap_te_csr_cfg_cnt_debug_t() { }

cap_te_csr_cnt_debug_t::cap_te_csr_cnt_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_te_csr_cnt_debug_t::~cap_te_csr_cnt_debug_t() { }

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

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode0: 0x" << int_var__km_mode0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode1: 0x" << int_var__km_mode1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode2: 0x" << int_var__km_mode2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_mode3: 0x" << int_var__km_mode3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile0: 0x" << int_var__km_profile0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile1: 0x" << int_var__km_profile1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile2: 0x" << int_var__km_profile2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_profile3: 0x" << int_var__km_profile3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key0: 0x" << int_var__km_new_key0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key1: 0x" << int_var__km_new_key1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key2: 0x" << int_var__km_new_key2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".km_new_key3: 0x" << int_var__km_new_key3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lkup: 0x" << int_var__lkup << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tableid: 0x" << int_var__tableid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_sel: 0x" << int_var__hash_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_chain: 0x" << int_var__hash_chain << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_store: 0x" << int_var__hash_store << dec << endl)
}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::show() {

    for(int ii = 0; ii < 192; ii++) {
        entry[ii].show();
    }
}

void cap_te_csr_sta_spare_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl)
}

void cap_te_csr_sta_wait_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".cam: 0x" << int_var__cam << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".haz: 0x" << int_var__haz << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dat: 0x" << int_var__dat << dec << endl)
}

void cap_te_csr_cfg_read_wait_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".idx: 0x" << int_var__idx << dec << endl)
}

void cap_te_csr_cfg_spare_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl)
}

void cap_te_csr_sta_debug_bus_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".vec: 0x" << int_var__vec << dec << endl)
}

void cap_te_csr_cfg_lock_timeout_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl)
}

void cap_te_csr_cfg_debug_bus_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rdata_sel: 0x" << int_var__axi_rdata_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_key_sel: 0x" << int_var__tcam_key_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_key_sel: 0x" << int_var__hash_key_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl)
}

void cap_te_csr_cfg_cnt_debug_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl)
}

void cap_te_csr_cnt_debug_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_mpu_out_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_tcam_rsp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_tcam_req_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_axi_rdrsp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_axi_rdreq_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_phv_out_eop_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_phv_out_sop_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_phv_in_eop_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_cnt_phv_in_sop_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl)
}

void cap_te_csr_dhs_single_step_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lkup: 0x" << int_var__lkup << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu: 0x" << int_var__mpu << dec << endl)
}

void cap_te_csr_cfg_km_profile_bit_loc_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".bit_loc: 0x" << int_var__bit_loc << dec << endl)
}

void cap_te_csr_cfg_km_profile_bit_sel_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".bit_sel: 0x" << int_var__bit_sel << dec << endl)
}

void cap_te_csr_cfg_km_profile_byte_sel_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".byte_sel: 0x" << int_var__byte_sel << dec << endl)
}

void cap_te_csr_cfg_table_mpu_const_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl)
}

void cap_te_csr_cfg_table_property_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".axi: 0x" << int_var__axi << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".key_mask_hi: 0x" << int_var__key_mask_hi << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".key_mask_lo: 0x" << int_var__key_mask_lo << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fullkey_km_sel0: 0x" << int_var__fullkey_km_sel0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fullkey_km_sel1: 0x" << int_var__fullkey_km_sel1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lock_en: 0x" << int_var__lock_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tbl_entry_sz_raw: 0x" << int_var__tbl_entry_sz_raw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_dyn: 0x" << int_var__mpu_pc_dyn << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc: 0x" << int_var__mpu_pc << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_raw: 0x" << int_var__mpu_pc_raw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_ofst_err: 0x" << int_var__mpu_pc_ofst_err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_vec: 0x" << int_var__mpu_vec << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_base: 0x" << int_var__addr_base << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_vf_id_en: 0x" << int_var__addr_vf_id_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_vf_id_loc: 0x" << int_var__addr_vf_id_loc << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_shift: 0x" << int_var__addr_shift << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".chain_shift: 0x" << int_var__chain_shift << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_sz: 0x" << int_var__addr_sz << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lg2_entry_size: 0x" << int_var__lg2_entry_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".max_bypass_cnt: 0x" << int_var__max_bypass_cnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lock_en_raw: 0x" << int_var__lock_en_raw << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_lb: 0x" << int_var__mpu_lb << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_pc_loc: 0x" << int_var__mpu_pc_loc << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oflow_base_idx: 0x" << int_var__oflow_base_idx << dec << endl)
}

void cap_te_csr_cfg_table_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_results: 0x" << int_var__mpu_results << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".seq_base: 0x" << int_var__seq_base << dec << endl)
}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".adv_phv_flit: 0x" << int_var__adv_phv_flit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".done: 0x" << int_var__done << dec << endl)
}

void cap_te_csr_cfg_table_profile_cam_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mask: 0x" << int_var__mask << dec << endl)
}

void cap_te_csr_cfg_table_profile_key_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sel0: 0x" << int_var__sel0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel1: 0x" << int_var__sel1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel2: 0x" << int_var__sel2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel3: 0x" << int_var__sel3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel4: 0x" << int_var__sel4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel5: 0x" << int_var__sel5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel6: 0x" << int_var__sel6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sel7: 0x" << int_var__sel7 << dec << endl)
}

void cap_te_csr_cfg_global_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sw_rst: 0x" << int_var__sw_rst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".step_key: 0x" << int_var__step_key << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".step_lkup: 0x" << int_var__step_lkup << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".step_mpu: 0x" << int_var__step_mpu << dec << endl)
}

void cap_te_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_te_csr_t::show() {

    base.show();
    cfg_global.show();
    cfg_table_profile_key.show();
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile_cam[ii].show();
    }
    for(int ii = 0; ii < 192; ii++) {
        cfg_table_profile_ctrl_sram_ext[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_property[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_mpu_const[ii].show();
    }
    for(int ii = 0; ii < 256; ii++) {
        cfg_km_profile_byte_sel[ii].show();
    }
    for(int ii = 0; ii < 128; ii++) {
        cfg_km_profile_bit_sel[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_km_profile_bit_loc[ii].show();
    }
    dhs_single_step.show();
    cnt_phv_in_sop.show();
    cnt_phv_in_eop.show();
    cnt_phv_out_sop.show();
    cnt_phv_out_eop.show();
    cnt_axi_rdreq.show();
    cnt_axi_rdrsp.show();
    cnt_tcam_req.show();
    cnt_tcam_rsp.show();
    for(int ii = 0; ii < 4; ii++) {
        cnt_mpu_out[ii].show();
    }
    for(int ii = 0; ii < 4; ii++) {
        cnt_debug[ii].show();
    }
    for(int ii = 0; ii < 4; ii++) {
        cfg_cnt_debug[ii].show();
    }
    cfg_debug_bus.show();
    cfg_lock_timeout.show();
    sta_debug_bus.show();
    cfg_spare.show();
    cfg_read_wait_entry.show();
    sta_wait_entry.show();
    sta_spare.show();
    dhs_table_profile_ctrl_sram.show();
}

int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::get_width() const {
    return cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::s_get_width();

}

int cap_te_csr_dhs_table_profile_ctrl_sram_t::get_width() const {
    return cap_te_csr_dhs_table_profile_ctrl_sram_t::s_get_width();

}

int cap_te_csr_sta_spare_t::get_width() const {
    return cap_te_csr_sta_spare_t::s_get_width();

}

int cap_te_csr_sta_wait_entry_t::get_width() const {
    return cap_te_csr_sta_wait_entry_t::s_get_width();

}

int cap_te_csr_cfg_read_wait_entry_t::get_width() const {
    return cap_te_csr_cfg_read_wait_entry_t::s_get_width();

}

int cap_te_csr_cfg_spare_t::get_width() const {
    return cap_te_csr_cfg_spare_t::s_get_width();

}

int cap_te_csr_sta_debug_bus_t::get_width() const {
    return cap_te_csr_sta_debug_bus_t::s_get_width();

}

int cap_te_csr_cfg_lock_timeout_t::get_width() const {
    return cap_te_csr_cfg_lock_timeout_t::s_get_width();

}

int cap_te_csr_cfg_debug_bus_t::get_width() const {
    return cap_te_csr_cfg_debug_bus_t::s_get_width();

}

int cap_te_csr_cfg_cnt_debug_t::get_width() const {
    return cap_te_csr_cfg_cnt_debug_t::s_get_width();

}

int cap_te_csr_cnt_debug_t::get_width() const {
    return cap_te_csr_cnt_debug_t::s_get_width();

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

int cap_te_csr_sta_spare_t::s_get_width() {
    int _count = 0;

    _count += 128; // spare
    return _count;
}

int cap_te_csr_sta_wait_entry_t::s_get_width() {
    int _count = 0;

    _count += 579; // key
    _count += 101; // cam
    _count += 17; // haz
    _count += 512; // dat
    return _count;
}

int cap_te_csr_cfg_read_wait_entry_t::s_get_width() {
    int _count = 0;

    _count += 4; // idx
    return _count;
}

int cap_te_csr_cfg_spare_t::s_get_width() {
    int _count = 0;

    _count += 64; // spare
    return _count;
}

int cap_te_csr_sta_debug_bus_t::s_get_width() {
    int _count = 0;

    _count += 256; // vec
    return _count;
}

int cap_te_csr_cfg_lock_timeout_t::s_get_width() {
    int _count = 0;

    _count += 16; // val
    _count += 1; // en
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

int cap_te_csr_cfg_cnt_debug_t::s_get_width() {
    int _count = 0;

    _count += 4; // sel
    return _count;
}

int cap_te_csr_cnt_debug_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
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
    _count += 5; // chain_shift
    _count += 7; // addr_sz
    _count += 3; // lg2_entry_size
    _count += 10; // max_bypass_cnt
    _count += 1; // lock_en_raw
    _count += 1; // mpu_lb
    _count += 6; // mpu_pc_loc
    _count += 20; // oflow_base_idx
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

    _count += 8; // sel0
    _count += 8; // sel1
    _count += 8; // sel2
    _count += 8; // sel3
    _count += 8; // sel4
    _count += 8; // sel5
    _count += 8; // sel6
    _count += 8; // sel7
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
    _count += (cap_te_csr_cnt_debug_t::s_get_width() * 4); // cnt_debug
    _count += (cap_te_csr_cfg_cnt_debug_t::s_get_width() * 4); // cfg_cnt_debug
    _count += cap_te_csr_cfg_debug_bus_t::s_get_width(); // cfg_debug_bus
    _count += cap_te_csr_cfg_lock_timeout_t::s_get_width(); // cfg_lock_timeout
    _count += cap_te_csr_sta_debug_bus_t::s_get_width(); // sta_debug_bus
    _count += cap_te_csr_cfg_spare_t::s_get_width(); // cfg_spare
    _count += cap_te_csr_cfg_read_wait_entry_t::s_get_width(); // cfg_read_wait_entry
    _count += cap_te_csr_sta_wait_entry_t::s_get_width(); // sta_wait_entry
    _count += cap_te_csr_sta_spare_t::s_get_width(); // sta_spare
    _count += cap_te_csr_dhs_table_profile_ctrl_sram_t::s_get_width(); // dhs_table_profile_ctrl_sram
    return _count;
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // km_mode0
    int_var__km_mode0 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< km_mode0_cpp_int_t >()  ;
    _count += 2;
    // km_mode1
    int_var__km_mode1 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< km_mode1_cpp_int_t >()  ;
    _count += 2;
    // km_mode2
    int_var__km_mode2 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< km_mode2_cpp_int_t >()  ;
    _count += 2;
    // km_mode3
    int_var__km_mode3 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< km_mode3_cpp_int_t >()  ;
    _count += 2;
    // km_profile0
    int_var__km_profile0 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< km_profile0_cpp_int_t >()  ;
    _count += 3;
    // km_profile1
    int_var__km_profile1 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< km_profile1_cpp_int_t >()  ;
    _count += 3;
    // km_profile2
    int_var__km_profile2 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< km_profile2_cpp_int_t >()  ;
    _count += 3;
    // km_profile3
    int_var__km_profile3 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< km_profile3_cpp_int_t >()  ;
    _count += 3;
    // km_new_key0
    int_var__km_new_key0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< km_new_key0_cpp_int_t >()  ;
    _count += 1;
    // km_new_key1
    int_var__km_new_key1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< km_new_key1_cpp_int_t >()  ;
    _count += 1;
    // km_new_key2
    int_var__km_new_key2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< km_new_key2_cpp_int_t >()  ;
    _count += 1;
    // km_new_key3
    int_var__km_new_key3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< km_new_key3_cpp_int_t >()  ;
    _count += 1;
    // lkup
    int_var__lkup = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< lkup_cpp_int_t >()  ;
    _count += 3;
    // tableid
    int_var__tableid = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< tableid_cpp_int_t >()  ;
    _count += 4;
    // hash_sel
    int_var__hash_sel = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< hash_sel_cpp_int_t >()  ;
    _count += 3;
    // hash_chain
    int_var__hash_chain = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< hash_chain_cpp_int_t >()  ;
    _count += 2;
    // hash_store
    int_var__hash_store = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< hash_store_cpp_int_t >()  ;
    _count += 2;
}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 192; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_te_csr_sta_spare_t::all(const cpp_int & _val) {
    int _count = 0;

    // spare
    int_var__spare = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< spare_cpp_int_t >()  ;
    _count += 128;
}

void cap_te_csr_sta_wait_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // key
    int_var__key = hlp.get_slc(_val, _count, _count -1 + 579 ).convert_to< key_cpp_int_t >()  ;
    _count += 579;
    // cam
    int_var__cam = hlp.get_slc(_val, _count, _count -1 + 101 ).convert_to< cam_cpp_int_t >()  ;
    _count += 101;
    // haz
    int_var__haz = hlp.get_slc(_val, _count, _count -1 + 17 ).convert_to< haz_cpp_int_t >()  ;
    _count += 17;
    // dat
    int_var__dat = hlp.get_slc(_val, _count, _count -1 + 512 ).convert_to< dat_cpp_int_t >()  ;
    _count += 512;
}

void cap_te_csr_cfg_read_wait_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // idx
    int_var__idx = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< idx_cpp_int_t >()  ;
    _count += 4;
}

void cap_te_csr_cfg_spare_t::all(const cpp_int & _val) {
    int _count = 0;

    // spare
    int_var__spare = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< spare_cpp_int_t >()  ;
    _count += 64;
}

void cap_te_csr_sta_debug_bus_t::all(const cpp_int & _val) {
    int _count = 0;

    // vec
    int_var__vec = hlp.get_slc(_val, _count, _count -1 + 256 ).convert_to< vec_cpp_int_t >()  ;
    _count += 256;
}

void cap_te_csr_cfg_lock_timeout_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< val_cpp_int_t >()  ;
    _count += 16;
    // en
    int_var__en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< en_cpp_int_t >()  ;
    _count += 1;
}

void cap_te_csr_cfg_debug_bus_t::all(const cpp_int & _val) {
    int _count = 0;

    // axi_rdata_sel
    int_var__axi_rdata_sel = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< axi_rdata_sel_cpp_int_t >()  ;
    _count += 3;
    // tcam_key_sel
    int_var__tcam_key_sel = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< tcam_key_sel_cpp_int_t >()  ;
    _count += 3;
    // hash_key_sel
    int_var__hash_key_sel = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< hash_key_sel_cpp_int_t >()  ;
    _count += 3;
    // sel
    int_var__sel = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< sel_cpp_int_t >()  ;
    _count += 4;
}

void cap_te_csr_cfg_cnt_debug_t::all(const cpp_int & _val) {
    int _count = 0;

    // sel
    int_var__sel = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< sel_cpp_int_t >()  ;
    _count += 4;
}

void cap_te_csr_cnt_debug_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_mpu_out_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_tcam_rsp_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_tcam_req_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_axi_rdrsp_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_axi_rdreq_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_phv_out_eop_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_phv_out_sop_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_phv_in_eop_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_cnt_phv_in_sop_t::all(const cpp_int & _val) {
    int _count = 0;

    // val
    int_var__val = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< val_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_dhs_single_step_t::all(const cpp_int & _val) {
    int _count = 0;

    // key
    int_var__key = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< key_cpp_int_t >()  ;
    _count += 1;
    // lkup
    int_var__lkup = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lkup_cpp_int_t >()  ;
    _count += 1;
    // mpu
    int_var__mpu = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mpu_cpp_int_t >()  ;
    _count += 1;
}

void cap_te_csr_cfg_km_profile_bit_loc_t::all(const cpp_int & _val) {
    int _count = 0;

    // valid
    int_var__valid = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< valid_cpp_int_t >()  ;
    _count += 1;
    // bit_loc
    int_var__bit_loc = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< bit_loc_cpp_int_t >()  ;
    _count += 5;
}

void cap_te_csr_cfg_km_profile_bit_sel_t::all(const cpp_int & _val) {
    int _count = 0;

    // bit_sel
    int_var__bit_sel = hlp.get_slc(_val, _count, _count -1 + 13 ).convert_to< bit_sel_cpp_int_t >()  ;
    _count += 13;
}

void cap_te_csr_cfg_km_profile_byte_sel_t::all(const cpp_int & _val) {
    int _count = 0;

    // byte_sel
    int_var__byte_sel = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< byte_sel_cpp_int_t >()  ;
    _count += 10;
}

void cap_te_csr_cfg_table_mpu_const_t::all(const cpp_int & _val) {
    int _count = 0;

    // value
    int_var__value = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< value_cpp_int_t >()  ;
    _count += 64;
}

void cap_te_csr_cfg_table_property_t::all(const cpp_int & _val) {
    int _count = 0;

    // axi
    int_var__axi = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< axi_cpp_int_t >()  ;
    _count += 1;
    // key_mask_hi
    int_var__key_mask_hi = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< key_mask_hi_cpp_int_t >()  ;
    _count += 10;
    // key_mask_lo
    int_var__key_mask_lo = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< key_mask_lo_cpp_int_t >()  ;
    _count += 9;
    // fullkey_km_sel0
    int_var__fullkey_km_sel0 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fullkey_km_sel0_cpp_int_t >()  ;
    _count += 2;
    // fullkey_km_sel1
    int_var__fullkey_km_sel1 = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< fullkey_km_sel1_cpp_int_t >()  ;
    _count += 2;
    // lock_en
    int_var__lock_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lock_en_cpp_int_t >()  ;
    _count += 1;
    // tbl_entry_sz_raw
    int_var__tbl_entry_sz_raw = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< tbl_entry_sz_raw_cpp_int_t >()  ;
    _count += 1;
    // mpu_pc_dyn
    int_var__mpu_pc_dyn = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mpu_pc_dyn_cpp_int_t >()  ;
    _count += 1;
    // mpu_pc
    int_var__mpu_pc = hlp.get_slc(_val, _count, _count -1 + 28 ).convert_to< mpu_pc_cpp_int_t >()  ;
    _count += 28;
    // mpu_pc_raw
    int_var__mpu_pc_raw = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mpu_pc_raw_cpp_int_t >()  ;
    _count += 1;
    // mpu_pc_ofst_err
    int_var__mpu_pc_ofst_err = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< mpu_pc_ofst_err_cpp_int_t >()  ;
    _count += 9;
    // mpu_vec
    int_var__mpu_vec = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< mpu_vec_cpp_int_t >()  ;
    _count += 4;
    // addr_base
    int_var__addr_base = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< addr_base_cpp_int_t >()  ;
    _count += 64;
    // addr_vf_id_en
    int_var__addr_vf_id_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< addr_vf_id_en_cpp_int_t >()  ;
    _count += 1;
    // addr_vf_id_loc
    int_var__addr_vf_id_loc = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< addr_vf_id_loc_cpp_int_t >()  ;
    _count += 6;
    // addr_shift
    int_var__addr_shift = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< addr_shift_cpp_int_t >()  ;
    _count += 5;
    // chain_shift
    int_var__chain_shift = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< chain_shift_cpp_int_t >()  ;
    _count += 5;
    // addr_sz
    int_var__addr_sz = hlp.get_slc(_val, _count, _count -1 + 7 ).convert_to< addr_sz_cpp_int_t >()  ;
    _count += 7;
    // lg2_entry_size
    int_var__lg2_entry_size = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< lg2_entry_size_cpp_int_t >()  ;
    _count += 3;
    // max_bypass_cnt
    int_var__max_bypass_cnt = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< max_bypass_cnt_cpp_int_t >()  ;
    _count += 10;
    // lock_en_raw
    int_var__lock_en_raw = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lock_en_raw_cpp_int_t >()  ;
    _count += 1;
    // mpu_lb
    int_var__mpu_lb = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mpu_lb_cpp_int_t >()  ;
    _count += 1;
    // mpu_pc_loc
    int_var__mpu_pc_loc = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< mpu_pc_loc_cpp_int_t >()  ;
    _count += 6;
    // oflow_base_idx
    int_var__oflow_base_idx = hlp.get_slc(_val, _count, _count -1 + 20 ).convert_to< oflow_base_idx_cpp_int_t >()  ;
    _count += 20;
}

void cap_te_csr_cfg_table_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // mpu_results
    int_var__mpu_results = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< mpu_results_cpp_int_t >()  ;
    _count += 4;
    // seq_base
    int_var__seq_base = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< seq_base_cpp_int_t >()  ;
    _count += 8;
}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::all(const cpp_int & _val) {
    int _count = 0;

    // adv_phv_flit
    int_var__adv_phv_flit = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< adv_phv_flit_cpp_int_t >()  ;
    _count += 1;
    // done
    int_var__done = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< done_cpp_int_t >()  ;
    _count += 1;
}

void cap_te_csr_cfg_table_profile_cam_t::all(const cpp_int & _val) {
    int _count = 0;

    // valid
    int_var__valid = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< valid_cpp_int_t >()  ;
    _count += 1;
    // value
    int_var__value = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< value_cpp_int_t >()  ;
    _count += 8;
    // mask
    int_var__mask = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< mask_cpp_int_t >()  ;
    _count += 8;
}

void cap_te_csr_cfg_table_profile_key_t::all(const cpp_int & _val) {
    int _count = 0;

    // sel0
    int_var__sel0 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel0_cpp_int_t >()  ;
    _count += 8;
    // sel1
    int_var__sel1 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel1_cpp_int_t >()  ;
    _count += 8;
    // sel2
    int_var__sel2 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel2_cpp_int_t >()  ;
    _count += 8;
    // sel3
    int_var__sel3 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel3_cpp_int_t >()  ;
    _count += 8;
    // sel4
    int_var__sel4 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel4_cpp_int_t >()  ;
    _count += 8;
    // sel5
    int_var__sel5 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel5_cpp_int_t >()  ;
    _count += 8;
    // sel6
    int_var__sel6 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel6_cpp_int_t >()  ;
    _count += 8;
    // sel7
    int_var__sel7 = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< sel7_cpp_int_t >()  ;
    _count += 8;
}

void cap_te_csr_cfg_global_t::all(const cpp_int & _val) {
    int _count = 0;

    // sw_rst
    int_var__sw_rst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< sw_rst_cpp_int_t >()  ;
    _count += 1;
    // step_key
    int_var__step_key = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< step_key_cpp_int_t >()  ;
    _count += 1;
    // step_lkup
    int_var__step_lkup = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< step_lkup_cpp_int_t >()  ;
    _count += 1;
    // step_mpu
    int_var__step_mpu = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< step_mpu_cpp_int_t >()  ;
    _count += 1;
}

void cap_te_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_te_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_global.all( hlp.get_slc(_val, _count, _count -1 + cfg_global.get_width() )); // cfg_global
    _count += cfg_global.get_width();
    cfg_table_profile_key.all( hlp.get_slc(_val, _count, _count -1 + cfg_table_profile_key.get_width() )); // cfg_table_profile_key
    _count += cfg_table_profile_key.get_width();
    // cfg_table_profile_cam
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile_cam[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_profile_cam[ii].get_width()));
        _count += cfg_table_profile_cam[ii].get_width();
    }
    // cfg_table_profile_ctrl_sram_ext
    for(int ii = 0; ii < 192; ii++) {
        cfg_table_profile_ctrl_sram_ext[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_profile_ctrl_sram_ext[ii].get_width()));
        _count += cfg_table_profile_ctrl_sram_ext[ii].get_width();
    }
    // cfg_table_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_profile[ii].get_width()));
        _count += cfg_table_profile[ii].get_width();
    }
    // cfg_table_property
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_property[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_property[ii].get_width()));
        _count += cfg_table_property[ii].get_width();
    }
    // cfg_table_mpu_const
    for(int ii = 0; ii < 16; ii++) {
        cfg_table_mpu_const[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_mpu_const[ii].get_width()));
        _count += cfg_table_mpu_const[ii].get_width();
    }
    // cfg_km_profile_byte_sel
    for(int ii = 0; ii < 256; ii++) {
        cfg_km_profile_byte_sel[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_km_profile_byte_sel[ii].get_width()));
        _count += cfg_km_profile_byte_sel[ii].get_width();
    }
    // cfg_km_profile_bit_sel
    for(int ii = 0; ii < 128; ii++) {
        cfg_km_profile_bit_sel[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_km_profile_bit_sel[ii].get_width()));
        _count += cfg_km_profile_bit_sel[ii].get_width();
    }
    // cfg_km_profile_bit_loc
    for(int ii = 0; ii < 16; ii++) {
        cfg_km_profile_bit_loc[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_km_profile_bit_loc[ii].get_width()));
        _count += cfg_km_profile_bit_loc[ii].get_width();
    }
    dhs_single_step.all( hlp.get_slc(_val, _count, _count -1 + dhs_single_step.get_width() )); // dhs_single_step
    _count += dhs_single_step.get_width();
    cnt_phv_in_sop.all( hlp.get_slc(_val, _count, _count -1 + cnt_phv_in_sop.get_width() )); // cnt_phv_in_sop
    _count += cnt_phv_in_sop.get_width();
    cnt_phv_in_eop.all( hlp.get_slc(_val, _count, _count -1 + cnt_phv_in_eop.get_width() )); // cnt_phv_in_eop
    _count += cnt_phv_in_eop.get_width();
    cnt_phv_out_sop.all( hlp.get_slc(_val, _count, _count -1 + cnt_phv_out_sop.get_width() )); // cnt_phv_out_sop
    _count += cnt_phv_out_sop.get_width();
    cnt_phv_out_eop.all( hlp.get_slc(_val, _count, _count -1 + cnt_phv_out_eop.get_width() )); // cnt_phv_out_eop
    _count += cnt_phv_out_eop.get_width();
    cnt_axi_rdreq.all( hlp.get_slc(_val, _count, _count -1 + cnt_axi_rdreq.get_width() )); // cnt_axi_rdreq
    _count += cnt_axi_rdreq.get_width();
    cnt_axi_rdrsp.all( hlp.get_slc(_val, _count, _count -1 + cnt_axi_rdrsp.get_width() )); // cnt_axi_rdrsp
    _count += cnt_axi_rdrsp.get_width();
    cnt_tcam_req.all( hlp.get_slc(_val, _count, _count -1 + cnt_tcam_req.get_width() )); // cnt_tcam_req
    _count += cnt_tcam_req.get_width();
    cnt_tcam_rsp.all( hlp.get_slc(_val, _count, _count -1 + cnt_tcam_rsp.get_width() )); // cnt_tcam_rsp
    _count += cnt_tcam_rsp.get_width();
    // cnt_mpu_out
    for(int ii = 0; ii < 4; ii++) {
        cnt_mpu_out[ii].all( hlp.get_slc(_val, _count, _count -1 + cnt_mpu_out[ii].get_width()));
        _count += cnt_mpu_out[ii].get_width();
    }
    // cnt_debug
    for(int ii = 0; ii < 4; ii++) {
        cnt_debug[ii].all( hlp.get_slc(_val, _count, _count -1 + cnt_debug[ii].get_width()));
        _count += cnt_debug[ii].get_width();
    }
    // cfg_cnt_debug
    for(int ii = 0; ii < 4; ii++) {
        cfg_cnt_debug[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_cnt_debug[ii].get_width()));
        _count += cfg_cnt_debug[ii].get_width();
    }
    cfg_debug_bus.all( hlp.get_slc(_val, _count, _count -1 + cfg_debug_bus.get_width() )); // cfg_debug_bus
    _count += cfg_debug_bus.get_width();
    cfg_lock_timeout.all( hlp.get_slc(_val, _count, _count -1 + cfg_lock_timeout.get_width() )); // cfg_lock_timeout
    _count += cfg_lock_timeout.get_width();
    sta_debug_bus.all( hlp.get_slc(_val, _count, _count -1 + sta_debug_bus.get_width() )); // sta_debug_bus
    _count += sta_debug_bus.get_width();
    cfg_spare.all( hlp.get_slc(_val, _count, _count -1 + cfg_spare.get_width() )); // cfg_spare
    _count += cfg_spare.get_width();
    cfg_read_wait_entry.all( hlp.get_slc(_val, _count, _count -1 + cfg_read_wait_entry.get_width() )); // cfg_read_wait_entry
    _count += cfg_read_wait_entry.get_width();
    sta_wait_entry.all( hlp.get_slc(_val, _count, _count -1 + sta_wait_entry.get_width() )); // sta_wait_entry
    _count += sta_wait_entry.get_width();
    sta_spare.all( hlp.get_slc(_val, _count, _count -1 + sta_spare.get_width() )); // sta_spare
    _count += sta_spare.get_width();
    dhs_table_profile_ctrl_sram.all( hlp.get_slc(_val, _count, _count -1 + dhs_table_profile_ctrl_sram.get_width() )); // dhs_table_profile_ctrl_sram
    _count += dhs_table_profile_ctrl_sram.get_width();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // km_mode0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_mode0) , _count, _count -1 + 2 );
    _count += 2;
    // km_mode1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_mode1) , _count, _count -1 + 2 );
    _count += 2;
    // km_mode2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_mode2) , _count, _count -1 + 2 );
    _count += 2;
    // km_mode3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_mode3) , _count, _count -1 + 2 );
    _count += 2;
    // km_profile0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_profile0) , _count, _count -1 + 3 );
    _count += 3;
    // km_profile1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_profile1) , _count, _count -1 + 3 );
    _count += 3;
    // km_profile2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_profile2) , _count, _count -1 + 3 );
    _count += 3;
    // km_profile3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_profile3) , _count, _count -1 + 3 );
    _count += 3;
    // km_new_key0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_new_key0) , _count, _count -1 + 1 );
    _count += 1;
    // km_new_key1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_new_key1) , _count, _count -1 + 1 );
    _count += 1;
    // km_new_key2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_new_key2) , _count, _count -1 + 1 );
    _count += 1;
    // km_new_key3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__km_new_key3) , _count, _count -1 + 1 );
    _count += 1;
    // lkup
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lkup) , _count, _count -1 + 3 );
    _count += 3;
    // tableid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tableid) , _count, _count -1 + 4 );
    _count += 4;
    // hash_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hash_sel) , _count, _count -1 + 3 );
    _count += 3;
    // hash_chain
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hash_chain) , _count, _count -1 + 2 );
    _count += 2;
    // hash_store
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hash_store) , _count, _count -1 + 2 );
    _count += 2;
    return ret_val;
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 192; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_te_csr_sta_spare_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // spare
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__spare) , _count, _count -1 + 128 );
    _count += 128;
    return ret_val;
}

cpp_int cap_te_csr_sta_wait_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // key
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__key) , _count, _count -1 + 579 );
    _count += 579;
    // cam
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cam) , _count, _count -1 + 101 );
    _count += 101;
    // haz
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__haz) , _count, _count -1 + 17 );
    _count += 17;
    // dat
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dat) , _count, _count -1 + 512 );
    _count += 512;
    return ret_val;
}

cpp_int cap_te_csr_cfg_read_wait_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // idx
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__idx) , _count, _count -1 + 4 );
    _count += 4;
    return ret_val;
}

cpp_int cap_te_csr_cfg_spare_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // spare
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__spare) , _count, _count -1 + 64 );
    _count += 64;
    return ret_val;
}

cpp_int cap_te_csr_sta_debug_bus_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // vec
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__vec) , _count, _count -1 + 256 );
    _count += 256;
    return ret_val;
}

cpp_int cap_te_csr_cfg_lock_timeout_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 16 );
    _count += 16;
    // en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__en) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_te_csr_cfg_debug_bus_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // axi_rdata_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__axi_rdata_sel) , _count, _count -1 + 3 );
    _count += 3;
    // tcam_key_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tcam_key_sel) , _count, _count -1 + 3 );
    _count += 3;
    // hash_key_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hash_key_sel) , _count, _count -1 + 3 );
    _count += 3;
    // sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel) , _count, _count -1 + 4 );
    _count += 4;
    return ret_val;
}

cpp_int cap_te_csr_cfg_cnt_debug_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel) , _count, _count -1 + 4 );
    _count += 4;
    return ret_val;
}

cpp_int cap_te_csr_cnt_debug_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_mpu_out_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_tcam_rsp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_tcam_req_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_axi_rdrsp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_axi_rdreq_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_out_eop_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_out_sop_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_in_eop_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_cnt_phv_in_sop_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__val) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_dhs_single_step_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // key
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__key) , _count, _count -1 + 1 );
    _count += 1;
    // lkup
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lkup) , _count, _count -1 + 1 );
    _count += 1;
    // mpu
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // valid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__valid) , _count, _count -1 + 1 );
    _count += 1;
    // bit_loc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__bit_loc) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_bit_sel_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // bit_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__bit_sel) , _count, _count -1 + 13 );
    _count += 13;
    return ret_val;
}

cpp_int cap_te_csr_cfg_km_profile_byte_sel_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // byte_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__byte_sel) , _count, _count -1 + 10 );
    _count += 10;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_mpu_const_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // value
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__value) , _count, _count -1 + 64 );
    _count += 64;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_property_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // axi
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__axi) , _count, _count -1 + 1 );
    _count += 1;
    // key_mask_hi
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__key_mask_hi) , _count, _count -1 + 10 );
    _count += 10;
    // key_mask_lo
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__key_mask_lo) , _count, _count -1 + 9 );
    _count += 9;
    // fullkey_km_sel0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fullkey_km_sel0) , _count, _count -1 + 2 );
    _count += 2;
    // fullkey_km_sel1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fullkey_km_sel1) , _count, _count -1 + 2 );
    _count += 2;
    // lock_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lock_en) , _count, _count -1 + 1 );
    _count += 1;
    // tbl_entry_sz_raw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tbl_entry_sz_raw) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_pc_dyn
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_pc_dyn) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_pc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_pc) , _count, _count -1 + 28 );
    _count += 28;
    // mpu_pc_raw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_pc_raw) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_pc_ofst_err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_pc_ofst_err) , _count, _count -1 + 9 );
    _count += 9;
    // mpu_vec
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_vec) , _count, _count -1 + 4 );
    _count += 4;
    // addr_base
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_base) , _count, _count -1 + 64 );
    _count += 64;
    // addr_vf_id_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_vf_id_en) , _count, _count -1 + 1 );
    _count += 1;
    // addr_vf_id_loc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_vf_id_loc) , _count, _count -1 + 6 );
    _count += 6;
    // addr_shift
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_shift) , _count, _count -1 + 5 );
    _count += 5;
    // chain_shift
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__chain_shift) , _count, _count -1 + 5 );
    _count += 5;
    // addr_sz
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_sz) , _count, _count -1 + 7 );
    _count += 7;
    // lg2_entry_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lg2_entry_size) , _count, _count -1 + 3 );
    _count += 3;
    // max_bypass_cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_bypass_cnt) , _count, _count -1 + 10 );
    _count += 10;
    // lock_en_raw
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lock_en_raw) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_lb
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_lb) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_pc_loc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_pc_loc) , _count, _count -1 + 6 );
    _count += 6;
    // oflow_base_idx
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oflow_base_idx) , _count, _count -1 + 20 );
    _count += 20;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // mpu_results
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_results) , _count, _count -1 + 4 );
    _count += 4;
    // seq_base
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__seq_base) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // adv_phv_flit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__adv_phv_flit) , _count, _count -1 + 1 );
    _count += 1;
    // done
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__done) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // valid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__valid) , _count, _count -1 + 1 );
    _count += 1;
    // value
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__value) , _count, _count -1 + 8 );
    _count += 8;
    // mask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mask) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_te_csr_cfg_table_profile_key_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sel0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel0) , _count, _count -1 + 8 );
    _count += 8;
    // sel1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel1) , _count, _count -1 + 8 );
    _count += 8;
    // sel2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel2) , _count, _count -1 + 8 );
    _count += 8;
    // sel3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel3) , _count, _count -1 + 8 );
    _count += 8;
    // sel4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel4) , _count, _count -1 + 8 );
    _count += 8;
    // sel5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel5) , _count, _count -1 + 8 );
    _count += 8;
    // sel6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel6) , _count, _count -1 + 8 );
    _count += 8;
    // sel7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sel7) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_te_csr_cfg_global_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sw_rst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sw_rst) , _count, _count -1 + 1 );
    _count += 1;
    // step_key
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__step_key) , _count, _count -1 + 1 );
    _count += 1;
    // step_lkup
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__step_lkup) , _count, _count -1 + 1 );
    _count += 1;
    // step_mpu
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__step_mpu) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_te_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_te_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_global.all() , _count, _count -1 + cfg_global.get_width() ); // cfg_global
    _count += cfg_global.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_table_profile_key.all() , _count, _count -1 + cfg_table_profile_key.get_width() ); // cfg_table_profile_key
    _count += cfg_table_profile_key.get_width();
    // cfg_table_profile_cam
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_profile_cam[ii].all() , _count, _count -1 + cfg_table_profile_cam[ii].get_width() );
        _count += cfg_table_profile_cam[ii].get_width();
    }
    // cfg_table_profile_ctrl_sram_ext
    for(int ii = 0; ii < 192; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_profile_ctrl_sram_ext[ii].all() , _count, _count -1 + cfg_table_profile_ctrl_sram_ext[ii].get_width() );
        _count += cfg_table_profile_ctrl_sram_ext[ii].get_width();
    }
    // cfg_table_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_profile[ii].all() , _count, _count -1 + cfg_table_profile[ii].get_width() );
        _count += cfg_table_profile[ii].get_width();
    }
    // cfg_table_property
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_property[ii].all() , _count, _count -1 + cfg_table_property[ii].get_width() );
        _count += cfg_table_property[ii].get_width();
    }
    // cfg_table_mpu_const
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_mpu_const[ii].all() , _count, _count -1 + cfg_table_mpu_const[ii].get_width() );
        _count += cfg_table_mpu_const[ii].get_width();
    }
    // cfg_km_profile_byte_sel
    for(int ii = 0; ii < 256; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_km_profile_byte_sel[ii].all() , _count, _count -1 + cfg_km_profile_byte_sel[ii].get_width() );
        _count += cfg_km_profile_byte_sel[ii].get_width();
    }
    // cfg_km_profile_bit_sel
    for(int ii = 0; ii < 128; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_km_profile_bit_sel[ii].all() , _count, _count -1 + cfg_km_profile_bit_sel[ii].get_width() );
        _count += cfg_km_profile_bit_sel[ii].get_width();
    }
    // cfg_km_profile_bit_loc
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_km_profile_bit_loc[ii].all() , _count, _count -1 + cfg_km_profile_bit_loc[ii].get_width() );
        _count += cfg_km_profile_bit_loc[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, dhs_single_step.all() , _count, _count -1 + dhs_single_step.get_width() ); // dhs_single_step
    _count += dhs_single_step.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_phv_in_sop.all() , _count, _count -1 + cnt_phv_in_sop.get_width() ); // cnt_phv_in_sop
    _count += cnt_phv_in_sop.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_phv_in_eop.all() , _count, _count -1 + cnt_phv_in_eop.get_width() ); // cnt_phv_in_eop
    _count += cnt_phv_in_eop.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_phv_out_sop.all() , _count, _count -1 + cnt_phv_out_sop.get_width() ); // cnt_phv_out_sop
    _count += cnt_phv_out_sop.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_phv_out_eop.all() , _count, _count -1 + cnt_phv_out_eop.get_width() ); // cnt_phv_out_eop
    _count += cnt_phv_out_eop.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_axi_rdreq.all() , _count, _count -1 + cnt_axi_rdreq.get_width() ); // cnt_axi_rdreq
    _count += cnt_axi_rdreq.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_axi_rdrsp.all() , _count, _count -1 + cnt_axi_rdrsp.get_width() ); // cnt_axi_rdrsp
    _count += cnt_axi_rdrsp.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_tcam_req.all() , _count, _count -1 + cnt_tcam_req.get_width() ); // cnt_tcam_req
    _count += cnt_tcam_req.get_width();
    ret_val = hlp.set_slc(ret_val, cnt_tcam_rsp.all() , _count, _count -1 + cnt_tcam_rsp.get_width() ); // cnt_tcam_rsp
    _count += cnt_tcam_rsp.get_width();
    // cnt_mpu_out
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, cnt_mpu_out[ii].all() , _count, _count -1 + cnt_mpu_out[ii].get_width() );
        _count += cnt_mpu_out[ii].get_width();
    }
    // cnt_debug
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, cnt_debug[ii].all() , _count, _count -1 + cnt_debug[ii].get_width() );
        _count += cnt_debug[ii].get_width();
    }
    // cfg_cnt_debug
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_cnt_debug[ii].all() , _count, _count -1 + cfg_cnt_debug[ii].get_width() );
        _count += cfg_cnt_debug[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, cfg_debug_bus.all() , _count, _count -1 + cfg_debug_bus.get_width() ); // cfg_debug_bus
    _count += cfg_debug_bus.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_lock_timeout.all() , _count, _count -1 + cfg_lock_timeout.get_width() ); // cfg_lock_timeout
    _count += cfg_lock_timeout.get_width();
    ret_val = hlp.set_slc(ret_val, sta_debug_bus.all() , _count, _count -1 + sta_debug_bus.get_width() ); // sta_debug_bus
    _count += sta_debug_bus.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_spare.all() , _count, _count -1 + cfg_spare.get_width() ); // cfg_spare
    _count += cfg_spare.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_read_wait_entry.all() , _count, _count -1 + cfg_read_wait_entry.get_width() ); // cfg_read_wait_entry
    _count += cfg_read_wait_entry.get_width();
    ret_val = hlp.set_slc(ret_val, sta_wait_entry.all() , _count, _count -1 + sta_wait_entry.get_width() ); // sta_wait_entry
    _count += sta_wait_entry.get_width();
    ret_val = hlp.set_slc(ret_val, sta_spare.all() , _count, _count -1 + sta_spare.get_width() ); // sta_spare
    _count += sta_spare.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_table_profile_ctrl_sram.all() , _count, _count -1 + dhs_table_profile_ctrl_sram.get_width() ); // dhs_table_profile_ctrl_sram
    _count += dhs_table_profile_ctrl_sram.get_width();
    return ret_val;
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_mode0", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0);
            register_get_func("km_mode0", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_mode1", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1);
            register_get_func("km_mode1", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_mode2", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2);
            register_get_func("km_mode2", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_mode3", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3);
            register_get_func("km_mode3", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_profile0", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0);
            register_get_func("km_profile0", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_profile1", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1);
            register_get_func("km_profile1", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_profile2", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2);
            register_get_func("km_profile2", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_profile3", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3);
            register_get_func("km_profile3", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_new_key0", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0);
            register_get_func("km_new_key0", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_new_key1", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1);
            register_get_func("km_new_key1", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_new_key2", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2);
            register_get_func("km_new_key2", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("km_new_key3", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3);
            register_get_func("km_new_key3", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lkup", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup);
            register_get_func("lkup", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tableid", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid);
            register_get_func("tableid", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hash_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel);
            register_get_func("hash_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hash_chain", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain);
            register_get_func("hash_chain", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hash_store", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store);
            register_get_func("hash_store", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store);
        }
        #endif
    
}

void cap_te_csr_dhs_table_profile_ctrl_sram_t::init() {

    for(int ii = 0; ii < 192; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_te_csr_sta_spare_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("spare", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_spare_t::spare);
            register_get_func("spare", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_spare_t::spare);
        }
        #endif
    
}

void cap_te_csr_sta_wait_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("key", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_wait_entry_t::key);
            register_get_func("key", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_wait_entry_t::key);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cam", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_wait_entry_t::cam);
            register_get_func("cam", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_wait_entry_t::cam);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("haz", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_wait_entry_t::haz);
            register_get_func("haz", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_wait_entry_t::haz);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dat", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_wait_entry_t::dat);
            register_get_func("dat", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_wait_entry_t::dat);
        }
        #endif
    
}

void cap_te_csr_cfg_read_wait_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("idx", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_read_wait_entry_t::idx);
            register_get_func("idx", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_read_wait_entry_t::idx);
        }
        #endif
    
}

void cap_te_csr_cfg_spare_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("spare", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_spare_t::spare);
            register_get_func("spare", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_spare_t::spare);
        }
        #endif
    
}

void cap_te_csr_sta_debug_bus_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("vec", (cap_csr_base::set_function_type_t)&cap_te_csr_sta_debug_bus_t::vec);
            register_get_func("vec", (cap_csr_base::get_function_type_t)&cap_te_csr_sta_debug_bus_t::vec);
        }
        #endif
    
}

void cap_te_csr_cfg_lock_timeout_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_lock_timeout_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_lock_timeout_t::val);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("en", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_lock_timeout_t::en);
            register_get_func("en", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_lock_timeout_t::en);
        }
        #endif
    
    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_te_csr_cfg_debug_bus_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("axi_rdata_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_debug_bus_t::axi_rdata_sel);
            register_get_func("axi_rdata_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_debug_bus_t::axi_rdata_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tcam_key_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_debug_bus_t::tcam_key_sel);
            register_get_func("tcam_key_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_debug_bus_t::tcam_key_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hash_key_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_debug_bus_t::hash_key_sel);
            register_get_func("hash_key_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_debug_bus_t::hash_key_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_debug_bus_t::sel);
            register_get_func("sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_debug_bus_t::sel);
        }
        #endif
    
}

void cap_te_csr_cfg_cnt_debug_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_cnt_debug_t::sel);
            register_get_func("sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_cnt_debug_t::sel);
        }
        #endif
    
}

void cap_te_csr_cnt_debug_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_debug_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_debug_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_mpu_out_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_mpu_out_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_mpu_out_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_tcam_rsp_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_tcam_rsp_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_tcam_rsp_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_tcam_req_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_tcam_req_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_tcam_req_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_axi_rdrsp_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_axi_rdrsp_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_axi_rdrsp_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_axi_rdreq_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_axi_rdreq_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_axi_rdreq_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_phv_out_eop_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_phv_out_eop_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_phv_out_eop_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_phv_out_sop_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_phv_out_sop_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_phv_out_sop_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_phv_in_eop_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_phv_in_eop_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_phv_in_eop_t::val);
        }
        #endif
    
}

void cap_te_csr_cnt_phv_in_sop_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("val", (cap_csr_base::set_function_type_t)&cap_te_csr_cnt_phv_in_sop_t::val);
            register_get_func("val", (cap_csr_base::get_function_type_t)&cap_te_csr_cnt_phv_in_sop_t::val);
        }
        #endif
    
}

void cap_te_csr_dhs_single_step_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("key", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_single_step_t::key);
            register_get_func("key", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_single_step_t::key);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lkup", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_single_step_t::lkup);
            register_get_func("lkup", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_single_step_t::lkup);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu", (cap_csr_base::set_function_type_t)&cap_te_csr_dhs_single_step_t::mpu);
            register_get_func("mpu", (cap_csr_base::get_function_type_t)&cap_te_csr_dhs_single_step_t::mpu);
        }
        #endif
    
}

void cap_te_csr_cfg_km_profile_bit_loc_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("valid", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_km_profile_bit_loc_t::valid);
            register_get_func("valid", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_km_profile_bit_loc_t::valid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("bit_loc", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc);
            register_get_func("bit_loc", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc);
        }
        #endif
    
}

void cap_te_csr_cfg_km_profile_bit_sel_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("bit_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel);
            register_get_func("bit_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel);
        }
        #endif
    
}

void cap_te_csr_cfg_km_profile_byte_sel_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("byte_sel", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel);
            register_get_func("byte_sel", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel);
        }
        #endif
    
}

void cap_te_csr_cfg_table_mpu_const_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("value", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_mpu_const_t::value);
            register_get_func("value", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_mpu_const_t::value);
        }
        #endif
    
}

void cap_te_csr_cfg_table_property_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("axi", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::axi);
            register_get_func("axi", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::axi);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("key_mask_hi", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::key_mask_hi);
            register_get_func("key_mask_hi", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::key_mask_hi);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("key_mask_lo", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::key_mask_lo);
            register_get_func("key_mask_lo", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::key_mask_lo);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fullkey_km_sel0", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::fullkey_km_sel0);
            register_get_func("fullkey_km_sel0", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::fullkey_km_sel0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fullkey_km_sel1", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::fullkey_km_sel1);
            register_get_func("fullkey_km_sel1", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::fullkey_km_sel1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lock_en", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::lock_en);
            register_get_func("lock_en", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::lock_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tbl_entry_sz_raw", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw);
            register_get_func("tbl_entry_sz_raw", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_pc_dyn", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_dyn);
            register_get_func("mpu_pc_dyn", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_dyn);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_pc", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc);
            register_get_func("mpu_pc", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_pc_raw", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_raw);
            register_get_func("mpu_pc_raw", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_raw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_pc_ofst_err", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err);
            register_get_func("mpu_pc_ofst_err", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_vec", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_vec);
            register_get_func("mpu_vec", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_vec);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_base", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::addr_base);
            register_get_func("addr_base", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::addr_base);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_vf_id_en", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::addr_vf_id_en);
            register_get_func("addr_vf_id_en", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::addr_vf_id_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_vf_id_loc", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::addr_vf_id_loc);
            register_get_func("addr_vf_id_loc", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::addr_vf_id_loc);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_shift", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::addr_shift);
            register_get_func("addr_shift", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::addr_shift);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("chain_shift", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::chain_shift);
            register_get_func("chain_shift", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::chain_shift);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_sz", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::addr_sz);
            register_get_func("addr_sz", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::addr_sz);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lg2_entry_size", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::lg2_entry_size);
            register_get_func("lg2_entry_size", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::lg2_entry_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_bypass_cnt", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::max_bypass_cnt);
            register_get_func("max_bypass_cnt", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::max_bypass_cnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lock_en_raw", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::lock_en_raw);
            register_get_func("lock_en_raw", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::lock_en_raw);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_lb", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_lb);
            register_get_func("mpu_lb", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_lb);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_pc_loc", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_loc);
            register_get_func("mpu_pc_loc", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::mpu_pc_loc);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oflow_base_idx", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_property_t::oflow_base_idx);
            register_get_func("oflow_base_idx", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_property_t::oflow_base_idx);
        }
        #endif
    
    set_reset_val(cpp_int("0xffc8000000000000000000001e0000000000000000"));
    all(get_reset_val());
}

void cap_te_csr_cfg_table_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mpu_results", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_t::mpu_results);
            register_get_func("mpu_results", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_t::mpu_results);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("seq_base", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_t::seq_base);
            register_get_func("seq_base", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_t::seq_base);
        }
        #endif
    
}

void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("adv_phv_flit", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit);
            register_get_func("adv_phv_flit", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("done", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done);
            register_get_func("done", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done);
        }
        #endif
    
    set_reset_val(cpp_int("0x3"));
    all(get_reset_val());
}

void cap_te_csr_cfg_table_profile_cam_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("valid", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::valid);
            register_get_func("valid", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::valid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("value", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::value);
            register_get_func("value", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::value);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mask", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::mask);
            register_get_func("mask", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_cam_t::mask);
        }
        #endif
    
}

void cap_te_csr_cfg_table_profile_key_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel0", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel0);
            register_get_func("sel0", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel1", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel1);
            register_get_func("sel1", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel2", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel2);
            register_get_func("sel2", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel3", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel3);
            register_get_func("sel3", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel4", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel4);
            register_get_func("sel4", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel5", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel5);
            register_get_func("sel5", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel6", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel6);
            register_get_func("sel6", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sel7", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel7);
            register_get_func("sel7", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_table_profile_key_t::sel7);
        }
        #endif
    
}

void cap_te_csr_cfg_global_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sw_rst", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_global_t::sw_rst);
            register_get_func("sw_rst", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_global_t::sw_rst);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("step_key", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_global_t::step_key);
            register_get_func("step_key", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_global_t::step_key);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("step_lkup", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_global_t::step_lkup);
            register_get_func("step_lkup", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_global_t::step_lkup);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("step_mpu", (cap_csr_base::set_function_type_t)&cap_te_csr_cfg_global_t::step_mpu);
            register_get_func("step_mpu", (cap_csr_base::get_function_type_t)&cap_te_csr_cfg_global_t::step_mpu);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_te_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_te_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_te_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_te_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_global.set_attributes(this,"cfg_global", 0x4 );
    cfg_table_profile_key.set_attributes(this,"cfg_table_profile_key", 0x8 );
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_profile_cam[ii].set_field_init_done(true);
        cfg_table_profile_cam[ii].set_attributes(this,"cfg_table_profile_cam["+to_string(ii)+"]",  0x40 + (cfg_table_profile_cam[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 192; ii++) {
        if(ii != 0) cfg_table_profile_ctrl_sram_ext[ii].set_field_init_done(true);
        cfg_table_profile_ctrl_sram_ext[ii].set_attributes(this,"cfg_table_profile_ctrl_sram_ext["+to_string(ii)+"]",  0x1000 + (cfg_table_profile_ctrl_sram_ext[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_profile[ii].set_field_init_done(true);
        cfg_table_profile[ii].set_attributes(this,"cfg_table_profile["+to_string(ii)+"]",  0x1400 + (cfg_table_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_property[ii].set_field_init_done(true);
        cfg_table_property[ii].set_attributes(this,"cfg_table_property["+to_string(ii)+"]",  0x1600 + (cfg_table_property[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_table_mpu_const[ii].set_field_init_done(true);
        cfg_table_mpu_const[ii].set_attributes(this,"cfg_table_mpu_const["+to_string(ii)+"]",  0x1800 + (cfg_table_mpu_const[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 256; ii++) {
        if(ii != 0) cfg_km_profile_byte_sel[ii].set_field_init_done(true);
        cfg_km_profile_byte_sel[ii].set_attributes(this,"cfg_km_profile_byte_sel["+to_string(ii)+"]",  0x1c00 + (cfg_km_profile_byte_sel[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_km_profile_bit_sel[ii].set_field_init_done(true);
        cfg_km_profile_bit_sel[ii].set_attributes(this,"cfg_km_profile_bit_sel["+to_string(ii)+"]",  0x2000 + (cfg_km_profile_bit_sel[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_km_profile_bit_loc[ii].set_field_init_done(true);
        cfg_km_profile_bit_loc[ii].set_attributes(this,"cfg_km_profile_bit_loc["+to_string(ii)+"]",  0x2200 + (cfg_km_profile_bit_loc[ii].get_byte_size()*ii));
    }
    dhs_single_step.set_attributes(this,"dhs_single_step", 0x2240 );
    cnt_phv_in_sop.set_attributes(this,"cnt_phv_in_sop", 0x2244 );
    cnt_phv_in_eop.set_attributes(this,"cnt_phv_in_eop", 0x2248 );
    cnt_phv_out_sop.set_attributes(this,"cnt_phv_out_sop", 0x224c );
    cnt_phv_out_eop.set_attributes(this,"cnt_phv_out_eop", 0x2250 );
    cnt_axi_rdreq.set_attributes(this,"cnt_axi_rdreq", 0x2254 );
    cnt_axi_rdrsp.set_attributes(this,"cnt_axi_rdrsp", 0x2258 );
    cnt_tcam_req.set_attributes(this,"cnt_tcam_req", 0x225c );
    cnt_tcam_rsp.set_attributes(this,"cnt_tcam_rsp", 0x2260 );
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cnt_mpu_out[ii].set_field_init_done(true);
        cnt_mpu_out[ii].set_attributes(this,"cnt_mpu_out["+to_string(ii)+"]",  0x2270 + (cnt_mpu_out[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cnt_debug[ii].set_field_init_done(true);
        cnt_debug[ii].set_attributes(this,"cnt_debug["+to_string(ii)+"]",  0x2280 + (cnt_debug[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_cnt_debug[ii].set_field_init_done(true);
        cfg_cnt_debug[ii].set_attributes(this,"cfg_cnt_debug["+to_string(ii)+"]",  0x2290 + (cfg_cnt_debug[ii].get_byte_size()*ii));
    }
    cfg_debug_bus.set_attributes(this,"cfg_debug_bus", 0x22a0 );
    cfg_lock_timeout.set_attributes(this,"cfg_lock_timeout", 0x22a4 );
    sta_debug_bus.set_attributes(this,"sta_debug_bus", 0x22c0 );
    cfg_spare.set_attributes(this,"cfg_spare", 0x22e0 );
    cfg_read_wait_entry.set_attributes(this,"cfg_read_wait_entry", 0x22e8 );
    sta_wait_entry.set_attributes(this,"sta_wait_entry", 0x2300 );
    sta_spare.set_attributes(this,"sta_spare", 0x2400 );
    dhs_table_profile_ctrl_sram.set_attributes(this,"dhs_table_profile_ctrl_sram", 0x800 );
}

void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0(const cpp_int & _val) { 
    // km_mode0
    int_var__km_mode0 = _val.convert_to< km_mode0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode0() const {
    return int_var__km_mode0.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1(const cpp_int & _val) { 
    // km_mode1
    int_var__km_mode1 = _val.convert_to< km_mode1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode1() const {
    return int_var__km_mode1.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2(const cpp_int & _val) { 
    // km_mode2
    int_var__km_mode2 = _val.convert_to< km_mode2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode2() const {
    return int_var__km_mode2.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3(const cpp_int & _val) { 
    // km_mode3
    int_var__km_mode3 = _val.convert_to< km_mode3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_mode3() const {
    return int_var__km_mode3.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0(const cpp_int & _val) { 
    // km_profile0
    int_var__km_profile0 = _val.convert_to< km_profile0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile0() const {
    return int_var__km_profile0.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1(const cpp_int & _val) { 
    // km_profile1
    int_var__km_profile1 = _val.convert_to< km_profile1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile1() const {
    return int_var__km_profile1.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2(const cpp_int & _val) { 
    // km_profile2
    int_var__km_profile2 = _val.convert_to< km_profile2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile2() const {
    return int_var__km_profile2.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3(const cpp_int & _val) { 
    // km_profile3
    int_var__km_profile3 = _val.convert_to< km_profile3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_profile3() const {
    return int_var__km_profile3.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0(const cpp_int & _val) { 
    // km_new_key0
    int_var__km_new_key0 = _val.convert_to< km_new_key0_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key0() const {
    return int_var__km_new_key0.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1(const cpp_int & _val) { 
    // km_new_key1
    int_var__km_new_key1 = _val.convert_to< km_new_key1_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key1() const {
    return int_var__km_new_key1.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2(const cpp_int & _val) { 
    // km_new_key2
    int_var__km_new_key2 = _val.convert_to< km_new_key2_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key2() const {
    return int_var__km_new_key2.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3(const cpp_int & _val) { 
    // km_new_key3
    int_var__km_new_key3 = _val.convert_to< km_new_key3_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::km_new_key3() const {
    return int_var__km_new_key3.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup(const cpp_int & _val) { 
    // lkup
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::lkup() const {
    return int_var__lkup.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid(const cpp_int & _val) { 
    // tableid
    int_var__tableid = _val.convert_to< tableid_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::tableid() const {
    return int_var__tableid.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel(const cpp_int & _val) { 
    // hash_sel
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_sel() const {
    return int_var__hash_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain(const cpp_int & _val) { 
    // hash_chain
    int_var__hash_chain = _val.convert_to< hash_chain_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_chain() const {
    return int_var__hash_chain.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store(const cpp_int & _val) { 
    // hash_store
    int_var__hash_store = _val.convert_to< hash_store_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_table_profile_ctrl_sram_entry_t::hash_store() const {
    return int_var__hash_store.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_spare_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_te_csr_sta_spare_t::spare() const {
    return int_var__spare.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_wait_entry_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::key() const {
    return int_var__key.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_wait_entry_t::cam(const cpp_int & _val) { 
    // cam
    int_var__cam = _val.convert_to< cam_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::cam() const {
    return int_var__cam.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_wait_entry_t::haz(const cpp_int & _val) { 
    // haz
    int_var__haz = _val.convert_to< haz_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::haz() const {
    return int_var__haz.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_wait_entry_t::dat(const cpp_int & _val) { 
    // dat
    int_var__dat = _val.convert_to< dat_cpp_int_t >();
}

cpp_int cap_te_csr_sta_wait_entry_t::dat() const {
    return int_var__dat.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_read_wait_entry_t::idx(const cpp_int & _val) { 
    // idx
    int_var__idx = _val.convert_to< idx_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_read_wait_entry_t::idx() const {
    return int_var__idx.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_spare_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_spare_t::spare() const {
    return int_var__spare.convert_to< cpp_int >();
}
    
void cap_te_csr_sta_debug_bus_t::vec(const cpp_int & _val) { 
    // vec
    int_var__vec = _val.convert_to< vec_cpp_int_t >();
}

cpp_int cap_te_csr_sta_debug_bus_t::vec() const {
    return int_var__vec.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_lock_timeout_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_lock_timeout_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_lock_timeout_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_lock_timeout_t::en() const {
    return int_var__en.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_debug_bus_t::axi_rdata_sel(const cpp_int & _val) { 
    // axi_rdata_sel
    int_var__axi_rdata_sel = _val.convert_to< axi_rdata_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::axi_rdata_sel() const {
    return int_var__axi_rdata_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_debug_bus_t::tcam_key_sel(const cpp_int & _val) { 
    // tcam_key_sel
    int_var__tcam_key_sel = _val.convert_to< tcam_key_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::tcam_key_sel() const {
    return int_var__tcam_key_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_debug_bus_t::hash_key_sel(const cpp_int & _val) { 
    // hash_key_sel
    int_var__hash_key_sel = _val.convert_to< hash_key_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::hash_key_sel() const {
    return int_var__hash_key_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_debug_bus_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_debug_bus_t::sel() const {
    return int_var__sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_cnt_debug_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_cnt_debug_t::sel() const {
    return int_var__sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_debug_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_debug_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_mpu_out_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_mpu_out_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_tcam_rsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_tcam_rsp_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_tcam_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_tcam_req_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_axi_rdrsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_axi_rdrsp_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_axi_rdreq_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_axi_rdreq_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_phv_out_eop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_out_eop_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_phv_out_sop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_out_sop_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_phv_in_eop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_in_eop_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_cnt_phv_in_sop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_te_csr_cnt_phv_in_sop_t::val() const {
    return int_var__val.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_single_step_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::key() const {
    return int_var__key.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_single_step_t::lkup(const cpp_int & _val) { 
    // lkup
    int_var__lkup = _val.convert_to< lkup_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::lkup() const {
    return int_var__lkup.convert_to< cpp_int >();
}
    
void cap_te_csr_dhs_single_step_t::mpu(const cpp_int & _val) { 
    // mpu
    int_var__mpu = _val.convert_to< mpu_cpp_int_t >();
}

cpp_int cap_te_csr_dhs_single_step_t::mpu() const {
    return int_var__mpu.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_km_profile_bit_loc_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::valid() const {
    return int_var__valid.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc(const cpp_int & _val) { 
    // bit_loc
    int_var__bit_loc = _val.convert_to< bit_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_loc_t::bit_loc() const {
    return int_var__bit_loc.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel(const cpp_int & _val) { 
    // bit_sel
    int_var__bit_sel = _val.convert_to< bit_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_bit_sel_t::bit_sel() const {
    return int_var__bit_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel(const cpp_int & _val) { 
    // byte_sel
    int_var__byte_sel = _val.convert_to< byte_sel_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_km_profile_byte_sel_t::byte_sel() const {
    return int_var__byte_sel.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_mpu_const_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_mpu_const_t::value() const {
    return int_var__value.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::axi(const cpp_int & _val) { 
    // axi
    int_var__axi = _val.convert_to< axi_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::axi() const {
    return int_var__axi.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::key_mask_hi(const cpp_int & _val) { 
    // key_mask_hi
    int_var__key_mask_hi = _val.convert_to< key_mask_hi_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::key_mask_hi() const {
    return int_var__key_mask_hi.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::key_mask_lo(const cpp_int & _val) { 
    // key_mask_lo
    int_var__key_mask_lo = _val.convert_to< key_mask_lo_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::key_mask_lo() const {
    return int_var__key_mask_lo.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::fullkey_km_sel0(const cpp_int & _val) { 
    // fullkey_km_sel0
    int_var__fullkey_km_sel0 = _val.convert_to< fullkey_km_sel0_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::fullkey_km_sel0() const {
    return int_var__fullkey_km_sel0.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::fullkey_km_sel1(const cpp_int & _val) { 
    // fullkey_km_sel1
    int_var__fullkey_km_sel1 = _val.convert_to< fullkey_km_sel1_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::fullkey_km_sel1() const {
    return int_var__fullkey_km_sel1.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::lock_en(const cpp_int & _val) { 
    // lock_en
    int_var__lock_en = _val.convert_to< lock_en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lock_en() const {
    return int_var__lock_en.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw(const cpp_int & _val) { 
    // tbl_entry_sz_raw
    int_var__tbl_entry_sz_raw = _val.convert_to< tbl_entry_sz_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::tbl_entry_sz_raw() const {
    return int_var__tbl_entry_sz_raw.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_dyn(const cpp_int & _val) { 
    // mpu_pc_dyn
    int_var__mpu_pc_dyn = _val.convert_to< mpu_pc_dyn_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_dyn() const {
    return int_var__mpu_pc_dyn.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc(const cpp_int & _val) { 
    // mpu_pc
    int_var__mpu_pc = _val.convert_to< mpu_pc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc() const {
    return int_var__mpu_pc.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_raw(const cpp_int & _val) { 
    // mpu_pc_raw
    int_var__mpu_pc_raw = _val.convert_to< mpu_pc_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_raw() const {
    return int_var__mpu_pc_raw.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err(const cpp_int & _val) { 
    // mpu_pc_ofst_err
    int_var__mpu_pc_ofst_err = _val.convert_to< mpu_pc_ofst_err_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_ofst_err() const {
    return int_var__mpu_pc_ofst_err.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_vec(const cpp_int & _val) { 
    // mpu_vec
    int_var__mpu_vec = _val.convert_to< mpu_vec_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_vec() const {
    return int_var__mpu_vec.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::addr_base(const cpp_int & _val) { 
    // addr_base
    int_var__addr_base = _val.convert_to< addr_base_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_base() const {
    return int_var__addr_base.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::addr_vf_id_en(const cpp_int & _val) { 
    // addr_vf_id_en
    int_var__addr_vf_id_en = _val.convert_to< addr_vf_id_en_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_vf_id_en() const {
    return int_var__addr_vf_id_en.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::addr_vf_id_loc(const cpp_int & _val) { 
    // addr_vf_id_loc
    int_var__addr_vf_id_loc = _val.convert_to< addr_vf_id_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_vf_id_loc() const {
    return int_var__addr_vf_id_loc.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::addr_shift(const cpp_int & _val) { 
    // addr_shift
    int_var__addr_shift = _val.convert_to< addr_shift_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_shift() const {
    return int_var__addr_shift.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::chain_shift(const cpp_int & _val) { 
    // chain_shift
    int_var__chain_shift = _val.convert_to< chain_shift_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::chain_shift() const {
    return int_var__chain_shift.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::addr_sz(const cpp_int & _val) { 
    // addr_sz
    int_var__addr_sz = _val.convert_to< addr_sz_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::addr_sz() const {
    return int_var__addr_sz.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::lg2_entry_size(const cpp_int & _val) { 
    // lg2_entry_size
    int_var__lg2_entry_size = _val.convert_to< lg2_entry_size_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lg2_entry_size() const {
    return int_var__lg2_entry_size.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::max_bypass_cnt(const cpp_int & _val) { 
    // max_bypass_cnt
    int_var__max_bypass_cnt = _val.convert_to< max_bypass_cnt_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::max_bypass_cnt() const {
    return int_var__max_bypass_cnt.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::lock_en_raw(const cpp_int & _val) { 
    // lock_en_raw
    int_var__lock_en_raw = _val.convert_to< lock_en_raw_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::lock_en_raw() const {
    return int_var__lock_en_raw.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_lb(const cpp_int & _val) { 
    // mpu_lb
    int_var__mpu_lb = _val.convert_to< mpu_lb_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_lb() const {
    return int_var__mpu_lb.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::mpu_pc_loc(const cpp_int & _val) { 
    // mpu_pc_loc
    int_var__mpu_pc_loc = _val.convert_to< mpu_pc_loc_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::mpu_pc_loc() const {
    return int_var__mpu_pc_loc.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_property_t::oflow_base_idx(const cpp_int & _val) { 
    // oflow_base_idx
    int_var__oflow_base_idx = _val.convert_to< oflow_base_idx_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_property_t::oflow_base_idx() const {
    return int_var__oflow_base_idx.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_t::mpu_results(const cpp_int & _val) { 
    // mpu_results
    int_var__mpu_results = _val.convert_to< mpu_results_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_t::mpu_results() const {
    return int_var__mpu_results.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_t::seq_base(const cpp_int & _val) { 
    // seq_base
    int_var__seq_base = _val.convert_to< seq_base_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_t::seq_base() const {
    return int_var__seq_base.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit(const cpp_int & _val) { 
    // adv_phv_flit
    int_var__adv_phv_flit = _val.convert_to< adv_phv_flit_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::adv_phv_flit() const {
    return int_var__adv_phv_flit.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done(const cpp_int & _val) { 
    // done
    int_var__done = _val.convert_to< done_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_ctrl_sram_ext_t::done() const {
    return int_var__done.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_cam_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::valid() const {
    return int_var__valid.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_cam_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::value() const {
    return int_var__value.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_cam_t::mask(const cpp_int & _val) { 
    // mask
    int_var__mask = _val.convert_to< mask_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_cam_t::mask() const {
    return int_var__mask.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel0(const cpp_int & _val) { 
    // sel0
    int_var__sel0 = _val.convert_to< sel0_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel0() const {
    return int_var__sel0.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel1(const cpp_int & _val) { 
    // sel1
    int_var__sel1 = _val.convert_to< sel1_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel1() const {
    return int_var__sel1.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel2(const cpp_int & _val) { 
    // sel2
    int_var__sel2 = _val.convert_to< sel2_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel2() const {
    return int_var__sel2.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel3(const cpp_int & _val) { 
    // sel3
    int_var__sel3 = _val.convert_to< sel3_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel3() const {
    return int_var__sel3.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel4(const cpp_int & _val) { 
    // sel4
    int_var__sel4 = _val.convert_to< sel4_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel4() const {
    return int_var__sel4.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel5(const cpp_int & _val) { 
    // sel5
    int_var__sel5 = _val.convert_to< sel5_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel5() const {
    return int_var__sel5.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel6(const cpp_int & _val) { 
    // sel6
    int_var__sel6 = _val.convert_to< sel6_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel6() const {
    return int_var__sel6.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_table_profile_key_t::sel7(const cpp_int & _val) { 
    // sel7
    int_var__sel7 = _val.convert_to< sel7_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_table_profile_key_t::sel7() const {
    return int_var__sel7.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_global_t::sw_rst(const cpp_int & _val) { 
    // sw_rst
    int_var__sw_rst = _val.convert_to< sw_rst_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::sw_rst() const {
    return int_var__sw_rst.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_global_t::step_key(const cpp_int & _val) { 
    // step_key
    int_var__step_key = _val.convert_to< step_key_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_key() const {
    return int_var__step_key.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_global_t::step_lkup(const cpp_int & _val) { 
    // step_lkup
    int_var__step_lkup = _val.convert_to< step_lkup_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_lkup() const {
    return int_var__step_lkup.convert_to< cpp_int >();
}
    
void cap_te_csr_cfg_global_t::step_mpu(const cpp_int & _val) { 
    // step_mpu
    int_var__step_mpu = _val.convert_to< step_mpu_cpp_int_t >();
}

cpp_int cap_te_csr_cfg_global_t::step_mpu() const {
    return int_var__step_mpu.convert_to< cpp_int >();
}
    
void cap_te_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_te_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    