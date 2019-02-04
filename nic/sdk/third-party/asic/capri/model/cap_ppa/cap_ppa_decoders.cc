
#include "cap_ppa_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_ppa_decoders_mux_idx_t::cap_ppa_decoders_mux_idx_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_mux_idx_t::~cap_ppa_decoders_mux_idx_t() { }

cap_ppa_decoders_mux_inst_t::cap_ppa_decoders_mux_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_mux_inst_t::~cap_ppa_decoders_mux_inst_t() { }

cap_ppa_decoders_offset_inst_t::cap_ppa_decoders_offset_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_offset_inst_t::~cap_ppa_decoders_offset_inst_t() { }

cap_ppa_decoders_lkp_val_inst_t::cap_ppa_decoders_lkp_val_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_lkp_val_inst_t::~cap_ppa_decoders_lkp_val_inst_t() { }

cap_ppa_decoders_extract_inst_t::cap_ppa_decoders_extract_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_extract_inst_t::~cap_ppa_decoders_extract_inst_t() { }

cap_ppa_decoders_meta_inst_t::cap_ppa_decoders_meta_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_meta_inst_t::~cap_ppa_decoders_meta_inst_t() { }

cap_ppa_decoders_ohi_inst_t::cap_ppa_decoders_ohi_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_ohi_inst_t::~cap_ppa_decoders_ohi_inst_t() { }

cap_ppa_decoders_crc_inst_t::cap_ppa_decoders_crc_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_crc_inst_t::~cap_ppa_decoders_crc_inst_t() { }

cap_ppa_decoders_csum_inst_t::cap_ppa_decoders_csum_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_inst_t::~cap_ppa_decoders_csum_inst_t() { }

cap_ppa_decoders_len_chk_inst_t::cap_ppa_decoders_len_chk_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_len_chk_inst_t::~cap_ppa_decoders_len_chk_inst_t() { }

cap_ppa_decoders_align_chk_inst_t::cap_ppa_decoders_align_chk_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_align_chk_inst_t::~cap_ppa_decoders_align_chk_inst_t() { }

cap_ppa_decoders_ppa_lkp_sram_entry_t::cap_ppa_decoders_ppa_lkp_sram_entry_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_ppa_lkp_sram_entry_t::~cap_ppa_decoders_ppa_lkp_sram_entry_t() { }

cap_ppa_decoders_chk_ctl_t::cap_ppa_decoders_chk_ctl_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_chk_ctl_t::~cap_ppa_decoders_chk_ctl_t() { }

cap_ppa_decoders_ppa_lkp_tcam_key_t::cap_ppa_decoders_ppa_lkp_tcam_key_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_ppa_lkp_tcam_key_t::~cap_ppa_decoders_ppa_lkp_tcam_key_t() { }

cap_ppa_decoders_ppa_lkp_tcam_entry_t::cap_ppa_decoders_ppa_lkp_tcam_entry_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_ppa_lkp_tcam_entry_t::~cap_ppa_decoders_ppa_lkp_tcam_entry_t() { }

cap_ppa_decoders_crc_prof_t::cap_ppa_decoders_crc_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_crc_prof_t::~cap_ppa_decoders_crc_prof_t() { }

cap_ppa_decoders_crc_mask_prof_inst_t::cap_ppa_decoders_crc_mask_prof_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_crc_mask_prof_inst_t::~cap_ppa_decoders_crc_mask_prof_inst_t() { }

cap_ppa_decoders_crc_mask_prof_t::cap_ppa_decoders_crc_mask_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_crc_mask_prof_t::~cap_ppa_decoders_crc_mask_prof_t() { }

cap_ppa_decoders_csum_prof_t::cap_ppa_decoders_csum_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_prof_t::~cap_ppa_decoders_csum_prof_t() { }

cap_ppa_decoders_csum_phdr_prof_inst_t::cap_ppa_decoders_csum_phdr_prof_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_phdr_prof_inst_t::~cap_ppa_decoders_csum_phdr_prof_inst_t() { }

cap_ppa_decoders_csum_phdr_prof_t::cap_ppa_decoders_csum_phdr_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_phdr_prof_t::~cap_ppa_decoders_csum_phdr_prof_t() { }

cap_ppa_decoders_csum_all_fld_inst_t::cap_ppa_decoders_csum_all_fld_inst_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_all_fld_inst_t::~cap_ppa_decoders_csum_all_fld_inst_t() { }

cap_ppa_decoders_csum_all_prof_t::cap_ppa_decoders_csum_all_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_csum_all_prof_t::~cap_ppa_decoders_csum_all_prof_t() { }

cap_ppa_decoders_len_chk_prof_t::cap_ppa_decoders_len_chk_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_len_chk_prof_t::~cap_ppa_decoders_len_chk_prof_t() { }

cap_ppa_decoders_pkt_ff_data_t::cap_ppa_decoders_pkt_ff_data_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_pkt_ff_data_t::~cap_ppa_decoders_pkt_ff_data_t() { }

cap_ppa_decoders_align_chk_prof_t::cap_ppa_decoders_align_chk_prof_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_align_chk_prof_t::~cap_ppa_decoders_align_chk_prof_t() { }

cap_ppa_decoders_err_vec_t::cap_ppa_decoders_err_vec_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_err_vec_t::~cap_ppa_decoders_err_vec_t() { }

cap_ppa_decoders_si_mux_pkt_rslt_t::cap_ppa_decoders_si_mux_pkt_rslt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_mux_pkt_rslt_t::~cap_ppa_decoders_si_mux_pkt_rslt_t() { }

cap_ppa_decoders_si_mux_inst_rslt_t::cap_ppa_decoders_si_mux_inst_rslt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_mux_inst_rslt_t::~cap_ppa_decoders_si_mux_inst_rslt_t() { }

cap_ppa_decoders_si_extract_rslt_t::cap_ppa_decoders_si_extract_rslt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_extract_rslt_t::~cap_ppa_decoders_si_extract_rslt_t() { }

cap_ppa_decoders_si_meta_rslt_t::cap_ppa_decoders_si_meta_rslt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_meta_rslt_t::~cap_ppa_decoders_si_meta_rslt_t() { }

cap_ppa_decoders_si_state_rslt_t::cap_ppa_decoders_si_state_rslt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_state_rslt_t::~cap_ppa_decoders_si_state_rslt_t() { }

cap_ppa_decoders_si_state_info_t::cap_ppa_decoders_si_state_info_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_state_info_t::~cap_ppa_decoders_si_state_info_t() { }

cap_ppa_decoders_si_pe_step_info_t::cap_ppa_decoders_si_pe_step_info_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_pe_step_info_t::~cap_ppa_decoders_si_pe_step_info_t() { }

cap_ppa_decoders_si_non_pe_step_info_t::cap_ppa_decoders_si_non_pe_step_info_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ppa_decoders_si_non_pe_step_info_t::~cap_ppa_decoders_si_non_pe_step_info_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_mux_idx_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lkpsel: 0x" << int_var__lkpsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".idx: 0x" << int_var__idx << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".load_stored_lkp: 0x" << int_var__load_stored_lkp << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_mux_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".muxsel: 0x" << int_var__muxsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_val: 0x" << int_var__mask_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".shift_val: 0x" << int_var__shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_val: 0x" << int_var__addsub_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".shift_left: 0x" << int_var__shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub: 0x" << int_var__addsub << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lkpsel: 0x" << int_var__lkpsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lkp_addsub: 0x" << int_var__lkp_addsub << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".load_mux_pkt: 0x" << int_var__load_mux_pkt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_offset_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".muxsel: 0x" << int_var__muxsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_lkp_val_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".muxsel: 0x" << int_var__muxsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".store_en: 0x" << int_var__store_en << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_extract_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_idx: 0x" << int_var__pkt_idx << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_idx: 0x" << int_var__phv_idx << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_meta_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".phv_idx: 0x" << int_var__phv_idx << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_ohi_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sel: 0x" << int_var__sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".muxsel: 0x" << int_var__muxsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".idx_val: 0x" << int_var__idx_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slot_num: 0x" << int_var__slot_num << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_crc_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prof_sel_en: 0x" << int_var__prof_sel_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prof_sel: 0x" << int_var__prof_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_start_sel: 0x" << int_var__ohi_start_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_len_sel: 0x" << int_var__ohi_len_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_mask_sel: 0x" << int_var__ohi_mask_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".unit_sel: 0x" << int_var__unit_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prof_sel: 0x" << int_var__prof_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_start_sel: 0x" << int_var__ohi_start_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_len_sel: 0x" << int_var__ohi_len_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_en: 0x" << int_var__phdr_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_sel: 0x" << int_var__phdr_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_ohi_sel: 0x" << int_var__phdr_ohi_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dis_zero: 0x" << int_var__dis_zero << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".load_phdr_prof_en: 0x" << int_var__load_phdr_prof_en << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_len_chk_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".unit_sel: 0x" << int_var__unit_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prof_sel: 0x" << int_var__prof_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_start_sel: 0x" << int_var__ohi_start_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_len_sel: 0x" << int_var__ohi_len_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".exact: 0x" << int_var__exact << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_align_chk_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".en: 0x" << int_var__en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prof_sel: 0x" << int_var__prof_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_start_sel: 0x" << int_var__ohi_start_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_ppa_lkp_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".action: 0x" << int_var__action << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nxt_state: 0x" << int_var__nxt_state << dec << endl);
    offset_inst.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_idx.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        mux_idx[ii].show();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].show();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    lkp_val_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 3; ii++) {
        lkp_val_inst[ii].show();
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_idx_upr: 0x" << int_var__phv_idx_upr << dec << endl);
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    extract_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].show();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    meta_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    ohi_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        ohi_inst[ii].show();
    }
    #endif
    
    crc_inst.show();
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    csum_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 2; ii++) {
        csum_inst[ii].show();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    len_chk_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 2; ii++) {
        len_chk_inst[ii].show();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    align_chk_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].show();
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".offset_jump_chk_en: 0x" << int_var__offset_jump_chk_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_size_chk_en: 0x" << int_var__pkt_size_chk_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_size: 0x" << int_var__pkt_size << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_chk_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".last_offset: 0x" << int_var__last_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".seq_id: 0x" << int_var__seq_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".err_vec: 0x" << int_var__err_vec << dec << endl);
    crc_inst.show();
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    csum_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 5; ii++) {
        csum_inst[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    len_chk_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        len_chk_inst[ii].show();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    align_chk_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_ppa_lkp_tcam_key_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".control: 0x" << int_var__control << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".state: 0x" << int_var__state << dec << endl);
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array lkp_val skipped" << endl);
    #else
    for(int ii = 0; ii < 3; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".lkp_val[" << ii <<"]: 0x" << int_var__lkp_val[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_ppa_lkp_tcam_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    key.show();
    mask.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_crc_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_start: 0x" << int_var__addsub_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_end: 0x" << int_var__addsub_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_mask: 0x" << int_var__addsub_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_adj: 0x" << int_var__mask_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_eop: 0x" << int_var__end_eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_prof_sel: 0x" << int_var__mask_prof_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_crc_mask_prof_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mask_en: 0x" << int_var__mask_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".use_ohi: 0x" << int_var__use_ohi << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fill: 0x" << int_var__fill << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble: 0x" << int_var__skip_first_nibble << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_crc_mask_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.show(); // large_array
    #else
    for(int ii = 0; ii < 6; ii++) {
        fld[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".csum_8b: 0x" << int_var__csum_8b << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_csum_flit_num: 0x" << int_var__phv_csum_flit_num << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_eop: 0x" << int_var__end_eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_start: 0x" << int_var__addsub_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_end: 0x" << int_var__addsub_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_phdr: 0x" << int_var__addsub_phdr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_adj: 0x" << int_var__phdr_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_csum_loc: 0x" << int_var__addsub_csum_loc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_loc_adj: 0x" << int_var__csum_loc_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".align: 0x" << int_var__align << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_val: 0x" << int_var__add_val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_phdr_prof_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en: 0x" << int_var__fld_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align: 0x" << int_var__fld_align << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start: 0x" << int_var__fld_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end: 0x" << int_var__fld_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_phdr_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        fld[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_all_fld_inst_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en: 0x" << int_var__fld_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align: 0x" << int_var__fld_align << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start: 0x" << int_var__fld_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end: 0x" << int_var__fld_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_eop: 0x" << int_var__end_eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_csum_flit_num: 0x" << int_var__phv_csum_flit_num << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_8b: 0x" << int_var__csum_8b << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_val: 0x" << int_var__add_val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_csum_all_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.show(); // large_array
    #else
    for(int ii = 0; ii < 5; ii++) {
        fld[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_len_chk_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_start: 0x" << int_var__addsub_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_pkt_ff_data_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".err: 0x" << int_var__err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".eop: 0x" << int_var__eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sop: 0x" << int_var__sop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".seq_id: 0x" << int_var__seq_id << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_align_chk_prof_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addsub_start: 0x" << int_var__addsub_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mod_log2: 0x" << int_var__mod_log2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_err_vec_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".offset_jump_chk: 0x" << int_var__offset_jump_chk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_size_chk: 0x" << int_var__pkt_size_chk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".offset_out_of_range: 0x" << int_var__offset_out_of_range << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".exceed_parse_loop_cnt: 0x" << int_var__exceed_parse_loop_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".exceed_phv_flit_cnt: 0x" << int_var__exceed_phv_flit_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_upr_idx_less: 0x" << int_var__phv_upr_idx_less << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".init_lkp_idx: 0x" << int_var__init_lkp_idx << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_mux_pkt_rslt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    inst.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".mux_pkt: 0x" << int_var__mux_pkt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_mux_inst_rslt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    inst.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".mux_pkt_inst: 0x" << int_var__mux_pkt_inst << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_extract_rslt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    inst.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_meta_rslt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    inst.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_state_rslt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_pkt.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        mux_pkt[ii].show();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    extract_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].show();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    meta_inst.show(); // large_array
    #else
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_state_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    lkp_key.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_hit: 0x" << int_var__tcam_hit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tcam_hit_idx: 0x" << int_var__tcam_hit_idx << dec << endl);
    state_rslt.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".flit_idx: 0x" << int_var__flit_idx << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_pe_step_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".init_profile_idx: 0x" << int_var__init_profile_idx << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_global_csum_err: 0x" << int_var__phv_global_csum_err << dec << endl);
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array lkp_cur_state skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".lkp_cur_state[" << ii <<"]: 0x" << int_var__lkp_cur_state[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array stored_lkp_val0 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".stored_lkp_val0[" << ii <<"]: 0x" << int_var__stored_lkp_val0[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array lkp_val0 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".lkp_val0[" << ii <<"]: 0x" << int_var__lkp_val0[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array stored_lkp_val1 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".stored_lkp_val1[" << ii <<"]: 0x" << int_var__stored_lkp_val1[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array lkp_val1 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".lkp_val1[" << ii <<"]: 0x" << int_var__lkp_val1[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array stored_lkp_val2 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".stored_lkp_val2[" << ii <<"]: 0x" << int_var__stored_lkp_val2[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array lkp_val2 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".lkp_val2[" << ii <<"]: 0x" << int_var__lkp_val2[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array tcam_hit skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".tcam_hit[" << ii <<"]: 0x" << int_var__tcam_hit[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array tcam_hit_idx skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".tcam_hit_idx[" << ii <<"]: 0x" << int_var__tcam_hit_idx[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array cur_flit_idx skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".cur_flit_idx[" << ii <<"]: 0x" << int_var__cur_flit_idx[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_pkt0 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_pkt0[" << ii <<"]: 0x" << int_var__mux_pkt0[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_pkt1 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_pkt1[" << ii <<"]: 0x" << int_var__mux_pkt1[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_pkt2 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_pkt2[" << ii <<"]: 0x" << int_var__mux_pkt2[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_pkt3 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_pkt3[" << ii <<"]: 0x" << int_var__mux_pkt3[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_inst_data0 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_inst_data0[" << ii <<"]: 0x" << int_var__mux_inst_data0[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_inst_data1 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_inst_data1[" << ii <<"]: 0x" << int_var__mux_inst_data1[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mux_inst_data2 skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mux_inst_data2[" << ii <<"]: 0x" << int_var__mux_inst_data2[ii] << dec << endl);
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array extract_phv_data skipped" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".extract_phv_data[" << ii <<"]: 0x" << int_var__extract_phv_data[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ppa_decoders_si_non_pe_step_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".preparser_csum_err_out: 0x" << int_var__preparser_csum_err_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parse_end_ptr: 0x" << int_var__parse_end_ptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".err_vec: 0x" << int_var__err_vec << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".err_code: 0x" << int_var__err_code << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_val: 0x" << int_var__crc_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_err: 0x" << int_var__crc_err << dec << endl);
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array csum_val skipped" << endl);
    #else
    for(int ii = 0; ii < 5; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".csum_val[" << ii <<"]: 0x" << int_var__csum_val[ii] << dec << endl);
    }
    #endif
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array csum_err skipped" << endl);
    #else
    for(int ii = 0; ii < 5; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".csum_err[" << ii <<"]: 0x" << int_var__csum_err[ii] << dec << endl);
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_val0: 0x" << int_var__len_chk_val0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_err0: 0x" << int_var__len_chk_err0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_val1: 0x" << int_var__len_chk_val1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_err1: 0x" << int_var__len_chk_err1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_val2: 0x" << int_var__len_chk_val2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_err2: 0x" << int_var__len_chk_err2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_val3: 0x" << int_var__len_chk_val3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_chk_err3: 0x" << int_var__len_chk_err3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".align_chk_val0: 0x" << int_var__align_chk_val0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".align_chk_err0: 0x" << int_var__align_chk_err0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".align_chk_val1: 0x" << int_var__align_chk_val1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".align_chk_err1: 0x" << int_var__align_chk_err1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

int cap_ppa_decoders_mux_idx_t::get_width() const {
    return cap_ppa_decoders_mux_idx_t::s_get_width();

}

int cap_ppa_decoders_mux_inst_t::get_width() const {
    return cap_ppa_decoders_mux_inst_t::s_get_width();

}

int cap_ppa_decoders_offset_inst_t::get_width() const {
    return cap_ppa_decoders_offset_inst_t::s_get_width();

}

int cap_ppa_decoders_lkp_val_inst_t::get_width() const {
    return cap_ppa_decoders_lkp_val_inst_t::s_get_width();

}

int cap_ppa_decoders_extract_inst_t::get_width() const {
    return cap_ppa_decoders_extract_inst_t::s_get_width();

}

int cap_ppa_decoders_meta_inst_t::get_width() const {
    return cap_ppa_decoders_meta_inst_t::s_get_width();

}

int cap_ppa_decoders_ohi_inst_t::get_width() const {
    return cap_ppa_decoders_ohi_inst_t::s_get_width();

}

int cap_ppa_decoders_crc_inst_t::get_width() const {
    return cap_ppa_decoders_crc_inst_t::s_get_width();

}

int cap_ppa_decoders_csum_inst_t::get_width() const {
    return cap_ppa_decoders_csum_inst_t::s_get_width();

}

int cap_ppa_decoders_len_chk_inst_t::get_width() const {
    return cap_ppa_decoders_len_chk_inst_t::s_get_width();

}

int cap_ppa_decoders_align_chk_inst_t::get_width() const {
    return cap_ppa_decoders_align_chk_inst_t::s_get_width();

}

int cap_ppa_decoders_ppa_lkp_sram_entry_t::get_width() const {
    return cap_ppa_decoders_ppa_lkp_sram_entry_t::s_get_width();

}

int cap_ppa_decoders_chk_ctl_t::get_width() const {
    return cap_ppa_decoders_chk_ctl_t::s_get_width();

}

int cap_ppa_decoders_ppa_lkp_tcam_key_t::get_width() const {
    return cap_ppa_decoders_ppa_lkp_tcam_key_t::s_get_width();

}

int cap_ppa_decoders_ppa_lkp_tcam_entry_t::get_width() const {
    return cap_ppa_decoders_ppa_lkp_tcam_entry_t::s_get_width();

}

int cap_ppa_decoders_crc_prof_t::get_width() const {
    return cap_ppa_decoders_crc_prof_t::s_get_width();

}

int cap_ppa_decoders_crc_mask_prof_inst_t::get_width() const {
    return cap_ppa_decoders_crc_mask_prof_inst_t::s_get_width();

}

int cap_ppa_decoders_crc_mask_prof_t::get_width() const {
    return cap_ppa_decoders_crc_mask_prof_t::s_get_width();

}

int cap_ppa_decoders_csum_prof_t::get_width() const {
    return cap_ppa_decoders_csum_prof_t::s_get_width();

}

int cap_ppa_decoders_csum_phdr_prof_inst_t::get_width() const {
    return cap_ppa_decoders_csum_phdr_prof_inst_t::s_get_width();

}

int cap_ppa_decoders_csum_phdr_prof_t::get_width() const {
    return cap_ppa_decoders_csum_phdr_prof_t::s_get_width();

}

int cap_ppa_decoders_csum_all_fld_inst_t::get_width() const {
    return cap_ppa_decoders_csum_all_fld_inst_t::s_get_width();

}

int cap_ppa_decoders_csum_all_prof_t::get_width() const {
    return cap_ppa_decoders_csum_all_prof_t::s_get_width();

}

int cap_ppa_decoders_len_chk_prof_t::get_width() const {
    return cap_ppa_decoders_len_chk_prof_t::s_get_width();

}

int cap_ppa_decoders_pkt_ff_data_t::get_width() const {
    return cap_ppa_decoders_pkt_ff_data_t::s_get_width();

}

int cap_ppa_decoders_align_chk_prof_t::get_width() const {
    return cap_ppa_decoders_align_chk_prof_t::s_get_width();

}

int cap_ppa_decoders_err_vec_t::get_width() const {
    return cap_ppa_decoders_err_vec_t::s_get_width();

}

int cap_ppa_decoders_si_mux_pkt_rslt_t::get_width() const {
    return cap_ppa_decoders_si_mux_pkt_rslt_t::s_get_width();

}

int cap_ppa_decoders_si_mux_inst_rslt_t::get_width() const {
    return cap_ppa_decoders_si_mux_inst_rslt_t::s_get_width();

}

int cap_ppa_decoders_si_extract_rslt_t::get_width() const {
    return cap_ppa_decoders_si_extract_rslt_t::s_get_width();

}

int cap_ppa_decoders_si_meta_rslt_t::get_width() const {
    return cap_ppa_decoders_si_meta_rslt_t::s_get_width();

}

int cap_ppa_decoders_si_state_rslt_t::get_width() const {
    return cap_ppa_decoders_si_state_rslt_t::s_get_width();

}

int cap_ppa_decoders_si_state_info_t::get_width() const {
    return cap_ppa_decoders_si_state_info_t::s_get_width();

}

int cap_ppa_decoders_si_pe_step_info_t::get_width() const {
    return cap_ppa_decoders_si_pe_step_info_t::s_get_width();

}

int cap_ppa_decoders_si_non_pe_step_info_t::get_width() const {
    return cap_ppa_decoders_si_non_pe_step_info_t::s_get_width();

}

int cap_ppa_decoders_mux_idx_t::s_get_width() {
    int _count = 0;

    _count += 2; // sel
    _count += 2; // lkpsel
    _count += 6; // idx
    _count += 1; // load_stored_lkp
    return _count;
}

int cap_ppa_decoders_mux_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // sel
    _count += 2; // muxsel
    _count += 16; // mask_val
    _count += 4; // shift_val
    _count += 8; // addsub_val
    _count += 1; // shift_left
    _count += 1; // addsub
    _count += 2; // lkpsel
    _count += 1; // lkp_addsub
    _count += 1; // load_mux_pkt
    return _count;
}

int cap_ppa_decoders_offset_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // sel
    _count += 2; // muxsel
    _count += 14; // val
    return _count;
}

int cap_ppa_decoders_lkp_val_inst_t::s_get_width() {
    int _count = 0;

    _count += 2; // sel
    _count += 2; // muxsel
    _count += 1; // store_en
    return _count;
}

int cap_ppa_decoders_extract_inst_t::s_get_width() {
    int _count = 0;

    _count += 6; // pkt_idx
    _count += 3; // len
    _count += 7; // phv_idx
    return _count;
}

int cap_ppa_decoders_meta_inst_t::s_get_width() {
    int _count = 0;

    _count += 7; // phv_idx
    _count += 8; // val
    _count += 3; // sel
    return _count;
}

int cap_ppa_decoders_ohi_inst_t::s_get_width() {
    int _count = 0;

    _count += 3; // sel
    _count += 2; // muxsel
    _count += 16; // idx_val
    _count += 6; // slot_num
    return _count;
}

int cap_ppa_decoders_crc_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // en
    _count += 1; // prof_sel_en
    _count += 3; // prof_sel
    _count += 6; // ohi_start_sel
    _count += 6; // ohi_len_sel
    _count += 6; // ohi_mask_sel
    return _count;
}

int cap_ppa_decoders_csum_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // en
    _count += 3; // unit_sel
    _count += 3; // prof_sel
    _count += 6; // ohi_start_sel
    _count += 6; // ohi_len_sel
    _count += 1; // phdr_en
    _count += 3; // phdr_sel
    _count += 6; // phdr_ohi_sel
    _count += 1; // dis_zero
    _count += 1; // load_phdr_prof_en
    return _count;
}

int cap_ppa_decoders_len_chk_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // en
    _count += 2; // unit_sel
    _count += 2; // prof_sel
    _count += 6; // ohi_start_sel
    _count += 6; // ohi_len_sel
    _count += 1; // exact
    return _count;
}

int cap_ppa_decoders_align_chk_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // en
    _count += 1; // prof_sel
    _count += 6; // ohi_start_sel
    return _count;
}

int cap_ppa_decoders_ppa_lkp_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // action
    _count += 9; // nxt_state
    _count += cap_ppa_decoders_offset_inst_t::s_get_width(); // offset_inst
    _count += (cap_ppa_decoders_mux_idx_t::s_get_width() * 4); // mux_idx
    _count += (cap_ppa_decoders_mux_inst_t::s_get_width() * 3); // mux_inst
    _count += (cap_ppa_decoders_lkp_val_inst_t::s_get_width() * 3); // lkp_val_inst
    _count += 3; // phv_idx_upr
    _count += (cap_ppa_decoders_extract_inst_t::s_get_width() * 16); // extract_inst
    _count += (cap_ppa_decoders_meta_inst_t::s_get_width() * 3); // meta_inst
    _count += (cap_ppa_decoders_ohi_inst_t::s_get_width() * 4); // ohi_inst
    _count += cap_ppa_decoders_crc_inst_t::s_get_width(); // crc_inst
    _count += (cap_ppa_decoders_csum_inst_t::s_get_width() * 2); // csum_inst
    _count += (cap_ppa_decoders_len_chk_inst_t::s_get_width() * 2); // len_chk_inst
    _count += (cap_ppa_decoders_align_chk_inst_t::s_get_width() * 2); // align_chk_inst
    _count += 1; // offset_jump_chk_en
    _count += 1; // pkt_size_chk_en
    _count += 6; // pkt_size
    return _count;
}

int cap_ppa_decoders_chk_ctl_t::s_get_width() {
    int _count = 0;

    _count += 14; // last_offset
    _count += 7; // seq_id
    _count += 7; // err_vec
    _count += cap_ppa_decoders_crc_inst_t::s_get_width(); // crc_inst
    _count += (cap_ppa_decoders_csum_inst_t::s_get_width() * 5); // csum_inst
    _count += (cap_ppa_decoders_len_chk_inst_t::s_get_width() * 4); // len_chk_inst
    _count += (cap_ppa_decoders_align_chk_inst_t::s_get_width() * 2); // align_chk_inst
    return _count;
}

int cap_ppa_decoders_ppa_lkp_tcam_key_t::s_get_width() {
    int _count = 0;

    _count += 7; // control
    _count += 9; // state
    _count += 16 * 3; // lkp_val
    return _count;
}

int cap_ppa_decoders_ppa_lkp_tcam_entry_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_ppa_lkp_tcam_key_t::s_get_width(); // key
    _count += cap_ppa_decoders_ppa_lkp_tcam_key_t::s_get_width(); // mask
    _count += 1; // valid
    return _count;
}

int cap_ppa_decoders_crc_prof_t::s_get_width() {
    int _count = 0;

    _count += 14; // len_mask
    _count += 1; // len_shift_left
    _count += 3; // len_shift_val
    _count += 1; // addsub_start
    _count += 6; // start_adj
    _count += 1; // addsub_end
    _count += 6; // end_adj
    _count += 1; // addsub_mask
    _count += 6; // mask_adj
    _count += 1; // end_eop
    _count += 2; // mask_prof_sel
    return _count;
}

int cap_ppa_decoders_crc_mask_prof_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // mask_en
    _count += 1; // use_ohi
    _count += 6; // start_adj
    _count += 6; // end_adj
    _count += 1; // fill
    _count += 1; // skip_first_nibble
    return _count;
}

int cap_ppa_decoders_crc_mask_prof_t::s_get_width() {
    int _count = 0;

    _count += (cap_ppa_decoders_crc_mask_prof_inst_t::s_get_width() * 6); // fld
    return _count;
}

int cap_ppa_decoders_csum_prof_t::s_get_width() {
    int _count = 0;

    _count += 1; // csum_8b
    _count += 4; // phv_csum_flit_num
    _count += 1; // end_eop
    _count += 14; // len_mask
    _count += 1; // len_shift_left
    _count += 3; // len_shift_val
    _count += 1; // addsub_start
    _count += 6; // start_adj
    _count += 1; // addsub_end
    _count += 6; // end_adj
    _count += 1; // addsub_phdr
    _count += 6; // phdr_adj
    _count += 1; // addsub_csum_loc
    _count += 6; // csum_loc_adj
    _count += 1; // align
    _count += 16; // add_val
    return _count;
}

int cap_ppa_decoders_csum_phdr_prof_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // fld_en
    _count += 1; // fld_align
    _count += 6; // fld_start
    _count += 6; // fld_end
    _count += 1; // add_len
    return _count;
}

int cap_ppa_decoders_csum_phdr_prof_t::s_get_width() {
    int _count = 0;

    _count += (cap_ppa_decoders_csum_phdr_prof_inst_t::s_get_width() * 4); // fld
    return _count;
}

int cap_ppa_decoders_csum_all_fld_inst_t::s_get_width() {
    int _count = 0;

    _count += 1; // fld_en
    _count += 1; // fld_align
    _count += 16; // fld_start
    _count += 16; // fld_end
    _count += 1; // add_len
    _count += 1; // end_eop
    _count += 4; // phv_csum_flit_num
    _count += 1; // csum_8b
    _count += 16; // add_val
    return _count;
}

int cap_ppa_decoders_csum_all_prof_t::s_get_width() {
    int _count = 0;

    _count += (cap_ppa_decoders_csum_all_fld_inst_t::s_get_width() * 5); // fld
    return _count;
}

int cap_ppa_decoders_len_chk_prof_t::s_get_width() {
    int _count = 0;

    _count += 14; // len_mask
    _count += 1; // len_shift_left
    _count += 3; // len_shift_val
    _count += 1; // addsub_start
    _count += 6; // start_adj
    return _count;
}

int cap_ppa_decoders_pkt_ff_data_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    _count += 6; // sz
    _count += 1; // err
    _count += 1; // eop
    _count += 1; // sop
    _count += 5; // seq_id
    return _count;
}

int cap_ppa_decoders_align_chk_prof_t::s_get_width() {
    int _count = 0;

    _count += 1; // addsub_start
    _count += 6; // start_adj
    _count += 8; // mod_log2
    return _count;
}

int cap_ppa_decoders_err_vec_t::s_get_width() {
    int _count = 0;

    _count += 1; // offset_jump_chk
    _count += 1; // pkt_size_chk
    _count += 1; // offset_out_of_range
    _count += 1; // exceed_parse_loop_cnt
    _count += 1; // exceed_phv_flit_cnt
    _count += 1; // phv_upr_idx_less
    _count += 1; // init_lkp_idx
    return _count;
}

int cap_ppa_decoders_si_mux_pkt_rslt_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_mux_idx_t::s_get_width(); // inst
    _count += 16; // mux_pkt
    return _count;
}

int cap_ppa_decoders_si_mux_inst_rslt_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_mux_inst_t::s_get_width(); // inst
    _count += 16; // mux_pkt_inst
    return _count;
}

int cap_ppa_decoders_si_extract_rslt_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_extract_inst_t::s_get_width(); // inst
    _count += 32; // data
    return _count;
}

int cap_ppa_decoders_si_meta_rslt_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_meta_inst_t::s_get_width(); // inst
    _count += 32; // data
    return _count;
}

int cap_ppa_decoders_si_state_rslt_t::s_get_width() {
    int _count = 0;

    _count += (cap_ppa_decoders_si_mux_pkt_rslt_t::s_get_width() * 4); // mux_pkt
    _count += (cap_ppa_decoders_si_mux_inst_rslt_t::s_get_width() * 3); // mux_inst
    _count += (cap_ppa_decoders_si_extract_rslt_t::s_get_width() * 16); // extract_inst
    _count += (cap_ppa_decoders_si_meta_rslt_t::s_get_width() * 3); // meta_inst
    return _count;
}

int cap_ppa_decoders_si_state_info_t::s_get_width() {
    int _count = 0;

    _count += cap_ppa_decoders_ppa_lkp_tcam_key_t::s_get_width(); // lkp_key
    _count += 1; // tcam_hit
    _count += 10; // tcam_hit_idx
    _count += cap_ppa_decoders_si_state_rslt_t::s_get_width(); // state_rslt
    _count += 3; // flit_idx
    return _count;
}

int cap_ppa_decoders_si_pe_step_info_t::s_get_width() {
    int _count = 0;

    _count += 4; // init_profile_idx
    _count += 5; // phv_global_csum_err
    _count += 9 * 100; // lkp_cur_state
    _count += 16 * 100; // stored_lkp_val0
    _count += 16 * 100; // lkp_val0
    _count += 16 * 100; // stored_lkp_val1
    _count += 16 * 100; // lkp_val1
    _count += 16 * 100; // stored_lkp_val2
    _count += 16 * 100; // lkp_val2
    _count += 1 * 100; // tcam_hit
    _count += 10 * 100; // tcam_hit_idx
    _count += 3 * 100; // cur_flit_idx
    _count += 16 * 100; // mux_pkt0
    _count += 16 * 100; // mux_pkt1
    _count += 16 * 100; // mux_pkt2
    _count += 16 * 100; // mux_pkt3
    _count += 16 * 100; // mux_inst_data0
    _count += 16 * 100; // mux_inst_data1
    _count += 16 * 100; // mux_inst_data2
    _count += 1024 * 100; // extract_phv_data
    return _count;
}

int cap_ppa_decoders_si_non_pe_step_info_t::s_get_width() {
    int _count = 0;

    _count += 5; // preparser_csum_err_out
    _count += 16; // parse_end_ptr
    _count += 6; // err_vec
    _count += 6; // err_code
    _count += 32; // crc_val
    _count += 1; // crc_err
    _count += 32 * 5; // csum_val
    _count += 1 * 5; // csum_err
    _count += 32; // len_chk_val0
    _count += 1; // len_chk_err0
    _count += 32; // len_chk_val1
    _count += 1; // len_chk_err1
    _count += 32; // len_chk_val2
    _count += 1; // len_chk_err2
    _count += 32; // len_chk_val3
    _count += 1; // len_chk_err3
    _count += 32; // align_chk_val0
    _count += 1; // align_chk_err0
    _count += 32; // align_chk_val1
    _count += 1; // align_chk_err1
    return _count;
}

void cap_ppa_decoders_mux_idx_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__lkpsel = _val.convert_to< lkpsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__idx = _val.convert_to< idx_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__load_stored_lkp = _val.convert_to< load_stored_lkp_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_mux_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__mask_val = _val.convert_to< mask_val_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__shift_val = _val.convert_to< shift_val_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__addsub_val = _val.convert_to< addsub_val_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__shift_left = _val.convert_to< shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addsub = _val.convert_to< addsub_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__lkpsel = _val.convert_to< lkpsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__lkp_addsub = _val.convert_to< lkp_addsub_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__load_mux_pkt = _val.convert_to< load_mux_pkt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_offset_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 14;
    
}

void cap_ppa_decoders_lkp_val_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__store_en = _val.convert_to< store_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_extract_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pkt_idx = _val.convert_to< pkt_idx_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phv_idx = _val.convert_to< phv_idx_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_ppa_decoders_meta_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__phv_idx = _val.convert_to< phv_idx_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_ppa_decoders_ohi_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sel = _val.convert_to< sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__idx_val = _val.convert_to< idx_val_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__slot_num = _val.convert_to< slot_num_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_ppa_decoders_crc_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__en = _val.convert_to< en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prof_sel_en = _val.convert_to< prof_sel_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ohi_mask_sel = _val.convert_to< ohi_mask_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_ppa_decoders_csum_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__en = _val.convert_to< en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__unit_sel = _val.convert_to< unit_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__phdr_en = _val.convert_to< phdr_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phdr_sel = _val.convert_to< phdr_sel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phdr_ohi_sel = _val.convert_to< phdr_ohi_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__dis_zero = _val.convert_to< dis_zero_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__load_phdr_prof_en = _val.convert_to< load_phdr_prof_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_len_chk_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__en = _val.convert_to< en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__unit_sel = _val.convert_to< unit_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__exact = _val.convert_to< exact_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_align_chk_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__en = _val.convert_to< en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_ppa_decoders_ppa_lkp_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__action = _val.convert_to< action_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nxt_state = _val.convert_to< nxt_state_cpp_int_t >()  ;
    _val = _val >> 9;
    
    offset_inst.all( _val);
    _val = _val >> offset_inst.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_idx
    for(int ii = 0; ii < 4; ii++) {
        mux_idx[ii].all(_val);
        _val = _val >> mux_idx[ii].get_width();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].all(_val);
        _val = _val >> mux_inst[ii].get_width();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val_inst
    for(int ii = 0; ii < 3; ii++) {
        lkp_val_inst[ii].all(_val);
        _val = _val >> lkp_val_inst[ii].get_width();
    }
    #endif
    
    int_var__phv_idx_upr = _val.convert_to< phv_idx_upr_cpp_int_t >()  ;
    _val = _val >> 3;
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].all(_val);
        _val = _val >> extract_inst[ii].get_width();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].all(_val);
        _val = _val >> meta_inst[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // ohi_inst
    for(int ii = 0; ii < 4; ii++) {
        ohi_inst[ii].all(_val);
        _val = _val >> ohi_inst[ii].get_width();
    }
    #endif
    
    crc_inst.all( _val);
    _val = _val >> crc_inst.get_width(); 
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 0; ii < 2; ii++) {
        csum_inst[ii].all(_val);
        _val = _val >> csum_inst[ii].get_width();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        len_chk_inst[ii].all(_val);
        _val = _val >> len_chk_inst[ii].get_width();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].all(_val);
        _val = _val >> align_chk_inst[ii].get_width();
    }
    #endif
    
    int_var__offset_jump_chk_en = _val.convert_to< offset_jump_chk_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_size_chk_en = _val.convert_to< pkt_size_chk_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_size = _val.convert_to< pkt_size_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_ppa_decoders_chk_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__last_offset = _val.convert_to< last_offset_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__seq_id = _val.convert_to< seq_id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__err_vec = _val.convert_to< err_vec_cpp_int_t >()  ;
    _val = _val >> 7;
    
    crc_inst.all( _val);
    _val = _val >> crc_inst.get_width(); 
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 0; ii < 5; ii++) {
        csum_inst[ii].all(_val);
        _val = _val >> csum_inst[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 0; ii < 4; ii++) {
        len_chk_inst[ii].all(_val);
        _val = _val >> len_chk_inst[ii].get_width();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].all(_val);
        _val = _val >> align_chk_inst[ii].get_width();
    }
    #endif
    
}

void cap_ppa_decoders_ppa_lkp_tcam_key_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__control = _val.convert_to< control_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__state = _val.convert_to< state_cpp_int_t >()  ;
    _val = _val >> 9;
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val
    for(int ii = 0; ii < 3; ii++) {
        int_var__lkp_val[ii] = _val.convert_to< lkp_val_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
}

void cap_ppa_decoders_ppa_lkp_tcam_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    key.all( _val);
    _val = _val >> key.get_width(); 
    mask.all( _val);
    _val = _val >> mask.get_width(); 
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_crc_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addsub_end = _val.convert_to< addsub_end_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addsub_mask = _val.convert_to< addsub_mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mask_adj = _val.convert_to< mask_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mask_prof_sel = _val.convert_to< mask_prof_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_ppa_decoders_crc_mask_prof_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mask_en = _val.convert_to< mask_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__use_ohi = _val.convert_to< use_ohi_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__fill = _val.convert_to< fill_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__skip_first_nibble = _val.convert_to< skip_first_nibble_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_crc_mask_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 6; ii++) {
        fld[ii].all(_val);
        _val = _val >> fld[ii].get_width();
    }
    #endif
    
}

void cap_ppa_decoders_csum_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__csum_8b = _val.convert_to< csum_8b_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_csum_flit_num = _val.convert_to< phv_csum_flit_num_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addsub_end = _val.convert_to< addsub_end_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addsub_phdr = _val.convert_to< addsub_phdr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phdr_adj = _val.convert_to< phdr_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addsub_csum_loc = _val.convert_to< addsub_csum_loc_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__csum_loc_adj = _val.convert_to< csum_loc_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__align = _val.convert_to< align_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__add_val = _val.convert_to< add_val_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_ppa_decoders_csum_phdr_prof_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fld_en = _val.convert_to< fld_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_align = _val.convert_to< fld_align_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start = _val.convert_to< fld_start_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__fld_end = _val.convert_to< fld_end_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_csum_phdr_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 4; ii++) {
        fld[ii].all(_val);
        _val = _val >> fld[ii].get_width();
    }
    #endif
    
}

void cap_ppa_decoders_csum_all_fld_inst_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fld_en = _val.convert_to< fld_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_align = _val.convert_to< fld_align_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start = _val.convert_to< fld_start_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__fld_end = _val.convert_to< fld_end_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_csum_flit_num = _val.convert_to< phv_csum_flit_num_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__csum_8b = _val.convert_to< csum_8b_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__add_val = _val.convert_to< add_val_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_ppa_decoders_csum_all_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 5; ii++) {
        fld[ii].all(_val);
        _val = _val >> fld[ii].get_width();
    }
    #endif
    
}

void cap_ppa_decoders_len_chk_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_ppa_decoders_pkt_ff_data_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__err = _val.convert_to< err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__eop = _val.convert_to< eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sop = _val.convert_to< sop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__seq_id = _val.convert_to< seq_id_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_ppa_decoders_align_chk_prof_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__mod_log2 = _val.convert_to< mod_log2_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_ppa_decoders_err_vec_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__offset_jump_chk = _val.convert_to< offset_jump_chk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_size_chk = _val.convert_to< pkt_size_chk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__offset_out_of_range = _val.convert_to< offset_out_of_range_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__exceed_parse_loop_cnt = _val.convert_to< exceed_parse_loop_cnt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__exceed_phv_flit_cnt = _val.convert_to< exceed_phv_flit_cnt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_upr_idx_less = _val.convert_to< phv_upr_idx_less_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__init_lkp_idx = _val.convert_to< init_lkp_idx_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ppa_decoders_si_mux_pkt_rslt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    inst.all( _val);
    _val = _val >> inst.get_width(); 
    int_var__mux_pkt = _val.convert_to< mux_pkt_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_ppa_decoders_si_mux_inst_rslt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    inst.all( _val);
    _val = _val >> inst.get_width(); 
    int_var__mux_pkt_inst = _val.convert_to< mux_pkt_inst_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_ppa_decoders_si_extract_rslt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    inst.all( _val);
    _val = _val >> inst.get_width(); 
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_ppa_decoders_si_meta_rslt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    inst.all( _val);
    _val = _val >> inst.get_width(); 
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_ppa_decoders_si_state_rslt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt
    for(int ii = 0; ii < 4; ii++) {
        mux_pkt[ii].all(_val);
        _val = _val >> mux_pkt[ii].get_width();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].all(_val);
        _val = _val >> mux_inst[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].all(_val);
        _val = _val >> extract_inst[ii].get_width();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].all(_val);
        _val = _val >> meta_inst[ii].get_width();
    }
    #endif
    
}

void cap_ppa_decoders_si_state_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    lkp_key.all( _val);
    _val = _val >> lkp_key.get_width(); 
    int_var__tcam_hit = _val.convert_to< tcam_hit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tcam_hit_idx = _val.convert_to< tcam_hit_idx_cpp_int_t >()  ;
    _val = _val >> 10;
    
    state_rslt.all( _val);
    _val = _val >> state_rslt.get_width(); 
    int_var__flit_idx = _val.convert_to< flit_idx_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_ppa_decoders_si_pe_step_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__init_profile_idx = _val.convert_to< init_profile_idx_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phv_global_csum_err = _val.convert_to< phv_global_csum_err_cpp_int_t >()  ;
    _val = _val >> 5;
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_cur_state
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_cur_state[ii] = _val.convert_to< lkp_cur_state_cpp_int_t >();
        _val = _val >> 9;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val0
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val0[ii] = _val.convert_to< stored_lkp_val0_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val0
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val0[ii] = _val.convert_to< lkp_val0_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val1
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val1[ii] = _val.convert_to< stored_lkp_val1_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val1
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val1[ii] = _val.convert_to< lkp_val1_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val2
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val2[ii] = _val.convert_to< stored_lkp_val2_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val2
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val2[ii] = _val.convert_to< lkp_val2_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tcam_hit
    for(int ii = 0; ii < 100; ii++) {
        int_var__tcam_hit[ii] = _val.convert_to< tcam_hit_cpp_int_t >();
        _val = _val >> 1;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tcam_hit_idx
    for(int ii = 0; ii < 100; ii++) {
        int_var__tcam_hit_idx[ii] = _val.convert_to< tcam_hit_idx_cpp_int_t >();
        _val = _val >> 10;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cur_flit_idx
    for(int ii = 0; ii < 100; ii++) {
        int_var__cur_flit_idx[ii] = _val.convert_to< cur_flit_idx_cpp_int_t >();
        _val = _val >> 3;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt0
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt0[ii] = _val.convert_to< mux_pkt0_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt1
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt1[ii] = _val.convert_to< mux_pkt1_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt2
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt2[ii] = _val.convert_to< mux_pkt2_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt3
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt3[ii] = _val.convert_to< mux_pkt3_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data0
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data0[ii] = _val.convert_to< mux_inst_data0_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data1
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data1[ii] = _val.convert_to< mux_inst_data1_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data2
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data2[ii] = _val.convert_to< mux_inst_data2_cpp_int_t >();
        _val = _val >> 16;
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_phv_data
    for(int ii = 0; ii < 100; ii++) {
        int_var__extract_phv_data[ii] = _val.convert_to< extract_phv_data_cpp_int_t >();
        _val = _val >> 1024;
    }
    #endif
    
}

void cap_ppa_decoders_si_non_pe_step_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__preparser_csum_err_out = _val.convert_to< preparser_csum_err_out_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__parse_end_ptr = _val.convert_to< parse_end_ptr_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__err_vec = _val.convert_to< err_vec_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__err_code = _val.convert_to< err_code_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__crc_val = _val.convert_to< crc_val_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_val
    for(int ii = 0; ii < 5; ii++) {
        int_var__csum_val[ii] = _val.convert_to< csum_val_cpp_int_t >();
        _val = _val >> 32;
    }
    #endif
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_err
    for(int ii = 0; ii < 5; ii++) {
        int_var__csum_err[ii] = _val.convert_to< csum_err_cpp_int_t >();
        _val = _val >> 1;
    }
    #endif
    
    int_var__len_chk_val0 = _val.convert_to< len_chk_val0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__len_chk_err0 = _val.convert_to< len_chk_err0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_chk_val1 = _val.convert_to< len_chk_val1_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__len_chk_err1 = _val.convert_to< len_chk_err1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_chk_val2 = _val.convert_to< len_chk_val2_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__len_chk_err2 = _val.convert_to< len_chk_err2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_chk_val3 = _val.convert_to< len_chk_val3_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__len_chk_err3 = _val.convert_to< len_chk_err3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__align_chk_val0 = _val.convert_to< align_chk_val0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__align_chk_err0 = _val.convert_to< align_chk_err0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__align_chk_val1 = _val.convert_to< align_chk_val1_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__align_chk_err1 = _val.convert_to< align_chk_err1_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::all() const {
    cpp_int ret_val;

    // align_chk_err1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__align_chk_err1; 
    
    // align_chk_val1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__align_chk_val1; 
    
    // align_chk_err0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__align_chk_err0; 
    
    // align_chk_val0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__align_chk_val0; 
    
    // len_chk_err3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_chk_err3; 
    
    // len_chk_val3
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len_chk_val3; 
    
    // len_chk_err2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_chk_err2; 
    
    // len_chk_val2
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len_chk_val2; 
    
    // len_chk_err1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_chk_err1; 
    
    // len_chk_val1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len_chk_val1; 
    
    // len_chk_err0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_chk_err0; 
    
    // len_chk_val0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len_chk_val0; 
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_err
    for(int ii = 5-1; ii >= 0; ii--) {
        ret_val = ret_val << 1; ret_val = ret_val  | int_var__csum_err[ii]; 
    }
    #endif
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_val
    for(int ii = 5-1; ii >= 0; ii--) {
        ret_val = ret_val << 32; ret_val = ret_val  | int_var__csum_val[ii]; 
    }
    #endif
    
    // crc_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_err; 
    
    // crc_val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__crc_val; 
    
    // err_code
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__err_code; 
    
    // err_vec
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__err_vec; 
    
    // parse_end_ptr
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__parse_end_ptr; 
    
    // preparser_csum_err_out
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__preparser_csum_err_out; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::all() const {
    cpp_int ret_val;

    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_phv_data
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 1024; ret_val = ret_val  | int_var__extract_phv_data[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data2
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_inst_data2[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data1
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_inst_data1[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst_data0
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_inst_data0[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt3
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt3[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt2
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt2[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt1
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt1[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt0
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt0[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cur_flit_idx
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 3; ret_val = ret_val  | int_var__cur_flit_idx[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tcam_hit_idx
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 10; ret_val = ret_val  | int_var__tcam_hit_idx[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tcam_hit
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_hit[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val2
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__lkp_val2[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val2
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__stored_lkp_val2[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val1
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__lkp_val1[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val1
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__stored_lkp_val1[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val0
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__lkp_val0[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // stored_lkp_val0
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__stored_lkp_val0[ii]; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_cur_state
    for(int ii = 100-1; ii >= 0; ii--) {
        ret_val = ret_val << 9; ret_val = ret_val  | int_var__lkp_cur_state[ii]; 
    }
    #endif
    
    // phv_global_csum_err
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__phv_global_csum_err; 
    
    // init_profile_idx
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__init_profile_idx; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_si_state_info_t::all() const {
    cpp_int ret_val;

    // flit_idx
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__flit_idx; 
    
    ret_val = ret_val << state_rslt.get_width(); ret_val = ret_val  | state_rslt.all(); 
    // tcam_hit_idx
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__tcam_hit_idx; 
    
    // tcam_hit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tcam_hit; 
    
    ret_val = ret_val << lkp_key.get_width(); ret_val = ret_val  | lkp_key.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_si_state_rslt_t::all() const {
    cpp_int ret_val;

    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 3-1; ii >= 0; ii--) {
         ret_val = ret_val << meta_inst[ii].get_width(); ret_val = ret_val  | meta_inst[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << extract_inst[ii].get_width(); ret_val = ret_val  | extract_inst[ii].all(); 
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 3-1; ii >= 0; ii--) {
         ret_val = ret_val << mux_inst[ii].get_width(); ret_val = ret_val  | mux_inst[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_pkt
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << mux_pkt[ii].get_width(); ret_val = ret_val  | mux_pkt[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_ppa_decoders_si_meta_rslt_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    ret_val = ret_val << inst.get_width(); ret_val = ret_val  | inst.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_si_extract_rslt_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    ret_val = ret_val << inst.get_width(); ret_val = ret_val  | inst.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_si_mux_inst_rslt_t::all() const {
    cpp_int ret_val;

    // mux_pkt_inst
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt_inst; 
    
    ret_val = ret_val << inst.get_width(); ret_val = ret_val  | inst.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_si_mux_pkt_rslt_t::all() const {
    cpp_int ret_val;

    // mux_pkt
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__mux_pkt; 
    
    ret_val = ret_val << inst.get_width(); ret_val = ret_val  | inst.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_err_vec_t::all() const {
    cpp_int ret_val;

    // init_lkp_idx
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__init_lkp_idx; 
    
    // phv_upr_idx_less
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phv_upr_idx_less; 
    
    // exceed_phv_flit_cnt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__exceed_phv_flit_cnt; 
    
    // exceed_parse_loop_cnt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__exceed_parse_loop_cnt; 
    
    // offset_out_of_range
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__offset_out_of_range; 
    
    // pkt_size_chk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkt_size_chk; 
    
    // offset_jump_chk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__offset_jump_chk; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_align_chk_prof_t::all() const {
    cpp_int ret_val;

    // mod_log2
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__mod_log2; 
    
    // start_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__start_adj; 
    
    // addsub_start
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_start; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::all() const {
    cpp_int ret_val;

    // seq_id
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__seq_id; 
    
    // sop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sop; 
    
    // eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__eop; 
    
    // err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__err; 
    
    // sz
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__sz; 
    
    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_len_chk_prof_t::all() const {
    cpp_int ret_val;

    // start_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__start_adj; 
    
    // addsub_start
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_start; 
    
    // len_shift_val
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__len_shift_val; 
    
    // len_shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_shift_left; 
    
    // len_mask
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__len_mask; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_all_prof_t::all() const {
    cpp_int ret_val;

    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 5-1; ii >= 0; ii--) {
         ret_val = ret_val << fld[ii].get_width(); ret_val = ret_val  | fld[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::all() const {
    cpp_int ret_val;

    // add_val
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__add_val; 
    
    // csum_8b
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__csum_8b; 
    
    // phv_csum_flit_num
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phv_csum_flit_num; 
    
    // end_eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__end_eop; 
    
    // add_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__add_len; 
    
    // fld_end
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__fld_end; 
    
    // fld_start
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__fld_start; 
    
    // fld_align
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align; 
    
    // fld_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_phdr_prof_t::all() const {
    cpp_int ret_val;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << fld[ii].get_width(); ret_val = ret_val  | fld[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::all() const {
    cpp_int ret_val;

    // add_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__add_len; 
    
    // fld_end
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__fld_end; 
    
    // fld_start
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__fld_start; 
    
    // fld_align
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align; 
    
    // fld_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_prof_t::all() const {
    cpp_int ret_val;

    // add_val
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__add_val; 
    
    // align
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__align; 
    
    // csum_loc_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__csum_loc_adj; 
    
    // addsub_csum_loc
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_csum_loc; 
    
    // phdr_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__phdr_adj; 
    
    // addsub_phdr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_phdr; 
    
    // end_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__end_adj; 
    
    // addsub_end
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_end; 
    
    // start_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__start_adj; 
    
    // addsub_start
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_start; 
    
    // len_shift_val
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__len_shift_val; 
    
    // len_shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_shift_left; 
    
    // len_mask
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__len_mask; 
    
    // end_eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__end_eop; 
    
    // phv_csum_flit_num
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phv_csum_flit_num; 
    
    // csum_8b
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__csum_8b; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_crc_mask_prof_t::all() const {
    cpp_int ret_val;

    #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 6-1; ii >= 0; ii--) {
         ret_val = ret_val << fld[ii].get_width(); ret_val = ret_val  | fld[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::all() const {
    cpp_int ret_val;

    // skip_first_nibble
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble; 
    
    // fill
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fill; 
    
    // end_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__end_adj; 
    
    // start_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__start_adj; 
    
    // use_ohi
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_ohi; 
    
    // mask_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask_en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_crc_prof_t::all() const {
    cpp_int ret_val;

    // mask_prof_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__mask_prof_sel; 
    
    // end_eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__end_eop; 
    
    // mask_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__mask_adj; 
    
    // addsub_mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_mask; 
    
    // end_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__end_adj; 
    
    // addsub_end
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_end; 
    
    // start_adj
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__start_adj; 
    
    // addsub_start
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub_start; 
    
    // len_shift_val
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__len_shift_val; 
    
    // len_shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_shift_left; 
    
    // len_mask
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__len_mask; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_entry_t::all() const {
    cpp_int ret_val;

    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    ret_val = ret_val << mask.get_width(); ret_val = ret_val  | mask.all(); 
    ret_val = ret_val << key.get_width(); ret_val = ret_val  | key.all(); 
    return ret_val;
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_key_t::all() const {
    cpp_int ret_val;

    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val
    for(int ii = 3-1; ii >= 0; ii--) {
        ret_val = ret_val << 16; ret_val = ret_val  | int_var__lkp_val[ii]; 
    }
    #endif
    
    // state
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__state; 
    
    // control
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__control; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_chk_ctl_t::all() const {
    cpp_int ret_val;

    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 2-1; ii >= 0; ii--) {
         ret_val = ret_val << align_chk_inst[ii].get_width(); ret_val = ret_val  | align_chk_inst[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << len_chk_inst[ii].get_width(); ret_val = ret_val  | len_chk_inst[ii].all(); 
    }
    #endif
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 5-1; ii >= 0; ii--) {
         ret_val = ret_val << csum_inst[ii].get_width(); ret_val = ret_val  | csum_inst[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << crc_inst.get_width(); ret_val = ret_val  | crc_inst.all(); 
    // err_vec
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__err_vec; 
    
    // seq_id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__seq_id; 
    
    // last_offset
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__last_offset; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::all() const {
    cpp_int ret_val;

    // pkt_size
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__pkt_size; 
    
    // pkt_size_chk_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkt_size_chk_en; 
    
    // offset_jump_chk_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__offset_jump_chk_en; 
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 2-1; ii >= 0; ii--) {
         ret_val = ret_val << align_chk_inst[ii].get_width(); ret_val = ret_val  | align_chk_inst[ii].all(); 
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 2-1; ii >= 0; ii--) {
         ret_val = ret_val << len_chk_inst[ii].get_width(); ret_val = ret_val  | len_chk_inst[ii].all(); 
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 2-1; ii >= 0; ii--) {
         ret_val = ret_val << csum_inst[ii].get_width(); ret_val = ret_val  | csum_inst[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << crc_inst.get_width(); ret_val = ret_val  | crc_inst.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // ohi_inst
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << ohi_inst[ii].get_width(); ret_val = ret_val  | ohi_inst[ii].all(); 
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 3-1; ii >= 0; ii--) {
         ret_val = ret_val << meta_inst[ii].get_width(); ret_val = ret_val  | meta_inst[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << extract_inst[ii].get_width(); ret_val = ret_val  | extract_inst[ii].all(); 
    }
    #endif
    
    // phv_idx_upr
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phv_idx_upr; 
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // lkp_val_inst
    for(int ii = 3-1; ii >= 0; ii--) {
         ret_val = ret_val << lkp_val_inst[ii].get_width(); ret_val = ret_val  | lkp_val_inst[ii].all(); 
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 3-1; ii >= 0; ii--) {
         ret_val = ret_val << mux_inst[ii].get_width(); ret_val = ret_val  | mux_inst[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mux_idx
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << mux_idx[ii].get_width(); ret_val = ret_val  | mux_idx[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << offset_inst.get_width(); ret_val = ret_val  | offset_inst.all(); 
    // nxt_state
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__nxt_state; 
    
    // action
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__action; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_align_chk_inst_t::all() const {
    cpp_int ret_val;

    // ohi_start_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_start_sel; 
    
    // prof_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__prof_sel; 
    
    // en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_len_chk_inst_t::all() const {
    cpp_int ret_val;

    // exact
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__exact; 
    
    // ohi_len_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_len_sel; 
    
    // ohi_start_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_start_sel; 
    
    // prof_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__prof_sel; 
    
    // unit_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__unit_sel; 
    
    // en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_csum_inst_t::all() const {
    cpp_int ret_val;

    // load_phdr_prof_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__load_phdr_prof_en; 
    
    // dis_zero
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dis_zero; 
    
    // phdr_ohi_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__phdr_ohi_sel; 
    
    // phdr_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phdr_sel; 
    
    // phdr_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phdr_en; 
    
    // ohi_len_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_len_sel; 
    
    // ohi_start_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_start_sel; 
    
    // prof_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prof_sel; 
    
    // unit_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__unit_sel; 
    
    // en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_crc_inst_t::all() const {
    cpp_int ret_val;

    // ohi_mask_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_mask_sel; 
    
    // ohi_len_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_len_sel; 
    
    // ohi_start_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ohi_start_sel; 
    
    // prof_sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prof_sel; 
    
    // prof_sel_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__prof_sel_en; 
    
    // en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_ohi_inst_t::all() const {
    cpp_int ret_val;

    // slot_num
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__slot_num; 
    
    // idx_val
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__idx_val; 
    
    // muxsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__muxsel; 
    
    // sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sel; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_meta_inst_t::all() const {
    cpp_int ret_val;

    // sel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sel; 
    
    // val
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__val; 
    
    // phv_idx
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__phv_idx; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_extract_inst_t::all() const {
    cpp_int ret_val;

    // phv_idx
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__phv_idx; 
    
    // len
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__len; 
    
    // pkt_idx
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__pkt_idx; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_lkp_val_inst_t::all() const {
    cpp_int ret_val;

    // store_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__store_en; 
    
    // muxsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__muxsel; 
    
    // sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__sel; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_offset_inst_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__val; 
    
    // muxsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__muxsel; 
    
    // sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sel; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_mux_inst_t::all() const {
    cpp_int ret_val;

    // load_mux_pkt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__load_mux_pkt; 
    
    // lkp_addsub
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lkp_addsub; 
    
    // lkpsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__lkpsel; 
    
    // addsub
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addsub; 
    
    // shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__shift_left; 
    
    // addsub_val
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addsub_val; 
    
    // shift_val
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__shift_val; 
    
    // mask_val
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__mask_val; 
    
    // muxsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__muxsel; 
    
    // sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sel; 
    
    return ret_val;
}

cpp_int cap_ppa_decoders_mux_idx_t::all() const {
    cpp_int ret_val;

    // load_stored_lkp
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__load_stored_lkp; 
    
    // idx
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__idx; 
    
    // lkpsel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__lkpsel; 
    
    // sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__sel; 
    
    return ret_val;
}

void cap_ppa_decoders_mux_idx_t::clear() {

    int_var__sel = 0; 
    
    int_var__lkpsel = 0; 
    
    int_var__idx = 0; 
    
    int_var__load_stored_lkp = 0; 
    
}

void cap_ppa_decoders_mux_inst_t::clear() {

    int_var__sel = 0; 
    
    int_var__muxsel = 0; 
    
    int_var__mask_val = 0; 
    
    int_var__shift_val = 0; 
    
    int_var__addsub_val = 0; 
    
    int_var__shift_left = 0; 
    
    int_var__addsub = 0; 
    
    int_var__lkpsel = 0; 
    
    int_var__lkp_addsub = 0; 
    
    int_var__load_mux_pkt = 0; 
    
}

void cap_ppa_decoders_offset_inst_t::clear() {

    int_var__sel = 0; 
    
    int_var__muxsel = 0; 
    
    int_var__val = 0; 
    
}

void cap_ppa_decoders_lkp_val_inst_t::clear() {

    int_var__sel = 0; 
    
    int_var__muxsel = 0; 
    
    int_var__store_en = 0; 
    
}

void cap_ppa_decoders_extract_inst_t::clear() {

    int_var__pkt_idx = 0; 
    
    int_var__len = 0; 
    
    int_var__phv_idx = 0; 
    
}

void cap_ppa_decoders_meta_inst_t::clear() {

    int_var__phv_idx = 0; 
    
    int_var__val = 0; 
    
    int_var__sel = 0; 
    
}

void cap_ppa_decoders_ohi_inst_t::clear() {

    int_var__sel = 0; 
    
    int_var__muxsel = 0; 
    
    int_var__idx_val = 0; 
    
    int_var__slot_num = 0; 
    
}

void cap_ppa_decoders_crc_inst_t::clear() {

    int_var__en = 0; 
    
    int_var__prof_sel_en = 0; 
    
    int_var__prof_sel = 0; 
    
    int_var__ohi_start_sel = 0; 
    
    int_var__ohi_len_sel = 0; 
    
    int_var__ohi_mask_sel = 0; 
    
}

void cap_ppa_decoders_csum_inst_t::clear() {

    int_var__en = 0; 
    
    int_var__unit_sel = 0; 
    
    int_var__prof_sel = 0; 
    
    int_var__ohi_start_sel = 0; 
    
    int_var__ohi_len_sel = 0; 
    
    int_var__phdr_en = 0; 
    
    int_var__phdr_sel = 0; 
    
    int_var__phdr_ohi_sel = 0; 
    
    int_var__dis_zero = 0; 
    
    int_var__load_phdr_prof_en = 0; 
    
}

void cap_ppa_decoders_len_chk_inst_t::clear() {

    int_var__en = 0; 
    
    int_var__unit_sel = 0; 
    
    int_var__prof_sel = 0; 
    
    int_var__ohi_start_sel = 0; 
    
    int_var__ohi_len_sel = 0; 
    
    int_var__exact = 0; 
    
}

void cap_ppa_decoders_align_chk_inst_t::clear() {

    int_var__en = 0; 
    
    int_var__prof_sel = 0; 
    
    int_var__ohi_start_sel = 0; 
    
}

void cap_ppa_decoders_ppa_lkp_sram_entry_t::clear() {

    int_var__action = 0; 
    
    int_var__nxt_state = 0; 
    
    offset_inst.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // mux_idx
    for(int ii = 0; ii < 4; ii++) {
        mux_idx[ii].clear();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].clear();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // lkp_val_inst
    for(int ii = 0; ii < 3; ii++) {
        lkp_val_inst[ii].clear();
    }
    #endif
    
    int_var__phv_idx_upr = 0; 
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].clear();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // ohi_inst
    for(int ii = 0; ii < 4; ii++) {
        ohi_inst[ii].clear();
    }
    #endif
    
    crc_inst.clear();
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 0; ii < 2; ii++) {
        csum_inst[ii].clear();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        len_chk_inst[ii].clear();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].clear();
    }
    #endif
    
    int_var__offset_jump_chk_en = 0; 
    
    int_var__pkt_size_chk_en = 0; 
    
    int_var__pkt_size = 0; 
    
}

void cap_ppa_decoders_chk_ctl_t::clear() {

    int_var__last_offset = 0; 
    
    int_var__seq_id = 0; 
    
    int_var__err_vec = 0; 
    
    crc_inst.clear();
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // csum_inst
    for(int ii = 0; ii < 5; ii++) {
        csum_inst[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // len_chk_inst
    for(int ii = 0; ii < 4; ii++) {
        len_chk_inst[ii].clear();
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // align_chk_inst
    for(int ii = 0; ii < 2; ii++) {
        align_chk_inst[ii].clear();
    }
    #endif
    
}

void cap_ppa_decoders_ppa_lkp_tcam_key_t::clear() {

    int_var__control = 0; 
    
    int_var__state = 0; 
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 3; ii++) {
        int_var__lkp_val[ii] = 0; 
    }
    #endif
    
}

void cap_ppa_decoders_ppa_lkp_tcam_entry_t::clear() {

    key.clear();
    mask.clear();
    int_var__valid = 0; 
    
}

void cap_ppa_decoders_crc_prof_t::clear() {

    int_var__len_mask = 0; 
    
    int_var__len_shift_left = 0; 
    
    int_var__len_shift_val = 0; 
    
    int_var__addsub_start = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__addsub_end = 0; 
    
    int_var__end_adj = 0; 
    
    int_var__addsub_mask = 0; 
    
    int_var__mask_adj = 0; 
    
    int_var__end_eop = 0; 
    
    int_var__mask_prof_sel = 0; 
    
}

void cap_ppa_decoders_crc_mask_prof_inst_t::clear() {

    int_var__mask_en = 0; 
    
    int_var__use_ohi = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__end_adj = 0; 
    
    int_var__fill = 0; 
    
    int_var__skip_first_nibble = 0; 
    
}

void cap_ppa_decoders_crc_mask_prof_t::clear() {

    #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 6; ii++) {
        fld[ii].clear();
    }
    #endif
    
}

void cap_ppa_decoders_csum_prof_t::clear() {

    int_var__csum_8b = 0; 
    
    int_var__phv_csum_flit_num = 0; 
    
    int_var__end_eop = 0; 
    
    int_var__len_mask = 0; 
    
    int_var__len_shift_left = 0; 
    
    int_var__len_shift_val = 0; 
    
    int_var__addsub_start = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__addsub_end = 0; 
    
    int_var__end_adj = 0; 
    
    int_var__addsub_phdr = 0; 
    
    int_var__phdr_adj = 0; 
    
    int_var__addsub_csum_loc = 0; 
    
    int_var__csum_loc_adj = 0; 
    
    int_var__align = 0; 
    
    int_var__add_val = 0; 
    
}

void cap_ppa_decoders_csum_phdr_prof_inst_t::clear() {

    int_var__fld_en = 0; 
    
    int_var__fld_align = 0; 
    
    int_var__fld_start = 0; 
    
    int_var__fld_end = 0; 
    
    int_var__add_len = 0; 
    
}

void cap_ppa_decoders_csum_phdr_prof_t::clear() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 4; ii++) {
        fld[ii].clear();
    }
    #endif
    
}

void cap_ppa_decoders_csum_all_fld_inst_t::clear() {

    int_var__fld_en = 0; 
    
    int_var__fld_align = 0; 
    
    int_var__fld_start = 0; 
    
    int_var__fld_end = 0; 
    
    int_var__add_len = 0; 
    
    int_var__end_eop = 0; 
    
    int_var__phv_csum_flit_num = 0; 
    
    int_var__csum_8b = 0; 
    
    int_var__add_val = 0; 
    
}

void cap_ppa_decoders_csum_all_prof_t::clear() {

    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // fld
    for(int ii = 0; ii < 5; ii++) {
        fld[ii].clear();
    }
    #endif
    
}

void cap_ppa_decoders_len_chk_prof_t::clear() {

    int_var__len_mask = 0; 
    
    int_var__len_shift_left = 0; 
    
    int_var__len_shift_val = 0; 
    
    int_var__addsub_start = 0; 
    
    int_var__start_adj = 0; 
    
}

void cap_ppa_decoders_pkt_ff_data_t::clear() {

    int_var__data = 0; 
    
    int_var__sz = 0; 
    
    int_var__err = 0; 
    
    int_var__eop = 0; 
    
    int_var__sop = 0; 
    
    int_var__seq_id = 0; 
    
}

void cap_ppa_decoders_align_chk_prof_t::clear() {

    int_var__addsub_start = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__mod_log2 = 0; 
    
}

void cap_ppa_decoders_err_vec_t::clear() {

    int_var__offset_jump_chk = 0; 
    
    int_var__pkt_size_chk = 0; 
    
    int_var__offset_out_of_range = 0; 
    
    int_var__exceed_parse_loop_cnt = 0; 
    
    int_var__exceed_phv_flit_cnt = 0; 
    
    int_var__phv_upr_idx_less = 0; 
    
    int_var__init_lkp_idx = 0; 
    
}

void cap_ppa_decoders_si_mux_pkt_rslt_t::clear() {

    inst.clear();
    int_var__mux_pkt = 0; 
    
}

void cap_ppa_decoders_si_mux_inst_rslt_t::clear() {

    inst.clear();
    int_var__mux_pkt_inst = 0; 
    
}

void cap_ppa_decoders_si_extract_rslt_t::clear() {

    inst.clear();
    int_var__data = 0; 
    
}

void cap_ppa_decoders_si_meta_rslt_t::clear() {

    inst.clear();
    int_var__data = 0; 
    
}

void cap_ppa_decoders_si_state_rslt_t::clear() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // mux_pkt
    for(int ii = 0; ii < 4; ii++) {
        mux_pkt[ii].clear();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // mux_inst
    for(int ii = 0; ii < 3; ii++) {
        mux_inst[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // extract_inst
    for(int ii = 0; ii < 16; ii++) {
        extract_inst[ii].clear();
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // meta_inst
    for(int ii = 0; ii < 3; ii++) {
        meta_inst[ii].clear();
    }
    #endif
    
}

void cap_ppa_decoders_si_state_info_t::clear() {

    lkp_key.clear();
    int_var__tcam_hit = 0; 
    
    int_var__tcam_hit_idx = 0; 
    
    state_rslt.clear();
    int_var__flit_idx = 0; 
    
}

void cap_ppa_decoders_si_pe_step_info_t::clear() {

    int_var__init_profile_idx = 0; 
    
    int_var__phv_global_csum_err = 0; 
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_cur_state[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val0[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val0[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val1[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val1[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__stored_lkp_val2[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__lkp_val2[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__tcam_hit[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__tcam_hit_idx[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__cur_flit_idx[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt0[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt1[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt2[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_pkt3[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data0[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data1[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__mux_inst_data2[ii] = 0; 
    }
    #endif
    
    #if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 100; ii++) {
        int_var__extract_phv_data[ii] = 0; 
    }
    #endif
    
}

void cap_ppa_decoders_si_non_pe_step_info_t::clear() {

    int_var__preparser_csum_err_out = 0; 
    
    int_var__parse_end_ptr = 0; 
    
    int_var__err_vec = 0; 
    
    int_var__err_code = 0; 
    
    int_var__crc_val = 0; 
    
    int_var__crc_err = 0; 
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 5; ii++) {
        int_var__csum_val[ii] = 0; 
    }
    #endif
    
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 5; ii++) {
        int_var__csum_err[ii] = 0; 
    }
    #endif
    
    int_var__len_chk_val0 = 0; 
    
    int_var__len_chk_err0 = 0; 
    
    int_var__len_chk_val1 = 0; 
    
    int_var__len_chk_err1 = 0; 
    
    int_var__len_chk_val2 = 0; 
    
    int_var__len_chk_err2 = 0; 
    
    int_var__len_chk_val3 = 0; 
    
    int_var__len_chk_err3 = 0; 
    
    int_var__align_chk_val0 = 0; 
    
    int_var__align_chk_err0 = 0; 
    
    int_var__align_chk_val1 = 0; 
    
    int_var__align_chk_err1 = 0; 
    
}

void cap_ppa_decoders_mux_idx_t::init() {

}

void cap_ppa_decoders_mux_inst_t::init() {

}

void cap_ppa_decoders_offset_inst_t::init() {

}

void cap_ppa_decoders_lkp_val_inst_t::init() {

}

void cap_ppa_decoders_extract_inst_t::init() {

}

void cap_ppa_decoders_meta_inst_t::init() {

}

void cap_ppa_decoders_ohi_inst_t::init() {

}

void cap_ppa_decoders_crc_inst_t::init() {

}

void cap_ppa_decoders_csum_inst_t::init() {

}

void cap_ppa_decoders_len_chk_inst_t::init() {

}

void cap_ppa_decoders_align_chk_inst_t::init() {

}

void cap_ppa_decoders_ppa_lkp_sram_entry_t::init() {

    offset_inst.set_attributes(this,"offset_inst", 0x0 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_idx.set_attributes(this, "mux_idx", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) mux_idx[ii].set_field_init_done(true, true);
        mux_idx[ii].set_attributes(this,"mux_idx["+to_string(ii)+"]",  0x0 + (mux_idx[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_inst.set_attributes(this, "mux_inst", 0x0);
    #else
    for(int ii = 0; ii < 3; ii++) {
        if(ii != 0) mux_inst[ii].set_field_init_done(true, true);
        mux_inst[ii].set_attributes(this,"mux_inst["+to_string(ii)+"]",  0x0 + (mux_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    lkp_val_inst.set_attributes(this, "lkp_val_inst", 0x0);
    #else
    for(int ii = 0; ii < 3; ii++) {
        if(ii != 0) lkp_val_inst[ii].set_field_init_done(true, true);
        lkp_val_inst[ii].set_attributes(this,"lkp_val_inst["+to_string(ii)+"]",  0x0 + (lkp_val_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    extract_inst.set_attributes(this, "extract_inst", 0x0);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) extract_inst[ii].set_field_init_done(true, true);
        extract_inst[ii].set_attributes(this,"extract_inst["+to_string(ii)+"]",  0x0 + (extract_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    meta_inst.set_attributes(this, "meta_inst", 0x0);
    #else
    for(int ii = 0; ii < 3; ii++) {
        if(ii != 0) meta_inst[ii].set_field_init_done(true, true);
        meta_inst[ii].set_attributes(this,"meta_inst["+to_string(ii)+"]",  0x0 + (meta_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    ohi_inst.set_attributes(this, "ohi_inst", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) ohi_inst[ii].set_field_init_done(true, true);
        ohi_inst[ii].set_attributes(this,"ohi_inst["+to_string(ii)+"]",  0x0 + (ohi_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    crc_inst.set_attributes(this,"crc_inst", 0x0 );
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    csum_inst.set_attributes(this, "csum_inst", 0x0);
    #else
    for(int ii = 0; ii < 2; ii++) {
        if(ii != 0) csum_inst[ii].set_field_init_done(true, true);
        csum_inst[ii].set_attributes(this,"csum_inst["+to_string(ii)+"]",  0x0 + (csum_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    len_chk_inst.set_attributes(this, "len_chk_inst", 0x0);
    #else
    for(int ii = 0; ii < 2; ii++) {
        if(ii != 0) len_chk_inst[ii].set_field_init_done(true, true);
        len_chk_inst[ii].set_attributes(this,"len_chk_inst["+to_string(ii)+"]",  0x0 + (len_chk_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    align_chk_inst.set_attributes(this, "align_chk_inst", 0x0);
    #else
    for(int ii = 0; ii < 2; ii++) {
        if(ii != 0) align_chk_inst[ii].set_field_init_done(true, true);
        align_chk_inst[ii].set_attributes(this,"align_chk_inst["+to_string(ii)+"]",  0x0 + (align_chk_inst[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_chk_ctl_t::init() {

    crc_inst.set_attributes(this,"crc_inst", 0x0 );
    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    csum_inst.set_attributes(this, "csum_inst", 0x0);
    #else
    for(int ii = 0; ii < 5; ii++) {
        if(ii != 0) csum_inst[ii].set_field_init_done(true, true);
        csum_inst[ii].set_attributes(this,"csum_inst["+to_string(ii)+"]",  0x0 + (csum_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    len_chk_inst.set_attributes(this, "len_chk_inst", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) len_chk_inst[ii].set_field_init_done(true, true);
        len_chk_inst[ii].set_attributes(this,"len_chk_inst["+to_string(ii)+"]",  0x0 + (len_chk_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    align_chk_inst.set_attributes(this, "align_chk_inst", 0x0);
    #else
    for(int ii = 0; ii < 2; ii++) {
        if(ii != 0) align_chk_inst[ii].set_field_init_done(true, true);
        align_chk_inst[ii].set_attributes(this,"align_chk_inst["+to_string(ii)+"]",  0x0 + (align_chk_inst[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_ppa_lkp_tcam_key_t::init() {

    
}

void cap_ppa_decoders_ppa_lkp_tcam_entry_t::init() {

    key.set_attributes(this,"key", 0x0 );
    mask.set_attributes(this,"mask", 0x0 );
}

void cap_ppa_decoders_crc_prof_t::init() {

}

void cap_ppa_decoders_crc_mask_prof_inst_t::init() {

}

void cap_ppa_decoders_crc_mask_prof_t::init() {

    #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.set_attributes(this, "fld", 0x0);
    #else
    for(int ii = 0; ii < 6; ii++) {
        if(ii != 0) fld[ii].set_field_init_done(true, true);
        fld[ii].set_attributes(this,"fld["+to_string(ii)+"]",  0x0 + (fld[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_csum_prof_t::init() {

}

void cap_ppa_decoders_csum_phdr_prof_inst_t::init() {

}

void cap_ppa_decoders_csum_phdr_prof_t::init() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.set_attributes(this, "fld", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) fld[ii].set_field_init_done(true, true);
        fld[ii].set_attributes(this,"fld["+to_string(ii)+"]",  0x0 + (fld[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_csum_all_fld_inst_t::init() {

}

void cap_ppa_decoders_csum_all_prof_t::init() {

    #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    fld.set_attributes(this, "fld", 0x0);
    #else
    for(int ii = 0; ii < 5; ii++) {
        if(ii != 0) fld[ii].set_field_init_done(true, true);
        fld[ii].set_attributes(this,"fld["+to_string(ii)+"]",  0x0 + (fld[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_len_chk_prof_t::init() {

}

void cap_ppa_decoders_pkt_ff_data_t::init() {

}

void cap_ppa_decoders_align_chk_prof_t::init() {

}

void cap_ppa_decoders_err_vec_t::init() {

}

void cap_ppa_decoders_si_mux_pkt_rslt_t::init() {

    inst.set_attributes(this,"inst", 0x0 );
}

void cap_ppa_decoders_si_mux_inst_rslt_t::init() {

    inst.set_attributes(this,"inst", 0x0 );
}

void cap_ppa_decoders_si_extract_rslt_t::init() {

    inst.set_attributes(this,"inst", 0x0 );
}

void cap_ppa_decoders_si_meta_rslt_t::init() {

    inst.set_attributes(this,"inst", 0x0 );
}

void cap_ppa_decoders_si_state_rslt_t::init() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_pkt.set_attributes(this, "mux_pkt", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) mux_pkt[ii].set_field_init_done(true, true);
        mux_pkt[ii].set_attributes(this,"mux_pkt["+to_string(ii)+"]",  0x0 + (mux_pkt[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mux_inst.set_attributes(this, "mux_inst", 0x0);
    #else
    for(int ii = 0; ii < 3; ii++) {
        if(ii != 0) mux_inst[ii].set_field_init_done(true, true);
        mux_inst[ii].set_attributes(this,"mux_inst["+to_string(ii)+"]",  0x0 + (mux_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    extract_inst.set_attributes(this, "extract_inst", 0x0);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) extract_inst[ii].set_field_init_done(true, true);
        extract_inst[ii].set_attributes(this,"extract_inst["+to_string(ii)+"]",  0x0 + (extract_inst[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    meta_inst.set_attributes(this, "meta_inst", 0x0);
    #else
    for(int ii = 0; ii < 3; ii++) {
        if(ii != 0) meta_inst[ii].set_field_init_done(true, true);
        meta_inst[ii].set_attributes(this,"meta_inst["+to_string(ii)+"]",  0x0 + (meta_inst[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_ppa_decoders_si_state_info_t::init() {

    lkp_key.set_attributes(this,"lkp_key", 0x0 );
    state_rslt.set_attributes(this,"state_rslt", 0x0 );
}

void cap_ppa_decoders_si_pe_step_info_t::init() {

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

void cap_ppa_decoders_si_non_pe_step_info_t::init() {

    
    
}

void cap_ppa_decoders_mux_idx_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_idx_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_mux_idx_t::lkpsel(const cpp_int & _val) { 
    // lkpsel
    int_var__lkpsel = _val.convert_to< lkpsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_idx_t::lkpsel() const {
    return int_var__lkpsel;
}
    
void cap_ppa_decoders_mux_idx_t::idx(const cpp_int & _val) { 
    // idx
    int_var__idx = _val.convert_to< idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_idx_t::idx() const {
    return int_var__idx;
}
    
void cap_ppa_decoders_mux_idx_t::load_stored_lkp(const cpp_int & _val) { 
    // load_stored_lkp
    int_var__load_stored_lkp = _val.convert_to< load_stored_lkp_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_idx_t::load_stored_lkp() const {
    return int_var__load_stored_lkp;
}
    
void cap_ppa_decoders_mux_inst_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_mux_inst_t::muxsel(const cpp_int & _val) { 
    // muxsel
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::muxsel() const {
    return int_var__muxsel;
}
    
void cap_ppa_decoders_mux_inst_t::mask_val(const cpp_int & _val) { 
    // mask_val
    int_var__mask_val = _val.convert_to< mask_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::mask_val() const {
    return int_var__mask_val;
}
    
void cap_ppa_decoders_mux_inst_t::shift_val(const cpp_int & _val) { 
    // shift_val
    int_var__shift_val = _val.convert_to< shift_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::shift_val() const {
    return int_var__shift_val;
}
    
void cap_ppa_decoders_mux_inst_t::addsub_val(const cpp_int & _val) { 
    // addsub_val
    int_var__addsub_val = _val.convert_to< addsub_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::addsub_val() const {
    return int_var__addsub_val;
}
    
void cap_ppa_decoders_mux_inst_t::shift_left(const cpp_int & _val) { 
    // shift_left
    int_var__shift_left = _val.convert_to< shift_left_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::shift_left() const {
    return int_var__shift_left;
}
    
void cap_ppa_decoders_mux_inst_t::addsub(const cpp_int & _val) { 
    // addsub
    int_var__addsub = _val.convert_to< addsub_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::addsub() const {
    return int_var__addsub;
}
    
void cap_ppa_decoders_mux_inst_t::lkpsel(const cpp_int & _val) { 
    // lkpsel
    int_var__lkpsel = _val.convert_to< lkpsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::lkpsel() const {
    return int_var__lkpsel;
}
    
void cap_ppa_decoders_mux_inst_t::lkp_addsub(const cpp_int & _val) { 
    // lkp_addsub
    int_var__lkp_addsub = _val.convert_to< lkp_addsub_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::lkp_addsub() const {
    return int_var__lkp_addsub;
}
    
void cap_ppa_decoders_mux_inst_t::load_mux_pkt(const cpp_int & _val) { 
    // load_mux_pkt
    int_var__load_mux_pkt = _val.convert_to< load_mux_pkt_cpp_int_t >();
}

cpp_int cap_ppa_decoders_mux_inst_t::load_mux_pkt() const {
    return int_var__load_mux_pkt;
}
    
void cap_ppa_decoders_offset_inst_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_offset_inst_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_offset_inst_t::muxsel(const cpp_int & _val) { 
    // muxsel
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_offset_inst_t::muxsel() const {
    return int_var__muxsel;
}
    
void cap_ppa_decoders_offset_inst_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_offset_inst_t::val() const {
    return int_var__val;
}
    
void cap_ppa_decoders_lkp_val_inst_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_lkp_val_inst_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_lkp_val_inst_t::muxsel(const cpp_int & _val) { 
    // muxsel
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_lkp_val_inst_t::muxsel() const {
    return int_var__muxsel;
}
    
void cap_ppa_decoders_lkp_val_inst_t::store_en(const cpp_int & _val) { 
    // store_en
    int_var__store_en = _val.convert_to< store_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_lkp_val_inst_t::store_en() const {
    return int_var__store_en;
}
    
void cap_ppa_decoders_extract_inst_t::pkt_idx(const cpp_int & _val) { 
    // pkt_idx
    int_var__pkt_idx = _val.convert_to< pkt_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_extract_inst_t::pkt_idx() const {
    return int_var__pkt_idx;
}
    
void cap_ppa_decoders_extract_inst_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_ppa_decoders_extract_inst_t::len() const {
    return int_var__len;
}
    
void cap_ppa_decoders_extract_inst_t::phv_idx(const cpp_int & _val) { 
    // phv_idx
    int_var__phv_idx = _val.convert_to< phv_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_extract_inst_t::phv_idx() const {
    return int_var__phv_idx;
}
    
void cap_ppa_decoders_meta_inst_t::phv_idx(const cpp_int & _val) { 
    // phv_idx
    int_var__phv_idx = _val.convert_to< phv_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_meta_inst_t::phv_idx() const {
    return int_var__phv_idx;
}
    
void cap_ppa_decoders_meta_inst_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_meta_inst_t::val() const {
    return int_var__val;
}
    
void cap_ppa_decoders_meta_inst_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_meta_inst_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_ohi_inst_t::sel(const cpp_int & _val) { 
    // sel
    int_var__sel = _val.convert_to< sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ohi_inst_t::sel() const {
    return int_var__sel;
}
    
void cap_ppa_decoders_ohi_inst_t::muxsel(const cpp_int & _val) { 
    // muxsel
    int_var__muxsel = _val.convert_to< muxsel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ohi_inst_t::muxsel() const {
    return int_var__muxsel;
}
    
void cap_ppa_decoders_ohi_inst_t::idx_val(const cpp_int & _val) { 
    // idx_val
    int_var__idx_val = _val.convert_to< idx_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ohi_inst_t::idx_val() const {
    return int_var__idx_val;
}
    
void cap_ppa_decoders_ohi_inst_t::slot_num(const cpp_int & _val) { 
    // slot_num
    int_var__slot_num = _val.convert_to< slot_num_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ohi_inst_t::slot_num() const {
    return int_var__slot_num;
}
    
void cap_ppa_decoders_crc_inst_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::en() const {
    return int_var__en;
}
    
void cap_ppa_decoders_crc_inst_t::prof_sel_en(const cpp_int & _val) { 
    // prof_sel_en
    int_var__prof_sel_en = _val.convert_to< prof_sel_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::prof_sel_en() const {
    return int_var__prof_sel_en;
}
    
void cap_ppa_decoders_crc_inst_t::prof_sel(const cpp_int & _val) { 
    // prof_sel
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::prof_sel() const {
    return int_var__prof_sel;
}
    
void cap_ppa_decoders_crc_inst_t::ohi_start_sel(const cpp_int & _val) { 
    // ohi_start_sel
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::ohi_start_sel() const {
    return int_var__ohi_start_sel;
}
    
void cap_ppa_decoders_crc_inst_t::ohi_len_sel(const cpp_int & _val) { 
    // ohi_len_sel
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::ohi_len_sel() const {
    return int_var__ohi_len_sel;
}
    
void cap_ppa_decoders_crc_inst_t::ohi_mask_sel(const cpp_int & _val) { 
    // ohi_mask_sel
    int_var__ohi_mask_sel = _val.convert_to< ohi_mask_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_inst_t::ohi_mask_sel() const {
    return int_var__ohi_mask_sel;
}
    
void cap_ppa_decoders_csum_inst_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::en() const {
    return int_var__en;
}
    
void cap_ppa_decoders_csum_inst_t::unit_sel(const cpp_int & _val) { 
    // unit_sel
    int_var__unit_sel = _val.convert_to< unit_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::unit_sel() const {
    return int_var__unit_sel;
}
    
void cap_ppa_decoders_csum_inst_t::prof_sel(const cpp_int & _val) { 
    // prof_sel
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::prof_sel() const {
    return int_var__prof_sel;
}
    
void cap_ppa_decoders_csum_inst_t::ohi_start_sel(const cpp_int & _val) { 
    // ohi_start_sel
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::ohi_start_sel() const {
    return int_var__ohi_start_sel;
}
    
void cap_ppa_decoders_csum_inst_t::ohi_len_sel(const cpp_int & _val) { 
    // ohi_len_sel
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::ohi_len_sel() const {
    return int_var__ohi_len_sel;
}
    
void cap_ppa_decoders_csum_inst_t::phdr_en(const cpp_int & _val) { 
    // phdr_en
    int_var__phdr_en = _val.convert_to< phdr_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::phdr_en() const {
    return int_var__phdr_en;
}
    
void cap_ppa_decoders_csum_inst_t::phdr_sel(const cpp_int & _val) { 
    // phdr_sel
    int_var__phdr_sel = _val.convert_to< phdr_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::phdr_sel() const {
    return int_var__phdr_sel;
}
    
void cap_ppa_decoders_csum_inst_t::phdr_ohi_sel(const cpp_int & _val) { 
    // phdr_ohi_sel
    int_var__phdr_ohi_sel = _val.convert_to< phdr_ohi_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::phdr_ohi_sel() const {
    return int_var__phdr_ohi_sel;
}
    
void cap_ppa_decoders_csum_inst_t::dis_zero(const cpp_int & _val) { 
    // dis_zero
    int_var__dis_zero = _val.convert_to< dis_zero_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::dis_zero() const {
    return int_var__dis_zero;
}
    
void cap_ppa_decoders_csum_inst_t::load_phdr_prof_en(const cpp_int & _val) { 
    // load_phdr_prof_en
    int_var__load_phdr_prof_en = _val.convert_to< load_phdr_prof_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_inst_t::load_phdr_prof_en() const {
    return int_var__load_phdr_prof_en;
}
    
void cap_ppa_decoders_len_chk_inst_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::en() const {
    return int_var__en;
}
    
void cap_ppa_decoders_len_chk_inst_t::unit_sel(const cpp_int & _val) { 
    // unit_sel
    int_var__unit_sel = _val.convert_to< unit_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::unit_sel() const {
    return int_var__unit_sel;
}
    
void cap_ppa_decoders_len_chk_inst_t::prof_sel(const cpp_int & _val) { 
    // prof_sel
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::prof_sel() const {
    return int_var__prof_sel;
}
    
void cap_ppa_decoders_len_chk_inst_t::ohi_start_sel(const cpp_int & _val) { 
    // ohi_start_sel
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::ohi_start_sel() const {
    return int_var__ohi_start_sel;
}
    
void cap_ppa_decoders_len_chk_inst_t::ohi_len_sel(const cpp_int & _val) { 
    // ohi_len_sel
    int_var__ohi_len_sel = _val.convert_to< ohi_len_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::ohi_len_sel() const {
    return int_var__ohi_len_sel;
}
    
void cap_ppa_decoders_len_chk_inst_t::exact(const cpp_int & _val) { 
    // exact
    int_var__exact = _val.convert_to< exact_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_inst_t::exact() const {
    return int_var__exact;
}
    
void cap_ppa_decoders_align_chk_inst_t::en(const cpp_int & _val) { 
    // en
    int_var__en = _val.convert_to< en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_inst_t::en() const {
    return int_var__en;
}
    
void cap_ppa_decoders_align_chk_inst_t::prof_sel(const cpp_int & _val) { 
    // prof_sel
    int_var__prof_sel = _val.convert_to< prof_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_inst_t::prof_sel() const {
    return int_var__prof_sel;
}
    
void cap_ppa_decoders_align_chk_inst_t::ohi_start_sel(const cpp_int & _val) { 
    // ohi_start_sel
    int_var__ohi_start_sel = _val.convert_to< ohi_start_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_inst_t::ohi_start_sel() const {
    return int_var__ohi_start_sel;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::action(const cpp_int & _val) { 
    // action
    int_var__action = _val.convert_to< action_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::action() const {
    return int_var__action;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::nxt_state(const cpp_int & _val) { 
    // nxt_state
    int_var__nxt_state = _val.convert_to< nxt_state_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::nxt_state() const {
    return int_var__nxt_state;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::phv_idx_upr(const cpp_int & _val) { 
    // phv_idx_upr
    int_var__phv_idx_upr = _val.convert_to< phv_idx_upr_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::phv_idx_upr() const {
    return int_var__phv_idx_upr;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::offset_jump_chk_en(const cpp_int & _val) { 
    // offset_jump_chk_en
    int_var__offset_jump_chk_en = _val.convert_to< offset_jump_chk_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::offset_jump_chk_en() const {
    return int_var__offset_jump_chk_en;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::pkt_size_chk_en(const cpp_int & _val) { 
    // pkt_size_chk_en
    int_var__pkt_size_chk_en = _val.convert_to< pkt_size_chk_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::pkt_size_chk_en() const {
    return int_var__pkt_size_chk_en;
}
    
void cap_ppa_decoders_ppa_lkp_sram_entry_t::pkt_size(const cpp_int & _val) { 
    // pkt_size
    int_var__pkt_size = _val.convert_to< pkt_size_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_sram_entry_t::pkt_size() const {
    return int_var__pkt_size;
}
    
void cap_ppa_decoders_chk_ctl_t::last_offset(const cpp_int & _val) { 
    // last_offset
    int_var__last_offset = _val.convert_to< last_offset_cpp_int_t >();
}

cpp_int cap_ppa_decoders_chk_ctl_t::last_offset() const {
    return int_var__last_offset;
}
    
void cap_ppa_decoders_chk_ctl_t::seq_id(const cpp_int & _val) { 
    // seq_id
    int_var__seq_id = _val.convert_to< seq_id_cpp_int_t >();
}

cpp_int cap_ppa_decoders_chk_ctl_t::seq_id() const {
    return int_var__seq_id;
}
    
void cap_ppa_decoders_chk_ctl_t::err_vec(const cpp_int & _val) { 
    // err_vec
    int_var__err_vec = _val.convert_to< err_vec_cpp_int_t >();
}

cpp_int cap_ppa_decoders_chk_ctl_t::err_vec() const {
    return int_var__err_vec;
}
    
void cap_ppa_decoders_ppa_lkp_tcam_key_t::control(const cpp_int & _val) { 
    // control
    int_var__control = _val.convert_to< control_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_key_t::control() const {
    return int_var__control;
}
    
void cap_ppa_decoders_ppa_lkp_tcam_key_t::state(const cpp_int & _val) { 
    // state
    int_var__state = _val.convert_to< state_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_key_t::state() const {
    return int_var__state;
}
    
#if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// lkp_val
void cap_ppa_decoders_ppa_lkp_tcam_key_t::lkp_val(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function lkp_val Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_key_t::lkp_val(int _idx) const {
    PLOG_ERR("get function lkp_val Not yet implemented"<< endl);
    return 0;
}
#else 
// lkp_val
void cap_ppa_decoders_ppa_lkp_tcam_key_t::lkp_val(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 3);
    int_var__lkp_val[_idx] = _val.convert_to< lkp_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_key_t::lkp_val(int _idx) const {
    PU_ASSERT(_idx < 3);
    return int_var__lkp_val[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_ppa_decoders_ppa_lkp_tcam_entry_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_ppa_decoders_ppa_lkp_tcam_entry_t::valid() const {
    return int_var__valid;
}
    
void cap_ppa_decoders_crc_prof_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::len_mask() const {
    return int_var__len_mask;
}
    
void cap_ppa_decoders_crc_prof_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::len_shift_left() const {
    return int_var__len_shift_left;
}
    
void cap_ppa_decoders_crc_prof_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::len_shift_val() const {
    return int_var__len_shift_val;
}
    
void cap_ppa_decoders_crc_prof_t::addsub_start(const cpp_int & _val) { 
    // addsub_start
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::addsub_start() const {
    return int_var__addsub_start;
}
    
void cap_ppa_decoders_crc_prof_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_ppa_decoders_crc_prof_t::addsub_end(const cpp_int & _val) { 
    // addsub_end
    int_var__addsub_end = _val.convert_to< addsub_end_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::addsub_end() const {
    return int_var__addsub_end;
}
    
void cap_ppa_decoders_crc_prof_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::end_adj() const {
    return int_var__end_adj;
}
    
void cap_ppa_decoders_crc_prof_t::addsub_mask(const cpp_int & _val) { 
    // addsub_mask
    int_var__addsub_mask = _val.convert_to< addsub_mask_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::addsub_mask() const {
    return int_var__addsub_mask;
}
    
void cap_ppa_decoders_crc_prof_t::mask_adj(const cpp_int & _val) { 
    // mask_adj
    int_var__mask_adj = _val.convert_to< mask_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::mask_adj() const {
    return int_var__mask_adj;
}
    
void cap_ppa_decoders_crc_prof_t::end_eop(const cpp_int & _val) { 
    // end_eop
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::end_eop() const {
    return int_var__end_eop;
}
    
void cap_ppa_decoders_crc_prof_t::mask_prof_sel(const cpp_int & _val) { 
    // mask_prof_sel
    int_var__mask_prof_sel = _val.convert_to< mask_prof_sel_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_prof_t::mask_prof_sel() const {
    return int_var__mask_prof_sel;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::mask_en(const cpp_int & _val) { 
    // mask_en
    int_var__mask_en = _val.convert_to< mask_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::mask_en() const {
    return int_var__mask_en;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::use_ohi(const cpp_int & _val) { 
    // use_ohi
    int_var__use_ohi = _val.convert_to< use_ohi_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::use_ohi() const {
    return int_var__use_ohi;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::end_adj() const {
    return int_var__end_adj;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::fill(const cpp_int & _val) { 
    // fill
    int_var__fill = _val.convert_to< fill_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::fill() const {
    return int_var__fill;
}
    
void cap_ppa_decoders_crc_mask_prof_inst_t::skip_first_nibble(const cpp_int & _val) { 
    // skip_first_nibble
    int_var__skip_first_nibble = _val.convert_to< skip_first_nibble_cpp_int_t >();
}

cpp_int cap_ppa_decoders_crc_mask_prof_inst_t::skip_first_nibble() const {
    return int_var__skip_first_nibble;
}
    
void cap_ppa_decoders_csum_prof_t::csum_8b(const cpp_int & _val) { 
    // csum_8b
    int_var__csum_8b = _val.convert_to< csum_8b_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::csum_8b() const {
    return int_var__csum_8b;
}
    
void cap_ppa_decoders_csum_prof_t::phv_csum_flit_num(const cpp_int & _val) { 
    // phv_csum_flit_num
    int_var__phv_csum_flit_num = _val.convert_to< phv_csum_flit_num_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::phv_csum_flit_num() const {
    return int_var__phv_csum_flit_num;
}
    
void cap_ppa_decoders_csum_prof_t::end_eop(const cpp_int & _val) { 
    // end_eop
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::end_eop() const {
    return int_var__end_eop;
}
    
void cap_ppa_decoders_csum_prof_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::len_mask() const {
    return int_var__len_mask;
}
    
void cap_ppa_decoders_csum_prof_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::len_shift_left() const {
    return int_var__len_shift_left;
}
    
void cap_ppa_decoders_csum_prof_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::len_shift_val() const {
    return int_var__len_shift_val;
}
    
void cap_ppa_decoders_csum_prof_t::addsub_start(const cpp_int & _val) { 
    // addsub_start
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::addsub_start() const {
    return int_var__addsub_start;
}
    
void cap_ppa_decoders_csum_prof_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_ppa_decoders_csum_prof_t::addsub_end(const cpp_int & _val) { 
    // addsub_end
    int_var__addsub_end = _val.convert_to< addsub_end_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::addsub_end() const {
    return int_var__addsub_end;
}
    
void cap_ppa_decoders_csum_prof_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::end_adj() const {
    return int_var__end_adj;
}
    
void cap_ppa_decoders_csum_prof_t::addsub_phdr(const cpp_int & _val) { 
    // addsub_phdr
    int_var__addsub_phdr = _val.convert_to< addsub_phdr_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::addsub_phdr() const {
    return int_var__addsub_phdr;
}
    
void cap_ppa_decoders_csum_prof_t::phdr_adj(const cpp_int & _val) { 
    // phdr_adj
    int_var__phdr_adj = _val.convert_to< phdr_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::phdr_adj() const {
    return int_var__phdr_adj;
}
    
void cap_ppa_decoders_csum_prof_t::addsub_csum_loc(const cpp_int & _val) { 
    // addsub_csum_loc
    int_var__addsub_csum_loc = _val.convert_to< addsub_csum_loc_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::addsub_csum_loc() const {
    return int_var__addsub_csum_loc;
}
    
void cap_ppa_decoders_csum_prof_t::csum_loc_adj(const cpp_int & _val) { 
    // csum_loc_adj
    int_var__csum_loc_adj = _val.convert_to< csum_loc_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::csum_loc_adj() const {
    return int_var__csum_loc_adj;
}
    
void cap_ppa_decoders_csum_prof_t::align(const cpp_int & _val) { 
    // align
    int_var__align = _val.convert_to< align_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::align() const {
    return int_var__align;
}
    
void cap_ppa_decoders_csum_prof_t::add_val(const cpp_int & _val) { 
    // add_val
    int_var__add_val = _val.convert_to< add_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_prof_t::add_val() const {
    return int_var__add_val;
}
    
void cap_ppa_decoders_csum_phdr_prof_inst_t::fld_en(const cpp_int & _val) { 
    // fld_en
    int_var__fld_en = _val.convert_to< fld_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::fld_en() const {
    return int_var__fld_en;
}
    
void cap_ppa_decoders_csum_phdr_prof_inst_t::fld_align(const cpp_int & _val) { 
    // fld_align
    int_var__fld_align = _val.convert_to< fld_align_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::fld_align() const {
    return int_var__fld_align;
}
    
void cap_ppa_decoders_csum_phdr_prof_inst_t::fld_start(const cpp_int & _val) { 
    // fld_start
    int_var__fld_start = _val.convert_to< fld_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::fld_start() const {
    return int_var__fld_start;
}
    
void cap_ppa_decoders_csum_phdr_prof_inst_t::fld_end(const cpp_int & _val) { 
    // fld_end
    int_var__fld_end = _val.convert_to< fld_end_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::fld_end() const {
    return int_var__fld_end;
}
    
void cap_ppa_decoders_csum_phdr_prof_inst_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_phdr_prof_inst_t::add_len() const {
    return int_var__add_len;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::fld_en(const cpp_int & _val) { 
    // fld_en
    int_var__fld_en = _val.convert_to< fld_en_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::fld_en() const {
    return int_var__fld_en;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::fld_align(const cpp_int & _val) { 
    // fld_align
    int_var__fld_align = _val.convert_to< fld_align_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::fld_align() const {
    return int_var__fld_align;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::fld_start(const cpp_int & _val) { 
    // fld_start
    int_var__fld_start = _val.convert_to< fld_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::fld_start() const {
    return int_var__fld_start;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::fld_end(const cpp_int & _val) { 
    // fld_end
    int_var__fld_end = _val.convert_to< fld_end_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::fld_end() const {
    return int_var__fld_end;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::add_len() const {
    return int_var__add_len;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::end_eop(const cpp_int & _val) { 
    // end_eop
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::end_eop() const {
    return int_var__end_eop;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::phv_csum_flit_num(const cpp_int & _val) { 
    // phv_csum_flit_num
    int_var__phv_csum_flit_num = _val.convert_to< phv_csum_flit_num_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::phv_csum_flit_num() const {
    return int_var__phv_csum_flit_num;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::csum_8b(const cpp_int & _val) { 
    // csum_8b
    int_var__csum_8b = _val.convert_to< csum_8b_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::csum_8b() const {
    return int_var__csum_8b;
}
    
void cap_ppa_decoders_csum_all_fld_inst_t::add_val(const cpp_int & _val) { 
    // add_val
    int_var__add_val = _val.convert_to< add_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_csum_all_fld_inst_t::add_val() const {
    return int_var__add_val;
}
    
void cap_ppa_decoders_len_chk_prof_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_prof_t::len_mask() const {
    return int_var__len_mask;
}
    
void cap_ppa_decoders_len_chk_prof_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_prof_t::len_shift_left() const {
    return int_var__len_shift_left;
}
    
void cap_ppa_decoders_len_chk_prof_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_prof_t::len_shift_val() const {
    return int_var__len_shift_val;
}
    
void cap_ppa_decoders_len_chk_prof_t::addsub_start(const cpp_int & _val) { 
    // addsub_start
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_prof_t::addsub_start() const {
    return int_var__addsub_start;
}
    
void cap_ppa_decoders_len_chk_prof_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_len_chk_prof_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_ppa_decoders_pkt_ff_data_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::data() const {
    return int_var__data;
}
    
void cap_ppa_decoders_pkt_ff_data_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::sz() const {
    return int_var__sz;
}
    
void cap_ppa_decoders_pkt_ff_data_t::err(const cpp_int & _val) { 
    // err
    int_var__err = _val.convert_to< err_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::err() const {
    return int_var__err;
}
    
void cap_ppa_decoders_pkt_ff_data_t::eop(const cpp_int & _val) { 
    // eop
    int_var__eop = _val.convert_to< eop_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::eop() const {
    return int_var__eop;
}
    
void cap_ppa_decoders_pkt_ff_data_t::sop(const cpp_int & _val) { 
    // sop
    int_var__sop = _val.convert_to< sop_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::sop() const {
    return int_var__sop;
}
    
void cap_ppa_decoders_pkt_ff_data_t::seq_id(const cpp_int & _val) { 
    // seq_id
    int_var__seq_id = _val.convert_to< seq_id_cpp_int_t >();
}

cpp_int cap_ppa_decoders_pkt_ff_data_t::seq_id() const {
    return int_var__seq_id;
}
    
void cap_ppa_decoders_align_chk_prof_t::addsub_start(const cpp_int & _val) { 
    // addsub_start
    int_var__addsub_start = _val.convert_to< addsub_start_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_prof_t::addsub_start() const {
    return int_var__addsub_start;
}
    
void cap_ppa_decoders_align_chk_prof_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_prof_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_ppa_decoders_align_chk_prof_t::mod_log2(const cpp_int & _val) { 
    // mod_log2
    int_var__mod_log2 = _val.convert_to< mod_log2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_align_chk_prof_t::mod_log2() const {
    return int_var__mod_log2;
}
    
void cap_ppa_decoders_err_vec_t::offset_jump_chk(const cpp_int & _val) { 
    // offset_jump_chk
    int_var__offset_jump_chk = _val.convert_to< offset_jump_chk_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::offset_jump_chk() const {
    return int_var__offset_jump_chk;
}
    
void cap_ppa_decoders_err_vec_t::pkt_size_chk(const cpp_int & _val) { 
    // pkt_size_chk
    int_var__pkt_size_chk = _val.convert_to< pkt_size_chk_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::pkt_size_chk() const {
    return int_var__pkt_size_chk;
}
    
void cap_ppa_decoders_err_vec_t::offset_out_of_range(const cpp_int & _val) { 
    // offset_out_of_range
    int_var__offset_out_of_range = _val.convert_to< offset_out_of_range_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::offset_out_of_range() const {
    return int_var__offset_out_of_range;
}
    
void cap_ppa_decoders_err_vec_t::exceed_parse_loop_cnt(const cpp_int & _val) { 
    // exceed_parse_loop_cnt
    int_var__exceed_parse_loop_cnt = _val.convert_to< exceed_parse_loop_cnt_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::exceed_parse_loop_cnt() const {
    return int_var__exceed_parse_loop_cnt;
}
    
void cap_ppa_decoders_err_vec_t::exceed_phv_flit_cnt(const cpp_int & _val) { 
    // exceed_phv_flit_cnt
    int_var__exceed_phv_flit_cnt = _val.convert_to< exceed_phv_flit_cnt_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::exceed_phv_flit_cnt() const {
    return int_var__exceed_phv_flit_cnt;
}
    
void cap_ppa_decoders_err_vec_t::phv_upr_idx_less(const cpp_int & _val) { 
    // phv_upr_idx_less
    int_var__phv_upr_idx_less = _val.convert_to< phv_upr_idx_less_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::phv_upr_idx_less() const {
    return int_var__phv_upr_idx_less;
}
    
void cap_ppa_decoders_err_vec_t::init_lkp_idx(const cpp_int & _val) { 
    // init_lkp_idx
    int_var__init_lkp_idx = _val.convert_to< init_lkp_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_err_vec_t::init_lkp_idx() const {
    return int_var__init_lkp_idx;
}
    
void cap_ppa_decoders_si_mux_pkt_rslt_t::mux_pkt(const cpp_int & _val) { 
    // mux_pkt
    int_var__mux_pkt = _val.convert_to< mux_pkt_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_mux_pkt_rslt_t::mux_pkt() const {
    return int_var__mux_pkt;
}
    
void cap_ppa_decoders_si_mux_inst_rslt_t::mux_pkt_inst(const cpp_int & _val) { 
    // mux_pkt_inst
    int_var__mux_pkt_inst = _val.convert_to< mux_pkt_inst_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_mux_inst_rslt_t::mux_pkt_inst() const {
    return int_var__mux_pkt_inst;
}
    
void cap_ppa_decoders_si_extract_rslt_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_extract_rslt_t::data() const {
    return int_var__data;
}
    
void cap_ppa_decoders_si_meta_rslt_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_meta_rslt_t::data() const {
    return int_var__data;
}
    
void cap_ppa_decoders_si_state_info_t::tcam_hit(const cpp_int & _val) { 
    // tcam_hit
    int_var__tcam_hit = _val.convert_to< tcam_hit_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_state_info_t::tcam_hit() const {
    return int_var__tcam_hit;
}
    
void cap_ppa_decoders_si_state_info_t::tcam_hit_idx(const cpp_int & _val) { 
    // tcam_hit_idx
    int_var__tcam_hit_idx = _val.convert_to< tcam_hit_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_state_info_t::tcam_hit_idx() const {
    return int_var__tcam_hit_idx;
}
    
void cap_ppa_decoders_si_state_info_t::flit_idx(const cpp_int & _val) { 
    // flit_idx
    int_var__flit_idx = _val.convert_to< flit_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_state_info_t::flit_idx() const {
    return int_var__flit_idx;
}
    
void cap_ppa_decoders_si_pe_step_info_t::init_profile_idx(const cpp_int & _val) { 
    // init_profile_idx
    int_var__init_profile_idx = _val.convert_to< init_profile_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::init_profile_idx() const {
    return int_var__init_profile_idx;
}
    
void cap_ppa_decoders_si_pe_step_info_t::phv_global_csum_err(const cpp_int & _val) { 
    // phv_global_csum_err
    int_var__phv_global_csum_err = _val.convert_to< phv_global_csum_err_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::phv_global_csum_err() const {
    return int_var__phv_global_csum_err;
}
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// lkp_cur_state
void cap_ppa_decoders_si_pe_step_info_t::lkp_cur_state(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function lkp_cur_state Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_cur_state(int _idx) const {
    PLOG_ERR("get function lkp_cur_state Not yet implemented"<< endl);
    return 0;
}
#else 
// lkp_cur_state
void cap_ppa_decoders_si_pe_step_info_t::lkp_cur_state(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__lkp_cur_state[_idx] = _val.convert_to< lkp_cur_state_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_cur_state(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__lkp_cur_state[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// stored_lkp_val0
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val0(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function stored_lkp_val0 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val0(int _idx) const {
    PLOG_ERR("get function stored_lkp_val0 Not yet implemented"<< endl);
    return 0;
}
#else 
// stored_lkp_val0
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val0(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__stored_lkp_val0[_idx] = _val.convert_to< stored_lkp_val0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val0(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__stored_lkp_val0[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// lkp_val0
void cap_ppa_decoders_si_pe_step_info_t::lkp_val0(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function lkp_val0 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val0(int _idx) const {
    PLOG_ERR("get function lkp_val0 Not yet implemented"<< endl);
    return 0;
}
#else 
// lkp_val0
void cap_ppa_decoders_si_pe_step_info_t::lkp_val0(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__lkp_val0[_idx] = _val.convert_to< lkp_val0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val0(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__lkp_val0[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// stored_lkp_val1
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val1(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function stored_lkp_val1 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val1(int _idx) const {
    PLOG_ERR("get function stored_lkp_val1 Not yet implemented"<< endl);
    return 0;
}
#else 
// stored_lkp_val1
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val1(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__stored_lkp_val1[_idx] = _val.convert_to< stored_lkp_val1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val1(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__stored_lkp_val1[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// lkp_val1
void cap_ppa_decoders_si_pe_step_info_t::lkp_val1(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function lkp_val1 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val1(int _idx) const {
    PLOG_ERR("get function lkp_val1 Not yet implemented"<< endl);
    return 0;
}
#else 
// lkp_val1
void cap_ppa_decoders_si_pe_step_info_t::lkp_val1(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__lkp_val1[_idx] = _val.convert_to< lkp_val1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val1(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__lkp_val1[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// stored_lkp_val2
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val2(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function stored_lkp_val2 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val2(int _idx) const {
    PLOG_ERR("get function stored_lkp_val2 Not yet implemented"<< endl);
    return 0;
}
#else 
// stored_lkp_val2
void cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val2(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__stored_lkp_val2[_idx] = _val.convert_to< stored_lkp_val2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::stored_lkp_val2(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__stored_lkp_val2[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// lkp_val2
void cap_ppa_decoders_si_pe_step_info_t::lkp_val2(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function lkp_val2 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val2(int _idx) const {
    PLOG_ERR("get function lkp_val2 Not yet implemented"<< endl);
    return 0;
}
#else 
// lkp_val2
void cap_ppa_decoders_si_pe_step_info_t::lkp_val2(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__lkp_val2[_idx] = _val.convert_to< lkp_val2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::lkp_val2(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__lkp_val2[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// tcam_hit
void cap_ppa_decoders_si_pe_step_info_t::tcam_hit(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function tcam_hit Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::tcam_hit(int _idx) const {
    PLOG_ERR("get function tcam_hit Not yet implemented"<< endl);
    return 0;
}
#else 
// tcam_hit
void cap_ppa_decoders_si_pe_step_info_t::tcam_hit(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__tcam_hit[_idx] = _val.convert_to< tcam_hit_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::tcam_hit(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__tcam_hit[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// tcam_hit_idx
void cap_ppa_decoders_si_pe_step_info_t::tcam_hit_idx(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function tcam_hit_idx Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::tcam_hit_idx(int _idx) const {
    PLOG_ERR("get function tcam_hit_idx Not yet implemented"<< endl);
    return 0;
}
#else 
// tcam_hit_idx
void cap_ppa_decoders_si_pe_step_info_t::tcam_hit_idx(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__tcam_hit_idx[_idx] = _val.convert_to< tcam_hit_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::tcam_hit_idx(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__tcam_hit_idx[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// cur_flit_idx
void cap_ppa_decoders_si_pe_step_info_t::cur_flit_idx(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function cur_flit_idx Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::cur_flit_idx(int _idx) const {
    PLOG_ERR("get function cur_flit_idx Not yet implemented"<< endl);
    return 0;
}
#else 
// cur_flit_idx
void cap_ppa_decoders_si_pe_step_info_t::cur_flit_idx(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__cur_flit_idx[_idx] = _val.convert_to< cur_flit_idx_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::cur_flit_idx(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__cur_flit_idx[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_pkt0
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt0(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_pkt0 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt0(int _idx) const {
    PLOG_ERR("get function mux_pkt0 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_pkt0
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt0(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_pkt0[_idx] = _val.convert_to< mux_pkt0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt0(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_pkt0[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_pkt1
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt1(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_pkt1 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt1(int _idx) const {
    PLOG_ERR("get function mux_pkt1 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_pkt1
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt1(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_pkt1[_idx] = _val.convert_to< mux_pkt1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt1(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_pkt1[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_pkt2
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt2(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_pkt2 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt2(int _idx) const {
    PLOG_ERR("get function mux_pkt2 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_pkt2
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt2(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_pkt2[_idx] = _val.convert_to< mux_pkt2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt2(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_pkt2[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_pkt3
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt3(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_pkt3 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt3(int _idx) const {
    PLOG_ERR("get function mux_pkt3 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_pkt3
void cap_ppa_decoders_si_pe_step_info_t::mux_pkt3(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_pkt3[_idx] = _val.convert_to< mux_pkt3_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_pkt3(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_pkt3[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_inst_data0
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data0(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_inst_data0 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data0(int _idx) const {
    PLOG_ERR("get function mux_inst_data0 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_inst_data0
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data0(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_inst_data0[_idx] = _val.convert_to< mux_inst_data0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data0(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_inst_data0[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_inst_data1
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data1(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_inst_data1 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data1(int _idx) const {
    PLOG_ERR("get function mux_inst_data1 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_inst_data1
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data1(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_inst_data1[_idx] = _val.convert_to< mux_inst_data1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data1(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_inst_data1[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mux_inst_data2
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data2(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mux_inst_data2 Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data2(int _idx) const {
    PLOG_ERR("get function mux_inst_data2 Not yet implemented"<< endl);
    return 0;
}
#else 
// mux_inst_data2
void cap_ppa_decoders_si_pe_step_info_t::mux_inst_data2(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__mux_inst_data2[_idx] = _val.convert_to< mux_inst_data2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::mux_inst_data2(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__mux_inst_data2[_idx].convert_to<cpp_int>();
}
#endif
    
#if 100 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// extract_phv_data
void cap_ppa_decoders_si_pe_step_info_t::extract_phv_data(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function extract_phv_data Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::extract_phv_data(int _idx) const {
    PLOG_ERR("get function extract_phv_data Not yet implemented"<< endl);
    return 0;
}
#else 
// extract_phv_data
void cap_ppa_decoders_si_pe_step_info_t::extract_phv_data(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 100);
    int_var__extract_phv_data[_idx] = _val.convert_to< extract_phv_data_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_pe_step_info_t::extract_phv_data(int _idx) const {
    PU_ASSERT(_idx < 100);
    return int_var__extract_phv_data[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_ppa_decoders_si_non_pe_step_info_t::preparser_csum_err_out(const cpp_int & _val) { 
    // preparser_csum_err_out
    int_var__preparser_csum_err_out = _val.convert_to< preparser_csum_err_out_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::preparser_csum_err_out() const {
    return int_var__preparser_csum_err_out;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::parse_end_ptr(const cpp_int & _val) { 
    // parse_end_ptr
    int_var__parse_end_ptr = _val.convert_to< parse_end_ptr_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::parse_end_ptr() const {
    return int_var__parse_end_ptr;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::err_vec(const cpp_int & _val) { 
    // err_vec
    int_var__err_vec = _val.convert_to< err_vec_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::err_vec() const {
    return int_var__err_vec;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::err_code(const cpp_int & _val) { 
    // err_code
    int_var__err_code = _val.convert_to< err_code_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::err_code() const {
    return int_var__err_code;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::crc_val(const cpp_int & _val) { 
    // crc_val
    int_var__crc_val = _val.convert_to< crc_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::crc_val() const {
    return int_var__crc_val;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::crc_err(const cpp_int & _val) { 
    // crc_err
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::crc_err() const {
    return int_var__crc_err;
}
    
#if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// csum_val
void cap_ppa_decoders_si_non_pe_step_info_t::csum_val(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function csum_val Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::csum_val(int _idx) const {
    PLOG_ERR("get function csum_val Not yet implemented"<< endl);
    return 0;
}
#else 
// csum_val
void cap_ppa_decoders_si_non_pe_step_info_t::csum_val(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 5);
    int_var__csum_val[_idx] = _val.convert_to< csum_val_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::csum_val(int _idx) const {
    PU_ASSERT(_idx < 5);
    return int_var__csum_val[_idx].convert_to<cpp_int>();
}
#endif
    
#if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// csum_err
void cap_ppa_decoders_si_non_pe_step_info_t::csum_err(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function csum_err Not yet implemented"<< endl);
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::csum_err(int _idx) const {
    PLOG_ERR("get function csum_err Not yet implemented"<< endl);
    return 0;
}
#else 
// csum_err
void cap_ppa_decoders_si_non_pe_step_info_t::csum_err(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 5);
    int_var__csum_err[_idx] = _val.convert_to< csum_err_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::csum_err(int _idx) const {
    PU_ASSERT(_idx < 5);
    return int_var__csum_err[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val0(const cpp_int & _val) { 
    // len_chk_val0
    int_var__len_chk_val0 = _val.convert_to< len_chk_val0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val0() const {
    return int_var__len_chk_val0;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err0(const cpp_int & _val) { 
    // len_chk_err0
    int_var__len_chk_err0 = _val.convert_to< len_chk_err0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err0() const {
    return int_var__len_chk_err0;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val1(const cpp_int & _val) { 
    // len_chk_val1
    int_var__len_chk_val1 = _val.convert_to< len_chk_val1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val1() const {
    return int_var__len_chk_val1;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err1(const cpp_int & _val) { 
    // len_chk_err1
    int_var__len_chk_err1 = _val.convert_to< len_chk_err1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err1() const {
    return int_var__len_chk_err1;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val2(const cpp_int & _val) { 
    // len_chk_val2
    int_var__len_chk_val2 = _val.convert_to< len_chk_val2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val2() const {
    return int_var__len_chk_val2;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err2(const cpp_int & _val) { 
    // len_chk_err2
    int_var__len_chk_err2 = _val.convert_to< len_chk_err2_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err2() const {
    return int_var__len_chk_err2;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val3(const cpp_int & _val) { 
    // len_chk_val3
    int_var__len_chk_val3 = _val.convert_to< len_chk_val3_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_val3() const {
    return int_var__len_chk_val3;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err3(const cpp_int & _val) { 
    // len_chk_err3
    int_var__len_chk_err3 = _val.convert_to< len_chk_err3_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::len_chk_err3() const {
    return int_var__len_chk_err3;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::align_chk_val0(const cpp_int & _val) { 
    // align_chk_val0
    int_var__align_chk_val0 = _val.convert_to< align_chk_val0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::align_chk_val0() const {
    return int_var__align_chk_val0;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::align_chk_err0(const cpp_int & _val) { 
    // align_chk_err0
    int_var__align_chk_err0 = _val.convert_to< align_chk_err0_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::align_chk_err0() const {
    return int_var__align_chk_err0;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::align_chk_val1(const cpp_int & _val) { 
    // align_chk_val1
    int_var__align_chk_val1 = _val.convert_to< align_chk_val1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::align_chk_val1() const {
    return int_var__align_chk_val1;
}
    
void cap_ppa_decoders_si_non_pe_step_info_t::align_chk_err1(const cpp_int & _val) { 
    // align_chk_err1
    int_var__align_chk_err1 = _val.convert_to< align_chk_err1_cpp_int_t >();
}

cpp_int cap_ppa_decoders_si_non_pe_step_info_t::align_chk_err1() const {
    return int_var__align_chk_err1;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_mux_idx_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkpsel")) { field_val = lkpsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "idx")) { field_val = idx(); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_stored_lkp")) { field_val = load_stored_lkp(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_mux_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { field_val = muxsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_val")) { field_val = mask_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "shift_val")) { field_val = shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_val")) { field_val = addsub_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "shift_left")) { field_val = shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub")) { field_val = addsub(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkpsel")) { field_val = lkpsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkp_addsub")) { field_val = lkp_addsub(); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_mux_pkt")) { field_val = load_mux_pkt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_offset_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { field_val = muxsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_lkp_val_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { field_val = muxsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "store_en")) { field_val = store_en(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_extract_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pkt_idx")) { field_val = pkt_idx(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_idx")) { field_val = phv_idx(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_meta_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phv_idx")) { field_val = phv_idx(); field_found=1; }
    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ohi_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { field_val = sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { field_val = muxsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "idx_val")) { field_val = idx_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot_num")) { field_val = slot_num(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { field_val = en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel_en")) { field_val = prof_sel_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { field_val = prof_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { field_val = ohi_start_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { field_val = ohi_len_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_mask_sel")) { field_val = ohi_mask_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { field_val = en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "unit_sel")) { field_val = unit_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { field_val = prof_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { field_val = ohi_start_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { field_val = ohi_len_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_en")) { field_val = phdr_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_sel")) { field_val = phdr_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_ohi_sel")) { field_val = phdr_ohi_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dis_zero")) { field_val = dis_zero(); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_phdr_prof_en")) { field_val = load_phdr_prof_en(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_len_chk_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { field_val = en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "unit_sel")) { field_val = unit_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { field_val = prof_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { field_val = ohi_start_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { field_val = ohi_len_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "exact")) { field_val = exact(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_align_chk_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { field_val = en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { field_val = prof_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { field_val = ohi_start_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "action")) { field_val = action(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nxt_state")) { field_val = nxt_state(); field_found=1; }
    if(!field_found) { field_found = offset_inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "phv_idx_upr")) { field_val = phv_idx_upr(); field_found=1; }
    if(!field_found) { field_found = crc_inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "offset_jump_chk_en")) { field_val = offset_jump_chk_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size_chk_en")) { field_val = pkt_size_chk_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size")) { field_val = pkt_size(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_chk_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "last_offset")) { field_val = last_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_id")) { field_val = seq_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_vec")) { field_val = err_vec(); field_found=1; }
    if(!field_found) { field_found = crc_inst.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_tcam_key_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "control")) { field_val = control(); field_found=1; }
    if(!field_found && !strcmp(field_name, "state")) { field_val = state(); field_found=1; }
    for(int ii = 0; !field_found && (ii < 3); ii++) {
        if(!strcmp(field_name, string("lkp_val["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__lkp_val[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_tcam_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = key.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mask.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "len_mask")) { field_val = len_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { field_val = len_shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { field_val = len_shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { field_val = addsub_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_end")) { field_val = addsub_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { field_val = end_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_mask")) { field_val = addsub_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_adj")) { field_val = mask_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { field_val = end_eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_prof_sel")) { field_val = mask_prof_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_mask_prof_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mask_en")) { field_val = mask_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_ohi")) { field_val = use_ohi(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { field_val = end_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fill")) { field_val = fill(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble")) { field_val = skip_first_nibble(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_mask_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "csum_8b")) { field_val = csum_8b(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_csum_flit_num")) { field_val = phv_csum_flit_num(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { field_val = end_eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { field_val = len_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { field_val = len_shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { field_val = len_shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { field_val = addsub_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_end")) { field_val = addsub_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { field_val = end_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_phdr")) { field_val = addsub_phdr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_adj")) { field_val = phdr_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_csum_loc")) { field_val = addsub_csum_loc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_loc_adj")) { field_val = csum_loc_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "align")) { field_val = align(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_val")) { field_val = add_val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_phdr_prof_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en")) { field_val = fld_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align")) { field_val = fld_align(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start")) { field_val = fld_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end")) { field_val = fld_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { field_val = add_len(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_phdr_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_all_fld_inst_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en")) { field_val = fld_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align")) { field_val = fld_align(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start")) { field_val = fld_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end")) { field_val = fld_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { field_val = add_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { field_val = end_eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_csum_flit_num")) { field_val = phv_csum_flit_num(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_8b")) { field_val = csum_8b(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_val")) { field_val = add_val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_all_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_len_chk_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "len_mask")) { field_val = len_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { field_val = len_shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { field_val = len_shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { field_val = addsub_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_pkt_ff_data_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "err")) { field_val = err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop")) { field_val = eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop")) { field_val = sop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_id")) { field_val = seq_id(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_align_chk_prof_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addsub_start")) { field_val = addsub_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mod_log2")) { field_val = mod_log2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_err_vec_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "offset_jump_chk")) { field_val = offset_jump_chk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size_chk")) { field_val = pkt_size_chk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "offset_out_of_range")) { field_val = offset_out_of_range(); field_found=1; }
    if(!field_found && !strcmp(field_name, "exceed_parse_loop_cnt")) { field_val = exceed_parse_loop_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "exceed_phv_flit_cnt")) { field_val = exceed_phv_flit_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_upr_idx_less")) { field_val = phv_upr_idx_less(); field_found=1; }
    if(!field_found && !strcmp(field_name, "init_lkp_idx")) { field_val = init_lkp_idx(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_mux_pkt_rslt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "mux_pkt")) { field_val = mux_pkt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_mux_inst_rslt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "mux_pkt_inst")) { field_val = mux_pkt_inst(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_extract_rslt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_meta_rslt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = inst.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_state_rslt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_state_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = lkp_key.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "tcam_hit")) { field_val = tcam_hit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_hit_idx")) { field_val = tcam_hit_idx(); field_found=1; }
    if(!field_found) { field_found = state_rslt.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "flit_idx")) { field_val = flit_idx(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_pe_step_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "init_profile_idx")) { field_val = init_profile_idx(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_global_csum_err")) { field_val = phv_global_csum_err(); field_found=1; }
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_cur_state["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__lkp_cur_state[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__stored_lkp_val0[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__lkp_val0[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__stored_lkp_val1[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__lkp_val1[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__stored_lkp_val2[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__lkp_val2[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("tcam_hit["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__tcam_hit[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("tcam_hit_idx["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__tcam_hit_idx[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("cur_flit_idx["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__cur_flit_idx[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_pkt0[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_pkt1[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_pkt2[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt3["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_pkt3[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_inst_data0[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_inst_data1[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mux_inst_data2[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("extract_phv_data["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__extract_phv_data[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_non_pe_step_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "preparser_csum_err_out")) { field_val = preparser_csum_err_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parse_end_ptr")) { field_val = parse_end_ptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_vec")) { field_val = err_vec(); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_code")) { field_val = err_code(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_val")) { field_val = crc_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { field_val = crc_err(); field_found=1; }
    for(int ii = 0; !field_found && (ii < 5); ii++) {
        if(!strcmp(field_name, string("csum_val["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__csum_val[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 5); ii++) {
        if(!strcmp(field_name, string("csum_err["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__csum_err[ii];
        }
    }
    
    if(!field_found && !strcmp(field_name, "len_chk_val0")) { field_val = len_chk_val0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err0")) { field_val = len_chk_err0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val1")) { field_val = len_chk_val1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err1")) { field_val = len_chk_err1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val2")) { field_val = len_chk_val2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err2")) { field_val = len_chk_err2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val3")) { field_val = len_chk_val3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err3")) { field_val = len_chk_err3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_val0")) { field_val = align_chk_val0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_err0")) { field_val = align_chk_err0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_val1")) { field_val = align_chk_val1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_err1")) { field_val = align_chk_err1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_mux_idx_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkpsel")) { lkpsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "idx")) { idx(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_stored_lkp")) { load_stored_lkp(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_mux_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { muxsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_val")) { mask_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "shift_val")) { shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_val")) { addsub_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "shift_left")) { shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub")) { addsub(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkpsel")) { lkpsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lkp_addsub")) { lkp_addsub(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_mux_pkt")) { load_mux_pkt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_offset_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { muxsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_lkp_val_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { muxsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "store_en")) { store_en(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_extract_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pkt_idx")) { pkt_idx(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_idx")) { phv_idx(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_meta_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phv_idx")) { phv_idx(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ohi_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sel")) { sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "muxsel")) { muxsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "idx_val")) { idx_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slot_num")) { slot_num(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel_en")) { prof_sel_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { prof_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { ohi_start_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { ohi_len_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_mask_sel")) { ohi_mask_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "unit_sel")) { unit_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { prof_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { ohi_start_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { ohi_len_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_en")) { phdr_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_sel")) { phdr_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_ohi_sel")) { phdr_ohi_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dis_zero")) { dis_zero(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "load_phdr_prof_en")) { load_phdr_prof_en(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_len_chk_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "unit_sel")) { unit_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { prof_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { ohi_start_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_len_sel")) { ohi_len_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "exact")) { exact(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_align_chk_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "en")) { en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prof_sel")) { prof_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ohi_start_sel")) { ohi_start_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "action")) { action(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nxt_state")) { nxt_state(field_val); field_found=1; }
    if(!field_found) { field_found = offset_inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "phv_idx_upr")) { phv_idx_upr(field_val); field_found=1; }
    if(!field_found) { field_found = crc_inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "offset_jump_chk_en")) { offset_jump_chk_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size_chk_en")) { pkt_size_chk_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size")) { pkt_size(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_chk_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "last_offset")) { last_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_id")) { seq_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_vec")) { err_vec(field_val); field_found=1; }
    if(!field_found) { field_found = crc_inst.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_tcam_key_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "control")) { control(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "state")) { state(field_val); field_found=1; }
    for(int ii = 0; !field_found && (ii < 3); ii++) {
        if(!strcmp(field_name, string("lkp_val["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__lkp_val[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_ppa_lkp_tcam_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = key.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mask.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "len_mask")) { len_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { len_shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { len_shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { addsub_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_end")) { addsub_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { end_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_mask")) { addsub_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_adj")) { mask_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { end_eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_prof_sel")) { mask_prof_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_mask_prof_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mask_en")) { mask_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_ohi")) { use_ohi(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { end_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fill")) { fill(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble")) { skip_first_nibble(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_crc_mask_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "csum_8b")) { csum_8b(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_csum_flit_num")) { phv_csum_flit_num(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { end_eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { len_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { len_shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { len_shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { addsub_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_end")) { addsub_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { end_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_phdr")) { addsub_phdr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_adj")) { phdr_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_csum_loc")) { addsub_csum_loc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_loc_adj")) { csum_loc_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "align")) { align(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_val")) { add_val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_phdr_prof_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en")) { fld_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align")) { fld_align(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start")) { fld_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end")) { fld_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { add_len(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_phdr_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_all_fld_inst_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en")) { fld_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align")) { fld_align(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start")) { fld_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end")) { fld_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { add_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { end_eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_csum_flit_num")) { phv_csum_flit_num(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_8b")) { csum_8b(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_val")) { add_val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_csum_all_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_len_chk_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "len_mask")) { len_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { len_shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { len_shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addsub_start")) { addsub_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_pkt_ff_data_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "err")) { err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop")) { eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop")) { sop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "seq_id")) { seq_id(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_align_chk_prof_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addsub_start")) { addsub_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mod_log2")) { mod_log2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_err_vec_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "offset_jump_chk")) { offset_jump_chk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_size_chk")) { pkt_size_chk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "offset_out_of_range")) { offset_out_of_range(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "exceed_parse_loop_cnt")) { exceed_parse_loop_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "exceed_phv_flit_cnt")) { exceed_phv_flit_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_upr_idx_less")) { phv_upr_idx_less(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "init_lkp_idx")) { init_lkp_idx(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_mux_pkt_rslt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "mux_pkt")) { mux_pkt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_mux_inst_rslt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "mux_pkt_inst")) { mux_pkt_inst(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_extract_rslt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_meta_rslt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = inst.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_state_rslt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_state_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = lkp_key.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "tcam_hit")) { tcam_hit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tcam_hit_idx")) { tcam_hit_idx(field_val); field_found=1; }
    if(!field_found) { field_found = state_rslt.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "flit_idx")) { flit_idx(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_pe_step_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "init_profile_idx")) { init_profile_idx(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_global_csum_err")) { phv_global_csum_err(field_val); field_found=1; }
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_cur_state["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__lkp_cur_state[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__stored_lkp_val0[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__lkp_val0[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__stored_lkp_val1[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__lkp_val1[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("stored_lkp_val2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__stored_lkp_val2[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("lkp_val2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__lkp_val2[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("tcam_hit["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__tcam_hit[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("tcam_hit_idx["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__tcam_hit_idx[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("cur_flit_idx["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__cur_flit_idx[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_pkt0[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_pkt1[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_pkt2[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_pkt3["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_pkt3[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data0["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_inst_data0[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data1["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_inst_data1[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("mux_inst_data2["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mux_inst_data2[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 100); ii++) {
        if(!strcmp(field_name, string("extract_phv_data["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__extract_phv_data[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ppa_decoders_si_non_pe_step_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "preparser_csum_err_out")) { preparser_csum_err_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parse_end_ptr")) { parse_end_ptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_vec")) { err_vec(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "err_code")) { err_code(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_val")) { crc_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { crc_err(field_val); field_found=1; }
    for(int ii = 0; !field_found && (ii < 5); ii++) {
        if(!strcmp(field_name, string("csum_val["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__csum_val[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 5); ii++) {
        if(!strcmp(field_name, string("csum_err["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__csum_err[ii] = field_val;
        }
    }
    
    if(!field_found && !strcmp(field_name, "len_chk_val0")) { len_chk_val0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err0")) { len_chk_err0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val1")) { len_chk_val1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err1")) { len_chk_err1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val2")) { len_chk_val2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err2")) { len_chk_err2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_val3")) { len_chk_val3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_chk_err3")) { len_chk_err3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_val0")) { align_chk_val0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_err0")) { align_chk_err0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_val1")) { align_chk_val1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "align_chk_err1")) { align_chk_err1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_mux_idx_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel");
    ret_vec.push_back("lkpsel");
    ret_vec.push_back("idx");
    ret_vec.push_back("load_stored_lkp");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_mux_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel");
    ret_vec.push_back("muxsel");
    ret_vec.push_back("mask_val");
    ret_vec.push_back("shift_val");
    ret_vec.push_back("addsub_val");
    ret_vec.push_back("shift_left");
    ret_vec.push_back("addsub");
    ret_vec.push_back("lkpsel");
    ret_vec.push_back("lkp_addsub");
    ret_vec.push_back("load_mux_pkt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_offset_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel");
    ret_vec.push_back("muxsel");
    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_lkp_val_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel");
    ret_vec.push_back("muxsel");
    ret_vec.push_back("store_en");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_extract_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pkt_idx");
    ret_vec.push_back("len");
    ret_vec.push_back("phv_idx");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_meta_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("phv_idx");
    ret_vec.push_back("val");
    ret_vec.push_back("sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_ohi_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sel");
    ret_vec.push_back("muxsel");
    ret_vec.push_back("idx_val");
    ret_vec.push_back("slot_num");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_crc_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("en");
    ret_vec.push_back("prof_sel_en");
    ret_vec.push_back("prof_sel");
    ret_vec.push_back("ohi_start_sel");
    ret_vec.push_back("ohi_len_sel");
    ret_vec.push_back("ohi_mask_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("en");
    ret_vec.push_back("unit_sel");
    ret_vec.push_back("prof_sel");
    ret_vec.push_back("ohi_start_sel");
    ret_vec.push_back("ohi_len_sel");
    ret_vec.push_back("phdr_en");
    ret_vec.push_back("phdr_sel");
    ret_vec.push_back("phdr_ohi_sel");
    ret_vec.push_back("dis_zero");
    ret_vec.push_back("load_phdr_prof_en");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_len_chk_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("en");
    ret_vec.push_back("unit_sel");
    ret_vec.push_back("prof_sel");
    ret_vec.push_back("ohi_start_sel");
    ret_vec.push_back("ohi_len_sel");
    ret_vec.push_back("exact");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_align_chk_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("en");
    ret_vec.push_back("prof_sel");
    ret_vec.push_back("ohi_start_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_ppa_lkp_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("action");
    ret_vec.push_back("nxt_state");
    {
        for(auto tmp_vec : offset_inst.get_fields(level-1)) {
            ret_vec.push_back("offset_inst." + tmp_vec);
        }
    }
    ret_vec.push_back("phv_idx_upr");
    {
        for(auto tmp_vec : crc_inst.get_fields(level-1)) {
            ret_vec.push_back("crc_inst." + tmp_vec);
        }
    }
    ret_vec.push_back("offset_jump_chk_en");
    ret_vec.push_back("pkt_size_chk_en");
    ret_vec.push_back("pkt_size");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_chk_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("last_offset");
    ret_vec.push_back("seq_id");
    ret_vec.push_back("err_vec");
    {
        for(auto tmp_vec : crc_inst.get_fields(level-1)) {
            ret_vec.push_back("crc_inst." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_ppa_lkp_tcam_key_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("control");
    ret_vec.push_back("state");
    for(int ii = 0; ii < 3; ii++) {
        ret_vec.push_back("lkp_val["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_ppa_lkp_tcam_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : key.get_fields(level-1)) {
            ret_vec.push_back("key." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mask.get_fields(level-1)) {
            ret_vec.push_back("mask." + tmp_vec);
        }
    }
    ret_vec.push_back("valid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_crc_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("len_mask");
    ret_vec.push_back("len_shift_left");
    ret_vec.push_back("len_shift_val");
    ret_vec.push_back("addsub_start");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("addsub_end");
    ret_vec.push_back("end_adj");
    ret_vec.push_back("addsub_mask");
    ret_vec.push_back("mask_adj");
    ret_vec.push_back("end_eop");
    ret_vec.push_back("mask_prof_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_crc_mask_prof_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mask_en");
    ret_vec.push_back("use_ohi");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("end_adj");
    ret_vec.push_back("fill");
    ret_vec.push_back("skip_first_nibble");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_crc_mask_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("csum_8b");
    ret_vec.push_back("phv_csum_flit_num");
    ret_vec.push_back("end_eop");
    ret_vec.push_back("len_mask");
    ret_vec.push_back("len_shift_left");
    ret_vec.push_back("len_shift_val");
    ret_vec.push_back("addsub_start");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("addsub_end");
    ret_vec.push_back("end_adj");
    ret_vec.push_back("addsub_phdr");
    ret_vec.push_back("phdr_adj");
    ret_vec.push_back("addsub_csum_loc");
    ret_vec.push_back("csum_loc_adj");
    ret_vec.push_back("align");
    ret_vec.push_back("add_val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_phdr_prof_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fld_en");
    ret_vec.push_back("fld_align");
    ret_vec.push_back("fld_start");
    ret_vec.push_back("fld_end");
    ret_vec.push_back("add_len");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_phdr_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_all_fld_inst_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fld_en");
    ret_vec.push_back("fld_align");
    ret_vec.push_back("fld_start");
    ret_vec.push_back("fld_end");
    ret_vec.push_back("add_len");
    ret_vec.push_back("end_eop");
    ret_vec.push_back("phv_csum_flit_num");
    ret_vec.push_back("csum_8b");
    ret_vec.push_back("add_val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_csum_all_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_len_chk_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("len_mask");
    ret_vec.push_back("len_shift_left");
    ret_vec.push_back("len_shift_val");
    ret_vec.push_back("addsub_start");
    ret_vec.push_back("start_adj");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_pkt_ff_data_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("sz");
    ret_vec.push_back("err");
    ret_vec.push_back("eop");
    ret_vec.push_back("sop");
    ret_vec.push_back("seq_id");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_align_chk_prof_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addsub_start");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("mod_log2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_err_vec_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("offset_jump_chk");
    ret_vec.push_back("pkt_size_chk");
    ret_vec.push_back("offset_out_of_range");
    ret_vec.push_back("exceed_parse_loop_cnt");
    ret_vec.push_back("exceed_phv_flit_cnt");
    ret_vec.push_back("phv_upr_idx_less");
    ret_vec.push_back("init_lkp_idx");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_mux_pkt_rslt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : inst.get_fields(level-1)) {
            ret_vec.push_back("inst." + tmp_vec);
        }
    }
    ret_vec.push_back("mux_pkt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_mux_inst_rslt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : inst.get_fields(level-1)) {
            ret_vec.push_back("inst." + tmp_vec);
        }
    }
    ret_vec.push_back("mux_pkt_inst");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_extract_rslt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : inst.get_fields(level-1)) {
            ret_vec.push_back("inst." + tmp_vec);
        }
    }
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_meta_rslt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : inst.get_fields(level-1)) {
            ret_vec.push_back("inst." + tmp_vec);
        }
    }
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_state_rslt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_state_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : lkp_key.get_fields(level-1)) {
            ret_vec.push_back("lkp_key." + tmp_vec);
        }
    }
    ret_vec.push_back("tcam_hit");
    ret_vec.push_back("tcam_hit_idx");
    {
        for(auto tmp_vec : state_rslt.get_fields(level-1)) {
            ret_vec.push_back("state_rslt." + tmp_vec);
        }
    }
    ret_vec.push_back("flit_idx");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_pe_step_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("init_profile_idx");
    ret_vec.push_back("phv_global_csum_err");
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("lkp_cur_state["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("stored_lkp_val0["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("lkp_val0["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("stored_lkp_val1["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("lkp_val1["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("stored_lkp_val2["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("lkp_val2["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("tcam_hit["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("tcam_hit_idx["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("cur_flit_idx["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_pkt0["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_pkt1["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_pkt2["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_pkt3["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_inst_data0["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_inst_data1["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("mux_inst_data2["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 100; ii++) {
        ret_vec.push_back("extract_phv_data["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ppa_decoders_si_non_pe_step_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("preparser_csum_err_out");
    ret_vec.push_back("parse_end_ptr");
    ret_vec.push_back("err_vec");
    ret_vec.push_back("err_code");
    ret_vec.push_back("crc_val");
    ret_vec.push_back("crc_err");
    for(int ii = 0; ii < 5; ii++) {
        ret_vec.push_back("csum_val["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 5; ii++) {
        ret_vec.push_back("csum_err["+to_string(ii)+"]");
    }
    
    ret_vec.push_back("len_chk_val0");
    ret_vec.push_back("len_chk_err0");
    ret_vec.push_back("len_chk_val1");
    ret_vec.push_back("len_chk_err1");
    ret_vec.push_back("len_chk_val2");
    ret_vec.push_back("len_chk_err2");
    ret_vec.push_back("len_chk_val3");
    ret_vec.push_back("len_chk_err3");
    ret_vec.push_back("align_chk_val0");
    ret_vec.push_back("align_chk_err0");
    ret_vec.push_back("align_chk_val1");
    ret_vec.push_back("align_chk_err1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
