
#include "cap_mch_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_mch_csr_int_mc_int_enable_clear_t::cap_mch_csr_int_mc_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_int_mc_int_enable_clear_t::~cap_mch_csr_int_mc_int_enable_clear_t() { }

cap_mch_csr_intreg_t::cap_mch_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_intreg_t::~cap_mch_csr_intreg_t() { }

cap_mch_csr_intgrp_t::cap_mch_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_intgrp_t::~cap_mch_csr_intgrp_t() { }

cap_mch_csr_intreg_status_t::cap_mch_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_intreg_status_t::~cap_mch_csr_intreg_status_t() { }

cap_mch_csr_int_groups_int_enable_rw_reg_t::cap_mch_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_int_groups_int_enable_rw_reg_t::~cap_mch_csr_int_groups_int_enable_rw_reg_t() { }

cap_mch_csr_intgrp_status_t::cap_mch_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_intgrp_status_t::~cap_mch_csr_intgrp_status_t() { }

cap_mch_csr_dhs_apb_entry_t::cap_mch_csr_dhs_apb_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_dhs_apb_entry_t::~cap_mch_csr_dhs_apb_entry_t() { }

cap_mch_csr_dhs_apb_t::cap_mch_csr_dhs_apb_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mch_csr_dhs_apb_t::~cap_mch_csr_dhs_apb_t() { }

cap_mch_csr_csr_intr_t::cap_mch_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_csr_intr_t::~cap_mch_csr_csr_intr_t() { }

cap_mch_csr_sta_bist_t::cap_mch_csr_sta_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_sta_bist_t::~cap_mch_csr_sta_bist_t() { }

cap_mch_csr_cfg_bist_t::cap_mch_csr_cfg_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_cfg_bist_t::~cap_mch_csr_cfg_bist_t() { }

cap_mch_csr_mc_sta_t::cap_mch_csr_mc_sta_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_mc_sta_t::~cap_mch_csr_mc_sta_t() { }

cap_mch_csr_cfg_t::cap_mch_csr_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_cfg_t::~cap_mch_csr_cfg_t() { }

cap_mch_csr_cfg_l_t::cap_mch_csr_cfg_l_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_cfg_l_t::~cap_mch_csr_cfg_l_t() { }

cap_mch_csr_base_t::cap_mch_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mch_csr_base_t::~cap_mch_csr_base_t() { }

cap_mch_csr_t::cap_mch_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(524288);
        set_attributes(0,get_name(), 0);
        }
cap_mch_csr_t::~cap_mch_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_int_mc_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_1bit_thresh_ps1_enable: 0x" << int_var__ecc_1bit_thresh_ps1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_1bit_thresh_ps0_enable: 0x" << int_var__ecc_1bit_thresh_ps0_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_1bit_thresh_ps1_interrupt: 0x" << int_var__ecc_1bit_thresh_ps1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_1bit_thresh_ps0_interrupt: 0x" << int_var__ecc_1bit_thresh_ps0_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mc_interrupt: 0x" << int_var__int_mc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_mc_enable: 0x" << int_var__int_mc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_dhs_apb_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_dhs_apb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_sta_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_data_xram_ps0_fail: 0x" << int_var__done_read_data_xram_ps0_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_data_xram_ps1_fail: 0x" << int_var__done_read_data_xram_ps1_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_data_xram_ps0_pass: 0x" << int_var__done_read_data_xram_ps0_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_data_xram_ps1_pass: 0x" << int_var__done_read_data_xram_ps1_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_reorder_details_buffer_xram_ps0_fail: 0x" << int_var__done_read_reorder_details_buffer_xram_ps0_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_reorder_details_buffer_xram_ps1_fail: 0x" << int_var__done_read_reorder_details_buffer_xram_ps1_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_reorder_details_buffer_xram_ps0_pass: 0x" << int_var__done_read_reorder_details_buffer_xram_ps0_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_read_reorder_details_buffer_xram_ps1_pass: 0x" << int_var__done_read_reorder_details_buffer_xram_ps1_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_write_data_xram_ps0_fail: 0x" << int_var__done_write_data_xram_ps0_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_write_data_xram_ps1_fail: 0x" << int_var__done_write_data_xram_ps1_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_write_data_xram_ps0_pass: 0x" << int_var__done_write_data_xram_ps0_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_write_data_xram_ps1_pass: 0x" << int_var__done_write_data_xram_ps1_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_cfg_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".read_data_xram_ps0_run: 0x" << int_var__read_data_xram_ps0_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".read_data_xram_ps1_run: 0x" << int_var__read_data_xram_ps1_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".read_reorder_details_buffer_xram_ps0_run: 0x" << int_var__read_reorder_details_buffer_xram_ps0_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".read_reorder_details_buffer_xram_ps1_run: 0x" << int_var__read_reorder_details_buffer_xram_ps1_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_data_xram_ps0_run: 0x" << int_var__write_data_xram_ps0_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_data_xram_ps1_run: 0x" << int_var__write_data_xram_ps1_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_mc_sta_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_1bit_ps0: 0x" << int_var__ecc_error_1bit_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_2bit_ps0: 0x" << int_var__ecc_error_2bit_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_pos_ps0: 0x" << int_var__ecc_error_pos_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_addr_ps0: 0x" << int_var__ecc_error_addr_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_rmw_error_ps0: 0x" << int_var__ecc_rmw_error_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_1bit_ps1: 0x" << int_var__ecc_error_1bit_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_2bit_ps1: 0x" << int_var__ecc_error_2bit_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_pos_ps1: 0x" << int_var__ecc_error_pos_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_addr_ps1: 0x" << int_var__ecc_error_addr_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_rmw_error_ps1: 0x" << int_var__ecc_rmw_error_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_cnt_1bit_ps0: 0x" << int_var__ecc_error_cnt_1bit_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_cnt_1bit_ps1: 0x" << int_var__ecc_error_cnt_1bit_ps1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_cnt_2bit_ps0: 0x" << int_var__ecc_error_cnt_2bit_ps0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_error_cnt_2bit_ps1: 0x" << int_var__ecc_error_cnt_2bit_ps1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".address_mode: 0x" << int_var__address_mode << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_cfg_l_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".self_refresh: 0x" << int_var__self_refresh << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".power_down: 0x" << int_var__power_down << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mch_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_l.show();
    cfg.show();
    mc_sta.show();
    cfg_bist.show();
    sta_bist.show();
    csr_intr.show();
    dhs_apb.show();
    int_groups.show();
    int_mc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_mch_csr_int_mc_int_enable_clear_t::get_width() const {
    return cap_mch_csr_int_mc_int_enable_clear_t::s_get_width();

}

int cap_mch_csr_intreg_t::get_width() const {
    return cap_mch_csr_intreg_t::s_get_width();

}

int cap_mch_csr_intgrp_t::get_width() const {
    return cap_mch_csr_intgrp_t::s_get_width();

}

int cap_mch_csr_intreg_status_t::get_width() const {
    return cap_mch_csr_intreg_status_t::s_get_width();

}

int cap_mch_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_mch_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_mch_csr_intgrp_status_t::get_width() const {
    return cap_mch_csr_intgrp_status_t::s_get_width();

}

int cap_mch_csr_dhs_apb_entry_t::get_width() const {
    return cap_mch_csr_dhs_apb_entry_t::s_get_width();

}

int cap_mch_csr_dhs_apb_t::get_width() const {
    return cap_mch_csr_dhs_apb_t::s_get_width();

}

int cap_mch_csr_csr_intr_t::get_width() const {
    return cap_mch_csr_csr_intr_t::s_get_width();

}

int cap_mch_csr_sta_bist_t::get_width() const {
    return cap_mch_csr_sta_bist_t::s_get_width();

}

int cap_mch_csr_cfg_bist_t::get_width() const {
    return cap_mch_csr_cfg_bist_t::s_get_width();

}

int cap_mch_csr_mc_sta_t::get_width() const {
    return cap_mch_csr_mc_sta_t::s_get_width();

}

int cap_mch_csr_cfg_t::get_width() const {
    return cap_mch_csr_cfg_t::s_get_width();

}

int cap_mch_csr_cfg_l_t::get_width() const {
    return cap_mch_csr_cfg_l_t::s_get_width();

}

int cap_mch_csr_base_t::get_width() const {
    return cap_mch_csr_base_t::s_get_width();

}

int cap_mch_csr_t::get_width() const {
    return cap_mch_csr_t::s_get_width();

}

int cap_mch_csr_int_mc_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_1bit_thresh_ps1_enable
    _count += 1; // ecc_1bit_thresh_ps0_enable
    return _count;
}

int cap_mch_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_1bit_thresh_ps1_interrupt
    _count += 1; // ecc_1bit_thresh_ps0_interrupt
    return _count;
}

int cap_mch_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_mch_csr_intreg_t::s_get_width(); // intreg
    _count += cap_mch_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_mch_csr_int_mc_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mch_csr_int_mc_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mch_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mc_interrupt
    return _count;
}

int cap_mch_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_mc_enable
    return _count;
}

int cap_mch_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_mch_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_mch_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_mch_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_mch_csr_dhs_apb_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_mch_csr_dhs_apb_t::s_get_width() {
    int _count = 0;

    _count += (cap_mch_csr_dhs_apb_entry_t::s_get_width() * 65536); // entry
    return _count;
}

int cap_mch_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_mch_csr_sta_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // done_read_data_xram_ps0_fail
    _count += 1; // done_read_data_xram_ps1_fail
    _count += 1; // done_read_data_xram_ps0_pass
    _count += 1; // done_read_data_xram_ps1_pass
    _count += 1; // done_read_reorder_details_buffer_xram_ps0_fail
    _count += 1; // done_read_reorder_details_buffer_xram_ps1_fail
    _count += 1; // done_read_reorder_details_buffer_xram_ps0_pass
    _count += 1; // done_read_reorder_details_buffer_xram_ps1_pass
    _count += 1; // done_write_data_xram_ps0_fail
    _count += 1; // done_write_data_xram_ps1_fail
    _count += 1; // done_write_data_xram_ps0_pass
    _count += 1; // done_write_data_xram_ps1_pass
    return _count;
}

int cap_mch_csr_cfg_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // read_data_xram_ps0_run
    _count += 1; // read_data_xram_ps1_run
    _count += 1; // read_reorder_details_buffer_xram_ps0_run
    _count += 1; // read_reorder_details_buffer_xram_ps1_run
    _count += 1; // write_data_xram_ps0_run
    _count += 1; // write_data_xram_ps1_run
    return _count;
}

int cap_mch_csr_mc_sta_t::s_get_width() {
    int _count = 0;

    _count += 4; // ecc_error_1bit_ps0
    _count += 4; // ecc_error_2bit_ps0
    _count += 28; // ecc_error_pos_ps0
    _count += 4; // ecc_error_addr_ps0
    _count += 1; // ecc_rmw_error_ps0
    _count += 4; // ecc_error_1bit_ps1
    _count += 4; // ecc_error_2bit_ps1
    _count += 28; // ecc_error_pos_ps1
    _count += 4; // ecc_error_addr_ps1
    _count += 1; // ecc_rmw_error_ps1
    _count += 32; // ecc_error_cnt_1bit_ps0
    _count += 32; // ecc_error_cnt_1bit_ps1
    _count += 32; // ecc_error_cnt_2bit_ps0
    _count += 32; // ecc_error_cnt_2bit_ps1
    return _count;
}

int cap_mch_csr_cfg_t::s_get_width() {
    int _count = 0;

    _count += 4; // address_mode
    return _count;
}

int cap_mch_csr_cfg_l_t::s_get_width() {
    int _count = 0;

    _count += 8; // self_refresh
    _count += 8; // power_down
    return _count;
}

int cap_mch_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_mch_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_mch_csr_base_t::s_get_width(); // base
    _count += cap_mch_csr_cfg_l_t::s_get_width(); // cfg_l
    _count += cap_mch_csr_cfg_t::s_get_width(); // cfg
    _count += cap_mch_csr_mc_sta_t::s_get_width(); // mc_sta
    _count += cap_mch_csr_cfg_bist_t::s_get_width(); // cfg_bist
    _count += cap_mch_csr_sta_bist_t::s_get_width(); // sta_bist
    _count += cap_mch_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_mch_csr_dhs_apb_t::s_get_width(); // dhs_apb
    _count += cap_mch_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_mch_csr_intgrp_t::s_get_width(); // int_mc
    return _count;
}

void cap_mch_csr_int_mc_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_1bit_thresh_ps1_enable = _val.convert_to< ecc_1bit_thresh_ps1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_1bit_thresh_ps0_enable = _val.convert_to< ecc_1bit_thresh_ps0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_1bit_thresh_ps1_interrupt = _val.convert_to< ecc_1bit_thresh_ps1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_1bit_thresh_ps0_interrupt = _val.convert_to< ecc_1bit_thresh_ps0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_intgrp_t::all(const cpp_int & in_val) {
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

void cap_mch_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mc_interrupt = _val.convert_to< int_mc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_mc_enable = _val.convert_to< int_mc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_mch_csr_dhs_apb_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mch_csr_dhs_apb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_mch_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_sta_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__done_read_data_xram_ps0_fail = _val.convert_to< done_read_data_xram_ps0_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_data_xram_ps1_fail = _val.convert_to< done_read_data_xram_ps1_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_data_xram_ps0_pass = _val.convert_to< done_read_data_xram_ps0_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_data_xram_ps1_pass = _val.convert_to< done_read_data_xram_ps1_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_reorder_details_buffer_xram_ps0_fail = _val.convert_to< done_read_reorder_details_buffer_xram_ps0_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_reorder_details_buffer_xram_ps1_fail = _val.convert_to< done_read_reorder_details_buffer_xram_ps1_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_reorder_details_buffer_xram_ps0_pass = _val.convert_to< done_read_reorder_details_buffer_xram_ps0_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_read_reorder_details_buffer_xram_ps1_pass = _val.convert_to< done_read_reorder_details_buffer_xram_ps1_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_write_data_xram_ps0_fail = _val.convert_to< done_write_data_xram_ps0_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_write_data_xram_ps1_fail = _val.convert_to< done_write_data_xram_ps1_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_write_data_xram_ps0_pass = _val.convert_to< done_write_data_xram_ps0_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_write_data_xram_ps1_pass = _val.convert_to< done_write_data_xram_ps1_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_cfg_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__read_data_xram_ps0_run = _val.convert_to< read_data_xram_ps0_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__read_data_xram_ps1_run = _val.convert_to< read_data_xram_ps1_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__read_reorder_details_buffer_xram_ps0_run = _val.convert_to< read_reorder_details_buffer_xram_ps0_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__read_reorder_details_buffer_xram_ps1_run = _val.convert_to< read_reorder_details_buffer_xram_ps1_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_data_xram_ps0_run = _val.convert_to< write_data_xram_ps0_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_data_xram_ps1_run = _val.convert_to< write_data_xram_ps1_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mch_csr_mc_sta_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_error_1bit_ps0 = _val.convert_to< ecc_error_1bit_ps0_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_error_2bit_ps0 = _val.convert_to< ecc_error_2bit_ps0_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_error_pos_ps0 = _val.convert_to< ecc_error_pos_ps0_cpp_int_t >()  ;
    _val = _val >> 28;
    
    int_var__ecc_error_addr_ps0 = _val.convert_to< ecc_error_addr_ps0_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_rmw_error_ps0 = _val.convert_to< ecc_rmw_error_ps0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_error_1bit_ps1 = _val.convert_to< ecc_error_1bit_ps1_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_error_2bit_ps1 = _val.convert_to< ecc_error_2bit_ps1_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_error_pos_ps1 = _val.convert_to< ecc_error_pos_ps1_cpp_int_t >()  ;
    _val = _val >> 28;
    
    int_var__ecc_error_addr_ps1 = _val.convert_to< ecc_error_addr_ps1_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc_rmw_error_ps1 = _val.convert_to< ecc_rmw_error_ps1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_error_cnt_1bit_ps0 = _val.convert_to< ecc_error_cnt_1bit_ps0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ecc_error_cnt_1bit_ps1 = _val.convert_to< ecc_error_cnt_1bit_ps1_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ecc_error_cnt_2bit_ps0 = _val.convert_to< ecc_error_cnt_2bit_ps0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ecc_error_cnt_2bit_ps1 = _val.convert_to< ecc_error_cnt_2bit_ps1_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mch_csr_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__address_mode = _val.convert_to< address_mode_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_mch_csr_cfg_l_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__self_refresh = _val.convert_to< self_refresh_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__power_down = _val.convert_to< power_down_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mch_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mch_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_l.all( _val);
    _val = _val >> cfg_l.get_width(); 
    cfg.all( _val);
    _val = _val >> cfg.get_width(); 
    mc_sta.all( _val);
    _val = _val >> mc_sta.get_width(); 
    cfg_bist.all( _val);
    _val = _val >> cfg_bist.get_width(); 
    sta_bist.all( _val);
    _val = _val >> sta_bist.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    dhs_apb.all( _val);
    _val = _val >> dhs_apb.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_mc.all( _val);
    _val = _val >> int_mc.get_width(); 
}

cpp_int cap_mch_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_mc.get_width(); ret_val = ret_val  | int_mc.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << dhs_apb.get_width(); ret_val = ret_val  | dhs_apb.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_bist.get_width(); ret_val = ret_val  | sta_bist.all(); 
    ret_val = ret_val << cfg_bist.get_width(); ret_val = ret_val  | cfg_bist.all(); 
    ret_val = ret_val << mc_sta.get_width(); ret_val = ret_val  | mc_sta.all(); 
    ret_val = ret_val << cfg.get_width(); ret_val = ret_val  | cfg.all(); 
    ret_val = ret_val << cfg_l.get_width(); ret_val = ret_val  | cfg_l.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_mch_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_mch_csr_cfg_l_t::all() const {
    cpp_int ret_val;

    // power_down
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__power_down; 
    
    // self_refresh
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__self_refresh; 
    
    return ret_val;
}

cpp_int cap_mch_csr_cfg_t::all() const {
    cpp_int ret_val;

    // address_mode
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__address_mode; 
    
    return ret_val;
}

cpp_int cap_mch_csr_mc_sta_t::all() const {
    cpp_int ret_val;

    // ecc_error_cnt_2bit_ps1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ecc_error_cnt_2bit_ps1; 
    
    // ecc_error_cnt_2bit_ps0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ecc_error_cnt_2bit_ps0; 
    
    // ecc_error_cnt_1bit_ps1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ecc_error_cnt_1bit_ps1; 
    
    // ecc_error_cnt_1bit_ps0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ecc_error_cnt_1bit_ps0; 
    
    // ecc_rmw_error_ps1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_rmw_error_ps1; 
    
    // ecc_error_addr_ps1
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_addr_ps1; 
    
    // ecc_error_pos_ps1
    ret_val = ret_val << 28; ret_val = ret_val  | int_var__ecc_error_pos_ps1; 
    
    // ecc_error_2bit_ps1
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_2bit_ps1; 
    
    // ecc_error_1bit_ps1
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_1bit_ps1; 
    
    // ecc_rmw_error_ps0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_rmw_error_ps0; 
    
    // ecc_error_addr_ps0
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_addr_ps0; 
    
    // ecc_error_pos_ps0
    ret_val = ret_val << 28; ret_val = ret_val  | int_var__ecc_error_pos_ps0; 
    
    // ecc_error_2bit_ps0
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_2bit_ps0; 
    
    // ecc_error_1bit_ps0
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ecc_error_1bit_ps0; 
    
    return ret_val;
}

cpp_int cap_mch_csr_cfg_bist_t::all() const {
    cpp_int ret_val;

    // write_data_xram_ps1_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_data_xram_ps1_run; 
    
    // write_data_xram_ps0_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_data_xram_ps0_run; 
    
    // read_reorder_details_buffer_xram_ps1_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_reorder_details_buffer_xram_ps1_run; 
    
    // read_reorder_details_buffer_xram_ps0_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_reorder_details_buffer_xram_ps0_run; 
    
    // read_data_xram_ps1_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_data_xram_ps1_run; 
    
    // read_data_xram_ps0_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__read_data_xram_ps0_run; 
    
    return ret_val;
}

cpp_int cap_mch_csr_sta_bist_t::all() const {
    cpp_int ret_val;

    // done_write_data_xram_ps1_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_write_data_xram_ps1_pass; 
    
    // done_write_data_xram_ps0_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_write_data_xram_ps0_pass; 
    
    // done_write_data_xram_ps1_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_write_data_xram_ps1_fail; 
    
    // done_write_data_xram_ps0_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_write_data_xram_ps0_fail; 
    
    // done_read_reorder_details_buffer_xram_ps1_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_reorder_details_buffer_xram_ps1_pass; 
    
    // done_read_reorder_details_buffer_xram_ps0_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_reorder_details_buffer_xram_ps0_pass; 
    
    // done_read_reorder_details_buffer_xram_ps1_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_reorder_details_buffer_xram_ps1_fail; 
    
    // done_read_reorder_details_buffer_xram_ps0_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_reorder_details_buffer_xram_ps0_fail; 
    
    // done_read_data_xram_ps1_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_data_xram_ps1_pass; 
    
    // done_read_data_xram_ps0_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_data_xram_ps0_pass; 
    
    // done_read_data_xram_ps1_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_data_xram_ps1_fail; 
    
    // done_read_data_xram_ps0_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_read_data_xram_ps0_fail; 
    
    return ret_val;
}

cpp_int cap_mch_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_mch_csr_dhs_apb_t::all() const {
    cpp_int ret_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 65536-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_mch_csr_dhs_apb_entry_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_mch_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mch_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_mc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mc_interrupt; 
    
    return ret_val;
}

cpp_int cap_mch_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_mc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_mc_enable; 
    
    return ret_val;
}

cpp_int cap_mch_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mch_csr_intreg_t::all() const {
    cpp_int ret_val;

    // ecc_1bit_thresh_ps0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_1bit_thresh_ps0_interrupt; 
    
    // ecc_1bit_thresh_ps1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_1bit_thresh_ps1_interrupt; 
    
    return ret_val;
}

cpp_int cap_mch_csr_int_mc_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // ecc_1bit_thresh_ps0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_1bit_thresh_ps0_enable; 
    
    // ecc_1bit_thresh_ps1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_1bit_thresh_ps1_enable; 
    
    return ret_val;
}

void cap_mch_csr_int_mc_int_enable_clear_t::clear() {

    int_var__ecc_1bit_thresh_ps1_enable = 0; 
    
    int_var__ecc_1bit_thresh_ps0_enable = 0; 
    
}

void cap_mch_csr_intreg_t::clear() {

    int_var__ecc_1bit_thresh_ps1_interrupt = 0; 
    
    int_var__ecc_1bit_thresh_ps0_interrupt = 0; 
    
}

void cap_mch_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mch_csr_intreg_status_t::clear() {

    int_var__int_mc_interrupt = 0; 
    
}

void cap_mch_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_mc_enable = 0; 
    
}

void cap_mch_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_mch_csr_dhs_apb_entry_t::clear() {

    int_var__data = 0; 
    
}

void cap_mch_csr_dhs_apb_t::clear() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_mch_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_mch_csr_sta_bist_t::clear() {

    int_var__done_read_data_xram_ps0_fail = 0; 
    
    int_var__done_read_data_xram_ps1_fail = 0; 
    
    int_var__done_read_data_xram_ps0_pass = 0; 
    
    int_var__done_read_data_xram_ps1_pass = 0; 
    
    int_var__done_read_reorder_details_buffer_xram_ps0_fail = 0; 
    
    int_var__done_read_reorder_details_buffer_xram_ps1_fail = 0; 
    
    int_var__done_read_reorder_details_buffer_xram_ps0_pass = 0; 
    
    int_var__done_read_reorder_details_buffer_xram_ps1_pass = 0; 
    
    int_var__done_write_data_xram_ps0_fail = 0; 
    
    int_var__done_write_data_xram_ps1_fail = 0; 
    
    int_var__done_write_data_xram_ps0_pass = 0; 
    
    int_var__done_write_data_xram_ps1_pass = 0; 
    
}

void cap_mch_csr_cfg_bist_t::clear() {

    int_var__read_data_xram_ps0_run = 0; 
    
    int_var__read_data_xram_ps1_run = 0; 
    
    int_var__read_reorder_details_buffer_xram_ps0_run = 0; 
    
    int_var__read_reorder_details_buffer_xram_ps1_run = 0; 
    
    int_var__write_data_xram_ps0_run = 0; 
    
    int_var__write_data_xram_ps1_run = 0; 
    
}

void cap_mch_csr_mc_sta_t::clear() {

    int_var__ecc_error_1bit_ps0 = 0; 
    
    int_var__ecc_error_2bit_ps0 = 0; 
    
    int_var__ecc_error_pos_ps0 = 0; 
    
    int_var__ecc_error_addr_ps0 = 0; 
    
    int_var__ecc_rmw_error_ps0 = 0; 
    
    int_var__ecc_error_1bit_ps1 = 0; 
    
    int_var__ecc_error_2bit_ps1 = 0; 
    
    int_var__ecc_error_pos_ps1 = 0; 
    
    int_var__ecc_error_addr_ps1 = 0; 
    
    int_var__ecc_rmw_error_ps1 = 0; 
    
    int_var__ecc_error_cnt_1bit_ps0 = 0; 
    
    int_var__ecc_error_cnt_1bit_ps1 = 0; 
    
    int_var__ecc_error_cnt_2bit_ps0 = 0; 
    
    int_var__ecc_error_cnt_2bit_ps1 = 0; 
    
}

void cap_mch_csr_cfg_t::clear() {

    int_var__address_mode = 0; 
    
}

void cap_mch_csr_cfg_l_t::clear() {

    int_var__self_refresh = 0; 
    
    int_var__power_down = 0; 
    
}

void cap_mch_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_mch_csr_t::clear() {

    base.clear();
    cfg_l.clear();
    cfg.clear();
    mc_sta.clear();
    cfg_bist.clear();
    sta_bist.clear();
    csr_intr.clear();
    dhs_apb.clear();
    int_groups.clear();
    int_mc.clear();
}

void cap_mch_csr_int_mc_int_enable_clear_t::init() {

}

void cap_mch_csr_intreg_t::init() {

}

void cap_mch_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mch_csr_intreg_status_t::init() {

}

void cap_mch_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_mch_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_mch_csr_dhs_apb_entry_t::init() {

}

void cap_mch_csr_dhs_apb_t::init() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 65536; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_mch_csr_csr_intr_t::init() {

}

void cap_mch_csr_sta_bist_t::init() {

}

void cap_mch_csr_cfg_bist_t::init() {

}

void cap_mch_csr_mc_sta_t::init() {

}

void cap_mch_csr_cfg_t::init() {

    set_reset_val(cpp_int("0x2"));
    all(get_reset_val());
}

void cap_mch_csr_cfg_l_t::init() {

}

void cap_mch_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mch_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_l.set_attributes(this,"cfg_l", 0x4 );
    cfg.set_attributes(this,"cfg", 0x8 );
    mc_sta.set_attributes(this,"mc_sta", 0x20 );
    cfg_bist.set_attributes(this,"cfg_bist", 0x40 );
    sta_bist.set_attributes(this,"sta_bist", 0x44 );
    csr_intr.set_attributes(this,"csr_intr", 0x48 );
    dhs_apb.set_attributes(this,"dhs_apb", 0x40000 );
    int_groups.set_attributes(this,"int_groups", 0x50 );
    int_mc.set_attributes(this,"int_mc", 0x60 );
}

void cap_mch_csr_int_mc_int_enable_clear_t::ecc_1bit_thresh_ps1_enable(const cpp_int & _val) { 
    // ecc_1bit_thresh_ps1_enable
    int_var__ecc_1bit_thresh_ps1_enable = _val.convert_to< ecc_1bit_thresh_ps1_enable_cpp_int_t >();
}

cpp_int cap_mch_csr_int_mc_int_enable_clear_t::ecc_1bit_thresh_ps1_enable() const {
    return int_var__ecc_1bit_thresh_ps1_enable;
}
    
void cap_mch_csr_int_mc_int_enable_clear_t::ecc_1bit_thresh_ps0_enable(const cpp_int & _val) { 
    // ecc_1bit_thresh_ps0_enable
    int_var__ecc_1bit_thresh_ps0_enable = _val.convert_to< ecc_1bit_thresh_ps0_enable_cpp_int_t >();
}

cpp_int cap_mch_csr_int_mc_int_enable_clear_t::ecc_1bit_thresh_ps0_enable() const {
    return int_var__ecc_1bit_thresh_ps0_enable;
}
    
void cap_mch_csr_intreg_t::ecc_1bit_thresh_ps1_interrupt(const cpp_int & _val) { 
    // ecc_1bit_thresh_ps1_interrupt
    int_var__ecc_1bit_thresh_ps1_interrupt = _val.convert_to< ecc_1bit_thresh_ps1_interrupt_cpp_int_t >();
}

cpp_int cap_mch_csr_intreg_t::ecc_1bit_thresh_ps1_interrupt() const {
    return int_var__ecc_1bit_thresh_ps1_interrupt;
}
    
void cap_mch_csr_intreg_t::ecc_1bit_thresh_ps0_interrupt(const cpp_int & _val) { 
    // ecc_1bit_thresh_ps0_interrupt
    int_var__ecc_1bit_thresh_ps0_interrupt = _val.convert_to< ecc_1bit_thresh_ps0_interrupt_cpp_int_t >();
}

cpp_int cap_mch_csr_intreg_t::ecc_1bit_thresh_ps0_interrupt() const {
    return int_var__ecc_1bit_thresh_ps0_interrupt;
}
    
void cap_mch_csr_intreg_status_t::int_mc_interrupt(const cpp_int & _val) { 
    // int_mc_interrupt
    int_var__int_mc_interrupt = _val.convert_to< int_mc_interrupt_cpp_int_t >();
}

cpp_int cap_mch_csr_intreg_status_t::int_mc_interrupt() const {
    return int_var__int_mc_interrupt;
}
    
void cap_mch_csr_int_groups_int_enable_rw_reg_t::int_mc_enable(const cpp_int & _val) { 
    // int_mc_enable
    int_var__int_mc_enable = _val.convert_to< int_mc_enable_cpp_int_t >();
}

cpp_int cap_mch_csr_int_groups_int_enable_rw_reg_t::int_mc_enable() const {
    return int_var__int_mc_enable;
}
    
void cap_mch_csr_dhs_apb_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_mch_csr_dhs_apb_entry_t::data() const {
    return int_var__data;
}
    
void cap_mch_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_mch_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_mch_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_mch_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_mch_csr_sta_bist_t::done_read_data_xram_ps0_fail(const cpp_int & _val) { 
    // done_read_data_xram_ps0_fail
    int_var__done_read_data_xram_ps0_fail = _val.convert_to< done_read_data_xram_ps0_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_data_xram_ps0_fail() const {
    return int_var__done_read_data_xram_ps0_fail;
}
    
void cap_mch_csr_sta_bist_t::done_read_data_xram_ps1_fail(const cpp_int & _val) { 
    // done_read_data_xram_ps1_fail
    int_var__done_read_data_xram_ps1_fail = _val.convert_to< done_read_data_xram_ps1_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_data_xram_ps1_fail() const {
    return int_var__done_read_data_xram_ps1_fail;
}
    
void cap_mch_csr_sta_bist_t::done_read_data_xram_ps0_pass(const cpp_int & _val) { 
    // done_read_data_xram_ps0_pass
    int_var__done_read_data_xram_ps0_pass = _val.convert_to< done_read_data_xram_ps0_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_data_xram_ps0_pass() const {
    return int_var__done_read_data_xram_ps0_pass;
}
    
void cap_mch_csr_sta_bist_t::done_read_data_xram_ps1_pass(const cpp_int & _val) { 
    // done_read_data_xram_ps1_pass
    int_var__done_read_data_xram_ps1_pass = _val.convert_to< done_read_data_xram_ps1_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_data_xram_ps1_pass() const {
    return int_var__done_read_data_xram_ps1_pass;
}
    
void cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps0_fail(const cpp_int & _val) { 
    // done_read_reorder_details_buffer_xram_ps0_fail
    int_var__done_read_reorder_details_buffer_xram_ps0_fail = _val.convert_to< done_read_reorder_details_buffer_xram_ps0_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps0_fail() const {
    return int_var__done_read_reorder_details_buffer_xram_ps0_fail;
}
    
void cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps1_fail(const cpp_int & _val) { 
    // done_read_reorder_details_buffer_xram_ps1_fail
    int_var__done_read_reorder_details_buffer_xram_ps1_fail = _val.convert_to< done_read_reorder_details_buffer_xram_ps1_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps1_fail() const {
    return int_var__done_read_reorder_details_buffer_xram_ps1_fail;
}
    
void cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps0_pass(const cpp_int & _val) { 
    // done_read_reorder_details_buffer_xram_ps0_pass
    int_var__done_read_reorder_details_buffer_xram_ps0_pass = _val.convert_to< done_read_reorder_details_buffer_xram_ps0_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps0_pass() const {
    return int_var__done_read_reorder_details_buffer_xram_ps0_pass;
}
    
void cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps1_pass(const cpp_int & _val) { 
    // done_read_reorder_details_buffer_xram_ps1_pass
    int_var__done_read_reorder_details_buffer_xram_ps1_pass = _val.convert_to< done_read_reorder_details_buffer_xram_ps1_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_read_reorder_details_buffer_xram_ps1_pass() const {
    return int_var__done_read_reorder_details_buffer_xram_ps1_pass;
}
    
void cap_mch_csr_sta_bist_t::done_write_data_xram_ps0_fail(const cpp_int & _val) { 
    // done_write_data_xram_ps0_fail
    int_var__done_write_data_xram_ps0_fail = _val.convert_to< done_write_data_xram_ps0_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_write_data_xram_ps0_fail() const {
    return int_var__done_write_data_xram_ps0_fail;
}
    
void cap_mch_csr_sta_bist_t::done_write_data_xram_ps1_fail(const cpp_int & _val) { 
    // done_write_data_xram_ps1_fail
    int_var__done_write_data_xram_ps1_fail = _val.convert_to< done_write_data_xram_ps1_fail_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_write_data_xram_ps1_fail() const {
    return int_var__done_write_data_xram_ps1_fail;
}
    
void cap_mch_csr_sta_bist_t::done_write_data_xram_ps0_pass(const cpp_int & _val) { 
    // done_write_data_xram_ps0_pass
    int_var__done_write_data_xram_ps0_pass = _val.convert_to< done_write_data_xram_ps0_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_write_data_xram_ps0_pass() const {
    return int_var__done_write_data_xram_ps0_pass;
}
    
void cap_mch_csr_sta_bist_t::done_write_data_xram_ps1_pass(const cpp_int & _val) { 
    // done_write_data_xram_ps1_pass
    int_var__done_write_data_xram_ps1_pass = _val.convert_to< done_write_data_xram_ps1_pass_cpp_int_t >();
}

cpp_int cap_mch_csr_sta_bist_t::done_write_data_xram_ps1_pass() const {
    return int_var__done_write_data_xram_ps1_pass;
}
    
void cap_mch_csr_cfg_bist_t::read_data_xram_ps0_run(const cpp_int & _val) { 
    // read_data_xram_ps0_run
    int_var__read_data_xram_ps0_run = _val.convert_to< read_data_xram_ps0_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::read_data_xram_ps0_run() const {
    return int_var__read_data_xram_ps0_run;
}
    
void cap_mch_csr_cfg_bist_t::read_data_xram_ps1_run(const cpp_int & _val) { 
    // read_data_xram_ps1_run
    int_var__read_data_xram_ps1_run = _val.convert_to< read_data_xram_ps1_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::read_data_xram_ps1_run() const {
    return int_var__read_data_xram_ps1_run;
}
    
void cap_mch_csr_cfg_bist_t::read_reorder_details_buffer_xram_ps0_run(const cpp_int & _val) { 
    // read_reorder_details_buffer_xram_ps0_run
    int_var__read_reorder_details_buffer_xram_ps0_run = _val.convert_to< read_reorder_details_buffer_xram_ps0_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::read_reorder_details_buffer_xram_ps0_run() const {
    return int_var__read_reorder_details_buffer_xram_ps0_run;
}
    
void cap_mch_csr_cfg_bist_t::read_reorder_details_buffer_xram_ps1_run(const cpp_int & _val) { 
    // read_reorder_details_buffer_xram_ps1_run
    int_var__read_reorder_details_buffer_xram_ps1_run = _val.convert_to< read_reorder_details_buffer_xram_ps1_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::read_reorder_details_buffer_xram_ps1_run() const {
    return int_var__read_reorder_details_buffer_xram_ps1_run;
}
    
void cap_mch_csr_cfg_bist_t::write_data_xram_ps0_run(const cpp_int & _val) { 
    // write_data_xram_ps0_run
    int_var__write_data_xram_ps0_run = _val.convert_to< write_data_xram_ps0_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::write_data_xram_ps0_run() const {
    return int_var__write_data_xram_ps0_run;
}
    
void cap_mch_csr_cfg_bist_t::write_data_xram_ps1_run(const cpp_int & _val) { 
    // write_data_xram_ps1_run
    int_var__write_data_xram_ps1_run = _val.convert_to< write_data_xram_ps1_run_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_bist_t::write_data_xram_ps1_run() const {
    return int_var__write_data_xram_ps1_run;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_1bit_ps0(const cpp_int & _val) { 
    // ecc_error_1bit_ps0
    int_var__ecc_error_1bit_ps0 = _val.convert_to< ecc_error_1bit_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_1bit_ps0() const {
    return int_var__ecc_error_1bit_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_2bit_ps0(const cpp_int & _val) { 
    // ecc_error_2bit_ps0
    int_var__ecc_error_2bit_ps0 = _val.convert_to< ecc_error_2bit_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_2bit_ps0() const {
    return int_var__ecc_error_2bit_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_pos_ps0(const cpp_int & _val) { 
    // ecc_error_pos_ps0
    int_var__ecc_error_pos_ps0 = _val.convert_to< ecc_error_pos_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_pos_ps0() const {
    return int_var__ecc_error_pos_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_addr_ps0(const cpp_int & _val) { 
    // ecc_error_addr_ps0
    int_var__ecc_error_addr_ps0 = _val.convert_to< ecc_error_addr_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_addr_ps0() const {
    return int_var__ecc_error_addr_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_rmw_error_ps0(const cpp_int & _val) { 
    // ecc_rmw_error_ps0
    int_var__ecc_rmw_error_ps0 = _val.convert_to< ecc_rmw_error_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_rmw_error_ps0() const {
    return int_var__ecc_rmw_error_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_1bit_ps1(const cpp_int & _val) { 
    // ecc_error_1bit_ps1
    int_var__ecc_error_1bit_ps1 = _val.convert_to< ecc_error_1bit_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_1bit_ps1() const {
    return int_var__ecc_error_1bit_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_2bit_ps1(const cpp_int & _val) { 
    // ecc_error_2bit_ps1
    int_var__ecc_error_2bit_ps1 = _val.convert_to< ecc_error_2bit_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_2bit_ps1() const {
    return int_var__ecc_error_2bit_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_pos_ps1(const cpp_int & _val) { 
    // ecc_error_pos_ps1
    int_var__ecc_error_pos_ps1 = _val.convert_to< ecc_error_pos_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_pos_ps1() const {
    return int_var__ecc_error_pos_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_addr_ps1(const cpp_int & _val) { 
    // ecc_error_addr_ps1
    int_var__ecc_error_addr_ps1 = _val.convert_to< ecc_error_addr_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_addr_ps1() const {
    return int_var__ecc_error_addr_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_rmw_error_ps1(const cpp_int & _val) { 
    // ecc_rmw_error_ps1
    int_var__ecc_rmw_error_ps1 = _val.convert_to< ecc_rmw_error_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_rmw_error_ps1() const {
    return int_var__ecc_rmw_error_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_cnt_1bit_ps0(const cpp_int & _val) { 
    // ecc_error_cnt_1bit_ps0
    int_var__ecc_error_cnt_1bit_ps0 = _val.convert_to< ecc_error_cnt_1bit_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_cnt_1bit_ps0() const {
    return int_var__ecc_error_cnt_1bit_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_cnt_1bit_ps1(const cpp_int & _val) { 
    // ecc_error_cnt_1bit_ps1
    int_var__ecc_error_cnt_1bit_ps1 = _val.convert_to< ecc_error_cnt_1bit_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_cnt_1bit_ps1() const {
    return int_var__ecc_error_cnt_1bit_ps1;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_cnt_2bit_ps0(const cpp_int & _val) { 
    // ecc_error_cnt_2bit_ps0
    int_var__ecc_error_cnt_2bit_ps0 = _val.convert_to< ecc_error_cnt_2bit_ps0_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_cnt_2bit_ps0() const {
    return int_var__ecc_error_cnt_2bit_ps0;
}
    
void cap_mch_csr_mc_sta_t::ecc_error_cnt_2bit_ps1(const cpp_int & _val) { 
    // ecc_error_cnt_2bit_ps1
    int_var__ecc_error_cnt_2bit_ps1 = _val.convert_to< ecc_error_cnt_2bit_ps1_cpp_int_t >();
}

cpp_int cap_mch_csr_mc_sta_t::ecc_error_cnt_2bit_ps1() const {
    return int_var__ecc_error_cnt_2bit_ps1;
}
    
void cap_mch_csr_cfg_t::address_mode(const cpp_int & _val) { 
    // address_mode
    int_var__address_mode = _val.convert_to< address_mode_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_t::address_mode() const {
    return int_var__address_mode;
}
    
void cap_mch_csr_cfg_l_t::self_refresh(const cpp_int & _val) { 
    // self_refresh
    int_var__self_refresh = _val.convert_to< self_refresh_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_l_t::self_refresh() const {
    return int_var__self_refresh;
}
    
void cap_mch_csr_cfg_l_t::power_down(const cpp_int & _val) { 
    // power_down
    int_var__power_down = _val.convert_to< power_down_cpp_int_t >();
}

cpp_int cap_mch_csr_cfg_l_t::power_down() const {
    return int_var__power_down;
}
    
void cap_mch_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_mch_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_int_mc_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps1_enable")) { field_val = ecc_1bit_thresh_ps1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps0_enable")) { field_val = ecc_1bit_thresh_ps0_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps1_interrupt")) { field_val = ecc_1bit_thresh_ps1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps0_interrupt")) { field_val = ecc_1bit_thresh_ps0_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mch_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_interrupt")) { field_val = int_mc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_enable")) { field_val = int_mc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mch_csr_dhs_apb_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_dhs_apb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_sta_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps0_fail")) { field_val = done_read_data_xram_ps0_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps1_fail")) { field_val = done_read_data_xram_ps1_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps0_pass")) { field_val = done_read_data_xram_ps0_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps1_pass")) { field_val = done_read_data_xram_ps1_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps0_fail")) { field_val = done_read_reorder_details_buffer_xram_ps0_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps1_fail")) { field_val = done_read_reorder_details_buffer_xram_ps1_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps0_pass")) { field_val = done_read_reorder_details_buffer_xram_ps0_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps1_pass")) { field_val = done_read_reorder_details_buffer_xram_ps1_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps0_fail")) { field_val = done_write_data_xram_ps0_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps1_fail")) { field_val = done_write_data_xram_ps1_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps0_pass")) { field_val = done_write_data_xram_ps0_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps1_pass")) { field_val = done_write_data_xram_ps1_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "read_data_xram_ps0_run")) { field_val = read_data_xram_ps0_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_data_xram_ps1_run")) { field_val = read_data_xram_ps1_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_reorder_details_buffer_xram_ps0_run")) { field_val = read_reorder_details_buffer_xram_ps0_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_reorder_details_buffer_xram_ps1_run")) { field_val = read_reorder_details_buffer_xram_ps1_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_data_xram_ps0_run")) { field_val = write_data_xram_ps0_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_data_xram_ps1_run")) { field_val = write_data_xram_ps1_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_mc_sta_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_error_1bit_ps0")) { field_val = ecc_error_1bit_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_2bit_ps0")) { field_val = ecc_error_2bit_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_pos_ps0")) { field_val = ecc_error_pos_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_addr_ps0")) { field_val = ecc_error_addr_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_rmw_error_ps0")) { field_val = ecc_rmw_error_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_1bit_ps1")) { field_val = ecc_error_1bit_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_2bit_ps1")) { field_val = ecc_error_2bit_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_pos_ps1")) { field_val = ecc_error_pos_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_addr_ps1")) { field_val = ecc_error_addr_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_rmw_error_ps1")) { field_val = ecc_rmw_error_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_1bit_ps0")) { field_val = ecc_error_cnt_1bit_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_1bit_ps1")) { field_val = ecc_error_cnt_1bit_ps1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_2bit_ps0")) { field_val = ecc_error_cnt_2bit_ps0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_2bit_ps1")) { field_val = ecc_error_cnt_2bit_ps1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "address_mode")) { field_val = address_mode(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_l_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "self_refresh")) { field_val = self_refresh(); field_found=1; }
    if(!field_found && !strcmp(field_name, "power_down")) { field_val = power_down(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_l.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mc_sta.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_int_mc_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps1_enable")) { ecc_1bit_thresh_ps1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps0_enable")) { ecc_1bit_thresh_ps0_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps1_interrupt")) { ecc_1bit_thresh_ps1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_1bit_thresh_ps0_interrupt")) { ecc_1bit_thresh_ps0_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mch_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_interrupt")) { int_mc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_mc_enable")) { int_mc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mch_csr_dhs_apb_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_dhs_apb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_sta_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps0_fail")) { done_read_data_xram_ps0_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps1_fail")) { done_read_data_xram_ps1_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps0_pass")) { done_read_data_xram_ps0_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_data_xram_ps1_pass")) { done_read_data_xram_ps1_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps0_fail")) { done_read_reorder_details_buffer_xram_ps0_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps1_fail")) { done_read_reorder_details_buffer_xram_ps1_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps0_pass")) { done_read_reorder_details_buffer_xram_ps0_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_read_reorder_details_buffer_xram_ps1_pass")) { done_read_reorder_details_buffer_xram_ps1_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps0_fail")) { done_write_data_xram_ps0_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps1_fail")) { done_write_data_xram_ps1_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps0_pass")) { done_write_data_xram_ps0_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_write_data_xram_ps1_pass")) { done_write_data_xram_ps1_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "read_data_xram_ps0_run")) { read_data_xram_ps0_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_data_xram_ps1_run")) { read_data_xram_ps1_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_reorder_details_buffer_xram_ps0_run")) { read_reorder_details_buffer_xram_ps0_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "read_reorder_details_buffer_xram_ps1_run")) { read_reorder_details_buffer_xram_ps1_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_data_xram_ps0_run")) { write_data_xram_ps0_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_data_xram_ps1_run")) { write_data_xram_ps1_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_mc_sta_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_error_1bit_ps0")) { ecc_error_1bit_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_2bit_ps0")) { ecc_error_2bit_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_pos_ps0")) { ecc_error_pos_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_addr_ps0")) { ecc_error_addr_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_rmw_error_ps0")) { ecc_rmw_error_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_1bit_ps1")) { ecc_error_1bit_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_2bit_ps1")) { ecc_error_2bit_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_pos_ps1")) { ecc_error_pos_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_addr_ps1")) { ecc_error_addr_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_rmw_error_ps1")) { ecc_rmw_error_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_1bit_ps0")) { ecc_error_cnt_1bit_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_1bit_ps1")) { ecc_error_cnt_1bit_ps1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_2bit_ps0")) { ecc_error_cnt_2bit_ps0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_error_cnt_2bit_ps1")) { ecc_error_cnt_2bit_ps1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "address_mode")) { address_mode(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_cfg_l_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "self_refresh")) { self_refresh(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "power_down")) { power_down(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mch_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_l.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mc_sta.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_apb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_mc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_int_mc_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_1bit_thresh_ps1_enable");
    ret_vec.push_back("ecc_1bit_thresh_ps0_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_1bit_thresh_ps1_interrupt");
    ret_vec.push_back("ecc_1bit_thresh_ps0_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_intgrp_t::get_fields(int level) const { 
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
std::vector<string> cap_mch_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_mc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_mch_csr_dhs_apb_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_dhs_apb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_sta_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("done_read_data_xram_ps0_fail");
    ret_vec.push_back("done_read_data_xram_ps1_fail");
    ret_vec.push_back("done_read_data_xram_ps0_pass");
    ret_vec.push_back("done_read_data_xram_ps1_pass");
    ret_vec.push_back("done_read_reorder_details_buffer_xram_ps0_fail");
    ret_vec.push_back("done_read_reorder_details_buffer_xram_ps1_fail");
    ret_vec.push_back("done_read_reorder_details_buffer_xram_ps0_pass");
    ret_vec.push_back("done_read_reorder_details_buffer_xram_ps1_pass");
    ret_vec.push_back("done_write_data_xram_ps0_fail");
    ret_vec.push_back("done_write_data_xram_ps1_fail");
    ret_vec.push_back("done_write_data_xram_ps0_pass");
    ret_vec.push_back("done_write_data_xram_ps1_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_cfg_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("read_data_xram_ps0_run");
    ret_vec.push_back("read_data_xram_ps1_run");
    ret_vec.push_back("read_reorder_details_buffer_xram_ps0_run");
    ret_vec.push_back("read_reorder_details_buffer_xram_ps1_run");
    ret_vec.push_back("write_data_xram_ps0_run");
    ret_vec.push_back("write_data_xram_ps1_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_mc_sta_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_error_1bit_ps0");
    ret_vec.push_back("ecc_error_2bit_ps0");
    ret_vec.push_back("ecc_error_pos_ps0");
    ret_vec.push_back("ecc_error_addr_ps0");
    ret_vec.push_back("ecc_rmw_error_ps0");
    ret_vec.push_back("ecc_error_1bit_ps1");
    ret_vec.push_back("ecc_error_2bit_ps1");
    ret_vec.push_back("ecc_error_pos_ps1");
    ret_vec.push_back("ecc_error_addr_ps1");
    ret_vec.push_back("ecc_rmw_error_ps1");
    ret_vec.push_back("ecc_error_cnt_1bit_ps0");
    ret_vec.push_back("ecc_error_cnt_1bit_ps1");
    ret_vec.push_back("ecc_error_cnt_2bit_ps0");
    ret_vec.push_back("ecc_error_cnt_2bit_ps1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("address_mode");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_cfg_l_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("self_refresh");
    ret_vec.push_back("power_down");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mch_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_l.get_fields(level-1)) {
            ret_vec.push_back("cfg_l." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg.get_fields(level-1)) {
            ret_vec.push_back("cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mc_sta.get_fields(level-1)) {
            ret_vec.push_back("mc_sta." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_bist.get_fields(level-1)) {
            ret_vec.push_back("cfg_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_bist.get_fields(level-1)) {
            ret_vec.push_back("sta_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_apb.get_fields(level-1)) {
            ret_vec.push_back("dhs_apb." + tmp_vec);
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
