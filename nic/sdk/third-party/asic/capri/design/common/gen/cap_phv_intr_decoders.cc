
#include "cap_phv_intr_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_phv_intr_global_t::cap_phv_intr_global_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_global_t::~cap_phv_intr_global_t() { }

cap_phv_intr_p4_t::cap_phv_intr_p4_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_p4_t::~cap_phv_intr_p4_t() { }

cap_phv_intr_txdma_t::cap_phv_intr_txdma_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_txdma_t::~cap_phv_intr_txdma_t() { }

cap_phv_intr_rxdma_t::cap_phv_intr_rxdma_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_rxdma_t::~cap_phv_intr_rxdma_t() { }

cap_phv_intr_txicrc_t::cap_phv_intr_txicrc_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_txicrc_t::~cap_phv_intr_txicrc_t() { }

cap_phv_intr_global_pad_t::cap_phv_intr_global_pad_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_phv_intr_global_pad_t::~cap_phv_intr_global_pad_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_global_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tm_instance_type: 0x" << int_var__tm_instance_type << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".error_bits: 0x" << int_var__error_bits << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_err: 0x" << int_var__csum_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_trace: 0x" << int_var__debug_trace << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_oq: 0x" << int_var__tm_oq << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hw_error: 0x" << int_var__hw_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bypass: 0x" << int_var__bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drop: 0x" << int_var__drop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_q_depth: 0x" << int_var__tm_q_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_cpu: 0x" << int_var__tm_cpu << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_replicate_en: 0x" << int_var__tm_replicate_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_replicate_ptr: 0x" << int_var__tm_replicate_ptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_span_session: 0x" << int_var__tm_span_session << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".timestamp: 0x" << int_var__timestamp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_iq: 0x" << int_var__tm_iq << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_oport: 0x" << int_var__tm_oport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tm_iport: 0x" << int_var__tm_iport << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_p4_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_vld: 0x" << int_var__hdr_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".padding: 0x" << int_var__padding << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".packet_len: 0x" << int_var__packet_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc: 0x" << int_var__recirc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".no_data: 0x" << int_var__no_data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".frame_size: 0x" << int_var__frame_size << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crypto_hdr: 0x" << int_var__crypto_hdr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parser_err: 0x" << int_var__parser_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc_count: 0x" << int_var__recirc_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_err: 0x" << int_var__len_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_err: 0x" << int_var__crc_err << dec << endl);
    phv_global.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_txdma_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".padding: 0x" << int_var__padding << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txdma_rsv: 0x" << int_var__txdma_rsv << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype: 0x" << int_var__qtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qstate_addr: 0x" << int_var__qstate_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dma_cmd_ptr: 0x" << int_var__dma_cmd_ptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".packet_len: 0x" << int_var__packet_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc: 0x" << int_var__recirc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".no_data: 0x" << int_var__no_data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".frame_size: 0x" << int_var__frame_size << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crypto_hdr: 0x" << int_var__crypto_hdr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parser_err: 0x" << int_var__parser_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc_count: 0x" << int_var__recirc_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_err: 0x" << int_var__len_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_err: 0x" << int_var__crc_err << dec << endl);
    phv_global.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_rxdma_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".padding: 0x" << int_var__padding << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rxdma_rsv: 0x" << int_var__rxdma_rsv << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rx_splitter_offset: 0x" << int_var__rx_splitter_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qtype: 0x" << int_var__qtype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qstate_addr: 0x" << int_var__qstate_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dma_cmd_ptr: 0x" << int_var__dma_cmd_ptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".packet_len: 0x" << int_var__packet_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc: 0x" << int_var__recirc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".no_data: 0x" << int_var__no_data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".frame_size: 0x" << int_var__frame_size << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crypto_hdr: 0x" << int_var__crypto_hdr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parser_err: 0x" << int_var__parser_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".recirc_count: 0x" << int_var__recirc_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len_err: 0x" << int_var__len_err << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".crc_err: 0x" << int_var__crc_err << dec << endl);
    phv_global.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_txicrc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".padding: 0x" << int_var__padding << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txicrc_rsv: 0x" << int_var__txicrc_rsv << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".udp_opt_csum_vld: 0x" << int_var__udp_opt_csum_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rdma_udp_len: 0x" << int_var__rdma_udp_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rdma_ip_pld_len: 0x" << int_var__rdma_ip_pld_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rdma_ip_offset: 0x" << int_var__rdma_ip_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rdma_ip_type: 0x" << int_var__rdma_ip_type << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".outer_udp_len: 0x" << int_var__outer_udp_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".outer_ip_pld_len: 0x" << int_var__outer_ip_pld_len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".outer_ip_offset: 0x" << int_var__outer_ip_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".outer_ip_type: 0x" << int_var__outer_ip_type << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".outer_layer_vld: 0x" << int_var__outer_layer_vld << dec << endl);
    phv_global.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_phv_intr_global_pad_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".padding: 0x" << int_var__padding << dec << endl);
    phv_global.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_phv_intr_global_t::get_width() const {
    return cap_phv_intr_global_t::s_get_width();

}

int cap_phv_intr_p4_t::get_width() const {
    return cap_phv_intr_p4_t::s_get_width();

}

int cap_phv_intr_txdma_t::get_width() const {
    return cap_phv_intr_txdma_t::s_get_width();

}

int cap_phv_intr_rxdma_t::get_width() const {
    return cap_phv_intr_rxdma_t::s_get_width();

}

int cap_phv_intr_txicrc_t::get_width() const {
    return cap_phv_intr_txicrc_t::s_get_width();

}

int cap_phv_intr_global_pad_t::get_width() const {
    return cap_phv_intr_global_pad_t::s_get_width();

}

int cap_phv_intr_global_t::s_get_width() {
    int _count = 0;

    _count += 4; // tm_instance_type
    _count += 6; // error_bits
    _count += 5; // csum_err
    _count += 1; // debug_trace
    _count += 5; // tm_oq
    _count += 1; // hw_error
    _count += 1; // bypass
    _count += 1; // drop
    _count += 14; // tm_q_depth
    _count += 1; // tm_cpu
    _count += 1; // tm_replicate_en
    _count += 16; // tm_replicate_ptr
    _count += 8; // tm_span_session
    _count += 48; // timestamp
    _count += 11; // lif
    _count += 5; // tm_iq
    _count += 4; // tm_oport
    _count += 4; // tm_iport
    return _count;
}

int cap_phv_intr_p4_t::s_get_width() {
    int _count = 0;

    _count += 128; // hdr_vld
    _count += 208; // padding
    _count += 14; // packet_len
    _count += 1; // recirc
    _count += 1; // no_data
    _count += 14; // frame_size
    _count += 1; // crypto_hdr
    _count += 1; // parser_err
    _count += 3; // recirc_count
    _count += 4; // len_err
    _count += 1; // crc_err
    _count += cap_phv_intr_global_t::s_get_width(); // phv_global
    return _count;
}

int cap_phv_intr_txdma_t::s_get_width() {
    int _count = 0;

    _count += 264; // padding
    _count += 5; // txdma_rsv
    _count += 3; // qtype
    _count += 34; // qstate_addr
    _count += 6; // dma_cmd_ptr
    _count += 24; // qid
    _count += 14; // packet_len
    _count += 1; // recirc
    _count += 1; // no_data
    _count += 14; // frame_size
    _count += 1; // crypto_hdr
    _count += 1; // parser_err
    _count += 3; // recirc_count
    _count += 4; // len_err
    _count += 1; // crc_err
    _count += cap_phv_intr_global_t::s_get_width(); // phv_global
    return _count;
}

int cap_phv_intr_rxdma_t::s_get_width() {
    int _count = 0;

    _count += 256; // padding
    _count += 3; // rxdma_rsv
    _count += 10; // rx_splitter_offset
    _count += 3; // qtype
    _count += 34; // qstate_addr
    _count += 6; // dma_cmd_ptr
    _count += 24; // qid
    _count += 14; // packet_len
    _count += 1; // recirc
    _count += 1; // no_data
    _count += 14; // frame_size
    _count += 1; // crypto_hdr
    _count += 1; // parser_err
    _count += 3; // recirc_count
    _count += 4; // len_err
    _count += 1; // crc_err
    _count += cap_phv_intr_global_t::s_get_width(); // phv_global
    return _count;
}

int cap_phv_intr_txicrc_t::s_get_width() {
    int _count = 0;

    _count += 296; // padding
    _count += 4; // txicrc_rsv
    _count += 1; // udp_opt_csum_vld
    _count += 14; // rdma_udp_len
    _count += 14; // rdma_ip_pld_len
    _count += 8; // rdma_ip_offset
    _count += 1; // rdma_ip_type
    _count += 14; // outer_udp_len
    _count += 14; // outer_ip_pld_len
    _count += 8; // outer_ip_offset
    _count += 1; // outer_ip_type
    _count += 1; // outer_layer_vld
    _count += cap_phv_intr_global_t::s_get_width(); // phv_global
    return _count;
}

int cap_phv_intr_global_pad_t::s_get_width() {
    int _count = 0;

    _count += 376; // padding
    _count += cap_phv_intr_global_t::s_get_width(); // phv_global
    return _count;
}

void cap_phv_intr_global_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tm_instance_type = _val.convert_to< tm_instance_type_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__error_bits = _val.convert_to< error_bits_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__csum_err = _val.convert_to< csum_err_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__debug_trace = _val.convert_to< debug_trace_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tm_oq = _val.convert_to< tm_oq_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__hw_error = _val.convert_to< hw_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bypass = _val.convert_to< bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__drop = _val.convert_to< drop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tm_q_depth = _val.convert_to< tm_q_depth_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__tm_cpu = _val.convert_to< tm_cpu_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tm_replicate_en = _val.convert_to< tm_replicate_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tm_replicate_ptr = _val.convert_to< tm_replicate_ptr_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__tm_span_session = _val.convert_to< tm_span_session_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__timestamp = _val.convert_to< timestamp_cpp_int_t >()  ;
    _val = _val >> 48;
    
    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__tm_iq = _val.convert_to< tm_iq_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__tm_oport = _val.convert_to< tm_oport_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__tm_iport = _val.convert_to< tm_iport_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_phv_intr_p4_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hdr_vld = _val.convert_to< hdr_vld_cpp_int_t >()  ;
    _val = _val >> 128;
    
    int_var__padding = _val.convert_to< padding_cpp_int_t >()  ;
    _val = _val >> 208;
    
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    phv_global.all( _val);
    _val = _val >> phv_global.get_width(); 
}

void cap_phv_intr_txdma_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__padding = _val.convert_to< padding_cpp_int_t >()  ;
    _val = _val >> 264;
    
    int_var__txdma_rsv = _val.convert_to< txdma_rsv_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qstate_addr = _val.convert_to< qstate_addr_cpp_int_t >()  ;
    _val = _val >> 34;
    
    int_var__dma_cmd_ptr = _val.convert_to< dma_cmd_ptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    phv_global.all( _val);
    _val = _val >> phv_global.get_width(); 
}

void cap_phv_intr_rxdma_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__padding = _val.convert_to< padding_cpp_int_t >()  ;
    _val = _val >> 256;
    
    int_var__rxdma_rsv = _val.convert_to< rxdma_rsv_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__rx_splitter_offset = _val.convert_to< rx_splitter_offset_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qstate_addr = _val.convert_to< qstate_addr_cpp_int_t >()  ;
    _val = _val >> 34;
    
    int_var__dma_cmd_ptr = _val.convert_to< dma_cmd_ptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >()  ;
    _val = _val >> 1;
    
    phv_global.all( _val);
    _val = _val >> phv_global.get_width(); 
}

void cap_phv_intr_txicrc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__padding = _val.convert_to< padding_cpp_int_t >()  ;
    _val = _val >> 296;
    
    int_var__txicrc_rsv = _val.convert_to< txicrc_rsv_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__udp_opt_csum_vld = _val.convert_to< udp_opt_csum_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rdma_udp_len = _val.convert_to< rdma_udp_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__rdma_ip_pld_len = _val.convert_to< rdma_ip_pld_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__rdma_ip_offset = _val.convert_to< rdma_ip_offset_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__rdma_ip_type = _val.convert_to< rdma_ip_type_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__outer_udp_len = _val.convert_to< outer_udp_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__outer_ip_pld_len = _val.convert_to< outer_ip_pld_len_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__outer_ip_offset = _val.convert_to< outer_ip_offset_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__outer_ip_type = _val.convert_to< outer_ip_type_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__outer_layer_vld = _val.convert_to< outer_layer_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    phv_global.all( _val);
    _val = _val >> phv_global.get_width(); 
}

void cap_phv_intr_global_pad_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__padding = _val.convert_to< padding_cpp_int_t >()  ;
    _val = _val >> 376;
    
    phv_global.all( _val);
    _val = _val >> phv_global.get_width(); 
}

cpp_int cap_phv_intr_global_pad_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << phv_global.get_width(); ret_val = ret_val  | phv_global.all(); 
    // padding
    ret_val = ret_val << 376; ret_val = ret_val  | int_var__padding; 
    
    return ret_val;
}

cpp_int cap_phv_intr_txicrc_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << phv_global.get_width(); ret_val = ret_val  | phv_global.all(); 
    // outer_layer_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__outer_layer_vld; 
    
    // outer_ip_type
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__outer_ip_type; 
    
    // outer_ip_offset
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__outer_ip_offset; 
    
    // outer_ip_pld_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__outer_ip_pld_len; 
    
    // outer_udp_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__outer_udp_len; 
    
    // rdma_ip_type
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rdma_ip_type; 
    
    // rdma_ip_offset
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__rdma_ip_offset; 
    
    // rdma_ip_pld_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__rdma_ip_pld_len; 
    
    // rdma_udp_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__rdma_udp_len; 
    
    // udp_opt_csum_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__udp_opt_csum_vld; 
    
    // txicrc_rsv
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__txicrc_rsv; 
    
    // padding
    ret_val = ret_val << 296; ret_val = ret_val  | int_var__padding; 
    
    return ret_val;
}

cpp_int cap_phv_intr_rxdma_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << phv_global.get_width(); ret_val = ret_val  | phv_global.all(); 
    // crc_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_err; 
    
    // len_err
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len_err; 
    
    // recirc_count
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__recirc_count; 
    
    // parser_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parser_err; 
    
    // crypto_hdr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crypto_hdr; 
    
    // frame_size
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__frame_size; 
    
    // no_data
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__no_data; 
    
    // recirc
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__recirc; 
    
    // packet_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__packet_len; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    // dma_cmd_ptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__dma_cmd_ptr; 
    
    // qstate_addr
    ret_val = ret_val << 34; ret_val = ret_val  | int_var__qstate_addr; 
    
    // qtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__qtype; 
    
    // rx_splitter_offset
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__rx_splitter_offset; 
    
    // rxdma_rsv
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__rxdma_rsv; 
    
    // padding
    ret_val = ret_val << 256; ret_val = ret_val  | int_var__padding; 
    
    return ret_val;
}

cpp_int cap_phv_intr_txdma_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << phv_global.get_width(); ret_val = ret_val  | phv_global.all(); 
    // crc_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_err; 
    
    // len_err
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len_err; 
    
    // recirc_count
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__recirc_count; 
    
    // parser_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parser_err; 
    
    // crypto_hdr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crypto_hdr; 
    
    // frame_size
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__frame_size; 
    
    // no_data
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__no_data; 
    
    // recirc
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__recirc; 
    
    // packet_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__packet_len; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    // dma_cmd_ptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__dma_cmd_ptr; 
    
    // qstate_addr
    ret_val = ret_val << 34; ret_val = ret_val  | int_var__qstate_addr; 
    
    // qtype
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__qtype; 
    
    // txdma_rsv
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__txdma_rsv; 
    
    // padding
    ret_val = ret_val << 264; ret_val = ret_val  | int_var__padding; 
    
    return ret_val;
}

cpp_int cap_phv_intr_p4_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << phv_global.get_width(); ret_val = ret_val  | phv_global.all(); 
    // crc_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crc_err; 
    
    // len_err
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len_err; 
    
    // recirc_count
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__recirc_count; 
    
    // parser_err
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parser_err; 
    
    // crypto_hdr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__crypto_hdr; 
    
    // frame_size
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__frame_size; 
    
    // no_data
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__no_data; 
    
    // recirc
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__recirc; 
    
    // packet_len
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__packet_len; 
    
    // padding
    ret_val = ret_val << 208; ret_val = ret_val  | int_var__padding; 
    
    // hdr_vld
    ret_val = ret_val << 128; ret_val = ret_val  | int_var__hdr_vld; 
    
    return ret_val;
}

cpp_int cap_phv_intr_global_t::all() const {
    cpp_int ret_val;

    // tm_iport
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__tm_iport; 
    
    // tm_oport
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__tm_oport; 
    
    // tm_iq
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__tm_iq; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    // timestamp
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__timestamp; 
    
    // tm_span_session
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__tm_span_session; 
    
    // tm_replicate_ptr
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__tm_replicate_ptr; 
    
    // tm_replicate_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tm_replicate_en; 
    
    // tm_cpu
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tm_cpu; 
    
    // tm_q_depth
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__tm_q_depth; 
    
    // drop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__drop; 
    
    // bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bypass; 
    
    // hw_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hw_error; 
    
    // tm_oq
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__tm_oq; 
    
    // debug_trace
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_trace; 
    
    // csum_err
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__csum_err; 
    
    // error_bits
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__error_bits; 
    
    // tm_instance_type
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__tm_instance_type; 
    
    return ret_val;
}

void cap_phv_intr_global_t::clear() {

    int_var__tm_instance_type = 0; 
    
    int_var__error_bits = 0; 
    
    int_var__csum_err = 0; 
    
    int_var__debug_trace = 0; 
    
    int_var__tm_oq = 0; 
    
    int_var__hw_error = 0; 
    
    int_var__bypass = 0; 
    
    int_var__drop = 0; 
    
    int_var__tm_q_depth = 0; 
    
    int_var__tm_cpu = 0; 
    
    int_var__tm_replicate_en = 0; 
    
    int_var__tm_replicate_ptr = 0; 
    
    int_var__tm_span_session = 0; 
    
    int_var__timestamp = 0; 
    
    int_var__lif = 0; 
    
    int_var__tm_iq = 0; 
    
    int_var__tm_oport = 0; 
    
    int_var__tm_iport = 0; 
    
}

void cap_phv_intr_p4_t::clear() {

    int_var__hdr_vld = 0; 
    
    int_var__padding = 0; 
    
    int_var__packet_len = 0; 
    
    int_var__recirc = 0; 
    
    int_var__no_data = 0; 
    
    int_var__frame_size = 0; 
    
    int_var__crypto_hdr = 0; 
    
    int_var__parser_err = 0; 
    
    int_var__recirc_count = 0; 
    
    int_var__len_err = 0; 
    
    int_var__crc_err = 0; 
    
    phv_global.clear();
}

void cap_phv_intr_txdma_t::clear() {

    int_var__padding = 0; 
    
    int_var__txdma_rsv = 0; 
    
    int_var__qtype = 0; 
    
    int_var__qstate_addr = 0; 
    
    int_var__dma_cmd_ptr = 0; 
    
    int_var__qid = 0; 
    
    int_var__packet_len = 0; 
    
    int_var__recirc = 0; 
    
    int_var__no_data = 0; 
    
    int_var__frame_size = 0; 
    
    int_var__crypto_hdr = 0; 
    
    int_var__parser_err = 0; 
    
    int_var__recirc_count = 0; 
    
    int_var__len_err = 0; 
    
    int_var__crc_err = 0; 
    
    phv_global.clear();
}

void cap_phv_intr_rxdma_t::clear() {

    int_var__padding = 0; 
    
    int_var__rxdma_rsv = 0; 
    
    int_var__rx_splitter_offset = 0; 
    
    int_var__qtype = 0; 
    
    int_var__qstate_addr = 0; 
    
    int_var__dma_cmd_ptr = 0; 
    
    int_var__qid = 0; 
    
    int_var__packet_len = 0; 
    
    int_var__recirc = 0; 
    
    int_var__no_data = 0; 
    
    int_var__frame_size = 0; 
    
    int_var__crypto_hdr = 0; 
    
    int_var__parser_err = 0; 
    
    int_var__recirc_count = 0; 
    
    int_var__len_err = 0; 
    
    int_var__crc_err = 0; 
    
    phv_global.clear();
}

void cap_phv_intr_txicrc_t::clear() {

    int_var__padding = 0; 
    
    int_var__txicrc_rsv = 0; 
    
    int_var__udp_opt_csum_vld = 0; 
    
    int_var__rdma_udp_len = 0; 
    
    int_var__rdma_ip_pld_len = 0; 
    
    int_var__rdma_ip_offset = 0; 
    
    int_var__rdma_ip_type = 0; 
    
    int_var__outer_udp_len = 0; 
    
    int_var__outer_ip_pld_len = 0; 
    
    int_var__outer_ip_offset = 0; 
    
    int_var__outer_ip_type = 0; 
    
    int_var__outer_layer_vld = 0; 
    
    phv_global.clear();
}

void cap_phv_intr_global_pad_t::clear() {

    int_var__padding = 0; 
    
    phv_global.clear();
}

void cap_phv_intr_global_t::init() {

}

void cap_phv_intr_p4_t::init() {

    phv_global.set_attributes(this,"phv_global", 0x0 );
}

void cap_phv_intr_txdma_t::init() {

    phv_global.set_attributes(this,"phv_global", 0x0 );
}

void cap_phv_intr_rxdma_t::init() {

    phv_global.set_attributes(this,"phv_global", 0x0 );
}

void cap_phv_intr_txicrc_t::init() {

    phv_global.set_attributes(this,"phv_global", 0x0 );
}

void cap_phv_intr_global_pad_t::init() {

    phv_global.set_attributes(this,"phv_global", 0x0 );
}

void cap_phv_intr_global_t::tm_instance_type(const cpp_int & _val) { 
    // tm_instance_type
    int_var__tm_instance_type = _val.convert_to< tm_instance_type_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_instance_type() const {
    return int_var__tm_instance_type;
}
    
void cap_phv_intr_global_t::error_bits(const cpp_int & _val) { 
    // error_bits
    int_var__error_bits = _val.convert_to< error_bits_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::error_bits() const {
    return int_var__error_bits;
}
    
void cap_phv_intr_global_t::csum_err(const cpp_int & _val) { 
    // csum_err
    int_var__csum_err = _val.convert_to< csum_err_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::csum_err() const {
    return int_var__csum_err;
}
    
void cap_phv_intr_global_t::debug_trace(const cpp_int & _val) { 
    // debug_trace
    int_var__debug_trace = _val.convert_to< debug_trace_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::debug_trace() const {
    return int_var__debug_trace;
}
    
void cap_phv_intr_global_t::tm_oq(const cpp_int & _val) { 
    // tm_oq
    int_var__tm_oq = _val.convert_to< tm_oq_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_oq() const {
    return int_var__tm_oq;
}
    
void cap_phv_intr_global_t::hw_error(const cpp_int & _val) { 
    // hw_error
    int_var__hw_error = _val.convert_to< hw_error_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::hw_error() const {
    return int_var__hw_error;
}
    
void cap_phv_intr_global_t::bypass(const cpp_int & _val) { 
    // bypass
    int_var__bypass = _val.convert_to< bypass_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::bypass() const {
    return int_var__bypass;
}
    
void cap_phv_intr_global_t::drop(const cpp_int & _val) { 
    // drop
    int_var__drop = _val.convert_to< drop_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::drop() const {
    return int_var__drop;
}
    
void cap_phv_intr_global_t::tm_q_depth(const cpp_int & _val) { 
    // tm_q_depth
    int_var__tm_q_depth = _val.convert_to< tm_q_depth_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_q_depth() const {
    return int_var__tm_q_depth;
}
    
void cap_phv_intr_global_t::tm_cpu(const cpp_int & _val) { 
    // tm_cpu
    int_var__tm_cpu = _val.convert_to< tm_cpu_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_cpu() const {
    return int_var__tm_cpu;
}
    
void cap_phv_intr_global_t::tm_replicate_en(const cpp_int & _val) { 
    // tm_replicate_en
    int_var__tm_replicate_en = _val.convert_to< tm_replicate_en_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_replicate_en() const {
    return int_var__tm_replicate_en;
}
    
void cap_phv_intr_global_t::tm_replicate_ptr(const cpp_int & _val) { 
    // tm_replicate_ptr
    int_var__tm_replicate_ptr = _val.convert_to< tm_replicate_ptr_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_replicate_ptr() const {
    return int_var__tm_replicate_ptr;
}
    
void cap_phv_intr_global_t::tm_span_session(const cpp_int & _val) { 
    // tm_span_session
    int_var__tm_span_session = _val.convert_to< tm_span_session_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_span_session() const {
    return int_var__tm_span_session;
}
    
void cap_phv_intr_global_t::timestamp(const cpp_int & _val) { 
    // timestamp
    int_var__timestamp = _val.convert_to< timestamp_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::timestamp() const {
    return int_var__timestamp;
}
    
void cap_phv_intr_global_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::lif() const {
    return int_var__lif;
}
    
void cap_phv_intr_global_t::tm_iq(const cpp_int & _val) { 
    // tm_iq
    int_var__tm_iq = _val.convert_to< tm_iq_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_iq() const {
    return int_var__tm_iq;
}
    
void cap_phv_intr_global_t::tm_oport(const cpp_int & _val) { 
    // tm_oport
    int_var__tm_oport = _val.convert_to< tm_oport_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_oport() const {
    return int_var__tm_oport;
}
    
void cap_phv_intr_global_t::tm_iport(const cpp_int & _val) { 
    // tm_iport
    int_var__tm_iport = _val.convert_to< tm_iport_cpp_int_t >();
}

cpp_int cap_phv_intr_global_t::tm_iport() const {
    return int_var__tm_iport;
}
    
void cap_phv_intr_p4_t::hdr_vld(const cpp_int & _val) { 
    // hdr_vld
    int_var__hdr_vld = _val.convert_to< hdr_vld_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::hdr_vld() const {
    return int_var__hdr_vld;
}
    
void cap_phv_intr_p4_t::padding(const cpp_int & _val) { 
    // padding
    int_var__padding = _val.convert_to< padding_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::padding() const {
    return int_var__padding;
}
    
void cap_phv_intr_p4_t::packet_len(const cpp_int & _val) { 
    // packet_len
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::packet_len() const {
    return int_var__packet_len;
}
    
void cap_phv_intr_p4_t::recirc(const cpp_int & _val) { 
    // recirc
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::recirc() const {
    return int_var__recirc;
}
    
void cap_phv_intr_p4_t::no_data(const cpp_int & _val) { 
    // no_data
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::no_data() const {
    return int_var__no_data;
}
    
void cap_phv_intr_p4_t::frame_size(const cpp_int & _val) { 
    // frame_size
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::frame_size() const {
    return int_var__frame_size;
}
    
void cap_phv_intr_p4_t::crypto_hdr(const cpp_int & _val) { 
    // crypto_hdr
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::crypto_hdr() const {
    return int_var__crypto_hdr;
}
    
void cap_phv_intr_p4_t::parser_err(const cpp_int & _val) { 
    // parser_err
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::parser_err() const {
    return int_var__parser_err;
}
    
void cap_phv_intr_p4_t::recirc_count(const cpp_int & _val) { 
    // recirc_count
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::recirc_count() const {
    return int_var__recirc_count;
}
    
void cap_phv_intr_p4_t::len_err(const cpp_int & _val) { 
    // len_err
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::len_err() const {
    return int_var__len_err;
}
    
void cap_phv_intr_p4_t::crc_err(const cpp_int & _val) { 
    // crc_err
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >();
}

cpp_int cap_phv_intr_p4_t::crc_err() const {
    return int_var__crc_err;
}
    
void cap_phv_intr_txdma_t::padding(const cpp_int & _val) { 
    // padding
    int_var__padding = _val.convert_to< padding_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::padding() const {
    return int_var__padding;
}
    
void cap_phv_intr_txdma_t::txdma_rsv(const cpp_int & _val) { 
    // txdma_rsv
    int_var__txdma_rsv = _val.convert_to< txdma_rsv_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::txdma_rsv() const {
    return int_var__txdma_rsv;
}
    
void cap_phv_intr_txdma_t::qtype(const cpp_int & _val) { 
    // qtype
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::qtype() const {
    return int_var__qtype;
}
    
void cap_phv_intr_txdma_t::qstate_addr(const cpp_int & _val) { 
    // qstate_addr
    int_var__qstate_addr = _val.convert_to< qstate_addr_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::qstate_addr() const {
    return int_var__qstate_addr;
}
    
void cap_phv_intr_txdma_t::dma_cmd_ptr(const cpp_int & _val) { 
    // dma_cmd_ptr
    int_var__dma_cmd_ptr = _val.convert_to< dma_cmd_ptr_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::dma_cmd_ptr() const {
    return int_var__dma_cmd_ptr;
}
    
void cap_phv_intr_txdma_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::qid() const {
    return int_var__qid;
}
    
void cap_phv_intr_txdma_t::packet_len(const cpp_int & _val) { 
    // packet_len
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::packet_len() const {
    return int_var__packet_len;
}
    
void cap_phv_intr_txdma_t::recirc(const cpp_int & _val) { 
    // recirc
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::recirc() const {
    return int_var__recirc;
}
    
void cap_phv_intr_txdma_t::no_data(const cpp_int & _val) { 
    // no_data
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::no_data() const {
    return int_var__no_data;
}
    
void cap_phv_intr_txdma_t::frame_size(const cpp_int & _val) { 
    // frame_size
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::frame_size() const {
    return int_var__frame_size;
}
    
void cap_phv_intr_txdma_t::crypto_hdr(const cpp_int & _val) { 
    // crypto_hdr
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::crypto_hdr() const {
    return int_var__crypto_hdr;
}
    
void cap_phv_intr_txdma_t::parser_err(const cpp_int & _val) { 
    // parser_err
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::parser_err() const {
    return int_var__parser_err;
}
    
void cap_phv_intr_txdma_t::recirc_count(const cpp_int & _val) { 
    // recirc_count
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::recirc_count() const {
    return int_var__recirc_count;
}
    
void cap_phv_intr_txdma_t::len_err(const cpp_int & _val) { 
    // len_err
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::len_err() const {
    return int_var__len_err;
}
    
void cap_phv_intr_txdma_t::crc_err(const cpp_int & _val) { 
    // crc_err
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >();
}

cpp_int cap_phv_intr_txdma_t::crc_err() const {
    return int_var__crc_err;
}
    
void cap_phv_intr_rxdma_t::padding(const cpp_int & _val) { 
    // padding
    int_var__padding = _val.convert_to< padding_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::padding() const {
    return int_var__padding;
}
    
void cap_phv_intr_rxdma_t::rxdma_rsv(const cpp_int & _val) { 
    // rxdma_rsv
    int_var__rxdma_rsv = _val.convert_to< rxdma_rsv_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::rxdma_rsv() const {
    return int_var__rxdma_rsv;
}
    
void cap_phv_intr_rxdma_t::rx_splitter_offset(const cpp_int & _val) { 
    // rx_splitter_offset
    int_var__rx_splitter_offset = _val.convert_to< rx_splitter_offset_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::rx_splitter_offset() const {
    return int_var__rx_splitter_offset;
}
    
void cap_phv_intr_rxdma_t::qtype(const cpp_int & _val) { 
    // qtype
    int_var__qtype = _val.convert_to< qtype_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::qtype() const {
    return int_var__qtype;
}
    
void cap_phv_intr_rxdma_t::qstate_addr(const cpp_int & _val) { 
    // qstate_addr
    int_var__qstate_addr = _val.convert_to< qstate_addr_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::qstate_addr() const {
    return int_var__qstate_addr;
}
    
void cap_phv_intr_rxdma_t::dma_cmd_ptr(const cpp_int & _val) { 
    // dma_cmd_ptr
    int_var__dma_cmd_ptr = _val.convert_to< dma_cmd_ptr_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::dma_cmd_ptr() const {
    return int_var__dma_cmd_ptr;
}
    
void cap_phv_intr_rxdma_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::qid() const {
    return int_var__qid;
}
    
void cap_phv_intr_rxdma_t::packet_len(const cpp_int & _val) { 
    // packet_len
    int_var__packet_len = _val.convert_to< packet_len_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::packet_len() const {
    return int_var__packet_len;
}
    
void cap_phv_intr_rxdma_t::recirc(const cpp_int & _val) { 
    // recirc
    int_var__recirc = _val.convert_to< recirc_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::recirc() const {
    return int_var__recirc;
}
    
void cap_phv_intr_rxdma_t::no_data(const cpp_int & _val) { 
    // no_data
    int_var__no_data = _val.convert_to< no_data_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::no_data() const {
    return int_var__no_data;
}
    
void cap_phv_intr_rxdma_t::frame_size(const cpp_int & _val) { 
    // frame_size
    int_var__frame_size = _val.convert_to< frame_size_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::frame_size() const {
    return int_var__frame_size;
}
    
void cap_phv_intr_rxdma_t::crypto_hdr(const cpp_int & _val) { 
    // crypto_hdr
    int_var__crypto_hdr = _val.convert_to< crypto_hdr_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::crypto_hdr() const {
    return int_var__crypto_hdr;
}
    
void cap_phv_intr_rxdma_t::parser_err(const cpp_int & _val) { 
    // parser_err
    int_var__parser_err = _val.convert_to< parser_err_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::parser_err() const {
    return int_var__parser_err;
}
    
void cap_phv_intr_rxdma_t::recirc_count(const cpp_int & _val) { 
    // recirc_count
    int_var__recirc_count = _val.convert_to< recirc_count_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::recirc_count() const {
    return int_var__recirc_count;
}
    
void cap_phv_intr_rxdma_t::len_err(const cpp_int & _val) { 
    // len_err
    int_var__len_err = _val.convert_to< len_err_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::len_err() const {
    return int_var__len_err;
}
    
void cap_phv_intr_rxdma_t::crc_err(const cpp_int & _val) { 
    // crc_err
    int_var__crc_err = _val.convert_to< crc_err_cpp_int_t >();
}

cpp_int cap_phv_intr_rxdma_t::crc_err() const {
    return int_var__crc_err;
}
    
void cap_phv_intr_txicrc_t::padding(const cpp_int & _val) { 
    // padding
    int_var__padding = _val.convert_to< padding_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::padding() const {
    return int_var__padding;
}
    
void cap_phv_intr_txicrc_t::txicrc_rsv(const cpp_int & _val) { 
    // txicrc_rsv
    int_var__txicrc_rsv = _val.convert_to< txicrc_rsv_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::txicrc_rsv() const {
    return int_var__txicrc_rsv;
}
    
void cap_phv_intr_txicrc_t::udp_opt_csum_vld(const cpp_int & _val) { 
    // udp_opt_csum_vld
    int_var__udp_opt_csum_vld = _val.convert_to< udp_opt_csum_vld_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::udp_opt_csum_vld() const {
    return int_var__udp_opt_csum_vld;
}
    
void cap_phv_intr_txicrc_t::rdma_udp_len(const cpp_int & _val) { 
    // rdma_udp_len
    int_var__rdma_udp_len = _val.convert_to< rdma_udp_len_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::rdma_udp_len() const {
    return int_var__rdma_udp_len;
}
    
void cap_phv_intr_txicrc_t::rdma_ip_pld_len(const cpp_int & _val) { 
    // rdma_ip_pld_len
    int_var__rdma_ip_pld_len = _val.convert_to< rdma_ip_pld_len_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::rdma_ip_pld_len() const {
    return int_var__rdma_ip_pld_len;
}
    
void cap_phv_intr_txicrc_t::rdma_ip_offset(const cpp_int & _val) { 
    // rdma_ip_offset
    int_var__rdma_ip_offset = _val.convert_to< rdma_ip_offset_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::rdma_ip_offset() const {
    return int_var__rdma_ip_offset;
}
    
void cap_phv_intr_txicrc_t::rdma_ip_type(const cpp_int & _val) { 
    // rdma_ip_type
    int_var__rdma_ip_type = _val.convert_to< rdma_ip_type_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::rdma_ip_type() const {
    return int_var__rdma_ip_type;
}
    
void cap_phv_intr_txicrc_t::outer_udp_len(const cpp_int & _val) { 
    // outer_udp_len
    int_var__outer_udp_len = _val.convert_to< outer_udp_len_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::outer_udp_len() const {
    return int_var__outer_udp_len;
}
    
void cap_phv_intr_txicrc_t::outer_ip_pld_len(const cpp_int & _val) { 
    // outer_ip_pld_len
    int_var__outer_ip_pld_len = _val.convert_to< outer_ip_pld_len_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::outer_ip_pld_len() const {
    return int_var__outer_ip_pld_len;
}
    
void cap_phv_intr_txicrc_t::outer_ip_offset(const cpp_int & _val) { 
    // outer_ip_offset
    int_var__outer_ip_offset = _val.convert_to< outer_ip_offset_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::outer_ip_offset() const {
    return int_var__outer_ip_offset;
}
    
void cap_phv_intr_txicrc_t::outer_ip_type(const cpp_int & _val) { 
    // outer_ip_type
    int_var__outer_ip_type = _val.convert_to< outer_ip_type_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::outer_ip_type() const {
    return int_var__outer_ip_type;
}
    
void cap_phv_intr_txicrc_t::outer_layer_vld(const cpp_int & _val) { 
    // outer_layer_vld
    int_var__outer_layer_vld = _val.convert_to< outer_layer_vld_cpp_int_t >();
}

cpp_int cap_phv_intr_txicrc_t::outer_layer_vld() const {
    return int_var__outer_layer_vld;
}
    
void cap_phv_intr_global_pad_t::padding(const cpp_int & _val) { 
    // padding
    int_var__padding = _val.convert_to< padding_cpp_int_t >();
}

cpp_int cap_phv_intr_global_pad_t::padding() const {
    return int_var__padding;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_global_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tm_instance_type")) { field_val = tm_instance_type(); field_found=1; }
    if(!field_found && !strcmp(field_name, "error_bits")) { field_val = error_bits(); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_err")) { field_val = csum_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_trace")) { field_val = debug_trace(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_oq")) { field_val = tm_oq(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_error")) { field_val = hw_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bypass")) { field_val = bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drop")) { field_val = drop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_q_depth")) { field_val = tm_q_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_cpu")) { field_val = tm_cpu(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_replicate_en")) { field_val = tm_replicate_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_replicate_ptr")) { field_val = tm_replicate_ptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_span_session")) { field_val = tm_span_session(); field_found=1; }
    if(!field_found && !strcmp(field_name, "timestamp")) { field_val = timestamp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_iq")) { field_val = tm_iq(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_oport")) { field_val = tm_oport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_iport")) { field_val = tm_iport(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_p4_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_vld")) { field_val = hdr_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "padding")) { field_val = padding(); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { field_val = packet_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { field_val = recirc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { field_val = no_data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { field_val = frame_size(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { field_val = crypto_hdr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { field_val = parser_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { field_val = recirc_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { field_val = len_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { field_val = crc_err(); field_found=1; }
    if(!field_found) { field_found = phv_global.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_txdma_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { field_val = padding(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_rsv")) { field_val = txdma_rsv(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qtype")) { field_val = qtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_addr")) { field_val = qstate_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dma_cmd_ptr")) { field_val = dma_cmd_ptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { field_val = packet_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { field_val = recirc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { field_val = no_data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { field_val = frame_size(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { field_val = crypto_hdr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { field_val = parser_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { field_val = recirc_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { field_val = len_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { field_val = crc_err(); field_found=1; }
    if(!field_found) { field_found = phv_global.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_rxdma_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { field_val = padding(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxdma_rsv")) { field_val = rxdma_rsv(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_splitter_offset")) { field_val = rx_splitter_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qtype")) { field_val = qtype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_addr")) { field_val = qstate_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dma_cmd_ptr")) { field_val = dma_cmd_ptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { field_val = packet_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { field_val = recirc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { field_val = no_data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { field_val = frame_size(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { field_val = crypto_hdr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { field_val = parser_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { field_val = recirc_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { field_val = len_err(); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { field_val = crc_err(); field_found=1; }
    if(!field_found) { field_found = phv_global.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_txicrc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { field_val = padding(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txicrc_rsv")) { field_val = txicrc_rsv(); field_found=1; }
    if(!field_found && !strcmp(field_name, "udp_opt_csum_vld")) { field_val = udp_opt_csum_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_udp_len")) { field_val = rdma_udp_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_pld_len")) { field_val = rdma_ip_pld_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_offset")) { field_val = rdma_ip_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_type")) { field_val = rdma_ip_type(); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_udp_len")) { field_val = outer_udp_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_pld_len")) { field_val = outer_ip_pld_len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_offset")) { field_val = outer_ip_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_type")) { field_val = outer_ip_type(); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_layer_vld")) { field_val = outer_layer_vld(); field_found=1; }
    if(!field_found) { field_found = phv_global.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_global_pad_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { field_val = padding(); field_found=1; }
    if(!field_found) { field_found = phv_global.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_global_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tm_instance_type")) { tm_instance_type(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "error_bits")) { error_bits(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "csum_err")) { csum_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_trace")) { debug_trace(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_oq")) { tm_oq(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hw_error")) { hw_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bypass")) { bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drop")) { drop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_q_depth")) { tm_q_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_cpu")) { tm_cpu(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_replicate_en")) { tm_replicate_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_replicate_ptr")) { tm_replicate_ptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_span_session")) { tm_span_session(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "timestamp")) { timestamp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_iq")) { tm_iq(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_oport")) { tm_oport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tm_iport")) { tm_iport(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_p4_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hdr_vld")) { hdr_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "padding")) { padding(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { packet_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { recirc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { no_data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { frame_size(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { crypto_hdr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { parser_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { recirc_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { len_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { crc_err(field_val); field_found=1; }
    if(!field_found) { field_found = phv_global.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_txdma_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { padding(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_rsv")) { txdma_rsv(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qtype")) { qtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_addr")) { qstate_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dma_cmd_ptr")) { dma_cmd_ptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { packet_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { recirc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { no_data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { frame_size(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { crypto_hdr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { parser_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { recirc_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { len_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { crc_err(field_val); field_found=1; }
    if(!field_found) { field_found = phv_global.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_rxdma_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { padding(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rxdma_rsv")) { rxdma_rsv(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rx_splitter_offset")) { rx_splitter_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qtype")) { qtype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qstate_addr")) { qstate_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dma_cmd_ptr")) { dma_cmd_ptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "packet_len")) { packet_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc")) { recirc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "no_data")) { no_data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "frame_size")) { frame_size(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crypto_hdr")) { crypto_hdr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parser_err")) { parser_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "recirc_count")) { recirc_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len_err")) { len_err(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "crc_err")) { crc_err(field_val); field_found=1; }
    if(!field_found) { field_found = phv_global.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_txicrc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { padding(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txicrc_rsv")) { txicrc_rsv(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "udp_opt_csum_vld")) { udp_opt_csum_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_udp_len")) { rdma_udp_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_pld_len")) { rdma_ip_pld_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_offset")) { rdma_ip_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rdma_ip_type")) { rdma_ip_type(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_udp_len")) { outer_udp_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_pld_len")) { outer_ip_pld_len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_offset")) { outer_ip_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_ip_type")) { outer_ip_type(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "outer_layer_vld")) { outer_layer_vld(field_val); field_found=1; }
    if(!field_found) { field_found = phv_global.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_phv_intr_global_pad_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "padding")) { padding(field_val); field_found=1; }
    if(!field_found) { field_found = phv_global.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_global_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tm_instance_type");
    ret_vec.push_back("error_bits");
    ret_vec.push_back("csum_err");
    ret_vec.push_back("debug_trace");
    ret_vec.push_back("tm_oq");
    ret_vec.push_back("hw_error");
    ret_vec.push_back("bypass");
    ret_vec.push_back("drop");
    ret_vec.push_back("tm_q_depth");
    ret_vec.push_back("tm_cpu");
    ret_vec.push_back("tm_replicate_en");
    ret_vec.push_back("tm_replicate_ptr");
    ret_vec.push_back("tm_span_session");
    ret_vec.push_back("timestamp");
    ret_vec.push_back("lif");
    ret_vec.push_back("tm_iq");
    ret_vec.push_back("tm_oport");
    ret_vec.push_back("tm_iport");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_p4_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hdr_vld");
    ret_vec.push_back("padding");
    ret_vec.push_back("packet_len");
    ret_vec.push_back("recirc");
    ret_vec.push_back("no_data");
    ret_vec.push_back("frame_size");
    ret_vec.push_back("crypto_hdr");
    ret_vec.push_back("parser_err");
    ret_vec.push_back("recirc_count");
    ret_vec.push_back("len_err");
    ret_vec.push_back("crc_err");
    {
        for(auto tmp_vec : phv_global.get_fields(level-1)) {
            ret_vec.push_back("phv_global." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_txdma_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("padding");
    ret_vec.push_back("txdma_rsv");
    ret_vec.push_back("qtype");
    ret_vec.push_back("qstate_addr");
    ret_vec.push_back("dma_cmd_ptr");
    ret_vec.push_back("qid");
    ret_vec.push_back("packet_len");
    ret_vec.push_back("recirc");
    ret_vec.push_back("no_data");
    ret_vec.push_back("frame_size");
    ret_vec.push_back("crypto_hdr");
    ret_vec.push_back("parser_err");
    ret_vec.push_back("recirc_count");
    ret_vec.push_back("len_err");
    ret_vec.push_back("crc_err");
    {
        for(auto tmp_vec : phv_global.get_fields(level-1)) {
            ret_vec.push_back("phv_global." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_rxdma_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("padding");
    ret_vec.push_back("rxdma_rsv");
    ret_vec.push_back("rx_splitter_offset");
    ret_vec.push_back("qtype");
    ret_vec.push_back("qstate_addr");
    ret_vec.push_back("dma_cmd_ptr");
    ret_vec.push_back("qid");
    ret_vec.push_back("packet_len");
    ret_vec.push_back("recirc");
    ret_vec.push_back("no_data");
    ret_vec.push_back("frame_size");
    ret_vec.push_back("crypto_hdr");
    ret_vec.push_back("parser_err");
    ret_vec.push_back("recirc_count");
    ret_vec.push_back("len_err");
    ret_vec.push_back("crc_err");
    {
        for(auto tmp_vec : phv_global.get_fields(level-1)) {
            ret_vec.push_back("phv_global." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_txicrc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("padding");
    ret_vec.push_back("txicrc_rsv");
    ret_vec.push_back("udp_opt_csum_vld");
    ret_vec.push_back("rdma_udp_len");
    ret_vec.push_back("rdma_ip_pld_len");
    ret_vec.push_back("rdma_ip_offset");
    ret_vec.push_back("rdma_ip_type");
    ret_vec.push_back("outer_udp_len");
    ret_vec.push_back("outer_ip_pld_len");
    ret_vec.push_back("outer_ip_offset");
    ret_vec.push_back("outer_ip_type");
    ret_vec.push_back("outer_layer_vld");
    {
        for(auto tmp_vec : phv_global.get_fields(level-1)) {
            ret_vec.push_back("phv_global." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_phv_intr_global_pad_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("padding");
    {
        for(auto tmp_vec : phv_global.get_fields(level-1)) {
            ret_vec.push_back("phv_global." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
