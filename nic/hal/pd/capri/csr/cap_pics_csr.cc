
#include "cap_pics_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pics_csr_dhs_table_data_entry_t::cap_pics_csr_dhs_table_data_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_table_data_entry_t::~cap_pics_csr_dhs_table_data_entry_t() { }

cap_pics_csr_dhs_table_data_t::cap_pics_csr_dhs_table_data_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_table_data_t::~cap_pics_csr_dhs_table_data_t() { }

cap_pics_csr_dhs_table_address_entry_t::cap_pics_csr_dhs_table_address_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_table_address_entry_t::~cap_pics_csr_dhs_table_address_entry_t() { }

cap_pics_csr_dhs_table_address_t::cap_pics_csr_dhs_table_address_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_table_address_t::~cap_pics_csr_dhs_table_address_t() { }

cap_pics_csr_dhs_sram_entry_t::cap_pics_csr_dhs_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_sram_entry_t::~cap_pics_csr_dhs_sram_entry_t() { }

cap_pics_csr_dhs_sram_t::cap_pics_csr_dhs_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pics_csr_dhs_sram_t::~cap_pics_csr_dhs_sram_t() { }

cap_pics_csr_cfg_meter_sram_t::cap_pics_csr_cfg_meter_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_meter_sram_t::~cap_pics_csr_cfg_meter_sram_t() { }

cap_pics_csr_cfg_meter_access_t::cap_pics_csr_cfg_meter_access_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_meter_access_t::~cap_pics_csr_cfg_meter_access_t() { }

cap_pics_csr_cfg_meter_timer_t::cap_pics_csr_cfg_meter_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_meter_timer_t::~cap_pics_csr_cfg_meter_timer_t() { }

cap_pics_csr_cfg_update_profile_t::cap_pics_csr_cfg_update_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_update_profile_t::~cap_pics_csr_cfg_update_profile_t() { }

cap_pics_csr_cfg_table_profile_t::cap_pics_csr_cfg_table_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_table_profile_t::~cap_pics_csr_cfg_table_profile_t() { }

cap_pics_csr_cfg_sram_t::cap_pics_csr_cfg_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_cfg_sram_t::~cap_pics_csr_cfg_sram_t() { }

cap_pics_csr_base_t::cap_pics_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pics_csr_base_t::~cap_pics_csr_base_t() { }

cap_pics_csr_t::cap_pics_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(8388608);
        set_attributes(0,get_name(), 0);
        }
cap_pics_csr_t::~cap_pics_csr_t() { }

void cap_pics_csr_dhs_table_data_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_pics_csr_dhs_table_data_t::show() {

    entry.show();
}

void cap_pics_csr_dhs_table_address_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".address: 0x" << int_var__address << dec << endl)
}

void cap_pics_csr_dhs_table_address_t::show() {

    entry.show();
}

void cap_pics_csr_dhs_sram_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_pics_csr_dhs_sram_t::show() {

    for(int ii = 0; ii < 40960; ii++) {
        entry[ii].show();
    }
}

void cap_pics_csr_cfg_meter_sram_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start: 0x" << int_var__start << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end: 0x" << int_var__end << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".scale: 0x" << int_var__scale << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".opcode: 0x" << int_var__opcode << dec << endl)
}

void cap_pics_csr_cfg_meter_access_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".max_cycle: 0x" << int_var__max_cycle << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".start_addr: 0x" << int_var__start_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_addr: 0x" << int_var__end_addr << dec << endl)
}

void cap_pics_csr_cfg_meter_timer_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".cycle: 0x" << int_var__cycle << dec << endl)
}

void cap_pics_csr_cfg_update_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".oprd1_sel: 0x" << int_var__oprd1_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd2_sel: 0x" << int_var__oprd2_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd3_sel: 0x" << int_var__oprd3_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd1_wid: 0x" << int_var__oprd1_wid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd2_wid: 0x" << int_var__oprd2_wid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd3_wid: 0x" << int_var__oprd3_wid << dec << endl)
}

void cap_pics_csr_cfg_table_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".start_addr: 0x" << int_var__start_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_addr: 0x" << int_var__end_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".width: 0x" << int_var__width << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hash: 0x" << int_var__hash << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".opcode: 0x" << int_var__opcode << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".log2bkts: 0x" << int_var__log2bkts << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".axishift: 0x" << int_var__axishift << dec << endl)
}

void cap_pics_csr_cfg_sram_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl)
}

void cap_pics_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_pics_csr_t::show() {

    base.show();
    cfg_sram.show();
    for(int ii = 0; ii < 128; ii++) {
        cfg_table_profile[ii].show();
    }
    for(int ii = 0; ii < 16; ii++) {
        cfg_update_profile[ii].show();
    }
    cfg_meter_timer.show();
    cfg_meter_access.show();
    for(int ii = 0; ii < 40; ii++) {
        cfg_meter_sram[ii].show();
    }
    dhs_sram.show();
    dhs_table_address.show();
    dhs_table_data.show();
}

int cap_pics_csr_dhs_table_data_entry_t::get_width() const {
    return cap_pics_csr_dhs_table_data_entry_t::s_get_width();

}

int cap_pics_csr_dhs_table_data_t::get_width() const {
    return cap_pics_csr_dhs_table_data_t::s_get_width();

}

int cap_pics_csr_dhs_table_address_entry_t::get_width() const {
    return cap_pics_csr_dhs_table_address_entry_t::s_get_width();

}

int cap_pics_csr_dhs_table_address_t::get_width() const {
    return cap_pics_csr_dhs_table_address_t::s_get_width();

}

int cap_pics_csr_dhs_sram_entry_t::get_width() const {
    return cap_pics_csr_dhs_sram_entry_t::s_get_width();

}

int cap_pics_csr_dhs_sram_t::get_width() const {
    return cap_pics_csr_dhs_sram_t::s_get_width();

}

int cap_pics_csr_cfg_meter_sram_t::get_width() const {
    return cap_pics_csr_cfg_meter_sram_t::s_get_width();

}

int cap_pics_csr_cfg_meter_access_t::get_width() const {
    return cap_pics_csr_cfg_meter_access_t::s_get_width();

}

int cap_pics_csr_cfg_meter_timer_t::get_width() const {
    return cap_pics_csr_cfg_meter_timer_t::s_get_width();

}

int cap_pics_csr_cfg_update_profile_t::get_width() const {
    return cap_pics_csr_cfg_update_profile_t::s_get_width();

}

int cap_pics_csr_cfg_table_profile_t::get_width() const {
    return cap_pics_csr_cfg_table_profile_t::s_get_width();

}

int cap_pics_csr_cfg_sram_t::get_width() const {
    return cap_pics_csr_cfg_sram_t::s_get_width();

}

int cap_pics_csr_base_t::get_width() const {
    return cap_pics_csr_base_t::s_get_width();

}

int cap_pics_csr_t::get_width() const {
    return cap_pics_csr_t::s_get_width();

}

int cap_pics_csr_dhs_table_data_entry_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    return _count;
}

int cap_pics_csr_dhs_table_data_t::s_get_width() {
    int _count = 0;

    _count += cap_pics_csr_dhs_table_data_entry_t::s_get_width(); // entry
    return _count;
}

int cap_pics_csr_dhs_table_address_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // address
    return _count;
}

int cap_pics_csr_dhs_table_address_t::s_get_width() {
    int _count = 0;

    _count += cap_pics_csr_dhs_table_address_entry_t::s_get_width(); // entry
    return _count;
}

int cap_pics_csr_dhs_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 128; // data
    _count += 9; // ecc
    return _count;
}

int cap_pics_csr_dhs_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_pics_csr_dhs_sram_entry_t::s_get_width() * 40960); // entry
    return _count;
}

int cap_pics_csr_cfg_meter_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 12; // start
    _count += 12; // end
    _count += 16; // scale
    _count += 9; // opcode
    return _count;
}

int cap_pics_csr_cfg_meter_access_t::s_get_width() {
    int _count = 0;

    _count += 8; // max_cycle
    _count += 12; // start_addr
    _count += 12; // end_addr
    return _count;
}

int cap_pics_csr_cfg_meter_timer_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 32; // cycle
    return _count;
}

int cap_pics_csr_cfg_update_profile_t::s_get_width() {
    int _count = 0;

    _count += 6; // oprd1_sel
    _count += 6; // oprd2_sel
    _count += 6; // oprd3_sel
    _count += 5; // oprd1_wid
    _count += 5; // oprd2_wid
    _count += 5; // oprd3_wid
    return _count;
}

int cap_pics_csr_cfg_table_profile_t::s_get_width() {
    int _count = 0;

    _count += 20; // start_addr
    _count += 20; // end_addr
    _count += 6; // width
    _count += 1; // hash
    _count += 9; // opcode
    _count += 3; // log2bkts
    _count += 5; // axishift
    return _count;
}

int cap_pics_csr_cfg_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_cor
    _count += 1; // ecc_disable_det
    return _count;
}

int cap_pics_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pics_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pics_csr_base_t::s_get_width(); // base
    _count += cap_pics_csr_cfg_sram_t::s_get_width(); // cfg_sram
    _count += (cap_pics_csr_cfg_table_profile_t::s_get_width() * 128); // cfg_table_profile
    _count += (cap_pics_csr_cfg_update_profile_t::s_get_width() * 16); // cfg_update_profile
    _count += cap_pics_csr_cfg_meter_timer_t::s_get_width(); // cfg_meter_timer
    _count += cap_pics_csr_cfg_meter_access_t::s_get_width(); // cfg_meter_access
    _count += (cap_pics_csr_cfg_meter_sram_t::s_get_width() * 40); // cfg_meter_sram
    _count += cap_pics_csr_dhs_sram_t::s_get_width(); // dhs_sram
    _count += cap_pics_csr_dhs_table_address_t::s_get_width(); // dhs_table_address
    _count += cap_pics_csr_dhs_table_data_t::s_get_width(); // dhs_table_data
    return _count;
}

void cap_pics_csr_dhs_table_data_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 512 ).convert_to< data_cpp_int_t >()  ;
    _count += 512;
}

void cap_pics_csr_dhs_table_data_t::all(const cpp_int & _val) {
    int _count = 0;

    entry.all( hlp.get_slc(_val, _count, _count -1 + entry.get_width() )); // entry
    _count += entry.get_width();
}

void cap_pics_csr_dhs_table_address_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // address
    int_var__address = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< address_cpp_int_t >()  ;
    _count += 64;
}

void cap_pics_csr_dhs_table_address_t::all(const cpp_int & _val) {
    int _count = 0;

    entry.all( hlp.get_slc(_val, _count, _count -1 + entry.get_width() )); // entry
    _count += entry.get_width();
}

void cap_pics_csr_dhs_sram_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< data_cpp_int_t >()  ;
    _count += 128;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 9;
}

void cap_pics_csr_dhs_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 40960; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_pics_csr_cfg_meter_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // start
    int_var__start = hlp.get_slc(_val, _count, _count -1 + 12 ).convert_to< start_cpp_int_t >()  ;
    _count += 12;
    // end
    int_var__end = hlp.get_slc(_val, _count, _count -1 + 12 ).convert_to< end_cpp_int_t >()  ;
    _count += 12;
    // scale
    int_var__scale = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< scale_cpp_int_t >()  ;
    _count += 16;
    // opcode
    int_var__opcode = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< opcode_cpp_int_t >()  ;
    _count += 9;
}

void cap_pics_csr_cfg_meter_access_t::all(const cpp_int & _val) {
    int _count = 0;

    // max_cycle
    int_var__max_cycle = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< max_cycle_cpp_int_t >()  ;
    _count += 8;
    // start_addr
    int_var__start_addr = hlp.get_slc(_val, _count, _count -1 + 12 ).convert_to< start_addr_cpp_int_t >()  ;
    _count += 12;
    // end_addr
    int_var__end_addr = hlp.get_slc(_val, _count, _count -1 + 12 ).convert_to< end_addr_cpp_int_t >()  ;
    _count += 12;
}

void cap_pics_csr_cfg_meter_timer_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // cycle
    int_var__cycle = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< cycle_cpp_int_t >()  ;
    _count += 32;
}

void cap_pics_csr_cfg_update_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // oprd1_sel
    int_var__oprd1_sel = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< oprd1_sel_cpp_int_t >()  ;
    _count += 6;
    // oprd2_sel
    int_var__oprd2_sel = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< oprd2_sel_cpp_int_t >()  ;
    _count += 6;
    // oprd3_sel
    int_var__oprd3_sel = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< oprd3_sel_cpp_int_t >()  ;
    _count += 6;
    // oprd1_wid
    int_var__oprd1_wid = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< oprd1_wid_cpp_int_t >()  ;
    _count += 5;
    // oprd2_wid
    int_var__oprd2_wid = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< oprd2_wid_cpp_int_t >()  ;
    _count += 5;
    // oprd3_wid
    int_var__oprd3_wid = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< oprd3_wid_cpp_int_t >()  ;
    _count += 5;
}

void cap_pics_csr_cfg_table_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // start_addr
    int_var__start_addr = hlp.get_slc(_val, _count, _count -1 + 20 ).convert_to< start_addr_cpp_int_t >()  ;
    _count += 20;
    // end_addr
    int_var__end_addr = hlp.get_slc(_val, _count, _count -1 + 20 ).convert_to< end_addr_cpp_int_t >()  ;
    _count += 20;
    // width
    int_var__width = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< width_cpp_int_t >()  ;
    _count += 6;
    // hash
    int_var__hash = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< hash_cpp_int_t >()  ;
    _count += 1;
    // opcode
    int_var__opcode = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< opcode_cpp_int_t >()  ;
    _count += 9;
    // log2bkts
    int_var__log2bkts = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< log2bkts_cpp_int_t >()  ;
    _count += 3;
    // axishift
    int_var__axishift = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< axishift_cpp_int_t >()  ;
    _count += 5;
}

void cap_pics_csr_cfg_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // ecc_disable_cor
    int_var__ecc_disable_cor = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _count += 1;
    // ecc_disable_det
    int_var__ecc_disable_det = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< ecc_disable_det_cpp_int_t >()  ;
    _count += 1;
}

void cap_pics_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_pics_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_sram.all( hlp.get_slc(_val, _count, _count -1 + cfg_sram.get_width() )); // cfg_sram
    _count += cfg_sram.get_width();
    // cfg_table_profile
    for(int ii = 0; ii < 128; ii++) {
        cfg_table_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_table_profile[ii].get_width()));
        _count += cfg_table_profile[ii].get_width();
    }
    // cfg_update_profile
    for(int ii = 0; ii < 16; ii++) {
        cfg_update_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_update_profile[ii].get_width()));
        _count += cfg_update_profile[ii].get_width();
    }
    cfg_meter_timer.all( hlp.get_slc(_val, _count, _count -1 + cfg_meter_timer.get_width() )); // cfg_meter_timer
    _count += cfg_meter_timer.get_width();
    cfg_meter_access.all( hlp.get_slc(_val, _count, _count -1 + cfg_meter_access.get_width() )); // cfg_meter_access
    _count += cfg_meter_access.get_width();
    // cfg_meter_sram
    for(int ii = 0; ii < 40; ii++) {
        cfg_meter_sram[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_meter_sram[ii].get_width()));
        _count += cfg_meter_sram[ii].get_width();
    }
    dhs_sram.all( hlp.get_slc(_val, _count, _count -1 + dhs_sram.get_width() )); // dhs_sram
    _count += dhs_sram.get_width();
    dhs_table_address.all( hlp.get_slc(_val, _count, _count -1 + dhs_table_address.get_width() )); // dhs_table_address
    _count += dhs_table_address.get_width();
    dhs_table_data.all( hlp.get_slc(_val, _count, _count -1 + dhs_table_data.get_width() )); // dhs_table_data
    _count += dhs_table_data.get_width();
}

cpp_int cap_pics_csr_dhs_table_data_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 512 );
    _count += 512;
    return ret_val;
}

cpp_int cap_pics_csr_dhs_table_data_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, entry.all() , _count, _count -1 + entry.get_width() ); // entry
    _count += entry.get_width();
    return ret_val;
}

cpp_int cap_pics_csr_dhs_table_address_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // address
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__address) , _count, _count -1 + 64 );
    _count += 64;
    return ret_val;
}

cpp_int cap_pics_csr_dhs_table_address_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, entry.all() , _count, _count -1 + entry.get_width() ); // entry
    _count += entry.get_width();
    return ret_val;
}

cpp_int cap_pics_csr_dhs_sram_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 128 );
    _count += 128;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 9 );
    _count += 9;
    return ret_val;
}

cpp_int cap_pics_csr_dhs_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 40960; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_pics_csr_cfg_meter_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // start
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start) , _count, _count -1 + 12 );
    _count += 12;
    // end
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end) , _count, _count -1 + 12 );
    _count += 12;
    // scale
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scale) , _count, _count -1 + 16 );
    _count += 16;
    // opcode
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__opcode) , _count, _count -1 + 9 );
    _count += 9;
    return ret_val;
}

cpp_int cap_pics_csr_cfg_meter_access_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // max_cycle
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_cycle) , _count, _count -1 + 8 );
    _count += 8;
    // start_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_addr) , _count, _count -1 + 12 );
    _count += 12;
    // end_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_addr) , _count, _count -1 + 12 );
    _count += 12;
    return ret_val;
}

cpp_int cap_pics_csr_cfg_meter_timer_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // cycle
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cycle) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pics_csr_cfg_update_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // oprd1_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd1_sel) , _count, _count -1 + 6 );
    _count += 6;
    // oprd2_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd2_sel) , _count, _count -1 + 6 );
    _count += 6;
    // oprd3_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd3_sel) , _count, _count -1 + 6 );
    _count += 6;
    // oprd1_wid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd1_wid) , _count, _count -1 + 5 );
    _count += 5;
    // oprd2_wid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd2_wid) , _count, _count -1 + 5 );
    _count += 5;
    // oprd3_wid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__oprd3_wid) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_pics_csr_cfg_table_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // start_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_addr) , _count, _count -1 + 20 );
    _count += 20;
    // end_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_addr) , _count, _count -1 + 20 );
    _count += 20;
    // width
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__width) , _count, _count -1 + 6 );
    _count += 6;
    // hash
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hash) , _count, _count -1 + 1 );
    _count += 1;
    // opcode
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__opcode) , _count, _count -1 + 9 );
    _count += 9;
    // log2bkts
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__log2bkts) , _count, _count -1 + 3 );
    _count += 3;
    // axishift
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__axishift) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_pics_csr_cfg_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // ecc_disable_cor
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc_disable_cor) , _count, _count -1 + 1 );
    _count += 1;
    // ecc_disable_det
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc_disable_det) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pics_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pics_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_sram.all() , _count, _count -1 + cfg_sram.get_width() ); // cfg_sram
    _count += cfg_sram.get_width();
    // cfg_table_profile
    for(int ii = 0; ii < 128; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_table_profile[ii].all() , _count, _count -1 + cfg_table_profile[ii].get_width() );
        _count += cfg_table_profile[ii].get_width();
    }
    // cfg_update_profile
    for(int ii = 0; ii < 16; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_update_profile[ii].all() , _count, _count -1 + cfg_update_profile[ii].get_width() );
        _count += cfg_update_profile[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, cfg_meter_timer.all() , _count, _count -1 + cfg_meter_timer.get_width() ); // cfg_meter_timer
    _count += cfg_meter_timer.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_meter_access.all() , _count, _count -1 + cfg_meter_access.get_width() ); // cfg_meter_access
    _count += cfg_meter_access.get_width();
    // cfg_meter_sram
    for(int ii = 0; ii < 40; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_meter_sram[ii].all() , _count, _count -1 + cfg_meter_sram[ii].get_width() );
        _count += cfg_meter_sram[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, dhs_sram.all() , _count, _count -1 + dhs_sram.get_width() ); // dhs_sram
    _count += dhs_sram.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_table_address.all() , _count, _count -1 + dhs_table_address.get_width() ); // dhs_table_address
    _count += dhs_table_address.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_table_data.all() , _count, _count -1 + dhs_table_data.get_width() ); // dhs_table_data
    _count += dhs_table_data.get_width();
    return ret_val;
}

void cap_pics_csr_dhs_table_data_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_pics_csr_dhs_table_data_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_pics_csr_dhs_table_data_entry_t::data);
        }
        #endif
    
}

void cap_pics_csr_dhs_table_data_t::init() {

    entry.set_attributes(this,"entry", 0x0 );
}

void cap_pics_csr_dhs_table_address_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("address", (cap_csr_base::set_function_type_t)&cap_pics_csr_dhs_table_address_entry_t::address);
            register_get_func("address", (cap_csr_base::get_function_type_t)&cap_pics_csr_dhs_table_address_entry_t::address);
        }
        #endif
    
}

void cap_pics_csr_dhs_table_address_t::init() {

    entry.set_attributes(this,"entry", 0x0 );
}

void cap_pics_csr_dhs_sram_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_pics_csr_dhs_sram_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_pics_csr_dhs_sram_entry_t::data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_pics_csr_dhs_sram_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_pics_csr_dhs_sram_entry_t::ecc);
        }
        #endif
    
}

void cap_pics_csr_dhs_sram_t::init() {

    for(int ii = 0; ii < 40960; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_pics_csr_cfg_meter_sram_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_sram_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_sram_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_sram_t::start);
            register_get_func("start", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_sram_t::start);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_sram_t::end);
            register_get_func("end", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_sram_t::end);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scale", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_sram_t::scale);
            register_get_func("scale", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_sram_t::scale);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("opcode", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_sram_t::opcode);
            register_get_func("opcode", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_sram_t::opcode);
        }
        #endif
    
}

void cap_pics_csr_cfg_meter_access_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_cycle", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_access_t::max_cycle);
            register_get_func("max_cycle", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_access_t::max_cycle);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_addr", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_access_t::start_addr);
            register_get_func("start_addr", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_access_t::start_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_addr", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_access_t::end_addr);
            register_get_func("end_addr", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_access_t::end_addr);
        }
        #endif
    
    set_reset_val(cpp_int("0xfff00020"));
    all(get_reset_val());
}

void cap_pics_csr_cfg_meter_timer_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_timer_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_timer_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cycle", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_meter_timer_t::cycle);
            register_get_func("cycle", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_meter_timer_t::cycle);
        }
        #endif
    
}

void cap_pics_csr_cfg_update_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd1_sel", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd1_sel);
            register_get_func("oprd1_sel", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd1_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd2_sel", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd2_sel);
            register_get_func("oprd2_sel", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd2_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd3_sel", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd3_sel);
            register_get_func("oprd3_sel", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd3_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd1_wid", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd1_wid);
            register_get_func("oprd1_wid", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd1_wid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd2_wid", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd2_wid);
            register_get_func("oprd2_wid", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd2_wid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("oprd3_wid", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd3_wid);
            register_get_func("oprd3_wid", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_update_profile_t::oprd3_wid);
        }
        #endif
    
}

void cap_pics_csr_cfg_table_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_addr", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::start_addr);
            register_get_func("start_addr", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::start_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_addr", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::end_addr);
            register_get_func("end_addr", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::end_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("width", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::width);
            register_get_func("width", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::width);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hash", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::hash);
            register_get_func("hash", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::hash);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("opcode", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::opcode);
            register_get_func("opcode", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::opcode);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("log2bkts", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::log2bkts);
            register_get_func("log2bkts", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::log2bkts);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("axishift", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_table_profile_t::axishift);
            register_get_func("axishift", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_table_profile_t::axishift);
        }
        #endif
    
}

void cap_pics_csr_cfg_sram_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc_disable_cor", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_sram_t::ecc_disable_cor);
            register_get_func("ecc_disable_cor", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_sram_t::ecc_disable_cor);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc_disable_det", (cap_csr_base::set_function_type_t)&cap_pics_csr_cfg_sram_t::ecc_disable_det);
            register_get_func("ecc_disable_det", (cap_csr_base::get_function_type_t)&cap_pics_csr_cfg_sram_t::ecc_disable_det);
        }
        #endif
    
}

void cap_pics_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_pics_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_pics_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pics_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_sram.set_attributes(this,"cfg_sram", 0x400000 );
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_table_profile[ii].set_field_init_done(true);
        cfg_table_profile[ii].set_attributes(this,"cfg_table_profile["+to_string(ii)+"]",  0x400400 + (cfg_table_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_update_profile[ii].set_field_init_done(true);
        cfg_update_profile[ii].set_attributes(this,"cfg_update_profile["+to_string(ii)+"]",  0x400800 + (cfg_update_profile[ii].get_byte_size()*ii));
    }
    cfg_meter_timer.set_attributes(this,"cfg_meter_timer", 0x400880 );
    cfg_meter_access.set_attributes(this,"cfg_meter_access", 0x400888 );
    for(int ii = 0; ii < 40; ii++) {
        if(ii != 0) cfg_meter_sram[ii].set_field_init_done(true);
        cfg_meter_sram[ii].set_attributes(this,"cfg_meter_sram["+to_string(ii)+"]",  0x400a00 + (cfg_meter_sram[ii].get_byte_size()*ii));
    }
    dhs_sram.set_attributes(this,"dhs_sram", 0x200000 );
    dhs_table_address.set_attributes(this,"dhs_table_address", 0x400c00 );
    dhs_table_data.set_attributes(this,"dhs_table_data", 0x400c40 );
}

void cap_pics_csr_dhs_table_data_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_pics_csr_dhs_table_data_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_pics_csr_dhs_table_address_entry_t::address(const cpp_int & _val) { 
    // address
    int_var__address = _val.convert_to< address_cpp_int_t >();
}

cpp_int cap_pics_csr_dhs_table_address_entry_t::address() const {
    return int_var__address.convert_to< cpp_int >();
}
    
void cap_pics_csr_dhs_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_pics_csr_dhs_sram_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_pics_csr_dhs_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_pics_csr_dhs_sram_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_sram_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_sram_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_sram_t::start(const cpp_int & _val) { 
    // start
    int_var__start = _val.convert_to< start_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_sram_t::start() const {
    return int_var__start.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_sram_t::end(const cpp_int & _val) { 
    // end
    int_var__end = _val.convert_to< end_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_sram_t::end() const {
    return int_var__end.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_sram_t::scale(const cpp_int & _val) { 
    // scale
    int_var__scale = _val.convert_to< scale_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_sram_t::scale() const {
    return int_var__scale.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_sram_t::opcode(const cpp_int & _val) { 
    // opcode
    int_var__opcode = _val.convert_to< opcode_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_sram_t::opcode() const {
    return int_var__opcode.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_access_t::max_cycle(const cpp_int & _val) { 
    // max_cycle
    int_var__max_cycle = _val.convert_to< max_cycle_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_access_t::max_cycle() const {
    return int_var__max_cycle.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_access_t::start_addr(const cpp_int & _val) { 
    // start_addr
    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_access_t::start_addr() const {
    return int_var__start_addr.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_access_t::end_addr(const cpp_int & _val) { 
    // end_addr
    int_var__end_addr = _val.convert_to< end_addr_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_access_t::end_addr() const {
    return int_var__end_addr.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_timer_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_timer_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_meter_timer_t::cycle(const cpp_int & _val) { 
    // cycle
    int_var__cycle = _val.convert_to< cycle_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_meter_timer_t::cycle() const {
    return int_var__cycle.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd1_sel(const cpp_int & _val) { 
    // oprd1_sel
    int_var__oprd1_sel = _val.convert_to< oprd1_sel_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd1_sel() const {
    return int_var__oprd1_sel.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd2_sel(const cpp_int & _val) { 
    // oprd2_sel
    int_var__oprd2_sel = _val.convert_to< oprd2_sel_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd2_sel() const {
    return int_var__oprd2_sel.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd3_sel(const cpp_int & _val) { 
    // oprd3_sel
    int_var__oprd3_sel = _val.convert_to< oprd3_sel_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd3_sel() const {
    return int_var__oprd3_sel.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd1_wid(const cpp_int & _val) { 
    // oprd1_wid
    int_var__oprd1_wid = _val.convert_to< oprd1_wid_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd1_wid() const {
    return int_var__oprd1_wid.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd2_wid(const cpp_int & _val) { 
    // oprd2_wid
    int_var__oprd2_wid = _val.convert_to< oprd2_wid_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd2_wid() const {
    return int_var__oprd2_wid.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_update_profile_t::oprd3_wid(const cpp_int & _val) { 
    // oprd3_wid
    int_var__oprd3_wid = _val.convert_to< oprd3_wid_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_update_profile_t::oprd3_wid() const {
    return int_var__oprd3_wid.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::start_addr(const cpp_int & _val) { 
    // start_addr
    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::start_addr() const {
    return int_var__start_addr.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::end_addr(const cpp_int & _val) { 
    // end_addr
    int_var__end_addr = _val.convert_to< end_addr_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::end_addr() const {
    return int_var__end_addr.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::width(const cpp_int & _val) { 
    // width
    int_var__width = _val.convert_to< width_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::width() const {
    return int_var__width.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::hash(const cpp_int & _val) { 
    // hash
    int_var__hash = _val.convert_to< hash_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::hash() const {
    return int_var__hash.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::opcode(const cpp_int & _val) { 
    // opcode
    int_var__opcode = _val.convert_to< opcode_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::opcode() const {
    return int_var__opcode.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::log2bkts(const cpp_int & _val) { 
    // log2bkts
    int_var__log2bkts = _val.convert_to< log2bkts_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::log2bkts() const {
    return int_var__log2bkts.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_table_profile_t::axishift(const cpp_int & _val) { 
    // axishift
    int_var__axishift = _val.convert_to< axishift_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_table_profile_t::axishift() const {
    return int_var__axishift.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_sram_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_sram_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor.convert_to< cpp_int >();
}
    
void cap_pics_csr_cfg_sram_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_pics_csr_cfg_sram_t::ecc_disable_det() const {
    return int_var__ecc_disable_det.convert_to< cpp_int >();
}
    
void cap_pics_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pics_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    