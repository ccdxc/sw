
#include "cap_txs_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_txs_csr_int_tmr_int_enable_clear_t::cap_txs_csr_int_tmr_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_tmr_int_enable_clear_t::~cap_txs_csr_int_tmr_int_enable_clear_t() { }

cap_txs_csr_int_tmr_int_test_set_t::cap_txs_csr_int_tmr_int_test_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_tmr_int_test_set_t::~cap_txs_csr_int_tmr_int_test_set_t() { }

cap_txs_csr_int_tmr_t::cap_txs_csr_int_tmr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_tmr_t::~cap_txs_csr_int_tmr_t() { }

cap_txs_csr_int_sch_int_enable_clear_t::cap_txs_csr_int_sch_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_sch_int_enable_clear_t::~cap_txs_csr_int_sch_int_enable_clear_t() { }

cap_txs_csr_int_sch_int_test_set_t::cap_txs_csr_int_sch_int_test_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_sch_int_test_set_t::~cap_txs_csr_int_sch_int_test_set_t() { }

cap_txs_csr_int_sch_t::cap_txs_csr_int_sch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_sch_t::~cap_txs_csr_int_sch_t() { }

cap_txs_csr_intreg_status_t::cap_txs_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_intreg_status_t::~cap_txs_csr_intreg_status_t() { }

cap_txs_csr_int_groups_int_enable_rw_reg_t::cap_txs_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_groups_int_enable_rw_reg_t::~cap_txs_csr_int_groups_int_enable_rw_reg_t() { }

cap_txs_csr_intgrp_status_t::cap_txs_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_intgrp_status_t::~cap_txs_csr_intgrp_status_t() { }

cap_txs_csr_int_srams_ecc_int_enable_set_t::cap_txs_csr_int_srams_ecc_int_enable_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_srams_ecc_int_enable_set_t::~cap_txs_csr_int_srams_ecc_int_enable_set_t() { }

cap_txs_csr_int_srams_ecc_intreg_t::cap_txs_csr_int_srams_ecc_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_srams_ecc_intreg_t::~cap_txs_csr_int_srams_ecc_intreg_t() { }

cap_txs_csr_int_srams_ecc_t::cap_txs_csr_int_srams_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_int_srams_ecc_t::~cap_txs_csr_int_srams_ecc_t() { }

cap_txs_csr_dhs_sch_grp_sram_entry_t::cap_txs_csr_dhs_sch_grp_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_sram_entry_t::~cap_txs_csr_dhs_sch_grp_sram_entry_t() { }

cap_txs_csr_dhs_sch_grp_sram_t::cap_txs_csr_dhs_sch_grp_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_sram_t::~cap_txs_csr_dhs_sch_grp_sram_t() { }

cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::cap_txs_csr_dhs_sch_rlid_map_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::~cap_txs_csr_dhs_sch_rlid_map_sram_entry_t() { }

cap_txs_csr_dhs_sch_rlid_map_sram_t::cap_txs_csr_dhs_sch_rlid_map_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_rlid_map_sram_t::~cap_txs_csr_dhs_sch_rlid_map_sram_t() { }

cap_txs_csr_dhs_sch_lif_map_sram_entry_t::cap_txs_csr_dhs_sch_lif_map_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_lif_map_sram_entry_t::~cap_txs_csr_dhs_sch_lif_map_sram_entry_t() { }

cap_txs_csr_dhs_sch_lif_map_sram_t::cap_txs_csr_dhs_sch_lif_map_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_lif_map_sram_t::~cap_txs_csr_dhs_sch_lif_map_sram_t() { }

cap_txs_csr_dhs_tmr_cnt_sram_entry_t::cap_txs_csr_dhs_tmr_cnt_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_tmr_cnt_sram_entry_t::~cap_txs_csr_dhs_tmr_cnt_sram_entry_t() { }

cap_txs_csr_dhs_tmr_cnt_sram_t::cap_txs_csr_dhs_tmr_cnt_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_tmr_cnt_sram_t::~cap_txs_csr_dhs_tmr_cnt_sram_t() { }

cap_txs_csr_dhs_rlid_stop_entry_t::cap_txs_csr_dhs_rlid_stop_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_rlid_stop_entry_t::~cap_txs_csr_dhs_rlid_stop_entry_t() { }

cap_txs_csr_dhs_rlid_stop_t::cap_txs_csr_dhs_rlid_stop_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_rlid_stop_t::~cap_txs_csr_dhs_rlid_stop_t() { }

cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::~cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t() { }

cap_txs_csr_dhs_sch_grp_cnt_entry_t::cap_txs_csr_dhs_sch_grp_cnt_entry_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_cnt_entry_t::~cap_txs_csr_dhs_sch_grp_cnt_entry_t() { }

cap_txs_csr_dhs_sch_grp_entry_entry_t::cap_txs_csr_dhs_sch_grp_entry_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_entry_entry_t::~cap_txs_csr_dhs_sch_grp_entry_entry_t() { }

cap_txs_csr_dhs_sch_grp_entry_t::cap_txs_csr_dhs_sch_grp_entry_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_grp_entry_t::~cap_txs_csr_dhs_sch_grp_entry_t() { }

cap_txs_csr_dhs_doorbell_entry_t::cap_txs_csr_dhs_doorbell_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_doorbell_entry_t::~cap_txs_csr_dhs_doorbell_entry_t() { }

cap_txs_csr_dhs_doorbell_t::cap_txs_csr_dhs_doorbell_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_doorbell_t::~cap_txs_csr_dhs_doorbell_t() { }

cap_txs_csr_dhs_sch_flags_entry_t::cap_txs_csr_dhs_sch_flags_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_flags_entry_t::~cap_txs_csr_dhs_sch_flags_entry_t() { }

cap_txs_csr_dhs_sch_flags_t::cap_txs_csr_dhs_sch_flags_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_sch_flags_t::~cap_txs_csr_dhs_sch_flags_t() { }

cap_txs_csr_dhs_dtdmhi_calendar_entry_t::cap_txs_csr_dhs_dtdmhi_calendar_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_dtdmhi_calendar_entry_t::~cap_txs_csr_dhs_dtdmhi_calendar_entry_t() { }

cap_txs_csr_dhs_dtdmhi_calendar_t::cap_txs_csr_dhs_dtdmhi_calendar_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_dtdmhi_calendar_t::~cap_txs_csr_dhs_dtdmhi_calendar_t() { }

cap_txs_csr_dhs_dtdmlo_calendar_entry_t::cap_txs_csr_dhs_dtdmlo_calendar_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_dtdmlo_calendar_entry_t::~cap_txs_csr_dhs_dtdmlo_calendar_entry_t() { }

cap_txs_csr_dhs_dtdmlo_calendar_t::cap_txs_csr_dhs_dtdmlo_calendar_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_dtdmlo_calendar_t::~cap_txs_csr_dhs_dtdmlo_calendar_t() { }

cap_txs_csr_dhs_slow_timer_pending_entry_t::cap_txs_csr_dhs_slow_timer_pending_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_slow_timer_pending_entry_t::~cap_txs_csr_dhs_slow_timer_pending_entry_t() { }

cap_txs_csr_dhs_slow_timer_pending_t::cap_txs_csr_dhs_slow_timer_pending_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_slow_timer_pending_t::~cap_txs_csr_dhs_slow_timer_pending_t() { }

cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::~cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t() { }

cap_txs_csr_dhs_slow_timer_start_no_stop_t::cap_txs_csr_dhs_slow_timer_start_no_stop_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_slow_timer_start_no_stop_t::~cap_txs_csr_dhs_slow_timer_start_no_stop_t() { }

cap_txs_csr_dhs_fast_timer_pending_entry_t::cap_txs_csr_dhs_fast_timer_pending_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_fast_timer_pending_entry_t::~cap_txs_csr_dhs_fast_timer_pending_entry_t() { }

cap_txs_csr_dhs_fast_timer_pending_t::cap_txs_csr_dhs_fast_timer_pending_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_fast_timer_pending_t::~cap_txs_csr_dhs_fast_timer_pending_t() { }

cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::~cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t() { }

cap_txs_csr_dhs_fast_timer_start_no_stop_t::cap_txs_csr_dhs_fast_timer_start_no_stop_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_txs_csr_dhs_fast_timer_start_no_stop_t::~cap_txs_csr_dhs_fast_timer_start_no_stop_t() { }

cap_txs_csr_cnt_tmr_axi_bid_uexp_t::cap_txs_csr_cnt_tmr_axi_bid_uexp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_bid_uexp_t::~cap_txs_csr_cnt_tmr_axi_bid_uexp_t() { }

cap_txs_csr_cnt_tmr_axi_bid_rerr_t::cap_txs_csr_cnt_tmr_axi_bid_rerr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_bid_rerr_t::~cap_txs_csr_cnt_tmr_axi_bid_rerr_t() { }

cap_txs_csr_cnt_tmr_axi_bid_err_t::cap_txs_csr_cnt_tmr_axi_bid_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_bid_err_t::~cap_txs_csr_cnt_tmr_axi_bid_err_t() { }

cap_txs_csr_cnt_tmr_axi_bid_t::cap_txs_csr_cnt_tmr_axi_bid_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_bid_t::~cap_txs_csr_cnt_tmr_axi_bid_t() { }

cap_txs_csr_cnt_tmr_axi_wr_req_t::cap_txs_csr_cnt_tmr_axi_wr_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_wr_req_t::~cap_txs_csr_cnt_tmr_axi_wr_req_t() { }

cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::~cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t() { }

cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::~cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t() { }

cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::~cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t() { }

cap_txs_csr_cnt_tmr_axi_rd_rsp_t::cap_txs_csr_cnt_tmr_axi_rd_rsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_rd_rsp_t::~cap_txs_csr_cnt_tmr_axi_rd_rsp_t() { }

cap_txs_csr_cnt_tmr_axi_rd_req_t::cap_txs_csr_cnt_tmr_axi_rd_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_tmr_axi_rd_req_t::~cap_txs_csr_cnt_tmr_axi_rd_req_t() { }

cap_txs_csr_cnt_sch_axi_bid_uexp_t::cap_txs_csr_cnt_sch_axi_bid_uexp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_bid_uexp_t::~cap_txs_csr_cnt_sch_axi_bid_uexp_t() { }

cap_txs_csr_cnt_sch_axi_bid_rerr_t::cap_txs_csr_cnt_sch_axi_bid_rerr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_bid_rerr_t::~cap_txs_csr_cnt_sch_axi_bid_rerr_t() { }

cap_txs_csr_cnt_sch_axi_bid_err_t::cap_txs_csr_cnt_sch_axi_bid_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_bid_err_t::~cap_txs_csr_cnt_sch_axi_bid_err_t() { }

cap_txs_csr_cnt_sch_axi_bid_t::cap_txs_csr_cnt_sch_axi_bid_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_bid_t::~cap_txs_csr_cnt_sch_axi_bid_t() { }

cap_txs_csr_cnt_sch_axi_wr_req_t::cap_txs_csr_cnt_sch_axi_wr_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_wr_req_t::~cap_txs_csr_cnt_sch_axi_wr_req_t() { }

cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::~cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t() { }

cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::~cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t() { }

cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::cap_txs_csr_cnt_sch_axi_rd_rsp_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::~cap_txs_csr_cnt_sch_axi_rd_rsp_err_t() { }

cap_txs_csr_cnt_sch_axi_rd_rsp_t::cap_txs_csr_cnt_sch_axi_rd_rsp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_rd_rsp_t::~cap_txs_csr_cnt_sch_axi_rd_rsp_t() { }

cap_txs_csr_cnt_sch_axi_rd_req_t::cap_txs_csr_cnt_sch_axi_rd_req_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_axi_rd_req_t::~cap_txs_csr_cnt_sch_axi_rd_req_t() { }

cap_txs_csr_sta_stmr_max_bcnt_t::cap_txs_csr_sta_stmr_max_bcnt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_stmr_max_bcnt_t::~cap_txs_csr_sta_stmr_max_bcnt_t() { }

cap_txs_csr_sta_ftmr_max_bcnt_t::cap_txs_csr_sta_ftmr_max_bcnt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_ftmr_max_bcnt_t::~cap_txs_csr_sta_ftmr_max_bcnt_t() { }

cap_txs_csr_csr_intr_t::cap_txs_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_csr_intr_t::~cap_txs_csr_csr_intr_t() { }

cap_txs_csr_sta_srams_sch_hbm_byp_t::cap_txs_csr_sta_srams_sch_hbm_byp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_sch_hbm_byp_t::~cap_txs_csr_sta_srams_sch_hbm_byp_t() { }

cap_txs_csr_sta_srams_sch_hbm_t::cap_txs_csr_sta_srams_sch_hbm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_sch_hbm_t::~cap_txs_csr_sta_srams_sch_hbm_t() { }

cap_txs_csr_cfg_sch_hbm_byp_sram_t::cap_txs_csr_cfg_sch_hbm_byp_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_hbm_byp_sram_t::~cap_txs_csr_cfg_sch_hbm_byp_sram_t() { }

cap_txs_csr_cfg_sch_hbm_sram_t::cap_txs_csr_cfg_sch_hbm_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_hbm_sram_t::~cap_txs_csr_cfg_sch_hbm_sram_t() { }

cap_txs_csr_sta_srams_ecc_sch_grp_t::cap_txs_csr_sta_srams_ecc_sch_grp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_ecc_sch_grp_t::~cap_txs_csr_sta_srams_ecc_sch_grp_t() { }

cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::cap_txs_csr_sta_srams_ecc_sch_rlid_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::~cap_txs_csr_sta_srams_ecc_sch_rlid_map_t() { }

cap_txs_csr_sta_srams_ecc_sch_lif_map_t::cap_txs_csr_sta_srams_ecc_sch_lif_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_ecc_sch_lif_map_t::~cap_txs_csr_sta_srams_ecc_sch_lif_map_t() { }

cap_txs_csr_sta_srams_ecc_tmr_cnt_t::cap_txs_csr_sta_srams_ecc_tmr_cnt_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_ecc_tmr_cnt_t::~cap_txs_csr_sta_srams_ecc_tmr_cnt_t() { }

cap_txs_csr_sta_scheduler_dbg2_t::cap_txs_csr_sta_scheduler_dbg2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_scheduler_dbg2_t::~cap_txs_csr_sta_scheduler_dbg2_t() { }

cap_txs_csr_sta_scheduler_dbg_t::cap_txs_csr_sta_scheduler_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_scheduler_dbg_t::~cap_txs_csr_sta_scheduler_dbg_t() { }

cap_txs_csr_sta_sch_lif_sg_mismatch_t::cap_txs_csr_sta_sch_lif_sg_mismatch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_sch_lif_sg_mismatch_t::~cap_txs_csr_sta_sch_lif_sg_mismatch_t() { }

cap_txs_csr_sta_sch_lif_map_notactive_t::cap_txs_csr_sta_sch_lif_map_notactive_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_sch_lif_map_notactive_t::~cap_txs_csr_sta_sch_lif_map_notactive_t() { }

cap_txs_csr_cfg_sch_grp_sram_t::cap_txs_csr_cfg_sch_grp_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_grp_sram_t::~cap_txs_csr_cfg_sch_grp_sram_t() { }

cap_txs_csr_cfg_sch_rlid_map_sram_t::cap_txs_csr_cfg_sch_rlid_map_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_rlid_map_sram_t::~cap_txs_csr_cfg_sch_rlid_map_sram_t() { }

cap_txs_csr_cfg_sch_lif_map_sram_t::cap_txs_csr_cfg_sch_lif_map_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_lif_map_sram_t::~cap_txs_csr_cfg_sch_lif_map_sram_t() { }

cap_txs_csr_cfg_tmr_cnt_sram_t::cap_txs_csr_cfg_tmr_cnt_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_tmr_cnt_sram_t::~cap_txs_csr_cfg_tmr_cnt_sram_t() { }

cap_txs_csr_sta_sch_max_hbm_byp_t::cap_txs_csr_sta_sch_max_hbm_byp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_sch_max_hbm_byp_t::~cap_txs_csr_sta_sch_max_hbm_byp_t() { }

cap_txs_csr_cfg_scheduler_dbg2_t::cap_txs_csr_cfg_scheduler_dbg2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_scheduler_dbg2_t::~cap_txs_csr_cfg_scheduler_dbg2_t() { }

cap_txs_csr_cfg_scheduler_dbg_t::cap_txs_csr_cfg_scheduler_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_scheduler_dbg_t::~cap_txs_csr_cfg_scheduler_dbg_t() { }

cap_txs_csr_sta_scheduler_rr_t::cap_txs_csr_sta_scheduler_rr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_scheduler_rr_t::~cap_txs_csr_sta_scheduler_rr_t() { }

cap_txs_csr_sta_scheduler_t::cap_txs_csr_sta_scheduler_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_scheduler_t::~cap_txs_csr_sta_scheduler_t() { }

cap_txs_csr_cnt_sch_rlid_start_t::cap_txs_csr_cnt_sch_rlid_start_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_rlid_start_t::~cap_txs_csr_cnt_sch_rlid_start_t() { }

cap_txs_csr_cnt_sch_rlid_stop_t::cap_txs_csr_cnt_sch_rlid_stop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_rlid_stop_t::~cap_txs_csr_cnt_sch_rlid_stop_t() { }

cap_txs_csr_sta_glb_t::cap_txs_csr_sta_glb_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_glb_t::~cap_txs_csr_sta_glb_t() { }

cap_txs_csr_cnt_sch_txdma_cos15_t::cap_txs_csr_cnt_sch_txdma_cos15_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos15_t::~cap_txs_csr_cnt_sch_txdma_cos15_t() { }

cap_txs_csr_cnt_sch_txdma_cos14_t::cap_txs_csr_cnt_sch_txdma_cos14_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos14_t::~cap_txs_csr_cnt_sch_txdma_cos14_t() { }

cap_txs_csr_cnt_sch_txdma_cos13_t::cap_txs_csr_cnt_sch_txdma_cos13_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos13_t::~cap_txs_csr_cnt_sch_txdma_cos13_t() { }

cap_txs_csr_cnt_sch_txdma_cos12_t::cap_txs_csr_cnt_sch_txdma_cos12_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos12_t::~cap_txs_csr_cnt_sch_txdma_cos12_t() { }

cap_txs_csr_cnt_sch_txdma_cos11_t::cap_txs_csr_cnt_sch_txdma_cos11_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos11_t::~cap_txs_csr_cnt_sch_txdma_cos11_t() { }

cap_txs_csr_cnt_sch_txdma_cos10_t::cap_txs_csr_cnt_sch_txdma_cos10_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos10_t::~cap_txs_csr_cnt_sch_txdma_cos10_t() { }

cap_txs_csr_cnt_sch_txdma_cos9_t::cap_txs_csr_cnt_sch_txdma_cos9_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos9_t::~cap_txs_csr_cnt_sch_txdma_cos9_t() { }

cap_txs_csr_cnt_sch_txdma_cos8_t::cap_txs_csr_cnt_sch_txdma_cos8_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos8_t::~cap_txs_csr_cnt_sch_txdma_cos8_t() { }

cap_txs_csr_cnt_sch_txdma_cos7_t::cap_txs_csr_cnt_sch_txdma_cos7_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos7_t::~cap_txs_csr_cnt_sch_txdma_cos7_t() { }

cap_txs_csr_cnt_sch_txdma_cos6_t::cap_txs_csr_cnt_sch_txdma_cos6_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos6_t::~cap_txs_csr_cnt_sch_txdma_cos6_t() { }

cap_txs_csr_cnt_sch_txdma_cos5_t::cap_txs_csr_cnt_sch_txdma_cos5_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos5_t::~cap_txs_csr_cnt_sch_txdma_cos5_t() { }

cap_txs_csr_cnt_sch_txdma_cos4_t::cap_txs_csr_cnt_sch_txdma_cos4_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos4_t::~cap_txs_csr_cnt_sch_txdma_cos4_t() { }

cap_txs_csr_cnt_sch_txdma_cos3_t::cap_txs_csr_cnt_sch_txdma_cos3_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos3_t::~cap_txs_csr_cnt_sch_txdma_cos3_t() { }

cap_txs_csr_cnt_sch_txdma_cos2_t::cap_txs_csr_cnt_sch_txdma_cos2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos2_t::~cap_txs_csr_cnt_sch_txdma_cos2_t() { }

cap_txs_csr_cnt_sch_txdma_cos1_t::cap_txs_csr_cnt_sch_txdma_cos1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos1_t::~cap_txs_csr_cnt_sch_txdma_cos1_t() { }

cap_txs_csr_cnt_sch_txdma_cos0_t::cap_txs_csr_cnt_sch_txdma_cos0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_txdma_cos0_t::~cap_txs_csr_cnt_sch_txdma_cos0_t() { }

cap_txs_csr_cnt_sch_fifo_empty_t::cap_txs_csr_cnt_sch_fifo_empty_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_fifo_empty_t::~cap_txs_csr_cnt_sch_fifo_empty_t() { }

cap_txs_csr_cnt_sch_doorbell_clr_t::cap_txs_csr_cnt_sch_doorbell_clr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_doorbell_clr_t::~cap_txs_csr_cnt_sch_doorbell_clr_t() { }

cap_txs_csr_cnt_sch_doorbell_set_t::cap_txs_csr_cnt_sch_doorbell_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_sch_doorbell_set_t::~cap_txs_csr_cnt_sch_doorbell_set_t() { }

cap_txs_csr_cfg_sch_t::cap_txs_csr_cfg_sch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_sch_t::~cap_txs_csr_cfg_sch_t() { }

cap_txs_csr_cfw_scheduler_static_t::cap_txs_csr_cfw_scheduler_static_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfw_scheduler_static_t::~cap_txs_csr_cfw_scheduler_static_t() { }

cap_txs_csr_cfw_scheduler_glb_t::cap_txs_csr_cfw_scheduler_glb_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfw_scheduler_glb_t::~cap_txs_csr_cfw_scheduler_glb_t() { }

cap_txs_csr_sta_srams_tmr_fifo_t::cap_txs_csr_sta_srams_tmr_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_tmr_fifo_t::~cap_txs_csr_sta_srams_tmr_fifo_t() { }

cap_txs_csr_sta_srams_tmr_hbm_byp_t::cap_txs_csr_sta_srams_tmr_hbm_byp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_tmr_hbm_byp_t::~cap_txs_csr_sta_srams_tmr_hbm_byp_t() { }

cap_txs_csr_sta_srams_tmr_hbm_t::cap_txs_csr_sta_srams_tmr_hbm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_srams_tmr_hbm_t::~cap_txs_csr_sta_srams_tmr_hbm_t() { }

cap_txs_csr_cfg_tmr_fifo_sram_t::cap_txs_csr_cfg_tmr_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_tmr_fifo_sram_t::~cap_txs_csr_cfg_tmr_fifo_sram_t() { }

cap_txs_csr_cfg_tmr_hbm_byp_sram_t::cap_txs_csr_cfg_tmr_hbm_byp_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_tmr_hbm_byp_sram_t::~cap_txs_csr_cfg_tmr_hbm_byp_sram_t() { }

cap_txs_csr_cfg_tmr_hbm_sram_t::cap_txs_csr_cfg_tmr_hbm_sram_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_tmr_hbm_sram_t::~cap_txs_csr_cfg_tmr_hbm_sram_t() { }

cap_txs_csr_cnt_stmr_pop_t::cap_txs_csr_cnt_stmr_pop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_stmr_pop_t::~cap_txs_csr_cnt_stmr_pop_t() { }

cap_txs_csr_cnt_stmr_key_not_found_t::cap_txs_csr_cnt_stmr_key_not_found_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_stmr_key_not_found_t::~cap_txs_csr_cnt_stmr_key_not_found_t() { }

cap_txs_csr_cnt_stmr_push_out_of_wheel_t::cap_txs_csr_cnt_stmr_push_out_of_wheel_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_stmr_push_out_of_wheel_t::~cap_txs_csr_cnt_stmr_push_out_of_wheel_t() { }

cap_txs_csr_cnt_stmr_key_not_push_t::cap_txs_csr_cnt_stmr_key_not_push_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_stmr_key_not_push_t::~cap_txs_csr_cnt_stmr_key_not_push_t() { }

cap_txs_csr_cnt_stmr_push_t::cap_txs_csr_cnt_stmr_push_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_stmr_push_t::~cap_txs_csr_cnt_stmr_push_t() { }

cap_txs_csr_sta_slow_timer_t::cap_txs_csr_sta_slow_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_slow_timer_t::~cap_txs_csr_sta_slow_timer_t() { }

cap_txs_csr_cfg_slow_timer_dbell_t::cap_txs_csr_cfg_slow_timer_dbell_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_slow_timer_dbell_t::~cap_txs_csr_cfg_slow_timer_dbell_t() { }

cap_txs_csr_cfg_force_slow_timer_t::cap_txs_csr_cfg_force_slow_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_force_slow_timer_t::~cap_txs_csr_cfg_force_slow_timer_t() { }

cap_txs_csr_cfg_slow_timer_t::cap_txs_csr_cfg_slow_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_slow_timer_t::~cap_txs_csr_cfg_slow_timer_t() { }

cap_txs_csr_cnt_ftmr_pop_t::cap_txs_csr_cnt_ftmr_pop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_ftmr_pop_t::~cap_txs_csr_cnt_ftmr_pop_t() { }

cap_txs_csr_cnt_ftmr_key_not_found_t::cap_txs_csr_cnt_ftmr_key_not_found_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_ftmr_key_not_found_t::~cap_txs_csr_cnt_ftmr_key_not_found_t() { }

cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::cap_txs_csr_cnt_ftmr_push_out_of_wheel_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::~cap_txs_csr_cnt_ftmr_push_out_of_wheel_t() { }

cap_txs_csr_cnt_ftmr_key_not_push_t::cap_txs_csr_cnt_ftmr_key_not_push_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_ftmr_key_not_push_t::~cap_txs_csr_cnt_ftmr_key_not_push_t() { }

cap_txs_csr_cnt_ftmr_push_t::cap_txs_csr_cnt_ftmr_push_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cnt_ftmr_push_t::~cap_txs_csr_cnt_ftmr_push_t() { }

cap_txs_csr_sta_fast_timer_t::cap_txs_csr_sta_fast_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_fast_timer_t::~cap_txs_csr_sta_fast_timer_t() { }

cap_txs_csr_cfg_fast_timer_dbell_t::cap_txs_csr_cfg_fast_timer_dbell_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_fast_timer_dbell_t::~cap_txs_csr_cfg_fast_timer_dbell_t() { }

cap_txs_csr_cfg_force_fast_timer_t::cap_txs_csr_cfg_force_fast_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_force_fast_timer_t::~cap_txs_csr_cfg_force_fast_timer_t() { }

cap_txs_csr_cfg_fast_timer_t::cap_txs_csr_cfg_fast_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_fast_timer_t::~cap_txs_csr_cfg_fast_timer_t() { }

cap_txs_csr_sta_timer_dbg2_t::cap_txs_csr_sta_timer_dbg2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_timer_dbg2_t::~cap_txs_csr_sta_timer_dbg2_t() { }

cap_txs_csr_sta_timer_dbg_t::cap_txs_csr_sta_timer_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_timer_dbg_t::~cap_txs_csr_sta_timer_dbg_t() { }

cap_txs_csr_sta_tmr_max_keys_t::cap_txs_csr_sta_tmr_max_keys_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_tmr_max_keys_t::~cap_txs_csr_sta_tmr_max_keys_t() { }

cap_txs_csr_sta_tmr_max_hbm_byp_t::cap_txs_csr_sta_tmr_max_hbm_byp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_tmr_max_hbm_byp_t::~cap_txs_csr_sta_tmr_max_hbm_byp_t() { }

cap_txs_csr_sta_timer_t::cap_txs_csr_sta_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_sta_timer_t::~cap_txs_csr_sta_timer_t() { }

cap_txs_csr_cfg_timer_dbg2_t::cap_txs_csr_cfg_timer_dbg2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_timer_dbg2_t::~cap_txs_csr_cfg_timer_dbg2_t() { }

cap_txs_csr_cfg_timer_dbg_t::cap_txs_csr_cfg_timer_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_timer_dbg_t::~cap_txs_csr_cfg_timer_dbg_t() { }

cap_txs_csr_cfw_timer_glb_t::cap_txs_csr_cfw_timer_glb_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfw_timer_glb_t::~cap_txs_csr_cfw_timer_glb_t() { }

cap_txs_csr_cfg_timer_static_t::cap_txs_csr_cfg_timer_static_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_timer_static_t::~cap_txs_csr_cfg_timer_static_t() { }

cap_txs_csr_cfg_axi_attr_t::cap_txs_csr_cfg_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_axi_attr_t::~cap_txs_csr_cfg_axi_attr_t() { }

cap_txs_csr_cfg_glb_t::cap_txs_csr_cfg_glb_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_cfg_glb_t::~cap_txs_csr_cfg_glb_t() { }

cap_txs_csr_base_t::cap_txs_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_txs_csr_base_t::~cap_txs_csr_base_t() { }

cap_txs_csr_t::cap_txs_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_txs_csr_t::~cap_txs_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_tmr_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_wid_empty_enable: 0x" << int_var__tmr_wid_empty_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rid_empty_enable: 0x" << int_var__tmr_rid_empty_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_wr_txfifo_ovf_enable: 0x" << int_var__tmr_wr_txfifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rd_txfifo_ovf_enable: 0x" << int_var__tmr_rd_txfifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_state_fifo_ovf_enable: 0x" << int_var__tmr_state_fifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_ovf_enable: 0x" << int_var__tmr_hbm_byp_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_wtag_wrap_enable: 0x" << int_var__tmr_hbm_byp_wtag_wrap_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_ctime_wrap_enable: 0x" << int_var__ftmr_ctime_wrap_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_ctime_wrap_enable: 0x" << int_var__stmr_ctime_wrap_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_push_out_of_wheel_enable: 0x" << int_var__stmr_push_out_of_wheel_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_push_out_of_wheel_enable: 0x" << int_var__ftmr_push_out_of_wheel_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_key_not_push_enable: 0x" << int_var__ftmr_key_not_push_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_key_not_found_enable: 0x" << int_var__ftmr_key_not_found_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_key_not_push_enable: 0x" << int_var__stmr_key_not_push_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_key_not_found_enable: 0x" << int_var__stmr_key_not_found_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_stall_enable: 0x" << int_var__stmr_stall_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_stall_enable: 0x" << int_var__ftmr_stall_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_ovf_enable: 0x" << int_var__ftmr_fifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_ovf_enable: 0x" << int_var__stmr_fifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_efull_enable: 0x" << int_var__ftmr_fifo_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_efull_enable: 0x" << int_var__stmr_fifo_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rejct_drb_ovf_enable: 0x" << int_var__tmr_rejct_drb_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rejct_drb_efull_enable: 0x" << int_var__tmr_rejct_drb_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_re_efull_enable: 0x" << int_var__tmr_hbm_re_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_we_efull_enable: 0x" << int_var__tmr_hbm_we_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_pending_efull_enable: 0x" << int_var__tmr_hbm_byp_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_rd_pending_efull_enable: 0x" << int_var__tmr_hbm_rd_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_wr_pending_efull_enable: 0x" << int_var__tmr_hbm_wr_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_bresp_err_enable: 0x" << int_var__tmr_bresp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_bid_err_enable: 0x" << int_var__tmr_bid_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rresp_err_enable: 0x" << int_var__tmr_rresp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rid_err_enable: 0x" << int_var__tmr_rid_err_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_tmr_int_test_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_wid_empty_interrupt: 0x" << int_var__tmr_wid_empty_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rid_empty_interrupt: 0x" << int_var__tmr_rid_empty_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_wr_txfifo_ovf_interrupt: 0x" << int_var__tmr_wr_txfifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rd_txfifo_ovf_interrupt: 0x" << int_var__tmr_rd_txfifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_state_fifo_ovf_interrupt: 0x" << int_var__tmr_state_fifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_ovf_interrupt: 0x" << int_var__tmr_hbm_byp_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_wtag_wrap_interrupt: 0x" << int_var__tmr_hbm_byp_wtag_wrap_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_ctime_wrap_interrupt: 0x" << int_var__ftmr_ctime_wrap_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_ctime_wrap_interrupt: 0x" << int_var__stmr_ctime_wrap_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_push_out_of_wheel_interrupt: 0x" << int_var__stmr_push_out_of_wheel_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_push_out_of_wheel_interrupt: 0x" << int_var__ftmr_push_out_of_wheel_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_key_not_push_interrupt: 0x" << int_var__ftmr_key_not_push_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_key_not_found_interrupt: 0x" << int_var__ftmr_key_not_found_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_key_not_push_interrupt: 0x" << int_var__stmr_key_not_push_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_key_not_found_interrupt: 0x" << int_var__stmr_key_not_found_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_stall_interrupt: 0x" << int_var__stmr_stall_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_stall_interrupt: 0x" << int_var__ftmr_stall_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_ovf_interrupt: 0x" << int_var__ftmr_fifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_ovf_interrupt: 0x" << int_var__stmr_fifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_efull_interrupt: 0x" << int_var__ftmr_fifo_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_efull_interrupt: 0x" << int_var__stmr_fifo_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rejct_drb_ovf_interrupt: 0x" << int_var__tmr_rejct_drb_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rejct_drb_efull_interrupt: 0x" << int_var__tmr_rejct_drb_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_re_efull_interrupt: 0x" << int_var__tmr_hbm_re_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_we_efull_interrupt: 0x" << int_var__tmr_hbm_we_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_byp_pending_efull_interrupt: 0x" << int_var__tmr_hbm_byp_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_rd_pending_efull_interrupt: 0x" << int_var__tmr_hbm_rd_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hbm_wr_pending_efull_interrupt: 0x" << int_var__tmr_hbm_wr_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_bresp_err_interrupt: 0x" << int_var__tmr_bresp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_bid_err_interrupt: 0x" << int_var__tmr_bid_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rresp_err_interrupt: 0x" << int_var__tmr_rresp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_rid_err_interrupt: 0x" << int_var__tmr_rid_err_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_tmr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_sch_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sch_wid_empty_enable: 0x" << int_var__sch_wid_empty_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rid_empty_enable: 0x" << int_var__sch_rid_empty_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_wr_txfifo_ovf_enable: 0x" << int_var__sch_wr_txfifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rd_txfifo_ovf_enable: 0x" << int_var__sch_rd_txfifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_state_fifo_ovf_enable: 0x" << int_var__sch_state_fifo_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_qid_fifo_enable: 0x" << int_var__sch_drb_cnt_qid_fifo_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_ovf_enable: 0x" << int_var__sch_hbm_byp_ovf_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_wtag_wrap_enable: 0x" << int_var__sch_hbm_byp_wtag_wrap_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_ovfl_enable: 0x" << int_var__sch_rlid_ovfl_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_unfl_enable: 0x" << int_var__sch_rlid_unfl_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_null_lif_enable: 0x" << int_var__sch_null_lif_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_sg_mismatch_enable: 0x" << int_var__sch_lif_sg_mismatch_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_aclr_hbm_ln_rollovr_enable: 0x" << int_var__sch_aclr_hbm_ln_rollovr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_txdma_msg_efull_enable: 0x" << int_var__sch_txdma_msg_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_re_efull_enable: 0x" << int_var__sch_hbm_re_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_we_efull_enable: 0x" << int_var__sch_hbm_we_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_pending_efull_enable: 0x" << int_var__sch_hbm_byp_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_rd_pending_efull_enable: 0x" << int_var__sch_hbm_rd_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_wr_pending_efull_enable: 0x" << int_var__sch_hbm_wr_pending_efull_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_bresp_err_enable: 0x" << int_var__sch_bresp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_bid_err_enable: 0x" << int_var__sch_bid_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rresp_err_enable: 0x" << int_var__sch_rresp_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rid_err_enable: 0x" << int_var__sch_rid_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_ovfl_enable: 0x" << int_var__sch_drb_cnt_ovfl_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_unfl_enable: 0x" << int_var__sch_drb_cnt_unfl_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_txdma_msg_ovfl_enable: 0x" << int_var__sch_txdma_msg_ovfl_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_sch_int_test_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sch_wid_empty_interrupt: 0x" << int_var__sch_wid_empty_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rid_empty_interrupt: 0x" << int_var__sch_rid_empty_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_wr_txfifo_ovf_interrupt: 0x" << int_var__sch_wr_txfifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rd_txfifo_ovf_interrupt: 0x" << int_var__sch_rd_txfifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_state_fifo_ovf_interrupt: 0x" << int_var__sch_state_fifo_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_qid_fifo_interrupt: 0x" << int_var__sch_drb_cnt_qid_fifo_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_ovf_interrupt: 0x" << int_var__sch_hbm_byp_ovf_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_wtag_wrap_interrupt: 0x" << int_var__sch_hbm_byp_wtag_wrap_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_ovfl_interrupt: 0x" << int_var__sch_rlid_ovfl_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_unfl_interrupt: 0x" << int_var__sch_rlid_unfl_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_null_lif_interrupt: 0x" << int_var__sch_null_lif_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_sg_mismatch_interrupt: 0x" << int_var__sch_lif_sg_mismatch_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_aclr_hbm_ln_rollovr_interrupt: 0x" << int_var__sch_aclr_hbm_ln_rollovr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_txdma_msg_efull_interrupt: 0x" << int_var__sch_txdma_msg_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_re_efull_interrupt: 0x" << int_var__sch_hbm_re_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_we_efull_interrupt: 0x" << int_var__sch_hbm_we_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_byp_pending_efull_interrupt: 0x" << int_var__sch_hbm_byp_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_rd_pending_efull_interrupt: 0x" << int_var__sch_hbm_rd_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_hbm_wr_pending_efull_interrupt: 0x" << int_var__sch_hbm_wr_pending_efull_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_bresp_err_interrupt: 0x" << int_var__sch_bresp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_bid_err_interrupt: 0x" << int_var__sch_bid_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rresp_err_interrupt: 0x" << int_var__sch_rresp_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rid_err_interrupt: 0x" << int_var__sch_rid_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_ovfl_interrupt: 0x" << int_var__sch_drb_cnt_ovfl_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_drb_cnt_unfl_interrupt: 0x" << int_var__sch_drb_cnt_unfl_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_txdma_msg_ovfl_interrupt: 0x" << int_var__sch_txdma_msg_ovfl_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_sch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_sch_interrupt: 0x" << int_var__int_sch_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_tmr_interrupt: 0x" << int_var__int_tmr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_srams_ecc_interrupt: 0x" << int_var__int_srams_ecc_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_sch_enable: 0x" << int_var__int_sch_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_tmr_enable: 0x" << int_var__int_tmr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_srams_ecc_enable: 0x" << int_var__int_srams_ecc_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_srams_ecc_int_enable_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_cnt_uncorrectable_enable: 0x" << int_var__tmr_cnt_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_cnt_correctable_enable: 0x" << int_var__tmr_cnt_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_map_uncorrectable_enable: 0x" << int_var__sch_lif_map_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_map_correctable_enable: 0x" << int_var__sch_lif_map_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_map_uncorrectable_enable: 0x" << int_var__sch_rlid_map_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_map_correctable_enable: 0x" << int_var__sch_rlid_map_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_grp_uncorrectable_enable: 0x" << int_var__sch_grp_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_grp_correctable_enable: 0x" << int_var__sch_grp_correctable_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_srams_ecc_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_cnt_uncorrectable_interrupt: 0x" << int_var__tmr_cnt_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_cnt_correctable_interrupt: 0x" << int_var__tmr_cnt_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_map_uncorrectable_interrupt: 0x" << int_var__sch_lif_map_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_lif_map_correctable_interrupt: 0x" << int_var__sch_lif_map_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_map_uncorrectable_interrupt: 0x" << int_var__sch_rlid_map_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_rlid_map_correctable_interrupt: 0x" << int_var__sch_rlid_map_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_grp_uncorrectable_interrupt: 0x" << int_var__sch_grp_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_grp_correctable_interrupt: 0x" << int_var__sch_grp_correctable_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_int_srams_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".clr_pend_cnt: 0x" << int_var__clr_pend_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rl_thr: 0x" << int_var__rl_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bckgr_cnt: 0x" << int_var__bckgr_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bckgr: 0x" << int_var__bckgr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_mode: 0x" << int_var__hbm_mode << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_offset: 0x" << int_var__qid_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".auto_clr: 0x" << int_var__auto_clr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rr_sel: 0x" << int_var__rr_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm0_vld: 0x" << int_var__qid_fifo_elm0_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm0_jnk: 0x" << int_var__qid_fifo_elm0_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm0_qid: 0x" << int_var__qid_fifo_elm0_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm1_vld: 0x" << int_var__qid_fifo_elm1_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm1_jnk: 0x" << int_var__qid_fifo_elm1_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm1_qid: 0x" << int_var__qid_fifo_elm1_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm2_vld: 0x" << int_var__qid_fifo_elm2_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm2_jnk: 0x" << int_var__qid_fifo_elm2_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm2_qid: 0x" << int_var__qid_fifo_elm2_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm3_vld: 0x" << int_var__qid_fifo_elm3_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm3_jnk: 0x" << int_var__qid_fifo_elm3_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm3_qid: 0x" << int_var__qid_fifo_elm3_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm4_vld: 0x" << int_var__qid_fifo_elm4_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm4_jnk: 0x" << int_var__qid_fifo_elm4_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm4_qid: 0x" << int_var__qid_fifo_elm4_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm5_vld: 0x" << int_var__qid_fifo_elm5_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm5_jnk: 0x" << int_var__qid_fifo_elm5_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm5_qid: 0x" << int_var__qid_fifo_elm5_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm6_vld: 0x" << int_var__qid_fifo_elm6_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm6_jnk: 0x" << int_var__qid_fifo_elm6_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm6_qid: 0x" << int_var__qid_fifo_elm6_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm7_vld: 0x" << int_var__qid_fifo_elm7_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm7_jnk: 0x" << int_var__qid_fifo_elm7_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm7_qid: 0x" << int_var__qid_fifo_elm7_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm8_vld: 0x" << int_var__qid_fifo_elm8_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm8_jnk: 0x" << int_var__qid_fifo_elm8_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm8_qid: 0x" << int_var__qid_fifo_elm8_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm9_vld: 0x" << int_var__qid_fifo_elm9_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm9_jnk: 0x" << int_var__qid_fifo_elm9_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm9_qid: 0x" << int_var__qid_fifo_elm9_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm10_vld: 0x" << int_var__qid_fifo_elm10_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm10_jnk: 0x" << int_var__qid_fifo_elm10_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm10_qid: 0x" << int_var__qid_fifo_elm10_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm11_vld: 0x" << int_var__qid_fifo_elm11_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm11_jnk: 0x" << int_var__qid_fifo_elm11_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm11_qid: 0x" << int_var__qid_fifo_elm11_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm12_vld: 0x" << int_var__qid_fifo_elm12_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm12_jnk: 0x" << int_var__qid_fifo_elm12_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm12_qid: 0x" << int_var__qid_fifo_elm12_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm13_vld: 0x" << int_var__qid_fifo_elm13_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm13_jnk: 0x" << int_var__qid_fifo_elm13_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm13_qid: 0x" << int_var__qid_fifo_elm13_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm14_vld: 0x" << int_var__qid_fifo_elm14_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm14_jnk: 0x" << int_var__qid_fifo_elm14_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm14_qid: 0x" << int_var__qid_fifo_elm14_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm15_vld: 0x" << int_var__qid_fifo_elm15_vld << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm15_jnk: 0x" << int_var__qid_fifo_elm15_jnk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_fifo_elm15_qid: 0x" << int_var__qid_fifo_elm15_qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln: 0x" << int_var__hbm_ln << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_ptr: 0x" << int_var__hbm_ln_ptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_rr_cnt: 0x" << int_var__hbm_rr_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drb_cnt: 0x" << int_var__drb_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt0: 0x" << int_var__hbm_ln_cnt0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt1: 0x" << int_var__hbm_ln_cnt1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt2: 0x" << int_var__hbm_ln_cnt2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt3: 0x" << int_var__hbm_ln_cnt3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt4: 0x" << int_var__hbm_ln_cnt4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt5: 0x" << int_var__hbm_ln_cnt5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt6: 0x" << int_var__hbm_ln_cnt6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt7: 0x" << int_var__hbm_ln_cnt7 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt8: 0x" << int_var__hbm_ln_cnt8 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt9: 0x" << int_var__hbm_ln_cnt9 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt10: 0x" << int_var__hbm_ln_cnt10 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt11: 0x" << int_var__hbm_ln_cnt11 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt12: 0x" << int_var__hbm_ln_cnt12 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt13: 0x" << int_var__hbm_ln_cnt13 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt14: 0x" << int_var__hbm_ln_cnt14 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_ln_cnt15: 0x" << int_var__hbm_ln_cnt15 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".popcnt: 0x" << int_var__popcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_start: 0x" << int_var__sg_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_end: 0x" << int_var__sg_end << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_rlid_map_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_active: 0x" << int_var__sg_active << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_start: 0x" << int_var__sg_start << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_per_cos: 0x" << int_var__sg_per_cos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sg_act_cos: 0x" << int_var__sg_act_cos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_lif_map_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slow_cbcnt: 0x" << int_var__slow_cbcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slow_bcnt: 0x" << int_var__slow_bcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".slow_lcnt: 0x" << int_var__slow_lcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fast_cbcnt: 0x" << int_var__fast_cbcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fast_bcnt: 0x" << int_var__fast_bcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fast_lcnt: 0x" << int_var__fast_lcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_tmr_cnt_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_rlid_stop_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".setmsk: 0x" << int_var__setmsk << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_rlid_stop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".popcnt: 0x" << int_var__popcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drb_cnt: 0x" << int_var__drb_cnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_cnt_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_entry_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_offset: 0x" << int_var__qid_offset << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".auto_clr: 0x" << int_var__auto_clr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rr_sel: 0x" << int_var__rr_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rl_thr: 0x" << int_var__rl_thr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_grp_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_doorbell_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cos: 0x" << int_var__cos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".set: 0x" << int_var__set << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_doorbell_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_flags_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sch_flags: 0x" << int_var__sch_flags << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_sch_flags_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_dtdmhi_calendar_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dtdm_calendar: 0x" << int_var__dtdm_calendar << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_dtdmhi_calendar_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_dtdmlo_calendar_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dtdm_calendar: 0x" << int_var__dtdm_calendar << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_dtdmlo_calendar_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_slow_timer_pending_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bcnt: 0x" << int_var__bcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lcnt: 0x" << int_var__lcnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_slow_timer_pending_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".typ: 0x" << int_var__typ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ring: 0x" << int_var__ring << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dtime: 0x" << int_var__dtime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_slow_timer_start_no_stop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_fast_timer_pending_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bcnt: 0x" << int_var__bcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lcnt: 0x" << int_var__lcnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_fast_timer_pending_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".typ: 0x" << int_var__typ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ring: 0x" << int_var__ring << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dtime: 0x" << int_var__dtime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_dhs_fast_timer_start_no_stop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_bid_uexp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_bid_rerr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_bid_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_bid_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_wr_req_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_rd_rsp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_tmr_axi_rd_req_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_bid_uexp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_bid_rerr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_bid_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_bid_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_wr_req_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_rd_rsp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_axi_rd_req_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_stmr_max_bcnt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bcnt: 0x" << int_var__bcnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_ftmr_max_bcnt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bcnt: 0x" << int_var__bcnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_sch_hbm_byp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_sch_hbm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_hbm_byp_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_hbm_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_ecc_sch_grp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_scheduler_dbg2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_byp_pending_cnt: 0x" << int_var__hbm_byp_pending_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_rd_pending_cnt: 0x" << int_var__hbm_rd_pending_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_wr_pending_cnt: 0x" << int_var__hbm_wr_pending_cnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_scheduler_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".txdma_drdy: 0x" << int_var__txdma_drdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_re_efull: 0x" << int_var__hbm_re_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_we_efull: 0x" << int_var__hbm_we_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txdma_msg_efull: 0x" << int_var__txdma_msg_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_byp_pending_efull: 0x" << int_var__hbm_byp_pending_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_rd_pending_efull: 0x" << int_var__hbm_rd_pending_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_wr_pending_efull: 0x" << int_var__hbm_wr_pending_efull << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_sch_lif_sg_mismatch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".set: 0x" << int_var__set << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_sch_lif_map_notactive_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cos: 0x" << int_var__cos << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".set: 0x" << int_var__set << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_grp_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_rlid_map_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_lif_map_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_tmr_cnt_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs_eccbypass: 0x" << int_var__dhs_eccbypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_sch_max_hbm_byp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_scheduler_dbg2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".qid_read: 0x" << int_var__qid_read << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cos_read: 0x" << int_var__cos_read << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_scheduler_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_efc_thr: 0x" << int_var__hbm_efc_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".txdma_efc_thr: 0x" << int_var__txdma_efc_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_mode_thr: 0x" << int_var__fifo_mode_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_byp: 0x" << int_var__max_hbm_byp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_wr: 0x" << int_var__max_hbm_wr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_rd: 0x" << int_var__max_hbm_rd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_scheduler_rr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr0: 0x" << int_var__curr_ptr0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr1: 0x" << int_var__curr_ptr1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr2: 0x" << int_var__curr_ptr2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr3: 0x" << int_var__curr_ptr3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr4: 0x" << int_var__curr_ptr4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr5: 0x" << int_var__curr_ptr5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr6: 0x" << int_var__curr_ptr6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr7: 0x" << int_var__curr_ptr7 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr8: 0x" << int_var__curr_ptr8 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr9: 0x" << int_var__curr_ptr9 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr10: 0x" << int_var__curr_ptr10 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr11: 0x" << int_var__curr_ptr11 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr12: 0x" << int_var__curr_ptr12 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr13: 0x" << int_var__curr_ptr13 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr14: 0x" << int_var__curr_ptr14 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".curr_ptr15: 0x" << int_var__curr_ptr15 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_scheduler_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_init_done: 0x" << int_var__hbm_init_done << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram_init_done: 0x" << int_var__sram_init_done << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_rlid_start_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_rlid_stop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_glb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".pb_xoff: 0x" << int_var__pb_xoff << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos15_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos14_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos13_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos12_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos11_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos10_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos9_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos8_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos7_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos6_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos5_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos4_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos3_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_txdma_cos0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_fifo_empty_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_doorbell_clr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_sch_doorbell_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_sch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dtdm_lo_map: 0x" << int_var__dtdm_lo_map << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dtdm_hi_map: 0x" << int_var__dtdm_hi_map << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".timeout: 0x" << int_var__timeout << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pause: 0x" << int_var__pause << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfw_scheduler_static_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_base: 0x" << int_var__hbm_base << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sch_grp_depth: 0x" << int_var__sch_grp_depth << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfw_scheduler_glb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_hw_init: 0x" << int_var__hbm_hw_init << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram_hw_init: 0x" << int_var__sram_hw_init << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_set_lkup: 0x" << int_var__enable_set_lkup << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_set_byp: 0x" << int_var__enable_set_byp << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_tmr_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_tmr_hbm_byp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_srams_tmr_hbm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_tmr_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_tmr_hbm_byp_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_tmr_hbm_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_stmr_pop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_stmr_key_not_found_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_stmr_push_out_of_wheel_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_stmr_key_not_push_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_stmr_push_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_slow_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tick: 0x" << int_var__tick << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cTime: 0x" << int_var__cTime << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pTime: 0x" << int_var__pTime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_slow_timer_dbell_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addr_update: 0x" << int_var__addr_update << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_pid: 0x" << int_var__data_pid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_reserved: 0x" << int_var__data_reserved << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_index: 0x" << int_var__data_index << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_force_slow_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ctime: 0x" << int_var__ctime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_slow_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tick: 0x" << int_var__tick << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_sel: 0x" << int_var__hash_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_ftmr_pop_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_ftmr_key_not_found_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_ftmr_key_not_push_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cnt_ftmr_push_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".val: 0x" << int_var__val << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_fast_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tick: 0x" << int_var__tick << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cTime: 0x" << int_var__cTime << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pTime: 0x" << int_var__pTime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_fast_timer_dbell_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addr_update: 0x" << int_var__addr_update << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_pid: 0x" << int_var__data_pid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_reserved: 0x" << int_var__data_reserved << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data_index: 0x" << int_var__data_index << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_force_fast_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ctime: 0x" << int_var__ctime << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_fast_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tick: 0x" << int_var__tick << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hash_sel: 0x" << int_var__hash_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_timer_dbg2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_byp_pending_cnt: 0x" << int_var__hbm_byp_pending_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_rd_pending_cnt: 0x" << int_var__hbm_rd_pending_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_wr_pending_cnt: 0x" << int_var__hbm_wr_pending_cnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_stall: 0x" << int_var__stmr_stall << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_stall: 0x" << int_var__ftmr_stall << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_timer_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_re_efull: 0x" << int_var__hbm_re_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_we_efull: 0x" << int_var__hbm_we_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rejct_drb_efull: 0x" << int_var__rejct_drb_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_byp_pending_efull: 0x" << int_var__hbm_byp_pending_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_rd_pending_efull: 0x" << int_var__hbm_rd_pending_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_wr_pending_efull: 0x" << int_var__hbm_wr_pending_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_raddr: 0x" << int_var__stmr_fifo_raddr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_raddr: 0x" << int_var__ftmr_fifo_raddr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_waddr: 0x" << int_var__stmr_fifo_waddr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_waddr: 0x" << int_var__ftmr_fifo_waddr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_fifo_efull: 0x" << int_var__stmr_fifo_efull << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_fifo_efull: 0x" << int_var__ftmr_fifo_efull << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_tmr_max_keys_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_tmr_max_hbm_byp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_sta_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_init_done: 0x" << int_var__hbm_init_done << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram_init_done: 0x" << int_var__sram_init_done << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_timer_dbg2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".max_bcnt: 0x" << int_var__max_bcnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_timer_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_efc_thr: 0x" << int_var__hbm_efc_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drb_efc_thr: 0x" << int_var__drb_efc_thr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_stall_thr_hi: 0x" << int_var__tmr_stall_thr_hi << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_stall_thr_lo: 0x" << int_var__tmr_stall_thr_lo << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_wr: 0x" << int_var__max_hbm_wr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_rd: 0x" << int_var__max_hbm_rd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_hbm_byp: 0x" << int_var__max_hbm_byp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_tmr_fifo: 0x" << int_var__max_tmr_fifo << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfw_timer_glb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_hw_init: 0x" << int_var__hbm_hw_init << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram_hw_init: 0x" << int_var__sram_hw_init << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_enable: 0x" << int_var__ftmr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_enable: 0x" << int_var__stmr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ftmr_pause: 0x" << int_var__ftmr_pause << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stmr_pause: 0x" << int_var__stmr_pause << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_timer_static_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".hbm_base: 0x" << int_var__hbm_base << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_hsh_depth: 0x" << int_var__tmr_hsh_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tmr_wheel_depth: 0x" << int_var__tmr_wheel_depth << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_cfg_axi_attr_t::show() {
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
void cap_txs_csr_cfg_glb_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dbg_port_select: 0x" << int_var__dbg_port_select << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dbg_port_enable: 0x" << int_var__dbg_port_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_txs_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_glb.show();
    cfg_axi_attr.show();
    cfg_timer_static.show();
    cfw_timer_glb.show();
    cfg_timer_dbg.show();
    cfg_timer_dbg2.show();
    sta_timer.show();
    sta_tmr_max_hbm_byp.show();
    sta_tmr_max_keys.show();
    sta_timer_dbg.show();
    sta_timer_dbg2.show();
    cfg_fast_timer.show();
    cfg_force_fast_timer.show();
    cfg_fast_timer_dbell.show();
    sta_fast_timer.show();
    cnt_ftmr_push.show();
    cnt_ftmr_key_not_push.show();
    cnt_ftmr_push_out_of_wheel.show();
    cnt_ftmr_key_not_found.show();
    cnt_ftmr_pop.show();
    cfg_slow_timer.show();
    cfg_force_slow_timer.show();
    cfg_slow_timer_dbell.show();
    sta_slow_timer.show();
    cnt_stmr_push.show();
    cnt_stmr_key_not_push.show();
    cnt_stmr_push_out_of_wheel.show();
    cnt_stmr_key_not_found.show();
    cnt_stmr_pop.show();
    cfg_tmr_hbm_sram.show();
    cfg_tmr_hbm_byp_sram.show();
    cfg_tmr_fifo_sram.show();
    sta_srams_tmr_hbm.show();
    sta_srams_tmr_hbm_byp.show();
    sta_srams_tmr_fifo.show();
    cfw_scheduler_glb.show();
    cfw_scheduler_static.show();
    cfg_sch.show();
    cnt_sch_doorbell_set.show();
    cnt_sch_doorbell_clr.show();
    cnt_sch_fifo_empty.show();
    cnt_sch_txdma_cos0.show();
    cnt_sch_txdma_cos1.show();
    cnt_sch_txdma_cos2.show();
    cnt_sch_txdma_cos3.show();
    cnt_sch_txdma_cos4.show();
    cnt_sch_txdma_cos5.show();
    cnt_sch_txdma_cos6.show();
    cnt_sch_txdma_cos7.show();
    cnt_sch_txdma_cos8.show();
    cnt_sch_txdma_cos9.show();
    cnt_sch_txdma_cos10.show();
    cnt_sch_txdma_cos11.show();
    cnt_sch_txdma_cos12.show();
    cnt_sch_txdma_cos13.show();
    cnt_sch_txdma_cos14.show();
    cnt_sch_txdma_cos15.show();
    sta_glb.show();
    cnt_sch_rlid_stop.show();
    cnt_sch_rlid_start.show();
    sta_scheduler.show();
    sta_scheduler_rr.show();
    cfg_scheduler_dbg.show();
    cfg_scheduler_dbg2.show();
    sta_sch_max_hbm_byp.show();
    cfg_tmr_cnt_sram.show();
    cfg_sch_lif_map_sram.show();
    cfg_sch_rlid_map_sram.show();
    cfg_sch_grp_sram.show();
    sta_sch_lif_map_notactive.show();
    sta_sch_lif_sg_mismatch.show();
    sta_scheduler_dbg.show();
    sta_scheduler_dbg2.show();
    sta_srams_ecc_tmr_cnt.show();
    sta_srams_ecc_sch_lif_map.show();
    sta_srams_ecc_sch_rlid_map.show();
    sta_srams_ecc_sch_grp.show();
    cfg_sch_hbm_sram.show();
    cfg_sch_hbm_byp_sram.show();
    sta_srams_sch_hbm.show();
    sta_srams_sch_hbm_byp.show();
    csr_intr.show();
    sta_ftmr_max_bcnt.show();
    sta_stmr_max_bcnt.show();
    cnt_sch_axi_rd_req.show();
    cnt_sch_axi_rd_rsp.show();
    cnt_sch_axi_rd_rsp_err.show();
    cnt_sch_axi_rd_rsp_rerr.show();
    cnt_sch_axi_rd_rsp_uexp.show();
    cnt_sch_axi_wr_req.show();
    cnt_sch_axi_bid.show();
    cnt_sch_axi_bid_err.show();
    cnt_sch_axi_bid_rerr.show();
    cnt_sch_axi_bid_uexp.show();
    cnt_tmr_axi_rd_req.show();
    cnt_tmr_axi_rd_rsp.show();
    cnt_tmr_axi_rd_rsp_err.show();
    cnt_tmr_axi_rd_rsp_rerr.show();
    cnt_tmr_axi_rd_rsp_uexp.show();
    cnt_tmr_axi_wr_req.show();
    cnt_tmr_axi_bid.show();
    cnt_tmr_axi_bid_err.show();
    cnt_tmr_axi_bid_rerr.show();
    cnt_tmr_axi_bid_uexp.show();
    dhs_fast_timer_start_no_stop.show();
    dhs_fast_timer_pending.show();
    dhs_slow_timer_start_no_stop.show();
    dhs_slow_timer_pending.show();
    dhs_dtdmlo_calendar.show();
    dhs_dtdmhi_calendar.show();
    dhs_sch_flags.show();
    dhs_doorbell.show();
    dhs_sch_grp_entry.show();
    dhs_sch_grp_cnt_entry.show();
    dhs_rlid_stop.show();
    dhs_tmr_cnt_sram.show();
    dhs_sch_lif_map_sram.show();
    dhs_sch_rlid_map_sram.show();
    dhs_sch_grp_sram.show();
    int_srams_ecc.show();
    int_groups.show();
    int_sch.show();
    int_tmr.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_txs_csr_int_tmr_int_enable_clear_t::get_width() const {
    return cap_txs_csr_int_tmr_int_enable_clear_t::s_get_width();

}

int cap_txs_csr_int_tmr_int_test_set_t::get_width() const {
    return cap_txs_csr_int_tmr_int_test_set_t::s_get_width();

}

int cap_txs_csr_int_tmr_t::get_width() const {
    return cap_txs_csr_int_tmr_t::s_get_width();

}

int cap_txs_csr_int_sch_int_enable_clear_t::get_width() const {
    return cap_txs_csr_int_sch_int_enable_clear_t::s_get_width();

}

int cap_txs_csr_int_sch_int_test_set_t::get_width() const {
    return cap_txs_csr_int_sch_int_test_set_t::s_get_width();

}

int cap_txs_csr_int_sch_t::get_width() const {
    return cap_txs_csr_int_sch_t::s_get_width();

}

int cap_txs_csr_intreg_status_t::get_width() const {
    return cap_txs_csr_intreg_status_t::s_get_width();

}

int cap_txs_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_txs_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_txs_csr_intgrp_status_t::get_width() const {
    return cap_txs_csr_intgrp_status_t::s_get_width();

}

int cap_txs_csr_int_srams_ecc_int_enable_set_t::get_width() const {
    return cap_txs_csr_int_srams_ecc_int_enable_set_t::s_get_width();

}

int cap_txs_csr_int_srams_ecc_intreg_t::get_width() const {
    return cap_txs_csr_int_srams_ecc_intreg_t::s_get_width();

}

int cap_txs_csr_int_srams_ecc_t::get_width() const {
    return cap_txs_csr_int_srams_ecc_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_sram_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_sram_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_sram_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_sram_t::s_get_width();

}

int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_rlid_map_sram_t::get_width() const {
    return cap_txs_csr_dhs_sch_rlid_map_sram_t::s_get_width();

}

int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_lif_map_sram_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_lif_map_sram_t::get_width() const {
    return cap_txs_csr_dhs_sch_lif_map_sram_t::s_get_width();

}

int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::get_width() const {
    return cap_txs_csr_dhs_tmr_cnt_sram_entry_t::s_get_width();

}

int cap_txs_csr_dhs_tmr_cnt_sram_t::get_width() const {
    return cap_txs_csr_dhs_tmr_cnt_sram_t::s_get_width();

}

int cap_txs_csr_dhs_rlid_stop_entry_t::get_width() const {
    return cap_txs_csr_dhs_rlid_stop_entry_t::s_get_width();

}

int cap_txs_csr_dhs_rlid_stop_t::get_width() const {
    return cap_txs_csr_dhs_rlid_stop_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_cnt_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_cnt_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_entry_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_entry_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_grp_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_grp_entry_t::s_get_width();

}

int cap_txs_csr_dhs_doorbell_entry_t::get_width() const {
    return cap_txs_csr_dhs_doorbell_entry_t::s_get_width();

}

int cap_txs_csr_dhs_doorbell_t::get_width() const {
    return cap_txs_csr_dhs_doorbell_t::s_get_width();

}

int cap_txs_csr_dhs_sch_flags_entry_t::get_width() const {
    return cap_txs_csr_dhs_sch_flags_entry_t::s_get_width();

}

int cap_txs_csr_dhs_sch_flags_t::get_width() const {
    return cap_txs_csr_dhs_sch_flags_t::s_get_width();

}

int cap_txs_csr_dhs_dtdmhi_calendar_entry_t::get_width() const {
    return cap_txs_csr_dhs_dtdmhi_calendar_entry_t::s_get_width();

}

int cap_txs_csr_dhs_dtdmhi_calendar_t::get_width() const {
    return cap_txs_csr_dhs_dtdmhi_calendar_t::s_get_width();

}

int cap_txs_csr_dhs_dtdmlo_calendar_entry_t::get_width() const {
    return cap_txs_csr_dhs_dtdmlo_calendar_entry_t::s_get_width();

}

int cap_txs_csr_dhs_dtdmlo_calendar_t::get_width() const {
    return cap_txs_csr_dhs_dtdmlo_calendar_t::s_get_width();

}

int cap_txs_csr_dhs_slow_timer_pending_entry_t::get_width() const {
    return cap_txs_csr_dhs_slow_timer_pending_entry_t::s_get_width();

}

int cap_txs_csr_dhs_slow_timer_pending_t::get_width() const {
    return cap_txs_csr_dhs_slow_timer_pending_t::s_get_width();

}

int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::get_width() const {
    return cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::s_get_width();

}

int cap_txs_csr_dhs_slow_timer_start_no_stop_t::get_width() const {
    return cap_txs_csr_dhs_slow_timer_start_no_stop_t::s_get_width();

}

int cap_txs_csr_dhs_fast_timer_pending_entry_t::get_width() const {
    return cap_txs_csr_dhs_fast_timer_pending_entry_t::s_get_width();

}

int cap_txs_csr_dhs_fast_timer_pending_t::get_width() const {
    return cap_txs_csr_dhs_fast_timer_pending_t::s_get_width();

}

int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::get_width() const {
    return cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::s_get_width();

}

int cap_txs_csr_dhs_fast_timer_start_no_stop_t::get_width() const {
    return cap_txs_csr_dhs_fast_timer_start_no_stop_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_bid_uexp_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_bid_uexp_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_bid_rerr_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_bid_rerr_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_bid_err_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_bid_err_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_bid_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_bid_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_wr_req_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_wr_req_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_rd_rsp_t::s_get_width();

}

int cap_txs_csr_cnt_tmr_axi_rd_req_t::get_width() const {
    return cap_txs_csr_cnt_tmr_axi_rd_req_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_bid_uexp_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_bid_uexp_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_bid_rerr_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_bid_rerr_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_bid_err_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_bid_err_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_bid_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_bid_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_wr_req_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_wr_req_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_rd_rsp_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_rd_rsp_t::s_get_width();

}

int cap_txs_csr_cnt_sch_axi_rd_req_t::get_width() const {
    return cap_txs_csr_cnt_sch_axi_rd_req_t::s_get_width();

}

int cap_txs_csr_sta_stmr_max_bcnt_t::get_width() const {
    return cap_txs_csr_sta_stmr_max_bcnt_t::s_get_width();

}

int cap_txs_csr_sta_ftmr_max_bcnt_t::get_width() const {
    return cap_txs_csr_sta_ftmr_max_bcnt_t::s_get_width();

}

int cap_txs_csr_csr_intr_t::get_width() const {
    return cap_txs_csr_csr_intr_t::s_get_width();

}

int cap_txs_csr_sta_srams_sch_hbm_byp_t::get_width() const {
    return cap_txs_csr_sta_srams_sch_hbm_byp_t::s_get_width();

}

int cap_txs_csr_sta_srams_sch_hbm_t::get_width() const {
    return cap_txs_csr_sta_srams_sch_hbm_t::s_get_width();

}

int cap_txs_csr_cfg_sch_hbm_byp_sram_t::get_width() const {
    return cap_txs_csr_cfg_sch_hbm_byp_sram_t::s_get_width();

}

int cap_txs_csr_cfg_sch_hbm_sram_t::get_width() const {
    return cap_txs_csr_cfg_sch_hbm_sram_t::s_get_width();

}

int cap_txs_csr_sta_srams_ecc_sch_grp_t::get_width() const {
    return cap_txs_csr_sta_srams_ecc_sch_grp_t::s_get_width();

}

int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::get_width() const {
    return cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::s_get_width();

}

int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::get_width() const {
    return cap_txs_csr_sta_srams_ecc_sch_lif_map_t::s_get_width();

}

int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::get_width() const {
    return cap_txs_csr_sta_srams_ecc_tmr_cnt_t::s_get_width();

}

int cap_txs_csr_sta_scheduler_dbg2_t::get_width() const {
    return cap_txs_csr_sta_scheduler_dbg2_t::s_get_width();

}

int cap_txs_csr_sta_scheduler_dbg_t::get_width() const {
    return cap_txs_csr_sta_scheduler_dbg_t::s_get_width();

}

int cap_txs_csr_sta_sch_lif_sg_mismatch_t::get_width() const {
    return cap_txs_csr_sta_sch_lif_sg_mismatch_t::s_get_width();

}

int cap_txs_csr_sta_sch_lif_map_notactive_t::get_width() const {
    return cap_txs_csr_sta_sch_lif_map_notactive_t::s_get_width();

}

int cap_txs_csr_cfg_sch_grp_sram_t::get_width() const {
    return cap_txs_csr_cfg_sch_grp_sram_t::s_get_width();

}

int cap_txs_csr_cfg_sch_rlid_map_sram_t::get_width() const {
    return cap_txs_csr_cfg_sch_rlid_map_sram_t::s_get_width();

}

int cap_txs_csr_cfg_sch_lif_map_sram_t::get_width() const {
    return cap_txs_csr_cfg_sch_lif_map_sram_t::s_get_width();

}

int cap_txs_csr_cfg_tmr_cnt_sram_t::get_width() const {
    return cap_txs_csr_cfg_tmr_cnt_sram_t::s_get_width();

}

int cap_txs_csr_sta_sch_max_hbm_byp_t::get_width() const {
    return cap_txs_csr_sta_sch_max_hbm_byp_t::s_get_width();

}

int cap_txs_csr_cfg_scheduler_dbg2_t::get_width() const {
    return cap_txs_csr_cfg_scheduler_dbg2_t::s_get_width();

}

int cap_txs_csr_cfg_scheduler_dbg_t::get_width() const {
    return cap_txs_csr_cfg_scheduler_dbg_t::s_get_width();

}

int cap_txs_csr_sta_scheduler_rr_t::get_width() const {
    return cap_txs_csr_sta_scheduler_rr_t::s_get_width();

}

int cap_txs_csr_sta_scheduler_t::get_width() const {
    return cap_txs_csr_sta_scheduler_t::s_get_width();

}

int cap_txs_csr_cnt_sch_rlid_start_t::get_width() const {
    return cap_txs_csr_cnt_sch_rlid_start_t::s_get_width();

}

int cap_txs_csr_cnt_sch_rlid_stop_t::get_width() const {
    return cap_txs_csr_cnt_sch_rlid_stop_t::s_get_width();

}

int cap_txs_csr_sta_glb_t::get_width() const {
    return cap_txs_csr_sta_glb_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos15_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos15_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos14_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos14_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos13_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos13_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos12_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos12_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos11_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos11_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos10_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos10_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos9_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos9_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos8_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos8_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos7_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos7_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos6_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos6_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos5_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos5_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos4_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos4_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos3_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos3_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos2_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos2_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos1_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos1_t::s_get_width();

}

int cap_txs_csr_cnt_sch_txdma_cos0_t::get_width() const {
    return cap_txs_csr_cnt_sch_txdma_cos0_t::s_get_width();

}

int cap_txs_csr_cnt_sch_fifo_empty_t::get_width() const {
    return cap_txs_csr_cnt_sch_fifo_empty_t::s_get_width();

}

int cap_txs_csr_cnt_sch_doorbell_clr_t::get_width() const {
    return cap_txs_csr_cnt_sch_doorbell_clr_t::s_get_width();

}

int cap_txs_csr_cnt_sch_doorbell_set_t::get_width() const {
    return cap_txs_csr_cnt_sch_doorbell_set_t::s_get_width();

}

int cap_txs_csr_cfg_sch_t::get_width() const {
    return cap_txs_csr_cfg_sch_t::s_get_width();

}

int cap_txs_csr_cfw_scheduler_static_t::get_width() const {
    return cap_txs_csr_cfw_scheduler_static_t::s_get_width();

}

int cap_txs_csr_cfw_scheduler_glb_t::get_width() const {
    return cap_txs_csr_cfw_scheduler_glb_t::s_get_width();

}

int cap_txs_csr_sta_srams_tmr_fifo_t::get_width() const {
    return cap_txs_csr_sta_srams_tmr_fifo_t::s_get_width();

}

int cap_txs_csr_sta_srams_tmr_hbm_byp_t::get_width() const {
    return cap_txs_csr_sta_srams_tmr_hbm_byp_t::s_get_width();

}

int cap_txs_csr_sta_srams_tmr_hbm_t::get_width() const {
    return cap_txs_csr_sta_srams_tmr_hbm_t::s_get_width();

}

int cap_txs_csr_cfg_tmr_fifo_sram_t::get_width() const {
    return cap_txs_csr_cfg_tmr_fifo_sram_t::s_get_width();

}

int cap_txs_csr_cfg_tmr_hbm_byp_sram_t::get_width() const {
    return cap_txs_csr_cfg_tmr_hbm_byp_sram_t::s_get_width();

}

int cap_txs_csr_cfg_tmr_hbm_sram_t::get_width() const {
    return cap_txs_csr_cfg_tmr_hbm_sram_t::s_get_width();

}

int cap_txs_csr_cnt_stmr_pop_t::get_width() const {
    return cap_txs_csr_cnt_stmr_pop_t::s_get_width();

}

int cap_txs_csr_cnt_stmr_key_not_found_t::get_width() const {
    return cap_txs_csr_cnt_stmr_key_not_found_t::s_get_width();

}

int cap_txs_csr_cnt_stmr_push_out_of_wheel_t::get_width() const {
    return cap_txs_csr_cnt_stmr_push_out_of_wheel_t::s_get_width();

}

int cap_txs_csr_cnt_stmr_key_not_push_t::get_width() const {
    return cap_txs_csr_cnt_stmr_key_not_push_t::s_get_width();

}

int cap_txs_csr_cnt_stmr_push_t::get_width() const {
    return cap_txs_csr_cnt_stmr_push_t::s_get_width();

}

int cap_txs_csr_sta_slow_timer_t::get_width() const {
    return cap_txs_csr_sta_slow_timer_t::s_get_width();

}

int cap_txs_csr_cfg_slow_timer_dbell_t::get_width() const {
    return cap_txs_csr_cfg_slow_timer_dbell_t::s_get_width();

}

int cap_txs_csr_cfg_force_slow_timer_t::get_width() const {
    return cap_txs_csr_cfg_force_slow_timer_t::s_get_width();

}

int cap_txs_csr_cfg_slow_timer_t::get_width() const {
    return cap_txs_csr_cfg_slow_timer_t::s_get_width();

}

int cap_txs_csr_cnt_ftmr_pop_t::get_width() const {
    return cap_txs_csr_cnt_ftmr_pop_t::s_get_width();

}

int cap_txs_csr_cnt_ftmr_key_not_found_t::get_width() const {
    return cap_txs_csr_cnt_ftmr_key_not_found_t::s_get_width();

}

int cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::get_width() const {
    return cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::s_get_width();

}

int cap_txs_csr_cnt_ftmr_key_not_push_t::get_width() const {
    return cap_txs_csr_cnt_ftmr_key_not_push_t::s_get_width();

}

int cap_txs_csr_cnt_ftmr_push_t::get_width() const {
    return cap_txs_csr_cnt_ftmr_push_t::s_get_width();

}

int cap_txs_csr_sta_fast_timer_t::get_width() const {
    return cap_txs_csr_sta_fast_timer_t::s_get_width();

}

int cap_txs_csr_cfg_fast_timer_dbell_t::get_width() const {
    return cap_txs_csr_cfg_fast_timer_dbell_t::s_get_width();

}

int cap_txs_csr_cfg_force_fast_timer_t::get_width() const {
    return cap_txs_csr_cfg_force_fast_timer_t::s_get_width();

}

int cap_txs_csr_cfg_fast_timer_t::get_width() const {
    return cap_txs_csr_cfg_fast_timer_t::s_get_width();

}

int cap_txs_csr_sta_timer_dbg2_t::get_width() const {
    return cap_txs_csr_sta_timer_dbg2_t::s_get_width();

}

int cap_txs_csr_sta_timer_dbg_t::get_width() const {
    return cap_txs_csr_sta_timer_dbg_t::s_get_width();

}

int cap_txs_csr_sta_tmr_max_keys_t::get_width() const {
    return cap_txs_csr_sta_tmr_max_keys_t::s_get_width();

}

int cap_txs_csr_sta_tmr_max_hbm_byp_t::get_width() const {
    return cap_txs_csr_sta_tmr_max_hbm_byp_t::s_get_width();

}

int cap_txs_csr_sta_timer_t::get_width() const {
    return cap_txs_csr_sta_timer_t::s_get_width();

}

int cap_txs_csr_cfg_timer_dbg2_t::get_width() const {
    return cap_txs_csr_cfg_timer_dbg2_t::s_get_width();

}

int cap_txs_csr_cfg_timer_dbg_t::get_width() const {
    return cap_txs_csr_cfg_timer_dbg_t::s_get_width();

}

int cap_txs_csr_cfw_timer_glb_t::get_width() const {
    return cap_txs_csr_cfw_timer_glb_t::s_get_width();

}

int cap_txs_csr_cfg_timer_static_t::get_width() const {
    return cap_txs_csr_cfg_timer_static_t::s_get_width();

}

int cap_txs_csr_cfg_axi_attr_t::get_width() const {
    return cap_txs_csr_cfg_axi_attr_t::s_get_width();

}

int cap_txs_csr_cfg_glb_t::get_width() const {
    return cap_txs_csr_cfg_glb_t::s_get_width();

}

int cap_txs_csr_base_t::get_width() const {
    return cap_txs_csr_base_t::s_get_width();

}

int cap_txs_csr_t::get_width() const {
    return cap_txs_csr_t::s_get_width();

}

int cap_txs_csr_int_tmr_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // tmr_wid_empty_enable
    _count += 1; // tmr_rid_empty_enable
    _count += 1; // tmr_wr_txfifo_ovf_enable
    _count += 1; // tmr_rd_txfifo_ovf_enable
    _count += 1; // tmr_state_fifo_ovf_enable
    _count += 1; // tmr_hbm_byp_ovf_enable
    _count += 1; // tmr_hbm_byp_wtag_wrap_enable
    _count += 1; // ftmr_ctime_wrap_enable
    _count += 1; // stmr_ctime_wrap_enable
    _count += 1; // stmr_push_out_of_wheel_enable
    _count += 1; // ftmr_push_out_of_wheel_enable
    _count += 1; // ftmr_key_not_push_enable
    _count += 1; // ftmr_key_not_found_enable
    _count += 1; // stmr_key_not_push_enable
    _count += 1; // stmr_key_not_found_enable
    _count += 1; // stmr_stall_enable
    _count += 1; // ftmr_stall_enable
    _count += 1; // ftmr_fifo_ovf_enable
    _count += 1; // stmr_fifo_ovf_enable
    _count += 1; // ftmr_fifo_efull_enable
    _count += 1; // stmr_fifo_efull_enable
    _count += 1; // tmr_rejct_drb_ovf_enable
    _count += 1; // tmr_rejct_drb_efull_enable
    _count += 1; // tmr_hbm_re_efull_enable
    _count += 1; // tmr_hbm_we_efull_enable
    _count += 1; // tmr_hbm_byp_pending_efull_enable
    _count += 1; // tmr_hbm_rd_pending_efull_enable
    _count += 1; // tmr_hbm_wr_pending_efull_enable
    _count += 1; // tmr_bresp_err_enable
    _count += 1; // tmr_bid_err_enable
    _count += 1; // tmr_rresp_err_enable
    _count += 1; // tmr_rid_err_enable
    return _count;
}

int cap_txs_csr_int_tmr_int_test_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // tmr_wid_empty_interrupt
    _count += 1; // tmr_rid_empty_interrupt
    _count += 1; // tmr_wr_txfifo_ovf_interrupt
    _count += 1; // tmr_rd_txfifo_ovf_interrupt
    _count += 1; // tmr_state_fifo_ovf_interrupt
    _count += 1; // tmr_hbm_byp_ovf_interrupt
    _count += 1; // tmr_hbm_byp_wtag_wrap_interrupt
    _count += 1; // ftmr_ctime_wrap_interrupt
    _count += 1; // stmr_ctime_wrap_interrupt
    _count += 1; // stmr_push_out_of_wheel_interrupt
    _count += 1; // ftmr_push_out_of_wheel_interrupt
    _count += 1; // ftmr_key_not_push_interrupt
    _count += 1; // ftmr_key_not_found_interrupt
    _count += 1; // stmr_key_not_push_interrupt
    _count += 1; // stmr_key_not_found_interrupt
    _count += 1; // stmr_stall_interrupt
    _count += 1; // ftmr_stall_interrupt
    _count += 1; // ftmr_fifo_ovf_interrupt
    _count += 1; // stmr_fifo_ovf_interrupt
    _count += 1; // ftmr_fifo_efull_interrupt
    _count += 1; // stmr_fifo_efull_interrupt
    _count += 1; // tmr_rejct_drb_ovf_interrupt
    _count += 1; // tmr_rejct_drb_efull_interrupt
    _count += 1; // tmr_hbm_re_efull_interrupt
    _count += 1; // tmr_hbm_we_efull_interrupt
    _count += 1; // tmr_hbm_byp_pending_efull_interrupt
    _count += 1; // tmr_hbm_rd_pending_efull_interrupt
    _count += 1; // tmr_hbm_wr_pending_efull_interrupt
    _count += 1; // tmr_bresp_err_interrupt
    _count += 1; // tmr_bid_err_interrupt
    _count += 1; // tmr_rresp_err_interrupt
    _count += 1; // tmr_rid_err_interrupt
    return _count;
}

int cap_txs_csr_int_tmr_t::s_get_width() {
    int _count = 0;

    _count += cap_txs_csr_int_tmr_int_test_set_t::s_get_width(); // intreg
    _count += cap_txs_csr_int_tmr_int_test_set_t::s_get_width(); // int_test_set
    _count += cap_txs_csr_int_tmr_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_txs_csr_int_tmr_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_txs_csr_int_sch_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // sch_wid_empty_enable
    _count += 1; // sch_rid_empty_enable
    _count += 1; // sch_wr_txfifo_ovf_enable
    _count += 1; // sch_rd_txfifo_ovf_enable
    _count += 1; // sch_state_fifo_ovf_enable
    _count += 1; // sch_drb_cnt_qid_fifo_enable
    _count += 1; // sch_hbm_byp_ovf_enable
    _count += 1; // sch_hbm_byp_wtag_wrap_enable
    _count += 1; // sch_rlid_ovfl_enable
    _count += 1; // sch_rlid_unfl_enable
    _count += 1; // sch_null_lif_enable
    _count += 1; // sch_lif_sg_mismatch_enable
    _count += 1; // sch_aclr_hbm_ln_rollovr_enable
    _count += 1; // sch_txdma_msg_efull_enable
    _count += 1; // sch_hbm_re_efull_enable
    _count += 1; // sch_hbm_we_efull_enable
    _count += 1; // sch_hbm_byp_pending_efull_enable
    _count += 1; // sch_hbm_rd_pending_efull_enable
    _count += 1; // sch_hbm_wr_pending_efull_enable
    _count += 1; // sch_bresp_err_enable
    _count += 1; // sch_bid_err_enable
    _count += 1; // sch_rresp_err_enable
    _count += 1; // sch_rid_err_enable
    _count += 1; // sch_drb_cnt_ovfl_enable
    _count += 1; // sch_drb_cnt_unfl_enable
    _count += 1; // sch_txdma_msg_ovfl_enable
    return _count;
}

int cap_txs_csr_int_sch_int_test_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // sch_wid_empty_interrupt
    _count += 1; // sch_rid_empty_interrupt
    _count += 1; // sch_wr_txfifo_ovf_interrupt
    _count += 1; // sch_rd_txfifo_ovf_interrupt
    _count += 1; // sch_state_fifo_ovf_interrupt
    _count += 1; // sch_drb_cnt_qid_fifo_interrupt
    _count += 1; // sch_hbm_byp_ovf_interrupt
    _count += 1; // sch_hbm_byp_wtag_wrap_interrupt
    _count += 1; // sch_rlid_ovfl_interrupt
    _count += 1; // sch_rlid_unfl_interrupt
    _count += 1; // sch_null_lif_interrupt
    _count += 1; // sch_lif_sg_mismatch_interrupt
    _count += 1; // sch_aclr_hbm_ln_rollovr_interrupt
    _count += 1; // sch_txdma_msg_efull_interrupt
    _count += 1; // sch_hbm_re_efull_interrupt
    _count += 1; // sch_hbm_we_efull_interrupt
    _count += 1; // sch_hbm_byp_pending_efull_interrupt
    _count += 1; // sch_hbm_rd_pending_efull_interrupt
    _count += 1; // sch_hbm_wr_pending_efull_interrupt
    _count += 1; // sch_bresp_err_interrupt
    _count += 1; // sch_bid_err_interrupt
    _count += 1; // sch_rresp_err_interrupt
    _count += 1; // sch_rid_err_interrupt
    _count += 1; // sch_drb_cnt_ovfl_interrupt
    _count += 1; // sch_drb_cnt_unfl_interrupt
    _count += 1; // sch_txdma_msg_ovfl_interrupt
    return _count;
}

int cap_txs_csr_int_sch_t::s_get_width() {
    int _count = 0;

    _count += cap_txs_csr_int_sch_int_test_set_t::s_get_width(); // intreg
    _count += cap_txs_csr_int_sch_int_test_set_t::s_get_width(); // int_test_set
    _count += cap_txs_csr_int_sch_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_txs_csr_int_sch_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_txs_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_sch_interrupt
    _count += 1; // int_tmr_interrupt
    _count += 1; // int_srams_ecc_interrupt
    return _count;
}

int cap_txs_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_sch_enable
    _count += 1; // int_tmr_enable
    _count += 1; // int_srams_ecc_enable
    return _count;
}

int cap_txs_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_txs_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_txs_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_txs_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_txs_csr_int_srams_ecc_int_enable_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // tmr_cnt_uncorrectable_enable
    _count += 1; // tmr_cnt_correctable_enable
    _count += 1; // sch_lif_map_uncorrectable_enable
    _count += 1; // sch_lif_map_correctable_enable
    _count += 1; // sch_rlid_map_uncorrectable_enable
    _count += 1; // sch_rlid_map_correctable_enable
    _count += 1; // sch_grp_uncorrectable_enable
    _count += 1; // sch_grp_correctable_enable
    return _count;
}

int cap_txs_csr_int_srams_ecc_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // tmr_cnt_uncorrectable_interrupt
    _count += 1; // tmr_cnt_correctable_interrupt
    _count += 1; // sch_lif_map_uncorrectable_interrupt
    _count += 1; // sch_lif_map_correctable_interrupt
    _count += 1; // sch_rlid_map_uncorrectable_interrupt
    _count += 1; // sch_rlid_map_correctable_interrupt
    _count += 1; // sch_grp_uncorrectable_interrupt
    _count += 1; // sch_grp_correctable_interrupt
    return _count;
}

int cap_txs_csr_int_srams_ecc_t::s_get_width() {
    int _count = 0;

    _count += cap_txs_csr_int_srams_ecc_intreg_t::s_get_width(); // intreg
    _count += cap_txs_csr_int_srams_ecc_intreg_t::s_get_width(); // int_test_set
    _count += cap_txs_csr_int_srams_ecc_int_enable_set_t::s_get_width(); // int_enable_set
    _count += cap_txs_csr_int_srams_ecc_int_enable_set_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_txs_csr_dhs_sch_grp_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // spare
    _count += 5; // clr_pend_cnt
    _count += 1; // rl_thr
    _count += 4; // bckgr_cnt
    _count += 1; // bckgr
    _count += 1; // hbm_mode
    _count += 11; // qid_offset
    _count += 11; // lif
    _count += 1; // auto_clr
    _count += 4; // rr_sel
    _count += 1; // qid_fifo_elm0_vld
    _count += 1; // qid_fifo_elm0_jnk
    _count += 13; // qid_fifo_elm0_qid
    _count += 1; // qid_fifo_elm1_vld
    _count += 1; // qid_fifo_elm1_jnk
    _count += 13; // qid_fifo_elm1_qid
    _count += 1; // qid_fifo_elm2_vld
    _count += 1; // qid_fifo_elm2_jnk
    _count += 13; // qid_fifo_elm2_qid
    _count += 1; // qid_fifo_elm3_vld
    _count += 1; // qid_fifo_elm3_jnk
    _count += 13; // qid_fifo_elm3_qid
    _count += 1; // qid_fifo_elm4_vld
    _count += 1; // qid_fifo_elm4_jnk
    _count += 13; // qid_fifo_elm4_qid
    _count += 1; // qid_fifo_elm5_vld
    _count += 1; // qid_fifo_elm5_jnk
    _count += 13; // qid_fifo_elm5_qid
    _count += 1; // qid_fifo_elm6_vld
    _count += 1; // qid_fifo_elm6_jnk
    _count += 13; // qid_fifo_elm6_qid
    _count += 1; // qid_fifo_elm7_vld
    _count += 1; // qid_fifo_elm7_jnk
    _count += 13; // qid_fifo_elm7_qid
    _count += 1; // qid_fifo_elm8_vld
    _count += 1; // qid_fifo_elm8_jnk
    _count += 13; // qid_fifo_elm8_qid
    _count += 1; // qid_fifo_elm9_vld
    _count += 1; // qid_fifo_elm9_jnk
    _count += 13; // qid_fifo_elm9_qid
    _count += 1; // qid_fifo_elm10_vld
    _count += 1; // qid_fifo_elm10_jnk
    _count += 13; // qid_fifo_elm10_qid
    _count += 1; // qid_fifo_elm11_vld
    _count += 1; // qid_fifo_elm11_jnk
    _count += 13; // qid_fifo_elm11_qid
    _count += 1; // qid_fifo_elm12_vld
    _count += 1; // qid_fifo_elm12_jnk
    _count += 13; // qid_fifo_elm12_qid
    _count += 1; // qid_fifo_elm13_vld
    _count += 1; // qid_fifo_elm13_jnk
    _count += 13; // qid_fifo_elm13_qid
    _count += 1; // qid_fifo_elm14_vld
    _count += 1; // qid_fifo_elm14_jnk
    _count += 13; // qid_fifo_elm14_qid
    _count += 1; // qid_fifo_elm15_vld
    _count += 1; // qid_fifo_elm15_jnk
    _count += 13; // qid_fifo_elm15_qid
    _count += 4; // hbm_ln
    _count += 9; // hbm_ln_ptr
    _count += 9; // hbm_rr_cnt
    _count += 14; // drb_cnt
    _count += 11; // hbm_ln_cnt0
    _count += 11; // hbm_ln_cnt1
    _count += 11; // hbm_ln_cnt2
    _count += 11; // hbm_ln_cnt3
    _count += 11; // hbm_ln_cnt4
    _count += 11; // hbm_ln_cnt5
    _count += 11; // hbm_ln_cnt6
    _count += 11; // hbm_ln_cnt7
    _count += 11; // hbm_ln_cnt8
    _count += 11; // hbm_ln_cnt9
    _count += 11; // hbm_ln_cnt10
    _count += 11; // hbm_ln_cnt11
    _count += 11; // hbm_ln_cnt12
    _count += 11; // hbm_ln_cnt13
    _count += 11; // hbm_ln_cnt14
    _count += 11; // hbm_ln_cnt15
    _count += 8; // popcnt
    _count += 36; // ecc
    return _count;
}

int cap_txs_csr_dhs_sch_grp_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_grp_sram_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 3; // spare
    _count += 11; // sg_start
    _count += 11; // sg_end
    _count += 6; // ecc
    return _count;
}

int cap_txs_csr_dhs_sch_rlid_map_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 2; // spare
    _count += 1; // sg_active
    _count += 11; // sg_start
    _count += 11; // sg_per_cos
    _count += 16; // sg_act_cos
    _count += 7; // ecc
    return _count;
}

int cap_txs_csr_dhs_sch_lif_map_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_lif_map_sram_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 5; // spare
    _count += 18; // slow_cbcnt
    _count += 18; // slow_bcnt
    _count += 4; // slow_lcnt
    _count += 18; // fast_cbcnt
    _count += 18; // fast_bcnt
    _count += 4; // fast_lcnt
    _count += 8; // ecc
    return _count;
}

int cap_txs_csr_dhs_tmr_cnt_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_tmr_cnt_sram_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_txs_csr_dhs_rlid_stop_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // setmsk
    return _count;
}

int cap_txs_csr_dhs_rlid_stop_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_rlid_stop_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::s_get_width() {
    int _count = 0;

    _count += 8; // popcnt
    _count += 14; // drb_cnt
    return _count;
}

int cap_txs_csr_dhs_sch_grp_cnt_entry_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_sch_grp_entry_entry_t::s_get_width() {
    int _count = 0;

    _count += 11; // lif
    _count += 11; // qid_offset
    _count += 1; // auto_clr
    _count += 4; // rr_sel
    _count += 1; // rl_thr
    return _count;
}

int cap_txs_csr_dhs_sch_grp_entry_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_grp_entry_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_doorbell_entry_t::s_get_width() {
    int _count = 0;

    _count += 24; // qid
    _count += 4; // cos
    _count += 1; // set
    return _count;
}

int cap_txs_csr_dhs_doorbell_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_doorbell_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_sch_flags_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // sch_flags
    return _count;
}

int cap_txs_csr_dhs_sch_flags_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_sch_flags_entry_t::s_get_width() * 64); // entry
    return _count;
}

int cap_txs_csr_dhs_dtdmhi_calendar_entry_t::s_get_width() {
    int _count = 0;

    _count += 4; // dtdm_calendar
    return _count;
}

int cap_txs_csr_dhs_dtdmhi_calendar_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_dtdmhi_calendar_entry_t::s_get_width() * 64); // entry
    return _count;
}

int cap_txs_csr_dhs_dtdmlo_calendar_entry_t::s_get_width() {
    int _count = 0;

    _count += 4; // dtdm_calendar
    return _count;
}

int cap_txs_csr_dhs_dtdmlo_calendar_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_dtdmlo_calendar_entry_t::s_get_width() * 64); // entry
    return _count;
}

int cap_txs_csr_dhs_slow_timer_pending_entry_t::s_get_width() {
    int _count = 0;

    _count += 18; // bcnt
    _count += 4; // lcnt
    return _count;
}

int cap_txs_csr_dhs_slow_timer_pending_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_slow_timer_pending_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::s_get_width() {
    int _count = 0;

    _count += 3; // typ
    _count += 24; // qid
    _count += 3; // ring
    _count += 10; // dtime
    return _count;
}

int cap_txs_csr_dhs_slow_timer_start_no_stop_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_dhs_fast_timer_pending_entry_t::s_get_width() {
    int _count = 0;

    _count += 18; // bcnt
    _count += 4; // lcnt
    return _count;
}

int cap_txs_csr_dhs_fast_timer_pending_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_fast_timer_pending_entry_t::s_get_width() * 4096); // entry
    return _count;
}

int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::s_get_width() {
    int _count = 0;

    _count += 3; // typ
    _count += 24; // qid
    _count += 3; // ring
    _count += 10; // dtime
    return _count;
}

int cap_txs_csr_dhs_fast_timer_start_no_stop_t::s_get_width() {
    int _count = 0;

    _count += (cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_bid_uexp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_bid_rerr_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_bid_err_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_bid_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_wr_req_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_rd_rsp_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_tmr_axi_rd_req_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_bid_uexp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_bid_rerr_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_bid_err_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_bid_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_wr_req_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_rd_rsp_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_cnt_sch_axi_rd_req_t::s_get_width() {
    int _count = 0;

    _count += 48; // val
    return _count;
}

int cap_txs_csr_sta_stmr_max_bcnt_t::s_get_width() {
    int _count = 0;

    _count += 18; // bcnt
    return _count;
}

int cap_txs_csr_sta_ftmr_max_bcnt_t::s_get_width() {
    int _count = 0;

    _count += 18; // bcnt
    return _count;
}

int cap_txs_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_txs_csr_sta_srams_sch_hbm_byp_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_sch_hbm_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_cfg_sch_hbm_byp_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_sch_hbm_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_sta_srams_ecc_sch_grp_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 36; // syndrome
    _count += 11; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 6; // syndrome
    _count += 11; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 7; // syndrome
    _count += 11; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 12; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_scheduler_dbg2_t::s_get_width() {
    int _count = 0;

    _count += 6; // hbm_byp_pending_cnt
    _count += 6; // hbm_rd_pending_cnt
    _count += 6; // hbm_wr_pending_cnt
    return _count;
}

int cap_txs_csr_sta_scheduler_dbg_t::s_get_width() {
    int _count = 0;

    _count += 1; // txdma_drdy
    _count += 1; // hbm_re_efull
    _count += 1; // hbm_we_efull
    _count += 1; // txdma_msg_efull
    _count += 1; // hbm_byp_pending_efull
    _count += 1; // hbm_rd_pending_efull
    _count += 1; // hbm_wr_pending_efull
    return _count;
}

int cap_txs_csr_sta_sch_lif_sg_mismatch_t::s_get_width() {
    int _count = 0;

    _count += 11; // lif
    _count += 13; // qid
    _count += 1; // set
    return _count;
}

int cap_txs_csr_sta_sch_lif_map_notactive_t::s_get_width() {
    int _count = 0;

    _count += 11; // lif
    _count += 24; // qid
    _count += 4; // cos
    _count += 1; // set
    return _count;
}

int cap_txs_csr_cfg_sch_grp_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_cor
    _count += 1; // ecc_disable_det
    _count += 1; // dhs_eccbypass
    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_sch_rlid_map_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_cor
    _count += 1; // ecc_disable_det
    _count += 1; // dhs_eccbypass
    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_sch_lif_map_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_cor
    _count += 1; // ecc_disable_det
    _count += 1; // dhs_eccbypass
    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_tmr_cnt_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_disable_cor
    _count += 1; // ecc_disable_det
    _count += 1; // dhs_eccbypass
    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_sta_sch_max_hbm_byp_t::s_get_width() {
    int _count = 0;

    _count += 6; // cnt
    return _count;
}

int cap_txs_csr_cfg_scheduler_dbg2_t::s_get_width() {
    int _count = 0;

    _count += 24; // qid_read
    _count += 4; // cos_read
    return _count;
}

int cap_txs_csr_cfg_scheduler_dbg_t::s_get_width() {
    int _count = 0;

    _count += 3; // hbm_efc_thr
    _count += 2; // txdma_efc_thr
    _count += 5; // fifo_mode_thr
    _count += 5; // max_hbm_byp
    _count += 5; // max_hbm_wr
    _count += 5; // max_hbm_rd
    _count += 4; // spare
    return _count;
}

int cap_txs_csr_sta_scheduler_rr_t::s_get_width() {
    int _count = 0;

    _count += 11; // curr_ptr0
    _count += 11; // curr_ptr1
    _count += 11; // curr_ptr2
    _count += 11; // curr_ptr3
    _count += 11; // curr_ptr4
    _count += 11; // curr_ptr5
    _count += 11; // curr_ptr6
    _count += 11; // curr_ptr7
    _count += 11; // curr_ptr8
    _count += 11; // curr_ptr9
    _count += 11; // curr_ptr10
    _count += 11; // curr_ptr11
    _count += 11; // curr_ptr12
    _count += 11; // curr_ptr13
    _count += 11; // curr_ptr14
    _count += 11; // curr_ptr15
    return _count;
}

int cap_txs_csr_sta_scheduler_t::s_get_width() {
    int _count = 0;

    _count += 1; // hbm_init_done
    _count += 1; // sram_init_done
    return _count;
}

int cap_txs_csr_cnt_sch_rlid_start_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_rlid_stop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_sta_glb_t::s_get_width() {
    int _count = 0;

    _count += 16; // pb_xoff
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos15_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos14_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos13_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos12_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos11_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos10_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos9_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos8_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos7_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos6_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos5_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos4_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos3_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos2_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos1_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_txdma_cos0_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_fifo_empty_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_sch_doorbell_clr_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cnt_sch_doorbell_set_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_cfg_sch_t::s_get_width() {
    int _count = 0;

    _count += 16; // dtdm_lo_map
    _count += 16; // dtdm_hi_map
    _count += 16; // timeout
    _count += 16; // pause
    _count += 1; // enable
    return _count;
}

int cap_txs_csr_cfw_scheduler_static_t::s_get_width() {
    int _count = 0;

    _count += 64; // hbm_base
    _count += 12; // sch_grp_depth
    return _count;
}

int cap_txs_csr_cfw_scheduler_glb_t::s_get_width() {
    int _count = 0;

    _count += 1; // hbm_hw_init
    _count += 1; // sram_hw_init
    _count += 3; // spare
    _count += 1; // enable
    _count += 1; // enable_set_lkup
    _count += 1; // enable_set_byp
    return _count;
}

int cap_txs_csr_sta_srams_tmr_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_tmr_hbm_byp_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_sta_srams_tmr_hbm_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_txs_csr_cfg_tmr_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_tmr_hbm_byp_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cfg_tmr_hbm_sram_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_txs_csr_cnt_stmr_pop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_stmr_key_not_found_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_stmr_push_out_of_wheel_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_stmr_key_not_push_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_stmr_push_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_sta_slow_timer_t::s_get_width() {
    int _count = 0;

    _count += 32; // tick
    _count += 12; // cTime
    _count += 12; // pTime
    return _count;
}

int cap_txs_csr_cfg_slow_timer_dbell_t::s_get_width() {
    int _count = 0;

    _count += 6; // addr_update
    _count += 16; // data_pid
    _count += 16; // data_reserved
    _count += 16; // data_index
    return _count;
}

int cap_txs_csr_cfg_force_slow_timer_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 12; // ctime
    return _count;
}

int cap_txs_csr_cfg_slow_timer_t::s_get_width() {
    int _count = 0;

    _count += 32; // tick
    _count += 2; // hash_sel
    return _count;
}

int cap_txs_csr_cnt_ftmr_pop_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_ftmr_key_not_found_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_ftmr_key_not_push_t::s_get_width() {
    int _count = 0;

    _count += 32; // val
    return _count;
}

int cap_txs_csr_cnt_ftmr_push_t::s_get_width() {
    int _count = 0;

    _count += 64; // val
    return _count;
}

int cap_txs_csr_sta_fast_timer_t::s_get_width() {
    int _count = 0;

    _count += 32; // tick
    _count += 12; // cTime
    _count += 12; // pTime
    return _count;
}

int cap_txs_csr_cfg_fast_timer_dbell_t::s_get_width() {
    int _count = 0;

    _count += 6; // addr_update
    _count += 16; // data_pid
    _count += 16; // data_reserved
    _count += 16; // data_index
    return _count;
}

int cap_txs_csr_cfg_force_fast_timer_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 12; // ctime
    return _count;
}

int cap_txs_csr_cfg_fast_timer_t::s_get_width() {
    int _count = 0;

    _count += 32; // tick
    _count += 2; // hash_sel
    return _count;
}

int cap_txs_csr_sta_timer_dbg2_t::s_get_width() {
    int _count = 0;

    _count += 5; // hbm_byp_pending_cnt
    _count += 5; // hbm_rd_pending_cnt
    _count += 5; // hbm_wr_pending_cnt
    _count += 1; // stmr_stall
    _count += 1; // ftmr_stall
    return _count;
}

int cap_txs_csr_sta_timer_dbg_t::s_get_width() {
    int _count = 0;

    _count += 1; // hbm_re_efull
    _count += 1; // hbm_we_efull
    _count += 1; // rejct_drb_efull
    _count += 1; // hbm_byp_pending_efull
    _count += 1; // hbm_rd_pending_efull
    _count += 1; // hbm_wr_pending_efull
    _count += 4; // stmr_fifo_raddr
    _count += 4; // ftmr_fifo_raddr
    _count += 4; // stmr_fifo_waddr
    _count += 4; // ftmr_fifo_waddr
    _count += 1; // stmr_fifo_efull
    _count += 1; // ftmr_fifo_efull
    return _count;
}

int cap_txs_csr_sta_tmr_max_keys_t::s_get_width() {
    int _count = 0;

    _count += 5; // cnt
    return _count;
}

int cap_txs_csr_sta_tmr_max_hbm_byp_t::s_get_width() {
    int _count = 0;

    _count += 5; // cnt
    return _count;
}

int cap_txs_csr_sta_timer_t::s_get_width() {
    int _count = 0;

    _count += 1; // hbm_init_done
    _count += 1; // sram_init_done
    return _count;
}

int cap_txs_csr_cfg_timer_dbg2_t::s_get_width() {
    int _count = 0;

    _count += 18; // max_bcnt
    _count += 16; // spare
    return _count;
}

int cap_txs_csr_cfg_timer_dbg_t::s_get_width() {
    int _count = 0;

    _count += 3; // hbm_efc_thr
    _count += 2; // drb_efc_thr
    _count += 12; // tmr_stall_thr_hi
    _count += 12; // tmr_stall_thr_lo
    _count += 4; // max_hbm_wr
    _count += 4; // max_hbm_rd
    _count += 4; // max_hbm_byp
    _count += 4; // max_tmr_fifo
    _count += 8; // spare
    return _count;
}

int cap_txs_csr_cfw_timer_glb_t::s_get_width() {
    int _count = 0;

    _count += 1; // hbm_hw_init
    _count += 1; // sram_hw_init
    _count += 3; // spare
    _count += 1; // ftmr_enable
    _count += 1; // stmr_enable
    _count += 1; // ftmr_pause
    _count += 1; // stmr_pause
    return _count;
}

int cap_txs_csr_cfg_timer_static_t::s_get_width() {
    int _count = 0;

    _count += 64; // hbm_base
    _count += 24; // tmr_hsh_depth
    _count += 12; // tmr_wheel_depth
    return _count;
}

int cap_txs_csr_cfg_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // arcache
    _count += 4; // awcache
    _count += 3; // prot
    _count += 4; // qos
    _count += 1; // lock
    return _count;
}

int cap_txs_csr_cfg_glb_t::s_get_width() {
    int _count = 0;

    _count += 2; // dbg_port_select
    _count += 1; // dbg_port_enable
    _count += 3; // spare
    return _count;
}

int cap_txs_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_txs_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_txs_csr_base_t::s_get_width(); // base
    _count += cap_txs_csr_cfg_glb_t::s_get_width(); // cfg_glb
    _count += cap_txs_csr_cfg_axi_attr_t::s_get_width(); // cfg_axi_attr
    _count += cap_txs_csr_cfg_timer_static_t::s_get_width(); // cfg_timer_static
    _count += cap_txs_csr_cfw_timer_glb_t::s_get_width(); // cfw_timer_glb
    _count += cap_txs_csr_cfg_timer_dbg_t::s_get_width(); // cfg_timer_dbg
    _count += cap_txs_csr_cfg_timer_dbg2_t::s_get_width(); // cfg_timer_dbg2
    _count += cap_txs_csr_sta_timer_t::s_get_width(); // sta_timer
    _count += cap_txs_csr_sta_tmr_max_hbm_byp_t::s_get_width(); // sta_tmr_max_hbm_byp
    _count += cap_txs_csr_sta_tmr_max_keys_t::s_get_width(); // sta_tmr_max_keys
    _count += cap_txs_csr_sta_timer_dbg_t::s_get_width(); // sta_timer_dbg
    _count += cap_txs_csr_sta_timer_dbg2_t::s_get_width(); // sta_timer_dbg2
    _count += cap_txs_csr_cfg_fast_timer_t::s_get_width(); // cfg_fast_timer
    _count += cap_txs_csr_cfg_force_fast_timer_t::s_get_width(); // cfg_force_fast_timer
    _count += cap_txs_csr_cfg_fast_timer_dbell_t::s_get_width(); // cfg_fast_timer_dbell
    _count += cap_txs_csr_sta_fast_timer_t::s_get_width(); // sta_fast_timer
    _count += cap_txs_csr_cnt_ftmr_push_t::s_get_width(); // cnt_ftmr_push
    _count += cap_txs_csr_cnt_ftmr_key_not_push_t::s_get_width(); // cnt_ftmr_key_not_push
    _count += cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::s_get_width(); // cnt_ftmr_push_out_of_wheel
    _count += cap_txs_csr_cnt_ftmr_key_not_found_t::s_get_width(); // cnt_ftmr_key_not_found
    _count += cap_txs_csr_cnt_ftmr_pop_t::s_get_width(); // cnt_ftmr_pop
    _count += cap_txs_csr_cfg_slow_timer_t::s_get_width(); // cfg_slow_timer
    _count += cap_txs_csr_cfg_force_slow_timer_t::s_get_width(); // cfg_force_slow_timer
    _count += cap_txs_csr_cfg_slow_timer_dbell_t::s_get_width(); // cfg_slow_timer_dbell
    _count += cap_txs_csr_sta_slow_timer_t::s_get_width(); // sta_slow_timer
    _count += cap_txs_csr_cnt_stmr_push_t::s_get_width(); // cnt_stmr_push
    _count += cap_txs_csr_cnt_stmr_key_not_push_t::s_get_width(); // cnt_stmr_key_not_push
    _count += cap_txs_csr_cnt_stmr_push_out_of_wheel_t::s_get_width(); // cnt_stmr_push_out_of_wheel
    _count += cap_txs_csr_cnt_stmr_key_not_found_t::s_get_width(); // cnt_stmr_key_not_found
    _count += cap_txs_csr_cnt_stmr_pop_t::s_get_width(); // cnt_stmr_pop
    _count += cap_txs_csr_cfg_tmr_hbm_sram_t::s_get_width(); // cfg_tmr_hbm_sram
    _count += cap_txs_csr_cfg_tmr_hbm_byp_sram_t::s_get_width(); // cfg_tmr_hbm_byp_sram
    _count += cap_txs_csr_cfg_tmr_fifo_sram_t::s_get_width(); // cfg_tmr_fifo_sram
    _count += cap_txs_csr_sta_srams_tmr_hbm_t::s_get_width(); // sta_srams_tmr_hbm
    _count += cap_txs_csr_sta_srams_tmr_hbm_byp_t::s_get_width(); // sta_srams_tmr_hbm_byp
    _count += cap_txs_csr_sta_srams_tmr_fifo_t::s_get_width(); // sta_srams_tmr_fifo
    _count += cap_txs_csr_cfw_scheduler_glb_t::s_get_width(); // cfw_scheduler_glb
    _count += cap_txs_csr_cfw_scheduler_static_t::s_get_width(); // cfw_scheduler_static
    _count += cap_txs_csr_cfg_sch_t::s_get_width(); // cfg_sch
    _count += cap_txs_csr_cnt_sch_doorbell_set_t::s_get_width(); // cnt_sch_doorbell_set
    _count += cap_txs_csr_cnt_sch_doorbell_clr_t::s_get_width(); // cnt_sch_doorbell_clr
    _count += cap_txs_csr_cnt_sch_fifo_empty_t::s_get_width(); // cnt_sch_fifo_empty
    _count += cap_txs_csr_cnt_sch_txdma_cos0_t::s_get_width(); // cnt_sch_txdma_cos0
    _count += cap_txs_csr_cnt_sch_txdma_cos1_t::s_get_width(); // cnt_sch_txdma_cos1
    _count += cap_txs_csr_cnt_sch_txdma_cos2_t::s_get_width(); // cnt_sch_txdma_cos2
    _count += cap_txs_csr_cnt_sch_txdma_cos3_t::s_get_width(); // cnt_sch_txdma_cos3
    _count += cap_txs_csr_cnt_sch_txdma_cos4_t::s_get_width(); // cnt_sch_txdma_cos4
    _count += cap_txs_csr_cnt_sch_txdma_cos5_t::s_get_width(); // cnt_sch_txdma_cos5
    _count += cap_txs_csr_cnt_sch_txdma_cos6_t::s_get_width(); // cnt_sch_txdma_cos6
    _count += cap_txs_csr_cnt_sch_txdma_cos7_t::s_get_width(); // cnt_sch_txdma_cos7
    _count += cap_txs_csr_cnt_sch_txdma_cos8_t::s_get_width(); // cnt_sch_txdma_cos8
    _count += cap_txs_csr_cnt_sch_txdma_cos9_t::s_get_width(); // cnt_sch_txdma_cos9
    _count += cap_txs_csr_cnt_sch_txdma_cos10_t::s_get_width(); // cnt_sch_txdma_cos10
    _count += cap_txs_csr_cnt_sch_txdma_cos11_t::s_get_width(); // cnt_sch_txdma_cos11
    _count += cap_txs_csr_cnt_sch_txdma_cos12_t::s_get_width(); // cnt_sch_txdma_cos12
    _count += cap_txs_csr_cnt_sch_txdma_cos13_t::s_get_width(); // cnt_sch_txdma_cos13
    _count += cap_txs_csr_cnt_sch_txdma_cos14_t::s_get_width(); // cnt_sch_txdma_cos14
    _count += cap_txs_csr_cnt_sch_txdma_cos15_t::s_get_width(); // cnt_sch_txdma_cos15
    _count += cap_txs_csr_sta_glb_t::s_get_width(); // sta_glb
    _count += cap_txs_csr_cnt_sch_rlid_stop_t::s_get_width(); // cnt_sch_rlid_stop
    _count += cap_txs_csr_cnt_sch_rlid_start_t::s_get_width(); // cnt_sch_rlid_start
    _count += cap_txs_csr_sta_scheduler_t::s_get_width(); // sta_scheduler
    _count += cap_txs_csr_sta_scheduler_rr_t::s_get_width(); // sta_scheduler_rr
    _count += cap_txs_csr_cfg_scheduler_dbg_t::s_get_width(); // cfg_scheduler_dbg
    _count += cap_txs_csr_cfg_scheduler_dbg2_t::s_get_width(); // cfg_scheduler_dbg2
    _count += cap_txs_csr_sta_sch_max_hbm_byp_t::s_get_width(); // sta_sch_max_hbm_byp
    _count += cap_txs_csr_cfg_tmr_cnt_sram_t::s_get_width(); // cfg_tmr_cnt_sram
    _count += cap_txs_csr_cfg_sch_lif_map_sram_t::s_get_width(); // cfg_sch_lif_map_sram
    _count += cap_txs_csr_cfg_sch_rlid_map_sram_t::s_get_width(); // cfg_sch_rlid_map_sram
    _count += cap_txs_csr_cfg_sch_grp_sram_t::s_get_width(); // cfg_sch_grp_sram
    _count += cap_txs_csr_sta_sch_lif_map_notactive_t::s_get_width(); // sta_sch_lif_map_notactive
    _count += cap_txs_csr_sta_sch_lif_sg_mismatch_t::s_get_width(); // sta_sch_lif_sg_mismatch
    _count += cap_txs_csr_sta_scheduler_dbg_t::s_get_width(); // sta_scheduler_dbg
    _count += cap_txs_csr_sta_scheduler_dbg2_t::s_get_width(); // sta_scheduler_dbg2
    _count += cap_txs_csr_sta_srams_ecc_tmr_cnt_t::s_get_width(); // sta_srams_ecc_tmr_cnt
    _count += cap_txs_csr_sta_srams_ecc_sch_lif_map_t::s_get_width(); // sta_srams_ecc_sch_lif_map
    _count += cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::s_get_width(); // sta_srams_ecc_sch_rlid_map
    _count += cap_txs_csr_sta_srams_ecc_sch_grp_t::s_get_width(); // sta_srams_ecc_sch_grp
    _count += cap_txs_csr_cfg_sch_hbm_sram_t::s_get_width(); // cfg_sch_hbm_sram
    _count += cap_txs_csr_cfg_sch_hbm_byp_sram_t::s_get_width(); // cfg_sch_hbm_byp_sram
    _count += cap_txs_csr_sta_srams_sch_hbm_t::s_get_width(); // sta_srams_sch_hbm
    _count += cap_txs_csr_sta_srams_sch_hbm_byp_t::s_get_width(); // sta_srams_sch_hbm_byp
    _count += cap_txs_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_txs_csr_sta_ftmr_max_bcnt_t::s_get_width(); // sta_ftmr_max_bcnt
    _count += cap_txs_csr_sta_stmr_max_bcnt_t::s_get_width(); // sta_stmr_max_bcnt
    _count += cap_txs_csr_cnt_sch_axi_rd_req_t::s_get_width(); // cnt_sch_axi_rd_req
    _count += cap_txs_csr_cnt_sch_axi_rd_rsp_t::s_get_width(); // cnt_sch_axi_rd_rsp
    _count += cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::s_get_width(); // cnt_sch_axi_rd_rsp_err
    _count += cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::s_get_width(); // cnt_sch_axi_rd_rsp_rerr
    _count += cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::s_get_width(); // cnt_sch_axi_rd_rsp_uexp
    _count += cap_txs_csr_cnt_sch_axi_wr_req_t::s_get_width(); // cnt_sch_axi_wr_req
    _count += cap_txs_csr_cnt_sch_axi_bid_t::s_get_width(); // cnt_sch_axi_bid
    _count += cap_txs_csr_cnt_sch_axi_bid_err_t::s_get_width(); // cnt_sch_axi_bid_err
    _count += cap_txs_csr_cnt_sch_axi_bid_rerr_t::s_get_width(); // cnt_sch_axi_bid_rerr
    _count += cap_txs_csr_cnt_sch_axi_bid_uexp_t::s_get_width(); // cnt_sch_axi_bid_uexp
    _count += cap_txs_csr_cnt_tmr_axi_rd_req_t::s_get_width(); // cnt_tmr_axi_rd_req
    _count += cap_txs_csr_cnt_tmr_axi_rd_rsp_t::s_get_width(); // cnt_tmr_axi_rd_rsp
    _count += cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::s_get_width(); // cnt_tmr_axi_rd_rsp_err
    _count += cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::s_get_width(); // cnt_tmr_axi_rd_rsp_rerr
    _count += cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::s_get_width(); // cnt_tmr_axi_rd_rsp_uexp
    _count += cap_txs_csr_cnt_tmr_axi_wr_req_t::s_get_width(); // cnt_tmr_axi_wr_req
    _count += cap_txs_csr_cnt_tmr_axi_bid_t::s_get_width(); // cnt_tmr_axi_bid
    _count += cap_txs_csr_cnt_tmr_axi_bid_err_t::s_get_width(); // cnt_tmr_axi_bid_err
    _count += cap_txs_csr_cnt_tmr_axi_bid_rerr_t::s_get_width(); // cnt_tmr_axi_bid_rerr
    _count += cap_txs_csr_cnt_tmr_axi_bid_uexp_t::s_get_width(); // cnt_tmr_axi_bid_uexp
    _count += cap_txs_csr_dhs_fast_timer_start_no_stop_t::s_get_width(); // dhs_fast_timer_start_no_stop
    _count += cap_txs_csr_dhs_fast_timer_pending_t::s_get_width(); // dhs_fast_timer_pending
    _count += cap_txs_csr_dhs_slow_timer_start_no_stop_t::s_get_width(); // dhs_slow_timer_start_no_stop
    _count += cap_txs_csr_dhs_slow_timer_pending_t::s_get_width(); // dhs_slow_timer_pending
    _count += cap_txs_csr_dhs_dtdmlo_calendar_t::s_get_width(); // dhs_dtdmlo_calendar
    _count += cap_txs_csr_dhs_dtdmhi_calendar_t::s_get_width(); // dhs_dtdmhi_calendar
    _count += cap_txs_csr_dhs_sch_flags_t::s_get_width(); // dhs_sch_flags
    _count += cap_txs_csr_dhs_doorbell_t::s_get_width(); // dhs_doorbell
    _count += cap_txs_csr_dhs_sch_grp_entry_t::s_get_width(); // dhs_sch_grp_entry
    _count += cap_txs_csr_dhs_sch_grp_cnt_entry_t::s_get_width(); // dhs_sch_grp_cnt_entry
    _count += cap_txs_csr_dhs_rlid_stop_t::s_get_width(); // dhs_rlid_stop
    _count += cap_txs_csr_dhs_tmr_cnt_sram_t::s_get_width(); // dhs_tmr_cnt_sram
    _count += cap_txs_csr_dhs_sch_lif_map_sram_t::s_get_width(); // dhs_sch_lif_map_sram
    _count += cap_txs_csr_dhs_sch_rlid_map_sram_t::s_get_width(); // dhs_sch_rlid_map_sram
    _count += cap_txs_csr_dhs_sch_grp_sram_t::s_get_width(); // dhs_sch_grp_sram
    _count += cap_txs_csr_int_srams_ecc_t::s_get_width(); // int_srams_ecc
    _count += cap_txs_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_txs_csr_int_sch_t::s_get_width(); // int_sch
    _count += cap_txs_csr_int_tmr_t::s_get_width(); // int_tmr
    return _count;
}

void cap_txs_csr_int_tmr_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tmr_wid_empty_enable = _val.convert_to< tmr_wid_empty_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rid_empty_enable = _val.convert_to< tmr_rid_empty_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_wr_txfifo_ovf_enable = _val.convert_to< tmr_wr_txfifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rd_txfifo_ovf_enable = _val.convert_to< tmr_rd_txfifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_state_fifo_ovf_enable = _val.convert_to< tmr_state_fifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_ovf_enable = _val.convert_to< tmr_hbm_byp_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_wtag_wrap_enable = _val.convert_to< tmr_hbm_byp_wtag_wrap_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_ctime_wrap_enable = _val.convert_to< ftmr_ctime_wrap_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_ctime_wrap_enable = _val.convert_to< stmr_ctime_wrap_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_push_out_of_wheel_enable = _val.convert_to< stmr_push_out_of_wheel_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_push_out_of_wheel_enable = _val.convert_to< ftmr_push_out_of_wheel_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_key_not_push_enable = _val.convert_to< ftmr_key_not_push_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_key_not_found_enable = _val.convert_to< ftmr_key_not_found_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_key_not_push_enable = _val.convert_to< stmr_key_not_push_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_key_not_found_enable = _val.convert_to< stmr_key_not_found_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_stall_enable = _val.convert_to< stmr_stall_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_stall_enable = _val.convert_to< ftmr_stall_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_fifo_ovf_enable = _val.convert_to< ftmr_fifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_fifo_ovf_enable = _val.convert_to< stmr_fifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_fifo_efull_enable = _val.convert_to< ftmr_fifo_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_fifo_efull_enable = _val.convert_to< stmr_fifo_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rejct_drb_ovf_enable = _val.convert_to< tmr_rejct_drb_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rejct_drb_efull_enable = _val.convert_to< tmr_rejct_drb_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_re_efull_enable = _val.convert_to< tmr_hbm_re_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_we_efull_enable = _val.convert_to< tmr_hbm_we_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_pending_efull_enable = _val.convert_to< tmr_hbm_byp_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_rd_pending_efull_enable = _val.convert_to< tmr_hbm_rd_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_wr_pending_efull_enable = _val.convert_to< tmr_hbm_wr_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_bresp_err_enable = _val.convert_to< tmr_bresp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_bid_err_enable = _val.convert_to< tmr_bid_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rresp_err_enable = _val.convert_to< tmr_rresp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rid_err_enable = _val.convert_to< tmr_rid_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_tmr_int_test_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tmr_wid_empty_interrupt = _val.convert_to< tmr_wid_empty_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rid_empty_interrupt = _val.convert_to< tmr_rid_empty_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_wr_txfifo_ovf_interrupt = _val.convert_to< tmr_wr_txfifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rd_txfifo_ovf_interrupt = _val.convert_to< tmr_rd_txfifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_state_fifo_ovf_interrupt = _val.convert_to< tmr_state_fifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_ovf_interrupt = _val.convert_to< tmr_hbm_byp_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_wtag_wrap_interrupt = _val.convert_to< tmr_hbm_byp_wtag_wrap_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_ctime_wrap_interrupt = _val.convert_to< ftmr_ctime_wrap_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_ctime_wrap_interrupt = _val.convert_to< stmr_ctime_wrap_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_push_out_of_wheel_interrupt = _val.convert_to< stmr_push_out_of_wheel_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_push_out_of_wheel_interrupt = _val.convert_to< ftmr_push_out_of_wheel_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_key_not_push_interrupt = _val.convert_to< ftmr_key_not_push_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_key_not_found_interrupt = _val.convert_to< ftmr_key_not_found_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_key_not_push_interrupt = _val.convert_to< stmr_key_not_push_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_key_not_found_interrupt = _val.convert_to< stmr_key_not_found_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_stall_interrupt = _val.convert_to< stmr_stall_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_stall_interrupt = _val.convert_to< ftmr_stall_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_fifo_ovf_interrupt = _val.convert_to< ftmr_fifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_fifo_ovf_interrupt = _val.convert_to< stmr_fifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_fifo_efull_interrupt = _val.convert_to< ftmr_fifo_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_fifo_efull_interrupt = _val.convert_to< stmr_fifo_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rejct_drb_ovf_interrupt = _val.convert_to< tmr_rejct_drb_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rejct_drb_efull_interrupt = _val.convert_to< tmr_rejct_drb_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_re_efull_interrupt = _val.convert_to< tmr_hbm_re_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_we_efull_interrupt = _val.convert_to< tmr_hbm_we_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_byp_pending_efull_interrupt = _val.convert_to< tmr_hbm_byp_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_rd_pending_efull_interrupt = _val.convert_to< tmr_hbm_rd_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_hbm_wr_pending_efull_interrupt = _val.convert_to< tmr_hbm_wr_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_bresp_err_interrupt = _val.convert_to< tmr_bresp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_bid_err_interrupt = _val.convert_to< tmr_bid_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rresp_err_interrupt = _val.convert_to< tmr_rresp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_rid_err_interrupt = _val.convert_to< tmr_rid_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_tmr_t::all(const cpp_int & in_val) {
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

void cap_txs_csr_int_sch_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sch_wid_empty_enable = _val.convert_to< sch_wid_empty_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rid_empty_enable = _val.convert_to< sch_rid_empty_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_wr_txfifo_ovf_enable = _val.convert_to< sch_wr_txfifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rd_txfifo_ovf_enable = _val.convert_to< sch_rd_txfifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_state_fifo_ovf_enable = _val.convert_to< sch_state_fifo_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_qid_fifo_enable = _val.convert_to< sch_drb_cnt_qid_fifo_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_ovf_enable = _val.convert_to< sch_hbm_byp_ovf_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_wtag_wrap_enable = _val.convert_to< sch_hbm_byp_wtag_wrap_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_ovfl_enable = _val.convert_to< sch_rlid_ovfl_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_unfl_enable = _val.convert_to< sch_rlid_unfl_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_null_lif_enable = _val.convert_to< sch_null_lif_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_sg_mismatch_enable = _val.convert_to< sch_lif_sg_mismatch_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_aclr_hbm_ln_rollovr_enable = _val.convert_to< sch_aclr_hbm_ln_rollovr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_txdma_msg_efull_enable = _val.convert_to< sch_txdma_msg_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_re_efull_enable = _val.convert_to< sch_hbm_re_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_we_efull_enable = _val.convert_to< sch_hbm_we_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_pending_efull_enable = _val.convert_to< sch_hbm_byp_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_rd_pending_efull_enable = _val.convert_to< sch_hbm_rd_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_wr_pending_efull_enable = _val.convert_to< sch_hbm_wr_pending_efull_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_bresp_err_enable = _val.convert_to< sch_bresp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_bid_err_enable = _val.convert_to< sch_bid_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rresp_err_enable = _val.convert_to< sch_rresp_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rid_err_enable = _val.convert_to< sch_rid_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_ovfl_enable = _val.convert_to< sch_drb_cnt_ovfl_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_unfl_enable = _val.convert_to< sch_drb_cnt_unfl_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_txdma_msg_ovfl_enable = _val.convert_to< sch_txdma_msg_ovfl_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_sch_int_test_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sch_wid_empty_interrupt = _val.convert_to< sch_wid_empty_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rid_empty_interrupt = _val.convert_to< sch_rid_empty_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_wr_txfifo_ovf_interrupt = _val.convert_to< sch_wr_txfifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rd_txfifo_ovf_interrupt = _val.convert_to< sch_rd_txfifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_state_fifo_ovf_interrupt = _val.convert_to< sch_state_fifo_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_qid_fifo_interrupt = _val.convert_to< sch_drb_cnt_qid_fifo_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_ovf_interrupt = _val.convert_to< sch_hbm_byp_ovf_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_wtag_wrap_interrupt = _val.convert_to< sch_hbm_byp_wtag_wrap_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_ovfl_interrupt = _val.convert_to< sch_rlid_ovfl_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_unfl_interrupt = _val.convert_to< sch_rlid_unfl_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_null_lif_interrupt = _val.convert_to< sch_null_lif_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_sg_mismatch_interrupt = _val.convert_to< sch_lif_sg_mismatch_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_aclr_hbm_ln_rollovr_interrupt = _val.convert_to< sch_aclr_hbm_ln_rollovr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_txdma_msg_efull_interrupt = _val.convert_to< sch_txdma_msg_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_re_efull_interrupt = _val.convert_to< sch_hbm_re_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_we_efull_interrupt = _val.convert_to< sch_hbm_we_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_byp_pending_efull_interrupt = _val.convert_to< sch_hbm_byp_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_rd_pending_efull_interrupt = _val.convert_to< sch_hbm_rd_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_hbm_wr_pending_efull_interrupt = _val.convert_to< sch_hbm_wr_pending_efull_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_bresp_err_interrupt = _val.convert_to< sch_bresp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_bid_err_interrupt = _val.convert_to< sch_bid_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rresp_err_interrupt = _val.convert_to< sch_rresp_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rid_err_interrupt = _val.convert_to< sch_rid_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_ovfl_interrupt = _val.convert_to< sch_drb_cnt_ovfl_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_drb_cnt_unfl_interrupt = _val.convert_to< sch_drb_cnt_unfl_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_txdma_msg_ovfl_interrupt = _val.convert_to< sch_txdma_msg_ovfl_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_sch_t::all(const cpp_int & in_val) {
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

void cap_txs_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_sch_interrupt = _val.convert_to< int_sch_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_tmr_interrupt = _val.convert_to< int_tmr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_srams_ecc_interrupt = _val.convert_to< int_srams_ecc_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_sch_enable = _val.convert_to< int_sch_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_tmr_enable = _val.convert_to< int_tmr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_srams_ecc_enable = _val.convert_to< int_srams_ecc_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_txs_csr_int_srams_ecc_int_enable_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tmr_cnt_uncorrectable_enable = _val.convert_to< tmr_cnt_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_cnt_correctable_enable = _val.convert_to< tmr_cnt_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_map_uncorrectable_enable = _val.convert_to< sch_lif_map_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_map_correctable_enable = _val.convert_to< sch_lif_map_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_map_uncorrectable_enable = _val.convert_to< sch_rlid_map_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_map_correctable_enable = _val.convert_to< sch_rlid_map_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_grp_uncorrectable_enable = _val.convert_to< sch_grp_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_grp_correctable_enable = _val.convert_to< sch_grp_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_srams_ecc_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tmr_cnt_uncorrectable_interrupt = _val.convert_to< tmr_cnt_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__tmr_cnt_correctable_interrupt = _val.convert_to< tmr_cnt_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_map_uncorrectable_interrupt = _val.convert_to< sch_lif_map_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_lif_map_correctable_interrupt = _val.convert_to< sch_lif_map_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_map_uncorrectable_interrupt = _val.convert_to< sch_rlid_map_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_rlid_map_correctable_interrupt = _val.convert_to< sch_rlid_map_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_grp_uncorrectable_interrupt = _val.convert_to< sch_grp_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sch_grp_correctable_interrupt = _val.convert_to< sch_grp_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_int_srams_ecc_t::all(const cpp_int & in_val) {
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

void cap_txs_csr_dhs_sch_grp_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__clr_pend_cnt = _val.convert_to< clr_pend_cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__rl_thr = _val.convert_to< rl_thr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bckgr_cnt = _val.convert_to< bckgr_cnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__bckgr = _val.convert_to< bckgr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_mode = _val.convert_to< hbm_mode_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_offset = _val.convert_to< qid_offset_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__auto_clr = _val.convert_to< auto_clr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rr_sel = _val.convert_to< rr_sel_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qid_fifo_elm0_vld = _val.convert_to< qid_fifo_elm0_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm0_jnk = _val.convert_to< qid_fifo_elm0_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm0_qid = _val.convert_to< qid_fifo_elm0_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm1_vld = _val.convert_to< qid_fifo_elm1_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm1_jnk = _val.convert_to< qid_fifo_elm1_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm1_qid = _val.convert_to< qid_fifo_elm1_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm2_vld = _val.convert_to< qid_fifo_elm2_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm2_jnk = _val.convert_to< qid_fifo_elm2_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm2_qid = _val.convert_to< qid_fifo_elm2_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm3_vld = _val.convert_to< qid_fifo_elm3_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm3_jnk = _val.convert_to< qid_fifo_elm3_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm3_qid = _val.convert_to< qid_fifo_elm3_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm4_vld = _val.convert_to< qid_fifo_elm4_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm4_jnk = _val.convert_to< qid_fifo_elm4_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm4_qid = _val.convert_to< qid_fifo_elm4_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm5_vld = _val.convert_to< qid_fifo_elm5_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm5_jnk = _val.convert_to< qid_fifo_elm5_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm5_qid = _val.convert_to< qid_fifo_elm5_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm6_vld = _val.convert_to< qid_fifo_elm6_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm6_jnk = _val.convert_to< qid_fifo_elm6_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm6_qid = _val.convert_to< qid_fifo_elm6_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm7_vld = _val.convert_to< qid_fifo_elm7_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm7_jnk = _val.convert_to< qid_fifo_elm7_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm7_qid = _val.convert_to< qid_fifo_elm7_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm8_vld = _val.convert_to< qid_fifo_elm8_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm8_jnk = _val.convert_to< qid_fifo_elm8_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm8_qid = _val.convert_to< qid_fifo_elm8_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm9_vld = _val.convert_to< qid_fifo_elm9_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm9_jnk = _val.convert_to< qid_fifo_elm9_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm9_qid = _val.convert_to< qid_fifo_elm9_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm10_vld = _val.convert_to< qid_fifo_elm10_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm10_jnk = _val.convert_to< qid_fifo_elm10_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm10_qid = _val.convert_to< qid_fifo_elm10_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm11_vld = _val.convert_to< qid_fifo_elm11_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm11_jnk = _val.convert_to< qid_fifo_elm11_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm11_qid = _val.convert_to< qid_fifo_elm11_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm12_vld = _val.convert_to< qid_fifo_elm12_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm12_jnk = _val.convert_to< qid_fifo_elm12_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm12_qid = _val.convert_to< qid_fifo_elm12_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm13_vld = _val.convert_to< qid_fifo_elm13_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm13_jnk = _val.convert_to< qid_fifo_elm13_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm13_qid = _val.convert_to< qid_fifo_elm13_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm14_vld = _val.convert_to< qid_fifo_elm14_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm14_jnk = _val.convert_to< qid_fifo_elm14_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm14_qid = _val.convert_to< qid_fifo_elm14_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__qid_fifo_elm15_vld = _val.convert_to< qid_fifo_elm15_vld_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm15_jnk = _val.convert_to< qid_fifo_elm15_jnk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__qid_fifo_elm15_qid = _val.convert_to< qid_fifo_elm15_qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__hbm_ln = _val.convert_to< hbm_ln_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__hbm_ln_ptr = _val.convert_to< hbm_ln_ptr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__hbm_rr_cnt = _val.convert_to< hbm_rr_cnt_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__drb_cnt = _val.convert_to< drb_cnt_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__hbm_ln_cnt0 = _val.convert_to< hbm_ln_cnt0_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt1 = _val.convert_to< hbm_ln_cnt1_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt2 = _val.convert_to< hbm_ln_cnt2_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt3 = _val.convert_to< hbm_ln_cnt3_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt4 = _val.convert_to< hbm_ln_cnt4_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt5 = _val.convert_to< hbm_ln_cnt5_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt6 = _val.convert_to< hbm_ln_cnt6_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt7 = _val.convert_to< hbm_ln_cnt7_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt8 = _val.convert_to< hbm_ln_cnt8_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt9 = _val.convert_to< hbm_ln_cnt9_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt10 = _val.convert_to< hbm_ln_cnt10_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt11 = _val.convert_to< hbm_ln_cnt11_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt12 = _val.convert_to< hbm_ln_cnt12_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt13 = _val.convert_to< hbm_ln_cnt13_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt14 = _val.convert_to< hbm_ln_cnt14_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__hbm_ln_cnt15 = _val.convert_to< hbm_ln_cnt15_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__popcnt = _val.convert_to< popcnt_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 36;
    
}

void cap_txs_csr_dhs_sch_grp_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__sg_start = _val.convert_to< sg_start_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__sg_end = _val.convert_to< sg_end_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_txs_csr_dhs_sch_rlid_map_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__sg_active = _val.convert_to< sg_active_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sg_start = _val.convert_to< sg_start_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__sg_per_cos = _val.convert_to< sg_per_cos_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__sg_act_cos = _val.convert_to< sg_act_cos_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_txs_csr_dhs_sch_lif_map_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__slow_cbcnt = _val.convert_to< slow_cbcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__slow_bcnt = _val.convert_to< slow_bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__slow_lcnt = _val.convert_to< slow_lcnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__fast_cbcnt = _val.convert_to< fast_cbcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__fast_bcnt = _val.convert_to< fast_bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__fast_lcnt = _val.convert_to< fast_lcnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_txs_csr_dhs_tmr_cnt_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_rlid_stop_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__setmsk = _val.convert_to< setmsk_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_dhs_rlid_stop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__popcnt = _val.convert_to< popcnt_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__drb_cnt = _val.convert_to< drb_cnt_cpp_int_t >()  ;
    _val = _val >> 14;
    
}

void cap_txs_csr_dhs_sch_grp_cnt_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_entry_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__qid_offset = _val.convert_to< qid_offset_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__auto_clr = _val.convert_to< auto_clr_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rr_sel = _val.convert_to< rr_sel_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__rl_thr = _val.convert_to< rl_thr_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_dhs_sch_grp_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_doorbell_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__cos = _val.convert_to< cos_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__set = _val.convert_to< set_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_dhs_doorbell_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_flags_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sch_flags = _val.convert_to< sch_flags_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_dhs_sch_flags_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmhi_calendar_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dtdm_calendar = _val.convert_to< dtdm_calendar_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_dhs_dtdmhi_calendar_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmlo_calendar_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dtdm_calendar = _val.convert_to< dtdm_calendar_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_dhs_dtdmlo_calendar_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_pending_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__lcnt = _val.convert_to< lcnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_dhs_slow_timer_pending_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__typ = _val.convert_to< typ_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__ring = _val.convert_to< ring_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__dtime = _val.convert_to< dtime_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_txs_csr_dhs_slow_timer_start_no_stop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_pending_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__lcnt = _val.convert_to< lcnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_dhs_fast_timer_pending_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__typ = _val.convert_to< typ_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__ring = _val.convert_to< ring_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__dtime = _val.convert_to< dtime_cpp_int_t >()  ;
    _val = _val >> 10;
    
}

void cap_txs_csr_dhs_fast_timer_start_no_stop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_txs_csr_cnt_tmr_axi_bid_uexp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_bid_rerr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_bid_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_bid_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_tmr_axi_wr_req_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_tmr_axi_rd_req_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_sch_axi_bid_uexp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_axi_bid_rerr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_axi_bid_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_sch_axi_bid_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_sch_axi_wr_req_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_cnt_sch_axi_rd_req_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 48;
    
}

void cap_txs_csr_sta_stmr_max_bcnt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
}

void cap_txs_csr_sta_ftmr_max_bcnt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
}

void cap_txs_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_sch_hbm_byp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_sch_hbm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_sch_hbm_byp_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_sch_hbm_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_ecc_sch_grp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 36;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_scheduler_dbg2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_byp_pending_cnt = _val.convert_to< hbm_byp_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__hbm_rd_pending_cnt = _val.convert_to< hbm_rd_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__hbm_wr_pending_cnt = _val.convert_to< hbm_wr_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_txs_csr_sta_scheduler_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__txdma_drdy = _val.convert_to< txdma_drdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_re_efull = _val.convert_to< hbm_re_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_we_efull = _val.convert_to< hbm_we_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__txdma_msg_efull = _val.convert_to< txdma_msg_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_byp_pending_efull = _val.convert_to< hbm_byp_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_rd_pending_efull = _val.convert_to< hbm_rd_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_wr_pending_efull = _val.convert_to< hbm_wr_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_sch_lif_sg_mismatch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__set = _val.convert_to< set_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_sch_lif_map_notactive_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__lif = _val.convert_to< lif_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__qid = _val.convert_to< qid_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__cos = _val.convert_to< cos_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__set = _val.convert_to< set_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_sch_grp_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_sch_rlid_map_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_sch_lif_map_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_tmr_cnt_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_sch_max_hbm_byp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cnt = _val.convert_to< cnt_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_txs_csr_cfg_scheduler_dbg2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__qid_read = _val.convert_to< qid_read_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__cos_read = _val.convert_to< cos_read_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_cfg_scheduler_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_efc_thr = _val.convert_to< hbm_efc_thr_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__txdma_efc_thr = _val.convert_to< txdma_efc_thr_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__fifo_mode_thr = _val.convert_to< fifo_mode_thr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__max_hbm_byp = _val.convert_to< max_hbm_byp_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__max_hbm_wr = _val.convert_to< max_hbm_wr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__max_hbm_rd = _val.convert_to< max_hbm_rd_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_txs_csr_sta_scheduler_rr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__curr_ptr0 = _val.convert_to< curr_ptr0_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr1 = _val.convert_to< curr_ptr1_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr2 = _val.convert_to< curr_ptr2_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr3 = _val.convert_to< curr_ptr3_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr4 = _val.convert_to< curr_ptr4_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr5 = _val.convert_to< curr_ptr5_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr6 = _val.convert_to< curr_ptr6_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr7 = _val.convert_to< curr_ptr7_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr8 = _val.convert_to< curr_ptr8_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr9 = _val.convert_to< curr_ptr9_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr10 = _val.convert_to< curr_ptr10_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr11 = _val.convert_to< curr_ptr11_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr12 = _val.convert_to< curr_ptr12_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr13 = _val.convert_to< curr_ptr13_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr14 = _val.convert_to< curr_ptr14_cpp_int_t >()  ;
    _val = _val >> 11;
    
    int_var__curr_ptr15 = _val.convert_to< curr_ptr15_cpp_int_t >()  ;
    _val = _val >> 11;
    
}

void cap_txs_csr_sta_scheduler_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_init_done = _val.convert_to< hbm_init_done_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram_init_done = _val.convert_to< sram_init_done_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cnt_sch_rlid_start_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_rlid_stop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_sta_glb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__pb_xoff = _val.convert_to< pb_xoff_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_txs_csr_cnt_sch_txdma_cos15_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos14_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos13_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos12_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos11_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos10_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos9_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos8_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos7_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos6_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos5_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos4_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos3_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_txdma_cos0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_fifo_empty_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_sch_doorbell_clr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cnt_sch_doorbell_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_cfg_sch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dtdm_lo_map = _val.convert_to< dtdm_lo_map_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__dtdm_hi_map = _val.convert_to< dtdm_hi_map_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__pause = _val.convert_to< pause_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfw_scheduler_static_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_base = _val.convert_to< hbm_base_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__sch_grp_depth = _val.convert_to< sch_grp_depth_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfw_scheduler_glb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_hw_init = _val.convert_to< hbm_hw_init_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram_hw_init = _val.convert_to< sram_hw_init_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__enable_set_lkup = _val.convert_to< enable_set_lkup_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__enable_set_byp = _val.convert_to< enable_set_byp_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_tmr_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_tmr_hbm_byp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_srams_tmr_hbm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_tmr_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_tmr_hbm_byp_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_tmr_hbm_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cnt_stmr_pop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_stmr_key_not_found_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_stmr_push_out_of_wheel_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_stmr_key_not_push_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_stmr_push_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_sta_slow_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tick = _val.convert_to< tick_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__cTime = _val.convert_to< cTime_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__pTime = _val.convert_to< pTime_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfg_slow_timer_dbell_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addr_update = _val.convert_to< addr_update_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__data_pid = _val.convert_to< data_pid_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data_reserved = _val.convert_to< data_reserved_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data_index = _val.convert_to< data_index_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_txs_csr_cfg_force_slow_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ctime = _val.convert_to< ctime_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfg_slow_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tick = _val.convert_to< tick_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_txs_csr_cnt_ftmr_pop_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_ftmr_key_not_found_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_ftmr_key_not_push_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_cnt_ftmr_push_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__val = _val.convert_to< val_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_txs_csr_sta_fast_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tick = _val.convert_to< tick_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__cTime = _val.convert_to< cTime_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__pTime = _val.convert_to< pTime_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfg_fast_timer_dbell_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addr_update = _val.convert_to< addr_update_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__data_pid = _val.convert_to< data_pid_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data_reserved = _val.convert_to< data_reserved_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__data_index = _val.convert_to< data_index_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_txs_csr_cfg_force_fast_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ctime = _val.convert_to< ctime_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfg_fast_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tick = _val.convert_to< tick_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_txs_csr_sta_timer_dbg2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_byp_pending_cnt = _val.convert_to< hbm_byp_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__hbm_rd_pending_cnt = _val.convert_to< hbm_rd_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__hbm_wr_pending_cnt = _val.convert_to< hbm_wr_pending_cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__stmr_stall = _val.convert_to< stmr_stall_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_stall = _val.convert_to< ftmr_stall_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_timer_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_re_efull = _val.convert_to< hbm_re_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_we_efull = _val.convert_to< hbm_we_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rejct_drb_efull = _val.convert_to< rejct_drb_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_byp_pending_efull = _val.convert_to< hbm_byp_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_rd_pending_efull = _val.convert_to< hbm_rd_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__hbm_wr_pending_efull = _val.convert_to< hbm_wr_pending_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_fifo_raddr = _val.convert_to< stmr_fifo_raddr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ftmr_fifo_raddr = _val.convert_to< ftmr_fifo_raddr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__stmr_fifo_waddr = _val.convert_to< stmr_fifo_waddr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__ftmr_fifo_waddr = _val.convert_to< ftmr_fifo_waddr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__stmr_fifo_efull = _val.convert_to< stmr_fifo_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_fifo_efull = _val.convert_to< ftmr_fifo_efull_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_sta_tmr_max_keys_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cnt = _val.convert_to< cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_txs_csr_sta_tmr_max_hbm_byp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cnt = _val.convert_to< cnt_cpp_int_t >()  ;
    _val = _val >> 5;
    
}

void cap_txs_csr_sta_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_init_done = _val.convert_to< hbm_init_done_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram_init_done = _val.convert_to< sram_init_done_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_timer_dbg2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__max_bcnt = _val.convert_to< max_bcnt_cpp_int_t >()  ;
    _val = _val >> 18;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_txs_csr_cfg_timer_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_efc_thr = _val.convert_to< hbm_efc_thr_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__drb_efc_thr = _val.convert_to< drb_efc_thr_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__tmr_stall_thr_hi = _val.convert_to< tmr_stall_thr_hi_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__tmr_stall_thr_lo = _val.convert_to< tmr_stall_thr_lo_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__max_hbm_wr = _val.convert_to< max_hbm_wr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__max_hbm_rd = _val.convert_to< max_hbm_rd_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__max_hbm_byp = _val.convert_to< max_hbm_byp_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__max_tmr_fifo = _val.convert_to< max_tmr_fifo_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_txs_csr_cfw_timer_glb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_hw_init = _val.convert_to< hbm_hw_init_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram_hw_init = _val.convert_to< sram_hw_init_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__ftmr_enable = _val.convert_to< ftmr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_enable = _val.convert_to< stmr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ftmr_pause = _val.convert_to< ftmr_pause_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stmr_pause = _val.convert_to< stmr_pause_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_txs_csr_cfg_timer_static_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__hbm_base = _val.convert_to< hbm_base_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__tmr_hsh_depth = _val.convert_to< tmr_hsh_depth_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__tmr_wheel_depth = _val.convert_to< tmr_wheel_depth_cpp_int_t >()  ;
    _val = _val >> 12;
    
}

void cap_txs_csr_cfg_axi_attr_t::all(const cpp_int & in_val) {
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

void cap_txs_csr_cfg_glb_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dbg_port_select = _val.convert_to< dbg_port_select_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__dbg_port_enable = _val.convert_to< dbg_port_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_txs_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_txs_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_glb.all( _val);
    _val = _val >> cfg_glb.get_width(); 
    cfg_axi_attr.all( _val);
    _val = _val >> cfg_axi_attr.get_width(); 
    cfg_timer_static.all( _val);
    _val = _val >> cfg_timer_static.get_width(); 
    cfw_timer_glb.all( _val);
    _val = _val >> cfw_timer_glb.get_width(); 
    cfg_timer_dbg.all( _val);
    _val = _val >> cfg_timer_dbg.get_width(); 
    cfg_timer_dbg2.all( _val);
    _val = _val >> cfg_timer_dbg2.get_width(); 
    sta_timer.all( _val);
    _val = _val >> sta_timer.get_width(); 
    sta_tmr_max_hbm_byp.all( _val);
    _val = _val >> sta_tmr_max_hbm_byp.get_width(); 
    sta_tmr_max_keys.all( _val);
    _val = _val >> sta_tmr_max_keys.get_width(); 
    sta_timer_dbg.all( _val);
    _val = _val >> sta_timer_dbg.get_width(); 
    sta_timer_dbg2.all( _val);
    _val = _val >> sta_timer_dbg2.get_width(); 
    cfg_fast_timer.all( _val);
    _val = _val >> cfg_fast_timer.get_width(); 
    cfg_force_fast_timer.all( _val);
    _val = _val >> cfg_force_fast_timer.get_width(); 
    cfg_fast_timer_dbell.all( _val);
    _val = _val >> cfg_fast_timer_dbell.get_width(); 
    sta_fast_timer.all( _val);
    _val = _val >> sta_fast_timer.get_width(); 
    cnt_ftmr_push.all( _val);
    _val = _val >> cnt_ftmr_push.get_width(); 
    cnt_ftmr_key_not_push.all( _val);
    _val = _val >> cnt_ftmr_key_not_push.get_width(); 
    cnt_ftmr_push_out_of_wheel.all( _val);
    _val = _val >> cnt_ftmr_push_out_of_wheel.get_width(); 
    cnt_ftmr_key_not_found.all( _val);
    _val = _val >> cnt_ftmr_key_not_found.get_width(); 
    cnt_ftmr_pop.all( _val);
    _val = _val >> cnt_ftmr_pop.get_width(); 
    cfg_slow_timer.all( _val);
    _val = _val >> cfg_slow_timer.get_width(); 
    cfg_force_slow_timer.all( _val);
    _val = _val >> cfg_force_slow_timer.get_width(); 
    cfg_slow_timer_dbell.all( _val);
    _val = _val >> cfg_slow_timer_dbell.get_width(); 
    sta_slow_timer.all( _val);
    _val = _val >> sta_slow_timer.get_width(); 
    cnt_stmr_push.all( _val);
    _val = _val >> cnt_stmr_push.get_width(); 
    cnt_stmr_key_not_push.all( _val);
    _val = _val >> cnt_stmr_key_not_push.get_width(); 
    cnt_stmr_push_out_of_wheel.all( _val);
    _val = _val >> cnt_stmr_push_out_of_wheel.get_width(); 
    cnt_stmr_key_not_found.all( _val);
    _val = _val >> cnt_stmr_key_not_found.get_width(); 
    cnt_stmr_pop.all( _val);
    _val = _val >> cnt_stmr_pop.get_width(); 
    cfg_tmr_hbm_sram.all( _val);
    _val = _val >> cfg_tmr_hbm_sram.get_width(); 
    cfg_tmr_hbm_byp_sram.all( _val);
    _val = _val >> cfg_tmr_hbm_byp_sram.get_width(); 
    cfg_tmr_fifo_sram.all( _val);
    _val = _val >> cfg_tmr_fifo_sram.get_width(); 
    sta_srams_tmr_hbm.all( _val);
    _val = _val >> sta_srams_tmr_hbm.get_width(); 
    sta_srams_tmr_hbm_byp.all( _val);
    _val = _val >> sta_srams_tmr_hbm_byp.get_width(); 
    sta_srams_tmr_fifo.all( _val);
    _val = _val >> sta_srams_tmr_fifo.get_width(); 
    cfw_scheduler_glb.all( _val);
    _val = _val >> cfw_scheduler_glb.get_width(); 
    cfw_scheduler_static.all( _val);
    _val = _val >> cfw_scheduler_static.get_width(); 
    cfg_sch.all( _val);
    _val = _val >> cfg_sch.get_width(); 
    cnt_sch_doorbell_set.all( _val);
    _val = _val >> cnt_sch_doorbell_set.get_width(); 
    cnt_sch_doorbell_clr.all( _val);
    _val = _val >> cnt_sch_doorbell_clr.get_width(); 
    cnt_sch_fifo_empty.all( _val);
    _val = _val >> cnt_sch_fifo_empty.get_width(); 
    cnt_sch_txdma_cos0.all( _val);
    _val = _val >> cnt_sch_txdma_cos0.get_width(); 
    cnt_sch_txdma_cos1.all( _val);
    _val = _val >> cnt_sch_txdma_cos1.get_width(); 
    cnt_sch_txdma_cos2.all( _val);
    _val = _val >> cnt_sch_txdma_cos2.get_width(); 
    cnt_sch_txdma_cos3.all( _val);
    _val = _val >> cnt_sch_txdma_cos3.get_width(); 
    cnt_sch_txdma_cos4.all( _val);
    _val = _val >> cnt_sch_txdma_cos4.get_width(); 
    cnt_sch_txdma_cos5.all( _val);
    _val = _val >> cnt_sch_txdma_cos5.get_width(); 
    cnt_sch_txdma_cos6.all( _val);
    _val = _val >> cnt_sch_txdma_cos6.get_width(); 
    cnt_sch_txdma_cos7.all( _val);
    _val = _val >> cnt_sch_txdma_cos7.get_width(); 
    cnt_sch_txdma_cos8.all( _val);
    _val = _val >> cnt_sch_txdma_cos8.get_width(); 
    cnt_sch_txdma_cos9.all( _val);
    _val = _val >> cnt_sch_txdma_cos9.get_width(); 
    cnt_sch_txdma_cos10.all( _val);
    _val = _val >> cnt_sch_txdma_cos10.get_width(); 
    cnt_sch_txdma_cos11.all( _val);
    _val = _val >> cnt_sch_txdma_cos11.get_width(); 
    cnt_sch_txdma_cos12.all( _val);
    _val = _val >> cnt_sch_txdma_cos12.get_width(); 
    cnt_sch_txdma_cos13.all( _val);
    _val = _val >> cnt_sch_txdma_cos13.get_width(); 
    cnt_sch_txdma_cos14.all( _val);
    _val = _val >> cnt_sch_txdma_cos14.get_width(); 
    cnt_sch_txdma_cos15.all( _val);
    _val = _val >> cnt_sch_txdma_cos15.get_width(); 
    sta_glb.all( _val);
    _val = _val >> sta_glb.get_width(); 
    cnt_sch_rlid_stop.all( _val);
    _val = _val >> cnt_sch_rlid_stop.get_width(); 
    cnt_sch_rlid_start.all( _val);
    _val = _val >> cnt_sch_rlid_start.get_width(); 
    sta_scheduler.all( _val);
    _val = _val >> sta_scheduler.get_width(); 
    sta_scheduler_rr.all( _val);
    _val = _val >> sta_scheduler_rr.get_width(); 
    cfg_scheduler_dbg.all( _val);
    _val = _val >> cfg_scheduler_dbg.get_width(); 
    cfg_scheduler_dbg2.all( _val);
    _val = _val >> cfg_scheduler_dbg2.get_width(); 
    sta_sch_max_hbm_byp.all( _val);
    _val = _val >> sta_sch_max_hbm_byp.get_width(); 
    cfg_tmr_cnt_sram.all( _val);
    _val = _val >> cfg_tmr_cnt_sram.get_width(); 
    cfg_sch_lif_map_sram.all( _val);
    _val = _val >> cfg_sch_lif_map_sram.get_width(); 
    cfg_sch_rlid_map_sram.all( _val);
    _val = _val >> cfg_sch_rlid_map_sram.get_width(); 
    cfg_sch_grp_sram.all( _val);
    _val = _val >> cfg_sch_grp_sram.get_width(); 
    sta_sch_lif_map_notactive.all( _val);
    _val = _val >> sta_sch_lif_map_notactive.get_width(); 
    sta_sch_lif_sg_mismatch.all( _val);
    _val = _val >> sta_sch_lif_sg_mismatch.get_width(); 
    sta_scheduler_dbg.all( _val);
    _val = _val >> sta_scheduler_dbg.get_width(); 
    sta_scheduler_dbg2.all( _val);
    _val = _val >> sta_scheduler_dbg2.get_width(); 
    sta_srams_ecc_tmr_cnt.all( _val);
    _val = _val >> sta_srams_ecc_tmr_cnt.get_width(); 
    sta_srams_ecc_sch_lif_map.all( _val);
    _val = _val >> sta_srams_ecc_sch_lif_map.get_width(); 
    sta_srams_ecc_sch_rlid_map.all( _val);
    _val = _val >> sta_srams_ecc_sch_rlid_map.get_width(); 
    sta_srams_ecc_sch_grp.all( _val);
    _val = _val >> sta_srams_ecc_sch_grp.get_width(); 
    cfg_sch_hbm_sram.all( _val);
    _val = _val >> cfg_sch_hbm_sram.get_width(); 
    cfg_sch_hbm_byp_sram.all( _val);
    _val = _val >> cfg_sch_hbm_byp_sram.get_width(); 
    sta_srams_sch_hbm.all( _val);
    _val = _val >> sta_srams_sch_hbm.get_width(); 
    sta_srams_sch_hbm_byp.all( _val);
    _val = _val >> sta_srams_sch_hbm_byp.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    sta_ftmr_max_bcnt.all( _val);
    _val = _val >> sta_ftmr_max_bcnt.get_width(); 
    sta_stmr_max_bcnt.all( _val);
    _val = _val >> sta_stmr_max_bcnt.get_width(); 
    cnt_sch_axi_rd_req.all( _val);
    _val = _val >> cnt_sch_axi_rd_req.get_width(); 
    cnt_sch_axi_rd_rsp.all( _val);
    _val = _val >> cnt_sch_axi_rd_rsp.get_width(); 
    cnt_sch_axi_rd_rsp_err.all( _val);
    _val = _val >> cnt_sch_axi_rd_rsp_err.get_width(); 
    cnt_sch_axi_rd_rsp_rerr.all( _val);
    _val = _val >> cnt_sch_axi_rd_rsp_rerr.get_width(); 
    cnt_sch_axi_rd_rsp_uexp.all( _val);
    _val = _val >> cnt_sch_axi_rd_rsp_uexp.get_width(); 
    cnt_sch_axi_wr_req.all( _val);
    _val = _val >> cnt_sch_axi_wr_req.get_width(); 
    cnt_sch_axi_bid.all( _val);
    _val = _val >> cnt_sch_axi_bid.get_width(); 
    cnt_sch_axi_bid_err.all( _val);
    _val = _val >> cnt_sch_axi_bid_err.get_width(); 
    cnt_sch_axi_bid_rerr.all( _val);
    _val = _val >> cnt_sch_axi_bid_rerr.get_width(); 
    cnt_sch_axi_bid_uexp.all( _val);
    _val = _val >> cnt_sch_axi_bid_uexp.get_width(); 
    cnt_tmr_axi_rd_req.all( _val);
    _val = _val >> cnt_tmr_axi_rd_req.get_width(); 
    cnt_tmr_axi_rd_rsp.all( _val);
    _val = _val >> cnt_tmr_axi_rd_rsp.get_width(); 
    cnt_tmr_axi_rd_rsp_err.all( _val);
    _val = _val >> cnt_tmr_axi_rd_rsp_err.get_width(); 
    cnt_tmr_axi_rd_rsp_rerr.all( _val);
    _val = _val >> cnt_tmr_axi_rd_rsp_rerr.get_width(); 
    cnt_tmr_axi_rd_rsp_uexp.all( _val);
    _val = _val >> cnt_tmr_axi_rd_rsp_uexp.get_width(); 
    cnt_tmr_axi_wr_req.all( _val);
    _val = _val >> cnt_tmr_axi_wr_req.get_width(); 
    cnt_tmr_axi_bid.all( _val);
    _val = _val >> cnt_tmr_axi_bid.get_width(); 
    cnt_tmr_axi_bid_err.all( _val);
    _val = _val >> cnt_tmr_axi_bid_err.get_width(); 
    cnt_tmr_axi_bid_rerr.all( _val);
    _val = _val >> cnt_tmr_axi_bid_rerr.get_width(); 
    cnt_tmr_axi_bid_uexp.all( _val);
    _val = _val >> cnt_tmr_axi_bid_uexp.get_width(); 
    dhs_fast_timer_start_no_stop.all( _val);
    _val = _val >> dhs_fast_timer_start_no_stop.get_width(); 
    dhs_fast_timer_pending.all( _val);
    _val = _val >> dhs_fast_timer_pending.get_width(); 
    dhs_slow_timer_start_no_stop.all( _val);
    _val = _val >> dhs_slow_timer_start_no_stop.get_width(); 
    dhs_slow_timer_pending.all( _val);
    _val = _val >> dhs_slow_timer_pending.get_width(); 
    dhs_dtdmlo_calendar.all( _val);
    _val = _val >> dhs_dtdmlo_calendar.get_width(); 
    dhs_dtdmhi_calendar.all( _val);
    _val = _val >> dhs_dtdmhi_calendar.get_width(); 
    dhs_sch_flags.all( _val);
    _val = _val >> dhs_sch_flags.get_width(); 
    dhs_doorbell.all( _val);
    _val = _val >> dhs_doorbell.get_width(); 
    dhs_sch_grp_entry.all( _val);
    _val = _val >> dhs_sch_grp_entry.get_width(); 
    dhs_sch_grp_cnt_entry.all( _val);
    _val = _val >> dhs_sch_grp_cnt_entry.get_width(); 
    dhs_rlid_stop.all( _val);
    _val = _val >> dhs_rlid_stop.get_width(); 
    dhs_tmr_cnt_sram.all( _val);
    _val = _val >> dhs_tmr_cnt_sram.get_width(); 
    dhs_sch_lif_map_sram.all( _val);
    _val = _val >> dhs_sch_lif_map_sram.get_width(); 
    dhs_sch_rlid_map_sram.all( _val);
    _val = _val >> dhs_sch_rlid_map_sram.get_width(); 
    dhs_sch_grp_sram.all( _val);
    _val = _val >> dhs_sch_grp_sram.get_width(); 
    int_srams_ecc.all( _val);
    _val = _val >> int_srams_ecc.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_sch.all( _val);
    _val = _val >> int_sch.get_width(); 
    int_tmr.all( _val);
    _val = _val >> int_tmr.get_width(); 
}

cpp_int cap_txs_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_tmr.get_width(); ret_val = ret_val  | int_tmr.all(); 
    ret_val = ret_val << int_sch.get_width(); ret_val = ret_val  | int_sch.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << int_srams_ecc.get_width(); ret_val = ret_val  | int_srams_ecc.all(); 
    ret_val = ret_val << dhs_sch_grp_sram.get_width(); ret_val = ret_val  | dhs_sch_grp_sram.all(); 
    ret_val = ret_val << dhs_sch_rlid_map_sram.get_width(); ret_val = ret_val  | dhs_sch_rlid_map_sram.all(); 
    ret_val = ret_val << dhs_sch_lif_map_sram.get_width(); ret_val = ret_val  | dhs_sch_lif_map_sram.all(); 
    ret_val = ret_val << dhs_tmr_cnt_sram.get_width(); ret_val = ret_val  | dhs_tmr_cnt_sram.all(); 
    ret_val = ret_val << dhs_rlid_stop.get_width(); ret_val = ret_val  | dhs_rlid_stop.all(); 
    ret_val = ret_val << dhs_sch_grp_cnt_entry.get_width(); ret_val = ret_val  | dhs_sch_grp_cnt_entry.all(); 
    ret_val = ret_val << dhs_sch_grp_entry.get_width(); ret_val = ret_val  | dhs_sch_grp_entry.all(); 
    ret_val = ret_val << dhs_doorbell.get_width(); ret_val = ret_val  | dhs_doorbell.all(); 
    ret_val = ret_val << dhs_sch_flags.get_width(); ret_val = ret_val  | dhs_sch_flags.all(); 
    ret_val = ret_val << dhs_dtdmhi_calendar.get_width(); ret_val = ret_val  | dhs_dtdmhi_calendar.all(); 
    ret_val = ret_val << dhs_dtdmlo_calendar.get_width(); ret_val = ret_val  | dhs_dtdmlo_calendar.all(); 
    ret_val = ret_val << dhs_slow_timer_pending.get_width(); ret_val = ret_val  | dhs_slow_timer_pending.all(); 
    ret_val = ret_val << dhs_slow_timer_start_no_stop.get_width(); ret_val = ret_val  | dhs_slow_timer_start_no_stop.all(); 
    ret_val = ret_val << dhs_fast_timer_pending.get_width(); ret_val = ret_val  | dhs_fast_timer_pending.all(); 
    ret_val = ret_val << dhs_fast_timer_start_no_stop.get_width(); ret_val = ret_val  | dhs_fast_timer_start_no_stop.all(); 
    ret_val = ret_val << cnt_tmr_axi_bid_uexp.get_width(); ret_val = ret_val  | cnt_tmr_axi_bid_uexp.all(); 
    ret_val = ret_val << cnt_tmr_axi_bid_rerr.get_width(); ret_val = ret_val  | cnt_tmr_axi_bid_rerr.all(); 
    ret_val = ret_val << cnt_tmr_axi_bid_err.get_width(); ret_val = ret_val  | cnt_tmr_axi_bid_err.all(); 
    ret_val = ret_val << cnt_tmr_axi_bid.get_width(); ret_val = ret_val  | cnt_tmr_axi_bid.all(); 
    ret_val = ret_val << cnt_tmr_axi_wr_req.get_width(); ret_val = ret_val  | cnt_tmr_axi_wr_req.all(); 
    ret_val = ret_val << cnt_tmr_axi_rd_rsp_uexp.get_width(); ret_val = ret_val  | cnt_tmr_axi_rd_rsp_uexp.all(); 
    ret_val = ret_val << cnt_tmr_axi_rd_rsp_rerr.get_width(); ret_val = ret_val  | cnt_tmr_axi_rd_rsp_rerr.all(); 
    ret_val = ret_val << cnt_tmr_axi_rd_rsp_err.get_width(); ret_val = ret_val  | cnt_tmr_axi_rd_rsp_err.all(); 
    ret_val = ret_val << cnt_tmr_axi_rd_rsp.get_width(); ret_val = ret_val  | cnt_tmr_axi_rd_rsp.all(); 
    ret_val = ret_val << cnt_tmr_axi_rd_req.get_width(); ret_val = ret_val  | cnt_tmr_axi_rd_req.all(); 
    ret_val = ret_val << cnt_sch_axi_bid_uexp.get_width(); ret_val = ret_val  | cnt_sch_axi_bid_uexp.all(); 
    ret_val = ret_val << cnt_sch_axi_bid_rerr.get_width(); ret_val = ret_val  | cnt_sch_axi_bid_rerr.all(); 
    ret_val = ret_val << cnt_sch_axi_bid_err.get_width(); ret_val = ret_val  | cnt_sch_axi_bid_err.all(); 
    ret_val = ret_val << cnt_sch_axi_bid.get_width(); ret_val = ret_val  | cnt_sch_axi_bid.all(); 
    ret_val = ret_val << cnt_sch_axi_wr_req.get_width(); ret_val = ret_val  | cnt_sch_axi_wr_req.all(); 
    ret_val = ret_val << cnt_sch_axi_rd_rsp_uexp.get_width(); ret_val = ret_val  | cnt_sch_axi_rd_rsp_uexp.all(); 
    ret_val = ret_val << cnt_sch_axi_rd_rsp_rerr.get_width(); ret_val = ret_val  | cnt_sch_axi_rd_rsp_rerr.all(); 
    ret_val = ret_val << cnt_sch_axi_rd_rsp_err.get_width(); ret_val = ret_val  | cnt_sch_axi_rd_rsp_err.all(); 
    ret_val = ret_val << cnt_sch_axi_rd_rsp.get_width(); ret_val = ret_val  | cnt_sch_axi_rd_rsp.all(); 
    ret_val = ret_val << cnt_sch_axi_rd_req.get_width(); ret_val = ret_val  | cnt_sch_axi_rd_req.all(); 
    ret_val = ret_val << sta_stmr_max_bcnt.get_width(); ret_val = ret_val  | sta_stmr_max_bcnt.all(); 
    ret_val = ret_val << sta_ftmr_max_bcnt.get_width(); ret_val = ret_val  | sta_ftmr_max_bcnt.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_srams_sch_hbm_byp.get_width(); ret_val = ret_val  | sta_srams_sch_hbm_byp.all(); 
    ret_val = ret_val << sta_srams_sch_hbm.get_width(); ret_val = ret_val  | sta_srams_sch_hbm.all(); 
    ret_val = ret_val << cfg_sch_hbm_byp_sram.get_width(); ret_val = ret_val  | cfg_sch_hbm_byp_sram.all(); 
    ret_val = ret_val << cfg_sch_hbm_sram.get_width(); ret_val = ret_val  | cfg_sch_hbm_sram.all(); 
    ret_val = ret_val << sta_srams_ecc_sch_grp.get_width(); ret_val = ret_val  | sta_srams_ecc_sch_grp.all(); 
    ret_val = ret_val << sta_srams_ecc_sch_rlid_map.get_width(); ret_val = ret_val  | sta_srams_ecc_sch_rlid_map.all(); 
    ret_val = ret_val << sta_srams_ecc_sch_lif_map.get_width(); ret_val = ret_val  | sta_srams_ecc_sch_lif_map.all(); 
    ret_val = ret_val << sta_srams_ecc_tmr_cnt.get_width(); ret_val = ret_val  | sta_srams_ecc_tmr_cnt.all(); 
    ret_val = ret_val << sta_scheduler_dbg2.get_width(); ret_val = ret_val  | sta_scheduler_dbg2.all(); 
    ret_val = ret_val << sta_scheduler_dbg.get_width(); ret_val = ret_val  | sta_scheduler_dbg.all(); 
    ret_val = ret_val << sta_sch_lif_sg_mismatch.get_width(); ret_val = ret_val  | sta_sch_lif_sg_mismatch.all(); 
    ret_val = ret_val << sta_sch_lif_map_notactive.get_width(); ret_val = ret_val  | sta_sch_lif_map_notactive.all(); 
    ret_val = ret_val << cfg_sch_grp_sram.get_width(); ret_val = ret_val  | cfg_sch_grp_sram.all(); 
    ret_val = ret_val << cfg_sch_rlid_map_sram.get_width(); ret_val = ret_val  | cfg_sch_rlid_map_sram.all(); 
    ret_val = ret_val << cfg_sch_lif_map_sram.get_width(); ret_val = ret_val  | cfg_sch_lif_map_sram.all(); 
    ret_val = ret_val << cfg_tmr_cnt_sram.get_width(); ret_val = ret_val  | cfg_tmr_cnt_sram.all(); 
    ret_val = ret_val << sta_sch_max_hbm_byp.get_width(); ret_val = ret_val  | sta_sch_max_hbm_byp.all(); 
    ret_val = ret_val << cfg_scheduler_dbg2.get_width(); ret_val = ret_val  | cfg_scheduler_dbg2.all(); 
    ret_val = ret_val << cfg_scheduler_dbg.get_width(); ret_val = ret_val  | cfg_scheduler_dbg.all(); 
    ret_val = ret_val << sta_scheduler_rr.get_width(); ret_val = ret_val  | sta_scheduler_rr.all(); 
    ret_val = ret_val << sta_scheduler.get_width(); ret_val = ret_val  | sta_scheduler.all(); 
    ret_val = ret_val << cnt_sch_rlid_start.get_width(); ret_val = ret_val  | cnt_sch_rlid_start.all(); 
    ret_val = ret_val << cnt_sch_rlid_stop.get_width(); ret_val = ret_val  | cnt_sch_rlid_stop.all(); 
    ret_val = ret_val << sta_glb.get_width(); ret_val = ret_val  | sta_glb.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos15.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos15.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos14.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos14.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos13.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos13.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos12.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos12.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos11.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos11.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos10.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos10.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos9.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos9.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos8.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos8.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos7.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos7.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos6.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos6.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos5.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos5.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos4.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos4.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos3.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos3.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos2.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos2.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos1.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos1.all(); 
    ret_val = ret_val << cnt_sch_txdma_cos0.get_width(); ret_val = ret_val  | cnt_sch_txdma_cos0.all(); 
    ret_val = ret_val << cnt_sch_fifo_empty.get_width(); ret_val = ret_val  | cnt_sch_fifo_empty.all(); 
    ret_val = ret_val << cnt_sch_doorbell_clr.get_width(); ret_val = ret_val  | cnt_sch_doorbell_clr.all(); 
    ret_val = ret_val << cnt_sch_doorbell_set.get_width(); ret_val = ret_val  | cnt_sch_doorbell_set.all(); 
    ret_val = ret_val << cfg_sch.get_width(); ret_val = ret_val  | cfg_sch.all(); 
    ret_val = ret_val << cfw_scheduler_static.get_width(); ret_val = ret_val  | cfw_scheduler_static.all(); 
    ret_val = ret_val << cfw_scheduler_glb.get_width(); ret_val = ret_val  | cfw_scheduler_glb.all(); 
    ret_val = ret_val << sta_srams_tmr_fifo.get_width(); ret_val = ret_val  | sta_srams_tmr_fifo.all(); 
    ret_val = ret_val << sta_srams_tmr_hbm_byp.get_width(); ret_val = ret_val  | sta_srams_tmr_hbm_byp.all(); 
    ret_val = ret_val << sta_srams_tmr_hbm.get_width(); ret_val = ret_val  | sta_srams_tmr_hbm.all(); 
    ret_val = ret_val << cfg_tmr_fifo_sram.get_width(); ret_val = ret_val  | cfg_tmr_fifo_sram.all(); 
    ret_val = ret_val << cfg_tmr_hbm_byp_sram.get_width(); ret_val = ret_val  | cfg_tmr_hbm_byp_sram.all(); 
    ret_val = ret_val << cfg_tmr_hbm_sram.get_width(); ret_val = ret_val  | cfg_tmr_hbm_sram.all(); 
    ret_val = ret_val << cnt_stmr_pop.get_width(); ret_val = ret_val  | cnt_stmr_pop.all(); 
    ret_val = ret_val << cnt_stmr_key_not_found.get_width(); ret_val = ret_val  | cnt_stmr_key_not_found.all(); 
    ret_val = ret_val << cnt_stmr_push_out_of_wheel.get_width(); ret_val = ret_val  | cnt_stmr_push_out_of_wheel.all(); 
    ret_val = ret_val << cnt_stmr_key_not_push.get_width(); ret_val = ret_val  | cnt_stmr_key_not_push.all(); 
    ret_val = ret_val << cnt_stmr_push.get_width(); ret_val = ret_val  | cnt_stmr_push.all(); 
    ret_val = ret_val << sta_slow_timer.get_width(); ret_val = ret_val  | sta_slow_timer.all(); 
    ret_val = ret_val << cfg_slow_timer_dbell.get_width(); ret_val = ret_val  | cfg_slow_timer_dbell.all(); 
    ret_val = ret_val << cfg_force_slow_timer.get_width(); ret_val = ret_val  | cfg_force_slow_timer.all(); 
    ret_val = ret_val << cfg_slow_timer.get_width(); ret_val = ret_val  | cfg_slow_timer.all(); 
    ret_val = ret_val << cnt_ftmr_pop.get_width(); ret_val = ret_val  | cnt_ftmr_pop.all(); 
    ret_val = ret_val << cnt_ftmr_key_not_found.get_width(); ret_val = ret_val  | cnt_ftmr_key_not_found.all(); 
    ret_val = ret_val << cnt_ftmr_push_out_of_wheel.get_width(); ret_val = ret_val  | cnt_ftmr_push_out_of_wheel.all(); 
    ret_val = ret_val << cnt_ftmr_key_not_push.get_width(); ret_val = ret_val  | cnt_ftmr_key_not_push.all(); 
    ret_val = ret_val << cnt_ftmr_push.get_width(); ret_val = ret_val  | cnt_ftmr_push.all(); 
    ret_val = ret_val << sta_fast_timer.get_width(); ret_val = ret_val  | sta_fast_timer.all(); 
    ret_val = ret_val << cfg_fast_timer_dbell.get_width(); ret_val = ret_val  | cfg_fast_timer_dbell.all(); 
    ret_val = ret_val << cfg_force_fast_timer.get_width(); ret_val = ret_val  | cfg_force_fast_timer.all(); 
    ret_val = ret_val << cfg_fast_timer.get_width(); ret_val = ret_val  | cfg_fast_timer.all(); 
    ret_val = ret_val << sta_timer_dbg2.get_width(); ret_val = ret_val  | sta_timer_dbg2.all(); 
    ret_val = ret_val << sta_timer_dbg.get_width(); ret_val = ret_val  | sta_timer_dbg.all(); 
    ret_val = ret_val << sta_tmr_max_keys.get_width(); ret_val = ret_val  | sta_tmr_max_keys.all(); 
    ret_val = ret_val << sta_tmr_max_hbm_byp.get_width(); ret_val = ret_val  | sta_tmr_max_hbm_byp.all(); 
    ret_val = ret_val << sta_timer.get_width(); ret_val = ret_val  | sta_timer.all(); 
    ret_val = ret_val << cfg_timer_dbg2.get_width(); ret_val = ret_val  | cfg_timer_dbg2.all(); 
    ret_val = ret_val << cfg_timer_dbg.get_width(); ret_val = ret_val  | cfg_timer_dbg.all(); 
    ret_val = ret_val << cfw_timer_glb.get_width(); ret_val = ret_val  | cfw_timer_glb.all(); 
    ret_val = ret_val << cfg_timer_static.get_width(); ret_val = ret_val  | cfg_timer_static.all(); 
    ret_val = ret_val << cfg_axi_attr.get_width(); ret_val = ret_val  | cfg_axi_attr.all(); 
    ret_val = ret_val << cfg_glb.get_width(); ret_val = ret_val  | cfg_glb.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_txs_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_glb_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__spare; 
    
    // dbg_port_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dbg_port_enable; 
    
    // dbg_port_select
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__dbg_port_select; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_axi_attr_t::all() const {
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

cpp_int cap_txs_csr_cfg_timer_static_t::all() const {
    cpp_int ret_val;

    // tmr_wheel_depth
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__tmr_wheel_depth; 
    
    // tmr_hsh_depth
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__tmr_hsh_depth; 
    
    // hbm_base
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__hbm_base; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfw_timer_glb_t::all() const {
    cpp_int ret_val;

    // stmr_pause
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_pause; 
    
    // ftmr_pause
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_pause; 
    
    // stmr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_enable; 
    
    // ftmr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_enable; 
    
    // spare
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__spare; 
    
    // sram_hw_init
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram_hw_init; 
    
    // hbm_hw_init
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_hw_init; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__spare; 
    
    // max_tmr_fifo
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__max_tmr_fifo; 
    
    // max_hbm_byp
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__max_hbm_byp; 
    
    // max_hbm_rd
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__max_hbm_rd; 
    
    // max_hbm_wr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__max_hbm_wr; 
    
    // tmr_stall_thr_lo
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__tmr_stall_thr_lo; 
    
    // tmr_stall_thr_hi
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__tmr_stall_thr_hi; 
    
    // drb_efc_thr
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__drb_efc_thr; 
    
    // hbm_efc_thr
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__hbm_efc_thr; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_timer_dbg2_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__spare; 
    
    // max_bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__max_bcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_timer_t::all() const {
    cpp_int ret_val;

    // sram_init_done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram_init_done; 
    
    // hbm_init_done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_init_done; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_tmr_max_hbm_byp_t::all() const {
    cpp_int ret_val;

    // cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__cnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_tmr_max_keys_t::all() const {
    cpp_int ret_val;

    // cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__cnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_timer_dbg_t::all() const {
    cpp_int ret_val;

    // ftmr_fifo_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_fifo_efull; 
    
    // stmr_fifo_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_fifo_efull; 
    
    // ftmr_fifo_waddr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ftmr_fifo_waddr; 
    
    // stmr_fifo_waddr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__stmr_fifo_waddr; 
    
    // ftmr_fifo_raddr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__ftmr_fifo_raddr; 
    
    // stmr_fifo_raddr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__stmr_fifo_raddr; 
    
    // hbm_wr_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_wr_pending_efull; 
    
    // hbm_rd_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_rd_pending_efull; 
    
    // hbm_byp_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_byp_pending_efull; 
    
    // rejct_drb_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rejct_drb_efull; 
    
    // hbm_we_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_we_efull; 
    
    // hbm_re_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_re_efull; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::all() const {
    cpp_int ret_val;

    // ftmr_stall
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_stall; 
    
    // stmr_stall
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_stall; 
    
    // hbm_wr_pending_cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__hbm_wr_pending_cnt; 
    
    // hbm_rd_pending_cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__hbm_rd_pending_cnt; 
    
    // hbm_byp_pending_cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__hbm_byp_pending_cnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_fast_timer_t::all() const {
    cpp_int ret_val;

    // hash_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__hash_sel; 
    
    // tick
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__tick; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_force_fast_timer_t::all() const {
    cpp_int ret_val;

    // ctime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__ctime; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_fast_timer_dbell_t::all() const {
    cpp_int ret_val;

    // data_index
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_index; 
    
    // data_reserved
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_reserved; 
    
    // data_pid
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_pid; 
    
    // addr_update
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__addr_update; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_fast_timer_t::all() const {
    cpp_int ret_val;

    // pTime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__pTime; 
    
    // cTime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__cTime; 
    
    // tick
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__tick; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_ftmr_push_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_ftmr_key_not_push_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_ftmr_key_not_found_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_ftmr_pop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_slow_timer_t::all() const {
    cpp_int ret_val;

    // hash_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__hash_sel; 
    
    // tick
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__tick; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_force_slow_timer_t::all() const {
    cpp_int ret_val;

    // ctime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__ctime; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_slow_timer_dbell_t::all() const {
    cpp_int ret_val;

    // data_index
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_index; 
    
    // data_reserved
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_reserved; 
    
    // data_pid
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__data_pid; 
    
    // addr_update
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__addr_update; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_slow_timer_t::all() const {
    cpp_int ret_val;

    // pTime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__pTime; 
    
    // cTime
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__cTime; 
    
    // tick
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__tick; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_stmr_push_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_stmr_key_not_push_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_stmr_push_out_of_wheel_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_stmr_key_not_found_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_stmr_pop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_tmr_hbm_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_tmr_hbm_byp_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_tmr_fifo_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_byp_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_tmr_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::all() const {
    cpp_int ret_val;

    // enable_set_byp
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_set_byp; 
    
    // enable_set_lkup
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_set_lkup; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    // spare
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__spare; 
    
    // sram_hw_init
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram_hw_init; 
    
    // hbm_hw_init
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_hw_init; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfw_scheduler_static_t::all() const {
    cpp_int ret_val;

    // sch_grp_depth
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__sch_grp_depth; 
    
    // hbm_base
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__hbm_base; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_t::all() const {
    cpp_int ret_val;

    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    // pause
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__pause; 
    
    // timeout
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__timeout; 
    
    // dtdm_hi_map
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__dtdm_hi_map; 
    
    // dtdm_lo_map
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__dtdm_lo_map; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_doorbell_set_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_doorbell_clr_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_fifo_empty_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos0_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos1_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos2_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos3_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos4_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos5_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos6_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos7_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos8_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos9_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos10_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos11_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos12_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos13_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos14_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos15_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_glb_t::all() const {
    cpp_int ret_val;

    // pb_xoff
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__pb_xoff; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_rlid_stop_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_rlid_start_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_scheduler_t::all() const {
    cpp_int ret_val;

    // sram_init_done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram_init_done; 
    
    // hbm_init_done
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_init_done; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::all() const {
    cpp_int ret_val;

    // curr_ptr15
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr15; 
    
    // curr_ptr14
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr14; 
    
    // curr_ptr13
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr13; 
    
    // curr_ptr12
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr12; 
    
    // curr_ptr11
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr11; 
    
    // curr_ptr10
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr10; 
    
    // curr_ptr9
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr9; 
    
    // curr_ptr8
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr8; 
    
    // curr_ptr7
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr7; 
    
    // curr_ptr6
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr6; 
    
    // curr_ptr5
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr5; 
    
    // curr_ptr4
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr4; 
    
    // curr_ptr3
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr3; 
    
    // curr_ptr2
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr2; 
    
    // curr_ptr1
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr1; 
    
    // curr_ptr0
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__curr_ptr0; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__spare; 
    
    // max_hbm_rd
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__max_hbm_rd; 
    
    // max_hbm_wr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__max_hbm_wr; 
    
    // max_hbm_byp
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__max_hbm_byp; 
    
    // fifo_mode_thr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__fifo_mode_thr; 
    
    // txdma_efc_thr
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__txdma_efc_thr; 
    
    // hbm_efc_thr
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__hbm_efc_thr; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_scheduler_dbg2_t::all() const {
    cpp_int ret_val;

    // cos_read
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cos_read; 
    
    // qid_read
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid_read; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_sch_max_hbm_byp_t::all() const {
    cpp_int ret_val;

    // cnt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__cnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_tmr_cnt_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // dhs_eccbypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_eccbypass; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_lif_map_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // dhs_eccbypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_eccbypass; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_rlid_map_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // dhs_eccbypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_eccbypass; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_grp_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // dhs_eccbypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dhs_eccbypass; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_sch_lif_map_notactive_t::all() const {
    cpp_int ret_val;

    // set
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__set; 
    
    // cos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cos; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_sch_lif_sg_mismatch_t::all() const {
    cpp_int ret_val;

    // set
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__set; 
    
    // qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::all() const {
    cpp_int ret_val;

    // hbm_wr_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_wr_pending_efull; 
    
    // hbm_rd_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_rd_pending_efull; 
    
    // hbm_byp_pending_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_byp_pending_efull; 
    
    // txdma_msg_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txdma_msg_efull; 
    
    // hbm_we_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_we_efull; 
    
    // hbm_re_efull
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_re_efull; 
    
    // txdma_drdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__txdma_drdy; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_scheduler_dbg2_t::all() const {
    cpp_int ret_val;

    // hbm_wr_pending_cnt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__hbm_wr_pending_cnt; 
    
    // hbm_rd_pending_cnt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__hbm_rd_pending_cnt; 
    
    // hbm_byp_pending_cnt
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__hbm_byp_pending_cnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_hbm_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cfg_sch_hbm_byp_sram_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_byp_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_txs_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_ftmr_max_bcnt_t::all() const {
    cpp_int ret_val;

    // bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__bcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_sta_stmr_max_bcnt_t::all() const {
    cpp_int ret_val;

    // bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__bcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_req_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_wr_req_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_err_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_rerr_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_uexp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_req_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_wr_req_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 48; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_err_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_rerr_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_uexp_t::all() const {
    cpp_int ret_val;

    // val
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__val; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::all() const {
    cpp_int ret_val;

    // dtime
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__dtime; 
    
    // ring
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__ring; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    // typ
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__typ; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_fast_timer_pending_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_fast_timer_pending_entry_t::all() const {
    cpp_int ret_val;

    // lcnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__lcnt; 
    
    // bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__bcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::all() const {
    cpp_int ret_val;

    // dtime
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__dtime; 
    
    // ring
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__ring; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    // typ
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__typ; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_slow_timer_pending_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_slow_timer_pending_entry_t::all() const {
    cpp_int ret_val;

    // lcnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__lcnt; 
    
    // bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__bcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_dtdmlo_calendar_t::all() const {
    cpp_int ret_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_dtdmlo_calendar_entry_t::all() const {
    cpp_int ret_val;

    // dtdm_calendar
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__dtdm_calendar; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_dtdmhi_calendar_t::all() const {
    cpp_int ret_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_dtdmhi_calendar_entry_t::all() const {
    cpp_int ret_val;

    // dtdm_calendar
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__dtdm_calendar; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_flags_t::all() const {
    cpp_int ret_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_flags_entry_t::all() const {
    cpp_int ret_val;

    // sch_flags
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__sch_flags; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_doorbell_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_doorbell_entry_t::all() const {
    cpp_int ret_val;

    // set
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__set; 
    
    // cos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cos; 
    
    // qid
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__qid; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::all() const {
    cpp_int ret_val;

    // rl_thr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rl_thr; 
    
    // rr_sel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__rr_sel; 
    
    // auto_clr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__auto_clr; 
    
    // qid_offset
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__qid_offset; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_cnt_entry_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::all() const {
    cpp_int ret_val;

    // drb_cnt
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__drb_cnt; 
    
    // popcnt
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__popcnt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_rlid_stop_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_rlid_stop_entry_t::all() const {
    cpp_int ret_val;

    // setmsk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__setmsk; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_t::all() const {
    cpp_int ret_val;

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 4096-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__ecc; 
    
    // fast_lcnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__fast_lcnt; 
    
    // fast_bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__fast_bcnt; 
    
    // fast_cbcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__fast_cbcnt; 
    
    // slow_lcnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__slow_lcnt; 
    
    // slow_bcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__slow_bcnt; 
    
    // slow_cbcnt
    ret_val = ret_val << 18; ret_val = ret_val  | int_var__slow_cbcnt; 
    
    // spare
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ecc; 
    
    // sg_act_cos
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__sg_act_cos; 
    
    // sg_per_cos
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__sg_per_cos; 
    
    // sg_start
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__sg_start; 
    
    // sg_active
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sg_active; 
    
    // spare
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__ecc; 
    
    // sg_end
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__sg_end; 
    
    // sg_start
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__sg_start; 
    
    // spare
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_t::all() const {
    cpp_int ret_val;

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 2048-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__ecc; 
    
    // popcnt
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__popcnt; 
    
    // hbm_ln_cnt15
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt15; 
    
    // hbm_ln_cnt14
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt14; 
    
    // hbm_ln_cnt13
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt13; 
    
    // hbm_ln_cnt12
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt12; 
    
    // hbm_ln_cnt11
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt11; 
    
    // hbm_ln_cnt10
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt10; 
    
    // hbm_ln_cnt9
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt9; 
    
    // hbm_ln_cnt8
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt8; 
    
    // hbm_ln_cnt7
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt7; 
    
    // hbm_ln_cnt6
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt6; 
    
    // hbm_ln_cnt5
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt5; 
    
    // hbm_ln_cnt4
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt4; 
    
    // hbm_ln_cnt3
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt3; 
    
    // hbm_ln_cnt2
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt2; 
    
    // hbm_ln_cnt1
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt1; 
    
    // hbm_ln_cnt0
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__hbm_ln_cnt0; 
    
    // drb_cnt
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__drb_cnt; 
    
    // hbm_rr_cnt
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__hbm_rr_cnt; 
    
    // hbm_ln_ptr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__hbm_ln_ptr; 
    
    // hbm_ln
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__hbm_ln; 
    
    // qid_fifo_elm15_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm15_qid; 
    
    // qid_fifo_elm15_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm15_jnk; 
    
    // qid_fifo_elm15_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm15_vld; 
    
    // qid_fifo_elm14_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm14_qid; 
    
    // qid_fifo_elm14_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm14_jnk; 
    
    // qid_fifo_elm14_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm14_vld; 
    
    // qid_fifo_elm13_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm13_qid; 
    
    // qid_fifo_elm13_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm13_jnk; 
    
    // qid_fifo_elm13_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm13_vld; 
    
    // qid_fifo_elm12_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm12_qid; 
    
    // qid_fifo_elm12_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm12_jnk; 
    
    // qid_fifo_elm12_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm12_vld; 
    
    // qid_fifo_elm11_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm11_qid; 
    
    // qid_fifo_elm11_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm11_jnk; 
    
    // qid_fifo_elm11_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm11_vld; 
    
    // qid_fifo_elm10_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm10_qid; 
    
    // qid_fifo_elm10_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm10_jnk; 
    
    // qid_fifo_elm10_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm10_vld; 
    
    // qid_fifo_elm9_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm9_qid; 
    
    // qid_fifo_elm9_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm9_jnk; 
    
    // qid_fifo_elm9_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm9_vld; 
    
    // qid_fifo_elm8_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm8_qid; 
    
    // qid_fifo_elm8_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm8_jnk; 
    
    // qid_fifo_elm8_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm8_vld; 
    
    // qid_fifo_elm7_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm7_qid; 
    
    // qid_fifo_elm7_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm7_jnk; 
    
    // qid_fifo_elm7_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm7_vld; 
    
    // qid_fifo_elm6_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm6_qid; 
    
    // qid_fifo_elm6_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm6_jnk; 
    
    // qid_fifo_elm6_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm6_vld; 
    
    // qid_fifo_elm5_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm5_qid; 
    
    // qid_fifo_elm5_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm5_jnk; 
    
    // qid_fifo_elm5_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm5_vld; 
    
    // qid_fifo_elm4_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm4_qid; 
    
    // qid_fifo_elm4_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm4_jnk; 
    
    // qid_fifo_elm4_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm4_vld; 
    
    // qid_fifo_elm3_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm3_qid; 
    
    // qid_fifo_elm3_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm3_jnk; 
    
    // qid_fifo_elm3_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm3_vld; 
    
    // qid_fifo_elm2_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm2_qid; 
    
    // qid_fifo_elm2_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm2_jnk; 
    
    // qid_fifo_elm2_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm2_vld; 
    
    // qid_fifo_elm1_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm1_qid; 
    
    // qid_fifo_elm1_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm1_jnk; 
    
    // qid_fifo_elm1_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm1_vld; 
    
    // qid_fifo_elm0_qid
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__qid_fifo_elm0_qid; 
    
    // qid_fifo_elm0_jnk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm0_jnk; 
    
    // qid_fifo_elm0_vld
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__qid_fifo_elm0_vld; 
    
    // rr_sel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__rr_sel; 
    
    // auto_clr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__auto_clr; 
    
    // lif
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__lif; 
    
    // qid_offset
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__qid_offset; 
    
    // hbm_mode
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__hbm_mode; 
    
    // bckgr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bckgr; 
    
    // bckgr_cnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__bckgr_cnt; 
    
    // rl_thr
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rl_thr; 
    
    // clr_pend_cnt
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__clr_pend_cnt; 
    
    // spare
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_srams_ecc_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::all() const {
    cpp_int ret_val;

    // sch_grp_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_grp_correctable_interrupt; 
    
    // sch_grp_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_grp_uncorrectable_interrupt; 
    
    // sch_rlid_map_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_map_correctable_interrupt; 
    
    // sch_rlid_map_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_map_uncorrectable_interrupt; 
    
    // sch_lif_map_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_map_correctable_interrupt; 
    
    // sch_lif_map_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_map_uncorrectable_interrupt; 
    
    // tmr_cnt_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_cnt_correctable_interrupt; 
    
    // tmr_cnt_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_cnt_uncorrectable_interrupt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::all() const {
    cpp_int ret_val;

    // sch_grp_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_grp_correctable_enable; 
    
    // sch_grp_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_grp_uncorrectable_enable; 
    
    // sch_rlid_map_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_map_correctable_enable; 
    
    // sch_rlid_map_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_map_uncorrectable_enable; 
    
    // sch_lif_map_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_map_correctable_enable; 
    
    // sch_lif_map_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_map_uncorrectable_enable; 
    
    // tmr_cnt_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_cnt_correctable_enable; 
    
    // tmr_cnt_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_cnt_uncorrectable_enable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_txs_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_srams_ecc_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_srams_ecc_interrupt; 
    
    // int_tmr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_tmr_interrupt; 
    
    // int_sch_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_sch_interrupt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_srams_ecc_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_srams_ecc_enable; 
    
    // int_tmr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_tmr_enable; 
    
    // int_sch_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_sch_enable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_sch_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::all() const {
    cpp_int ret_val;

    // sch_txdma_msg_ovfl_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_txdma_msg_ovfl_interrupt; 
    
    // sch_drb_cnt_unfl_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_unfl_interrupt; 
    
    // sch_drb_cnt_ovfl_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_ovfl_interrupt; 
    
    // sch_rid_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rid_err_interrupt; 
    
    // sch_rresp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rresp_err_interrupt; 
    
    // sch_bid_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_bid_err_interrupt; 
    
    // sch_bresp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_bresp_err_interrupt; 
    
    // sch_hbm_wr_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_wr_pending_efull_interrupt; 
    
    // sch_hbm_rd_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_rd_pending_efull_interrupt; 
    
    // sch_hbm_byp_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_pending_efull_interrupt; 
    
    // sch_hbm_we_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_we_efull_interrupt; 
    
    // sch_hbm_re_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_re_efull_interrupt; 
    
    // sch_txdma_msg_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_txdma_msg_efull_interrupt; 
    
    // sch_aclr_hbm_ln_rollovr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_aclr_hbm_ln_rollovr_interrupt; 
    
    // sch_lif_sg_mismatch_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_sg_mismatch_interrupt; 
    
    // sch_null_lif_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_null_lif_interrupt; 
    
    // sch_rlid_unfl_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_unfl_interrupt; 
    
    // sch_rlid_ovfl_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_ovfl_interrupt; 
    
    // sch_hbm_byp_wtag_wrap_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_wtag_wrap_interrupt; 
    
    // sch_hbm_byp_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_ovf_interrupt; 
    
    // sch_drb_cnt_qid_fifo_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_qid_fifo_interrupt; 
    
    // sch_state_fifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_state_fifo_ovf_interrupt; 
    
    // sch_rd_txfifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rd_txfifo_ovf_interrupt; 
    
    // sch_wr_txfifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_wr_txfifo_ovf_interrupt; 
    
    // sch_rid_empty_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rid_empty_interrupt; 
    
    // sch_wid_empty_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_wid_empty_interrupt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // sch_txdma_msg_ovfl_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_txdma_msg_ovfl_enable; 
    
    // sch_drb_cnt_unfl_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_unfl_enable; 
    
    // sch_drb_cnt_ovfl_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_ovfl_enable; 
    
    // sch_rid_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rid_err_enable; 
    
    // sch_rresp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rresp_err_enable; 
    
    // sch_bid_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_bid_err_enable; 
    
    // sch_bresp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_bresp_err_enable; 
    
    // sch_hbm_wr_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_wr_pending_efull_enable; 
    
    // sch_hbm_rd_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_rd_pending_efull_enable; 
    
    // sch_hbm_byp_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_pending_efull_enable; 
    
    // sch_hbm_we_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_we_efull_enable; 
    
    // sch_hbm_re_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_re_efull_enable; 
    
    // sch_txdma_msg_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_txdma_msg_efull_enable; 
    
    // sch_aclr_hbm_ln_rollovr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_aclr_hbm_ln_rollovr_enable; 
    
    // sch_lif_sg_mismatch_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_lif_sg_mismatch_enable; 
    
    // sch_null_lif_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_null_lif_enable; 
    
    // sch_rlid_unfl_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_unfl_enable; 
    
    // sch_rlid_ovfl_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rlid_ovfl_enable; 
    
    // sch_hbm_byp_wtag_wrap_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_wtag_wrap_enable; 
    
    // sch_hbm_byp_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_hbm_byp_ovf_enable; 
    
    // sch_drb_cnt_qid_fifo_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_drb_cnt_qid_fifo_enable; 
    
    // sch_state_fifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_state_fifo_ovf_enable; 
    
    // sch_rd_txfifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rd_txfifo_ovf_enable; 
    
    // sch_wr_txfifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_wr_txfifo_ovf_enable; 
    
    // sch_rid_empty_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_rid_empty_enable; 
    
    // sch_wid_empty_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sch_wid_empty_enable; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_tmr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::all() const {
    cpp_int ret_val;

    // tmr_rid_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rid_err_interrupt; 
    
    // tmr_rresp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rresp_err_interrupt; 
    
    // tmr_bid_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_bid_err_interrupt; 
    
    // tmr_bresp_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_bresp_err_interrupt; 
    
    // tmr_hbm_wr_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_wr_pending_efull_interrupt; 
    
    // tmr_hbm_rd_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_rd_pending_efull_interrupt; 
    
    // tmr_hbm_byp_pending_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_pending_efull_interrupt; 
    
    // tmr_hbm_we_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_we_efull_interrupt; 
    
    // tmr_hbm_re_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_re_efull_interrupt; 
    
    // tmr_rejct_drb_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rejct_drb_efull_interrupt; 
    
    // tmr_rejct_drb_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rejct_drb_ovf_interrupt; 
    
    // stmr_fifo_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_fifo_efull_interrupt; 
    
    // ftmr_fifo_efull_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_fifo_efull_interrupt; 
    
    // stmr_fifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_fifo_ovf_interrupt; 
    
    // ftmr_fifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_fifo_ovf_interrupt; 
    
    // ftmr_stall_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_stall_interrupt; 
    
    // stmr_stall_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_stall_interrupt; 
    
    // stmr_key_not_found_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_key_not_found_interrupt; 
    
    // stmr_key_not_push_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_key_not_push_interrupt; 
    
    // ftmr_key_not_found_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_key_not_found_interrupt; 
    
    // ftmr_key_not_push_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_key_not_push_interrupt; 
    
    // ftmr_push_out_of_wheel_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_push_out_of_wheel_interrupt; 
    
    // stmr_push_out_of_wheel_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_push_out_of_wheel_interrupt; 
    
    // stmr_ctime_wrap_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_ctime_wrap_interrupt; 
    
    // ftmr_ctime_wrap_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_ctime_wrap_interrupt; 
    
    // tmr_hbm_byp_wtag_wrap_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_wtag_wrap_interrupt; 
    
    // tmr_hbm_byp_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_ovf_interrupt; 
    
    // tmr_state_fifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_state_fifo_ovf_interrupt; 
    
    // tmr_rd_txfifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rd_txfifo_ovf_interrupt; 
    
    // tmr_wr_txfifo_ovf_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_wr_txfifo_ovf_interrupt; 
    
    // tmr_rid_empty_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rid_empty_interrupt; 
    
    // tmr_wid_empty_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_wid_empty_interrupt; 
    
    return ret_val;
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // tmr_rid_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rid_err_enable; 
    
    // tmr_rresp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rresp_err_enable; 
    
    // tmr_bid_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_bid_err_enable; 
    
    // tmr_bresp_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_bresp_err_enable; 
    
    // tmr_hbm_wr_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_wr_pending_efull_enable; 
    
    // tmr_hbm_rd_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_rd_pending_efull_enable; 
    
    // tmr_hbm_byp_pending_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_pending_efull_enable; 
    
    // tmr_hbm_we_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_we_efull_enable; 
    
    // tmr_hbm_re_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_re_efull_enable; 
    
    // tmr_rejct_drb_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rejct_drb_efull_enable; 
    
    // tmr_rejct_drb_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rejct_drb_ovf_enable; 
    
    // stmr_fifo_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_fifo_efull_enable; 
    
    // ftmr_fifo_efull_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_fifo_efull_enable; 
    
    // stmr_fifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_fifo_ovf_enable; 
    
    // ftmr_fifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_fifo_ovf_enable; 
    
    // ftmr_stall_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_stall_enable; 
    
    // stmr_stall_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_stall_enable; 
    
    // stmr_key_not_found_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_key_not_found_enable; 
    
    // stmr_key_not_push_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_key_not_push_enable; 
    
    // ftmr_key_not_found_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_key_not_found_enable; 
    
    // ftmr_key_not_push_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_key_not_push_enable; 
    
    // ftmr_push_out_of_wheel_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_push_out_of_wheel_enable; 
    
    // stmr_push_out_of_wheel_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_push_out_of_wheel_enable; 
    
    // stmr_ctime_wrap_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stmr_ctime_wrap_enable; 
    
    // ftmr_ctime_wrap_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ftmr_ctime_wrap_enable; 
    
    // tmr_hbm_byp_wtag_wrap_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_wtag_wrap_enable; 
    
    // tmr_hbm_byp_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_hbm_byp_ovf_enable; 
    
    // tmr_state_fifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_state_fifo_ovf_enable; 
    
    // tmr_rd_txfifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rd_txfifo_ovf_enable; 
    
    // tmr_wr_txfifo_ovf_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_wr_txfifo_ovf_enable; 
    
    // tmr_rid_empty_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_rid_empty_enable; 
    
    // tmr_wid_empty_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__tmr_wid_empty_enable; 
    
    return ret_val;
}

void cap_txs_csr_int_tmr_int_enable_clear_t::clear() {

    int_var__tmr_wid_empty_enable = 0; 
    
    int_var__tmr_rid_empty_enable = 0; 
    
    int_var__tmr_wr_txfifo_ovf_enable = 0; 
    
    int_var__tmr_rd_txfifo_ovf_enable = 0; 
    
    int_var__tmr_state_fifo_ovf_enable = 0; 
    
    int_var__tmr_hbm_byp_ovf_enable = 0; 
    
    int_var__tmr_hbm_byp_wtag_wrap_enable = 0; 
    
    int_var__ftmr_ctime_wrap_enable = 0; 
    
    int_var__stmr_ctime_wrap_enable = 0; 
    
    int_var__stmr_push_out_of_wheel_enable = 0; 
    
    int_var__ftmr_push_out_of_wheel_enable = 0; 
    
    int_var__ftmr_key_not_push_enable = 0; 
    
    int_var__ftmr_key_not_found_enable = 0; 
    
    int_var__stmr_key_not_push_enable = 0; 
    
    int_var__stmr_key_not_found_enable = 0; 
    
    int_var__stmr_stall_enable = 0; 
    
    int_var__ftmr_stall_enable = 0; 
    
    int_var__ftmr_fifo_ovf_enable = 0; 
    
    int_var__stmr_fifo_ovf_enable = 0; 
    
    int_var__ftmr_fifo_efull_enable = 0; 
    
    int_var__stmr_fifo_efull_enable = 0; 
    
    int_var__tmr_rejct_drb_ovf_enable = 0; 
    
    int_var__tmr_rejct_drb_efull_enable = 0; 
    
    int_var__tmr_hbm_re_efull_enable = 0; 
    
    int_var__tmr_hbm_we_efull_enable = 0; 
    
    int_var__tmr_hbm_byp_pending_efull_enable = 0; 
    
    int_var__tmr_hbm_rd_pending_efull_enable = 0; 
    
    int_var__tmr_hbm_wr_pending_efull_enable = 0; 
    
    int_var__tmr_bresp_err_enable = 0; 
    
    int_var__tmr_bid_err_enable = 0; 
    
    int_var__tmr_rresp_err_enable = 0; 
    
    int_var__tmr_rid_err_enable = 0; 
    
}

void cap_txs_csr_int_tmr_int_test_set_t::clear() {

    int_var__tmr_wid_empty_interrupt = 0; 
    
    int_var__tmr_rid_empty_interrupt = 0; 
    
    int_var__tmr_wr_txfifo_ovf_interrupt = 0; 
    
    int_var__tmr_rd_txfifo_ovf_interrupt = 0; 
    
    int_var__tmr_state_fifo_ovf_interrupt = 0; 
    
    int_var__tmr_hbm_byp_ovf_interrupt = 0; 
    
    int_var__tmr_hbm_byp_wtag_wrap_interrupt = 0; 
    
    int_var__ftmr_ctime_wrap_interrupt = 0; 
    
    int_var__stmr_ctime_wrap_interrupt = 0; 
    
    int_var__stmr_push_out_of_wheel_interrupt = 0; 
    
    int_var__ftmr_push_out_of_wheel_interrupt = 0; 
    
    int_var__ftmr_key_not_push_interrupt = 0; 
    
    int_var__ftmr_key_not_found_interrupt = 0; 
    
    int_var__stmr_key_not_push_interrupt = 0; 
    
    int_var__stmr_key_not_found_interrupt = 0; 
    
    int_var__stmr_stall_interrupt = 0; 
    
    int_var__ftmr_stall_interrupt = 0; 
    
    int_var__ftmr_fifo_ovf_interrupt = 0; 
    
    int_var__stmr_fifo_ovf_interrupt = 0; 
    
    int_var__ftmr_fifo_efull_interrupt = 0; 
    
    int_var__stmr_fifo_efull_interrupt = 0; 
    
    int_var__tmr_rejct_drb_ovf_interrupt = 0; 
    
    int_var__tmr_rejct_drb_efull_interrupt = 0; 
    
    int_var__tmr_hbm_re_efull_interrupt = 0; 
    
    int_var__tmr_hbm_we_efull_interrupt = 0; 
    
    int_var__tmr_hbm_byp_pending_efull_interrupt = 0; 
    
    int_var__tmr_hbm_rd_pending_efull_interrupt = 0; 
    
    int_var__tmr_hbm_wr_pending_efull_interrupt = 0; 
    
    int_var__tmr_bresp_err_interrupt = 0; 
    
    int_var__tmr_bid_err_interrupt = 0; 
    
    int_var__tmr_rresp_err_interrupt = 0; 
    
    int_var__tmr_rid_err_interrupt = 0; 
    
}

void cap_txs_csr_int_tmr_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_txs_csr_int_sch_int_enable_clear_t::clear() {

    int_var__sch_wid_empty_enable = 0; 
    
    int_var__sch_rid_empty_enable = 0; 
    
    int_var__sch_wr_txfifo_ovf_enable = 0; 
    
    int_var__sch_rd_txfifo_ovf_enable = 0; 
    
    int_var__sch_state_fifo_ovf_enable = 0; 
    
    int_var__sch_drb_cnt_qid_fifo_enable = 0; 
    
    int_var__sch_hbm_byp_ovf_enable = 0; 
    
    int_var__sch_hbm_byp_wtag_wrap_enable = 0; 
    
    int_var__sch_rlid_ovfl_enable = 0; 
    
    int_var__sch_rlid_unfl_enable = 0; 
    
    int_var__sch_null_lif_enable = 0; 
    
    int_var__sch_lif_sg_mismatch_enable = 0; 
    
    int_var__sch_aclr_hbm_ln_rollovr_enable = 0; 
    
    int_var__sch_txdma_msg_efull_enable = 0; 
    
    int_var__sch_hbm_re_efull_enable = 0; 
    
    int_var__sch_hbm_we_efull_enable = 0; 
    
    int_var__sch_hbm_byp_pending_efull_enable = 0; 
    
    int_var__sch_hbm_rd_pending_efull_enable = 0; 
    
    int_var__sch_hbm_wr_pending_efull_enable = 0; 
    
    int_var__sch_bresp_err_enable = 0; 
    
    int_var__sch_bid_err_enable = 0; 
    
    int_var__sch_rresp_err_enable = 0; 
    
    int_var__sch_rid_err_enable = 0; 
    
    int_var__sch_drb_cnt_ovfl_enable = 0; 
    
    int_var__sch_drb_cnt_unfl_enable = 0; 
    
    int_var__sch_txdma_msg_ovfl_enable = 0; 
    
}

void cap_txs_csr_int_sch_int_test_set_t::clear() {

    int_var__sch_wid_empty_interrupt = 0; 
    
    int_var__sch_rid_empty_interrupt = 0; 
    
    int_var__sch_wr_txfifo_ovf_interrupt = 0; 
    
    int_var__sch_rd_txfifo_ovf_interrupt = 0; 
    
    int_var__sch_state_fifo_ovf_interrupt = 0; 
    
    int_var__sch_drb_cnt_qid_fifo_interrupt = 0; 
    
    int_var__sch_hbm_byp_ovf_interrupt = 0; 
    
    int_var__sch_hbm_byp_wtag_wrap_interrupt = 0; 
    
    int_var__sch_rlid_ovfl_interrupt = 0; 
    
    int_var__sch_rlid_unfl_interrupt = 0; 
    
    int_var__sch_null_lif_interrupt = 0; 
    
    int_var__sch_lif_sg_mismatch_interrupt = 0; 
    
    int_var__sch_aclr_hbm_ln_rollovr_interrupt = 0; 
    
    int_var__sch_txdma_msg_efull_interrupt = 0; 
    
    int_var__sch_hbm_re_efull_interrupt = 0; 
    
    int_var__sch_hbm_we_efull_interrupt = 0; 
    
    int_var__sch_hbm_byp_pending_efull_interrupt = 0; 
    
    int_var__sch_hbm_rd_pending_efull_interrupt = 0; 
    
    int_var__sch_hbm_wr_pending_efull_interrupt = 0; 
    
    int_var__sch_bresp_err_interrupt = 0; 
    
    int_var__sch_bid_err_interrupt = 0; 
    
    int_var__sch_rresp_err_interrupt = 0; 
    
    int_var__sch_rid_err_interrupt = 0; 
    
    int_var__sch_drb_cnt_ovfl_interrupt = 0; 
    
    int_var__sch_drb_cnt_unfl_interrupt = 0; 
    
    int_var__sch_txdma_msg_ovfl_interrupt = 0; 
    
}

void cap_txs_csr_int_sch_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_txs_csr_intreg_status_t::clear() {

    int_var__int_sch_interrupt = 0; 
    
    int_var__int_tmr_interrupt = 0; 
    
    int_var__int_srams_ecc_interrupt = 0; 
    
}

void cap_txs_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_sch_enable = 0; 
    
    int_var__int_tmr_enable = 0; 
    
    int_var__int_srams_ecc_enable = 0; 
    
}

void cap_txs_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_txs_csr_int_srams_ecc_int_enable_set_t::clear() {

    int_var__tmr_cnt_uncorrectable_enable = 0; 
    
    int_var__tmr_cnt_correctable_enable = 0; 
    
    int_var__sch_lif_map_uncorrectable_enable = 0; 
    
    int_var__sch_lif_map_correctable_enable = 0; 
    
    int_var__sch_rlid_map_uncorrectable_enable = 0; 
    
    int_var__sch_rlid_map_correctable_enable = 0; 
    
    int_var__sch_grp_uncorrectable_enable = 0; 
    
    int_var__sch_grp_correctable_enable = 0; 
    
}

void cap_txs_csr_int_srams_ecc_intreg_t::clear() {

    int_var__tmr_cnt_uncorrectable_interrupt = 0; 
    
    int_var__tmr_cnt_correctable_interrupt = 0; 
    
    int_var__sch_lif_map_uncorrectable_interrupt = 0; 
    
    int_var__sch_lif_map_correctable_interrupt = 0; 
    
    int_var__sch_rlid_map_uncorrectable_interrupt = 0; 
    
    int_var__sch_rlid_map_correctable_interrupt = 0; 
    
    int_var__sch_grp_uncorrectable_interrupt = 0; 
    
    int_var__sch_grp_correctable_interrupt = 0; 
    
}

void cap_txs_csr_int_srams_ecc_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_txs_csr_dhs_sch_grp_sram_entry_t::clear() {

    int_var__spare = 0; 
    
    int_var__clr_pend_cnt = 0; 
    
    int_var__rl_thr = 0; 
    
    int_var__bckgr_cnt = 0; 
    
    int_var__bckgr = 0; 
    
    int_var__hbm_mode = 0; 
    
    int_var__qid_offset = 0; 
    
    int_var__lif = 0; 
    
    int_var__auto_clr = 0; 
    
    int_var__rr_sel = 0; 
    
    int_var__qid_fifo_elm0_vld = 0; 
    
    int_var__qid_fifo_elm0_jnk = 0; 
    
    int_var__qid_fifo_elm0_qid = 0; 
    
    int_var__qid_fifo_elm1_vld = 0; 
    
    int_var__qid_fifo_elm1_jnk = 0; 
    
    int_var__qid_fifo_elm1_qid = 0; 
    
    int_var__qid_fifo_elm2_vld = 0; 
    
    int_var__qid_fifo_elm2_jnk = 0; 
    
    int_var__qid_fifo_elm2_qid = 0; 
    
    int_var__qid_fifo_elm3_vld = 0; 
    
    int_var__qid_fifo_elm3_jnk = 0; 
    
    int_var__qid_fifo_elm3_qid = 0; 
    
    int_var__qid_fifo_elm4_vld = 0; 
    
    int_var__qid_fifo_elm4_jnk = 0; 
    
    int_var__qid_fifo_elm4_qid = 0; 
    
    int_var__qid_fifo_elm5_vld = 0; 
    
    int_var__qid_fifo_elm5_jnk = 0; 
    
    int_var__qid_fifo_elm5_qid = 0; 
    
    int_var__qid_fifo_elm6_vld = 0; 
    
    int_var__qid_fifo_elm6_jnk = 0; 
    
    int_var__qid_fifo_elm6_qid = 0; 
    
    int_var__qid_fifo_elm7_vld = 0; 
    
    int_var__qid_fifo_elm7_jnk = 0; 
    
    int_var__qid_fifo_elm7_qid = 0; 
    
    int_var__qid_fifo_elm8_vld = 0; 
    
    int_var__qid_fifo_elm8_jnk = 0; 
    
    int_var__qid_fifo_elm8_qid = 0; 
    
    int_var__qid_fifo_elm9_vld = 0; 
    
    int_var__qid_fifo_elm9_jnk = 0; 
    
    int_var__qid_fifo_elm9_qid = 0; 
    
    int_var__qid_fifo_elm10_vld = 0; 
    
    int_var__qid_fifo_elm10_jnk = 0; 
    
    int_var__qid_fifo_elm10_qid = 0; 
    
    int_var__qid_fifo_elm11_vld = 0; 
    
    int_var__qid_fifo_elm11_jnk = 0; 
    
    int_var__qid_fifo_elm11_qid = 0; 
    
    int_var__qid_fifo_elm12_vld = 0; 
    
    int_var__qid_fifo_elm12_jnk = 0; 
    
    int_var__qid_fifo_elm12_qid = 0; 
    
    int_var__qid_fifo_elm13_vld = 0; 
    
    int_var__qid_fifo_elm13_jnk = 0; 
    
    int_var__qid_fifo_elm13_qid = 0; 
    
    int_var__qid_fifo_elm14_vld = 0; 
    
    int_var__qid_fifo_elm14_jnk = 0; 
    
    int_var__qid_fifo_elm14_qid = 0; 
    
    int_var__qid_fifo_elm15_vld = 0; 
    
    int_var__qid_fifo_elm15_jnk = 0; 
    
    int_var__qid_fifo_elm15_qid = 0; 
    
    int_var__hbm_ln = 0; 
    
    int_var__hbm_ln_ptr = 0; 
    
    int_var__hbm_rr_cnt = 0; 
    
    int_var__drb_cnt = 0; 
    
    int_var__hbm_ln_cnt0 = 0; 
    
    int_var__hbm_ln_cnt1 = 0; 
    
    int_var__hbm_ln_cnt2 = 0; 
    
    int_var__hbm_ln_cnt3 = 0; 
    
    int_var__hbm_ln_cnt4 = 0; 
    
    int_var__hbm_ln_cnt5 = 0; 
    
    int_var__hbm_ln_cnt6 = 0; 
    
    int_var__hbm_ln_cnt7 = 0; 
    
    int_var__hbm_ln_cnt8 = 0; 
    
    int_var__hbm_ln_cnt9 = 0; 
    
    int_var__hbm_ln_cnt10 = 0; 
    
    int_var__hbm_ln_cnt11 = 0; 
    
    int_var__hbm_ln_cnt12 = 0; 
    
    int_var__hbm_ln_cnt13 = 0; 
    
    int_var__hbm_ln_cnt14 = 0; 
    
    int_var__hbm_ln_cnt15 = 0; 
    
    int_var__popcnt = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_txs_csr_dhs_sch_grp_sram_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::clear() {

    int_var__spare = 0; 
    
    int_var__sg_start = 0; 
    
    int_var__sg_end = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_txs_csr_dhs_sch_rlid_map_sram_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::clear() {

    int_var__spare = 0; 
    
    int_var__sg_active = 0; 
    
    int_var__sg_start = 0; 
    
    int_var__sg_per_cos = 0; 
    
    int_var__sg_act_cos = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_txs_csr_dhs_sch_lif_map_sram_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::clear() {

    int_var__spare = 0; 
    
    int_var__slow_cbcnt = 0; 
    
    int_var__slow_bcnt = 0; 
    
    int_var__slow_lcnt = 0; 
    
    int_var__fast_cbcnt = 0; 
    
    int_var__fast_bcnt = 0; 
    
    int_var__fast_lcnt = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_txs_csr_dhs_tmr_cnt_sram_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_rlid_stop_entry_t::clear() {

    int_var__setmsk = 0; 
    
}

void cap_txs_csr_dhs_rlid_stop_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::clear() {

    int_var__popcnt = 0; 
    
    int_var__drb_cnt = 0; 
    
}

void cap_txs_csr_dhs_sch_grp_cnt_entry_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_entry_entry_t::clear() {

    int_var__lif = 0; 
    
    int_var__qid_offset = 0; 
    
    int_var__auto_clr = 0; 
    
    int_var__rr_sel = 0; 
    
    int_var__rl_thr = 0; 
    
}

void cap_txs_csr_dhs_sch_grp_entry_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_doorbell_entry_t::clear() {

    int_var__qid = 0; 
    
    int_var__cos = 0; 
    
    int_var__set = 0; 
    
}

void cap_txs_csr_dhs_doorbell_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_flags_entry_t::clear() {

    int_var__sch_flags = 0; 
    
}

void cap_txs_csr_dhs_sch_flags_t::clear() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmhi_calendar_entry_t::clear() {

    int_var__dtdm_calendar = 0; 
    
}

void cap_txs_csr_dhs_dtdmhi_calendar_t::clear() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmlo_calendar_entry_t::clear() {

    int_var__dtdm_calendar = 0; 
    
}

void cap_txs_csr_dhs_dtdmlo_calendar_t::clear() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_pending_entry_t::clear() {

    int_var__bcnt = 0; 
    
    int_var__lcnt = 0; 
    
}

void cap_txs_csr_dhs_slow_timer_pending_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::clear() {

    int_var__typ = 0; 
    
    int_var__qid = 0; 
    
    int_var__ring = 0; 
    
    int_var__dtime = 0; 
    
}

void cap_txs_csr_dhs_slow_timer_start_no_stop_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_pending_entry_t::clear() {

    int_var__bcnt = 0; 
    
    int_var__lcnt = 0; 
    
}

void cap_txs_csr_dhs_fast_timer_pending_t::clear() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 4096; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::clear() {

    int_var__typ = 0; 
    
    int_var__qid = 0; 
    
    int_var__ring = 0; 
    
    int_var__dtime = 0; 
    
}

void cap_txs_csr_dhs_fast_timer_start_no_stop_t::clear() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_txs_csr_cnt_tmr_axi_bid_uexp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_bid_rerr_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_bid_err_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_bid_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_wr_req_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_tmr_axi_rd_req_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_bid_uexp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_bid_rerr_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_bid_err_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_bid_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_wr_req_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_rd_rsp_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_axi_rd_req_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_sta_stmr_max_bcnt_t::clear() {

    int_var__bcnt = 0; 
    
}

void cap_txs_csr_sta_ftmr_max_bcnt_t::clear() {

    int_var__bcnt = 0; 
    
}

void cap_txs_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_txs_csr_sta_srams_sch_hbm_byp_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_sch_hbm_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_cfg_sch_hbm_byp_sram_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_sch_hbm_sram_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_txs_csr_sta_srams_ecc_sch_grp_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_scheduler_dbg2_t::clear() {

    int_var__hbm_byp_pending_cnt = 0; 
    
    int_var__hbm_rd_pending_cnt = 0; 
    
    int_var__hbm_wr_pending_cnt = 0; 
    
}

void cap_txs_csr_sta_scheduler_dbg_t::clear() {

    int_var__txdma_drdy = 0; 
    
    int_var__hbm_re_efull = 0; 
    
    int_var__hbm_we_efull = 0; 
    
    int_var__txdma_msg_efull = 0; 
    
    int_var__hbm_byp_pending_efull = 0; 
    
    int_var__hbm_rd_pending_efull = 0; 
    
    int_var__hbm_wr_pending_efull = 0; 
    
}

void cap_txs_csr_sta_sch_lif_sg_mismatch_t::clear() {

    int_var__lif = 0; 
    
    int_var__qid = 0; 
    
    int_var__set = 0; 
    
}

void cap_txs_csr_sta_sch_lif_map_notactive_t::clear() {

    int_var__lif = 0; 
    
    int_var__qid = 0; 
    
    int_var__cos = 0; 
    
    int_var__set = 0; 
    
}

void cap_txs_csr_cfg_sch_grp_sram_t::clear() {

    int_var__ecc_disable_cor = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__dhs_eccbypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_sch_rlid_map_sram_t::clear() {

    int_var__ecc_disable_cor = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__dhs_eccbypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_sch_lif_map_sram_t::clear() {

    int_var__ecc_disable_cor = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__dhs_eccbypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_tmr_cnt_sram_t::clear() {

    int_var__ecc_disable_cor = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__dhs_eccbypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_txs_csr_sta_sch_max_hbm_byp_t::clear() {

    int_var__cnt = 0; 
    
}

void cap_txs_csr_cfg_scheduler_dbg2_t::clear() {

    int_var__qid_read = 0; 
    
    int_var__cos_read = 0; 
    
}

void cap_txs_csr_cfg_scheduler_dbg_t::clear() {

    int_var__hbm_efc_thr = 0; 
    
    int_var__txdma_efc_thr = 0; 
    
    int_var__fifo_mode_thr = 0; 
    
    int_var__max_hbm_byp = 0; 
    
    int_var__max_hbm_wr = 0; 
    
    int_var__max_hbm_rd = 0; 
    
    int_var__spare = 0; 
    
}

void cap_txs_csr_sta_scheduler_rr_t::clear() {

    int_var__curr_ptr0 = 0; 
    
    int_var__curr_ptr1 = 0; 
    
    int_var__curr_ptr2 = 0; 
    
    int_var__curr_ptr3 = 0; 
    
    int_var__curr_ptr4 = 0; 
    
    int_var__curr_ptr5 = 0; 
    
    int_var__curr_ptr6 = 0; 
    
    int_var__curr_ptr7 = 0; 
    
    int_var__curr_ptr8 = 0; 
    
    int_var__curr_ptr9 = 0; 
    
    int_var__curr_ptr10 = 0; 
    
    int_var__curr_ptr11 = 0; 
    
    int_var__curr_ptr12 = 0; 
    
    int_var__curr_ptr13 = 0; 
    
    int_var__curr_ptr14 = 0; 
    
    int_var__curr_ptr15 = 0; 
    
}

void cap_txs_csr_sta_scheduler_t::clear() {

    int_var__hbm_init_done = 0; 
    
    int_var__sram_init_done = 0; 
    
}

void cap_txs_csr_cnt_sch_rlid_start_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_rlid_stop_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_sta_glb_t::clear() {

    int_var__pb_xoff = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos15_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos14_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos13_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos12_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos11_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos10_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos9_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos8_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos7_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos6_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos5_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos4_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos3_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos2_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos1_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_txdma_cos0_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_fifo_empty_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_doorbell_clr_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_sch_doorbell_set_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cfg_sch_t::clear() {

    int_var__dtdm_lo_map = 0; 
    
    int_var__dtdm_hi_map = 0; 
    
    int_var__timeout = 0; 
    
    int_var__pause = 0; 
    
    int_var__enable = 0; 
    
}

void cap_txs_csr_cfw_scheduler_static_t::clear() {

    int_var__hbm_base = 0; 
    
    int_var__sch_grp_depth = 0; 
    
}

void cap_txs_csr_cfw_scheduler_glb_t::clear() {

    int_var__hbm_hw_init = 0; 
    
    int_var__sram_hw_init = 0; 
    
    int_var__spare = 0; 
    
    int_var__enable = 0; 
    
    int_var__enable_set_lkup = 0; 
    
    int_var__enable_set_byp = 0; 
    
}

void cap_txs_csr_sta_srams_tmr_fifo_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_tmr_hbm_byp_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_sta_srams_tmr_hbm_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_txs_csr_cfg_tmr_fifo_sram_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_tmr_hbm_byp_sram_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cfg_tmr_hbm_sram_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_txs_csr_cnt_stmr_pop_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_stmr_key_not_found_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_stmr_push_out_of_wheel_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_stmr_key_not_push_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_stmr_push_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_sta_slow_timer_t::clear() {

    int_var__tick = 0; 
    
    int_var__cTime = 0; 
    
    int_var__pTime = 0; 
    
}

void cap_txs_csr_cfg_slow_timer_dbell_t::clear() {

    int_var__addr_update = 0; 
    
    int_var__data_pid = 0; 
    
    int_var__data_reserved = 0; 
    
    int_var__data_index = 0; 
    
}

void cap_txs_csr_cfg_force_slow_timer_t::clear() {

    int_var__enable = 0; 
    
    int_var__ctime = 0; 
    
}

void cap_txs_csr_cfg_slow_timer_t::clear() {

    int_var__tick = 0; 
    
    int_var__hash_sel = 0; 
    
}

void cap_txs_csr_cnt_ftmr_pop_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_ftmr_key_not_found_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_ftmr_key_not_push_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_cnt_ftmr_push_t::clear() {

    int_var__val = 0; 
    
}

void cap_txs_csr_sta_fast_timer_t::clear() {

    int_var__tick = 0; 
    
    int_var__cTime = 0; 
    
    int_var__pTime = 0; 
    
}

void cap_txs_csr_cfg_fast_timer_dbell_t::clear() {

    int_var__addr_update = 0; 
    
    int_var__data_pid = 0; 
    
    int_var__data_reserved = 0; 
    
    int_var__data_index = 0; 
    
}

void cap_txs_csr_cfg_force_fast_timer_t::clear() {

    int_var__enable = 0; 
    
    int_var__ctime = 0; 
    
}

void cap_txs_csr_cfg_fast_timer_t::clear() {

    int_var__tick = 0; 
    
    int_var__hash_sel = 0; 
    
}

void cap_txs_csr_sta_timer_dbg2_t::clear() {

    int_var__hbm_byp_pending_cnt = 0; 
    
    int_var__hbm_rd_pending_cnt = 0; 
    
    int_var__hbm_wr_pending_cnt = 0; 
    
    int_var__stmr_stall = 0; 
    
    int_var__ftmr_stall = 0; 
    
}

void cap_txs_csr_sta_timer_dbg_t::clear() {

    int_var__hbm_re_efull = 0; 
    
    int_var__hbm_we_efull = 0; 
    
    int_var__rejct_drb_efull = 0; 
    
    int_var__hbm_byp_pending_efull = 0; 
    
    int_var__hbm_rd_pending_efull = 0; 
    
    int_var__hbm_wr_pending_efull = 0; 
    
    int_var__stmr_fifo_raddr = 0; 
    
    int_var__ftmr_fifo_raddr = 0; 
    
    int_var__stmr_fifo_waddr = 0; 
    
    int_var__ftmr_fifo_waddr = 0; 
    
    int_var__stmr_fifo_efull = 0; 
    
    int_var__ftmr_fifo_efull = 0; 
    
}

void cap_txs_csr_sta_tmr_max_keys_t::clear() {

    int_var__cnt = 0; 
    
}

void cap_txs_csr_sta_tmr_max_hbm_byp_t::clear() {

    int_var__cnt = 0; 
    
}

void cap_txs_csr_sta_timer_t::clear() {

    int_var__hbm_init_done = 0; 
    
    int_var__sram_init_done = 0; 
    
}

void cap_txs_csr_cfg_timer_dbg2_t::clear() {

    int_var__max_bcnt = 0; 
    
    int_var__spare = 0; 
    
}

void cap_txs_csr_cfg_timer_dbg_t::clear() {

    int_var__hbm_efc_thr = 0; 
    
    int_var__drb_efc_thr = 0; 
    
    int_var__tmr_stall_thr_hi = 0; 
    
    int_var__tmr_stall_thr_lo = 0; 
    
    int_var__max_hbm_wr = 0; 
    
    int_var__max_hbm_rd = 0; 
    
    int_var__max_hbm_byp = 0; 
    
    int_var__max_tmr_fifo = 0; 
    
    int_var__spare = 0; 
    
}

void cap_txs_csr_cfw_timer_glb_t::clear() {

    int_var__hbm_hw_init = 0; 
    
    int_var__sram_hw_init = 0; 
    
    int_var__spare = 0; 
    
    int_var__ftmr_enable = 0; 
    
    int_var__stmr_enable = 0; 
    
    int_var__ftmr_pause = 0; 
    
    int_var__stmr_pause = 0; 
    
}

void cap_txs_csr_cfg_timer_static_t::clear() {

    int_var__hbm_base = 0; 
    
    int_var__tmr_hsh_depth = 0; 
    
    int_var__tmr_wheel_depth = 0; 
    
}

void cap_txs_csr_cfg_axi_attr_t::clear() {

    int_var__arcache = 0; 
    
    int_var__awcache = 0; 
    
    int_var__prot = 0; 
    
    int_var__qos = 0; 
    
    int_var__lock = 0; 
    
}

void cap_txs_csr_cfg_glb_t::clear() {

    int_var__dbg_port_select = 0; 
    
    int_var__dbg_port_enable = 0; 
    
    int_var__spare = 0; 
    
}

void cap_txs_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_txs_csr_t::clear() {

    base.clear();
    cfg_glb.clear();
    cfg_axi_attr.clear();
    cfg_timer_static.clear();
    cfw_timer_glb.clear();
    cfg_timer_dbg.clear();
    cfg_timer_dbg2.clear();
    sta_timer.clear();
    sta_tmr_max_hbm_byp.clear();
    sta_tmr_max_keys.clear();
    sta_timer_dbg.clear();
    sta_timer_dbg2.clear();
    cfg_fast_timer.clear();
    cfg_force_fast_timer.clear();
    cfg_fast_timer_dbell.clear();
    sta_fast_timer.clear();
    cnt_ftmr_push.clear();
    cnt_ftmr_key_not_push.clear();
    cnt_ftmr_push_out_of_wheel.clear();
    cnt_ftmr_key_not_found.clear();
    cnt_ftmr_pop.clear();
    cfg_slow_timer.clear();
    cfg_force_slow_timer.clear();
    cfg_slow_timer_dbell.clear();
    sta_slow_timer.clear();
    cnt_stmr_push.clear();
    cnt_stmr_key_not_push.clear();
    cnt_stmr_push_out_of_wheel.clear();
    cnt_stmr_key_not_found.clear();
    cnt_stmr_pop.clear();
    cfg_tmr_hbm_sram.clear();
    cfg_tmr_hbm_byp_sram.clear();
    cfg_tmr_fifo_sram.clear();
    sta_srams_tmr_hbm.clear();
    sta_srams_tmr_hbm_byp.clear();
    sta_srams_tmr_fifo.clear();
    cfw_scheduler_glb.clear();
    cfw_scheduler_static.clear();
    cfg_sch.clear();
    cnt_sch_doorbell_set.clear();
    cnt_sch_doorbell_clr.clear();
    cnt_sch_fifo_empty.clear();
    cnt_sch_txdma_cos0.clear();
    cnt_sch_txdma_cos1.clear();
    cnt_sch_txdma_cos2.clear();
    cnt_sch_txdma_cos3.clear();
    cnt_sch_txdma_cos4.clear();
    cnt_sch_txdma_cos5.clear();
    cnt_sch_txdma_cos6.clear();
    cnt_sch_txdma_cos7.clear();
    cnt_sch_txdma_cos8.clear();
    cnt_sch_txdma_cos9.clear();
    cnt_sch_txdma_cos10.clear();
    cnt_sch_txdma_cos11.clear();
    cnt_sch_txdma_cos12.clear();
    cnt_sch_txdma_cos13.clear();
    cnt_sch_txdma_cos14.clear();
    cnt_sch_txdma_cos15.clear();
    sta_glb.clear();
    cnt_sch_rlid_stop.clear();
    cnt_sch_rlid_start.clear();
    sta_scheduler.clear();
    sta_scheduler_rr.clear();
    cfg_scheduler_dbg.clear();
    cfg_scheduler_dbg2.clear();
    sta_sch_max_hbm_byp.clear();
    cfg_tmr_cnt_sram.clear();
    cfg_sch_lif_map_sram.clear();
    cfg_sch_rlid_map_sram.clear();
    cfg_sch_grp_sram.clear();
    sta_sch_lif_map_notactive.clear();
    sta_sch_lif_sg_mismatch.clear();
    sta_scheduler_dbg.clear();
    sta_scheduler_dbg2.clear();
    sta_srams_ecc_tmr_cnt.clear();
    sta_srams_ecc_sch_lif_map.clear();
    sta_srams_ecc_sch_rlid_map.clear();
    sta_srams_ecc_sch_grp.clear();
    cfg_sch_hbm_sram.clear();
    cfg_sch_hbm_byp_sram.clear();
    sta_srams_sch_hbm.clear();
    sta_srams_sch_hbm_byp.clear();
    csr_intr.clear();
    sta_ftmr_max_bcnt.clear();
    sta_stmr_max_bcnt.clear();
    cnt_sch_axi_rd_req.clear();
    cnt_sch_axi_rd_rsp.clear();
    cnt_sch_axi_rd_rsp_err.clear();
    cnt_sch_axi_rd_rsp_rerr.clear();
    cnt_sch_axi_rd_rsp_uexp.clear();
    cnt_sch_axi_wr_req.clear();
    cnt_sch_axi_bid.clear();
    cnt_sch_axi_bid_err.clear();
    cnt_sch_axi_bid_rerr.clear();
    cnt_sch_axi_bid_uexp.clear();
    cnt_tmr_axi_rd_req.clear();
    cnt_tmr_axi_rd_rsp.clear();
    cnt_tmr_axi_rd_rsp_err.clear();
    cnt_tmr_axi_rd_rsp_rerr.clear();
    cnt_tmr_axi_rd_rsp_uexp.clear();
    cnt_tmr_axi_wr_req.clear();
    cnt_tmr_axi_bid.clear();
    cnt_tmr_axi_bid_err.clear();
    cnt_tmr_axi_bid_rerr.clear();
    cnt_tmr_axi_bid_uexp.clear();
    dhs_fast_timer_start_no_stop.clear();
    dhs_fast_timer_pending.clear();
    dhs_slow_timer_start_no_stop.clear();
    dhs_slow_timer_pending.clear();
    dhs_dtdmlo_calendar.clear();
    dhs_dtdmhi_calendar.clear();
    dhs_sch_flags.clear();
    dhs_doorbell.clear();
    dhs_sch_grp_entry.clear();
    dhs_sch_grp_cnt_entry.clear();
    dhs_rlid_stop.clear();
    dhs_tmr_cnt_sram.clear();
    dhs_sch_lif_map_sram.clear();
    dhs_sch_rlid_map_sram.clear();
    dhs_sch_grp_sram.clear();
    int_srams_ecc.clear();
    int_groups.clear();
    int_sch.clear();
    int_tmr.clear();
}

void cap_txs_csr_int_tmr_int_enable_clear_t::init() {

}

void cap_txs_csr_int_tmr_int_test_set_t::init() {

}

void cap_txs_csr_int_tmr_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_txs_csr_int_sch_int_enable_clear_t::init() {

}

void cap_txs_csr_int_sch_int_test_set_t::init() {

}

void cap_txs_csr_int_sch_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_txs_csr_intreg_status_t::init() {

}

void cap_txs_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_txs_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_txs_csr_int_srams_ecc_int_enable_set_t::init() {

}

void cap_txs_csr_int_srams_ecc_intreg_t::init() {

}

void cap_txs_csr_int_srams_ecc_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_txs_csr_dhs_sch_grp_sram_entry_t::init() {

}

void cap_txs_csr_dhs_sch_grp_sram_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::init() {

}

void cap_txs_csr_dhs_sch_rlid_map_sram_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::init() {

}

void cap_txs_csr_dhs_sch_lif_map_sram_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::init() {

}

void cap_txs_csr_dhs_tmr_cnt_sram_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_rlid_stop_entry_t::init() {

}

void cap_txs_csr_dhs_rlid_stop_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::init() {

}

void cap_txs_csr_dhs_sch_grp_cnt_entry_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_grp_entry_entry_t::init() {

}

void cap_txs_csr_dhs_sch_grp_entry_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_doorbell_entry_t::init() {

}

void cap_txs_csr_dhs_doorbell_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_sch_flags_entry_t::init() {

}

void cap_txs_csr_dhs_sch_flags_t::init() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmhi_calendar_entry_t::init() {

}

void cap_txs_csr_dhs_dtdmhi_calendar_t::init() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_dtdmlo_calendar_entry_t::init() {

}

void cap_txs_csr_dhs_dtdmlo_calendar_t::init() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_pending_entry_t::init() {

}

void cap_txs_csr_dhs_slow_timer_pending_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::init() {

}

void cap_txs_csr_dhs_slow_timer_start_no_stop_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_pending_entry_t::init() {

}

void cap_txs_csr_dhs_fast_timer_pending_t::init() {

    #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 4096; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::init() {

}

void cap_txs_csr_dhs_fast_timer_start_no_stop_t::init() {

    #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_txs_csr_cnt_tmr_axi_bid_uexp_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_bid_rerr_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_bid_err_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_bid_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_wr_req_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_rd_rsp_t::init() {

}

void cap_txs_csr_cnt_tmr_axi_rd_req_t::init() {

}

void cap_txs_csr_cnt_sch_axi_bid_uexp_t::init() {

}

void cap_txs_csr_cnt_sch_axi_bid_rerr_t::init() {

}

void cap_txs_csr_cnt_sch_axi_bid_err_t::init() {

}

void cap_txs_csr_cnt_sch_axi_bid_t::init() {

}

void cap_txs_csr_cnt_sch_axi_wr_req_t::init() {

}

void cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::init() {

}

void cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::init() {

}

void cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::init() {

}

void cap_txs_csr_cnt_sch_axi_rd_rsp_t::init() {

}

void cap_txs_csr_cnt_sch_axi_rd_req_t::init() {

}

void cap_txs_csr_sta_stmr_max_bcnt_t::init() {

}

void cap_txs_csr_sta_ftmr_max_bcnt_t::init() {

}

void cap_txs_csr_csr_intr_t::init() {

}

void cap_txs_csr_sta_srams_sch_hbm_byp_t::init() {

}

void cap_txs_csr_sta_srams_sch_hbm_t::init() {

}

void cap_txs_csr_cfg_sch_hbm_byp_sram_t::init() {

}

void cap_txs_csr_cfg_sch_hbm_sram_t::init() {

}

void cap_txs_csr_sta_srams_ecc_sch_grp_t::init() {

}

void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::init() {

}

void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::init() {

}

void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::init() {

}

void cap_txs_csr_sta_scheduler_dbg2_t::init() {

}

void cap_txs_csr_sta_scheduler_dbg_t::init() {

}

void cap_txs_csr_sta_sch_lif_sg_mismatch_t::init() {

}

void cap_txs_csr_sta_sch_lif_map_notactive_t::init() {

}

void cap_txs_csr_cfg_sch_grp_sram_t::init() {

}

void cap_txs_csr_cfg_sch_rlid_map_sram_t::init() {

}

void cap_txs_csr_cfg_sch_lif_map_sram_t::init() {

}

void cap_txs_csr_cfg_tmr_cnt_sram_t::init() {

}

void cap_txs_csr_sta_sch_max_hbm_byp_t::init() {

}

void cap_txs_csr_cfg_scheduler_dbg2_t::init() {

}

void cap_txs_csr_cfg_scheduler_dbg_t::init() {

    set_reset_val(cpp_int("0xf7fa1a"));
    all(get_reset_val());
}

void cap_txs_csr_sta_scheduler_rr_t::init() {

}

void cap_txs_csr_sta_scheduler_t::init() {

}

void cap_txs_csr_cnt_sch_rlid_start_t::init() {

}

void cap_txs_csr_cnt_sch_rlid_stop_t::init() {

}

void cap_txs_csr_sta_glb_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos15_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos14_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos13_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos12_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos11_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos10_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos9_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos8_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos7_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos6_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos5_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos4_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos3_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos2_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos1_t::init() {

}

void cap_txs_csr_cnt_sch_txdma_cos0_t::init() {

}

void cap_txs_csr_cnt_sch_fifo_empty_t::init() {

}

void cap_txs_csr_cnt_sch_doorbell_clr_t::init() {

}

void cap_txs_csr_cnt_sch_doorbell_set_t::init() {

}

void cap_txs_csr_cfg_sch_t::init() {

}

void cap_txs_csr_cfw_scheduler_static_t::init() {

    set_reset_val(cpp_int("0x8000000000000000000"));
    all(get_reset_val());
}

void cap_txs_csr_cfw_scheduler_glb_t::init() {

    set_reset_val(cpp_int("0x40"));
    all(get_reset_val());
}

void cap_txs_csr_sta_srams_tmr_fifo_t::init() {

}

void cap_txs_csr_sta_srams_tmr_hbm_byp_t::init() {

}

void cap_txs_csr_sta_srams_tmr_hbm_t::init() {

}

void cap_txs_csr_cfg_tmr_fifo_sram_t::init() {

}

void cap_txs_csr_cfg_tmr_hbm_byp_sram_t::init() {

}

void cap_txs_csr_cfg_tmr_hbm_sram_t::init() {

}

void cap_txs_csr_cnt_stmr_pop_t::init() {

}

void cap_txs_csr_cnt_stmr_key_not_found_t::init() {

}

void cap_txs_csr_cnt_stmr_push_out_of_wheel_t::init() {

}

void cap_txs_csr_cnt_stmr_key_not_push_t::init() {

}

void cap_txs_csr_cnt_stmr_push_t::init() {

}

void cap_txs_csr_sta_slow_timer_t::init() {

}

void cap_txs_csr_cfg_slow_timer_dbell_t::init() {

}

void cap_txs_csr_cfg_force_slow_timer_t::init() {

}

void cap_txs_csr_cfg_slow_timer_t::init() {

    set_reset_val(cpp_int("0x400"));
    all(get_reset_val());
}

void cap_txs_csr_cnt_ftmr_pop_t::init() {

}

void cap_txs_csr_cnt_ftmr_key_not_found_t::init() {

}

void cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::init() {

}

void cap_txs_csr_cnt_ftmr_key_not_push_t::init() {

}

void cap_txs_csr_cnt_ftmr_push_t::init() {

}

void cap_txs_csr_sta_fast_timer_t::init() {

}

void cap_txs_csr_cfg_fast_timer_dbell_t::init() {

}

void cap_txs_csr_cfg_force_fast_timer_t::init() {

}

void cap_txs_csr_cfg_fast_timer_t::init() {

    set_reset_val(cpp_int("0x400"));
    all(get_reset_val());
}

void cap_txs_csr_sta_timer_dbg2_t::init() {

}

void cap_txs_csr_sta_timer_dbg_t::init() {

}

void cap_txs_csr_sta_tmr_max_keys_t::init() {

}

void cap_txs_csr_sta_tmr_max_hbm_byp_t::init() {

}

void cap_txs_csr_sta_timer_t::init() {

}

void cap_txs_csr_cfg_timer_dbg2_t::init() {

    set_reset_val(cpp_int("0x3ffff"));
    all(get_reset_val());
}

void cap_txs_csr_cfg_timer_dbg_t::init() {

    set_reset_val(cpp_int("0x9aee0c9f41a"));
    all(get_reset_val());
}

void cap_txs_csr_cfw_timer_glb_t::init() {

}

void cap_txs_csr_cfg_timer_static_t::init() {

    set_reset_val(cpp_int("0xfff0fffff0000000000000000"));
    all(get_reset_val());
}

void cap_txs_csr_cfg_axi_attr_t::init() {

    set_reset_val(cpp_int("0x2ff"));
    all(get_reset_val());
}

void cap_txs_csr_cfg_glb_t::init() {

}

void cap_txs_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_txs_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_glb.set_attributes(this,"cfg_glb", 0x4 );
    cfg_axi_attr.set_attributes(this,"cfg_axi_attr", 0x8 );
    cfg_timer_static.set_attributes(this,"cfg_timer_static", 0x10 );
    cfw_timer_glb.set_attributes(this,"cfw_timer_glb", 0x20 );
    cfg_timer_dbg.set_attributes(this,"cfg_timer_dbg", 0x28 );
    cfg_timer_dbg2.set_attributes(this,"cfg_timer_dbg2", 0x30 );
    sta_timer.set_attributes(this,"sta_timer", 0x38 );
    sta_tmr_max_hbm_byp.set_attributes(this,"sta_tmr_max_hbm_byp", 0x3c );
    sta_tmr_max_keys.set_attributes(this,"sta_tmr_max_keys", 0x40 );
    sta_timer_dbg.set_attributes(this,"sta_timer_dbg", 0x44 );
    sta_timer_dbg2.set_attributes(this,"sta_timer_dbg2", 0x48 );
    cfg_fast_timer.set_attributes(this,"cfg_fast_timer", 0x50 );
    cfg_force_fast_timer.set_attributes(this,"cfg_force_fast_timer", 0x58 );
    cfg_fast_timer_dbell.set_attributes(this,"cfg_fast_timer_dbell", 0x60 );
    sta_fast_timer.set_attributes(this,"sta_fast_timer", 0xc000 );
    cnt_ftmr_push.set_attributes(this,"cnt_ftmr_push", 0xc008 );
    cnt_ftmr_key_not_push.set_attributes(this,"cnt_ftmr_key_not_push", 0xc010 );
    cnt_ftmr_push_out_of_wheel.set_attributes(this,"cnt_ftmr_push_out_of_wheel", 0xc014 );
    cnt_ftmr_key_not_found.set_attributes(this,"cnt_ftmr_key_not_found", 0xc018 );
    cnt_ftmr_pop.set_attributes(this,"cnt_ftmr_pop", 0xc01c );
    cfg_slow_timer.set_attributes(this,"cfg_slow_timer", 0xc020 );
    cfg_force_slow_timer.set_attributes(this,"cfg_force_slow_timer", 0xc028 );
    cfg_slow_timer_dbell.set_attributes(this,"cfg_slow_timer_dbell", 0xc030 );
    sta_slow_timer.set_attributes(this,"sta_slow_timer", 0x18000 );
    cnt_stmr_push.set_attributes(this,"cnt_stmr_push", 0x18008 );
    cnt_stmr_key_not_push.set_attributes(this,"cnt_stmr_key_not_push", 0x18010 );
    cnt_stmr_push_out_of_wheel.set_attributes(this,"cnt_stmr_push_out_of_wheel", 0x18014 );
    cnt_stmr_key_not_found.set_attributes(this,"cnt_stmr_key_not_found", 0x18018 );
    cnt_stmr_pop.set_attributes(this,"cnt_stmr_pop", 0x1801c );
    cfg_tmr_hbm_sram.set_attributes(this,"cfg_tmr_hbm_sram", 0x18020 );
    cfg_tmr_hbm_byp_sram.set_attributes(this,"cfg_tmr_hbm_byp_sram", 0x18024 );
    cfg_tmr_fifo_sram.set_attributes(this,"cfg_tmr_fifo_sram", 0x18028 );
    sta_srams_tmr_hbm.set_attributes(this,"sta_srams_tmr_hbm", 0x1802c );
    sta_srams_tmr_hbm_byp.set_attributes(this,"sta_srams_tmr_hbm_byp", 0x18030 );
    sta_srams_tmr_fifo.set_attributes(this,"sta_srams_tmr_fifo", 0x18034 );
    cfw_scheduler_glb.set_attributes(this,"cfw_scheduler_glb", 0x18038 );
    cfw_scheduler_static.set_attributes(this,"cfw_scheduler_static", 0x18040 );
    cfg_sch.set_attributes(this,"cfg_sch", 0x18300 );
    cnt_sch_doorbell_set.set_attributes(this,"cnt_sch_doorbell_set", 0x18310 );
    cnt_sch_doorbell_clr.set_attributes(this,"cnt_sch_doorbell_clr", 0x18318 );
    cnt_sch_fifo_empty.set_attributes(this,"cnt_sch_fifo_empty", 0x18320 );
    cnt_sch_txdma_cos0.set_attributes(this,"cnt_sch_txdma_cos0", 0x18500 );
    cnt_sch_txdma_cos1.set_attributes(this,"cnt_sch_txdma_cos1", 0x18508 );
    cnt_sch_txdma_cos2.set_attributes(this,"cnt_sch_txdma_cos2", 0x18510 );
    cnt_sch_txdma_cos3.set_attributes(this,"cnt_sch_txdma_cos3", 0x18518 );
    cnt_sch_txdma_cos4.set_attributes(this,"cnt_sch_txdma_cos4", 0x18520 );
    cnt_sch_txdma_cos5.set_attributes(this,"cnt_sch_txdma_cos5", 0x18528 );
    cnt_sch_txdma_cos6.set_attributes(this,"cnt_sch_txdma_cos6", 0x18530 );
    cnt_sch_txdma_cos7.set_attributes(this,"cnt_sch_txdma_cos7", 0x18538 );
    cnt_sch_txdma_cos8.set_attributes(this,"cnt_sch_txdma_cos8", 0x18540 );
    cnt_sch_txdma_cos9.set_attributes(this,"cnt_sch_txdma_cos9", 0x18548 );
    cnt_sch_txdma_cos10.set_attributes(this,"cnt_sch_txdma_cos10", 0x18550 );
    cnt_sch_txdma_cos11.set_attributes(this,"cnt_sch_txdma_cos11", 0x18558 );
    cnt_sch_txdma_cos12.set_attributes(this,"cnt_sch_txdma_cos12", 0x18560 );
    cnt_sch_txdma_cos13.set_attributes(this,"cnt_sch_txdma_cos13", 0x18568 );
    cnt_sch_txdma_cos14.set_attributes(this,"cnt_sch_txdma_cos14", 0x18570 );
    cnt_sch_txdma_cos15.set_attributes(this,"cnt_sch_txdma_cos15", 0x18578 );
    sta_glb.set_attributes(this,"sta_glb", 0x18580 );
    cnt_sch_rlid_stop.set_attributes(this,"cnt_sch_rlid_stop", 0x22000 );
    cnt_sch_rlid_start.set_attributes(this,"cnt_sch_rlid_start", 0x22004 );
    sta_scheduler.set_attributes(this,"sta_scheduler", 0x22008 );
    sta_scheduler_rr.set_attributes(this,"sta_scheduler_rr", 0x22020 );
    cfg_scheduler_dbg.set_attributes(this,"cfg_scheduler_dbg", 0x22040 );
    cfg_scheduler_dbg2.set_attributes(this,"cfg_scheduler_dbg2", 0x22044 );
    sta_sch_max_hbm_byp.set_attributes(this,"sta_sch_max_hbm_byp", 0x22048 );
    cfg_tmr_cnt_sram.set_attributes(this,"cfg_tmr_cnt_sram", 0x2204c );
    cfg_sch_lif_map_sram.set_attributes(this,"cfg_sch_lif_map_sram", 0x22050 );
    cfg_sch_rlid_map_sram.set_attributes(this,"cfg_sch_rlid_map_sram", 0x22054 );
    cfg_sch_grp_sram.set_attributes(this,"cfg_sch_grp_sram", 0x22058 );
    sta_sch_lif_map_notactive.set_attributes(this,"sta_sch_lif_map_notactive", 0x44000 );
    sta_sch_lif_sg_mismatch.set_attributes(this,"sta_sch_lif_sg_mismatch", 0x44008 );
    sta_scheduler_dbg.set_attributes(this,"sta_scheduler_dbg", 0xc0000 );
    sta_scheduler_dbg2.set_attributes(this,"sta_scheduler_dbg2", 0xc0004 );
    sta_srams_ecc_tmr_cnt.set_attributes(this,"sta_srams_ecc_tmr_cnt", 0xc0008 );
    sta_srams_ecc_sch_lif_map.set_attributes(this,"sta_srams_ecc_sch_lif_map", 0xc000c );
    sta_srams_ecc_sch_rlid_map.set_attributes(this,"sta_srams_ecc_sch_rlid_map", 0xc0010 );
    sta_srams_ecc_sch_grp.set_attributes(this,"sta_srams_ecc_sch_grp", 0xc0018 );
    cfg_sch_hbm_sram.set_attributes(this,"cfg_sch_hbm_sram", 0xc0020 );
    cfg_sch_hbm_byp_sram.set_attributes(this,"cfg_sch_hbm_byp_sram", 0xc0024 );
    sta_srams_sch_hbm.set_attributes(this,"sta_srams_sch_hbm", 0xc0028 );
    sta_srams_sch_hbm_byp.set_attributes(this,"sta_srams_sch_hbm_byp", 0xc002c );
    csr_intr.set_attributes(this,"csr_intr", 0xc0040 );
    sta_ftmr_max_bcnt.set_attributes(this,"sta_ftmr_max_bcnt", 0xc0080 );
    sta_stmr_max_bcnt.set_attributes(this,"sta_stmr_max_bcnt", 0xc0084 );
    cnt_sch_axi_rd_req.set_attributes(this,"cnt_sch_axi_rd_req", 0xc0088 );
    cnt_sch_axi_rd_rsp.set_attributes(this,"cnt_sch_axi_rd_rsp", 0xc0090 );
    cnt_sch_axi_rd_rsp_err.set_attributes(this,"cnt_sch_axi_rd_rsp_err", 0xc0098 );
    cnt_sch_axi_rd_rsp_rerr.set_attributes(this,"cnt_sch_axi_rd_rsp_rerr", 0xc009c );
    cnt_sch_axi_rd_rsp_uexp.set_attributes(this,"cnt_sch_axi_rd_rsp_uexp", 0xc00a0 );
    cnt_sch_axi_wr_req.set_attributes(this,"cnt_sch_axi_wr_req", 0xc00a8 );
    cnt_sch_axi_bid.set_attributes(this,"cnt_sch_axi_bid", 0xc00b0 );
    cnt_sch_axi_bid_err.set_attributes(this,"cnt_sch_axi_bid_err", 0xc00b8 );
    cnt_sch_axi_bid_rerr.set_attributes(this,"cnt_sch_axi_bid_rerr", 0xc00c0 );
    cnt_sch_axi_bid_uexp.set_attributes(this,"cnt_sch_axi_bid_uexp", 0xc00c4 );
    cnt_tmr_axi_rd_req.set_attributes(this,"cnt_tmr_axi_rd_req", 0xc00c8 );
    cnt_tmr_axi_rd_rsp.set_attributes(this,"cnt_tmr_axi_rd_rsp", 0xc00d0 );
    cnt_tmr_axi_rd_rsp_err.set_attributes(this,"cnt_tmr_axi_rd_rsp_err", 0xc00d8 );
    cnt_tmr_axi_rd_rsp_rerr.set_attributes(this,"cnt_tmr_axi_rd_rsp_rerr", 0xc00dc );
    cnt_tmr_axi_rd_rsp_uexp.set_attributes(this,"cnt_tmr_axi_rd_rsp_uexp", 0xc00e0 );
    cnt_tmr_axi_wr_req.set_attributes(this,"cnt_tmr_axi_wr_req", 0xc00e8 );
    cnt_tmr_axi_bid.set_attributes(this,"cnt_tmr_axi_bid", 0xc00f0 );
    cnt_tmr_axi_bid_err.set_attributes(this,"cnt_tmr_axi_bid_err", 0xc00f8 );
    cnt_tmr_axi_bid_rerr.set_attributes(this,"cnt_tmr_axi_bid_rerr", 0xc00fc );
    cnt_tmr_axi_bid_uexp.set_attributes(this,"cnt_tmr_axi_bid_uexp", 0xc0100 );
    dhs_fast_timer_start_no_stop.set_attributes(this,"dhs_fast_timer_start_no_stop", 0x4000 );
    dhs_fast_timer_pending.set_attributes(this,"dhs_fast_timer_pending", 0x8000 );
    dhs_slow_timer_start_no_stop.set_attributes(this,"dhs_slow_timer_start_no_stop", 0x10000 );
    dhs_slow_timer_pending.set_attributes(this,"dhs_slow_timer_pending", 0x14000 );
    dhs_dtdmlo_calendar.set_attributes(this,"dhs_dtdmlo_calendar", 0x18100 );
    dhs_dtdmhi_calendar.set_attributes(this,"dhs_dtdmhi_calendar", 0x18200 );
    dhs_sch_flags.set_attributes(this,"dhs_sch_flags", 0x18400 );
    dhs_doorbell.set_attributes(this,"dhs_doorbell", 0x1a000 );
    dhs_sch_grp_entry.set_attributes(this,"dhs_sch_grp_entry", 0x1c000 );
    dhs_sch_grp_cnt_entry.set_attributes(this,"dhs_sch_grp_cnt_entry", 0x1e000 );
    dhs_rlid_stop.set_attributes(this,"dhs_rlid_stop", 0x20000 );
    dhs_tmr_cnt_sram.set_attributes(this,"dhs_tmr_cnt_sram", 0x30000 );
    dhs_sch_lif_map_sram.set_attributes(this,"dhs_sch_lif_map_sram", 0x40000 );
    dhs_sch_rlid_map_sram.set_attributes(this,"dhs_sch_rlid_map_sram", 0x46000 );
    dhs_sch_grp_sram.set_attributes(this,"dhs_sch_grp_sram", 0x80000 );
    int_srams_ecc.set_attributes(this,"int_srams_ecc", 0xc0030 );
    int_groups.set_attributes(this,"int_groups", 0xc0050 );
    int_sch.set_attributes(this,"int_sch", 0xc0060 );
    int_tmr.set_attributes(this,"int_tmr", 0xc0070 );
}

void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_wid_empty_enable(const cpp_int & _val) { 
    // tmr_wid_empty_enable
    int_var__tmr_wid_empty_enable = _val.convert_to< tmr_wid_empty_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_wid_empty_enable() const {
    return int_var__tmr_wid_empty_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rid_empty_enable(const cpp_int & _val) { 
    // tmr_rid_empty_enable
    int_var__tmr_rid_empty_enable = _val.convert_to< tmr_rid_empty_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rid_empty_enable() const {
    return int_var__tmr_rid_empty_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_wr_txfifo_ovf_enable(const cpp_int & _val) { 
    // tmr_wr_txfifo_ovf_enable
    int_var__tmr_wr_txfifo_ovf_enable = _val.convert_to< tmr_wr_txfifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_wr_txfifo_ovf_enable() const {
    return int_var__tmr_wr_txfifo_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rd_txfifo_ovf_enable(const cpp_int & _val) { 
    // tmr_rd_txfifo_ovf_enable
    int_var__tmr_rd_txfifo_ovf_enable = _val.convert_to< tmr_rd_txfifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rd_txfifo_ovf_enable() const {
    return int_var__tmr_rd_txfifo_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_state_fifo_ovf_enable(const cpp_int & _val) { 
    // tmr_state_fifo_ovf_enable
    int_var__tmr_state_fifo_ovf_enable = _val.convert_to< tmr_state_fifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_state_fifo_ovf_enable() const {
    return int_var__tmr_state_fifo_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_ovf_enable(const cpp_int & _val) { 
    // tmr_hbm_byp_ovf_enable
    int_var__tmr_hbm_byp_ovf_enable = _val.convert_to< tmr_hbm_byp_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_ovf_enable() const {
    return int_var__tmr_hbm_byp_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_wtag_wrap_enable(const cpp_int & _val) { 
    // tmr_hbm_byp_wtag_wrap_enable
    int_var__tmr_hbm_byp_wtag_wrap_enable = _val.convert_to< tmr_hbm_byp_wtag_wrap_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_wtag_wrap_enable() const {
    return int_var__tmr_hbm_byp_wtag_wrap_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_ctime_wrap_enable(const cpp_int & _val) { 
    // ftmr_ctime_wrap_enable
    int_var__ftmr_ctime_wrap_enable = _val.convert_to< ftmr_ctime_wrap_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_ctime_wrap_enable() const {
    return int_var__ftmr_ctime_wrap_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_ctime_wrap_enable(const cpp_int & _val) { 
    // stmr_ctime_wrap_enable
    int_var__stmr_ctime_wrap_enable = _val.convert_to< stmr_ctime_wrap_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_ctime_wrap_enable() const {
    return int_var__stmr_ctime_wrap_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_push_out_of_wheel_enable(const cpp_int & _val) { 
    // stmr_push_out_of_wheel_enable
    int_var__stmr_push_out_of_wheel_enable = _val.convert_to< stmr_push_out_of_wheel_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_push_out_of_wheel_enable() const {
    return int_var__stmr_push_out_of_wheel_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_push_out_of_wheel_enable(const cpp_int & _val) { 
    // ftmr_push_out_of_wheel_enable
    int_var__ftmr_push_out_of_wheel_enable = _val.convert_to< ftmr_push_out_of_wheel_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_push_out_of_wheel_enable() const {
    return int_var__ftmr_push_out_of_wheel_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_key_not_push_enable(const cpp_int & _val) { 
    // ftmr_key_not_push_enable
    int_var__ftmr_key_not_push_enable = _val.convert_to< ftmr_key_not_push_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_key_not_push_enable() const {
    return int_var__ftmr_key_not_push_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_key_not_found_enable(const cpp_int & _val) { 
    // ftmr_key_not_found_enable
    int_var__ftmr_key_not_found_enable = _val.convert_to< ftmr_key_not_found_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_key_not_found_enable() const {
    return int_var__ftmr_key_not_found_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_key_not_push_enable(const cpp_int & _val) { 
    // stmr_key_not_push_enable
    int_var__stmr_key_not_push_enable = _val.convert_to< stmr_key_not_push_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_key_not_push_enable() const {
    return int_var__stmr_key_not_push_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_key_not_found_enable(const cpp_int & _val) { 
    // stmr_key_not_found_enable
    int_var__stmr_key_not_found_enable = _val.convert_to< stmr_key_not_found_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_key_not_found_enable() const {
    return int_var__stmr_key_not_found_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_stall_enable(const cpp_int & _val) { 
    // stmr_stall_enable
    int_var__stmr_stall_enable = _val.convert_to< stmr_stall_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_stall_enable() const {
    return int_var__stmr_stall_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_stall_enable(const cpp_int & _val) { 
    // ftmr_stall_enable
    int_var__ftmr_stall_enable = _val.convert_to< ftmr_stall_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_stall_enable() const {
    return int_var__ftmr_stall_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_fifo_ovf_enable(const cpp_int & _val) { 
    // ftmr_fifo_ovf_enable
    int_var__ftmr_fifo_ovf_enable = _val.convert_to< ftmr_fifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_fifo_ovf_enable() const {
    return int_var__ftmr_fifo_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_fifo_ovf_enable(const cpp_int & _val) { 
    // stmr_fifo_ovf_enable
    int_var__stmr_fifo_ovf_enable = _val.convert_to< stmr_fifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_fifo_ovf_enable() const {
    return int_var__stmr_fifo_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_fifo_efull_enable(const cpp_int & _val) { 
    // ftmr_fifo_efull_enable
    int_var__ftmr_fifo_efull_enable = _val.convert_to< ftmr_fifo_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::ftmr_fifo_efull_enable() const {
    return int_var__ftmr_fifo_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::stmr_fifo_efull_enable(const cpp_int & _val) { 
    // stmr_fifo_efull_enable
    int_var__stmr_fifo_efull_enable = _val.convert_to< stmr_fifo_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::stmr_fifo_efull_enable() const {
    return int_var__stmr_fifo_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rejct_drb_ovf_enable(const cpp_int & _val) { 
    // tmr_rejct_drb_ovf_enable
    int_var__tmr_rejct_drb_ovf_enable = _val.convert_to< tmr_rejct_drb_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rejct_drb_ovf_enable() const {
    return int_var__tmr_rejct_drb_ovf_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rejct_drb_efull_enable(const cpp_int & _val) { 
    // tmr_rejct_drb_efull_enable
    int_var__tmr_rejct_drb_efull_enable = _val.convert_to< tmr_rejct_drb_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rejct_drb_efull_enable() const {
    return int_var__tmr_rejct_drb_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_re_efull_enable(const cpp_int & _val) { 
    // tmr_hbm_re_efull_enable
    int_var__tmr_hbm_re_efull_enable = _val.convert_to< tmr_hbm_re_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_re_efull_enable() const {
    return int_var__tmr_hbm_re_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_we_efull_enable(const cpp_int & _val) { 
    // tmr_hbm_we_efull_enable
    int_var__tmr_hbm_we_efull_enable = _val.convert_to< tmr_hbm_we_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_we_efull_enable() const {
    return int_var__tmr_hbm_we_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_pending_efull_enable(const cpp_int & _val) { 
    // tmr_hbm_byp_pending_efull_enable
    int_var__tmr_hbm_byp_pending_efull_enable = _val.convert_to< tmr_hbm_byp_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_byp_pending_efull_enable() const {
    return int_var__tmr_hbm_byp_pending_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_rd_pending_efull_enable(const cpp_int & _val) { 
    // tmr_hbm_rd_pending_efull_enable
    int_var__tmr_hbm_rd_pending_efull_enable = _val.convert_to< tmr_hbm_rd_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_rd_pending_efull_enable() const {
    return int_var__tmr_hbm_rd_pending_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_wr_pending_efull_enable(const cpp_int & _val) { 
    // tmr_hbm_wr_pending_efull_enable
    int_var__tmr_hbm_wr_pending_efull_enable = _val.convert_to< tmr_hbm_wr_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_hbm_wr_pending_efull_enable() const {
    return int_var__tmr_hbm_wr_pending_efull_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_bresp_err_enable(const cpp_int & _val) { 
    // tmr_bresp_err_enable
    int_var__tmr_bresp_err_enable = _val.convert_to< tmr_bresp_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_bresp_err_enable() const {
    return int_var__tmr_bresp_err_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_bid_err_enable(const cpp_int & _val) { 
    // tmr_bid_err_enable
    int_var__tmr_bid_err_enable = _val.convert_to< tmr_bid_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_bid_err_enable() const {
    return int_var__tmr_bid_err_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rresp_err_enable(const cpp_int & _val) { 
    // tmr_rresp_err_enable
    int_var__tmr_rresp_err_enable = _val.convert_to< tmr_rresp_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rresp_err_enable() const {
    return int_var__tmr_rresp_err_enable;
}
    
void cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rid_err_enable(const cpp_int & _val) { 
    // tmr_rid_err_enable
    int_var__tmr_rid_err_enable = _val.convert_to< tmr_rid_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_enable_clear_t::tmr_rid_err_enable() const {
    return int_var__tmr_rid_err_enable;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_wid_empty_interrupt(const cpp_int & _val) { 
    // tmr_wid_empty_interrupt
    int_var__tmr_wid_empty_interrupt = _val.convert_to< tmr_wid_empty_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_wid_empty_interrupt() const {
    return int_var__tmr_wid_empty_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rid_empty_interrupt(const cpp_int & _val) { 
    // tmr_rid_empty_interrupt
    int_var__tmr_rid_empty_interrupt = _val.convert_to< tmr_rid_empty_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rid_empty_interrupt() const {
    return int_var__tmr_rid_empty_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_wr_txfifo_ovf_interrupt(const cpp_int & _val) { 
    // tmr_wr_txfifo_ovf_interrupt
    int_var__tmr_wr_txfifo_ovf_interrupt = _val.convert_to< tmr_wr_txfifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_wr_txfifo_ovf_interrupt() const {
    return int_var__tmr_wr_txfifo_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rd_txfifo_ovf_interrupt(const cpp_int & _val) { 
    // tmr_rd_txfifo_ovf_interrupt
    int_var__tmr_rd_txfifo_ovf_interrupt = _val.convert_to< tmr_rd_txfifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rd_txfifo_ovf_interrupt() const {
    return int_var__tmr_rd_txfifo_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_state_fifo_ovf_interrupt(const cpp_int & _val) { 
    // tmr_state_fifo_ovf_interrupt
    int_var__tmr_state_fifo_ovf_interrupt = _val.convert_to< tmr_state_fifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_state_fifo_ovf_interrupt() const {
    return int_var__tmr_state_fifo_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_ovf_interrupt(const cpp_int & _val) { 
    // tmr_hbm_byp_ovf_interrupt
    int_var__tmr_hbm_byp_ovf_interrupt = _val.convert_to< tmr_hbm_byp_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_ovf_interrupt() const {
    return int_var__tmr_hbm_byp_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_wtag_wrap_interrupt(const cpp_int & _val) { 
    // tmr_hbm_byp_wtag_wrap_interrupt
    int_var__tmr_hbm_byp_wtag_wrap_interrupt = _val.convert_to< tmr_hbm_byp_wtag_wrap_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_wtag_wrap_interrupt() const {
    return int_var__tmr_hbm_byp_wtag_wrap_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_ctime_wrap_interrupt(const cpp_int & _val) { 
    // ftmr_ctime_wrap_interrupt
    int_var__ftmr_ctime_wrap_interrupt = _val.convert_to< ftmr_ctime_wrap_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_ctime_wrap_interrupt() const {
    return int_var__ftmr_ctime_wrap_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_ctime_wrap_interrupt(const cpp_int & _val) { 
    // stmr_ctime_wrap_interrupt
    int_var__stmr_ctime_wrap_interrupt = _val.convert_to< stmr_ctime_wrap_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_ctime_wrap_interrupt() const {
    return int_var__stmr_ctime_wrap_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_push_out_of_wheel_interrupt(const cpp_int & _val) { 
    // stmr_push_out_of_wheel_interrupt
    int_var__stmr_push_out_of_wheel_interrupt = _val.convert_to< stmr_push_out_of_wheel_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_push_out_of_wheel_interrupt() const {
    return int_var__stmr_push_out_of_wheel_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_push_out_of_wheel_interrupt(const cpp_int & _val) { 
    // ftmr_push_out_of_wheel_interrupt
    int_var__ftmr_push_out_of_wheel_interrupt = _val.convert_to< ftmr_push_out_of_wheel_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_push_out_of_wheel_interrupt() const {
    return int_var__ftmr_push_out_of_wheel_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_key_not_push_interrupt(const cpp_int & _val) { 
    // ftmr_key_not_push_interrupt
    int_var__ftmr_key_not_push_interrupt = _val.convert_to< ftmr_key_not_push_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_key_not_push_interrupt() const {
    return int_var__ftmr_key_not_push_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_key_not_found_interrupt(const cpp_int & _val) { 
    // ftmr_key_not_found_interrupt
    int_var__ftmr_key_not_found_interrupt = _val.convert_to< ftmr_key_not_found_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_key_not_found_interrupt() const {
    return int_var__ftmr_key_not_found_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_key_not_push_interrupt(const cpp_int & _val) { 
    // stmr_key_not_push_interrupt
    int_var__stmr_key_not_push_interrupt = _val.convert_to< stmr_key_not_push_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_key_not_push_interrupt() const {
    return int_var__stmr_key_not_push_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_key_not_found_interrupt(const cpp_int & _val) { 
    // stmr_key_not_found_interrupt
    int_var__stmr_key_not_found_interrupt = _val.convert_to< stmr_key_not_found_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_key_not_found_interrupt() const {
    return int_var__stmr_key_not_found_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_stall_interrupt(const cpp_int & _val) { 
    // stmr_stall_interrupt
    int_var__stmr_stall_interrupt = _val.convert_to< stmr_stall_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_stall_interrupt() const {
    return int_var__stmr_stall_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_stall_interrupt(const cpp_int & _val) { 
    // ftmr_stall_interrupt
    int_var__ftmr_stall_interrupt = _val.convert_to< ftmr_stall_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_stall_interrupt() const {
    return int_var__ftmr_stall_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_fifo_ovf_interrupt(const cpp_int & _val) { 
    // ftmr_fifo_ovf_interrupt
    int_var__ftmr_fifo_ovf_interrupt = _val.convert_to< ftmr_fifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_fifo_ovf_interrupt() const {
    return int_var__ftmr_fifo_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_fifo_ovf_interrupt(const cpp_int & _val) { 
    // stmr_fifo_ovf_interrupt
    int_var__stmr_fifo_ovf_interrupt = _val.convert_to< stmr_fifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_fifo_ovf_interrupt() const {
    return int_var__stmr_fifo_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::ftmr_fifo_efull_interrupt(const cpp_int & _val) { 
    // ftmr_fifo_efull_interrupt
    int_var__ftmr_fifo_efull_interrupt = _val.convert_to< ftmr_fifo_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::ftmr_fifo_efull_interrupt() const {
    return int_var__ftmr_fifo_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::stmr_fifo_efull_interrupt(const cpp_int & _val) { 
    // stmr_fifo_efull_interrupt
    int_var__stmr_fifo_efull_interrupt = _val.convert_to< stmr_fifo_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::stmr_fifo_efull_interrupt() const {
    return int_var__stmr_fifo_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rejct_drb_ovf_interrupt(const cpp_int & _val) { 
    // tmr_rejct_drb_ovf_interrupt
    int_var__tmr_rejct_drb_ovf_interrupt = _val.convert_to< tmr_rejct_drb_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rejct_drb_ovf_interrupt() const {
    return int_var__tmr_rejct_drb_ovf_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rejct_drb_efull_interrupt(const cpp_int & _val) { 
    // tmr_rejct_drb_efull_interrupt
    int_var__tmr_rejct_drb_efull_interrupt = _val.convert_to< tmr_rejct_drb_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rejct_drb_efull_interrupt() const {
    return int_var__tmr_rejct_drb_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_re_efull_interrupt(const cpp_int & _val) { 
    // tmr_hbm_re_efull_interrupt
    int_var__tmr_hbm_re_efull_interrupt = _val.convert_to< tmr_hbm_re_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_re_efull_interrupt() const {
    return int_var__tmr_hbm_re_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_we_efull_interrupt(const cpp_int & _val) { 
    // tmr_hbm_we_efull_interrupt
    int_var__tmr_hbm_we_efull_interrupt = _val.convert_to< tmr_hbm_we_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_we_efull_interrupt() const {
    return int_var__tmr_hbm_we_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_pending_efull_interrupt(const cpp_int & _val) { 
    // tmr_hbm_byp_pending_efull_interrupt
    int_var__tmr_hbm_byp_pending_efull_interrupt = _val.convert_to< tmr_hbm_byp_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_byp_pending_efull_interrupt() const {
    return int_var__tmr_hbm_byp_pending_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_rd_pending_efull_interrupt(const cpp_int & _val) { 
    // tmr_hbm_rd_pending_efull_interrupt
    int_var__tmr_hbm_rd_pending_efull_interrupt = _val.convert_to< tmr_hbm_rd_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_rd_pending_efull_interrupt() const {
    return int_var__tmr_hbm_rd_pending_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_wr_pending_efull_interrupt(const cpp_int & _val) { 
    // tmr_hbm_wr_pending_efull_interrupt
    int_var__tmr_hbm_wr_pending_efull_interrupt = _val.convert_to< tmr_hbm_wr_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_hbm_wr_pending_efull_interrupt() const {
    return int_var__tmr_hbm_wr_pending_efull_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_bresp_err_interrupt(const cpp_int & _val) { 
    // tmr_bresp_err_interrupt
    int_var__tmr_bresp_err_interrupt = _val.convert_to< tmr_bresp_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_bresp_err_interrupt() const {
    return int_var__tmr_bresp_err_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_bid_err_interrupt(const cpp_int & _val) { 
    // tmr_bid_err_interrupt
    int_var__tmr_bid_err_interrupt = _val.convert_to< tmr_bid_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_bid_err_interrupt() const {
    return int_var__tmr_bid_err_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rresp_err_interrupt(const cpp_int & _val) { 
    // tmr_rresp_err_interrupt
    int_var__tmr_rresp_err_interrupt = _val.convert_to< tmr_rresp_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rresp_err_interrupt() const {
    return int_var__tmr_rresp_err_interrupt;
}
    
void cap_txs_csr_int_tmr_int_test_set_t::tmr_rid_err_interrupt(const cpp_int & _val) { 
    // tmr_rid_err_interrupt
    int_var__tmr_rid_err_interrupt = _val.convert_to< tmr_rid_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_tmr_int_test_set_t::tmr_rid_err_interrupt() const {
    return int_var__tmr_rid_err_interrupt;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_wid_empty_enable(const cpp_int & _val) { 
    // sch_wid_empty_enable
    int_var__sch_wid_empty_enable = _val.convert_to< sch_wid_empty_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_wid_empty_enable() const {
    return int_var__sch_wid_empty_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rid_empty_enable(const cpp_int & _val) { 
    // sch_rid_empty_enable
    int_var__sch_rid_empty_enable = _val.convert_to< sch_rid_empty_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rid_empty_enable() const {
    return int_var__sch_rid_empty_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_wr_txfifo_ovf_enable(const cpp_int & _val) { 
    // sch_wr_txfifo_ovf_enable
    int_var__sch_wr_txfifo_ovf_enable = _val.convert_to< sch_wr_txfifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_wr_txfifo_ovf_enable() const {
    return int_var__sch_wr_txfifo_ovf_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rd_txfifo_ovf_enable(const cpp_int & _val) { 
    // sch_rd_txfifo_ovf_enable
    int_var__sch_rd_txfifo_ovf_enable = _val.convert_to< sch_rd_txfifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rd_txfifo_ovf_enable() const {
    return int_var__sch_rd_txfifo_ovf_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_state_fifo_ovf_enable(const cpp_int & _val) { 
    // sch_state_fifo_ovf_enable
    int_var__sch_state_fifo_ovf_enable = _val.convert_to< sch_state_fifo_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_state_fifo_ovf_enable() const {
    return int_var__sch_state_fifo_ovf_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_qid_fifo_enable(const cpp_int & _val) { 
    // sch_drb_cnt_qid_fifo_enable
    int_var__sch_drb_cnt_qid_fifo_enable = _val.convert_to< sch_drb_cnt_qid_fifo_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_qid_fifo_enable() const {
    return int_var__sch_drb_cnt_qid_fifo_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_ovf_enable(const cpp_int & _val) { 
    // sch_hbm_byp_ovf_enable
    int_var__sch_hbm_byp_ovf_enable = _val.convert_to< sch_hbm_byp_ovf_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_ovf_enable() const {
    return int_var__sch_hbm_byp_ovf_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_wtag_wrap_enable(const cpp_int & _val) { 
    // sch_hbm_byp_wtag_wrap_enable
    int_var__sch_hbm_byp_wtag_wrap_enable = _val.convert_to< sch_hbm_byp_wtag_wrap_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_wtag_wrap_enable() const {
    return int_var__sch_hbm_byp_wtag_wrap_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rlid_ovfl_enable(const cpp_int & _val) { 
    // sch_rlid_ovfl_enable
    int_var__sch_rlid_ovfl_enable = _val.convert_to< sch_rlid_ovfl_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rlid_ovfl_enable() const {
    return int_var__sch_rlid_ovfl_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rlid_unfl_enable(const cpp_int & _val) { 
    // sch_rlid_unfl_enable
    int_var__sch_rlid_unfl_enable = _val.convert_to< sch_rlid_unfl_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rlid_unfl_enable() const {
    return int_var__sch_rlid_unfl_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_null_lif_enable(const cpp_int & _val) { 
    // sch_null_lif_enable
    int_var__sch_null_lif_enable = _val.convert_to< sch_null_lif_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_null_lif_enable() const {
    return int_var__sch_null_lif_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_lif_sg_mismatch_enable(const cpp_int & _val) { 
    // sch_lif_sg_mismatch_enable
    int_var__sch_lif_sg_mismatch_enable = _val.convert_to< sch_lif_sg_mismatch_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_lif_sg_mismatch_enable() const {
    return int_var__sch_lif_sg_mismatch_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_aclr_hbm_ln_rollovr_enable(const cpp_int & _val) { 
    // sch_aclr_hbm_ln_rollovr_enable
    int_var__sch_aclr_hbm_ln_rollovr_enable = _val.convert_to< sch_aclr_hbm_ln_rollovr_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_aclr_hbm_ln_rollovr_enable() const {
    return int_var__sch_aclr_hbm_ln_rollovr_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_txdma_msg_efull_enable(const cpp_int & _val) { 
    // sch_txdma_msg_efull_enable
    int_var__sch_txdma_msg_efull_enable = _val.convert_to< sch_txdma_msg_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_txdma_msg_efull_enable() const {
    return int_var__sch_txdma_msg_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_re_efull_enable(const cpp_int & _val) { 
    // sch_hbm_re_efull_enable
    int_var__sch_hbm_re_efull_enable = _val.convert_to< sch_hbm_re_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_re_efull_enable() const {
    return int_var__sch_hbm_re_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_we_efull_enable(const cpp_int & _val) { 
    // sch_hbm_we_efull_enable
    int_var__sch_hbm_we_efull_enable = _val.convert_to< sch_hbm_we_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_we_efull_enable() const {
    return int_var__sch_hbm_we_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_pending_efull_enable(const cpp_int & _val) { 
    // sch_hbm_byp_pending_efull_enable
    int_var__sch_hbm_byp_pending_efull_enable = _val.convert_to< sch_hbm_byp_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_byp_pending_efull_enable() const {
    return int_var__sch_hbm_byp_pending_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_rd_pending_efull_enable(const cpp_int & _val) { 
    // sch_hbm_rd_pending_efull_enable
    int_var__sch_hbm_rd_pending_efull_enable = _val.convert_to< sch_hbm_rd_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_rd_pending_efull_enable() const {
    return int_var__sch_hbm_rd_pending_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_wr_pending_efull_enable(const cpp_int & _val) { 
    // sch_hbm_wr_pending_efull_enable
    int_var__sch_hbm_wr_pending_efull_enable = _val.convert_to< sch_hbm_wr_pending_efull_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_hbm_wr_pending_efull_enable() const {
    return int_var__sch_hbm_wr_pending_efull_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_bresp_err_enable(const cpp_int & _val) { 
    // sch_bresp_err_enable
    int_var__sch_bresp_err_enable = _val.convert_to< sch_bresp_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_bresp_err_enable() const {
    return int_var__sch_bresp_err_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_bid_err_enable(const cpp_int & _val) { 
    // sch_bid_err_enable
    int_var__sch_bid_err_enable = _val.convert_to< sch_bid_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_bid_err_enable() const {
    return int_var__sch_bid_err_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rresp_err_enable(const cpp_int & _val) { 
    // sch_rresp_err_enable
    int_var__sch_rresp_err_enable = _val.convert_to< sch_rresp_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rresp_err_enable() const {
    return int_var__sch_rresp_err_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_rid_err_enable(const cpp_int & _val) { 
    // sch_rid_err_enable
    int_var__sch_rid_err_enable = _val.convert_to< sch_rid_err_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_rid_err_enable() const {
    return int_var__sch_rid_err_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_ovfl_enable(const cpp_int & _val) { 
    // sch_drb_cnt_ovfl_enable
    int_var__sch_drb_cnt_ovfl_enable = _val.convert_to< sch_drb_cnt_ovfl_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_ovfl_enable() const {
    return int_var__sch_drb_cnt_ovfl_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_unfl_enable(const cpp_int & _val) { 
    // sch_drb_cnt_unfl_enable
    int_var__sch_drb_cnt_unfl_enable = _val.convert_to< sch_drb_cnt_unfl_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_drb_cnt_unfl_enable() const {
    return int_var__sch_drb_cnt_unfl_enable;
}
    
void cap_txs_csr_int_sch_int_enable_clear_t::sch_txdma_msg_ovfl_enable(const cpp_int & _val) { 
    // sch_txdma_msg_ovfl_enable
    int_var__sch_txdma_msg_ovfl_enable = _val.convert_to< sch_txdma_msg_ovfl_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_enable_clear_t::sch_txdma_msg_ovfl_enable() const {
    return int_var__sch_txdma_msg_ovfl_enable;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_wid_empty_interrupt(const cpp_int & _val) { 
    // sch_wid_empty_interrupt
    int_var__sch_wid_empty_interrupt = _val.convert_to< sch_wid_empty_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_wid_empty_interrupt() const {
    return int_var__sch_wid_empty_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rid_empty_interrupt(const cpp_int & _val) { 
    // sch_rid_empty_interrupt
    int_var__sch_rid_empty_interrupt = _val.convert_to< sch_rid_empty_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rid_empty_interrupt() const {
    return int_var__sch_rid_empty_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_wr_txfifo_ovf_interrupt(const cpp_int & _val) { 
    // sch_wr_txfifo_ovf_interrupt
    int_var__sch_wr_txfifo_ovf_interrupt = _val.convert_to< sch_wr_txfifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_wr_txfifo_ovf_interrupt() const {
    return int_var__sch_wr_txfifo_ovf_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rd_txfifo_ovf_interrupt(const cpp_int & _val) { 
    // sch_rd_txfifo_ovf_interrupt
    int_var__sch_rd_txfifo_ovf_interrupt = _val.convert_to< sch_rd_txfifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rd_txfifo_ovf_interrupt() const {
    return int_var__sch_rd_txfifo_ovf_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_state_fifo_ovf_interrupt(const cpp_int & _val) { 
    // sch_state_fifo_ovf_interrupt
    int_var__sch_state_fifo_ovf_interrupt = _val.convert_to< sch_state_fifo_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_state_fifo_ovf_interrupt() const {
    return int_var__sch_state_fifo_ovf_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_qid_fifo_interrupt(const cpp_int & _val) { 
    // sch_drb_cnt_qid_fifo_interrupt
    int_var__sch_drb_cnt_qid_fifo_interrupt = _val.convert_to< sch_drb_cnt_qid_fifo_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_qid_fifo_interrupt() const {
    return int_var__sch_drb_cnt_qid_fifo_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_ovf_interrupt(const cpp_int & _val) { 
    // sch_hbm_byp_ovf_interrupt
    int_var__sch_hbm_byp_ovf_interrupt = _val.convert_to< sch_hbm_byp_ovf_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_ovf_interrupt() const {
    return int_var__sch_hbm_byp_ovf_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_wtag_wrap_interrupt(const cpp_int & _val) { 
    // sch_hbm_byp_wtag_wrap_interrupt
    int_var__sch_hbm_byp_wtag_wrap_interrupt = _val.convert_to< sch_hbm_byp_wtag_wrap_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_wtag_wrap_interrupt() const {
    return int_var__sch_hbm_byp_wtag_wrap_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rlid_ovfl_interrupt(const cpp_int & _val) { 
    // sch_rlid_ovfl_interrupt
    int_var__sch_rlid_ovfl_interrupt = _val.convert_to< sch_rlid_ovfl_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rlid_ovfl_interrupt() const {
    return int_var__sch_rlid_ovfl_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rlid_unfl_interrupt(const cpp_int & _val) { 
    // sch_rlid_unfl_interrupt
    int_var__sch_rlid_unfl_interrupt = _val.convert_to< sch_rlid_unfl_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rlid_unfl_interrupt() const {
    return int_var__sch_rlid_unfl_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_null_lif_interrupt(const cpp_int & _val) { 
    // sch_null_lif_interrupt
    int_var__sch_null_lif_interrupt = _val.convert_to< sch_null_lif_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_null_lif_interrupt() const {
    return int_var__sch_null_lif_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_lif_sg_mismatch_interrupt(const cpp_int & _val) { 
    // sch_lif_sg_mismatch_interrupt
    int_var__sch_lif_sg_mismatch_interrupt = _val.convert_to< sch_lif_sg_mismatch_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_lif_sg_mismatch_interrupt() const {
    return int_var__sch_lif_sg_mismatch_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_aclr_hbm_ln_rollovr_interrupt(const cpp_int & _val) { 
    // sch_aclr_hbm_ln_rollovr_interrupt
    int_var__sch_aclr_hbm_ln_rollovr_interrupt = _val.convert_to< sch_aclr_hbm_ln_rollovr_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_aclr_hbm_ln_rollovr_interrupt() const {
    return int_var__sch_aclr_hbm_ln_rollovr_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_txdma_msg_efull_interrupt(const cpp_int & _val) { 
    // sch_txdma_msg_efull_interrupt
    int_var__sch_txdma_msg_efull_interrupt = _val.convert_to< sch_txdma_msg_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_txdma_msg_efull_interrupt() const {
    return int_var__sch_txdma_msg_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_re_efull_interrupt(const cpp_int & _val) { 
    // sch_hbm_re_efull_interrupt
    int_var__sch_hbm_re_efull_interrupt = _val.convert_to< sch_hbm_re_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_re_efull_interrupt() const {
    return int_var__sch_hbm_re_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_we_efull_interrupt(const cpp_int & _val) { 
    // sch_hbm_we_efull_interrupt
    int_var__sch_hbm_we_efull_interrupt = _val.convert_to< sch_hbm_we_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_we_efull_interrupt() const {
    return int_var__sch_hbm_we_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_pending_efull_interrupt(const cpp_int & _val) { 
    // sch_hbm_byp_pending_efull_interrupt
    int_var__sch_hbm_byp_pending_efull_interrupt = _val.convert_to< sch_hbm_byp_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_byp_pending_efull_interrupt() const {
    return int_var__sch_hbm_byp_pending_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_rd_pending_efull_interrupt(const cpp_int & _val) { 
    // sch_hbm_rd_pending_efull_interrupt
    int_var__sch_hbm_rd_pending_efull_interrupt = _val.convert_to< sch_hbm_rd_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_rd_pending_efull_interrupt() const {
    return int_var__sch_hbm_rd_pending_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_hbm_wr_pending_efull_interrupt(const cpp_int & _val) { 
    // sch_hbm_wr_pending_efull_interrupt
    int_var__sch_hbm_wr_pending_efull_interrupt = _val.convert_to< sch_hbm_wr_pending_efull_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_hbm_wr_pending_efull_interrupt() const {
    return int_var__sch_hbm_wr_pending_efull_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_bresp_err_interrupt(const cpp_int & _val) { 
    // sch_bresp_err_interrupt
    int_var__sch_bresp_err_interrupt = _val.convert_to< sch_bresp_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_bresp_err_interrupt() const {
    return int_var__sch_bresp_err_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_bid_err_interrupt(const cpp_int & _val) { 
    // sch_bid_err_interrupt
    int_var__sch_bid_err_interrupt = _val.convert_to< sch_bid_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_bid_err_interrupt() const {
    return int_var__sch_bid_err_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rresp_err_interrupt(const cpp_int & _val) { 
    // sch_rresp_err_interrupt
    int_var__sch_rresp_err_interrupt = _val.convert_to< sch_rresp_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rresp_err_interrupt() const {
    return int_var__sch_rresp_err_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_rid_err_interrupt(const cpp_int & _val) { 
    // sch_rid_err_interrupt
    int_var__sch_rid_err_interrupt = _val.convert_to< sch_rid_err_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_rid_err_interrupt() const {
    return int_var__sch_rid_err_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_ovfl_interrupt(const cpp_int & _val) { 
    // sch_drb_cnt_ovfl_interrupt
    int_var__sch_drb_cnt_ovfl_interrupt = _val.convert_to< sch_drb_cnt_ovfl_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_ovfl_interrupt() const {
    return int_var__sch_drb_cnt_ovfl_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_unfl_interrupt(const cpp_int & _val) { 
    // sch_drb_cnt_unfl_interrupt
    int_var__sch_drb_cnt_unfl_interrupt = _val.convert_to< sch_drb_cnt_unfl_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_drb_cnt_unfl_interrupt() const {
    return int_var__sch_drb_cnt_unfl_interrupt;
}
    
void cap_txs_csr_int_sch_int_test_set_t::sch_txdma_msg_ovfl_interrupt(const cpp_int & _val) { 
    // sch_txdma_msg_ovfl_interrupt
    int_var__sch_txdma_msg_ovfl_interrupt = _val.convert_to< sch_txdma_msg_ovfl_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_sch_int_test_set_t::sch_txdma_msg_ovfl_interrupt() const {
    return int_var__sch_txdma_msg_ovfl_interrupt;
}
    
void cap_txs_csr_intreg_status_t::int_sch_interrupt(const cpp_int & _val) { 
    // int_sch_interrupt
    int_var__int_sch_interrupt = _val.convert_to< int_sch_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_intreg_status_t::int_sch_interrupt() const {
    return int_var__int_sch_interrupt;
}
    
void cap_txs_csr_intreg_status_t::int_tmr_interrupt(const cpp_int & _val) { 
    // int_tmr_interrupt
    int_var__int_tmr_interrupt = _val.convert_to< int_tmr_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_intreg_status_t::int_tmr_interrupt() const {
    return int_var__int_tmr_interrupt;
}
    
void cap_txs_csr_intreg_status_t::int_srams_ecc_interrupt(const cpp_int & _val) { 
    // int_srams_ecc_interrupt
    int_var__int_srams_ecc_interrupt = _val.convert_to< int_srams_ecc_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_intreg_status_t::int_srams_ecc_interrupt() const {
    return int_var__int_srams_ecc_interrupt;
}
    
void cap_txs_csr_int_groups_int_enable_rw_reg_t::int_sch_enable(const cpp_int & _val) { 
    // int_sch_enable
    int_var__int_sch_enable = _val.convert_to< int_sch_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_groups_int_enable_rw_reg_t::int_sch_enable() const {
    return int_var__int_sch_enable;
}
    
void cap_txs_csr_int_groups_int_enable_rw_reg_t::int_tmr_enable(const cpp_int & _val) { 
    // int_tmr_enable
    int_var__int_tmr_enable = _val.convert_to< int_tmr_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_groups_int_enable_rw_reg_t::int_tmr_enable() const {
    return int_var__int_tmr_enable;
}
    
void cap_txs_csr_int_groups_int_enable_rw_reg_t::int_srams_ecc_enable(const cpp_int & _val) { 
    // int_srams_ecc_enable
    int_var__int_srams_ecc_enable = _val.convert_to< int_srams_ecc_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_groups_int_enable_rw_reg_t::int_srams_ecc_enable() const {
    return int_var__int_srams_ecc_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::tmr_cnt_uncorrectable_enable(const cpp_int & _val) { 
    // tmr_cnt_uncorrectable_enable
    int_var__tmr_cnt_uncorrectable_enable = _val.convert_to< tmr_cnt_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::tmr_cnt_uncorrectable_enable() const {
    return int_var__tmr_cnt_uncorrectable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::tmr_cnt_correctable_enable(const cpp_int & _val) { 
    // tmr_cnt_correctable_enable
    int_var__tmr_cnt_correctable_enable = _val.convert_to< tmr_cnt_correctable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::tmr_cnt_correctable_enable() const {
    return int_var__tmr_cnt_correctable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_lif_map_uncorrectable_enable(const cpp_int & _val) { 
    // sch_lif_map_uncorrectable_enable
    int_var__sch_lif_map_uncorrectable_enable = _val.convert_to< sch_lif_map_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_lif_map_uncorrectable_enable() const {
    return int_var__sch_lif_map_uncorrectable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_lif_map_correctable_enable(const cpp_int & _val) { 
    // sch_lif_map_correctable_enable
    int_var__sch_lif_map_correctable_enable = _val.convert_to< sch_lif_map_correctable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_lif_map_correctable_enable() const {
    return int_var__sch_lif_map_correctable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_rlid_map_uncorrectable_enable(const cpp_int & _val) { 
    // sch_rlid_map_uncorrectable_enable
    int_var__sch_rlid_map_uncorrectable_enable = _val.convert_to< sch_rlid_map_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_rlid_map_uncorrectable_enable() const {
    return int_var__sch_rlid_map_uncorrectable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_rlid_map_correctable_enable(const cpp_int & _val) { 
    // sch_rlid_map_correctable_enable
    int_var__sch_rlid_map_correctable_enable = _val.convert_to< sch_rlid_map_correctable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_rlid_map_correctable_enable() const {
    return int_var__sch_rlid_map_correctable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_grp_uncorrectable_enable(const cpp_int & _val) { 
    // sch_grp_uncorrectable_enable
    int_var__sch_grp_uncorrectable_enable = _val.convert_to< sch_grp_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_grp_uncorrectable_enable() const {
    return int_var__sch_grp_uncorrectable_enable;
}
    
void cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_grp_correctable_enable(const cpp_int & _val) { 
    // sch_grp_correctable_enable
    int_var__sch_grp_correctable_enable = _val.convert_to< sch_grp_correctable_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_int_enable_set_t::sch_grp_correctable_enable() const {
    return int_var__sch_grp_correctable_enable;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::tmr_cnt_uncorrectable_interrupt(const cpp_int & _val) { 
    // tmr_cnt_uncorrectable_interrupt
    int_var__tmr_cnt_uncorrectable_interrupt = _val.convert_to< tmr_cnt_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::tmr_cnt_uncorrectable_interrupt() const {
    return int_var__tmr_cnt_uncorrectable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::tmr_cnt_correctable_interrupt(const cpp_int & _val) { 
    // tmr_cnt_correctable_interrupt
    int_var__tmr_cnt_correctable_interrupt = _val.convert_to< tmr_cnt_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::tmr_cnt_correctable_interrupt() const {
    return int_var__tmr_cnt_correctable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_lif_map_uncorrectable_interrupt(const cpp_int & _val) { 
    // sch_lif_map_uncorrectable_interrupt
    int_var__sch_lif_map_uncorrectable_interrupt = _val.convert_to< sch_lif_map_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_lif_map_uncorrectable_interrupt() const {
    return int_var__sch_lif_map_uncorrectable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_lif_map_correctable_interrupt(const cpp_int & _val) { 
    // sch_lif_map_correctable_interrupt
    int_var__sch_lif_map_correctable_interrupt = _val.convert_to< sch_lif_map_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_lif_map_correctable_interrupt() const {
    return int_var__sch_lif_map_correctable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_rlid_map_uncorrectable_interrupt(const cpp_int & _val) { 
    // sch_rlid_map_uncorrectable_interrupt
    int_var__sch_rlid_map_uncorrectable_interrupt = _val.convert_to< sch_rlid_map_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_rlid_map_uncorrectable_interrupt() const {
    return int_var__sch_rlid_map_uncorrectable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_rlid_map_correctable_interrupt(const cpp_int & _val) { 
    // sch_rlid_map_correctable_interrupt
    int_var__sch_rlid_map_correctable_interrupt = _val.convert_to< sch_rlid_map_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_rlid_map_correctable_interrupt() const {
    return int_var__sch_rlid_map_correctable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_grp_uncorrectable_interrupt(const cpp_int & _val) { 
    // sch_grp_uncorrectable_interrupt
    int_var__sch_grp_uncorrectable_interrupt = _val.convert_to< sch_grp_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_grp_uncorrectable_interrupt() const {
    return int_var__sch_grp_uncorrectable_interrupt;
}
    
void cap_txs_csr_int_srams_ecc_intreg_t::sch_grp_correctable_interrupt(const cpp_int & _val) { 
    // sch_grp_correctable_interrupt
    int_var__sch_grp_correctable_interrupt = _val.convert_to< sch_grp_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_txs_csr_int_srams_ecc_intreg_t::sch_grp_correctable_interrupt() const {
    return int_var__sch_grp_correctable_interrupt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::clr_pend_cnt(const cpp_int & _val) { 
    // clr_pend_cnt
    int_var__clr_pend_cnt = _val.convert_to< clr_pend_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::clr_pend_cnt() const {
    return int_var__clr_pend_cnt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::rl_thr(const cpp_int & _val) { 
    // rl_thr
    int_var__rl_thr = _val.convert_to< rl_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::rl_thr() const {
    return int_var__rl_thr;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::bckgr_cnt(const cpp_int & _val) { 
    // bckgr_cnt
    int_var__bckgr_cnt = _val.convert_to< bckgr_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::bckgr_cnt() const {
    return int_var__bckgr_cnt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::bckgr(const cpp_int & _val) { 
    // bckgr
    int_var__bckgr = _val.convert_to< bckgr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::bckgr() const {
    return int_var__bckgr;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_mode(const cpp_int & _val) { 
    // hbm_mode
    int_var__hbm_mode = _val.convert_to< hbm_mode_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_mode() const {
    return int_var__hbm_mode;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_offset(const cpp_int & _val) { 
    // qid_offset
    int_var__qid_offset = _val.convert_to< qid_offset_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_offset() const {
    return int_var__qid_offset;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::lif() const {
    return int_var__lif;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::auto_clr(const cpp_int & _val) { 
    // auto_clr
    int_var__auto_clr = _val.convert_to< auto_clr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::auto_clr() const {
    return int_var__auto_clr;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::rr_sel(const cpp_int & _val) { 
    // rr_sel
    int_var__rr_sel = _val.convert_to< rr_sel_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::rr_sel() const {
    return int_var__rr_sel;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_vld(const cpp_int & _val) { 
    // qid_fifo_elm0_vld
    int_var__qid_fifo_elm0_vld = _val.convert_to< qid_fifo_elm0_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_vld() const {
    return int_var__qid_fifo_elm0_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_jnk(const cpp_int & _val) { 
    // qid_fifo_elm0_jnk
    int_var__qid_fifo_elm0_jnk = _val.convert_to< qid_fifo_elm0_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_jnk() const {
    return int_var__qid_fifo_elm0_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_qid(const cpp_int & _val) { 
    // qid_fifo_elm0_qid
    int_var__qid_fifo_elm0_qid = _val.convert_to< qid_fifo_elm0_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm0_qid() const {
    return int_var__qid_fifo_elm0_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_vld(const cpp_int & _val) { 
    // qid_fifo_elm1_vld
    int_var__qid_fifo_elm1_vld = _val.convert_to< qid_fifo_elm1_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_vld() const {
    return int_var__qid_fifo_elm1_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_jnk(const cpp_int & _val) { 
    // qid_fifo_elm1_jnk
    int_var__qid_fifo_elm1_jnk = _val.convert_to< qid_fifo_elm1_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_jnk() const {
    return int_var__qid_fifo_elm1_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_qid(const cpp_int & _val) { 
    // qid_fifo_elm1_qid
    int_var__qid_fifo_elm1_qid = _val.convert_to< qid_fifo_elm1_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm1_qid() const {
    return int_var__qid_fifo_elm1_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_vld(const cpp_int & _val) { 
    // qid_fifo_elm2_vld
    int_var__qid_fifo_elm2_vld = _val.convert_to< qid_fifo_elm2_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_vld() const {
    return int_var__qid_fifo_elm2_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_jnk(const cpp_int & _val) { 
    // qid_fifo_elm2_jnk
    int_var__qid_fifo_elm2_jnk = _val.convert_to< qid_fifo_elm2_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_jnk() const {
    return int_var__qid_fifo_elm2_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_qid(const cpp_int & _val) { 
    // qid_fifo_elm2_qid
    int_var__qid_fifo_elm2_qid = _val.convert_to< qid_fifo_elm2_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm2_qid() const {
    return int_var__qid_fifo_elm2_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_vld(const cpp_int & _val) { 
    // qid_fifo_elm3_vld
    int_var__qid_fifo_elm3_vld = _val.convert_to< qid_fifo_elm3_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_vld() const {
    return int_var__qid_fifo_elm3_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_jnk(const cpp_int & _val) { 
    // qid_fifo_elm3_jnk
    int_var__qid_fifo_elm3_jnk = _val.convert_to< qid_fifo_elm3_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_jnk() const {
    return int_var__qid_fifo_elm3_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_qid(const cpp_int & _val) { 
    // qid_fifo_elm3_qid
    int_var__qid_fifo_elm3_qid = _val.convert_to< qid_fifo_elm3_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm3_qid() const {
    return int_var__qid_fifo_elm3_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_vld(const cpp_int & _val) { 
    // qid_fifo_elm4_vld
    int_var__qid_fifo_elm4_vld = _val.convert_to< qid_fifo_elm4_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_vld() const {
    return int_var__qid_fifo_elm4_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_jnk(const cpp_int & _val) { 
    // qid_fifo_elm4_jnk
    int_var__qid_fifo_elm4_jnk = _val.convert_to< qid_fifo_elm4_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_jnk() const {
    return int_var__qid_fifo_elm4_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_qid(const cpp_int & _val) { 
    // qid_fifo_elm4_qid
    int_var__qid_fifo_elm4_qid = _val.convert_to< qid_fifo_elm4_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm4_qid() const {
    return int_var__qid_fifo_elm4_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_vld(const cpp_int & _val) { 
    // qid_fifo_elm5_vld
    int_var__qid_fifo_elm5_vld = _val.convert_to< qid_fifo_elm5_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_vld() const {
    return int_var__qid_fifo_elm5_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_jnk(const cpp_int & _val) { 
    // qid_fifo_elm5_jnk
    int_var__qid_fifo_elm5_jnk = _val.convert_to< qid_fifo_elm5_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_jnk() const {
    return int_var__qid_fifo_elm5_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_qid(const cpp_int & _val) { 
    // qid_fifo_elm5_qid
    int_var__qid_fifo_elm5_qid = _val.convert_to< qid_fifo_elm5_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm5_qid() const {
    return int_var__qid_fifo_elm5_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_vld(const cpp_int & _val) { 
    // qid_fifo_elm6_vld
    int_var__qid_fifo_elm6_vld = _val.convert_to< qid_fifo_elm6_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_vld() const {
    return int_var__qid_fifo_elm6_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_jnk(const cpp_int & _val) { 
    // qid_fifo_elm6_jnk
    int_var__qid_fifo_elm6_jnk = _val.convert_to< qid_fifo_elm6_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_jnk() const {
    return int_var__qid_fifo_elm6_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_qid(const cpp_int & _val) { 
    // qid_fifo_elm6_qid
    int_var__qid_fifo_elm6_qid = _val.convert_to< qid_fifo_elm6_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm6_qid() const {
    return int_var__qid_fifo_elm6_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_vld(const cpp_int & _val) { 
    // qid_fifo_elm7_vld
    int_var__qid_fifo_elm7_vld = _val.convert_to< qid_fifo_elm7_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_vld() const {
    return int_var__qid_fifo_elm7_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_jnk(const cpp_int & _val) { 
    // qid_fifo_elm7_jnk
    int_var__qid_fifo_elm7_jnk = _val.convert_to< qid_fifo_elm7_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_jnk() const {
    return int_var__qid_fifo_elm7_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_qid(const cpp_int & _val) { 
    // qid_fifo_elm7_qid
    int_var__qid_fifo_elm7_qid = _val.convert_to< qid_fifo_elm7_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm7_qid() const {
    return int_var__qid_fifo_elm7_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_vld(const cpp_int & _val) { 
    // qid_fifo_elm8_vld
    int_var__qid_fifo_elm8_vld = _val.convert_to< qid_fifo_elm8_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_vld() const {
    return int_var__qid_fifo_elm8_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_jnk(const cpp_int & _val) { 
    // qid_fifo_elm8_jnk
    int_var__qid_fifo_elm8_jnk = _val.convert_to< qid_fifo_elm8_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_jnk() const {
    return int_var__qid_fifo_elm8_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_qid(const cpp_int & _val) { 
    // qid_fifo_elm8_qid
    int_var__qid_fifo_elm8_qid = _val.convert_to< qid_fifo_elm8_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm8_qid() const {
    return int_var__qid_fifo_elm8_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_vld(const cpp_int & _val) { 
    // qid_fifo_elm9_vld
    int_var__qid_fifo_elm9_vld = _val.convert_to< qid_fifo_elm9_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_vld() const {
    return int_var__qid_fifo_elm9_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_jnk(const cpp_int & _val) { 
    // qid_fifo_elm9_jnk
    int_var__qid_fifo_elm9_jnk = _val.convert_to< qid_fifo_elm9_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_jnk() const {
    return int_var__qid_fifo_elm9_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_qid(const cpp_int & _val) { 
    // qid_fifo_elm9_qid
    int_var__qid_fifo_elm9_qid = _val.convert_to< qid_fifo_elm9_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm9_qid() const {
    return int_var__qid_fifo_elm9_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_vld(const cpp_int & _val) { 
    // qid_fifo_elm10_vld
    int_var__qid_fifo_elm10_vld = _val.convert_to< qid_fifo_elm10_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_vld() const {
    return int_var__qid_fifo_elm10_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_jnk(const cpp_int & _val) { 
    // qid_fifo_elm10_jnk
    int_var__qid_fifo_elm10_jnk = _val.convert_to< qid_fifo_elm10_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_jnk() const {
    return int_var__qid_fifo_elm10_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_qid(const cpp_int & _val) { 
    // qid_fifo_elm10_qid
    int_var__qid_fifo_elm10_qid = _val.convert_to< qid_fifo_elm10_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm10_qid() const {
    return int_var__qid_fifo_elm10_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_vld(const cpp_int & _val) { 
    // qid_fifo_elm11_vld
    int_var__qid_fifo_elm11_vld = _val.convert_to< qid_fifo_elm11_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_vld() const {
    return int_var__qid_fifo_elm11_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_jnk(const cpp_int & _val) { 
    // qid_fifo_elm11_jnk
    int_var__qid_fifo_elm11_jnk = _val.convert_to< qid_fifo_elm11_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_jnk() const {
    return int_var__qid_fifo_elm11_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_qid(const cpp_int & _val) { 
    // qid_fifo_elm11_qid
    int_var__qid_fifo_elm11_qid = _val.convert_to< qid_fifo_elm11_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm11_qid() const {
    return int_var__qid_fifo_elm11_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_vld(const cpp_int & _val) { 
    // qid_fifo_elm12_vld
    int_var__qid_fifo_elm12_vld = _val.convert_to< qid_fifo_elm12_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_vld() const {
    return int_var__qid_fifo_elm12_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_jnk(const cpp_int & _val) { 
    // qid_fifo_elm12_jnk
    int_var__qid_fifo_elm12_jnk = _val.convert_to< qid_fifo_elm12_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_jnk() const {
    return int_var__qid_fifo_elm12_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_qid(const cpp_int & _val) { 
    // qid_fifo_elm12_qid
    int_var__qid_fifo_elm12_qid = _val.convert_to< qid_fifo_elm12_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm12_qid() const {
    return int_var__qid_fifo_elm12_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_vld(const cpp_int & _val) { 
    // qid_fifo_elm13_vld
    int_var__qid_fifo_elm13_vld = _val.convert_to< qid_fifo_elm13_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_vld() const {
    return int_var__qid_fifo_elm13_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_jnk(const cpp_int & _val) { 
    // qid_fifo_elm13_jnk
    int_var__qid_fifo_elm13_jnk = _val.convert_to< qid_fifo_elm13_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_jnk() const {
    return int_var__qid_fifo_elm13_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_qid(const cpp_int & _val) { 
    // qid_fifo_elm13_qid
    int_var__qid_fifo_elm13_qid = _val.convert_to< qid_fifo_elm13_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm13_qid() const {
    return int_var__qid_fifo_elm13_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_vld(const cpp_int & _val) { 
    // qid_fifo_elm14_vld
    int_var__qid_fifo_elm14_vld = _val.convert_to< qid_fifo_elm14_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_vld() const {
    return int_var__qid_fifo_elm14_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_jnk(const cpp_int & _val) { 
    // qid_fifo_elm14_jnk
    int_var__qid_fifo_elm14_jnk = _val.convert_to< qid_fifo_elm14_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_jnk() const {
    return int_var__qid_fifo_elm14_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_qid(const cpp_int & _val) { 
    // qid_fifo_elm14_qid
    int_var__qid_fifo_elm14_qid = _val.convert_to< qid_fifo_elm14_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm14_qid() const {
    return int_var__qid_fifo_elm14_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_vld(const cpp_int & _val) { 
    // qid_fifo_elm15_vld
    int_var__qid_fifo_elm15_vld = _val.convert_to< qid_fifo_elm15_vld_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_vld() const {
    return int_var__qid_fifo_elm15_vld;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_jnk(const cpp_int & _val) { 
    // qid_fifo_elm15_jnk
    int_var__qid_fifo_elm15_jnk = _val.convert_to< qid_fifo_elm15_jnk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_jnk() const {
    return int_var__qid_fifo_elm15_jnk;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_qid(const cpp_int & _val) { 
    // qid_fifo_elm15_qid
    int_var__qid_fifo_elm15_qid = _val.convert_to< qid_fifo_elm15_qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::qid_fifo_elm15_qid() const {
    return int_var__qid_fifo_elm15_qid;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln(const cpp_int & _val) { 
    // hbm_ln
    int_var__hbm_ln = _val.convert_to< hbm_ln_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln() const {
    return int_var__hbm_ln;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_ptr(const cpp_int & _val) { 
    // hbm_ln_ptr
    int_var__hbm_ln_ptr = _val.convert_to< hbm_ln_ptr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_ptr() const {
    return int_var__hbm_ln_ptr;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_rr_cnt(const cpp_int & _val) { 
    // hbm_rr_cnt
    int_var__hbm_rr_cnt = _val.convert_to< hbm_rr_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_rr_cnt() const {
    return int_var__hbm_rr_cnt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::drb_cnt(const cpp_int & _val) { 
    // drb_cnt
    int_var__drb_cnt = _val.convert_to< drb_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::drb_cnt() const {
    return int_var__drb_cnt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt0(const cpp_int & _val) { 
    // hbm_ln_cnt0
    int_var__hbm_ln_cnt0 = _val.convert_to< hbm_ln_cnt0_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt0() const {
    return int_var__hbm_ln_cnt0;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt1(const cpp_int & _val) { 
    // hbm_ln_cnt1
    int_var__hbm_ln_cnt1 = _val.convert_to< hbm_ln_cnt1_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt1() const {
    return int_var__hbm_ln_cnt1;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt2(const cpp_int & _val) { 
    // hbm_ln_cnt2
    int_var__hbm_ln_cnt2 = _val.convert_to< hbm_ln_cnt2_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt2() const {
    return int_var__hbm_ln_cnt2;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt3(const cpp_int & _val) { 
    // hbm_ln_cnt3
    int_var__hbm_ln_cnt3 = _val.convert_to< hbm_ln_cnt3_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt3() const {
    return int_var__hbm_ln_cnt3;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt4(const cpp_int & _val) { 
    // hbm_ln_cnt4
    int_var__hbm_ln_cnt4 = _val.convert_to< hbm_ln_cnt4_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt4() const {
    return int_var__hbm_ln_cnt4;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt5(const cpp_int & _val) { 
    // hbm_ln_cnt5
    int_var__hbm_ln_cnt5 = _val.convert_to< hbm_ln_cnt5_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt5() const {
    return int_var__hbm_ln_cnt5;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt6(const cpp_int & _val) { 
    // hbm_ln_cnt6
    int_var__hbm_ln_cnt6 = _val.convert_to< hbm_ln_cnt6_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt6() const {
    return int_var__hbm_ln_cnt6;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt7(const cpp_int & _val) { 
    // hbm_ln_cnt7
    int_var__hbm_ln_cnt7 = _val.convert_to< hbm_ln_cnt7_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt7() const {
    return int_var__hbm_ln_cnt7;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt8(const cpp_int & _val) { 
    // hbm_ln_cnt8
    int_var__hbm_ln_cnt8 = _val.convert_to< hbm_ln_cnt8_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt8() const {
    return int_var__hbm_ln_cnt8;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt9(const cpp_int & _val) { 
    // hbm_ln_cnt9
    int_var__hbm_ln_cnt9 = _val.convert_to< hbm_ln_cnt9_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt9() const {
    return int_var__hbm_ln_cnt9;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt10(const cpp_int & _val) { 
    // hbm_ln_cnt10
    int_var__hbm_ln_cnt10 = _val.convert_to< hbm_ln_cnt10_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt10() const {
    return int_var__hbm_ln_cnt10;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt11(const cpp_int & _val) { 
    // hbm_ln_cnt11
    int_var__hbm_ln_cnt11 = _val.convert_to< hbm_ln_cnt11_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt11() const {
    return int_var__hbm_ln_cnt11;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt12(const cpp_int & _val) { 
    // hbm_ln_cnt12
    int_var__hbm_ln_cnt12 = _val.convert_to< hbm_ln_cnt12_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt12() const {
    return int_var__hbm_ln_cnt12;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt13(const cpp_int & _val) { 
    // hbm_ln_cnt13
    int_var__hbm_ln_cnt13 = _val.convert_to< hbm_ln_cnt13_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt13() const {
    return int_var__hbm_ln_cnt13;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt14(const cpp_int & _val) { 
    // hbm_ln_cnt14
    int_var__hbm_ln_cnt14 = _val.convert_to< hbm_ln_cnt14_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt14() const {
    return int_var__hbm_ln_cnt14;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt15(const cpp_int & _val) { 
    // hbm_ln_cnt15
    int_var__hbm_ln_cnt15 = _val.convert_to< hbm_ln_cnt15_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::hbm_ln_cnt15() const {
    return int_var__hbm_ln_cnt15;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::popcnt(const cpp_int & _val) { 
    // popcnt
    int_var__popcnt = _val.convert_to< popcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::popcnt() const {
    return int_var__popcnt;
}
    
void cap_txs_csr_dhs_sch_grp_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::sg_start(const cpp_int & _val) { 
    // sg_start
    int_var__sg_start = _val.convert_to< sg_start_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::sg_start() const {
    return int_var__sg_start;
}
    
void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::sg_end(const cpp_int & _val) { 
    // sg_end
    int_var__sg_end = _val.convert_to< sg_end_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::sg_end() const {
    return int_var__sg_end;
}
    
void cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_active(const cpp_int & _val) { 
    // sg_active
    int_var__sg_active = _val.convert_to< sg_active_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_active() const {
    return int_var__sg_active;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_start(const cpp_int & _val) { 
    // sg_start
    int_var__sg_start = _val.convert_to< sg_start_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_start() const {
    return int_var__sg_start;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_per_cos(const cpp_int & _val) { 
    // sg_per_cos
    int_var__sg_per_cos = _val.convert_to< sg_per_cos_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_per_cos() const {
    return int_var__sg_per_cos;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_act_cos(const cpp_int & _val) { 
    // sg_act_cos
    int_var__sg_act_cos = _val.convert_to< sg_act_cos_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::sg_act_cos() const {
    return int_var__sg_act_cos;
}
    
void cap_txs_csr_dhs_sch_lif_map_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_lif_map_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_cbcnt(const cpp_int & _val) { 
    // slow_cbcnt
    int_var__slow_cbcnt = _val.convert_to< slow_cbcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_cbcnt() const {
    return int_var__slow_cbcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_bcnt(const cpp_int & _val) { 
    // slow_bcnt
    int_var__slow_bcnt = _val.convert_to< slow_bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_bcnt() const {
    return int_var__slow_bcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_lcnt(const cpp_int & _val) { 
    // slow_lcnt
    int_var__slow_lcnt = _val.convert_to< slow_lcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::slow_lcnt() const {
    return int_var__slow_lcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_cbcnt(const cpp_int & _val) { 
    // fast_cbcnt
    int_var__fast_cbcnt = _val.convert_to< fast_cbcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_cbcnt() const {
    return int_var__fast_cbcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_bcnt(const cpp_int & _val) { 
    // fast_bcnt
    int_var__fast_bcnt = _val.convert_to< fast_bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_bcnt() const {
    return int_var__fast_bcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_lcnt(const cpp_int & _val) { 
    // fast_lcnt
    int_var__fast_lcnt = _val.convert_to< fast_lcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::fast_lcnt() const {
    return int_var__fast_lcnt;
}
    
void cap_txs_csr_dhs_tmr_cnt_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_tmr_cnt_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_txs_csr_dhs_rlid_stop_entry_t::setmsk(const cpp_int & _val) { 
    // setmsk
    int_var__setmsk = _val.convert_to< setmsk_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_rlid_stop_entry_t::setmsk() const {
    return int_var__setmsk;
}
    
void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::popcnt(const cpp_int & _val) { 
    // popcnt
    int_var__popcnt = _val.convert_to< popcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::popcnt() const {
    return int_var__popcnt;
}
    
void cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::drb_cnt(const cpp_int & _val) { 
    // drb_cnt
    int_var__drb_cnt = _val.convert_to< drb_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::drb_cnt() const {
    return int_var__drb_cnt;
}
    
void cap_txs_csr_dhs_sch_grp_entry_entry_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::lif() const {
    return int_var__lif;
}
    
void cap_txs_csr_dhs_sch_grp_entry_entry_t::qid_offset(const cpp_int & _val) { 
    // qid_offset
    int_var__qid_offset = _val.convert_to< qid_offset_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::qid_offset() const {
    return int_var__qid_offset;
}
    
void cap_txs_csr_dhs_sch_grp_entry_entry_t::auto_clr(const cpp_int & _val) { 
    // auto_clr
    int_var__auto_clr = _val.convert_to< auto_clr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::auto_clr() const {
    return int_var__auto_clr;
}
    
void cap_txs_csr_dhs_sch_grp_entry_entry_t::rr_sel(const cpp_int & _val) { 
    // rr_sel
    int_var__rr_sel = _val.convert_to< rr_sel_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::rr_sel() const {
    return int_var__rr_sel;
}
    
void cap_txs_csr_dhs_sch_grp_entry_entry_t::rl_thr(const cpp_int & _val) { 
    // rl_thr
    int_var__rl_thr = _val.convert_to< rl_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_grp_entry_entry_t::rl_thr() const {
    return int_var__rl_thr;
}
    
void cap_txs_csr_dhs_doorbell_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_doorbell_entry_t::qid() const {
    return int_var__qid;
}
    
void cap_txs_csr_dhs_doorbell_entry_t::cos(const cpp_int & _val) { 
    // cos
    int_var__cos = _val.convert_to< cos_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_doorbell_entry_t::cos() const {
    return int_var__cos;
}
    
void cap_txs_csr_dhs_doorbell_entry_t::set(const cpp_int & _val) { 
    // set
    int_var__set = _val.convert_to< set_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_doorbell_entry_t::set() const {
    return int_var__set;
}
    
void cap_txs_csr_dhs_sch_flags_entry_t::sch_flags(const cpp_int & _val) { 
    // sch_flags
    int_var__sch_flags = _val.convert_to< sch_flags_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_sch_flags_entry_t::sch_flags() const {
    return int_var__sch_flags;
}
    
void cap_txs_csr_dhs_dtdmhi_calendar_entry_t::dtdm_calendar(const cpp_int & _val) { 
    // dtdm_calendar
    int_var__dtdm_calendar = _val.convert_to< dtdm_calendar_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_dtdmhi_calendar_entry_t::dtdm_calendar() const {
    return int_var__dtdm_calendar;
}
    
void cap_txs_csr_dhs_dtdmlo_calendar_entry_t::dtdm_calendar(const cpp_int & _val) { 
    // dtdm_calendar
    int_var__dtdm_calendar = _val.convert_to< dtdm_calendar_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_dtdmlo_calendar_entry_t::dtdm_calendar() const {
    return int_var__dtdm_calendar;
}
    
void cap_txs_csr_dhs_slow_timer_pending_entry_t::bcnt(const cpp_int & _val) { 
    // bcnt
    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_pending_entry_t::bcnt() const {
    return int_var__bcnt;
}
    
void cap_txs_csr_dhs_slow_timer_pending_entry_t::lcnt(const cpp_int & _val) { 
    // lcnt
    int_var__lcnt = _val.convert_to< lcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_pending_entry_t::lcnt() const {
    return int_var__lcnt;
}
    
void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::typ(const cpp_int & _val) { 
    // typ
    int_var__typ = _val.convert_to< typ_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::typ() const {
    return int_var__typ;
}
    
void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::qid() const {
    return int_var__qid;
}
    
void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::ring(const cpp_int & _val) { 
    // ring
    int_var__ring = _val.convert_to< ring_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::ring() const {
    return int_var__ring;
}
    
void cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::dtime(const cpp_int & _val) { 
    // dtime
    int_var__dtime = _val.convert_to< dtime_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::dtime() const {
    return int_var__dtime;
}
    
void cap_txs_csr_dhs_fast_timer_pending_entry_t::bcnt(const cpp_int & _val) { 
    // bcnt
    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_pending_entry_t::bcnt() const {
    return int_var__bcnt;
}
    
void cap_txs_csr_dhs_fast_timer_pending_entry_t::lcnt(const cpp_int & _val) { 
    // lcnt
    int_var__lcnt = _val.convert_to< lcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_pending_entry_t::lcnt() const {
    return int_var__lcnt;
}
    
void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::typ(const cpp_int & _val) { 
    // typ
    int_var__typ = _val.convert_to< typ_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::typ() const {
    return int_var__typ;
}
    
void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::qid() const {
    return int_var__qid;
}
    
void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::ring(const cpp_int & _val) { 
    // ring
    int_var__ring = _val.convert_to< ring_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::ring() const {
    return int_var__ring;
}
    
void cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::dtime(const cpp_int & _val) { 
    // dtime
    int_var__dtime = _val.convert_to< dtime_cpp_int_t >();
}

cpp_int cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::dtime() const {
    return int_var__dtime;
}
    
void cap_txs_csr_cnt_tmr_axi_bid_uexp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_uexp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_bid_rerr_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_rerr_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_bid_err_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_err_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_bid_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_bid_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_wr_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_wr_req_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_rd_rsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_rsp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_tmr_axi_rd_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_tmr_axi_rd_req_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_bid_uexp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_uexp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_bid_rerr_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_rerr_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_bid_err_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_err_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_bid_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_bid_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_wr_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_wr_req_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_rd_rsp_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_rsp_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_axi_rd_req_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_axi_rd_req_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_sta_stmr_max_bcnt_t::bcnt(const cpp_int & _val) { 
    // bcnt
    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_stmr_max_bcnt_t::bcnt() const {
    return int_var__bcnt;
}
    
void cap_txs_csr_sta_ftmr_max_bcnt_t::bcnt(const cpp_int & _val) { 
    // bcnt
    int_var__bcnt = _val.convert_to< bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_ftmr_max_bcnt_t::bcnt() const {
    return int_var__bcnt;
}
    
void cap_txs_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_txs_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_txs_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_txs_csr_sta_srams_sch_hbm_byp_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_byp_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_sch_hbm_byp_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_byp_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_sch_hbm_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_sch_hbm_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_sch_hbm_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_cfg_sch_hbm_byp_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_hbm_byp_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_sch_hbm_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_hbm_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::correctable() const {
    return int_var__correctable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::addr() const {
    return int_var__addr;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_ecc_sch_grp_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_grp_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::correctable() const {
    return int_var__correctable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::addr() const {
    return int_var__addr;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::correctable() const {
    return int_var__correctable;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::addr() const {
    return int_var__addr;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_ecc_sch_lif_map_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_sch_lif_map_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::correctable() const {
    return int_var__correctable;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::addr() const {
    return int_var__addr;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_ecc_tmr_cnt_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_ecc_tmr_cnt_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_scheduler_dbg2_t::hbm_byp_pending_cnt(const cpp_int & _val) { 
    // hbm_byp_pending_cnt
    int_var__hbm_byp_pending_cnt = _val.convert_to< hbm_byp_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg2_t::hbm_byp_pending_cnt() const {
    return int_var__hbm_byp_pending_cnt;
}
    
void cap_txs_csr_sta_scheduler_dbg2_t::hbm_rd_pending_cnt(const cpp_int & _val) { 
    // hbm_rd_pending_cnt
    int_var__hbm_rd_pending_cnt = _val.convert_to< hbm_rd_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg2_t::hbm_rd_pending_cnt() const {
    return int_var__hbm_rd_pending_cnt;
}
    
void cap_txs_csr_sta_scheduler_dbg2_t::hbm_wr_pending_cnt(const cpp_int & _val) { 
    // hbm_wr_pending_cnt
    int_var__hbm_wr_pending_cnt = _val.convert_to< hbm_wr_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg2_t::hbm_wr_pending_cnt() const {
    return int_var__hbm_wr_pending_cnt;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::txdma_drdy(const cpp_int & _val) { 
    // txdma_drdy
    int_var__txdma_drdy = _val.convert_to< txdma_drdy_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::txdma_drdy() const {
    return int_var__txdma_drdy;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::hbm_re_efull(const cpp_int & _val) { 
    // hbm_re_efull
    int_var__hbm_re_efull = _val.convert_to< hbm_re_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::hbm_re_efull() const {
    return int_var__hbm_re_efull;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::hbm_we_efull(const cpp_int & _val) { 
    // hbm_we_efull
    int_var__hbm_we_efull = _val.convert_to< hbm_we_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::hbm_we_efull() const {
    return int_var__hbm_we_efull;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::txdma_msg_efull(const cpp_int & _val) { 
    // txdma_msg_efull
    int_var__txdma_msg_efull = _val.convert_to< txdma_msg_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::txdma_msg_efull() const {
    return int_var__txdma_msg_efull;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::hbm_byp_pending_efull(const cpp_int & _val) { 
    // hbm_byp_pending_efull
    int_var__hbm_byp_pending_efull = _val.convert_to< hbm_byp_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::hbm_byp_pending_efull() const {
    return int_var__hbm_byp_pending_efull;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::hbm_rd_pending_efull(const cpp_int & _val) { 
    // hbm_rd_pending_efull
    int_var__hbm_rd_pending_efull = _val.convert_to< hbm_rd_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::hbm_rd_pending_efull() const {
    return int_var__hbm_rd_pending_efull;
}
    
void cap_txs_csr_sta_scheduler_dbg_t::hbm_wr_pending_efull(const cpp_int & _val) { 
    // hbm_wr_pending_efull
    int_var__hbm_wr_pending_efull = _val.convert_to< hbm_wr_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_dbg_t::hbm_wr_pending_efull() const {
    return int_var__hbm_wr_pending_efull;
}
    
void cap_txs_csr_sta_sch_lif_sg_mismatch_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_sg_mismatch_t::lif() const {
    return int_var__lif;
}
    
void cap_txs_csr_sta_sch_lif_sg_mismatch_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_sg_mismatch_t::qid() const {
    return int_var__qid;
}
    
void cap_txs_csr_sta_sch_lif_sg_mismatch_t::set(const cpp_int & _val) { 
    // set
    int_var__set = _val.convert_to< set_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_sg_mismatch_t::set() const {
    return int_var__set;
}
    
void cap_txs_csr_sta_sch_lif_map_notactive_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_map_notactive_t::lif() const {
    return int_var__lif;
}
    
void cap_txs_csr_sta_sch_lif_map_notactive_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_map_notactive_t::qid() const {
    return int_var__qid;
}
    
void cap_txs_csr_sta_sch_lif_map_notactive_t::cos(const cpp_int & _val) { 
    // cos
    int_var__cos = _val.convert_to< cos_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_map_notactive_t::cos() const {
    return int_var__cos;
}
    
void cap_txs_csr_sta_sch_lif_map_notactive_t::set(const cpp_int & _val) { 
    // set
    int_var__set = _val.convert_to< set_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_lif_map_notactive_t::set() const {
    return int_var__set;
}
    
void cap_txs_csr_cfg_sch_grp_sram_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_grp_sram_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_txs_csr_cfg_sch_grp_sram_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_grp_sram_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_txs_csr_cfg_sch_grp_sram_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_grp_sram_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass;
}
    
void cap_txs_csr_cfg_sch_grp_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_grp_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_sch_rlid_map_sram_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_rlid_map_sram_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_txs_csr_cfg_sch_rlid_map_sram_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_rlid_map_sram_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_txs_csr_cfg_sch_rlid_map_sram_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_rlid_map_sram_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass;
}
    
void cap_txs_csr_cfg_sch_rlid_map_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_rlid_map_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_sch_lif_map_sram_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_lif_map_sram_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_txs_csr_cfg_sch_lif_map_sram_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_lif_map_sram_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_txs_csr_cfg_sch_lif_map_sram_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_lif_map_sram_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass;
}
    
void cap_txs_csr_cfg_sch_lif_map_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_lif_map_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_tmr_cnt_sram_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_cnt_sram_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_txs_csr_cfg_tmr_cnt_sram_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_cnt_sram_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_txs_csr_cfg_tmr_cnt_sram_t::dhs_eccbypass(const cpp_int & _val) { 
    // dhs_eccbypass
    int_var__dhs_eccbypass = _val.convert_to< dhs_eccbypass_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_cnt_sram_t::dhs_eccbypass() const {
    return int_var__dhs_eccbypass;
}
    
void cap_txs_csr_cfg_tmr_cnt_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_cnt_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_sta_sch_max_hbm_byp_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_sch_max_hbm_byp_t::cnt() const {
    return int_var__cnt;
}
    
void cap_txs_csr_cfg_scheduler_dbg2_t::qid_read(const cpp_int & _val) { 
    // qid_read
    int_var__qid_read = _val.convert_to< qid_read_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg2_t::qid_read() const {
    return int_var__qid_read;
}
    
void cap_txs_csr_cfg_scheduler_dbg2_t::cos_read(const cpp_int & _val) { 
    // cos_read
    int_var__cos_read = _val.convert_to< cos_read_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg2_t::cos_read() const {
    return int_var__cos_read;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::hbm_efc_thr(const cpp_int & _val) { 
    // hbm_efc_thr
    int_var__hbm_efc_thr = _val.convert_to< hbm_efc_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::hbm_efc_thr() const {
    return int_var__hbm_efc_thr;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::txdma_efc_thr(const cpp_int & _val) { 
    // txdma_efc_thr
    int_var__txdma_efc_thr = _val.convert_to< txdma_efc_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::txdma_efc_thr() const {
    return int_var__txdma_efc_thr;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::fifo_mode_thr(const cpp_int & _val) { 
    // fifo_mode_thr
    int_var__fifo_mode_thr = _val.convert_to< fifo_mode_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::fifo_mode_thr() const {
    return int_var__fifo_mode_thr;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_byp(const cpp_int & _val) { 
    // max_hbm_byp
    int_var__max_hbm_byp = _val.convert_to< max_hbm_byp_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_byp() const {
    return int_var__max_hbm_byp;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_wr(const cpp_int & _val) { 
    // max_hbm_wr
    int_var__max_hbm_wr = _val.convert_to< max_hbm_wr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_wr() const {
    return int_var__max_hbm_wr;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_rd(const cpp_int & _val) { 
    // max_hbm_rd
    int_var__max_hbm_rd = _val.convert_to< max_hbm_rd_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::max_hbm_rd() const {
    return int_var__max_hbm_rd;
}
    
void cap_txs_csr_cfg_scheduler_dbg_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_scheduler_dbg_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr0(const cpp_int & _val) { 
    // curr_ptr0
    int_var__curr_ptr0 = _val.convert_to< curr_ptr0_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr0() const {
    return int_var__curr_ptr0;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr1(const cpp_int & _val) { 
    // curr_ptr1
    int_var__curr_ptr1 = _val.convert_to< curr_ptr1_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr1() const {
    return int_var__curr_ptr1;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr2(const cpp_int & _val) { 
    // curr_ptr2
    int_var__curr_ptr2 = _val.convert_to< curr_ptr2_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr2() const {
    return int_var__curr_ptr2;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr3(const cpp_int & _val) { 
    // curr_ptr3
    int_var__curr_ptr3 = _val.convert_to< curr_ptr3_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr3() const {
    return int_var__curr_ptr3;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr4(const cpp_int & _val) { 
    // curr_ptr4
    int_var__curr_ptr4 = _val.convert_to< curr_ptr4_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr4() const {
    return int_var__curr_ptr4;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr5(const cpp_int & _val) { 
    // curr_ptr5
    int_var__curr_ptr5 = _val.convert_to< curr_ptr5_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr5() const {
    return int_var__curr_ptr5;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr6(const cpp_int & _val) { 
    // curr_ptr6
    int_var__curr_ptr6 = _val.convert_to< curr_ptr6_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr6() const {
    return int_var__curr_ptr6;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr7(const cpp_int & _val) { 
    // curr_ptr7
    int_var__curr_ptr7 = _val.convert_to< curr_ptr7_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr7() const {
    return int_var__curr_ptr7;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr8(const cpp_int & _val) { 
    // curr_ptr8
    int_var__curr_ptr8 = _val.convert_to< curr_ptr8_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr8() const {
    return int_var__curr_ptr8;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr9(const cpp_int & _val) { 
    // curr_ptr9
    int_var__curr_ptr9 = _val.convert_to< curr_ptr9_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr9() const {
    return int_var__curr_ptr9;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr10(const cpp_int & _val) { 
    // curr_ptr10
    int_var__curr_ptr10 = _val.convert_to< curr_ptr10_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr10() const {
    return int_var__curr_ptr10;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr11(const cpp_int & _val) { 
    // curr_ptr11
    int_var__curr_ptr11 = _val.convert_to< curr_ptr11_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr11() const {
    return int_var__curr_ptr11;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr12(const cpp_int & _val) { 
    // curr_ptr12
    int_var__curr_ptr12 = _val.convert_to< curr_ptr12_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr12() const {
    return int_var__curr_ptr12;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr13(const cpp_int & _val) { 
    // curr_ptr13
    int_var__curr_ptr13 = _val.convert_to< curr_ptr13_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr13() const {
    return int_var__curr_ptr13;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr14(const cpp_int & _val) { 
    // curr_ptr14
    int_var__curr_ptr14 = _val.convert_to< curr_ptr14_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr14() const {
    return int_var__curr_ptr14;
}
    
void cap_txs_csr_sta_scheduler_rr_t::curr_ptr15(const cpp_int & _val) { 
    // curr_ptr15
    int_var__curr_ptr15 = _val.convert_to< curr_ptr15_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_rr_t::curr_ptr15() const {
    return int_var__curr_ptr15;
}
    
void cap_txs_csr_sta_scheduler_t::hbm_init_done(const cpp_int & _val) { 
    // hbm_init_done
    int_var__hbm_init_done = _val.convert_to< hbm_init_done_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_t::hbm_init_done() const {
    return int_var__hbm_init_done;
}
    
void cap_txs_csr_sta_scheduler_t::sram_init_done(const cpp_int & _val) { 
    // sram_init_done
    int_var__sram_init_done = _val.convert_to< sram_init_done_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_scheduler_t::sram_init_done() const {
    return int_var__sram_init_done;
}
    
void cap_txs_csr_cnt_sch_rlid_start_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_rlid_start_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_rlid_stop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_rlid_stop_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_sta_glb_t::pb_xoff(const cpp_int & _val) { 
    // pb_xoff
    int_var__pb_xoff = _val.convert_to< pb_xoff_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_glb_t::pb_xoff() const {
    return int_var__pb_xoff;
}
    
void cap_txs_csr_cnt_sch_txdma_cos15_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos15_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos14_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos14_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos13_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos13_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos12_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos12_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos11_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos11_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos10_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos10_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos9_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos9_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos8_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos8_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos7_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos7_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos6_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos6_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos5_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos5_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos4_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos4_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos3_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos3_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos2_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos2_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos1_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos1_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_txdma_cos0_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_txdma_cos0_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_fifo_empty_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_fifo_empty_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_doorbell_clr_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_doorbell_clr_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_sch_doorbell_set_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_sch_doorbell_set_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cfg_sch_t::dtdm_lo_map(const cpp_int & _val) { 
    // dtdm_lo_map
    int_var__dtdm_lo_map = _val.convert_to< dtdm_lo_map_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_t::dtdm_lo_map() const {
    return int_var__dtdm_lo_map;
}
    
void cap_txs_csr_cfg_sch_t::dtdm_hi_map(const cpp_int & _val) { 
    // dtdm_hi_map
    int_var__dtdm_hi_map = _val.convert_to< dtdm_hi_map_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_t::dtdm_hi_map() const {
    return int_var__dtdm_hi_map;
}
    
void cap_txs_csr_cfg_sch_t::timeout(const cpp_int & _val) { 
    // timeout
    int_var__timeout = _val.convert_to< timeout_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_t::timeout() const {
    return int_var__timeout;
}
    
void cap_txs_csr_cfg_sch_t::pause(const cpp_int & _val) { 
    // pause
    int_var__pause = _val.convert_to< pause_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_t::pause() const {
    return int_var__pause;
}
    
void cap_txs_csr_cfg_sch_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_sch_t::enable() const {
    return int_var__enable;
}
    
void cap_txs_csr_cfw_scheduler_static_t::hbm_base(const cpp_int & _val) { 
    // hbm_base
    int_var__hbm_base = _val.convert_to< hbm_base_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_static_t::hbm_base() const {
    return int_var__hbm_base;
}
    
void cap_txs_csr_cfw_scheduler_static_t::sch_grp_depth(const cpp_int & _val) { 
    // sch_grp_depth
    int_var__sch_grp_depth = _val.convert_to< sch_grp_depth_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_static_t::sch_grp_depth() const {
    return int_var__sch_grp_depth;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::hbm_hw_init(const cpp_int & _val) { 
    // hbm_hw_init
    int_var__hbm_hw_init = _val.convert_to< hbm_hw_init_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::hbm_hw_init() const {
    return int_var__hbm_hw_init;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::sram_hw_init(const cpp_int & _val) { 
    // sram_hw_init
    int_var__sram_hw_init = _val.convert_to< sram_hw_init_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::sram_hw_init() const {
    return int_var__sram_hw_init;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::enable() const {
    return int_var__enable;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::enable_set_lkup(const cpp_int & _val) { 
    // enable_set_lkup
    int_var__enable_set_lkup = _val.convert_to< enable_set_lkup_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::enable_set_lkup() const {
    return int_var__enable_set_lkup;
}
    
void cap_txs_csr_cfw_scheduler_glb_t::enable_set_byp(const cpp_int & _val) { 
    // enable_set_byp
    int_var__enable_set_byp = _val.convert_to< enable_set_byp_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_scheduler_glb_t::enable_set_byp() const {
    return int_var__enable_set_byp;
}
    
void cap_txs_csr_sta_srams_tmr_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_tmr_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_tmr_hbm_byp_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_byp_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_tmr_hbm_byp_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_byp_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_sta_srams_tmr_hbm_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_txs_csr_sta_srams_tmr_hbm_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_srams_tmr_hbm_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_txs_csr_cfg_tmr_fifo_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_fifo_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_tmr_hbm_byp_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_hbm_byp_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cfg_tmr_hbm_sram_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_tmr_hbm_sram_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_txs_csr_cnt_stmr_pop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_stmr_pop_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_stmr_key_not_found_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_stmr_key_not_found_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_stmr_push_out_of_wheel_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_stmr_push_out_of_wheel_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_stmr_key_not_push_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_stmr_key_not_push_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_stmr_push_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_stmr_push_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_sta_slow_timer_t::tick(const cpp_int & _val) { 
    // tick
    int_var__tick = _val.convert_to< tick_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_slow_timer_t::tick() const {
    return int_var__tick;
}
    
void cap_txs_csr_sta_slow_timer_t::cTime(const cpp_int & _val) { 
    // cTime
    int_var__cTime = _val.convert_to< cTime_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_slow_timer_t::cTime() const {
    return int_var__cTime;
}
    
void cap_txs_csr_sta_slow_timer_t::pTime(const cpp_int & _val) { 
    // pTime
    int_var__pTime = _val.convert_to< pTime_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_slow_timer_t::pTime() const {
    return int_var__pTime;
}
    
void cap_txs_csr_cfg_slow_timer_dbell_t::addr_update(const cpp_int & _val) { 
    // addr_update
    int_var__addr_update = _val.convert_to< addr_update_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_dbell_t::addr_update() const {
    return int_var__addr_update;
}
    
void cap_txs_csr_cfg_slow_timer_dbell_t::data_pid(const cpp_int & _val) { 
    // data_pid
    int_var__data_pid = _val.convert_to< data_pid_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_dbell_t::data_pid() const {
    return int_var__data_pid;
}
    
void cap_txs_csr_cfg_slow_timer_dbell_t::data_reserved(const cpp_int & _val) { 
    // data_reserved
    int_var__data_reserved = _val.convert_to< data_reserved_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_dbell_t::data_reserved() const {
    return int_var__data_reserved;
}
    
void cap_txs_csr_cfg_slow_timer_dbell_t::data_index(const cpp_int & _val) { 
    // data_index
    int_var__data_index = _val.convert_to< data_index_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_dbell_t::data_index() const {
    return int_var__data_index;
}
    
void cap_txs_csr_cfg_force_slow_timer_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_force_slow_timer_t::enable() const {
    return int_var__enable;
}
    
void cap_txs_csr_cfg_force_slow_timer_t::ctime(const cpp_int & _val) { 
    // ctime
    int_var__ctime = _val.convert_to< ctime_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_force_slow_timer_t::ctime() const {
    return int_var__ctime;
}
    
void cap_txs_csr_cfg_slow_timer_t::tick(const cpp_int & _val) { 
    // tick
    int_var__tick = _val.convert_to< tick_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_t::tick() const {
    return int_var__tick;
}
    
void cap_txs_csr_cfg_slow_timer_t::hash_sel(const cpp_int & _val) { 
    // hash_sel
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_slow_timer_t::hash_sel() const {
    return int_var__hash_sel;
}
    
void cap_txs_csr_cnt_ftmr_pop_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_ftmr_pop_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_ftmr_key_not_found_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_ftmr_key_not_found_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_ftmr_key_not_push_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_ftmr_key_not_push_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_cnt_ftmr_push_t::val(const cpp_int & _val) { 
    // val
    int_var__val = _val.convert_to< val_cpp_int_t >();
}

cpp_int cap_txs_csr_cnt_ftmr_push_t::val() const {
    return int_var__val;
}
    
void cap_txs_csr_sta_fast_timer_t::tick(const cpp_int & _val) { 
    // tick
    int_var__tick = _val.convert_to< tick_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_fast_timer_t::tick() const {
    return int_var__tick;
}
    
void cap_txs_csr_sta_fast_timer_t::cTime(const cpp_int & _val) { 
    // cTime
    int_var__cTime = _val.convert_to< cTime_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_fast_timer_t::cTime() const {
    return int_var__cTime;
}
    
void cap_txs_csr_sta_fast_timer_t::pTime(const cpp_int & _val) { 
    // pTime
    int_var__pTime = _val.convert_to< pTime_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_fast_timer_t::pTime() const {
    return int_var__pTime;
}
    
void cap_txs_csr_cfg_fast_timer_dbell_t::addr_update(const cpp_int & _val) { 
    // addr_update
    int_var__addr_update = _val.convert_to< addr_update_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_dbell_t::addr_update() const {
    return int_var__addr_update;
}
    
void cap_txs_csr_cfg_fast_timer_dbell_t::data_pid(const cpp_int & _val) { 
    // data_pid
    int_var__data_pid = _val.convert_to< data_pid_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_dbell_t::data_pid() const {
    return int_var__data_pid;
}
    
void cap_txs_csr_cfg_fast_timer_dbell_t::data_reserved(const cpp_int & _val) { 
    // data_reserved
    int_var__data_reserved = _val.convert_to< data_reserved_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_dbell_t::data_reserved() const {
    return int_var__data_reserved;
}
    
void cap_txs_csr_cfg_fast_timer_dbell_t::data_index(const cpp_int & _val) { 
    // data_index
    int_var__data_index = _val.convert_to< data_index_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_dbell_t::data_index() const {
    return int_var__data_index;
}
    
void cap_txs_csr_cfg_force_fast_timer_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_force_fast_timer_t::enable() const {
    return int_var__enable;
}
    
void cap_txs_csr_cfg_force_fast_timer_t::ctime(const cpp_int & _val) { 
    // ctime
    int_var__ctime = _val.convert_to< ctime_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_force_fast_timer_t::ctime() const {
    return int_var__ctime;
}
    
void cap_txs_csr_cfg_fast_timer_t::tick(const cpp_int & _val) { 
    // tick
    int_var__tick = _val.convert_to< tick_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_t::tick() const {
    return int_var__tick;
}
    
void cap_txs_csr_cfg_fast_timer_t::hash_sel(const cpp_int & _val) { 
    // hash_sel
    int_var__hash_sel = _val.convert_to< hash_sel_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_fast_timer_t::hash_sel() const {
    return int_var__hash_sel;
}
    
void cap_txs_csr_sta_timer_dbg2_t::hbm_byp_pending_cnt(const cpp_int & _val) { 
    // hbm_byp_pending_cnt
    int_var__hbm_byp_pending_cnt = _val.convert_to< hbm_byp_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::hbm_byp_pending_cnt() const {
    return int_var__hbm_byp_pending_cnt;
}
    
void cap_txs_csr_sta_timer_dbg2_t::hbm_rd_pending_cnt(const cpp_int & _val) { 
    // hbm_rd_pending_cnt
    int_var__hbm_rd_pending_cnt = _val.convert_to< hbm_rd_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::hbm_rd_pending_cnt() const {
    return int_var__hbm_rd_pending_cnt;
}
    
void cap_txs_csr_sta_timer_dbg2_t::hbm_wr_pending_cnt(const cpp_int & _val) { 
    // hbm_wr_pending_cnt
    int_var__hbm_wr_pending_cnt = _val.convert_to< hbm_wr_pending_cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::hbm_wr_pending_cnt() const {
    return int_var__hbm_wr_pending_cnt;
}
    
void cap_txs_csr_sta_timer_dbg2_t::stmr_stall(const cpp_int & _val) { 
    // stmr_stall
    int_var__stmr_stall = _val.convert_to< stmr_stall_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::stmr_stall() const {
    return int_var__stmr_stall;
}
    
void cap_txs_csr_sta_timer_dbg2_t::ftmr_stall(const cpp_int & _val) { 
    // ftmr_stall
    int_var__ftmr_stall = _val.convert_to< ftmr_stall_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg2_t::ftmr_stall() const {
    return int_var__ftmr_stall;
}
    
void cap_txs_csr_sta_timer_dbg_t::hbm_re_efull(const cpp_int & _val) { 
    // hbm_re_efull
    int_var__hbm_re_efull = _val.convert_to< hbm_re_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::hbm_re_efull() const {
    return int_var__hbm_re_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::hbm_we_efull(const cpp_int & _val) { 
    // hbm_we_efull
    int_var__hbm_we_efull = _val.convert_to< hbm_we_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::hbm_we_efull() const {
    return int_var__hbm_we_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::rejct_drb_efull(const cpp_int & _val) { 
    // rejct_drb_efull
    int_var__rejct_drb_efull = _val.convert_to< rejct_drb_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::rejct_drb_efull() const {
    return int_var__rejct_drb_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::hbm_byp_pending_efull(const cpp_int & _val) { 
    // hbm_byp_pending_efull
    int_var__hbm_byp_pending_efull = _val.convert_to< hbm_byp_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::hbm_byp_pending_efull() const {
    return int_var__hbm_byp_pending_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::hbm_rd_pending_efull(const cpp_int & _val) { 
    // hbm_rd_pending_efull
    int_var__hbm_rd_pending_efull = _val.convert_to< hbm_rd_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::hbm_rd_pending_efull() const {
    return int_var__hbm_rd_pending_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::hbm_wr_pending_efull(const cpp_int & _val) { 
    // hbm_wr_pending_efull
    int_var__hbm_wr_pending_efull = _val.convert_to< hbm_wr_pending_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::hbm_wr_pending_efull() const {
    return int_var__hbm_wr_pending_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::stmr_fifo_raddr(const cpp_int & _val) { 
    // stmr_fifo_raddr
    int_var__stmr_fifo_raddr = _val.convert_to< stmr_fifo_raddr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::stmr_fifo_raddr() const {
    return int_var__stmr_fifo_raddr;
}
    
void cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_raddr(const cpp_int & _val) { 
    // ftmr_fifo_raddr
    int_var__ftmr_fifo_raddr = _val.convert_to< ftmr_fifo_raddr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_raddr() const {
    return int_var__ftmr_fifo_raddr;
}
    
void cap_txs_csr_sta_timer_dbg_t::stmr_fifo_waddr(const cpp_int & _val) { 
    // stmr_fifo_waddr
    int_var__stmr_fifo_waddr = _val.convert_to< stmr_fifo_waddr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::stmr_fifo_waddr() const {
    return int_var__stmr_fifo_waddr;
}
    
void cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_waddr(const cpp_int & _val) { 
    // ftmr_fifo_waddr
    int_var__ftmr_fifo_waddr = _val.convert_to< ftmr_fifo_waddr_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_waddr() const {
    return int_var__ftmr_fifo_waddr;
}
    
void cap_txs_csr_sta_timer_dbg_t::stmr_fifo_efull(const cpp_int & _val) { 
    // stmr_fifo_efull
    int_var__stmr_fifo_efull = _val.convert_to< stmr_fifo_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::stmr_fifo_efull() const {
    return int_var__stmr_fifo_efull;
}
    
void cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_efull(const cpp_int & _val) { 
    // ftmr_fifo_efull
    int_var__ftmr_fifo_efull = _val.convert_to< ftmr_fifo_efull_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_dbg_t::ftmr_fifo_efull() const {
    return int_var__ftmr_fifo_efull;
}
    
void cap_txs_csr_sta_tmr_max_keys_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_tmr_max_keys_t::cnt() const {
    return int_var__cnt;
}
    
void cap_txs_csr_sta_tmr_max_hbm_byp_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_tmr_max_hbm_byp_t::cnt() const {
    return int_var__cnt;
}
    
void cap_txs_csr_sta_timer_t::hbm_init_done(const cpp_int & _val) { 
    // hbm_init_done
    int_var__hbm_init_done = _val.convert_to< hbm_init_done_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_t::hbm_init_done() const {
    return int_var__hbm_init_done;
}
    
void cap_txs_csr_sta_timer_t::sram_init_done(const cpp_int & _val) { 
    // sram_init_done
    int_var__sram_init_done = _val.convert_to< sram_init_done_cpp_int_t >();
}

cpp_int cap_txs_csr_sta_timer_t::sram_init_done() const {
    return int_var__sram_init_done;
}
    
void cap_txs_csr_cfg_timer_dbg2_t::max_bcnt(const cpp_int & _val) { 
    // max_bcnt
    int_var__max_bcnt = _val.convert_to< max_bcnt_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg2_t::max_bcnt() const {
    return int_var__max_bcnt;
}
    
void cap_txs_csr_cfg_timer_dbg2_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg2_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_cfg_timer_dbg_t::hbm_efc_thr(const cpp_int & _val) { 
    // hbm_efc_thr
    int_var__hbm_efc_thr = _val.convert_to< hbm_efc_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::hbm_efc_thr() const {
    return int_var__hbm_efc_thr;
}
    
void cap_txs_csr_cfg_timer_dbg_t::drb_efc_thr(const cpp_int & _val) { 
    // drb_efc_thr
    int_var__drb_efc_thr = _val.convert_to< drb_efc_thr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::drb_efc_thr() const {
    return int_var__drb_efc_thr;
}
    
void cap_txs_csr_cfg_timer_dbg_t::tmr_stall_thr_hi(const cpp_int & _val) { 
    // tmr_stall_thr_hi
    int_var__tmr_stall_thr_hi = _val.convert_to< tmr_stall_thr_hi_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::tmr_stall_thr_hi() const {
    return int_var__tmr_stall_thr_hi;
}
    
void cap_txs_csr_cfg_timer_dbg_t::tmr_stall_thr_lo(const cpp_int & _val) { 
    // tmr_stall_thr_lo
    int_var__tmr_stall_thr_lo = _val.convert_to< tmr_stall_thr_lo_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::tmr_stall_thr_lo() const {
    return int_var__tmr_stall_thr_lo;
}
    
void cap_txs_csr_cfg_timer_dbg_t::max_hbm_wr(const cpp_int & _val) { 
    // max_hbm_wr
    int_var__max_hbm_wr = _val.convert_to< max_hbm_wr_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::max_hbm_wr() const {
    return int_var__max_hbm_wr;
}
    
void cap_txs_csr_cfg_timer_dbg_t::max_hbm_rd(const cpp_int & _val) { 
    // max_hbm_rd
    int_var__max_hbm_rd = _val.convert_to< max_hbm_rd_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::max_hbm_rd() const {
    return int_var__max_hbm_rd;
}
    
void cap_txs_csr_cfg_timer_dbg_t::max_hbm_byp(const cpp_int & _val) { 
    // max_hbm_byp
    int_var__max_hbm_byp = _val.convert_to< max_hbm_byp_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::max_hbm_byp() const {
    return int_var__max_hbm_byp;
}
    
void cap_txs_csr_cfg_timer_dbg_t::max_tmr_fifo(const cpp_int & _val) { 
    // max_tmr_fifo
    int_var__max_tmr_fifo = _val.convert_to< max_tmr_fifo_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::max_tmr_fifo() const {
    return int_var__max_tmr_fifo;
}
    
void cap_txs_csr_cfg_timer_dbg_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_dbg_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_cfw_timer_glb_t::hbm_hw_init(const cpp_int & _val) { 
    // hbm_hw_init
    int_var__hbm_hw_init = _val.convert_to< hbm_hw_init_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::hbm_hw_init() const {
    return int_var__hbm_hw_init;
}
    
void cap_txs_csr_cfw_timer_glb_t::sram_hw_init(const cpp_int & _val) { 
    // sram_hw_init
    int_var__sram_hw_init = _val.convert_to< sram_hw_init_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::sram_hw_init() const {
    return int_var__sram_hw_init;
}
    
void cap_txs_csr_cfw_timer_glb_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_cfw_timer_glb_t::ftmr_enable(const cpp_int & _val) { 
    // ftmr_enable
    int_var__ftmr_enable = _val.convert_to< ftmr_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::ftmr_enable() const {
    return int_var__ftmr_enable;
}
    
void cap_txs_csr_cfw_timer_glb_t::stmr_enable(const cpp_int & _val) { 
    // stmr_enable
    int_var__stmr_enable = _val.convert_to< stmr_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::stmr_enable() const {
    return int_var__stmr_enable;
}
    
void cap_txs_csr_cfw_timer_glb_t::ftmr_pause(const cpp_int & _val) { 
    // ftmr_pause
    int_var__ftmr_pause = _val.convert_to< ftmr_pause_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::ftmr_pause() const {
    return int_var__ftmr_pause;
}
    
void cap_txs_csr_cfw_timer_glb_t::stmr_pause(const cpp_int & _val) { 
    // stmr_pause
    int_var__stmr_pause = _val.convert_to< stmr_pause_cpp_int_t >();
}

cpp_int cap_txs_csr_cfw_timer_glb_t::stmr_pause() const {
    return int_var__stmr_pause;
}
    
void cap_txs_csr_cfg_timer_static_t::hbm_base(const cpp_int & _val) { 
    // hbm_base
    int_var__hbm_base = _val.convert_to< hbm_base_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_static_t::hbm_base() const {
    return int_var__hbm_base;
}
    
void cap_txs_csr_cfg_timer_static_t::tmr_hsh_depth(const cpp_int & _val) { 
    // tmr_hsh_depth
    int_var__tmr_hsh_depth = _val.convert_to< tmr_hsh_depth_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_static_t::tmr_hsh_depth() const {
    return int_var__tmr_hsh_depth;
}
    
void cap_txs_csr_cfg_timer_static_t::tmr_wheel_depth(const cpp_int & _val) { 
    // tmr_wheel_depth
    int_var__tmr_wheel_depth = _val.convert_to< tmr_wheel_depth_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_timer_static_t::tmr_wheel_depth() const {
    return int_var__tmr_wheel_depth;
}
    
void cap_txs_csr_cfg_axi_attr_t::arcache(const cpp_int & _val) { 
    // arcache
    int_var__arcache = _val.convert_to< arcache_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_axi_attr_t::arcache() const {
    return int_var__arcache;
}
    
void cap_txs_csr_cfg_axi_attr_t::awcache(const cpp_int & _val) { 
    // awcache
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_axi_attr_t::awcache() const {
    return int_var__awcache;
}
    
void cap_txs_csr_cfg_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_axi_attr_t::prot() const {
    return int_var__prot;
}
    
void cap_txs_csr_cfg_axi_attr_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_axi_attr_t::qos() const {
    return int_var__qos;
}
    
void cap_txs_csr_cfg_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_axi_attr_t::lock() const {
    return int_var__lock;
}
    
void cap_txs_csr_cfg_glb_t::dbg_port_select(const cpp_int & _val) { 
    // dbg_port_select
    int_var__dbg_port_select = _val.convert_to< dbg_port_select_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_glb_t::dbg_port_select() const {
    return int_var__dbg_port_select;
}
    
void cap_txs_csr_cfg_glb_t::dbg_port_enable(const cpp_int & _val) { 
    // dbg_port_enable
    int_var__dbg_port_enable = _val.convert_to< dbg_port_enable_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_glb_t::dbg_port_enable() const {
    return int_var__dbg_port_enable;
}
    
void cap_txs_csr_cfg_glb_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_txs_csr_cfg_glb_t::spare() const {
    return int_var__spare;
}
    
void cap_txs_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_txs_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_wid_empty_enable")) { field_val = tmr_wid_empty_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_empty_enable")) { field_val = tmr_rid_empty_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wr_txfifo_ovf_enable")) { field_val = tmr_wr_txfifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rd_txfifo_ovf_enable")) { field_val = tmr_rd_txfifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_state_fifo_ovf_enable")) { field_val = tmr_state_fifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_ovf_enable")) { field_val = tmr_hbm_byp_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_wtag_wrap_enable")) { field_val = tmr_hbm_byp_wtag_wrap_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_ctime_wrap_enable")) { field_val = ftmr_ctime_wrap_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_ctime_wrap_enable")) { field_val = stmr_ctime_wrap_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_push_out_of_wheel_enable")) { field_val = stmr_push_out_of_wheel_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_push_out_of_wheel_enable")) { field_val = ftmr_push_out_of_wheel_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_push_enable")) { field_val = ftmr_key_not_push_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_found_enable")) { field_val = ftmr_key_not_found_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_push_enable")) { field_val = stmr_key_not_push_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_found_enable")) { field_val = stmr_key_not_found_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall_enable")) { field_val = stmr_stall_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall_enable")) { field_val = ftmr_stall_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_ovf_enable")) { field_val = ftmr_fifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_ovf_enable")) { field_val = stmr_fifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull_enable")) { field_val = ftmr_fifo_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull_enable")) { field_val = stmr_fifo_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_ovf_enable")) { field_val = tmr_rejct_drb_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_efull_enable")) { field_val = tmr_rejct_drb_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_re_efull_enable")) { field_val = tmr_hbm_re_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_we_efull_enable")) { field_val = tmr_hbm_we_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_pending_efull_enable")) { field_val = tmr_hbm_byp_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_rd_pending_efull_enable")) { field_val = tmr_hbm_rd_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_wr_pending_efull_enable")) { field_val = tmr_hbm_wr_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bresp_err_enable")) { field_val = tmr_bresp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bid_err_enable")) { field_val = tmr_bid_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rresp_err_enable")) { field_val = tmr_rresp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_err_enable")) { field_val = tmr_rid_err_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_int_test_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_wid_empty_interrupt")) { field_val = tmr_wid_empty_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_empty_interrupt")) { field_val = tmr_rid_empty_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wr_txfifo_ovf_interrupt")) { field_val = tmr_wr_txfifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rd_txfifo_ovf_interrupt")) { field_val = tmr_rd_txfifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_state_fifo_ovf_interrupt")) { field_val = tmr_state_fifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_ovf_interrupt")) { field_val = tmr_hbm_byp_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_wtag_wrap_interrupt")) { field_val = tmr_hbm_byp_wtag_wrap_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_ctime_wrap_interrupt")) { field_val = ftmr_ctime_wrap_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_ctime_wrap_interrupt")) { field_val = stmr_ctime_wrap_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_push_out_of_wheel_interrupt")) { field_val = stmr_push_out_of_wheel_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_push_out_of_wheel_interrupt")) { field_val = ftmr_push_out_of_wheel_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_push_interrupt")) { field_val = ftmr_key_not_push_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_found_interrupt")) { field_val = ftmr_key_not_found_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_push_interrupt")) { field_val = stmr_key_not_push_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_found_interrupt")) { field_val = stmr_key_not_found_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall_interrupt")) { field_val = stmr_stall_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall_interrupt")) { field_val = ftmr_stall_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_ovf_interrupt")) { field_val = ftmr_fifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_ovf_interrupt")) { field_val = stmr_fifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull_interrupt")) { field_val = ftmr_fifo_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull_interrupt")) { field_val = stmr_fifo_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_ovf_interrupt")) { field_val = tmr_rejct_drb_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_efull_interrupt")) { field_val = tmr_rejct_drb_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_re_efull_interrupt")) { field_val = tmr_hbm_re_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_we_efull_interrupt")) { field_val = tmr_hbm_we_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_pending_efull_interrupt")) { field_val = tmr_hbm_byp_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_rd_pending_efull_interrupt")) { field_val = tmr_hbm_rd_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_wr_pending_efull_interrupt")) { field_val = tmr_hbm_wr_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bresp_err_interrupt")) { field_val = tmr_bresp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bid_err_interrupt")) { field_val = tmr_bid_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rresp_err_interrupt")) { field_val = tmr_rresp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_err_interrupt")) { field_val = tmr_rid_err_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_int_sch_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_wid_empty_enable")) { field_val = sch_wid_empty_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_empty_enable")) { field_val = sch_rid_empty_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_wr_txfifo_ovf_enable")) { field_val = sch_wr_txfifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rd_txfifo_ovf_enable")) { field_val = sch_rd_txfifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_state_fifo_ovf_enable")) { field_val = sch_state_fifo_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_qid_fifo_enable")) { field_val = sch_drb_cnt_qid_fifo_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_ovf_enable")) { field_val = sch_hbm_byp_ovf_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_wtag_wrap_enable")) { field_val = sch_hbm_byp_wtag_wrap_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_ovfl_enable")) { field_val = sch_rlid_ovfl_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_unfl_enable")) { field_val = sch_rlid_unfl_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_null_lif_enable")) { field_val = sch_null_lif_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_sg_mismatch_enable")) { field_val = sch_lif_sg_mismatch_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_aclr_hbm_ln_rollovr_enable")) { field_val = sch_aclr_hbm_ln_rollovr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_efull_enable")) { field_val = sch_txdma_msg_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_re_efull_enable")) { field_val = sch_hbm_re_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_we_efull_enable")) { field_val = sch_hbm_we_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_pending_efull_enable")) { field_val = sch_hbm_byp_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_rd_pending_efull_enable")) { field_val = sch_hbm_rd_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_wr_pending_efull_enable")) { field_val = sch_hbm_wr_pending_efull_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bresp_err_enable")) { field_val = sch_bresp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bid_err_enable")) { field_val = sch_bid_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rresp_err_enable")) { field_val = sch_rresp_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_err_enable")) { field_val = sch_rid_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_ovfl_enable")) { field_val = sch_drb_cnt_ovfl_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_unfl_enable")) { field_val = sch_drb_cnt_unfl_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_ovfl_enable")) { field_val = sch_txdma_msg_ovfl_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_sch_int_test_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_wid_empty_interrupt")) { field_val = sch_wid_empty_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_empty_interrupt")) { field_val = sch_rid_empty_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_wr_txfifo_ovf_interrupt")) { field_val = sch_wr_txfifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rd_txfifo_ovf_interrupt")) { field_val = sch_rd_txfifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_state_fifo_ovf_interrupt")) { field_val = sch_state_fifo_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_qid_fifo_interrupt")) { field_val = sch_drb_cnt_qid_fifo_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_ovf_interrupt")) { field_val = sch_hbm_byp_ovf_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_wtag_wrap_interrupt")) { field_val = sch_hbm_byp_wtag_wrap_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_ovfl_interrupt")) { field_val = sch_rlid_ovfl_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_unfl_interrupt")) { field_val = sch_rlid_unfl_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_null_lif_interrupt")) { field_val = sch_null_lif_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_sg_mismatch_interrupt")) { field_val = sch_lif_sg_mismatch_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_aclr_hbm_ln_rollovr_interrupt")) { field_val = sch_aclr_hbm_ln_rollovr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_efull_interrupt")) { field_val = sch_txdma_msg_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_re_efull_interrupt")) { field_val = sch_hbm_re_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_we_efull_interrupt")) { field_val = sch_hbm_we_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_pending_efull_interrupt")) { field_val = sch_hbm_byp_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_rd_pending_efull_interrupt")) { field_val = sch_hbm_rd_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_wr_pending_efull_interrupt")) { field_val = sch_hbm_wr_pending_efull_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bresp_err_interrupt")) { field_val = sch_bresp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bid_err_interrupt")) { field_val = sch_bid_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rresp_err_interrupt")) { field_val = sch_rresp_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_err_interrupt")) { field_val = sch_rid_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_ovfl_interrupt")) { field_val = sch_drb_cnt_ovfl_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_unfl_interrupt")) { field_val = sch_drb_cnt_unfl_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_ovfl_interrupt")) { field_val = sch_txdma_msg_ovfl_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_sch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_sch_interrupt")) { field_val = int_sch_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_tmr_interrupt")) { field_val = int_tmr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_srams_ecc_interrupt")) { field_val = int_srams_ecc_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_sch_enable")) { field_val = int_sch_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_tmr_enable")) { field_val = int_tmr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_srams_ecc_enable")) { field_val = int_srams_ecc_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_int_srams_ecc_int_enable_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_cnt_uncorrectable_enable")) { field_val = tmr_cnt_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_cnt_correctable_enable")) { field_val = tmr_cnt_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_uncorrectable_enable")) { field_val = sch_lif_map_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_correctable_enable")) { field_val = sch_lif_map_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_uncorrectable_enable")) { field_val = sch_rlid_map_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_correctable_enable")) { field_val = sch_rlid_map_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_uncorrectable_enable")) { field_val = sch_grp_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_correctable_enable")) { field_val = sch_grp_correctable_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_srams_ecc_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_cnt_uncorrectable_interrupt")) { field_val = tmr_cnt_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_cnt_correctable_interrupt")) { field_val = tmr_cnt_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_uncorrectable_interrupt")) { field_val = sch_lif_map_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_correctable_interrupt")) { field_val = sch_lif_map_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_uncorrectable_interrupt")) { field_val = sch_rlid_map_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_correctable_interrupt")) { field_val = sch_rlid_map_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_uncorrectable_interrupt")) { field_val = sch_grp_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_correctable_interrupt")) { field_val = sch_grp_correctable_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_srams_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_dhs_sch_grp_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "clr_pend_cnt")) { field_val = clr_pend_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rl_thr")) { field_val = rl_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bckgr_cnt")) { field_val = bckgr_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bckgr")) { field_val = bckgr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_mode")) { field_val = hbm_mode(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_offset")) { field_val = qid_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "auto_clr")) { field_val = auto_clr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rr_sel")) { field_val = rr_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_vld")) { field_val = qid_fifo_elm0_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_jnk")) { field_val = qid_fifo_elm0_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_qid")) { field_val = qid_fifo_elm0_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_vld")) { field_val = qid_fifo_elm1_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_jnk")) { field_val = qid_fifo_elm1_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_qid")) { field_val = qid_fifo_elm1_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_vld")) { field_val = qid_fifo_elm2_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_jnk")) { field_val = qid_fifo_elm2_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_qid")) { field_val = qid_fifo_elm2_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_vld")) { field_val = qid_fifo_elm3_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_jnk")) { field_val = qid_fifo_elm3_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_qid")) { field_val = qid_fifo_elm3_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_vld")) { field_val = qid_fifo_elm4_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_jnk")) { field_val = qid_fifo_elm4_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_qid")) { field_val = qid_fifo_elm4_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_vld")) { field_val = qid_fifo_elm5_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_jnk")) { field_val = qid_fifo_elm5_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_qid")) { field_val = qid_fifo_elm5_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_vld")) { field_val = qid_fifo_elm6_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_jnk")) { field_val = qid_fifo_elm6_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_qid")) { field_val = qid_fifo_elm6_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_vld")) { field_val = qid_fifo_elm7_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_jnk")) { field_val = qid_fifo_elm7_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_qid")) { field_val = qid_fifo_elm7_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_vld")) { field_val = qid_fifo_elm8_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_jnk")) { field_val = qid_fifo_elm8_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_qid")) { field_val = qid_fifo_elm8_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_vld")) { field_val = qid_fifo_elm9_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_jnk")) { field_val = qid_fifo_elm9_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_qid")) { field_val = qid_fifo_elm9_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_vld")) { field_val = qid_fifo_elm10_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_jnk")) { field_val = qid_fifo_elm10_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_qid")) { field_val = qid_fifo_elm10_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_vld")) { field_val = qid_fifo_elm11_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_jnk")) { field_val = qid_fifo_elm11_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_qid")) { field_val = qid_fifo_elm11_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_vld")) { field_val = qid_fifo_elm12_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_jnk")) { field_val = qid_fifo_elm12_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_qid")) { field_val = qid_fifo_elm12_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_vld")) { field_val = qid_fifo_elm13_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_jnk")) { field_val = qid_fifo_elm13_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_qid")) { field_val = qid_fifo_elm13_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_vld")) { field_val = qid_fifo_elm14_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_jnk")) { field_val = qid_fifo_elm14_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_qid")) { field_val = qid_fifo_elm14_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_vld")) { field_val = qid_fifo_elm15_vld(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_jnk")) { field_val = qid_fifo_elm15_jnk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_qid")) { field_val = qid_fifo_elm15_qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln")) { field_val = hbm_ln(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_ptr")) { field_val = hbm_ln_ptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rr_cnt")) { field_val = hbm_rr_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_cnt")) { field_val = drb_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt0")) { field_val = hbm_ln_cnt0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt1")) { field_val = hbm_ln_cnt1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt2")) { field_val = hbm_ln_cnt2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt3")) { field_val = hbm_ln_cnt3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt4")) { field_val = hbm_ln_cnt4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt5")) { field_val = hbm_ln_cnt5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt6")) { field_val = hbm_ln_cnt6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt7")) { field_val = hbm_ln_cnt7(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt8")) { field_val = hbm_ln_cnt8(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt9")) { field_val = hbm_ln_cnt9(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt10")) { field_val = hbm_ln_cnt10(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt11")) { field_val = hbm_ln_cnt11(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt12")) { field_val = hbm_ln_cnt12(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt13")) { field_val = hbm_ln_cnt13(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt14")) { field_val = hbm_ln_cnt14(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt15")) { field_val = hbm_ln_cnt15(); field_found=1; }
    if(!field_found && !strcmp(field_name, "popcnt")) { field_val = popcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_start")) { field_val = sg_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_end")) { field_val = sg_end(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_rlid_map_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_lif_map_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_active")) { field_val = sg_active(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_start")) { field_val = sg_start(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_per_cos")) { field_val = sg_per_cos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_act_cos")) { field_val = sg_act_cos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_lif_map_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_tmr_cnt_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_cbcnt")) { field_val = slow_cbcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_bcnt")) { field_val = slow_bcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_lcnt")) { field_val = slow_lcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_cbcnt")) { field_val = fast_cbcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_bcnt")) { field_val = fast_bcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_lcnt")) { field_val = fast_lcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_tmr_cnt_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_rlid_stop_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "setmsk")) { field_val = setmsk(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_rlid_stop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "popcnt")) { field_val = popcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_cnt")) { field_val = drb_cnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_cnt_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_entry_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_offset")) { field_val = qid_offset(); field_found=1; }
    if(!field_found && !strcmp(field_name, "auto_clr")) { field_val = auto_clr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rr_sel")) { field_val = rr_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rl_thr")) { field_val = rl_thr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_doorbell_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos")) { field_val = cos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { field_val = set(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_doorbell_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_flags_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_flags")) { field_val = sch_flags(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_flags_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmhi_calendar_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_calendar")) { field_val = dtdm_calendar(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmhi_calendar_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmlo_calendar_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_calendar")) { field_val = dtdm_calendar(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmlo_calendar_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_pending_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { field_val = bcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lcnt")) { field_val = lcnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_pending_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "typ")) { field_val = typ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ring")) { field_val = ring(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtime")) { field_val = dtime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_start_no_stop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_pending_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { field_val = bcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lcnt")) { field_val = lcnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_pending_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "typ")) { field_val = typ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ring")) { field_val = ring(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtime")) { field_val = dtime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_start_no_stop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_uexp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_rerr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_wr_req_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_req_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_uexp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_rerr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_wr_req_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_req_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_stmr_max_bcnt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { field_val = bcnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_ftmr_max_bcnt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { field_val = bcnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_sch_hbm_byp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_sch_hbm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_hbm_byp_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_hbm_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_grp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_lif_map_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_tmr_cnt_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_dbg2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_byp_pending_cnt")) { field_val = hbm_byp_pending_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_cnt")) { field_val = hbm_rd_pending_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_cnt")) { field_val = hbm_wr_pending_cnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "txdma_drdy")) { field_val = txdma_drdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_re_efull")) { field_val = hbm_re_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_we_efull")) { field_val = hbm_we_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_msg_efull")) { field_val = txdma_msg_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_byp_pending_efull")) { field_val = hbm_byp_pending_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_efull")) { field_val = hbm_rd_pending_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_efull")) { field_val = hbm_wr_pending_efull(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_lif_sg_mismatch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { field_val = set(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_lif_map_notactive_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { field_val = lif(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { field_val = qid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos")) { field_val = cos(); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { field_val = set(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_grp_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { field_val = dhs_eccbypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_rlid_map_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { field_val = dhs_eccbypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_lif_map_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { field_val = dhs_eccbypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_cnt_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { field_val = dhs_eccbypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_max_hbm_byp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { field_val = cnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_scheduler_dbg2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "qid_read")) { field_val = qid_read(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos_read")) { field_val = cos_read(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_scheduler_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_efc_thr")) { field_val = hbm_efc_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_efc_thr")) { field_val = txdma_efc_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_mode_thr")) { field_val = fifo_mode_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_byp")) { field_val = max_hbm_byp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_wr")) { field_val = max_hbm_wr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_rd")) { field_val = max_hbm_rd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_rr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "curr_ptr0")) { field_val = curr_ptr0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr1")) { field_val = curr_ptr1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr2")) { field_val = curr_ptr2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr3")) { field_val = curr_ptr3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr4")) { field_val = curr_ptr4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr5")) { field_val = curr_ptr5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr6")) { field_val = curr_ptr6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr7")) { field_val = curr_ptr7(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr8")) { field_val = curr_ptr8(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr9")) { field_val = curr_ptr9(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr10")) { field_val = curr_ptr10(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr11")) { field_val = curr_ptr11(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr12")) { field_val = curr_ptr12(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr13")) { field_val = curr_ptr13(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr14")) { field_val = curr_ptr14(); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr15")) { field_val = curr_ptr15(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_init_done")) { field_val = hbm_init_done(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_init_done")) { field_val = sram_init_done(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_rlid_start_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_rlid_stop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_glb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pb_xoff")) { field_val = pb_xoff(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos15_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos14_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos13_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos12_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos11_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos10_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos9_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos8_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos7_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos6_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos5_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos4_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos3_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_fifo_empty_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_doorbell_clr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_doorbell_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_lo_map")) { field_val = dtdm_lo_map(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtdm_hi_map")) { field_val = dtdm_hi_map(); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { field_val = timeout(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pause")) { field_val = pause(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_scheduler_static_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_base")) { field_val = hbm_base(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_depth")) { field_val = sch_grp_depth(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_scheduler_glb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_hw_init")) { field_val = hbm_hw_init(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_hw_init")) { field_val = sram_hw_init(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_set_lkup")) { field_val = enable_set_lkup(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_set_byp")) { field_val = enable_set_byp(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_hbm_byp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_hbm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_hbm_byp_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_hbm_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_pop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_key_not_found_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_push_out_of_wheel_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_key_not_push_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_push_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_slow_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { field_val = tick(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cTime")) { field_val = cTime(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pTime")) { field_val = pTime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_slow_timer_dbell_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_update")) { field_val = addr_update(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_pid")) { field_val = data_pid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_reserved")) { field_val = data_reserved(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_index")) { field_val = data_index(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_force_slow_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ctime")) { field_val = ctime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_slow_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { field_val = tick(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { field_val = hash_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_pop_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_key_not_found_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_key_not_push_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_push_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { field_val = val(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_fast_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { field_val = tick(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cTime")) { field_val = cTime(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pTime")) { field_val = pTime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_fast_timer_dbell_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_update")) { field_val = addr_update(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_pid")) { field_val = data_pid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_reserved")) { field_val = data_reserved(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_index")) { field_val = data_index(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_force_fast_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ctime")) { field_val = ctime(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_fast_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { field_val = tick(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { field_val = hash_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_dbg2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_byp_pending_cnt")) { field_val = hbm_byp_pending_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_cnt")) { field_val = hbm_rd_pending_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_cnt")) { field_val = hbm_wr_pending_cnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall")) { field_val = stmr_stall(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall")) { field_val = ftmr_stall(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_re_efull")) { field_val = hbm_re_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_we_efull")) { field_val = hbm_we_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rejct_drb_efull")) { field_val = rejct_drb_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_byp_pending_efull")) { field_val = hbm_byp_pending_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_efull")) { field_val = hbm_rd_pending_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_efull")) { field_val = hbm_wr_pending_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_raddr")) { field_val = stmr_fifo_raddr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_raddr")) { field_val = ftmr_fifo_raddr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_waddr")) { field_val = stmr_fifo_waddr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_waddr")) { field_val = ftmr_fifo_waddr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull")) { field_val = stmr_fifo_efull(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull")) { field_val = ftmr_fifo_efull(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_tmr_max_keys_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { field_val = cnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_tmr_max_hbm_byp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { field_val = cnt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_init_done")) { field_val = hbm_init_done(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_init_done")) { field_val = sram_init_done(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_dbg2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "max_bcnt")) { field_val = max_bcnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_efc_thr")) { field_val = hbm_efc_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_efc_thr")) { field_val = drb_efc_thr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_stall_thr_hi")) { field_val = tmr_stall_thr_hi(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_stall_thr_lo")) { field_val = tmr_stall_thr_lo(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_wr")) { field_val = max_hbm_wr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_rd")) { field_val = max_hbm_rd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_byp")) { field_val = max_hbm_byp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_tmr_fifo")) { field_val = max_tmr_fifo(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_timer_glb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_hw_init")) { field_val = hbm_hw_init(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_hw_init")) { field_val = sram_hw_init(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_enable")) { field_val = ftmr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_enable")) { field_val = stmr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_pause")) { field_val = ftmr_pause(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_pause")) { field_val = stmr_pause(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_static_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_base")) { field_val = hbm_base(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hsh_depth")) { field_val = tmr_hsh_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wheel_depth")) { field_val = tmr_wheel_depth(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_axi_attr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_cfg_glb_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dbg_port_select")) { field_val = dbg_port_select(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dbg_port_enable")) { field_val = dbg_port_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_glb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_axi_attr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_static.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_timer_glb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_dbg2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tmr_max_hbm_byp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tmr_max_keys.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer_dbg2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fast_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_force_fast_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fast_timer_dbell.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_fast_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_push.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_key_not_push.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_push_out_of_wheel.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_key_not_found.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_pop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_slow_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_force_slow_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_slow_timer_dbell.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_slow_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_push.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_key_not_push.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_push_out_of_wheel.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_key_not_found.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_pop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_hbm_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_hbm_byp_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_hbm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_hbm_byp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_scheduler_glb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_scheduler_static.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_doorbell_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_doorbell_clr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_fifo_empty.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos3.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos4.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos5.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos6.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos7.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos8.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos9.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos10.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos11.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos12.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos13.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos14.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos15.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_glb.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_rlid_stop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_rlid_start.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_rr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_scheduler_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_scheduler_dbg2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_max_hbm_byp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_cnt_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_lif_map_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_rlid_map_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_grp_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_lif_map_notactive.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_lif_sg_mismatch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_dbg2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_tmr_cnt.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_lif_map.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_rlid_map.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_grp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_hbm_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_hbm_byp_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_sch_hbm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_sch_hbm_byp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ftmr_max_bcnt.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stmr_max_bcnt.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_req.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_rerr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_uexp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_wr_req.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_rerr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_uexp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_req.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_rerr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_uexp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_wr_req.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_rerr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_uexp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_fast_timer_start_no_stop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_fast_timer_pending.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_slow_timer_start_no_stop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_slow_timer_pending.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dtdmlo_calendar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dtdmhi_calendar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_flags.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_doorbell.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_entry.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_cnt_entry.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_rlid_stop.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tmr_cnt_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_lif_map_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_rlid_map_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_srams_ecc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_sch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_tmr.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_wid_empty_enable")) { tmr_wid_empty_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_empty_enable")) { tmr_rid_empty_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wr_txfifo_ovf_enable")) { tmr_wr_txfifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rd_txfifo_ovf_enable")) { tmr_rd_txfifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_state_fifo_ovf_enable")) { tmr_state_fifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_ovf_enable")) { tmr_hbm_byp_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_wtag_wrap_enable")) { tmr_hbm_byp_wtag_wrap_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_ctime_wrap_enable")) { ftmr_ctime_wrap_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_ctime_wrap_enable")) { stmr_ctime_wrap_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_push_out_of_wheel_enable")) { stmr_push_out_of_wheel_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_push_out_of_wheel_enable")) { ftmr_push_out_of_wheel_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_push_enable")) { ftmr_key_not_push_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_found_enable")) { ftmr_key_not_found_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_push_enable")) { stmr_key_not_push_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_found_enable")) { stmr_key_not_found_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall_enable")) { stmr_stall_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall_enable")) { ftmr_stall_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_ovf_enable")) { ftmr_fifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_ovf_enable")) { stmr_fifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull_enable")) { ftmr_fifo_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull_enable")) { stmr_fifo_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_ovf_enable")) { tmr_rejct_drb_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_efull_enable")) { tmr_rejct_drb_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_re_efull_enable")) { tmr_hbm_re_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_we_efull_enable")) { tmr_hbm_we_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_pending_efull_enable")) { tmr_hbm_byp_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_rd_pending_efull_enable")) { tmr_hbm_rd_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_wr_pending_efull_enable")) { tmr_hbm_wr_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bresp_err_enable")) { tmr_bresp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bid_err_enable")) { tmr_bid_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rresp_err_enable")) { tmr_rresp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_err_enable")) { tmr_rid_err_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_int_test_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_wid_empty_interrupt")) { tmr_wid_empty_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_empty_interrupt")) { tmr_rid_empty_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wr_txfifo_ovf_interrupt")) { tmr_wr_txfifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rd_txfifo_ovf_interrupt")) { tmr_rd_txfifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_state_fifo_ovf_interrupt")) { tmr_state_fifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_ovf_interrupt")) { tmr_hbm_byp_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_wtag_wrap_interrupt")) { tmr_hbm_byp_wtag_wrap_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_ctime_wrap_interrupt")) { ftmr_ctime_wrap_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_ctime_wrap_interrupt")) { stmr_ctime_wrap_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_push_out_of_wheel_interrupt")) { stmr_push_out_of_wheel_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_push_out_of_wheel_interrupt")) { ftmr_push_out_of_wheel_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_push_interrupt")) { ftmr_key_not_push_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_key_not_found_interrupt")) { ftmr_key_not_found_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_push_interrupt")) { stmr_key_not_push_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_key_not_found_interrupt")) { stmr_key_not_found_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall_interrupt")) { stmr_stall_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall_interrupt")) { ftmr_stall_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_ovf_interrupt")) { ftmr_fifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_ovf_interrupt")) { stmr_fifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull_interrupt")) { ftmr_fifo_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull_interrupt")) { stmr_fifo_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_ovf_interrupt")) { tmr_rejct_drb_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rejct_drb_efull_interrupt")) { tmr_rejct_drb_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_re_efull_interrupt")) { tmr_hbm_re_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_we_efull_interrupt")) { tmr_hbm_we_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_byp_pending_efull_interrupt")) { tmr_hbm_byp_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_rd_pending_efull_interrupt")) { tmr_hbm_rd_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hbm_wr_pending_efull_interrupt")) { tmr_hbm_wr_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bresp_err_interrupt")) { tmr_bresp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_bid_err_interrupt")) { tmr_bid_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rresp_err_interrupt")) { tmr_rresp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_rid_err_interrupt")) { tmr_rid_err_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_tmr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_int_sch_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_wid_empty_enable")) { sch_wid_empty_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_empty_enable")) { sch_rid_empty_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_wr_txfifo_ovf_enable")) { sch_wr_txfifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rd_txfifo_ovf_enable")) { sch_rd_txfifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_state_fifo_ovf_enable")) { sch_state_fifo_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_qid_fifo_enable")) { sch_drb_cnt_qid_fifo_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_ovf_enable")) { sch_hbm_byp_ovf_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_wtag_wrap_enable")) { sch_hbm_byp_wtag_wrap_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_ovfl_enable")) { sch_rlid_ovfl_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_unfl_enable")) { sch_rlid_unfl_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_null_lif_enable")) { sch_null_lif_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_sg_mismatch_enable")) { sch_lif_sg_mismatch_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_aclr_hbm_ln_rollovr_enable")) { sch_aclr_hbm_ln_rollovr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_efull_enable")) { sch_txdma_msg_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_re_efull_enable")) { sch_hbm_re_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_we_efull_enable")) { sch_hbm_we_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_pending_efull_enable")) { sch_hbm_byp_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_rd_pending_efull_enable")) { sch_hbm_rd_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_wr_pending_efull_enable")) { sch_hbm_wr_pending_efull_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bresp_err_enable")) { sch_bresp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bid_err_enable")) { sch_bid_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rresp_err_enable")) { sch_rresp_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_err_enable")) { sch_rid_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_ovfl_enable")) { sch_drb_cnt_ovfl_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_unfl_enable")) { sch_drb_cnt_unfl_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_ovfl_enable")) { sch_txdma_msg_ovfl_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_sch_int_test_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_wid_empty_interrupt")) { sch_wid_empty_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_empty_interrupt")) { sch_rid_empty_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_wr_txfifo_ovf_interrupt")) { sch_wr_txfifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rd_txfifo_ovf_interrupt")) { sch_rd_txfifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_state_fifo_ovf_interrupt")) { sch_state_fifo_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_qid_fifo_interrupt")) { sch_drb_cnt_qid_fifo_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_ovf_interrupt")) { sch_hbm_byp_ovf_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_wtag_wrap_interrupt")) { sch_hbm_byp_wtag_wrap_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_ovfl_interrupt")) { sch_rlid_ovfl_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_unfl_interrupt")) { sch_rlid_unfl_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_null_lif_interrupt")) { sch_null_lif_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_sg_mismatch_interrupt")) { sch_lif_sg_mismatch_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_aclr_hbm_ln_rollovr_interrupt")) { sch_aclr_hbm_ln_rollovr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_efull_interrupt")) { sch_txdma_msg_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_re_efull_interrupt")) { sch_hbm_re_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_we_efull_interrupt")) { sch_hbm_we_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_byp_pending_efull_interrupt")) { sch_hbm_byp_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_rd_pending_efull_interrupt")) { sch_hbm_rd_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_hbm_wr_pending_efull_interrupt")) { sch_hbm_wr_pending_efull_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bresp_err_interrupt")) { sch_bresp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_bid_err_interrupt")) { sch_bid_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rresp_err_interrupt")) { sch_rresp_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rid_err_interrupt")) { sch_rid_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_ovfl_interrupt")) { sch_drb_cnt_ovfl_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_drb_cnt_unfl_interrupt")) { sch_drb_cnt_unfl_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_txdma_msg_ovfl_interrupt")) { sch_txdma_msg_ovfl_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_sch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_sch_interrupt")) { int_sch_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_tmr_interrupt")) { int_tmr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_srams_ecc_interrupt")) { int_srams_ecc_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_sch_enable")) { int_sch_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_tmr_enable")) { int_tmr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_srams_ecc_enable")) { int_srams_ecc_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_int_srams_ecc_int_enable_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_cnt_uncorrectable_enable")) { tmr_cnt_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_cnt_correctable_enable")) { tmr_cnt_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_uncorrectable_enable")) { sch_lif_map_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_correctable_enable")) { sch_lif_map_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_uncorrectable_enable")) { sch_rlid_map_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_correctable_enable")) { sch_rlid_map_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_uncorrectable_enable")) { sch_grp_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_correctable_enable")) { sch_grp_correctable_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_srams_ecc_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tmr_cnt_uncorrectable_interrupt")) { tmr_cnt_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_cnt_correctable_interrupt")) { tmr_cnt_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_uncorrectable_interrupt")) { sch_lif_map_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_lif_map_correctable_interrupt")) { sch_lif_map_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_uncorrectable_interrupt")) { sch_rlid_map_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_rlid_map_correctable_interrupt")) { sch_rlid_map_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_uncorrectable_interrupt")) { sch_grp_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_correctable_interrupt")) { sch_grp_correctable_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_int_srams_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_dhs_sch_grp_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "clr_pend_cnt")) { clr_pend_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rl_thr")) { rl_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bckgr_cnt")) { bckgr_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bckgr")) { bckgr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_mode")) { hbm_mode(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_offset")) { qid_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "auto_clr")) { auto_clr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rr_sel")) { rr_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_vld")) { qid_fifo_elm0_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_jnk")) { qid_fifo_elm0_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm0_qid")) { qid_fifo_elm0_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_vld")) { qid_fifo_elm1_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_jnk")) { qid_fifo_elm1_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm1_qid")) { qid_fifo_elm1_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_vld")) { qid_fifo_elm2_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_jnk")) { qid_fifo_elm2_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm2_qid")) { qid_fifo_elm2_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_vld")) { qid_fifo_elm3_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_jnk")) { qid_fifo_elm3_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm3_qid")) { qid_fifo_elm3_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_vld")) { qid_fifo_elm4_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_jnk")) { qid_fifo_elm4_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm4_qid")) { qid_fifo_elm4_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_vld")) { qid_fifo_elm5_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_jnk")) { qid_fifo_elm5_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm5_qid")) { qid_fifo_elm5_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_vld")) { qid_fifo_elm6_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_jnk")) { qid_fifo_elm6_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm6_qid")) { qid_fifo_elm6_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_vld")) { qid_fifo_elm7_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_jnk")) { qid_fifo_elm7_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm7_qid")) { qid_fifo_elm7_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_vld")) { qid_fifo_elm8_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_jnk")) { qid_fifo_elm8_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm8_qid")) { qid_fifo_elm8_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_vld")) { qid_fifo_elm9_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_jnk")) { qid_fifo_elm9_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm9_qid")) { qid_fifo_elm9_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_vld")) { qid_fifo_elm10_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_jnk")) { qid_fifo_elm10_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm10_qid")) { qid_fifo_elm10_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_vld")) { qid_fifo_elm11_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_jnk")) { qid_fifo_elm11_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm11_qid")) { qid_fifo_elm11_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_vld")) { qid_fifo_elm12_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_jnk")) { qid_fifo_elm12_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm12_qid")) { qid_fifo_elm12_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_vld")) { qid_fifo_elm13_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_jnk")) { qid_fifo_elm13_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm13_qid")) { qid_fifo_elm13_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_vld")) { qid_fifo_elm14_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_jnk")) { qid_fifo_elm14_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm14_qid")) { qid_fifo_elm14_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_vld")) { qid_fifo_elm15_vld(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_jnk")) { qid_fifo_elm15_jnk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_fifo_elm15_qid")) { qid_fifo_elm15_qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln")) { hbm_ln(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_ptr")) { hbm_ln_ptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rr_cnt")) { hbm_rr_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_cnt")) { drb_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt0")) { hbm_ln_cnt0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt1")) { hbm_ln_cnt1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt2")) { hbm_ln_cnt2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt3")) { hbm_ln_cnt3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt4")) { hbm_ln_cnt4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt5")) { hbm_ln_cnt5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt6")) { hbm_ln_cnt6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt7")) { hbm_ln_cnt7(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt8")) { hbm_ln_cnt8(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt9")) { hbm_ln_cnt9(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt10")) { hbm_ln_cnt10(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt11")) { hbm_ln_cnt11(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt12")) { hbm_ln_cnt12(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt13")) { hbm_ln_cnt13(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt14")) { hbm_ln_cnt14(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_ln_cnt15")) { hbm_ln_cnt15(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "popcnt")) { popcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_start")) { sg_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_end")) { sg_end(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_rlid_map_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_lif_map_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_active")) { sg_active(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_start")) { sg_start(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_per_cos")) { sg_per_cos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sg_act_cos")) { sg_act_cos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_lif_map_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_tmr_cnt_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_cbcnt")) { slow_cbcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_bcnt")) { slow_bcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "slow_lcnt")) { slow_lcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_cbcnt")) { fast_cbcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_bcnt")) { fast_bcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fast_lcnt")) { fast_lcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_tmr_cnt_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_rlid_stop_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "setmsk")) { setmsk(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_rlid_stop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "popcnt")) { popcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_cnt")) { drb_cnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_cnt_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_entry_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid_offset")) { qid_offset(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "auto_clr")) { auto_clr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rr_sel")) { rr_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rl_thr")) { rl_thr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_grp_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_doorbell_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos")) { cos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { set(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_doorbell_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_flags_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sch_flags")) { sch_flags(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_sch_flags_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmhi_calendar_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_calendar")) { dtdm_calendar(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmhi_calendar_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmlo_calendar_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_calendar")) { dtdm_calendar(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_dtdmlo_calendar_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_pending_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { bcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lcnt")) { lcnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_pending_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "typ")) { typ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ring")) { ring(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtime")) { dtime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_slow_timer_start_no_stop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_pending_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { bcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lcnt")) { lcnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_pending_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "typ")) { typ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ring")) { ring(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtime")) { dtime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_dhs_fast_timer_start_no_stop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_uexp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_rerr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_bid_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_wr_req_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_rsp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_tmr_axi_rd_req_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_uexp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_rerr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_bid_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_wr_req_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_rsp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_axi_rd_req_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_stmr_max_bcnt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { bcnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_ftmr_max_bcnt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bcnt")) { bcnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_sch_hbm_byp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_sch_hbm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_hbm_byp_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_hbm_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_grp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_sch_lif_map_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_ecc_tmr_cnt_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_dbg2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_byp_pending_cnt")) { hbm_byp_pending_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_cnt")) { hbm_rd_pending_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_cnt")) { hbm_wr_pending_cnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "txdma_drdy")) { txdma_drdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_re_efull")) { hbm_re_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_we_efull")) { hbm_we_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_msg_efull")) { txdma_msg_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_byp_pending_efull")) { hbm_byp_pending_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_efull")) { hbm_rd_pending_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_efull")) { hbm_wr_pending_efull(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_lif_sg_mismatch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { set(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_lif_map_notactive_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "lif")) { lif(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qid")) { qid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos")) { cos(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "set")) { set(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_grp_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { dhs_eccbypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_rlid_map_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { dhs_eccbypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_lif_map_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { dhs_eccbypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_cnt_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dhs_eccbypass")) { dhs_eccbypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_sch_max_hbm_byp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { cnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_scheduler_dbg2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "qid_read")) { qid_read(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cos_read")) { cos_read(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_scheduler_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_efc_thr")) { hbm_efc_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "txdma_efc_thr")) { txdma_efc_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_mode_thr")) { fifo_mode_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_byp")) { max_hbm_byp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_wr")) { max_hbm_wr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_rd")) { max_hbm_rd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_rr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "curr_ptr0")) { curr_ptr0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr1")) { curr_ptr1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr2")) { curr_ptr2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr3")) { curr_ptr3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr4")) { curr_ptr4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr5")) { curr_ptr5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr6")) { curr_ptr6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr7")) { curr_ptr7(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr8")) { curr_ptr8(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr9")) { curr_ptr9(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr10")) { curr_ptr10(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr11")) { curr_ptr11(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr12")) { curr_ptr12(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr13")) { curr_ptr13(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr14")) { curr_ptr14(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "curr_ptr15")) { curr_ptr15(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_scheduler_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_init_done")) { hbm_init_done(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_init_done")) { sram_init_done(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_rlid_start_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_rlid_stop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_glb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "pb_xoff")) { pb_xoff(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos15_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos14_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos13_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos12_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos11_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos10_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos9_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos8_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos7_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos6_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos5_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos4_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos3_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_txdma_cos0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_fifo_empty_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_doorbell_clr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_sch_doorbell_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_sch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dtdm_lo_map")) { dtdm_lo_map(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dtdm_hi_map")) { dtdm_hi_map(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "timeout")) { timeout(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pause")) { pause(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_scheduler_static_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_base")) { hbm_base(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sch_grp_depth")) { sch_grp_depth(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_scheduler_glb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_hw_init")) { hbm_hw_init(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_hw_init")) { sram_hw_init(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_set_lkup")) { enable_set_lkup(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_set_byp")) { enable_set_byp(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_hbm_byp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_srams_tmr_hbm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_hbm_byp_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_tmr_hbm_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_pop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_key_not_found_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_push_out_of_wheel_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_key_not_push_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_stmr_push_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_slow_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { tick(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cTime")) { cTime(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pTime")) { pTime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_slow_timer_dbell_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_update")) { addr_update(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_pid")) { data_pid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_reserved")) { data_reserved(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_index")) { data_index(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_force_slow_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ctime")) { ctime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_slow_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { tick(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { hash_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_pop_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_key_not_found_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_key_not_push_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cnt_ftmr_push_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "val")) { val(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_fast_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { tick(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cTime")) { cTime(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pTime")) { pTime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_fast_timer_dbell_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_update")) { addr_update(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_pid")) { data_pid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_reserved")) { data_reserved(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data_index")) { data_index(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_force_fast_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ctime")) { ctime(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_fast_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tick")) { tick(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hash_sel")) { hash_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_dbg2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_byp_pending_cnt")) { hbm_byp_pending_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_cnt")) { hbm_rd_pending_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_cnt")) { hbm_wr_pending_cnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_stall")) { stmr_stall(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_stall")) { ftmr_stall(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_re_efull")) { hbm_re_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_we_efull")) { hbm_we_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rejct_drb_efull")) { rejct_drb_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_byp_pending_efull")) { hbm_byp_pending_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_rd_pending_efull")) { hbm_rd_pending_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "hbm_wr_pending_efull")) { hbm_wr_pending_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_raddr")) { stmr_fifo_raddr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_raddr")) { ftmr_fifo_raddr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_waddr")) { stmr_fifo_waddr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_waddr")) { ftmr_fifo_waddr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_fifo_efull")) { stmr_fifo_efull(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_fifo_efull")) { ftmr_fifo_efull(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_tmr_max_keys_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { cnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_tmr_max_hbm_byp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cnt")) { cnt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_sta_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_init_done")) { hbm_init_done(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_init_done")) { sram_init_done(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_dbg2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "max_bcnt")) { max_bcnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_efc_thr")) { hbm_efc_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drb_efc_thr")) { drb_efc_thr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_stall_thr_hi")) { tmr_stall_thr_hi(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_stall_thr_lo")) { tmr_stall_thr_lo(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_wr")) { max_hbm_wr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_rd")) { max_hbm_rd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_hbm_byp")) { max_hbm_byp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_tmr_fifo")) { max_tmr_fifo(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfw_timer_glb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_hw_init")) { hbm_hw_init(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram_hw_init")) { sram_hw_init(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_enable")) { ftmr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_enable")) { stmr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ftmr_pause")) { ftmr_pause(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stmr_pause")) { stmr_pause(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_timer_static_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "hbm_base")) { hbm_base(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_hsh_depth")) { tmr_hsh_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tmr_wheel_depth")) { tmr_wheel_depth(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_cfg_axi_attr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_txs_csr_cfg_glb_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dbg_port_select")) { dbg_port_select(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dbg_port_enable")) { dbg_port_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_txs_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_glb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_axi_attr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_static.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_timer_glb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_timer_dbg2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tmr_max_hbm_byp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_tmr_max_keys.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer_dbg2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fast_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_force_fast_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_fast_timer_dbell.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_fast_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_push.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_key_not_push.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_push_out_of_wheel.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_key_not_found.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_ftmr_pop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_slow_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_force_slow_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_slow_timer_dbell.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_slow_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_push.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_key_not_push.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_push_out_of_wheel.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_key_not_found.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_stmr_pop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_hbm_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_hbm_byp_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_hbm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_hbm_byp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_tmr_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_scheduler_glb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_scheduler_static.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_doorbell_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_doorbell_clr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_fifo_empty.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos3.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos4.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos5.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos6.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos7.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos8.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos9.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos10.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos11.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos12.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos13.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos14.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_txdma_cos15.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_glb.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_rlid_stop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_rlid_start.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_rr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_scheduler_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_scheduler_dbg2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_max_hbm_byp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_tmr_cnt_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_lif_map_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_rlid_map_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_grp_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_lif_map_notactive.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sch_lif_sg_mismatch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_scheduler_dbg2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_tmr_cnt.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_lif_map.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_rlid_map.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_sch_grp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_hbm_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sch_hbm_byp_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_sch_hbm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_sch_hbm_byp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ftmr_max_bcnt.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_stmr_max_bcnt.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_req.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_rerr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_rd_rsp_uexp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_wr_req.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_rerr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_sch_axi_bid_uexp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_req.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_rerr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_rd_rsp_uexp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_wr_req.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_rerr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cnt_tmr_axi_bid_uexp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_fast_timer_start_no_stop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_fast_timer_pending.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_slow_timer_start_no_stop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_slow_timer_pending.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dtdmlo_calendar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dtdmhi_calendar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_flags.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_doorbell.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_entry.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_cnt_entry.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_rlid_stop.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_tmr_cnt_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_lif_map_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_rlid_map_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_sch_grp_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_srams_ecc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_sch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_tmr.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_tmr_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tmr_wid_empty_enable");
    ret_vec.push_back("tmr_rid_empty_enable");
    ret_vec.push_back("tmr_wr_txfifo_ovf_enable");
    ret_vec.push_back("tmr_rd_txfifo_ovf_enable");
    ret_vec.push_back("tmr_state_fifo_ovf_enable");
    ret_vec.push_back("tmr_hbm_byp_ovf_enable");
    ret_vec.push_back("tmr_hbm_byp_wtag_wrap_enable");
    ret_vec.push_back("ftmr_ctime_wrap_enable");
    ret_vec.push_back("stmr_ctime_wrap_enable");
    ret_vec.push_back("stmr_push_out_of_wheel_enable");
    ret_vec.push_back("ftmr_push_out_of_wheel_enable");
    ret_vec.push_back("ftmr_key_not_push_enable");
    ret_vec.push_back("ftmr_key_not_found_enable");
    ret_vec.push_back("stmr_key_not_push_enable");
    ret_vec.push_back("stmr_key_not_found_enable");
    ret_vec.push_back("stmr_stall_enable");
    ret_vec.push_back("ftmr_stall_enable");
    ret_vec.push_back("ftmr_fifo_ovf_enable");
    ret_vec.push_back("stmr_fifo_ovf_enable");
    ret_vec.push_back("ftmr_fifo_efull_enable");
    ret_vec.push_back("stmr_fifo_efull_enable");
    ret_vec.push_back("tmr_rejct_drb_ovf_enable");
    ret_vec.push_back("tmr_rejct_drb_efull_enable");
    ret_vec.push_back("tmr_hbm_re_efull_enable");
    ret_vec.push_back("tmr_hbm_we_efull_enable");
    ret_vec.push_back("tmr_hbm_byp_pending_efull_enable");
    ret_vec.push_back("tmr_hbm_rd_pending_efull_enable");
    ret_vec.push_back("tmr_hbm_wr_pending_efull_enable");
    ret_vec.push_back("tmr_bresp_err_enable");
    ret_vec.push_back("tmr_bid_err_enable");
    ret_vec.push_back("tmr_rresp_err_enable");
    ret_vec.push_back("tmr_rid_err_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_tmr_int_test_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tmr_wid_empty_interrupt");
    ret_vec.push_back("tmr_rid_empty_interrupt");
    ret_vec.push_back("tmr_wr_txfifo_ovf_interrupt");
    ret_vec.push_back("tmr_rd_txfifo_ovf_interrupt");
    ret_vec.push_back("tmr_state_fifo_ovf_interrupt");
    ret_vec.push_back("tmr_hbm_byp_ovf_interrupt");
    ret_vec.push_back("tmr_hbm_byp_wtag_wrap_interrupt");
    ret_vec.push_back("ftmr_ctime_wrap_interrupt");
    ret_vec.push_back("stmr_ctime_wrap_interrupt");
    ret_vec.push_back("stmr_push_out_of_wheel_interrupt");
    ret_vec.push_back("ftmr_push_out_of_wheel_interrupt");
    ret_vec.push_back("ftmr_key_not_push_interrupt");
    ret_vec.push_back("ftmr_key_not_found_interrupt");
    ret_vec.push_back("stmr_key_not_push_interrupt");
    ret_vec.push_back("stmr_key_not_found_interrupt");
    ret_vec.push_back("stmr_stall_interrupt");
    ret_vec.push_back("ftmr_stall_interrupt");
    ret_vec.push_back("ftmr_fifo_ovf_interrupt");
    ret_vec.push_back("stmr_fifo_ovf_interrupt");
    ret_vec.push_back("ftmr_fifo_efull_interrupt");
    ret_vec.push_back("stmr_fifo_efull_interrupt");
    ret_vec.push_back("tmr_rejct_drb_ovf_interrupt");
    ret_vec.push_back("tmr_rejct_drb_efull_interrupt");
    ret_vec.push_back("tmr_hbm_re_efull_interrupt");
    ret_vec.push_back("tmr_hbm_we_efull_interrupt");
    ret_vec.push_back("tmr_hbm_byp_pending_efull_interrupt");
    ret_vec.push_back("tmr_hbm_rd_pending_efull_interrupt");
    ret_vec.push_back("tmr_hbm_wr_pending_efull_interrupt");
    ret_vec.push_back("tmr_bresp_err_interrupt");
    ret_vec.push_back("tmr_bid_err_interrupt");
    ret_vec.push_back("tmr_rresp_err_interrupt");
    ret_vec.push_back("tmr_rid_err_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_tmr_t::get_fields(int level) const { 
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
std::vector<string> cap_txs_csr_int_sch_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sch_wid_empty_enable");
    ret_vec.push_back("sch_rid_empty_enable");
    ret_vec.push_back("sch_wr_txfifo_ovf_enable");
    ret_vec.push_back("sch_rd_txfifo_ovf_enable");
    ret_vec.push_back("sch_state_fifo_ovf_enable");
    ret_vec.push_back("sch_drb_cnt_qid_fifo_enable");
    ret_vec.push_back("sch_hbm_byp_ovf_enable");
    ret_vec.push_back("sch_hbm_byp_wtag_wrap_enable");
    ret_vec.push_back("sch_rlid_ovfl_enable");
    ret_vec.push_back("sch_rlid_unfl_enable");
    ret_vec.push_back("sch_null_lif_enable");
    ret_vec.push_back("sch_lif_sg_mismatch_enable");
    ret_vec.push_back("sch_aclr_hbm_ln_rollovr_enable");
    ret_vec.push_back("sch_txdma_msg_efull_enable");
    ret_vec.push_back("sch_hbm_re_efull_enable");
    ret_vec.push_back("sch_hbm_we_efull_enable");
    ret_vec.push_back("sch_hbm_byp_pending_efull_enable");
    ret_vec.push_back("sch_hbm_rd_pending_efull_enable");
    ret_vec.push_back("sch_hbm_wr_pending_efull_enable");
    ret_vec.push_back("sch_bresp_err_enable");
    ret_vec.push_back("sch_bid_err_enable");
    ret_vec.push_back("sch_rresp_err_enable");
    ret_vec.push_back("sch_rid_err_enable");
    ret_vec.push_back("sch_drb_cnt_ovfl_enable");
    ret_vec.push_back("sch_drb_cnt_unfl_enable");
    ret_vec.push_back("sch_txdma_msg_ovfl_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_sch_int_test_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sch_wid_empty_interrupt");
    ret_vec.push_back("sch_rid_empty_interrupt");
    ret_vec.push_back("sch_wr_txfifo_ovf_interrupt");
    ret_vec.push_back("sch_rd_txfifo_ovf_interrupt");
    ret_vec.push_back("sch_state_fifo_ovf_interrupt");
    ret_vec.push_back("sch_drb_cnt_qid_fifo_interrupt");
    ret_vec.push_back("sch_hbm_byp_ovf_interrupt");
    ret_vec.push_back("sch_hbm_byp_wtag_wrap_interrupt");
    ret_vec.push_back("sch_rlid_ovfl_interrupt");
    ret_vec.push_back("sch_rlid_unfl_interrupt");
    ret_vec.push_back("sch_null_lif_interrupt");
    ret_vec.push_back("sch_lif_sg_mismatch_interrupt");
    ret_vec.push_back("sch_aclr_hbm_ln_rollovr_interrupt");
    ret_vec.push_back("sch_txdma_msg_efull_interrupt");
    ret_vec.push_back("sch_hbm_re_efull_interrupt");
    ret_vec.push_back("sch_hbm_we_efull_interrupt");
    ret_vec.push_back("sch_hbm_byp_pending_efull_interrupt");
    ret_vec.push_back("sch_hbm_rd_pending_efull_interrupt");
    ret_vec.push_back("sch_hbm_wr_pending_efull_interrupt");
    ret_vec.push_back("sch_bresp_err_interrupt");
    ret_vec.push_back("sch_bid_err_interrupt");
    ret_vec.push_back("sch_rresp_err_interrupt");
    ret_vec.push_back("sch_rid_err_interrupt");
    ret_vec.push_back("sch_drb_cnt_ovfl_interrupt");
    ret_vec.push_back("sch_drb_cnt_unfl_interrupt");
    ret_vec.push_back("sch_txdma_msg_ovfl_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_sch_t::get_fields(int level) const { 
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
std::vector<string> cap_txs_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_sch_interrupt");
    ret_vec.push_back("int_tmr_interrupt");
    ret_vec.push_back("int_srams_ecc_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_sch_enable");
    ret_vec.push_back("int_tmr_enable");
    ret_vec.push_back("int_srams_ecc_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_txs_csr_int_srams_ecc_int_enable_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tmr_cnt_uncorrectable_enable");
    ret_vec.push_back("tmr_cnt_correctable_enable");
    ret_vec.push_back("sch_lif_map_uncorrectable_enable");
    ret_vec.push_back("sch_lif_map_correctable_enable");
    ret_vec.push_back("sch_rlid_map_uncorrectable_enable");
    ret_vec.push_back("sch_rlid_map_correctable_enable");
    ret_vec.push_back("sch_grp_uncorrectable_enable");
    ret_vec.push_back("sch_grp_correctable_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_srams_ecc_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tmr_cnt_uncorrectable_interrupt");
    ret_vec.push_back("tmr_cnt_correctable_interrupt");
    ret_vec.push_back("sch_lif_map_uncorrectable_interrupt");
    ret_vec.push_back("sch_lif_map_correctable_interrupt");
    ret_vec.push_back("sch_rlid_map_uncorrectable_interrupt");
    ret_vec.push_back("sch_rlid_map_correctable_interrupt");
    ret_vec.push_back("sch_grp_uncorrectable_interrupt");
    ret_vec.push_back("sch_grp_correctable_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_int_srams_ecc_t::get_fields(int level) const { 
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
std::vector<string> cap_txs_csr_dhs_sch_grp_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("clr_pend_cnt");
    ret_vec.push_back("rl_thr");
    ret_vec.push_back("bckgr_cnt");
    ret_vec.push_back("bckgr");
    ret_vec.push_back("hbm_mode");
    ret_vec.push_back("qid_offset");
    ret_vec.push_back("lif");
    ret_vec.push_back("auto_clr");
    ret_vec.push_back("rr_sel");
    ret_vec.push_back("qid_fifo_elm0_vld");
    ret_vec.push_back("qid_fifo_elm0_jnk");
    ret_vec.push_back("qid_fifo_elm0_qid");
    ret_vec.push_back("qid_fifo_elm1_vld");
    ret_vec.push_back("qid_fifo_elm1_jnk");
    ret_vec.push_back("qid_fifo_elm1_qid");
    ret_vec.push_back("qid_fifo_elm2_vld");
    ret_vec.push_back("qid_fifo_elm2_jnk");
    ret_vec.push_back("qid_fifo_elm2_qid");
    ret_vec.push_back("qid_fifo_elm3_vld");
    ret_vec.push_back("qid_fifo_elm3_jnk");
    ret_vec.push_back("qid_fifo_elm3_qid");
    ret_vec.push_back("qid_fifo_elm4_vld");
    ret_vec.push_back("qid_fifo_elm4_jnk");
    ret_vec.push_back("qid_fifo_elm4_qid");
    ret_vec.push_back("qid_fifo_elm5_vld");
    ret_vec.push_back("qid_fifo_elm5_jnk");
    ret_vec.push_back("qid_fifo_elm5_qid");
    ret_vec.push_back("qid_fifo_elm6_vld");
    ret_vec.push_back("qid_fifo_elm6_jnk");
    ret_vec.push_back("qid_fifo_elm6_qid");
    ret_vec.push_back("qid_fifo_elm7_vld");
    ret_vec.push_back("qid_fifo_elm7_jnk");
    ret_vec.push_back("qid_fifo_elm7_qid");
    ret_vec.push_back("qid_fifo_elm8_vld");
    ret_vec.push_back("qid_fifo_elm8_jnk");
    ret_vec.push_back("qid_fifo_elm8_qid");
    ret_vec.push_back("qid_fifo_elm9_vld");
    ret_vec.push_back("qid_fifo_elm9_jnk");
    ret_vec.push_back("qid_fifo_elm9_qid");
    ret_vec.push_back("qid_fifo_elm10_vld");
    ret_vec.push_back("qid_fifo_elm10_jnk");
    ret_vec.push_back("qid_fifo_elm10_qid");
    ret_vec.push_back("qid_fifo_elm11_vld");
    ret_vec.push_back("qid_fifo_elm11_jnk");
    ret_vec.push_back("qid_fifo_elm11_qid");
    ret_vec.push_back("qid_fifo_elm12_vld");
    ret_vec.push_back("qid_fifo_elm12_jnk");
    ret_vec.push_back("qid_fifo_elm12_qid");
    ret_vec.push_back("qid_fifo_elm13_vld");
    ret_vec.push_back("qid_fifo_elm13_jnk");
    ret_vec.push_back("qid_fifo_elm13_qid");
    ret_vec.push_back("qid_fifo_elm14_vld");
    ret_vec.push_back("qid_fifo_elm14_jnk");
    ret_vec.push_back("qid_fifo_elm14_qid");
    ret_vec.push_back("qid_fifo_elm15_vld");
    ret_vec.push_back("qid_fifo_elm15_jnk");
    ret_vec.push_back("qid_fifo_elm15_qid");
    ret_vec.push_back("hbm_ln");
    ret_vec.push_back("hbm_ln_ptr");
    ret_vec.push_back("hbm_rr_cnt");
    ret_vec.push_back("drb_cnt");
    ret_vec.push_back("hbm_ln_cnt0");
    ret_vec.push_back("hbm_ln_cnt1");
    ret_vec.push_back("hbm_ln_cnt2");
    ret_vec.push_back("hbm_ln_cnt3");
    ret_vec.push_back("hbm_ln_cnt4");
    ret_vec.push_back("hbm_ln_cnt5");
    ret_vec.push_back("hbm_ln_cnt6");
    ret_vec.push_back("hbm_ln_cnt7");
    ret_vec.push_back("hbm_ln_cnt8");
    ret_vec.push_back("hbm_ln_cnt9");
    ret_vec.push_back("hbm_ln_cnt10");
    ret_vec.push_back("hbm_ln_cnt11");
    ret_vec.push_back("hbm_ln_cnt12");
    ret_vec.push_back("hbm_ln_cnt13");
    ret_vec.push_back("hbm_ln_cnt14");
    ret_vec.push_back("hbm_ln_cnt15");
    ret_vec.push_back("popcnt");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_grp_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_rlid_map_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("sg_start");
    ret_vec.push_back("sg_end");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_rlid_map_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_lif_map_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("sg_active");
    ret_vec.push_back("sg_start");
    ret_vec.push_back("sg_per_cos");
    ret_vec.push_back("sg_act_cos");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_lif_map_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_tmr_cnt_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("slow_cbcnt");
    ret_vec.push_back("slow_bcnt");
    ret_vec.push_back("slow_lcnt");
    ret_vec.push_back("fast_cbcnt");
    ret_vec.push_back("fast_bcnt");
    ret_vec.push_back("fast_lcnt");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_tmr_cnt_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_rlid_stop_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("setmsk");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_rlid_stop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("popcnt");
    ret_vec.push_back("drb_cnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_grp_cnt_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_grp_entry_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lif");
    ret_vec.push_back("qid_offset");
    ret_vec.push_back("auto_clr");
    ret_vec.push_back("rr_sel");
    ret_vec.push_back("rl_thr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_grp_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_doorbell_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("qid");
    ret_vec.push_back("cos");
    ret_vec.push_back("set");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_doorbell_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_flags_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sch_flags");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_sch_flags_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_dtdmhi_calendar_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dtdm_calendar");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_dtdmhi_calendar_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_dtdmlo_calendar_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dtdm_calendar");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_dtdmlo_calendar_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_slow_timer_pending_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bcnt");
    ret_vec.push_back("lcnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_slow_timer_pending_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("typ");
    ret_vec.push_back("qid");
    ret_vec.push_back("ring");
    ret_vec.push_back("dtime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_slow_timer_start_no_stop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_fast_timer_pending_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bcnt");
    ret_vec.push_back("lcnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_fast_timer_pending_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("typ");
    ret_vec.push_back("qid");
    ret_vec.push_back("ring");
    ret_vec.push_back("dtime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_dhs_fast_timer_start_no_stop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_bid_uexp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_bid_rerr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_bid_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_bid_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_wr_req_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_rd_rsp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_tmr_axi_rd_req_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_bid_uexp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_bid_rerr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_bid_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_bid_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_wr_req_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_rd_rsp_err_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_rd_rsp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_axi_rd_req_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_stmr_max_bcnt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bcnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_ftmr_max_bcnt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bcnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_sch_hbm_byp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_sch_hbm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_hbm_byp_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_hbm_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_ecc_sch_grp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_ecc_sch_rlid_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_ecc_sch_lif_map_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_ecc_tmr_cnt_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_scheduler_dbg2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_byp_pending_cnt");
    ret_vec.push_back("hbm_rd_pending_cnt");
    ret_vec.push_back("hbm_wr_pending_cnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_scheduler_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("txdma_drdy");
    ret_vec.push_back("hbm_re_efull");
    ret_vec.push_back("hbm_we_efull");
    ret_vec.push_back("txdma_msg_efull");
    ret_vec.push_back("hbm_byp_pending_efull");
    ret_vec.push_back("hbm_rd_pending_efull");
    ret_vec.push_back("hbm_wr_pending_efull");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_sch_lif_sg_mismatch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lif");
    ret_vec.push_back("qid");
    ret_vec.push_back("set");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_sch_lif_map_notactive_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("lif");
    ret_vec.push_back("qid");
    ret_vec.push_back("cos");
    ret_vec.push_back("set");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_grp_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("dhs_eccbypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_rlid_map_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("dhs_eccbypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_lif_map_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("dhs_eccbypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_tmr_cnt_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_disable_cor");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("dhs_eccbypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_sch_max_hbm_byp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_scheduler_dbg2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("qid_read");
    ret_vec.push_back("cos_read");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_scheduler_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_efc_thr");
    ret_vec.push_back("txdma_efc_thr");
    ret_vec.push_back("fifo_mode_thr");
    ret_vec.push_back("max_hbm_byp");
    ret_vec.push_back("max_hbm_wr");
    ret_vec.push_back("max_hbm_rd");
    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_scheduler_rr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("curr_ptr0");
    ret_vec.push_back("curr_ptr1");
    ret_vec.push_back("curr_ptr2");
    ret_vec.push_back("curr_ptr3");
    ret_vec.push_back("curr_ptr4");
    ret_vec.push_back("curr_ptr5");
    ret_vec.push_back("curr_ptr6");
    ret_vec.push_back("curr_ptr7");
    ret_vec.push_back("curr_ptr8");
    ret_vec.push_back("curr_ptr9");
    ret_vec.push_back("curr_ptr10");
    ret_vec.push_back("curr_ptr11");
    ret_vec.push_back("curr_ptr12");
    ret_vec.push_back("curr_ptr13");
    ret_vec.push_back("curr_ptr14");
    ret_vec.push_back("curr_ptr15");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_scheduler_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_init_done");
    ret_vec.push_back("sram_init_done");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_rlid_start_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_rlid_stop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_glb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("pb_xoff");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos15_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos14_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos13_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos12_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos11_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos10_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos9_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos8_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos7_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos6_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos5_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos4_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos3_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_txdma_cos0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_fifo_empty_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_doorbell_clr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_sch_doorbell_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_sch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dtdm_lo_map");
    ret_vec.push_back("dtdm_hi_map");
    ret_vec.push_back("timeout");
    ret_vec.push_back("pause");
    ret_vec.push_back("enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfw_scheduler_static_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_base");
    ret_vec.push_back("sch_grp_depth");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfw_scheduler_glb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_hw_init");
    ret_vec.push_back("sram_hw_init");
    ret_vec.push_back("spare");
    ret_vec.push_back("enable");
    ret_vec.push_back("enable_set_lkup");
    ret_vec.push_back("enable_set_byp");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_tmr_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_tmr_hbm_byp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_srams_tmr_hbm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_tmr_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_tmr_hbm_byp_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_tmr_hbm_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_stmr_pop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_stmr_key_not_found_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_stmr_push_out_of_wheel_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_stmr_key_not_push_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_stmr_push_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_slow_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tick");
    ret_vec.push_back("cTime");
    ret_vec.push_back("pTime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_slow_timer_dbell_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addr_update");
    ret_vec.push_back("data_pid");
    ret_vec.push_back("data_reserved");
    ret_vec.push_back("data_index");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_force_slow_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("ctime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_slow_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tick");
    ret_vec.push_back("hash_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_ftmr_pop_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_ftmr_key_not_found_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_ftmr_push_out_of_wheel_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_ftmr_key_not_push_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cnt_ftmr_push_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("val");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_fast_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tick");
    ret_vec.push_back("cTime");
    ret_vec.push_back("pTime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_fast_timer_dbell_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addr_update");
    ret_vec.push_back("data_pid");
    ret_vec.push_back("data_reserved");
    ret_vec.push_back("data_index");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_force_fast_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("ctime");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_fast_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tick");
    ret_vec.push_back("hash_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_timer_dbg2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_byp_pending_cnt");
    ret_vec.push_back("hbm_rd_pending_cnt");
    ret_vec.push_back("hbm_wr_pending_cnt");
    ret_vec.push_back("stmr_stall");
    ret_vec.push_back("ftmr_stall");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_timer_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_re_efull");
    ret_vec.push_back("hbm_we_efull");
    ret_vec.push_back("rejct_drb_efull");
    ret_vec.push_back("hbm_byp_pending_efull");
    ret_vec.push_back("hbm_rd_pending_efull");
    ret_vec.push_back("hbm_wr_pending_efull");
    ret_vec.push_back("stmr_fifo_raddr");
    ret_vec.push_back("ftmr_fifo_raddr");
    ret_vec.push_back("stmr_fifo_waddr");
    ret_vec.push_back("ftmr_fifo_waddr");
    ret_vec.push_back("stmr_fifo_efull");
    ret_vec.push_back("ftmr_fifo_efull");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_tmr_max_keys_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_tmr_max_hbm_byp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cnt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_sta_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_init_done");
    ret_vec.push_back("sram_init_done");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_timer_dbg2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("max_bcnt");
    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_timer_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_efc_thr");
    ret_vec.push_back("drb_efc_thr");
    ret_vec.push_back("tmr_stall_thr_hi");
    ret_vec.push_back("tmr_stall_thr_lo");
    ret_vec.push_back("max_hbm_wr");
    ret_vec.push_back("max_hbm_rd");
    ret_vec.push_back("max_hbm_byp");
    ret_vec.push_back("max_tmr_fifo");
    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfw_timer_glb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_hw_init");
    ret_vec.push_back("sram_hw_init");
    ret_vec.push_back("spare");
    ret_vec.push_back("ftmr_enable");
    ret_vec.push_back("stmr_enable");
    ret_vec.push_back("ftmr_pause");
    ret_vec.push_back("stmr_pause");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_timer_static_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("hbm_base");
    ret_vec.push_back("tmr_hsh_depth");
    ret_vec.push_back("tmr_wheel_depth");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_cfg_axi_attr_t::get_fields(int level) const { 
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
std::vector<string> cap_txs_csr_cfg_glb_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dbg_port_select");
    ret_vec.push_back("dbg_port_enable");
    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_txs_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_glb.get_fields(level-1)) {
            ret_vec.push_back("cfg_glb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_axi_attr.get_fields(level-1)) {
            ret_vec.push_back("cfg_axi_attr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_timer_static.get_fields(level-1)) {
            ret_vec.push_back("cfg_timer_static." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_timer_glb.get_fields(level-1)) {
            ret_vec.push_back("cfw_timer_glb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_timer_dbg.get_fields(level-1)) {
            ret_vec.push_back("cfg_timer_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_timer_dbg2.get_fields(level-1)) {
            ret_vec.push_back("cfg_timer_dbg2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_timer.get_fields(level-1)) {
            ret_vec.push_back("sta_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_tmr_max_hbm_byp.get_fields(level-1)) {
            ret_vec.push_back("sta_tmr_max_hbm_byp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_tmr_max_keys.get_fields(level-1)) {
            ret_vec.push_back("sta_tmr_max_keys." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_timer_dbg.get_fields(level-1)) {
            ret_vec.push_back("sta_timer_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_timer_dbg2.get_fields(level-1)) {
            ret_vec.push_back("sta_timer_dbg2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_fast_timer.get_fields(level-1)) {
            ret_vec.push_back("cfg_fast_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_force_fast_timer.get_fields(level-1)) {
            ret_vec.push_back("cfg_force_fast_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_fast_timer_dbell.get_fields(level-1)) {
            ret_vec.push_back("cfg_fast_timer_dbell." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_fast_timer.get_fields(level-1)) {
            ret_vec.push_back("sta_fast_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_ftmr_push.get_fields(level-1)) {
            ret_vec.push_back("cnt_ftmr_push." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_ftmr_key_not_push.get_fields(level-1)) {
            ret_vec.push_back("cnt_ftmr_key_not_push." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_ftmr_push_out_of_wheel.get_fields(level-1)) {
            ret_vec.push_back("cnt_ftmr_push_out_of_wheel." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_ftmr_key_not_found.get_fields(level-1)) {
            ret_vec.push_back("cnt_ftmr_key_not_found." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_ftmr_pop.get_fields(level-1)) {
            ret_vec.push_back("cnt_ftmr_pop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_slow_timer.get_fields(level-1)) {
            ret_vec.push_back("cfg_slow_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_force_slow_timer.get_fields(level-1)) {
            ret_vec.push_back("cfg_force_slow_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_slow_timer_dbell.get_fields(level-1)) {
            ret_vec.push_back("cfg_slow_timer_dbell." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_slow_timer.get_fields(level-1)) {
            ret_vec.push_back("sta_slow_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_stmr_push.get_fields(level-1)) {
            ret_vec.push_back("cnt_stmr_push." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_stmr_key_not_push.get_fields(level-1)) {
            ret_vec.push_back("cnt_stmr_key_not_push." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_stmr_push_out_of_wheel.get_fields(level-1)) {
            ret_vec.push_back("cnt_stmr_push_out_of_wheel." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_stmr_key_not_found.get_fields(level-1)) {
            ret_vec.push_back("cnt_stmr_key_not_found." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_stmr_pop.get_fields(level-1)) {
            ret_vec.push_back("cnt_stmr_pop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tmr_hbm_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_tmr_hbm_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tmr_hbm_byp_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_tmr_hbm_byp_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tmr_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_tmr_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_tmr_hbm.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_tmr_hbm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_tmr_hbm_byp.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_tmr_hbm_byp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_tmr_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_tmr_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_scheduler_glb.get_fields(level-1)) {
            ret_vec.push_back("cfw_scheduler_glb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_scheduler_static.get_fields(level-1)) {
            ret_vec.push_back("cfw_scheduler_static." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_doorbell_set.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_doorbell_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_doorbell_clr.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_doorbell_clr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_fifo_empty.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_fifo_empty." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos0.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos1.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos2.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos3.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos3." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos4.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos4." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos5.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos5." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos6.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos6." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos7.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos7." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos8.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos8." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos9.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos9." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos10.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos10." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos11.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos11." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos12.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos12." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos13.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos13." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos14.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos14." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_txdma_cos15.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_txdma_cos15." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_glb.get_fields(level-1)) {
            ret_vec.push_back("sta_glb." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_rlid_stop.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_rlid_stop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_rlid_start.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_rlid_start." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_scheduler.get_fields(level-1)) {
            ret_vec.push_back("sta_scheduler." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_scheduler_rr.get_fields(level-1)) {
            ret_vec.push_back("sta_scheduler_rr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_scheduler_dbg.get_fields(level-1)) {
            ret_vec.push_back("cfg_scheduler_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_scheduler_dbg2.get_fields(level-1)) {
            ret_vec.push_back("cfg_scheduler_dbg2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sch_max_hbm_byp.get_fields(level-1)) {
            ret_vec.push_back("sta_sch_max_hbm_byp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_tmr_cnt_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_tmr_cnt_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch_lif_map_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch_lif_map_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch_rlid_map_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch_rlid_map_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch_grp_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch_grp_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sch_lif_map_notactive.get_fields(level-1)) {
            ret_vec.push_back("sta_sch_lif_map_notactive." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sch_lif_sg_mismatch.get_fields(level-1)) {
            ret_vec.push_back("sta_sch_lif_sg_mismatch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_scheduler_dbg.get_fields(level-1)) {
            ret_vec.push_back("sta_scheduler_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_scheduler_dbg2.get_fields(level-1)) {
            ret_vec.push_back("sta_scheduler_dbg2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_tmr_cnt.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_tmr_cnt." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_sch_lif_map.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_sch_lif_map." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_sch_rlid_map.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_sch_rlid_map." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_sch_grp.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_sch_grp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch_hbm_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch_hbm_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sch_hbm_byp_sram.get_fields(level-1)) {
            ret_vec.push_back("cfg_sch_hbm_byp_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_sch_hbm.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_sch_hbm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_sch_hbm_byp.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_sch_hbm_byp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ftmr_max_bcnt.get_fields(level-1)) {
            ret_vec.push_back("sta_ftmr_max_bcnt." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_stmr_max_bcnt.get_fields(level-1)) {
            ret_vec.push_back("sta_stmr_max_bcnt." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_rd_req.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_rd_req." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_rd_rsp.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_rd_rsp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_rd_rsp_err.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_rd_rsp_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_rd_rsp_rerr.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_rd_rsp_rerr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_rd_rsp_uexp.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_rd_rsp_uexp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_wr_req.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_wr_req." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_bid.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_bid." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_bid_err.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_bid_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_bid_rerr.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_bid_rerr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_sch_axi_bid_uexp.get_fields(level-1)) {
            ret_vec.push_back("cnt_sch_axi_bid_uexp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_rd_req.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_rd_req." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_rd_rsp.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_rd_rsp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_rd_rsp_err.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_rd_rsp_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_rd_rsp_rerr.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_rd_rsp_rerr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_rd_rsp_uexp.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_rd_rsp_uexp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_wr_req.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_wr_req." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_bid.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_bid." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_bid_err.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_bid_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_bid_rerr.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_bid_rerr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cnt_tmr_axi_bid_uexp.get_fields(level-1)) {
            ret_vec.push_back("cnt_tmr_axi_bid_uexp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_fast_timer_start_no_stop.get_fields(level-1)) {
            ret_vec.push_back("dhs_fast_timer_start_no_stop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_fast_timer_pending.get_fields(level-1)) {
            ret_vec.push_back("dhs_fast_timer_pending." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_slow_timer_start_no_stop.get_fields(level-1)) {
            ret_vec.push_back("dhs_slow_timer_start_no_stop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_slow_timer_pending.get_fields(level-1)) {
            ret_vec.push_back("dhs_slow_timer_pending." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dtdmlo_calendar.get_fields(level-1)) {
            ret_vec.push_back("dhs_dtdmlo_calendar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dtdmhi_calendar.get_fields(level-1)) {
            ret_vec.push_back("dhs_dtdmhi_calendar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_flags.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_flags." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_doorbell.get_fields(level-1)) {
            ret_vec.push_back("dhs_doorbell." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_grp_entry.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_grp_entry." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_grp_cnt_entry.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_grp_cnt_entry." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_rlid_stop.get_fields(level-1)) {
            ret_vec.push_back("dhs_rlid_stop." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_tmr_cnt_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_tmr_cnt_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_lif_map_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_lif_map_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_rlid_map_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_rlid_map_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_sch_grp_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_sch_grp_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_srams_ecc.get_fields(level-1)) {
            ret_vec.push_back("int_srams_ecc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_sch.get_fields(level-1)) {
            ret_vec.push_back("int_sch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_tmr.get_fields(level-1)) {
            ret_vec.push_back("int_tmr." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
