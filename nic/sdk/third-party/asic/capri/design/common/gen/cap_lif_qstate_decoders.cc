
#include "cap_lif_qstate_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_lif_qstate_data_bundle_t::cap_lif_qstate_data_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_lif_qstate_data_bundle_t::~cap_lif_qstate_data_bundle_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_lif_qstate_data_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".vld: 0x" << int_var__vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qstate_base: 0x" << int_var__qstate_base << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length0: 0x" << int_var__length0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size0: 0x" << int_var__size0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length1: 0x" << int_var__length1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size1: 0x" << int_var__size1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length2: 0x" << int_var__length2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size2: 0x" << int_var__size2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length3: 0x" << int_var__length3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size3: 0x" << int_var__size3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length4: 0x" << int_var__length4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size4: 0x" << int_var__size4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length5: 0x" << int_var__length5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size5: 0x" << int_var__size5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length6: 0x" << int_var__length6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size6: 0x" << int_var__size6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".length7: 0x" << int_var__length7 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".size7: 0x" << int_var__size7 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sched_hint_en: 0x" << int_var__sched_hint_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sched_hint_cos: 0x" << int_var__sched_hint_cos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

int cap_lif_qstate_data_bundle_t::get_width() const {
    return cap_lif_qstate_data_bundle_t::s_get_width();

}

int cap_lif_qstate_data_bundle_t::s_get_width() {
    int _count = 0;

    _count += 1; // vld
    _count += 22; // qstate_base
    _count += 5; // length0
    _count += 3; // size0
    _count += 5; // length1
    _count += 3; // size1
    _count += 5; // length2
    _count += 3; // size2
    _count += 5; // length3
    _count += 3; // size3
    _count += 5; // length4
    _count += 3; // size4
    _count += 5; // length5
    _count += 3; // size5
    _count += 5; // length6
    _count += 3; // size6
    _count += 5; // length7
    _count += 3; // size7
    _count += 1; // sched_hint_en
    _count += 4; // sched_hint_cos
    _count += 9; // spare
    _count += 8; // ecc
    return _count;
}

void cap_lif_qstate_data_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__vld = _val.convert_to< vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qstate_base = _val.convert_to< qstate_base_cpp_int_t >()  ;
    _val = _val >> 22;
    
    int_var__length0 = _val.convert_to< length0_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size0 = _val.convert_to< size0_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length1 = _val.convert_to< length1_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size1 = _val.convert_to< size1_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length2 = _val.convert_to< length2_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size2 = _val.convert_to< size2_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length3 = _val.convert_to< length3_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size3 = _val.convert_to< size3_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length4 = _val.convert_to< length4_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size4 = _val.convert_to< size4_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length5 = _val.convert_to< length5_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size5 = _val.convert_to< size5_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length6 = _val.convert_to< length6_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size6 = _val.convert_to< size6_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__length7 = _val.convert_to< length7_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__size7 = _val.convert_to< size7_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__sched_hint_en = _val.convert_to< sched_hint_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sched_hint_cos = _val.convert_to< sched_hint_cos_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

cpp_int cap_lif_qstate_data_bundle_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ecc; 
    
    // spare
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__spare; 
    
    // sched_hint_cos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sched_hint_cos; 
    
    // sched_hint_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sched_hint_en; 
    
    // size7
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size7; 
    
    // length7
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length7; 
    
    // size6
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size6; 
    
    // length6
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length6; 
    
    // size5
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size5; 
    
    // length5
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length5; 
    
    // size4
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size4; 
    
    // length4
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length4; 
    
    // size3
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size3; 
    
    // length3
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length3; 
    
    // size2
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size2; 
    
    // length2
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length2; 
    
    // size1
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size1; 
    
    // length1
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length1; 
    
    // size0
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__size0; 
    
    // length0
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__length0; 
    
    // qstate_base
    ret_val = ret_val << 22; ret_val = ret_val  | int_var__qstate_base; 
    
    // vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__vld; 
    
    return ret_val;
}

void cap_lif_qstate_data_bundle_t::clear() {

    int_var__vld = 0; 
    
    int_var__qstate_base = 0; 
    
    int_var__length0 = 0; 
    
    int_var__size0 = 0; 
    
    int_var__length1 = 0; 
    
    int_var__size1 = 0; 
    
    int_var__length2 = 0; 
    
    int_var__size2 = 0; 
    
    int_var__length3 = 0; 
    
    int_var__size3 = 0; 
    
    int_var__length4 = 0; 
    
    int_var__size4 = 0; 
    
    int_var__length5 = 0; 
    
    int_var__size5 = 0; 
    
    int_var__length6 = 0; 
    
    int_var__size6 = 0; 
    
    int_var__length7 = 0; 
    
    int_var__size7 = 0; 
    
    int_var__sched_hint_en = 0; 
    
    int_var__sched_hint_cos = 0; 
    
    int_var__spare = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_lif_qstate_data_bundle_t::init() {

}

void cap_lif_qstate_data_bundle_t::vld(const cpp_int & _val) { 
    // vld
    int_var__vld = _val.convert_to< vld_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::vld() const {
    return int_var__vld;
}
    
void cap_lif_qstate_data_bundle_t::qstate_base(const cpp_int & _val) { 
    // qstate_base
    int_var__qstate_base = _val.convert_to< qstate_base_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::qstate_base() const {
    return int_var__qstate_base;
}
    
void cap_lif_qstate_data_bundle_t::length0(const cpp_int & _val) { 
    // length0
    int_var__length0 = _val.convert_to< length0_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length0() const {
    return int_var__length0;
}
    
void cap_lif_qstate_data_bundle_t::size0(const cpp_int & _val) { 
    // size0
    int_var__size0 = _val.convert_to< size0_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size0() const {
    return int_var__size0;
}
    
void cap_lif_qstate_data_bundle_t::length1(const cpp_int & _val) { 
    // length1
    int_var__length1 = _val.convert_to< length1_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length1() const {
    return int_var__length1;
}
    
void cap_lif_qstate_data_bundle_t::size1(const cpp_int & _val) { 
    // size1
    int_var__size1 = _val.convert_to< size1_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size1() const {
    return int_var__size1;
}
    
void cap_lif_qstate_data_bundle_t::length2(const cpp_int & _val) { 
    // length2
    int_var__length2 = _val.convert_to< length2_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length2() const {
    return int_var__length2;
}
    
void cap_lif_qstate_data_bundle_t::size2(const cpp_int & _val) { 
    // size2
    int_var__size2 = _val.convert_to< size2_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size2() const {
    return int_var__size2;
}
    
void cap_lif_qstate_data_bundle_t::length3(const cpp_int & _val) { 
    // length3
    int_var__length3 = _val.convert_to< length3_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length3() const {
    return int_var__length3;
}
    
void cap_lif_qstate_data_bundle_t::size3(const cpp_int & _val) { 
    // size3
    int_var__size3 = _val.convert_to< size3_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size3() const {
    return int_var__size3;
}
    
void cap_lif_qstate_data_bundle_t::length4(const cpp_int & _val) { 
    // length4
    int_var__length4 = _val.convert_to< length4_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length4() const {
    return int_var__length4;
}
    
void cap_lif_qstate_data_bundle_t::size4(const cpp_int & _val) { 
    // size4
    int_var__size4 = _val.convert_to< size4_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size4() const {
    return int_var__size4;
}
    
void cap_lif_qstate_data_bundle_t::length5(const cpp_int & _val) { 
    // length5
    int_var__length5 = _val.convert_to< length5_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length5() const {
    return int_var__length5;
}
    
void cap_lif_qstate_data_bundle_t::size5(const cpp_int & _val) { 
    // size5
    int_var__size5 = _val.convert_to< size5_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size5() const {
    return int_var__size5;
}
    
void cap_lif_qstate_data_bundle_t::length6(const cpp_int & _val) { 
    // length6
    int_var__length6 = _val.convert_to< length6_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length6() const {
    return int_var__length6;
}
    
void cap_lif_qstate_data_bundle_t::size6(const cpp_int & _val) { 
    // size6
    int_var__size6 = _val.convert_to< size6_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size6() const {
    return int_var__size6;
}
    
void cap_lif_qstate_data_bundle_t::length7(const cpp_int & _val) { 
    // length7
    int_var__length7 = _val.convert_to< length7_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::length7() const {
    return int_var__length7;
}
    
void cap_lif_qstate_data_bundle_t::size7(const cpp_int & _val) { 
    // size7
    int_var__size7 = _val.convert_to< size7_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::size7() const {
    return int_var__size7;
}
    
void cap_lif_qstate_data_bundle_t::sched_hint_en(const cpp_int & _val) { 
    // sched_hint_en
    int_var__sched_hint_en = _val.convert_to< sched_hint_en_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::sched_hint_en() const {
    return int_var__sched_hint_en;
}
    
void cap_lif_qstate_data_bundle_t::sched_hint_cos(const cpp_int & _val) { 
    // sched_hint_cos
    int_var__sched_hint_cos = _val.convert_to< sched_hint_cos_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::sched_hint_cos() const {
    return int_var__sched_hint_cos;
}
    
void cap_lif_qstate_data_bundle_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::spare() const {
    return int_var__spare;
}
    
void cap_lif_qstate_data_bundle_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_lif_qstate_data_bundle_t::ecc() const {
    return int_var__ecc;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_lif_qstate_data_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vld")) { field_val = vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_base")) { field_val = qstate_base(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length0")) { field_val = length0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size0")) { field_val = size0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length1")) { field_val = length1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size1")) { field_val = size1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length2")) { field_val = length2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size2")) { field_val = size2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length3")) { field_val = length3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size3")) { field_val = size3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length4")) { field_val = length4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size4")) { field_val = size4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length5")) { field_val = length5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size5")) { field_val = size5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length6")) { field_val = length6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size6")) { field_val = size6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "length7")) { field_val = length7(); field_found=1; }
    if(!field_found && !strcmp(field_name, "size7")) { field_val = size7(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sched_hint_en")) { field_val = sched_hint_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sched_hint_cos")) { field_val = sched_hint_cos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_lif_qstate_data_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vld")) { vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_base")) { qstate_base(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length0")) { length0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size0")) { size0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length1")) { length1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size1")) { size1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length2")) { length2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size2")) { size2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length3")) { length3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size3")) { size3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length4")) { length4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size4")) { size4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length5")) { length5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size5")) { size5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length6")) { length6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size6")) { size6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "length7")) { length7(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "size7")) { size7(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sched_hint_en")) { sched_hint_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sched_hint_cos")) { sched_hint_cos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_lif_qstate_data_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("vld");
    ret_vec.push_back("qstate_base");
    ret_vec.push_back("length0");
    ret_vec.push_back("size0");
    ret_vec.push_back("length1");
    ret_vec.push_back("size1");
    ret_vec.push_back("length2");
    ret_vec.push_back("size2");
    ret_vec.push_back("length3");
    ret_vec.push_back("size3");
    ret_vec.push_back("length4");
    ret_vec.push_back("size4");
    ret_vec.push_back("length5");
    ret_vec.push_back("size5");
    ret_vec.push_back("length6");
    ret_vec.push_back("size6");
    ret_vec.push_back("length7");
    ret_vec.push_back("size7");
    ret_vec.push_back("sched_hint_en");
    ret_vec.push_back("sched_hint_cos");
    ret_vec.push_back("spare");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
