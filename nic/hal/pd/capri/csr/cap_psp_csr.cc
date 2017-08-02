
#include "cap_psp_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_psp_csr_dhs_lif_qstate_map_entry_t::cap_psp_csr_dhs_lif_qstate_map_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_dhs_lif_qstate_map_entry_t::~cap_psp_csr_dhs_lif_qstate_map_entry_t() { }

cap_psp_csr_dhs_lif_qstate_map_t::cap_psp_csr_dhs_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_psp_csr_dhs_lif_qstate_map_t::~cap_psp_csr_dhs_lif_qstate_map_t() { }

cap_psp_csr_dhs_sw_phv_mem_0_entry_t::cap_psp_csr_dhs_sw_phv_mem_0_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_dhs_sw_phv_mem_0_entry_t::~cap_psp_csr_dhs_sw_phv_mem_0_entry_t() { }

cap_psp_csr_dhs_sw_phv_mem_0_t::cap_psp_csr_dhs_sw_phv_mem_0_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_psp_csr_dhs_sw_phv_mem_0_t::~cap_psp_csr_dhs_sw_phv_mem_0_t() { }

cap_psp_csr_sta_ecc_lif_qstate_map_t::cap_psp_csr_sta_ecc_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_sta_ecc_lif_qstate_map_t::~cap_psp_csr_sta_ecc_lif_qstate_map_t() { }

cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::~cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t() { }

cap_psp_csr_sta_sw_phv_mem_t::cap_psp_csr_sta_sw_phv_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_sta_sw_phv_mem_t::~cap_psp_csr_sta_sw_phv_mem_t() { }

cap_psp_csr_cfg_sw_phv_mem_0_t::cap_psp_csr_cfg_sw_phv_mem_0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_sw_phv_mem_0_t::~cap_psp_csr_cfg_sw_phv_mem_0_t() { }

cap_psp_csr_cfg_sw_phv_profiles_t::cap_psp_csr_cfg_sw_phv_profiles_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_sw_phv_profiles_t::~cap_psp_csr_cfg_sw_phv_profiles_t() { }

cap_psp_csr_cfg_qstate_map_rsp_t::cap_psp_csr_cfg_qstate_map_rsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_qstate_map_rsp_t::~cap_psp_csr_cfg_qstate_map_rsp_t() { }

cap_psp_csr_cfg_qstate_map_req_t::cap_psp_csr_cfg_qstate_map_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_qstate_map_req_t::~cap_psp_csr_cfg_qstate_map_req_t() { }

cap_psp_csr_cfg_profile_t::cap_psp_csr_cfg_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_profile_t::~cap_psp_csr_cfg_profile_t() { }

cap_psp_csr_cfg_debug_port_t::cap_psp_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_cfg_debug_port_t::~cap_psp_csr_cfg_debug_port_t() { }

cap_psp_csr_base_t::cap_psp_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_psp_csr_base_t::~cap_psp_csr_base_t() { }

cap_psp_csr_t::cap_psp_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(131072);
        set_attributes(0,get_name(), 0);
        }
cap_psp_csr_t::~cap_psp_csr_t() { }

void cap_psp_csr_dhs_lif_qstate_map_entry_t::show() {

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

void cap_psp_csr_dhs_lif_qstate_map_t::show() {

    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
}

void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sop: 0x" << int_var__sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".eop: 0x" << int_var__eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err: 0x" << int_var__err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_psp_csr_dhs_sw_phv_mem_0_t::show() {

    entry.show();
}

void cap_psp_csr_sta_ecc_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cor: 0x" << int_var__cor << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".det: 0x" << int_var__det << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs: 0x" << int_var__dhs << dec << endl)
}

void cap_psp_csr_sta_sw_phv_mem_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_psp_csr_cfg_sw_phv_mem_0_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cfg_ecc_disable_det: 0x" << int_var__cfg_ecc_disable_det << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".cfg_ecc_disable_cor: 0x" << int_var__cfg_ecc_disable_cor << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl)
}

void cap_psp_csr_cfg_sw_phv_profiles_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".start_enable: 0x" << int_var__start_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start_addr: 0x" << int_var__start_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".insertion_period_clocks: 0x" << int_var__insertion_period_clocks << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".counter_max: 0x" << int_var__counter_max << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".counter_repeat: 0x" << int_var__counter_repeat << dec << endl)
}

void cap_psp_csr_cfg_qstate_map_rsp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_enable: 0x" << int_var__qid_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype_enable: 0x" << int_var__qtype_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_enable: 0x" << int_var__addr_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".no_data_enable: 0x" << int_var__no_data_enable << dec << endl)
}

void cap_psp_csr_cfg_qstate_map_req_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".lif_override_enable: 0x" << int_var__lif_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype_override_enable: 0x" << int_var__qtype_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype: 0x" << int_var__qtype << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_override_enable: 0x" << int_var__qid_override_enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
}

void cap_psp_csr_cfg_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".ptd_npv_phv_full_enable: 0x" << int_var__ptd_npv_phv_full_enable << dec << endl)
}

void cap_psp_csr_cfg_debug_port_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl)
}

void cap_psp_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_psp_csr_t::show() {

    base.show();
    cfg_debug_port.show();
    cfg_profile.show();
    cfg_qstate_map_req.show();
    cfg_qstate_map_rsp.show();
    for(int ii = 0; ii < 8; ii++) {
        cfg_sw_phv_profiles[ii].show();
    }
    cfg_sw_phv_mem_0.show();
    sta_sw_phv_mem.show();
    cfg_ecc_disable_lif_qstate_map.show();
    sta_ecc_lif_qstate_map.show();
    dhs_sw_phv_mem_0.show();
    dhs_lif_qstate_map.show();
}

int cap_psp_csr_dhs_lif_qstate_map_entry_t::get_width() const {
    return cap_psp_csr_dhs_lif_qstate_map_entry_t::s_get_width();

}

int cap_psp_csr_dhs_lif_qstate_map_t::get_width() const {
    return cap_psp_csr_dhs_lif_qstate_map_t::s_get_width();

}

int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::get_width() const {
    return cap_psp_csr_dhs_sw_phv_mem_0_entry_t::s_get_width();

}

int cap_psp_csr_dhs_sw_phv_mem_0_t::get_width() const {
    return cap_psp_csr_dhs_sw_phv_mem_0_t::s_get_width();

}

int cap_psp_csr_sta_ecc_lif_qstate_map_t::get_width() const {
    return cap_psp_csr_sta_ecc_lif_qstate_map_t::s_get_width();

}

int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::get_width() const {
    return cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width();

}

int cap_psp_csr_sta_sw_phv_mem_t::get_width() const {
    return cap_psp_csr_sta_sw_phv_mem_t::s_get_width();

}

int cap_psp_csr_cfg_sw_phv_mem_0_t::get_width() const {
    return cap_psp_csr_cfg_sw_phv_mem_0_t::s_get_width();

}

int cap_psp_csr_cfg_sw_phv_profiles_t::get_width() const {
    return cap_psp_csr_cfg_sw_phv_profiles_t::s_get_width();

}

int cap_psp_csr_cfg_qstate_map_rsp_t::get_width() const {
    return cap_psp_csr_cfg_qstate_map_rsp_t::s_get_width();

}

int cap_psp_csr_cfg_qstate_map_req_t::get_width() const {
    return cap_psp_csr_cfg_qstate_map_req_t::s_get_width();

}

int cap_psp_csr_cfg_profile_t::get_width() const {
    return cap_psp_csr_cfg_profile_t::s_get_width();

}

int cap_psp_csr_cfg_debug_port_t::get_width() const {
    return cap_psp_csr_cfg_debug_port_t::s_get_width();

}

int cap_psp_csr_base_t::get_width() const {
    return cap_psp_csr_base_t::s_get_width();

}

int cap_psp_csr_t::get_width() const {
    return cap_psp_csr_t::s_get_width();

}

int cap_psp_csr_dhs_lif_qstate_map_entry_t::s_get_width() {
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

int cap_psp_csr_dhs_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += (cap_psp_csr_dhs_lif_qstate_map_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    _count += 1; // sop
    _count += 1; // eop
    _count += 1; // err
    _count += 20; // ecc
    return _count;
}

int cap_psp_csr_dhs_sw_phv_mem_0_t::s_get_width() {
    int _count = 0;

    _count += cap_psp_csr_dhs_sw_phv_mem_0_entry_t::s_get_width(); // entry
    return _count;
}

int cap_psp_csr_sta_ecc_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 11; // addr
    return _count;
}

int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // cor
    _count += 1; // det
    _count += 1; // dhs
    return _count;
}

int cap_psp_csr_sta_sw_phv_mem_t::s_get_width() {
    int _count = 0;

    _count += 11; // syndrome
    _count += 5; // addr
    return _count;
}

int cap_psp_csr_cfg_sw_phv_mem_0_t::s_get_width() {
    int _count = 0;

    _count += 1; // cfg_ecc_disable_det
    _count += 1; // cfg_ecc_disable_cor
    _count += 1; // dhs_eccbypass
    return _count;
}

int cap_psp_csr_cfg_sw_phv_profiles_t::s_get_width() {
    int _count = 0;

    _count += 1; // start_enable
    _count += 3; // start_addr
    _count += 42; // insertion_period_clocks
    _count += 16; // counter_max
    _count += 1; // counter_repeat
    return _count;
}

int cap_psp_csr_cfg_qstate_map_rsp_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // qid_enable
    _count += 1; // qtype_enable
    _count += 1; // addr_enable
    _count += 1; // no_data_enable
    return _count;
}

int cap_psp_csr_cfg_qstate_map_req_t::s_get_width() {
    int _count = 0;

    _count += 1; // lif_override_enable
    _count += 11; // lif
    _count += 1; // qtype_override_enable
    _count += 3; // qtype
    _count += 1; // qid_override_enable
    _count += 24; // qid
    return _count;
}

int cap_psp_csr_cfg_profile_t::s_get_width() {
    int _count = 0;

    _count += 1; // ptd_npv_phv_full_enable
    return _count;
}

int cap_psp_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // select
    return _count;
}

int cap_psp_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_psp_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_psp_csr_base_t::s_get_width(); // base
    _count += cap_psp_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_psp_csr_cfg_profile_t::s_get_width(); // cfg_profile
    _count += cap_psp_csr_cfg_qstate_map_req_t::s_get_width(); // cfg_qstate_map_req
    _count += cap_psp_csr_cfg_qstate_map_rsp_t::s_get_width(); // cfg_qstate_map_rsp
    _count += (cap_psp_csr_cfg_sw_phv_profiles_t::s_get_width() * 8); // cfg_sw_phv_profiles
    _count += cap_psp_csr_cfg_sw_phv_mem_0_t::s_get_width(); // cfg_sw_phv_mem_0
    _count += cap_psp_csr_sta_sw_phv_mem_t::s_get_width(); // sta_sw_phv_mem
    _count += cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width(); // cfg_ecc_disable_lif_qstate_map
    _count += cap_psp_csr_sta_ecc_lif_qstate_map_t::s_get_width(); // sta_ecc_lif_qstate_map
    _count += cap_psp_csr_dhs_sw_phv_mem_0_t::s_get_width(); // dhs_sw_phv_mem_0
    _count += cap_psp_csr_dhs_lif_qstate_map_t::s_get_width(); // dhs_lif_qstate_map
    return _count;
}

void cap_psp_csr_dhs_lif_qstate_map_entry_t::all(const cpp_int & _val) {
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

void cap_psp_csr_dhs_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 512 ).convert_to< data_cpp_int_t >()  ;
    _count += 512;
    // sop
    int_var__sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< sop_cpp_int_t >()  ;
    _count += 1;
    // eop
    int_var__eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< eop_cpp_int_t >()  ;
    _count += 1;
    // err
    int_var__err = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_cpp_int_t >()  ;
    _count += 1;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 20 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 20;
}

void cap_psp_csr_dhs_sw_phv_mem_0_t::all(const cpp_int & _val) {
    int _count = 0;

    entry.all( hlp.get_slc(_val, _count, _count -1 + entry.get_width() )); // entry
    _count += entry.get_width();
}

void cap_psp_csr_sta_ecc_lif_qstate_map_t::all(const cpp_int & _val) {
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

void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::all(const cpp_int & _val) {
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

void cap_psp_csr_sta_sw_phv_mem_t::all(const cpp_int & _val) {
    int _count = 0;

    // syndrome
    int_var__syndrome = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< syndrome_cpp_int_t >()  ;
    _count += 11;
    // addr
    int_var__addr = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< addr_cpp_int_t >()  ;
    _count += 5;
}

void cap_psp_csr_cfg_sw_phv_mem_0_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_ecc_disable_det
    int_var__cfg_ecc_disable_det = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< cfg_ecc_disable_det_cpp_int_t >()  ;
    _count += 1;
    // cfg_ecc_disable_cor
    int_var__cfg_ecc_disable_cor = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< cfg_ecc_disable_cor_cpp_int_t >()  ;
    _count += 1;
    // dhs_eccbypass
    int_var__dhs_eccbypass = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _count += 1;
}

void cap_psp_csr_cfg_sw_phv_profiles_t::all(const cpp_int & _val) {
    int _count = 0;

    // start_enable
    int_var__start_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< start_enable_cpp_int_t >()  ;
    _count += 1;
    // start_addr
    int_var__start_addr = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< start_addr_cpp_int_t >()  ;
    _count += 3;
    // insertion_period_clocks
    int_var__insertion_period_clocks = hlp.get_slc(_val, _count, _count -1 + 42 ).convert_to< insertion_period_clocks_cpp_int_t >()  ;
    _count += 42;
    // counter_max
    int_var__counter_max = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< counter_max_cpp_int_t >()  ;
    _count += 16;
    // counter_repeat
    int_var__counter_repeat = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< counter_repeat_cpp_int_t >()  ;
    _count += 1;
}

void cap_psp_csr_cfg_qstate_map_rsp_t::all(const cpp_int & _val) {
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
    // no_data_enable
    int_var__no_data_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< no_data_enable_cpp_int_t >()  ;
    _count += 1;
}

void cap_psp_csr_cfg_qstate_map_req_t::all(const cpp_int & _val) {
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

void cap_psp_csr_cfg_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // ptd_npv_phv_full_enable
    int_var__ptd_npv_phv_full_enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< ptd_npv_phv_full_enable_cpp_int_t >()  ;
    _count += 1;
}

void cap_psp_csr_cfg_debug_port_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // select
    int_var__select = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< select_cpp_int_t >()  ;
    _count += 1;
}

void cap_psp_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_psp_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_debug_port.all( hlp.get_slc(_val, _count, _count -1 + cfg_debug_port.get_width() )); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    cfg_profile.all( hlp.get_slc(_val, _count, _count -1 + cfg_profile.get_width() )); // cfg_profile
    _count += cfg_profile.get_width();
    cfg_qstate_map_req.all( hlp.get_slc(_val, _count, _count -1 + cfg_qstate_map_req.get_width() )); // cfg_qstate_map_req
    _count += cfg_qstate_map_req.get_width();
    cfg_qstate_map_rsp.all( hlp.get_slc(_val, _count, _count -1 + cfg_qstate_map_rsp.get_width() )); // cfg_qstate_map_rsp
    _count += cfg_qstate_map_rsp.get_width();
    // cfg_sw_phv_profiles
    for(int ii = 0; ii < 8; ii++) {
        cfg_sw_phv_profiles[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_sw_phv_profiles[ii].get_width()));
        _count += cfg_sw_phv_profiles[ii].get_width();
    }
    cfg_sw_phv_mem_0.all( hlp.get_slc(_val, _count, _count -1 + cfg_sw_phv_mem_0.get_width() )); // cfg_sw_phv_mem_0
    _count += cfg_sw_phv_mem_0.get_width();
    sta_sw_phv_mem.all( hlp.get_slc(_val, _count, _count -1 + sta_sw_phv_mem.get_width() )); // sta_sw_phv_mem
    _count += sta_sw_phv_mem.get_width();
    cfg_ecc_disable_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() )); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    sta_ecc_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + sta_ecc_lif_qstate_map.get_width() )); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    dhs_sw_phv_mem_0.all( hlp.get_slc(_val, _count, _count -1 + dhs_sw_phv_mem_0.get_width() )); // dhs_sw_phv_mem_0
    _count += dhs_sw_phv_mem_0.get_width();
    dhs_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + dhs_lif_qstate_map.get_width() )); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::all() const {
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

cpp_int cap_psp_csr_dhs_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 512 );
    _count += 512;
    // sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sop) , _count, _count -1 + 1 );
    _count += 1;
    // eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__eop) , _count, _count -1 + 1 );
    _count += 1;
    // err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err) , _count, _count -1 + 1 );
    _count += 1;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 20 );
    _count += 20;
    return ret_val;
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, entry.all() , _count, _count -1 + entry.get_width() ); // entry
    _count += entry.get_width();
    return ret_val;
}

cpp_int cap_psp_csr_sta_ecc_lif_qstate_map_t::all() const {
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

cpp_int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::all() const {
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

cpp_int cap_psp_csr_sta_sw_phv_mem_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // syndrome
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__syndrome) , _count, _count -1 + 11 );
    _count += 11;
    // addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_psp_csr_cfg_sw_phv_mem_0_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_ecc_disable_det
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cfg_ecc_disable_det) , _count, _count -1 + 1 );
    _count += 1;
    // cfg_ecc_disable_cor
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cfg_ecc_disable_cor) , _count, _count -1 + 1 );
    _count += 1;
    // dhs_eccbypass
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dhs_eccbypass) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // start_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_enable) , _count, _count -1 + 1 );
    _count += 1;
    // start_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_addr) , _count, _count -1 + 3 );
    _count += 3;
    // insertion_period_clocks
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__insertion_period_clocks) , _count, _count -1 + 42 );
    _count += 42;
    // counter_max
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__counter_max) , _count, _count -1 + 16 );
    _count += 16;
    // counter_repeat
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__counter_repeat) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::all() const {
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
    // no_data_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__no_data_enable) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::all() const {
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

cpp_int cap_psp_csr_cfg_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // ptd_npv_phv_full_enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ptd_npv_phv_full_enable) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_psp_csr_cfg_debug_port_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // select
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__select) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_psp_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_psp_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_debug_port.all() , _count, _count -1 + cfg_debug_port.get_width() ); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_profile.all() , _count, _count -1 + cfg_profile.get_width() ); // cfg_profile
    _count += cfg_profile.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_qstate_map_req.all() , _count, _count -1 + cfg_qstate_map_req.get_width() ); // cfg_qstate_map_req
    _count += cfg_qstate_map_req.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_qstate_map_rsp.all() , _count, _count -1 + cfg_qstate_map_rsp.get_width() ); // cfg_qstate_map_rsp
    _count += cfg_qstate_map_rsp.get_width();
    // cfg_sw_phv_profiles
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_sw_phv_profiles[ii].all() , _count, _count -1 + cfg_sw_phv_profiles[ii].get_width() );
        _count += cfg_sw_phv_profiles[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, cfg_sw_phv_mem_0.all() , _count, _count -1 + cfg_sw_phv_mem_0.get_width() ); // cfg_sw_phv_mem_0
    _count += cfg_sw_phv_mem_0.get_width();
    ret_val = hlp.set_slc(ret_val, sta_sw_phv_mem.all() , _count, _count -1 + sta_sw_phv_mem.get_width() ); // sta_sw_phv_mem
    _count += sta_sw_phv_mem.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_ecc_disable_lif_qstate_map.all() , _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() ); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, sta_ecc_lif_qstate_map.all() , _count, _count -1 + sta_ecc_lif_qstate_map.get_width() ); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_sw_phv_mem_0.all() , _count, _count -1 + dhs_sw_phv_mem_0.get_width() ); // dhs_sw_phv_mem_0
    _count += dhs_sw_phv_mem_0.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_lif_qstate_map.all() , _count, _count -1 + dhs_lif_qstate_map.get_width() ); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
    return ret_val;
}

void cap_psp_csr_dhs_lif_qstate_map_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("vld", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::vld);
            register_get_func("vld", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qstate_base", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::qstate_base);
            register_get_func("qstate_base", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::qstate_base);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length0", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length0);
            register_get_func("length0", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size0", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size0);
            register_get_func("size0", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length1", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length1);
            register_get_func("length1", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size1", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size1);
            register_get_func("size1", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length2", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length2);
            register_get_func("length2", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size2", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size2);
            register_get_func("size2", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length3", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length3);
            register_get_func("length3", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size3", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size3);
            register_get_func("size3", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length4", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length4);
            register_get_func("length4", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size4", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size4);
            register_get_func("size4", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length5", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length5);
            register_get_func("length5", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size5", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size5);
            register_get_func("size5", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length6", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length6);
            register_get_func("length6", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size6", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size6);
            register_get_func("size6", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length7", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length7);
            register_get_func("length7", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::length7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size7", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size7);
            register_get_func("size7", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::size7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("spare", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::spare);
            register_get_func("spare", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::spare);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_lif_qstate_map_entry_t::ecc);
        }
        #endif
    
}

void cap_psp_csr_dhs_lif_qstate_map_t::init() {

    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sop", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::sop);
            register_get_func("sop", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("eop", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::eop);
            register_get_func("eop", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::err);
            register_get_func("err", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_psp_csr_dhs_sw_phv_mem_0_entry_t::ecc);
        }
        #endif
    
}

void cap_psp_csr_dhs_sw_phv_mem_0_t::init() {

    entry.set_attributes(this,"entry", 0x0 );
}

void cap_psp_csr_sta_ecc_lif_qstate_map_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("uncorrectable", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
            register_get_func("uncorrectable", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("correctable", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::correctable);
            register_get_func("correctable", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::correctable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::syndrome);
            register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::syndrome);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::addr);
            register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_ecc_lif_qstate_map_t::addr);
        }
        #endif
    
}

void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cor", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
            register_get_func("cor", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("det", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::det);
            register_get_func("det", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::det);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dhs", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
            register_get_func("dhs", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
        }
        #endif
    
}

void cap_psp_csr_sta_sw_phv_mem_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_sw_phv_mem_t::syndrome);
            register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_sw_phv_mem_t::syndrome);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_psp_csr_sta_sw_phv_mem_t::addr);
            register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_psp_csr_sta_sw_phv_mem_t::addr);
        }
        #endif
    
}

void cap_psp_csr_cfg_sw_phv_mem_0_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cfg_ecc_disable_det", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_det);
            register_get_func("cfg_ecc_disable_det", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_det);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cfg_ecc_disable_cor", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_cor);
            register_get_func("cfg_ecc_disable_cor", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_cor);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dhs_eccbypass", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::dhs_eccbypass);
            register_get_func("dhs_eccbypass", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_mem_0_t::dhs_eccbypass);
        }
        #endif
    
}

void cap_psp_csr_cfg_sw_phv_profiles_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::start_enable);
            register_get_func("start_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::start_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_addr", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::start_addr);
            register_get_func("start_addr", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::start_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("insertion_period_clocks", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::insertion_period_clocks);
            register_get_func("insertion_period_clocks", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::insertion_period_clocks);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("counter_max", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::counter_max);
            register_get_func("counter_max", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::counter_max);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("counter_repeat", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::counter_repeat);
            register_get_func("counter_repeat", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_sw_phv_profiles_t::counter_repeat);
        }
        #endif
    
}

void cap_psp_csr_cfg_qstate_map_rsp_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::qid_enable);
            register_get_func("qid_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::qid_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qtype_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::qtype_enable);
            register_get_func("qtype_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::qtype_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::addr_enable);
            register_get_func("addr_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::addr_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("no_data_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::no_data_enable);
            register_get_func("no_data_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_rsp_t::no_data_enable);
        }
        #endif
    
    set_reset_val(cpp_int("0x8"));
    all(get_reset_val());
}

void cap_psp_csr_cfg_qstate_map_req_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lif_override_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::lif_override_enable);
            register_get_func("lif_override_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::lif_override_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lif", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::lif);
            register_get_func("lif", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::lif);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qtype_override_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qtype_override_enable);
            register_get_func("qtype_override_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qtype_override_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qtype", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qtype);
            register_get_func("qtype", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qtype);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid_override_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qid_override_enable);
            register_get_func("qid_override_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qid_override_enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qid);
            register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_qstate_map_req_t::qid);
        }
        #endif
    
}

void cap_psp_csr_cfg_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ptd_npv_phv_full_enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_profile_t::ptd_npv_phv_full_enable);
            register_get_func("ptd_npv_phv_full_enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_profile_t::ptd_npv_phv_full_enable);
        }
        #endif
    
}

void cap_psp_csr_cfg_debug_port_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_debug_port_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_debug_port_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("select", (cap_csr_base::set_function_type_t)&cap_psp_csr_cfg_debug_port_t::select);
            register_get_func("select", (cap_csr_base::get_function_type_t)&cap_psp_csr_cfg_debug_port_t::select);
        }
        #endif
    
}

void cap_psp_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_psp_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_psp_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_psp_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x4 );
    cfg_profile.set_attributes(this,"cfg_profile", 0x8 );
    cfg_qstate_map_req.set_attributes(this,"cfg_qstate_map_req", 0x10 );
    cfg_qstate_map_rsp.set_attributes(this,"cfg_qstate_map_rsp", 0x18 );
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cfg_sw_phv_profiles[ii].set_field_init_done(true);
        cfg_sw_phv_profiles[ii].set_attributes(this,"cfg_sw_phv_profiles["+to_string(ii)+"]",  0x40 + (cfg_sw_phv_profiles[ii].get_byte_size()*ii));
    }
    cfg_sw_phv_mem_0.set_attributes(this,"cfg_sw_phv_mem_0", 0x2000 );
    sta_sw_phv_mem.set_attributes(this,"sta_sw_phv_mem", 0x2004 );
    cfg_ecc_disable_lif_qstate_map.set_attributes(this,"cfg_ecc_disable_lif_qstate_map", 0x10000 );
    sta_ecc_lif_qstate_map.set_attributes(this,"sta_ecc_lif_qstate_map", 0x10004 );
    dhs_sw_phv_mem_0.set_attributes(this,"dhs_sw_phv_mem_0", 0x1000 );
    dhs_lif_qstate_map.set_attributes(this,"dhs_lif_qstate_map", 0x8000 );
}

void cap_psp_csr_dhs_lif_qstate_map_entry_t::vld(const cpp_int & _val) { 
    // vld
    int_var__vld = _val.convert_to< vld_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::vld() const {
    return int_var__vld.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::qstate_base(const cpp_int & _val) { 
    // qstate_base
    int_var__qstate_base = _val.convert_to< qstate_base_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::qstate_base() const {
    return int_var__qstate_base.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length0(const cpp_int & _val) { 
    // length0
    int_var__length0 = _val.convert_to< length0_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length0() const {
    return int_var__length0.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size0(const cpp_int & _val) { 
    // size0
    int_var__size0 = _val.convert_to< size0_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size0() const {
    return int_var__size0.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length1(const cpp_int & _val) { 
    // length1
    int_var__length1 = _val.convert_to< length1_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length1() const {
    return int_var__length1.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size1(const cpp_int & _val) { 
    // size1
    int_var__size1 = _val.convert_to< size1_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size1() const {
    return int_var__size1.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length2(const cpp_int & _val) { 
    // length2
    int_var__length2 = _val.convert_to< length2_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length2() const {
    return int_var__length2.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size2(const cpp_int & _val) { 
    // size2
    int_var__size2 = _val.convert_to< size2_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size2() const {
    return int_var__size2.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length3(const cpp_int & _val) { 
    // length3
    int_var__length3 = _val.convert_to< length3_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length3() const {
    return int_var__length3.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size3(const cpp_int & _val) { 
    // size3
    int_var__size3 = _val.convert_to< size3_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size3() const {
    return int_var__size3.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length4(const cpp_int & _val) { 
    // length4
    int_var__length4 = _val.convert_to< length4_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length4() const {
    return int_var__length4.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size4(const cpp_int & _val) { 
    // size4
    int_var__size4 = _val.convert_to< size4_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size4() const {
    return int_var__size4.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length5(const cpp_int & _val) { 
    // length5
    int_var__length5 = _val.convert_to< length5_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length5() const {
    return int_var__length5.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size5(const cpp_int & _val) { 
    // size5
    int_var__size5 = _val.convert_to< size5_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size5() const {
    return int_var__size5.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length6(const cpp_int & _val) { 
    // length6
    int_var__length6 = _val.convert_to< length6_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length6() const {
    return int_var__length6.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size6(const cpp_int & _val) { 
    // size6
    int_var__size6 = _val.convert_to< size6_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size6() const {
    return int_var__size6.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::length7(const cpp_int & _val) { 
    // length7
    int_var__length7 = _val.convert_to< length7_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::length7() const {
    return int_var__length7.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::size7(const cpp_int & _val) { 
    // size7
    int_var__size7 = _val.convert_to< size7_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::size7() const {
    return int_var__size7.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::spare() const {
    return int_var__spare.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_lif_qstate_map_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_lif_qstate_map_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::sop(const cpp_int & _val) { 
    // sop
    int_var__sop = _val.convert_to< sop_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::sop() const {
    return int_var__sop.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::eop(const cpp_int & _val) { 
    // eop
    int_var__eop = _val.convert_to< eop_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::eop() const {
    return int_var__eop.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::err(const cpp_int & _val) { 
    // err
    int_var__err = _val.convert_to< err_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::err() const {
    return int_var__err.convert_to< cpp_int >();
}
    
void cap_psp_csr_dhs_sw_phv_mem_0_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_psp_csr_dhs_sw_phv_mem_0_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_ecc_lif_qstate_map_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_ecc_lif_qstate_map_t::uncorrectable() const {
    return int_var__uncorrectable.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_ecc_lif_qstate_map_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_ecc_lif_qstate_map_t::correctable() const {
    return int_var__correctable.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_ecc_lif_qstate_map_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_ecc_lif_qstate_map_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_ecc_lif_qstate_map_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_ecc_lif_qstate_map_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::cor(const cpp_int & _val) { 
    // cor
    int_var__cor = _val.convert_to< cor_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::cor() const {
    return int_var__cor.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::det(const cpp_int & _val) { 
    // det
    int_var__det = _val.convert_to< det_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::det() const {
    return int_var__det.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::dhs(const cpp_int & _val) { 
    // dhs
    int_var__dhs = _val.convert_to< dhs_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t::dhs() const {
    return int_var__dhs.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_sw_phv_mem_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_sw_phv_mem_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_psp_csr_sta_sw_phv_mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_psp_csr_sta_sw_phv_mem_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_det(const cpp_int & _val) { 
    // cfg_ecc_disable_det
    int_var__cfg_ecc_disable_det = _val.convert_to< cfg_ecc_disable_det_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_det() const {
    return int_var__cfg_ecc_disable_det.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_cor(const cpp_int & _val) { 
    // cfg_ecc_disable_cor
    int_var__cfg_ecc_disable_cor = _val.convert_to< cfg_ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_mem_0_t::cfg_ecc_disable_cor() const {
    return int_var__cfg_ecc_disable_cor.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_mem_0_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_mem_0_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_profiles_t::start_enable(const cpp_int & _val) { 
    // start_enable
    int_var__start_enable = _val.convert_to< start_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::start_enable() const {
    return int_var__start_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_profiles_t::start_addr(const cpp_int & _val) { 
    // start_addr
    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::start_addr() const {
    return int_var__start_addr.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_profiles_t::insertion_period_clocks(const cpp_int & _val) { 
    // insertion_period_clocks
    int_var__insertion_period_clocks = _val.convert_to< insertion_period_clocks_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::insertion_period_clocks() const {
    return int_var__insertion_period_clocks.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_profiles_t::counter_max(const cpp_int & _val) { 
    // counter_max
    int_var__counter_max = _val.convert_to< counter_max_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::counter_max() const {
    return int_var__counter_max.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_sw_phv_profiles_t::counter_repeat(const cpp_int & _val) { 
    // counter_repeat
    int_var__counter_repeat = _val.convert_to< counter_repeat_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_sw_phv_profiles_t::counter_repeat() const {
    return int_var__counter_repeat.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_rsp_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_rsp_t::qid_enable(const cpp_int & _val) { 
    // qid_enable
    int_var__qid_enable = _val.convert_to< qid_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::qid_enable() const {
    return int_var__qid_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_rsp_t::qtype_enable(const cpp_int & _val) { 
    // qtype_enable
    int_var__qtype_enable = _val.convert_to< qtype_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::qtype_enable() const {
    return int_var__qtype_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_rsp_t::addr_enable(const cpp_int & _val) { 
    // addr_enable
    int_var__addr_enable = _val.convert_to< addr_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::addr_enable() const {
    return int_var__addr_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_rsp_t::no_data_enable(const cpp_int & _val) { 
    // no_data_enable
    int_var__no_data_enable = _val.convert_to< no_data_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_rsp_t::no_data_enable() const {
    return int_var__no_data_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::lif_override_enable(const cpp_int & _val) { 
    // lif_override_enable
    int_var__lif_override_enable = _val.convert_to< lif_override_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::lif_override_enable() const {
    return int_var__lif_override_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::lif() const {
    return int_var__lif.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::qtype_override_enable(const cpp_int & _val) { 
    // qtype_override_enable
    int_var__qtype_override_enable = _val.convert_to< qtype_override_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::qtype_override_enable() const {
    return int_var__qtype_override_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::qtype(const cpp_int & _val) { 
    // qtype
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::qtype() const {
    return int_var__qtype.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::qid_override_enable(const cpp_int & _val) { 
    // qid_override_enable
    int_var__qid_override_enable = _val.convert_to< qid_override_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::qid_override_enable() const {
    return int_var__qid_override_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_qstate_map_req_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_qstate_map_req_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_profile_t::ptd_npv_phv_full_enable(const cpp_int & _val) { 
    // ptd_npv_phv_full_enable
    int_var__ptd_npv_phv_full_enable = _val.convert_to< ptd_npv_phv_full_enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_profile_t::ptd_npv_phv_full_enable() const {
    return int_var__ptd_npv_phv_full_enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_debug_port_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_psp_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_psp_csr_cfg_debug_port_t::select() const {
    return int_var__select.convert_to< cpp_int >();
}
    
void cap_psp_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_psp_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    