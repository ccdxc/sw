
#include "cap_dppcsum_csr.h"
#include "LogMsg.h"        
using namespace std;
        
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
        set_byte_size(2048);
        set_attributes(0,get_name(), 0);
        }
cap_dppcsum_csr_t::~cap_dppcsum_csr_t() { }

void cap_dppcsum_csr_cfg_crc_mask_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_0: 0x" << int_var__fld_en_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_0: 0x" << int_var__fld_start_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_0: 0x" << int_var__fld_end_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_0: 0x" << int_var__skip_first_nibble_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_1: 0x" << int_var__fld_en_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_1: 0x" << int_var__fld_start_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_1: 0x" << int_var__fld_end_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_1: 0x" << int_var__skip_first_nibble_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_2: 0x" << int_var__fld_en_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_2: 0x" << int_var__fld_start_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_2: 0x" << int_var__fld_end_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_2: 0x" << int_var__skip_first_nibble_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_3: 0x" << int_var__fld_en_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_3: 0x" << int_var__fld_start_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_3: 0x" << int_var__fld_end_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_3: 0x" << int_var__skip_first_nibble_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_4: 0x" << int_var__fld_en_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_4: 0x" << int_var__fld_start_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_4: 0x" << int_var__fld_end_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_first_nibble_4: 0x" << int_var__skip_first_nibble_4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fill: 0x" << int_var__fill << dec << endl)
}

void cap_dppcsum_csr_cfg_crc_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".use_phv_len: 0x" << int_var__use_phv_len << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_len_sel: 0x" << int_var__phv_len_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start_sop: 0x" << int_var__start_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_eop: 0x" << int_var__end_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".loc_adj: 0x" << int_var__loc_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_unit_include_bm: 0x" << int_var__crc_unit_include_bm << dec << endl)
}

void cap_dppcsum_csr_cfg_crc_hdrs_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_num: 0x" << int_var__hdr_num << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_vld: 0x" << int_var__crc_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_unit: 0x" << int_var__crc_unit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_profile: 0x" << int_var__crc_profile << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_vld: 0x" << int_var__mask_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_unit: 0x" << int_var__mask_unit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_profile: 0x" << int_var__mask_profile << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_start: 0x" << int_var__hdrfld_start << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_end: 0x" << int_var__hdrfld_end << dec << endl)
}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_0: 0x" << int_var__fld_en_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_0: 0x" << int_var__fld_start_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_0: 0x" << int_var__fld_end_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_0: 0x" << int_var__fld_align_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_1: 0x" << int_var__fld_en_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_1: 0x" << int_var__fld_start_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_1: 0x" << int_var__fld_end_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_1: 0x" << int_var__fld_align_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_2: 0x" << int_var__fld_en_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_2: 0x" << int_var__fld_start_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_2: 0x" << int_var__fld_end_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_2: 0x" << int_var__fld_align_2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_en_3: 0x" << int_var__fld_en_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start_3: 0x" << int_var__fld_start_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end_3: 0x" << int_var__fld_end_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_align_3: 0x" << int_var__fld_align_3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl)
}

void cap_dppcsum_csr_cfg_csum_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".use_phv_len: 0x" << int_var__use_phv_len << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_len_sel: 0x" << int_var__phv_len_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_mask: 0x" << int_var__len_mask << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_left: 0x" << int_var__len_shift_left << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".len_shift_val: 0x" << int_var__len_shift_val << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start_adj: 0x" << int_var__start_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_adj: 0x" << int_var__end_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".loc_adj: 0x" << int_var__loc_adj << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".add_len: 0x" << int_var__add_len << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_unit_include_bm: 0x" << int_var__csum_unit_include_bm << dec << endl)
}

void cap_dppcsum_csr_cfg_csum_hdrs_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_num: 0x" << int_var__hdr_num << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_vld: 0x" << int_var__csum_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_unit: 0x" << int_var__csum_unit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_profile: 0x" << int_var__csum_profile << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_vld: 0x" << int_var__phdr_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_unit: 0x" << int_var__phdr_unit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phdr_profile: 0x" << int_var__phdr_profile << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_unit_include_bm: 0x" << int_var__csum_unit_include_bm << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_include_bm: 0x" << int_var__crc_include_bm << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_start: 0x" << int_var__hdrfld_start << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hdrfld_end: 0x" << int_var__hdrfld_end << dec << endl)
}

void cap_dppcsum_csr_t::show() {

    for(int ii = 0; ii < 32; ii++) {
        cfg_csum_hdrs[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_profile[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_phdr_profile[ii].show();
    }
    for(int ii = 0; ii < 32; ii++) {
        cfg_crc_hdrs[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_profile[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_mask_profile[ii].show();
    }
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

int cap_dppcsum_csr_cfg_crc_mask_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // fld_en_0
    _count += 16; // fld_start_0
    _count += 16; // fld_end_0
    _count += 1; // skip_first_nibble_0
    _count += 1; // fld_en_1
    _count += 16; // fld_start_1
    _count += 16; // fld_end_1
    _count += 1; // skip_first_nibble_1
    _count += 1; // fld_en_2
    _count += 16; // fld_start_2
    _count += 16; // fld_end_2
    _count += 1; // skip_first_nibble_2
    _count += 1; // fld_en_3
    _count += 16; // fld_start_3
    _count += 16; // fld_end_3
    _count += 1; // skip_first_nibble_3
    _count += 1; // fld_en_4
    _count += 16; // fld_start_4
    _count += 16; // fld_end_4
    _count += 1; // skip_first_nibble_4
    _count += 1; // fill
    return _count;
}

int cap_dppcsum_csr_cfg_crc_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // use_phv_len
    _count += 5; // phv_len_sel
    _count += 16; // len_mask
    _count += 1; // len_shift_left
    _count += 4; // len_shift_val
    _count += 1; // start_sop
    _count += 1; // end_eop
    _count += 16; // start_adj
    _count += 16; // end_adj
    _count += 16; // loc_adj
    _count += 1; // crc_unit_include_bm
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
    _count += 16; // fld_start_0
    _count += 16; // fld_end_0
    _count += 1; // fld_align_0
    _count += 1; // fld_en_1
    _count += 16; // fld_start_1
    _count += 16; // fld_end_1
    _count += 1; // fld_align_1
    _count += 1; // fld_en_2
    _count += 16; // fld_start_2
    _count += 16; // fld_end_2
    _count += 1; // fld_align_2
    _count += 1; // fld_en_3
    _count += 16; // fld_start_3
    _count += 16; // fld_end_3
    _count += 1; // fld_align_3
    _count += 1; // add_len
    return _count;
}

int cap_dppcsum_csr_cfg_csum_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // use_phv_len
    _count += 5; // phv_len_sel
    _count += 16; // len_mask
    _count += 1; // len_shift_left
    _count += 4; // len_shift_val
    _count += 16; // start_adj
    _count += 16; // end_adj
    _count += 16; // loc_adj
    _count += 1; // add_len
    _count += 3; // csum_unit_include_bm
    return _count;
}

int cap_dppcsum_csr_cfg_csum_hdrs_t::s_get_width() {
    int _count = 0;

    _count += 7; // hdr_num
    _count += 1; // csum_vld
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

    _count += (cap_dppcsum_csr_cfg_csum_hdrs_t::s_get_width() * 32); // cfg_csum_hdrs
    _count += (cap_dppcsum_csr_cfg_csum_profile_t::s_get_width() * 16); // cfg_csum_profile
    _count += (cap_dppcsum_csr_cfg_csum_phdr_profile_t::s_get_width() * 16); // cfg_csum_phdr_profile
    _count += (cap_dppcsum_csr_cfg_crc_hdrs_t::s_get_width() * 32); // cfg_crc_hdrs
    _count += (cap_dppcsum_csr_cfg_crc_profile_t::s_get_width() * 16); // cfg_crc_profile
    _count += (cap_dppcsum_csr_cfg_crc_mask_profile_t::s_get_width() * 16); // cfg_crc_mask_profile
    return _count;
}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // fld_en_0
    int_var__fld_en_0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_0_cpp_int_t >()  ;
    _count += 1;
    // fld_start_0
    int_var__fld_start_0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_0_cpp_int_t >()  ;
    _count += 16;
    // fld_end_0
    int_var__fld_end_0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_0_cpp_int_t >()  ;
    _count += 16;
    // skip_first_nibble_0
    int_var__skip_first_nibble_0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< skip_first_nibble_0_cpp_int_t >()  ;
    _count += 1;
    // fld_en_1
    int_var__fld_en_1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_1_cpp_int_t >()  ;
    _count += 1;
    // fld_start_1
    int_var__fld_start_1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_1_cpp_int_t >()  ;
    _count += 16;
    // fld_end_1
    int_var__fld_end_1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_1_cpp_int_t >()  ;
    _count += 16;
    // skip_first_nibble_1
    int_var__skip_first_nibble_1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< skip_first_nibble_1_cpp_int_t >()  ;
    _count += 1;
    // fld_en_2
    int_var__fld_en_2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_2_cpp_int_t >()  ;
    _count += 1;
    // fld_start_2
    int_var__fld_start_2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_2_cpp_int_t >()  ;
    _count += 16;
    // fld_end_2
    int_var__fld_end_2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_2_cpp_int_t >()  ;
    _count += 16;
    // skip_first_nibble_2
    int_var__skip_first_nibble_2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< skip_first_nibble_2_cpp_int_t >()  ;
    _count += 1;
    // fld_en_3
    int_var__fld_en_3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_3_cpp_int_t >()  ;
    _count += 1;
    // fld_start_3
    int_var__fld_start_3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_3_cpp_int_t >()  ;
    _count += 16;
    // fld_end_3
    int_var__fld_end_3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_3_cpp_int_t >()  ;
    _count += 16;
    // skip_first_nibble_3
    int_var__skip_first_nibble_3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< skip_first_nibble_3_cpp_int_t >()  ;
    _count += 1;
    // fld_en_4
    int_var__fld_en_4 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_4_cpp_int_t >()  ;
    _count += 1;
    // fld_start_4
    int_var__fld_start_4 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_4_cpp_int_t >()  ;
    _count += 16;
    // fld_end_4
    int_var__fld_end_4 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_4_cpp_int_t >()  ;
    _count += 16;
    // skip_first_nibble_4
    int_var__skip_first_nibble_4 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< skip_first_nibble_4_cpp_int_t >()  ;
    _count += 1;
    // fill
    int_var__fill = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fill_cpp_int_t >()  ;
    _count += 1;
}

void cap_dppcsum_csr_cfg_crc_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // use_phv_len
    int_var__use_phv_len = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< use_phv_len_cpp_int_t >()  ;
    _count += 1;
    // phv_len_sel
    int_var__phv_len_sel = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< phv_len_sel_cpp_int_t >()  ;
    _count += 5;
    // len_mask
    int_var__len_mask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< len_mask_cpp_int_t >()  ;
    _count += 16;
    // len_shift_left
    int_var__len_shift_left = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< len_shift_left_cpp_int_t >()  ;
    _count += 1;
    // len_shift_val
    int_var__len_shift_val = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< len_shift_val_cpp_int_t >()  ;
    _count += 4;
    // start_sop
    int_var__start_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< start_sop_cpp_int_t >()  ;
    _count += 1;
    // end_eop
    int_var__end_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< end_eop_cpp_int_t >()  ;
    _count += 1;
    // start_adj
    int_var__start_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< start_adj_cpp_int_t >()  ;
    _count += 16;
    // end_adj
    int_var__end_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< end_adj_cpp_int_t >()  ;
    _count += 16;
    // loc_adj
    int_var__loc_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< loc_adj_cpp_int_t >()  ;
    _count += 16;
    // crc_unit_include_bm
    int_var__crc_unit_include_bm = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< crc_unit_include_bm_cpp_int_t >()  ;
    _count += 1;
}

void cap_dppcsum_csr_cfg_crc_hdrs_t::all(const cpp_int & _val) {
    int _count = 0;

    // hdr_num
    int_var__hdr_num = hlp.get_slc(_val, _count, _count -1 + 7 ).convert_to< hdr_num_cpp_int_t >()  ;
    _count += 7;
    // crc_vld
    int_var__crc_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< crc_vld_cpp_int_t >()  ;
    _count += 1;
    // crc_unit
    int_var__crc_unit = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< crc_unit_cpp_int_t >()  ;
    _count += 1;
    // crc_profile
    int_var__crc_profile = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< crc_profile_cpp_int_t >()  ;
    _count += 4;
    // mask_vld
    int_var__mask_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mask_vld_cpp_int_t >()  ;
    _count += 1;
    // mask_unit
    int_var__mask_unit = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mask_unit_cpp_int_t >()  ;
    _count += 1;
    // mask_profile
    int_var__mask_profile = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< mask_profile_cpp_int_t >()  ;
    _count += 4;
    // hdrfld_start
    int_var__hdrfld_start = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< hdrfld_start_cpp_int_t >()  ;
    _count += 8;
    // hdrfld_end
    int_var__hdrfld_end = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< hdrfld_end_cpp_int_t >()  ;
    _count += 8;
}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // fld_en_0
    int_var__fld_en_0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_0_cpp_int_t >()  ;
    _count += 1;
    // fld_start_0
    int_var__fld_start_0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_0_cpp_int_t >()  ;
    _count += 16;
    // fld_end_0
    int_var__fld_end_0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_0_cpp_int_t >()  ;
    _count += 16;
    // fld_align_0
    int_var__fld_align_0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_align_0_cpp_int_t >()  ;
    _count += 1;
    // fld_en_1
    int_var__fld_en_1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_1_cpp_int_t >()  ;
    _count += 1;
    // fld_start_1
    int_var__fld_start_1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_1_cpp_int_t >()  ;
    _count += 16;
    // fld_end_1
    int_var__fld_end_1 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_1_cpp_int_t >()  ;
    _count += 16;
    // fld_align_1
    int_var__fld_align_1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_align_1_cpp_int_t >()  ;
    _count += 1;
    // fld_en_2
    int_var__fld_en_2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_2_cpp_int_t >()  ;
    _count += 1;
    // fld_start_2
    int_var__fld_start_2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_2_cpp_int_t >()  ;
    _count += 16;
    // fld_end_2
    int_var__fld_end_2 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_2_cpp_int_t >()  ;
    _count += 16;
    // fld_align_2
    int_var__fld_align_2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_align_2_cpp_int_t >()  ;
    _count += 1;
    // fld_en_3
    int_var__fld_en_3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_en_3_cpp_int_t >()  ;
    _count += 1;
    // fld_start_3
    int_var__fld_start_3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_start_3_cpp_int_t >()  ;
    _count += 16;
    // fld_end_3
    int_var__fld_end_3 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< fld_end_3_cpp_int_t >()  ;
    _count += 16;
    // fld_align_3
    int_var__fld_align_3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< fld_align_3_cpp_int_t >()  ;
    _count += 1;
    // add_len
    int_var__add_len = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< add_len_cpp_int_t >()  ;
    _count += 1;
}

void cap_dppcsum_csr_cfg_csum_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // use_phv_len
    int_var__use_phv_len = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< use_phv_len_cpp_int_t >()  ;
    _count += 1;
    // phv_len_sel
    int_var__phv_len_sel = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< phv_len_sel_cpp_int_t >()  ;
    _count += 5;
    // len_mask
    int_var__len_mask = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< len_mask_cpp_int_t >()  ;
    _count += 16;
    // len_shift_left
    int_var__len_shift_left = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< len_shift_left_cpp_int_t >()  ;
    _count += 1;
    // len_shift_val
    int_var__len_shift_val = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< len_shift_val_cpp_int_t >()  ;
    _count += 4;
    // start_adj
    int_var__start_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< start_adj_cpp_int_t >()  ;
    _count += 16;
    // end_adj
    int_var__end_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< end_adj_cpp_int_t >()  ;
    _count += 16;
    // loc_adj
    int_var__loc_adj = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< loc_adj_cpp_int_t >()  ;
    _count += 16;
    // add_len
    int_var__add_len = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< add_len_cpp_int_t >()  ;
    _count += 1;
    // csum_unit_include_bm
    int_var__csum_unit_include_bm = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< csum_unit_include_bm_cpp_int_t >()  ;
    _count += 3;
}

void cap_dppcsum_csr_cfg_csum_hdrs_t::all(const cpp_int & _val) {
    int _count = 0;

    // hdr_num
    int_var__hdr_num = hlp.get_slc(_val, _count, _count -1 + 7 ).convert_to< hdr_num_cpp_int_t >()  ;
    _count += 7;
    // csum_vld
    int_var__csum_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< csum_vld_cpp_int_t >()  ;
    _count += 1;
    // csum_unit
    int_var__csum_unit = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< csum_unit_cpp_int_t >()  ;
    _count += 3;
    // csum_profile
    int_var__csum_profile = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< csum_profile_cpp_int_t >()  ;
    _count += 4;
    // phdr_vld
    int_var__phdr_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< phdr_vld_cpp_int_t >()  ;
    _count += 1;
    // phdr_unit
    int_var__phdr_unit = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< phdr_unit_cpp_int_t >()  ;
    _count += 3;
    // phdr_profile
    int_var__phdr_profile = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< phdr_profile_cpp_int_t >()  ;
    _count += 4;
    // csum_unit_include_bm
    int_var__csum_unit_include_bm = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< csum_unit_include_bm_cpp_int_t >()  ;
    _count += 5;
    // crc_include_bm
    int_var__crc_include_bm = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< crc_include_bm_cpp_int_t >()  ;
    _count += 1;
    // hdrfld_start
    int_var__hdrfld_start = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< hdrfld_start_cpp_int_t >()  ;
    _count += 8;
    // hdrfld_end
    int_var__hdrfld_end = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< hdrfld_end_cpp_int_t >()  ;
    _count += 8;
}

void cap_dppcsum_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_csum_hdrs
    for(int ii = 0; ii < 32; ii++) {
        cfg_csum_hdrs[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_csum_hdrs[ii].get_width()));
        _count += cfg_csum_hdrs[ii].get_width();
    }
    // cfg_csum_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_csum_profile[ii].get_width()));
        _count += cfg_csum_profile[ii].get_width();
    }
    // cfg_csum_phdr_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_csum_phdr_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_csum_phdr_profile[ii].get_width()));
        _count += cfg_csum_phdr_profile[ii].get_width();
    }
    // cfg_crc_hdrs
    for(int ii = 0; ii < 32; ii++) {
        cfg_crc_hdrs[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_crc_hdrs[ii].get_width()));
        _count += cfg_crc_hdrs[ii].get_width();
    }
    // cfg_crc_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_crc_profile[ii].get_width()));
        _count += cfg_crc_profile[ii].get_width();
    }
    // cfg_crc_mask_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_crc_mask_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_crc_mask_profile[ii].get_width()));
        _count += cfg_crc_mask_profile[ii].get_width();
    }
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // fld_en_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_0) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_0) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_0) , _count, _count -1 + 16 );
    _count += 16;
    // skip_first_nibble_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__skip_first_nibble_0) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_1) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_1) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_1) , _count, _count -1 + 16 );
    _count += 16;
    // skip_first_nibble_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__skip_first_nibble_1) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_2) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_2) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_2) , _count, _count -1 + 16 );
    _count += 16;
    // skip_first_nibble_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__skip_first_nibble_2) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_3) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_3) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_3) , _count, _count -1 + 16 );
    _count += 16;
    // skip_first_nibble_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__skip_first_nibble_3) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_4) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_4) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_4) , _count, _count -1 + 16 );
    _count += 16;
    // skip_first_nibble_4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__skip_first_nibble_4) , _count, _count -1 + 1 );
    _count += 1;
    // fill
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fill) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // use_phv_len
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__use_phv_len) , _count, _count -1 + 1 );
    _count += 1;
    // phv_len_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_len_sel) , _count, _count -1 + 5 );
    _count += 5;
    // len_mask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_mask) , _count, _count -1 + 16 );
    _count += 16;
    // len_shift_left
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_shift_left) , _count, _count -1 + 1 );
    _count += 1;
    // len_shift_val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_shift_val) , _count, _count -1 + 4 );
    _count += 4;
    // start_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_sop) , _count, _count -1 + 1 );
    _count += 1;
    // end_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_eop) , _count, _count -1 + 1 );
    _count += 1;
    // start_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_adj) , _count, _count -1 + 16 );
    _count += 16;
    // end_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_adj) , _count, _count -1 + 16 );
    _count += 16;
    // loc_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__loc_adj) , _count, _count -1 + 16 );
    _count += 16;
    // crc_unit_include_bm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__crc_unit_include_bm) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // hdr_num
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdr_num) , _count, _count -1 + 7 );
    _count += 7;
    // crc_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__crc_vld) , _count, _count -1 + 1 );
    _count += 1;
    // crc_unit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__crc_unit) , _count, _count -1 + 1 );
    _count += 1;
    // crc_profile
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__crc_profile) , _count, _count -1 + 4 );
    _count += 4;
    // mask_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mask_vld) , _count, _count -1 + 1 );
    _count += 1;
    // mask_unit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mask_unit) , _count, _count -1 + 1 );
    _count += 1;
    // mask_profile
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mask_profile) , _count, _count -1 + 4 );
    _count += 4;
    // hdrfld_start
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdrfld_start) , _count, _count -1 + 8 );
    _count += 8;
    // hdrfld_end
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdrfld_end) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // fld_en_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_0) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_0) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_0) , _count, _count -1 + 16 );
    _count += 16;
    // fld_align_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_align_0) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_1) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_1) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_1) , _count, _count -1 + 16 );
    _count += 16;
    // fld_align_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_align_1) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_2) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_2) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_2) , _count, _count -1 + 16 );
    _count += 16;
    // fld_align_2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_align_2) , _count, _count -1 + 1 );
    _count += 1;
    // fld_en_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_en_3) , _count, _count -1 + 1 );
    _count += 1;
    // fld_start_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start_3) , _count, _count -1 + 16 );
    _count += 16;
    // fld_end_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end_3) , _count, _count -1 + 16 );
    _count += 16;
    // fld_align_3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_align_3) , _count, _count -1 + 1 );
    _count += 1;
    // add_len
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__add_len) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // use_phv_len
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__use_phv_len) , _count, _count -1 + 1 );
    _count += 1;
    // phv_len_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_len_sel) , _count, _count -1 + 5 );
    _count += 5;
    // len_mask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_mask) , _count, _count -1 + 16 );
    _count += 16;
    // len_shift_left
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_shift_left) , _count, _count -1 + 1 );
    _count += 1;
    // len_shift_val
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__len_shift_val) , _count, _count -1 + 4 );
    _count += 4;
    // start_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_adj) , _count, _count -1 + 16 );
    _count += 16;
    // end_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_adj) , _count, _count -1 + 16 );
    _count += 16;
    // loc_adj
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__loc_adj) , _count, _count -1 + 16 );
    _count += 16;
    // add_len
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__add_len) , _count, _count -1 + 1 );
    _count += 1;
    // csum_unit_include_bm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_unit_include_bm) , _count, _count -1 + 3 );
    _count += 3;
    return ret_val;
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // hdr_num
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdr_num) , _count, _count -1 + 7 );
    _count += 7;
    // csum_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_vld) , _count, _count -1 + 1 );
    _count += 1;
    // csum_unit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_unit) , _count, _count -1 + 3 );
    _count += 3;
    // csum_profile
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_profile) , _count, _count -1 + 4 );
    _count += 4;
    // phdr_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phdr_vld) , _count, _count -1 + 1 );
    _count += 1;
    // phdr_unit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phdr_unit) , _count, _count -1 + 3 );
    _count += 3;
    // phdr_profile
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phdr_profile) , _count, _count -1 + 4 );
    _count += 4;
    // csum_unit_include_bm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_unit_include_bm) , _count, _count -1 + 5 );
    _count += 5;
    // crc_include_bm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__crc_include_bm) , _count, _count -1 + 1 );
    _count += 1;
    // hdrfld_start
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdrfld_start) , _count, _count -1 + 8 );
    _count += 8;
    // hdrfld_end
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdrfld_end) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_dppcsum_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_csum_hdrs
    for(int ii = 0; ii < 32; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_csum_hdrs[ii].all() , _count, _count -1 + cfg_csum_hdrs[ii].get_width() );
        _count += cfg_csum_hdrs[ii].get_width();
    }
    // cfg_csum_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_csum_profile[ii].all() , _count, _count -1 + cfg_csum_profile[ii].get_width() );
        _count += cfg_csum_profile[ii].get_width();
    }
    // cfg_csum_phdr_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_csum_phdr_profile[ii].all() , _count, _count -1 + cfg_csum_phdr_profile[ii].get_width() );
        _count += cfg_csum_phdr_profile[ii].get_width();
    }
    // cfg_crc_hdrs
    for(int ii = 0; ii < 32; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_crc_hdrs[ii].all() , _count, _count -1 + cfg_crc_hdrs[ii].get_width() );
        _count += cfg_crc_hdrs[ii].get_width();
    }
    // cfg_crc_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_crc_profile[ii].all() , _count, _count -1 + cfg_crc_profile[ii].get_width() );
        _count += cfg_crc_profile[ii].get_width();
    }
    // cfg_crc_mask_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_crc_mask_profile[ii].all() , _count, _count -1 + cfg_crc_mask_profile[ii].get_width() );
        _count += cfg_crc_mask_profile[ii].get_width();
    }
    return ret_val;
}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0);
            register_get_func("fld_en_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0);
            register_get_func("fld_start_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0);
            register_get_func("fld_end_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("skip_first_nibble_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0);
            register_get_func("skip_first_nibble_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1);
            register_get_func("fld_en_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1);
            register_get_func("fld_start_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1);
            register_get_func("fld_end_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("skip_first_nibble_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1);
            register_get_func("skip_first_nibble_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2);
            register_get_func("fld_en_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2);
            register_get_func("fld_start_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2);
            register_get_func("fld_end_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("skip_first_nibble_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2);
            register_get_func("skip_first_nibble_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3);
            register_get_func("fld_en_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3);
            register_get_func("fld_start_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3);
            register_get_func("fld_end_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("skip_first_nibble_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3);
            register_get_func("skip_first_nibble_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_4", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4);
            register_get_func("fld_en_4", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_4", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4);
            register_get_func("fld_start_4", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_4", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4);
            register_get_func("fld_end_4", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("skip_first_nibble_4", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4);
            register_get_func("skip_first_nibble_4", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fill", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fill);
            register_get_func("fill", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_mask_profile_t::fill);
        }
        #endif
    
}

void cap_dppcsum_csr_cfg_crc_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("use_phv_len", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len);
            register_get_func("use_phv_len", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_len_sel", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel);
            register_get_func("phv_len_sel", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_mask", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_mask);
            register_get_func("len_mask", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_mask);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_shift_left", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left);
            register_get_func("len_shift_left", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_shift_val", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val);
            register_get_func("len_shift_val", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_sop", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::start_sop);
            register_get_func("start_sop", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::start_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_eop", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::end_eop);
            register_get_func("end_eop", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::end_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::start_adj);
            register_get_func("start_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::start_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::end_adj);
            register_get_func("end_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::end_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("loc_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::loc_adj);
            register_get_func("loc_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::loc_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("crc_unit_include_bm", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm);
            register_get_func("crc_unit_include_bm", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm);
        }
        #endif
    
}

void cap_dppcsum_csr_cfg_crc_hdrs_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdr_num", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num);
            register_get_func("hdr_num", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("crc_vld", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld);
            register_get_func("crc_vld", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("crc_unit", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit);
            register_get_func("crc_unit", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("crc_profile", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile);
            register_get_func("crc_profile", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mask_vld", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld);
            register_get_func("mask_vld", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mask_unit", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit);
            register_get_func("mask_unit", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mask_profile", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile);
            register_get_func("mask_profile", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdrfld_start", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start);
            register_get_func("hdrfld_start", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdrfld_end", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end);
            register_get_func("hdrfld_end", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end);
        }
        #endif
    
}

void cap_dppcsum_csr_cfg_csum_phdr_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0);
            register_get_func("fld_en_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0);
            register_get_func("fld_start_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0);
            register_get_func("fld_end_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_align_0", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0);
            register_get_func("fld_align_0", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1);
            register_get_func("fld_en_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1);
            register_get_func("fld_start_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1);
            register_get_func("fld_end_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_align_1", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1);
            register_get_func("fld_align_1", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2);
            register_get_func("fld_en_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2);
            register_get_func("fld_start_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2);
            register_get_func("fld_end_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_align_2", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2);
            register_get_func("fld_align_2", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_en_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3);
            register_get_func("fld_en_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3);
            register_get_func("fld_start_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3);
            register_get_func("fld_end_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_align_3", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3);
            register_get_func("fld_align_3", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("add_len", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len);
            register_get_func("add_len", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len);
        }
        #endif
    
}

void cap_dppcsum_csr_cfg_csum_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("use_phv_len", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len);
            register_get_func("use_phv_len", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_len_sel", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel);
            register_get_func("phv_len_sel", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_mask", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_mask);
            register_get_func("len_mask", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_mask);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_shift_left", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left);
            register_get_func("len_shift_left", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("len_shift_val", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val);
            register_get_func("len_shift_val", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::start_adj);
            register_get_func("start_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::start_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::end_adj);
            register_get_func("end_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::end_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("loc_adj", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::loc_adj);
            register_get_func("loc_adj", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::loc_adj);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("add_len", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::add_len);
            register_get_func("add_len", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::add_len);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_unit_include_bm", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::csum_unit_include_bm);
            register_get_func("csum_unit_include_bm", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_profile_t::csum_unit_include_bm);
        }
        #endif
    
}

void cap_dppcsum_csr_cfg_csum_hdrs_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdr_num", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num);
            register_get_func("hdr_num", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_vld", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld);
            register_get_func("csum_vld", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_unit", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit);
            register_get_func("csum_unit", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_profile", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile);
            register_get_func("csum_profile", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phdr_vld", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld);
            register_get_func("phdr_vld", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phdr_unit", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit);
            register_get_func("phdr_unit", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phdr_profile", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile);
            register_get_func("phdr_profile", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_unit_include_bm", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm);
            register_get_func("csum_unit_include_bm", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("crc_include_bm", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm);
            register_get_func("crc_include_bm", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdrfld_start", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start);
            register_get_func("hdrfld_start", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdrfld_end", (cap_csr_base::set_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end);
            register_get_func("hdrfld_end", (cap_csr_base::get_function_type_t)&cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end);
        }
        #endif
    
}

void cap_dppcsum_csr_t::init() {

    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) cfg_csum_hdrs[ii].set_field_init_done(true);
        cfg_csum_hdrs[ii].set_attributes(this,"cfg_csum_hdrs["+to_string(ii)+"]",  0x0 + (cfg_csum_hdrs[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_csum_profile[ii].set_field_init_done(true);
        cfg_csum_profile[ii].set_attributes(this,"cfg_csum_profile["+to_string(ii)+"]",  0x100 + (cfg_csum_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_csum_phdr_profile[ii].set_field_init_done(true);
        cfg_csum_phdr_profile[ii].set_attributes(this,"cfg_csum_phdr_profile["+to_string(ii)+"]",  0x200 + (cfg_csum_phdr_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) cfg_crc_hdrs[ii].set_field_init_done(true);
        cfg_crc_hdrs[ii].set_attributes(this,"cfg_crc_hdrs["+to_string(ii)+"]",  0x400 + (cfg_crc_hdrs[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_crc_profile[ii].set_field_init_done(true);
        cfg_crc_profile[ii].set_attributes(this,"cfg_crc_profile["+to_string(ii)+"]",  0x500 + (cfg_crc_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_crc_mask_profile[ii].set_field_init_done(true);
        cfg_crc_mask_profile[ii].set_attributes(this,"cfg_crc_mask_profile["+to_string(ii)+"]",  0x600 + (cfg_crc_mask_profile[ii].get_byte_size()*ii));
    }
}

void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0(const cpp_int & _val) { 
    // fld_en_0
    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_0() const {
    return int_var__fld_en_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0(const cpp_int & _val) { 
    // fld_start_0
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_0() const {
    return int_var__fld_start_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0(const cpp_int & _val) { 
    // fld_end_0
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_0() const {
    return int_var__fld_end_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0(const cpp_int & _val) { 
    // skip_first_nibble_0
    int_var__skip_first_nibble_0 = _val.convert_to< skip_first_nibble_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_0() const {
    return int_var__skip_first_nibble_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1(const cpp_int & _val) { 
    // fld_en_1
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_1() const {
    return int_var__fld_en_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1(const cpp_int & _val) { 
    // fld_start_1
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_1() const {
    return int_var__fld_start_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1(const cpp_int & _val) { 
    // fld_end_1
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_1() const {
    return int_var__fld_end_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1(const cpp_int & _val) { 
    // skip_first_nibble_1
    int_var__skip_first_nibble_1 = _val.convert_to< skip_first_nibble_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_1() const {
    return int_var__skip_first_nibble_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2(const cpp_int & _val) { 
    // fld_en_2
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_2() const {
    return int_var__fld_en_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2(const cpp_int & _val) { 
    // fld_start_2
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_2() const {
    return int_var__fld_start_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2(const cpp_int & _val) { 
    // fld_end_2
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_2() const {
    return int_var__fld_end_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2(const cpp_int & _val) { 
    // skip_first_nibble_2
    int_var__skip_first_nibble_2 = _val.convert_to< skip_first_nibble_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_2() const {
    return int_var__skip_first_nibble_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3(const cpp_int & _val) { 
    // fld_en_3
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_3() const {
    return int_var__fld_en_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3(const cpp_int & _val) { 
    // fld_start_3
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_3() const {
    return int_var__fld_start_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3(const cpp_int & _val) { 
    // fld_end_3
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_3() const {
    return int_var__fld_end_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3(const cpp_int & _val) { 
    // skip_first_nibble_3
    int_var__skip_first_nibble_3 = _val.convert_to< skip_first_nibble_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_3() const {
    return int_var__skip_first_nibble_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4(const cpp_int & _val) { 
    // fld_en_4
    int_var__fld_en_4 = _val.convert_to< fld_en_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_en_4() const {
    return int_var__fld_en_4.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4(const cpp_int & _val) { 
    // fld_start_4
    int_var__fld_start_4 = _val.convert_to< fld_start_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_start_4() const {
    return int_var__fld_start_4.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4(const cpp_int & _val) { 
    // fld_end_4
    int_var__fld_end_4 = _val.convert_to< fld_end_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fld_end_4() const {
    return int_var__fld_end_4.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4(const cpp_int & _val) { 
    // skip_first_nibble_4
    int_var__skip_first_nibble_4 = _val.convert_to< skip_first_nibble_4_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::skip_first_nibble_4() const {
    return int_var__skip_first_nibble_4.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_mask_profile_t::fill(const cpp_int & _val) { 
    // fill
    int_var__fill = _val.convert_to< fill_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_mask_profile_t::fill() const {
    return int_var__fill.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len(const cpp_int & _val) { 
    // use_phv_len
    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::use_phv_len() const {
    return int_var__use_phv_len.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel(const cpp_int & _val) { 
    // phv_len_sel
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::phv_len_sel() const {
    return int_var__phv_len_sel.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_mask() const {
    return int_var__len_mask.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_shift_left() const {
    return int_var__len_shift_left.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::len_shift_val() const {
    return int_var__len_shift_val.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::start_sop(const cpp_int & _val) { 
    // start_sop
    int_var__start_sop = _val.convert_to< start_sop_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::start_sop() const {
    return int_var__start_sop.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::end_eop(const cpp_int & _val) { 
    // end_eop
    int_var__end_eop = _val.convert_to< end_eop_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::end_eop() const {
    return int_var__end_eop.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::start_adj() const {
    return int_var__start_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::end_adj() const {
    return int_var__end_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::loc_adj(const cpp_int & _val) { 
    // loc_adj
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::loc_adj() const {
    return int_var__loc_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm(const cpp_int & _val) { 
    // crc_unit_include_bm
    int_var__crc_unit_include_bm = _val.convert_to< crc_unit_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_profile_t::crc_unit_include_bm() const {
    return int_var__crc_unit_include_bm.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num(const cpp_int & _val) { 
    // hdr_num
    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdr_num() const {
    return int_var__hdr_num.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld(const cpp_int & _val) { 
    // crc_vld
    int_var__crc_vld = _val.convert_to< crc_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_vld() const {
    return int_var__crc_vld.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit(const cpp_int & _val) { 
    // crc_unit
    int_var__crc_unit = _val.convert_to< crc_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_unit() const {
    return int_var__crc_unit.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile(const cpp_int & _val) { 
    // crc_profile
    int_var__crc_profile = _val.convert_to< crc_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::crc_profile() const {
    return int_var__crc_profile.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld(const cpp_int & _val) { 
    // mask_vld
    int_var__mask_vld = _val.convert_to< mask_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_vld() const {
    return int_var__mask_vld.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit(const cpp_int & _val) { 
    // mask_unit
    int_var__mask_unit = _val.convert_to< mask_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_unit() const {
    return int_var__mask_unit.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile(const cpp_int & _val) { 
    // mask_profile
    int_var__mask_profile = _val.convert_to< mask_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::mask_profile() const {
    return int_var__mask_profile.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start(const cpp_int & _val) { 
    // hdrfld_start
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_start() const {
    return int_var__hdrfld_start.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end(const cpp_int & _val) { 
    // hdrfld_end
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_crc_hdrs_t::hdrfld_end() const {
    return int_var__hdrfld_end.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0(const cpp_int & _val) { 
    // fld_en_0
    int_var__fld_en_0 = _val.convert_to< fld_en_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_0() const {
    return int_var__fld_en_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0(const cpp_int & _val) { 
    // fld_start_0
    int_var__fld_start_0 = _val.convert_to< fld_start_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_0() const {
    return int_var__fld_start_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0(const cpp_int & _val) { 
    // fld_end_0
    int_var__fld_end_0 = _val.convert_to< fld_end_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_0() const {
    return int_var__fld_end_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0(const cpp_int & _val) { 
    // fld_align_0
    int_var__fld_align_0 = _val.convert_to< fld_align_0_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_0() const {
    return int_var__fld_align_0.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1(const cpp_int & _val) { 
    // fld_en_1
    int_var__fld_en_1 = _val.convert_to< fld_en_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_1() const {
    return int_var__fld_en_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1(const cpp_int & _val) { 
    // fld_start_1
    int_var__fld_start_1 = _val.convert_to< fld_start_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_1() const {
    return int_var__fld_start_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1(const cpp_int & _val) { 
    // fld_end_1
    int_var__fld_end_1 = _val.convert_to< fld_end_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_1() const {
    return int_var__fld_end_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1(const cpp_int & _val) { 
    // fld_align_1
    int_var__fld_align_1 = _val.convert_to< fld_align_1_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_1() const {
    return int_var__fld_align_1.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2(const cpp_int & _val) { 
    // fld_en_2
    int_var__fld_en_2 = _val.convert_to< fld_en_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_2() const {
    return int_var__fld_en_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2(const cpp_int & _val) { 
    // fld_start_2
    int_var__fld_start_2 = _val.convert_to< fld_start_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_2() const {
    return int_var__fld_start_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2(const cpp_int & _val) { 
    // fld_end_2
    int_var__fld_end_2 = _val.convert_to< fld_end_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_2() const {
    return int_var__fld_end_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2(const cpp_int & _val) { 
    // fld_align_2
    int_var__fld_align_2 = _val.convert_to< fld_align_2_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_2() const {
    return int_var__fld_align_2.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3(const cpp_int & _val) { 
    // fld_en_3
    int_var__fld_en_3 = _val.convert_to< fld_en_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_en_3() const {
    return int_var__fld_en_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3(const cpp_int & _val) { 
    // fld_start_3
    int_var__fld_start_3 = _val.convert_to< fld_start_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_start_3() const {
    return int_var__fld_start_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3(const cpp_int & _val) { 
    // fld_end_3
    int_var__fld_end_3 = _val.convert_to< fld_end_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_end_3() const {
    return int_var__fld_end_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3(const cpp_int & _val) { 
    // fld_align_3
    int_var__fld_align_3 = _val.convert_to< fld_align_3_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::fld_align_3() const {
    return int_var__fld_align_3.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_phdr_profile_t::add_len() const {
    return int_var__add_len.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len(const cpp_int & _val) { 
    // use_phv_len
    int_var__use_phv_len = _val.convert_to< use_phv_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::use_phv_len() const {
    return int_var__use_phv_len.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel(const cpp_int & _val) { 
    // phv_len_sel
    int_var__phv_len_sel = _val.convert_to< phv_len_sel_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::phv_len_sel() const {
    return int_var__phv_len_sel.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_mask(const cpp_int & _val) { 
    // len_mask
    int_var__len_mask = _val.convert_to< len_mask_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_mask() const {
    return int_var__len_mask.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left(const cpp_int & _val) { 
    // len_shift_left
    int_var__len_shift_left = _val.convert_to< len_shift_left_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_shift_left() const {
    return int_var__len_shift_left.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val(const cpp_int & _val) { 
    // len_shift_val
    int_var__len_shift_val = _val.convert_to< len_shift_val_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::len_shift_val() const {
    return int_var__len_shift_val.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::start_adj(const cpp_int & _val) { 
    // start_adj
    int_var__start_adj = _val.convert_to< start_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::start_adj() const {
    return int_var__start_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::end_adj(const cpp_int & _val) { 
    // end_adj
    int_var__end_adj = _val.convert_to< end_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::end_adj() const {
    return int_var__end_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::loc_adj(const cpp_int & _val) { 
    // loc_adj
    int_var__loc_adj = _val.convert_to< loc_adj_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::loc_adj() const {
    return int_var__loc_adj.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::add_len(const cpp_int & _val) { 
    // add_len
    int_var__add_len = _val.convert_to< add_len_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::add_len() const {
    return int_var__add_len.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_profile_t::csum_unit_include_bm(const cpp_int & _val) { 
    // csum_unit_include_bm
    int_var__csum_unit_include_bm = _val.convert_to< csum_unit_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_profile_t::csum_unit_include_bm() const {
    return int_var__csum_unit_include_bm.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num(const cpp_int & _val) { 
    // hdr_num
    int_var__hdr_num = _val.convert_to< hdr_num_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdr_num() const {
    return int_var__hdr_num.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld(const cpp_int & _val) { 
    // csum_vld
    int_var__csum_vld = _val.convert_to< csum_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_vld() const {
    return int_var__csum_vld.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit(const cpp_int & _val) { 
    // csum_unit
    int_var__csum_unit = _val.convert_to< csum_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit() const {
    return int_var__csum_unit.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile(const cpp_int & _val) { 
    // csum_profile
    int_var__csum_profile = _val.convert_to< csum_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_profile() const {
    return int_var__csum_profile.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld(const cpp_int & _val) { 
    // phdr_vld
    int_var__phdr_vld = _val.convert_to< phdr_vld_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_vld() const {
    return int_var__phdr_vld.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit(const cpp_int & _val) { 
    // phdr_unit
    int_var__phdr_unit = _val.convert_to< phdr_unit_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_unit() const {
    return int_var__phdr_unit.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile(const cpp_int & _val) { 
    // phdr_profile
    int_var__phdr_profile = _val.convert_to< phdr_profile_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::phdr_profile() const {
    return int_var__phdr_profile.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm(const cpp_int & _val) { 
    // csum_unit_include_bm
    int_var__csum_unit_include_bm = _val.convert_to< csum_unit_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::csum_unit_include_bm() const {
    return int_var__csum_unit_include_bm.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm(const cpp_int & _val) { 
    // crc_include_bm
    int_var__crc_include_bm = _val.convert_to< crc_include_bm_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::crc_include_bm() const {
    return int_var__crc_include_bm.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start(const cpp_int & _val) { 
    // hdrfld_start
    int_var__hdrfld_start = _val.convert_to< hdrfld_start_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_start() const {
    return int_var__hdrfld_start.convert_to< cpp_int >();
}
    
void cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end(const cpp_int & _val) { 
    // hdrfld_end
    int_var__hdrfld_end = _val.convert_to< hdrfld_end_cpp_int_t >();
}

cpp_int cap_dppcsum_csr_cfg_csum_hdrs_t::hdrfld_end() const {
    return int_var__hdrfld_end.convert_to< cpp_int >();
}
    