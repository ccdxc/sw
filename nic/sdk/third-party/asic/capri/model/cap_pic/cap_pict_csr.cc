
#include "cap_pict_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pict_csr_dhs_tcam_srch_entry_t::cap_pict_csr_dhs_tcam_srch_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_srch_entry_t::~cap_pict_csr_dhs_tcam_srch_entry_t() { }

cap_pict_csr_dhs_tcam_srch_t::cap_pict_csr_dhs_tcam_srch_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_srch_t::~cap_pict_csr_dhs_tcam_srch_t() { }

cap_pict_csr_dhs_tcam_xy_entry_t::cap_pict_csr_dhs_tcam_xy_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_xy_entry_t::~cap_pict_csr_dhs_tcam_xy_entry_t() { }

cap_pict_csr_dhs_tcam_xy_t::cap_pict_csr_dhs_tcam_xy_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pict_csr_dhs_tcam_xy_t::~cap_pict_csr_dhs_tcam_xy_t() { }

cap_pict_csr_sta_tcam_t::cap_pict_csr_sta_tcam_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_sta_tcam_t::~cap_pict_csr_sta_tcam_t() { }

cap_pict_csr_cfg_tcam_t::cap_pict_csr_cfg_tcam_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_t::~cap_pict_csr_cfg_tcam_t() { }

cap_pict_csr_cnt_tcam_search_t::cap_pict_csr_cnt_tcam_search_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cnt_tcam_search_t::~cap_pict_csr_cnt_tcam_search_t() { }

cap_pict_csr_cnt_axi_pot_rdrsp_t::cap_pict_csr_cnt_axi_pot_rdrsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cnt_axi_pot_rdrsp_t::~cap_pict_csr_cnt_axi_pot_rdrsp_t() { }

cap_pict_csr_cnt_axi_pot_rdreq_t::cap_pict_csr_cnt_axi_pot_rdreq_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cnt_axi_pot_rdreq_t::~cap_pict_csr_cnt_axi_pot_rdreq_t() { }

cap_pict_csr_sta_tcam_srch_t::cap_pict_csr_sta_tcam_srch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_sta_tcam_srch_t::~cap_pict_csr_sta_tcam_srch_t() { }

cap_pict_csr_cfg_tcam_srch_t::cap_pict_csr_cfg_tcam_srch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_srch_t::~cap_pict_csr_cfg_tcam_srch_t() { }

cap_pict_csr_cfg_tcam_reset_t::cap_pict_csr_cfg_tcam_reset_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_reset_t::~cap_pict_csr_cfg_tcam_reset_t() { }

cap_pict_csr_cfg_tcam_table_partition_t::cap_pict_csr_cfg_tcam_table_partition_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_table_partition_t::~cap_pict_csr_cfg_tcam_table_partition_t() { }

cap_pict_csr_cfg_tcam_table_profile_t::cap_pict_csr_cfg_tcam_table_profile_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_cfg_tcam_table_profile_t::~cap_pict_csr_cfg_tcam_table_profile_t() { }

cap_pict_csr_base_t::cap_pict_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pict_csr_base_t::~cap_pict_csr_base_t() { }

cap_pict_csr_t::cap_pict_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(2097152);
        set_attributes(0,get_name(), 0);
        }
cap_pict_csr_t::~cap_pict_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_dhs_tcam_srch_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmd: 0x" << int_var__cmd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_dhs_tcam_srch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_dhs_tcam_xy_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".x: 0x" << int_var__x << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".y: 0x" << int_var__y << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_dhs_tcam_xy_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 8192; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_sta_tcam_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cfg_tcam_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cnt_tcam_search_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cnt_axi_pot_rdrsp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cnt_axi_pot_rdreq_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_sta_tcam_srch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hit: 0x" << int_var__hit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hit_addr: 0x" << int_var__hit_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cfg_tcam_srch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".key: 0x" << int_var__key << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask: 0x" << int_var__mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tbl_mask: 0x" << int_var__tbl_mask << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cfg_tcam_reset_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".vec: 0x" << int_var__vec << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cfg_tcam_table_partition_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".vbe: 0x" << int_var__vbe << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_cfg_tcam_table_profile_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".start_addr: 0x" << int_var__start_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".end_addr: 0x" << int_var__end_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".width: 0x" << int_var__width << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".en_tbid: 0x" << int_var__en_tbid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tbid: 0x" << int_var__tbid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bkts: 0x" << int_var__bkts << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".keyshift: 0x" << int_var__keyshift << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pict_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_tcam_table_profile.show(); // large_array
    #else
    for(int ii = 0; ii < 128; ii++) {
        cfg_tcam_table_profile[ii].show();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_tcam_table_partition.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        cfg_tcam_table_partition[ii].show();
    }
    #endif
    
    cfg_tcam_reset.show();
    cfg_tcam_srch.show();
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_tcam_srch.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        sta_tcam_srch[ii].show();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_axi_pot_rdreq.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdreq[ii].show();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_axi_pot_rdrsp.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdrsp[ii].show();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_tcam_search.show(); // large_array
    #else
    for(int ii = 0; ii < 8; ii++) {
        cnt_tcam_search[ii].show();
    }
    #endif
    
    cfg_tcam.show();
    sta_tcam.show();
    dhs_tcam_xy.show();
    dhs_tcam_srch.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_pict_csr_dhs_tcam_srch_entry_t::get_width() const {
    return cap_pict_csr_dhs_tcam_srch_entry_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_srch_t::get_width() const {
    return cap_pict_csr_dhs_tcam_srch_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_xy_entry_t::get_width() const {
    return cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_xy_t::get_width() const {
    return cap_pict_csr_dhs_tcam_xy_t::s_get_width();

}

int cap_pict_csr_sta_tcam_t::get_width() const {
    return cap_pict_csr_sta_tcam_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_t::get_width() const {
    return cap_pict_csr_cfg_tcam_t::s_get_width();

}

int cap_pict_csr_cnt_tcam_search_t::get_width() const {
    return cap_pict_csr_cnt_tcam_search_t::s_get_width();

}

int cap_pict_csr_cnt_axi_pot_rdrsp_t::get_width() const {
    return cap_pict_csr_cnt_axi_pot_rdrsp_t::s_get_width();

}

int cap_pict_csr_cnt_axi_pot_rdreq_t::get_width() const {
    return cap_pict_csr_cnt_axi_pot_rdreq_t::s_get_width();

}

int cap_pict_csr_sta_tcam_srch_t::get_width() const {
    return cap_pict_csr_sta_tcam_srch_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_srch_t::get_width() const {
    return cap_pict_csr_cfg_tcam_srch_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_reset_t::get_width() const {
    return cap_pict_csr_cfg_tcam_reset_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_table_partition_t::get_width() const {
    return cap_pict_csr_cfg_tcam_table_partition_t::s_get_width();

}

int cap_pict_csr_cfg_tcam_table_profile_t::get_width() const {
    return cap_pict_csr_cfg_tcam_table_profile_t::s_get_width();

}

int cap_pict_csr_base_t::get_width() const {
    return cap_pict_csr_base_t::s_get_width();

}

int cap_pict_csr_t::get_width() const {
    return cap_pict_csr_t::s_get_width();

}

int cap_pict_csr_dhs_tcam_srch_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // cmd
    return _count;
}

int cap_pict_csr_dhs_tcam_srch_t::s_get_width() {
    int _count = 0;

    _count += (cap_pict_csr_dhs_tcam_srch_entry_t::s_get_width() * 8); // entry
    return _count;
}

int cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width() {
    int _count = 0;

    _count += 128; // x
    _count += 128; // y
    _count += 1; // valid
    return _count;
}

int cap_pict_csr_dhs_tcam_xy_t::s_get_width() {
    int _count = 0;

    _count += (cap_pict_csr_dhs_tcam_xy_entry_t::s_get_width() * 8192); // entry
    return _count;
}

int cap_pict_csr_sta_tcam_t::s_get_width() {
    int _count = 0;

    _count += 8; // bist_done_fail
    _count += 8; // bist_done_pass
    return _count;
}

int cap_pict_csr_cfg_tcam_t::s_get_width() {
    int _count = 0;

    _count += 8; // bist_run
    return _count;
}

int cap_pict_csr_cnt_tcam_search_t::s_get_width() {
    int _count = 0;

    _count += 40; // val
    return _count;
}

int cap_pict_csr_cnt_axi_pot_rdrsp_t::s_get_width() {
    int _count = 0;

    _count += 40; // val
    return _count;
}

int cap_pict_csr_cnt_axi_pot_rdreq_t::s_get_width() {
    int _count = 0;

    _count += 40; // val
    return _count;
}

int cap_pict_csr_sta_tcam_srch_t::s_get_width() {
    int _count = 0;

    _count += 1; // hit
    _count += 10; // hit_addr
    return _count;
}

int cap_pict_csr_cfg_tcam_srch_t::s_get_width() {
    int _count = 0;

    _count += 128; // key
    _count += 128; // mask
    _count += 16; // tbl_mask
    return _count;
}

int cap_pict_csr_cfg_tcam_reset_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 8; // vec
    return _count;
}

int cap_pict_csr_cfg_tcam_table_partition_t::s_get_width() {
    int _count = 0;

    _count += 8; // vbe
    return _count;
}

int cap_pict_csr_cfg_tcam_table_profile_t::s_get_width() {
    int _count = 0;

    _count += 17; // start_addr
    _count += 17; // end_addr
    _count += 6; // width
    _count += 1; // en_tbid
    _count += 7; // tbid
    _count += 3; // bkts
    _count += 5; // keyshift
    return _count;
}

int cap_pict_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pict_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pict_csr_base_t::s_get_width(); // base
    _count += (cap_pict_csr_cfg_tcam_table_profile_t::s_get_width() * 128); // cfg_tcam_table_profile
    _count += (cap_pict_csr_cfg_tcam_table_partition_t::s_get_width() * 8); // cfg_tcam_table_partition
    _count += cap_pict_csr_cfg_tcam_reset_t::s_get_width(); // cfg_tcam_reset
    _count += cap_pict_csr_cfg_tcam_srch_t::s_get_width(); // cfg_tcam_srch
    _count += (cap_pict_csr_sta_tcam_srch_t::s_get_width() * 8); // sta_tcam_srch
    _count += (cap_pict_csr_cnt_axi_pot_rdreq_t::s_get_width() * 8); // cnt_axi_pot_rdreq
    _count += (cap_pict_csr_cnt_axi_pot_rdrsp_t::s_get_width() * 8); // cnt_axi_pot_rdrsp
    _count += (cap_pict_csr_cnt_tcam_search_t::s_get_width() * 8); // cnt_tcam_search
    _count += cap_pict_csr_cfg_tcam_t::s_get_width(); // cfg_tcam
    _count += cap_pict_csr_sta_tcam_t::s_get_width(); // sta_tcam
    _count += cap_pict_csr_dhs_tcam_xy_t::s_get_width(); // dhs_tcam_xy
    _count += cap_pict_csr_dhs_tcam_srch_t::s_get_width(); // dhs_tcam_srch
    return _count;
}

void cap_pict_csr_dhs_tcam_srch_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmd = _val.convert_to< cmd_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pict_csr_dhs_tcam_srch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 8; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_pict_csr_dhs_tcam_xy_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__x = _val.convert_to< x_cpp_int_t >()  ;
    _val = _val >> 128;
    
    int_var__y = _val.convert_to< y_cpp_int_t >()  ;
    _val = _val >> 128;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pict_csr_dhs_tcam_xy_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 8192; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_pict_csr_sta_tcam_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pict_csr_cfg_tcam_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pict_csr_cnt_tcam_search_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pict_csr_cnt_axi_pot_rdrsp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pict_csr_cnt_axi_pot_rdreq_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 40;
    
}

void cap_pict_csr_sta_tcam_srch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hit = _val.convert_to< hit_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hit_addr = _val.convert_to< hit_addr_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_pict_csr_cfg_tcam_srch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__key = _val.convert_to< key_cpp_int_t >()  ;
    _val = _val >> 128;
    
    int_var__mask = _val.convert_to< mask_cpp_int_t >()  ;
    _val = _val >> 128;
    
    int_var__tbl_mask = _val.convert_to< tbl_mask_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_pict_csr_cfg_tcam_reset_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__vec = _val.convert_to< vec_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pict_csr_cfg_tcam_table_partition_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__vbe = _val.convert_to< vbe_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pict_csr_cfg_tcam_table_profile_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >()  ;
    _val = _val >> 17;
    
    int_var__end_addr = _val.convert_to< end_addr_cpp_int_t >()  ;
    _val = _val >> 17;
    
    int_var__width = _val.convert_to< width_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__en_tbid = _val.convert_to< en_tbid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tbid = _val.convert_to< tbid_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__bkts = _val.convert_to< bkts_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__keyshift = _val.convert_to< keyshift_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_pict_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_pict_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_profile
    for(int ii = 0; ii < 128; ii++) {
        cfg_tcam_table_profile[ii].all(_val);
        _val = _val >> cfg_tcam_table_profile[ii].get_width();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_partition
    for(int ii = 0; ii < 8; ii++) {
        cfg_tcam_table_partition[ii].all(_val);
        _val = _val >> cfg_tcam_table_partition[ii].get_width();
    }
    #endif
    
    cfg_tcam_reset.all( _val);
    _val = _val >> cfg_tcam_reset.get_width(); 
    cfg_tcam_srch.all( _val);
    _val = _val >> cfg_tcam_srch.get_width(); 
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_tcam_srch
    for(int ii = 0; ii < 8; ii++) {
        sta_tcam_srch[ii].all(_val);
        _val = _val >> sta_tcam_srch[ii].get_width();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdreq
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdreq[ii].all(_val);
        _val = _val >> cnt_axi_pot_rdreq[ii].get_width();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdrsp
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdrsp[ii].all(_val);
        _val = _val >> cnt_axi_pot_rdrsp[ii].get_width();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_tcam_search
    for(int ii = 0; ii < 8; ii++) {
        cnt_tcam_search[ii].all(_val);
        _val = _val >> cnt_tcam_search[ii].get_width();
    }
    #endif
    
    cfg_tcam.all( _val);
    _val = _val >> cfg_tcam.get_width(); 
    sta_tcam.all( _val);
    _val = _val >> sta_tcam.get_width(); 
    dhs_tcam_xy.all( _val);
    _val = _val >> dhs_tcam_xy.get_width(); 
    dhs_tcam_srch.all( _val);
    _val = _val >> dhs_tcam_srch.get_width(); 
}

cpp_int cap_pict_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << dhs_tcam_srch.get_width(); ret_val = ret_val  | dhs_tcam_srch.all(); 
    ret_val = ret_val << dhs_tcam_xy.get_width(); ret_val = ret_val  | dhs_tcam_xy.all(); 
    ret_val = ret_val << sta_tcam.get_width(); ret_val = ret_val  | sta_tcam.all(); 
    ret_val = ret_val << cfg_tcam.get_width(); ret_val = ret_val  | cfg_tcam.all(); 
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_tcam_search
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << cnt_tcam_search[ii].get_width(); ret_val = ret_val  | cnt_tcam_search[ii].all(); 
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdrsp
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << cnt_axi_pot_rdrsp[ii].get_width(); ret_val = ret_val  | cnt_axi_pot_rdrsp[ii].all(); 
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdreq
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << cnt_axi_pot_rdreq[ii].get_width(); ret_val = ret_val  | cnt_axi_pot_rdreq[ii].all(); 
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_tcam_srch
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_tcam_srch[ii].get_width(); ret_val = ret_val  | sta_tcam_srch[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << cfg_tcam_srch.get_width(); ret_val = ret_val  | cfg_tcam_srch.all(); 
    ret_val = ret_val << cfg_tcam_reset.get_width(); ret_val = ret_val  | cfg_tcam_reset.all(); 
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_partition
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_tcam_table_partition[ii].get_width(); ret_val = ret_val  | cfg_tcam_table_partition[ii].all(); 
    }
    #endif
    
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_profile
    for(int ii = 128-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_tcam_table_profile[ii].get_width(); ret_val = ret_val  | cfg_tcam_table_profile[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_pict_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::all() const {
    cpp_int ret_val;

    // keyshift
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__keyshift; 
    
    // bkts
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__bkts; 
    
    // tbid
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__tbid; 
    
    // en_tbid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__en_tbid; 
    
    // width
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__width; 
    
    // end_addr
    ret_val = ret_val << 17; ret_val = ret_val  | int_var__end_addr; 
    
    // start_addr
    ret_val = ret_val << 17; ret_val = ret_val  | int_var__start_addr; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_table_partition_t::all() const {
    cpp_int ret_val;

    // vbe
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__vbe; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_reset_t::all() const {
    cpp_int ret_val;

    // vec
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__vec; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_srch_t::all() const {
    cpp_int ret_val;

    // tbl_mask
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__tbl_mask; 
    
    // mask
    ret_val = ret_val << 128; ret_val = ret_val  | int_var__mask; 
    
    // key
    ret_val = ret_val << 128; ret_val = ret_val  | int_var__key; 
    
    return ret_val;
}

cpp_int cap_pict_csr_sta_tcam_srch_t::all() const {
    cpp_int ret_val;

    // hit_addr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__hit_addr; 
    
    // hit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hit; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cnt_axi_pot_rdreq_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cnt_axi_pot_rdrsp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cnt_tcam_search_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_pict_csr_cfg_tcam_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_pict_csr_sta_tcam_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_xy_t::all() const {
    cpp_int ret_val;

    #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 8192-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::all() const {
    cpp_int ret_val;

    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // y
    ret_val = ret_val << 128; ret_val = ret_val  | int_var__y; 
    
    // x
    ret_val = ret_val << 128; ret_val = ret_val  | int_var__x; 
    
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_srch_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 8-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pict_csr_dhs_tcam_srch_entry_t::all() const {
    cpp_int ret_val;

    // cmd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmd; 
    
    return ret_val;
}

void cap_pict_csr_dhs_tcam_srch_entry_t::clear() {

    int_var__cmd = 0; 
    
}

void cap_pict_csr_dhs_tcam_srch_t::clear() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 8; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_pict_csr_dhs_tcam_xy_entry_t::clear() {

    int_var__x = 0; 
    
    int_var__y = 0; 
    
    int_var__valid = 0; 
    
}

void cap_pict_csr_dhs_tcam_xy_t::clear() {

    #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 8192; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_pict_csr_sta_tcam_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_pict_csr_cfg_tcam_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_pict_csr_cnt_tcam_search_t::clear() {

    int_var__val = 0; 
    
}

void cap_pict_csr_cnt_axi_pot_rdrsp_t::clear() {

    int_var__val = 0; 
    
}

void cap_pict_csr_cnt_axi_pot_rdreq_t::clear() {

    int_var__val = 0; 
    
}

void cap_pict_csr_sta_tcam_srch_t::clear() {

    int_var__hit = 0; 
    
    int_var__hit_addr = 0; 
    
}

void cap_pict_csr_cfg_tcam_srch_t::clear() {

    int_var__key = 0; 
    
    int_var__mask = 0; 
    
    int_var__tbl_mask = 0; 
    
}

void cap_pict_csr_cfg_tcam_reset_t::clear() {

    int_var__enable = 0; 
    
    int_var__vec = 0; 
    
}

void cap_pict_csr_cfg_tcam_table_partition_t::clear() {

    int_var__vbe = 0; 
    
}

void cap_pict_csr_cfg_tcam_table_profile_t::clear() {

    int_var__start_addr = 0; 
    
    int_var__end_addr = 0; 
    
    int_var__width = 0; 
    
    int_var__en_tbid = 0; 
    
    int_var__tbid = 0; 
    
    int_var__bkts = 0; 
    
    int_var__keyshift = 0; 
    
}

void cap_pict_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_pict_csr_t::clear() {

    base.clear();
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_profile
    for(int ii = 0; ii < 128; ii++) {
        cfg_tcam_table_profile[ii].clear();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_tcam_table_partition
    for(int ii = 0; ii < 8; ii++) {
        cfg_tcam_table_partition[ii].clear();
    }
    #endif
    
    cfg_tcam_reset.clear();
    cfg_tcam_srch.clear();
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_tcam_srch
    for(int ii = 0; ii < 8; ii++) {
        sta_tcam_srch[ii].clear();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdreq
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdreq[ii].clear();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cnt_axi_pot_rdrsp
    for(int ii = 0; ii < 8; ii++) {
        cnt_axi_pot_rdrsp[ii].clear();
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cnt_tcam_search
    for(int ii = 0; ii < 8; ii++) {
        cnt_tcam_search[ii].clear();
    }
    #endif
    
    cfg_tcam.clear();
    sta_tcam.clear();
    dhs_tcam_xy.clear();
    dhs_tcam_srch.clear();
}

void cap_pict_csr_dhs_tcam_srch_entry_t::init() {

}

void cap_pict_csr_dhs_tcam_srch_t::init() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_pict_csr_dhs_tcam_xy_entry_t::init() {

}

void cap_pict_csr_dhs_tcam_xy_t::init() {

    #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 8192; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_pict_csr_sta_tcam_t::init() {

}

void cap_pict_csr_cfg_tcam_t::init() {

}

void cap_pict_csr_cnt_tcam_search_t::init() {

}

void cap_pict_csr_cnt_axi_pot_rdrsp_t::init() {

}

void cap_pict_csr_cnt_axi_pot_rdreq_t::init() {

}

void cap_pict_csr_sta_tcam_srch_t::init() {

}

void cap_pict_csr_cfg_tcam_srch_t::init() {

    set_reset_val(cpp_int("0xffff0000000000000000000000000000000000000000000000000000000000000000"));
    all(get_reset_val());
}

void cap_pict_csr_cfg_tcam_reset_t::init() {

}

void cap_pict_csr_cfg_tcam_table_partition_t::init() {

    set_reset_val(cpp_int("0xff"));
    all(get_reset_val());
}

void cap_pict_csr_cfg_tcam_table_profile_t::init() {

}

void cap_pict_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pict_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_tcam_table_profile.set_attributes(this, "cfg_tcam_table_profile", 0x400);
    #else
    for(int ii = 0; ii < 128; ii++) {
        if(ii != 0) cfg_tcam_table_profile[ii].set_field_init_done(true, true);
        cfg_tcam_table_profile[ii].set_attributes(this,"cfg_tcam_table_profile["+to_string(ii)+"]",  0x400 + (cfg_tcam_table_profile[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_tcam_table_partition.set_attributes(this, "cfg_tcam_table_partition", 0x800);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cfg_tcam_table_partition[ii].set_field_init_done(true, true);
        cfg_tcam_table_partition[ii].set_attributes(this,"cfg_tcam_table_partition["+to_string(ii)+"]",  0x800 + (cfg_tcam_table_partition[ii].get_byte_size()*ii));
    }
    #endif
    
    cfg_tcam_reset.set_attributes(this,"cfg_tcam_reset", 0x100000 );
    cfg_tcam_srch.set_attributes(this,"cfg_tcam_srch", 0x100040 );
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_tcam_srch.set_attributes(this, "sta_tcam_srch", 0x100080);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) sta_tcam_srch[ii].set_field_init_done(true, true);
        sta_tcam_srch[ii].set_attributes(this,"sta_tcam_srch["+to_string(ii)+"]",  0x100080 + (sta_tcam_srch[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_axi_pot_rdreq.set_attributes(this, "cnt_axi_pot_rdreq", 0x1000c0);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cnt_axi_pot_rdreq[ii].set_field_init_done(true, true);
        cnt_axi_pot_rdreq[ii].set_attributes(this,"cnt_axi_pot_rdreq["+to_string(ii)+"]",  0x1000c0 + (cnt_axi_pot_rdreq[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_axi_pot_rdrsp.set_attributes(this, "cnt_axi_pot_rdrsp", 0x100100);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cnt_axi_pot_rdrsp[ii].set_field_init_done(true, true);
        cnt_axi_pot_rdrsp[ii].set_attributes(this,"cnt_axi_pot_rdrsp["+to_string(ii)+"]",  0x100100 + (cnt_axi_pot_rdrsp[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cnt_tcam_search.set_attributes(this, "cnt_tcam_search", 0x100140);
    #else
    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) cnt_tcam_search[ii].set_field_init_done(true, true);
        cnt_tcam_search[ii].set_attributes(this,"cnt_tcam_search["+to_string(ii)+"]",  0x100140 + (cnt_tcam_search[ii].get_byte_size()*ii));
    }
    #endif
    
    cfg_tcam.set_attributes(this,"cfg_tcam", 0x100180 );
    sta_tcam.set_attributes(this,"sta_tcam", 0x100184 );
    dhs_tcam_xy.set_attributes(this,"dhs_tcam_xy", 0x80000 );
    dhs_tcam_srch.set_attributes(this,"dhs_tcam_srch", 0x1000a0 );
}

void cap_pict_csr_dhs_tcam_srch_entry_t::cmd(const cpp_int & _val) { 
    // cmd
    int_var__cmd = _val.convert_to< cmd_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_srch_entry_t::cmd() const {
    return int_var__cmd;
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::x(const cpp_int & _val) { 
    // x
    int_var__x = _val.convert_to< x_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::x() const {
    return int_var__x;
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::y(const cpp_int & _val) { 
    // y
    int_var__y = _val.convert_to< y_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::y() const {
    return int_var__y;
}
    
void cap_pict_csr_dhs_tcam_xy_entry_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_pict_csr_dhs_tcam_xy_entry_t::valid() const {
    return int_var__valid;
}
    
void cap_pict_csr_sta_tcam_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_pict_csr_sta_tcam_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_pict_csr_cfg_tcam_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_pict_csr_cnt_tcam_search_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_pict_csr_cnt_tcam_search_t::val() const {
    return int_var__val;
}
    
void cap_pict_csr_cnt_axi_pot_rdrsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_pict_csr_cnt_axi_pot_rdrsp_t::val() const {
    return int_var__val;
}
    
void cap_pict_csr_cnt_axi_pot_rdreq_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_pict_csr_cnt_axi_pot_rdreq_t::val() const {
    return int_var__val;
}
    
void cap_pict_csr_sta_tcam_srch_t::hit(const cpp_int & _val) { 
    // hit
    int_var__hit = _val.convert_to< hit_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_srch_t::hit() const {
    return int_var__hit;
}
    
void cap_pict_csr_sta_tcam_srch_t::hit_addr(const cpp_int & _val) { 
    // hit_addr
    int_var__hit_addr = _val.convert_to< hit_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_sta_tcam_srch_t::hit_addr() const {
    return int_var__hit_addr;
}
    
void cap_pict_csr_cfg_tcam_srch_t::key(const cpp_int & _val) { 
    // key
    int_var__key = _val.convert_to< key_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_srch_t::key() const {
    return int_var__key;
}
    
void cap_pict_csr_cfg_tcam_srch_t::mask(const cpp_int & _val) { 
    // mask
    int_var__mask = _val.convert_to< mask_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_srch_t::mask() const {
    return int_var__mask;
}
    
void cap_pict_csr_cfg_tcam_srch_t::tbl_mask(const cpp_int & _val) { 
    // tbl_mask
    int_var__tbl_mask = _val.convert_to< tbl_mask_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_srch_t::tbl_mask() const {
    return int_var__tbl_mask;
}
    
void cap_pict_csr_cfg_tcam_reset_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_reset_t::enable() const {
    return int_var__enable;
}
    
void cap_pict_csr_cfg_tcam_reset_t::vec(const cpp_int & _val) { 
    // vec
    int_var__vec = _val.convert_to< vec_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_reset_t::vec() const {
    return int_var__vec;
}
    
void cap_pict_csr_cfg_tcam_table_partition_t::vbe(const cpp_int & _val) { 
    // vbe
    int_var__vbe = _val.convert_to< vbe_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_partition_t::vbe() const {
    return int_var__vbe;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::start_addr(const cpp_int & _val) { 
    // start_addr
    int_var__start_addr = _val.convert_to< start_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::start_addr() const {
    return int_var__start_addr;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::end_addr(const cpp_int & _val) { 
    // end_addr
    int_var__end_addr = _val.convert_to< end_addr_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::end_addr() const {
    return int_var__end_addr;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::width(const cpp_int & _val) { 
    // width
    int_var__width = _val.convert_to< width_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::width() const {
    return int_var__width;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::en_tbid(const cpp_int & _val) { 
    // en_tbid
    int_var__en_tbid = _val.convert_to< en_tbid_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::en_tbid() const {
    return int_var__en_tbid;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::tbid(const cpp_int & _val) { 
    // tbid
    int_var__tbid = _val.convert_to< tbid_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::tbid() const {
    return int_var__tbid;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::bkts(const cpp_int & _val) { 
    // bkts
    int_var__bkts = _val.convert_to< bkts_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::bkts() const {
    return int_var__bkts;
}
    
void cap_pict_csr_cfg_tcam_table_profile_t::keyshift(const cpp_int & _val) { 
    // keyshift
    int_var__keyshift = _val.convert_to< keyshift_cpp_int_t >();
}

cpp_int cap_pict_csr_cfg_tcam_table_profile_t::keyshift() const {
    return int_var__keyshift;
}
    
void cap_pict_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pict_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_srch_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmd")) { field_val = cmd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_srch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_xy_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "x")) { field_val = x(); field_found=1; }
    if(!field_found && !strcmp(field_name, "y")) { field_val = y(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_xy_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_sta_tcam_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_tcam_search_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_axi_pot_rdrsp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_axi_pot_rdreq_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_sta_tcam_srch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hit")) { field_val = hit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hit_addr")) { field_val = hit_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_srch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { field_val = key(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { field_val = mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_mask")) { field_val = tbl_mask(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_reset_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "vec")) { field_val = vec(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_table_partition_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vbe")) { field_val = vbe(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_table_profile_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start_addr")) { field_val = start_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_addr")) { field_val = end_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "width")) { field_val = width(); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_tbid")) { field_val = en_tbid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbid")) { field_val = tbid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bkts")) { field_val = bkts(); field_found=1; }
    if(!field_found && !strcmp(field_name, "keyshift")) { field_val = keyshift(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam_reset.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam_srch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tcam.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tcam_xy.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tcam_srch.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_srch_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmd")) { cmd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_srch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_xy_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "x")) { x(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "y")) { y(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_dhs_tcam_xy_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_sta_tcam_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_tcam_search_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_axi_pot_rdrsp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cnt_axi_pot_rdreq_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_sta_tcam_srch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hit")) { hit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hit_addr")) { hit_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_srch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "key")) { key(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask")) { mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_mask")) { tbl_mask(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_reset_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "vec")) { vec(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_table_partition_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "vbe")) { vbe(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_cfg_tcam_table_profile_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start_addr")) { start_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "end_addr")) { end_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "width")) { width(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "en_tbid")) { en_tbid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbid")) { tbid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bkts")) { bkts(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "keyshift")) { keyshift(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pict_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam_reset.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam_srch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tcam.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tcam.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tcam_xy.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tcam_srch.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_dhs_tcam_srch_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_dhs_tcam_srch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_dhs_tcam_xy_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("x");
    ret_vec.push_back("y");
    ret_vec.push_back("valid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_dhs_tcam_xy_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_sta_tcam_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cfg_tcam_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cnt_tcam_search_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cnt_axi_pot_rdrsp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cnt_axi_pot_rdreq_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_sta_tcam_srch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hit");
    ret_vec.push_back("hit_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cfg_tcam_srch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("key");
    ret_vec.push_back("mask");
    ret_vec.push_back("tbl_mask");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cfg_tcam_reset_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("vec");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cfg_tcam_table_partition_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("vbe");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_cfg_tcam_table_profile_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("start_addr");
    ret_vec.push_back("end_addr");
    ret_vec.push_back("width");
    ret_vec.push_back("en_tbid");
    ret_vec.push_back("tbid");
    ret_vec.push_back("bkts");
    ret_vec.push_back("keyshift");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pict_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tcam_reset.get_fields(level-1)) {
            ret_vec.push_back("cfg_tcam_reset." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tcam_srch.get_fields(level-1)) {
            ret_vec.push_back("cfg_tcam_srch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tcam.get_fields(level-1)) {
            ret_vec.push_back("cfg_tcam." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_tcam.get_fields(level-1)) {
            ret_vec.push_back("sta_tcam." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_tcam_xy.get_fields(level-1)) {
            ret_vec.push_back("dhs_tcam_xy." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_tcam_srch.get_fields(level-1)) {
            ret_vec.push_back("dhs_tcam_srch." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
