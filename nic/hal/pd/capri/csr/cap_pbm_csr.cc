
#include "cap_pbm_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pbm_csr_cfg_debug_t::cap_pbm_csr_cfg_debug_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbm_csr_cfg_debug_t::~cap_pbm_csr_cfg_debug_t() { }

cap_pbm_csr_cfg_control_t::cap_pbm_csr_cfg_control_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbm_csr_cfg_control_t::~cap_pbm_csr_cfg_control_t() { }

cap_pbm_csr_t::cap_pbm_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(64);
        set_attributes(0,get_name(), 0);
        }
cap_pbm_csr_t::~cap_pbm_csr_t() { }

void cap_pbm_csr_cfg_debug_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl)
}

void cap_pbm_csr_cfg_control_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sw_reset: 0x" << int_var__sw_reset << dec << endl)
}

void cap_pbm_csr_t::show() {

    cfg_control.show();
    cfg_debug.show();
}

int cap_pbm_csr_cfg_debug_t::get_width() const {
    return cap_pbm_csr_cfg_debug_t::s_get_width();

}

int cap_pbm_csr_cfg_control_t::get_width() const {
    return cap_pbm_csr_cfg_control_t::s_get_width();

}

int cap_pbm_csr_t::get_width() const {
    return cap_pbm_csr_t::s_get_width();

}

int cap_pbm_csr_cfg_debug_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 4; // select
    return _count;
}

int cap_pbm_csr_cfg_control_t::s_get_width() {
    int _count = 0;

    _count += 1; // sw_reset
    return _count;
}

int cap_pbm_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pbm_csr_cfg_control_t::s_get_width(); // cfg_control
    _count += cap_pbm_csr_cfg_debug_t::s_get_width(); // cfg_debug
    return _count;
}

void cap_pbm_csr_cfg_debug_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // select
    int_var__select = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< select_cpp_int_t >()  ;
    _count += 4;
}

void cap_pbm_csr_cfg_control_t::all(const cpp_int & _val) {
    int _count = 0;

    // sw_reset
    int_var__sw_reset = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< sw_reset_cpp_int_t >()  ;
    _count += 1;
}

void cap_pbm_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    cfg_control.all( hlp.get_slc(_val, _count, _count -1 + cfg_control.get_width() )); // cfg_control
    _count += cfg_control.get_width();
    cfg_debug.all( hlp.get_slc(_val, _count, _count -1 + cfg_debug.get_width() )); // cfg_debug
    _count += cfg_debug.get_width();
}

cpp_int cap_pbm_csr_cfg_debug_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // select
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__select) , _count, _count -1 + 4 );
    _count += 4;
    return ret_val;
}

cpp_int cap_pbm_csr_cfg_control_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sw_reset
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sw_reset) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_pbm_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, cfg_control.all() , _count, _count -1 + cfg_control.get_width() ); // cfg_control
    _count += cfg_control.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_debug.all() , _count, _count -1 + cfg_debug.get_width() ); // cfg_debug
    _count += cfg_debug.get_width();
    return ret_val;
}

void cap_pbm_csr_cfg_debug_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_pbm_csr_cfg_debug_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_pbm_csr_cfg_debug_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("select", (cap_csr_base::set_function_type_t)&cap_pbm_csr_cfg_debug_t::select);
            register_get_func("select", (cap_csr_base::get_function_type_t)&cap_pbm_csr_cfg_debug_t::select);
        }
        #endif
    
}

void cap_pbm_csr_cfg_control_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("sw_reset", (cap_csr_base::set_function_type_t)&cap_pbm_csr_cfg_control_t::sw_reset);
            register_get_func("sw_reset", (cap_csr_base::get_function_type_t)&cap_pbm_csr_cfg_control_t::sw_reset);
        }
        #endif
    
}

void cap_pbm_csr_t::init() {

    cfg_control.set_attributes(this,"cfg_control", 0x0 );
    cfg_debug.set_attributes(this,"cfg_debug", 0x4 );
}

void cap_pbm_csr_cfg_debug_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pbm_csr_cfg_debug_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_pbm_csr_cfg_debug_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_pbm_csr_cfg_debug_t::select() const {
    return int_var__select.convert_to< cpp_int >();
}
    
void cap_pbm_csr_cfg_control_t::sw_reset(const cpp_int & _val) { 
    // sw_reset
    int_var__sw_reset = _val.convert_to< sw_reset_cpp_int_t >();
}

cpp_int cap_pbm_csr_cfg_control_t::sw_reset() const {
    return int_var__sw_reset.convert_to< cpp_int >();
}
    