
#include "cap_dprhdrfld_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprhdrfld_csr_cfg_hdrfld_info_t::cap_dprhdrfld_csr_cfg_hdrfld_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprhdrfld_csr_cfg_hdrfld_info_t::~cap_dprhdrfld_csr_cfg_hdrfld_info_t() { }

cap_dprhdrfld_csr_t::cap_dprhdrfld_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1024);
        set_attributes(0,get_name(), 0);
        }
cap_dprhdrfld_csr_t::~cap_dprhdrfld_csr_t() { }

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".source_sel: 0x" << int_var__source_sel << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".source_oft: 0x" << int_var__source_oft << dec << endl)
}

void cap_dprhdrfld_csr_t::show() {

    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].show();
    }
}

int cap_dprhdrfld_csr_cfg_hdrfld_info_t::get_width() const {
    return cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width();

}

int cap_dprhdrfld_csr_t::get_width() const {
    return cap_dprhdrfld_csr_t::s_get_width();

}

int cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width() {
    int _count = 0;

    _count += 2; // source_sel
    _count += 16; // source_oft
    return _count;
}

int cap_dprhdrfld_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width() * 256); // cfg_hdrfld_info
    return _count;
}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::all(const cpp_int & _val) {
    int _count = 0;

    // source_sel
    int_var__source_sel = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< source_sel_cpp_int_t >()  ;
    _count += 2;
    // source_oft
    int_var__source_oft = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< source_oft_cpp_int_t >()  ;
    _count += 16;
}

void cap_dprhdrfld_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_hdrfld_info[ii].get_width()));
        _count += cfg_hdrfld_info[ii].get_width();
    }
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // source_sel
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__source_sel) , _count, _count -1 + 2 );
    _count += 2;
    // source_oft
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__source_oft) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_dprhdrfld_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_hdrfld_info[ii].all() , _count, _count -1 + cfg_hdrfld_info[ii].get_width() );
        _count += cfg_hdrfld_info[ii].get_width();
    }
    return ret_val;
}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("source_sel", (cap_csr_base::set_function_type_t)&cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel);
            register_get_func("source_sel", (cap_csr_base::get_function_type_t)&cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("source_oft", (cap_csr_base::set_function_type_t)&cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft);
            register_get_func("source_oft", (cap_csr_base::get_function_type_t)&cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft);
        }
        #endif
    
    set_reset_val(cpp_int("0xe"));
    all(get_reset_val());
}

void cap_dprhdrfld_csr_t::init() {

    for(int ii = 0; ii < 256; ii++) {
        if(ii != 0) cfg_hdrfld_info[ii].set_field_init_done(true);
        cfg_hdrfld_info[ii].set_attributes(this,"cfg_hdrfld_info["+to_string(ii)+"]",  0x0 + (cfg_hdrfld_info[ii].get_byte_size()*ii));
    }
}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel(const cpp_int & _val) { 
    // source_sel
    int_var__source_sel = _val.convert_to< source_sel_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel() const {
    return int_var__source_sel.convert_to< cpp_int >();
}
    
void cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft(const cpp_int & _val) { 
    // source_oft
    int_var__source_oft = _val.convert_to< source_oft_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft() const {
    return int_var__source_oft.convert_to< cpp_int >();
}
    