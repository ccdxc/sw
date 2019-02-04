
#include "cap_mc_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_mc_csr_int_mc_int_enable_clear_t::cap_mc_csr_int_mc_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_int_mc_int_enable_clear_t::~cap_mc_csr_int_mc_int_enable_clear_t() { }

cap_mc_csr_intreg_t::cap_mc_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_intreg_t::~cap_mc_csr_intreg_t() { }

cap_mc_csr_intgrp_t::cap_mc_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_intgrp_t::~cap_mc_csr_intgrp_t() { }

cap_mc_csr_intreg_status_t::cap_mc_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_intreg_status_t::~cap_mc_csr_intreg_status_t() { }

cap_mc_csr_int_groups_int_enable_rw_reg_t::cap_mc_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_int_groups_int_enable_rw_reg_t::~cap_mc_csr_int_groups_int_enable_rw_reg_t() { }

cap_mc_csr_intgrp_status_t::cap_mc_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_intgrp_status_t::~cap_mc_csr_intgrp_status_t() { }

cap_mc_csr_csr_intr_t::cap_mc_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_csr_intr_t::~cap_mc_csr_csr_intr_t() { }

cap_mc_csr_mc_cfg_t::cap_mc_csr_mc_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mc_csr_mc_cfg_t::~cap_mc_csr_mc_cfg_t() { }

cap_mc_csr_t::cap_mc_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_mc_csr_t::~cap_mc_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_int_mc_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mch_int_enable: 0x" << int_var__mch_int_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mch_int_interrupt: 0x" << int_var__mch_int_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mc_interrupt: 0x" << int_var__int_mc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mc_enable: 0x" << int_var__int_mc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_mc_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_enable: 0x" << int_var__debug_port_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_select: 0x" << int_var__debug_port_select << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mc_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    mc_cfg.show();
    csr_intr.show();
    mch.show();
    int_groups.show();
    int_mc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_mc_csr_int_mc_int_enable_clear_t::get_width() const {
    return cap_mc_csr_int_mc_int_enable_clear_t::s_get_width();

}

int cap_mc_csr_intreg_t::get_width() const {
    return cap_mc_csr_intreg_t::s_get_width();

}

int cap_mc_csr_intgrp_t::get_width() const {
    return cap_mc_csr_intgrp_t::s_get_width();

}

int cap_mc_csr_intreg_status_t::get_width() const {
    return cap_mc_csr_intreg_status_t::s_get_width();

}

int cap_mc_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_mc_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_mc_csr_intgrp_status_t::get_width() const {
    return cap_mc_csr_intgrp_status_t::s_get_width();

}

int cap_mc_csr_csr_intr_t::get_width() const {
    return cap_mc_csr_csr_intr_t::s_get_width();

}

int cap_mc_csr_mc_cfg_t::get_width() const {
    return cap_mc_csr_mc_cfg_t::s_get_width();

}

int cap_mc_csr_t::get_width() const {
    return cap_mc_csr_t::s_get_width();

}

int cap_mc_csr_int_mc_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // mch_int_enable
    return _count;
}

int cap_mc_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // mch_int_interrupt
    return _count;
}

int cap_mc_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_mc_csr_intreg_t::s_get_width(); // intreg
    _count += cap_mc_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_mc_csr_int_mc_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mc_csr_int_mc_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mc_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mc_interrupt
    return _count;
}

int cap_mc_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mc_enable
    return _count;
}

int cap_mc_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_mc_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_mc_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_mc_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_mc_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_mc_csr_mc_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // debug_port_enable
    _count += 1; // debug_port_select
    return _count;
}

int cap_mc_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_mc_csr_mc_cfg_t::s_get_width(); // mc_cfg
    _count += cap_mc_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_mch_csr_t::s_get_width(); // mch
    _count += cap_mc_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_mc_csr_intgrp_t::s_get_width(); // int_mc
    return _count;
}

void cap_mc_csr_int_mc_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mch_int_enable = _val.convert_to< mch_int_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mch_int_interrupt = _val.convert_to< mch_int_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_intgrp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_test_set.all( _val);
    _val = _val >> int_test_set.get_width(); 
    int_enable_set.all( _val);
    _val = _val >> int_enable_set.get_width(); 
    int_enable_clear.all( _val);
    _val = _val >> int_enable_clear.get_width(); 
}

void cap_mc_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mc_interrupt = _val.convert_to< int_mc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mc_enable = _val.convert_to< int_mc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_mc_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_mc_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mc_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    mc_cfg.all( _val);
    _val = _val >> mc_cfg.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    mch.all( _val);
    _val = _val >> mch.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_mc.all( _val);
    _val = _val >> int_mc.get_width(); 
}

cpp_int cap_mc_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_mc.get_width(); ret_val = ret_val  | int_mc.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << mch.get_width(); ret_val = ret_val  | mch.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << mc_cfg.get_width(); ret_val = ret_val  | mc_cfg.all(); 
    return ret_val;
}

cpp_int cap_mc_csr_mc_cfg_t::all() const {
    cpp_int ret_val;

    // debug_port_select
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_select; 
    
    // debug_port_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_enable; 
    
    return ret_val;
}

cpp_int cap_mc_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_mc_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mc_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_mc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mc_interrupt; 
    
    return ret_val;
}

cpp_int cap_mc_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_mc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mc_enable; 
    
    return ret_val;
}

cpp_int cap_mc_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mc_csr_intreg_t::all() const {
    cpp_int ret_val;

    // mch_int_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mch_int_interrupt; 
    
    return ret_val;
}

cpp_int cap_mc_csr_int_mc_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // mch_int_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mch_int_enable; 
    
    return ret_val;
}

void cap_mc_csr_int_mc_int_enable_clear_t::clear() {

    int_var__mch_int_enable = 0; 
    
}

void cap_mc_csr_intreg_t::clear() {

    int_var__mch_int_interrupt = 0; 
    
}

void cap_mc_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mc_csr_intreg_status_t::clear() {

    int_var__int_mc_interrupt = 0; 
    
}

void cap_mc_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_mc_enable = 0; 
    
}

void cap_mc_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_mc_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_mc_csr_mc_cfg_t::clear() {

    int_var__debug_port_enable = 0; 
    
    int_var__debug_port_select = 0; 
    
}

void cap_mc_csr_t::clear() {

    mc_cfg.clear();
    csr_intr.clear();
    mch.clear();
    int_groups.clear();
    int_mc.clear();
}

void cap_mc_csr_int_mc_int_enable_clear_t::init() {

}

void cap_mc_csr_intreg_t::init() {

}

void cap_mc_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mc_csr_intreg_status_t::init() {

}

void cap_mc_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_mc_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_mc_csr_csr_intr_t::init() {

}

void cap_mc_csr_mc_cfg_t::init() {

}

void cap_mc_csr_t::init() {

    mc_cfg.set_attributes(this,"mc_cfg", 0x80000 );
    csr_intr.set_attributes(this,"csr_intr", 0x80004 );
    mch.set_attributes(this,"mch", 0x0 );
    int_groups.set_attributes(this,"int_groups", 0x80010 );
    int_mc.set_attributes(this,"int_mc", 0x80020 );
}

void cap_mc_csr_int_mc_int_enable_clear_t::mch_int_enable(const cpp_int & _val) { 
    // mch_int_enable
    int_var__mch_int_enable = _val.convert_to< mch_int_enable_cpp_int_t >();
}

cpp_int cap_mc_csr_int_mc_int_enable_clear_t::mch_int_enable() const {
    return int_var__mch_int_enable;
}
    
void cap_mc_csr_intreg_t::mch_int_interrupt(const cpp_int & _val) { 
    // mch_int_interrupt
    int_var__mch_int_interrupt = _val.convert_to< mch_int_interrupt_cpp_int_t >();
}

cpp_int cap_mc_csr_intreg_t::mch_int_interrupt() const {
    return int_var__mch_int_interrupt;
}
    
void cap_mc_csr_intreg_status_t::int_mc_interrupt(const cpp_int & _val) { 
    // int_mc_interrupt
    int_var__int_mc_interrupt = _val.convert_to< int_mc_interrupt_cpp_int_t >();
}

cpp_int cap_mc_csr_intreg_status_t::int_mc_interrupt() const {
    return int_var__int_mc_interrupt;
}
    
void cap_mc_csr_int_groups_int_enable_rw_reg_t::int_mc_enable(const cpp_int & _val) { 
    // int_mc_enable
    int_var__int_mc_enable = _val.convert_to< int_mc_enable_cpp_int_t >();
}

cpp_int cap_mc_csr_int_groups_int_enable_rw_reg_t::int_mc_enable() const {
    return int_var__int_mc_enable;
}
    
void cap_mc_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_mc_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_mc_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_mc_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_mc_csr_mc_cfg_t::debug_port_enable(const cpp_int & _val) { 
    // debug_port_enable
    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >();
}

cpp_int cap_mc_csr_mc_cfg_t::debug_port_enable() const {
    return int_var__debug_port_enable;
}
    
void cap_mc_csr_mc_cfg_t::debug_port_select(const cpp_int & _val) { 
    // debug_port_select
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >();
}

cpp_int cap_mc_csr_mc_cfg_t::debug_port_select() const {
    return int_var__debug_port_select;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_int_mc_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mch_int_enable")) { field_val = mch_int_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mch_int_interrupt")) { field_val = mch_int_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_interrupt")) { field_val = int_mc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_enable")) { field_val = int_mc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_mc_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { field_val = debug_port_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { field_val = debug_port_select(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = mc_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_int_mc_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mch_int_enable")) { mch_int_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mch_int_interrupt")) { mch_int_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_interrupt")) { int_mc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_enable")) { int_mc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_mc_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { debug_port_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { debug_port_select(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mc_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = mc_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_int_mc_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mch_int_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mch_int_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_intgrp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_test_set.get_fields(level-1)) {
            ret_vec.push_back("int_test_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_set.get_fields(level-1)) {
            ret_vec.push_back("int_enable_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_clear.get_fields(level-1)) {
            ret_vec.push_back("int_enable_clear." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_intgrp_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_enable_rw_reg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_rw_reg." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_mc_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("debug_port_enable");
    ret_vec.push_back("debug_port_select");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mc_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : mc_cfg.get_fields(level-1)) {
            ret_vec.push_back("mc_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mch.get_fields(level-1)) {
            ret_vec.push_back("mch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_mc.get_fields(level-1)) {
            ret_vec.push_back("int_mc." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
