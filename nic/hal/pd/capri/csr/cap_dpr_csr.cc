
#include "cap_dpr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dpr_csr_cfg_interrupt_mask_t::cap_dpr_csr_cfg_interrupt_mask_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_cfg_interrupt_mask_t::~cap_dpr_csr_cfg_interrupt_mask_t() { }

cap_dpr_csr_cfg_error_mask_t::cap_dpr_csr_cfg_error_mask_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_cfg_error_mask_t::~cap_dpr_csr_cfg_error_mask_t() { }

cap_dpr_csr_cfg_global_2_t::cap_dpr_csr_cfg_global_2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_cfg_global_2_t::~cap_dpr_csr_cfg_global_2_t() { }

cap_dpr_csr_cfg_global_1_t::cap_dpr_csr_cfg_global_1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_cfg_global_1_t::~cap_dpr_csr_cfg_global_1_t() { }

cap_dpr_csr_cfg_global_t::cap_dpr_csr_cfg_global_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_cfg_global_t::~cap_dpr_csr_cfg_global_t() { }

cap_dpr_csr_base_t::cap_dpr_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dpr_csr_base_t::~cap_dpr_csr_base_t() { }

cap_dpr_csr_t::cap_dpr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_dpr_csr_t::~cap_dpr_csr_t() { }

void cap_dpr_csr_cfg_interrupt_mask_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_sop_no_eop: 0x" << int_var__err_phv_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_eop_no_sop: 0x" << int_var__err_phv_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_sop_no_eop: 0x" << int_var__err_ohi_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_eop_no_sop: 0x" << int_var__err_ohi_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktin_sop_no_eop: 0x" << int_var__err_pktin_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktin_eop_no_sop: 0x" << int_var__err_pktin_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_offset_gt_pkt_size: 0x" << int_var__err_csum_offset_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_loc_gt_pkt_size: 0x" << int_var__err_csum_loc_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_offset_gt_pkt_size: 0x" << int_var__err_crc_offset_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_loc_gt_pkt_size: 0x" << int_var__err_crc_loc_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pkt_eop_early: 0x" << int_var__err_pkt_eop_early << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ptr_ff_overflow: 0x" << int_var__err_ptr_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_ff_overflow: 0x" << int_var__err_csum_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktout_ff_overflow: 0x" << int_var__err_pktout_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpr_csr_cfg_error_mask_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_sop_no_eop: 0x" << int_var__err_phv_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_phv_eop_no_sop: 0x" << int_var__err_phv_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_sop_no_eop: 0x" << int_var__err_ohi_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ohi_eop_no_sop: 0x" << int_var__err_ohi_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktin_sop_no_eop: 0x" << int_var__err_pktin_sop_no_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktin_eop_no_sop: 0x" << int_var__err_pktin_eop_no_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_offset_gt_pkt_size: 0x" << int_var__err_csum_offset_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_loc_gt_pkt_size: 0x" << int_var__err_csum_loc_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_offset_gt_pkt_size: 0x" << int_var__err_crc_offset_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_crc_loc_gt_pkt_size: 0x" << int_var__err_crc_loc_gt_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pkt_eop_early: 0x" << int_var__err_pkt_eop_early << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_ptr_ff_overflow: 0x" << int_var__err_ptr_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_csum_ff_overflow: 0x" << int_var__err_csum_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".err_pktout_ff_overflow: 0x" << int_var__err_pktout_ff_overflow << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpr_csr_cfg_global_2_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".dump_dop_no_data_phv: 0x" << int_var__dump_dop_no_data_phv << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dpr_csr_cfg_global_1_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".ptr_fifo_fc_thr: 0x" << int_var__ptr_fifo_fc_thr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".intrinsic_len_rw_en: 0x" << int_var__intrinsic_len_rw_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".intrinsic_drop_rw_en: 0x" << int_var__intrinsic_drop_rw_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".padding_en: 0x" << int_var__padding_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_min_size: 0x" << int_var__pkt_min_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".max_pkt_size: 0x" << int_var__max_pkt_size << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_truncation_en: 0x" << int_var__pkt_truncation_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ignore_hdrfld_size_0: 0x" << int_var__ignore_hdrfld_size_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".bypass_mode: 0x" << int_var__bypass_mode << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_drop_bypass_en: 0x" << int_var__phv_drop_bypass_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".no_csum_update_on_csum_err: 0x" << int_var__no_csum_update_on_csum_err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".no_crc_update_on_crc_err: 0x" << int_var__no_crc_update_on_crc_err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".no_sf: 0x" << int_var__no_sf << dec << endl)
}

void cap_dpr_csr_cfg_global_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".input_pacer_timer: 0x" << int_var__input_pacer_timer << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".input_pacer_release: 0x" << int_var__input_pacer_release << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pktout_fc_threshold: 0x" << int_var__pktout_fc_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".csum_ff_fc_threshold: 0x" << int_var__csum_ff_fc_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_fifo_almost_full_threshold: 0x" << int_var__phv_fifo_almost_full_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ohi_fifo_almost_full_threshold: 0x" << int_var__ohi_fifo_almost_full_threshold << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pktin_fifo_almost_full_threshold: 0x" << int_var__pktin_fifo_almost_full_threshold << dec << endl)
}

void cap_dpr_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_dpr_csr_t::show() {

    base.show();
    cfg_global.show();
    cfg_global_1.show();
    cfg_global_2.show();
    cfg_error_mask.show();
    cfg_interrupt_mask.show();
    hdrfld.show();
    cfg.show();
    stats.show();
    mem.show();
}

int cap_dpr_csr_cfg_interrupt_mask_t::get_width() const {
    return cap_dpr_csr_cfg_interrupt_mask_t::s_get_width();

}

int cap_dpr_csr_cfg_error_mask_t::get_width() const {
    return cap_dpr_csr_cfg_error_mask_t::s_get_width();

}

int cap_dpr_csr_cfg_global_2_t::get_width() const {
    return cap_dpr_csr_cfg_global_2_t::s_get_width();

}

int cap_dpr_csr_cfg_global_1_t::get_width() const {
    return cap_dpr_csr_cfg_global_1_t::s_get_width();

}

int cap_dpr_csr_cfg_global_t::get_width() const {
    return cap_dpr_csr_cfg_global_t::s_get_width();

}

int cap_dpr_csr_base_t::get_width() const {
    return cap_dpr_csr_base_t::s_get_width();

}

int cap_dpr_csr_t::get_width() const {
    return cap_dpr_csr_t::s_get_width();

}

int cap_dpr_csr_cfg_interrupt_mask_t::s_get_width() {
    int _count = 0;

    _count += 1; // err_phv_sop_no_eop
    _count += 1; // err_phv_eop_no_sop
    _count += 1; // err_ohi_sop_no_eop
    _count += 1; // err_ohi_eop_no_sop
    _count += 1; // err_pktin_sop_no_eop
    _count += 1; // err_pktin_eop_no_sop
    _count += 1; // err_csum_offset_gt_pkt_size
    _count += 1; // err_csum_loc_gt_pkt_size
    _count += 1; // err_crc_offset_gt_pkt_size
    _count += 1; // err_crc_loc_gt_pkt_size
    _count += 1; // err_pkt_eop_early
    _count += 1; // err_ptr_ff_overflow
    _count += 1; // err_csum_ff_overflow
    _count += 1; // err_pktout_ff_overflow
    _count += 32; // rsvd
    return _count;
}

int cap_dpr_csr_cfg_error_mask_t::s_get_width() {
    int _count = 0;

    _count += 1; // err_phv_sop_no_eop
    _count += 1; // err_phv_eop_no_sop
    _count += 1; // err_ohi_sop_no_eop
    _count += 1; // err_ohi_eop_no_sop
    _count += 1; // err_pktin_sop_no_eop
    _count += 1; // err_pktin_eop_no_sop
    _count += 1; // err_csum_offset_gt_pkt_size
    _count += 1; // err_csum_loc_gt_pkt_size
    _count += 1; // err_crc_offset_gt_pkt_size
    _count += 1; // err_crc_loc_gt_pkt_size
    _count += 1; // err_pkt_eop_early
    _count += 1; // err_ptr_ff_overflow
    _count += 1; // err_csum_ff_overflow
    _count += 1; // err_pktout_ff_overflow
    _count += 32; // rsvd
    return _count;
}

int cap_dpr_csr_cfg_global_2_t::s_get_width() {
    int _count = 0;

    _count += 1; // dump_dop_no_data_phv
    _count += 31; // rsvd
    return _count;
}

int cap_dpr_csr_cfg_global_1_t::s_get_width() {
    int _count = 0;

    _count += 5; // ptr_fifo_fc_thr
    _count += 1; // intrinsic_len_rw_en
    _count += 1; // intrinsic_drop_rw_en
    _count += 1; // padding_en
    _count += 16; // pkt_min_size
    _count += 16; // max_pkt_size
    _count += 1; // pkt_truncation_en
    _count += 16; // ignore_hdrfld_size_0
    _count += 1; // bypass_mode
    _count += 1; // phv_drop_bypass_en
    _count += 1; // no_csum_update_on_csum_err
    _count += 1; // no_crc_update_on_crc_err
    _count += 1; // no_sf
    return _count;
}

int cap_dpr_csr_cfg_global_t::s_get_width() {
    int _count = 0;

    _count += 4; // input_pacer_timer
    _count += 4; // input_pacer_release
    _count += 3; // pktout_fc_threshold
    _count += 10; // csum_ff_fc_threshold
    _count += 8; // phv_fifo_almost_full_threshold
    _count += 4; // ohi_fifo_almost_full_threshold
    _count += 10; // pktin_fifo_almost_full_threshold
    return _count;
}

int cap_dpr_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_dpr_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_dpr_csr_base_t::s_get_width(); // base
    _count += cap_dpr_csr_cfg_global_t::s_get_width(); // cfg_global
    _count += cap_dpr_csr_cfg_global_1_t::s_get_width(); // cfg_global_1
    _count += cap_dpr_csr_cfg_global_2_t::s_get_width(); // cfg_global_2
    _count += cap_dpr_csr_cfg_error_mask_t::s_get_width(); // cfg_error_mask
    _count += cap_dpr_csr_cfg_interrupt_mask_t::s_get_width(); // cfg_interrupt_mask
    _count += cap_dprhdrfld_csr_t::s_get_width(); // hdrfld
    _count += cap_dprcfg_csr_t::s_get_width(); // cfg
    _count += cap_dprstats_csr_t::s_get_width(); // stats
    _count += cap_dprmem_csr_t::s_get_width(); // mem
    return _count;
}

void cap_dpr_csr_cfg_interrupt_mask_t::all(const cpp_int & _val) {
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
    // err_pktin_sop_no_eop
    int_var__err_pktin_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktin_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_pktin_eop_no_sop
    int_var__err_pktin_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktin_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_csum_offset_gt_pkt_size
    int_var__err_csum_offset_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_offset_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_csum_loc_gt_pkt_size
    int_var__err_csum_loc_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_loc_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_crc_offset_gt_pkt_size
    int_var__err_crc_offset_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_offset_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_crc_loc_gt_pkt_size
    int_var__err_crc_loc_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_loc_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_pkt_eop_early
    int_var__err_pkt_eop_early = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pkt_eop_early_cpp_int_t >()  ;
    _count += 1;
    // err_ptr_ff_overflow
    int_var__err_ptr_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ptr_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // err_csum_ff_overflow
    int_var__err_csum_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // err_pktout_ff_overflow
    int_var__err_pktout_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktout_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpr_csr_cfg_error_mask_t::all(const cpp_int & _val) {
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
    // err_pktin_sop_no_eop
    int_var__err_pktin_sop_no_eop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktin_sop_no_eop_cpp_int_t >()  ;
    _count += 1;
    // err_pktin_eop_no_sop
    int_var__err_pktin_eop_no_sop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktin_eop_no_sop_cpp_int_t >()  ;
    _count += 1;
    // err_csum_offset_gt_pkt_size
    int_var__err_csum_offset_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_offset_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_csum_loc_gt_pkt_size
    int_var__err_csum_loc_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_loc_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_crc_offset_gt_pkt_size
    int_var__err_crc_offset_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_offset_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_crc_loc_gt_pkt_size
    int_var__err_crc_loc_gt_pkt_size = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_crc_loc_gt_pkt_size_cpp_int_t >()  ;
    _count += 1;
    // err_pkt_eop_early
    int_var__err_pkt_eop_early = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pkt_eop_early_cpp_int_t >()  ;
    _count += 1;
    // err_ptr_ff_overflow
    int_var__err_ptr_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_ptr_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // err_csum_ff_overflow
    int_var__err_csum_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_csum_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // err_pktout_ff_overflow
    int_var__err_pktout_ff_overflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< err_pktout_ff_overflow_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpr_csr_cfg_global_2_t::all(const cpp_int & _val) {
    int _count = 0;

    // dump_dop_no_data_phv
    int_var__dump_dop_no_data_phv = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< dump_dop_no_data_phv_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 31 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 31;
}

void cap_dpr_csr_cfg_global_1_t::all(const cpp_int & _val) {
    int _count = 0;

    // ptr_fifo_fc_thr
    int_var__ptr_fifo_fc_thr = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< ptr_fifo_fc_thr_cpp_int_t >()  ;
    _count += 5;
    // intrinsic_len_rw_en
    int_var__intrinsic_len_rw_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< intrinsic_len_rw_en_cpp_int_t >()  ;
    _count += 1;
    // intrinsic_drop_rw_en
    int_var__intrinsic_drop_rw_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< intrinsic_drop_rw_en_cpp_int_t >()  ;
    _count += 1;
    // padding_en
    int_var__padding_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< padding_en_cpp_int_t >()  ;
    _count += 1;
    // pkt_min_size
    int_var__pkt_min_size = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< pkt_min_size_cpp_int_t >()  ;
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
    // no_csum_update_on_csum_err
    int_var__no_csum_update_on_csum_err = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< no_csum_update_on_csum_err_cpp_int_t >()  ;
    _count += 1;
    // no_crc_update_on_crc_err
    int_var__no_crc_update_on_crc_err = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< no_crc_update_on_crc_err_cpp_int_t >()  ;
    _count += 1;
    // no_sf
    int_var__no_sf = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< no_sf_cpp_int_t >()  ;
    _count += 1;
}

void cap_dpr_csr_cfg_global_t::all(const cpp_int & _val) {
    int _count = 0;

    // input_pacer_timer
    int_var__input_pacer_timer = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< input_pacer_timer_cpp_int_t >()  ;
    _count += 4;
    // input_pacer_release
    int_var__input_pacer_release = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< input_pacer_release_cpp_int_t >()  ;
    _count += 4;
    // pktout_fc_threshold
    int_var__pktout_fc_threshold = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< pktout_fc_threshold_cpp_int_t >()  ;
    _count += 3;
    // csum_ff_fc_threshold
    int_var__csum_ff_fc_threshold = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< csum_ff_fc_threshold_cpp_int_t >()  ;
    _count += 10;
    // phv_fifo_almost_full_threshold
    int_var__phv_fifo_almost_full_threshold = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< phv_fifo_almost_full_threshold_cpp_int_t >()  ;
    _count += 8;
    // ohi_fifo_almost_full_threshold
    int_var__ohi_fifo_almost_full_threshold = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< ohi_fifo_almost_full_threshold_cpp_int_t >()  ;
    _count += 4;
    // pktin_fifo_almost_full_threshold
    int_var__pktin_fifo_almost_full_threshold = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< pktin_fifo_almost_full_threshold_cpp_int_t >()  ;
    _count += 10;
}

void cap_dpr_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_dpr_csr_t::all(const cpp_int & _val) {
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
    hdrfld.all( hlp.get_slc(_val, _count, _count -1 + hdrfld.get_width() )); // hdrfld
    _count += hdrfld.get_width();
    cfg.all( hlp.get_slc(_val, _count, _count -1 + cfg.get_width() )); // cfg
    _count += cfg.get_width();
    stats.all( hlp.get_slc(_val, _count, _count -1 + stats.get_width() )); // stats
    _count += stats.get_width();
    mem.all( hlp.get_slc(_val, _count, _count -1 + mem.get_width() )); // mem
    _count += mem.get_width();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::all() const {
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
    // err_pktin_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktin_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_pktin_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktin_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_offset_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_offset_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_loc_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_loc_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_offset_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_offset_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_loc_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_loc_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_pkt_eop_early
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pkt_eop_early) , _count, _count -1 + 1 );
    _count += 1;
    // err_ptr_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ptr_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // err_pktout_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktout_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpr_csr_cfg_error_mask_t::all() const {
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
    // err_pktin_sop_no_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktin_sop_no_eop) , _count, _count -1 + 1 );
    _count += 1;
    // err_pktin_eop_no_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktin_eop_no_sop) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_offset_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_offset_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_loc_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_loc_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_offset_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_offset_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_crc_loc_gt_pkt_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_crc_loc_gt_pkt_size) , _count, _count -1 + 1 );
    _count += 1;
    // err_pkt_eop_early
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pkt_eop_early) , _count, _count -1 + 1 );
    _count += 1;
    // err_ptr_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_ptr_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // err_csum_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_csum_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // err_pktout_ff_overflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__err_pktout_ff_overflow) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpr_csr_cfg_global_2_t::all() const {
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

cpp_int cap_dpr_csr_cfg_global_1_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // ptr_fifo_fc_thr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ptr_fifo_fc_thr) , _count, _count -1 + 5 );
    _count += 5;
    // intrinsic_len_rw_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__intrinsic_len_rw_en) , _count, _count -1 + 1 );
    _count += 1;
    // intrinsic_drop_rw_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__intrinsic_drop_rw_en) , _count, _count -1 + 1 );
    _count += 1;
    // padding_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__padding_en) , _count, _count -1 + 1 );
    _count += 1;
    // pkt_min_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_min_size) , _count, _count -1 + 16 );
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
    // no_csum_update_on_csum_err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__no_csum_update_on_csum_err) , _count, _count -1 + 1 );
    _count += 1;
    // no_crc_update_on_crc_err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__no_crc_update_on_crc_err) , _count, _count -1 + 1 );
    _count += 1;
    // no_sf
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__no_sf) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_dpr_csr_cfg_global_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // input_pacer_timer
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__input_pacer_timer) , _count, _count -1 + 4 );
    _count += 4;
    // input_pacer_release
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__input_pacer_release) , _count, _count -1 + 4 );
    _count += 4;
    // pktout_fc_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pktout_fc_threshold) , _count, _count -1 + 3 );
    _count += 3;
    // csum_ff_fc_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__csum_ff_fc_threshold) , _count, _count -1 + 10 );
    _count += 10;
    // phv_fifo_almost_full_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_fifo_almost_full_threshold) , _count, _count -1 + 8 );
    _count += 8;
    // ohi_fifo_almost_full_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ohi_fifo_almost_full_threshold) , _count, _count -1 + 4 );
    _count += 4;
    // pktin_fifo_almost_full_threshold
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pktin_fifo_almost_full_threshold) , _count, _count -1 + 10 );
    _count += 10;
    return ret_val;
}

cpp_int cap_dpr_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dpr_csr_t::all() const {
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
    ret_val = hlp.set_slc(ret_val, hdrfld.all() , _count, _count -1 + hdrfld.get_width() ); // hdrfld
    _count += hdrfld.get_width();
    ret_val = hlp.set_slc(ret_val, cfg.all() , _count, _count -1 + cfg.get_width() ); // cfg
    _count += cfg.get_width();
    ret_val = hlp.set_slc(ret_val, stats.all() , _count, _count -1 + stats.get_width() ); // stats
    _count += stats.get_width();
    ret_val = hlp.set_slc(ret_val, mem.all() , _count, _count -1 + mem.get_width() ); // mem
    _count += mem.get_width();
    return ret_val;
}

void cap_dpr_csr_cfg_interrupt_mask_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop);
            register_get_func("err_phv_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop);
            register_get_func("err_phv_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop);
            register_get_func("err_ohi_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop);
            register_get_func("err_ohi_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktin_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_sop_no_eop);
            register_get_func("err_pktin_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktin_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_eop_no_sop);
            register_get_func("err_pktin_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_offset_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_offset_gt_pkt_size);
            register_get_func("err_csum_offset_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_offset_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_loc_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_loc_gt_pkt_size);
            register_get_func("err_csum_loc_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_loc_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_offset_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_crc_offset_gt_pkt_size);
            register_get_func("err_crc_offset_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_crc_offset_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_loc_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_crc_loc_gt_pkt_size);
            register_get_func("err_crc_loc_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_crc_loc_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pkt_eop_early", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pkt_eop_early);
            register_get_func("err_pkt_eop_early", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pkt_eop_early);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ptr_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ptr_ff_overflow);
            register_get_func("err_ptr_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_ptr_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_ff_overflow);
            register_get_func("err_csum_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_csum_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktout_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktout_ff_overflow);
            register_get_func("err_pktout_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::err_pktout_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_interrupt_mask_t::rsvd);
        }
        #endif
    
}

void cap_dpr_csr_cfg_error_mask_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_phv_sop_no_eop);
            register_get_func("err_phv_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_phv_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_phv_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_phv_eop_no_sop);
            register_get_func("err_phv_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_phv_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ohi_sop_no_eop);
            register_get_func("err_ohi_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ohi_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ohi_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ohi_eop_no_sop);
            register_get_func("err_ohi_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ohi_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktin_sop_no_eop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktin_sop_no_eop);
            register_get_func("err_pktin_sop_no_eop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktin_sop_no_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktin_eop_no_sop", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktin_eop_no_sop);
            register_get_func("err_pktin_eop_no_sop", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktin_eop_no_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_offset_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_offset_gt_pkt_size);
            register_get_func("err_csum_offset_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_offset_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_loc_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_loc_gt_pkt_size);
            register_get_func("err_csum_loc_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_loc_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_offset_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_crc_offset_gt_pkt_size);
            register_get_func("err_crc_offset_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_crc_offset_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_crc_loc_gt_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_crc_loc_gt_pkt_size);
            register_get_func("err_crc_loc_gt_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_crc_loc_gt_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pkt_eop_early", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pkt_eop_early);
            register_get_func("err_pkt_eop_early", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pkt_eop_early);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_ptr_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ptr_ff_overflow);
            register_get_func("err_ptr_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_ptr_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_csum_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_ff_overflow);
            register_get_func("err_csum_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_csum_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("err_pktout_ff_overflow", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktout_ff_overflow);
            register_get_func("err_pktout_ff_overflow", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::err_pktout_ff_overflow);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_error_mask_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_error_mask_t::rsvd);
        }
        #endif
    
}

void cap_dpr_csr_cfg_global_2_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dump_dop_no_data_phv", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_2_t::dump_dop_no_data_phv);
            register_get_func("dump_dop_no_data_phv", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_2_t::dump_dop_no_data_phv);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_2_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_2_t::rsvd);
        }
        #endif
    
}

void cap_dpr_csr_cfg_global_1_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ptr_fifo_fc_thr", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::ptr_fifo_fc_thr);
            register_get_func("ptr_fifo_fc_thr", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::ptr_fifo_fc_thr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("intrinsic_len_rw_en", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::intrinsic_len_rw_en);
            register_get_func("intrinsic_len_rw_en", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::intrinsic_len_rw_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("intrinsic_drop_rw_en", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::intrinsic_drop_rw_en);
            register_get_func("intrinsic_drop_rw_en", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::intrinsic_drop_rw_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("padding_en", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::padding_en);
            register_get_func("padding_en", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::padding_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_min_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::pkt_min_size);
            register_get_func("pkt_min_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::pkt_min_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_pkt_size", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::max_pkt_size);
            register_get_func("max_pkt_size", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::max_pkt_size);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_truncation_en", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::pkt_truncation_en);
            register_get_func("pkt_truncation_en", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::pkt_truncation_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ignore_hdrfld_size_0", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::ignore_hdrfld_size_0);
            register_get_func("ignore_hdrfld_size_0", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::ignore_hdrfld_size_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("bypass_mode", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::bypass_mode);
            register_get_func("bypass_mode", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::bypass_mode);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_drop_bypass_en", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::phv_drop_bypass_en);
            register_get_func("phv_drop_bypass_en", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::phv_drop_bypass_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("no_csum_update_on_csum_err", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_csum_update_on_csum_err);
            register_get_func("no_csum_update_on_csum_err", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_csum_update_on_csum_err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("no_crc_update_on_crc_err", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_crc_update_on_crc_err);
            register_get_func("no_crc_update_on_crc_err", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_crc_update_on_crc_err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("no_sf", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_sf);
            register_get_func("no_sf", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_1_t::no_sf);
        }
        #endif
    
    set_reset_val(cpp_int("0x44e20271027101f"));
    all(get_reset_val());
}

void cap_dpr_csr_cfg_global_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("input_pacer_timer", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::input_pacer_timer);
            register_get_func("input_pacer_timer", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::input_pacer_timer);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("input_pacer_release", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::input_pacer_release);
            register_get_func("input_pacer_release", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::input_pacer_release);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pktout_fc_threshold", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::pktout_fc_threshold);
            register_get_func("pktout_fc_threshold", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::pktout_fc_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("csum_ff_fc_threshold", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::csum_ff_fc_threshold);
            register_get_func("csum_ff_fc_threshold", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::csum_ff_fc_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("phv_fifo_almost_full_threshold", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::phv_fifo_almost_full_threshold);
            register_get_func("phv_fifo_almost_full_threshold", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::phv_fifo_almost_full_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ohi_fifo_almost_full_threshold", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::ohi_fifo_almost_full_threshold);
            register_get_func("ohi_fifo_almost_full_threshold", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::ohi_fifo_almost_full_threshold);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pktin_fifo_almost_full_threshold", (cap_csr_base::set_function_type_t)&cap_dpr_csr_cfg_global_t::pktin_fifo_almost_full_threshold);
            register_get_func("pktin_fifo_almost_full_threshold", (cap_csr_base::get_function_type_t)&cap_dpr_csr_cfg_global_t::pktin_fifo_almost_full_threshold);
        }
        #endif
    
    set_reset_val(cpp_int("0x11010c8437"));
    all(get_reset_val());
}

void cap_dpr_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_dpr_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_dpr_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_dpr_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_global.set_attributes(this,"cfg_global", 0x8 );
    cfg_global_1.set_attributes(this,"cfg_global_1", 0x10 );
    cfg_global_2.set_attributes(this,"cfg_global_2", 0x18 );
    cfg_error_mask.set_attributes(this,"cfg_error_mask", 0x20 );
    cfg_interrupt_mask.set_attributes(this,"cfg_interrupt_mask", 0x28 );
    hdrfld.set_attributes(this,"hdrfld", 0x400 );
    cfg.set_attributes(this,"cfg", 0x800 );
    stats.set_attributes(this,"stats", 0x4000 );
    mem.set_attributes(this,"mem", 0x80000 );
}

void cap_dpr_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop(const cpp_int & _val) { 
    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = _val.convert_to< err_phv_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_phv_sop_no_eop() const {
    return int_var__err_phv_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop(const cpp_int & _val) { 
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = _val.convert_to< err_phv_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_phv_eop_no_sop() const {
    return int_var__err_phv_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop(const cpp_int & _val) { 
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = _val.convert_to< err_ohi_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_sop_no_eop() const {
    return int_var__err_ohi_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop(const cpp_int & _val) { 
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = _val.convert_to< err_ohi_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_ohi_eop_no_sop() const {
    return int_var__err_ohi_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_sop_no_eop(const cpp_int & _val) { 
    // err_pktin_sop_no_eop
    int_var__err_pktin_sop_no_eop = _val.convert_to< err_pktin_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_sop_no_eop() const {
    return int_var__err_pktin_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_eop_no_sop(const cpp_int & _val) { 
    // err_pktin_eop_no_sop
    int_var__err_pktin_eop_no_sop = _val.convert_to< err_pktin_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_pktin_eop_no_sop() const {
    return int_var__err_pktin_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_csum_offset_gt_pkt_size(const cpp_int & _val) { 
    // err_csum_offset_gt_pkt_size
    int_var__err_csum_offset_gt_pkt_size = _val.convert_to< err_csum_offset_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_csum_offset_gt_pkt_size() const {
    return int_var__err_csum_offset_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_csum_loc_gt_pkt_size(const cpp_int & _val) { 
    // err_csum_loc_gt_pkt_size
    int_var__err_csum_loc_gt_pkt_size = _val.convert_to< err_csum_loc_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_csum_loc_gt_pkt_size() const {
    return int_var__err_csum_loc_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_crc_offset_gt_pkt_size(const cpp_int & _val) { 
    // err_crc_offset_gt_pkt_size
    int_var__err_crc_offset_gt_pkt_size = _val.convert_to< err_crc_offset_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_crc_offset_gt_pkt_size() const {
    return int_var__err_crc_offset_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_crc_loc_gt_pkt_size(const cpp_int & _val) { 
    // err_crc_loc_gt_pkt_size
    int_var__err_crc_loc_gt_pkt_size = _val.convert_to< err_crc_loc_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_crc_loc_gt_pkt_size() const {
    return int_var__err_crc_loc_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_pkt_eop_early(const cpp_int & _val) { 
    // err_pkt_eop_early
    int_var__err_pkt_eop_early = _val.convert_to< err_pkt_eop_early_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_pkt_eop_early() const {
    return int_var__err_pkt_eop_early.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_ptr_ff_overflow(const cpp_int & _val) { 
    // err_ptr_ff_overflow
    int_var__err_ptr_ff_overflow = _val.convert_to< err_ptr_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_ptr_ff_overflow() const {
    return int_var__err_ptr_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_csum_ff_overflow(const cpp_int & _val) { 
    // err_csum_ff_overflow
    int_var__err_csum_ff_overflow = _val.convert_to< err_csum_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_csum_ff_overflow() const {
    return int_var__err_csum_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::err_pktout_ff_overflow(const cpp_int & _val) { 
    // err_pktout_ff_overflow
    int_var__err_pktout_ff_overflow = _val.convert_to< err_pktout_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::err_pktout_ff_overflow() const {
    return int_var__err_pktout_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_interrupt_mask_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_interrupt_mask_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_phv_sop_no_eop(const cpp_int & _val) { 
    // err_phv_sop_no_eop
    int_var__err_phv_sop_no_eop = _val.convert_to< err_phv_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_phv_sop_no_eop() const {
    return int_var__err_phv_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_phv_eop_no_sop(const cpp_int & _val) { 
    // err_phv_eop_no_sop
    int_var__err_phv_eop_no_sop = _val.convert_to< err_phv_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_phv_eop_no_sop() const {
    return int_var__err_phv_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_ohi_sop_no_eop(const cpp_int & _val) { 
    // err_ohi_sop_no_eop
    int_var__err_ohi_sop_no_eop = _val.convert_to< err_ohi_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_ohi_sop_no_eop() const {
    return int_var__err_ohi_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_ohi_eop_no_sop(const cpp_int & _val) { 
    // err_ohi_eop_no_sop
    int_var__err_ohi_eop_no_sop = _val.convert_to< err_ohi_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_ohi_eop_no_sop() const {
    return int_var__err_ohi_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_pktin_sop_no_eop(const cpp_int & _val) { 
    // err_pktin_sop_no_eop
    int_var__err_pktin_sop_no_eop = _val.convert_to< err_pktin_sop_no_eop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_pktin_sop_no_eop() const {
    return int_var__err_pktin_sop_no_eop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_pktin_eop_no_sop(const cpp_int & _val) { 
    // err_pktin_eop_no_sop
    int_var__err_pktin_eop_no_sop = _val.convert_to< err_pktin_eop_no_sop_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_pktin_eop_no_sop() const {
    return int_var__err_pktin_eop_no_sop.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_csum_offset_gt_pkt_size(const cpp_int & _val) { 
    // err_csum_offset_gt_pkt_size
    int_var__err_csum_offset_gt_pkt_size = _val.convert_to< err_csum_offset_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_csum_offset_gt_pkt_size() const {
    return int_var__err_csum_offset_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_csum_loc_gt_pkt_size(const cpp_int & _val) { 
    // err_csum_loc_gt_pkt_size
    int_var__err_csum_loc_gt_pkt_size = _val.convert_to< err_csum_loc_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_csum_loc_gt_pkt_size() const {
    return int_var__err_csum_loc_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_crc_offset_gt_pkt_size(const cpp_int & _val) { 
    // err_crc_offset_gt_pkt_size
    int_var__err_crc_offset_gt_pkt_size = _val.convert_to< err_crc_offset_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_crc_offset_gt_pkt_size() const {
    return int_var__err_crc_offset_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_crc_loc_gt_pkt_size(const cpp_int & _val) { 
    // err_crc_loc_gt_pkt_size
    int_var__err_crc_loc_gt_pkt_size = _val.convert_to< err_crc_loc_gt_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_crc_loc_gt_pkt_size() const {
    return int_var__err_crc_loc_gt_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_pkt_eop_early(const cpp_int & _val) { 
    // err_pkt_eop_early
    int_var__err_pkt_eop_early = _val.convert_to< err_pkt_eop_early_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_pkt_eop_early() const {
    return int_var__err_pkt_eop_early.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_ptr_ff_overflow(const cpp_int & _val) { 
    // err_ptr_ff_overflow
    int_var__err_ptr_ff_overflow = _val.convert_to< err_ptr_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_ptr_ff_overflow() const {
    return int_var__err_ptr_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_csum_ff_overflow(const cpp_int & _val) { 
    // err_csum_ff_overflow
    int_var__err_csum_ff_overflow = _val.convert_to< err_csum_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_csum_ff_overflow() const {
    return int_var__err_csum_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::err_pktout_ff_overflow(const cpp_int & _val) { 
    // err_pktout_ff_overflow
    int_var__err_pktout_ff_overflow = _val.convert_to< err_pktout_ff_overflow_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::err_pktout_ff_overflow() const {
    return int_var__err_pktout_ff_overflow.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_error_mask_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_error_mask_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_2_t::dump_dop_no_data_phv(const cpp_int & _val) { 
    // dump_dop_no_data_phv
    int_var__dump_dop_no_data_phv = _val.convert_to< dump_dop_no_data_phv_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_2_t::dump_dop_no_data_phv() const {
    return int_var__dump_dop_no_data_phv.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_2_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_2_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::ptr_fifo_fc_thr(const cpp_int & _val) { 
    // ptr_fifo_fc_thr
    int_var__ptr_fifo_fc_thr = _val.convert_to< ptr_fifo_fc_thr_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::ptr_fifo_fc_thr() const {
    return int_var__ptr_fifo_fc_thr.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::intrinsic_len_rw_en(const cpp_int & _val) { 
    // intrinsic_len_rw_en
    int_var__intrinsic_len_rw_en = _val.convert_to< intrinsic_len_rw_en_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::intrinsic_len_rw_en() const {
    return int_var__intrinsic_len_rw_en.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::intrinsic_drop_rw_en(const cpp_int & _val) { 
    // intrinsic_drop_rw_en
    int_var__intrinsic_drop_rw_en = _val.convert_to< intrinsic_drop_rw_en_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::intrinsic_drop_rw_en() const {
    return int_var__intrinsic_drop_rw_en.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::padding_en(const cpp_int & _val) { 
    // padding_en
    int_var__padding_en = _val.convert_to< padding_en_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::padding_en() const {
    return int_var__padding_en.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::pkt_min_size(const cpp_int & _val) { 
    // pkt_min_size
    int_var__pkt_min_size = _val.convert_to< pkt_min_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::pkt_min_size() const {
    return int_var__pkt_min_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::max_pkt_size(const cpp_int & _val) { 
    // max_pkt_size
    int_var__max_pkt_size = _val.convert_to< max_pkt_size_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::max_pkt_size() const {
    return int_var__max_pkt_size.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::pkt_truncation_en(const cpp_int & _val) { 
    // pkt_truncation_en
    int_var__pkt_truncation_en = _val.convert_to< pkt_truncation_en_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::pkt_truncation_en() const {
    return int_var__pkt_truncation_en.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::ignore_hdrfld_size_0(const cpp_int & _val) { 
    // ignore_hdrfld_size_0
    int_var__ignore_hdrfld_size_0 = _val.convert_to< ignore_hdrfld_size_0_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::ignore_hdrfld_size_0() const {
    return int_var__ignore_hdrfld_size_0.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::bypass_mode(const cpp_int & _val) { 
    // bypass_mode
    int_var__bypass_mode = _val.convert_to< bypass_mode_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::bypass_mode() const {
    return int_var__bypass_mode.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::phv_drop_bypass_en(const cpp_int & _val) { 
    // phv_drop_bypass_en
    int_var__phv_drop_bypass_en = _val.convert_to< phv_drop_bypass_en_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::phv_drop_bypass_en() const {
    return int_var__phv_drop_bypass_en.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::no_csum_update_on_csum_err(const cpp_int & _val) { 
    // no_csum_update_on_csum_err
    int_var__no_csum_update_on_csum_err = _val.convert_to< no_csum_update_on_csum_err_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::no_csum_update_on_csum_err() const {
    return int_var__no_csum_update_on_csum_err.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::no_crc_update_on_crc_err(const cpp_int & _val) { 
    // no_crc_update_on_crc_err
    int_var__no_crc_update_on_crc_err = _val.convert_to< no_crc_update_on_crc_err_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::no_crc_update_on_crc_err() const {
    return int_var__no_crc_update_on_crc_err.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_1_t::no_sf(const cpp_int & _val) { 
    // no_sf
    int_var__no_sf = _val.convert_to< no_sf_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_1_t::no_sf() const {
    return int_var__no_sf.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::input_pacer_timer(const cpp_int & _val) { 
    // input_pacer_timer
    int_var__input_pacer_timer = _val.convert_to< input_pacer_timer_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::input_pacer_timer() const {
    return int_var__input_pacer_timer.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::input_pacer_release(const cpp_int & _val) { 
    // input_pacer_release
    int_var__input_pacer_release = _val.convert_to< input_pacer_release_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::input_pacer_release() const {
    return int_var__input_pacer_release.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::pktout_fc_threshold(const cpp_int & _val) { 
    // pktout_fc_threshold
    int_var__pktout_fc_threshold = _val.convert_to< pktout_fc_threshold_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::pktout_fc_threshold() const {
    return int_var__pktout_fc_threshold.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::csum_ff_fc_threshold(const cpp_int & _val) { 
    // csum_ff_fc_threshold
    int_var__csum_ff_fc_threshold = _val.convert_to< csum_ff_fc_threshold_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::csum_ff_fc_threshold() const {
    return int_var__csum_ff_fc_threshold.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::phv_fifo_almost_full_threshold(const cpp_int & _val) { 
    // phv_fifo_almost_full_threshold
    int_var__phv_fifo_almost_full_threshold = _val.convert_to< phv_fifo_almost_full_threshold_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::phv_fifo_almost_full_threshold() const {
    return int_var__phv_fifo_almost_full_threshold.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::ohi_fifo_almost_full_threshold(const cpp_int & _val) { 
    // ohi_fifo_almost_full_threshold
    int_var__ohi_fifo_almost_full_threshold = _val.convert_to< ohi_fifo_almost_full_threshold_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::ohi_fifo_almost_full_threshold() const {
    return int_var__ohi_fifo_almost_full_threshold.convert_to< cpp_int >();
}
    
void cap_dpr_csr_cfg_global_t::pktin_fifo_almost_full_threshold(const cpp_int & _val) { 
    // pktin_fifo_almost_full_threshold
    int_var__pktin_fifo_almost_full_threshold = _val.convert_to< pktin_fifo_almost_full_threshold_cpp_int_t >();
}

cpp_int cap_dpr_csr_cfg_global_t::pktin_fifo_almost_full_threshold() const {
    return int_var__pktin_fifo_almost_full_threshold.convert_to< cpp_int >();
}
    
void cap_dpr_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_dpr_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    