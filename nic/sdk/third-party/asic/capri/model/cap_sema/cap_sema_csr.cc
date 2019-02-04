
#include "cap_sema_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_sema_csr_sema_err_int_enable_clear_t::cap_sema_csr_sema_err_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_sema_err_int_enable_clear_t::~cap_sema_csr_sema_err_int_enable_clear_t() { }

cap_sema_csr_intreg_t::cap_sema_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_intreg_t::~cap_sema_csr_intreg_t() { }

cap_sema_csr_intgrp_t::cap_sema_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_intgrp_t::~cap_sema_csr_intgrp_t() { }

cap_sema_csr_intreg_status_t::cap_sema_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_intreg_status_t::~cap_sema_csr_intreg_status_t() { }

cap_sema_csr_int_groups_int_enable_rw_reg_t::cap_sema_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_int_groups_int_enable_rw_reg_t::~cap_sema_csr_int_groups_int_enable_rw_reg_t() { }

cap_sema_csr_intgrp_status_t::cap_sema_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_intgrp_status_t::~cap_sema_csr_intgrp_status_t() { }

cap_sema_csr_atomic_add_entry_t::cap_sema_csr_atomic_add_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_atomic_add_entry_t::~cap_sema_csr_atomic_add_entry_t() { }

cap_sema_csr_atomic_add_t::cap_sema_csr_atomic_add_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_sema_csr_atomic_add_t::~cap_sema_csr_atomic_add_t() { }

cap_sema_csr_semaphore_inc_not_full_entry_t::cap_sema_csr_semaphore_inc_not_full_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_inc_not_full_entry_t::~cap_sema_csr_semaphore_inc_not_full_entry_t() { }

cap_sema_csr_semaphore_inc_not_full_t::cap_sema_csr_semaphore_inc_not_full_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_inc_not_full_t::~cap_sema_csr_semaphore_inc_not_full_t() { }

cap_sema_csr_semaphore_dec_entry_t::cap_sema_csr_semaphore_dec_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_dec_entry_t::~cap_sema_csr_semaphore_dec_entry_t() { }

cap_sema_csr_semaphore_dec_t::cap_sema_csr_semaphore_dec_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_dec_t::~cap_sema_csr_semaphore_dec_t() { }

cap_sema_csr_semaphore_inc_entry_t::cap_sema_csr_semaphore_inc_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_inc_entry_t::~cap_sema_csr_semaphore_inc_entry_t() { }

cap_sema_csr_semaphore_inc_t::cap_sema_csr_semaphore_inc_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_inc_t::~cap_sema_csr_semaphore_inc_t() { }

cap_sema_csr_semaphore_raw_entry_t::cap_sema_csr_semaphore_raw_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_raw_entry_t::~cap_sema_csr_semaphore_raw_entry_t() { }

cap_sema_csr_semaphore_raw_t::cap_sema_csr_semaphore_raw_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_sema_csr_semaphore_raw_t::~cap_sema_csr_semaphore_raw_t() { }

cap_sema_csr_STA_sema_t::cap_sema_csr_STA_sema_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_STA_sema_t::~cap_sema_csr_STA_sema_t() { }

cap_sema_csr_sta_sema_mem_t::cap_sema_csr_sta_sema_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_sta_sema_mem_t::~cap_sema_csr_sta_sema_mem_t() { }

cap_sema_csr_cfg_sema_mem_t::cap_sema_csr_cfg_sema_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_cfg_sema_mem_t::~cap_sema_csr_cfg_sema_mem_t() { }

cap_sema_csr_STA_err_resp_t::cap_sema_csr_STA_err_resp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_STA_err_resp_t::~cap_sema_csr_STA_err_resp_t() { }

cap_sema_csr_STA_pending_t::cap_sema_csr_STA_pending_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_STA_pending_t::~cap_sema_csr_STA_pending_t() { }

cap_sema_csr_csr_intr_t::cap_sema_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_csr_intr_t::~cap_sema_csr_csr_intr_t() { }

cap_sema_csr_sema_cfg_t::cap_sema_csr_sema_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_sema_cfg_t::~cap_sema_csr_sema_cfg_t() { }

cap_sema_csr_axi_attr_t::cap_sema_csr_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_axi_attr_t::~cap_sema_csr_axi_attr_t() { }

cap_sema_csr_base_t::cap_sema_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_sema_csr_base_t::~cap_sema_csr_base_t() { }

cap_sema_csr_t::cap_sema_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(268435456);
        set_attributes(0,get_name(), 0);
        }
cap_sema_csr_t::~cap_sema_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_sema_err_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_enable: 0x" << int_var__write_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".read_err_enable: 0x" << int_var__read_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_enable: 0x" << int_var__uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_enable: 0x" << int_var__correctable_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_interrupt: 0x" << int_var__write_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".read_err_interrupt: 0x" << int_var__read_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_interrupt: 0x" << int_var__uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_interrupt: 0x" << int_var__correctable_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sema_err_interrupt: 0x" << int_var__sema_err_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sema_err_enable: 0x" << int_var__sema_err_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_atomic_add_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_atomic_add_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 16777216; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_inc_not_full_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".full: 0x" << int_var__full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_inc_not_full_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_dec_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_dec_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_inc_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_inc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_raw_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_semaphore_raw_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_STA_sema_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".atomic_state: 0x" << int_var__atomic_state << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_wready: 0x" << int_var__axi_wready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_awready: 0x" << int_var__axi_awready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_rready: 0x" << int_var__axi_rready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cam_full: 0x" << int_var__cam_full << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cam_hit: 0x" << int_var__cam_hit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr_pending_hit: 0x" << int_var__wr_pending_hit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pending_count: 0x" << int_var__pending_count << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_sta_sema_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_cfg_sema_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_STA_err_resp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_STA_pending_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr: 0x" << int_var__wr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_sema_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".allow_merge: 0x" << int_var__allow_merge << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_axi_attr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".arcache: 0x" << int_var__arcache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awcache: 0x" << int_var__awcache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_sema_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    axi_attr.show();
    sema_cfg.show();
    csr_intr.show();
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    STA_pending.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        STA_pending[ii].show();
    }
    #endif
    
    STA_err_resp.show();
    cfg_sema_mem.show();
    sta_sema_mem.show();
    STA_sema.show();
    semaphore_raw.show();
    semaphore_inc.show();
    semaphore_dec.show();
    semaphore_inc_not_full.show();
    atomic_add.show();
    int_groups.show();
    sema_err.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_sema_csr_sema_err_int_enable_clear_t::get_width() const {
    return cap_sema_csr_sema_err_int_enable_clear_t::s_get_width();

}

int cap_sema_csr_intreg_t::get_width() const {
    return cap_sema_csr_intreg_t::s_get_width();

}

int cap_sema_csr_intgrp_t::get_width() const {
    return cap_sema_csr_intgrp_t::s_get_width();

}

int cap_sema_csr_intreg_status_t::get_width() const {
    return cap_sema_csr_intreg_status_t::s_get_width();

}

int cap_sema_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_sema_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_sema_csr_intgrp_status_t::get_width() const {
    return cap_sema_csr_intgrp_status_t::s_get_width();

}

int cap_sema_csr_atomic_add_entry_t::get_width() const {
    return cap_sema_csr_atomic_add_entry_t::s_get_width();

}

int cap_sema_csr_atomic_add_t::get_width() const {
    return cap_sema_csr_atomic_add_t::s_get_width();

}

int cap_sema_csr_semaphore_inc_not_full_entry_t::get_width() const {
    return cap_sema_csr_semaphore_inc_not_full_entry_t::s_get_width();

}

int cap_sema_csr_semaphore_inc_not_full_t::get_width() const {
    return cap_sema_csr_semaphore_inc_not_full_t::s_get_width();

}

int cap_sema_csr_semaphore_dec_entry_t::get_width() const {
    return cap_sema_csr_semaphore_dec_entry_t::s_get_width();

}

int cap_sema_csr_semaphore_dec_t::get_width() const {
    return cap_sema_csr_semaphore_dec_t::s_get_width();

}

int cap_sema_csr_semaphore_inc_entry_t::get_width() const {
    return cap_sema_csr_semaphore_inc_entry_t::s_get_width();

}

int cap_sema_csr_semaphore_inc_t::get_width() const {
    return cap_sema_csr_semaphore_inc_t::s_get_width();

}

int cap_sema_csr_semaphore_raw_entry_t::get_width() const {
    return cap_sema_csr_semaphore_raw_entry_t::s_get_width();

}

int cap_sema_csr_semaphore_raw_t::get_width() const {
    return cap_sema_csr_semaphore_raw_t::s_get_width();

}

int cap_sema_csr_STA_sema_t::get_width() const {
    return cap_sema_csr_STA_sema_t::s_get_width();

}

int cap_sema_csr_sta_sema_mem_t::get_width() const {
    return cap_sema_csr_sta_sema_mem_t::s_get_width();

}

int cap_sema_csr_cfg_sema_mem_t::get_width() const {
    return cap_sema_csr_cfg_sema_mem_t::s_get_width();

}

int cap_sema_csr_STA_err_resp_t::get_width() const {
    return cap_sema_csr_STA_err_resp_t::s_get_width();

}

int cap_sema_csr_STA_pending_t::get_width() const {
    return cap_sema_csr_STA_pending_t::s_get_width();

}

int cap_sema_csr_csr_intr_t::get_width() const {
    return cap_sema_csr_csr_intr_t::s_get_width();

}

int cap_sema_csr_sema_cfg_t::get_width() const {
    return cap_sema_csr_sema_cfg_t::s_get_width();

}

int cap_sema_csr_axi_attr_t::get_width() const {
    return cap_sema_csr_axi_attr_t::s_get_width();

}

int cap_sema_csr_base_t::get_width() const {
    return cap_sema_csr_base_t::s_get_width();

}

int cap_sema_csr_t::get_width() const {
    return cap_sema_csr_t::s_get_width();

}

int cap_sema_csr_sema_err_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // write_err_enable
    _count += 1; // read_err_enable
    _count += 1; // uncorrectable_enable
    _count += 1; // correctable_enable
    return _count;
}

int cap_sema_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // write_err_interrupt
    _count += 1; // read_err_interrupt
    _count += 1; // uncorrectable_interrupt
    _count += 1; // correctable_interrupt
    return _count;
}

int cap_sema_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_sema_csr_intreg_t::s_get_width(); // intreg
    _count += cap_sema_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_sema_csr_sema_err_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_sema_csr_sema_err_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_sema_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // sema_err_interrupt
    return _count;
}

int cap_sema_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // sema_err_enable
    return _count;
}

int cap_sema_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_sema_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_sema_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_sema_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_sema_csr_atomic_add_entry_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_sema_csr_atomic_add_t::s_get_width() {
    int _count = 0;

    _count += (cap_sema_csr_atomic_add_entry_t::s_get_width() * 16777216); // entry
    return _count;
}

int cap_sema_csr_semaphore_inc_not_full_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // value
    _count += 1; // full
    return _count;
}

int cap_sema_csr_semaphore_inc_not_full_t::s_get_width() {
    int _count = 0;

    _count += (cap_sema_csr_semaphore_inc_not_full_entry_t::s_get_width() * 512); // entry
    return _count;
}

int cap_sema_csr_semaphore_dec_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // value
    return _count;
}

int cap_sema_csr_semaphore_dec_t::s_get_width() {
    int _count = 0;

    _count += (cap_sema_csr_semaphore_dec_entry_t::s_get_width() * 1024); // entry
    return _count;
}

int cap_sema_csr_semaphore_inc_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // value
    return _count;
}

int cap_sema_csr_semaphore_inc_t::s_get_width() {
    int _count = 0;

    _count += (cap_sema_csr_semaphore_inc_entry_t::s_get_width() * 1024); // entry
    return _count;
}

int cap_sema_csr_semaphore_raw_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // value
    return _count;
}

int cap_sema_csr_semaphore_raw_t::s_get_width() {
    int _count = 0;

    _count += (cap_sema_csr_semaphore_raw_entry_t::s_get_width() * 1024); // entry
    return _count;
}

int cap_sema_csr_STA_sema_t::s_get_width() {
    int _count = 0;

    _count += 3; // atomic_state
    _count += 1; // axi_wready
    _count += 1; // axi_awready
    _count += 1; // axi_rready
    _count += 1; // cam_full
    _count += 1; // cam_hit
    _count += 1; // wr_pending_hit
    _count += 7; // pending_count
    return _count;
}

int cap_sema_csr_sta_sema_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // correctable
    _count += 1; // uncorrectable
    _count += 7; // syndrome
    _count += 10; // addr
    return _count;
}

int cap_sema_csr_cfg_sema_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_det
    _count += 1; // ecc_disable_cor
    _count += 1; // bist_run
    return _count;
}

int cap_sema_csr_STA_err_resp_t::s_get_width() {
    int _count = 0;

    _count += 31; // addr
    _count += 2; // value
    return _count;
}

int cap_sema_csr_STA_pending_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 1; // wr
    _count += 31; // addr
    _count += 58; // data
    return _count;
}

int cap_sema_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_sema_csr_sema_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // allow_merge
    return _count;
}

int cap_sema_csr_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // arcache
    _count += 4; // awcache
    _count += 3; // prot
    _count += 4; // qos
    _count += 1; // lock
    return _count;
}

int cap_sema_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_sema_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_sema_csr_base_t::s_get_width(); // base
    _count += cap_sema_csr_axi_attr_t::s_get_width(); // axi_attr
    _count += cap_sema_csr_sema_cfg_t::s_get_width(); // sema_cfg
    _count += cap_sema_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += (cap_sema_csr_STA_pending_t::s_get_width() * 64); // STA_pending
    _count += cap_sema_csr_STA_err_resp_t::s_get_width(); // STA_err_resp
    _count += cap_sema_csr_cfg_sema_mem_t::s_get_width(); // cfg_sema_mem
    _count += cap_sema_csr_sta_sema_mem_t::s_get_width(); // sta_sema_mem
    _count += cap_sema_csr_STA_sema_t::s_get_width(); // STA_sema
    _count += cap_sema_csr_semaphore_raw_t::s_get_width(); // semaphore_raw
    _count += cap_sema_csr_semaphore_inc_t::s_get_width(); // semaphore_inc
    _count += cap_sema_csr_semaphore_dec_t::s_get_width(); // semaphore_dec
    _count += cap_sema_csr_semaphore_inc_not_full_t::s_get_width(); // semaphore_inc_not_full
    _count += cap_sema_csr_atomic_add_t::s_get_width(); // atomic_add
    _count += cap_sema_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_sema_csr_intgrp_t::s_get_width(); // sema_err
    return _count;
}

void cap_sema_csr_sema_err_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__write_err_enable = _val.convert_to< write_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__read_err_enable = _val.convert_to< read_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__uncorrectable_enable = _val.convert_to< uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable_enable = _val.convert_to< correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__write_err_interrupt = _val.convert_to< write_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__read_err_interrupt = _val.convert_to< read_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__uncorrectable_interrupt = _val.convert_to< uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable_interrupt = _val.convert_to< correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_intgrp_t::all(const cpp_int & in_val) {
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

void cap_sema_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sema_err_interrupt = _val.convert_to< sema_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sema_err_enable = _val.convert_to< sema_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_sema_csr_atomic_add_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_sema_csr_atomic_add_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16777216; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_not_full_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__full = _val.convert_to< full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_semaphore_inc_not_full_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_sema_csr_semaphore_dec_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_sema_csr_semaphore_dec_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_sema_csr_semaphore_inc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_sema_csr_semaphore_raw_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_sema_csr_semaphore_raw_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_sema_csr_STA_sema_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__atomic_state = _val.convert_to< atomic_state_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__axi_wready = _val.convert_to< axi_wready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_awready = _val.convert_to< axi_awready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_rready = _val.convert_to< axi_rready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cam_full = _val.convert_to< cam_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cam_hit = _val.convert_to< cam_hit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr_pending_hit = _val.convert_to< wr_pending_hit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pending_count = _val.convert_to< pending_count_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_sema_csr_sta_sema_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_sema_csr_cfg_sema_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_STA_err_resp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_sema_csr_STA_pending_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr = _val.convert_to< wr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 58;
    
}

void cap_sema_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_sema_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__allow_merge = _val.convert_to< allow_merge_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_axi_attr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__arcache = _val.convert_to< arcache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_sema_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_sema_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    axi_attr.all( _val);
    _val = _val >> axi_attr.get_width(); 
    sema_cfg.all( _val);
    _val = _val >> sema_cfg.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // STA_pending
    for(int ii = 0; ii < 64; ii++) {
        STA_pending[ii].all(_val);
        _val = _val >> STA_pending[ii].get_width();
    }
    #endif
    
    STA_err_resp.all( _val);
    _val = _val >> STA_err_resp.get_width(); 
    cfg_sema_mem.all( _val);
    _val = _val >> cfg_sema_mem.get_width(); 
    sta_sema_mem.all( _val);
    _val = _val >> sta_sema_mem.get_width(); 
    STA_sema.all( _val);
    _val = _val >> STA_sema.get_width(); 
    semaphore_raw.all( _val);
    _val = _val >> semaphore_raw.get_width(); 
    semaphore_inc.all( _val);
    _val = _val >> semaphore_inc.get_width(); 
    semaphore_dec.all( _val);
    _val = _val >> semaphore_dec.get_width(); 
    semaphore_inc_not_full.all( _val);
    _val = _val >> semaphore_inc_not_full.get_width(); 
    atomic_add.all( _val);
    _val = _val >> atomic_add.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    sema_err.all( _val);
    _val = _val >> sema_err.get_width(); 
}

cpp_int cap_sema_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << sema_err.get_width(); ret_val = ret_val  | sema_err.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << atomic_add.get_width(); ret_val = ret_val  | atomic_add.all(); 
    ret_val = ret_val << semaphore_inc_not_full.get_width(); ret_val = ret_val  | semaphore_inc_not_full.all(); 
    ret_val = ret_val << semaphore_dec.get_width(); ret_val = ret_val  | semaphore_dec.all(); 
    ret_val = ret_val << semaphore_inc.get_width(); ret_val = ret_val  | semaphore_inc.all(); 
    ret_val = ret_val << semaphore_raw.get_width(); ret_val = ret_val  | semaphore_raw.all(); 
    ret_val = ret_val << STA_sema.get_width(); ret_val = ret_val  | STA_sema.all(); 
    ret_val = ret_val << sta_sema_mem.get_width(); ret_val = ret_val  | sta_sema_mem.all(); 
    ret_val = ret_val << cfg_sema_mem.get_width(); ret_val = ret_val  | cfg_sema_mem.all(); 
    ret_val = ret_val << STA_err_resp.get_width(); ret_val = ret_val  | STA_err_resp.all(); 
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // STA_pending
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << STA_pending[ii].get_width(); ret_val = ret_val  | STA_pending[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sema_cfg.get_width(); ret_val = ret_val  | sema_cfg.all(); 
    ret_val = ret_val << axi_attr.get_width(); ret_val = ret_val  | axi_attr.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_sema_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_sema_csr_axi_attr_t::all() const {
    cpp_int ret_val;

    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // awcache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__awcache; 
    
    // arcache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__arcache; 
    
    return ret_val;
}

cpp_int cap_sema_csr_sema_cfg_t::all() const {
    cpp_int ret_val;

    // allow_merge
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__allow_merge; 
    
    return ret_val;
}

cpp_int cap_sema_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_sema_csr_STA_pending_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 58; ret_val = ret_val  | int_var__data; 
    
    // addr
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__addr; 
    
    // wr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    return ret_val;
}

cpp_int cap_sema_csr_STA_err_resp_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__value; 
    
    // addr
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__addr; 
    
    return ret_val;
}

cpp_int cap_sema_csr_cfg_sema_mem_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    return ret_val;
}

cpp_int cap_sema_csr_sta_sema_mem_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__syndrome; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_sema_csr_STA_sema_t::all() const {
    cpp_int ret_val;

    // pending_count
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__pending_count; 
    
    // wr_pending_hit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr_pending_hit; 
    
    // cam_hit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cam_hit; 
    
    // cam_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cam_full; 
    
    // axi_rready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_rready; 
    
    // axi_awready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_awready; 
    
    // axi_wready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_wready; 
    
    // atomic_state
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__atomic_state; 
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_raw_t::all() const {
    cpp_int ret_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 1024-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_raw_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_inc_t::all() const {
    cpp_int ret_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 1024-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_inc_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_dec_t::all() const {
    cpp_int ret_val;

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 1024-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_dec_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_inc_not_full_t::all() const {
    cpp_int ret_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 512-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_sema_csr_semaphore_inc_not_full_entry_t::all() const {
    cpp_int ret_val;

    // full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__full; 
    
    // value
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_sema_csr_atomic_add_t::all() const {
    cpp_int ret_val;

    #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 16777216-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_sema_csr_atomic_add_entry_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_sema_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_sema_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // sema_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sema_err_interrupt; 
    
    return ret_val;
}

cpp_int cap_sema_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // sema_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sema_err_enable; 
    
    return ret_val;
}

cpp_int cap_sema_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_sema_csr_intreg_t::all() const {
    cpp_int ret_val;

    // correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_interrupt; 
    
    // uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_interrupt; 
    
    // read_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_err_interrupt; 
    
    // write_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_interrupt; 
    
    return ret_val;
}

cpp_int cap_sema_csr_sema_err_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_enable; 
    
    // uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_enable; 
    
    // read_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_err_enable; 
    
    // write_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_enable; 
    
    return ret_val;
}

void cap_sema_csr_sema_err_int_enable_clear_t::clear() {

    int_var__write_err_enable = 0; 
    
    int_var__read_err_enable = 0; 
    
    int_var__uncorrectable_enable = 0; 
    
    int_var__correctable_enable = 0; 
    
}

void cap_sema_csr_intreg_t::clear() {

    int_var__write_err_interrupt = 0; 
    
    int_var__read_err_interrupt = 0; 
    
    int_var__uncorrectable_interrupt = 0; 
    
    int_var__correctable_interrupt = 0; 
    
}

void cap_sema_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_sema_csr_intreg_status_t::clear() {

    int_var__sema_err_interrupt = 0; 
    
}

void cap_sema_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__sema_err_enable = 0; 
    
}

void cap_sema_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_sema_csr_atomic_add_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_sema_csr_atomic_add_t::clear() {

    #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16777216; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_not_full_entry_t::clear() {

    int_var__value = 0; 
    
    int_var__full = 0; 
    
}

void cap_sema_csr_semaphore_inc_not_full_t::clear() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_sema_csr_semaphore_dec_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_sema_csr_semaphore_dec_t::clear() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_sema_csr_semaphore_inc_t::clear() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_sema_csr_semaphore_raw_entry_t::clear() {

    int_var__value = 0; 
    
}

void cap_sema_csr_semaphore_raw_t::clear() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 1024; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_sema_csr_STA_sema_t::clear() {

    int_var__atomic_state = 0; 
    
    int_var__axi_wready = 0; 
    
    int_var__axi_awready = 0; 
    
    int_var__axi_rready = 0; 
    
    int_var__cam_full = 0; 
    
    int_var__cam_hit = 0; 
    
    int_var__wr_pending_hit = 0; 
    
    int_var__pending_count = 0; 
    
}

void cap_sema_csr_sta_sema_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__correctable = 0; 
    
    int_var__uncorrectable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_sema_csr_cfg_sema_mem_t::clear() {

    int_var__ecc_disable_det = 0; 
    
    int_var__ecc_disable_cor = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_sema_csr_STA_err_resp_t::clear() {

    int_var__addr = 0; 
    
    int_var__value = 0; 
    
}

void cap_sema_csr_STA_pending_t::clear() {

    int_var__valid = 0; 
    
    int_var__wr = 0; 
    
    int_var__addr = 0; 
    
    int_var__data = 0; 
    
}

void cap_sema_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_sema_csr_sema_cfg_t::clear() {

    int_var__allow_merge = 0; 
    
}

void cap_sema_csr_axi_attr_t::clear() {

    int_var__arcache = 0; 
    
    int_var__awcache = 0; 
    
    int_var__prot = 0; 
    
    int_var__qos = 0; 
    
    int_var__lock = 0; 
    
}

void cap_sema_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_sema_csr_t::clear() {

    base.clear();
    axi_attr.clear();
    sema_cfg.clear();
    csr_intr.clear();
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // STA_pending
    for(int ii = 0; ii < 64; ii++) {
        STA_pending[ii].clear();
    }
    #endif
    
    STA_err_resp.clear();
    cfg_sema_mem.clear();
    sta_sema_mem.clear();
    STA_sema.clear();
    semaphore_raw.clear();
    semaphore_inc.clear();
    semaphore_dec.clear();
    semaphore_inc_not_full.clear();
    atomic_add.clear();
    int_groups.clear();
    sema_err.clear();
}

void cap_sema_csr_sema_err_int_enable_clear_t::init() {

}

void cap_sema_csr_intreg_t::init() {

}

void cap_sema_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_sema_csr_intreg_status_t::init() {

}

void cap_sema_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_sema_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_sema_csr_atomic_add_entry_t::init() {

}

void cap_sema_csr_atomic_add_t::init() {

    #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 16777216; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_not_full_entry_t::init() {

}

void cap_sema_csr_semaphore_inc_not_full_t::init() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 512; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_sema_csr_semaphore_dec_entry_t::init() {

}

void cap_sema_csr_semaphore_dec_t::init() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 1024; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_sema_csr_semaphore_inc_entry_t::init() {

}

void cap_sema_csr_semaphore_inc_t::init() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 1024; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_sema_csr_semaphore_raw_entry_t::init() {

}

void cap_sema_csr_semaphore_raw_t::init() {

    #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 1024; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_sema_csr_STA_sema_t::init() {

}

void cap_sema_csr_sta_sema_mem_t::init() {

}

void cap_sema_csr_cfg_sema_mem_t::init() {

}

void cap_sema_csr_STA_err_resp_t::init() {

}

void cap_sema_csr_STA_pending_t::init() {

}

void cap_sema_csr_csr_intr_t::init() {

}

void cap_sema_csr_sema_cfg_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_sema_csr_axi_attr_t::init() {

    set_reset_val(cpp_int("0x2ff"));
    all(get_reset_val());
}

void cap_sema_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_sema_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    axi_attr.set_attributes(this,"axi_attr", 0x4 );
    sema_cfg.set_attributes(this,"sema_cfg", 0x8 );
    csr_intr.set_attributes(this,"csr_intr", 0xc );
    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    STA_pending.set_attributes(this, "STA_pending", 0x400);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) STA_pending[ii].set_field_init_done(true, true);
        STA_pending[ii].set_attributes(this,"STA_pending["+to_string(ii)+"]",  0x400 + (STA_pending[ii].get_byte_size()*ii));
    }
    #endif
    
    STA_err_resp.set_attributes(this,"STA_err_resp", 0x800 );
    cfg_sema_mem.set_attributes(this,"cfg_sema_mem", 0x808 );
    sta_sema_mem.set_attributes(this,"sta_sema_mem", 0x80c );
    STA_sema.set_attributes(this,"STA_sema", 0x810 );
    semaphore_raw.set_attributes(this,"semaphore_raw", 0x1000 );
    semaphore_inc.set_attributes(this,"semaphore_inc", 0x2000 );
    semaphore_dec.set_attributes(this,"semaphore_dec", 0x3000 );
    semaphore_inc_not_full.set_attributes(this,"semaphore_inc_not_full", 0x4000 );
    atomic_add.set_attributes(this,"atomic_add", 0x8000000 );
    int_groups.set_attributes(this,"int_groups", 0x10 );
    sema_err.set_attributes(this,"sema_err", 0x20 );
}

void cap_sema_csr_sema_err_int_enable_clear_t::write_err_enable(const cpp_int & _val) { 
    // write_err_enable
    int_var__write_err_enable = _val.convert_to< write_err_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_sema_err_int_enable_clear_t::write_err_enable() const {
    return int_var__write_err_enable;
}
    
void cap_sema_csr_sema_err_int_enable_clear_t::read_err_enable(const cpp_int & _val) { 
    // read_err_enable
    int_var__read_err_enable = _val.convert_to< read_err_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_sema_err_int_enable_clear_t::read_err_enable() const {
    return int_var__read_err_enable;
}
    
void cap_sema_csr_sema_err_int_enable_clear_t::uncorrectable_enable(const cpp_int & _val) { 
    // uncorrectable_enable
    int_var__uncorrectable_enable = _val.convert_to< uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_sema_err_int_enable_clear_t::uncorrectable_enable() const {
    return int_var__uncorrectable_enable;
}
    
void cap_sema_csr_sema_err_int_enable_clear_t::correctable_enable(const cpp_int & _val) { 
    // correctable_enable
    int_var__correctable_enable = _val.convert_to< correctable_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_sema_err_int_enable_clear_t::correctable_enable() const {
    return int_var__correctable_enable;
}
    
void cap_sema_csr_intreg_t::write_err_interrupt(const cpp_int & _val) { 
    // write_err_interrupt
    int_var__write_err_interrupt = _val.convert_to< write_err_interrupt_cpp_int_t >();
}

cpp_int cap_sema_csr_intreg_t::write_err_interrupt() const {
    return int_var__write_err_interrupt;
}
    
void cap_sema_csr_intreg_t::read_err_interrupt(const cpp_int & _val) { 
    // read_err_interrupt
    int_var__read_err_interrupt = _val.convert_to< read_err_interrupt_cpp_int_t >();
}

cpp_int cap_sema_csr_intreg_t::read_err_interrupt() const {
    return int_var__read_err_interrupt;
}
    
void cap_sema_csr_intreg_t::uncorrectable_interrupt(const cpp_int & _val) { 
    // uncorrectable_interrupt
    int_var__uncorrectable_interrupt = _val.convert_to< uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_sema_csr_intreg_t::uncorrectable_interrupt() const {
    return int_var__uncorrectable_interrupt;
}
    
void cap_sema_csr_intreg_t::correctable_interrupt(const cpp_int & _val) { 
    // correctable_interrupt
    int_var__correctable_interrupt = _val.convert_to< correctable_interrupt_cpp_int_t >();
}

cpp_int cap_sema_csr_intreg_t::correctable_interrupt() const {
    return int_var__correctable_interrupt;
}
    
void cap_sema_csr_intreg_status_t::sema_err_interrupt(const cpp_int & _val) { 
    // sema_err_interrupt
    int_var__sema_err_interrupt = _val.convert_to< sema_err_interrupt_cpp_int_t >();
}

cpp_int cap_sema_csr_intreg_status_t::sema_err_interrupt() const {
    return int_var__sema_err_interrupt;
}
    
void cap_sema_csr_int_groups_int_enable_rw_reg_t::sema_err_enable(const cpp_int & _val) { 
    // sema_err_enable
    int_var__sema_err_enable = _val.convert_to< sema_err_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_int_groups_int_enable_rw_reg_t::sema_err_enable() const {
    return int_var__sema_err_enable;
}
    
void cap_sema_csr_atomic_add_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_atomic_add_entry_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_semaphore_inc_not_full_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_semaphore_inc_not_full_entry_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_semaphore_inc_not_full_entry_t::full(const cpp_int & _val) { 
    // full
    int_var__full = _val.convert_to< full_cpp_int_t >();
}

cpp_int cap_sema_csr_semaphore_inc_not_full_entry_t::full() const {
    return int_var__full;
}
    
void cap_sema_csr_semaphore_dec_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_semaphore_dec_entry_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_semaphore_inc_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_semaphore_inc_entry_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_semaphore_raw_entry_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_semaphore_raw_entry_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_STA_sema_t::atomic_state(const cpp_int & _val) { 
    // atomic_state
    int_var__atomic_state = _val.convert_to< atomic_state_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::atomic_state() const {
    return int_var__atomic_state;
}
    
void cap_sema_csr_STA_sema_t::axi_wready(const cpp_int & _val) { 
    // axi_wready
    int_var__axi_wready = _val.convert_to< axi_wready_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::axi_wready() const {
    return int_var__axi_wready;
}
    
void cap_sema_csr_STA_sema_t::axi_awready(const cpp_int & _val) { 
    // axi_awready
    int_var__axi_awready = _val.convert_to< axi_awready_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::axi_awready() const {
    return int_var__axi_awready;
}
    
void cap_sema_csr_STA_sema_t::axi_rready(const cpp_int & _val) { 
    // axi_rready
    int_var__axi_rready = _val.convert_to< axi_rready_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::axi_rready() const {
    return int_var__axi_rready;
}
    
void cap_sema_csr_STA_sema_t::cam_full(const cpp_int & _val) { 
    // cam_full
    int_var__cam_full = _val.convert_to< cam_full_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::cam_full() const {
    return int_var__cam_full;
}
    
void cap_sema_csr_STA_sema_t::cam_hit(const cpp_int & _val) { 
    // cam_hit
    int_var__cam_hit = _val.convert_to< cam_hit_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::cam_hit() const {
    return int_var__cam_hit;
}
    
void cap_sema_csr_STA_sema_t::wr_pending_hit(const cpp_int & _val) { 
    // wr_pending_hit
    int_var__wr_pending_hit = _val.convert_to< wr_pending_hit_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::wr_pending_hit() const {
    return int_var__wr_pending_hit;
}
    
void cap_sema_csr_STA_sema_t::pending_count(const cpp_int & _val) { 
    // pending_count
    int_var__pending_count = _val.convert_to< pending_count_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_sema_t::pending_count() const {
    return int_var__pending_count;
}
    
void cap_sema_csr_sta_sema_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_sema_csr_sta_sema_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_sema_csr_sta_sema_mem_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::correctable() const {
    return int_var__correctable;
}
    
void cap_sema_csr_sta_sema_mem_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_sema_csr_sta_sema_mem_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_sema_csr_sta_sema_mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_sema_csr_sta_sema_mem_t::addr() const {
    return int_var__addr;
}
    
void cap_sema_csr_cfg_sema_mem_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_sema_csr_cfg_sema_mem_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_sema_csr_cfg_sema_mem_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_sema_csr_cfg_sema_mem_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_sema_csr_cfg_sema_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_sema_csr_cfg_sema_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_sema_csr_STA_err_resp_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_err_resp_t::addr() const {
    return int_var__addr;
}
    
void cap_sema_csr_STA_err_resp_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_err_resp_t::value() const {
    return int_var__value;
}
    
void cap_sema_csr_STA_pending_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_pending_t::valid() const {
    return int_var__valid;
}
    
void cap_sema_csr_STA_pending_t::wr(const cpp_int & _val) { 
    // wr
    int_var__wr = _val.convert_to< wr_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_pending_t::wr() const {
    return int_var__wr;
}
    
void cap_sema_csr_STA_pending_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_pending_t::addr() const {
    return int_var__addr;
}
    
void cap_sema_csr_STA_pending_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_sema_csr_STA_pending_t::data() const {
    return int_var__data;
}
    
void cap_sema_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_sema_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_sema_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_sema_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_sema_csr_sema_cfg_t::allow_merge(const cpp_int & _val) { 
    // allow_merge
    int_var__allow_merge = _val.convert_to< allow_merge_cpp_int_t >();
}

cpp_int cap_sema_csr_sema_cfg_t::allow_merge() const {
    return int_var__allow_merge;
}
    
void cap_sema_csr_axi_attr_t::arcache(const cpp_int & _val) { 
    // arcache
    int_var__arcache = _val.convert_to< arcache_cpp_int_t >();
}

cpp_int cap_sema_csr_axi_attr_t::arcache() const {
    return int_var__arcache;
}
    
void cap_sema_csr_axi_attr_t::awcache(const cpp_int & _val) { 
    // awcache
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >();
}

cpp_int cap_sema_csr_axi_attr_t::awcache() const {
    return int_var__awcache;
}
    
void cap_sema_csr_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_sema_csr_axi_attr_t::prot() const {
    return int_var__prot;
}
    
void cap_sema_csr_axi_attr_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_sema_csr_axi_attr_t::qos() const {
    return int_var__qos;
}
    
void cap_sema_csr_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_sema_csr_axi_attr_t::lock() const {
    return int_var__lock;
}
    
void cap_sema_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_sema_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sema_err_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "write_err_enable")) { field_val = write_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_err_enable")) { field_val = read_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_enable")) { field_val = uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_enable")) { field_val = correctable_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "write_err_interrupt")) { field_val = write_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_err_interrupt")) { field_val = read_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_interrupt")) { field_val = uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_interrupt")) { field_val = correctable_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_sema_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sema_err_interrupt")) { field_val = sema_err_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sema_err_enable")) { field_val = sema_err_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_sema_csr_atomic_add_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_atomic_add_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_not_full_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    if(!field_found && !strcmp(field_name, "full")) { field_val = full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_not_full_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_dec_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_dec_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_raw_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_raw_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_sema_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "atomic_state")) { field_val = atomic_state(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wready")) { field_val = axi_wready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_awready")) { field_val = axi_awready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rready")) { field_val = axi_rready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam_full")) { field_val = cam_full(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam_hit")) { field_val = cam_hit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_pending_hit")) { field_val = wr_pending_hit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_count")) { field_val = pending_count(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sta_sema_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_cfg_sema_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_err_resp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_pending_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr")) { field_val = wr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sema_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "allow_merge")) { field_val = allow_merge(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_axi_attr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "arcache")) { field_val = arcache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awcache")) { field_val = awcache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi_attr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sema_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_err_resp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sema_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sema_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_sema.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_raw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_inc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_dec.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_inc_not_full.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = atomic_add.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sema_err.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sema_err_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "write_err_enable")) { write_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_err_enable")) { read_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_enable")) { uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_enable")) { correctable_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "write_err_interrupt")) { write_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_err_interrupt")) { read_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_interrupt")) { uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_interrupt")) { correctable_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_sema_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sema_err_interrupt")) { sema_err_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sema_err_enable")) { sema_err_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_sema_csr_atomic_add_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_atomic_add_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_not_full_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "full")) { full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_not_full_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_dec_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_dec_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_inc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_raw_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_semaphore_raw_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_sema_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "atomic_state")) { atomic_state(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wready")) { axi_wready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_awready")) { axi_awready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_rready")) { axi_rready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam_full")) { cam_full(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cam_hit")) { cam_hit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_pending_hit")) { wr_pending_hit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_count")) { pending_count(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sta_sema_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_cfg_sema_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_err_resp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_STA_pending_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr")) { wr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_sema_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "allow_merge")) { allow_merge(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_axi_attr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "arcache")) { arcache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awcache")) { awcache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_sema_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi_attr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sema_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_err_resp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sema_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sema_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_sema.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_raw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_inc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_dec.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = semaphore_inc_not_full.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = atomic_add.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sema_err.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_sema_err_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("write_err_enable");
    ret_vec.push_back("read_err_enable");
    ret_vec.push_back("uncorrectable_enable");
    ret_vec.push_back("correctable_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("write_err_interrupt");
    ret_vec.push_back("read_err_interrupt");
    ret_vec.push_back("uncorrectable_interrupt");
    ret_vec.push_back("correctable_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_intgrp_t::get_fields(int level) const { 
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
std::vector<string> cap_sema_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sema_err_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sema_err_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_sema_csr_atomic_add_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_atomic_add_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_inc_not_full_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    ret_vec.push_back("full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_inc_not_full_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_dec_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_dec_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_inc_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_inc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_raw_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_semaphore_raw_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_STA_sema_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("atomic_state");
    ret_vec.push_back("axi_wready");
    ret_vec.push_back("axi_awready");
    ret_vec.push_back("axi_rready");
    ret_vec.push_back("cam_full");
    ret_vec.push_back("cam_hit");
    ret_vec.push_back("wr_pending_hit");
    ret_vec.push_back("pending_count");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_sta_sema_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("correctable");
    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_cfg_sema_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_STA_err_resp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addr");
    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_STA_pending_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("valid");
    ret_vec.push_back("wr");
    ret_vec.push_back("addr");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_sema_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("allow_merge");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_axi_attr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("arcache");
    ret_vec.push_back("awcache");
    ret_vec.push_back("prot");
    ret_vec.push_back("qos");
    ret_vec.push_back("lock");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_sema_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : axi_attr.get_fields(level-1)) {
            ret_vec.push_back("axi_attr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sema_cfg.get_fields(level-1)) {
            ret_vec.push_back("sema_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : STA_err_resp.get_fields(level-1)) {
            ret_vec.push_back("STA_err_resp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sema_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_sema_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sema_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_sema_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : STA_sema.get_fields(level-1)) {
            ret_vec.push_back("STA_sema." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : semaphore_raw.get_fields(level-1)) {
            ret_vec.push_back("semaphore_raw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : semaphore_inc.get_fields(level-1)) {
            ret_vec.push_back("semaphore_inc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : semaphore_dec.get_fields(level-1)) {
            ret_vec.push_back("semaphore_dec." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : semaphore_inc_not_full.get_fields(level-1)) {
            ret_vec.push_back("semaphore_inc_not_full." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : atomic_add.get_fields(level-1)) {
            ret_vec.push_back("atomic_add." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sema_err.get_fields(level-1)) {
            ret_vec.push_back("sema_err." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
