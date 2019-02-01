
#include "cap_ptd_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_ptd_decoders_cmd_mem2pkt_t::cap_ptd_decoders_cmd_mem2pkt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_mem2pkt_t::~cap_ptd_decoders_cmd_mem2pkt_t() { }

cap_ptd_decoders_cmd_phv2pkt_t::cap_ptd_decoders_cmd_phv2pkt_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_phv2pkt_t::~cap_ptd_decoders_cmd_phv2pkt_t() { }

cap_ptd_decoders_cmd_phv2mem_t::cap_ptd_decoders_cmd_phv2mem_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_phv2mem_t::~cap_ptd_decoders_cmd_phv2mem_t() { }

cap_ptd_decoders_cmd_pkt2mem_t::cap_ptd_decoders_cmd_pkt2mem_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_pkt2mem_t::~cap_ptd_decoders_cmd_pkt2mem_t() { }

cap_ptd_decoders_cmd_skip_t::cap_ptd_decoders_cmd_skip_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_skip_t::~cap_ptd_decoders_cmd_skip_t() { }

cap_ptd_decoders_cmd_mem2mem_t::cap_ptd_decoders_cmd_mem2mem_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_ptd_decoders_cmd_mem2mem_t::~cap_ptd_decoders_cmd_mem2mem_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_mem2pkt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkteop: 0x" << int_var__pkteop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".host_addr: 0x" << int_var__host_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".use_override_lif: 0x" << int_var__use_override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".override_lif: 0x" << int_var__override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd1: 0x" << int_var__rsvd1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".psize: 0x" << int_var__psize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_phv2pkt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkteop: 0x" << int_var__pkteop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdsize: 0x" << int_var__cmdsize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start: 0x" << int_var__phv_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end: 0x" << int_var__phv_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start1: 0x" << int_var__phv_start1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end1: 0x" << int_var__phv_end1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start2: 0x" << int_var__phv_start2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end2: 0x" << int_var__phv_end2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start3: 0x" << int_var__phv_start3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end3: 0x" << int_var__phv_end3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_phv2mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".host_addr: 0x" << int_var__host_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr_data_fence: 0x" << int_var__wr_data_fence << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr_fence_fence: 0x" << int_var__wr_fence_fence << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start: 0x" << int_var__phv_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end: 0x" << int_var__phv_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".use_override_lif: 0x" << int_var__use_override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_msg: 0x" << int_var__pcie_msg << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".round: 0x" << int_var__round << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".barrier: 0x" << int_var__barrier << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".override_lif: 0x" << int_var__override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_pkt2mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".round: 0x" << int_var__round << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".host_addr: 0x" << int_var__host_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".use_override_lif: 0x" << int_var__use_override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".override_lif: 0x" << int_var__override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd1: 0x" << int_var__rsvd1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".psize: 0x" << int_var__psize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_skip_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".psize: 0x" << int_var__psize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".skip_to_eop: 0x" << int_var__skip_to_eop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ptd_decoders_cmd_mem2mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cmdtype: 0x" << int_var__cmdtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cmdeop: 0x" << int_var__cmdeop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mem2mem_type: 0x" << int_var__mem2mem_type << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".host_addr: 0x" << int_var__host_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr_data_fence: 0x" << int_var__wr_data_fence << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wr_fence_fence: 0x" << int_var__wr_fence_fence << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_start: 0x" << int_var__phv_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_end: 0x" << int_var__phv_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".use_override_lif: 0x" << int_var__use_override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pcie_msg: 0x" << int_var__pcie_msg << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".round: 0x" << int_var__round << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".barrier: 0x" << int_var__barrier << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".override_lif: 0x" << int_var__override_lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd1: 0x" << int_var__rsvd1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".psize: 0x" << int_var__psize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

int cap_ptd_decoders_cmd_mem2pkt_t::get_width() const {
    return cap_ptd_decoders_cmd_mem2pkt_t::s_get_width();

}

int cap_ptd_decoders_cmd_phv2pkt_t::get_width() const {
    return cap_ptd_decoders_cmd_phv2pkt_t::s_get_width();

}

int cap_ptd_decoders_cmd_phv2mem_t::get_width() const {
    return cap_ptd_decoders_cmd_phv2mem_t::s_get_width();

}

int cap_ptd_decoders_cmd_pkt2mem_t::get_width() const {
    return cap_ptd_decoders_cmd_pkt2mem_t::s_get_width();

}

int cap_ptd_decoders_cmd_skip_t::get_width() const {
    return cap_ptd_decoders_cmd_skip_t::s_get_width();

}

int cap_ptd_decoders_cmd_mem2mem_t::get_width() const {
    return cap_ptd_decoders_cmd_mem2mem_t::s_get_width();

}

int cap_ptd_decoders_cmd_mem2pkt_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 1; // pkteop
    _count += 1; // host_addr
    _count += 1; // cache
    _count += 1; // use_override_lif
    _count += 52; // addr
    _count += 11; // override_lif
    _count += 1; // rsvd1
    _count += 14; // psize
    _count += 42; // rsvd
    return _count;
}

int cap_ptd_decoders_cmd_phv2pkt_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 1; // pkteop
    _count += 2; // cmdsize
    _count += 10; // phv_start
    _count += 10; // phv_end
    _count += 10; // phv_start1
    _count += 10; // phv_end1
    _count += 10; // phv_start2
    _count += 10; // phv_end2
    _count += 10; // phv_start3
    _count += 10; // phv_end3
    _count += 41; // rsvd
    return _count;
}

int cap_ptd_decoders_cmd_phv2mem_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 1; // host_addr
    _count += 1; // cache
    _count += 1; // wr_data_fence
    _count += 1; // wr_fence_fence
    _count += 10; // phv_start
    _count += 10; // phv_end
    _count += 1; // use_override_lif
    _count += 1; // pcie_msg
    _count += 1; // round
    _count += 1; // barrier
    _count += 52; // addr
    _count += 11; // override_lif
    _count += 33; // rsvd
    return _count;
}

int cap_ptd_decoders_cmd_pkt2mem_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 1; // round
    _count += 1; // host_addr
    _count += 1; // cache
    _count += 1; // use_override_lif
    _count += 52; // addr
    _count += 11; // override_lif
    _count += 1; // rsvd1
    _count += 14; // psize
    _count += 42; // rsvd
    return _count;
}

int cap_ptd_decoders_cmd_skip_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 14; // psize
    _count += 1; // skip_to_eop
    _count += 109; // rsvd
    return _count;
}

int cap_ptd_decoders_cmd_mem2mem_t::s_get_width() {
    int _count = 0;

    _count += 3; // cmdtype
    _count += 1; // cmdeop
    _count += 2; // mem2mem_type
    _count += 1; // host_addr
    _count += 1; // cache
    _count += 1; // wr_data_fence
    _count += 1; // wr_fence_fence
    _count += 10; // phv_start
    _count += 10; // phv_end
    _count += 1; // use_override_lif
    _count += 1; // pcie_msg
    _count += 1; // round
    _count += 1; // barrier
    _count += 52; // addr
    _count += 11; // override_lif
    _count += 1; // rsvd1
    _count += 14; // psize
    _count += 16; // rsvd
    return _count;
}

void cap_ptd_decoders_cmd_mem2pkt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkteop = _val.convert_to< pkteop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 52;
    
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__psize = _val.convert_to< psize_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 42;
    
}

void cap_ptd_decoders_cmd_phv2pkt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkteop = _val.convert_to< pkteop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cmdsize = _val.convert_to< cmdsize_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_start1 = _val.convert_to< phv_start1_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end1 = _val.convert_to< phv_end1_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_start2 = _val.convert_to< phv_start2_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end2 = _val.convert_to< phv_end2_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_start3 = _val.convert_to< phv_start3_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end3 = _val.convert_to< phv_end3_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 41;
    
}

void cap_ptd_decoders_cmd_phv2mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr_data_fence = _val.convert_to< wr_data_fence_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr_fence_fence = _val.convert_to< wr_fence_fence_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pcie_msg = _val.convert_to< pcie_msg_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__round = _val.convert_to< round_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__barrier = _val.convert_to< barrier_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 52;
    
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 33;
    
}

void cap_ptd_decoders_cmd_pkt2mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__round = _val.convert_to< round_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 52;
    
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__psize = _val.convert_to< psize_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 42;
    
}

void cap_ptd_decoders_cmd_skip_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__psize = _val.convert_to< psize_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__skip_to_eop = _val.convert_to< skip_to_eop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 109;
    
}

void cap_ptd_decoders_cmd_mem2mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mem2mem_type = _val.convert_to< mem2mem_type_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr_data_fence = _val.convert_to< wr_data_fence_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wr_fence_fence = _val.convert_to< wr_fence_fence_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pcie_msg = _val.convert_to< pcie_msg_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__round = _val.convert_to< round_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__barrier = _val.convert_to< barrier_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 52;
    
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__psize = _val.convert_to< psize_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__rsvd; 
    
    // psize
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__psize; 
    
    // rsvd1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rsvd1; 
    
    // override_lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__override_lif; 
    
    // addr
    ret_val = ret_val << 52; ret_val = ret_val  | int_var__addr; 
    
    // barrier
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__barrier; 
    
    // round
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__round; 
    
    // pcie_msg
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pcie_msg; 
    
    // use_override_lif
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_override_lif; 
    
    // phv_end
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end; 
    
    // phv_start
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start; 
    
    // wr_fence_fence
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr_fence_fence; 
    
    // wr_data_fence
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr_data_fence; 
    
    // cache
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache; 
    
    // host_addr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__host_addr; 
    
    // mem2mem_type
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__mem2mem_type; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

cpp_int cap_ptd_decoders_cmd_skip_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 109; ret_val = ret_val  | int_var__rsvd; 
    
    // skip_to_eop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__skip_to_eop; 
    
    // psize
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__psize; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 42; ret_val = ret_val  | int_var__rsvd; 
    
    // psize
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__psize; 
    
    // rsvd1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rsvd1; 
    
    // override_lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__override_lif; 
    
    // addr
    ret_val = ret_val << 52; ret_val = ret_val  | int_var__addr; 
    
    // use_override_lif
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_override_lif; 
    
    // cache
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache; 
    
    // host_addr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__host_addr; 
    
    // round
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__round; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 33; ret_val = ret_val  | int_var__rsvd; 
    
    // override_lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__override_lif; 
    
    // addr
    ret_val = ret_val << 52; ret_val = ret_val  | int_var__addr; 
    
    // barrier
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__barrier; 
    
    // round
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__round; 
    
    // pcie_msg
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pcie_msg; 
    
    // use_override_lif
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_override_lif; 
    
    // phv_end
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end; 
    
    // phv_start
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start; 
    
    // wr_fence_fence
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr_fence_fence; 
    
    // wr_data_fence
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wr_data_fence; 
    
    // cache
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache; 
    
    // host_addr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__host_addr; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 41; ret_val = ret_val  | int_var__rsvd; 
    
    // phv_end3
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end3; 
    
    // phv_start3
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start3; 
    
    // phv_end2
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end2; 
    
    // phv_start2
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start2; 
    
    // phv_end1
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end1; 
    
    // phv_start1
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start1; 
    
    // phv_end
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_end; 
    
    // phv_start
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__phv_start; 
    
    // cmdsize
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__cmdsize; 
    
    // pkteop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkteop; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 42; ret_val = ret_val  | int_var__rsvd; 
    
    // psize
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__psize; 
    
    // rsvd1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rsvd1; 
    
    // override_lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__override_lif; 
    
    // addr
    ret_val = ret_val << 52; ret_val = ret_val  | int_var__addr; 
    
    // use_override_lif
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__use_override_lif; 
    
    // cache
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache; 
    
    // host_addr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__host_addr; 
    
    // pkteop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkteop; 
    
    // cmdeop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cmdeop; 
    
    // cmdtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__cmdtype; 
    
    return ret_val;
}

void cap_ptd_decoders_cmd_mem2pkt_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__pkteop = 0; 
    
    int_var__host_addr = 0; 
    
    int_var__cache = 0; 
    
    int_var__use_override_lif = 0; 
    
    int_var__addr = 0; 
    
    int_var__override_lif = 0; 
    
    int_var__rsvd1 = 0; 
    
    int_var__psize = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_phv2pkt_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__pkteop = 0; 
    
    int_var__cmdsize = 0; 
    
    int_var__phv_start = 0; 
    
    int_var__phv_end = 0; 
    
    int_var__phv_start1 = 0; 
    
    int_var__phv_end1 = 0; 
    
    int_var__phv_start2 = 0; 
    
    int_var__phv_end2 = 0; 
    
    int_var__phv_start3 = 0; 
    
    int_var__phv_end3 = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_phv2mem_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__host_addr = 0; 
    
    int_var__cache = 0; 
    
    int_var__wr_data_fence = 0; 
    
    int_var__wr_fence_fence = 0; 
    
    int_var__phv_start = 0; 
    
    int_var__phv_end = 0; 
    
    int_var__use_override_lif = 0; 
    
    int_var__pcie_msg = 0; 
    
    int_var__round = 0; 
    
    int_var__barrier = 0; 
    
    int_var__addr = 0; 
    
    int_var__override_lif = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_pkt2mem_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__round = 0; 
    
    int_var__host_addr = 0; 
    
    int_var__cache = 0; 
    
    int_var__use_override_lif = 0; 
    
    int_var__addr = 0; 
    
    int_var__override_lif = 0; 
    
    int_var__rsvd1 = 0; 
    
    int_var__psize = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_skip_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__psize = 0; 
    
    int_var__skip_to_eop = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_mem2mem_t::clear() {

    int_var__cmdtype = 0; 
    
    int_var__cmdeop = 0; 
    
    int_var__mem2mem_type = 0; 
    
    int_var__host_addr = 0; 
    
    int_var__cache = 0; 
    
    int_var__wr_data_fence = 0; 
    
    int_var__wr_fence_fence = 0; 
    
    int_var__phv_start = 0; 
    
    int_var__phv_end = 0; 
    
    int_var__use_override_lif = 0; 
    
    int_var__pcie_msg = 0; 
    
    int_var__round = 0; 
    
    int_var__barrier = 0; 
    
    int_var__addr = 0; 
    
    int_var__override_lif = 0; 
    
    int_var__rsvd1 = 0; 
    
    int_var__psize = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_ptd_decoders_cmd_mem2pkt_t::init() {

}

void cap_ptd_decoders_cmd_phv2pkt_t::init() {

}

void cap_ptd_decoders_cmd_phv2mem_t::init() {

}

void cap_ptd_decoders_cmd_pkt2mem_t::init() {

}

void cap_ptd_decoders_cmd_skip_t::init() {

}

void cap_ptd_decoders_cmd_mem2mem_t::init() {

}

void cap_ptd_decoders_cmd_mem2pkt_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::pkteop(const cpp_int & _val) { 
    // pkteop
    int_var__pkteop = _val.convert_to< pkteop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::pkteop() const {
    return int_var__pkteop;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::host_addr(const cpp_int & _val) { 
    // host_addr
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::host_addr() const {
    return int_var__host_addr;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::cache() const {
    return int_var__cache;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::use_override_lif(const cpp_int & _val) { 
    // use_override_lif
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::use_override_lif() const {
    return int_var__use_override_lif;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::addr() const {
    return int_var__addr;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::override_lif(const cpp_int & _val) { 
    // override_lif
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::override_lif() const {
    return int_var__override_lif;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::rsvd1(const cpp_int & _val) { 
    // rsvd1
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::rsvd1() const {
    return int_var__rsvd1;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::psize(const cpp_int & _val) { 
    // psize
    int_var__psize = _val.convert_to< psize_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::psize() const {
    return int_var__psize;
}
    
void cap_ptd_decoders_cmd_mem2pkt_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2pkt_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::pkteop(const cpp_int & _val) { 
    // pkteop
    int_var__pkteop = _val.convert_to< pkteop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::pkteop() const {
    return int_var__pkteop;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::cmdsize(const cpp_int & _val) { 
    // cmdsize
    int_var__cmdsize = _val.convert_to< cmdsize_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::cmdsize() const {
    return int_var__cmdsize;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_start(const cpp_int & _val) { 
    // phv_start
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_start() const {
    return int_var__phv_start;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_end(const cpp_int & _val) { 
    // phv_end
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_end() const {
    return int_var__phv_end;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_start1(const cpp_int & _val) { 
    // phv_start1
    int_var__phv_start1 = _val.convert_to< phv_start1_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_start1() const {
    return int_var__phv_start1;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_end1(const cpp_int & _val) { 
    // phv_end1
    int_var__phv_end1 = _val.convert_to< phv_end1_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_end1() const {
    return int_var__phv_end1;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_start2(const cpp_int & _val) { 
    // phv_start2
    int_var__phv_start2 = _val.convert_to< phv_start2_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_start2() const {
    return int_var__phv_start2;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_end2(const cpp_int & _val) { 
    // phv_end2
    int_var__phv_end2 = _val.convert_to< phv_end2_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_end2() const {
    return int_var__phv_end2;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_start3(const cpp_int & _val) { 
    // phv_start3
    int_var__phv_start3 = _val.convert_to< phv_start3_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_start3() const {
    return int_var__phv_start3;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::phv_end3(const cpp_int & _val) { 
    // phv_end3
    int_var__phv_end3 = _val.convert_to< phv_end3_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::phv_end3() const {
    return int_var__phv_end3;
}
    
void cap_ptd_decoders_cmd_phv2pkt_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2pkt_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::host_addr(const cpp_int & _val) { 
    // host_addr
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::host_addr() const {
    return int_var__host_addr;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::cache() const {
    return int_var__cache;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::wr_data_fence(const cpp_int & _val) { 
    // wr_data_fence
    int_var__wr_data_fence = _val.convert_to< wr_data_fence_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::wr_data_fence() const {
    return int_var__wr_data_fence;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::wr_fence_fence(const cpp_int & _val) { 
    // wr_fence_fence
    int_var__wr_fence_fence = _val.convert_to< wr_fence_fence_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::wr_fence_fence() const {
    return int_var__wr_fence_fence;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::phv_start(const cpp_int & _val) { 
    // phv_start
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::phv_start() const {
    return int_var__phv_start;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::phv_end(const cpp_int & _val) { 
    // phv_end
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::phv_end() const {
    return int_var__phv_end;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::use_override_lif(const cpp_int & _val) { 
    // use_override_lif
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::use_override_lif() const {
    return int_var__use_override_lif;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::pcie_msg(const cpp_int & _val) { 
    // pcie_msg
    int_var__pcie_msg = _val.convert_to< pcie_msg_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::pcie_msg() const {
    return int_var__pcie_msg;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::round(const cpp_int & _val) { 
    // round
    int_var__round = _val.convert_to< round_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::round() const {
    return int_var__round;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::barrier(const cpp_int & _val) { 
    // barrier
    int_var__barrier = _val.convert_to< barrier_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::barrier() const {
    return int_var__barrier;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::addr() const {
    return int_var__addr;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::override_lif(const cpp_int & _val) { 
    // override_lif
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::override_lif() const {
    return int_var__override_lif;
}
    
void cap_ptd_decoders_cmd_phv2mem_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_phv2mem_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::round(const cpp_int & _val) { 
    // round
    int_var__round = _val.convert_to< round_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::round() const {
    return int_var__round;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::host_addr(const cpp_int & _val) { 
    // host_addr
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::host_addr() const {
    return int_var__host_addr;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::cache() const {
    return int_var__cache;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::use_override_lif(const cpp_int & _val) { 
    // use_override_lif
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::use_override_lif() const {
    return int_var__use_override_lif;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::addr() const {
    return int_var__addr;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::override_lif(const cpp_int & _val) { 
    // override_lif
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::override_lif() const {
    return int_var__override_lif;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::rsvd1(const cpp_int & _val) { 
    // rsvd1
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::rsvd1() const {
    return int_var__rsvd1;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::psize(const cpp_int & _val) { 
    // psize
    int_var__psize = _val.convert_to< psize_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::psize() const {
    return int_var__psize;
}
    
void cap_ptd_decoders_cmd_pkt2mem_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_pkt2mem_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_ptd_decoders_cmd_skip_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_skip_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_skip_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_skip_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_skip_t::psize(const cpp_int & _val) { 
    // psize
    int_var__psize = _val.convert_to< psize_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_skip_t::psize() const {
    return int_var__psize;
}
    
void cap_ptd_decoders_cmd_skip_t::skip_to_eop(const cpp_int & _val) { 
    // skip_to_eop
    int_var__skip_to_eop = _val.convert_to< skip_to_eop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_skip_t::skip_to_eop() const {
    return int_var__skip_to_eop;
}
    
void cap_ptd_decoders_cmd_skip_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_skip_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::cmdtype(const cpp_int & _val) { 
    // cmdtype
    int_var__cmdtype = _val.convert_to< cmdtype_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::cmdtype() const {
    return int_var__cmdtype;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::cmdeop(const cpp_int & _val) { 
    // cmdeop
    int_var__cmdeop = _val.convert_to< cmdeop_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::cmdeop() const {
    return int_var__cmdeop;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::mem2mem_type(const cpp_int & _val) { 
    // mem2mem_type
    int_var__mem2mem_type = _val.convert_to< mem2mem_type_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::mem2mem_type() const {
    return int_var__mem2mem_type;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::host_addr(const cpp_int & _val) { 
    // host_addr
    int_var__host_addr = _val.convert_to< host_addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::host_addr() const {
    return int_var__host_addr;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::cache() const {
    return int_var__cache;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::wr_data_fence(const cpp_int & _val) { 
    // wr_data_fence
    int_var__wr_data_fence = _val.convert_to< wr_data_fence_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::wr_data_fence() const {
    return int_var__wr_data_fence;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::wr_fence_fence(const cpp_int & _val) { 
    // wr_fence_fence
    int_var__wr_fence_fence = _val.convert_to< wr_fence_fence_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::wr_fence_fence() const {
    return int_var__wr_fence_fence;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::phv_start(const cpp_int & _val) { 
    // phv_start
    int_var__phv_start = _val.convert_to< phv_start_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::phv_start() const {
    return int_var__phv_start;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::phv_end(const cpp_int & _val) { 
    // phv_end
    int_var__phv_end = _val.convert_to< phv_end_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::phv_end() const {
    return int_var__phv_end;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::use_override_lif(const cpp_int & _val) { 
    // use_override_lif
    int_var__use_override_lif = _val.convert_to< use_override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::use_override_lif() const {
    return int_var__use_override_lif;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::pcie_msg(const cpp_int & _val) { 
    // pcie_msg
    int_var__pcie_msg = _val.convert_to< pcie_msg_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::pcie_msg() const {
    return int_var__pcie_msg;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::round(const cpp_int & _val) { 
    // round
    int_var__round = _val.convert_to< round_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::round() const {
    return int_var__round;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::barrier(const cpp_int & _val) { 
    // barrier
    int_var__barrier = _val.convert_to< barrier_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::barrier() const {
    return int_var__barrier;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::addr() const {
    return int_var__addr;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::override_lif(const cpp_int & _val) { 
    // override_lif
    int_var__override_lif = _val.convert_to< override_lif_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::override_lif() const {
    return int_var__override_lif;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::rsvd1(const cpp_int & _val) { 
    // rsvd1
    int_var__rsvd1 = _val.convert_to< rsvd1_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::rsvd1() const {
    return int_var__rsvd1;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::psize(const cpp_int & _val) { 
    // psize
    int_var__psize = _val.convert_to< psize_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::psize() const {
    return int_var__psize;
}
    
void cap_ptd_decoders_cmd_mem2mem_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_ptd_decoders_cmd_mem2mem_t::rsvd() const {
    return int_var__rsvd;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_mem2pkt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkteop")) { field_val = pkteop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { field_val = host_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { field_val = use_override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { field_val = override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { field_val = rsvd1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { field_val = psize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_phv2pkt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkteop")) { field_val = pkteop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdsize")) { field_val = cmdsize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { field_val = phv_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { field_val = phv_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start1")) { field_val = phv_start1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end1")) { field_val = phv_end1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start2")) { field_val = phv_start2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end2")) { field_val = phv_end2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start3")) { field_val = phv_start3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end3")) { field_val = phv_end3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_phv2mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { field_val = host_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_data_fence")) { field_val = wr_data_fence(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_fence_fence")) { field_val = wr_fence_fence(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { field_val = phv_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { field_val = phv_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { field_val = use_override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pcie_msg")) { field_val = pcie_msg(); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { field_val = round(); field_found=1; }
    if(!field_found && !strcmp(field_name, "barrier")) { field_val = barrier(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { field_val = override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_pkt2mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { field_val = round(); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { field_val = host_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { field_val = use_override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { field_val = override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { field_val = rsvd1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { field_val = psize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_skip_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { field_val = psize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_to_eop")) { field_val = skip_to_eop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_mem2mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { field_val = cmdtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { field_val = cmdeop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mem2mem_type")) { field_val = mem2mem_type(); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { field_val = host_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_data_fence")) { field_val = wr_data_fence(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_fence_fence")) { field_val = wr_fence_fence(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { field_val = phv_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { field_val = phv_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { field_val = use_override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pcie_msg")) { field_val = pcie_msg(); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { field_val = round(); field_found=1; }
    if(!field_found && !strcmp(field_name, "barrier")) { field_val = barrier(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { field_val = override_lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { field_val = rsvd1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { field_val = psize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_mem2pkt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkteop")) { pkteop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { host_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { use_override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { rsvd1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { psize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_phv2pkt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkteop")) { pkteop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdsize")) { cmdsize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { phv_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { phv_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start1")) { phv_start1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end1")) { phv_end1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start2")) { phv_start2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end2")) { phv_end2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start3")) { phv_start3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end3")) { phv_end3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_phv2mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { host_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_data_fence")) { wr_data_fence(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_fence_fence")) { wr_fence_fence(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { phv_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { phv_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { use_override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pcie_msg")) { pcie_msg(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { round(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "barrier")) { barrier(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_pkt2mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { round(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { host_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { use_override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { rsvd1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { psize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_skip_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { psize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "skip_to_eop")) { skip_to_eop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ptd_decoders_cmd_mem2mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cmdtype")) { cmdtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cmdeop")) { cmdeop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mem2mem_type")) { mem2mem_type(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "host_addr")) { host_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_data_fence")) { wr_data_fence(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wr_fence_fence")) { wr_fence_fence(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_start")) { phv_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_end")) { phv_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "use_override_lif")) { use_override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pcie_msg")) { pcie_msg(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "round")) { round(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "barrier")) { barrier(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "override_lif")) { override_lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd1")) { rsvd1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "psize")) { psize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_mem2pkt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("pkteop");
    ret_vec.push_back("host_addr");
    ret_vec.push_back("cache");
    ret_vec.push_back("use_override_lif");
    ret_vec.push_back("addr");
    ret_vec.push_back("override_lif");
    ret_vec.push_back("rsvd1");
    ret_vec.push_back("psize");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_phv2pkt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("pkteop");
    ret_vec.push_back("cmdsize");
    ret_vec.push_back("phv_start");
    ret_vec.push_back("phv_end");
    ret_vec.push_back("phv_start1");
    ret_vec.push_back("phv_end1");
    ret_vec.push_back("phv_start2");
    ret_vec.push_back("phv_end2");
    ret_vec.push_back("phv_start3");
    ret_vec.push_back("phv_end3");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_phv2mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("host_addr");
    ret_vec.push_back("cache");
    ret_vec.push_back("wr_data_fence");
    ret_vec.push_back("wr_fence_fence");
    ret_vec.push_back("phv_start");
    ret_vec.push_back("phv_end");
    ret_vec.push_back("use_override_lif");
    ret_vec.push_back("pcie_msg");
    ret_vec.push_back("round");
    ret_vec.push_back("barrier");
    ret_vec.push_back("addr");
    ret_vec.push_back("override_lif");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_pkt2mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("round");
    ret_vec.push_back("host_addr");
    ret_vec.push_back("cache");
    ret_vec.push_back("use_override_lif");
    ret_vec.push_back("addr");
    ret_vec.push_back("override_lif");
    ret_vec.push_back("rsvd1");
    ret_vec.push_back("psize");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_skip_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("psize");
    ret_vec.push_back("skip_to_eop");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ptd_decoders_cmd_mem2mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cmdtype");
    ret_vec.push_back("cmdeop");
    ret_vec.push_back("mem2mem_type");
    ret_vec.push_back("host_addr");
    ret_vec.push_back("cache");
    ret_vec.push_back("wr_data_fence");
    ret_vec.push_back("wr_fence_fence");
    ret_vec.push_back("phv_start");
    ret_vec.push_back("phv_end");
    ret_vec.push_back("use_override_lif");
    ret_vec.push_back("pcie_msg");
    ret_vec.push_back("round");
    ret_vec.push_back("barrier");
    ret_vec.push_back("addr");
    ret_vec.push_back("override_lif");
    ret_vec.push_back("rsvd1");
    ret_vec.push_back("psize");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
