
#include "cap_qstate_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_qstate_index_pair_t::cap_qstate_index_pair_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_qstate_index_pair_t::~cap_qstate_index_pair_t() { }

cap_qstate_decoder_t::cap_qstate_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_qstate_decoder_t::~cap_qstate_decoder_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_qstate_index_pair_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pindex: 0x" << int_var__pindex << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cindex: 0x" << int_var__cindex << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_qstate_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pc: 0x" << int_var__pc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cosa: 0x" << int_var__cosa << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cosb: 0x" << int_var__cosb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cos_sel: 0x" << int_var__cos_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".eval_last: 0x" << int_var__eval_last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".host: 0x" << int_var__host << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".total: 0x" << int_var__total << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pid: 0x" << int_var__pid << dec << endl);
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    index.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        index[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_qstate_index_pair_t::get_width() const {
    return cap_qstate_index_pair_t::s_get_width();

}

int cap_qstate_decoder_t::get_width() const {
    return cap_qstate_decoder_t::s_get_width();

}

int cap_qstate_index_pair_t::s_get_width() {
    int _count = 0;

    _count += 16; // pindex
    _count += 16; // cindex
    return _count;
}

int cap_qstate_decoder_t::s_get_width() {
    int _count = 0;

    _count += 8; // pc
    _count += 8; // rsvd
    _count += 4; // cosa
    _count += 4; // cosb
    _count += 8; // cos_sel
    _count += 8; // eval_last
    _count += 4; // host
    _count += 4; // total
    _count += 16; // pid
    _count += (cap_qstate_index_pair_t::s_get_width() * 8); // index
    return _count;
}

void cap_qstate_index_pair_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pindex = _val.convert_to< pindex_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__cindex = _val.convert_to< cindex_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_qstate_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pc = _val.convert_to< pc_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__cosa = _val.convert_to< cosa_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__cosb = _val.convert_to< cosb_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__cos_sel = _val.convert_to< cos_sel_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__eval_last = _val.convert_to< eval_last_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__host = _val.convert_to< host_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__total = _val.convert_to< total_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__pid = _val.convert_to< pid_cpp_int_t >()  ;
    _val = _val >> 16;
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // index
    for(int ii = 0; ii < 8; ii++) {
        index[ii].all(_val);
        _val = _val >> index[ii].get_width();
    }
    #endif
    
}

cpp_int cap_qstate_decoder_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // index
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << index[ii].get_width(); ret_val = ret_val  | index[ii].all(); 
    }
    #endif
    
    // pid
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__pid; 
    
    // total
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__total; 
    
    // host
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__host; 
    
    // eval_last
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__eval_last; 
    
    // cos_sel
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__cos_sel; 
    
    // cosb
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cosb; 
    
    // cosa
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cosa; 
    
    // rsvd
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__rsvd; 
    
    // pc
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__pc; 
    
    return ret_val;
}

cpp_int cap_qstate_index_pair_t::all() const {
    cpp_int ret_val;

    // cindex
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__cindex; 
    
    // pindex
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__pindex; 
    
    return ret_val;
}

void cap_qstate_index_pair_t::clear() {

    int_var__pindex = 0; 
    
    int_var__cindex = 0; 
    
}

void cap_qstate_decoder_t::clear() {

    int_var__pc = 0; 
    
    int_var__rsvd = 0; 
    
    int_var__cosa = 0; 
    
    int_var__cosb = 0; 
    
    int_var__cos_sel = 0; 
    
    int_var__eval_last = 0; 
    
    int_var__host = 0; 
    
    int_var__total = 0; 
    
    int_var__pid = 0; 
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // index
    for(int ii = 0; ii < 8; ii++) {
        index[ii].clear();
    }
    #endif
    
}

void cap_qstate_index_pair_t::init() {

}

void cap_qstate_decoder_t::init() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    index.set_attributes(this, "index", 0x0);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) index[ii].set_field_init_done(true, true);
        index[ii].set_attributes(this,"index["+to_string(ii)+"]",  0x0 + (index[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_qstate_index_pair_t::pindex(const cpp_int & _val) { 
    // pindex
    int_var__pindex = _val.convert_to< pindex_cpp_int_t >();
}

cpp_int cap_qstate_index_pair_t::pindex() const {
    return int_var__pindex;
}
    
void cap_qstate_index_pair_t::cindex(const cpp_int & _val) { 
    // cindex
    int_var__cindex = _val.convert_to< cindex_cpp_int_t >();
}

cpp_int cap_qstate_index_pair_t::cindex() const {
    return int_var__cindex;
}
    
void cap_qstate_decoder_t::pc(const cpp_int & _val) { 
    // pc
    int_var__pc = _val.convert_to< pc_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::pc() const {
    return int_var__pc;
}
    
void cap_qstate_decoder_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_qstate_decoder_t::cosa(const cpp_int & _val) { 
    // cosa
    int_var__cosa = _val.convert_to< cosa_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::cosa() const {
    return int_var__cosa;
}
    
void cap_qstate_decoder_t::cosb(const cpp_int & _val) { 
    // cosb
    int_var__cosb = _val.convert_to< cosb_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::cosb() const {
    return int_var__cosb;
}
    
void cap_qstate_decoder_t::cos_sel(const cpp_int & _val) { 
    // cos_sel
    int_var__cos_sel = _val.convert_to< cos_sel_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::cos_sel() const {
    return int_var__cos_sel;
}
    
void cap_qstate_decoder_t::eval_last(const cpp_int & _val) { 
    // eval_last
    int_var__eval_last = _val.convert_to< eval_last_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::eval_last() const {
    return int_var__eval_last;
}
    
void cap_qstate_decoder_t::host(const cpp_int & _val) { 
    // host
    int_var__host = _val.convert_to< host_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::host() const {
    return int_var__host;
}
    
void cap_qstate_decoder_t::total(const cpp_int & _val) { 
    // total
    int_var__total = _val.convert_to< total_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::total() const {
    return int_var__total;
}
    
void cap_qstate_decoder_t::pid(const cpp_int & _val) { 
    // pid
    int_var__pid = _val.convert_to< pid_cpp_int_t >();
}

cpp_int cap_qstate_decoder_t::pid() const {
    return int_var__pid;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_qstate_index_pair_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pindex")) { field_val = pindex(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cindex")) { field_val = cindex(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_qstate_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pc")) { field_val = pc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cosa")) { field_val = cosa(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cosb")) { field_val = cosb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos_sel")) { field_val = cos_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "eval_last")) { field_val = eval_last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "host")) { field_val = host(); field_found=1; }
    if(!field_found && !strcmp(field_name, "total")) { field_val = total(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pid")) { field_val = pid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_qstate_index_pair_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pindex")) { pindex(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cindex")) { cindex(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_qstate_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pc")) { pc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cosa")) { cosa(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cosb")) { cosb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos_sel")) { cos_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "eval_last")) { eval_last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "host")) { host(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "total")) { total(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pid")) { pid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_qstate_index_pair_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pindex");
    ret_vec.push_back("cindex");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_qstate_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pc");
    ret_vec.push_back("rsvd");
    ret_vec.push_back("cosa");
    ret_vec.push_back("cosb");
    ret_vec.push_back("cos_sel");
    ret_vec.push_back("eval_last");
    ret_vec.push_back("host");
    ret_vec.push_back("total");
    ret_vec.push_back("pid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
