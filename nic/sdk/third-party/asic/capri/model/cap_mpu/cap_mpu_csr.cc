
#include "cap_mpu_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_mpu_csr_int_info_int_enable_clear_t::cap_mpu_csr_int_info_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_info_int_enable_clear_t::~cap_mpu_csr_int_info_int_enable_clear_t() { }

cap_mpu_csr_int_info_int_test_set_t::cap_mpu_csr_int_info_int_test_set_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_info_int_test_set_t::~cap_mpu_csr_int_info_int_test_set_t() { }

cap_mpu_csr_int_info_t::cap_mpu_csr_int_info_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_info_t::~cap_mpu_csr_int_info_t() { }

cap_mpu_csr_int_err_int_enable_clear_t::cap_mpu_csr_int_err_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_err_int_enable_clear_t::~cap_mpu_csr_int_err_int_enable_clear_t() { }

cap_mpu_csr_int_err_intreg_t::cap_mpu_csr_int_err_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_err_intreg_t::~cap_mpu_csr_int_err_intreg_t() { }

cap_mpu_csr_int_err_t::cap_mpu_csr_int_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_err_t::~cap_mpu_csr_int_err_t() { }

cap_mpu_csr_intreg_status_t::cap_mpu_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_intreg_status_t::~cap_mpu_csr_intreg_status_t() { }

cap_mpu_csr_int_groups_int_enable_rw_reg_t::cap_mpu_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_int_groups_int_enable_rw_reg_t::~cap_mpu_csr_int_groups_int_enable_rw_reg_t() { }

cap_mpu_csr_intgrp_status_t::cap_mpu_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_intgrp_status_t::~cap_mpu_csr_intgrp_status_t() { }

cap_mpu_csr_spr_reg_data_t::cap_mpu_csr_spr_reg_data_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_spr_reg_data_t::~cap_mpu_csr_spr_reg_data_t() { }

cap_mpu_csr_spr_reg_t::cap_mpu_csr_spr_reg_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_spr_reg_t::~cap_mpu_csr_spr_reg_t() { }

cap_mpu_csr_icache_sram_read_data_t::cap_mpu_csr_icache_sram_read_data_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_icache_sram_read_data_t::~cap_mpu_csr_icache_sram_read_data_t() { }

cap_mpu_csr_icache_sram_read_t::cap_mpu_csr_icache_sram_read_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_icache_sram_read_t::~cap_mpu_csr_icache_sram_read_t() { }

cap_mpu_csr_tag_read_data_t::cap_mpu_csr_tag_read_data_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_tag_read_data_t::~cap_mpu_csr_tag_read_data_t() { }

cap_mpu_csr_tag_read_t::cap_mpu_csr_tag_read_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_tag_read_t::~cap_mpu_csr_tag_read_t() { }

cap_mpu_csr_csr_intr_t::cap_mpu_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_csr_intr_t::~cap_mpu_csr_csr_intr_t() { }

cap_mpu_csr_sta_spare_t::cap_mpu_csr_sta_spare_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_spare_t::~cap_mpu_csr_sta_spare_t() { }

cap_mpu_csr_cfg_spare_t::cap_mpu_csr_cfg_spare_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_spare_t::~cap_mpu_csr_cfg_spare_t() { }

cap_mpu_csr_sta_mpu3_icache_tag_t::cap_mpu_csr_sta_mpu3_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu3_icache_tag_t::~cap_mpu_csr_sta_mpu3_icache_tag_t() { }

cap_mpu_csr_cfg_mpu3_icache_tag_t::cap_mpu_csr_cfg_mpu3_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu3_icache_tag_t::~cap_mpu_csr_cfg_mpu3_icache_tag_t() { }

cap_mpu_csr_sta_mpu2_icache_tag_t::cap_mpu_csr_sta_mpu2_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu2_icache_tag_t::~cap_mpu_csr_sta_mpu2_icache_tag_t() { }

cap_mpu_csr_cfg_mpu2_icache_tag_t::cap_mpu_csr_cfg_mpu2_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu2_icache_tag_t::~cap_mpu_csr_cfg_mpu2_icache_tag_t() { }

cap_mpu_csr_sta_mpu1_icache_tag_t::cap_mpu_csr_sta_mpu1_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu1_icache_tag_t::~cap_mpu_csr_sta_mpu1_icache_tag_t() { }

cap_mpu_csr_cfg_mpu1_icache_tag_t::cap_mpu_csr_cfg_mpu1_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu1_icache_tag_t::~cap_mpu_csr_cfg_mpu1_icache_tag_t() { }

cap_mpu_csr_sta_mpu0_icache_tag_t::cap_mpu_csr_sta_mpu0_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu0_icache_tag_t::~cap_mpu_csr_sta_mpu0_icache_tag_t() { }

cap_mpu_csr_cfg_mpu0_icache_tag_t::cap_mpu_csr_cfg_mpu0_icache_tag_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu0_icache_tag_t::~cap_mpu_csr_cfg_mpu0_icache_tag_t() { }

cap_mpu_csr_sta_mpu3_icache_data1_t::cap_mpu_csr_sta_mpu3_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu3_icache_data1_t::~cap_mpu_csr_sta_mpu3_icache_data1_t() { }

cap_mpu_csr_cfg_mpu3_icache_data1_t::cap_mpu_csr_cfg_mpu3_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu3_icache_data1_t::~cap_mpu_csr_cfg_mpu3_icache_data1_t() { }

cap_mpu_csr_sta_mpu3_icache_data0_t::cap_mpu_csr_sta_mpu3_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu3_icache_data0_t::~cap_mpu_csr_sta_mpu3_icache_data0_t() { }

cap_mpu_csr_cfg_mpu3_icache_data0_t::cap_mpu_csr_cfg_mpu3_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu3_icache_data0_t::~cap_mpu_csr_cfg_mpu3_icache_data0_t() { }

cap_mpu_csr_sta_mpu2_icache_data1_t::cap_mpu_csr_sta_mpu2_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu2_icache_data1_t::~cap_mpu_csr_sta_mpu2_icache_data1_t() { }

cap_mpu_csr_cfg_mpu2_icache_data1_t::cap_mpu_csr_cfg_mpu2_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu2_icache_data1_t::~cap_mpu_csr_cfg_mpu2_icache_data1_t() { }

cap_mpu_csr_sta_mpu2_icache_data0_t::cap_mpu_csr_sta_mpu2_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu2_icache_data0_t::~cap_mpu_csr_sta_mpu2_icache_data0_t() { }

cap_mpu_csr_cfg_mpu2_icache_data0_t::cap_mpu_csr_cfg_mpu2_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu2_icache_data0_t::~cap_mpu_csr_cfg_mpu2_icache_data0_t() { }

cap_mpu_csr_sta_mpu1_icache_data1_t::cap_mpu_csr_sta_mpu1_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu1_icache_data1_t::~cap_mpu_csr_sta_mpu1_icache_data1_t() { }

cap_mpu_csr_cfg_mpu1_icache_data1_t::cap_mpu_csr_cfg_mpu1_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu1_icache_data1_t::~cap_mpu_csr_cfg_mpu1_icache_data1_t() { }

cap_mpu_csr_sta_mpu1_icache_data0_t::cap_mpu_csr_sta_mpu1_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu1_icache_data0_t::~cap_mpu_csr_sta_mpu1_icache_data0_t() { }

cap_mpu_csr_cfg_mpu1_icache_data0_t::cap_mpu_csr_cfg_mpu1_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu1_icache_data0_t::~cap_mpu_csr_cfg_mpu1_icache_data0_t() { }

cap_mpu_csr_sta_mpu0_icache_data1_t::cap_mpu_csr_sta_mpu0_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu0_icache_data1_t::~cap_mpu_csr_sta_mpu0_icache_data1_t() { }

cap_mpu_csr_cfg_mpu0_icache_data1_t::cap_mpu_csr_cfg_mpu0_icache_data1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu0_icache_data1_t::~cap_mpu_csr_cfg_mpu0_icache_data1_t() { }

cap_mpu_csr_sta_mpu0_icache_data0_t::cap_mpu_csr_sta_mpu0_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu0_icache_data0_t::~cap_mpu_csr_sta_mpu0_icache_data0_t() { }

cap_mpu_csr_cfg_mpu0_icache_data0_t::cap_mpu_csr_cfg_mpu0_icache_data0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_mpu0_icache_data0_t::~cap_mpu_csr_cfg_mpu0_icache_data0_t() { }

cap_mpu_csr_sta_sdp_mem_t::cap_mpu_csr_sta_sdp_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_sdp_mem_t::~cap_mpu_csr_sta_sdp_mem_t() { }

cap_mpu_csr_cfg_sdp_mem_t::cap_mpu_csr_cfg_sdp_mem_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_sdp_mem_t::~cap_mpu_csr_cfg_sdp_mem_t() { }

cap_mpu_csr_cfg_debug_port_t::cap_mpu_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_cfg_debug_port_t::~cap_mpu_csr_cfg_debug_port_t() { }

cap_mpu_csr_mismatch_t::cap_mpu_csr_mismatch_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mismatch_t::~cap_mpu_csr_mismatch_t() { }

cap_mpu_csr_STA_stg_t::cap_mpu_csr_STA_stg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_STA_stg_t::~cap_mpu_csr_STA_stg_t() { }

cap_mpu_csr_STA_live_sdp_t::cap_mpu_csr_STA_live_sdp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_STA_live_sdp_t::~cap_mpu_csr_STA_live_sdp_t() { }

cap_mpu_csr_CNT_sdp_t::cap_mpu_csr_CNT_sdp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_sdp_t::~cap_mpu_csr_CNT_sdp_t() { }

cap_mpu_csr_sta_error_t::cap_mpu_csr_sta_error_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_error_t::~cap_mpu_csr_sta_error_t() { }

cap_mpu_csr_sta_pend_t::cap_mpu_csr_sta_pend_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_pend_t::~cap_mpu_csr_sta_pend_t() { }

cap_mpu_csr_sta_gpr7_mpu_t::cap_mpu_csr_sta_gpr7_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr7_mpu_t::~cap_mpu_csr_sta_gpr7_mpu_t() { }

cap_mpu_csr_sta_gpr6_mpu_t::cap_mpu_csr_sta_gpr6_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr6_mpu_t::~cap_mpu_csr_sta_gpr6_mpu_t() { }

cap_mpu_csr_sta_gpr5_mpu_t::cap_mpu_csr_sta_gpr5_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr5_mpu_t::~cap_mpu_csr_sta_gpr5_mpu_t() { }

cap_mpu_csr_sta_gpr4_mpu_t::cap_mpu_csr_sta_gpr4_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr4_mpu_t::~cap_mpu_csr_sta_gpr4_mpu_t() { }

cap_mpu_csr_sta_gpr3_mpu_t::cap_mpu_csr_sta_gpr3_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr3_mpu_t::~cap_mpu_csr_sta_gpr3_mpu_t() { }

cap_mpu_csr_sta_gpr2_mpu_t::cap_mpu_csr_sta_gpr2_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr2_mpu_t::~cap_mpu_csr_sta_gpr2_mpu_t() { }

cap_mpu_csr_sta_gpr1_mpu_t::cap_mpu_csr_sta_gpr1_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_gpr1_mpu_t::~cap_mpu_csr_sta_gpr1_mpu_t() { }

cap_mpu_csr_sta_mpu_t::cap_mpu_csr_sta_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu_t::~cap_mpu_csr_sta_mpu_t() { }

cap_mpu_csr_sta_ctl_mpu_t::cap_mpu_csr_sta_ctl_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_ctl_mpu_t::~cap_mpu_csr_sta_ctl_mpu_t() { }

cap_mpu_csr_sta_pc_mpu_t::cap_mpu_csr_sta_pc_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_pc_mpu_t::~cap_mpu_csr_sta_pc_mpu_t() { }

cap_mpu_csr_sta_key_t::cap_mpu_csr_sta_key_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_key_t::~cap_mpu_csr_sta_key_t() { }

cap_mpu_csr_sta_data_t::cap_mpu_csr_sta_data_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_data_t::~cap_mpu_csr_sta_data_t() { }

cap_mpu_csr_sta_table_t::cap_mpu_csr_sta_table_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_table_t::~cap_mpu_csr_sta_table_t() { }

cap_mpu_csr_sta_tbl_addr_t::cap_mpu_csr_sta_tbl_addr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_tbl_addr_t::~cap_mpu_csr_sta_tbl_addr_t() { }

cap_mpu_csr_CNT_fence_stall_t::cap_mpu_csr_CNT_fence_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_fence_stall_t::~cap_mpu_csr_CNT_fence_stall_t() { }

cap_mpu_csr_CNT_tblwr_stall_t::cap_mpu_csr_CNT_tblwr_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_tblwr_stall_t::~cap_mpu_csr_CNT_tblwr_stall_t() { }

cap_mpu_csr_CNT_memwr_stall_t::cap_mpu_csr_CNT_memwr_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_memwr_stall_t::~cap_mpu_csr_CNT_memwr_stall_t() { }

cap_mpu_csr_CNT_phvwr_stall_t::cap_mpu_csr_CNT_phvwr_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_phvwr_stall_t::~cap_mpu_csr_CNT_phvwr_stall_t() { }

cap_mpu_csr_CNT_hazard_stall_t::cap_mpu_csr_CNT_hazard_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_hazard_stall_t::~cap_mpu_csr_CNT_hazard_stall_t() { }

cap_mpu_csr_CNT_icache_fill_stall_t::cap_mpu_csr_CNT_icache_fill_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_icache_fill_stall_t::~cap_mpu_csr_CNT_icache_fill_stall_t() { }

cap_mpu_csr_CNT_icache_miss_t::cap_mpu_csr_CNT_icache_miss_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_icache_miss_t::~cap_mpu_csr_CNT_icache_miss_t() { }

cap_mpu_csr_CNT_inst_executed_t::cap_mpu_csr_CNT_inst_executed_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_inst_executed_t::~cap_mpu_csr_CNT_inst_executed_t() { }

cap_mpu_csr_CNT_phv_executed_t::cap_mpu_csr_CNT_phv_executed_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_phv_executed_t::~cap_mpu_csr_CNT_phv_executed_t() { }

cap_mpu_csr_CNT_cycles_t::cap_mpu_csr_CNT_cycles_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_cycles_t::~cap_mpu_csr_CNT_cycles_t() { }

cap_mpu_csr_count_stage_t::cap_mpu_csr_count_stage_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_count_stage_t::~cap_mpu_csr_count_stage_t() { }

cap_mpu_csr_mpu_run_t::cap_mpu_csr_mpu_run_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mpu_run_t::~cap_mpu_csr_mpu_run_t() { }

cap_mpu_csr_mpu_cfg_t::cap_mpu_csr_mpu_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mpu_cfg_t::~cap_mpu_csr_mpu_cfg_t() { }

cap_mpu_csr_mpu_ctl_t::cap_mpu_csr_mpu_ctl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mpu_ctl_t::~cap_mpu_csr_mpu_ctl_t() { }

cap_mpu_csr_sdp_ctl_t::cap_mpu_csr_sdp_ctl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sdp_ctl_t::~cap_mpu_csr_sdp_ctl_t() { }

cap_mpu_csr_axi_attr_t::cap_mpu_csr_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_axi_attr_t::~cap_mpu_csr_axi_attr_t() { }

cap_mpu_csr_icache_t::cap_mpu_csr_icache_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_icache_t::~cap_mpu_csr_icache_t() { }

cap_mpu_csr_trace_t::cap_mpu_csr_trace_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_trace_t::~cap_mpu_csr_trace_t() { }

cap_mpu_csr_base_t::cap_mpu_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_base_t::~cap_mpu_csr_base_t() { }

cap_mpu_csr_t::cap_mpu_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(65536);
        set_attributes(0,get_name(), 0);
        }
cap_mpu_csr_t::~cap_mpu_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_info_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_0_enable: 0x" << int_var__trace_full_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_1_enable: 0x" << int_var__trace_full_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_2_enable: 0x" << int_var__trace_full_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_3_enable: 0x" << int_var__trace_full_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_0_enable: 0x" << int_var__mpu_stop_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_1_enable: 0x" << int_var__mpu_stop_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_2_enable: 0x" << int_var__mpu_stop_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_3_enable: 0x" << int_var__mpu_stop_3_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_info_int_test_set_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_0_interrupt: 0x" << int_var__trace_full_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_1_interrupt: 0x" << int_var__trace_full_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_2_interrupt: 0x" << int_var__trace_full_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_full_3_interrupt: 0x" << int_var__trace_full_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_0_interrupt: 0x" << int_var__mpu_stop_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_1_interrupt: 0x" << int_var__mpu_stop_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_2_interrupt: 0x" << int_var__mpu_stop_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_stop_3_interrupt: 0x" << int_var__mpu_stop_3_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_info_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_err_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".results_mismatch_enable: 0x" << int_var__results_mismatch_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_mem_uncorrectable_enable: 0x" << int_var__sdp_mem_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_mem_correctable_enable: 0x" << int_var__sdp_mem_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_0_enable: 0x" << int_var__illegal_op_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_1_enable: 0x" << int_var__illegal_op_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_2_enable: 0x" << int_var__illegal_op_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_3_enable: 0x" << int_var__illegal_op_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_0_enable: 0x" << int_var__max_inst_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_1_enable: 0x" << int_var__max_inst_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_2_enable: 0x" << int_var__max_inst_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_3_enable: 0x" << int_var__max_inst_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_0_enable: 0x" << int_var__phvwr_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_1_enable: 0x" << int_var__phvwr_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_2_enable: 0x" << int_var__phvwr_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_3_enable: 0x" << int_var__phvwr_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_0_enable: 0x" << int_var__write_err_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_1_enable: 0x" << int_var__write_err_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_2_enable: 0x" << int_var__write_err_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_3_enable: 0x" << int_var__write_err_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_0_enable: 0x" << int_var__cache_axi_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_1_enable: 0x" << int_var__cache_axi_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_2_enable: 0x" << int_var__cache_axi_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_3_enable: 0x" << int_var__cache_axi_3_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_0_enable: 0x" << int_var__cache_parity_0_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_1_enable: 0x" << int_var__cache_parity_1_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_2_enable: 0x" << int_var__cache_parity_2_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_3_enable: 0x" << int_var__cache_parity_3_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_err_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".results_mismatch_interrupt: 0x" << int_var__results_mismatch_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_mem_uncorrectable_interrupt: 0x" << int_var__sdp_mem_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_mem_correctable_interrupt: 0x" << int_var__sdp_mem_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_0_interrupt: 0x" << int_var__illegal_op_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_1_interrupt: 0x" << int_var__illegal_op_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_2_interrupt: 0x" << int_var__illegal_op_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".illegal_op_3_interrupt: 0x" << int_var__illegal_op_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_0_interrupt: 0x" << int_var__max_inst_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_1_interrupt: 0x" << int_var__max_inst_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_2_interrupt: 0x" << int_var__max_inst_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_3_interrupt: 0x" << int_var__max_inst_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_0_interrupt: 0x" << int_var__phvwr_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_1_interrupt: 0x" << int_var__phvwr_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_2_interrupt: 0x" << int_var__phvwr_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_3_interrupt: 0x" << int_var__phvwr_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_0_interrupt: 0x" << int_var__write_err_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_1_interrupt: 0x" << int_var__write_err_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_2_interrupt: 0x" << int_var__write_err_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".write_err_3_interrupt: 0x" << int_var__write_err_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_0_interrupt: 0x" << int_var__cache_axi_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_1_interrupt: 0x" << int_var__cache_axi_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_2_interrupt: 0x" << int_var__cache_axi_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_axi_3_interrupt: 0x" << int_var__cache_axi_3_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_0_interrupt: 0x" << int_var__cache_parity_0_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_1_interrupt: 0x" << int_var__cache_parity_1_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_2_interrupt: 0x" << int_var__cache_parity_2_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache_parity_3_interrupt: 0x" << int_var__cache_parity_3_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_err_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_err_interrupt: 0x" << int_var__int_err_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_info_interrupt: 0x" << int_var__int_info_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".int_err_enable: 0x" << int_var__int_err_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".int_info_enable: 0x" << int_var__int_info_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_spr_reg_data_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_spr_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    data.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_icache_sram_read_data_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_icache_sram_read_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    data.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_tag_read_data_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_tag_read_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    data.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_spare_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_spare_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu3_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu3_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu2_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu2_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu1_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu1_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu0_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu0_icache_tag_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu3_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu3_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu3_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu3_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu2_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu2_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu2_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu2_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu1_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu1_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu1_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu1_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu0_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu0_icache_data1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu0_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error: 0x" << int_var__parity_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".parity_error_addr: 0x" << int_var__parity_error_addr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_mpu0_icache_data0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_sdp_mem_t::show() {
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
void cap_mpu_csr_cfg_sdp_mem_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_det: 0x" << int_var__ecc_disable_det << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_disable_cor: 0x" << int_var__ecc_disable_cor << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_cfg_debug_port_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_mismatch_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".update_result_count: 0x" << int_var__update_result_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_result_count: 0x" << int_var__sdp_result_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_id: 0x" << int_var__pkt_id << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_STA_stg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_srdy_in: 0x" << int_var__sdp_srdy_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sdp_drdy_in: 0x" << int_var__sdp_drdy_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".srdy_out: 0x" << int_var__srdy_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drdy_out: 0x" << int_var__drdy_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".srdy: 0x" << int_var__srdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".drdy: 0x" << int_var__drdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".te_valid: 0x" << int_var__te_valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing: 0x" << int_var__mpu_processing << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_STA_live_sdp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".update_empty: 0x" << int_var__update_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".update_pkt_id: 0x" << int_var__update_pkt_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_pkt_id: 0x" << int_var__phv_pkt_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_update_valid: 0x" << int_var__phv_update_valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".update_result_count: 0x" << int_var__update_result_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".staging_full: 0x" << int_var__staging_full << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wptr: 0x" << int_var__wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rptr: 0x" << int_var__rptr << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_sdp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".phv_fifo_depth: 0x" << int_var__phv_fifo_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sop_in: 0x" << int_var__sop_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".eop_in: 0x" << int_var__eop_in << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sop_out: 0x" << int_var__sop_out << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".eop_out: 0x" << int_var__eop_out << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_error_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".icache0: 0x" << int_var__icache0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".icache1: 0x" << int_var__icache1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_pend_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".wr_id: 0x" << int_var__wr_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pending_table_write_valid0: 0x" << int_var__pending_table_write_valid0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pending_table_write_valid1: 0x" << int_var__pending_table_write_valid1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr7_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr6_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr5_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr4_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr3_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr2_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_gpr1_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".value: 0x" << int_var__value << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".c1: 0x" << int_var__c1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c2: 0x" << int_var__c2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c3: 0x" << int_var__c3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c4: 0x" << int_var__c4 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c5: 0x" << int_var__c5 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c6: 0x" << int_var__c6 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".c7: 0x" << int_var__c7 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_ctl_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".stopped: 0x" << int_var__stopped << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ex_valid: 0x" << int_var__ex_valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ex_execute: 0x" << int_var__ex_execute << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ex_pc: 0x" << int_var__ex_pc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".icache_state: 0x" << int_var__icache_state << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_cmd_fifo_depth: 0x" << int_var__phv_cmd_fifo_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_data_fifo_depth: 0x" << int_var__phv_data_fifo_depth << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stall_vector: 0x" << int_var__stall_vector << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_pc_mpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".inst: 0x" << int_var__inst << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_key_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".kd: 0x" << int_var__kd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_data_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".td: 0x" << int_var__td << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_table_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_pcie: 0x" << int_var__mpu_processing_table_pcie << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_sram: 0x" << int_var__mpu_processing_table_sram << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_id: 0x" << int_var__mpu_processing_table_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_pkt_id: 0x" << int_var__mpu_processing_pkt_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_latency: 0x" << int_var__mpu_processing_table_latency << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sta_tbl_addr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".in_mpu: 0x" << int_var__in_mpu << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_fence_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_tblwr_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_memwr_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_phvwr_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_hazard_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_icache_fill_stall_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_icache_miss_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_inst_executed_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_phv_executed_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_CNT_cycles_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".counter: 0x" << int_var__counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_count_stage_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".always_on: 0x" << int_var__always_on << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug: 0x" << int_var__debug << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".watch: 0x" << int_var__watch << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stop_on_saturate: 0x" << int_var__stop_on_saturate << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_mpu_run_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".start_pulse: 0x" << int_var__start_pulse << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stop_pulse: 0x" << int_var__stop_pulse << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".step_pulse: 0x" << int_var__step_pulse << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_mpu_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".stall_wb_full: 0x" << int_var__stall_wb_full << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".icache_table_id: 0x" << int_var__icache_table_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".max_inst_count: 0x" << int_var__max_inst_count << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".te_clean_wb_enable: 0x" << int_var__te_clean_wb_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_mpu_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable_stop: 0x" << int_var__enable_stop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_error_stop: 0x" << int_var__enable_error_stop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_pipe_freeze_on_stop: 0x" << int_var__enable_pipe_freeze_on_stop << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_sdp_ctl_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable_compress: 0x" << int_var__enable_compress << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mask_frame_size: 0x" << int_var__mask_frame_size << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_axi_attr_t::show() {
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
void cap_mpu_csr_icache_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".invalidate: 0x" << int_var__invalidate << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_trace_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".trace_enable: 0x" << int_var__trace_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_debug: 0x" << int_var__phv_debug << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_error: 0x" << int_var__phv_error << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".watch_enable: 0x" << int_var__watch_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".watch_pc: 0x" << int_var__watch_pc << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".table_and_key: 0x" << int_var__table_and_key << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".instructions: 0x" << int_var__instructions << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".wrap: 0x" << int_var__wrap << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rst: 0x" << int_var__rst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".base_addr: 0x" << int_var__base_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".buf_size: 0x" << int_var__buf_size << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_index: 0x" << int_var__debug_index << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_generation: 0x" << int_var__debug_generation << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_mpu_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    trace.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        trace[ii].show();
    }
    #endif
    
    icache.show();
    axi_attr.show();
    sdp_ctl.show();
    mpu_ctl.show();
    mpu_cfg.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mpu_run.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].show();
    }
    #endif
    
    count_stage.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_cycles.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_cycles[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_phv_executed.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_phv_executed[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_inst_executed.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_inst_executed[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_icache_miss.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_miss[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_icache_fill_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_fill_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_hazard_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_hazard_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_phvwr_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_phvwr_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_memwr_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_memwr_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_tblwr_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_tblwr_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_fence_stall.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        CNT_fence_stall[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_tbl_addr.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_tbl_addr[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_table.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_table[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_data.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_data[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_key.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_key[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_pc_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_pc_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_ctl_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_ctl_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr1_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr1_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr2_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr2_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr3_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr3_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr4_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr4_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr5_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr5_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr6_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr6_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr7_mpu.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr7_mpu[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_pend.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_pend[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_error.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].show();
    }
    #endif
    
    CNT_sdp.show();
    STA_live_sdp.show();
    STA_stg.show();
    mismatch.show();
    cfg_debug_port.show();
    cfg_sdp_mem.show();
    sta_sdp_mem.show();
    cfg_mpu0_icache_data0.show();
    sta_mpu0_icache_data0.show();
    cfg_mpu0_icache_data1.show();
    sta_mpu0_icache_data1.show();
    cfg_mpu1_icache_data0.show();
    sta_mpu1_icache_data0.show();
    cfg_mpu1_icache_data1.show();
    sta_mpu1_icache_data1.show();
    cfg_mpu2_icache_data0.show();
    sta_mpu2_icache_data0.show();
    cfg_mpu2_icache_data1.show();
    sta_mpu2_icache_data1.show();
    cfg_mpu3_icache_data0.show();
    sta_mpu3_icache_data0.show();
    cfg_mpu3_icache_data1.show();
    sta_mpu3_icache_data1.show();
    cfg_mpu0_icache_tag.show();
    sta_mpu0_icache_tag.show();
    cfg_mpu1_icache_tag.show();
    sta_mpu1_icache_tag.show();
    cfg_mpu2_icache_tag.show();
    sta_mpu2_icache_tag.show();
    cfg_mpu3_icache_tag.show();
    sta_mpu3_icache_tag.show();
    cfg_spare.show();
    sta_spare.show();
    csr_intr.show();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    tag_read.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        tag_read[ii].show();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    icache_sram_read.show(); // large_array
    #else
    for(int ii = 0; ii < 4; ii++) {
        icache_sram_read[ii].show();
    }
    #endif
    
    spr_reg.show();
    int_groups.show();
    int_err.show();
    int_info.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_mpu_csr_int_info_int_enable_clear_t::get_width() const {
    return cap_mpu_csr_int_info_int_enable_clear_t::s_get_width();

}

int cap_mpu_csr_int_info_int_test_set_t::get_width() const {
    return cap_mpu_csr_int_info_int_test_set_t::s_get_width();

}

int cap_mpu_csr_int_info_t::get_width() const {
    return cap_mpu_csr_int_info_t::s_get_width();

}

int cap_mpu_csr_int_err_int_enable_clear_t::get_width() const {
    return cap_mpu_csr_int_err_int_enable_clear_t::s_get_width();

}

int cap_mpu_csr_int_err_intreg_t::get_width() const {
    return cap_mpu_csr_int_err_intreg_t::s_get_width();

}

int cap_mpu_csr_int_err_t::get_width() const {
    return cap_mpu_csr_int_err_t::s_get_width();

}

int cap_mpu_csr_intreg_status_t::get_width() const {
    return cap_mpu_csr_intreg_status_t::s_get_width();

}

int cap_mpu_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_mpu_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_mpu_csr_intgrp_status_t::get_width() const {
    return cap_mpu_csr_intgrp_status_t::s_get_width();

}

int cap_mpu_csr_spr_reg_data_t::get_width() const {
    return cap_mpu_csr_spr_reg_data_t::s_get_width();

}

int cap_mpu_csr_spr_reg_t::get_width() const {
    return cap_mpu_csr_spr_reg_t::s_get_width();

}

int cap_mpu_csr_icache_sram_read_data_t::get_width() const {
    return cap_mpu_csr_icache_sram_read_data_t::s_get_width();

}

int cap_mpu_csr_icache_sram_read_t::get_width() const {
    return cap_mpu_csr_icache_sram_read_t::s_get_width();

}

int cap_mpu_csr_tag_read_data_t::get_width() const {
    return cap_mpu_csr_tag_read_data_t::s_get_width();

}

int cap_mpu_csr_tag_read_t::get_width() const {
    return cap_mpu_csr_tag_read_t::s_get_width();

}

int cap_mpu_csr_csr_intr_t::get_width() const {
    return cap_mpu_csr_csr_intr_t::s_get_width();

}

int cap_mpu_csr_sta_spare_t::get_width() const {
    return cap_mpu_csr_sta_spare_t::s_get_width();

}

int cap_mpu_csr_cfg_spare_t::get_width() const {
    return cap_mpu_csr_cfg_spare_t::s_get_width();

}

int cap_mpu_csr_sta_mpu3_icache_tag_t::get_width() const {
    return cap_mpu_csr_sta_mpu3_icache_tag_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu3_icache_tag_t::get_width() const {
    return cap_mpu_csr_cfg_mpu3_icache_tag_t::s_get_width();

}

int cap_mpu_csr_sta_mpu2_icache_tag_t::get_width() const {
    return cap_mpu_csr_sta_mpu2_icache_tag_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu2_icache_tag_t::get_width() const {
    return cap_mpu_csr_cfg_mpu2_icache_tag_t::s_get_width();

}

int cap_mpu_csr_sta_mpu1_icache_tag_t::get_width() const {
    return cap_mpu_csr_sta_mpu1_icache_tag_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu1_icache_tag_t::get_width() const {
    return cap_mpu_csr_cfg_mpu1_icache_tag_t::s_get_width();

}

int cap_mpu_csr_sta_mpu0_icache_tag_t::get_width() const {
    return cap_mpu_csr_sta_mpu0_icache_tag_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu0_icache_tag_t::get_width() const {
    return cap_mpu_csr_cfg_mpu0_icache_tag_t::s_get_width();

}

int cap_mpu_csr_sta_mpu3_icache_data1_t::get_width() const {
    return cap_mpu_csr_sta_mpu3_icache_data1_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu3_icache_data1_t::get_width() const {
    return cap_mpu_csr_cfg_mpu3_icache_data1_t::s_get_width();

}

int cap_mpu_csr_sta_mpu3_icache_data0_t::get_width() const {
    return cap_mpu_csr_sta_mpu3_icache_data0_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu3_icache_data0_t::get_width() const {
    return cap_mpu_csr_cfg_mpu3_icache_data0_t::s_get_width();

}

int cap_mpu_csr_sta_mpu2_icache_data1_t::get_width() const {
    return cap_mpu_csr_sta_mpu2_icache_data1_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu2_icache_data1_t::get_width() const {
    return cap_mpu_csr_cfg_mpu2_icache_data1_t::s_get_width();

}

int cap_mpu_csr_sta_mpu2_icache_data0_t::get_width() const {
    return cap_mpu_csr_sta_mpu2_icache_data0_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu2_icache_data0_t::get_width() const {
    return cap_mpu_csr_cfg_mpu2_icache_data0_t::s_get_width();

}

int cap_mpu_csr_sta_mpu1_icache_data1_t::get_width() const {
    return cap_mpu_csr_sta_mpu1_icache_data1_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu1_icache_data1_t::get_width() const {
    return cap_mpu_csr_cfg_mpu1_icache_data1_t::s_get_width();

}

int cap_mpu_csr_sta_mpu1_icache_data0_t::get_width() const {
    return cap_mpu_csr_sta_mpu1_icache_data0_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu1_icache_data0_t::get_width() const {
    return cap_mpu_csr_cfg_mpu1_icache_data0_t::s_get_width();

}

int cap_mpu_csr_sta_mpu0_icache_data1_t::get_width() const {
    return cap_mpu_csr_sta_mpu0_icache_data1_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu0_icache_data1_t::get_width() const {
    return cap_mpu_csr_cfg_mpu0_icache_data1_t::s_get_width();

}

int cap_mpu_csr_sta_mpu0_icache_data0_t::get_width() const {
    return cap_mpu_csr_sta_mpu0_icache_data0_t::s_get_width();

}

int cap_mpu_csr_cfg_mpu0_icache_data0_t::get_width() const {
    return cap_mpu_csr_cfg_mpu0_icache_data0_t::s_get_width();

}

int cap_mpu_csr_sta_sdp_mem_t::get_width() const {
    return cap_mpu_csr_sta_sdp_mem_t::s_get_width();

}

int cap_mpu_csr_cfg_sdp_mem_t::get_width() const {
    return cap_mpu_csr_cfg_sdp_mem_t::s_get_width();

}

int cap_mpu_csr_cfg_debug_port_t::get_width() const {
    return cap_mpu_csr_cfg_debug_port_t::s_get_width();

}

int cap_mpu_csr_mismatch_t::get_width() const {
    return cap_mpu_csr_mismatch_t::s_get_width();

}

int cap_mpu_csr_STA_stg_t::get_width() const {
    return cap_mpu_csr_STA_stg_t::s_get_width();

}

int cap_mpu_csr_STA_live_sdp_t::get_width() const {
    return cap_mpu_csr_STA_live_sdp_t::s_get_width();

}

int cap_mpu_csr_CNT_sdp_t::get_width() const {
    return cap_mpu_csr_CNT_sdp_t::s_get_width();

}

int cap_mpu_csr_sta_error_t::get_width() const {
    return cap_mpu_csr_sta_error_t::s_get_width();

}

int cap_mpu_csr_sta_pend_t::get_width() const {
    return cap_mpu_csr_sta_pend_t::s_get_width();

}

int cap_mpu_csr_sta_gpr7_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr7_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr6_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr6_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr5_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr5_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr4_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr4_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr3_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr3_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr2_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr2_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_gpr1_mpu_t::get_width() const {
    return cap_mpu_csr_sta_gpr1_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_mpu_t::get_width() const {
    return cap_mpu_csr_sta_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_ctl_mpu_t::get_width() const {
    return cap_mpu_csr_sta_ctl_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_pc_mpu_t::get_width() const {
    return cap_mpu_csr_sta_pc_mpu_t::s_get_width();

}

int cap_mpu_csr_sta_key_t::get_width() const {
    return cap_mpu_csr_sta_key_t::s_get_width();

}

int cap_mpu_csr_sta_data_t::get_width() const {
    return cap_mpu_csr_sta_data_t::s_get_width();

}

int cap_mpu_csr_sta_table_t::get_width() const {
    return cap_mpu_csr_sta_table_t::s_get_width();

}

int cap_mpu_csr_sta_tbl_addr_t::get_width() const {
    return cap_mpu_csr_sta_tbl_addr_t::s_get_width();

}

int cap_mpu_csr_CNT_fence_stall_t::get_width() const {
    return cap_mpu_csr_CNT_fence_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_tblwr_stall_t::get_width() const {
    return cap_mpu_csr_CNT_tblwr_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_memwr_stall_t::get_width() const {
    return cap_mpu_csr_CNT_memwr_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_phvwr_stall_t::get_width() const {
    return cap_mpu_csr_CNT_phvwr_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_hazard_stall_t::get_width() const {
    return cap_mpu_csr_CNT_hazard_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_icache_fill_stall_t::get_width() const {
    return cap_mpu_csr_CNT_icache_fill_stall_t::s_get_width();

}

int cap_mpu_csr_CNT_icache_miss_t::get_width() const {
    return cap_mpu_csr_CNT_icache_miss_t::s_get_width();

}

int cap_mpu_csr_CNT_inst_executed_t::get_width() const {
    return cap_mpu_csr_CNT_inst_executed_t::s_get_width();

}

int cap_mpu_csr_CNT_phv_executed_t::get_width() const {
    return cap_mpu_csr_CNT_phv_executed_t::s_get_width();

}

int cap_mpu_csr_CNT_cycles_t::get_width() const {
    return cap_mpu_csr_CNT_cycles_t::s_get_width();

}

int cap_mpu_csr_count_stage_t::get_width() const {
    return cap_mpu_csr_count_stage_t::s_get_width();

}

int cap_mpu_csr_mpu_run_t::get_width() const {
    return cap_mpu_csr_mpu_run_t::s_get_width();

}

int cap_mpu_csr_mpu_cfg_t::get_width() const {
    return cap_mpu_csr_mpu_cfg_t::s_get_width();

}

int cap_mpu_csr_mpu_ctl_t::get_width() const {
    return cap_mpu_csr_mpu_ctl_t::s_get_width();

}

int cap_mpu_csr_sdp_ctl_t::get_width() const {
    return cap_mpu_csr_sdp_ctl_t::s_get_width();

}

int cap_mpu_csr_axi_attr_t::get_width() const {
    return cap_mpu_csr_axi_attr_t::s_get_width();

}

int cap_mpu_csr_icache_t::get_width() const {
    return cap_mpu_csr_icache_t::s_get_width();

}

int cap_mpu_csr_trace_t::get_width() const {
    return cap_mpu_csr_trace_t::s_get_width();

}

int cap_mpu_csr_base_t::get_width() const {
    return cap_mpu_csr_base_t::s_get_width();

}

int cap_mpu_csr_t::get_width() const {
    return cap_mpu_csr_t::s_get_width();

}

int cap_mpu_csr_int_info_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // trace_full_0_enable
    _count += 1; // trace_full_1_enable
    _count += 1; // trace_full_2_enable
    _count += 1; // trace_full_3_enable
    _count += 1; // mpu_stop_0_enable
    _count += 1; // mpu_stop_1_enable
    _count += 1; // mpu_stop_2_enable
    _count += 1; // mpu_stop_3_enable
    return _count;
}

int cap_mpu_csr_int_info_int_test_set_t::s_get_width() {
    int _count = 0;

    _count += 1; // trace_full_0_interrupt
    _count += 1; // trace_full_1_interrupt
    _count += 1; // trace_full_2_interrupt
    _count += 1; // trace_full_3_interrupt
    _count += 1; // mpu_stop_0_interrupt
    _count += 1; // mpu_stop_1_interrupt
    _count += 1; // mpu_stop_2_interrupt
    _count += 1; // mpu_stop_3_interrupt
    return _count;
}

int cap_mpu_csr_int_info_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_int_info_int_test_set_t::s_get_width(); // intreg
    _count += cap_mpu_csr_int_info_int_test_set_t::s_get_width(); // int_test_set
    _count += cap_mpu_csr_int_info_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mpu_csr_int_info_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mpu_csr_int_err_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // results_mismatch_enable
    _count += 1; // sdp_mem_uncorrectable_enable
    _count += 1; // sdp_mem_correctable_enable
    _count += 1; // illegal_op_0_enable
    _count += 1; // illegal_op_1_enable
    _count += 1; // illegal_op_2_enable
    _count += 1; // illegal_op_3_enable
    _count += 1; // max_inst_0_enable
    _count += 1; // max_inst_1_enable
    _count += 1; // max_inst_2_enable
    _count += 1; // max_inst_3_enable
    _count += 1; // phvwr_0_enable
    _count += 1; // phvwr_1_enable
    _count += 1; // phvwr_2_enable
    _count += 1; // phvwr_3_enable
    _count += 1; // write_err_0_enable
    _count += 1; // write_err_1_enable
    _count += 1; // write_err_2_enable
    _count += 1; // write_err_3_enable
    _count += 1; // cache_axi_0_enable
    _count += 1; // cache_axi_1_enable
    _count += 1; // cache_axi_2_enable
    _count += 1; // cache_axi_3_enable
    _count += 1; // cache_parity_0_enable
    _count += 1; // cache_parity_1_enable
    _count += 1; // cache_parity_2_enable
    _count += 1; // cache_parity_3_enable
    return _count;
}

int cap_mpu_csr_int_err_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // results_mismatch_interrupt
    _count += 1; // sdp_mem_uncorrectable_interrupt
    _count += 1; // sdp_mem_correctable_interrupt
    _count += 1; // illegal_op_0_interrupt
    _count += 1; // illegal_op_1_interrupt
    _count += 1; // illegal_op_2_interrupt
    _count += 1; // illegal_op_3_interrupt
    _count += 1; // max_inst_0_interrupt
    _count += 1; // max_inst_1_interrupt
    _count += 1; // max_inst_2_interrupt
    _count += 1; // max_inst_3_interrupt
    _count += 1; // phvwr_0_interrupt
    _count += 1; // phvwr_1_interrupt
    _count += 1; // phvwr_2_interrupt
    _count += 1; // phvwr_3_interrupt
    _count += 1; // write_err_0_interrupt
    _count += 1; // write_err_1_interrupt
    _count += 1; // write_err_2_interrupt
    _count += 1; // write_err_3_interrupt
    _count += 1; // cache_axi_0_interrupt
    _count += 1; // cache_axi_1_interrupt
    _count += 1; // cache_axi_2_interrupt
    _count += 1; // cache_axi_3_interrupt
    _count += 1; // cache_parity_0_interrupt
    _count += 1; // cache_parity_1_interrupt
    _count += 1; // cache_parity_2_interrupt
    _count += 1; // cache_parity_3_interrupt
    return _count;
}

int cap_mpu_csr_int_err_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_int_err_intreg_t::s_get_width(); // intreg
    _count += cap_mpu_csr_int_err_intreg_t::s_get_width(); // int_test_set
    _count += cap_mpu_csr_int_err_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_mpu_csr_int_err_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_mpu_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_err_interrupt
    _count += 1; // int_info_interrupt
    return _count;
}

int cap_mpu_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // int_err_enable
    _count += 1; // int_info_enable
    return _count;
}

int cap_mpu_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_mpu_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_mpu_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_mpu_csr_spr_reg_data_t::s_get_width() {
    int _count = 0;

    _count += 1; // value
    return _count;
}

int cap_mpu_csr_spr_reg_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_spr_reg_data_t::s_get_width(); // data
    return _count;
}

int cap_mpu_csr_icache_sram_read_data_t::s_get_width() {
    int _count = 0;

    _count += 1; // value
    return _count;
}

int cap_mpu_csr_icache_sram_read_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_icache_sram_read_data_t::s_get_width(); // data
    return _count;
}

int cap_mpu_csr_tag_read_data_t::s_get_width() {
    int _count = 0;

    _count += 1; // value
    return _count;
}

int cap_mpu_csr_tag_read_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_tag_read_data_t::s_get_width(); // data
    return _count;
}

int cap_mpu_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_mpu_csr_sta_spare_t::s_get_width() {
    int _count = 0;

    _count += 32; // spare
    return _count;
}

int cap_mpu_csr_cfg_spare_t::s_get_width() {
    int _count = 0;

    _count += 32; // spare
    return _count;
}

int cap_mpu_csr_sta_mpu3_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mpu_csr_cfg_mpu3_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu2_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mpu_csr_cfg_mpu2_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu1_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mpu_csr_cfg_mpu1_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu0_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_mpu_csr_cfg_mpu0_icache_tag_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu3_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu3_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu3_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu3_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu2_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu2_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu2_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu2_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu1_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu1_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu1_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu1_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu0_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu0_icache_data1_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_mpu0_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // parity_error
    _count += 8; // parity_error_addr
    return _count;
}

int cap_mpu_csr_cfg_mpu0_icache_data0_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    return _count;
}

int cap_mpu_csr_sta_sdp_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    _count += 1; // correctable
    _count += 1; // uncorrectable
    _count += 27; // syndrome
    _count += 7; // addr
    return _count;
}

int cap_mpu_csr_cfg_sdp_mem_t::s_get_width() {
    int _count = 0;

    _count += 1; // bist_run
    _count += 1; // ecc_disable_det
    _count += 1; // ecc_disable_cor
    return _count;
}

int cap_mpu_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 3; // select
    return _count;
}

int cap_mpu_csr_mismatch_t::s_get_width() {
    int _count = 0;

    _count += 4; // update_result_count
    _count += 4; // sdp_result_count
    _count += 7; // pkt_id
    return _count;
}

int cap_mpu_csr_STA_stg_t::s_get_width() {
    int _count = 0;

    _count += 1; // sdp_srdy_in
    _count += 1; // sdp_drdy_in
    _count += 1; // srdy_out
    _count += 1; // drdy_out
    _count += 1; // srdy
    _count += 1; // drdy
    _count += 16; // te_valid
    _count += 4; // mpu_processing
    return _count;
}

int cap_mpu_csr_STA_live_sdp_t::s_get_width() {
    int _count = 0;

    _count += 4; // update_empty
    _count += 32; // update_pkt_id
    _count += 8; // phv_pkt_id
    _count += 12; // phv_update_valid
    _count += 4; // update_result_count
    _count += 4; // staging_full
    _count += 7; // wptr
    _count += 7; // rptr
    return _count;
}

int cap_mpu_csr_CNT_sdp_t::s_get_width() {
    int _count = 0;

    _count += 8; // phv_fifo_depth
    _count += 16; // sop_in
    _count += 16; // eop_in
    _count += 16; // sop_out
    _count += 16; // eop_out
    return _count;
}

int cap_mpu_csr_sta_error_t::s_get_width() {
    int _count = 0;

    _count += 1; // icache0
    _count += 1; // icache1
    return _count;
}

int cap_mpu_csr_sta_pend_t::s_get_width() {
    int _count = 0;

    _count += 32; // wr_id
    _count += 1; // pending_table_write_valid0
    _count += 1; // pending_table_write_valid1
    return _count;
}

int cap_mpu_csr_sta_gpr7_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr6_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr5_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr4_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr3_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr2_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_gpr1_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // value
    return _count;
}

int cap_mpu_csr_sta_mpu_t::s_get_width() {
    int _count = 0;

    _count += 1; // c1
    _count += 1; // c2
    _count += 1; // c3
    _count += 1; // c4
    _count += 1; // c5
    _count += 1; // c6
    _count += 1; // c7
    return _count;
}

int cap_mpu_csr_sta_ctl_mpu_t::s_get_width() {
    int _count = 0;

    _count += 1; // stopped
    _count += 1; // ex_valid
    _count += 1; // ex_execute
    _count += 31; // ex_pc
    _count += 3; // icache_state
    _count += 4; // phv_cmd_fifo_depth
    _count += 5; // phv_data_fifo_depth
    _count += 4; // stall_vector
    return _count;
}

int cap_mpu_csr_sta_pc_mpu_t::s_get_width() {
    int _count = 0;

    _count += 64; // inst
    return _count;
}

int cap_mpu_csr_sta_key_t::s_get_width() {
    int _count = 0;

    _count += 512; // kd
    return _count;
}

int cap_mpu_csr_sta_data_t::s_get_width() {
    int _count = 0;

    _count += 512; // td
    return _count;
}

int cap_mpu_csr_sta_table_t::s_get_width() {
    int _count = 0;

    _count += 1; // mpu_processing_table_pcie
    _count += 1; // mpu_processing_table_sram
    _count += 4; // mpu_processing_table_id
    _count += 8; // mpu_processing_pkt_id
    _count += 16; // mpu_processing_table_latency
    return _count;
}

int cap_mpu_csr_sta_tbl_addr_t::s_get_width() {
    int _count = 0;

    _count += 64; // in_mpu
    return _count;
}

int cap_mpu_csr_CNT_fence_stall_t::s_get_width() {
    int _count = 0;

    _count += 16; // counter
    return _count;
}

int cap_mpu_csr_CNT_tblwr_stall_t::s_get_width() {
    int _count = 0;

    _count += 20; // counter
    return _count;
}

int cap_mpu_csr_CNT_memwr_stall_t::s_get_width() {
    int _count = 0;

    _count += 16; // counter
    return _count;
}

int cap_mpu_csr_CNT_phvwr_stall_t::s_get_width() {
    int _count = 0;

    _count += 20; // counter
    return _count;
}

int cap_mpu_csr_CNT_hazard_stall_t::s_get_width() {
    int _count = 0;

    _count += 20; // counter
    return _count;
}

int cap_mpu_csr_CNT_icache_fill_stall_t::s_get_width() {
    int _count = 0;

    _count += 24; // counter
    return _count;
}

int cap_mpu_csr_CNT_icache_miss_t::s_get_width() {
    int _count = 0;

    _count += 16; // counter
    return _count;
}

int cap_mpu_csr_CNT_inst_executed_t::s_get_width() {
    int _count = 0;

    _count += 25; // counter
    return _count;
}

int cap_mpu_csr_CNT_phv_executed_t::s_get_width() {
    int _count = 0;

    _count += 20; // counter
    return _count;
}

int cap_mpu_csr_CNT_cycles_t::s_get_width() {
    int _count = 0;

    _count += 26; // counter
    return _count;
}

int cap_mpu_csr_count_stage_t::s_get_width() {
    int _count = 0;

    _count += 1; // always_on
    _count += 1; // debug
    _count += 1; // watch
    _count += 1; // stop_on_saturate
    return _count;
}

int cap_mpu_csr_mpu_run_t::s_get_width() {
    int _count = 0;

    _count += 1; // start_pulse
    _count += 1; // stop_pulse
    _count += 1; // step_pulse
    return _count;
}

int cap_mpu_csr_mpu_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // stall_wb_full
    _count += 4; // icache_table_id
    _count += 16; // max_inst_count
    _count += 1; // te_clean_wb_enable
    return _count;
}

int cap_mpu_csr_mpu_ctl_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable_stop
    _count += 1; // enable_error_stop
    _count += 1; // enable_pipe_freeze_on_stop
    return _count;
}

int cap_mpu_csr_sdp_ctl_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable_compress
    _count += 1; // mask_frame_size
    return _count;
}

int cap_mpu_csr_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // arcache
    _count += 4; // awcache
    _count += 3; // prot
    _count += 4; // qos
    _count += 1; // lock
    return _count;
}

int cap_mpu_csr_icache_t::s_get_width() {
    int _count = 0;

    _count += 1; // invalidate
    return _count;
}

int cap_mpu_csr_trace_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // trace_enable
    _count += 1; // phv_debug
    _count += 1; // phv_error
    _count += 1; // watch_enable
    _count += 31; // watch_pc
    _count += 1; // table_and_key
    _count += 1; // instructions
    _count += 1; // wrap
    _count += 1; // rst
    _count += 28; // base_addr
    _count += 5; // buf_size
    _count += 20; // debug_index
    _count += 1; // debug_generation
    return _count;
}

int cap_mpu_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_mpu_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_mpu_csr_base_t::s_get_width(); // base
    _count += (cap_mpu_csr_trace_t::s_get_width() * 4); // trace
    _count += cap_mpu_csr_icache_t::s_get_width(); // icache
    _count += cap_mpu_csr_axi_attr_t::s_get_width(); // axi_attr
    _count += cap_mpu_csr_sdp_ctl_t::s_get_width(); // sdp_ctl
    _count += cap_mpu_csr_mpu_ctl_t::s_get_width(); // mpu_ctl
    _count += cap_mpu_csr_mpu_cfg_t::s_get_width(); // mpu_cfg
    _count += (cap_mpu_csr_mpu_run_t::s_get_width() * 4); // mpu_run
    _count += cap_mpu_csr_count_stage_t::s_get_width(); // count_stage
    _count += (cap_mpu_csr_CNT_cycles_t::s_get_width() * 4); // CNT_cycles
    _count += (cap_mpu_csr_CNT_phv_executed_t::s_get_width() * 4); // CNT_phv_executed
    _count += (cap_mpu_csr_CNT_inst_executed_t::s_get_width() * 4); // CNT_inst_executed
    _count += (cap_mpu_csr_CNT_icache_miss_t::s_get_width() * 4); // CNT_icache_miss
    _count += (cap_mpu_csr_CNT_icache_fill_stall_t::s_get_width() * 4); // CNT_icache_fill_stall
    _count += (cap_mpu_csr_CNT_hazard_stall_t::s_get_width() * 4); // CNT_hazard_stall
    _count += (cap_mpu_csr_CNT_phvwr_stall_t::s_get_width() * 4); // CNT_phvwr_stall
    _count += (cap_mpu_csr_CNT_memwr_stall_t::s_get_width() * 4); // CNT_memwr_stall
    _count += (cap_mpu_csr_CNT_tblwr_stall_t::s_get_width() * 4); // CNT_tblwr_stall
    _count += (cap_mpu_csr_CNT_fence_stall_t::s_get_width() * 4); // CNT_fence_stall
    _count += (cap_mpu_csr_sta_tbl_addr_t::s_get_width() * 4); // sta_tbl_addr
    _count += (cap_mpu_csr_sta_table_t::s_get_width() * 4); // sta_table
    _count += (cap_mpu_csr_sta_data_t::s_get_width() * 4); // sta_data
    _count += (cap_mpu_csr_sta_key_t::s_get_width() * 4); // sta_key
    _count += (cap_mpu_csr_sta_pc_mpu_t::s_get_width() * 4); // sta_pc_mpu
    _count += (cap_mpu_csr_sta_ctl_mpu_t::s_get_width() * 4); // sta_ctl_mpu
    _count += (cap_mpu_csr_sta_mpu_t::s_get_width() * 4); // sta_mpu
    _count += (cap_mpu_csr_sta_gpr1_mpu_t::s_get_width() * 4); // sta_gpr1_mpu
    _count += (cap_mpu_csr_sta_gpr2_mpu_t::s_get_width() * 4); // sta_gpr2_mpu
    _count += (cap_mpu_csr_sta_gpr3_mpu_t::s_get_width() * 4); // sta_gpr3_mpu
    _count += (cap_mpu_csr_sta_gpr4_mpu_t::s_get_width() * 4); // sta_gpr4_mpu
    _count += (cap_mpu_csr_sta_gpr5_mpu_t::s_get_width() * 4); // sta_gpr5_mpu
    _count += (cap_mpu_csr_sta_gpr6_mpu_t::s_get_width() * 4); // sta_gpr6_mpu
    _count += (cap_mpu_csr_sta_gpr7_mpu_t::s_get_width() * 4); // sta_gpr7_mpu
    _count += (cap_mpu_csr_sta_pend_t::s_get_width() * 4); // sta_pend
    _count += (cap_mpu_csr_sta_error_t::s_get_width() * 4); // sta_error
    _count += cap_mpu_csr_CNT_sdp_t::s_get_width(); // CNT_sdp
    _count += cap_mpu_csr_STA_live_sdp_t::s_get_width(); // STA_live_sdp
    _count += cap_mpu_csr_STA_stg_t::s_get_width(); // STA_stg
    _count += cap_mpu_csr_mismatch_t::s_get_width(); // mismatch
    _count += cap_mpu_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_mpu_csr_cfg_sdp_mem_t::s_get_width(); // cfg_sdp_mem
    _count += cap_mpu_csr_sta_sdp_mem_t::s_get_width(); // sta_sdp_mem
    _count += cap_mpu_csr_cfg_mpu0_icache_data0_t::s_get_width(); // cfg_mpu0_icache_data0
    _count += cap_mpu_csr_sta_mpu0_icache_data0_t::s_get_width(); // sta_mpu0_icache_data0
    _count += cap_mpu_csr_cfg_mpu0_icache_data1_t::s_get_width(); // cfg_mpu0_icache_data1
    _count += cap_mpu_csr_sta_mpu0_icache_data1_t::s_get_width(); // sta_mpu0_icache_data1
    _count += cap_mpu_csr_cfg_mpu1_icache_data0_t::s_get_width(); // cfg_mpu1_icache_data0
    _count += cap_mpu_csr_sta_mpu1_icache_data0_t::s_get_width(); // sta_mpu1_icache_data0
    _count += cap_mpu_csr_cfg_mpu1_icache_data1_t::s_get_width(); // cfg_mpu1_icache_data1
    _count += cap_mpu_csr_sta_mpu1_icache_data1_t::s_get_width(); // sta_mpu1_icache_data1
    _count += cap_mpu_csr_cfg_mpu2_icache_data0_t::s_get_width(); // cfg_mpu2_icache_data0
    _count += cap_mpu_csr_sta_mpu2_icache_data0_t::s_get_width(); // sta_mpu2_icache_data0
    _count += cap_mpu_csr_cfg_mpu2_icache_data1_t::s_get_width(); // cfg_mpu2_icache_data1
    _count += cap_mpu_csr_sta_mpu2_icache_data1_t::s_get_width(); // sta_mpu2_icache_data1
    _count += cap_mpu_csr_cfg_mpu3_icache_data0_t::s_get_width(); // cfg_mpu3_icache_data0
    _count += cap_mpu_csr_sta_mpu3_icache_data0_t::s_get_width(); // sta_mpu3_icache_data0
    _count += cap_mpu_csr_cfg_mpu3_icache_data1_t::s_get_width(); // cfg_mpu3_icache_data1
    _count += cap_mpu_csr_sta_mpu3_icache_data1_t::s_get_width(); // sta_mpu3_icache_data1
    _count += cap_mpu_csr_cfg_mpu0_icache_tag_t::s_get_width(); // cfg_mpu0_icache_tag
    _count += cap_mpu_csr_sta_mpu0_icache_tag_t::s_get_width(); // sta_mpu0_icache_tag
    _count += cap_mpu_csr_cfg_mpu1_icache_tag_t::s_get_width(); // cfg_mpu1_icache_tag
    _count += cap_mpu_csr_sta_mpu1_icache_tag_t::s_get_width(); // sta_mpu1_icache_tag
    _count += cap_mpu_csr_cfg_mpu2_icache_tag_t::s_get_width(); // cfg_mpu2_icache_tag
    _count += cap_mpu_csr_sta_mpu2_icache_tag_t::s_get_width(); // sta_mpu2_icache_tag
    _count += cap_mpu_csr_cfg_mpu3_icache_tag_t::s_get_width(); // cfg_mpu3_icache_tag
    _count += cap_mpu_csr_sta_mpu3_icache_tag_t::s_get_width(); // sta_mpu3_icache_tag
    _count += cap_mpu_csr_cfg_spare_t::s_get_width(); // cfg_spare
    _count += cap_mpu_csr_sta_spare_t::s_get_width(); // sta_spare
    _count += cap_mpu_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += (cap_mpu_csr_tag_read_t::s_get_width() * 4); // tag_read
    _count += (cap_mpu_csr_icache_sram_read_t::s_get_width() * 4); // icache_sram_read
    _count += cap_mpu_csr_spr_reg_t::s_get_width(); // spr_reg
    _count += cap_mpu_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_mpu_csr_int_err_t::s_get_width(); // int_err
    _count += cap_mpu_csr_int_info_t::s_get_width(); // int_info
    return _count;
}

void cap_mpu_csr_int_info_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__trace_full_0_enable = _val.convert_to< trace_full_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_1_enable = _val.convert_to< trace_full_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_2_enable = _val.convert_to< trace_full_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_3_enable = _val.convert_to< trace_full_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_0_enable = _val.convert_to< mpu_stop_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_1_enable = _val.convert_to< mpu_stop_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_2_enable = _val.convert_to< mpu_stop_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_3_enable = _val.convert_to< mpu_stop_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_int_info_int_test_set_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__trace_full_0_interrupt = _val.convert_to< trace_full_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_1_interrupt = _val.convert_to< trace_full_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_2_interrupt = _val.convert_to< trace_full_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_full_3_interrupt = _val.convert_to< trace_full_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_0_interrupt = _val.convert_to< mpu_stop_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_1_interrupt = _val.convert_to< mpu_stop_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_2_interrupt = _val.convert_to< mpu_stop_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_stop_3_interrupt = _val.convert_to< mpu_stop_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_int_info_t::all(const cpp_int & in_val) {
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

void cap_mpu_csr_int_err_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__results_mismatch_enable = _val.convert_to< results_mismatch_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sdp_mem_uncorrectable_enable = _val.convert_to< sdp_mem_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sdp_mem_correctable_enable = _val.convert_to< sdp_mem_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_0_enable = _val.convert_to< illegal_op_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_1_enable = _val.convert_to< illegal_op_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_2_enable = _val.convert_to< illegal_op_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_3_enable = _val.convert_to< illegal_op_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_0_enable = _val.convert_to< max_inst_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_1_enable = _val.convert_to< max_inst_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_2_enable = _val.convert_to< max_inst_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_3_enable = _val.convert_to< max_inst_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_0_enable = _val.convert_to< phvwr_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_1_enable = _val.convert_to< phvwr_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_2_enable = _val.convert_to< phvwr_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_3_enable = _val.convert_to< phvwr_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_0_enable = _val.convert_to< write_err_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_1_enable = _val.convert_to< write_err_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_2_enable = _val.convert_to< write_err_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_3_enable = _val.convert_to< write_err_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_0_enable = _val.convert_to< cache_axi_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_1_enable = _val.convert_to< cache_axi_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_2_enable = _val.convert_to< cache_axi_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_3_enable = _val.convert_to< cache_axi_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_0_enable = _val.convert_to< cache_parity_0_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_1_enable = _val.convert_to< cache_parity_1_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_2_enable = _val.convert_to< cache_parity_2_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_3_enable = _val.convert_to< cache_parity_3_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_int_err_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__results_mismatch_interrupt = _val.convert_to< results_mismatch_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sdp_mem_uncorrectable_interrupt = _val.convert_to< sdp_mem_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sdp_mem_correctable_interrupt = _val.convert_to< sdp_mem_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_0_interrupt = _val.convert_to< illegal_op_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_1_interrupt = _val.convert_to< illegal_op_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_2_interrupt = _val.convert_to< illegal_op_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__illegal_op_3_interrupt = _val.convert_to< illegal_op_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_0_interrupt = _val.convert_to< max_inst_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_1_interrupt = _val.convert_to< max_inst_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_2_interrupt = _val.convert_to< max_inst_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__max_inst_3_interrupt = _val.convert_to< max_inst_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_0_interrupt = _val.convert_to< phvwr_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_1_interrupt = _val.convert_to< phvwr_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_2_interrupt = _val.convert_to< phvwr_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phvwr_3_interrupt = _val.convert_to< phvwr_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_0_interrupt = _val.convert_to< write_err_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_1_interrupt = _val.convert_to< write_err_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_2_interrupt = _val.convert_to< write_err_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__write_err_3_interrupt = _val.convert_to< write_err_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_0_interrupt = _val.convert_to< cache_axi_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_1_interrupt = _val.convert_to< cache_axi_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_2_interrupt = _val.convert_to< cache_axi_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_axi_3_interrupt = _val.convert_to< cache_axi_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_0_interrupt = _val.convert_to< cache_parity_0_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_1_interrupt = _val.convert_to< cache_parity_1_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_2_interrupt = _val.convert_to< cache_parity_2_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache_parity_3_interrupt = _val.convert_to< cache_parity_3_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_int_err_t::all(const cpp_int & in_val) {
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

void cap_mpu_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_err_interrupt = _val.convert_to< int_err_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_info_interrupt = _val.convert_to< int_info_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__int_err_enable = _val.convert_to< int_err_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__int_info_enable = _val.convert_to< int_info_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_mpu_csr_spr_reg_data_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_spr_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    data.all( _val);
    _val = _val >> data.get_width(); 
}

void cap_mpu_csr_icache_sram_read_data_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_icache_sram_read_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    data.all( _val);
    _val = _val >> data.get_width(); 
}

void cap_mpu_csr_tag_read_data_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_tag_read_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    data.all( _val);
    _val = _val >> data.get_width(); 
}

void cap_mpu_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_spare_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mpu_csr_cfg_spare_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mpu_csr_sta_mpu3_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_cfg_mpu3_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu2_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_cfg_mpu2_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu1_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_cfg_mpu1_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu0_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_cfg_mpu0_icache_tag_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu3_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu3_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu3_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu3_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu2_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu2_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu2_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu2_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu1_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu1_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu1_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu1_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu0_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu0_icache_data1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_mpu0_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_mpu_csr_cfg_mpu0_icache_data0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_sdp_mem_t::all(const cpp_int & in_val) {
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
    _val = _val >> 27;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_mpu_csr_cfg_sdp_mem_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_cfg_debug_port_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__select = _val.convert_to< select_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_mpu_csr_mismatch_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__update_result_count = _val.convert_to< update_result_count_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sdp_result_count = _val.convert_to< sdp_result_count_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__pkt_id = _val.convert_to< pkt_id_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_mpu_csr_STA_stg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sdp_srdy_in = _val.convert_to< sdp_srdy_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sdp_drdy_in = _val.convert_to< sdp_drdy_in_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__srdy_out = _val.convert_to< srdy_out_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__drdy_out = _val.convert_to< drdy_out_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__srdy = _val.convert_to< srdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__drdy = _val.convert_to< drdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__te_valid = _val.convert_to< te_valid_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__mpu_processing = _val.convert_to< mpu_processing_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_mpu_csr_STA_live_sdp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__update_empty = _val.convert_to< update_empty_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__update_pkt_id = _val.convert_to< update_pkt_id_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__phv_pkt_id = _val.convert_to< phv_pkt_id_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phv_update_valid = _val.convert_to< phv_update_valid_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__update_result_count = _val.convert_to< update_result_count_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__staging_full = _val.convert_to< staging_full_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__wptr = _val.convert_to< wptr_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__rptr = _val.convert_to< rptr_cpp_int_t >()  ;
    _val = _val >> 7;
    
}

void cap_mpu_csr_CNT_sdp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__phv_fifo_depth = _val.convert_to< phv_fifo_depth_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__sop_in = _val.convert_to< sop_in_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__eop_in = _val.convert_to< eop_in_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__sop_out = _val.convert_to< sop_out_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__eop_out = _val.convert_to< eop_out_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mpu_csr_sta_error_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__icache0 = _val.convert_to< icache0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__icache1 = _val.convert_to< icache1_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_pend_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__wr_id = _val.convert_to< wr_id_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__pending_table_write_valid0 = _val.convert_to< pending_table_write_valid0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pending_table_write_valid1 = _val.convert_to< pending_table_write_valid1_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_gpr7_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr6_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr5_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr4_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr3_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr2_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_gpr1_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__value = _val.convert_to< value_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__c1 = _val.convert_to< c1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c2 = _val.convert_to< c2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c3 = _val.convert_to< c3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c4 = _val.convert_to< c4_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c5 = _val.convert_to< c5_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c6 = _val.convert_to< c6_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__c7 = _val.convert_to< c7_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sta_ctl_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__stopped = _val.convert_to< stopped_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ex_valid = _val.convert_to< ex_valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ex_execute = _val.convert_to< ex_execute_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ex_pc = _val.convert_to< ex_pc_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__icache_state = _val.convert_to< icache_state_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phv_cmd_fifo_depth = _val.convert_to< phv_cmd_fifo_depth_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phv_data_fifo_depth = _val.convert_to< phv_data_fifo_depth_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__stall_vector = _val.convert_to< stall_vector_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_mpu_csr_sta_pc_mpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__inst = _val.convert_to< inst_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_sta_key_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__kd = _val.convert_to< kd_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_mpu_csr_sta_data_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__td = _val.convert_to< td_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_mpu_csr_sta_table_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mpu_processing_table_pcie = _val.convert_to< mpu_processing_table_pcie_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_processing_table_sram = _val.convert_to< mpu_processing_table_sram_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mpu_processing_table_id = _val.convert_to< mpu_processing_table_id_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__mpu_processing_pkt_id = _val.convert_to< mpu_processing_pkt_id_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__mpu_processing_table_latency = _val.convert_to< mpu_processing_table_latency_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mpu_csr_sta_tbl_addr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__in_mpu = _val.convert_to< in_mpu_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_mpu_csr_CNT_fence_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mpu_csr_CNT_tblwr_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 20;
    
}

void cap_mpu_csr_CNT_memwr_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mpu_csr_CNT_phvwr_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 20;
    
}

void cap_mpu_csr_CNT_hazard_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 20;
    
}

void cap_mpu_csr_CNT_icache_fill_stall_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 24;
    
}

void cap_mpu_csr_CNT_icache_miss_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 16;
    
}

void cap_mpu_csr_CNT_inst_executed_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 25;
    
}

void cap_mpu_csr_CNT_phv_executed_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 20;
    
}

void cap_mpu_csr_CNT_cycles_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__counter = _val.convert_to< counter_cpp_int_t >()  ;
    _val = _val >> 26;
    
}

void cap_mpu_csr_count_stage_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__always_on = _val.convert_to< always_on_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__debug = _val.convert_to< debug_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__watch = _val.convert_to< watch_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stop_on_saturate = _val.convert_to< stop_on_saturate_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_mpu_run_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__start_pulse = _val.convert_to< start_pulse_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__stop_pulse = _val.convert_to< stop_pulse_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__step_pulse = _val.convert_to< step_pulse_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_mpu_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__stall_wb_full = _val.convert_to< stall_wb_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__icache_table_id = _val.convert_to< icache_table_id_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__max_inst_count = _val.convert_to< max_inst_count_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__te_clean_wb_enable = _val.convert_to< te_clean_wb_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_mpu_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable_stop = _val.convert_to< enable_stop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__enable_error_stop = _val.convert_to< enable_error_stop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__enable_pipe_freeze_on_stop = _val.convert_to< enable_pipe_freeze_on_stop_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_sdp_ctl_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable_compress = _val.convert_to< enable_compress_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mask_frame_size = _val.convert_to< mask_frame_size_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_axi_attr_t::all(const cpp_int & in_val) {
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

void cap_mpu_csr_icache_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__invalidate = _val.convert_to< invalidate_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_trace_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__enable = _val.convert_to< enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__trace_enable = _val.convert_to< trace_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_debug = _val.convert_to< phv_debug_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phv_error = _val.convert_to< phv_error_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__watch_enable = _val.convert_to< watch_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__watch_pc = _val.convert_to< watch_pc_cpp_int_t >()  ;
    _val = _val >> 31;
    
    int_var__table_and_key = _val.convert_to< table_and_key_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__instructions = _val.convert_to< instructions_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__wrap = _val.convert_to< wrap_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rst = _val.convert_to< rst_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__base_addr = _val.convert_to< base_addr_cpp_int_t >()  ;
    _val = _val >> 28;
    
    int_var__buf_size = _val.convert_to< buf_size_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__debug_index = _val.convert_to< debug_index_cpp_int_t >()  ;
    _val = _val >> 20;
    
    int_var__debug_generation = _val.convert_to< debug_generation_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_mpu_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_mpu_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // trace
    for(int ii = 0; ii < 4; ii++) {
        trace[ii].all(_val);
        _val = _val >> trace[ii].get_width();
    }
    #endif
    
    icache.all( _val);
    _val = _val >> icache.get_width(); 
    axi_attr.all( _val);
    _val = _val >> axi_attr.get_width(); 
    sdp_ctl.all( _val);
    _val = _val >> sdp_ctl.get_width(); 
    mpu_ctl.all( _val);
    _val = _val >> mpu_ctl.get_width(); 
    mpu_cfg.all( _val);
    _val = _val >> mpu_cfg.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mpu_run
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].all(_val);
        _val = _val >> mpu_run[ii].get_width();
    }
    #endif
    
    count_stage.all( _val);
    _val = _val >> count_stage.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_cycles
    for(int ii = 0; ii < 4; ii++) {
        CNT_cycles[ii].all(_val);
        _val = _val >> CNT_cycles[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_phv_executed
    for(int ii = 0; ii < 4; ii++) {
        CNT_phv_executed[ii].all(_val);
        _val = _val >> CNT_phv_executed[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_inst_executed
    for(int ii = 0; ii < 4; ii++) {
        CNT_inst_executed[ii].all(_val);
        _val = _val >> CNT_inst_executed[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_icache_miss
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_miss[ii].all(_val);
        _val = _val >> CNT_icache_miss[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_icache_fill_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_fill_stall[ii].all(_val);
        _val = _val >> CNT_icache_fill_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_hazard_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_hazard_stall[ii].all(_val);
        _val = _val >> CNT_hazard_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_phvwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_phvwr_stall[ii].all(_val);
        _val = _val >> CNT_phvwr_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_memwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_memwr_stall[ii].all(_val);
        _val = _val >> CNT_memwr_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_tblwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_tblwr_stall[ii].all(_val);
        _val = _val >> CNT_tblwr_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_fence_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_fence_stall[ii].all(_val);
        _val = _val >> CNT_fence_stall[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_tbl_addr
    for(int ii = 0; ii < 4; ii++) {
        sta_tbl_addr[ii].all(_val);
        _val = _val >> sta_tbl_addr[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_table
    for(int ii = 0; ii < 4; ii++) {
        sta_table[ii].all(_val);
        _val = _val >> sta_table[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_data
    for(int ii = 0; ii < 4; ii++) {
        sta_data[ii].all(_val);
        _val = _val >> sta_data[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_key
    for(int ii = 0; ii < 4; ii++) {
        sta_key[ii].all(_val);
        _val = _val >> sta_key[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_pc_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_pc_mpu[ii].all(_val);
        _val = _val >> sta_pc_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_ctl_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_ctl_mpu[ii].all(_val);
        _val = _val >> sta_ctl_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].all(_val);
        _val = _val >> sta_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr1_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr1_mpu[ii].all(_val);
        _val = _val >> sta_gpr1_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr2_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr2_mpu[ii].all(_val);
        _val = _val >> sta_gpr2_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr3_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr3_mpu[ii].all(_val);
        _val = _val >> sta_gpr3_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr4_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr4_mpu[ii].all(_val);
        _val = _val >> sta_gpr4_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr5_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr5_mpu[ii].all(_val);
        _val = _val >> sta_gpr5_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr6_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr6_mpu[ii].all(_val);
        _val = _val >> sta_gpr6_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr7_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr7_mpu[ii].all(_val);
        _val = _val >> sta_gpr7_mpu[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_pend
    for(int ii = 0; ii < 4; ii++) {
        sta_pend[ii].all(_val);
        _val = _val >> sta_pend[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_error
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].all(_val);
        _val = _val >> sta_error[ii].get_width();
    }
    #endif
    
    CNT_sdp.all( _val);
    _val = _val >> CNT_sdp.get_width(); 
    STA_live_sdp.all( _val);
    _val = _val >> STA_live_sdp.get_width(); 
    STA_stg.all( _val);
    _val = _val >> STA_stg.get_width(); 
    mismatch.all( _val);
    _val = _val >> mismatch.get_width(); 
    cfg_debug_port.all( _val);
    _val = _val >> cfg_debug_port.get_width(); 
    cfg_sdp_mem.all( _val);
    _val = _val >> cfg_sdp_mem.get_width(); 
    sta_sdp_mem.all( _val);
    _val = _val >> sta_sdp_mem.get_width(); 
    cfg_mpu0_icache_data0.all( _val);
    _val = _val >> cfg_mpu0_icache_data0.get_width(); 
    sta_mpu0_icache_data0.all( _val);
    _val = _val >> sta_mpu0_icache_data0.get_width(); 
    cfg_mpu0_icache_data1.all( _val);
    _val = _val >> cfg_mpu0_icache_data1.get_width(); 
    sta_mpu0_icache_data1.all( _val);
    _val = _val >> sta_mpu0_icache_data1.get_width(); 
    cfg_mpu1_icache_data0.all( _val);
    _val = _val >> cfg_mpu1_icache_data0.get_width(); 
    sta_mpu1_icache_data0.all( _val);
    _val = _val >> sta_mpu1_icache_data0.get_width(); 
    cfg_mpu1_icache_data1.all( _val);
    _val = _val >> cfg_mpu1_icache_data1.get_width(); 
    sta_mpu1_icache_data1.all( _val);
    _val = _val >> sta_mpu1_icache_data1.get_width(); 
    cfg_mpu2_icache_data0.all( _val);
    _val = _val >> cfg_mpu2_icache_data0.get_width(); 
    sta_mpu2_icache_data0.all( _val);
    _val = _val >> sta_mpu2_icache_data0.get_width(); 
    cfg_mpu2_icache_data1.all( _val);
    _val = _val >> cfg_mpu2_icache_data1.get_width(); 
    sta_mpu2_icache_data1.all( _val);
    _val = _val >> sta_mpu2_icache_data1.get_width(); 
    cfg_mpu3_icache_data0.all( _val);
    _val = _val >> cfg_mpu3_icache_data0.get_width(); 
    sta_mpu3_icache_data0.all( _val);
    _val = _val >> sta_mpu3_icache_data0.get_width(); 
    cfg_mpu3_icache_data1.all( _val);
    _val = _val >> cfg_mpu3_icache_data1.get_width(); 
    sta_mpu3_icache_data1.all( _val);
    _val = _val >> sta_mpu3_icache_data1.get_width(); 
    cfg_mpu0_icache_tag.all( _val);
    _val = _val >> cfg_mpu0_icache_tag.get_width(); 
    sta_mpu0_icache_tag.all( _val);
    _val = _val >> sta_mpu0_icache_tag.get_width(); 
    cfg_mpu1_icache_tag.all( _val);
    _val = _val >> cfg_mpu1_icache_tag.get_width(); 
    sta_mpu1_icache_tag.all( _val);
    _val = _val >> sta_mpu1_icache_tag.get_width(); 
    cfg_mpu2_icache_tag.all( _val);
    _val = _val >> cfg_mpu2_icache_tag.get_width(); 
    sta_mpu2_icache_tag.all( _val);
    _val = _val >> sta_mpu2_icache_tag.get_width(); 
    cfg_mpu3_icache_tag.all( _val);
    _val = _val >> cfg_mpu3_icache_tag.get_width(); 
    sta_mpu3_icache_tag.all( _val);
    _val = _val >> sta_mpu3_icache_tag.get_width(); 
    cfg_spare.all( _val);
    _val = _val >> cfg_spare.get_width(); 
    sta_spare.all( _val);
    _val = _val >> sta_spare.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tag_read
    for(int ii = 0; ii < 4; ii++) {
        tag_read[ii].all(_val);
        _val = _val >> tag_read[ii].get_width();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // icache_sram_read
    for(int ii = 0; ii < 4; ii++) {
        icache_sram_read[ii].all(_val);
        _val = _val >> icache_sram_read[ii].get_width();
    }
    #endif
    
    spr_reg.all( _val);
    _val = _val >> spr_reg.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    int_err.all( _val);
    _val = _val >> int_err.get_width(); 
    int_info.all( _val);
    _val = _val >> int_info.get_width(); 
}

cpp_int cap_mpu_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_info.get_width(); ret_val = ret_val  | int_info.all(); 
    ret_val = ret_val << int_err.get_width(); ret_val = ret_val  | int_err.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << spr_reg.get_width(); ret_val = ret_val  | spr_reg.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // icache_sram_read
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << icache_sram_read[ii].get_width(); ret_val = ret_val  | icache_sram_read[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // tag_read
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << tag_read[ii].get_width(); ret_val = ret_val  | tag_read[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << sta_spare.get_width(); ret_val = ret_val  | sta_spare.all(); 
    ret_val = ret_val << cfg_spare.get_width(); ret_val = ret_val  | cfg_spare.all(); 
    ret_val = ret_val << sta_mpu3_icache_tag.get_width(); ret_val = ret_val  | sta_mpu3_icache_tag.all(); 
    ret_val = ret_val << cfg_mpu3_icache_tag.get_width(); ret_val = ret_val  | cfg_mpu3_icache_tag.all(); 
    ret_val = ret_val << sta_mpu2_icache_tag.get_width(); ret_val = ret_val  | sta_mpu2_icache_tag.all(); 
    ret_val = ret_val << cfg_mpu2_icache_tag.get_width(); ret_val = ret_val  | cfg_mpu2_icache_tag.all(); 
    ret_val = ret_val << sta_mpu1_icache_tag.get_width(); ret_val = ret_val  | sta_mpu1_icache_tag.all(); 
    ret_val = ret_val << cfg_mpu1_icache_tag.get_width(); ret_val = ret_val  | cfg_mpu1_icache_tag.all(); 
    ret_val = ret_val << sta_mpu0_icache_tag.get_width(); ret_val = ret_val  | sta_mpu0_icache_tag.all(); 
    ret_val = ret_val << cfg_mpu0_icache_tag.get_width(); ret_val = ret_val  | cfg_mpu0_icache_tag.all(); 
    ret_val = ret_val << sta_mpu3_icache_data1.get_width(); ret_val = ret_val  | sta_mpu3_icache_data1.all(); 
    ret_val = ret_val << cfg_mpu3_icache_data1.get_width(); ret_val = ret_val  | cfg_mpu3_icache_data1.all(); 
    ret_val = ret_val << sta_mpu3_icache_data0.get_width(); ret_val = ret_val  | sta_mpu3_icache_data0.all(); 
    ret_val = ret_val << cfg_mpu3_icache_data0.get_width(); ret_val = ret_val  | cfg_mpu3_icache_data0.all(); 
    ret_val = ret_val << sta_mpu2_icache_data1.get_width(); ret_val = ret_val  | sta_mpu2_icache_data1.all(); 
    ret_val = ret_val << cfg_mpu2_icache_data1.get_width(); ret_val = ret_val  | cfg_mpu2_icache_data1.all(); 
    ret_val = ret_val << sta_mpu2_icache_data0.get_width(); ret_val = ret_val  | sta_mpu2_icache_data0.all(); 
    ret_val = ret_val << cfg_mpu2_icache_data0.get_width(); ret_val = ret_val  | cfg_mpu2_icache_data0.all(); 
    ret_val = ret_val << sta_mpu1_icache_data1.get_width(); ret_val = ret_val  | sta_mpu1_icache_data1.all(); 
    ret_val = ret_val << cfg_mpu1_icache_data1.get_width(); ret_val = ret_val  | cfg_mpu1_icache_data1.all(); 
    ret_val = ret_val << sta_mpu1_icache_data0.get_width(); ret_val = ret_val  | sta_mpu1_icache_data0.all(); 
    ret_val = ret_val << cfg_mpu1_icache_data0.get_width(); ret_val = ret_val  | cfg_mpu1_icache_data0.all(); 
    ret_val = ret_val << sta_mpu0_icache_data1.get_width(); ret_val = ret_val  | sta_mpu0_icache_data1.all(); 
    ret_val = ret_val << cfg_mpu0_icache_data1.get_width(); ret_val = ret_val  | cfg_mpu0_icache_data1.all(); 
    ret_val = ret_val << sta_mpu0_icache_data0.get_width(); ret_val = ret_val  | sta_mpu0_icache_data0.all(); 
    ret_val = ret_val << cfg_mpu0_icache_data0.get_width(); ret_val = ret_val  | cfg_mpu0_icache_data0.all(); 
    ret_val = ret_val << sta_sdp_mem.get_width(); ret_val = ret_val  | sta_sdp_mem.all(); 
    ret_val = ret_val << cfg_sdp_mem.get_width(); ret_val = ret_val  | cfg_sdp_mem.all(); 
    ret_val = ret_val << cfg_debug_port.get_width(); ret_val = ret_val  | cfg_debug_port.all(); 
    ret_val = ret_val << mismatch.get_width(); ret_val = ret_val  | mismatch.all(); 
    ret_val = ret_val << STA_stg.get_width(); ret_val = ret_val  | STA_stg.all(); 
    ret_val = ret_val << STA_live_sdp.get_width(); ret_val = ret_val  | STA_live_sdp.all(); 
    ret_val = ret_val << CNT_sdp.get_width(); ret_val = ret_val  | CNT_sdp.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_error
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_error[ii].get_width(); ret_val = ret_val  | sta_error[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_pend
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_pend[ii].get_width(); ret_val = ret_val  | sta_pend[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr7_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr7_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr7_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr6_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr6_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr6_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr5_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr5_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr5_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr4_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr4_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr4_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr3_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr3_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr3_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr2_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr2_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr2_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_gpr1_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_gpr1_mpu[ii].get_width(); ret_val = ret_val  | sta_gpr1_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_mpu[ii].get_width(); ret_val = ret_val  | sta_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_ctl_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_ctl_mpu[ii].get_width(); ret_val = ret_val  | sta_ctl_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_pc_mpu
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_pc_mpu[ii].get_width(); ret_val = ret_val  | sta_pc_mpu[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_key
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_key[ii].get_width(); ret_val = ret_val  | sta_key[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_data
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_data[ii].get_width(); ret_val = ret_val  | sta_data[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_table
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_table[ii].get_width(); ret_val = ret_val  | sta_table[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // sta_tbl_addr
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << sta_tbl_addr[ii].get_width(); ret_val = ret_val  | sta_tbl_addr[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_fence_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_fence_stall[ii].get_width(); ret_val = ret_val  | CNT_fence_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_tblwr_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_tblwr_stall[ii].get_width(); ret_val = ret_val  | CNT_tblwr_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_memwr_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_memwr_stall[ii].get_width(); ret_val = ret_val  | CNT_memwr_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_phvwr_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_phvwr_stall[ii].get_width(); ret_val = ret_val  | CNT_phvwr_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_hazard_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_hazard_stall[ii].get_width(); ret_val = ret_val  | CNT_hazard_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_icache_fill_stall
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_icache_fill_stall[ii].get_width(); ret_val = ret_val  | CNT_icache_fill_stall[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_icache_miss
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_icache_miss[ii].get_width(); ret_val = ret_val  | CNT_icache_miss[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_inst_executed
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_inst_executed[ii].get_width(); ret_val = ret_val  | CNT_inst_executed[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_phv_executed
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_phv_executed[ii].get_width(); ret_val = ret_val  | CNT_phv_executed[ii].all(); 
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // CNT_cycles
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << CNT_cycles[ii].get_width(); ret_val = ret_val  | CNT_cycles[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << count_stage.get_width(); ret_val = ret_val  | count_stage.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // mpu_run
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << mpu_run[ii].get_width(); ret_val = ret_val  | mpu_run[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << mpu_cfg.get_width(); ret_val = ret_val  | mpu_cfg.all(); 
    ret_val = ret_val << mpu_ctl.get_width(); ret_val = ret_val  | mpu_ctl.all(); 
    ret_val = ret_val << sdp_ctl.get_width(); ret_val = ret_val  | sdp_ctl.all(); 
    ret_val = ret_val << axi_attr.get_width(); ret_val = ret_val  | axi_attr.all(); 
    ret_val = ret_val << icache.get_width(); ret_val = ret_val  | icache.all(); 
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // trace
    for(int ii = 4-1; ii >= 0; ii--) {
         ret_val = ret_val << trace[ii].get_width(); ret_val = ret_val  | trace[ii].all(); 
    }
    #endif
    
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_trace_t::all() const {
    cpp_int ret_val;

    // debug_generation
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_generation; 
    
    // debug_index
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__debug_index; 
    
    // buf_size
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__buf_size; 
    
    // base_addr
    ret_val = ret_val << 28; ret_val = ret_val  | int_var__base_addr; 
    
    // rst
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rst; 
    
    // wrap
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__wrap; 
    
    // instructions
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__instructions; 
    
    // table_and_key
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__table_and_key; 
    
    // watch_pc
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__watch_pc; 
    
    // watch_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__watch_enable; 
    
    // phv_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phv_error; 
    
    // phv_debug
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phv_debug; 
    
    // trace_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_enable; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_icache_t::all() const {
    cpp_int ret_val;

    // invalidate
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__invalidate; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_axi_attr_t::all() const {
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

cpp_int cap_mpu_csr_sdp_ctl_t::all() const {
    cpp_int ret_val;

    // mask_frame_size
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mask_frame_size; 
    
    // enable_compress
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_compress; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_mpu_ctl_t::all() const {
    cpp_int ret_val;

    // enable_pipe_freeze_on_stop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_pipe_freeze_on_stop; 
    
    // enable_error_stop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_error_stop; 
    
    // enable_stop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable_stop; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_mpu_cfg_t::all() const {
    cpp_int ret_val;

    // te_clean_wb_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__te_clean_wb_enable; 
    
    // max_inst_count
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__max_inst_count; 
    
    // icache_table_id
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__icache_table_id; 
    
    // stall_wb_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stall_wb_full; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_mpu_run_t::all() const {
    cpp_int ret_val;

    // step_pulse
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__step_pulse; 
    
    // stop_pulse
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stop_pulse; 
    
    // start_pulse
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__start_pulse; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_count_stage_t::all() const {
    cpp_int ret_val;

    // stop_on_saturate
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stop_on_saturate; 
    
    // watch
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__watch; 
    
    // debug
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug; 
    
    // always_on
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__always_on; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_cycles_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 26; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_phv_executed_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_inst_executed_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 25; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_icache_miss_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_icache_fill_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_hazard_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_phvwr_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_memwr_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_tblwr_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_fence_stall_t::all() const {
    cpp_int ret_val;

    // counter
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__counter; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_tbl_addr_t::all() const {
    cpp_int ret_val;

    // in_mpu
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__in_mpu; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_table_t::all() const {
    cpp_int ret_val;

    // mpu_processing_table_latency
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__mpu_processing_table_latency; 
    
    // mpu_processing_pkt_id
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__mpu_processing_pkt_id; 
    
    // mpu_processing_table_id
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mpu_processing_table_id; 
    
    // mpu_processing_table_sram
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_processing_table_sram; 
    
    // mpu_processing_table_pcie
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_processing_table_pcie; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_data_t::all() const {
    cpp_int ret_val;

    // td
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__td; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_key_t::all() const {
    cpp_int ret_val;

    // kd
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__kd; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_pc_mpu_t::all() const {
    cpp_int ret_val;

    // inst
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__inst; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::all() const {
    cpp_int ret_val;

    // stall_vector
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__stall_vector; 
    
    // phv_data_fifo_depth
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__phv_data_fifo_depth; 
    
    // phv_cmd_fifo_depth
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phv_cmd_fifo_depth; 
    
    // icache_state
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__icache_state; 
    
    // ex_pc
    ret_val = ret_val << 31; ret_val = ret_val  | int_var__ex_pc; 
    
    // ex_execute
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ex_execute; 
    
    // ex_valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ex_valid; 
    
    // stopped
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stopped; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu_t::all() const {
    cpp_int ret_val;

    // c7
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c7; 
    
    // c6
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c6; 
    
    // c5
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c5; 
    
    // c4
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c4; 
    
    // c3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c3; 
    
    // c2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c2; 
    
    // c1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__c1; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr1_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr2_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr3_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr4_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr5_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr6_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_gpr7_mpu_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_pend_t::all() const {
    cpp_int ret_val;

    // pending_table_write_valid1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pending_table_write_valid1; 
    
    // pending_table_write_valid0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pending_table_write_valid0; 
    
    // wr_id
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__wr_id; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_error_t::all() const {
    cpp_int ret_val;

    // icache1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__icache1; 
    
    // icache0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__icache0; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_sdp_t::all() const {
    cpp_int ret_val;

    // eop_out
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__eop_out; 
    
    // sop_out
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__sop_out; 
    
    // eop_in
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__eop_in; 
    
    // sop_in
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__sop_in; 
    
    // phv_fifo_depth
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phv_fifo_depth; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_STA_live_sdp_t::all() const {
    cpp_int ret_val;

    // rptr
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__rptr; 
    
    // wptr
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__wptr; 
    
    // staging_full
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__staging_full; 
    
    // update_result_count
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__update_result_count; 
    
    // phv_update_valid
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__phv_update_valid; 
    
    // phv_pkt_id
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phv_pkt_id; 
    
    // update_pkt_id
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__update_pkt_id; 
    
    // update_empty
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__update_empty; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_STA_stg_t::all() const {
    cpp_int ret_val;

    // mpu_processing
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__mpu_processing; 
    
    // te_valid
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__te_valid; 
    
    // drdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__drdy; 
    
    // srdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__srdy; 
    
    // drdy_out
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__drdy_out; 
    
    // srdy_out
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__srdy_out; 
    
    // sdp_drdy_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_drdy_in; 
    
    // sdp_srdy_in
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_srdy_in; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_mismatch_t::all() const {
    cpp_int ret_val;

    // pkt_id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__pkt_id; 
    
    // sdp_result_count
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sdp_result_count; 
    
    // update_result_count
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__update_result_count; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_debug_port_t::all() const {
    cpp_int ret_val;

    // select
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__select; 
    
    // enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__enable; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_sdp_mem_t::all() const {
    cpp_int ret_val;

    // ecc_disable_cor
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_cor; 
    
    // ecc_disable_det
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_disable_det; 
    
    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::all() const {
    cpp_int ret_val;

    // addr
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 27; ret_val = ret_val  | int_var__syndrome; 
    
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

cpp_int cap_mpu_csr_cfg_mpu0_icache_data0_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data0_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu0_icache_data1_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data1_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_data0_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data0_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_data1_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data1_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_data0_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data0_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_data1_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data1_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_data0_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data0_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_data1_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data1_t::all() const {
    cpp_int ret_val;

    // parity_error_addr
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__parity_error_addr; 
    
    // parity_error
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__parity_error; 
    
    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu0_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu0_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu1_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu2_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu3_icache_tag_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_cfg_spare_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_sta_spare_t::all() const {
    cpp_int ret_val;

    // spare
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_tag_read_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << data.get_width(); ret_val = ret_val  | data.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_tag_read_data_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_icache_sram_read_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << data.get_width(); ret_val = ret_val  | data.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_icache_sram_read_data_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_spr_reg_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << data.get_width(); ret_val = ret_val  | data.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_spr_reg_data_t::all() const {
    cpp_int ret_val;

    // value
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__value; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // int_info_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_info_interrupt; 
    
    // int_err_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_err_interrupt; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // int_info_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_info_enable; 
    
    // int_err_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__int_err_enable; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_int_err_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_int_err_intreg_t::all() const {
    cpp_int ret_val;

    // cache_parity_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_3_interrupt; 
    
    // cache_parity_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_2_interrupt; 
    
    // cache_parity_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_1_interrupt; 
    
    // cache_parity_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_0_interrupt; 
    
    // cache_axi_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_3_interrupt; 
    
    // cache_axi_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_2_interrupt; 
    
    // cache_axi_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_1_interrupt; 
    
    // cache_axi_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_0_interrupt; 
    
    // write_err_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_3_interrupt; 
    
    // write_err_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_2_interrupt; 
    
    // write_err_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_1_interrupt; 
    
    // write_err_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_0_interrupt; 
    
    // phvwr_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_3_interrupt; 
    
    // phvwr_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_2_interrupt; 
    
    // phvwr_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_1_interrupt; 
    
    // phvwr_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_0_interrupt; 
    
    // max_inst_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_3_interrupt; 
    
    // max_inst_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_2_interrupt; 
    
    // max_inst_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_1_interrupt; 
    
    // max_inst_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_0_interrupt; 
    
    // illegal_op_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_3_interrupt; 
    
    // illegal_op_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_2_interrupt; 
    
    // illegal_op_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_1_interrupt; 
    
    // illegal_op_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_0_interrupt; 
    
    // sdp_mem_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_mem_correctable_interrupt; 
    
    // sdp_mem_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_mem_uncorrectable_interrupt; 
    
    // results_mismatch_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__results_mismatch_interrupt; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // cache_parity_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_3_enable; 
    
    // cache_parity_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_2_enable; 
    
    // cache_parity_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_1_enable; 
    
    // cache_parity_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_parity_0_enable; 
    
    // cache_axi_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_3_enable; 
    
    // cache_axi_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_2_enable; 
    
    // cache_axi_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_1_enable; 
    
    // cache_axi_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cache_axi_0_enable; 
    
    // write_err_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_3_enable; 
    
    // write_err_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_2_enable; 
    
    // write_err_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_1_enable; 
    
    // write_err_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__write_err_0_enable; 
    
    // phvwr_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_3_enable; 
    
    // phvwr_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_2_enable; 
    
    // phvwr_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_1_enable; 
    
    // phvwr_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phvwr_0_enable; 
    
    // max_inst_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_3_enable; 
    
    // max_inst_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_2_enable; 
    
    // max_inst_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_1_enable; 
    
    // max_inst_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__max_inst_0_enable; 
    
    // illegal_op_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_3_enable; 
    
    // illegal_op_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_2_enable; 
    
    // illegal_op_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_1_enable; 
    
    // illegal_op_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__illegal_op_0_enable; 
    
    // sdp_mem_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_mem_correctable_enable; 
    
    // sdp_mem_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sdp_mem_uncorrectable_enable; 
    
    // results_mismatch_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__results_mismatch_enable; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_int_info_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::all() const {
    cpp_int ret_val;

    // mpu_stop_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_3_interrupt; 
    
    // mpu_stop_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_2_interrupt; 
    
    // mpu_stop_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_1_interrupt; 
    
    // mpu_stop_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_0_interrupt; 
    
    // trace_full_3_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_3_interrupt; 
    
    // trace_full_2_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_2_interrupt; 
    
    // trace_full_1_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_1_interrupt; 
    
    // trace_full_0_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_0_interrupt; 
    
    return ret_val;
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // mpu_stop_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_3_enable; 
    
    // mpu_stop_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_2_enable; 
    
    // mpu_stop_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_1_enable; 
    
    // mpu_stop_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mpu_stop_0_enable; 
    
    // trace_full_3_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_3_enable; 
    
    // trace_full_2_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_2_enable; 
    
    // trace_full_1_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_1_enable; 
    
    // trace_full_0_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__trace_full_0_enable; 
    
    return ret_val;
}

void cap_mpu_csr_int_info_int_enable_clear_t::clear() {

    int_var__trace_full_0_enable = 0; 
    
    int_var__trace_full_1_enable = 0; 
    
    int_var__trace_full_2_enable = 0; 
    
    int_var__trace_full_3_enable = 0; 
    
    int_var__mpu_stop_0_enable = 0; 
    
    int_var__mpu_stop_1_enable = 0; 
    
    int_var__mpu_stop_2_enable = 0; 
    
    int_var__mpu_stop_3_enable = 0; 
    
}

void cap_mpu_csr_int_info_int_test_set_t::clear() {

    int_var__trace_full_0_interrupt = 0; 
    
    int_var__trace_full_1_interrupt = 0; 
    
    int_var__trace_full_2_interrupt = 0; 
    
    int_var__trace_full_3_interrupt = 0; 
    
    int_var__mpu_stop_0_interrupt = 0; 
    
    int_var__mpu_stop_1_interrupt = 0; 
    
    int_var__mpu_stop_2_interrupt = 0; 
    
    int_var__mpu_stop_3_interrupt = 0; 
    
}

void cap_mpu_csr_int_info_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mpu_csr_int_err_int_enable_clear_t::clear() {

    int_var__results_mismatch_enable = 0; 
    
    int_var__sdp_mem_uncorrectable_enable = 0; 
    
    int_var__sdp_mem_correctable_enable = 0; 
    
    int_var__illegal_op_0_enable = 0; 
    
    int_var__illegal_op_1_enable = 0; 
    
    int_var__illegal_op_2_enable = 0; 
    
    int_var__illegal_op_3_enable = 0; 
    
    int_var__max_inst_0_enable = 0; 
    
    int_var__max_inst_1_enable = 0; 
    
    int_var__max_inst_2_enable = 0; 
    
    int_var__max_inst_3_enable = 0; 
    
    int_var__phvwr_0_enable = 0; 
    
    int_var__phvwr_1_enable = 0; 
    
    int_var__phvwr_2_enable = 0; 
    
    int_var__phvwr_3_enable = 0; 
    
    int_var__write_err_0_enable = 0; 
    
    int_var__write_err_1_enable = 0; 
    
    int_var__write_err_2_enable = 0; 
    
    int_var__write_err_3_enable = 0; 
    
    int_var__cache_axi_0_enable = 0; 
    
    int_var__cache_axi_1_enable = 0; 
    
    int_var__cache_axi_2_enable = 0; 
    
    int_var__cache_axi_3_enable = 0; 
    
    int_var__cache_parity_0_enable = 0; 
    
    int_var__cache_parity_1_enable = 0; 
    
    int_var__cache_parity_2_enable = 0; 
    
    int_var__cache_parity_3_enable = 0; 
    
}

void cap_mpu_csr_int_err_intreg_t::clear() {

    int_var__results_mismatch_interrupt = 0; 
    
    int_var__sdp_mem_uncorrectable_interrupt = 0; 
    
    int_var__sdp_mem_correctable_interrupt = 0; 
    
    int_var__illegal_op_0_interrupt = 0; 
    
    int_var__illegal_op_1_interrupt = 0; 
    
    int_var__illegal_op_2_interrupt = 0; 
    
    int_var__illegal_op_3_interrupt = 0; 
    
    int_var__max_inst_0_interrupt = 0; 
    
    int_var__max_inst_1_interrupt = 0; 
    
    int_var__max_inst_2_interrupt = 0; 
    
    int_var__max_inst_3_interrupt = 0; 
    
    int_var__phvwr_0_interrupt = 0; 
    
    int_var__phvwr_1_interrupt = 0; 
    
    int_var__phvwr_2_interrupt = 0; 
    
    int_var__phvwr_3_interrupt = 0; 
    
    int_var__write_err_0_interrupt = 0; 
    
    int_var__write_err_1_interrupt = 0; 
    
    int_var__write_err_2_interrupt = 0; 
    
    int_var__write_err_3_interrupt = 0; 
    
    int_var__cache_axi_0_interrupt = 0; 
    
    int_var__cache_axi_1_interrupt = 0; 
    
    int_var__cache_axi_2_interrupt = 0; 
    
    int_var__cache_axi_3_interrupt = 0; 
    
    int_var__cache_parity_0_interrupt = 0; 
    
    int_var__cache_parity_1_interrupt = 0; 
    
    int_var__cache_parity_2_interrupt = 0; 
    
    int_var__cache_parity_3_interrupt = 0; 
    
}

void cap_mpu_csr_int_err_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_mpu_csr_intreg_status_t::clear() {

    int_var__int_err_interrupt = 0; 
    
    int_var__int_info_interrupt = 0; 
    
}

void cap_mpu_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__int_err_enable = 0; 
    
    int_var__int_info_enable = 0; 
    
}

void cap_mpu_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_mpu_csr_spr_reg_data_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_spr_reg_t::clear() {

    data.clear();
}

void cap_mpu_csr_icache_sram_read_data_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_icache_sram_read_t::clear() {

    data.clear();
}

void cap_mpu_csr_tag_read_data_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_tag_read_t::clear() {

    data.clear();
}

void cap_mpu_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_mpu_csr_sta_spare_t::clear() {

    int_var__spare = 0; 
    
}

void cap_mpu_csr_cfg_spare_t::clear() {

    int_var__spare = 0; 
    
}

void cap_mpu_csr_sta_mpu3_icache_tag_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mpu_csr_cfg_mpu3_icache_tag_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu2_icache_tag_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mpu_csr_cfg_mpu2_icache_tag_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu1_icache_tag_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mpu_csr_cfg_mpu1_icache_tag_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu0_icache_tag_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_mpu_csr_cfg_mpu0_icache_tag_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu3_icache_data1_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu3_icache_data1_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu3_icache_data0_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu3_icache_data0_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu2_icache_data1_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu2_icache_data1_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu2_icache_data0_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu2_icache_data0_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu1_icache_data1_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu1_icache_data1_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu1_icache_data0_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu1_icache_data0_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu0_icache_data1_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu0_icache_data1_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_mpu0_icache_data0_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__parity_error = 0; 
    
    int_var__parity_error_addr = 0; 
    
}

void cap_mpu_csr_cfg_mpu0_icache_data0_t::clear() {

    int_var__bist_run = 0; 
    
}

void cap_mpu_csr_sta_sdp_mem_t::clear() {

    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
    int_var__correctable = 0; 
    
    int_var__uncorrectable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
}

void cap_mpu_csr_cfg_sdp_mem_t::clear() {

    int_var__bist_run = 0; 
    
    int_var__ecc_disable_det = 0; 
    
    int_var__ecc_disable_cor = 0; 
    
}

void cap_mpu_csr_cfg_debug_port_t::clear() {

    int_var__enable = 0; 
    
    int_var__select = 0; 
    
}

void cap_mpu_csr_mismatch_t::clear() {

    int_var__update_result_count = 0; 
    
    int_var__sdp_result_count = 0; 
    
    int_var__pkt_id = 0; 
    
}

void cap_mpu_csr_STA_stg_t::clear() {

    int_var__sdp_srdy_in = 0; 
    
    int_var__sdp_drdy_in = 0; 
    
    int_var__srdy_out = 0; 
    
    int_var__drdy_out = 0; 
    
    int_var__srdy = 0; 
    
    int_var__drdy = 0; 
    
    int_var__te_valid = 0; 
    
    int_var__mpu_processing = 0; 
    
}

void cap_mpu_csr_STA_live_sdp_t::clear() {

    int_var__update_empty = 0; 
    
    int_var__update_pkt_id = 0; 
    
    int_var__phv_pkt_id = 0; 
    
    int_var__phv_update_valid = 0; 
    
    int_var__update_result_count = 0; 
    
    int_var__staging_full = 0; 
    
    int_var__wptr = 0; 
    
    int_var__rptr = 0; 
    
}

void cap_mpu_csr_CNT_sdp_t::clear() {

    int_var__phv_fifo_depth = 0; 
    
    int_var__sop_in = 0; 
    
    int_var__eop_in = 0; 
    
    int_var__sop_out = 0; 
    
    int_var__eop_out = 0; 
    
}

void cap_mpu_csr_sta_error_t::clear() {

    int_var__icache0 = 0; 
    
    int_var__icache1 = 0; 
    
}

void cap_mpu_csr_sta_pend_t::clear() {

    int_var__wr_id = 0; 
    
    int_var__pending_table_write_valid0 = 0; 
    
    int_var__pending_table_write_valid1 = 0; 
    
}

void cap_mpu_csr_sta_gpr7_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr6_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr5_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr4_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr3_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr2_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_gpr1_mpu_t::clear() {

    int_var__value = 0; 
    
}

void cap_mpu_csr_sta_mpu_t::clear() {

    int_var__c1 = 0; 
    
    int_var__c2 = 0; 
    
    int_var__c3 = 0; 
    
    int_var__c4 = 0; 
    
    int_var__c5 = 0; 
    
    int_var__c6 = 0; 
    
    int_var__c7 = 0; 
    
}

void cap_mpu_csr_sta_ctl_mpu_t::clear() {

    int_var__stopped = 0; 
    
    int_var__ex_valid = 0; 
    
    int_var__ex_execute = 0; 
    
    int_var__ex_pc = 0; 
    
    int_var__icache_state = 0; 
    
    int_var__phv_cmd_fifo_depth = 0; 
    
    int_var__phv_data_fifo_depth = 0; 
    
    int_var__stall_vector = 0; 
    
}

void cap_mpu_csr_sta_pc_mpu_t::clear() {

    int_var__inst = 0; 
    
}

void cap_mpu_csr_sta_key_t::clear() {

    int_var__kd = 0; 
    
}

void cap_mpu_csr_sta_data_t::clear() {

    int_var__td = 0; 
    
}

void cap_mpu_csr_sta_table_t::clear() {

    int_var__mpu_processing_table_pcie = 0; 
    
    int_var__mpu_processing_table_sram = 0; 
    
    int_var__mpu_processing_table_id = 0; 
    
    int_var__mpu_processing_pkt_id = 0; 
    
    int_var__mpu_processing_table_latency = 0; 
    
}

void cap_mpu_csr_sta_tbl_addr_t::clear() {

    int_var__in_mpu = 0; 
    
}

void cap_mpu_csr_CNT_fence_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_tblwr_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_memwr_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_phvwr_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_hazard_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_icache_fill_stall_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_icache_miss_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_inst_executed_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_phv_executed_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_CNT_cycles_t::clear() {

    int_var__counter = 0; 
    
}

void cap_mpu_csr_count_stage_t::clear() {

    int_var__always_on = 0; 
    
    int_var__debug = 0; 
    
    int_var__watch = 0; 
    
    int_var__stop_on_saturate = 0; 
    
}

void cap_mpu_csr_mpu_run_t::clear() {

    int_var__start_pulse = 0; 
    
    int_var__stop_pulse = 0; 
    
    int_var__step_pulse = 0; 
    
}

void cap_mpu_csr_mpu_cfg_t::clear() {

    int_var__stall_wb_full = 0; 
    
    int_var__icache_table_id = 0; 
    
    int_var__max_inst_count = 0; 
    
    int_var__te_clean_wb_enable = 0; 
    
}

void cap_mpu_csr_mpu_ctl_t::clear() {

    int_var__enable_stop = 0; 
    
    int_var__enable_error_stop = 0; 
    
    int_var__enable_pipe_freeze_on_stop = 0; 
    
}

void cap_mpu_csr_sdp_ctl_t::clear() {

    int_var__enable_compress = 0; 
    
    int_var__mask_frame_size = 0; 
    
}

void cap_mpu_csr_axi_attr_t::clear() {

    int_var__arcache = 0; 
    
    int_var__awcache = 0; 
    
    int_var__prot = 0; 
    
    int_var__qos = 0; 
    
    int_var__lock = 0; 
    
}

void cap_mpu_csr_icache_t::clear() {

    int_var__invalidate = 0; 
    
}

void cap_mpu_csr_trace_t::clear() {

    int_var__enable = 0; 
    
    int_var__trace_enable = 0; 
    
    int_var__phv_debug = 0; 
    
    int_var__phv_error = 0; 
    
    int_var__watch_enable = 0; 
    
    int_var__watch_pc = 0; 
    
    int_var__table_and_key = 0; 
    
    int_var__instructions = 0; 
    
    int_var__wrap = 0; 
    
    int_var__rst = 0; 
    
    int_var__base_addr = 0; 
    
    int_var__buf_size = 0; 
    
    int_var__debug_index = 0; 
    
    int_var__debug_generation = 0; 
    
}

void cap_mpu_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_mpu_csr_t::clear() {

    base.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // trace
    for(int ii = 0; ii < 4; ii++) {
        trace[ii].clear();
    }
    #endif
    
    icache.clear();
    axi_attr.clear();
    sdp_ctl.clear();
    mpu_ctl.clear();
    mpu_cfg.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // mpu_run
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].clear();
    }
    #endif
    
    count_stage.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_cycles
    for(int ii = 0; ii < 4; ii++) {
        CNT_cycles[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_phv_executed
    for(int ii = 0; ii < 4; ii++) {
        CNT_phv_executed[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_inst_executed
    for(int ii = 0; ii < 4; ii++) {
        CNT_inst_executed[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_icache_miss
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_miss[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_icache_fill_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_icache_fill_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_hazard_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_hazard_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_phvwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_phvwr_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_memwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_memwr_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_tblwr_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_tblwr_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // CNT_fence_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_fence_stall[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_tbl_addr
    for(int ii = 0; ii < 4; ii++) {
        sta_tbl_addr[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_table
    for(int ii = 0; ii < 4; ii++) {
        sta_table[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_data
    for(int ii = 0; ii < 4; ii++) {
        sta_data[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_key
    for(int ii = 0; ii < 4; ii++) {
        sta_key[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_pc_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_pc_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_ctl_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_ctl_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr1_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr1_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr2_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr2_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr3_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr3_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr4_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr4_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr5_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr5_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr6_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr6_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_gpr7_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_gpr7_mpu[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_pend
    for(int ii = 0; ii < 4; ii++) {
        sta_pend[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // sta_error
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].clear();
    }
    #endif
    
    CNT_sdp.clear();
    STA_live_sdp.clear();
    STA_stg.clear();
    mismatch.clear();
    cfg_debug_port.clear();
    cfg_sdp_mem.clear();
    sta_sdp_mem.clear();
    cfg_mpu0_icache_data0.clear();
    sta_mpu0_icache_data0.clear();
    cfg_mpu0_icache_data1.clear();
    sta_mpu0_icache_data1.clear();
    cfg_mpu1_icache_data0.clear();
    sta_mpu1_icache_data0.clear();
    cfg_mpu1_icache_data1.clear();
    sta_mpu1_icache_data1.clear();
    cfg_mpu2_icache_data0.clear();
    sta_mpu2_icache_data0.clear();
    cfg_mpu2_icache_data1.clear();
    sta_mpu2_icache_data1.clear();
    cfg_mpu3_icache_data0.clear();
    sta_mpu3_icache_data0.clear();
    cfg_mpu3_icache_data1.clear();
    sta_mpu3_icache_data1.clear();
    cfg_mpu0_icache_tag.clear();
    sta_mpu0_icache_tag.clear();
    cfg_mpu1_icache_tag.clear();
    sta_mpu1_icache_tag.clear();
    cfg_mpu2_icache_tag.clear();
    sta_mpu2_icache_tag.clear();
    cfg_mpu3_icache_tag.clear();
    sta_mpu3_icache_tag.clear();
    cfg_spare.clear();
    sta_spare.clear();
    csr_intr.clear();
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // tag_read
    for(int ii = 0; ii < 4; ii++) {
        tag_read[ii].clear();
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // icache_sram_read
    for(int ii = 0; ii < 4; ii++) {
        icache_sram_read[ii].clear();
    }
    #endif
    
    spr_reg.clear();
    int_groups.clear();
    int_err.clear();
    int_info.clear();
}

void cap_mpu_csr_int_info_int_enable_clear_t::init() {

}

void cap_mpu_csr_int_info_int_test_set_t::init() {

}

void cap_mpu_csr_int_info_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mpu_csr_int_err_int_enable_clear_t::init() {

}

void cap_mpu_csr_int_err_intreg_t::init() {

}

void cap_mpu_csr_int_err_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_mpu_csr_intreg_status_t::init() {

}

void cap_mpu_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_mpu_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_mpu_csr_spr_reg_data_t::init() {

}

void cap_mpu_csr_spr_reg_t::init() {

    data.set_attributes(this,"data", 0x0 );
}

void cap_mpu_csr_icache_sram_read_data_t::init() {

}

void cap_mpu_csr_icache_sram_read_t::init() {

    data.set_attributes(this,"data", 0x0 );
}

void cap_mpu_csr_tag_read_data_t::init() {

}

void cap_mpu_csr_tag_read_t::init() {

    data.set_attributes(this,"data", 0x0 );
}

void cap_mpu_csr_csr_intr_t::init() {

}

void cap_mpu_csr_sta_spare_t::init() {

}

void cap_mpu_csr_cfg_spare_t::init() {

}

void cap_mpu_csr_sta_mpu3_icache_tag_t::init() {

}

void cap_mpu_csr_cfg_mpu3_icache_tag_t::init() {

}

void cap_mpu_csr_sta_mpu2_icache_tag_t::init() {

}

void cap_mpu_csr_cfg_mpu2_icache_tag_t::init() {

}

void cap_mpu_csr_sta_mpu1_icache_tag_t::init() {

}

void cap_mpu_csr_cfg_mpu1_icache_tag_t::init() {

}

void cap_mpu_csr_sta_mpu0_icache_tag_t::init() {

}

void cap_mpu_csr_cfg_mpu0_icache_tag_t::init() {

}

void cap_mpu_csr_sta_mpu3_icache_data1_t::init() {

}

void cap_mpu_csr_cfg_mpu3_icache_data1_t::init() {

}

void cap_mpu_csr_sta_mpu3_icache_data0_t::init() {

}

void cap_mpu_csr_cfg_mpu3_icache_data0_t::init() {

}

void cap_mpu_csr_sta_mpu2_icache_data1_t::init() {

}

void cap_mpu_csr_cfg_mpu2_icache_data1_t::init() {

}

void cap_mpu_csr_sta_mpu2_icache_data0_t::init() {

}

void cap_mpu_csr_cfg_mpu2_icache_data0_t::init() {

}

void cap_mpu_csr_sta_mpu1_icache_data1_t::init() {

}

void cap_mpu_csr_cfg_mpu1_icache_data1_t::init() {

}

void cap_mpu_csr_sta_mpu1_icache_data0_t::init() {

}

void cap_mpu_csr_cfg_mpu1_icache_data0_t::init() {

}

void cap_mpu_csr_sta_mpu0_icache_data1_t::init() {

}

void cap_mpu_csr_cfg_mpu0_icache_data1_t::init() {

}

void cap_mpu_csr_sta_mpu0_icache_data0_t::init() {

}

void cap_mpu_csr_cfg_mpu0_icache_data0_t::init() {

}

void cap_mpu_csr_sta_sdp_mem_t::init() {

}

void cap_mpu_csr_cfg_sdp_mem_t::init() {

}

void cap_mpu_csr_cfg_debug_port_t::init() {

}

void cap_mpu_csr_mismatch_t::init() {

}

void cap_mpu_csr_STA_stg_t::init() {

}

void cap_mpu_csr_STA_live_sdp_t::init() {

}

void cap_mpu_csr_CNT_sdp_t::init() {

}

void cap_mpu_csr_sta_error_t::init() {

}

void cap_mpu_csr_sta_pend_t::init() {

}

void cap_mpu_csr_sta_gpr7_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr6_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr5_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr4_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr3_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr2_mpu_t::init() {

}

void cap_mpu_csr_sta_gpr1_mpu_t::init() {

}

void cap_mpu_csr_sta_mpu_t::init() {

}

void cap_mpu_csr_sta_ctl_mpu_t::init() {

}

void cap_mpu_csr_sta_pc_mpu_t::init() {

}

void cap_mpu_csr_sta_key_t::init() {

}

void cap_mpu_csr_sta_data_t::init() {

}

void cap_mpu_csr_sta_table_t::init() {

}

void cap_mpu_csr_sta_tbl_addr_t::init() {

}

void cap_mpu_csr_CNT_fence_stall_t::init() {

}

void cap_mpu_csr_CNT_tblwr_stall_t::init() {

}

void cap_mpu_csr_CNT_memwr_stall_t::init() {

}

void cap_mpu_csr_CNT_phvwr_stall_t::init() {

}

void cap_mpu_csr_CNT_hazard_stall_t::init() {

}

void cap_mpu_csr_CNT_icache_fill_stall_t::init() {

}

void cap_mpu_csr_CNT_icache_miss_t::init() {

}

void cap_mpu_csr_CNT_inst_executed_t::init() {

}

void cap_mpu_csr_CNT_phv_executed_t::init() {

}

void cap_mpu_csr_CNT_cycles_t::init() {

}

void cap_mpu_csr_count_stage_t::init() {

    set_reset_val(cpp_int("0xf"));
    all(get_reset_val());
}

void cap_mpu_csr_mpu_run_t::init() {

}

void cap_mpu_csr_mpu_cfg_t::init() {

    set_reset_val(cpp_int("0x20801e"));
    all(get_reset_val());
}

void cap_mpu_csr_mpu_ctl_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mpu_csr_sdp_ctl_t::init() {

    set_reset_val(cpp_int("0x3"));
    all(get_reset_val());
}

void cap_mpu_csr_axi_attr_t::init() {

    set_reset_val(cpp_int("0x2ff"));
    all(get_reset_val());
}

void cap_mpu_csr_icache_t::init() {

}

void cap_mpu_csr_trace_t::init() {

    set_reset_val(cpp_int("0x3000000000"));
    all(get_reset_val());
}

void cap_mpu_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mpu_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    trace.set_attributes(this, "trace", 0x40);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) trace[ii].set_field_init_done(true, true);
        trace[ii].set_attributes(this,"trace["+to_string(ii)+"]",  0x40 + (trace[ii].get_byte_size()*ii));
    }
    #endif
    
    icache.set_attributes(this,"icache", 0x80 );
    axi_attr.set_attributes(this,"axi_attr", 0x8000 );
    sdp_ctl.set_attributes(this,"sdp_ctl", 0x8004 );
    mpu_ctl.set_attributes(this,"mpu_ctl", 0x8008 );
    mpu_cfg.set_attributes(this,"mpu_cfg", 0x800c );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    mpu_run.set_attributes(this, "mpu_run", 0x8010);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) mpu_run[ii].set_field_init_done(true, true);
        mpu_run[ii].set_attributes(this,"mpu_run["+to_string(ii)+"]",  0x8010 + (mpu_run[ii].get_byte_size()*ii));
    }
    #endif
    
    count_stage.set_attributes(this,"count_stage", 0x8030 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_cycles.set_attributes(this, "CNT_cycles", 0x8040);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_cycles[ii].set_field_init_done(true, true);
        CNT_cycles[ii].set_attributes(this,"CNT_cycles["+to_string(ii)+"]",  0x8040 + (CNT_cycles[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_phv_executed.set_attributes(this, "CNT_phv_executed", 0x8050);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_phv_executed[ii].set_field_init_done(true, true);
        CNT_phv_executed[ii].set_attributes(this,"CNT_phv_executed["+to_string(ii)+"]",  0x8050 + (CNT_phv_executed[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_inst_executed.set_attributes(this, "CNT_inst_executed", 0x8060);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_inst_executed[ii].set_field_init_done(true, true);
        CNT_inst_executed[ii].set_attributes(this,"CNT_inst_executed["+to_string(ii)+"]",  0x8060 + (CNT_inst_executed[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_icache_miss.set_attributes(this, "CNT_icache_miss", 0x8070);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_icache_miss[ii].set_field_init_done(true, true);
        CNT_icache_miss[ii].set_attributes(this,"CNT_icache_miss["+to_string(ii)+"]",  0x8070 + (CNT_icache_miss[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_icache_fill_stall.set_attributes(this, "CNT_icache_fill_stall", 0x8080);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_icache_fill_stall[ii].set_field_init_done(true, true);
        CNT_icache_fill_stall[ii].set_attributes(this,"CNT_icache_fill_stall["+to_string(ii)+"]",  0x8080 + (CNT_icache_fill_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_hazard_stall.set_attributes(this, "CNT_hazard_stall", 0x8090);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_hazard_stall[ii].set_field_init_done(true, true);
        CNT_hazard_stall[ii].set_attributes(this,"CNT_hazard_stall["+to_string(ii)+"]",  0x8090 + (CNT_hazard_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_phvwr_stall.set_attributes(this, "CNT_phvwr_stall", 0x80a0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_phvwr_stall[ii].set_field_init_done(true, true);
        CNT_phvwr_stall[ii].set_attributes(this,"CNT_phvwr_stall["+to_string(ii)+"]",  0x80a0 + (CNT_phvwr_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_memwr_stall.set_attributes(this, "CNT_memwr_stall", 0x80b0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_memwr_stall[ii].set_field_init_done(true, true);
        CNT_memwr_stall[ii].set_attributes(this,"CNT_memwr_stall["+to_string(ii)+"]",  0x80b0 + (CNT_memwr_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_tblwr_stall.set_attributes(this, "CNT_tblwr_stall", 0x80c0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_tblwr_stall[ii].set_field_init_done(true, true);
        CNT_tblwr_stall[ii].set_attributes(this,"CNT_tblwr_stall["+to_string(ii)+"]",  0x80c0 + (CNT_tblwr_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    CNT_fence_stall.set_attributes(this, "CNT_fence_stall", 0x80d0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) CNT_fence_stall[ii].set_field_init_done(true, true);
        CNT_fence_stall[ii].set_attributes(this,"CNT_fence_stall["+to_string(ii)+"]",  0x80d0 + (CNT_fence_stall[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_tbl_addr.set_attributes(this, "sta_tbl_addr", 0x80e0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_tbl_addr[ii].set_field_init_done(true, true);
        sta_tbl_addr[ii].set_attributes(this,"sta_tbl_addr["+to_string(ii)+"]",  0x80e0 + (sta_tbl_addr[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_table.set_attributes(this, "sta_table", 0x8100);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_table[ii].set_field_init_done(true, true);
        sta_table[ii].set_attributes(this,"sta_table["+to_string(ii)+"]",  0x8100 + (sta_table[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_data.set_attributes(this, "sta_data", 0x8200);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_data[ii].set_field_init_done(true, true);
        sta_data[ii].set_attributes(this,"sta_data["+to_string(ii)+"]",  0x8200 + (sta_data[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_key.set_attributes(this, "sta_key", 0x8300);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_key[ii].set_field_init_done(true, true);
        sta_key[ii].set_attributes(this,"sta_key["+to_string(ii)+"]",  0x8300 + (sta_key[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_pc_mpu.set_attributes(this, "sta_pc_mpu", 0x8400);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_pc_mpu[ii].set_field_init_done(true, true);
        sta_pc_mpu[ii].set_attributes(this,"sta_pc_mpu["+to_string(ii)+"]",  0x8400 + (sta_pc_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_ctl_mpu.set_attributes(this, "sta_ctl_mpu", 0x8420);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_ctl_mpu[ii].set_field_init_done(true, true);
        sta_ctl_mpu[ii].set_attributes(this,"sta_ctl_mpu["+to_string(ii)+"]",  0x8420 + (sta_ctl_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_mpu.set_attributes(this, "sta_mpu", 0x8440);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_mpu[ii].set_field_init_done(true, true);
        sta_mpu[ii].set_attributes(this,"sta_mpu["+to_string(ii)+"]",  0x8440 + (sta_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr1_mpu.set_attributes(this, "sta_gpr1_mpu", 0x8460);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr1_mpu[ii].set_field_init_done(true, true);
        sta_gpr1_mpu[ii].set_attributes(this,"sta_gpr1_mpu["+to_string(ii)+"]",  0x8460 + (sta_gpr1_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr2_mpu.set_attributes(this, "sta_gpr2_mpu", 0x8480);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr2_mpu[ii].set_field_init_done(true, true);
        sta_gpr2_mpu[ii].set_attributes(this,"sta_gpr2_mpu["+to_string(ii)+"]",  0x8480 + (sta_gpr2_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr3_mpu.set_attributes(this, "sta_gpr3_mpu", 0x84a0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr3_mpu[ii].set_field_init_done(true, true);
        sta_gpr3_mpu[ii].set_attributes(this,"sta_gpr3_mpu["+to_string(ii)+"]",  0x84a0 + (sta_gpr3_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr4_mpu.set_attributes(this, "sta_gpr4_mpu", 0x84c0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr4_mpu[ii].set_field_init_done(true, true);
        sta_gpr4_mpu[ii].set_attributes(this,"sta_gpr4_mpu["+to_string(ii)+"]",  0x84c0 + (sta_gpr4_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr5_mpu.set_attributes(this, "sta_gpr5_mpu", 0x84e0);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr5_mpu[ii].set_field_init_done(true, true);
        sta_gpr5_mpu[ii].set_attributes(this,"sta_gpr5_mpu["+to_string(ii)+"]",  0x84e0 + (sta_gpr5_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr6_mpu.set_attributes(this, "sta_gpr6_mpu", 0x8500);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr6_mpu[ii].set_field_init_done(true, true);
        sta_gpr6_mpu[ii].set_attributes(this,"sta_gpr6_mpu["+to_string(ii)+"]",  0x8500 + (sta_gpr6_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_gpr7_mpu.set_attributes(this, "sta_gpr7_mpu", 0x8520);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_gpr7_mpu[ii].set_field_init_done(true, true);
        sta_gpr7_mpu[ii].set_attributes(this,"sta_gpr7_mpu["+to_string(ii)+"]",  0x8520 + (sta_gpr7_mpu[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_pend.set_attributes(this, "sta_pend", 0x8540);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_pend[ii].set_field_init_done(true, true);
        sta_pend[ii].set_attributes(this,"sta_pend["+to_string(ii)+"]",  0x8540 + (sta_pend[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    sta_error.set_attributes(this, "sta_error", 0x8560);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) sta_error[ii].set_field_init_done(true, true);
        sta_error[ii].set_attributes(this,"sta_error["+to_string(ii)+"]",  0x8560 + (sta_error[ii].get_byte_size()*ii));
    }
    #endif
    
    CNT_sdp.set_attributes(this,"CNT_sdp", 0x8570 );
    STA_live_sdp.set_attributes(this,"STA_live_sdp", 0x8580 );
    STA_stg.set_attributes(this,"STA_stg", 0x8590 );
    mismatch.set_attributes(this,"mismatch", 0x8594 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x8598 );
    cfg_sdp_mem.set_attributes(this,"cfg_sdp_mem", 0x859c );
    sta_sdp_mem.set_attributes(this,"sta_sdp_mem", 0x85a0 );
    cfg_mpu0_icache_data0.set_attributes(this,"cfg_mpu0_icache_data0", 0x85a8 );
    sta_mpu0_icache_data0.set_attributes(this,"sta_mpu0_icache_data0", 0x85ac );
    cfg_mpu0_icache_data1.set_attributes(this,"cfg_mpu0_icache_data1", 0x85b0 );
    sta_mpu0_icache_data1.set_attributes(this,"sta_mpu0_icache_data1", 0x85b4 );
    cfg_mpu1_icache_data0.set_attributes(this,"cfg_mpu1_icache_data0", 0x85b8 );
    sta_mpu1_icache_data0.set_attributes(this,"sta_mpu1_icache_data0", 0x85bc );
    cfg_mpu1_icache_data1.set_attributes(this,"cfg_mpu1_icache_data1", 0x85c0 );
    sta_mpu1_icache_data1.set_attributes(this,"sta_mpu1_icache_data1", 0x85c4 );
    cfg_mpu2_icache_data0.set_attributes(this,"cfg_mpu2_icache_data0", 0x85c8 );
    sta_mpu2_icache_data0.set_attributes(this,"sta_mpu2_icache_data0", 0x85cc );
    cfg_mpu2_icache_data1.set_attributes(this,"cfg_mpu2_icache_data1", 0x85d0 );
    sta_mpu2_icache_data1.set_attributes(this,"sta_mpu2_icache_data1", 0x85d4 );
    cfg_mpu3_icache_data0.set_attributes(this,"cfg_mpu3_icache_data0", 0x85d8 );
    sta_mpu3_icache_data0.set_attributes(this,"sta_mpu3_icache_data0", 0x85dc );
    cfg_mpu3_icache_data1.set_attributes(this,"cfg_mpu3_icache_data1", 0x85e0 );
    sta_mpu3_icache_data1.set_attributes(this,"sta_mpu3_icache_data1", 0x85e4 );
    cfg_mpu0_icache_tag.set_attributes(this,"cfg_mpu0_icache_tag", 0x85e8 );
    sta_mpu0_icache_tag.set_attributes(this,"sta_mpu0_icache_tag", 0x85ec );
    cfg_mpu1_icache_tag.set_attributes(this,"cfg_mpu1_icache_tag", 0x85f0 );
    sta_mpu1_icache_tag.set_attributes(this,"sta_mpu1_icache_tag", 0x85f4 );
    cfg_mpu2_icache_tag.set_attributes(this,"cfg_mpu2_icache_tag", 0x85f8 );
    sta_mpu2_icache_tag.set_attributes(this,"sta_mpu2_icache_tag", 0x85fc );
    cfg_mpu3_icache_tag.set_attributes(this,"cfg_mpu3_icache_tag", 0x8600 );
    sta_mpu3_icache_tag.set_attributes(this,"sta_mpu3_icache_tag", 0x8604 );
    cfg_spare.set_attributes(this,"cfg_spare", 0x8608 );
    sta_spare.set_attributes(this,"sta_spare", 0x860c );
    csr_intr.set_attributes(this,"csr_intr", 0x8610 );
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    tag_read.set_attributes(this, "tag_read", 0x400);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) tag_read[ii].set_field_init_done(true, true);
        tag_read[ii].set_attributes(this,"tag_read["+to_string(ii)+"]",  0x400 + (tag_read[ii].get_byte_size()*ii));
    }
    #endif
    
    #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    icache_sram_read.set_attributes(this, "icache_sram_read", 0x4000);
    #else
    for(int ii = 0; ii < 4; ii++) {
        if(ii != 0) icache_sram_read[ii].set_field_init_done(true, true);
        icache_sram_read[ii].set_attributes(this,"icache_sram_read["+to_string(ii)+"]",  0x4000 + (icache_sram_read[ii].get_byte_size()*ii));
    }
    #endif
    
    spr_reg.set_attributes(this,"spr_reg", 0x8020 );
    int_groups.set_attributes(this,"int_groups", 0x8620 );
    int_err.set_attributes(this,"int_err", 0x8630 );
    int_info.set_attributes(this,"int_info", 0x8640 );
}

void cap_mpu_csr_int_info_int_enable_clear_t::trace_full_0_enable(const cpp_int & _val) { 
    // trace_full_0_enable
    int_var__trace_full_0_enable = _val.convert_to< trace_full_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::trace_full_0_enable() const {
    return int_var__trace_full_0_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::trace_full_1_enable(const cpp_int & _val) { 
    // trace_full_1_enable
    int_var__trace_full_1_enable = _val.convert_to< trace_full_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::trace_full_1_enable() const {
    return int_var__trace_full_1_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::trace_full_2_enable(const cpp_int & _val) { 
    // trace_full_2_enable
    int_var__trace_full_2_enable = _val.convert_to< trace_full_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::trace_full_2_enable() const {
    return int_var__trace_full_2_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::trace_full_3_enable(const cpp_int & _val) { 
    // trace_full_3_enable
    int_var__trace_full_3_enable = _val.convert_to< trace_full_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::trace_full_3_enable() const {
    return int_var__trace_full_3_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_0_enable(const cpp_int & _val) { 
    // mpu_stop_0_enable
    int_var__mpu_stop_0_enable = _val.convert_to< mpu_stop_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_0_enable() const {
    return int_var__mpu_stop_0_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_1_enable(const cpp_int & _val) { 
    // mpu_stop_1_enable
    int_var__mpu_stop_1_enable = _val.convert_to< mpu_stop_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_1_enable() const {
    return int_var__mpu_stop_1_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_2_enable(const cpp_int & _val) { 
    // mpu_stop_2_enable
    int_var__mpu_stop_2_enable = _val.convert_to< mpu_stop_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_2_enable() const {
    return int_var__mpu_stop_2_enable;
}
    
void cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_3_enable(const cpp_int & _val) { 
    // mpu_stop_3_enable
    int_var__mpu_stop_3_enable = _val.convert_to< mpu_stop_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_enable_clear_t::mpu_stop_3_enable() const {
    return int_var__mpu_stop_3_enable;
}
    
void cap_mpu_csr_int_info_int_test_set_t::trace_full_0_interrupt(const cpp_int & _val) { 
    // trace_full_0_interrupt
    int_var__trace_full_0_interrupt = _val.convert_to< trace_full_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::trace_full_0_interrupt() const {
    return int_var__trace_full_0_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::trace_full_1_interrupt(const cpp_int & _val) { 
    // trace_full_1_interrupt
    int_var__trace_full_1_interrupt = _val.convert_to< trace_full_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::trace_full_1_interrupt() const {
    return int_var__trace_full_1_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::trace_full_2_interrupt(const cpp_int & _val) { 
    // trace_full_2_interrupt
    int_var__trace_full_2_interrupt = _val.convert_to< trace_full_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::trace_full_2_interrupt() const {
    return int_var__trace_full_2_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::trace_full_3_interrupt(const cpp_int & _val) { 
    // trace_full_3_interrupt
    int_var__trace_full_3_interrupt = _val.convert_to< trace_full_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::trace_full_3_interrupt() const {
    return int_var__trace_full_3_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::mpu_stop_0_interrupt(const cpp_int & _val) { 
    // mpu_stop_0_interrupt
    int_var__mpu_stop_0_interrupt = _val.convert_to< mpu_stop_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::mpu_stop_0_interrupt() const {
    return int_var__mpu_stop_0_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::mpu_stop_1_interrupt(const cpp_int & _val) { 
    // mpu_stop_1_interrupt
    int_var__mpu_stop_1_interrupt = _val.convert_to< mpu_stop_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::mpu_stop_1_interrupt() const {
    return int_var__mpu_stop_1_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::mpu_stop_2_interrupt(const cpp_int & _val) { 
    // mpu_stop_2_interrupt
    int_var__mpu_stop_2_interrupt = _val.convert_to< mpu_stop_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::mpu_stop_2_interrupt() const {
    return int_var__mpu_stop_2_interrupt;
}
    
void cap_mpu_csr_int_info_int_test_set_t::mpu_stop_3_interrupt(const cpp_int & _val) { 
    // mpu_stop_3_interrupt
    int_var__mpu_stop_3_interrupt = _val.convert_to< mpu_stop_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_info_int_test_set_t::mpu_stop_3_interrupt() const {
    return int_var__mpu_stop_3_interrupt;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::results_mismatch_enable(const cpp_int & _val) { 
    // results_mismatch_enable
    int_var__results_mismatch_enable = _val.convert_to< results_mismatch_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::results_mismatch_enable() const {
    return int_var__results_mismatch_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::sdp_mem_uncorrectable_enable(const cpp_int & _val) { 
    // sdp_mem_uncorrectable_enable
    int_var__sdp_mem_uncorrectable_enable = _val.convert_to< sdp_mem_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::sdp_mem_uncorrectable_enable() const {
    return int_var__sdp_mem_uncorrectable_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::sdp_mem_correctable_enable(const cpp_int & _val) { 
    // sdp_mem_correctable_enable
    int_var__sdp_mem_correctable_enable = _val.convert_to< sdp_mem_correctable_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::sdp_mem_correctable_enable() const {
    return int_var__sdp_mem_correctable_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_0_enable(const cpp_int & _val) { 
    // illegal_op_0_enable
    int_var__illegal_op_0_enable = _val.convert_to< illegal_op_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_0_enable() const {
    return int_var__illegal_op_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_1_enable(const cpp_int & _val) { 
    // illegal_op_1_enable
    int_var__illegal_op_1_enable = _val.convert_to< illegal_op_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_1_enable() const {
    return int_var__illegal_op_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_2_enable(const cpp_int & _val) { 
    // illegal_op_2_enable
    int_var__illegal_op_2_enable = _val.convert_to< illegal_op_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_2_enable() const {
    return int_var__illegal_op_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_3_enable(const cpp_int & _val) { 
    // illegal_op_3_enable
    int_var__illegal_op_3_enable = _val.convert_to< illegal_op_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::illegal_op_3_enable() const {
    return int_var__illegal_op_3_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::max_inst_0_enable(const cpp_int & _val) { 
    // max_inst_0_enable
    int_var__max_inst_0_enable = _val.convert_to< max_inst_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::max_inst_0_enable() const {
    return int_var__max_inst_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::max_inst_1_enable(const cpp_int & _val) { 
    // max_inst_1_enable
    int_var__max_inst_1_enable = _val.convert_to< max_inst_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::max_inst_1_enable() const {
    return int_var__max_inst_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::max_inst_2_enable(const cpp_int & _val) { 
    // max_inst_2_enable
    int_var__max_inst_2_enable = _val.convert_to< max_inst_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::max_inst_2_enable() const {
    return int_var__max_inst_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::max_inst_3_enable(const cpp_int & _val) { 
    // max_inst_3_enable
    int_var__max_inst_3_enable = _val.convert_to< max_inst_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::max_inst_3_enable() const {
    return int_var__max_inst_3_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::phvwr_0_enable(const cpp_int & _val) { 
    // phvwr_0_enable
    int_var__phvwr_0_enable = _val.convert_to< phvwr_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::phvwr_0_enable() const {
    return int_var__phvwr_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::phvwr_1_enable(const cpp_int & _val) { 
    // phvwr_1_enable
    int_var__phvwr_1_enable = _val.convert_to< phvwr_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::phvwr_1_enable() const {
    return int_var__phvwr_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::phvwr_2_enable(const cpp_int & _val) { 
    // phvwr_2_enable
    int_var__phvwr_2_enable = _val.convert_to< phvwr_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::phvwr_2_enable() const {
    return int_var__phvwr_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::phvwr_3_enable(const cpp_int & _val) { 
    // phvwr_3_enable
    int_var__phvwr_3_enable = _val.convert_to< phvwr_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::phvwr_3_enable() const {
    return int_var__phvwr_3_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::write_err_0_enable(const cpp_int & _val) { 
    // write_err_0_enable
    int_var__write_err_0_enable = _val.convert_to< write_err_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::write_err_0_enable() const {
    return int_var__write_err_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::write_err_1_enable(const cpp_int & _val) { 
    // write_err_1_enable
    int_var__write_err_1_enable = _val.convert_to< write_err_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::write_err_1_enable() const {
    return int_var__write_err_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::write_err_2_enable(const cpp_int & _val) { 
    // write_err_2_enable
    int_var__write_err_2_enable = _val.convert_to< write_err_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::write_err_2_enable() const {
    return int_var__write_err_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::write_err_3_enable(const cpp_int & _val) { 
    // write_err_3_enable
    int_var__write_err_3_enable = _val.convert_to< write_err_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::write_err_3_enable() const {
    return int_var__write_err_3_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_0_enable(const cpp_int & _val) { 
    // cache_axi_0_enable
    int_var__cache_axi_0_enable = _val.convert_to< cache_axi_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_0_enable() const {
    return int_var__cache_axi_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_1_enable(const cpp_int & _val) { 
    // cache_axi_1_enable
    int_var__cache_axi_1_enable = _val.convert_to< cache_axi_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_1_enable() const {
    return int_var__cache_axi_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_2_enable(const cpp_int & _val) { 
    // cache_axi_2_enable
    int_var__cache_axi_2_enable = _val.convert_to< cache_axi_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_2_enable() const {
    return int_var__cache_axi_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_3_enable(const cpp_int & _val) { 
    // cache_axi_3_enable
    int_var__cache_axi_3_enable = _val.convert_to< cache_axi_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_axi_3_enable() const {
    return int_var__cache_axi_3_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_0_enable(const cpp_int & _val) { 
    // cache_parity_0_enable
    int_var__cache_parity_0_enable = _val.convert_to< cache_parity_0_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_0_enable() const {
    return int_var__cache_parity_0_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_1_enable(const cpp_int & _val) { 
    // cache_parity_1_enable
    int_var__cache_parity_1_enable = _val.convert_to< cache_parity_1_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_1_enable() const {
    return int_var__cache_parity_1_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_2_enable(const cpp_int & _val) { 
    // cache_parity_2_enable
    int_var__cache_parity_2_enable = _val.convert_to< cache_parity_2_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_2_enable() const {
    return int_var__cache_parity_2_enable;
}
    
void cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_3_enable(const cpp_int & _val) { 
    // cache_parity_3_enable
    int_var__cache_parity_3_enable = _val.convert_to< cache_parity_3_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_int_enable_clear_t::cache_parity_3_enable() const {
    return int_var__cache_parity_3_enable;
}
    
void cap_mpu_csr_int_err_intreg_t::results_mismatch_interrupt(const cpp_int & _val) { 
    // results_mismatch_interrupt
    int_var__results_mismatch_interrupt = _val.convert_to< results_mismatch_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::results_mismatch_interrupt() const {
    return int_var__results_mismatch_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::sdp_mem_uncorrectable_interrupt(const cpp_int & _val) { 
    // sdp_mem_uncorrectable_interrupt
    int_var__sdp_mem_uncorrectable_interrupt = _val.convert_to< sdp_mem_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::sdp_mem_uncorrectable_interrupt() const {
    return int_var__sdp_mem_uncorrectable_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::sdp_mem_correctable_interrupt(const cpp_int & _val) { 
    // sdp_mem_correctable_interrupt
    int_var__sdp_mem_correctable_interrupt = _val.convert_to< sdp_mem_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::sdp_mem_correctable_interrupt() const {
    return int_var__sdp_mem_correctable_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::illegal_op_0_interrupt(const cpp_int & _val) { 
    // illegal_op_0_interrupt
    int_var__illegal_op_0_interrupt = _val.convert_to< illegal_op_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::illegal_op_0_interrupt() const {
    return int_var__illegal_op_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::illegal_op_1_interrupt(const cpp_int & _val) { 
    // illegal_op_1_interrupt
    int_var__illegal_op_1_interrupt = _val.convert_to< illegal_op_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::illegal_op_1_interrupt() const {
    return int_var__illegal_op_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::illegal_op_2_interrupt(const cpp_int & _val) { 
    // illegal_op_2_interrupt
    int_var__illegal_op_2_interrupt = _val.convert_to< illegal_op_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::illegal_op_2_interrupt() const {
    return int_var__illegal_op_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::illegal_op_3_interrupt(const cpp_int & _val) { 
    // illegal_op_3_interrupt
    int_var__illegal_op_3_interrupt = _val.convert_to< illegal_op_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::illegal_op_3_interrupt() const {
    return int_var__illegal_op_3_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::max_inst_0_interrupt(const cpp_int & _val) { 
    // max_inst_0_interrupt
    int_var__max_inst_0_interrupt = _val.convert_to< max_inst_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::max_inst_0_interrupt() const {
    return int_var__max_inst_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::max_inst_1_interrupt(const cpp_int & _val) { 
    // max_inst_1_interrupt
    int_var__max_inst_1_interrupt = _val.convert_to< max_inst_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::max_inst_1_interrupt() const {
    return int_var__max_inst_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::max_inst_2_interrupt(const cpp_int & _val) { 
    // max_inst_2_interrupt
    int_var__max_inst_2_interrupt = _val.convert_to< max_inst_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::max_inst_2_interrupt() const {
    return int_var__max_inst_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::max_inst_3_interrupt(const cpp_int & _val) { 
    // max_inst_3_interrupt
    int_var__max_inst_3_interrupt = _val.convert_to< max_inst_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::max_inst_3_interrupt() const {
    return int_var__max_inst_3_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::phvwr_0_interrupt(const cpp_int & _val) { 
    // phvwr_0_interrupt
    int_var__phvwr_0_interrupt = _val.convert_to< phvwr_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::phvwr_0_interrupt() const {
    return int_var__phvwr_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::phvwr_1_interrupt(const cpp_int & _val) { 
    // phvwr_1_interrupt
    int_var__phvwr_1_interrupt = _val.convert_to< phvwr_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::phvwr_1_interrupt() const {
    return int_var__phvwr_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::phvwr_2_interrupt(const cpp_int & _val) { 
    // phvwr_2_interrupt
    int_var__phvwr_2_interrupt = _val.convert_to< phvwr_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::phvwr_2_interrupt() const {
    return int_var__phvwr_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::phvwr_3_interrupt(const cpp_int & _val) { 
    // phvwr_3_interrupt
    int_var__phvwr_3_interrupt = _val.convert_to< phvwr_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::phvwr_3_interrupt() const {
    return int_var__phvwr_3_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::write_err_0_interrupt(const cpp_int & _val) { 
    // write_err_0_interrupt
    int_var__write_err_0_interrupt = _val.convert_to< write_err_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::write_err_0_interrupt() const {
    return int_var__write_err_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::write_err_1_interrupt(const cpp_int & _val) { 
    // write_err_1_interrupt
    int_var__write_err_1_interrupt = _val.convert_to< write_err_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::write_err_1_interrupt() const {
    return int_var__write_err_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::write_err_2_interrupt(const cpp_int & _val) { 
    // write_err_2_interrupt
    int_var__write_err_2_interrupt = _val.convert_to< write_err_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::write_err_2_interrupt() const {
    return int_var__write_err_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::write_err_3_interrupt(const cpp_int & _val) { 
    // write_err_3_interrupt
    int_var__write_err_3_interrupt = _val.convert_to< write_err_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::write_err_3_interrupt() const {
    return int_var__write_err_3_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_axi_0_interrupt(const cpp_int & _val) { 
    // cache_axi_0_interrupt
    int_var__cache_axi_0_interrupt = _val.convert_to< cache_axi_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_axi_0_interrupt() const {
    return int_var__cache_axi_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_axi_1_interrupt(const cpp_int & _val) { 
    // cache_axi_1_interrupt
    int_var__cache_axi_1_interrupt = _val.convert_to< cache_axi_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_axi_1_interrupt() const {
    return int_var__cache_axi_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_axi_2_interrupt(const cpp_int & _val) { 
    // cache_axi_2_interrupt
    int_var__cache_axi_2_interrupt = _val.convert_to< cache_axi_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_axi_2_interrupt() const {
    return int_var__cache_axi_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_axi_3_interrupt(const cpp_int & _val) { 
    // cache_axi_3_interrupt
    int_var__cache_axi_3_interrupt = _val.convert_to< cache_axi_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_axi_3_interrupt() const {
    return int_var__cache_axi_3_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_parity_0_interrupt(const cpp_int & _val) { 
    // cache_parity_0_interrupt
    int_var__cache_parity_0_interrupt = _val.convert_to< cache_parity_0_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_parity_0_interrupt() const {
    return int_var__cache_parity_0_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_parity_1_interrupt(const cpp_int & _val) { 
    // cache_parity_1_interrupt
    int_var__cache_parity_1_interrupt = _val.convert_to< cache_parity_1_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_parity_1_interrupt() const {
    return int_var__cache_parity_1_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_parity_2_interrupt(const cpp_int & _val) { 
    // cache_parity_2_interrupt
    int_var__cache_parity_2_interrupt = _val.convert_to< cache_parity_2_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_parity_2_interrupt() const {
    return int_var__cache_parity_2_interrupt;
}
    
void cap_mpu_csr_int_err_intreg_t::cache_parity_3_interrupt(const cpp_int & _val) { 
    // cache_parity_3_interrupt
    int_var__cache_parity_3_interrupt = _val.convert_to< cache_parity_3_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_err_intreg_t::cache_parity_3_interrupt() const {
    return int_var__cache_parity_3_interrupt;
}
    
void cap_mpu_csr_intreg_status_t::int_err_interrupt(const cpp_int & _val) { 
    // int_err_interrupt
    int_var__int_err_interrupt = _val.convert_to< int_err_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_intreg_status_t::int_err_interrupt() const {
    return int_var__int_err_interrupt;
}
    
void cap_mpu_csr_intreg_status_t::int_info_interrupt(const cpp_int & _val) { 
    // int_info_interrupt
    int_var__int_info_interrupt = _val.convert_to< int_info_interrupt_cpp_int_t >();
}

cpp_int cap_mpu_csr_intreg_status_t::int_info_interrupt() const {
    return int_var__int_info_interrupt;
}
    
void cap_mpu_csr_int_groups_int_enable_rw_reg_t::int_err_enable(const cpp_int & _val) { 
    // int_err_enable
    int_var__int_err_enable = _val.convert_to< int_err_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_groups_int_enable_rw_reg_t::int_err_enable() const {
    return int_var__int_err_enable;
}
    
void cap_mpu_csr_int_groups_int_enable_rw_reg_t::int_info_enable(const cpp_int & _val) { 
    // int_info_enable
    int_var__int_info_enable = _val.convert_to< int_info_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_int_groups_int_enable_rw_reg_t::int_info_enable() const {
    return int_var__int_info_enable;
}
    
void cap_mpu_csr_spr_reg_data_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_spr_reg_data_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_icache_sram_read_data_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_icache_sram_read_data_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_tag_read_data_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_tag_read_data_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_mpu_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_mpu_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_mpu_csr_sta_spare_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_spare_t::spare() const {
    return int_var__spare;
}
    
void cap_mpu_csr_cfg_spare_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_spare_t::spare() const {
    return int_var__spare;
}
    
void cap_mpu_csr_sta_mpu3_icache_tag_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_tag_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu3_icache_tag_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_tag_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_cfg_mpu3_icache_tag_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_tag_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu2_icache_tag_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_tag_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu2_icache_tag_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_tag_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_cfg_mpu2_icache_tag_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_tag_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu1_icache_tag_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_tag_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu1_icache_tag_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_tag_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_cfg_mpu1_icache_tag_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_tag_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu0_icache_tag_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_tag_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu0_icache_tag_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_tag_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_cfg_mpu0_icache_tag_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu0_icache_tag_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu3_icache_data1_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data1_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu3_icache_data1_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data1_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu3_icache_data1_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data1_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu3_icache_data1_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data1_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu3_icache_data1_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_data1_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu3_icache_data0_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data0_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu3_icache_data0_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data0_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu3_icache_data0_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data0_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu3_icache_data0_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu3_icache_data0_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu3_icache_data0_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu3_icache_data0_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu2_icache_data1_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data1_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu2_icache_data1_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data1_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu2_icache_data1_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data1_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu2_icache_data1_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data1_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu2_icache_data1_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_data1_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu2_icache_data0_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data0_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu2_icache_data0_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data0_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu2_icache_data0_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data0_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu2_icache_data0_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu2_icache_data0_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu2_icache_data0_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu2_icache_data0_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu1_icache_data1_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data1_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu1_icache_data1_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data1_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu1_icache_data1_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data1_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu1_icache_data1_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data1_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu1_icache_data1_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_data1_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu1_icache_data0_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data0_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu1_icache_data0_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data0_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu1_icache_data0_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data0_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu1_icache_data0_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu1_icache_data0_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu1_icache_data0_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu1_icache_data0_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu0_icache_data1_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data1_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu0_icache_data1_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data1_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu0_icache_data1_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data1_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu0_icache_data1_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data1_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu0_icache_data1_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu0_icache_data1_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_mpu0_icache_data0_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data0_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_mpu0_icache_data0_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data0_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_mpu0_icache_data0_t::parity_error(const cpp_int & _val) { 
    // parity_error
    int_var__parity_error = _val.convert_to< parity_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data0_t::parity_error() const {
    return int_var__parity_error;
}
    
void cap_mpu_csr_sta_mpu0_icache_data0_t::parity_error_addr(const cpp_int & _val) { 
    // parity_error_addr
    int_var__parity_error_addr = _val.convert_to< parity_error_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu0_icache_data0_t::parity_error_addr() const {
    return int_var__parity_error_addr;
}
    
void cap_mpu_csr_cfg_mpu0_icache_data0_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_mpu0_icache_data0_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_sta_sdp_mem_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_mpu_csr_sta_sdp_mem_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_mpu_csr_sta_sdp_mem_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::correctable() const {
    return int_var__correctable;
}
    
void cap_mpu_csr_sta_sdp_mem_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_mpu_csr_sta_sdp_mem_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_mpu_csr_sta_sdp_mem_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_sdp_mem_t::addr() const {
    return int_var__addr;
}
    
void cap_mpu_csr_cfg_sdp_mem_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_sdp_mem_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_mpu_csr_cfg_sdp_mem_t::ecc_disable_det(const cpp_int & _val) { 
    // ecc_disable_det
    int_var__ecc_disable_det = _val.convert_to< ecc_disable_det_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_sdp_mem_t::ecc_disable_det() const {
    return int_var__ecc_disable_det;
}
    
void cap_mpu_csr_cfg_sdp_mem_t::ecc_disable_cor(const cpp_int & _val) { 
    // ecc_disable_cor
    int_var__ecc_disable_cor = _val.convert_to< ecc_disable_cor_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_sdp_mem_t::ecc_disable_cor() const {
    return int_var__ecc_disable_cor;
}
    
void cap_mpu_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_debug_port_t::enable() const {
    return int_var__enable;
}
    
void cap_mpu_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_mpu_csr_cfg_debug_port_t::select() const {
    return int_var__select;
}
    
void cap_mpu_csr_mismatch_t::update_result_count(const cpp_int & _val) { 
    // update_result_count
    int_var__update_result_count = _val.convert_to< update_result_count_cpp_int_t >();
}

cpp_int cap_mpu_csr_mismatch_t::update_result_count() const {
    return int_var__update_result_count;
}
    
void cap_mpu_csr_mismatch_t::sdp_result_count(const cpp_int & _val) { 
    // sdp_result_count
    int_var__sdp_result_count = _val.convert_to< sdp_result_count_cpp_int_t >();
}

cpp_int cap_mpu_csr_mismatch_t::sdp_result_count() const {
    return int_var__sdp_result_count;
}
    
void cap_mpu_csr_mismatch_t::pkt_id(const cpp_int & _val) { 
    // pkt_id
    int_var__pkt_id = _val.convert_to< pkt_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_mismatch_t::pkt_id() const {
    return int_var__pkt_id;
}
    
void cap_mpu_csr_STA_stg_t::sdp_srdy_in(const cpp_int & _val) { 
    // sdp_srdy_in
    int_var__sdp_srdy_in = _val.convert_to< sdp_srdy_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::sdp_srdy_in() const {
    return int_var__sdp_srdy_in;
}
    
void cap_mpu_csr_STA_stg_t::sdp_drdy_in(const cpp_int & _val) { 
    // sdp_drdy_in
    int_var__sdp_drdy_in = _val.convert_to< sdp_drdy_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::sdp_drdy_in() const {
    return int_var__sdp_drdy_in;
}
    
void cap_mpu_csr_STA_stg_t::srdy_out(const cpp_int & _val) { 
    // srdy_out
    int_var__srdy_out = _val.convert_to< srdy_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::srdy_out() const {
    return int_var__srdy_out;
}
    
void cap_mpu_csr_STA_stg_t::drdy_out(const cpp_int & _val) { 
    // drdy_out
    int_var__drdy_out = _val.convert_to< drdy_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::drdy_out() const {
    return int_var__drdy_out;
}
    
void cap_mpu_csr_STA_stg_t::srdy(const cpp_int & _val) { 
    // srdy
    int_var__srdy = _val.convert_to< srdy_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::srdy() const {
    return int_var__srdy;
}
    
void cap_mpu_csr_STA_stg_t::drdy(const cpp_int & _val) { 
    // drdy
    int_var__drdy = _val.convert_to< drdy_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::drdy() const {
    return int_var__drdy;
}
    
void cap_mpu_csr_STA_stg_t::te_valid(const cpp_int & _val) { 
    // te_valid
    int_var__te_valid = _val.convert_to< te_valid_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::te_valid() const {
    return int_var__te_valid;
}
    
void cap_mpu_csr_STA_stg_t::mpu_processing(const cpp_int & _val) { 
    // mpu_processing
    int_var__mpu_processing = _val.convert_to< mpu_processing_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_stg_t::mpu_processing() const {
    return int_var__mpu_processing;
}
    
void cap_mpu_csr_STA_live_sdp_t::update_empty(const cpp_int & _val) { 
    // update_empty
    int_var__update_empty = _val.convert_to< update_empty_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::update_empty() const {
    return int_var__update_empty;
}
    
void cap_mpu_csr_STA_live_sdp_t::update_pkt_id(const cpp_int & _val) { 
    // update_pkt_id
    int_var__update_pkt_id = _val.convert_to< update_pkt_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::update_pkt_id() const {
    return int_var__update_pkt_id;
}
    
void cap_mpu_csr_STA_live_sdp_t::phv_pkt_id(const cpp_int & _val) { 
    // phv_pkt_id
    int_var__phv_pkt_id = _val.convert_to< phv_pkt_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::phv_pkt_id() const {
    return int_var__phv_pkt_id;
}
    
void cap_mpu_csr_STA_live_sdp_t::phv_update_valid(const cpp_int & _val) { 
    // phv_update_valid
    int_var__phv_update_valid = _val.convert_to< phv_update_valid_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::phv_update_valid() const {
    return int_var__phv_update_valid;
}
    
void cap_mpu_csr_STA_live_sdp_t::update_result_count(const cpp_int & _val) { 
    // update_result_count
    int_var__update_result_count = _val.convert_to< update_result_count_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::update_result_count() const {
    return int_var__update_result_count;
}
    
void cap_mpu_csr_STA_live_sdp_t::staging_full(const cpp_int & _val) { 
    // staging_full
    int_var__staging_full = _val.convert_to< staging_full_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::staging_full() const {
    return int_var__staging_full;
}
    
void cap_mpu_csr_STA_live_sdp_t::wptr(const cpp_int & _val) { 
    // wptr
    int_var__wptr = _val.convert_to< wptr_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::wptr() const {
    return int_var__wptr;
}
    
void cap_mpu_csr_STA_live_sdp_t::rptr(const cpp_int & _val) { 
    // rptr
    int_var__rptr = _val.convert_to< rptr_cpp_int_t >();
}

cpp_int cap_mpu_csr_STA_live_sdp_t::rptr() const {
    return int_var__rptr;
}
    
void cap_mpu_csr_CNT_sdp_t::phv_fifo_depth(const cpp_int & _val) { 
    // phv_fifo_depth
    int_var__phv_fifo_depth = _val.convert_to< phv_fifo_depth_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::phv_fifo_depth() const {
    return int_var__phv_fifo_depth;
}
    
void cap_mpu_csr_CNT_sdp_t::sop_in(const cpp_int & _val) { 
    // sop_in
    int_var__sop_in = _val.convert_to< sop_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::sop_in() const {
    return int_var__sop_in;
}
    
void cap_mpu_csr_CNT_sdp_t::eop_in(const cpp_int & _val) { 
    // eop_in
    int_var__eop_in = _val.convert_to< eop_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::eop_in() const {
    return int_var__eop_in;
}
    
void cap_mpu_csr_CNT_sdp_t::sop_out(const cpp_int & _val) { 
    // sop_out
    int_var__sop_out = _val.convert_to< sop_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::sop_out() const {
    return int_var__sop_out;
}
    
void cap_mpu_csr_CNT_sdp_t::eop_out(const cpp_int & _val) { 
    // eop_out
    int_var__eop_out = _val.convert_to< eop_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::eop_out() const {
    return int_var__eop_out;
}
    
void cap_mpu_csr_sta_error_t::icache0(const cpp_int & _val) { 
    // icache0
    int_var__icache0 = _val.convert_to< icache0_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_error_t::icache0() const {
    return int_var__icache0;
}
    
void cap_mpu_csr_sta_error_t::icache1(const cpp_int & _val) { 
    // icache1
    int_var__icache1 = _val.convert_to< icache1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_error_t::icache1() const {
    return int_var__icache1;
}
    
void cap_mpu_csr_sta_pend_t::wr_id(const cpp_int & _val) { 
    // wr_id
    int_var__wr_id = _val.convert_to< wr_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_pend_t::wr_id() const {
    return int_var__wr_id;
}
    
void cap_mpu_csr_sta_pend_t::pending_table_write_valid0(const cpp_int & _val) { 
    // pending_table_write_valid0
    int_var__pending_table_write_valid0 = _val.convert_to< pending_table_write_valid0_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_pend_t::pending_table_write_valid0() const {
    return int_var__pending_table_write_valid0;
}
    
void cap_mpu_csr_sta_pend_t::pending_table_write_valid1(const cpp_int & _val) { 
    // pending_table_write_valid1
    int_var__pending_table_write_valid1 = _val.convert_to< pending_table_write_valid1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_pend_t::pending_table_write_valid1() const {
    return int_var__pending_table_write_valid1;
}
    
void cap_mpu_csr_sta_gpr7_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr7_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr6_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr6_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr5_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr5_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr4_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr4_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr3_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr3_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr2_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr2_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_gpr1_mpu_t::value(const cpp_int & _val) { 
    // value
    int_var__value = _val.convert_to< value_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_gpr1_mpu_t::value() const {
    return int_var__value;
}
    
void cap_mpu_csr_sta_mpu_t::c1(const cpp_int & _val) { 
    // c1
    int_var__c1 = _val.convert_to< c1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c1() const {
    return int_var__c1;
}
    
void cap_mpu_csr_sta_mpu_t::c2(const cpp_int & _val) { 
    // c2
    int_var__c2 = _val.convert_to< c2_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c2() const {
    return int_var__c2;
}
    
void cap_mpu_csr_sta_mpu_t::c3(const cpp_int & _val) { 
    // c3
    int_var__c3 = _val.convert_to< c3_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c3() const {
    return int_var__c3;
}
    
void cap_mpu_csr_sta_mpu_t::c4(const cpp_int & _val) { 
    // c4
    int_var__c4 = _val.convert_to< c4_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c4() const {
    return int_var__c4;
}
    
void cap_mpu_csr_sta_mpu_t::c5(const cpp_int & _val) { 
    // c5
    int_var__c5 = _val.convert_to< c5_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c5() const {
    return int_var__c5;
}
    
void cap_mpu_csr_sta_mpu_t::c6(const cpp_int & _val) { 
    // c6
    int_var__c6 = _val.convert_to< c6_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c6() const {
    return int_var__c6;
}
    
void cap_mpu_csr_sta_mpu_t::c7(const cpp_int & _val) { 
    // c7
    int_var__c7 = _val.convert_to< c7_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c7() const {
    return int_var__c7;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::stopped(const cpp_int & _val) { 
    // stopped
    int_var__stopped = _val.convert_to< stopped_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::stopped() const {
    return int_var__stopped;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::ex_valid(const cpp_int & _val) { 
    // ex_valid
    int_var__ex_valid = _val.convert_to< ex_valid_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::ex_valid() const {
    return int_var__ex_valid;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::ex_execute(const cpp_int & _val) { 
    // ex_execute
    int_var__ex_execute = _val.convert_to< ex_execute_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::ex_execute() const {
    return int_var__ex_execute;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::ex_pc(const cpp_int & _val) { 
    // ex_pc
    int_var__ex_pc = _val.convert_to< ex_pc_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::ex_pc() const {
    return int_var__ex_pc;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::icache_state(const cpp_int & _val) { 
    // icache_state
    int_var__icache_state = _val.convert_to< icache_state_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::icache_state() const {
    return int_var__icache_state;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::phv_cmd_fifo_depth(const cpp_int & _val) { 
    // phv_cmd_fifo_depth
    int_var__phv_cmd_fifo_depth = _val.convert_to< phv_cmd_fifo_depth_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::phv_cmd_fifo_depth() const {
    return int_var__phv_cmd_fifo_depth;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::phv_data_fifo_depth(const cpp_int & _val) { 
    // phv_data_fifo_depth
    int_var__phv_data_fifo_depth = _val.convert_to< phv_data_fifo_depth_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::phv_data_fifo_depth() const {
    return int_var__phv_data_fifo_depth;
}
    
void cap_mpu_csr_sta_ctl_mpu_t::stall_vector(const cpp_int & _val) { 
    // stall_vector
    int_var__stall_vector = _val.convert_to< stall_vector_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_ctl_mpu_t::stall_vector() const {
    return int_var__stall_vector;
}
    
void cap_mpu_csr_sta_pc_mpu_t::inst(const cpp_int & _val) { 
    // inst
    int_var__inst = _val.convert_to< inst_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_pc_mpu_t::inst() const {
    return int_var__inst;
}
    
void cap_mpu_csr_sta_key_t::kd(const cpp_int & _val) { 
    // kd
    int_var__kd = _val.convert_to< kd_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_key_t::kd() const {
    return int_var__kd;
}
    
void cap_mpu_csr_sta_data_t::td(const cpp_int & _val) { 
    // td
    int_var__td = _val.convert_to< td_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_data_t::td() const {
    return int_var__td;
}
    
void cap_mpu_csr_sta_table_t::mpu_processing_table_pcie(const cpp_int & _val) { 
    // mpu_processing_table_pcie
    int_var__mpu_processing_table_pcie = _val.convert_to< mpu_processing_table_pcie_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_table_t::mpu_processing_table_pcie() const {
    return int_var__mpu_processing_table_pcie;
}
    
void cap_mpu_csr_sta_table_t::mpu_processing_table_sram(const cpp_int & _val) { 
    // mpu_processing_table_sram
    int_var__mpu_processing_table_sram = _val.convert_to< mpu_processing_table_sram_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_table_t::mpu_processing_table_sram() const {
    return int_var__mpu_processing_table_sram;
}
    
void cap_mpu_csr_sta_table_t::mpu_processing_table_id(const cpp_int & _val) { 
    // mpu_processing_table_id
    int_var__mpu_processing_table_id = _val.convert_to< mpu_processing_table_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_table_t::mpu_processing_table_id() const {
    return int_var__mpu_processing_table_id;
}
    
void cap_mpu_csr_sta_table_t::mpu_processing_pkt_id(const cpp_int & _val) { 
    // mpu_processing_pkt_id
    int_var__mpu_processing_pkt_id = _val.convert_to< mpu_processing_pkt_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_table_t::mpu_processing_pkt_id() const {
    return int_var__mpu_processing_pkt_id;
}
    
void cap_mpu_csr_sta_table_t::mpu_processing_table_latency(const cpp_int & _val) { 
    // mpu_processing_table_latency
    int_var__mpu_processing_table_latency = _val.convert_to< mpu_processing_table_latency_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_table_t::mpu_processing_table_latency() const {
    return int_var__mpu_processing_table_latency;
}
    
void cap_mpu_csr_sta_tbl_addr_t::in_mpu(const cpp_int & _val) { 
    // in_mpu
    int_var__in_mpu = _val.convert_to< in_mpu_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_tbl_addr_t::in_mpu() const {
    return int_var__in_mpu;
}
    
void cap_mpu_csr_CNT_fence_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_fence_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_tblwr_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_tblwr_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_memwr_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_memwr_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_phvwr_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_phvwr_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_hazard_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_hazard_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_icache_fill_stall_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_icache_fill_stall_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_icache_miss_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_icache_miss_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_inst_executed_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_inst_executed_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_phv_executed_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_phv_executed_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_CNT_cycles_t::counter(const cpp_int & _val) { 
    // counter
    int_var__counter = _val.convert_to< counter_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_cycles_t::counter() const {
    return int_var__counter;
}
    
void cap_mpu_csr_count_stage_t::always_on(const cpp_int & _val) { 
    // always_on
    int_var__always_on = _val.convert_to< always_on_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::always_on() const {
    return int_var__always_on;
}
    
void cap_mpu_csr_count_stage_t::debug(const cpp_int & _val) { 
    // debug
    int_var__debug = _val.convert_to< debug_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::debug() const {
    return int_var__debug;
}
    
void cap_mpu_csr_count_stage_t::watch(const cpp_int & _val) { 
    // watch
    int_var__watch = _val.convert_to< watch_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::watch() const {
    return int_var__watch;
}
    
void cap_mpu_csr_count_stage_t::stop_on_saturate(const cpp_int & _val) { 
    // stop_on_saturate
    int_var__stop_on_saturate = _val.convert_to< stop_on_saturate_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::stop_on_saturate() const {
    return int_var__stop_on_saturate;
}
    
void cap_mpu_csr_mpu_run_t::start_pulse(const cpp_int & _val) { 
    // start_pulse
    int_var__start_pulse = _val.convert_to< start_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::start_pulse() const {
    return int_var__start_pulse;
}
    
void cap_mpu_csr_mpu_run_t::stop_pulse(const cpp_int & _val) { 
    // stop_pulse
    int_var__stop_pulse = _val.convert_to< stop_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::stop_pulse() const {
    return int_var__stop_pulse;
}
    
void cap_mpu_csr_mpu_run_t::step_pulse(const cpp_int & _val) { 
    // step_pulse
    int_var__step_pulse = _val.convert_to< step_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::step_pulse() const {
    return int_var__step_pulse;
}
    
void cap_mpu_csr_mpu_cfg_t::stall_wb_full(const cpp_int & _val) { 
    // stall_wb_full
    int_var__stall_wb_full = _val.convert_to< stall_wb_full_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_cfg_t::stall_wb_full() const {
    return int_var__stall_wb_full;
}
    
void cap_mpu_csr_mpu_cfg_t::icache_table_id(const cpp_int & _val) { 
    // icache_table_id
    int_var__icache_table_id = _val.convert_to< icache_table_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_cfg_t::icache_table_id() const {
    return int_var__icache_table_id;
}
    
void cap_mpu_csr_mpu_cfg_t::max_inst_count(const cpp_int & _val) { 
    // max_inst_count
    int_var__max_inst_count = _val.convert_to< max_inst_count_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_cfg_t::max_inst_count() const {
    return int_var__max_inst_count;
}
    
void cap_mpu_csr_mpu_cfg_t::te_clean_wb_enable(const cpp_int & _val) { 
    // te_clean_wb_enable
    int_var__te_clean_wb_enable = _val.convert_to< te_clean_wb_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_cfg_t::te_clean_wb_enable() const {
    return int_var__te_clean_wb_enable;
}
    
void cap_mpu_csr_mpu_ctl_t::enable_stop(const cpp_int & _val) { 
    // enable_stop
    int_var__enable_stop = _val.convert_to< enable_stop_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_ctl_t::enable_stop() const {
    return int_var__enable_stop;
}
    
void cap_mpu_csr_mpu_ctl_t::enable_error_stop(const cpp_int & _val) { 
    // enable_error_stop
    int_var__enable_error_stop = _val.convert_to< enable_error_stop_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_ctl_t::enable_error_stop() const {
    return int_var__enable_error_stop;
}
    
void cap_mpu_csr_mpu_ctl_t::enable_pipe_freeze_on_stop(const cpp_int & _val) { 
    // enable_pipe_freeze_on_stop
    int_var__enable_pipe_freeze_on_stop = _val.convert_to< enable_pipe_freeze_on_stop_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_ctl_t::enable_pipe_freeze_on_stop() const {
    return int_var__enable_pipe_freeze_on_stop;
}
    
void cap_mpu_csr_sdp_ctl_t::enable_compress(const cpp_int & _val) { 
    // enable_compress
    int_var__enable_compress = _val.convert_to< enable_compress_cpp_int_t >();
}

cpp_int cap_mpu_csr_sdp_ctl_t::enable_compress() const {
    return int_var__enable_compress;
}
    
void cap_mpu_csr_sdp_ctl_t::mask_frame_size(const cpp_int & _val) { 
    // mask_frame_size
    int_var__mask_frame_size = _val.convert_to< mask_frame_size_cpp_int_t >();
}

cpp_int cap_mpu_csr_sdp_ctl_t::mask_frame_size() const {
    return int_var__mask_frame_size;
}
    
void cap_mpu_csr_axi_attr_t::arcache(const cpp_int & _val) { 
    // arcache
    int_var__arcache = _val.convert_to< arcache_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::arcache() const {
    return int_var__arcache;
}
    
void cap_mpu_csr_axi_attr_t::awcache(const cpp_int & _val) { 
    // awcache
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::awcache() const {
    return int_var__awcache;
}
    
void cap_mpu_csr_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::prot() const {
    return int_var__prot;
}
    
void cap_mpu_csr_axi_attr_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::qos() const {
    return int_var__qos;
}
    
void cap_mpu_csr_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::lock() const {
    return int_var__lock;
}
    
void cap_mpu_csr_icache_t::invalidate(const cpp_int & _val) { 
    // invalidate
    int_var__invalidate = _val.convert_to< invalidate_cpp_int_t >();
}

cpp_int cap_mpu_csr_icache_t::invalidate() const {
    return int_var__invalidate;
}
    
void cap_mpu_csr_trace_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::enable() const {
    return int_var__enable;
}
    
void cap_mpu_csr_trace_t::trace_enable(const cpp_int & _val) { 
    // trace_enable
    int_var__trace_enable = _val.convert_to< trace_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::trace_enable() const {
    return int_var__trace_enable;
}
    
void cap_mpu_csr_trace_t::phv_debug(const cpp_int & _val) { 
    // phv_debug
    int_var__phv_debug = _val.convert_to< phv_debug_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::phv_debug() const {
    return int_var__phv_debug;
}
    
void cap_mpu_csr_trace_t::phv_error(const cpp_int & _val) { 
    // phv_error
    int_var__phv_error = _val.convert_to< phv_error_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::phv_error() const {
    return int_var__phv_error;
}
    
void cap_mpu_csr_trace_t::watch_enable(const cpp_int & _val) { 
    // watch_enable
    int_var__watch_enable = _val.convert_to< watch_enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::watch_enable() const {
    return int_var__watch_enable;
}
    
void cap_mpu_csr_trace_t::watch_pc(const cpp_int & _val) { 
    // watch_pc
    int_var__watch_pc = _val.convert_to< watch_pc_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::watch_pc() const {
    return int_var__watch_pc;
}
    
void cap_mpu_csr_trace_t::table_and_key(const cpp_int & _val) { 
    // table_and_key
    int_var__table_and_key = _val.convert_to< table_and_key_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::table_and_key() const {
    return int_var__table_and_key;
}
    
void cap_mpu_csr_trace_t::instructions(const cpp_int & _val) { 
    // instructions
    int_var__instructions = _val.convert_to< instructions_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::instructions() const {
    return int_var__instructions;
}
    
void cap_mpu_csr_trace_t::wrap(const cpp_int & _val) { 
    // wrap
    int_var__wrap = _val.convert_to< wrap_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::wrap() const {
    return int_var__wrap;
}
    
void cap_mpu_csr_trace_t::rst(const cpp_int & _val) { 
    // rst
    int_var__rst = _val.convert_to< rst_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::rst() const {
    return int_var__rst;
}
    
void cap_mpu_csr_trace_t::base_addr(const cpp_int & _val) { 
    // base_addr
    int_var__base_addr = _val.convert_to< base_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::base_addr() const {
    return int_var__base_addr;
}
    
void cap_mpu_csr_trace_t::buf_size(const cpp_int & _val) { 
    // buf_size
    int_var__buf_size = _val.convert_to< buf_size_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::buf_size() const {
    return int_var__buf_size;
}
    
void cap_mpu_csr_trace_t::debug_index(const cpp_int & _val) { 
    // debug_index
    int_var__debug_index = _val.convert_to< debug_index_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::debug_index() const {
    return int_var__debug_index;
}
    
void cap_mpu_csr_trace_t::debug_generation(const cpp_int & _val) { 
    // debug_generation
    int_var__debug_generation = _val.convert_to< debug_generation_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::debug_generation() const {
    return int_var__debug_generation;
}
    
void cap_mpu_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_mpu_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "trace_full_0_enable")) { field_val = trace_full_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_1_enable")) { field_val = trace_full_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_2_enable")) { field_val = trace_full_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_3_enable")) { field_val = trace_full_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_0_enable")) { field_val = mpu_stop_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_1_enable")) { field_val = mpu_stop_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_2_enable")) { field_val = mpu_stop_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_3_enable")) { field_val = mpu_stop_3_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_int_test_set_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "trace_full_0_interrupt")) { field_val = trace_full_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_1_interrupt")) { field_val = trace_full_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_2_interrupt")) { field_val = trace_full_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_3_interrupt")) { field_val = trace_full_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_0_interrupt")) { field_val = mpu_stop_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_1_interrupt")) { field_val = mpu_stop_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_2_interrupt")) { field_val = mpu_stop_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_3_interrupt")) { field_val = mpu_stop_3_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_int_err_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "results_mismatch_enable")) { field_val = results_mismatch_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_uncorrectable_enable")) { field_val = sdp_mem_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_correctable_enable")) { field_val = sdp_mem_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_0_enable")) { field_val = illegal_op_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_1_enable")) { field_val = illegal_op_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_2_enable")) { field_val = illegal_op_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_3_enable")) { field_val = illegal_op_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_0_enable")) { field_val = max_inst_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_1_enable")) { field_val = max_inst_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_2_enable")) { field_val = max_inst_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_3_enable")) { field_val = max_inst_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_0_enable")) { field_val = phvwr_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_1_enable")) { field_val = phvwr_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_2_enable")) { field_val = phvwr_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_3_enable")) { field_val = phvwr_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_0_enable")) { field_val = write_err_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_1_enable")) { field_val = write_err_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_2_enable")) { field_val = write_err_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_3_enable")) { field_val = write_err_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_0_enable")) { field_val = cache_axi_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_1_enable")) { field_val = cache_axi_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_2_enable")) { field_val = cache_axi_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_3_enable")) { field_val = cache_axi_3_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_0_enable")) { field_val = cache_parity_0_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_1_enable")) { field_val = cache_parity_1_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_2_enable")) { field_val = cache_parity_2_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_3_enable")) { field_val = cache_parity_3_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_err_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "results_mismatch_interrupt")) { field_val = results_mismatch_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_uncorrectable_interrupt")) { field_val = sdp_mem_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_correctable_interrupt")) { field_val = sdp_mem_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_0_interrupt")) { field_val = illegal_op_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_1_interrupt")) { field_val = illegal_op_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_2_interrupt")) { field_val = illegal_op_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_3_interrupt")) { field_val = illegal_op_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_0_interrupt")) { field_val = max_inst_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_1_interrupt")) { field_val = max_inst_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_2_interrupt")) { field_val = max_inst_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_3_interrupt")) { field_val = max_inst_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_0_interrupt")) { field_val = phvwr_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_1_interrupt")) { field_val = phvwr_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_2_interrupt")) { field_val = phvwr_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_3_interrupt")) { field_val = phvwr_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_0_interrupt")) { field_val = write_err_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_1_interrupt")) { field_val = write_err_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_2_interrupt")) { field_val = write_err_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_3_interrupt")) { field_val = write_err_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_0_interrupt")) { field_val = cache_axi_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_1_interrupt")) { field_val = cache_axi_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_2_interrupt")) { field_val = cache_axi_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_3_interrupt")) { field_val = cache_axi_3_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_0_interrupt")) { field_val = cache_parity_0_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_1_interrupt")) { field_val = cache_parity_1_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_2_interrupt")) { field_val = cache_parity_2_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_3_interrupt")) { field_val = cache_parity_3_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_err_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_interrupt")) { field_val = int_err_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_interrupt")) { field_val = int_info_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_enable")) { field_val = int_err_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_enable")) { field_val = int_info_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_spr_reg_data_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_spr_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = data.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_icache_sram_read_data_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_icache_sram_read_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = data.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_tag_read_data_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_tag_read_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = data.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_spare_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_spare_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_tag_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_data1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { field_val = parity_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { field_val = parity_error_addr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_data0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_sdp_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_cfg_sdp_mem_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { field_val = ecc_disable_det(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { field_val = ecc_disable_cor(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_debug_port_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { field_val = select(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mismatch_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "update_result_count")) { field_val = update_result_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_result_count")) { field_val = sdp_result_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_id")) { field_val = pkt_id(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_STA_stg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sdp_srdy_in")) { field_val = sdp_srdy_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_drdy_in")) { field_val = sdp_drdy_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "srdy_out")) { field_val = srdy_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drdy_out")) { field_val = drdy_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "srdy")) { field_val = srdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "drdy")) { field_val = drdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "te_valid")) { field_val = te_valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing")) { field_val = mpu_processing(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_STA_live_sdp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "update_empty")) { field_val = update_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "update_pkt_id")) { field_val = update_pkt_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_pkt_id")) { field_val = phv_pkt_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_update_valid")) { field_val = phv_update_valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "update_result_count")) { field_val = update_result_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "staging_full")) { field_val = staging_full(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wptr")) { field_val = wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rptr")) { field_val = rptr(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_sdp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phv_fifo_depth")) { field_val = phv_fifo_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop_in")) { field_val = sop_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop_in")) { field_val = eop_in(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop_out")) { field_val = sop_out(); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop_out")) { field_val = eop_out(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_error_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "icache0")) { field_val = icache0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache1")) { field_val = icache1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_pend_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "wr_id")) { field_val = wr_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_table_write_valid0")) { field_val = pending_table_write_valid0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_table_write_valid1")) { field_val = pending_table_write_valid1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr7_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr6_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr5_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr4_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr3_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr2_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr1_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { field_val = value(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "c1")) { field_val = c1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c2")) { field_val = c2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c3")) { field_val = c3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c4")) { field_val = c4(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c5")) { field_val = c5(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c6")) { field_val = c6(); field_found=1; }
    if(!field_found && !strcmp(field_name, "c7")) { field_val = c7(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_ctl_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "stopped")) { field_val = stopped(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_valid")) { field_val = ex_valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_execute")) { field_val = ex_execute(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_pc")) { field_val = ex_pc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache_state")) { field_val = icache_state(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_cmd_fifo_depth")) { field_val = phv_cmd_fifo_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_data_fifo_depth")) { field_val = phv_data_fifo_depth(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stall_vector")) { field_val = stall_vector(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_pc_mpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "inst")) { field_val = inst(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_key_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "kd")) { field_val = kd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_data_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "td")) { field_val = td(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_table_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mpu_processing_table_pcie")) { field_val = mpu_processing_table_pcie(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_sram")) { field_val = mpu_processing_table_sram(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_id")) { field_val = mpu_processing_table_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_pkt_id")) { field_val = mpu_processing_pkt_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_latency")) { field_val = mpu_processing_table_latency(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_tbl_addr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_mpu")) { field_val = in_mpu(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_fence_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_tblwr_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_memwr_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_phvwr_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_hazard_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_icache_fill_stall_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_icache_miss_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_inst_executed_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_phv_executed_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_cycles_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { field_val = counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_count_stage_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "always_on")) { field_val = always_on(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug")) { field_val = debug(); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch")) { field_val = watch(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop_on_saturate")) { field_val = stop_on_saturate(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_run_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start_pulse")) { field_val = start_pulse(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop_pulse")) { field_val = stop_pulse(); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_pulse")) { field_val = step_pulse(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "stall_wb_full")) { field_val = stall_wb_full(); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache_table_id")) { field_val = icache_table_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_count")) { field_val = max_inst_count(); field_found=1; }
    if(!field_found && !strcmp(field_name, "te_clean_wb_enable")) { field_val = te_clean_wb_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable_stop")) { field_val = enable_stop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_error_stop")) { field_val = enable_error_stop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_pipe_freeze_on_stop")) { field_val = enable_pipe_freeze_on_stop(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sdp_ctl_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable_compress")) { field_val = enable_compress(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_frame_size")) { field_val = mask_frame_size(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_axi_attr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_icache_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "invalidate")) { field_val = invalidate(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_trace_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { field_val = enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_enable")) { field_val = trace_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_debug")) { field_val = phv_debug(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_error")) { field_val = phv_error(); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch_enable")) { field_val = watch_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch_pc")) { field_val = watch_pc(); field_found=1; }
    if(!field_found && !strcmp(field_name, "table_and_key")) { field_val = table_and_key(); field_found=1; }
    if(!field_found && !strcmp(field_name, "instructions")) { field_val = instructions(); field_found=1; }
    if(!field_found && !strcmp(field_name, "wrap")) { field_val = wrap(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rst")) { field_val = rst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "base_addr")) { field_val = base_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "buf_size")) { field_val = buf_size(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_index")) { field_val = debug_index(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_generation")) { field_val = debug_generation(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = icache.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi_attr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sdp_ctl.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mpu_ctl.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mpu_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = count_stage.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = CNT_sdp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_live_sdp.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_stg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mismatch.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sdp_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sdp_mem.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_data0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_data1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_tag.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_spare.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_spare.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = spr_reg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_err.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_info.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "trace_full_0_enable")) { trace_full_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_1_enable")) { trace_full_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_2_enable")) { trace_full_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_3_enable")) { trace_full_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_0_enable")) { mpu_stop_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_1_enable")) { mpu_stop_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_2_enable")) { mpu_stop_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_3_enable")) { mpu_stop_3_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_int_test_set_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "trace_full_0_interrupt")) { trace_full_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_1_interrupt")) { trace_full_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_2_interrupt")) { trace_full_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_full_3_interrupt")) { trace_full_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_0_interrupt")) { mpu_stop_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_1_interrupt")) { mpu_stop_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_2_interrupt")) { mpu_stop_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_stop_3_interrupt")) { mpu_stop_3_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_info_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_int_err_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "results_mismatch_enable")) { results_mismatch_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_uncorrectable_enable")) { sdp_mem_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_correctable_enable")) { sdp_mem_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_0_enable")) { illegal_op_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_1_enable")) { illegal_op_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_2_enable")) { illegal_op_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_3_enable")) { illegal_op_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_0_enable")) { max_inst_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_1_enable")) { max_inst_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_2_enable")) { max_inst_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_3_enable")) { max_inst_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_0_enable")) { phvwr_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_1_enable")) { phvwr_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_2_enable")) { phvwr_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_3_enable")) { phvwr_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_0_enable")) { write_err_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_1_enable")) { write_err_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_2_enable")) { write_err_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_3_enable")) { write_err_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_0_enable")) { cache_axi_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_1_enable")) { cache_axi_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_2_enable")) { cache_axi_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_3_enable")) { cache_axi_3_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_0_enable")) { cache_parity_0_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_1_enable")) { cache_parity_1_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_2_enable")) { cache_parity_2_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_3_enable")) { cache_parity_3_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_err_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "results_mismatch_interrupt")) { results_mismatch_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_uncorrectable_interrupt")) { sdp_mem_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_mem_correctable_interrupt")) { sdp_mem_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_0_interrupt")) { illegal_op_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_1_interrupt")) { illegal_op_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_2_interrupt")) { illegal_op_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "illegal_op_3_interrupt")) { illegal_op_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_0_interrupt")) { max_inst_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_1_interrupt")) { max_inst_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_2_interrupt")) { max_inst_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_3_interrupt")) { max_inst_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_0_interrupt")) { phvwr_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_1_interrupt")) { phvwr_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_2_interrupt")) { phvwr_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phvwr_3_interrupt")) { phvwr_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_0_interrupt")) { write_err_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_1_interrupt")) { write_err_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_2_interrupt")) { write_err_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "write_err_3_interrupt")) { write_err_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_0_interrupt")) { cache_axi_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_1_interrupt")) { cache_axi_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_2_interrupt")) { cache_axi_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_axi_3_interrupt")) { cache_axi_3_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_0_interrupt")) { cache_parity_0_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_1_interrupt")) { cache_parity_1_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_2_interrupt")) { cache_parity_2_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache_parity_3_interrupt")) { cache_parity_3_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_err_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_interrupt")) { int_err_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_interrupt")) { int_info_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "int_err_enable")) { int_err_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "int_info_enable")) { int_info_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_spr_reg_data_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_spr_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = data.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_icache_sram_read_data_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_icache_sram_read_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = data.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_tag_read_data_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_tag_read_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = data.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_spare_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_spare_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_tag_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu3_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu3_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu2_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu2_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu1_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu1_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_data1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu0_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error")) { parity_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "parity_error_addr")) { parity_error_addr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_mpu0_icache_data0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_sdp_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_cfg_sdp_mem_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_det")) { ecc_disable_det(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_disable_cor")) { ecc_disable_cor(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_cfg_debug_port_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "select")) { select(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mismatch_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "update_result_count")) { update_result_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_result_count")) { sdp_result_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_id")) { pkt_id(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_STA_stg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sdp_srdy_in")) { sdp_srdy_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sdp_drdy_in")) { sdp_drdy_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "srdy_out")) { srdy_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drdy_out")) { drdy_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "srdy")) { srdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "drdy")) { drdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "te_valid")) { te_valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing")) { mpu_processing(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_STA_live_sdp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "update_empty")) { update_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "update_pkt_id")) { update_pkt_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_pkt_id")) { phv_pkt_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_update_valid")) { phv_update_valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "update_result_count")) { update_result_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "staging_full")) { staging_full(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wptr")) { wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rptr")) { rptr(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_sdp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phv_fifo_depth")) { phv_fifo_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop_in")) { sop_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop_in")) { eop_in(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sop_out")) { sop_out(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "eop_out")) { eop_out(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_error_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "icache0")) { icache0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache1")) { icache1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_pend_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "wr_id")) { wr_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_table_write_valid0")) { pending_table_write_valid0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pending_table_write_valid1")) { pending_table_write_valid1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr7_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr6_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr5_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr4_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr3_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr2_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_gpr1_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "value")) { value(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "c1")) { c1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c2")) { c2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c3")) { c3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c4")) { c4(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c5")) { c5(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c6")) { c6(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "c7")) { c7(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_ctl_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "stopped")) { stopped(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_valid")) { ex_valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_execute")) { ex_execute(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ex_pc")) { ex_pc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache_state")) { icache_state(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_cmd_fifo_depth")) { phv_cmd_fifo_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_data_fifo_depth")) { phv_data_fifo_depth(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stall_vector")) { stall_vector(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_pc_mpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "inst")) { inst(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_key_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "kd")) { kd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_data_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "td")) { td(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_table_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mpu_processing_table_pcie")) { mpu_processing_table_pcie(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_sram")) { mpu_processing_table_sram(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_id")) { mpu_processing_table_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_pkt_id")) { mpu_processing_pkt_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mpu_processing_table_latency")) { mpu_processing_table_latency(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sta_tbl_addr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "in_mpu")) { in_mpu(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_fence_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_tblwr_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_memwr_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_phvwr_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_hazard_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_icache_fill_stall_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_icache_miss_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_inst_executed_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_phv_executed_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_CNT_cycles_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "counter")) { counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_count_stage_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "always_on")) { always_on(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug")) { debug(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch")) { watch(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop_on_saturate")) { stop_on_saturate(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_run_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "start_pulse")) { start_pulse(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop_pulse")) { stop_pulse(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "step_pulse")) { step_pulse(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "stall_wb_full")) { stall_wb_full(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "icache_table_id")) { icache_table_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "max_inst_count")) { max_inst_count(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "te_clean_wb_enable")) { te_clean_wb_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_mpu_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable_stop")) { enable_stop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_error_stop")) { enable_error_stop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "enable_pipe_freeze_on_stop")) { enable_pipe_freeze_on_stop(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_sdp_ctl_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable_compress")) { enable_compress(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mask_frame_size")) { mask_frame_size(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_axi_attr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
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
bool cap_mpu_csr_icache_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "invalidate")) { invalidate(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_trace_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "enable")) { enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "trace_enable")) { trace_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_debug")) { phv_debug(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phv_error")) { phv_error(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch_enable")) { watch_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "watch_pc")) { watch_pc(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "table_and_key")) { table_and_key(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "instructions")) { instructions(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "wrap")) { wrap(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rst")) { rst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "base_addr")) { base_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "buf_size")) { buf_size(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_index")) { debug_index(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_generation")) { debug_generation(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_mpu_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = icache.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi_attr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sdp_ctl.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mpu_ctl.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mpu_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = count_stage.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = CNT_sdp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_live_sdp.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = STA_stg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = mismatch.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_debug_port.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_sdp_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_sdp_mem.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_data0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_data1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu0_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu0_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu1_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu1_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu2_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu2_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_mpu3_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_mpu3_icache_tag.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_spare.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_spare.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = spr_reg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_err.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_info.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_info_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("trace_full_0_enable");
    ret_vec.push_back("trace_full_1_enable");
    ret_vec.push_back("trace_full_2_enable");
    ret_vec.push_back("trace_full_3_enable");
    ret_vec.push_back("mpu_stop_0_enable");
    ret_vec.push_back("mpu_stop_1_enable");
    ret_vec.push_back("mpu_stop_2_enable");
    ret_vec.push_back("mpu_stop_3_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_info_int_test_set_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("trace_full_0_interrupt");
    ret_vec.push_back("trace_full_1_interrupt");
    ret_vec.push_back("trace_full_2_interrupt");
    ret_vec.push_back("trace_full_3_interrupt");
    ret_vec.push_back("mpu_stop_0_interrupt");
    ret_vec.push_back("mpu_stop_1_interrupt");
    ret_vec.push_back("mpu_stop_2_interrupt");
    ret_vec.push_back("mpu_stop_3_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_info_t::get_fields(int level) const { 
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
std::vector<string> cap_mpu_csr_int_err_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("results_mismatch_enable");
    ret_vec.push_back("sdp_mem_uncorrectable_enable");
    ret_vec.push_back("sdp_mem_correctable_enable");
    ret_vec.push_back("illegal_op_0_enable");
    ret_vec.push_back("illegal_op_1_enable");
    ret_vec.push_back("illegal_op_2_enable");
    ret_vec.push_back("illegal_op_3_enable");
    ret_vec.push_back("max_inst_0_enable");
    ret_vec.push_back("max_inst_1_enable");
    ret_vec.push_back("max_inst_2_enable");
    ret_vec.push_back("max_inst_3_enable");
    ret_vec.push_back("phvwr_0_enable");
    ret_vec.push_back("phvwr_1_enable");
    ret_vec.push_back("phvwr_2_enable");
    ret_vec.push_back("phvwr_3_enable");
    ret_vec.push_back("write_err_0_enable");
    ret_vec.push_back("write_err_1_enable");
    ret_vec.push_back("write_err_2_enable");
    ret_vec.push_back("write_err_3_enable");
    ret_vec.push_back("cache_axi_0_enable");
    ret_vec.push_back("cache_axi_1_enable");
    ret_vec.push_back("cache_axi_2_enable");
    ret_vec.push_back("cache_axi_3_enable");
    ret_vec.push_back("cache_parity_0_enable");
    ret_vec.push_back("cache_parity_1_enable");
    ret_vec.push_back("cache_parity_2_enable");
    ret_vec.push_back("cache_parity_3_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_err_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("results_mismatch_interrupt");
    ret_vec.push_back("sdp_mem_uncorrectable_interrupt");
    ret_vec.push_back("sdp_mem_correctable_interrupt");
    ret_vec.push_back("illegal_op_0_interrupt");
    ret_vec.push_back("illegal_op_1_interrupt");
    ret_vec.push_back("illegal_op_2_interrupt");
    ret_vec.push_back("illegal_op_3_interrupt");
    ret_vec.push_back("max_inst_0_interrupt");
    ret_vec.push_back("max_inst_1_interrupt");
    ret_vec.push_back("max_inst_2_interrupt");
    ret_vec.push_back("max_inst_3_interrupt");
    ret_vec.push_back("phvwr_0_interrupt");
    ret_vec.push_back("phvwr_1_interrupt");
    ret_vec.push_back("phvwr_2_interrupt");
    ret_vec.push_back("phvwr_3_interrupt");
    ret_vec.push_back("write_err_0_interrupt");
    ret_vec.push_back("write_err_1_interrupt");
    ret_vec.push_back("write_err_2_interrupt");
    ret_vec.push_back("write_err_3_interrupt");
    ret_vec.push_back("cache_axi_0_interrupt");
    ret_vec.push_back("cache_axi_1_interrupt");
    ret_vec.push_back("cache_axi_2_interrupt");
    ret_vec.push_back("cache_axi_3_interrupt");
    ret_vec.push_back("cache_parity_0_interrupt");
    ret_vec.push_back("cache_parity_1_interrupt");
    ret_vec.push_back("cache_parity_2_interrupt");
    ret_vec.push_back("cache_parity_3_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_err_t::get_fields(int level) const { 
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
std::vector<string> cap_mpu_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_err_interrupt");
    ret_vec.push_back("int_info_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("int_err_enable");
    ret_vec.push_back("int_info_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_intgrp_status_t::get_fields(int level) const { 
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
std::vector<string> cap_mpu_csr_spr_reg_data_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_spr_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : data.get_fields(level-1)) {
            ret_vec.push_back("data." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_icache_sram_read_data_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_icache_sram_read_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : data.get_fields(level-1)) {
            ret_vec.push_back("data." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_tag_read_data_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_tag_read_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : data.get_fields(level-1)) {
            ret_vec.push_back("data." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_spare_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_spare_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu3_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu3_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu2_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu2_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu1_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu1_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu0_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu0_icache_tag_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu3_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu3_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu3_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu3_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu2_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu2_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu2_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu2_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu1_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu1_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu1_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu1_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu0_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu0_icache_data1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu0_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    ret_vec.push_back("parity_error");
    ret_vec.push_back("parity_error_addr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_mpu0_icache_data0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_sdp_mem_t::get_fields(int level) const { 
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
std::vector<string> cap_mpu_csr_cfg_sdp_mem_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("bist_run");
    ret_vec.push_back("ecc_disable_det");
    ret_vec.push_back("ecc_disable_cor");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_cfg_debug_port_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("select");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_mismatch_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("update_result_count");
    ret_vec.push_back("sdp_result_count");
    ret_vec.push_back("pkt_id");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_STA_stg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sdp_srdy_in");
    ret_vec.push_back("sdp_drdy_in");
    ret_vec.push_back("srdy_out");
    ret_vec.push_back("drdy_out");
    ret_vec.push_back("srdy");
    ret_vec.push_back("drdy");
    ret_vec.push_back("te_valid");
    ret_vec.push_back("mpu_processing");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_STA_live_sdp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("update_empty");
    ret_vec.push_back("update_pkt_id");
    ret_vec.push_back("phv_pkt_id");
    ret_vec.push_back("phv_update_valid");
    ret_vec.push_back("update_result_count");
    ret_vec.push_back("staging_full");
    ret_vec.push_back("wptr");
    ret_vec.push_back("rptr");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_sdp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("phv_fifo_depth");
    ret_vec.push_back("sop_in");
    ret_vec.push_back("eop_in");
    ret_vec.push_back("sop_out");
    ret_vec.push_back("eop_out");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_error_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("icache0");
    ret_vec.push_back("icache1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_pend_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("wr_id");
    ret_vec.push_back("pending_table_write_valid0");
    ret_vec.push_back("pending_table_write_valid1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr7_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr6_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr5_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr4_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr3_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr2_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_gpr1_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("value");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("c1");
    ret_vec.push_back("c2");
    ret_vec.push_back("c3");
    ret_vec.push_back("c4");
    ret_vec.push_back("c5");
    ret_vec.push_back("c6");
    ret_vec.push_back("c7");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_ctl_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("stopped");
    ret_vec.push_back("ex_valid");
    ret_vec.push_back("ex_execute");
    ret_vec.push_back("ex_pc");
    ret_vec.push_back("icache_state");
    ret_vec.push_back("phv_cmd_fifo_depth");
    ret_vec.push_back("phv_data_fifo_depth");
    ret_vec.push_back("stall_vector");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_pc_mpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("inst");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_key_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("kd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_data_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("td");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_table_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mpu_processing_table_pcie");
    ret_vec.push_back("mpu_processing_table_sram");
    ret_vec.push_back("mpu_processing_table_id");
    ret_vec.push_back("mpu_processing_pkt_id");
    ret_vec.push_back("mpu_processing_table_latency");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sta_tbl_addr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("in_mpu");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_fence_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_tblwr_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_memwr_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_phvwr_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_hazard_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_icache_fill_stall_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_icache_miss_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_inst_executed_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_phv_executed_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_CNT_cycles_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_count_stage_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("always_on");
    ret_vec.push_back("debug");
    ret_vec.push_back("watch");
    ret_vec.push_back("stop_on_saturate");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_mpu_run_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("start_pulse");
    ret_vec.push_back("stop_pulse");
    ret_vec.push_back("step_pulse");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_mpu_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("stall_wb_full");
    ret_vec.push_back("icache_table_id");
    ret_vec.push_back("max_inst_count");
    ret_vec.push_back("te_clean_wb_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_mpu_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable_stop");
    ret_vec.push_back("enable_error_stop");
    ret_vec.push_back("enable_pipe_freeze_on_stop");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_sdp_ctl_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable_compress");
    ret_vec.push_back("mask_frame_size");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_axi_attr_t::get_fields(int level) const { 
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
std::vector<string> cap_mpu_csr_icache_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("invalidate");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_trace_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("enable");
    ret_vec.push_back("trace_enable");
    ret_vec.push_back("phv_debug");
    ret_vec.push_back("phv_error");
    ret_vec.push_back("watch_enable");
    ret_vec.push_back("watch_pc");
    ret_vec.push_back("table_and_key");
    ret_vec.push_back("instructions");
    ret_vec.push_back("wrap");
    ret_vec.push_back("rst");
    ret_vec.push_back("base_addr");
    ret_vec.push_back("buf_size");
    ret_vec.push_back("debug_index");
    ret_vec.push_back("debug_generation");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_mpu_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : icache.get_fields(level-1)) {
            ret_vec.push_back("icache." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : axi_attr.get_fields(level-1)) {
            ret_vec.push_back("axi_attr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sdp_ctl.get_fields(level-1)) {
            ret_vec.push_back("sdp_ctl." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mpu_ctl.get_fields(level-1)) {
            ret_vec.push_back("mpu_ctl." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mpu_cfg.get_fields(level-1)) {
            ret_vec.push_back("mpu_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : count_stage.get_fields(level-1)) {
            ret_vec.push_back("count_stage." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : CNT_sdp.get_fields(level-1)) {
            ret_vec.push_back("CNT_sdp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : STA_live_sdp.get_fields(level-1)) {
            ret_vec.push_back("STA_live_sdp." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : STA_stg.get_fields(level-1)) {
            ret_vec.push_back("STA_stg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : mismatch.get_fields(level-1)) {
            ret_vec.push_back("mismatch." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_debug_port.get_fields(level-1)) {
            ret_vec.push_back("cfg_debug_port." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_sdp_mem.get_fields(level-1)) {
            ret_vec.push_back("cfg_sdp_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_sdp_mem.get_fields(level-1)) {
            ret_vec.push_back("sta_sdp_mem." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu0_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu0_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu0_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu0_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu0_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu0_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu0_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu0_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu1_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu1_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu1_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu1_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu1_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu1_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu1_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu1_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu2_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu2_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu2_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu2_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu2_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu2_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu2_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu2_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu3_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu3_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu3_icache_data0.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu3_icache_data0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu3_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu3_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu3_icache_data1.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu3_icache_data1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu0_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu0_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu0_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu0_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu1_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu1_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu1_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu1_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu2_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu2_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu2_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu2_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_mpu3_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("cfg_mpu3_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_mpu3_icache_tag.get_fields(level-1)) {
            ret_vec.push_back("sta_mpu3_icache_tag." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_spare.get_fields(level-1)) {
            ret_vec.push_back("cfg_spare." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_spare.get_fields(level-1)) {
            ret_vec.push_back("sta_spare." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : spr_reg.get_fields(level-1)) {
            ret_vec.push_back("spr_reg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_err.get_fields(level-1)) {
            ret_vec.push_back("int_err." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_info.get_fields(level-1)) {
            ret_vec.push_back("int_info." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
