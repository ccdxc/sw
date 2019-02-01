
#include "cap_pr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pr_csr_int_reg1_int_enable_clear_t::cap_pr_csr_int_reg1_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_int_reg1_int_enable_clear_t::~cap_pr_csr_int_reg1_int_enable_clear_t() { }

cap_pr_csr_intreg_t::cap_pr_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_intreg_t::~cap_pr_csr_intreg_t() { }

cap_pr_csr_intgrp_t::cap_pr_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_intgrp_t::~cap_pr_csr_intgrp_t() { }

cap_pr_csr_intreg_status_t::cap_pr_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_intreg_status_t::~cap_pr_csr_intreg_status_t() { }

cap_pr_csr_int_groups_int_enable_rw_reg_t::cap_pr_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_int_groups_int_enable_rw_reg_t::~cap_pr_csr_int_groups_int_enable_rw_reg_t() { }

cap_pr_csr_intgrp_status_t::cap_pr_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_intgrp_status_t::~cap_pr_csr_intgrp_status_t() { }

cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t() { }

cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t() { }

cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t() { }

cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t() { }

cap_pr_csr_cfg_uid2sidLL_t::cap_pr_csr_cfg_uid2sidLL_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_cfg_uid2sidLL_t::~cap_pr_csr_cfg_uid2sidLL_t() { }

cap_pr_csr_base_t::cap_pr_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_base_t::~cap_pr_csr_base_t() { }

cap_pr_csr_csr_intr_t::cap_pr_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_csr_intr_t::~cap_pr_csr_csr_intr_t() { }

cap_pr_csr_t::cap_pr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(262144);
        set_attributes(0,get_name(), 0);
        }
cap_pr_csr_t::~cap_pr_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_int_reg1_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".slave_prd_enable: 0x" << int_var__slave_prd_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slave_psp_enable: 0x" << int_var__slave_psp_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".slave_prd_interrupt: 0x" << int_var__slave_prd_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slave_psp_interrupt: 0x" << int_var__slave_psp_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_reg1_interrupt: 0x" << int_var__int_reg1_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_reg1_enable: 0x" << int_var__int_reg1_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msk: 0x" << int_var__msk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msk: 0x" << int_var__msk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msk: 0x" << int_var__msk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".msk: 0x" << int_var__msk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_cfg_uid2sidLL_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mode: 0x" << int_var__mode << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".base: 0x" << int_var__base << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pr_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    csr_intr.show();
    base.show();
    cfg_uid2sidLL.show();
    cfg_uid2sidLL_hbm_hash_msk_bit0.show();
    cfg_uid2sidLL_hbm_hash_msk_bit1.show();
    cfg_uid2sidLL_hbm_hash_msk_bit2.show();
    cfg_uid2sidLL_hbm_hash_msk_bit3.show();
    psp.show();
    prd.show();
    int_groups.show();
    int_reg1.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_pr_csr_int_reg1_int_enable_clear_t::get_width() const {
    return cap_pr_csr_int_reg1_int_enable_clear_t::s_get_width();

}

int cap_pr_csr_intreg_t::get_width() const {
    return cap_pr_csr_intreg_t::s_get_width();

}

int cap_pr_csr_intgrp_t::get_width() const {
    return cap_pr_csr_intgrp_t::s_get_width();

}

int cap_pr_csr_intreg_status_t::get_width() const {
    return cap_pr_csr_intreg_status_t::s_get_width();

}

int cap_pr_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_pr_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_pr_csr_intgrp_status_t::get_width() const {
    return cap_pr_csr_intgrp_status_t::s_get_width();

}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::get_width() const {
    return cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::s_get_width();

}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::get_width() const {
    return cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::s_get_width();

}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::get_width() const {
    return cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::s_get_width();

}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::get_width() const {
    return cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::s_get_width();

}

int cap_pr_csr_cfg_uid2sidLL_t::get_width() const {
    return cap_pr_csr_cfg_uid2sidLL_t::s_get_width();

}

int cap_pr_csr_base_t::get_width() const {
    return cap_pr_csr_base_t::s_get_width();

}

int cap_pr_csr_csr_intr_t::get_width() const {
    return cap_pr_csr_csr_intr_t::s_get_width();

}

int cap_pr_csr_t::get_width() const {
    return cap_pr_csr_t::s_get_width();

}

int cap_pr_csr_int_reg1_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // slave_prd_enable
    _count += 1; // slave_psp_enable
    return _count;
}

int cap_pr_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // slave_prd_interrupt
    _count += 1; // slave_psp_interrupt
    return _count;
}

int cap_pr_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_pr_csr_intreg_t::s_get_width(); // intreg
    _count += cap_pr_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_pr_csr_int_reg1_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_pr_csr_int_reg1_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_pr_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_reg1_interrupt
    return _count;
}

int cap_pr_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_reg1_enable
    return _count;
}

int cap_pr_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_pr_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_pr_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_pr_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::s_get_width() {
    int _count = 0;

    _count += 40; // msk
    return _count;
}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::s_get_width() {
    int _count = 0;

    _count += 40; // msk
    return _count;
}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::s_get_width() {
    int _count = 0;

    _count += 40; // msk
    return _count;
}

int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::s_get_width() {
    int _count = 0;

    _count += 40; // msk
    return _count;
}

int cap_pr_csr_cfg_uid2sidLL_t::s_get_width() {
    int _count = 0;

    _count += 7; // spare
    _count += 2; // mode
    _count += 7; // base
    return _count;
}

int cap_pr_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pr_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_pr_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pr_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_pr_csr_base_t::s_get_width(); // base
    _count += cap_pr_csr_cfg_uid2sidLL_t::s_get_width(); // cfg_uid2sidLL
    _count += cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::s_get_width(); // cfg_uid2sidLL_hbm_hash_msk_bit0
    _count += cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::s_get_width(); // cfg_uid2sidLL_hbm_hash_msk_bit1
    _count += cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::s_get_width(); // cfg_uid2sidLL_hbm_hash_msk_bit2
    _count += cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::s_get_width(); // cfg_uid2sidLL_hbm_hash_msk_bit3
    _count += cap_psp_csr_t::s_get_width(); // psp
    _count += cap_prd_csr_t::s_get_width(); // prd
    _count += cap_pr_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_pr_csr_intgrp_t::s_get_width(); // int_reg1
    return _count;
}

void cap_pr_csr_int_reg1_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__slave_prd_enable = _val.convert_to< slave_prd_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__slave_psp_enable = _val.convert_to< slave_psp_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pr_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__slave_prd_interrupt = _val.convert_to< slave_prd_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__slave_psp_interrupt = _val.convert_to< slave_psp_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pr_csr_intgrp_t::all(const cpp_int & in_val) {
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

void cap_pr_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_reg1_interrupt = _val.convert_to< int_reg1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pr_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_reg1_enable = _val.convert_to< int_reg1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pr_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msk = _val.convert_to< msk_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msk = _val.convert_to< msk_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msk = _val.convert_to< msk_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__msk = _val.convert_to< msk_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pr_csr_cfg_uid2sidLL_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__mode = _val.convert_to< mode_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__base = _val.convert_to< base_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_pr_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_pr_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pr_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_uid2sidLL.all( _val);
    _val = _val >> cfg_uid2sidLL.get_width(); 
    cfg_uid2sidLL_hbm_hash_msk_bit0.all( _val);
    _val = _val >> cfg_uid2sidLL_hbm_hash_msk_bit0.get_width(); 
    cfg_uid2sidLL_hbm_hash_msk_bit1.all( _val);
    _val = _val >> cfg_uid2sidLL_hbm_hash_msk_bit1.get_width(); 
    cfg_uid2sidLL_hbm_hash_msk_bit2.all( _val);
    _val = _val >> cfg_uid2sidLL_hbm_hash_msk_bit2.get_width(); 
    cfg_uid2sidLL_hbm_hash_msk_bit3.all( _val);
    _val = _val >> cfg_uid2sidLL_hbm_hash_msk_bit3.get_width(); 
    psp.all( _val);
    _val = _val >> psp.get_width(); 
    prd.all( _val);
    _val = _val >> prd.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_reg1.all( _val);
    _val = _val >> int_reg1.get_width(); 
}

cpp_int cap_pr_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_reg1.get_width(); ret_val = ret_val  | int_reg1.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << prd.get_width(); ret_val = ret_val  | prd.all(); 
    ret_val = ret_val << psp.get_width(); ret_val = ret_val  | psp.all(); 
    ret_val = ret_val << cfg_uid2sidLL_hbm_hash_msk_bit3.get_width(); ret_val = ret_val  | cfg_uid2sidLL_hbm_hash_msk_bit3.all(); 
    ret_val = ret_val << cfg_uid2sidLL_hbm_hash_msk_bit2.get_width(); ret_val = ret_val  | cfg_uid2sidLL_hbm_hash_msk_bit2.all(); 
    ret_val = ret_val << cfg_uid2sidLL_hbm_hash_msk_bit1.get_width(); ret_val = ret_val  | cfg_uid2sidLL_hbm_hash_msk_bit1.all(); 
    ret_val = ret_val << cfg_uid2sidLL_hbm_hash_msk_bit0.get_width(); ret_val = ret_val  | cfg_uid2sidLL_hbm_hash_msk_bit0.all(); 
    ret_val = ret_val << cfg_uid2sidLL.get_width(); ret_val = ret_val  | cfg_uid2sidLL.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    return ret_val;
}

cpp_int cap_pr_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_pr_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_pr_csr_cfg_uid2sidLL_t::all() const {
    cpp_int ret_val;

    // base
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__base; 
    
    // mode
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__mode; 
    
    // spare
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::all() const {
    cpp_int ret_val;

    // msk
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__msk; 
    
    return ret_val;
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::all() const {
    cpp_int ret_val;

    // msk
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__msk; 
    
    return ret_val;
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::all() const {
    cpp_int ret_val;

    // msk
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__msk; 
    
    return ret_val;
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::all() const {
    cpp_int ret_val;

    // msk
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__msk; 
    
    return ret_val;
}

cpp_int cap_pr_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_pr_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_reg1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_reg1_interrupt; 
    
    return ret_val;
}

cpp_int cap_pr_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_reg1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_reg1_enable; 
    
    return ret_val;
}

cpp_int cap_pr_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_pr_csr_intreg_t::all() const {
    cpp_int ret_val;

    // slave_psp_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__slave_psp_interrupt; 
    
    // slave_prd_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__slave_prd_interrupt; 
    
    return ret_val;
}

cpp_int cap_pr_csr_int_reg1_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // slave_psp_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__slave_psp_enable; 
    
    // slave_prd_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__slave_prd_enable; 
    
    return ret_val;
}

void cap_pr_csr_int_reg1_int_enable_clear_t::clear() {

    int_var__slave_prd_enable = 0; 
    
    int_var__slave_psp_enable = 0; 
    
}

void cap_pr_csr_intreg_t::clear() {

    int_var__slave_prd_interrupt = 0; 
    
    int_var__slave_psp_interrupt = 0; 
    
}

void cap_pr_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_pr_csr_intreg_status_t::clear() {

    int_var__int_reg1_interrupt = 0; 
    
}

void cap_pr_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_reg1_enable = 0; 
    
}

void cap_pr_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::clear() {

    int_var__msk = 0; 
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::clear() {

    int_var__msk = 0; 
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::clear() {

    int_var__msk = 0; 
    
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::clear() {

    int_var__msk = 0; 
    
}

void cap_pr_csr_cfg_uid2sidLL_t::clear() {

    int_var__spare = 0; 
    
    int_var__mode = 0; 
    
    int_var__base = 0; 
    
}

void cap_pr_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_pr_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_pr_csr_t::clear() {

    csr_intr.clear();
    base.clear();
    cfg_uid2sidLL.clear();
    cfg_uid2sidLL_hbm_hash_msk_bit0.clear();
    cfg_uid2sidLL_hbm_hash_msk_bit1.clear();
    cfg_uid2sidLL_hbm_hash_msk_bit2.clear();
    cfg_uid2sidLL_hbm_hash_msk_bit3.clear();
    psp.clear();
    prd.clear();
    int_groups.clear();
    int_reg1.clear();
}

void cap_pr_csr_int_reg1_int_enable_clear_t::init() {

}

void cap_pr_csr_intreg_t::init() {

}

void cap_pr_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_pr_csr_intreg_status_t::init() {

}

void cap_pr_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_pr_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::init() {

    set_reset_val(cpp_int("0x8888888800"));
    all(get_reset_val());
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::init() {

    set_reset_val(cpp_int("0x4444444400"));
    all(get_reset_val());
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::init() {

    set_reset_val(cpp_int("0x2222222200"));
    all(get_reset_val());
}

void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::init() {

    set_reset_val(cpp_int("0x1111111100"));
    all(get_reset_val());
}

void cap_pr_csr_cfg_uid2sidLL_t::init() {

}

void cap_pr_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pr_csr_csr_intr_t::init() {

}

void cap_pr_csr_t::init() {

    csr_intr.set_attributes(this,"csr_intr", 0x20400 );
    base.set_attributes(this,"base", 0x20430 );
    cfg_uid2sidLL.set_attributes(this,"cfg_uid2sidLL", 0x20434 );
    cfg_uid2sidLL_hbm_hash_msk_bit0.set_attributes(this,"cfg_uid2sidLL_hbm_hash_msk_bit0", 0x20438 );
    cfg_uid2sidLL_hbm_hash_msk_bit1.set_attributes(this,"cfg_uid2sidLL_hbm_hash_msk_bit1", 0x20440 );
    cfg_uid2sidLL_hbm_hash_msk_bit2.set_attributes(this,"cfg_uid2sidLL_hbm_hash_msk_bit2", 0x20448 );
    cfg_uid2sidLL_hbm_hash_msk_bit3.set_attributes(this,"cfg_uid2sidLL_hbm_hash_msk_bit3", 0x20450 );
    psp.set_attributes(this,"psp", 0x0 );
    prd.set_attributes(this,"prd", 0x20000 );
    int_groups.set_attributes(this,"int_groups", 0x20410 );
    int_reg1.set_attributes(this,"int_reg1", 0x20420 );
}

void cap_pr_csr_int_reg1_int_enable_clear_t::slave_prd_enable(const cpp_int & _val) { 
    // slave_prd_enable
    int_var__slave_prd_enable = _val.convert_to< slave_prd_enable_cpp_int_t >();
}

cpp_int cap_pr_csr_int_reg1_int_enable_clear_t::slave_prd_enable() const {
    return int_var__slave_prd_enable;
}
    
void cap_pr_csr_int_reg1_int_enable_clear_t::slave_psp_enable(const cpp_int & _val) { 
    // slave_psp_enable
    int_var__slave_psp_enable = _val.convert_to< slave_psp_enable_cpp_int_t >();
}

cpp_int cap_pr_csr_int_reg1_int_enable_clear_t::slave_psp_enable() const {
    return int_var__slave_psp_enable;
}
    
void cap_pr_csr_intreg_t::slave_prd_interrupt(const cpp_int & _val) { 
    // slave_prd_interrupt
    int_var__slave_prd_interrupt = _val.convert_to< slave_prd_interrupt_cpp_int_t >();
}

cpp_int cap_pr_csr_intreg_t::slave_prd_interrupt() const {
    return int_var__slave_prd_interrupt;
}
    
void cap_pr_csr_intreg_t::slave_psp_interrupt(const cpp_int & _val) { 
    // slave_psp_interrupt
    int_var__slave_psp_interrupt = _val.convert_to< slave_psp_interrupt_cpp_int_t >();
}

cpp_int cap_pr_csr_intreg_t::slave_psp_interrupt() const {
    return int_var__slave_psp_interrupt;
}
    
void cap_pr_csr_intreg_status_t::int_reg1_interrupt(const cpp_int & _val) { 
    // int_reg1_interrupt
    int_var__int_reg1_interrupt = _val.convert_to< int_reg1_interrupt_cpp_int_t >();
}

cpp_int cap_pr_csr_intreg_status_t::int_reg1_interrupt() const {
    return int_var__int_reg1_interrupt;
}
    
void cap_pr_csr_int_groups_int_enable_rw_reg_t::int_reg1_enable(const cpp_int & _val) { 
    // int_reg1_enable
    int_var__int_reg1_enable = _val.convert_to< int_reg1_enable_cpp_int_t >();
}

cpp_int cap_pr_csr_int_groups_int_enable_rw_reg_t::int_reg1_enable() const {
    return int_var__int_reg1_enable;
}
    
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::msk(const cpp_int & _val) { 
    // msk
    int_var__msk = _val.convert_to< msk_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::msk() const {
    return int_var__msk;
}
    
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::msk(const cpp_int & _val) { 
    // msk
    int_var__msk = _val.convert_to< msk_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::msk() const {
    return int_var__msk;
}
    
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::msk(const cpp_int & _val) { 
    // msk
    int_var__msk = _val.convert_to< msk_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::msk() const {
    return int_var__msk;
}
    
void cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::msk(const cpp_int & _val) { 
    // msk
    int_var__msk = _val.convert_to< msk_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::msk() const {
    return int_var__msk;
}
    
void cap_pr_csr_cfg_uid2sidLL_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_t::spare() const {
    return int_var__spare;
}
    
void cap_pr_csr_cfg_uid2sidLL_t::mode(const cpp_int & _val) { 
    // mode
    int_var__mode = _val.convert_to< mode_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_t::mode() const {
    return int_var__mode;
}
    
void cap_pr_csr_cfg_uid2sidLL_t::base(const cpp_int & _val) { 
    // base
    int_var__base = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pr_csr_cfg_uid2sidLL_t::base() const {
    return int_var__base;
}
    
void cap_pr_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pr_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
void cap_pr_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_pr_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_pr_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_pr_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_int_reg1_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slave_prd_enable")) { field_val = slave_prd_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slave_psp_enable")) { field_val = slave_psp_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slave_prd_interrupt")) { field_val = slave_prd_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slave_psp_interrupt")) { field_val = slave_psp_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_pr_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_reg1_interrupt")) { field_val = int_reg1_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_reg1_enable")) { field_val = int_reg1_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { field_val = msk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { field_val = msk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { field_val = msk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { field_val = msk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mode")) { field_val = mode(); field_found=1; }
    if(!field_found && !strcmp(field_name, "base")) { field_val = base(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit3.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = psp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = prd.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_reg1.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_int_reg1_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slave_prd_enable")) { slave_prd_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slave_psp_enable")) { slave_psp_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "slave_prd_interrupt")) { slave_prd_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slave_psp_interrupt")) { slave_psp_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_pr_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_reg1_interrupt")) { int_reg1_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_reg1_enable")) { int_reg1_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { msk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { msk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { msk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "msk")) { msk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_cfg_uid2sidLL_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mode")) { mode(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "base")) { base(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pr_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_uid2sidLL_hbm_hash_msk_bit3.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = psp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = prd.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_reg1.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_int_reg1_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("slave_prd_enable");
    ret_vec.push_back("slave_psp_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("slave_prd_interrupt");
    ret_vec.push_back("slave_psp_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_intgrp_t::get_fields(int level) const { 
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
std::vector<string> cap_pr_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_reg1_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_reg1_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("msk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_cfg_uid2sidLL_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("mode");
    ret_vec.push_back("base");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pr_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_uid2sidLL.get_fields(level-1)) {
            ret_vec.push_back("cfg_uid2sidLL." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_uid2sidLL_hbm_hash_msk_bit0.get_fields(level-1)) {
            ret_vec.push_back("cfg_uid2sidLL_hbm_hash_msk_bit0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_uid2sidLL_hbm_hash_msk_bit1.get_fields(level-1)) {
            ret_vec.push_back("cfg_uid2sidLL_hbm_hash_msk_bit1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_uid2sidLL_hbm_hash_msk_bit2.get_fields(level-1)) {
            ret_vec.push_back("cfg_uid2sidLL_hbm_hash_msk_bit2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_uid2sidLL_hbm_hash_msk_bit3.get_fields(level-1)) {
            ret_vec.push_back("cfg_uid2sidLL_hbm_hash_msk_bit3." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : psp.get_fields(level-1)) {
            ret_vec.push_back("psp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : prd.get_fields(level-1)) {
            ret_vec.push_back("prd." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_reg1.get_fields(level-1)) {
            ret_vec.push_back("int_reg1." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
