
#include "cap_npv_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_npv_csr_dhs_lif_qstate_map_entry_t::cap_npv_csr_dhs_lif_qstate_map_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_dhs_lif_qstate_map_entry_t::~cap_npv_csr_dhs_lif_qstate_map_entry_t() { }

cap_npv_csr_dhs_lif_qstate_map_t::cap_npv_csr_dhs_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_npv_csr_dhs_lif_qstate_map_t::~cap_npv_csr_dhs_lif_qstate_map_t() { }

cap_npv_csr_sta_ecc_lif_qstate_map_t::cap_npv_csr_sta_ecc_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_sta_ecc_lif_qstate_map_t::~cap_npv_csr_sta_ecc_lif_qstate_map_t() { }

cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::~cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t() { }

cap_npv_csr_cfg_qstate_map_rsp_t::cap_npv_csr_cfg_qstate_map_rsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_cfg_qstate_map_rsp_t::~cap_npv_csr_cfg_qstate_map_rsp_t() { }

cap_npv_csr_cfg_qstate_map_req_t::cap_npv_csr_cfg_qstate_map_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_cfg_qstate_map_req_t::~cap_npv_csr_cfg_qstate_map_req_t() { }

cap_npv_csr_cfg_profile_t::cap_npv_csr_cfg_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_npv_csr_cfg_profile_t::~cap_npv_csr_cfg_profile_t() { }

cap_npv_csr_t::cap_npv_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(131072);
        set_attributes(0,get_name(), 0);
        }
cap_npv_csr_t::~cap_npv_csr_t() { }

void cap_npv_csr_dhs_lif_qstate_map_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".vld: 0x" << int_var__vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qstate_base: 0x" << int_var__qstate_base << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length0: 0x" << int_var__length0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size0: 0x" << int_var__size0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length1: 0x" << int_var__length1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size1: 0x" << int_var__size1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length2: 0x" << int_var__length2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size2: 0x" << int_var__size2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length3: 0x" << int_var__length3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size3: 0x" << int_var__size3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length4: 0x" << int_var__length4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size4: 0x" << int_var__size4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length5: 0x" << int_var__length5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size5: 0x" << int_var__size5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length6: 0x" << int_var__length6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size6: 0x" << int_var__size6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length7: 0x" << int_var__length7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size7: 0x" << int_var__size7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_npv_csr_dhs_lif_qstate_map_t::show() {

    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
}

void cap_npv_csr_sta_ecc_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cor: 0x" << int_var__cor << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".det: 0x" << int_var__det << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs: 0x" << int_var__dhs << dec << endl)
}

void cap_npv_csr_cfg_qstate_map_rsp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_enable: 0x" << int_var__qid_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype_enable: 0x" << int_var__qtype_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_enable: 0x" << int_var__addr_enable << dec << endl)
}

void cap_npv_csr_cfg_qstate_map_req_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lif_override_enable: 0x" << int_var__lif_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype_override_enable: 0x" << int_var__qtype_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype: 0x" << int_var__qtype << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_override_enable: 0x" << int_var__qid_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
}

void cap_npv_csr_cfg_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".start_offset: 0x" << int_var__start_offset << dec << endl)
}

void cap_npv_csr_t::show() {

    for(int ii = 0; ii < 8; ii++) {
        cfg_profile[ii].show();
    }
    cfg_qstate_map_req.show();
    cfg_qstate_map_rsp.show();
    cfg_ecc_disable_lif_qstate_map.show();
    sta_ecc_lif_qstate_map.show();
    dhs_lif_qstate_map.show();
}

int cap_npv_csr_dhs_lif_qstate_map_entry_t::get_width() const {
    return cap_npv_csr_dhs_lif_qstate_map_entry_t::s_get_width();

}

int cap_npv_csr_dhs_lif_qstate_map_t::get_width() const {
    return cap_npv_csr_dhs_lif_qstate_map_t::s_get_width();

}

int cap_npv_csr_sta_ecc_lif_qstate_map_t::get_width() const {
    return cap_npv_csr_sta_ecc_lif_qstate_map_t::s_get_width();

}

int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::get_width() const {
    return cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width();

}

int cap_npv_csr_cfg_qstate_map_rsp_t::get_width() const {
    return cap_npv_csr_cfg_qstate_map_rsp_t::s_get_width();

}

int cap_npv_csr_cfg_qstate_map_req_t::get_width() const {
    return cap_npv_csr_cfg_qstate_map_req_t::s_get_width();

}

int cap_npv_csr_cfg_profile_t::get_width() const {
    return cap_npv_csr_cfg_profile_t::s_get_width();

}

int cap_npv_csr_t::get_width() const {
    return cap_npv_csr_t::s_get_width();

}

int cap_npv_csr_dhs_lif_qstate_map_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // vld
    _count += 22; // qstate_base
    _count += 5; // length0
    _count += 3; // size0
    _count += 5; // length1
    _count += 3; // size1
    _count += 5; // length2
    _count += 3; // size2
    _count += 5; // length3
    _count += 3; // size3
    _count += 5; // length4
    _count += 3; // size4
    _count += 5; // length5
    _count += 3; // size5
    _count += 5; // length6
    _count += 3; // size6
    _count += 5; // length7
    _count += 3; // size7
    _count += 9; // spare
    _count += 8; // ecc
    return _count;
}

int cap_npv_csr_dhs_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += (cap_npv_csr_dhs_lif_qstate_map_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_npv_csr_sta_ecc_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 11; // addr
    return _count;
}

int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // cor
    _count += 1; // det
    _count += 1; // dhs
    return _count;
}

int cap_npv_csr_cfg_qstate_map_rsp_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // qid_enable
    _count += 1; // qtype_enable
    _count += 1; // addr_enable
    return _count;
}

int cap_npv_csr_cfg_qstate_map_req_t::s_get_width() {
    int _count = 0;

    _count += 1; // lif_override_enable
    _count += 11; // lif
    _count += 1; // qtype_override_enable
    _count += 3; // qtype
    _count += 1; // qid_override_enable
    _count += 24; // qid
    return _count;
}

int cap_npv_csr_cfg_profile_t::s_get_width() {
    int _count = 0;

    _count += 16; // start_offset
    return _count;
}

int cap_npv_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_npv_csr_cfg_profile_t::s_get_width() * 8); // cfg_profile
    _count += cap_npv_csr_cfg_qstate_map_req_t::s_get_width(); // cfg_qstate_map_req
    _count += cap_npv_csr_cfg_qstate_map_rsp_t::s_get_width(); // cfg_qstate_map_rsp
    _count += cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width(); // cfg_ecc_disable_lif_qstate_map
    _count += cap_npv_csr_sta_ecc_lif_qstate_map_t::s_get_width(); // sta_ecc_lif_qstate_map
    _count += cap_npv_csr_dhs_lif_qstate_map_t::s_get_width(); // dhs_lif_qstate_map
    return _count;
}

void cap_npv_csr_dhs_lif_qstate_map_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // vld
    int_var__vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< vld_cpp_int_t >()  ;
    _count += 1;
    // qstate_base
    int_var__qstate_base = hlp.get_slc(_val, _count, _count -1 + 22 ).convert_to< qstate_base_cpp_int_t >()  ;
    _count += 22;
    // length0
    int_var__length0 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length0_cpp_int_t >()  ;
    _count += 5;
    // size0
    int_var__size0 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size0_cpp_int_t >()  ;
    _count += 3;
    // length1
    int_var__length1 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length1_cpp_int_t >()  ;
    _count += 5;
    // size1
    int_var__size1 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size1_cpp_int_t >()  ;
    _count += 3;
    // length2
    int_var__length2 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length2_cpp_int_t >()  ;
    _count += 5;
    // size2
    int_var__size2 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size2_cpp_int_t >()  ;
    _count += 3;
    // length3
    int_var__length3 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length3_cpp_int_t >()  ;
    _count += 5;
    // size3
    int_var__size3 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size3_cpp_int_t >()  ;
    _count += 3;
    // length4
    int_var__length4 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length4_cpp_int_t >()  ;
    _count += 5;
    // size4
    int_var__size4 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size4_cpp_int_t >()  ;
    _count += 3;
    // length5
    int_var__length5 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length5_cpp_int_t >()  ;
    _count += 5;
    // size5
    int_var__size5 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size5_cpp_int_t >()  ;
    _count += 3;
    // length6
    int_var__length6 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length6_cpp_int_t >()  ;
    _count += 5;
    // size6
    int_var__size6 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size6_cpp_int_t >()  ;
    _count += 3;
    // length7
    int_var__length7 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length7_cpp_int_t >()  ;
    _count += 5;
    // size7
    int_var__size7 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size7_cpp_int_t >()  ;
    _count += 3;
    // spare
    int_var__spare = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< spare_cpp_int_t >()  ;
    _count += 9;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 8;
}

void cap_npv_csr_dhs_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_npv_csr_sta_ecc_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // uncorrectable
    int_var__uncorrectable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< uncorrectable_cpp_int_t >()  ;
    _count += 1;
    // correctable
    int_var__correctable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< correctable_cpp_int_t >()  ;
    _count += 1;
    // syndrome
    int_var__syndrome = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< syndrome_cpp_int_t >()  ;
    _count += 8;
    // addr
    int_var__addr = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< addr_cpp_int_t >()  ;
    _count += 11;
}

void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // cor
    int_var__cor = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< cor_cpp_int_t >()  ;
    _count += 1;
    // det
    int_var__det = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< det_cpp_int_t >()  ;
    _count += 1;
    // dhs
    int_var__dhs = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< dhs_cpp_int_t >()  ;
    _count += 1;
}

void cap_npv_csr_cfg_qstate_map_rsp_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // qid_enable
    int_var__qid_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< qid_enable_cpp_int_t >()  ;
    _count += 1;
    // qtype_enable
    int_var__qtype_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< qtype_enable_cpp_int_t >()  ;
    _count += 1;
    // addr_enable
    int_var__addr_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< addr_enable_cpp_int_t >()  ;
    _count += 1;
}

void cap_npv_csr_cfg_qstate_map_req_t::all(const cpp_int & _val) {
    int _count = 0;

    // lif_override_enable
    int_var__lif_override_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lif_override_enable_cpp_int_t >()  ;
    _count += 1;
    // lif
    int_var__lif = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< lif_cpp_int_t >()  ;
    _count += 11;
    // qtype_override_enable
    int_var__qtype_override_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< qtype_override_enable_cpp_int_t >()  ;
    _count += 1;
    // qtype
    int_var__qtype = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< qtype_cpp_int_t >()  ;
    _count += 3;
    // qid_override_enable
    int_var__qid_override_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< qid_override_enable_cpp_int_t >()  ;
    _count += 1;
    // qid
    int_var__qid = hlp.get_slc(_val, _count, _count -1 + 24 ).convert_to< qid_cpp_int_t >()  ;
    _count += 24;
}

void cap_npv_csr_cfg_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // start_offset
    int_var__start_offset = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< start_offset_cpp_int_t >()  ;
    _count += 16;
}

void cap_npv_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_profile
    for(int ii = 0; ii < 8; ii++) {
        cfg_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_profile[ii].get_width()));
        _count += cfg_profile[ii].get_width();
    }
    cfg_qstate_map_req.all( hlp.get_slc(_val, _count, _count -1 + cfg_qstate_map_req.get_width() )); // cfg_qstate_map_req
    _count += cfg_qstate_map_req.get_width();
    cfg_qstate_map_rsp.all( hlp.get_slc(_val, _count, _count -1 + cfg_qstate_map_rsp.get_width() )); // cfg_qstate_map_rsp
    _count += cfg_qstate_map_rsp.get_width();
    cfg_ecc_disable_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() )); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    sta_ecc_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + sta_ecc_lif_qstate_map.get_width() )); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    dhs_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + dhs_lif_qstate_map.get_width() )); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__vld) , _count, _count -1 + 1 );
    _count += 1;
    // qstate_base
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qstate_base) , _count, _count -1 + 22 );
    _count += 22;
    // length0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length0) , _count, _count -1 + 5 );
    _count += 5;
    // size0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size0) , _count, _count -1 + 3 );
    _count += 3;
    // length1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length1) , _count, _count -1 + 5 );
    _count += 5;
    // size1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size1) , _count, _count -1 + 3 );
    _count += 3;
    // length2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length2) , _count, _count -1 + 5 );
    _count += 5;
    // size2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size2) , _count, _count -1 + 3 );
    _count += 3;
    // length3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length3) , _count, _count -1 + 5 );
    _count += 5;
    // size3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size3) , _count, _count -1 + 3 );
    _count += 3;
    // length4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length4) , _count, _count -1 + 5 );
    _count += 5;
    // size4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size4) , _count, _count -1 + 3 );
    _count += 3;
    // length5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length5) , _count, _count -1 + 5 );
    _count += 5;
    // size5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size5) , _count, _count -1 + 3 );
    _count += 3;
    // length6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length6) , _count, _count -1 + 5 );
    _count += 5;
    // size6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size6) , _count, _count -1 + 3 );
    _count += 3;
    // length7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length7) , _count, _count -1 + 5 );
    _count += 5;
    // size7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size7) , _count, _count -1 + 3 );
    _count += 3;
    // spare
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__spare) , _count, _count -1 + 9 );
    _count += 9;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_npv_csr_sta_ecc_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // uncorrectable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__uncorrectable) , _count, _count -1 + 1 );
    _count += 1;
    // correctable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__correctable) , _count, _count -1 + 1 );
    _count += 1;
    // syndrome
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__syndrome) , _count, _count -1 + 8 );
    _count += 8;
    // addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr) , _count, _count -1 + 11 );
    _count += 11;
    return ret_val;
}

cpp_int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cor
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cor) , _count, _count -1 + 1 );
    _count += 1;
    // det
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__det) , _count, _count -1 + 1 );
    _count += 1;
    // dhs
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dhs) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_npv_csr_cfg_qstate_map_rsp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // qid_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid_enable) , _count, _count -1 + 1 );
    _count += 1;
    // qtype_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qtype_enable) , _count, _count -1 + 1 );
    _count += 1;
    // addr_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_enable) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // lif_override_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lif_override_enable) , _count, _count -1 + 1 );
    _count += 1;
    // lif
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lif) , _count, _count -1 + 11 );
    _count += 11;
    // qtype_override_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qtype_override_enable) , _count, _count -1 + 1 );
    _count += 1;
    // qtype
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qtype) , _count, _count -1 + 3 );
    _count += 3;
    // qid_override_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid_override_enable) , _count, _count -1 + 1 );
    _count += 1;
    // qid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid) , _count, _count -1 + 24 );
    _count += 24;
    return ret_val;
}

cpp_int cap_npv_csr_cfg_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // start_offset
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_offset) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_npv_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_profile
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_profile[ii].all() , _count, _count -1 + cfg_profile[ii].get_width() );
        _count += cfg_profile[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, cfg_qstate_map_req.all() , _count, _count -1 + cfg_qstate_map_req.get_width() ); // cfg_qstate_map_req
    _count += cfg_qstate_map_req.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_qstate_map_rsp.all() , _count, _count -1 + cfg_qstate_map_rsp.get_width() ); // cfg_qstate_map_rsp
    _count += cfg_qstate_map_rsp.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_ecc_disable_lif_qstate_map.all() , _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() ); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, sta_ecc_lif_qstate_map.all() , _count, _count -1 + sta_ecc_lif_qstate_map.get_width() ); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_lif_qstate_map.all() , _count, _count -1 + dhs_lif_qstate_map.get_width() ); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
    return ret_val;
}

void cap_npv_csr_dhs_lif_qstate_map_entry_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("vld", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::vld);
        register_get_func("vld", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::vld);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qstate_base", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::qstate_base);
        register_get_func("qstate_base", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::qstate_base);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length0", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length0);
        register_get_func("length0", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length0);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size0", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size0);
        register_get_func("size0", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size0);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length1", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length1);
        register_get_func("length1", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length1);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size1", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size1);
        register_get_func("size1", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size1);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length2", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length2);
        register_get_func("length2", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length2);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size2", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size2);
        register_get_func("size2", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size2);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length3", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length3);
        register_get_func("length3", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length3);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size3", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size3);
        register_get_func("size3", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size3);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length4", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length4);
        register_get_func("length4", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length4);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size4", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size4);
        register_get_func("size4", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size4);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length5", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length5);
        register_get_func("length5", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length5);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size5", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size5);
        register_get_func("size5", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size5);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length6", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length6);
        register_get_func("length6", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length6);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size6", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size6);
        register_get_func("size6", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size6);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("length7", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length7);
        register_get_func("length7", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::length7);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("size7", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size7);
        register_get_func("size7", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::size7);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("spare", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::spare);
        register_get_func("spare", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::spare);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::ecc);
        register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_npv_csr_dhs_lif_qstate_map_entry_t::ecc);
    #endif
    
}

void cap_npv_csr_dhs_lif_qstate_map_t::init() {

    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_npv_csr_sta_ecc_lif_qstate_map_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("uncorrectable", (cap_csr_base::set_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
        register_get_func("uncorrectable", (cap_csr_base::get_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("correctable", (cap_csr_base::set_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::correctable);
        register_get_func("correctable", (cap_csr_base::get_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::correctable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::syndrome);
        register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::syndrome);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::addr);
        register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_npv_csr_sta_ecc_lif_qstate_map_t::addr);
    #endif
    
}

void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("cor", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
        register_get_func("cor", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("det", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::det);
        register_get_func("det", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::det);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("dhs", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
        register_get_func("dhs", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
    #endif
    
}

void cap_npv_csr_cfg_qstate_map_rsp_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::enable);
        register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qid_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::qid_enable);
        register_get_func("qid_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::qid_enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qtype_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::qtype_enable);
        register_get_func("qtype_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::qtype_enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("addr_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::addr_enable);
        register_get_func("addr_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_rsp_t::addr_enable);
    #endif
    
    set_reset_val(cpp_int("0xe"));
    all(get_reset_val());
}

void cap_npv_csr_cfg_qstate_map_req_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("lif_override_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::lif_override_enable);
        register_get_func("lif_override_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::lif_override_enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("lif", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::lif);
        register_get_func("lif", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::lif);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qtype_override_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qtype_override_enable);
        register_get_func("qtype_override_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qtype_override_enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qtype", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qtype);
        register_get_func("qtype", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qtype);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qid_override_enable", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qid_override_enable);
        register_get_func("qid_override_enable", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qid_override_enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qid);
        register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_qstate_map_req_t::qid);
    #endif
    
    set_reset_val(cpp_int("0x11001"));
    all(get_reset_val());
}

void cap_npv_csr_cfg_profile_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("start_offset", (cap_csr_base::set_function_type_t)&cap_npv_csr_cfg_profile_t::start_offset);
        register_get_func("start_offset", (cap_csr_base::get_function_type_t)&cap_npv_csr_cfg_profile_t::start_offset);
    #endif
    
    set_reset_val(cpp_int("0x100"));
    all(get_reset_val());
}

void cap_npv_csr_t::init() {

    for(int ii = 0; ii < 8; ii++) {
        cfg_profile[ii].set_attributes(this,"cfg_profile["+to_string(ii)+"]",  0x40 + (cfg_profile[ii].get_byte_size()*ii));
    }
    cfg_qstate_map_req.set_attributes(this,"cfg_qstate_map_req", 0x60 );
    cfg_qstate_map_rsp.set_attributes(this,"cfg_qstate_map_rsp", 0x68 );
    cfg_ecc_disable_lif_qstate_map.set_attributes(this,"cfg_ecc_disable_lif_qstate_map", 0x10000 );
    sta_ecc_lif_qstate_map.set_attributes(this,"sta_ecc_lif_qstate_map", 0x10004 );
    dhs_lif_qstate_map.set_attributes(this,"dhs_lif_qstate_map", 0x8000 );
}

void cap_npv_csr_dhs_lif_qstate_map_entry_t::vld(const cpp_int & _val) { 
    // vld
    int_var__vld = _val.convert_to< vld_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::vld() const {
    return int_var__vld.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::qstate_base(const cpp_int & _val) { 
    // qstate_base
    int_var__qstate_base = _val.convert_to< qstate_base_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::qstate_base() const {
    return int_var__qstate_base.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length0(const cpp_int & _val) { 
    // length0
    int_var__length0 = _val.convert_to< length0_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length0() const {
    return int_var__length0.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size0(const cpp_int & _val) { 
    // size0
    int_var__size0 = _val.convert_to< size0_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size0() const {
    return int_var__size0.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length1(const cpp_int & _val) { 
    // length1
    int_var__length1 = _val.convert_to< length1_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length1() const {
    return int_var__length1.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size1(const cpp_int & _val) { 
    // size1
    int_var__size1 = _val.convert_to< size1_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size1() const {
    return int_var__size1.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length2(const cpp_int & _val) { 
    // length2
    int_var__length2 = _val.convert_to< length2_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length2() const {
    return int_var__length2.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size2(const cpp_int & _val) { 
    // size2
    int_var__size2 = _val.convert_to< size2_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size2() const {
    return int_var__size2.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length3(const cpp_int & _val) { 
    // length3
    int_var__length3 = _val.convert_to< length3_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length3() const {
    return int_var__length3.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size3(const cpp_int & _val) { 
    // size3
    int_var__size3 = _val.convert_to< size3_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size3() const {
    return int_var__size3.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length4(const cpp_int & _val) { 
    // length4
    int_var__length4 = _val.convert_to< length4_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length4() const {
    return int_var__length4.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size4(const cpp_int & _val) { 
    // size4
    int_var__size4 = _val.convert_to< size4_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size4() const {
    return int_var__size4.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length5(const cpp_int & _val) { 
    // length5
    int_var__length5 = _val.convert_to< length5_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length5() const {
    return int_var__length5.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size5(const cpp_int & _val) { 
    // size5
    int_var__size5 = _val.convert_to< size5_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size5() const {
    return int_var__size5.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length6(const cpp_int & _val) { 
    // length6
    int_var__length6 = _val.convert_to< length6_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length6() const {
    return int_var__length6.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size6(const cpp_int & _val) { 
    // size6
    int_var__size6 = _val.convert_to< size6_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size6() const {
    return int_var__size6.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::length7(const cpp_int & _val) { 
    // length7
    int_var__length7 = _val.convert_to< length7_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::length7() const {
    return int_var__length7.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::size7(const cpp_int & _val) { 
    // size7
    int_var__size7 = _val.convert_to< size7_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::size7() const {
    return int_var__size7.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::spare() const {
    return int_var__spare.convert_to< cpp_int >();
}
    
void cap_npv_csr_dhs_lif_qstate_map_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_npv_csr_dhs_lif_qstate_map_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_npv_csr_sta_ecc_lif_qstate_map_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_npv_csr_sta_ecc_lif_qstate_map_t::uncorrectable() const {
    return int_var__uncorrectable.convert_to< cpp_int >();
}
    
void cap_npv_csr_sta_ecc_lif_qstate_map_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_npv_csr_sta_ecc_lif_qstate_map_t::correctable() const {
    return int_var__correctable.convert_to< cpp_int >();
}
    
void cap_npv_csr_sta_ecc_lif_qstate_map_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_npv_csr_sta_ecc_lif_qstate_map_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_npv_csr_sta_ecc_lif_qstate_map_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_npv_csr_sta_ecc_lif_qstate_map_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::cor(const cpp_int & _val) { 
    // cor
    int_var__cor = _val.convert_to< cor_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::cor() const {
    return int_var__cor.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::det(const cpp_int & _val) { 
    // det
    int_var__det = _val.convert_to< det_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::det() const {
    return int_var__det.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::dhs(const cpp_int & _val) { 
    // dhs
    int_var__dhs = _val.convert_to< dhs_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t::dhs() const {
    return int_var__dhs.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_rsp_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_rsp_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_rsp_t::qid_enable(const cpp_int & _val) { 
    // qid_enable
    int_var__qid_enable = _val.convert_to< qid_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_rsp_t::qid_enable() const {
    return int_var__qid_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_rsp_t::qtype_enable(const cpp_int & _val) { 
    // qtype_enable
    int_var__qtype_enable = _val.convert_to< qtype_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_rsp_t::qtype_enable() const {
    return int_var__qtype_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_rsp_t::addr_enable(const cpp_int & _val) { 
    // addr_enable
    int_var__addr_enable = _val.convert_to< addr_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_rsp_t::addr_enable() const {
    return int_var__addr_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::lif_override_enable(const cpp_int & _val) { 
    // lif_override_enable
    int_var__lif_override_enable = _val.convert_to< lif_override_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::lif_override_enable() const {
    return int_var__lif_override_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::lif() const {
    return int_var__lif.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::qtype_override_enable(const cpp_int & _val) { 
    // qtype_override_enable
    int_var__qtype_override_enable = _val.convert_to< qtype_override_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::qtype_override_enable() const {
    return int_var__qtype_override_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::qtype(const cpp_int & _val) { 
    // qtype
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::qtype() const {
    return int_var__qtype.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::qid_override_enable(const cpp_int & _val) { 
    // qid_override_enable
    int_var__qid_override_enable = _val.convert_to< qid_override_enable_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::qid_override_enable() const {
    return int_var__qid_override_enable.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_qstate_map_req_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_qstate_map_req_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_npv_csr_cfg_profile_t::start_offset(const cpp_int & _val) { 
    // start_offset
    int_var__start_offset = _val.convert_to< start_offset_cpp_int_t >();
}

cpp_int cap_npv_csr_cfg_profile_t::start_offset() const {
    return int_var__start_offset.convert_to< cpp_int >();
}
    
