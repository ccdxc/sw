#include "cap_mpu_api.h"
#include "cap_csr_util_api.h"
#include "LogMsg.h"
#include "pknobs_reader.h"

// soft reset sequence 
void cap_mpu_soft_reset(int chip_id, int inst_id) {
}

// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_mpu_set_soft_reset(int chip_id, int inst_id, int value) {
}

// init start, no polling yet
void cap_mpu_init_start(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
  PLOG_API_MSG("MPU[" << inst_id << "]", "Enabling all interrupts" << endl);
  enable_all_interrupts(&mpu);
  PLOG_API_MSG("MPU[" << inst_id << "]", "Clearing icache_data STAs" << endl);
  mpu.int_err.int_enable_clear.results_mismatch_enable(1);
  mpu.int_err.int_enable_clear.write();
  mpu.sta_mpu0_icache_data0.all(0);
  mpu.sta_mpu0_icache_data0.write();
  mpu.sta_mpu0_icache_data1.all(0);
  mpu.sta_mpu0_icache_data1.write();

}

// poll for init done
void cap_mpu_init_done(int chip_id, int inst_id) {
}


// use sknobs base load cfg 
void cap_mpu_load_from_cfg(int chip_id, int inst_id) {
    cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
    mpu.trace[0].set_access_no_zero_time(1);
    PLOG_MSG("MPU load from config..." << endl)
    mpu.load_from_cfg();
}

// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_mpu_eos(int chip_id, int inst_id) {
    cap_mpu_eos_cnt(chip_id,inst_id);
    cap_mpu_eos_int(chip_id,inst_id);
    cap_mpu_eos_sta(chip_id,inst_id);
}

void cap_mpu_eos_cnt(int chip_id, int inst_id) {
}

void cap_mpu_eos_int(int chip_id, int inst_id) {
    cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
    PLOG_MSG("In EOS INT" << endl);
    mpu.int_err.intreg.read();
    sknobs_set_value((char *)"mpu_csr/results_mismatch", mpu.int_err.intreg.results_mismatch_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/illegal", mpu.int_err.intreg.illegal_op_0_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/max_inst", mpu.int_err.intreg.max_inst_0_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/phvwr_int", mpu.int_err.intreg.phvwr_0_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/write_err", mpu.int_err.intreg.write_err_0_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/cache_axi", mpu.int_err.intreg.cache_axi_0_interrupt().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/cache_parity", mpu.int_err.intreg.cache_parity_0_interrupt().convert_to<unsigned long long>());
    mpu.int_err.intreg.show();
    mpu.int_err.intreg.read();
    mpu.int_err.intreg.show();

    mpu.int_info.read();
    mpu.int_info.show();
    check_interrupts(&mpu, 0);
    mpu.int_err.read();
    mpu.int_err.show();
    mpu.int_info.read();
    mpu.int_info.show();

}

void cap_mpu_eos_sta(int chip_id, int inst_id) {
    cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
    PLOG_MSG("In EOS STA" << endl);
    mpu.CNT_fence_stall[0].read();
    mpu.CNT_fence_stall[0].show();
    mpu.CNT_tblwr_stall[0].read();
    mpu.CNT_tblwr_stall[0].show();
    mpu.CNT_memwr_stall[0].read();
    mpu.CNT_memwr_stall[0].show();
    mpu.CNT_phvwr_stall[0].read();
    mpu.CNT_phvwr_stall[0].show();
    mpu.CNT_hazard_stall[0].read();
    mpu.CNT_hazard_stall[0].show();
    mpu.CNT_icache_fill_stall[0].read();
    mpu.CNT_icache_fill_stall[0].show();
    mpu.CNT_phv_executed[0].read();
    mpu.CNT_phv_executed[0].show();
    mpu.CNT_cycles[0].read();
    mpu.CNT_cycles[0].show();
    mpu.sta_pend[0].read();
    mpu.sta_pend[0].show();
    sknobs_set_value((char *)"mpu_csr/pending_write_valid0", mpu.sta_pend[0].pending_table_write_valid0().convert_to<unsigned long long>());
    sknobs_set_value((char *)"mpu_csr/pending_write_valid1", mpu.sta_pend[0].pending_table_write_valid1().convert_to<unsigned long long>());
    mpu.sta_mpu0_icache_data0.read();
    mpu.sta_mpu0_icache_data0.show();
    mpu.sta_mpu0_icache_data1.read();
    mpu.sta_mpu0_icache_data1.show();

}

void cap_mpu_csr_set_hdl_path(int chip_id, int inst_id, string path) {
    cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
    mpu.set_csr_inst_path(0, (path + ".mpu_csr"), 0);
}
