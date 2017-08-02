
#include "cap_dpp_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dpp_csr_cfg_ohi_payload_t::cap_dpp_csr_cfg_ohi_payload_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_ohi_payload_t::~cap_dpp_csr_cfg_ohi_payload_t() { }

cap_dpp_csr_cfg_interrupt_mask_t::cap_dpp_csr_cfg_interrupt_mask_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_interrupt_mask_t::~cap_dpp_csr_cfg_interrupt_mask_t() { }

cap_dpp_csr_cfg_error_mask_t::cap_dpp_csr_cfg_error_mask_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_error_mask_t::~cap_dpp_csr_cfg_error_mask_t() { }

cap_dpp_csr_cfg_global_2_t::cap_dpp_csr_cfg_global_2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_global_2_t::~cap_dpp_csr_cfg_global_2_t() { }

cap_dpp_csr_cfg_global_1_t::cap_dpp_csr_cfg_global_1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_global_1_t::~cap_dpp_csr_cfg_global_1_t() { }

cap_dpp_csr_cfg_global_t::cap_dpp_csr_cfg_global_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_cfg_global_t::~cap_dpp_csr_cfg_global_t() { }

cap_dpp_csr_base_t::cap_dpp_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpp_csr_base_t::~cap_dpp_csr_base_t() { }

cap_dpp_csr_t::cap_dpp_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(524288);
        set_attributes(0,get_name(), 0);
        }
cap_dpp_csr_t::~cap_dpp_csr_t() { }

void cap_dpp_csr_cfg_ohi_payload_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_slot_payload_ptr_bm: 0x" << int_var__ohi_slot_payload_ptr_bm << dec << endl)
}

void cap_dpp_csr_cfg_interrupt_mask_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_sop_no_eop: 0x" << int_var__err_phv_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_eop_no_sop: 0x" << int_var__err_phv_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_sop_no_eop: 0x" << int_var__err_ohi_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_eop_no_sop: 0x" << int_var__err_ohi_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_framer_credit_overrun: 0x" << int_var__err_framer_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_packets_in_flight_credit_overrun: 0x" << int_var__err_packets_in_flight_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_null_hdr_vld: 0x" << int_var__err_null_hdr_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_null_hdrfld_vld: 0x" << int_var__err_null_hdrfld_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_max_pkt_size: 0x" << int_var__err_max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_max_active_hdrs: 0x" << int_var__err_max_active_hdrs << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_no_data_reference_ohi: 0x" << int_var__err_phv_no_data_reference_ohi << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_multiple_hdr: 0x" << int_var__err_csum_multiple_hdr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_multiple_hdr: 0x" << int_var__err_crc_multiple_hdr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ptr_fifo_credit_overrun: 0x" << int_var__err_ptr_fifo_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_clip_max_pkt_size: 0x" << int_var__err_clip_max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpp_csr_cfg_error_mask_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_sop_no_eop: 0x" << int_var__err_phv_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_eop_no_sop: 0x" << int_var__err_phv_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_sop_no_eop: 0x" << int_var__err_ohi_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_eop_no_sop: 0x" << int_var__err_ohi_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_framer_credit_overrun: 0x" << int_var__err_framer_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_packets_in_flight_credit_overrun: 0x" << int_var__err_packets_in_flight_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_null_hdr_vld: 0x" << int_var__err_null_hdr_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_null_hdrfld_vld: 0x" << int_var__err_null_hdrfld_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_max_pkt_size: 0x" << int_var__err_max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_max_active_hdrs: 0x" << int_var__err_max_active_hdrs << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_no_data_reference_ohi: 0x" << int_var__err_phv_no_data_reference_ohi << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_multiple_hdr: 0x" << int_var__err_csum_multiple_hdr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_multiple_hdr: 0x" << int_var__err_crc_multiple_hdr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ptr_fifo_credit_overrun: 0x" << int_var__err_ptr_fifo_credit_overrun << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_clip_max_pkt_size: 0x" << int_var__err_clip_max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpp_csr_cfg_global_2_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".dump_dop_no_data_phv: 0x" << int_var__dump_dop_no_data_phv << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpp_csr_cfg_global_1_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".ptr_fifo_fc_thr: 0x" << int_var__ptr_fifo_fc_thr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_fifo_almost_full_threshold: 0x" << int_var__phv_fifo_almost_full_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_fifo_almost_full_threshold: 0x" << int_var__ohi_fifo_almost_full_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hdrfld_size: 0x" << int_var__max_hdrfld_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".max_pkt_size: 0x" << int_var__max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_truncation_en: 0x" << int_var__pkt_truncation_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ignore_hdrfld_size_0: 0x" << int_var__ignore_hdrfld_size_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".bypass_mode: 0x" << int_var__bypass_mode << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_drop_bypass_en: 0x" << int_var__phv_drop_bypass_en << dec << endl)
}

void cap_dpp_csr_cfg_global_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".hdr_vld_phv_mask: 0x" << int_var__hdr_vld_phv_mask << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".input_pacer_timer: 0x" << int_var__input_pacer_timer << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".input_pacer_release: 0x" << int_var__input_pacer_release << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".packet_in_flight_credit: 0x" << int_var__packet_in_flight_credit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".packet_in_flight_credit_en: 0x" << int_var__packet_in_flight_credit_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_framer_credit: 0x" << int_var__phv_framer_credit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_framer_credit_en: 0x" << int_var__phv_framer_credit_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".framer_ptr_fifo_credit: 0x" << int_var__framer_ptr_fifo_credit << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".framer_ptr_fifo_credit_en: 0x" << int_var__framer_ptr_fifo_credit_en << dec << endl)
}

void cap_dpp_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_dpp_csr_t::show() {

    base.show();
    cfg_global.show();
    cfg_global_1.show();
    cfg_global_2.show();
    cfg_error_mask.show();
    cfg_interrupt_mask.show();
    cfg_ohi_payload.show();
    hdr.show();
    hdrfld.show();
    csum.show();
    stats.show();
    mem.show();
}

int cap_dpp_csr_cfg_ohi_payload_t::get_width() const {
    return cap_dpp_csr_cfg_ohi_payload_t::s_get_width();

}

int cap_dpp_csr_cfg_interrupt_mask_t::get_width() const {
    return cap_dpp_csr_cfg_interrupt_mask_t::s_get_width();

}

int cap_dpp_csr_cfg_error_mask_t::get_width() const {
    return cap_dpp_csr_cfg_error_mask_t::s_get_width();

}

int cap_dpp_csr_cfg_global_2_t::get_width() const {
    return cap_dpp_csr_cfg_global_2_t::s_get_width();

}

int cap_dpp_csr_cfg_global_1_t::get_width() const {
    return cap_dpp_csr_cfg_global_1_t::s_get_width();

}

int cap_dpp_csr_cfg_global_t::get_width() const {
    return cap_dpp_csr_cfg_global_t::s_get_width();

}

int cap_dpp_csr_base_t::get_width() const {
    return cap_dpp_csr_base_t::s_get_width();

}

int cap_dpp_csr_t::get_width() const {
    return cap_dpp_csr_t::s_get_width();

}

int cap_dpp_csr_cfg_ohi_payload_t::s_get_width() {
    int _count = 0;

    _count += 64; // ohi_slot_payload_ptr_bm
    return _count;
}

int cap_dpp_csr_cfg_interrupt_mask_t::s_get_width() {
    int _count = 0;

    _count += 1; // err_phv_sop_no_eop
    _count += 1; // err_phv_eop_no_sop
    _count += 1; // err_ohi_sop_no_eop
    _count += 1; // err_ohi_eop_no_sop
    _count += 1; // err_framer_credit_overrun
    _count += 1; // err_packets_in_flight_credit_overrun
    _count += 1; // err_null_hdr_vld
    _count += 1; // err_null_hdrfld_vld
    _count += 1; // err_max_pkt_size
    _count += 1; // err_max_active_hdrs
    _count += 1; // err_phv_no_data_reference_ohi
    _count += 1; // err_csum_multiple_hdr
    _count += 1; // err_crc_multiple_hdr
    _count += 1; // err_ptr_fifo_credit_overrun
    _count += 1; // err_clip_max_pkt_size
    _count += 32; // rsvd
    return _count;
}

int cap_dpp_csr_cfg_error_mask_t::s_get_width() {
    int _count = 0;

    _count += 1; // err_phv_sop_no_eop
    _count += 1; // err_phv_eop_no_sop
    _count += 1; // err_ohi_sop_no_eop
    _count += 1; // err_ohi_eop_no_sop
    _count += 1; // err_framer_credit_overrun
    _count += 1; // err_packets_in_flight_credit_overrun
    _count += 1; // err_null_hdr_vld
    _count += 1; // err_null_hdrfld_vld
    _count += 1; // err_max_pkt_size
    _count += 1; // err_max_active_hdrs
    _count += 1; // err_phv_no_data_reference_ohi
    _count += 1; // err_csum_multiple_hdr
    _count += 1; // err_crc_multiple_hdr
    _count += 1; // err_ptr_fifo_credit_overrun
    _count += 1; // err_clip_max_pkt_size
    _count += 32; // rsvd
    return _count;
}

int cap_dpp_csr_cfg_global_2_t::s_get_width() {
    int _count = 0;

    _count += 1; // dump_dop_no_data_phv
    _count += 31; // rsvd
    return _count;
}

int cap_dpp_csr_cfg_global_1_t::s_get_width() {
    int _count = 0;

    _count += 5; // ptr_fifo_fc_thr
    _count += 10; // phv_fifo_almost_full_threshold
    _count += 10; // ohi_fifo_almost_full_threshold
    _count += 16; // max_hdrfld_size
    _count += 16; // max_pkt_size
    _count += 1; // pkt_truncation_en
    _count += 16; // ignore_hdrfld_size_0
    _count += 1; // bypass_mode
    _count += 1; // phv_drop_bypass_en
    return _count;
}

int cap_dpp_csr_cfg_global_t::s_get_width() {
    int _count = 0;

    _count += 7; // hdr_vld_phv_mask
    _count += 4; // input_pacer_timer
    _count += 4; // input_pacer_release
    _count += 3; // packet_in_flight_credit
    _count += 1; // packet_in_flight_credit_en
    _count += 2; // phv_framer_credit
    _count += 1; // phv_framer_credit_en
    _count += 5; // framer_ptr_fifo_credit
    _count += 1; // framer_ptr_fifo_credit_en
    return _count;
}

int cap_dpp_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_dpp_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_dpp_csr_base_t::s_get_width(); // base
    _count += cap_dpp_csr_cfg_global_t::s_get_width(); // cfg_global
    _count += cap_dpp_csr_cfg_global_1_t::s_get_width(); // cfg_global_1
    _count += cap_dpp_csr_cfg_global_2_t::s_get_width(); // cfg_global_2
    _count += cap_dpp_csr_cfg_error_mask_t::s_get_width(); // cfg_error_mask
    _count += cap_dpp_csr_cfg_interrupt_mask_t::s_get_width(); // cfg_interrupt_mask
    _count += cap_dpp_csr_cfg_ohi_payload_t::s_get_width(); // cfg_ohi_payload
    _count += cap_dpphdr_csr_t::s_get_width(); // hdr
    _count += cap_dpphdrfld_csr_t::s_get_width(); // hdrfld
    _count += cap_dppcsum_csr_t::s_get_width(); // csum
    _count += cap_dppstats_csr_t::s_get_width(); // stats
    _count += cap_dppmem_csr_t::s_get_width(); // mem
    return _count;
}

void cap_dpp_csr_cfg_ohi_payload_t::all(const cpp_int & _val) {
    int _count = 0;

    // ohi_slot_payload_ptr_bm
    int_var__ohi_slot_payload_ptr_bm = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< ohi_slot_payload_ptr_bm_cpp_int_t >()  ;
    _count += 64;
}

void cap_dpp_csr_cfg_interrupt_mask_t::all(const cpp_int & _val) {
    int _count = 0;

    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ohi_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ohi_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_framer_credit_overrun
    int_var__err_framer_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_framer_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_packets_in_flight_credit_overrun
    int_var__err_packets_in_flight_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_packets_in_flight_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_null_hdr_vld
    int_var__err_null_hdr_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_null_hdr_vld_cpp_int_t >()  ;
    _count += 1;
    // err_null_hdrfld_vld
    int_var__err_null_hdrfld_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_null_hdrfld_vld_cpp_int_t >()  ;
    _count += 1;
    // err_max_pkt_size
    int_var__err_max_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_max_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_max_active_hdrs
    int_var__err_max_active_hdrs = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_max_active_hdrs_cpp_int_t >()  ;
    _count += 1;
    // err_phv_no_data_reference_ohi
    int_var__err_phv_no_data_reference_ohi = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_no_data_reference_ohi_cpp_int_t >()  ;
    _count += 1;
    // err_csum_multiple_hdr
    int_var__err_csum_multiple_hdr = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_multiple_hdr_cpp_int_t >()  ;
    _count += 1;
    // err_crc_multiple_hdr
    int_var__err_crc_multiple_hdr = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_multiple_hdr_cpp_int_t >()  ;
    _count += 1;
    // err_ptr_fifo_credit_overrun
    int_var__err_ptr_fifo_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ptr_fifo_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_clip_max_pkt_size
    int_var__err_clip_max_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_clip_max_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpp_csr_cfg_error_mask_t::all(const cpp_int & _val) {
    int _count = 0;

    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ohi_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ohi_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_framer_credit_overrun
    int_var__err_framer_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_framer_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_packets_in_flight_credit_overrun
    int_var__err_packets_in_flight_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_packets_in_flight_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_null_hdr_vld
    int_var__err_null_hdr_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_null_hdr_vld_cpp_int_t >()  ;
    _count += 1;
    // err_null_hdrfld_vld
    int_var__err_null_hdrfld_vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_null_hdrfld_vld_cpp_int_t >()  ;
    _count += 1;
    // err_max_pkt_size
    int_var__err_max_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_max_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_max_active_hdrs
    int_var__err_max_active_hdrs = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_max_active_hdrs_cpp_int_t >()  ;
    _count += 1;
    // err_phv_no_data_reference_ohi
    int_var__err_phv_no_data_reference_ohi = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_phv_no_data_reference_ohi_cpp_int_t >()  ;
    _count += 1;
    // err_csum_multiple_hdr
    int_var__err_csum_multiple_hdr = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_multiple_hdr_cpp_int_t >()  ;
    _count += 1;
    // err_crc_multiple_hdr
    int_var__err_crc_multiple_hdr = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_multiple_hdr_cpp_int_t >()  ;
    _count += 1;
    // err_ptr_fifo_credit_overrun
    int_var__err_ptr_fifo_credit_overrun = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ptr_fifo_credit_overrun_cpp_int_t >()  ;
    _count += 1;
    // err_clip_max_pkt_size
    int_var__err_clip_max_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_clip_max_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpp_csr_cfg_global_2_t::all(const cpp_int & _val) {
    int _count = 0;

    // dump_dop_no_data_phv
    int_var__dump_dop_no_data_phv = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< dump_dop_no_data_phv_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 31 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 31;
}

void cap_dpp_csr_cfg_global_1_t::all(const cpp_int & _val) {
    int _count = 0;

    // ptr_fifo_fc_thr
    int_var__ptr_fifo_fc_thr = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< ptr_fifo_fc_thr_cpp_int_t >()  ;
    _count += 5;
    // phv_fifo_almost_full_threshold
    int_var__phv_fifo_almost_full_threshold = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< phv_fifo_almost_full_threshold_cpp_int_t >()  ;
    _count += 10;
    // ohi_fifo_almost_full_threshold
    int_var__ohi_fifo_almost_full_threshold = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< ohi_fifo_almost_full_threshold_cpp_int_t >()  ;
    _count += 10;
    // max_hdrfld_size
    int_var__max_hdrfld_size = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< max_hdrfld_size_cpp_int_t >()  ;
    _count += 16;
    // max_pkt_size
    int_var__max_pkt_size = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< max_pkt_size_cpp_int_t >()  ;
    _count += 16;
    // pkt_truncation_en
    int_var__pkt_truncation_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pkt_truncation_en_cpp_int_t >()  ;
    _count += 1;
    // ignore_hdrfld_size_0
    int_var__ignore_hdrfld_size_0 = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< ignore_hdrfld_size_0_cpp_int_t >()  ;
    _count += 16;
    // bypass_mode
    int_var__bypass_mode = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< bypass_mode_cpp_int_t >()  ;
    _count += 1;
    // phv_drop_bypass_en
    int_var__phv_drop_bypass_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< phv_drop_bypass_en_cpp_int_t >()  ;
    _count += 1;
}

void cap_dpp_csr_cfg_global_t::all(const cpp_int & _val) {
    int _count = 0;

    // hdr_vld_phv_mask
    int_var__hdr_vld_phv_mask = hlp.get_slc(_val, _count, _count -1 + 7 ).convert_to< hdr_vld_phv_mask_cpp_int_t >()  ;
    _count += 7;
    // input_pacer_timer
    int_var__input_pacer_timer = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< input_pacer_timer_cpp_int_t >()  ;
    _count += 4;
    // input_pacer_release
    int_var__input_pacer_release = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< input_pacer_release_cpp_int_t >()  ;
    _count += 4;
    // packet_in_flight_credit
    int_var__packet_in_flight_credit = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< packet_in_flight_credit_cpp_int_t >()  ;
    _count += 3;
    // packet_in_flight_credit_en
    int_var__packet_in_flight_credit_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< packet_in_flight_credit_en_cpp_int_t >()  ;
    _count += 1;
    // phv_framer_credit
    int_var__phv_framer_credit = hlp.get_slc(_val, _count, _count -1 + 2 ).convert_to< phv_framer_credit_cpp_int_t >()  ;
    _count += 2;
    // phv_framer_credit_en
    int_var__phv_framer_credit_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< phv_framer_credit_en_cpp_int_t >()  ;
    _count += 1;
    // framer_ptr_fifo_credit
    int_var__framer_ptr_fifo_credit = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< framer_ptr_fifo_credit_cpp_int_t >()  ;
    _count += 5;
    // framer_ptr_fifo_credit_en
    int_var__framer_ptr_fifo_credit_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< framer_ptr_fifo_credit_en_cpp_int_t >()  ;
    _count += 1;
}

void cap_dpp_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpp_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_global.all( hlp.get_slc(_val, _count, _count -1 + cfg_global.get_width() )); // cfg_global
    _count += cfg_global.get_width();
    cfg_global_1.all( hlp.get_slc(_val, _count, _count -1 + cfg_global_1.get_width() )); // cfg_global_1
    _count += cfg_global_1.get_width();
    cfg_global_2.all( hlp.get_slc(_val, _count, _count -1 + cfg_global_2.get_width() )); // cfg_global_2
    _count += cfg_global_2.get_width();
    cfg_error_mask.all( hlp.get_slc(_val, _count, _count -1 + cfg_error_mask.get_width() )); // cfg_error_mask
    _count += cfg_error_mask.get_width();
    cfg_interrupt_mask.all( hlp.get_slc(_val, _count, _count -1 + cfg_interrupt_mask.get_width() )); // cfg_interrupt_mask
    _count += cfg_interrupt_mask.get_width();
    cfg_ohi_payload.all( hlp.get_slc(_val, _count, _count -1 + cfg_ohi_payload.get_width() )); // cfg_ohi_payload
    _count += cfg_ohi_payload.get_width();
    hdr.all( hlp.get_slc(_val, _count, _count -1 + hdr.get_width() )); // hdr
    _count += hdr.get_width();
    hdrfld.all( hlp.get_slc(_val, _count, _count -1 + hdrfld.get_width() )); // hdrfld
    _count += hdrfld.get_width();
    csum.all( hlp.get_slc(_val, _count, _count -1 + csum.get_width() )); // csum
    _count += csum.get_width();
    stats.all( hlp.get_slc(_val, _count, _count -1 + stats.get_width() )); // stats
    _count += stats.get_width();
    mem.all( hlp.get_slc(_val, _count, _count -1 + mem.get_width() )); // mem
    _count += mem.get_width();
}

cpp_int cap_dpp_csr_cfg_ohi_payload_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // ohi_slot_payload_ptr_bm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ohi_slot_payload_ptr_bm) , _count, _count -1 + 64 );
    _count += 64;
    return ret_val;
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // err_phv_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_phv_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_ohi_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ohi_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_ohi_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ohi_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_framer_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_framer_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_packets_in_flight_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_packets_in_flight_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_null_hdr_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_null_hdr_vld) , _count, _count -1 + 1 );
    _count += 1;
    // err_null_hdrfld_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_null_hdrfld_vld) , _count, _count -1 + 1 );
    _count += 1;
    // err_max_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_max_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_max_active_hdrs
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_max_active_hdrs) , _count, _count -1 + 1 );
    _count += 1;
    // err_phv_no_data_reference_ohi
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_no_data_reference_ohi) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_multiple_hdr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_multiple_hdr) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_multiple_hdr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_multiple_hdr) , _count, _count -1 + 1 );
    _count += 1;
    // err_ptr_fifo_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ptr_fifo_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_clip_max_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_clip_max_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpp_csr_cfg_error_mask_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // err_phv_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_phv_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_ohi_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ohi_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_ohi_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ohi_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_framer_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_framer_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_packets_in_flight_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_packets_in_flight_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_null_hdr_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_null_hdr_vld) , _count, _count -1 + 1 );
    _count += 1;
    // err_null_hdrfld_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_null_hdrfld_vld) , _count, _count -1 + 1 );
    _count += 1;
    // err_max_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_max_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_max_active_hdrs
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_max_active_hdrs) , _count, _count -1 + 1 );
    _count += 1;
    // err_phv_no_data_reference_ohi
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_phv_no_data_reference_ohi) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_multiple_hdr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_multiple_hdr) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_multiple_hdr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_multiple_hdr) , _count, _count -1 + 1 );
    _count += 1;
    // err_ptr_fifo_credit_overrun
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ptr_fifo_credit_overrun) , _count, _count -1 + 1 );
    _count += 1;
    // err_clip_max_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_clip_max_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpp_csr_cfg_global_2_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // dump_dop_no_data_phv
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dump_dop_no_data_phv) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 31 );
    _count += 31;
    return ret_val;
}

cpp_int cap_dpp_csr_cfg_global_1_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // ptr_fifo_fc_thr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ptr_fifo_fc_thr) , _count, _count -1 + 5 );
    _count += 5;
    // phv_fifo_almost_full_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_fifo_almost_full_threshold) , _count, _count -1 + 10 );
    _count += 10;
    // ohi_fifo_almost_full_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ohi_fifo_almost_full_threshold) , _count, _count -1 + 10 );
    _count += 10;
    // max_hdrfld_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_hdrfld_size) , _count, _count -1 + 16 );
    _count += 16;
    // max_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_pkt_size) , _count, _count -1 + 16 );
    _count += 16;
    // pkt_truncation_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_truncation_en) , _count, _count -1 + 1 );
    _count += 1;
    // ignore_hdrfld_size_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ignore_hdrfld_size_0) , _count, _count -1 + 16 );
    _count += 16;
    // bypass_mode
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__bypass_mode) , _count, _count -1 + 1 );
    _count += 1;
    // phv_drop_bypass_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_drop_bypass_en) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dpp_csr_cfg_global_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // hdr_vld_phv_mask
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__hdr_vld_phv_mask) , _count, _count -1 + 7 );
    _count += 7;
    // input_pacer_timer
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__input_pacer_timer) , _count, _count -1 + 4 );
    _count += 4;
    // input_pacer_release
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__input_pacer_release) , _count, _count -1 + 4 );
    _count += 4;
    // packet_in_flight_credit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__packet_in_flight_credit) , _count, _count -1 + 3 );
    _count += 3;
    // packet_in_flight_credit_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__packet_in_flight_credit_en) , _count, _count -1 + 1 );
    _count += 1;
    // phv_framer_credit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_framer_credit) , _count, _count -1 + 2 );
    _count += 2;
    // phv_framer_credit_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_framer_credit_en) , _count, _count -1 + 1 );
    _count += 1;
    // framer_ptr_fifo_credit
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__framer_ptr_fifo_credit) , _count, _count -1 + 5 );
    _count += 5;
    // framer_ptr_fifo_credit_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__framer_ptr_fifo_credit_en) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dpp_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpp_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_global.all() , _count, _count -1 + cfg_global.get_width() ); // cfg_global
    _count += cfg_global.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_global_1.all() , _count, _count -1 + cfg_global_1.get_width() ); // cfg_global_1
    _count += cfg_global_1.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_global_2.all() , _count, _count -1 + cfg_global_2.get_width() ); // cfg_global_2
    _count += cfg_global_2.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_error_mask.all() , _count, _count -1 + cfg_error_mask.get_width() ); // cfg_error_mask
    _count += cfg_error_mask.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_interrupt_mask.all() , _count, _count -1 + cfg_interrupt_mask.get_width() ); // cfg_interrupt_mask
    _count += cfg_interrupt_mask.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_ohi_payload.all() , _count, _count -1 + cfg_ohi_payload.get_width() ); // cfg_ohi_payload
    _count += cfg_ohi_payload.get_width();
    ret_val = hlp.set_slc(ret_val, hdr.all() , _count, _count -1 + hdr.get_width() ); // hdr
    _count += hdr.get_width();
    ret_val = hlp.set_slc(ret_val, hdrfld.all() , _count, _count -1 + hdrfld.get_width() ); // hdrfld
    _count += hdrfld.get_width();
    ret_val = hlp.set_slc(ret_val, csum.all() , _count, _count -1 + csum.get_width() ); // csum
    _count += csum.get_width();
    ret_val = hlp.set_slc(ret_val, stats.all() , _count, _count -1 + stats.get_width() ); // stats
    _count += stats.get_width();
    ret_val = hlp.set_slc(ret_val, mem.all() , _count, _count -1 + mem.get_width() ); // mem
    _count += mem.get_width();
    return ret_val;
}

void cap_dpp_csr_cfg_ohi_payload_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ohi_slot_payload_ptr_bm", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_ohi_payload_t::ohi_slot_payload_ptr_bm);
            register_get_func("ohi_slot_payload_ptr_bm", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_ohi_payload_t::ohi_slot_payload_ptr_bm);
        }
        #endif
    
}

void cap_dpp_csr_cfg_interrupt_mask_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop);
            register_get_func("err_phv_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop);
            register_get_func("err_phv_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop);
            register_get_func("err_ohi_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop);
            register_get_func("err_ohi_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_framer_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_framer_credit_overrun);
            register_get_func("err_framer_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_framer_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_packets_in_flight_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_packets_in_flight_credit_overrun);
            register_get_func("err_packets_in_flight_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_packets_in_flight_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_null_hdr_vld", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdr_vld);
            register_get_func("err_null_hdr_vld", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdr_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_null_hdrfld_vld", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdrfld_vld);
            register_get_func("err_null_hdrfld_vld", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdrfld_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_max_pkt_size);
            register_get_func("err_max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_max_active_hdrs", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_max_active_hdrs);
            register_get_func("err_max_active_hdrs", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_max_active_hdrs);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_no_data_reference_ohi", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_no_data_reference_ohi);
            register_get_func("err_phv_no_data_reference_ohi", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_phv_no_data_reference_ohi);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_multiple_hdr", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_csum_multiple_hdr);
            register_get_func("err_csum_multiple_hdr", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_csum_multiple_hdr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_multiple_hdr", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_crc_multiple_hdr);
            register_get_func("err_crc_multiple_hdr", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_crc_multiple_hdr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ptr_fifo_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ptr_fifo_credit_overrun);
            register_get_func("err_ptr_fifo_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_ptr_fifo_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_clip_max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_clip_max_pkt_size);
            register_get_func("err_clip_max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::err_clip_max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_interrupt_mask_t::rsvd);
        }
        #endif
    
}

void cap_dpp_csr_cfg_error_mask_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_sop_no_eop);
            register_get_func("err_phv_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_eop_no_sop);
            register_get_func("err_phv_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ohi_sop_no_eop);
            register_get_func("err_ohi_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ohi_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ohi_eop_no_sop);
            register_get_func("err_ohi_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ohi_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_framer_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_framer_credit_overrun);
            register_get_func("err_framer_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_framer_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_packets_in_flight_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_packets_in_flight_credit_overrun);
            register_get_func("err_packets_in_flight_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_packets_in_flight_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_null_hdr_vld", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_null_hdr_vld);
            register_get_func("err_null_hdr_vld", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_null_hdr_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_null_hdrfld_vld", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_null_hdrfld_vld);
            register_get_func("err_null_hdrfld_vld", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_null_hdrfld_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_max_pkt_size);
            register_get_func("err_max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_max_active_hdrs", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_max_active_hdrs);
            register_get_func("err_max_active_hdrs", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_max_active_hdrs);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_no_data_reference_ohi", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_no_data_reference_ohi);
            register_get_func("err_phv_no_data_reference_ohi", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_phv_no_data_reference_ohi);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_multiple_hdr", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_csum_multiple_hdr);
            register_get_func("err_csum_multiple_hdr", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_csum_multiple_hdr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_multiple_hdr", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_crc_multiple_hdr);
            register_get_func("err_crc_multiple_hdr", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_crc_multiple_hdr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ptr_fifo_credit_overrun", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ptr_fifo_credit_overrun);
            register_get_func("err_ptr_fifo_credit_overrun", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_ptr_fifo_credit_overrun);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_clip_max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_clip_max_pkt_size);
            register_get_func("err_clip_max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::err_clip_max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_error_mask_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_error_mask_t::rsvd);
        }
        #endif
    
}

void cap_dpp_csr_cfg_global_2_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dump_dop_no_data_phv", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_2_t::dump_dop_no_data_phv);
            register_get_func("dump_dop_no_data_phv", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_2_t::dump_dop_no_data_phv);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_2_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_2_t::rsvd);
        }
        #endif
    
}

void cap_dpp_csr_cfg_global_1_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ptr_fifo_fc_thr", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::ptr_fifo_fc_thr);
            register_get_func("ptr_fifo_fc_thr", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::ptr_fifo_fc_thr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_fifo_almost_full_threshold", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::phv_fifo_almost_full_threshold);
            register_get_func("phv_fifo_almost_full_threshold", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::phv_fifo_almost_full_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ohi_fifo_almost_full_threshold", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::ohi_fifo_almost_full_threshold);
            register_get_func("ohi_fifo_almost_full_threshold", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::ohi_fifo_almost_full_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_hdrfld_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::max_hdrfld_size);
            register_get_func("max_hdrfld_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::max_hdrfld_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::max_pkt_size);
            register_get_func("max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_truncation_en", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::pkt_truncation_en);
            register_get_func("pkt_truncation_en", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::pkt_truncation_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ignore_hdrfld_size_0", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::ignore_hdrfld_size_0);
            register_get_func("ignore_hdrfld_size_0", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::ignore_hdrfld_size_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("bypass_mode", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::bypass_mode);
            register_get_func("bypass_mode", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::bypass_mode);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_drop_bypass_en", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_1_t::phv_drop_bypass_en);
            register_get_func("phv_drop_bypass_en", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_1_t::phv_drop_bypass_en);
        }
        #endif
    
    set_reset_val(cpp_int("0x89c404e204e2004011f"));
    all(get_reset_val());
}

void cap_dpp_csr_cfg_global_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("hdr_vld_phv_mask", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::hdr_vld_phv_mask);
            register_get_func("hdr_vld_phv_mask", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::hdr_vld_phv_mask);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("input_pacer_timer", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::input_pacer_timer);
            register_get_func("input_pacer_timer", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::input_pacer_timer);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("input_pacer_release", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::input_pacer_release);
            register_get_func("input_pacer_release", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::input_pacer_release);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("packet_in_flight_credit", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::packet_in_flight_credit);
            register_get_func("packet_in_flight_credit", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::packet_in_flight_credit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("packet_in_flight_credit_en", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::packet_in_flight_credit_en);
            register_get_func("packet_in_flight_credit_en", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::packet_in_flight_credit_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_framer_credit", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::phv_framer_credit);
            register_get_func("phv_framer_credit", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::phv_framer_credit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_framer_credit_en", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::phv_framer_credit_en);
            register_get_func("phv_framer_credit_en", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::phv_framer_credit_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("framer_ptr_fifo_credit", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit);
            register_get_func("framer_ptr_fifo_credit", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("framer_ptr_fifo_credit_en", (cap_csr_base::set_function_type_t)&cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit_en);
            register_get_func("framer_ptr_fifo_credit_en", (cap_csr_base::get_function_type_t)&cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit_en);
        }
        #endif
    
    set_reset_val(cpp_int("0xd371b80"));
    all(get_reset_val());
}

void cap_dpp_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_dpp_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_dpp_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_dpp_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_global.set_attributes(this,"cfg_global", 0x4 );
    cfg_global_1.set_attributes(this,"cfg_global_1", 0x10 );
    cfg_global_2.set_attributes(this,"cfg_global_2", 0x20 );
    cfg_error_mask.set_attributes(this,"cfg_error_mask", 0x28 );
    cfg_interrupt_mask.set_attributes(this,"cfg_interrupt_mask", 0x30 );
    cfg_ohi_payload.set_attributes(this,"cfg_ohi_payload", 0x38 );
    hdr.set_attributes(this,"hdr", 0x200 );
    hdrfld.set_attributes(this,"hdrfld", 0x400 );
    csum.set_attributes(this,"csum", 0x800 );
    stats.set_attributes(this,"stats", 0x4000 );
    mem.set_attributes(this,"mem", 0x40000 );
}

void cap_dpp_csr_cfg_ohi_payload_t::ohi_slot_payload_ptr_bm(const cpp_int & _val) { 
    // ohi_slot_payload_ptr_bm
    int_var__ohi_slot_payload_ptr_bm = _val.convert_to< ohi_slot_payload_ptr_bm_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_ohi_payload_t::ohi_slot_payload_ptr_bm() const {
    return int_var__ohi_slot_payload_ptr_bm.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop(const cpp_int & _val) { 
    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = _val.convert_to< err_phv_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop() const {
    return int_var__err_phv_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop(const cpp_int & _val) { 
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = _val.convert_to< err_phv_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop() const {
    return int_var__err_phv_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop(const cpp_int & _val) { 
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = _val.convert_to< err_ohi_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop() const {
    return int_var__err_ohi_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop(const cpp_int & _val) { 
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = _val.convert_to< err_ohi_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop() const {
    return int_var__err_ohi_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_framer_credit_overrun(const cpp_int & _val) { 
    // err_framer_credit_overrun
    int_var__err_framer_credit_overrun = _val.convert_to< err_framer_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_framer_credit_overrun() const {
    return int_var__err_framer_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_packets_in_flight_credit_overrun(const cpp_int & _val) { 
    // err_packets_in_flight_credit_overrun
    int_var__err_packets_in_flight_credit_overrun = _val.convert_to< err_packets_in_flight_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_packets_in_flight_credit_overrun() const {
    return int_var__err_packets_in_flight_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdr_vld(const cpp_int & _val) { 
    // err_null_hdr_vld
    int_var__err_null_hdr_vld = _val.convert_to< err_null_hdr_vld_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdr_vld() const {
    return int_var__err_null_hdr_vld.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdrfld_vld(const cpp_int & _val) { 
    // err_null_hdrfld_vld
    int_var__err_null_hdrfld_vld = _val.convert_to< err_null_hdrfld_vld_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_null_hdrfld_vld() const {
    return int_var__err_null_hdrfld_vld.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_max_pkt_size(const cpp_int & _val) { 
    // err_max_pkt_size
    int_var__err_max_pkt_size = _val.convert_to< err_max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_max_pkt_size() const {
    return int_var__err_max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_max_active_hdrs(const cpp_int & _val) { 
    // err_max_active_hdrs
    int_var__err_max_active_hdrs = _val.convert_to< err_max_active_hdrs_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_max_active_hdrs() const {
    return int_var__err_max_active_hdrs.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_phv_no_data_reference_ohi(const cpp_int & _val) { 
    // err_phv_no_data_reference_ohi
    int_var__err_phv_no_data_reference_ohi = _val.convert_to< err_phv_no_data_reference_ohi_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_phv_no_data_reference_ohi() const {
    return int_var__err_phv_no_data_reference_ohi.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_csum_multiple_hdr(const cpp_int & _val) { 
    // err_csum_multiple_hdr
    int_var__err_csum_multiple_hdr = _val.convert_to< err_csum_multiple_hdr_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_csum_multiple_hdr() const {
    return int_var__err_csum_multiple_hdr.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_crc_multiple_hdr(const cpp_int & _val) { 
    // err_crc_multiple_hdr
    int_var__err_crc_multiple_hdr = _val.convert_to< err_crc_multiple_hdr_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_crc_multiple_hdr() const {
    return int_var__err_crc_multiple_hdr.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_ptr_fifo_credit_overrun(const cpp_int & _val) { 
    // err_ptr_fifo_credit_overrun
    int_var__err_ptr_fifo_credit_overrun = _val.convert_to< err_ptr_fifo_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_ptr_fifo_credit_overrun() const {
    return int_var__err_ptr_fifo_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::err_clip_max_pkt_size(const cpp_int & _val) { 
    // err_clip_max_pkt_size
    int_var__err_clip_max_pkt_size = _val.convert_to< err_clip_max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::err_clip_max_pkt_size() const {
    return int_var__err_clip_max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_interrupt_mask_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_interrupt_mask_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_phv_sop_no_eop(const cpp_int & _val) { 
    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = _val.convert_to< err_phv_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_phv_sop_no_eop() const {
    return int_var__err_phv_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_phv_eop_no_sop(const cpp_int & _val) { 
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = _val.convert_to< err_phv_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_phv_eop_no_sop() const {
    return int_var__err_phv_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_ohi_sop_no_eop(const cpp_int & _val) { 
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = _val.convert_to< err_ohi_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_ohi_sop_no_eop() const {
    return int_var__err_ohi_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_ohi_eop_no_sop(const cpp_int & _val) { 
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = _val.convert_to< err_ohi_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_ohi_eop_no_sop() const {
    return int_var__err_ohi_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_framer_credit_overrun(const cpp_int & _val) { 
    // err_framer_credit_overrun
    int_var__err_framer_credit_overrun = _val.convert_to< err_framer_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_framer_credit_overrun() const {
    return int_var__err_framer_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_packets_in_flight_credit_overrun(const cpp_int & _val) { 
    // err_packets_in_flight_credit_overrun
    int_var__err_packets_in_flight_credit_overrun = _val.convert_to< err_packets_in_flight_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_packets_in_flight_credit_overrun() const {
    return int_var__err_packets_in_flight_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_null_hdr_vld(const cpp_int & _val) { 
    // err_null_hdr_vld
    int_var__err_null_hdr_vld = _val.convert_to< err_null_hdr_vld_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_null_hdr_vld() const {
    return int_var__err_null_hdr_vld.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_null_hdrfld_vld(const cpp_int & _val) { 
    // err_null_hdrfld_vld
    int_var__err_null_hdrfld_vld = _val.convert_to< err_null_hdrfld_vld_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_null_hdrfld_vld() const {
    return int_var__err_null_hdrfld_vld.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_max_pkt_size(const cpp_int & _val) { 
    // err_max_pkt_size
    int_var__err_max_pkt_size = _val.convert_to< err_max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_max_pkt_size() const {
    return int_var__err_max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_max_active_hdrs(const cpp_int & _val) { 
    // err_max_active_hdrs
    int_var__err_max_active_hdrs = _val.convert_to< err_max_active_hdrs_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_max_active_hdrs() const {
    return int_var__err_max_active_hdrs.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_phv_no_data_reference_ohi(const cpp_int & _val) { 
    // err_phv_no_data_reference_ohi
    int_var__err_phv_no_data_reference_ohi = _val.convert_to< err_phv_no_data_reference_ohi_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_phv_no_data_reference_ohi() const {
    return int_var__err_phv_no_data_reference_ohi.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_csum_multiple_hdr(const cpp_int & _val) { 
    // err_csum_multiple_hdr
    int_var__err_csum_multiple_hdr = _val.convert_to< err_csum_multiple_hdr_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_csum_multiple_hdr() const {
    return int_var__err_csum_multiple_hdr.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_crc_multiple_hdr(const cpp_int & _val) { 
    // err_crc_multiple_hdr
    int_var__err_crc_multiple_hdr = _val.convert_to< err_crc_multiple_hdr_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_crc_multiple_hdr() const {
    return int_var__err_crc_multiple_hdr.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_ptr_fifo_credit_overrun(const cpp_int & _val) { 
    // err_ptr_fifo_credit_overrun
    int_var__err_ptr_fifo_credit_overrun = _val.convert_to< err_ptr_fifo_credit_overrun_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_ptr_fifo_credit_overrun() const {
    return int_var__err_ptr_fifo_credit_overrun.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::err_clip_max_pkt_size(const cpp_int & _val) { 
    // err_clip_max_pkt_size
    int_var__err_clip_max_pkt_size = _val.convert_to< err_clip_max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::err_clip_max_pkt_size() const {
    return int_var__err_clip_max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_error_mask_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_error_mask_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_2_t::dump_dop_no_data_phv(const cpp_int & _val) { 
    // dump_dop_no_data_phv
    int_var__dump_dop_no_data_phv = _val.convert_to< dump_dop_no_data_phv_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_2_t::dump_dop_no_data_phv() const {
    return int_var__dump_dop_no_data_phv.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_2_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_2_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::ptr_fifo_fc_thr(const cpp_int & _val) { 
    // ptr_fifo_fc_thr
    int_var__ptr_fifo_fc_thr = _val.convert_to< ptr_fifo_fc_thr_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::ptr_fifo_fc_thr() const {
    return int_var__ptr_fifo_fc_thr.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::phv_fifo_almost_full_threshold(const cpp_int & _val) { 
    // phv_fifo_almost_full_threshold
    int_var__phv_fifo_almost_full_threshold = _val.convert_to< phv_fifo_almost_full_threshold_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::phv_fifo_almost_full_threshold() const {
    return int_var__phv_fifo_almost_full_threshold.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::ohi_fifo_almost_full_threshold(const cpp_int & _val) { 
    // ohi_fifo_almost_full_threshold
    int_var__ohi_fifo_almost_full_threshold = _val.convert_to< ohi_fifo_almost_full_threshold_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::ohi_fifo_almost_full_threshold() const {
    return int_var__ohi_fifo_almost_full_threshold.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::max_hdrfld_size(const cpp_int & _val) { 
    // max_hdrfld_size
    int_var__max_hdrfld_size = _val.convert_to< max_hdrfld_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::max_hdrfld_size() const {
    return int_var__max_hdrfld_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::max_pkt_size(const cpp_int & _val) { 
    // max_pkt_size
    int_var__max_pkt_size = _val.convert_to< max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::max_pkt_size() const {
    return int_var__max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::pkt_truncation_en(const cpp_int & _val) { 
    // pkt_truncation_en
    int_var__pkt_truncation_en = _val.convert_to< pkt_truncation_en_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::pkt_truncation_en() const {
    return int_var__pkt_truncation_en.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::ignore_hdrfld_size_0(const cpp_int & _val) { 
    // ignore_hdrfld_size_0
    int_var__ignore_hdrfld_size_0 = _val.convert_to< ignore_hdrfld_size_0_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::ignore_hdrfld_size_0() const {
    return int_var__ignore_hdrfld_size_0.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::bypass_mode(const cpp_int & _val) { 
    // bypass_mode
    int_var__bypass_mode = _val.convert_to< bypass_mode_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::bypass_mode() const {
    return int_var__bypass_mode.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_1_t::phv_drop_bypass_en(const cpp_int & _val) { 
    // phv_drop_bypass_en
    int_var__phv_drop_bypass_en = _val.convert_to< phv_drop_bypass_en_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_1_t::phv_drop_bypass_en() const {
    return int_var__phv_drop_bypass_en.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::hdr_vld_phv_mask(const cpp_int & _val) { 
    // hdr_vld_phv_mask
    int_var__hdr_vld_phv_mask = _val.convert_to< hdr_vld_phv_mask_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::hdr_vld_phv_mask() const {
    return int_var__hdr_vld_phv_mask.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::input_pacer_timer(const cpp_int & _val) { 
    // input_pacer_timer
    int_var__input_pacer_timer = _val.convert_to< input_pacer_timer_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::input_pacer_timer() const {
    return int_var__input_pacer_timer.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::input_pacer_release(const cpp_int & _val) { 
    // input_pacer_release
    int_var__input_pacer_release = _val.convert_to< input_pacer_release_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::input_pacer_release() const {
    return int_var__input_pacer_release.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::packet_in_flight_credit(const cpp_int & _val) { 
    // packet_in_flight_credit
    int_var__packet_in_flight_credit = _val.convert_to< packet_in_flight_credit_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::packet_in_flight_credit() const {
    return int_var__packet_in_flight_credit.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::packet_in_flight_credit_en(const cpp_int & _val) { 
    // packet_in_flight_credit_en
    int_var__packet_in_flight_credit_en = _val.convert_to< packet_in_flight_credit_en_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::packet_in_flight_credit_en() const {
    return int_var__packet_in_flight_credit_en.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::phv_framer_credit(const cpp_int & _val) { 
    // phv_framer_credit
    int_var__phv_framer_credit = _val.convert_to< phv_framer_credit_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::phv_framer_credit() const {
    return int_var__phv_framer_credit.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::phv_framer_credit_en(const cpp_int & _val) { 
    // phv_framer_credit_en
    int_var__phv_framer_credit_en = _val.convert_to< phv_framer_credit_en_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::phv_framer_credit_en() const {
    return int_var__phv_framer_credit_en.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit(const cpp_int & _val) { 
    // framer_ptr_fifo_credit
    int_var__framer_ptr_fifo_credit = _val.convert_to< framer_ptr_fifo_credit_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit() const {
    return int_var__framer_ptr_fifo_credit.convert_to< cpp_int >();
}
    
void cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit_en(const cpp_int & _val) { 
    // framer_ptr_fifo_credit_en
    int_var__framer_ptr_fifo_credit_en = _val.convert_to< framer_ptr_fifo_credit_en_cpp_int_t >();
}

cpp_int cap_dpp_csr_cfg_global_t::framer_ptr_fifo_credit_en() const {
    return int_var__framer_ptr_fifo_credit_en.convert_to< cpp_int >();
}
    
void cap_dpp_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_dpp_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    