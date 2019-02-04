#include "pen_csr_base.h"
#include "sknobs.h"
#include "pknobs_reader.h"
#include <boost/algorithm/string/case_conv.hpp>


pen_csr_base::pen_csr_base(string _name, pen_csr_base * _parent) {
#ifndef CSR_NO_BASE_NAME    
    base__name = _name;
#endif    
//#ifndef CSR_NO_TYPE_NAME
//    type__name = _name;
//#endif
    base__parent = _parent;
    base__int__offset = 0;
#ifndef CSR_NO_RESET_VAL    
    int_var__reset_value = 0;
#endif    
    int__csr_end_addr = 0;
#ifndef CSR_NO_CSR_TYPE    
    base__int__csr_type = CSR_TYPE_NONE; //valgrind complains about this if uninitialized
#endif    
}


pen_csr_base::~pen_csr_base() { }

string pen_csr_base::get_name() const {
#ifndef CSR_NO_BASE_NAME    
    return base__name;
#else
    return "none";
#endif
}

//string pen_csr_base::get_type_name() const {
//#ifndef CSR_NO_TYPE_NAME
//    return type__name;
//#else
//    return "none";
//#endif
//}

pen_csr_base * pen_csr_base::get_parent() const {
    return base__parent;
}
void pen_csr_base::set_name(string _name) {
#ifndef CSR_NO_BASE_NAME    
    base__name = _name;
#endif    
}
void pen_csr_base::set_parent(pen_csr_base * _base) {
    if(base__parent && (_base != base__parent)) {
        PLOG_WARN("parent is already non-null, unexpected behavior" << endl);
    }

    if( (_base != 0) && (base__parent == 0)) {
        base__parent = _base;
        register_child(this,1);
    }
    base__parent = _base;
}

void pen_csr_base::register_child(pen_csr_base * _child, bool add_to_parent) {
    if(_child ==0) return;
    if(add_to_parent) {
        get_parent()->register_child(_child, 0);
    } else {
#ifndef CSR_NO_CHILDREN_REG        
        int__children.push_back(_child);
#endif        
    }
}

void pen_csr_base::update_block_boundaries(pen_csr_base * _child) {
    if(_child->get_csr_end_addr() >= int__csr_end_addr) {
        int__csr_end_addr = _child->get_csr_end_addr();
    }
}


string pen_csr_base::get_hier_path() const {
    string ret_val = "";
    if(get_parent() != 0) {
        ret_val = get_parent()->get_hier_path() + "." + get_name();
    } else {
        ret_val = get_name();
    }
    return ret_val;
}

int pen_csr_base::get_width() const {
    return 0;
}
void pen_csr_base::init() {
    return;
}
void pen_csr_base::show() {
    return; 
}
void pen_csr_base::all(const cpp_int & l__val) {
    return;
}
cpp_int pen_csr_base::all() const {
    return cpp_int('0');
}

int pen_csr_base::get_byte_size() const {
    int my_width = get_width();
    if(my_width < 32) my_width = 32;
    return pow(2,ceil(log2(my_width)))/8;
}

void pen_csr_base::set_offset(uint64_t _offset) {
    base__int__offset = _offset;
}

uint64_t pen_csr_base::get_offset() const {
    uint64_t ret_val = 0;
    if(get_parent() != 0) {
        ret_val = get_parent()->get_offset() + base__int__offset;
    } else {
        ret_val = base__int__offset;
    }
    return ret_val;
}

void pen_csr_base::set_reset_val(cpp_int _val) {
#ifndef CSR_NO_RESET_VAL
    int_var__reset_value = _val;
#endif    
}
cpp_int pen_csr_base::get_reset_val() const {
#ifndef CSR_NO_RESET_VAL
    return int_var__reset_value;
#else 
    return 0;
#endif
}
vector<pen_csr_base *> pen_csr_base::get_children(int level) const {
    vector<pen_csr_base *> ret_val;
    if( (level == -1) || (level > 0)) {
#ifndef CSR_NO_CHILDREN_REG        
        for (auto i: int__children) {
            ret_val.push_back(i);
            vector<pen_csr_base *> tmp = i->get_children(level >0 ? level -1 : level);
            ret_val.insert(ret_val.end(),  tmp.begin(), tmp.end());
        }
#endif        
    }
    return ret_val;
}
vector<pen_csr_base *> pen_csr_base::get_children_prefix(string pre, int level) const {

    vector<pen_csr_base *> ret_val;
    string                 csr_name;
    boost::algorithm::to_lower(pre);
    for(auto i: get_children(level)) {
        csr_name = i->get_name();
        boost::algorithm::to_lower(csr_name);
        if(csr_name.find(pre) == 0) {
            ret_val.push_back(i);
        }
    }

    return ret_val;
}

vector<pen_csr_base *> pen_csr_base::get_children_string(string str, int level) const {

    vector<pen_csr_base *> ret_val;
    string                 csr_name;
    boost::algorithm::to_lower(str);
    for(auto i: get_children(level)) {
        csr_name = i->get_name();
        boost::algorithm::to_lower(csr_name);
        if(csr_name.find(str) != string::npos) {
            ret_val.push_back(i);
        }
    }

    return ret_val;
}

vector<pen_csr_base *> pen_csr_base::get_children_hier_name(string str, int level) const {

    vector<pen_csr_base *> ret_val;
    string                 csr_name;
    boost::algorithm::to_lower(str);
    for(auto i: get_children(level)) {
        csr_name = i->get_hier_path() + i->get_name();
        boost::algorithm::to_lower(csr_name);
        //if(csr_name.find(str) != string::npos) {
        if(csr_name == str) {
            ret_val.push_back(i);
        }
    }

    return ret_val;
}

pen_csr_base::csr_type_t pen_csr_base::get_csr_type() const {
#ifndef CSR_NO_CSR_TYPE    
    return base__int__csr_type;
#else
    return CSR_TYPE_NONE;
#endif
}
void pen_csr_base::set_csr_type(csr_type_t _type) {
#ifndef CSR_NO_CSR_TYPE    
    base__int__csr_type = _type;
#endif    
}

unsigned pen_csr_base::get_csr_id() const {
    PLOG_ERR("pen_csr_base:: get_csr_id not to be used" << endl);
    return 0;
}
void pen_csr_base::set_csr_id(unsigned _id) {}

void pen_csr_base::write() {
    PLOG_ERR("pen_csr_base:: write not to be used" << endl);
}

void pen_csr_base::read() {
    PLOG_ERR("pen_csr_base:: read not to be used" << endl);
}
void pen_csr_base::write_hw(const cpp_int &, int block_write) {
    PLOG_ERR("pen_csr_base:: write_hw not to be used" << endl);
}
cpp_int pen_csr_base::read_hw(int block_read) {
    PLOG_ERR("pen_csr_base:: read_hw not to be used" << endl);
    return 0;
}
void pen_csr_base::read_compare(int block_read) {
    PLOG_ERR("pen_csr_base:: read_compare not to be used" << endl);
}
bool pen_csr_base::search_field_in_db(string _name) {
    PLOG_ERR("pen_csr_base:: search_field_in_db not to be used" << endl);
    return 0;
}

unsigned pen_csr_base::diff(pen_csr_base * rhs, bool report_error) {
    PLOG_ERR("pen_csr_base:: diff not to be used" << endl);
    return 0;
}
void pen_csr_base::load_from_cfg(bool do_write , bool load_fields ) {
    PLOG_ERR("pen_csr_base:: load_from_cfg not to be used" << endl);
}

pen_csr_base * pen_csr_base::search_csr_by_name(string _name, bool ingore_error ) {
    PLOG_ERR("pen_csr_base:: search_csr_by_name not to be used" << endl);
    return 0;
}
pen_csr_base * pen_csr_base::search_csr_by_addr(uint64_t _addr) {
    PLOG_ERR("pen_csr_base:: search_csr_by_addr not to be used" << endl);
    return 0;
}
vector<string> pen_csr_base::get_fields(int level) const {
    vector<string> ret;
    PLOG_ERR("pen_csr_base:: get_fields not to be used" << endl);
    return ret;
}
bool pen_csr_base::get_field_val(string field_name, cpp_int & val, int level) {
    PLOG_ERR("pen_csr_base:: get_field_val not to be used" << endl);
    return 0;
}
bool pen_csr_base::set_field_val(string field_name, cpp_int & val, int level) {
    PLOG_ERR("pen_csr_base:: set_field_val not to be used" << endl);
    return 0;
}
void pen_csr_base::field_show() {
    PLOG_ERR("pen_csr_base:: field_show not to be used" << endl);
}

pen_decoder_base::pen_decoder_base(string _name, pen_csr_base * _parent):
    pen_csr_base(_name, _parent) {
        set_csr_type(CSR_TYPE_DECODER);
}
pen_decoder_base::~pen_decoder_base() { }

