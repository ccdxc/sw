
#include "cap_pbchbmtx_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::~cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t() { }

cap_pbchbmtx_csr_dhs_hbm_cdt_t::cap_pbchbmtx_csr_dhs_hbm_cdt_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_dhs_hbm_cdt_t::~cap_pbchbmtx_csr_dhs_hbm_cdt_t() { }

cap_pbchbmtx_csr_cfg_hbm_arb_t::cap_pbchbmtx_csr_cfg_hbm_arb_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_arb_t::~cap_pbchbmtx_csr_cfg_hbm_arb_t() { }

cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::cap_pbchbmtx_csr_cfg_hbm_read_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::~cap_pbchbmtx_csr_cfg_hbm_read_fifo_t() { }

cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::~cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t() { }

cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::cap_pbchbmtx_csr_cfg_hbm_tx_payload_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::~cap_pbchbmtx_csr_cfg_hbm_tx_payload_t() { }

cap_pbchbmtx_csr_cnt_hbm_cut_t::cap_pbchbmtx_csr_cnt_hbm_cut_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cnt_hbm_cut_t::~cap_pbchbmtx_csr_cnt_hbm_cut_t() { }

cap_pbchbmtx_csr_cfg_hbm_addr_t::cap_pbchbmtx_csr_cfg_hbm_addr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_addr_t::~cap_pbchbmtx_csr_cfg_hbm_addr_t() { }

cap_pbchbmtx_csr_cnt_hbm_t::cap_pbchbmtx_csr_cnt_hbm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cnt_hbm_t::~cap_pbchbmtx_csr_cnt_hbm_t() { }

cap_pbchbmtx_csr_cfg_hbm_t::cap_pbchbmtx_csr_cfg_hbm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_t::~cap_pbchbmtx_csr_cfg_hbm_t() { }

cap_pbchbmtx_csr_cfg_hbm_context_t::cap_pbchbmtx_csr_cfg_hbm_context_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_context_t::~cap_pbchbmtx_csr_cfg_hbm_context_t() { }

cap_pbchbmtx_csr_cfg_hbm_mem_t::cap_pbchbmtx_csr_cfg_hbm_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_mem_t::~cap_pbchbmtx_csr_cfg_hbm_mem_t() { }

cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::~cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t() { }

cap_pbchbmtx_csr_t::cap_pbchbmtx_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(512);
        set_attributes(0,get_name(), 0);
        }
cap_pbchbmtx_csr_t::~cap_pbchbmtx_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".commmand: 0x" << int_var__commmand << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".credit: 0x" << int_var__credit << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".quota: 0x" << int_var__quota << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_dhs_hbm_cdt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    entry.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_arb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".strict_priority: 0x" << int_var__strict_priority << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_wrr: 0x" << int_var__enable_wrr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_selection: 0x" << int_var__dhs_selection << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".max_packets: 0x" << int_var__max_packets << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_flits: 0x" << int_var__max_flits << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_cells: 0x" << int_var__max_cells << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".threshold: 0x" << int_var__threshold << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".base: 0x" << int_var__base << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mem_sz: 0x" << int_var__mem_sz << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cnt_hbm_cut_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".thru: 0x" << int_var__thru << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_addr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".swizzle: 0x" << int_var__swizzle << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cnt_hbm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".flits_sop_in: 0x" << int_var__flits_sop_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".flits_eop_in: 0x" << int_var__flits_eop_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".flits_sop_out: 0x" << int_var__flits_sop_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".flits_eop_out: 0x" << int_var__flits_eop_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".occupancy_stop: 0x" << int_var__occupancy_stop << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".rate_limiter: 0x" << int_var__rate_limiter << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rewrite: 0x" << int_var__rewrite << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".depth_shift: 0x" << int_var__depth_shift << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_pop_size_enable: 0x" << int_var__max_pop_size_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_multiple_reads: 0x" << int_var__enable_multiple_reads << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_context_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".xoff_timeout: 0x" << int_var__xoff_timeout << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".no_drop: 0x" << int_var__no_drop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".base: 0x" << int_var__base << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_selection: 0x" << int_var__dhs_selection << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".table: 0x" << int_var__table << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbchbmtx_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    cfg_hbm_tc_to_q.show();
    cfg_hbm_mem.show();
    cfg_hbm_context.show();
    cfg_hbm.show();
    cnt_hbm.show();
    cfg_hbm_addr.show();
    cnt_hbm_cut.show();
    cfg_hbm_tx_payload.show();
    cfg_hbm_tx_payload_occupancy.show();
    cfg_hbm_read_fifo.show();
    cfg_hbm_arb.show();
    dhs_hbm_cdt.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::get_width() const {
    return cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::s_get_width();

}

int cap_pbchbmtx_csr_dhs_hbm_cdt_t::get_width() const {
    return cap_pbchbmtx_csr_dhs_hbm_cdt_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_arb_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_arb_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::s_get_width();

}

int cap_pbchbmtx_csr_cnt_hbm_cut_t::get_width() const {
    return cap_pbchbmtx_csr_cnt_hbm_cut_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_addr_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_addr_t::s_get_width();

}

int cap_pbchbmtx_csr_cnt_hbm_t::get_width() const {
    return cap_pbchbmtx_csr_cnt_hbm_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_context_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_context_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_mem_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_mem_t::s_get_width();

}

int cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::get_width() const {
    return cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::s_get_width();

}

int cap_pbchbmtx_csr_t::get_width() const {
    return cap_pbchbmtx_csr_t::s_get_width();

}

int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::s_get_width() {
    int _count = 0;

    _count += 2; // commmand
    _count += 32; // credit
    _count += 32; // quota
    return _count;
}

int cap_pbchbmtx_csr_dhs_hbm_cdt_t::s_get_width() {
    int _count = 0;

    _count += cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::s_get_width(); // entry
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_arb_t::s_get_width() {
    int _count = 0;

    _count += 2; // strict_priority
    _count += 1; // enable_wrr
    _count += 1; // dhs_selection
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::s_get_width() {
    int _count = 0;

    _count += 8; // max_packets
    _count += 9; // max_flits
    _count += 12; // max_cells
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::s_get_width() {
    int _count = 0;

    _count += 304; // threshold
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::s_get_width() {
    int _count = 0;

    _count += 432; // base
    _count += 368; // mem_sz
    return _count;
}

int cap_pbchbmtx_csr_cnt_hbm_cut_t::s_get_width() {
    int _count = 0;

    _count += 16; // thru
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_addr_t::s_get_width() {
    int _count = 0;

    _count += 1; // swizzle
    return _count;
}

int cap_pbchbmtx_csr_cnt_hbm_t::s_get_width() {
    int _count = 0;

    _count += 32; // flits_sop_in
    _count += 32; // flits_eop_in
    _count += 32; // flits_sop_out
    _count += 32; // flits_eop_out
    _count += 32; // occupancy_stop
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_t::s_get_width() {
    int _count = 0;

    _count += 4; // rate_limiter
    _count += 1; // rewrite
    _count += 7; // depth_shift
    _count += 1; // max_pop_size_enable
    _count += 1; // enable_multiple_reads
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_context_t::s_get_width() {
    int _count = 0;

    _count += 27; // xoff_timeout
    _count += 5; // rsvd
    _count += 16; // enable
    _count += 16; // no_drop
    _count += 5; // base
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // dhs_eccbypass
    _count += 1; // dhs_selection
    _count += 2; // ecc_disable_cor
    _count += 2; // ecc_disable_det
    _count += 2; // bist_run
    return _count;
}

int cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::s_get_width() {
    int _count = 0;

    _count += 64; // table
    return _count;
}

int cap_pbchbmtx_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::s_get_width(); // cfg_hbm_tc_to_q
    _count += cap_pbchbmtx_csr_cfg_hbm_mem_t::s_get_width(); // cfg_hbm_mem
    _count += cap_pbchbmtx_csr_cfg_hbm_context_t::s_get_width(); // cfg_hbm_context
    _count += cap_pbchbmtx_csr_cfg_hbm_t::s_get_width(); // cfg_hbm
    _count += cap_pbchbmtx_csr_cnt_hbm_t::s_get_width(); // cnt_hbm
    _count += cap_pbchbmtx_csr_cfg_hbm_addr_t::s_get_width(); // cfg_hbm_addr
    _count += cap_pbchbmtx_csr_cnt_hbm_cut_t::s_get_width(); // cnt_hbm_cut
    _count += cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::s_get_width(); // cfg_hbm_tx_payload
    _count += cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::s_get_width(); // cfg_hbm_tx_payload_occupancy
    _count += cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::s_get_width(); // cfg_hbm_read_fifo
    _count += cap_pbchbmtx_csr_cfg_hbm_arb_t::s_get_width(); // cfg_hbm_arb
    _count += cap_pbchbmtx_csr_dhs_hbm_cdt_t::s_get_width(); // dhs_hbm_cdt
    return _count;
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__commmand = _val.convert_to< commmand_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__credit = _val.convert_to< credit_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__quota = _val.convert_to< quota_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    entry.all( _val);
    _val = _val >> entry.get_width(); 
}

void cap_pbchbmtx_csr_cfg_hbm_arb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__strict_priority = _val.convert_to< strict_priority_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__enable_wrr = _val.convert_to< enable_wrr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_selection = _val.convert_to< dhs_selection_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__max_packets = _val.convert_to< max_packets_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__max_flits = _val.convert_to< max_flits_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__max_cells = _val.convert_to< max_cells_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__threshold = _val.convert_to< threshold_cpp_int_t >()  ;
    _val = _val >> 304;
    
}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__base = _val.convert_to< base_cpp_int_t >()  ;
    _val = _val >> 432;
    
    int_var__mem_sz = _val.convert_to< mem_sz_cpp_int_t >()  ;
    _val = _val >> 368;
    
}

void cap_pbchbmtx_csr_cnt_hbm_cut_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__thru = _val.convert_to< thru_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_pbchbmtx_csr_cfg_hbm_addr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__swizzle = _val.convert_to< swizzle_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pbchbmtx_csr_cnt_hbm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__flits_sop_in = _val.convert_to< flits_sop_in_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__flits_eop_in = _val.convert_to< flits_eop_in_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__flits_sop_out = _val.convert_to< flits_sop_out_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__flits_eop_out = _val.convert_to< flits_eop_out_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__occupancy_stop = _val.convert_to< occupancy_stop_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_pbchbmtx_csr_cfg_hbm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__rate_limiter = _val.convert_to< rate_limiter_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__rewrite = _val.convert_to< rewrite_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__depth_shift = _val.convert_to< depth_shift_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__max_pop_size_enable = _val.convert_to< max_pop_size_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__enable_multiple_reads = _val.convert_to< enable_multiple_reads_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pbchbmtx_csr_cfg_hbm_context_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__xoff_timeout = _val.convert_to< xoff_timeout_cpp_int_t >()  ;
    _val = _val >> 27;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__no_drop = _val.convert_to< no_drop_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__base = _val.convert_to< base_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_pbchbmtx_csr_cfg_hbm_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_selection = _val.convert_to< dhs_selection_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__table = _val.convert_to< table_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_pbchbmtx_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    cfg_hbm_tc_to_q.all( _val);
    _val = _val >> cfg_hbm_tc_to_q.get_width(); 
    cfg_hbm_mem.all( _val);
    _val = _val >> cfg_hbm_mem.get_width(); 
    cfg_hbm_context.all( _val);
    _val = _val >> cfg_hbm_context.get_width(); 
    cfg_hbm.all( _val);
    _val = _val >> cfg_hbm.get_width(); 
    cnt_hbm.all( _val);
    _val = _val >> cnt_hbm.get_width(); 
    cfg_hbm_addr.all( _val);
    _val = _val >> cfg_hbm_addr.get_width(); 
    cnt_hbm_cut.all( _val);
    _val = _val >> cnt_hbm_cut.get_width(); 
    cfg_hbm_tx_payload.all( _val);
    _val = _val >> cfg_hbm_tx_payload.get_width(); 
    cfg_hbm_tx_payload_occupancy.all( _val);
    _val = _val >> cfg_hbm_tx_payload_occupancy.get_width(); 
    cfg_hbm_read_fifo.all( _val);
    _val = _val >> cfg_hbm_read_fifo.get_width(); 
    cfg_hbm_arb.all( _val);
    _val = _val >> cfg_hbm_arb.get_width(); 
    dhs_hbm_cdt.all( _val);
    _val = _val >> dhs_hbm_cdt.get_width(); 
}

cpp_int cap_pbchbmtx_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << dhs_hbm_cdt.get_width(); ret_val = ret_val  | dhs_hbm_cdt.all(); 
    ret_val = ret_val << cfg_hbm_arb.get_width(); ret_val = ret_val  | cfg_hbm_arb.all(); 
    ret_val = ret_val << cfg_hbm_read_fifo.get_width(); ret_val = ret_val  | cfg_hbm_read_fifo.all(); 
    ret_val = ret_val << cfg_hbm_tx_payload_occupancy.get_width(); ret_val = ret_val  | cfg_hbm_tx_payload_occupancy.all(); 
    ret_val = ret_val << cfg_hbm_tx_payload.get_width(); ret_val = ret_val  | cfg_hbm_tx_payload.all(); 
    ret_val = ret_val << cnt_hbm_cut.get_width(); ret_val = ret_val  | cnt_hbm_cut.all(); 
    ret_val = ret_val << cfg_hbm_addr.get_width(); ret_val = ret_val  | cfg_hbm_addr.all(); 
    ret_val = ret_val << cnt_hbm.get_width(); ret_val = ret_val  | cnt_hbm.all(); 
    ret_val = ret_val << cfg_hbm.get_width(); ret_val = ret_val  | cfg_hbm.all(); 
    ret_val = ret_val << cfg_hbm_context.get_width(); ret_val = ret_val  | cfg_hbm_context.all(); 
    ret_val = ret_val << cfg_hbm_mem.get_width(); ret_val = ret_val  | cfg_hbm_mem.all(); 
    ret_val = ret_val << cfg_hbm_tc_to_q.get_width(); ret_val = ret_val  | cfg_hbm_tc_to_q.all(); 
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::all() const {
    cpp_int ret_val;

    // table
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__table; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_disable_det
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // ecc_disable_cor
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    // dhs_selection
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_selection; 
    
    // dhs_eccbypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_eccbypass; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::all() const {
    cpp_int ret_val;

    // base
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__base; 
    
    // no_drop
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__no_drop; 
    
    // enable
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__enable; 
    
    // rsvd
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__rsvd; 
    
    // xoff_timeout
    ret_val = ret_val << 27; ret_val = ret_val  | int_var__xoff_timeout; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::all() const {
    cpp_int ret_val;

    // enable_multiple_reads
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_multiple_reads; 
    
    // max_pop_size_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_pop_size_enable; 
    
    // depth_shift
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__depth_shift; 
    
    // rewrite
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rewrite; 
    
    // rate_limiter
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__rate_limiter; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::all() const {
    cpp_int ret_val;

    // occupancy_stop
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__occupancy_stop; 
    
    // flits_eop_out
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__flits_eop_out; 
    
    // flits_sop_out
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__flits_sop_out; 
    
    // flits_eop_in
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__flits_eop_in; 
    
    // flits_sop_in
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__flits_sop_in; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_addr_t::all() const {
    cpp_int ret_val;

    // swizzle
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__swizzle; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_cut_t::all() const {
    cpp_int ret_val;

    // thru
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__thru; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::all() const {
    cpp_int ret_val;

    // mem_sz
    ret_val = ret_val << 368; ret_val = ret_val  | int_var__mem_sz; 
    
    // base
    ret_val = ret_val << 432; ret_val = ret_val  | int_var__base; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::all() const {
    cpp_int ret_val;

    // threshold
    ret_val = ret_val << 304; ret_val = ret_val  | int_var__threshold; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::all() const {
    cpp_int ret_val;

    // max_cells
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__max_cells; 
    
    // max_flits
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__max_flits; 
    
    // max_packets
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__max_packets; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_arb_t::all() const {
    cpp_int ret_val;

    // dhs_selection
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_selection; 
    
    // enable_wrr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_wrr; 
    
    // strict_priority
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__strict_priority; 
    
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_dhs_hbm_cdt_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << entry.get_width(); ret_val = ret_val  | entry.all(); 
    return ret_val;
}

cpp_int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::all() const {
    cpp_int ret_val;

    // quota
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__quota; 
    
    // credit
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__credit; 
    
    // commmand
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__commmand; 
    
    return ret_val;
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::clear() {

    int_var__commmand = 0; 
    
    int_var__credit = 0; 
    
    int_var__quota = 0; 
    
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_t::clear() {

    entry.clear();
}

void cap_pbchbmtx_csr_cfg_hbm_arb_t::clear() {

    int_var__strict_priority = 0; 
    
    int_var__enable_wrr = 0; 
    
    int_var__dhs_selection = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::clear() {

    int_var__max_packets = 0; 
    
    int_var__max_flits = 0; 
    
    int_var__max_cells = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::clear() {

    int_var__threshold = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::clear() {

    int_var__base = 0; 
    
    int_var__mem_sz = 0; 
    
}

void cap_pbchbmtx_csr_cnt_hbm_cut_t::clear() {

    int_var__thru = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_addr_t::clear() {

    int_var__swizzle = 0; 
    
}

void cap_pbchbmtx_csr_cnt_hbm_t::clear() {

    int_var__flits_sop_in = 0; 
    
    int_var__flits_eop_in = 0; 
    
    int_var__flits_sop_out = 0; 
    
    int_var__flits_eop_out = 0; 
    
    int_var__occupancy_stop = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_t::clear() {

    int_var__rate_limiter = 0; 
    
    int_var__rewrite = 0; 
    
    int_var__depth_shift = 0; 
    
    int_var__max_pop_size_enable = 0; 
    
    int_var__enable_multiple_reads = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_context_t::clear() {

    int_var__xoff_timeout = 0; 
    
    int_var__rsvd = 0; 
    
    int_var__enable = 0; 
    
    int_var__no_drop = 0; 
    
    int_var__base = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_mem_t::clear() {

    int_var__dhs_eccbypass = 0; 
    
    int_var__dhs_selection = 0; 
    
    int_var__ecc_disable_cor = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::clear() {

    int_var__table = 0; 
    
}

void cap_pbchbmtx_csr_t::clear() {

    cfg_hbm_tc_to_q.clear();
    cfg_hbm_mem.clear();
    cfg_hbm_context.clear();
    cfg_hbm.clear();
    cnt_hbm.clear();
    cfg_hbm_addr.clear();
    cnt_hbm_cut.clear();
    cfg_hbm_tx_payload.clear();
    cfg_hbm_tx_payload_occupancy.clear();
    cfg_hbm_read_fifo.clear();
    cfg_hbm_arb.clear();
    dhs_hbm_cdt.clear();
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::init() {

}

void cap_pbchbmtx_csr_dhs_hbm_cdt_t::init() {

    entry.set_attributes(this,"entry", 0x0 );
}

void cap_pbchbmtx_csr_cfg_hbm_arb_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::init() {

    set_reset_val(cpp_int("0xf07828"));
    all(get_reset_val());
}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::init() {

}

void cap_pbchbmtx_csr_cnt_hbm_cut_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_addr_t::init() {

}

void cap_pbchbmtx_csr_cnt_hbm_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_t::init() {

    set_reset_val(cpp_int("0x2013"));
    all(get_reset_val());
}

void cap_pbchbmtx_csr_cfg_hbm_context_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_mem_t::init() {

}

void cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::init() {

    set_reset_val(cpp_int("0xfedcba9876543210"));
    all(get_reset_val());
}

void cap_pbchbmtx_csr_t::init() {

    cfg_hbm_tc_to_q.set_attributes(this,"cfg_hbm_tc_to_q", 0x0 );
    cfg_hbm_mem.set_attributes(this,"cfg_hbm_mem", 0x8 );
    cfg_hbm_context.set_attributes(this,"cfg_hbm_context", 0x10 );
    cfg_hbm.set_attributes(this,"cfg_hbm", 0x20 );
    cnt_hbm.set_attributes(this,"cnt_hbm", 0x40 );
    cfg_hbm_addr.set_attributes(this,"cfg_hbm_addr", 0x60 );
    cnt_hbm_cut.set_attributes(this,"cnt_hbm_cut", 0x64 );
    cfg_hbm_tx_payload.set_attributes(this,"cfg_hbm_tx_payload", 0x80 );
    cfg_hbm_tx_payload_occupancy.set_attributes(this,"cfg_hbm_tx_payload_occupancy", 0x100 );
    cfg_hbm_read_fifo.set_attributes(this,"cfg_hbm_read_fifo", 0x140 );
    cfg_hbm_arb.set_attributes(this,"cfg_hbm_arb", 0x144 );
    dhs_hbm_cdt.set_attributes(this,"dhs_hbm_cdt", 0x150 );
}

void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::commmand(const cpp_int & _val) { 
    // commmand
    int_var__commmand = _val.convert_to< commmand_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::commmand() const {
    return int_var__commmand;
}
    
void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::credit(const cpp_int & _val) { 
    // credit
    int_var__credit = _val.convert_to< credit_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::credit() const {
    return int_var__credit;
}
    
void cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::quota(const cpp_int & _val) { 
    // quota
    int_var__quota = _val.convert_to< quota_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::quota() const {
    return int_var__quota;
}
    
void cap_pbchbmtx_csr_cfg_hbm_arb_t::strict_priority(const cpp_int & _val) { 
    // strict_priority
    int_var__strict_priority = _val.convert_to< strict_priority_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_arb_t::strict_priority() const {
    return int_var__strict_priority;
}
    
void cap_pbchbmtx_csr_cfg_hbm_arb_t::enable_wrr(const cpp_int & _val) { 
    // enable_wrr
    int_var__enable_wrr = _val.convert_to< enable_wrr_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_arb_t::enable_wrr() const {
    return int_var__enable_wrr;
}
    
void cap_pbchbmtx_csr_cfg_hbm_arb_t::dhs_selection(const cpp_int & _val) { 
    // dhs_selection
    int_var__dhs_selection = _val.convert_to< dhs_selection_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_arb_t::dhs_selection() const {
    return int_var__dhs_selection;
}
    
void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_packets(const cpp_int & _val) { 
    // max_packets
    int_var__max_packets = _val.convert_to< max_packets_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_packets() const {
    return int_var__max_packets;
}
    
void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_flits(const cpp_int & _val) { 
    // max_flits
    int_var__max_flits = _val.convert_to< max_flits_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_flits() const {
    return int_var__max_flits;
}
    
void cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_cells(const cpp_int & _val) { 
    // max_cells
    int_var__max_cells = _val.convert_to< max_cells_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::max_cells() const {
    return int_var__max_cells;
}
    
void cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::threshold(const cpp_int & _val) { 
    // threshold
    int_var__threshold = _val.convert_to< threshold_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::threshold() const {
    return int_var__threshold;
}
    
void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::base(const cpp_int & _val) { 
    // base
    int_var__base = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::base() const {
    return int_var__base;
}
    
void cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::mem_sz(const cpp_int & _val) { 
    // mem_sz
    int_var__mem_sz = _val.convert_to< mem_sz_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::mem_sz() const {
    return int_var__mem_sz;
}
    
void cap_pbchbmtx_csr_cnt_hbm_cut_t::thru(const cpp_int & _val) { 
    // thru
    int_var__thru = _val.convert_to< thru_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_cut_t::thru() const {
    return int_var__thru;
}
    
void cap_pbchbmtx_csr_cfg_hbm_addr_t::swizzle(const cpp_int & _val) { 
    // swizzle
    int_var__swizzle = _val.convert_to< swizzle_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_addr_t::swizzle() const {
    return int_var__swizzle;
}
    
void cap_pbchbmtx_csr_cnt_hbm_t::flits_sop_in(const cpp_int & _val) { 
    // flits_sop_in
    int_var__flits_sop_in = _val.convert_to< flits_sop_in_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::flits_sop_in() const {
    return int_var__flits_sop_in;
}
    
void cap_pbchbmtx_csr_cnt_hbm_t::flits_eop_in(const cpp_int & _val) { 
    // flits_eop_in
    int_var__flits_eop_in = _val.convert_to< flits_eop_in_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::flits_eop_in() const {
    return int_var__flits_eop_in;
}
    
void cap_pbchbmtx_csr_cnt_hbm_t::flits_sop_out(const cpp_int & _val) { 
    // flits_sop_out
    int_var__flits_sop_out = _val.convert_to< flits_sop_out_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::flits_sop_out() const {
    return int_var__flits_sop_out;
}
    
void cap_pbchbmtx_csr_cnt_hbm_t::flits_eop_out(const cpp_int & _val) { 
    // flits_eop_out
    int_var__flits_eop_out = _val.convert_to< flits_eop_out_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::flits_eop_out() const {
    return int_var__flits_eop_out;
}
    
void cap_pbchbmtx_csr_cnt_hbm_t::occupancy_stop(const cpp_int & _val) { 
    // occupancy_stop
    int_var__occupancy_stop = _val.convert_to< occupancy_stop_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cnt_hbm_t::occupancy_stop() const {
    return int_var__occupancy_stop;
}
    
void cap_pbchbmtx_csr_cfg_hbm_t::rate_limiter(const cpp_int & _val) { 
    // rate_limiter
    int_var__rate_limiter = _val.convert_to< rate_limiter_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::rate_limiter() const {
    return int_var__rate_limiter;
}
    
void cap_pbchbmtx_csr_cfg_hbm_t::rewrite(const cpp_int & _val) { 
    // rewrite
    int_var__rewrite = _val.convert_to< rewrite_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::rewrite() const {
    return int_var__rewrite;
}
    
void cap_pbchbmtx_csr_cfg_hbm_t::depth_shift(const cpp_int & _val) { 
    // depth_shift
    int_var__depth_shift = _val.convert_to< depth_shift_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::depth_shift() const {
    return int_var__depth_shift;
}
    
void cap_pbchbmtx_csr_cfg_hbm_t::max_pop_size_enable(const cpp_int & _val) { 
    // max_pop_size_enable
    int_var__max_pop_size_enable = _val.convert_to< max_pop_size_enable_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::max_pop_size_enable() const {
    return int_var__max_pop_size_enable;
}
    
void cap_pbchbmtx_csr_cfg_hbm_t::enable_multiple_reads(const cpp_int & _val) { 
    // enable_multiple_reads
    int_var__enable_multiple_reads = _val.convert_to< enable_multiple_reads_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_t::enable_multiple_reads() const {
    return int_var__enable_multiple_reads;
}
    
void cap_pbchbmtx_csr_cfg_hbm_context_t::xoff_timeout(const cpp_int & _val) { 
    // xoff_timeout
    int_var__xoff_timeout = _val.convert_to< xoff_timeout_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::xoff_timeout() const {
    return int_var__xoff_timeout;
}
    
void cap_pbchbmtx_csr_cfg_hbm_context_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_pbchbmtx_csr_cfg_hbm_context_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::enable() const {
    return int_var__enable;
}
    
void cap_pbchbmtx_csr_cfg_hbm_context_t::no_drop(const cpp_int & _val) { 
    // no_drop
    int_var__no_drop = _val.convert_to< no_drop_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::no_drop() const {
    return int_var__no_drop;
}
    
void cap_pbchbmtx_csr_cfg_hbm_context_t::base(const cpp_int & _val) { 
    // base
    int_var__base = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_context_t::base() const {
    return int_var__base;
}
    
void cap_pbchbmtx_csr_cfg_hbm_mem_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass;
}
    
void cap_pbchbmtx_csr_cfg_hbm_mem_t::dhs_selection(const cpp_int & _val) { 
    // dhs_selection
    int_var__dhs_selection = _val.convert_to< dhs_selection_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::dhs_selection() const {
    return int_var__dhs_selection;
}
    
void cap_pbchbmtx_csr_cfg_hbm_mem_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_pbchbmtx_csr_cfg_hbm_mem_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_pbchbmtx_csr_cfg_hbm_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::table(const cpp_int & _val) { 
    // table
    int_var__table = _val.convert_to< table_cpp_int_t >();
}

cpp_int cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::table() const {
    return int_var__table;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "commmand")) { field_val = commmand(); field_found=1; }
    if(!field_found && !strcmp(field_name, "credit")) { field_val = credit(); field_found=1; }
    if(!field_found && !strcmp(field_name, "quota")) { field_val = quota(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_dhs_hbm_cdt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = entry.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_arb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "strict_priority")) { field_val = strict_priority(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_wrr")) { field_val = enable_wrr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_selection")) { field_val = dhs_selection(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "max_packets")) { field_val = max_packets(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_flits")) { field_val = max_flits(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_cells")) { field_val = max_cells(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "threshold")) { field_val = threshold(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "base")) { field_val = base(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mem_sz")) { field_val = mem_sz(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cnt_hbm_cut_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "thru")) { field_val = thru(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_addr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "swizzle")) { field_val = swizzle(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cnt_hbm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "flits_sop_in")) { field_val = flits_sop_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_eop_in")) { field_val = flits_eop_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_sop_out")) { field_val = flits_sop_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_eop_out")) { field_val = flits_eop_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "occupancy_stop")) { field_val = occupancy_stop(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rate_limiter")) { field_val = rate_limiter(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rewrite")) { field_val = rewrite(); field_found=1; }
    if(!field_found && !strcmp(field_name, "depth_shift")) { field_val = depth_shift(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_pop_size_enable")) { field_val = max_pop_size_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_multiple_reads")) { field_val = enable_multiple_reads(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_context_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "xoff_timeout")) { field_val = xoff_timeout(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_drop")) { field_val = no_drop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "base")) { field_val = base(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { field_val = dhs_eccbypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_selection")) { field_val = dhs_selection(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "table")) { field_val = table(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = cfg_hbm_tc_to_q.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_context.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_hbm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_addr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_hbm_cut.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_tx_payload.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_tx_payload_occupancy.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_read_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_arb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_hbm_cdt.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "commmand")) { commmand(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "credit")) { credit(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "quota")) { quota(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_dhs_hbm_cdt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = entry.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_arb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "strict_priority")) { strict_priority(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_wrr")) { enable_wrr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_selection")) { dhs_selection(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "max_packets")) { max_packets(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_flits")) { max_flits(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_cells")) { max_cells(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "threshold")) { threshold(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "base")) { base(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mem_sz")) { mem_sz(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cnt_hbm_cut_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "thru")) { thru(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_addr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "swizzle")) { swizzle(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cnt_hbm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "flits_sop_in")) { flits_sop_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_eop_in")) { flits_eop_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_sop_out")) { flits_sop_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "flits_eop_out")) { flits_eop_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "occupancy_stop")) { occupancy_stop(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "rate_limiter")) { rate_limiter(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rewrite")) { rewrite(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "depth_shift")) { depth_shift(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_pop_size_enable")) { max_pop_size_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_multiple_reads")) { enable_multiple_reads(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_context_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "xoff_timeout")) { xoff_timeout(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_drop")) { no_drop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "base")) { base(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { dhs_eccbypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_selection")) { dhs_selection(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "table")) { table(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbchbmtx_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = cfg_hbm_tc_to_q.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_context.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_hbm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_addr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_hbm_cut.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_tx_payload.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_tx_payload_occupancy.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_read_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_hbm_arb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_hbm_cdt.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("commmand");
    ret_vec.push_back("credit");
    ret_vec.push_back("quota");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_dhs_hbm_cdt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : entry.get_fields(level-1)) {
            ret_vec.push_back("entry." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_arb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("strict_priority");
    ret_vec.push_back("enable_wrr");
    ret_vec.push_back("dhs_selection");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_read_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("max_packets");
    ret_vec.push_back("max_flits");
    ret_vec.push_back("max_cells");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("threshold");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_tx_payload_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("base");
    ret_vec.push_back("mem_sz");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cnt_hbm_cut_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("thru");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_addr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("swizzle");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cnt_hbm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("flits_sop_in");
    ret_vec.push_back("flits_eop_in");
    ret_vec.push_back("flits_sop_out");
    ret_vec.push_back("flits_eop_out");
    ret_vec.push_back("occupancy_stop");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("rate_limiter");
    ret_vec.push_back("rewrite");
    ret_vec.push_back("depth_shift");
    ret_vec.push_back("max_pop_size_enable");
    ret_vec.push_back("enable_multiple_reads");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_context_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("xoff_timeout");
    ret_vec.push_back("rsvd");
    ret_vec.push_back("enable");
    ret_vec.push_back("no_drop");
    ret_vec.push_back("base");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dhs_eccbypass");
    ret_vec.push_back("dhs_selection");
    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("table");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbchbmtx_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : cfg_hbm_tc_to_q.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_tc_to_q." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_context.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_context." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_hbm.get_fields(level-1)) {
            ret_vec.push_back("cnt_hbm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_addr.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_addr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_hbm_cut.get_fields(level-1)) {
            ret_vec.push_back("cnt_hbm_cut." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_tx_payload.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_tx_payload." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_tx_payload_occupancy.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_tx_payload_occupancy." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_read_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_read_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_hbm_arb.get_fields(level-1)) {
            ret_vec.push_back("cfg_hbm_arb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_hbm_cdt.get_fields(level-1)) {
            ret_vec.push_back("dhs_hbm_cdt." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
