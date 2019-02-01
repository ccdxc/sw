
#include "cap_dprhdrfld_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprhdrfld_csr_cfg_spare_hdrfld_t::cap_dprhdrfld_csr_cfg_spare_hdrfld_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprhdrfld_csr_cfg_spare_hdrfld_t::~cap_dprhdrfld_csr_cfg_spare_hdrfld_t() { }

cap_dprhdrfld_csr_cfg_hdrfld_info_t::cap_dprhdrfld_csr_cfg_hdrfld_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprhdrfld_csr_cfg_hdrfld_info_t::~cap_dprhdrfld_csr_cfg_hdrfld_info_t() { }

cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::~cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t() { }

cap_dprhdrfld_csr_t::cap_dprhdrfld_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(4096);
        set_attributes(0,get_name(), 0);
        }
cap_dprhdrfld_csr_t::~cap_dprhdrfld_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprhdrfld_csr_cfg_spare_hdrfld_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprhdrfld_csr_cfg_hdrfld_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".source_sel: 0x" << int_var__source_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".source_oft: 0x" << int_var__source_oft << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".start_loc: 0x" << int_var__start_loc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fld_size: 0x" << int_var__fld_size << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprhdrfld_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_ingress_rw_phv_info.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cfg_ingress_rw_phv_info[ii].show();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_hdrfld_info.show(); // large_array
    #else
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_hdrfld.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        cfg_spare_hdrfld[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_dprhdrfld_csr_cfg_spare_hdrfld_t::get_width() const {
    return cap_dprhdrfld_csr_cfg_spare_hdrfld_t::s_get_width();

}

int cap_dprhdrfld_csr_cfg_hdrfld_info_t::get_width() const {
    return cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width();

}

int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::get_width() const {
    return cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::s_get_width();

}

int cap_dprhdrfld_csr_t::get_width() const {
    return cap_dprhdrfld_csr_t::s_get_width();

}

int cap_dprhdrfld_csr_cfg_spare_hdrfld_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width() {
    int _count = 0;

    _count += 2; // source_sel
    _count += 14; // source_oft
    return _count;
}

int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 16; // start_loc
    _count += 16; // fld_size
    return _count;
}

int cap_dprhdrfld_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::s_get_width() * 4); // cfg_ingress_rw_phv_info
    _count += (cap_dprhdrfld_csr_cfg_hdrfld_info_t::s_get_width() * 256); // cfg_hdrfld_info
    _count += (cap_dprhdrfld_csr_cfg_spare_hdrfld_t::s_get_width() * 4); // cfg_spare_hdrfld
    return _count;
}

void cap_dprhdrfld_csr_cfg_spare_hdrfld_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__source_sel = _val.convert_to< source_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__source_oft = _val.convert_to< source_oft_cpp_int_t >()  ;
    _val = _val >> 14;
    
}

void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__start_loc = _val.convert_to< start_loc_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__fld_size = _val.convert_to< fld_size_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_dprhdrfld_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_ingress_rw_phv_info
    for(int ii = 0; ii < 4; ii++) {
        cfg_ingress_rw_phv_info[ii].all(_val);
        _val = _val >> cfg_ingress_rw_phv_info[ii].get_width();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].all(_val);
        _val = _val >> cfg_hdrfld_info[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 0; ii < 4; ii++) {
        cfg_spare_hdrfld[ii].all(_val);
        _val = _val >> cfg_spare_hdrfld[ii].get_width();
    }
    #endif
    
}

cpp_int cap_dprhdrfld_csr_t::all() const {
    cpp_int ret_val;

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 4-1; ii >= 0; ii--) {
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
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_ingress_rw_phv_info
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_ingress_rw_phv_info[ii].get_width(); ret_val = ret_val  | cfg_ingress_rw_phv_info[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::all() const {
    cpp_int ret_val;

    // fld_size
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__fld_size; 
    
    // start_loc
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__start_loc; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::all() const {
    cpp_int ret_val;

    // source_oft
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__source_oft; 
    
    // source_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__source_sel; 
    
    return ret_val;
}

cpp_int cap_dprhdrfld_csr_cfg_spare_hdrfld_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_dprhdrfld_csr_cfg_spare_hdrfld_t::clear() {

    int_var__data = 0; 
    
}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::clear() {

    int_var__source_sel = 0; 
    
    int_var__source_oft = 0; 
    
}

void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::clear() {

    int_var__enable = 0; 
    
    int_var__start_loc = 0; 
    
    int_var__fld_size = 0; 
    
}

void cap_dprhdrfld_csr_t::clear() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_ingress_rw_phv_info
    for(int ii = 0; ii < 4; ii++) {
        cfg_ingress_rw_phv_info[ii].clear();
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_hdrfld_info
    for(int ii = 0; ii < 256; ii++) {
        cfg_hdrfld_info[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_spare_hdrfld
    for(int ii = 0; ii < 4; ii++) {
        cfg_spare_hdrfld[ii].clear();
    }
    #endif
    
}

void cap_dprhdrfld_csr_cfg_spare_hdrfld_t::init() {

}

void cap_dprhdrfld_csr_cfg_hdrfld_info_t::init() {

    set_reset_val(cpp_int("0xe"));
    all(get_reset_val());
}

void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::init() {

}

void cap_dprhdrfld_csr_t::init() {

    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_ingress_rw_phv_info.set_attributes(this, "cfg_ingress_rw_phv_info", 0x0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_ingress_rw_phv_info[ii].set_field_init_done(true, true);
        cfg_ingress_rw_phv_info[ii].set_attributes(this,"cfg_ingress_rw_phv_info["+to_string(ii)+"]",  0x0 + (cfg_ingress_rw_phv_info[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_hdrfld_info.set_attributes(this, "cfg_hdrfld_info", 0x400);
    #else
    for(int ii = 0; ii < 256; ii++) {
        if(ii != 0) cfg_hdrfld_info[ii].set_field_init_done(true, true);
        cfg_hdrfld_info[ii].set_attributes(this,"cfg_hdrfld_info["+to_string(ii)+"]",  0x400 + (cfg_hdrfld_info[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_spare_hdrfld.set_attributes(this, "cfg_spare_hdrfld", 0x800);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) cfg_spare_hdrfld[ii].set_field_init_done(true, true);
        cfg_spare_hdrfld[ii].set_attributes(this,"cfg_spare_hdrfld["+to_string(ii)+"]",  0x800 + (cfg_spare_hdrfld[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprhdrfld_csr_cfg_spare_hdrfld_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_spare_hdrfld_t::data() const {
    return int_var__data;
}
    
void cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel(const cpp_int & _val) { 
    // source_sel
    int_var__source_sel = _val.convert_to< source_sel_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_sel() const {
    return int_var__source_sel;
}
    
void cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft(const cpp_int & _val) { 
    // source_oft
    int_var__source_oft = _val.convert_to< source_oft_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_hdrfld_info_t::source_oft() const {
    return int_var__source_oft;
}
    
void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::enable() const {
    return int_var__enable;
}
    
void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::start_loc(const cpp_int & _val) { 
    // start_loc
    int_var__start_loc = _val.convert_to< start_loc_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::start_loc() const {
    return int_var__start_loc;
}
    
void cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::fld_size(const cpp_int & _val) { 
    // fld_size
    int_var__fld_size = _val.convert_to< fld_size_cpp_int_t >();
}

cpp_int cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::fld_size() const {
    return int_var__fld_size;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_spare_hdrfld_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_hdrfld_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "source_sel")) { field_val = source_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "source_oft")) { field_val = source_oft(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_loc")) { field_val = start_loc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_size")) { field_val = fld_size(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_spare_hdrfld_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_hdrfld_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "source_sel")) { source_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "source_oft")) { source_oft(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "start_loc")) { start_loc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fld_size")) { fld_size(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprhdrfld_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprhdrfld_csr_cfg_spare_hdrfld_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprhdrfld_csr_cfg_hdrfld_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("source_sel");
    ret_vec.push_back("source_oft");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("start_loc");
    ret_vec.push_back("fld_size");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprhdrfld_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
