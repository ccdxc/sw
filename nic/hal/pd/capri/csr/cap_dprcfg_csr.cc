
#include "cap_dprcfg_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprcfg_csr_cfg_static_field_t::cap_dprcfg_csr_cfg_static_field_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprcfg_csr_cfg_static_field_t::~cap_dprcfg_csr_cfg_static_field_t() { }

cap_dprcfg_csr_t::cap_dprcfg_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(256);
        set_attributes(0,get_name(), 0);
        }
cap_dprcfg_csr_t::~cap_dprcfg_csr_t() { }

void cap_dprcfg_csr_cfg_static_field_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_dprcfg_csr_t::show() {

    for(int ii = 0; ii < 64; ii++) {
        cfg_static_field[ii].show();
    }
}

int cap_dprcfg_csr_cfg_static_field_t::get_width() const {
    return cap_dprcfg_csr_cfg_static_field_t::s_get_width();

}

int cap_dprcfg_csr_t::get_width() const {
    return cap_dprcfg_csr_t::s_get_width();

}

int cap_dprcfg_csr_cfg_static_field_t::s_get_width() {
    int _count = 0;

    _count += 8; // data
    return _count;
}

int cap_dprcfg_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprcfg_csr_cfg_static_field_t::s_get_width() * 64); // cfg_static_field
    return _count;
}

void cap_dprcfg_csr_cfg_static_field_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< data_cpp_int_t >()  ;
    _count += 8;
}

void cap_dprcfg_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_static_field
    for(int ii = 0; ii < 64; ii++) {
        cfg_static_field[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_static_field[ii].get_width()));
        _count += cfg_static_field[ii].get_width();
    }
}

cpp_int cap_dprcfg_csr_cfg_static_field_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_dprcfg_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_static_field
    for(int ii = 0; ii < 64; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_static_field[ii].all() , _count, _count -1 + cfg_static_field[ii].get_width() );
        _count += cfg_static_field[ii].get_width();
    }
    return ret_val;
}

void cap_dprcfg_csr_cfg_static_field_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_dprcfg_csr_cfg_static_field_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_dprcfg_csr_cfg_static_field_t::data);
        }
        #endif
    
    set_reset_val(cpp_int("0x55"));
    all(get_reset_val());
}

void cap_dprcfg_csr_t::init() {

    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) cfg_static_field[ii].set_field_init_done(true);
        cfg_static_field[ii].set_attributes(this,"cfg_static_field["+to_string(ii)+"]",  0x0 + (cfg_static_field[ii].get_byte_size()*ii));
    }
}

void cap_dprcfg_csr_cfg_static_field_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprcfg_csr_cfg_static_field_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    