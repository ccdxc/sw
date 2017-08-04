
#include "cap_pict_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pict_csr_dhs_tcam_ind_entry_t::cap_pict_csr_dhs_tcam_ind_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_ind_entry_t::~cap_pict_csr_dhs_tcam_ind_entry_t() { }

cap_pict_csr_dhs_tcam_ind_t::cap_pict_csr_dhs_tcam_ind_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_ind_t::~cap_pict_csr_dhs_tcam_ind_t() { }

cap_pict_csr_dhs_tcam_xy_entry_t::cap_pict_csr_dhs_tcam_xy_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_xy_entry_t::~cap_pict_csr_dhs_tcam_xy_entry_t() { }

cap_pict_csr_dhs_tcam_xy_t::cap_pict_csr_dhs_tcam_xy_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_xy_t::~cap_pict_csr_dhs_tcam_xy_t() { }

cap_pict_csr_sta_tcam_ind_t::cap_pict_csr_sta_tcam_ind_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_sta_tcam_ind_t::~cap_pict_csr_sta_tcam_ind_t() { }

cap_pict_csr_cfg_tcam_ind_t::cap_pict_csr_cfg_tcam_ind_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_ind_t::~cap_pict_csr_cfg_tcam_ind_t() { }

cap_pict_csr_cfg_tcam_table_profile_t::cap_pict_csr_cfg_tcam_table_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_table_profile_t::~cap_pict_csr_cfg_tcam_table_profile_t() { }

cap_pict_csr_cfg_global_t::cap_pict_csr_cfg_global_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_global_t::~cap_pict_csr_cfg_global_t() { }

cap_pict_csr_base_t::cap_pict_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_base_t::~cap_pict_csr_base_t() { }

cap_pict_csr_t::cap_pict_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(2097152);
        set_attributes(0,get_name(), 0);
        }
cap_pict_csr_t::~cap_pict_csr_t() { }

void cap_pict_csr_dhs_tcam_ind_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cmd: 0x" << int_var__cmd << dec << endl)
}

void cap_pict_csr_dhs_tcam_ind_t::show() {

    for(int ii = 0; ii < 8; ii++) {
        entry[ii].show();
    }
}

void cap_pict_csr_dhs_tcam_xy_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".x: 0x" << int_var__x << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".y: 0x" << int_var__y << dec << endl)
}

void cap_pict_csr_dhs_tcam_xy_t::show() {

    for(int ii = 0; ii < 8192; ii++) {
        entry[ii].show();
    }
}

void cap_pict_csr_sta_tcam_ind_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".hit: 0x" << int_var__hit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".hit_addr: 0x" << int_var__hit_addr << dec << endl)
}

void cap_pict_csr_cfg_tcam_ind_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".grst_pulse: 0x" << int_var__grst_pulse << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mask: 0x" << int_var__mask << dec << endl)
}

void cap_pict_csr_cfg_tcam_table_profile_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".start_addr: 0x" << int_var__start_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".end_addr: 0x" << int_var__end_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".width: 0x" << int_var__width << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".en_tbid: 0x" << int_var__en_tbid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tbid: 0x" << int_var__tbid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".bkts: 0x" << int_var__bkts << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".keyshift: 0x" << int_var__keyshift << dec << endl)
}

void cap_pict_csr_cfg_global_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sw_rst: 0x" << int_var__sw_rst << dec << endl)
}

void cap_pict_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_pict_csr_t::show() {

    base.show();
    cfg_global.show();
    for(int ii = 0; ii < 128; ii++) {
        cfg_tcam_table_profile[ii].show();
    }
    for(int ii = 0; ii < 8; ii++) {
        cfg_tcam_ind[ii].show();
    }
    for(int ii = 0; ii < 8; ii++) {
        sta_tcam_ind[ii].show();
    }
    dhs_tcam_xy.show();
    dhs_tcam_ind.show();
}

int cap_pict_csr_dhs_tcam_ind_entry_t::get_width() const {
    return cap_pict_csr_dhs_tcam_ind_entry_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_ind_t::get_width() const {
    return cap_pict_csr_dhs_tcam_ind_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_xy_entry_t::get_width() const {
    return cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_xy_t::get_width() const {
    return cap_pict_csr_dhs_tcam_xy_t::s_get_width();

}

int cap_pict_csr_sta_tcam_ind_t::get_width() const {
    return cap_pict_csr_sta_tcam_ind_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_ind_t::get_width() const {
    return cap_pict_csr_cfg_tcam_ind_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_table_profile_t::get_width() const {
    return cap_pict_csr_cfg_tcam_table_profile_t::s_get_width();

}

int cap_pict_csr_cfg_global_t::get_width() const {
    return cap_pict_csr_cfg_global_t::s_get_width();

}

int cap_pict_csr_base_t::get_width() const {
    return cap_pict_csr_base_t::s_get_width();

}

int cap_pict_csr_t::get_width() const {
    return cap_pict_csr_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_ind_entry_t::s_get_width() {
    int _count = 0;

    _count += 2; // cmd
    return _count;
}

int cap_pict_csr_dhs_tcam_ind_t::s_get_width() {
    int _count = 0;

    _count += (cap_pict_csr_dhs_tcam_ind_entry_t::s_get_width() * 8); // entry
    return _count;
}

int cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 128; // x
    _count += 128; // y
    return _count;
}

int cap_pict_csr_dhs_tcam_xy_t::s_get_width() {
    int _count = 0;

    _count += (cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width() * 8192); // entry
    return _count;
}

int cap_pict_csr_sta_tcam_ind_t::s_get_width() {
    int _count = 0;

    _count += 1; // hit
    _count += 10; // hit_addr
    return _count;
}

int cap_pict_csr_cfg_tcam_ind_t::s_get_width() {
    int _count = 0;

    _count += 1; // grst_pulse
    _count += 128; // key
    _count += 128; // mask
    return _count;
}

int cap_pict_csr_cfg_tcam_table_profile_t::s_get_width() {
    int _count = 0;

    _count += 17; // start_addr
    _count += 17; // end_addr
    _count += 6; // width
    _count += 1; // en_tbid
    _count += 7; // tbid
    _count += 3; // bkts
    _count += 5; // keyshift
    return _count;
}

int cap_pict_csr_cfg_global_t::s_get_width() {
    int _count = 0;

    _count += 1; // sw_rst
    return _count;
}

int cap_pict_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pict_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pict_csr_base_t::s_get_width(); // base
    _count += cap_pict_csr_cfg_global_t::s_get_width(); // cfg_global
    _count += (cap_pict_csr_cfg_tcam_table_profile_t::s_get_width() * 128); // cfg_tcam_table_profile
    _count += (cap_pict_csr_cfg_tcam_ind_t::s_get_width() * 8); // cfg_tcam_ind
    _count += (cap_pict_csr_sta_tcam_ind_t::s_get_width() * 8); // sta_tcam_ind
    _count += cap_pict_csr_dhs_tcam_xy_t::s_get_width(); // dhs_tcam_xy
    _count += cap_pict_csr_dhs_tcam_ind_t::s_get_width(); // dhs_tcam_ind
    return _count;
}

void cap_pict_csr_dhs_tcam_ind_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // cmd
    int_var__cmd = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< cmd_cpp_int_t >()  ;
    _count += 2;
}

void cap_pict_csr_dhs_tcam_ind_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 8; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_pict_csr_dhs_tcam_xy_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // valid
    int_var__valid = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< valid_cpp_int_t >()  ;
    _count += 1;
    // x
    int_var__x = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< x_cpp_int_t >()  ;
    _count += 128;
    // y
    int_var__y = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< y_cpp_int_t >()  ;
    _count += 128;
}

void cap_pict_csr_dhs_tcam_xy_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 8192; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_pict_csr_sta_tcam_ind_t::all(const cpp_int & _val) {
    int _count = 0;

    // hit
    int_var__hit = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< hit_cpp_int_t >()  ;
    _count += 1;
    // hit_addr
    int_var__hit_addr = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< hit_addr_cpp_int_t >()  ;
    _count += 10;
}

void cap_pict_csr_cfg_tcam_ind_t::all(const cpp_int & _val) {
    int _count = 0;

    // grst_pulse
    int_var__grst_pulse = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< grst_pulse_cpp_int_t >()  ;
    _count += 1;
    // key
    int_var__key = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< key_cpp_int_t >()  ;
    _count += 128;
    // mask
    int_var__mask = hlp.get_slc(_val, _count, _count -1 + 128 ).convert_to< mask_cpp_int_t >()  ;
    _count += 128;
}

void cap_pict_csr_cfg_tcam_table_profile_t::all(const cpp_int & _val) {
    int _count = 0;

    // start_addr
    int_var__start_addr = hlp.get_slc(_val, _count, _count -1 + 17 ).convert_to< start_addr_cpp_int_t >()  ;
    _count += 17;
    // end_addr
    int_var__end_addr = hlp.get_slc(_val, _count, _count -1 + 17 ).convert_to< end_addr_cpp_int_t >()  ;
    _count += 17;
    // width
    int_var__width = hlp.get_slc(_val, _count, _count -1 + 6 ).convert_to< width_cpp_int_t >()  ;
    _count += 6;
    // en_tbid
    int_var__en_tbid = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< en_tbid_cpp_int_t >()  ;
    _count += 1;
    // tbid
    int_var__tbid = hlp.get_slc(_val, _count, _count -1 + 7 ).convert_to< tbid_cpp_int_t >()  ;
    _count += 7;
    // bkts
    int_var__bkts = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< bkts_cpp_int_t >()  ;
    _count += 3;
    // keyshift
    int_var__keyshift = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< keyshift_cpp_int_t >()  ;
    _count += 5;
}

void cap_pict_csr_cfg_global_t::all(const cpp_int & _val) {
    int _count = 0;

    // sw_rst
    int_var__sw_rst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< sw_rst_cpp_int_t >()  ;
    _count += 1;
}

void cap_pict_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_pict_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_global.all( hlp.get_slc(_val, _count, _count -1 + cfg_global.get_width() )); // cfg_global
    _count += cfg_global.get_width();
    // cfg_tcam_table_profile
    for(int ii = 0; ii < 128; ii++) {
        cfg_tcam_table_profile[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_tcam_table_profile[ii].get_width()));
        _count += cfg_tcam_table_profile[ii].get_width();
    }
    // cfg_tcam_ind
    for(int ii = 0; ii < 8; ii++) {
        cfg_tcam_ind[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_tcam_ind[ii].get_width()));
        _count += cfg_tcam_ind[ii].get_width();
    }
    // sta_tcam_ind
    for(int ii = 0; ii < 8; ii++) {
        sta_tcam_ind[ii].all( hlp.get_slc(_val, _count, _count -1 + sta_tcam_ind[ii].get_width()));
        _count += sta_tcam_ind[ii].get_width();
    }
    dhs_tcam_xy.all( hlp.get_slc(_val, _count, _count -1 + dhs_tcam_xy.get_width() )); // dhs_tcam_xy
    _count += dhs_tcam_xy.get_width();
    dhs_tcam_ind.all( hlp.get_slc(_val, _count, _count -1 + dhs_tcam_ind.get_width() )); // dhs_tcam_ind
    _count += dhs_tcam_ind.get_width();
}

cpp_int cap_pict_csr_dhs_tcam_ind_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cmd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cmd) , _count, _count -1 + 2 );
    _count += 2;
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_ind_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // valid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__valid) , _count, _count -1 + 1 );
    _count += 1;
    // x
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__x) , _count, _count -1 + 128 );
    _count += 128;
    // y
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__y) , _count, _count -1 + 128 );
    _count += 128;
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_xy_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 8192; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_pict_csr_sta_tcam_ind_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // hit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hit) , _count, _count -1 + 1 );
    _count += 1;
    // hit_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hit_addr) , _count, _count -1 + 10 );
    _count += 10;
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_ind_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // grst_pulse
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__grst_pulse) , _count, _count -1 + 1 );
    _count += 1;
    // key
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__key) , _count, _count -1 + 128 );
    _count += 128;
    // mask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mask) , _count, _count -1 + 128 );
    _count += 128;
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // start_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_addr) , _count, _count -1 + 17 );
    _count += 17;
    // end_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__end_addr) , _count, _count -1 + 17 );
    _count += 17;
    // width
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__width) , _count, _count -1 + 6 );
    _count += 6;
    // en_tbid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__en_tbid) , _count, _count -1 + 1 );
    _count += 1;
    // tbid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tbid) , _count, _count -1 + 7 );
    _count += 7;
    // bkts
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__bkts) , _count, _count -1 + 3 );
    _count += 3;
    // keyshift
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__keyshift) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_pict_csr_cfg_global_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sw_rst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sw_rst) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pict_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pict_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_global.all() , _count, _count -1 + cfg_global.get_width() ); // cfg_global
    _count += cfg_global.get_width();
    // cfg_tcam_table_profile
    for(int ii = 0; ii < 128; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_tcam_table_profile[ii].all() , _count, _count -1 + cfg_tcam_table_profile[ii].get_width() );
        _count += cfg_tcam_table_profile[ii].get_width();
    }
    // cfg_tcam_ind
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_tcam_ind[ii].all() , _count, _count -1 + cfg_tcam_ind[ii].get_width() );
        _count += cfg_tcam_ind[ii].get_width();
    }
    // sta_tcam_ind
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, sta_tcam_ind[ii].all() , _count, _count -1 + sta_tcam_ind[ii].get_width() );
        _count += sta_tcam_ind[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, dhs_tcam_xy.all() , _count, _count -1 + dhs_tcam_xy.get_width() ); // dhs_tcam_xy
    _count += dhs_tcam_xy.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_tcam_ind.all() , _count, _count -1 + dhs_tcam_ind.get_width() ); // dhs_tcam_ind
    _count += dhs_tcam_ind.get_width();
    return ret_val;
}

void cap_pict_csr_dhs_tcam_ind_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cmd", (cap_csr_base::set_function_type_t)&cap_pict_csr_dhs_tcam_ind_entry_t::cmd);
            register_get_func("cmd", (cap_csr_base::get_function_type_t)&cap_pict_csr_dhs_tcam_ind_entry_t::cmd);
        }
        #endif
    
}

void cap_pict_csr_dhs_tcam_ind_t::init() {

    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_pict_csr_dhs_tcam_xy_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("valid", (cap_csr_base::set_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::valid);
            register_get_func("valid", (cap_csr_base::get_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::valid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("x", (cap_csr_base::set_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::x);
            register_get_func("x", (cap_csr_base::get_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::x);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("y", (cap_csr_base::set_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::y);
            register_get_func("y", (cap_csr_base::get_function_type_t)&cap_pict_csr_dhs_tcam_xy_entry_t::y);
        }
        #endif
    
}

void cap_pict_csr_dhs_tcam_xy_t::init() {

    for(int ii = 0; ii < 8192; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_pict_csr_sta_tcam_ind_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hit", (cap_csr_base::set_function_type_t)&cap_pict_csr_sta_tcam_ind_t::hit);
            register_get_func("hit", (cap_csr_base::get_function_type_t)&cap_pict_csr_sta_tcam_ind_t::hit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hit_addr", (cap_csr_base::set_function_type_t)&cap_pict_csr_sta_tcam_ind_t::hit_addr);
            register_get_func("hit_addr", (cap_csr_base::get_function_type_t)&cap_pict_csr_sta_tcam_ind_t::hit_addr);
        }
        #endif
    
}

void cap_pict_csr_cfg_tcam_ind_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("grst_pulse", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::grst_pulse);
            register_get_func("grst_pulse", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::grst_pulse);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("key", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::key);
            register_get_func("key", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::key);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mask", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::mask);
            register_get_func("mask", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_ind_t::mask);
        }
        #endif
    
}

void cap_pict_csr_cfg_tcam_table_profile_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("start_addr", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::start_addr);
            register_get_func("start_addr", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::start_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("end_addr", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::end_addr);
            register_get_func("end_addr", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::end_addr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("width", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::width);
            register_get_func("width", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::width);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("en_tbid", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::en_tbid);
            register_get_func("en_tbid", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::en_tbid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tbid", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::tbid);
            register_get_func("tbid", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::tbid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("bkts", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::bkts);
            register_get_func("bkts", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::bkts);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("keyshift", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::keyshift);
            register_get_func("keyshift", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_tcam_table_profile_t::keyshift);
        }
        #endif
    
}

void cap_pict_csr_cfg_global_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sw_rst", (cap_csr_base::set_function_type_t)&cap_pict_csr_cfg_global_t::sw_rst);
            register_get_func("sw_rst", (cap_csr_base::get_function_type_t)&cap_pict_csr_cfg_global_t::sw_rst);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pict_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_pict_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_pict_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pict_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_global.set_attributes(this,"cfg_global", 0x4 );
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_tcam_table_profile[ii].set_field_init_done(true);
        cfg_tcam_table_profile[ii].set_attributes(this,"cfg_tcam_table_profile["+to_string(ii)+"]",  0x400 + (cfg_tcam_table_profile[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cfg_tcam_ind[ii].set_field_init_done(true);
        cfg_tcam_ind[ii].set_attributes(this,"cfg_tcam_ind["+to_string(ii)+"]",  0x100000 + (cfg_tcam_ind[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) sta_tcam_ind[ii].set_field_init_done(true);
        sta_tcam_ind[ii].set_attributes(this,"sta_tcam_ind["+to_string(ii)+"]",  0x100200 + (sta_tcam_ind[ii].get_byte_size()*ii));
    }
    dhs_tcam_xy.set_attributes(this,"dhs_tcam_xy", 0x80000 );
    dhs_tcam_ind.set_attributes(this,"dhs_tcam_ind", 0x100220 );
}

void cap_pict_csr_dhs_tcam_ind_entry_t::cmd(const cpp_int & _val) { 
    // cmd
    int_var__cmd = _val.convert_to< cmd_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_ind_entry_t::cmd() const {
    return int_var__cmd.convert_to< cpp_int >();
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::valid() const {
    return int_var__valid.convert_to< cpp_int >();
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::x(const cpp_int & _val) { 
    // x
    int_var__x = _val.convert_to< x_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::x() const {
    return int_var__x.convert_to< cpp_int >();
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::y(const cpp_int & _val) { 
    // y
    int_var__y = _val.convert_to< y_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::y() const {
    return int_var__y.convert_to< cpp_int >();
}
    
void cap_pict_csr_sta_tcam_ind_t::hit(const cpp_int & _val) { 
    // hit
    int_var__hit = _val.convert_to< hit_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_ind_t::hit() const {
    return int_var__hit.convert_to< cpp_int >();
}
    
void cap_pict_csr_sta_tcam_ind_t::hit_addr(const cpp_int & _val) { 
    // hit_addr
    int_var__hit_addr = _val.convert_to< hit_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_ind_t::hit_addr() const {
    return int_var__hit_addr.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_ind_t::grst_pulse(const cpp_int & _val) { 
    // grst_pulse
    int_var__grst_pulse = _val.convert_to< grst_pulse_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_ind_t::grst_pulse() const {
    return int_var__grst_pulse.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_ind_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_ind_t::key() const {
    return int_var__key.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_ind_t::mask(const cpp_int & _val) { 
    // mask
    int_var__mask = _val.convert_to< mask_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_ind_t::mask() const {
    return int_var__mask.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::start_addr(const cpp_int & _val) { 
    // start_addr
    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::start_addr() const {
    return int_var__start_addr.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::end_addr(const cpp_int & _val) { 
    // end_addr
    int_var__end_addr = _val.convert_to< end_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::end_addr() const {
    return int_var__end_addr.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::width(const cpp_int & _val) { 
    // width
    int_var__width = _val.convert_to< width_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::width() const {
    return int_var__width.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::en_tbid(const cpp_int & _val) { 
    // en_tbid
    int_var__en_tbid = _val.convert_to< en_tbid_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::en_tbid() const {
    return int_var__en_tbid.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::tbid(const cpp_int & _val) { 
    // tbid
    int_var__tbid = _val.convert_to< tbid_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::tbid() const {
    return int_var__tbid.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::bkts(const cpp_int & _val) { 
    // bkts
    int_var__bkts = _val.convert_to< bkts_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::bkts() const {
    return int_var__bkts.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::keyshift(const cpp_int & _val) { 
    // keyshift
    int_var__keyshift = _val.convert_to< keyshift_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::keyshift() const {
    return int_var__keyshift.convert_to< cpp_int >();
}
    
void cap_pict_csr_cfg_global_t::sw_rst(const cpp_int & _val) { 
    // sw_rst
    int_var__sw_rst = _val.convert_to< sw_rst_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_global_t::sw_rst() const {
    return int_var__sw_rst.convert_to< cpp_int >();
}
    
void cap_pict_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pict_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    