
#include "cap_dpphdrfld_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dpphdrfld_csr_cfg_spare_hdrfld_t::cap_dpphdrfld_csr_cfg_spare_hdrfld_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpphdrfld_csr_cfg_spare_hdrfld_t::~cap_dpphdrfld_csr_cfg_spare_hdrfld_t() { }

cap_dpphdrfld_csr_cfg_hdrfld_info_t::cap_dpphdrfld_csr_cfg_hdrfld_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpphdrfld_csr_cfg_hdrfld_info_t::~cap_dpphdrfld_csr_cfg_hdrfld_info_t() { }

cap_dpphdrfld_csr_t::cap_dpphdrfld_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(2048);
        set_attributes(0,get_name(), 0);
        }
cap_dpphdrfld_csr_t::~cap_dpphdrfld_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_dpphdrfld_csr_cfg_spare_hdrfld_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dpphdrfld_csr_cfg_hdrfld_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".size_sel: 0x" << int_var__size_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size_val: 0x" << int_var__size_val << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".allow_size0: 0x" << int_var__allow_size0 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dpphdrfld_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_hdrfld_info.show(); // large_array
    #else
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].show();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_hdrfld.show(); // large_array
    #else
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_hdrfld[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_dpphdrfld_csr_cfg_spare_hdrfld_t::get_width() const {
    return cap_dpphdrfld_csr_cfg_spare_hdrfld_t::s_get_width();

}

int cap_dpphdrfld_csr_cfg_hdrfld_info_t::get_width() const {
    return cap_dpphdrfld_csr_cfg_hdrfld_info_t::s_get_width();

}

int cap_dpphdrfld_csr_t::get_width() const {
    return cap_dpphdrfld_csr_t::s_get_width();

}

int cap_dpphdrfld_csr_cfg_spare_hdrfld_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_dpphdrfld_csr_cfg_hdrfld_info_t::s_get_width() {
    int _count = 0;

    _count += 2; // size_sel
    _count += 14; // size_val
    _count += 1; // allow_size0
    return _count;
}

int cap_dpphdrfld_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dpphdrfld_csr_cfg_hdrfld_info_t::s_get_width() * 256); // cfg_hdrfld_info
    _count += (cap_dpphdrfld_csr_cfg_spare_hdrfld_t::s_get_width() * 32); // cfg_spare_hdrfld
    return _count;
}

void cap_dpphdrfld_csr_cfg_spare_hdrfld_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_dpphdrfld_csr_cfg_hdrfld_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__size_sel = _val.convert_to< size_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__size_val = _val.convert_to< size_val_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__allow_size0 = _val.convert_to< allow_size0_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dpphdrfld_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].all(_val);
        _val = _val >> cfg_hdrfld_info[ii].get_width();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_hdrfld[ii].all(_val);
        _val = _val >> cfg_spare_hdrfld[ii].get_width();
    }
    #endif
    
}

cpp_int cap_dpphdrfld_csr_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 32-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_spare_hdrfld[ii].get_width(); ret_val = ret_val  | cfg_spare_hdrfld[ii].all(); 
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_hdrfld_info
    for(int ii = 256-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_hdrfld_info[ii].get_width(); ret_val = ret_val  | cfg_hdrfld_info[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dpphdrfld_csr_cfg_hdrfld_info_t::all() const {
    cpp_int ret_val;

    // allow_size0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__allow_size0; 
    
    // size_val
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__size_val; 
    
    // size_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__size_sel; 
    
    return ret_val;
}

cpp_int cap_dpphdrfld_csr_cfg_spare_hdrfld_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_dpphdrfld_csr_cfg_spare_hdrfld_t::clear() {

    int_var__data = 0; 
    
}

void cap_dpphdrfld_csr_cfg_hdrfld_info_t::clear() {

    int_var__size_sel = 0; 
    
    int_var__size_val = 0; 
    
    int_var__allow_size0 = 0; 
    
}

void cap_dpphdrfld_csr_t::clear() {

    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].clear();
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 0; ii < 32; ii++) {
        cfg_spare_hdrfld[ii].clear();
    }
    #endif
    
}

void cap_dpphdrfld_csr_cfg_spare_hdrfld_t::init() {

}

void cap_dpphdrfld_csr_cfg_hdrfld_info_t::init() {

    set_reset_val(cpp_int("0x20"));
    all(get_reset_val());
}

void cap_dpphdrfld_csr_t::init() {

    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_hdrfld_info.set_attributes(this, "cfg_hdrfld_info", 0x0);
    #else
    for(int ii = 0; ii < 256; ii++) {
        if(ii != 0) cfg_hdrfld_info[ii].set_field_init_done(true, true);
        cfg_hdrfld_info[ii].set_attributes(this,"cfg_hdrfld_info["+to_string(ii)+"]",  0x0 + (cfg_hdrfld_info[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_hdrfld.set_attributes(this, "cfg_spare_hdrfld", 0x400);
    #else
    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) cfg_spare_hdrfld[ii].set_field_init_done(true, true);
        cfg_spare_hdrfld[ii].set_attributes(this,"cfg_spare_hdrfld["+to_string(ii)+"]",  0x400 + (cfg_spare_hdrfld[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dpphdrfld_csr_cfg_spare_hdrfld_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dpphdrfld_csr_cfg_spare_hdrfld_t::data() const {
    return int_var__data;
}
    
void cap_dpphdrfld_csr_cfg_hdrfld_info_t::size_sel(const cpp_int & _val) { 
    // size_sel
    int_var__size_sel = _val.convert_to< size_sel_cpp_int_t >();
}

cpp_int cap_dpphdrfld_csr_cfg_hdrfld_info_t::size_sel() const {
    return int_var__size_sel;
}
    
void cap_dpphdrfld_csr_cfg_hdrfld_info_t::size_val(const cpp_int & _val) { 
    // size_val
    int_var__size_val = _val.convert_to< size_val_cpp_int_t >();
}

cpp_int cap_dpphdrfld_csr_cfg_hdrfld_info_t::size_val() const {
    return int_var__size_val;
}
    
void cap_dpphdrfld_csr_cfg_hdrfld_info_t::allow_size0(const cpp_int & _val) { 
    // allow_size0
    int_var__allow_size0 = _val.convert_to< allow_size0_cpp_int_t >();
}

cpp_int cap_dpphdrfld_csr_cfg_hdrfld_info_t::allow_size0() const {
    return int_var__allow_size0;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_cfg_spare_hdrfld_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_cfg_hdrfld_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "size_sel")) { field_val = size_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size_val")) { field_val = size_val(); field_found=1; }
    if(!field_found && !strcmp(field_name, "allow_size0")) { field_val = allow_size0(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_cfg_spare_hdrfld_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_cfg_hdrfld_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "size_sel")) { size_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size_val")) { size_val(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "allow_size0")) { allow_size0(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dpphdrfld_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dpphdrfld_csr_cfg_spare_hdrfld_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dpphdrfld_csr_cfg_hdrfld_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("size_sel");
    ret_vec.push_back("size_val");
    ret_vec.push_back("allow_size0");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dpphdrfld_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
