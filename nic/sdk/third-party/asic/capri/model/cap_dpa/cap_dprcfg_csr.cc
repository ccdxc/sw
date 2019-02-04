
#include "cap_dprcfg_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprcfg_csr_cfg_spare_cfg_t::cap_dprcfg_csr_cfg_spare_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprcfg_csr_cfg_spare_cfg_t::~cap_dprcfg_csr_cfg_spare_cfg_t() { }

cap_dprcfg_csr_cfg_static_field_t::cap_dprcfg_csr_cfg_static_field_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprcfg_csr_cfg_static_field_t::~cap_dprcfg_csr_cfg_static_field_t() { }

cap_dprcfg_csr_t::cap_dprcfg_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1024);
        set_attributes(0,get_name(), 0);
        }
cap_dprcfg_csr_t::~cap_dprcfg_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprcfg_csr_cfg_spare_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprcfg_csr_cfg_static_field_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprcfg_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_static_field.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        cfg_static_field[ii].show();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_cfg.show(); // large_array
    #else
    for(int ii = 0; ii < 128; ii++) {
        cfg_spare_cfg[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_dprcfg_csr_cfg_spare_cfg_t::get_width() const {
    return cap_dprcfg_csr_cfg_spare_cfg_t::s_get_width();

}

int cap_dprcfg_csr_cfg_static_field_t::get_width() const {
    return cap_dprcfg_csr_cfg_static_field_t::s_get_width();

}

int cap_dprcfg_csr_t::get_width() const {
    return cap_dprcfg_csr_t::s_get_width();

}

int cap_dprcfg_csr_cfg_spare_cfg_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_dprcfg_csr_cfg_static_field_t::s_get_width() {
    int _count = 0;

    _count += 8; // data
    return _count;
}

int cap_dprcfg_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprcfg_csr_cfg_static_field_t::s_get_width() * 64); // cfg_static_field
    _count += (cap_dprcfg_csr_cfg_spare_cfg_t::s_get_width() * 128); // cfg_spare_cfg
    return _count;
}

void cap_dprcfg_csr_cfg_spare_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_dprcfg_csr_cfg_static_field_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_dprcfg_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_static_field
    for(int ii = 0; ii < 64; ii++) {
        cfg_static_field[ii].all(_val);
        _val = _val >> cfg_static_field[ii].get_width();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_cfg
    for(int ii = 0; ii < 128; ii++) {
        cfg_spare_cfg[ii].all(_val);
        _val = _val >> cfg_spare_cfg[ii].get_width();
    }
    #endif
    
}

cpp_int cap_dprcfg_csr_t::all() const {
    cpp_int ret_val;

    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_cfg
    for(int ii = 128-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_spare_cfg[ii].get_width(); ret_val = ret_val  | cfg_spare_cfg[ii].all(); 
    }
    #endif
    
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_static_field
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_static_field[ii].get_width(); ret_val = ret_val  | cfg_static_field[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprcfg_csr_cfg_static_field_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprcfg_csr_cfg_spare_cfg_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_dprcfg_csr_cfg_spare_cfg_t::clear() {

    int_var__data = 0; 
    
}

void cap_dprcfg_csr_cfg_static_field_t::clear() {

    int_var__data = 0; 
    
}

void cap_dprcfg_csr_t::clear() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_static_field
    for(int ii = 0; ii < 64; ii++) {
        cfg_static_field[ii].clear();
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_spare_cfg
    for(int ii = 0; ii < 128; ii++) {
        cfg_spare_cfg[ii].clear();
    }
    #endif
    
}

void cap_dprcfg_csr_cfg_spare_cfg_t::init() {

}

void cap_dprcfg_csr_cfg_static_field_t::init() {

    set_reset_val(cpp_int("0x55"));
    all(get_reset_val());
}

void cap_dprcfg_csr_t::init() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_static_field.set_attributes(this, "cfg_static_field", 0x0);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) cfg_static_field[ii].set_field_init_done(true, true);
        cfg_static_field[ii].set_attributes(this,"cfg_static_field["+to_string(ii)+"]",  0x0 + (cfg_static_field[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_cfg.set_attributes(this, "cfg_spare_cfg", 0x200);
    #else
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_spare_cfg[ii].set_field_init_done(true, true);
        cfg_spare_cfg[ii].set_attributes(this,"cfg_spare_cfg["+to_string(ii)+"]",  0x200 + (cfg_spare_cfg[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprcfg_csr_cfg_spare_cfg_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprcfg_csr_cfg_spare_cfg_t::data() const {
    return int_var__data;
}
    
void cap_dprcfg_csr_cfg_static_field_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprcfg_csr_cfg_static_field_t::data() const {
    return int_var__data;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_cfg_spare_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_cfg_static_field_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_cfg_spare_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_cfg_static_field_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprcfg_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprcfg_csr_cfg_spare_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprcfg_csr_cfg_static_field_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprcfg_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
