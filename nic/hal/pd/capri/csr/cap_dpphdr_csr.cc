
#include "cap_dpphdr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dpphdr_csr_cfg_hdr_info_t::cap_dpphdr_csr_cfg_hdr_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpphdr_csr_cfg_hdr_info_t::~cap_dpphdr_csr_cfg_hdr_info_t() { }

cap_dpphdr_csr_t::cap_dpphdr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(512);
        set_attributes(0,get_name(), 0);
        }
cap_dpphdr_csr_t::~cap_dpphdr_csr_t() { }

void cap_dpphdr_csr_cfg_hdr_info_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".fld_start: 0x" << int_var__fld_start << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_end: 0x" << int_var__fld_end << dec << endl)
}

void cap_dpphdr_csr_t::show() {

    for(int ii = 0; ii < 128; ii++) {
        cfg_hdr_info[ii].show();
    }
}

int cap_dpphdr_csr_cfg_hdr_info_t::get_width() const {
    return cap_dpphdr_csr_cfg_hdr_info_t::s_get_width();

}

int cap_dpphdr_csr_t::get_width() const {
    return cap_dpphdr_csr_t::s_get_width();

}

int cap_dpphdr_csr_cfg_hdr_info_t::s_get_width() {
    int _count = 0;

    _count += 8; // fld_start
    _count += 8; // fld_end
    return _count;
}

int cap_dpphdr_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dpphdr_csr_cfg_hdr_info_t::s_get_width() * 128); // cfg_hdr_info
    return _count;
}

void cap_dpphdr_csr_cfg_hdr_info_t::all(const cpp_int & _val) {
    int _count = 0;

    // fld_start
    int_var__fld_start = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< fld_start_cpp_int_t >()  ;
    _count += 8;
    // fld_end
    int_var__fld_end = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< fld_end_cpp_int_t >()  ;
    _count += 8;
}

void cap_dpphdr_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_hdr_info
    for(int ii = 0; ii < 128; ii++) {
        cfg_hdr_info[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_hdr_info[ii].get_width()));
        _count += cfg_hdr_info[ii].get_width();
    }
}

cpp_int cap_dpphdr_csr_cfg_hdr_info_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // fld_start
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_start) , _count, _count -1 + 8 );
    _count += 8;
    // fld_end
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__fld_end) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_dpphdr_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_hdr_info
    for(int ii = 0; ii < 128; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_hdr_info[ii].all() , _count, _count -1 + cfg_hdr_info[ii].get_width() );
        _count += cfg_hdr_info[ii].get_width();
    }
    return ret_val;
}

void cap_dpphdr_csr_cfg_hdr_info_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_start", (cap_csr_base::set_function_type_t)&cap_dpphdr_csr_cfg_hdr_info_t::fld_start);
            register_get_func("fld_start", (cap_csr_base::get_function_type_t)&cap_dpphdr_csr_cfg_hdr_info_t::fld_start);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("fld_end", (cap_csr_base::set_function_type_t)&cap_dpphdr_csr_cfg_hdr_info_t::fld_end);
            register_get_func("fld_end", (cap_csr_base::get_function_type_t)&cap_dpphdr_csr_cfg_hdr_info_t::fld_end);
        }
        #endif
    
    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_dpphdr_csr_t::init() {

    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_hdr_info[ii].set_field_init_done(true);
        cfg_hdr_info[ii].set_attributes(this,"cfg_hdr_info["+to_string(ii)+"]",  0x0 + (cfg_hdr_info[ii].get_byte_size()*ii));
    }
}

void cap_dpphdr_csr_cfg_hdr_info_t::fld_start(const cpp_int & _val) { 
    // fld_start
    int_var__fld_start = _val.convert_to< fld_start_cpp_int_t >();
}

cpp_int cap_dpphdr_csr_cfg_hdr_info_t::fld_start() const {
    return int_var__fld_start.convert_to< cpp_int >();
}
    
void cap_dpphdr_csr_cfg_hdr_info_t::fld_end(const cpp_int & _val) { 
    // fld_end
    int_var__fld_end = _val.convert_to< fld_end_cpp_int_t >();
}

cpp_int cap_dpphdr_csr_cfg_hdr_info_t::fld_end() const {
    return int_var__fld_end.convert_to< cpp_int >();
}
    