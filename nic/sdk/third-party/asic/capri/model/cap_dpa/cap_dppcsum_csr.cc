
#include "cap_dppcsum_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dppcsum_csr_cfg_spare_csum_t::cap_dppcsum_csr_cfg_spare_csum_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_spare_csum_t::~cap_dppcsum_csr_cfg_spare_csum_t() { }

cap_dppcsum_csr_cfg_crc_mask_profile_t::cap_dppcsum_csr_cfg_crc_mask_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_crc_mask_profile_t::~cap_dppcsum_csr_cfg_crc_mask_profile_t() { }

cap_dppcsum_csr_cfg_crc_profile_t::cap_dppcsum_csr_cfg_crc_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_crc_profile_t::~cap_dppcsum_csr_cfg_crc_profile_t() { }

cap_dppcsum_csr_cfg_crc_hdrs_t::cap_dppcsum_csr_cfg_crc_hdrs_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_crc_hdrs_t::~cap_dppcsum_csr_cfg_crc_hdrs_t() { }

cap_dppcsum_csr_cfg_csum_phdr_profile_t::cap_dppcsum_csr_cfg_csum_phdr_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_csum_phdr_profile_t::~cap_dppcsum_csr_cfg_csum_phdr_profile_t() { }

cap_dppcsum_csr_cfg_csum_profile_t::cap_dppcsum_csr_cfg_csum_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_csum_profile_t::~cap_dppcsum_csr_cfg_csum_profile_t() { }

cap_dppcsum_csr_cfg_csum_hdrs_t::cap_dppcsum_csr_cfg_csum_hdrs_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppcsum_csr_cfg_csum_hdrs_t::~cap_dppcsum_csr_cfg_csum_hdrs_t() { }

cap_dppcsum_csr_t::cap_dppcsum_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(4096);
        set_attributes(0,get_name(), 0);
        }
cap_dppcsum_csr_t::~cap_dppcsum_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_spare_csum_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_crc_mask_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_0: 0x" << int_var__fld_en_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_sub_0: 0x" << int_var__fld_start_sub_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_0: 0x" << int_var__fld_start_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_sub_0: 0x" << int_var__fld_end_sub_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_0: 0x" << int_var__fld_end_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_0: 0x" << int_var__skip_first_nibble_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_1: 0x" << int_var__fld_en_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_sub_1: 0x" << int_var__fld_start_sub_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_1: 0x" << int_var__fld_start_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_sub_1: 0x" << int_var__fld_end_sub_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_1: 0x" << int_var__fld_end_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_1: 0x" << int_var__skip_first_nibble_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_2: 0x" << int_var__fld_en_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_sub_2: 0x" << int_var__fld_start_sub_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_2: 0x" << int_var__fld_start_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_sub_2: 0x" << int_var__fld_end_sub_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_2: 0x" << int_var__fld_end_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_2: 0x" << int_var__skip_first_nibble_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_3: 0x" << int_var__fld_en_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_sub_3: 0x" << int_var__fld_start_sub_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_3: 0x" << int_var__fld_start_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_sub_3: 0x" << int_var__fld_end_sub_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_3: 0x" << int_var__fld_end_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_3: 0x" << int_var__skip_first_nibble_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_4: 0x" << int_var__fld_en_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_sub_4: 0x" << int_var__fld_start_sub_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_4: 0x" << int_var__fld_start_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_sub_4: 0x" << int_var__fld_end_sub_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_4: 0x" << int_var__fld_end_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_4: 0x" << int_var__skip_first_nibble_4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fill: 0x" << int_var__fill << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_crc_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".use_phv_len: 0x" << int_var__use_phv_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_len_sel: 0x" << int_var__phv_len_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_sop: 0x" << int_var__start_sop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_eop: 0x" << int_var__end_eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj_sub: 0x" << int_var__start_adj_sub << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj_sub: 0x" << int_var__end_adj_sub << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loc_adj_sub: 0x" << int_var__loc_adj_sub << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loc_adj: 0x" << int_var__loc_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_unit_include_bm: 0x" << int_var__crc_unit_include_bm << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_fix_mask: 0x" << int_var__add_fix_mask << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_crc_hdrs_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_num: 0x" << int_var__hdr_num << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_vld: 0x" << int_var__crc_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_unit: 0x" << int_var__crc_unit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_profile: 0x" << int_var__crc_profile << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_vld: 0x" << int_var__mask_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_unit: 0x" << int_var__mask_unit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_profile: 0x" << int_var__mask_profile << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_start: 0x" << int_var__hdrfld_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_end: 0x" << int_var__hdrfld_end << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_0: 0x" << int_var__fld_en_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_0: 0x" << int_var__fld_start_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_0: 0x" << int_var__fld_end_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_0: 0x" << int_var__fld_align_0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_1: 0x" << int_var__fld_en_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_1: 0x" << int_var__fld_start_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_1: 0x" << int_var__fld_end_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_1: 0x" << int_var__fld_align_1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_2: 0x" << int_var__fld_en_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_2: 0x" << int_var__fld_start_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_2: 0x" << int_var__fld_end_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_2: 0x" << int_var__fld_align_2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_3: 0x" << int_var__fld_en_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_3: 0x" << int_var__fld_start_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_3: 0x" << int_var__fld_end_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_3: 0x" << int_var__fld_align_3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_csum_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".use_phv_len: 0x" << int_var__use_phv_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_len_sel: 0x" << int_var__phv_len_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".loc_adj: 0x" << int_var__loc_adj << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".eight_bit: 0x" << int_var__eight_bit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".invert_zero: 0x" << int_var__invert_zero << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".no_csum_rw: 0x" << int_var__no_csum_rw << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_next_hdr: 0x" << int_var__phdr_next_hdr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_cfg_csum_hdrs_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_num: 0x" << int_var__hdr_num << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_vld: 0x" << int_var__csum_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_copy_vld: 0x" << int_var__csum_copy_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_unit: 0x" << int_var__csum_unit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_profile: 0x" << int_var__csum_profile << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_vld: 0x" << int_var__phdr_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_unit: 0x" << int_var__phdr_unit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_profile: 0x" << int_var__phdr_profile << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_unit_include_bm: 0x" << int_var__csum_unit_include_bm << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_include_bm: 0x" << int_var__crc_include_bm << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_start: 0x" << int_var__hdrfld_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_end: 0x" << int_var__hdrfld_end << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dppcsum_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_hdrs.show(); // large_array
    #else
    for(int ii = 0; ii < 24; ii++) {
        cfg_csum_hdrs[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_profile[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_phdr_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_phdr_profile[ii].show();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_hdrs.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_hdrs[ii].show();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_profile[ii].show();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_mask_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_mask_profile[ii].show();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_csum.show(); // large_array
    #else
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_csum[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_dppcsum_csr_cfg_spare_csum_t::get_width() const {
    return cap_dppcsum_csr_cfg_spare_csum_t::s_get_width();

}

int cap_dppcsum_csr_cfg_crc_mask_profile_t::get_width() const {
    return cap_dppcsum_csr_cfg_crc_mask_profile_t::s_get_width();

}

int cap_dppcsum_csr_cfg_crc_profile_t::get_width() const {
    return cap_dppcsum_csr_cfg_crc_profile_t::s_get_width();

}

int cap_dppcsum_csr_cfg_crc_hdrs_t::get_width() const {
    return cap_dppcsum_csr_cfg_crc_hdrs_t::s_get_width();

}

int cap_dppcsum_csr_cfg_csum_phdr_profile_t::get_width() const {
    return cap_dppcsum_csr_cfg_csum_phdr_profile_t::s_get_width();

}

int cap_dppcsum_csr_cfg_csum_profile_t::get_width() const {
    return cap_dppcsum_csr_cfg_csum_profile_t::s_get_width();

}

int cap_dppcsum_csr_cfg_csum_hdrs_t::get_width() const {
    return cap_dppcsum_csr_cfg_csum_hdrs_t::s_get_width();

}

int cap_dppcsum_csr_t::get_width() const {
    return cap_dppcsum_csr_t::s_get_width();

}

int cap_dppcsum_csr_cfg_spare_csum_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_dppcsum_csr_cfg_crc_mask_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // fld_en_0
    _count += 1; // fld_start_sub_0
    _count += 14; // fld_start_0
    _count += 1; // fld_end_sub_0
    _count += 14; // fld_end_0
    _count += 1; // skip_first_nibble_0
    _count += 1; // fld_en_1
    _count += 1; // fld_start_sub_1
    _count += 14; // fld_start_1
    _count += 1; // fld_end_sub_1
    _count += 14; // fld_end_1
    _count += 1; // skip_first_nibble_1
    _count += 1; // fld_en_2
    _count += 1; // fld_start_sub_2
    _count += 14; // fld_start_2
    _count += 1; // fld_end_sub_2
    _count += 14; // fld_end_2
    _count += 1; // skip_first_nibble_2
    _count += 1; // fld_en_3
    _count += 1; // fld_start_sub_3
    _count += 14; // fld_start_3
    _count += 1; // fld_end_sub_3
    _count += 14; // fld_end_3
    _count += 1; // skip_first_nibble_3
    _count += 1; // fld_en_4
    _count += 1; // fld_start_sub_4
    _count += 14; // fld_start_4
    _count += 1; // fld_end_sub_4
    _count += 14; // fld_end_4
    _count += 1; // skip_first_nibble_4
    _count += 1; // fill
    return _count;
}

int cap_dppcsum_csr_cfg_crc_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // use_phv_len
    _count += 6; // phv_len_sel
    _count += 16; // len_mask
    _count += 1; // len_shift_left
    _count += 4; // len_shift_val
    _count += 1; // start_sop
    _count += 1; // end_eop
    _count += 1; // start_adj_sub
    _count += 14; // start_adj
    _count += 1; // end_adj_sub
    _count += 14; // end_adj
    _count += 1; // loc_adj_sub
    _count += 14; // loc_adj
    _count += 1; // crc_unit_include_bm
    _count += 1; // add_fix_mask
    return _count;
}

int cap_dppcsum_csr_cfg_crc_hdrs_t::s_get_width() {
    int _count = 0;

    _count += 7; // hdr_num
    _count += 1; // crc_vld
    _count += 1; // crc_unit
    _count += 4; // crc_profile
    _count += 1; // mask_vld
    _count += 1; // mask_unit
    _count += 4; // mask_profile
    _count += 8; // hdrfld_start
    _count += 8; // hdrfld_end
    return _count;
}

int cap_dppcsum_csr_cfg_csum_phdr_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // fld_en_0
    _count += 14; // fld_start_0
    _count += 14; // fld_end_0
    _count += 1; // fld_align_0
    _count += 1; // fld_en_1
    _count += 14; // fld_start_1
    _count += 14; // fld_end_1
    _count += 1; // fld_align_1
    _count += 1; // fld_en_2
    _count += 14; // fld_start_2
    _count += 14; // fld_end_2
    _count += 1; // fld_align_2
    _count += 1; // fld_en_3
    _count += 14; // fld_start_3
    _count += 14; // fld_end_3
    _count += 1; // fld_align_3
    _count += 1; // add_len
    return _count;
}

int cap_dppcsum_csr_cfg_csum_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // use_phv_len
    _count += 6; // phv_len_sel
    _count += 16; // len_mask
    _count += 1; // len_shift_left
    _count += 4; // len_shift_val
    _count += 14; // start_adj
    _count += 14; // end_adj
    _count += 14; // loc_adj
    _count += 1; // add_len
    _count += 1; // eight_bit
    _count += 1; // invert_zero
    _count += 1; // no_csum_rw
    _count += 16; // phdr_next_hdr
    return _count;
}

int cap_dppcsum_csr_cfg_csum_hdrs_t::s_get_width() {
    int _count = 0;

    _count += 7; // hdr_num
    _count += 1; // csum_vld
    _count += 1; // csum_copy_vld
    _count += 3; // csum_unit
    _count += 4; // csum_profile
    _count += 1; // phdr_vld
    _count += 3; // phdr_unit
    _count += 4; // phdr_profile
    _count += 5; // csum_unit_include_bm
    _count += 1; // crc_include_bm
    _count += 8; // hdrfld_start
    _count += 8; // hdrfld_end
    return _count;
}

int cap_dppcsum_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dppcsum_csr_cfg_csum_hdrs_t::s_get_width() * 24); // cfg_csum_hdrs
    _count += (cap_dppcsum_csr_cfg_csum_profile_t::s_get_width() * 16); // cfg_csum_profile
    _count += (cap_dppcsum_csr_cfg_csum_phdr_profile_t::s_get_width() * 16); // cfg_csum_phdr_profile
    _count += (cap_dppcsum_csr_cfg_crc_hdrs_t::s_get_width() * 16); // cfg_crc_hdrs
    _count += (cap_dppcsum_csr_cfg_crc_profile_t::s_get_width() * 12); // cfg_crc_profile
    _count += (cap_dppcsum_csr_cfg_crc_mask_profile_t::s_get_width() * 12); // cfg_crc_mask_profile
    _count += (cap_dppcsum_csr_cfg_spare_csum_t::s_get_width() * 32); // cfg_spare_csum
    return _count;
}

void cap_dppcsum_csr_cfg_spare_csum_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_sub_0 = _val.convert_to< fld_start_sub_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_sub_0 = _val.convert_to< fld_end_sub_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_first_nibble_0 = _val.convert_to< skip_first_nibble_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_sub_1 = _val.convert_to< fld_start_sub_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_sub_1 = _val.convert_to< fld_end_sub_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_first_nibble_1 = _val.convert_to< skip_first_nibble_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_sub_2 = _val.convert_to< fld_start_sub_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_sub_2 = _val.convert_to< fld_end_sub_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_first_nibble_2 = _val.convert_to< skip_first_nibble_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_sub_3 = _val.convert_to< fld_start_sub_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_sub_3 = _val.convert_to< fld_end_sub_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_first_nibble_3 = _val.convert_to< skip_first_nibble_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_4 = _val.convert_to< fld_en_4_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_sub_4 = _val.convert_to< fld_start_sub_4_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_4 = _val.convert_to< fld_start_4_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_sub_4 = _val.convert_to< fld_end_sub_4_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_end_4 = _val.convert_to< fld_end_4_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_first_nibble_4 = _val.convert_to< skip_first_nibble_4_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fill = _val.convert_to< fill_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dppcsum_csr_cfg_crc_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__start_sop = _val.convert_to< start_sop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj_sub = _val.convert_to< start_adj_sub_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__end_adj_sub = _val.convert_to< end_adj_sub_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__loc_adj_sub = _val.convert_to< loc_adj_sub_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__crc_unit_include_bm = _val.convert_to< crc_unit_include_bm_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__add_fix_mask = _val.convert_to< add_fix_mask_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dppcsum_csr_cfg_crc_hdrs_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__crc_vld = _val.convert_to< crc_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__crc_unit = _val.convert_to< crc_unit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__crc_profile = _val.convert_to< crc_profile_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__mask_vld = _val.convert_to< mask_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mask_unit = _val.convert_to< mask_unit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mask_profile = _val.convert_to< mask_profile_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_align_0 = _val.convert_to< fld_align_0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_align_1 = _val.convert_to< fld_align_1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_align_2 = _val.convert_to< fld_align_2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__fld_align_3 = _val.convert_to< fld_align_3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dppcsum_csr_cfg_csum_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__eight_bit = _val.convert_to< eight_bit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__invert_zero = _val.convert_to< invert_zero_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__no_csum_rw = _val.convert_to< no_csum_rw_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phdr_next_hdr = _val.convert_to< phdr_next_hdr_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_dppcsum_csr_cfg_csum_hdrs_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__csum_vld = _val.convert_to< csum_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__csum_copy_vld = _val.convert_to< csum_copy_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__csum_unit = _val.convert_to< csum_unit_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__csum_profile = _val.convert_to< csum_profile_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phdr_vld = _val.convert_to< phdr_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phdr_unit = _val.convert_to< phdr_unit_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phdr_profile = _val.convert_to< phdr_profile_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__csum_unit_include_bm = _val.convert_to< csum_unit_include_bm_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__crc_include_bm = _val.convert_to< crc_include_bm_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_dppcsum_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_hdrs
    for(int ii = 0; ii < 24; ii++) {
        cfg_csum_hdrs[ii].all(_val);
        _val = _val >> cfg_csum_hdrs[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_profile[ii].all(_val);
        _val = _val >> cfg_csum_profile[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_phdr_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_phdr_profile[ii].all(_val);
        _val = _val >> cfg_csum_phdr_profile[ii].get_width();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_hdrs
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_hdrs[ii].all(_val);
        _val = _val >> cfg_crc_hdrs[ii].get_width();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_profile
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_profile[ii].all(_val);
        _val = _val >> cfg_crc_profile[ii].get_width();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_mask_profile
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_mask_profile[ii].all(_val);
        _val = _val >> cfg_crc_mask_profile[ii].get_width();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_csum
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_csum[ii].all(_val);
        _val = _val >> cfg_spare_csum[ii].get_width();
    }
    #endif
    
}

cpp_int cap_dppcsum_csr_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_csum
    for(int ii = 32-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_spare_csum[ii].get_width(); ret_val = ret_val  | cfg_spare_csum[ii].all(); 
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_mask_profile
    for(int ii = 12-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_crc_mask_profile[ii].get_width(); ret_val = ret_val  | cfg_crc_mask_profile[ii].all(); 
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_profile
    for(int ii = 12-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_crc_profile[ii].get_width(); ret_val = ret_val  | cfg_crc_profile[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_crc_hdrs
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_crc_hdrs[ii].get_width(); ret_val = ret_val  | cfg_crc_hdrs[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_phdr_profile
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_csum_phdr_profile[ii].get_width(); ret_val = ret_val  | cfg_csum_phdr_profile[ii].all(); 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_profile
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_csum_profile[ii].get_width(); ret_val = ret_val  | cfg_csum_profile[ii].all(); 
    }
    #endif
    
    #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_csum_hdrs
    for(int ii = 24-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_csum_hdrs[ii].get_width(); ret_val = ret_val  | cfg_csum_hdrs[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::all() const {
    cpp_int ret_val;

    // hdrfld_end
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__hdrfld_end; 
    
    // hdrfld_start
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__hdrfld_start; 
    
    // crc_include_bm
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_include_bm; 
    
    // csum_unit_include_bm
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__csum_unit_include_bm; 
    
    // phdr_profile
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phdr_profile; 
    
    // phdr_unit
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phdr_unit; 
    
    // phdr_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phdr_vld; 
    
    // csum_profile
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__csum_profile; 
    
    // csum_unit
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__csum_unit; 
    
    // csum_copy_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__csum_copy_vld; 
    
    // csum_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__csum_vld; 
    
    // hdr_num
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__hdr_num; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::all() const {
    cpp_int ret_val;

    // phdr_next_hdr
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__phdr_next_hdr; 
    
    // no_csum_rw
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__no_csum_rw; 
    
    // invert_zero
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__invert_zero; 
    
    // eight_bit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__eight_bit; 
    
    // add_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__add_len; 
    
    // loc_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__loc_adj; 
    
    // end_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__end_adj; 
    
    // start_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__start_adj; 
    
    // len_shift_val
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len_shift_val; 
    
    // len_shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_shift_left; 
    
    // len_mask
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__len_mask; 
    
    // phv_len_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__phv_len_sel; 
    
    // use_phv_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_phv_len; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::all() const {
    cpp_int ret_val;

    // add_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__add_len; 
    
    // fld_align_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align_3; 
    
    // fld_end_3
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_3; 
    
    // fld_start_3
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_3; 
    
    // fld_en_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_3; 
    
    // fld_align_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align_2; 
    
    // fld_end_2
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_2; 
    
    // fld_start_2
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_2; 
    
    // fld_en_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_2; 
    
    // fld_align_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align_1; 
    
    // fld_end_1
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_1; 
    
    // fld_start_1
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_1; 
    
    // fld_en_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_1; 
    
    // fld_align_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_align_0; 
    
    // fld_end_0
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_0; 
    
    // fld_start_0
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_0; 
    
    // fld_en_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_0; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::all() const {
    cpp_int ret_val;

    // hdrfld_end
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__hdrfld_end; 
    
    // hdrfld_start
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__hdrfld_start; 
    
    // mask_profile
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mask_profile; 
    
    // mask_unit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask_unit; 
    
    // mask_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask_vld; 
    
    // crc_profile
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__crc_profile; 
    
    // crc_unit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_unit; 
    
    // crc_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_vld; 
    
    // hdr_num
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__hdr_num; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::all() const {
    cpp_int ret_val;

    // add_fix_mask
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__add_fix_mask; 
    
    // crc_unit_include_bm
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_unit_include_bm; 
    
    // loc_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__loc_adj; 
    
    // loc_adj_sub
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__loc_adj_sub; 
    
    // end_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__end_adj; 
    
    // end_adj_sub
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__end_adj_sub; 
    
    // start_adj
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__start_adj; 
    
    // start_adj_sub
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__start_adj_sub; 
    
    // end_eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__end_eop; 
    
    // start_sop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__start_sop; 
    
    // len_shift_val
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len_shift_val; 
    
    // len_shift_left
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__len_shift_left; 
    
    // len_mask
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__len_mask; 
    
    // phv_len_sel
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__phv_len_sel; 
    
    // use_phv_len
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_phv_len; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::all() const {
    cpp_int ret_val;

    // fill
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fill; 
    
    // skip_first_nibble_4
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble_4; 
    
    // fld_end_4
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_4; 
    
    // fld_end_sub_4
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_end_sub_4; 
    
    // fld_start_4
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_4; 
    
    // fld_start_sub_4
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_start_sub_4; 
    
    // fld_en_4
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_4; 
    
    // skip_first_nibble_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble_3; 
    
    // fld_end_3
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_3; 
    
    // fld_end_sub_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_end_sub_3; 
    
    // fld_start_3
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_3; 
    
    // fld_start_sub_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_start_sub_3; 
    
    // fld_en_3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_3; 
    
    // skip_first_nibble_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble_2; 
    
    // fld_end_2
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_2; 
    
    // fld_end_sub_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_end_sub_2; 
    
    // fld_start_2
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_2; 
    
    // fld_start_sub_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_start_sub_2; 
    
    // fld_en_2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_2; 
    
    // skip_first_nibble_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble_1; 
    
    // fld_end_1
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_1; 
    
    // fld_end_sub_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_end_sub_1; 
    
    // fld_start_1
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_1; 
    
    // fld_start_sub_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_start_sub_1; 
    
    // fld_en_1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_1; 
    
    // skip_first_nibble_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_first_nibble_0; 
    
    // fld_end_0
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_end_0; 
    
    // fld_end_sub_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_end_sub_0; 
    
    // fld_start_0
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__fld_start_0; 
    
    // fld_start_sub_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_start_sub_0; 
    
    // fld_en_0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fld_en_0; 
    
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_spare_csum_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_dppcsum_csr_cfg_spare_csum_t::clear() {

    int_var__data = 0; 
    
}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::clear() {

    int_var__fld_en_0 = 0; 
    
    int_var__fld_start_sub_0 = 0; 
    
    int_var__fld_start_0 = 0; 
    
    int_var__fld_end_sub_0 = 0; 
    
    int_var__fld_end_0 = 0; 
    
    int_var__skip_first_nibble_0 = 0; 
    
    int_var__fld_en_1 = 0; 
    
    int_var__fld_start_sub_1 = 0; 
    
    int_var__fld_start_1 = 0; 
    
    int_var__fld_end_sub_1 = 0; 
    
    int_var__fld_end_1 = 0; 
    
    int_var__skip_first_nibble_1 = 0; 
    
    int_var__fld_en_2 = 0; 
    
    int_var__fld_start_sub_2 = 0; 
    
    int_var__fld_start_2 = 0; 
    
    int_var__fld_end_sub_2 = 0; 
    
    int_var__fld_end_2 = 0; 
    
    int_var__skip_first_nibble_2 = 0; 
    
    int_var__fld_en_3 = 0; 
    
    int_var__fld_start_sub_3 = 0; 
    
    int_var__fld_start_3 = 0; 
    
    int_var__fld_end_sub_3 = 0; 
    
    int_var__fld_end_3 = 0; 
    
    int_var__skip_first_nibble_3 = 0; 
    
    int_var__fld_en_4 = 0; 
    
    int_var__fld_start_sub_4 = 0; 
    
    int_var__fld_start_4 = 0; 
    
    int_var__fld_end_sub_4 = 0; 
    
    int_var__fld_end_4 = 0; 
    
    int_var__skip_first_nibble_4 = 0; 
    
    int_var__fill = 0; 
    
}

void cap_dppcsum_csr_cfg_crc_profile_t::clear() {

    int_var__use_phv_len = 0; 
    
    int_var__phv_len_sel = 0; 
    
    int_var__len_mask = 0; 
    
    int_var__len_shift_left = 0; 
    
    int_var__len_shift_val = 0; 
    
    int_var__start_sop = 0; 
    
    int_var__end_eop = 0; 
    
    int_var__start_adj_sub = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__end_adj_sub = 0; 
    
    int_var__end_adj = 0; 
    
    int_var__loc_adj_sub = 0; 
    
    int_var__loc_adj = 0; 
    
    int_var__crc_unit_include_bm = 0; 
    
    int_var__add_fix_mask = 0; 
    
}

void cap_dppcsum_csr_cfg_crc_hdrs_t::clear() {

    int_var__hdr_num = 0; 
    
    int_var__crc_vld = 0; 
    
    int_var__crc_unit = 0; 
    
    int_var__crc_profile = 0; 
    
    int_var__mask_vld = 0; 
    
    int_var__mask_unit = 0; 
    
    int_var__mask_profile = 0; 
    
    int_var__hdrfld_start = 0; 
    
    int_var__hdrfld_end = 0; 
    
}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::clear() {

    int_var__fld_en_0 = 0; 
    
    int_var__fld_start_0 = 0; 
    
    int_var__fld_end_0 = 0; 
    
    int_var__fld_align_0 = 0; 
    
    int_var__fld_en_1 = 0; 
    
    int_var__fld_start_1 = 0; 
    
    int_var__fld_end_1 = 0; 
    
    int_var__fld_align_1 = 0; 
    
    int_var__fld_en_2 = 0; 
    
    int_var__fld_start_2 = 0; 
    
    int_var__fld_end_2 = 0; 
    
    int_var__fld_align_2 = 0; 
    
    int_var__fld_en_3 = 0; 
    
    int_var__fld_start_3 = 0; 
    
    int_var__fld_end_3 = 0; 
    
    int_var__fld_align_3 = 0; 
    
    int_var__add_len = 0; 
    
}

void cap_dppcsum_csr_cfg_csum_profile_t::clear() {

    int_var__use_phv_len = 0; 
    
    int_var__phv_len_sel = 0; 
    
    int_var__len_mask = 0; 
    
    int_var__len_shift_left = 0; 
    
    int_var__len_shift_val = 0; 
    
    int_var__start_adj = 0; 
    
    int_var__end_adj = 0; 
    
    int_var__loc_adj = 0; 
    
    int_var__add_len = 0; 
    
    int_var__eight_bit = 0; 
    
    int_var__invert_zero = 0; 
    
    int_var__no_csum_rw = 0; 
    
    int_var__phdr_next_hdr = 0; 
    
}

void cap_dppcsum_csr_cfg_csum_hdrs_t::clear() {

    int_var__hdr_num = 0; 
    
    int_var__csum_vld = 0; 
    
    int_var__csum_copy_vld = 0; 
    
    int_var__csum_unit = 0; 
    
    int_var__csum_profile = 0; 
    
    int_var__phdr_vld = 0; 
    
    int_var__phdr_unit = 0; 
    
    int_var__phdr_profile = 0; 
    
    int_var__csum_unit_include_bm = 0; 
    
    int_var__crc_include_bm = 0; 
    
    int_var__hdrfld_start = 0; 
    
    int_var__hdrfld_end = 0; 
    
}

void cap_dppcsum_csr_t::clear() {

    #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_csum_hdrs
    for(int ii = 0; ii < 24; ii++) {
        cfg_csum_hdrs[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_csum_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_profile[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_csum_phdr_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_phdr_profile[ii].clear();
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_crc_hdrs
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_hdrs[ii].clear();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_crc_profile
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_profile[ii].clear();
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_crc_mask_profile
    for(int ii = 0; ii < 12; ii++) {
        cfg_crc_mask_profile[ii].clear();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_spare_csum
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_csum[ii].clear();
    }
    #endif
    
}

void cap_dppcsum_csr_cfg_spare_csum_t::init() {

}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::init() {

}

void cap_dppcsum_csr_cfg_crc_profile_t::init() {

}

void cap_dppcsum_csr_cfg_crc_hdrs_t::init() {

}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::init() {

}

void cap_dppcsum_csr_cfg_csum_profile_t::init() {

}

void cap_dppcsum_csr_cfg_csum_hdrs_t::init() {

}

void cap_dppcsum_csr_t::init() {

    #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_hdrs.set_attributes(this, "cfg_csum_hdrs", 0x0);
    #else
    for(int ii = 0; ii < 24; ii++) {
        if(ii != 0) cfg_csum_hdrs[ii].set_field_init_done(true, true);
        cfg_csum_hdrs[ii].set_attributes(this,"cfg_csum_hdrs["+to_string(ii)+"]",  0x0 + (cfg_csum_hdrs[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_profile.set_attributes(this, "cfg_csum_profile", 0x100);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_csum_profile[ii].set_field_init_done(true, true);
        cfg_csum_profile[ii].set_attributes(this,"cfg_csum_profile["+to_string(ii)+"]",  0x100 + (cfg_csum_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_csum_phdr_profile.set_attributes(this, "cfg_csum_phdr_profile", 0x200);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_csum_phdr_profile[ii].set_field_init_done(true, true);
        cfg_csum_phdr_profile[ii].set_attributes(this,"cfg_csum_phdr_profile["+to_string(ii)+"]",  0x200 + (cfg_csum_phdr_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_hdrs.set_attributes(this, "cfg_crc_hdrs", 0x300);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_crc_hdrs[ii].set_field_init_done(true, true);
        cfg_crc_hdrs[ii].set_attributes(this,"cfg_crc_hdrs["+to_string(ii)+"]",  0x300 + (cfg_crc_hdrs[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_profile.set_attributes(this, "cfg_crc_profile", 0x400);
    #else
    for(int ii = 0; ii < 12; ii++) {
        if(ii != 0) cfg_crc_profile[ii].set_field_init_done(true, true);
        cfg_crc_profile[ii].set_attributes(this,"cfg_crc_profile["+to_string(ii)+"]",  0x400 + (cfg_crc_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_crc_mask_profile.set_attributes(this, "cfg_crc_mask_profile", 0x600);
    #else
    for(int ii = 0; ii < 12; ii++) {
        if(ii != 0) cfg_crc_mask_profile[ii].set_field_init_done(true, true);
        cfg_crc_mask_profile[ii].set_attributes(this,"cfg_crc_mask_profile["+to_string(ii)+"]",  0x600 + (cfg_crc_mask_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_csum.set_attributes(this, "cfg_spare_csum", 0x800);
    #else
    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) cfg_spare_csum[ii].set_field_init_done(true, true);
        cfg_spare_csum[ii].set_attributes(this,"cfg_spare_csum["+to_string(ii)+"]",  0x800 + (cfg_spare_csum[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dppcsum_csr_cfg_spare_csum_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_spare_csum_t::data() const {
    return int_var__data;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0(const cpp_int & _val) { 
    // fld_en_0
    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0() const {
    return int_var__fld_en_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_0(const cpp_int & _val) { 
    // fld_start_sub_0
    int_var__fld_start_sub_0 = _val.convert_to< fld_start_sub_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_0() const {
    return int_var__fld_start_sub_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0(const cpp_int & _val) { 
    // fld_start_0
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0() const {
    return int_var__fld_start_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_0(const cpp_int & _val) { 
    // fld_end_sub_0
    int_var__fld_end_sub_0 = _val.convert_to< fld_end_sub_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_0() const {
    return int_var__fld_end_sub_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0(const cpp_int & _val) { 
    // fld_end_0
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0() const {
    return int_var__fld_end_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0(const cpp_int & _val) { 
    // skip_first_nibble_0
    int_var__skip_first_nibble_0 = _val.convert_to< skip_first_nibble_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0() const {
    return int_var__skip_first_nibble_0;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1(const cpp_int & _val) { 
    // fld_en_1
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1() const {
    return int_var__fld_en_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_1(const cpp_int & _val) { 
    // fld_start_sub_1
    int_var__fld_start_sub_1 = _val.convert_to< fld_start_sub_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_1() const {
    return int_var__fld_start_sub_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1(const cpp_int & _val) { 
    // fld_start_1
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1() const {
    return int_var__fld_start_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_1(const cpp_int & _val) { 
    // fld_end_sub_1
    int_var__fld_end_sub_1 = _val.convert_to< fld_end_sub_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_1() const {
    return int_var__fld_end_sub_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1(const cpp_int & _val) { 
    // fld_end_1
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1() const {
    return int_var__fld_end_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1(const cpp_int & _val) { 
    // skip_first_nibble_1
    int_var__skip_first_nibble_1 = _val.convert_to< skip_first_nibble_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1() const {
    return int_var__skip_first_nibble_1;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2(const cpp_int & _val) { 
    // fld_en_2
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2() const {
    return int_var__fld_en_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_2(const cpp_int & _val) { 
    // fld_start_sub_2
    int_var__fld_start_sub_2 = _val.convert_to< fld_start_sub_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_2() const {
    return int_var__fld_start_sub_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2(const cpp_int & _val) { 
    // fld_start_2
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2() const {
    return int_var__fld_start_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_2(const cpp_int & _val) { 
    // fld_end_sub_2
    int_var__fld_end_sub_2 = _val.convert_to< fld_end_sub_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_2() const {
    return int_var__fld_end_sub_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2(const cpp_int & _val) { 
    // fld_end_2
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2() const {
    return int_var__fld_end_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2(const cpp_int & _val) { 
    // skip_first_nibble_2
    int_var__skip_first_nibble_2 = _val.convert_to< skip_first_nibble_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2() const {
    return int_var__skip_first_nibble_2;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3(const cpp_int & _val) { 
    // fld_en_3
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3() const {
    return int_var__fld_en_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_3(const cpp_int & _val) { 
    // fld_start_sub_3
    int_var__fld_start_sub_3 = _val.convert_to< fld_start_sub_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_3() const {
    return int_var__fld_start_sub_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3(const cpp_int & _val) { 
    // fld_start_3
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3() const {
    return int_var__fld_start_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_3(const cpp_int & _val) { 
    // fld_end_sub_3
    int_var__fld_end_sub_3 = _val.convert_to< fld_end_sub_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_3() const {
    return int_var__fld_end_sub_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3(const cpp_int & _val) { 
    // fld_end_3
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3() const {
    return int_var__fld_end_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3(const cpp_int & _val) { 
    // skip_first_nibble_3
    int_var__skip_first_nibble_3 = _val.convert_to< skip_first_nibble_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3() const {
    return int_var__skip_first_nibble_3;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4(const cpp_int & _val) { 
    // fld_en_4
    int_var__fld_en_4 = _val.convert_to< fld_en_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4() const {
    return int_var__fld_en_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_4(const cpp_int & _val) { 
    // fld_start_sub_4
    int_var__fld_start_sub_4 = _val.convert_to< fld_start_sub_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_sub_4() const {
    return int_var__fld_start_sub_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4(const cpp_int & _val) { 
    // fld_start_4
    int_var__fld_start_4 = _val.convert_to< fld_start_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4() const {
    return int_var__fld_start_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_4(const cpp_int & _val) { 
    // fld_end_sub_4
    int_var__fld_end_sub_4 = _val.convert_to< fld_end_sub_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_sub_4() const {
    return int_var__fld_end_sub_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4(const cpp_int & _val) { 
    // fld_end_4
    int_var__fld_end_4 = _val.convert_to< fld_end_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4() const {
    return int_var__fld_end_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4(const cpp_int & _val) { 
    // skip_first_nibble_4
    int_var__skip_first_nibble_4 = _val.convert_to< skip_first_nibble_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4() const {
    return int_var__skip_first_nibble_4;
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fill(const cpp_int & _val) { 
    // fill
    int_var__fill = _val.convert_to< fill_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fill() const {
    return int_var__fill;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len(const cpp_int & _val) { 
    // use_phv_len
    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len() const {
    return int_var__use_phv_len;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel(const cpp_int & _val) { 
    // phv_len_sel
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel() const {
    return int_var__phv_len_sel;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_mask() const {
    return int_var__len_mask;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left() const {
    return int_var__len_shift_left;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val() const {
    return int_var__len_shift_val;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::start_sop(const cpp_int & _val) { 
    // start_sop
    int_var__start_sop = _val.convert_to< start_sop_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::start_sop() const {
    return int_var__start_sop;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::end_eop(const cpp_int & _val) { 
    // end_eop
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::end_eop() const {
    return int_var__end_eop;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::start_adj_sub(const cpp_int & _val) { 
    // start_adj_sub
    int_var__start_adj_sub = _val.convert_to< start_adj_sub_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::start_adj_sub() const {
    return int_var__start_adj_sub;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::end_adj_sub(const cpp_int & _val) { 
    // end_adj_sub
    int_var__end_adj_sub = _val.convert_to< end_adj_sub_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::end_adj_sub() const {
    return int_var__end_adj_sub;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::end_adj() const {
    return int_var__end_adj;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::loc_adj_sub(const cpp_int & _val) { 
    // loc_adj_sub
    int_var__loc_adj_sub = _val.convert_to< loc_adj_sub_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::loc_adj_sub() const {
    return int_var__loc_adj_sub;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::loc_adj(const cpp_int & _val) { 
    // loc_adj
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::loc_adj() const {
    return int_var__loc_adj;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm(const cpp_int & _val) { 
    // crc_unit_include_bm
    int_var__crc_unit_include_bm = _val.convert_to< crc_unit_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm() const {
    return int_var__crc_unit_include_bm;
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::add_fix_mask(const cpp_int & _val) { 
    // add_fix_mask
    int_var__add_fix_mask = _val.convert_to< add_fix_mask_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::add_fix_mask() const {
    return int_var__add_fix_mask;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num(const cpp_int & _val) { 
    // hdr_num
    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num() const {
    return int_var__hdr_num;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld(const cpp_int & _val) { 
    // crc_vld
    int_var__crc_vld = _val.convert_to< crc_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld() const {
    return int_var__crc_vld;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit(const cpp_int & _val) { 
    // crc_unit
    int_var__crc_unit = _val.convert_to< crc_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit() const {
    return int_var__crc_unit;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile(const cpp_int & _val) { 
    // crc_profile
    int_var__crc_profile = _val.convert_to< crc_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile() const {
    return int_var__crc_profile;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld(const cpp_int & _val) { 
    // mask_vld
    int_var__mask_vld = _val.convert_to< mask_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld() const {
    return int_var__mask_vld;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit(const cpp_int & _val) { 
    // mask_unit
    int_var__mask_unit = _val.convert_to< mask_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit() const {
    return int_var__mask_unit;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile(const cpp_int & _val) { 
    // mask_profile
    int_var__mask_profile = _val.convert_to< mask_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile() const {
    return int_var__mask_profile;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start(const cpp_int & _val) { 
    // hdrfld_start
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start() const {
    return int_var__hdrfld_start;
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end(const cpp_int & _val) { 
    // hdrfld_end
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end() const {
    return int_var__hdrfld_end;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0(const cpp_int & _val) { 
    // fld_en_0
    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0() const {
    return int_var__fld_en_0;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0(const cpp_int & _val) { 
    // fld_start_0
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0() const {
    return int_var__fld_start_0;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0(const cpp_int & _val) { 
    // fld_end_0
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0() const {
    return int_var__fld_end_0;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0(const cpp_int & _val) { 
    // fld_align_0
    int_var__fld_align_0 = _val.convert_to< fld_align_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0() const {
    return int_var__fld_align_0;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1(const cpp_int & _val) { 
    // fld_en_1
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1() const {
    return int_var__fld_en_1;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1(const cpp_int & _val) { 
    // fld_start_1
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1() const {
    return int_var__fld_start_1;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1(const cpp_int & _val) { 
    // fld_end_1
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1() const {
    return int_var__fld_end_1;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1(const cpp_int & _val) { 
    // fld_align_1
    int_var__fld_align_1 = _val.convert_to< fld_align_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1() const {
    return int_var__fld_align_1;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2(const cpp_int & _val) { 
    // fld_en_2
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2() const {
    return int_var__fld_en_2;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2(const cpp_int & _val) { 
    // fld_start_2
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2() const {
    return int_var__fld_start_2;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2(const cpp_int & _val) { 
    // fld_end_2
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2() const {
    return int_var__fld_end_2;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2(const cpp_int & _val) { 
    // fld_align_2
    int_var__fld_align_2 = _val.convert_to< fld_align_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2() const {
    return int_var__fld_align_2;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3(const cpp_int & _val) { 
    // fld_en_3
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3() const {
    return int_var__fld_en_3;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3(const cpp_int & _val) { 
    // fld_start_3
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3() const {
    return int_var__fld_start_3;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3(const cpp_int & _val) { 
    // fld_end_3
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3() const {
    return int_var__fld_end_3;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3(const cpp_int & _val) { 
    // fld_align_3
    int_var__fld_align_3 = _val.convert_to< fld_align_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3() const {
    return int_var__fld_align_3;
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len() const {
    return int_var__add_len;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len(const cpp_int & _val) { 
    // use_phv_len
    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len() const {
    return int_var__use_phv_len;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel(const cpp_int & _val) { 
    // phv_len_sel
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel() const {
    return int_var__phv_len_sel;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_mask() const {
    return int_var__len_mask;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left() const {
    return int_var__len_shift_left;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val() const {
    return int_var__len_shift_val;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::start_adj() const {
    return int_var__start_adj;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::end_adj() const {
    return int_var__end_adj;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::loc_adj(const cpp_int & _val) { 
    // loc_adj
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::loc_adj() const {
    return int_var__loc_adj;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::add_len() const {
    return int_var__add_len;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::eight_bit(const cpp_int & _val) { 
    // eight_bit
    int_var__eight_bit = _val.convert_to< eight_bit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::eight_bit() const {
    return int_var__eight_bit;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::invert_zero(const cpp_int & _val) { 
    // invert_zero
    int_var__invert_zero = _val.convert_to< invert_zero_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::invert_zero() const {
    return int_var__invert_zero;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::no_csum_rw(const cpp_int & _val) { 
    // no_csum_rw
    int_var__no_csum_rw = _val.convert_to< no_csum_rw_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::no_csum_rw() const {
    return int_var__no_csum_rw;
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::phdr_next_hdr(const cpp_int & _val) { 
    // phdr_next_hdr
    int_var__phdr_next_hdr = _val.convert_to< phdr_next_hdr_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::phdr_next_hdr() const {
    return int_var__phdr_next_hdr;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num(const cpp_int & _val) { 
    // hdr_num
    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num() const {
    return int_var__hdr_num;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld(const cpp_int & _val) { 
    // csum_vld
    int_var__csum_vld = _val.convert_to< csum_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld() const {
    return int_var__csum_vld;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_copy_vld(const cpp_int & _val) { 
    // csum_copy_vld
    int_var__csum_copy_vld = _val.convert_to< csum_copy_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_copy_vld() const {
    return int_var__csum_copy_vld;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit(const cpp_int & _val) { 
    // csum_unit
    int_var__csum_unit = _val.convert_to< csum_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit() const {
    return int_var__csum_unit;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile(const cpp_int & _val) { 
    // csum_profile
    int_var__csum_profile = _val.convert_to< csum_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile() const {
    return int_var__csum_profile;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld(const cpp_int & _val) { 
    // phdr_vld
    int_var__phdr_vld = _val.convert_to< phdr_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld() const {
    return int_var__phdr_vld;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit(const cpp_int & _val) { 
    // phdr_unit
    int_var__phdr_unit = _val.convert_to< phdr_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit() const {
    return int_var__phdr_unit;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile(const cpp_int & _val) { 
    // phdr_profile
    int_var__phdr_profile = _val.convert_to< phdr_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile() const {
    return int_var__phdr_profile;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm(const cpp_int & _val) { 
    // csum_unit_include_bm
    int_var__csum_unit_include_bm = _val.convert_to< csum_unit_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm() const {
    return int_var__csum_unit_include_bm;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm(const cpp_int & _val) { 
    // crc_include_bm
    int_var__crc_include_bm = _val.convert_to< crc_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm() const {
    return int_var__crc_include_bm;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start(const cpp_int & _val) { 
    // hdrfld_start
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start() const {
    return int_var__hdrfld_start;
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end(const cpp_int & _val) { 
    // hdrfld_end
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end() const {
    return int_var__hdrfld_end;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_spare_csum_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_mask_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en_0")) { field_val = fld_en_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_0")) { field_val = fld_start_sub_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_0")) { field_val = fld_start_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_0")) { field_val = fld_end_sub_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_0")) { field_val = fld_end_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_0")) { field_val = skip_first_nibble_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_1")) { field_val = fld_en_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_1")) { field_val = fld_start_sub_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_1")) { field_val = fld_start_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_1")) { field_val = fld_end_sub_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_1")) { field_val = fld_end_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_1")) { field_val = skip_first_nibble_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_2")) { field_val = fld_en_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_2")) { field_val = fld_start_sub_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_2")) { field_val = fld_start_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_2")) { field_val = fld_end_sub_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_2")) { field_val = fld_end_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_2")) { field_val = skip_first_nibble_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_3")) { field_val = fld_en_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_3")) { field_val = fld_start_sub_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_3")) { field_val = fld_start_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_3")) { field_val = fld_end_sub_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_3")) { field_val = fld_end_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_3")) { field_val = skip_first_nibble_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_4")) { field_val = fld_en_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_4")) { field_val = fld_start_sub_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_4")) { field_val = fld_start_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_4")) { field_val = fld_end_sub_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_4")) { field_val = fld_end_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_4")) { field_val = skip_first_nibble_4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fill")) { field_val = fill(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "use_phv_len")) { field_val = use_phv_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_len_sel")) { field_val = phv_len_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { field_val = len_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { field_val = len_shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { field_val = len_shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_sop")) { field_val = start_sop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { field_val = end_eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj_sub")) { field_val = start_adj_sub(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj_sub")) { field_val = end_adj_sub(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { field_val = end_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj_sub")) { field_val = loc_adj_sub(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj")) { field_val = loc_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_unit_include_bm")) { field_val = crc_unit_include_bm(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_fix_mask")) { field_val = add_fix_mask(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_hdrs_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_num")) { field_val = hdr_num(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_vld")) { field_val = crc_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_unit")) { field_val = crc_unit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_profile")) { field_val = crc_profile(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_vld")) { field_val = mask_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_unit")) { field_val = mask_unit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_profile")) { field_val = mask_profile(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_start")) { field_val = hdrfld_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_end")) { field_val = hdrfld_end(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_phdr_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en_0")) { field_val = fld_en_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_0")) { field_val = fld_start_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_0")) { field_val = fld_end_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_0")) { field_val = fld_align_0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_1")) { field_val = fld_en_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_1")) { field_val = fld_start_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_1")) { field_val = fld_end_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_1")) { field_val = fld_align_1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_2")) { field_val = fld_en_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_2")) { field_val = fld_start_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_2")) { field_val = fld_end_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_2")) { field_val = fld_align_2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_3")) { field_val = fld_en_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_3")) { field_val = fld_start_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_3")) { field_val = fld_end_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_3")) { field_val = fld_align_3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { field_val = add_len(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "use_phv_len")) { field_val = use_phv_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_len_sel")) { field_val = phv_len_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { field_val = len_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { field_val = len_shift_left(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { field_val = len_shift_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { field_val = start_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { field_val = end_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj")) { field_val = loc_adj(); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { field_val = add_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "eight_bit")) { field_val = eight_bit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "invert_zero")) { field_val = invert_zero(); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_csum_rw")) { field_val = no_csum_rw(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_next_hdr")) { field_val = phdr_next_hdr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_hdrs_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_num")) { field_val = hdr_num(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_vld")) { field_val = csum_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_copy_vld")) { field_val = csum_copy_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_unit")) { field_val = csum_unit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_profile")) { field_val = csum_profile(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_vld")) { field_val = phdr_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_unit")) { field_val = phdr_unit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_profile")) { field_val = phdr_profile(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_unit_include_bm")) { field_val = csum_unit_include_bm(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_include_bm")) { field_val = crc_include_bm(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_start")) { field_val = hdrfld_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_end")) { field_val = hdrfld_end(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_spare_csum_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_mask_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en_0")) { fld_en_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_0")) { fld_start_sub_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_0")) { fld_start_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_0")) { fld_end_sub_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_0")) { fld_end_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_0")) { skip_first_nibble_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_1")) { fld_en_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_1")) { fld_start_sub_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_1")) { fld_start_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_1")) { fld_end_sub_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_1")) { fld_end_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_1")) { skip_first_nibble_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_2")) { fld_en_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_2")) { fld_start_sub_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_2")) { fld_start_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_2")) { fld_end_sub_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_2")) { fld_end_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_2")) { skip_first_nibble_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_3")) { fld_en_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_3")) { fld_start_sub_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_3")) { fld_start_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_3")) { fld_end_sub_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_3")) { fld_end_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_3")) { skip_first_nibble_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_4")) { fld_en_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_sub_4")) { fld_start_sub_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_4")) { fld_start_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_sub_4")) { fld_end_sub_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_4")) { fld_end_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_first_nibble_4")) { skip_first_nibble_4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fill")) { fill(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "use_phv_len")) { use_phv_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_len_sel")) { phv_len_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { len_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { len_shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { len_shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_sop")) { start_sop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_eop")) { end_eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj_sub")) { start_adj_sub(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj_sub")) { end_adj_sub(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { end_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj_sub")) { loc_adj_sub(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj")) { loc_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_unit_include_bm")) { crc_unit_include_bm(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_fix_mask")) { add_fix_mask(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_crc_hdrs_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_num")) { hdr_num(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_vld")) { crc_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_unit")) { crc_unit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_profile")) { crc_profile(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_vld")) { mask_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_unit")) { mask_unit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_profile")) { mask_profile(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_start")) { hdrfld_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_end")) { hdrfld_end(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_phdr_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fld_en_0")) { fld_en_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_0")) { fld_start_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_0")) { fld_end_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_0")) { fld_align_0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_1")) { fld_en_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_1")) { fld_start_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_1")) { fld_end_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_1")) { fld_align_1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_2")) { fld_en_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_2")) { fld_start_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_2")) { fld_end_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_2")) { fld_align_2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_en_3")) { fld_en_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_start_3")) { fld_start_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_end_3")) { fld_end_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_align_3")) { fld_align_3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { add_len(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "use_phv_len")) { use_phv_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_len_sel")) { phv_len_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_mask")) { len_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_left")) { len_shift_left(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_shift_val")) { len_shift_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_adj")) { start_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_adj")) { end_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "loc_adj")) { loc_adj(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "add_len")) { add_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "eight_bit")) { eight_bit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "invert_zero")) { invert_zero(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_csum_rw")) { no_csum_rw(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_next_hdr")) { phdr_next_hdr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_cfg_csum_hdrs_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_num")) { hdr_num(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_vld")) { csum_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_copy_vld")) { csum_copy_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_unit")) { csum_unit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_profile")) { csum_profile(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_vld")) { phdr_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_unit")) { phdr_unit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phdr_profile")) { phdr_profile(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_unit_include_bm")) { csum_unit_include_bm(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_include_bm")) { crc_include_bm(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_start")) { hdrfld_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hdrfld_end")) { hdrfld_end(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dppcsum_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_spare_csum_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_crc_mask_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fld_en_0");
    ret_vec.push_back("fld_start_sub_0");
    ret_vec.push_back("fld_start_0");
    ret_vec.push_back("fld_end_sub_0");
    ret_vec.push_back("fld_end_0");
    ret_vec.push_back("skip_first_nibble_0");
    ret_vec.push_back("fld_en_1");
    ret_vec.push_back("fld_start_sub_1");
    ret_vec.push_back("fld_start_1");
    ret_vec.push_back("fld_end_sub_1");
    ret_vec.push_back("fld_end_1");
    ret_vec.push_back("skip_first_nibble_1");
    ret_vec.push_back("fld_en_2");
    ret_vec.push_back("fld_start_sub_2");
    ret_vec.push_back("fld_start_2");
    ret_vec.push_back("fld_end_sub_2");
    ret_vec.push_back("fld_end_2");
    ret_vec.push_back("skip_first_nibble_2");
    ret_vec.push_back("fld_en_3");
    ret_vec.push_back("fld_start_sub_3");
    ret_vec.push_back("fld_start_3");
    ret_vec.push_back("fld_end_sub_3");
    ret_vec.push_back("fld_end_3");
    ret_vec.push_back("skip_first_nibble_3");
    ret_vec.push_back("fld_en_4");
    ret_vec.push_back("fld_start_sub_4");
    ret_vec.push_back("fld_start_4");
    ret_vec.push_back("fld_end_sub_4");
    ret_vec.push_back("fld_end_4");
    ret_vec.push_back("skip_first_nibble_4");
    ret_vec.push_back("fill");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_crc_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("use_phv_len");
    ret_vec.push_back("phv_len_sel");
    ret_vec.push_back("len_mask");
    ret_vec.push_back("len_shift_left");
    ret_vec.push_back("len_shift_val");
    ret_vec.push_back("start_sop");
    ret_vec.push_back("end_eop");
    ret_vec.push_back("start_adj_sub");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("end_adj_sub");
    ret_vec.push_back("end_adj");
    ret_vec.push_back("loc_adj_sub");
    ret_vec.push_back("loc_adj");
    ret_vec.push_back("crc_unit_include_bm");
    ret_vec.push_back("add_fix_mask");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_crc_hdrs_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hdr_num");
    ret_vec.push_back("crc_vld");
    ret_vec.push_back("crc_unit");
    ret_vec.push_back("crc_profile");
    ret_vec.push_back("mask_vld");
    ret_vec.push_back("mask_unit");
    ret_vec.push_back("mask_profile");
    ret_vec.push_back("hdrfld_start");
    ret_vec.push_back("hdrfld_end");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_csum_phdr_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fld_en_0");
    ret_vec.push_back("fld_start_0");
    ret_vec.push_back("fld_end_0");
    ret_vec.push_back("fld_align_0");
    ret_vec.push_back("fld_en_1");
    ret_vec.push_back("fld_start_1");
    ret_vec.push_back("fld_end_1");
    ret_vec.push_back("fld_align_1");
    ret_vec.push_back("fld_en_2");
    ret_vec.push_back("fld_start_2");
    ret_vec.push_back("fld_end_2");
    ret_vec.push_back("fld_align_2");
    ret_vec.push_back("fld_en_3");
    ret_vec.push_back("fld_start_3");
    ret_vec.push_back("fld_end_3");
    ret_vec.push_back("fld_align_3");
    ret_vec.push_back("add_len");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_csum_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("use_phv_len");
    ret_vec.push_back("phv_len_sel");
    ret_vec.push_back("len_mask");
    ret_vec.push_back("len_shift_left");
    ret_vec.push_back("len_shift_val");
    ret_vec.push_back("start_adj");
    ret_vec.push_back("end_adj");
    ret_vec.push_back("loc_adj");
    ret_vec.push_back("add_len");
    ret_vec.push_back("eight_bit");
    ret_vec.push_back("invert_zero");
    ret_vec.push_back("no_csum_rw");
    ret_vec.push_back("phdr_next_hdr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_cfg_csum_hdrs_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hdr_num");
    ret_vec.push_back("csum_vld");
    ret_vec.push_back("csum_copy_vld");
    ret_vec.push_back("csum_unit");
    ret_vec.push_back("csum_profile");
    ret_vec.push_back("phdr_vld");
    ret_vec.push_back("phdr_unit");
    ret_vec.push_back("phdr_profile");
    ret_vec.push_back("csum_unit_include_bm");
    ret_vec.push_back("crc_include_bm");
    ret_vec.push_back("hdrfld_start");
    ret_vec.push_back("hdrfld_end");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dppcsum_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
