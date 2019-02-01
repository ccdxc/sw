
#include "cap_pbc_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pbc_replicate_node32_t::cap_pbc_replicate_node32_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_replicate_node32_t::~cap_pbc_replicate_node32_t() { }

cap_pbc_replicate_node48_t::cap_pbc_replicate_node48_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_replicate_node48_t::~cap_pbc_replicate_node48_t() { }

cap_pbc_replicate_node64_t::cap_pbc_replicate_node64_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_replicate_node64_t::~cap_pbc_replicate_node64_t() { }

cap_pbc_oq_map_t::cap_pbc_oq_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_oq_map_t::~cap_pbc_oq_map_t() { }

cap_pbc_pg8_map_t::cap_pbc_pg8_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_pg8_map_t::~cap_pbc_pg8_map_t() { }

cap_pbc_pg16_map_t::cap_pbc_pg16_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_pg16_map_t::~cap_pbc_pg16_map_t() { }

cap_pbc_pg32_map_t::cap_pbc_pg32_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_pg32_map_t::~cap_pbc_pg32_map_t() { }

cap_pbc_max_growth_map_t::cap_pbc_max_growth_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_max_growth_map_t::~cap_pbc_max_growth_map_t() { }

cap_pbc_hbm_eth_ctl_t::cap_pbc_hbm_eth_ctl_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_eth_ctl_t::~cap_pbc_hbm_eth_ctl_t() { }

cap_pbc_hbm_tx_ctl_t::cap_pbc_hbm_tx_ctl_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_tx_ctl_t::~cap_pbc_hbm_tx_ctl_t() { }

cap_pbc_hbm_eth_port_ctl_t::cap_pbc_hbm_eth_port_ctl_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_eth_port_ctl_t::~cap_pbc_hbm_eth_port_ctl_t() { }

cap_pbc_hbm_eth_occ_thr_t::cap_pbc_hbm_eth_occ_thr_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_eth_occ_thr_t::~cap_pbc_hbm_eth_occ_thr_t() { }

cap_pbc_hbm_tx_occ_thr_t::cap_pbc_hbm_tx_occ_thr_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_tx_occ_thr_t::~cap_pbc_hbm_tx_occ_thr_t() { }

cap_pbc_hbm_eth_xoff_thr_t::cap_pbc_hbm_eth_xoff_thr_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_eth_xoff_thr_t::~cap_pbc_hbm_eth_xoff_thr_t() { }

cap_pbc_hbm_eth_hdr_t::cap_pbc_hbm_eth_hdr_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_hbm_eth_hdr_t::~cap_pbc_hbm_eth_hdr_t() { }

cap_pbc_egr_tstamp_t::cap_pbc_egr_tstamp_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_egr_tstamp_t::~cap_pbc_egr_tstamp_t() { }

cap_pbc_eth_oq_xoff_map_t::cap_pbc_eth_oq_xoff_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_eth_oq_xoff_map_t::~cap_pbc_eth_oq_xoff_map_t() { }

cap_pbc_p4_oq_xoff_map_t::cap_pbc_p4_oq_xoff_map_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pbc_p4_oq_xoff_map_t::~cap_pbc_p4_oq_xoff_map_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_replicate_node32_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 15 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array token skipped" << endl);
    #else
    for(int ii = 0; ii < 15; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".token[" << ii <<"]: 0x" << int_var__token[ii] << dec << endl);
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".last_node: 0x" << int_var__last_node << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".token_cnt: 0x" << int_var__token_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".next_ptr: 0x" << int_var__next_ptr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_replicate_node48_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array token skipped" << endl);
    #else
    for(int ii = 0; ii < 10; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".token[" << ii <<"]: 0x" << int_var__token[ii] << dec << endl);
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".last_node: 0x" << int_var__last_node << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".token_cnt: 0x" << int_var__token_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".next_ptr: 0x" << int_var__next_ptr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_replicate_node64_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 7 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array token skipped" << endl);
    #else
    for(int ii = 0; ii < 7; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".token[" << ii <<"]: 0x" << int_var__token[ii] << dec << endl);
    }
    #endif
    
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last_node: 0x" << int_var__last_node << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".token_cnt: 0x" << int_var__token_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".next_ptr: 0x" << int_var__next_ptr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_oq_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array oqueue skipped" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".oqueue[" << ii <<"]: 0x" << int_var__oqueue[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_pg8_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array pg skipped" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".pg[" << ii <<"]: 0x" << int_var__pg[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_pg16_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array pg skipped" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".pg[" << ii <<"]: 0x" << int_var__pg[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_pg32_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array pg skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".pg[" << ii <<"]: 0x" << int_var__pg[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_max_growth_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array max_growth skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".max_growth[" << ii <<"]: 0x" << int_var__max_growth[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_eth_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array base skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".base[" << ii <<"]: 0x" << int_var__base[ii] << dec << endl);
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mem_sz skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mem_sz[" << ii <<"]: 0x" << int_var__mem_sz[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_tx_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array base skipped" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".base[" << ii <<"]: 0x" << int_var__base[ii] << dec << endl);
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mem_sz skipped" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mem_sz[" << ii <<"]: 0x" << int_var__mem_sz[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_eth_port_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array base skipped" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".base[" << ii <<"]: 0x" << int_var__base[ii] << dec << endl);
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array mem_sz skipped" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".mem_sz[" << ii <<"]: 0x" << int_var__mem_sz[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_eth_occ_thr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array thr skipped" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".thr[" << ii <<"]: 0x" << int_var__thr[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_tx_occ_thr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array thr skipped" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".thr[" << ii <<"]: 0x" << int_var__thr[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_eth_xoff_thr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array xoff skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".xoff[" << ii <<"]: 0x" << int_var__xoff[ii] << dec << endl);
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array xon skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".xon[" << ii <<"]: 0x" << int_var__xon[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_hbm_eth_hdr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_q_depth: 0x" << int_var__hbm_q_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_timestamp: 0x" << int_var__hbm_timestamp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_qnum: 0x" << int_var__hbm_qnum << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_q_depth_vld: 0x" << int_var__hbm_q_depth_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_spare: 0x" << int_var__hbm_spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_egr_tstamp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 9 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array base skipped" << endl);
    #else
    for(int ii = 0; ii < 9; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".base[" << ii <<"]: 0x" << int_var__base[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_eth_oq_xoff_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array xoff_pri skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".xoff_pri[" << ii <<"]: 0x" << int_var__xoff_pri[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pbc_p4_oq_xoff_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_MSG("large_array xoff_pri skipped" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        PLOG_MSG(hex << string(get_hier_path()) + ".xoff_pri[" << ii <<"]: 0x" << int_var__xoff_pri[ii] << dec << endl);
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

int cap_pbc_replicate_node32_t::get_width() const {
    return cap_pbc_replicate_node32_t::s_get_width();

}

int cap_pbc_replicate_node48_t::get_width() const {
    return cap_pbc_replicate_node48_t::s_get_width();

}

int cap_pbc_replicate_node64_t::get_width() const {
    return cap_pbc_replicate_node64_t::s_get_width();

}

int cap_pbc_oq_map_t::get_width() const {
    return cap_pbc_oq_map_t::s_get_width();

}

int cap_pbc_pg8_map_t::get_width() const {
    return cap_pbc_pg8_map_t::s_get_width();

}

int cap_pbc_pg16_map_t::get_width() const {
    return cap_pbc_pg16_map_t::s_get_width();

}

int cap_pbc_pg32_map_t::get_width() const {
    return cap_pbc_pg32_map_t::s_get_width();

}

int cap_pbc_max_growth_map_t::get_width() const {
    return cap_pbc_max_growth_map_t::s_get_width();

}

int cap_pbc_hbm_eth_ctl_t::get_width() const {
    return cap_pbc_hbm_eth_ctl_t::s_get_width();

}

int cap_pbc_hbm_tx_ctl_t::get_width() const {
    return cap_pbc_hbm_tx_ctl_t::s_get_width();

}

int cap_pbc_hbm_eth_port_ctl_t::get_width() const {
    return cap_pbc_hbm_eth_port_ctl_t::s_get_width();

}

int cap_pbc_hbm_eth_occ_thr_t::get_width() const {
    return cap_pbc_hbm_eth_occ_thr_t::s_get_width();

}

int cap_pbc_hbm_tx_occ_thr_t::get_width() const {
    return cap_pbc_hbm_tx_occ_thr_t::s_get_width();

}

int cap_pbc_hbm_eth_xoff_thr_t::get_width() const {
    return cap_pbc_hbm_eth_xoff_thr_t::s_get_width();

}

int cap_pbc_hbm_eth_hdr_t::get_width() const {
    return cap_pbc_hbm_eth_hdr_t::s_get_width();

}

int cap_pbc_egr_tstamp_t::get_width() const {
    return cap_pbc_egr_tstamp_t::s_get_width();

}

int cap_pbc_eth_oq_xoff_map_t::get_width() const {
    return cap_pbc_eth_oq_xoff_map_t::s_get_width();

}

int cap_pbc_p4_oq_xoff_map_t::get_width() const {
    return cap_pbc_p4_oq_xoff_map_t::s_get_width();

}

int cap_pbc_replicate_node32_t::s_get_width() {
    int _count = 0;

    _count += 32 * 15; // token
    _count += 1; // last_node
    _count += 4; // token_cnt
    _count += 27; // next_ptr
    return _count;
}

int cap_pbc_replicate_node48_t::s_get_width() {
    int _count = 0;

    _count += 48 * 10; // token
    _count += 1; // last_node
    _count += 4; // token_cnt
    _count += 27; // next_ptr
    return _count;
}

int cap_pbc_replicate_node64_t::s_get_width() {
    int _count = 0;

    _count += 64 * 7; // token
    _count += 32; // spare
    _count += 1; // last_node
    _count += 4; // token_cnt
    _count += 27; // next_ptr
    return _count;
}

int cap_pbc_oq_map_t::s_get_width() {
    int _count = 0;

    _count += 5 * 8; // oqueue
    return _count;
}

int cap_pbc_pg8_map_t::s_get_width() {
    int _count = 0;

    _count += 3 * 8; // pg
    return _count;
}

int cap_pbc_pg16_map_t::s_get_width() {
    int _count = 0;

    _count += 4 * 16; // pg
    return _count;
}

int cap_pbc_pg32_map_t::s_get_width() {
    int _count = 0;

    _count += 5 * 32; // pg
    return _count;
}

int cap_pbc_max_growth_map_t::s_get_width() {
    int _count = 0;

    _count += 5 * 32; // max_growth
    return _count;
}

int cap_pbc_hbm_eth_ctl_t::s_get_width() {
    int _count = 0;

    _count += 27 * 32; // base
    _count += 23 * 32; // mem_sz
    return _count;
}

int cap_pbc_hbm_tx_ctl_t::s_get_width() {
    int _count = 0;

    _count += 27 * 16; // base
    _count += 23 * 16; // mem_sz
    return _count;
}

int cap_pbc_hbm_eth_port_ctl_t::s_get_width() {
    int _count = 0;

    _count += 27 * 8; // base
    _count += 23 * 8; // mem_sz
    return _count;
}

int cap_pbc_hbm_eth_occ_thr_t::s_get_width() {
    int _count = 0;

    _count += 19 * 8; // thr
    return _count;
}

int cap_pbc_hbm_tx_occ_thr_t::s_get_width() {
    int _count = 0;

    _count += 19 * 16; // thr
    return _count;
}

int cap_pbc_hbm_eth_xoff_thr_t::s_get_width() {
    int _count = 0;

    _count += 20 * 32; // xoff
    _count += 20 * 32; // xon
    return _count;
}

int cap_pbc_hbm_eth_hdr_t::s_get_width() {
    int _count = 0;

    _count += 14; // hbm_q_depth
    _count += 48; // hbm_timestamp
    _count += 3; // hbm_qnum
    _count += 1; // hbm_q_depth_vld
    _count += 6; // hbm_spare
    return _count;
}

int cap_pbc_egr_tstamp_t::s_get_width() {
    int _count = 0;

    _count += 27 * 9; // base
    return _count;
}

int cap_pbc_eth_oq_xoff_map_t::s_get_width() {
    int _count = 0;

    _count += 3 * 32; // xoff_pri
    return _count;
}

int cap_pbc_p4_oq_xoff_map_t::s_get_width() {
    int _count = 0;

    _count += 5 * 32; // xoff_pri
    return _count;
}

void cap_pbc_replicate_node32_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 15 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 0; ii < 15; ii++) {
        int_var__token[ii] = _val.convert_to< token_cpp_int_t >();
        _val = _val >> 32;
    }
    #endif
    
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >()  ;
    _val = _val >> 27;
    
}

void cap_pbc_replicate_node48_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 0; ii < 10; ii++) {
        int_var__token[ii] = _val.convert_to< token_cpp_int_t >();
        _val = _val >> 48;
    }
    #endif
    
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >()  ;
    _val = _val >> 27;
    
}

void cap_pbc_replicate_node64_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 7 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 0; ii < 7; ii++) {
        int_var__token[ii] = _val.convert_to< token_cpp_int_t >();
        _val = _val >> 64;
    }
    #endif
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >()  ;
    _val = _val >> 27;
    
}

void cap_pbc_oq_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // oqueue
    for(int ii = 0; ii < 8; ii++) {
        int_var__oqueue[ii] = _val.convert_to< oqueue_cpp_int_t >();
        _val = _val >> 5;
    }
    #endif
    
}

void cap_pbc_pg8_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 0; ii < 8; ii++) {
        int_var__pg[ii] = _val.convert_to< pg_cpp_int_t >();
        _val = _val >> 3;
    }
    #endif
    
}

void cap_pbc_pg16_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 0; ii < 16; ii++) {
        int_var__pg[ii] = _val.convert_to< pg_cpp_int_t >();
        _val = _val >> 4;
    }
    #endif
    
}

void cap_pbc_pg32_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 0; ii < 32; ii++) {
        int_var__pg[ii] = _val.convert_to< pg_cpp_int_t >();
        _val = _val >> 5;
    }
    #endif
    
}

void cap_pbc_max_growth_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // max_growth
    for(int ii = 0; ii < 32; ii++) {
        int_var__max_growth[ii] = _val.convert_to< max_growth_cpp_int_t >();
        _val = _val >> 5;
    }
    #endif
    
}

void cap_pbc_hbm_eth_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 0; ii < 32; ii++) {
        int_var__base[ii] = _val.convert_to< base_cpp_int_t >();
        _val = _val >> 27;
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 0; ii < 32; ii++) {
        int_var__mem_sz[ii] = _val.convert_to< mem_sz_cpp_int_t >();
        _val = _val >> 23;
    }
    #endif
    
}

void cap_pbc_hbm_tx_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 0; ii < 16; ii++) {
        int_var__base[ii] = _val.convert_to< base_cpp_int_t >();
        _val = _val >> 27;
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 0; ii < 16; ii++) {
        int_var__mem_sz[ii] = _val.convert_to< mem_sz_cpp_int_t >();
        _val = _val >> 23;
    }
    #endif
    
}

void cap_pbc_hbm_eth_port_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 0; ii < 8; ii++) {
        int_var__base[ii] = _val.convert_to< base_cpp_int_t >();
        _val = _val >> 27;
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 0; ii < 8; ii++) {
        int_var__mem_sz[ii] = _val.convert_to< mem_sz_cpp_int_t >();
        _val = _val >> 23;
    }
    #endif
    
}

void cap_pbc_hbm_eth_occ_thr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // thr
    for(int ii = 0; ii < 8; ii++) {
        int_var__thr[ii] = _val.convert_to< thr_cpp_int_t >();
        _val = _val >> 19;
    }
    #endif
    
}

void cap_pbc_hbm_tx_occ_thr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // thr
    for(int ii = 0; ii < 16; ii++) {
        int_var__thr[ii] = _val.convert_to< thr_cpp_int_t >();
        _val = _val >> 19;
    }
    #endif
    
}

void cap_pbc_hbm_eth_xoff_thr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff[ii] = _val.convert_to< xoff_cpp_int_t >();
        _val = _val >> 20;
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xon
    for(int ii = 0; ii < 32; ii++) {
        int_var__xon[ii] = _val.convert_to< xon_cpp_int_t >();
        _val = _val >> 20;
    }
    #endif
    
}

void cap_pbc_hbm_eth_hdr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_q_depth = _val.convert_to< hbm_q_depth_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__hbm_timestamp = _val.convert_to< hbm_timestamp_cpp_int_t >()  ;
    _val = _val >> 48;
    
    int_var__hbm_qnum = _val.convert_to< hbm_qnum_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__hbm_q_depth_vld = _val.convert_to< hbm_q_depth_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_spare = _val.convert_to< hbm_spare_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_pbc_egr_tstamp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 9 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 0; ii < 9; ii++) {
        int_var__base[ii] = _val.convert_to< base_cpp_int_t >();
        _val = _val >> 27;
    }
    #endif
    
}

void cap_pbc_eth_oq_xoff_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff_pri
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff_pri[ii] = _val.convert_to< xoff_pri_cpp_int_t >();
        _val = _val >> 3;
    }
    #endif
    
}

void cap_pbc_p4_oq_xoff_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff_pri
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff_pri[ii] = _val.convert_to< xoff_pri_cpp_int_t >();
        _val = _val >> 5;
    }
    #endif
    
}

cpp_int cap_pbc_p4_oq_xoff_map_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff_pri
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 5; ret_val = ret_val  | int_var__xoff_pri[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_eth_oq_xoff_map_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff_pri
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 3; ret_val = ret_val  | int_var__xoff_pri[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_egr_tstamp_t::all() const {
    cpp_int ret_val;

    #if 9 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 9-1; ii >= 0; ii--) {
        ret_val = ret_val << 27; ret_val = ret_val  | int_var__base[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_eth_hdr_t::all() const {
    cpp_int ret_val;

    // hbm_spare
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__hbm_spare; 
    
    // hbm_q_depth_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_q_depth_vld; 
    
    // hbm_qnum
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__hbm_qnum; 
    
    // hbm_timestamp
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__hbm_timestamp; 
    
    // hbm_q_depth
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__hbm_q_depth; 
    
    return ret_val;
}

cpp_int cap_pbc_hbm_eth_xoff_thr_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xon
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 20; ret_val = ret_val  | int_var__xon[ii]; 
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // xoff
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 20; ret_val = ret_val  | int_var__xoff[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_tx_occ_thr_t::all() const {
    cpp_int ret_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // thr
    for(int ii = 16-1; ii >= 0; ii--) {
        ret_val = ret_val << 19; ret_val = ret_val  | int_var__thr[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_eth_occ_thr_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // thr
    for(int ii = 8-1; ii >= 0; ii--) {
        ret_val = ret_val << 19; ret_val = ret_val  | int_var__thr[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_eth_port_ctl_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 8-1; ii >= 0; ii--) {
        ret_val = ret_val << 23; ret_val = ret_val  | int_var__mem_sz[ii]; 
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 8-1; ii >= 0; ii--) {
        ret_val = ret_val << 27; ret_val = ret_val  | int_var__base[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_tx_ctl_t::all() const {
    cpp_int ret_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 16-1; ii >= 0; ii--) {
        ret_val = ret_val << 23; ret_val = ret_val  | int_var__mem_sz[ii]; 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 16-1; ii >= 0; ii--) {
        ret_val = ret_val << 27; ret_val = ret_val  | int_var__base[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_hbm_eth_ctl_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mem_sz
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 23; ret_val = ret_val  | int_var__mem_sz[ii]; 
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // base
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 27; ret_val = ret_val  | int_var__base[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_max_growth_map_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // max_growth
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 5; ret_val = ret_val  | int_var__max_growth[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_pg32_map_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 32-1; ii >= 0; ii--) {
        ret_val = ret_val << 5; ret_val = ret_val  | int_var__pg[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_pg16_map_t::all() const {
    cpp_int ret_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 16-1; ii >= 0; ii--) {
        ret_val = ret_val << 4; ret_val = ret_val  | int_var__pg[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_pg8_map_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // pg
    for(int ii = 8-1; ii >= 0; ii--) {
        ret_val = ret_val << 3; ret_val = ret_val  | int_var__pg[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_oq_map_t::all() const {
    cpp_int ret_val;

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // oqueue
    for(int ii = 8-1; ii >= 0; ii--) {
        ret_val = ret_val << 5; ret_val = ret_val  | int_var__oqueue[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_replicate_node64_t::all() const {
    cpp_int ret_val;

    // next_ptr
    ret_val = ret_val << 27; ret_val = ret_val  | int_var__next_ptr; 
    
    // token_cnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__token_cnt; 
    
    // last_node
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last_node; 
    
    // spare
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__spare; 
    
    #if 7 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 7-1; ii >= 0; ii--) {
        ret_val = ret_val << 64; ret_val = ret_val  | int_var__token[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_replicate_node48_t::all() const {
    cpp_int ret_val;

    // next_ptr
    ret_val = ret_val << 27; ret_val = ret_val  | int_var__next_ptr; 
    
    // token_cnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__token_cnt; 
    
    // last_node
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last_node; 
    
    #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 10-1; ii >= 0; ii--) {
        ret_val = ret_val << 48; ret_val = ret_val  | int_var__token[ii]; 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_pbc_replicate_node32_t::all() const {
    cpp_int ret_val;

    // next_ptr
    ret_val = ret_val << 27; ret_val = ret_val  | int_var__next_ptr; 
    
    // token_cnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__token_cnt; 
    
    // last_node
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last_node; 
    
    #if 15 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // token
    for(int ii = 15-1; ii >= 0; ii--) {
        ret_val = ret_val << 32; ret_val = ret_val  | int_var__token[ii]; 
    }
    #endif
    
    return ret_val;
}

void cap_pbc_replicate_node32_t::clear() {

    #if 15 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 15; ii++) {
        int_var__token[ii] = 0; 
    }
    #endif
    
    int_var__last_node = 0; 
    
    int_var__token_cnt = 0; 
    
    int_var__next_ptr = 0; 
    
}

void cap_pbc_replicate_node48_t::clear() {

    #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 10; ii++) {
        int_var__token[ii] = 0; 
    }
    #endif
    
    int_var__last_node = 0; 
    
    int_var__token_cnt = 0; 
    
    int_var__next_ptr = 0; 
    
}

void cap_pbc_replicate_node64_t::clear() {

    #if 7 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 7; ii++) {
        int_var__token[ii] = 0; 
    }
    #endif
    
    int_var__spare = 0; 
    
    int_var__last_node = 0; 
    
    int_var__token_cnt = 0; 
    
    int_var__next_ptr = 0; 
    
}

void cap_pbc_oq_map_t::clear() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        int_var__oqueue[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_pg8_map_t::clear() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        int_var__pg[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_pg16_map_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        int_var__pg[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_pg32_map_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__pg[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_max_growth_map_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__max_growth[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_eth_ctl_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__base[ii] = 0; 
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__mem_sz[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_tx_ctl_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        int_var__base[ii] = 0; 
    }
    #endif
    
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        int_var__mem_sz[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_eth_port_ctl_t::clear() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        int_var__base[ii] = 0; 
    }
    #endif
    
    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        int_var__mem_sz[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_eth_occ_thr_t::clear() {

    #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 8; ii++) {
        int_var__thr[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_tx_occ_thr_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 16; ii++) {
        int_var__thr[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_eth_xoff_thr_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff[ii] = 0; 
    }
    #endif
    
    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__xon[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_hbm_eth_hdr_t::clear() {

    int_var__hbm_q_depth = 0; 
    
    int_var__hbm_timestamp = 0; 
    
    int_var__hbm_qnum = 0; 
    
    int_var__hbm_q_depth_vld = 0; 
    
    int_var__hbm_spare = 0; 
    
}

void cap_pbc_egr_tstamp_t::clear() {

    #if 9 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 9; ii++) {
        int_var__base[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_eth_oq_xoff_map_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff_pri[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_p4_oq_xoff_map_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    for(int ii = 0; ii < 32; ii++) {
        int_var__xoff_pri[ii] = 0; 
    }
    #endif
    
}

void cap_pbc_replicate_node32_t::init() {

    
}

void cap_pbc_replicate_node48_t::init() {

    
}

void cap_pbc_replicate_node64_t::init() {

    
}

void cap_pbc_oq_map_t::init() {

    
}

void cap_pbc_pg8_map_t::init() {

    
}

void cap_pbc_pg16_map_t::init() {

    
}

void cap_pbc_pg32_map_t::init() {

    
}

void cap_pbc_max_growth_map_t::init() {

    
}

void cap_pbc_hbm_eth_ctl_t::init() {

    
    
}

void cap_pbc_hbm_tx_ctl_t::init() {

    
    
}

void cap_pbc_hbm_eth_port_ctl_t::init() {

    
    
}

void cap_pbc_hbm_eth_occ_thr_t::init() {

    
}

void cap_pbc_hbm_tx_occ_thr_t::init() {

    
}

void cap_pbc_hbm_eth_xoff_thr_t::init() {

    
    
}

void cap_pbc_hbm_eth_hdr_t::init() {

}

void cap_pbc_egr_tstamp_t::init() {

    
}

void cap_pbc_eth_oq_xoff_map_t::init() {

    
}

void cap_pbc_p4_oq_xoff_map_t::init() {

    
}

#if 15 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// token
void cap_pbc_replicate_node32_t::token(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function token Not yet implemented"<< endl);
}

cpp_int cap_pbc_replicate_node32_t::token(int _idx) const {
    PLOG_ERR("get function token Not yet implemented"<< endl);
    return 0;
}
#else 
// token
void cap_pbc_replicate_node32_t::token(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 15);
    int_var__token[_idx] = _val.convert_to< token_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node32_t::token(int _idx) const {
    PU_ASSERT(_idx < 15);
    return int_var__token[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_pbc_replicate_node32_t::last_node(const cpp_int & _val) { 
    // last_node
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node32_t::last_node() const {
    return int_var__last_node;
}
    
void cap_pbc_replicate_node32_t::token_cnt(const cpp_int & _val) { 
    // token_cnt
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node32_t::token_cnt() const {
    return int_var__token_cnt;
}
    
void cap_pbc_replicate_node32_t::next_ptr(const cpp_int & _val) { 
    // next_ptr
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node32_t::next_ptr() const {
    return int_var__next_ptr;
}
    
#if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// token
void cap_pbc_replicate_node48_t::token(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function token Not yet implemented"<< endl);
}

cpp_int cap_pbc_replicate_node48_t::token(int _idx) const {
    PLOG_ERR("get function token Not yet implemented"<< endl);
    return 0;
}
#else 
// token
void cap_pbc_replicate_node48_t::token(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 10);
    int_var__token[_idx] = _val.convert_to< token_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node48_t::token(int _idx) const {
    PU_ASSERT(_idx < 10);
    return int_var__token[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_pbc_replicate_node48_t::last_node(const cpp_int & _val) { 
    // last_node
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node48_t::last_node() const {
    return int_var__last_node;
}
    
void cap_pbc_replicate_node48_t::token_cnt(const cpp_int & _val) { 
    // token_cnt
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node48_t::token_cnt() const {
    return int_var__token_cnt;
}
    
void cap_pbc_replicate_node48_t::next_ptr(const cpp_int & _val) { 
    // next_ptr
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node48_t::next_ptr() const {
    return int_var__next_ptr;
}
    
#if 7 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// token
void cap_pbc_replicate_node64_t::token(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function token Not yet implemented"<< endl);
}

cpp_int cap_pbc_replicate_node64_t::token(int _idx) const {
    PLOG_ERR("get function token Not yet implemented"<< endl);
    return 0;
}
#else 
// token
void cap_pbc_replicate_node64_t::token(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 7);
    int_var__token[_idx] = _val.convert_to< token_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node64_t::token(int _idx) const {
    PU_ASSERT(_idx < 7);
    return int_var__token[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_pbc_replicate_node64_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node64_t::spare() const {
    return int_var__spare;
}
    
void cap_pbc_replicate_node64_t::last_node(const cpp_int & _val) { 
    // last_node
    int_var__last_node = _val.convert_to< last_node_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node64_t::last_node() const {
    return int_var__last_node;
}
    
void cap_pbc_replicate_node64_t::token_cnt(const cpp_int & _val) { 
    // token_cnt
    int_var__token_cnt = _val.convert_to< token_cnt_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node64_t::token_cnt() const {
    return int_var__token_cnt;
}
    
void cap_pbc_replicate_node64_t::next_ptr(const cpp_int & _val) { 
    // next_ptr
    int_var__next_ptr = _val.convert_to< next_ptr_cpp_int_t >();
}

cpp_int cap_pbc_replicate_node64_t::next_ptr() const {
    return int_var__next_ptr;
}
    
#if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// oqueue
void cap_pbc_oq_map_t::oqueue(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function oqueue Not yet implemented"<< endl);
}

cpp_int cap_pbc_oq_map_t::oqueue(int _idx) const {
    PLOG_ERR("get function oqueue Not yet implemented"<< endl);
    return 0;
}
#else 
// oqueue
void cap_pbc_oq_map_t::oqueue(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 8);
    int_var__oqueue[_idx] = _val.convert_to< oqueue_cpp_int_t >();
}

cpp_int cap_pbc_oq_map_t::oqueue(int _idx) const {
    PU_ASSERT(_idx < 8);
    return int_var__oqueue[_idx].convert_to<cpp_int>();
}
#endif
    
#if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// pg
void cap_pbc_pg8_map_t::pg(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function pg Not yet implemented"<< endl);
}

cpp_int cap_pbc_pg8_map_t::pg(int _idx) const {
    PLOG_ERR("get function pg Not yet implemented"<< endl);
    return 0;
}
#else 
// pg
void cap_pbc_pg8_map_t::pg(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 8);
    int_var__pg[_idx] = _val.convert_to< pg_cpp_int_t >();
}

cpp_int cap_pbc_pg8_map_t::pg(int _idx) const {
    PU_ASSERT(_idx < 8);
    return int_var__pg[_idx].convert_to<cpp_int>();
}
#endif
    
#if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// pg
void cap_pbc_pg16_map_t::pg(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function pg Not yet implemented"<< endl);
}

cpp_int cap_pbc_pg16_map_t::pg(int _idx) const {
    PLOG_ERR("get function pg Not yet implemented"<< endl);
    return 0;
}
#else 
// pg
void cap_pbc_pg16_map_t::pg(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 16);
    int_var__pg[_idx] = _val.convert_to< pg_cpp_int_t >();
}

cpp_int cap_pbc_pg16_map_t::pg(int _idx) const {
    PU_ASSERT(_idx < 16);
    return int_var__pg[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// pg
void cap_pbc_pg32_map_t::pg(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function pg Not yet implemented"<< endl);
}

cpp_int cap_pbc_pg32_map_t::pg(int _idx) const {
    PLOG_ERR("get function pg Not yet implemented"<< endl);
    return 0;
}
#else 
// pg
void cap_pbc_pg32_map_t::pg(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__pg[_idx] = _val.convert_to< pg_cpp_int_t >();
}

cpp_int cap_pbc_pg32_map_t::pg(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__pg[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// max_growth
void cap_pbc_max_growth_map_t::max_growth(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function max_growth Not yet implemented"<< endl);
}

cpp_int cap_pbc_max_growth_map_t::max_growth(int _idx) const {
    PLOG_ERR("get function max_growth Not yet implemented"<< endl);
    return 0;
}
#else 
// max_growth
void cap_pbc_max_growth_map_t::max_growth(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__max_growth[_idx] = _val.convert_to< max_growth_cpp_int_t >();
}

cpp_int cap_pbc_max_growth_map_t::max_growth(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__max_growth[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// base
void cap_pbc_hbm_eth_ctl_t::base(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function base Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_ctl_t::base(int _idx) const {
    PLOG_ERR("get function base Not yet implemented"<< endl);
    return 0;
}
#else 
// base
void cap_pbc_hbm_eth_ctl_t::base(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__base[_idx] = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_ctl_t::base(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__base[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mem_sz
void cap_pbc_hbm_eth_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mem_sz Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_ctl_t::mem_sz(int _idx) const {
    PLOG_ERR("get function mem_sz Not yet implemented"<< endl);
    return 0;
}
#else 
// mem_sz
void cap_pbc_hbm_eth_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__mem_sz[_idx] = _val.convert_to< mem_sz_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_ctl_t::mem_sz(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__mem_sz[_idx].convert_to<cpp_int>();
}
#endif
    
#if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// base
void cap_pbc_hbm_tx_ctl_t::base(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function base Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_tx_ctl_t::base(int _idx) const {
    PLOG_ERR("get function base Not yet implemented"<< endl);
    return 0;
}
#else 
// base
void cap_pbc_hbm_tx_ctl_t::base(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 16);
    int_var__base[_idx] = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbc_hbm_tx_ctl_t::base(int _idx) const {
    PU_ASSERT(_idx < 16);
    return int_var__base[_idx].convert_to<cpp_int>();
}
#endif
    
#if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mem_sz
void cap_pbc_hbm_tx_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mem_sz Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_tx_ctl_t::mem_sz(int _idx) const {
    PLOG_ERR("get function mem_sz Not yet implemented"<< endl);
    return 0;
}
#else 
// mem_sz
void cap_pbc_hbm_tx_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 16);
    int_var__mem_sz[_idx] = _val.convert_to< mem_sz_cpp_int_t >();
}

cpp_int cap_pbc_hbm_tx_ctl_t::mem_sz(int _idx) const {
    PU_ASSERT(_idx < 16);
    return int_var__mem_sz[_idx].convert_to<cpp_int>();
}
#endif
    
#if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// base
void cap_pbc_hbm_eth_port_ctl_t::base(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function base Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_port_ctl_t::base(int _idx) const {
    PLOG_ERR("get function base Not yet implemented"<< endl);
    return 0;
}
#else 
// base
void cap_pbc_hbm_eth_port_ctl_t::base(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 8);
    int_var__base[_idx] = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_port_ctl_t::base(int _idx) const {
    PU_ASSERT(_idx < 8);
    return int_var__base[_idx].convert_to<cpp_int>();
}
#endif
    
#if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// mem_sz
void cap_pbc_hbm_eth_port_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function mem_sz Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_port_ctl_t::mem_sz(int _idx) const {
    PLOG_ERR("get function mem_sz Not yet implemented"<< endl);
    return 0;
}
#else 
// mem_sz
void cap_pbc_hbm_eth_port_ctl_t::mem_sz(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 8);
    int_var__mem_sz[_idx] = _val.convert_to< mem_sz_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_port_ctl_t::mem_sz(int _idx) const {
    PU_ASSERT(_idx < 8);
    return int_var__mem_sz[_idx].convert_to<cpp_int>();
}
#endif
    
#if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// thr
void cap_pbc_hbm_eth_occ_thr_t::thr(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function thr Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_occ_thr_t::thr(int _idx) const {
    PLOG_ERR("get function thr Not yet implemented"<< endl);
    return 0;
}
#else 
// thr
void cap_pbc_hbm_eth_occ_thr_t::thr(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 8);
    int_var__thr[_idx] = _val.convert_to< thr_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_occ_thr_t::thr(int _idx) const {
    PU_ASSERT(_idx < 8);
    return int_var__thr[_idx].convert_to<cpp_int>();
}
#endif
    
#if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// thr
void cap_pbc_hbm_tx_occ_thr_t::thr(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function thr Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_tx_occ_thr_t::thr(int _idx) const {
    PLOG_ERR("get function thr Not yet implemented"<< endl);
    return 0;
}
#else 
// thr
void cap_pbc_hbm_tx_occ_thr_t::thr(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 16);
    int_var__thr[_idx] = _val.convert_to< thr_cpp_int_t >();
}

cpp_int cap_pbc_hbm_tx_occ_thr_t::thr(int _idx) const {
    PU_ASSERT(_idx < 16);
    return int_var__thr[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// xoff
void cap_pbc_hbm_eth_xoff_thr_t::xoff(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function xoff Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_xoff_thr_t::xoff(int _idx) const {
    PLOG_ERR("get function xoff Not yet implemented"<< endl);
    return 0;
}
#else 
// xoff
void cap_pbc_hbm_eth_xoff_thr_t::xoff(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__xoff[_idx] = _val.convert_to< xoff_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_xoff_thr_t::xoff(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__xoff[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// xon
void cap_pbc_hbm_eth_xoff_thr_t::xon(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function xon Not yet implemented"<< endl);
}

cpp_int cap_pbc_hbm_eth_xoff_thr_t::xon(int _idx) const {
    PLOG_ERR("get function xon Not yet implemented"<< endl);
    return 0;
}
#else 
// xon
void cap_pbc_hbm_eth_xoff_thr_t::xon(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__xon[_idx] = _val.convert_to< xon_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_xoff_thr_t::xon(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__xon[_idx].convert_to<cpp_int>();
}
#endif
    
void cap_pbc_hbm_eth_hdr_t::hbm_q_depth(const cpp_int & _val) { 
    // hbm_q_depth
    int_var__hbm_q_depth = _val.convert_to< hbm_q_depth_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_hdr_t::hbm_q_depth() const {
    return int_var__hbm_q_depth;
}
    
void cap_pbc_hbm_eth_hdr_t::hbm_timestamp(const cpp_int & _val) { 
    // hbm_timestamp
    int_var__hbm_timestamp = _val.convert_to< hbm_timestamp_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_hdr_t::hbm_timestamp() const {
    return int_var__hbm_timestamp;
}
    
void cap_pbc_hbm_eth_hdr_t::hbm_qnum(const cpp_int & _val) { 
    // hbm_qnum
    int_var__hbm_qnum = _val.convert_to< hbm_qnum_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_hdr_t::hbm_qnum() const {
    return int_var__hbm_qnum;
}
    
void cap_pbc_hbm_eth_hdr_t::hbm_q_depth_vld(const cpp_int & _val) { 
    // hbm_q_depth_vld
    int_var__hbm_q_depth_vld = _val.convert_to< hbm_q_depth_vld_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_hdr_t::hbm_q_depth_vld() const {
    return int_var__hbm_q_depth_vld;
}
    
void cap_pbc_hbm_eth_hdr_t::hbm_spare(const cpp_int & _val) { 
    // hbm_spare
    int_var__hbm_spare = _val.convert_to< hbm_spare_cpp_int_t >();
}

cpp_int cap_pbc_hbm_eth_hdr_t::hbm_spare() const {
    return int_var__hbm_spare;
}
    
#if 9 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// base
void cap_pbc_egr_tstamp_t::base(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function base Not yet implemented"<< endl);
}

cpp_int cap_pbc_egr_tstamp_t::base(int _idx) const {
    PLOG_ERR("get function base Not yet implemented"<< endl);
    return 0;
}
#else 
// base
void cap_pbc_egr_tstamp_t::base(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 9);
    int_var__base[_idx] = _val.convert_to< base_cpp_int_t >();
}

cpp_int cap_pbc_egr_tstamp_t::base(int _idx) const {
    PU_ASSERT(_idx < 9);
    return int_var__base[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// xoff_pri
void cap_pbc_eth_oq_xoff_map_t::xoff_pri(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function xoff_pri Not yet implemented"<< endl);
}

cpp_int cap_pbc_eth_oq_xoff_map_t::xoff_pri(int _idx) const {
    PLOG_ERR("get function xoff_pri Not yet implemented"<< endl);
    return 0;
}
#else 
// xoff_pri
void cap_pbc_eth_oq_xoff_map_t::xoff_pri(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__xoff_pri[_idx] = _val.convert_to< xoff_pri_cpp_int_t >();
}

cpp_int cap_pbc_eth_oq_xoff_map_t::xoff_pri(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__xoff_pri[_idx].convert_to<cpp_int>();
}
#endif
    
#if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
// xoff_pri
void cap_pbc_p4_oq_xoff_map_t::xoff_pri(const cpp_int & _val, int _idx) { 
    PLOG_ERR("set function xoff_pri Not yet implemented"<< endl);
}

cpp_int cap_pbc_p4_oq_xoff_map_t::xoff_pri(int _idx) const {
    PLOG_ERR("get function xoff_pri Not yet implemented"<< endl);
    return 0;
}
#else 
// xoff_pri
void cap_pbc_p4_oq_xoff_map_t::xoff_pri(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < 32);
    int_var__xoff_pri[_idx] = _val.convert_to< xoff_pri_cpp_int_t >();
}

cpp_int cap_pbc_p4_oq_xoff_map_t::xoff_pri(int _idx) const {
    PU_ASSERT(_idx < 32);
    return int_var__xoff_pri[_idx].convert_to<cpp_int>();
}
#endif
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node32_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 15); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__token[ii];
        }
    }
    
    if(!field_found && !strcmp(field_name, "last_node")) { field_val = last_node(); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { field_val = token_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { field_val = next_ptr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node48_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 10); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__token[ii];
        }
    }
    
    if(!field_found && !strcmp(field_name, "last_node")) { field_val = last_node(); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { field_val = token_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { field_val = next_ptr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node64_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 7); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__token[ii];
        }
    }
    
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last_node")) { field_val = last_node(); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { field_val = token_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { field_val = next_ptr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_oq_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("oqueue["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__oqueue[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg8_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__pg[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg16_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__pg[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg32_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__pg[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_max_growth_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("max_growth["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__max_growth[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__base[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mem_sz[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_tx_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__base[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mem_sz[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_port_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__base[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__mem_sz[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_occ_thr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("thr["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__thr[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_tx_occ_thr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("thr["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__thr[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_xoff_thr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__xoff[ii];
        }
    }
    
    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xon["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__xon[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_hdr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_q_depth")) { field_val = hbm_q_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_timestamp")) { field_val = hbm_timestamp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_qnum")) { field_val = hbm_qnum(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_q_depth_vld")) { field_val = hbm_q_depth_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_spare")) { field_val = hbm_spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_egr_tstamp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 9); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__base[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_eth_oq_xoff_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff_pri["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__xoff_pri[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_p4_oq_xoff_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff_pri["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            field_val = int_var__xoff_pri[ii];
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node32_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 15); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__token[ii] = field_val;
        }
    }
    
    if(!field_found && !strcmp(field_name, "last_node")) { last_node(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { token_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { next_ptr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node48_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 10); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__token[ii] = field_val;
        }
    }
    
    if(!field_found && !strcmp(field_name, "last_node")) { last_node(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { token_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { next_ptr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_replicate_node64_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 7); ii++) {
        if(!strcmp(field_name, string("token["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__token[ii] = field_val;
        }
    }
    
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last_node")) { last_node(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "token_cnt")) { token_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "next_ptr")) { next_ptr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_oq_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("oqueue["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__oqueue[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg8_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__pg[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg16_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__pg[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_pg32_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("pg["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__pg[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_max_growth_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("max_growth["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__max_growth[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__base[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mem_sz[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_tx_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__base[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mem_sz[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_port_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__base[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("mem_sz["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__mem_sz[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_occ_thr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 8); ii++) {
        if(!strcmp(field_name, string("thr["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__thr[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_tx_occ_thr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 16); ii++) {
        if(!strcmp(field_name, string("thr["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__thr[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_xoff_thr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__xoff[ii] = field_val;
        }
    }
    
    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xon["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__xon[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_hbm_eth_hdr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_q_depth")) { hbm_q_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_timestamp")) { hbm_timestamp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_qnum")) { hbm_qnum(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_q_depth_vld")) { hbm_q_depth_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_spare")) { hbm_spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_egr_tstamp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 9); ii++) {
        if(!strcmp(field_name, string("base["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__base[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_eth_oq_xoff_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff_pri["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__xoff_pri[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pbc_p4_oq_xoff_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    for(int ii = 0; !field_found && (ii < 32); ii++) {
        if(!strcmp(field_name, string("xoff_pri["+ to_string(ii) +"]").c_str())) { 
            field_found = 1;
            int_var__xoff_pri[ii] = field_val;
        }
    }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_replicate_node32_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 15; ii++) {
        ret_vec.push_back("token["+to_string(ii)+"]");
    }
    
    ret_vec.push_back("last_node");
    ret_vec.push_back("token_cnt");
    ret_vec.push_back("next_ptr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_replicate_node48_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 10; ii++) {
        ret_vec.push_back("token["+to_string(ii)+"]");
    }
    
    ret_vec.push_back("last_node");
    ret_vec.push_back("token_cnt");
    ret_vec.push_back("next_ptr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_replicate_node64_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 7; ii++) {
        ret_vec.push_back("token["+to_string(ii)+"]");
    }
    
    ret_vec.push_back("spare");
    ret_vec.push_back("last_node");
    ret_vec.push_back("token_cnt");
    ret_vec.push_back("next_ptr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_oq_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 8; ii++) {
        ret_vec.push_back("oqueue["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_pg8_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 8; ii++) {
        ret_vec.push_back("pg["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_pg16_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 16; ii++) {
        ret_vec.push_back("pg["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_pg32_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("pg["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_max_growth_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("max_growth["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_eth_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("base["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("mem_sz["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_tx_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 16; ii++) {
        ret_vec.push_back("base["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 16; ii++) {
        ret_vec.push_back("mem_sz["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_eth_port_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 8; ii++) {
        ret_vec.push_back("base["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 8; ii++) {
        ret_vec.push_back("mem_sz["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_eth_occ_thr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 8; ii++) {
        ret_vec.push_back("thr["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_tx_occ_thr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 16; ii++) {
        ret_vec.push_back("thr["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_eth_xoff_thr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("xoff["+to_string(ii)+"]");
    }
    
    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("xon["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_hbm_eth_hdr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_q_depth");
    ret_vec.push_back("hbm_timestamp");
    ret_vec.push_back("hbm_qnum");
    ret_vec.push_back("hbm_q_depth_vld");
    ret_vec.push_back("hbm_spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_egr_tstamp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 9; ii++) {
        ret_vec.push_back("base["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_eth_oq_xoff_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("xoff_pri["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pbc_p4_oq_xoff_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    for(int ii = 0; ii < 32; ii++) {
        ret_vec.push_back("xoff_pri["+to_string(ii)+"]");
    }
    
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
