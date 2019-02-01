#include "cap_stg_api.h"
#include "cap_te_api.h"
#include "cap_mpu_api.h"

void cap_stg_soft_reset(int chip_id, int inst_id) {

  cap_te_soft_reset(chip_id, inst_id);
  cap_mpu_soft_reset(chip_id, inst_id);
  PLOG_API_MSG("STG #" << inst_id, "inside softreset" << endl);
}

void cap_stg_set_soft_reset(int chip_id, int inst_id, int value) {
}

void cap_stg_init_start(int chip_id, int inst_id) {

  cap_te_init_start(chip_id, inst_id);
  cap_mpu_init_start(chip_id, inst_id);
  PLOG_API_MSG("STG #" <<inst_id, "After init_start" << endl);
}

void cap_stg_init_done(int chip_id, int inst_id) {
}
#ifndef CAPRI_HAL
void cap_stg_load_from_cfg(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
  cap_te_load_from_cfg(chip_id, inst_id);
  mpu_csr.axi_attr.set_access_secure(1);
  mpu_csr.load_from_cfg();
  PLOG_API_MSG("STG #" << inst_id, "After load_from_cfg" << endl);
}

void cap_stg_eos(int chip_id, int inst_id) {

   cap_stg_eos_cnt(chip_id, inst_id);
   cap_stg_eos_int(chip_id, inst_id);
   cap_stg_eos_sta(chip_id, inst_id);
}

void cap_stg_eos_cnt(int chip_id, int inst_id) {

  cap_te_eos_cnt(chip_id, inst_id);
  cap_mpu_eos_cnt(chip_id, inst_id);

  cap_te_csr_t & te_csr = CAP_BLK_REG_MODEL_ACCESS(cap_te_csr_t, chip_id, inst_id);
  cap_mpu_csr_t & mpu_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);

  mpu_csr.CNT_sdp.read();
  uint32_t cnt_phv_in_sop = mpu_csr.CNT_sdp.sop_in().convert_to<uint32_t>();
  uint32_t cnt_phv_in_eop = mpu_csr.CNT_sdp.eop_in().convert_to<uint32_t>();
  if (cnt_phv_in_sop != cnt_phv_in_eop) {
    PLOG_ERR("SDP[" << inst_id << hex << "] cnt_phv_in_sop 0x" << cnt_phv_in_sop << " != cnt_phv_in_eop 0x" << cnt_phv_in_eop << dec << endl);
  }
  else {
    PLOG_API_MSG("SDP[" << inst_id << "]", "Processed 0x" << hex << cnt_phv_in_sop << " input PHVs" << dec << endl);
  }

  uint32_t cnt_phv_out_sop = mpu_csr.CNT_sdp.sop_out().convert_to<uint32_t>();
  uint32_t cnt_phv_out_eop = mpu_csr.CNT_sdp.eop_out().convert_to<uint32_t>();
  if (cnt_phv_out_sop != cnt_phv_out_eop) {
    PLOG_ERR("SDP[" << inst_id << hex << "] cnt_phv_out_sop 0x" << cnt_phv_out_sop << " != cnt_phv_out_eop 0x" << cnt_phv_out_eop << dec << endl);
  }
  else {
    PLOG_API_MSG("SDP[" << inst_id << "]", "Processed 0x" << hex << cnt_phv_out_sop << " output PHVs" << dec << endl);
  }
  if (cnt_phv_in_sop != cnt_phv_out_sop) {
    PLOG_ERR("SDP[" << inst_id << "] cnt_phv_in_sop 0x" << hex << cnt_phv_in_sop << " != cnt_phv_out_sop 0x" << cnt_phv_out_sop << dec << endl);
  }

  // compare TE with SDP intf counters
  te_csr.cnt_phv_out_eop.read();
  uint32_t te_cnt_phv_out_eop = te_csr.cnt_phv_out_eop.val().convert_to<uint32_t>();
  // SDP counters are only 16 bits, TE are 32
  if ((te_cnt_phv_out_eop & 0xffff) != cnt_phv_in_sop) {
    PLOG_ERR("STG[" << inst_id << hex << "] TE cnt_phv_out_eop 0x" << te_cnt_phv_out_eop << " != SDP cnt_phv_in_sop 0x" << cnt_phv_in_sop << dec << endl);
  }
  else {
    PLOG_API_MSG("SDP[" << inst_id << "]", "TE & SDP processed the same number of PHVs" << endl);
  }
  
  // compare issued MPU programs but only if they don't stop on saturate
  mpu_csr.count_stage.read();
  for (auto mpu = 0; mpu < 4; mpu++) {
    te_csr.cnt_mpu_out[mpu].read();
    mpu_csr.CNT_phv_executed[mpu].read();

    uint32_t te_mpu_cnt = te_csr.cnt_mpu_out[mpu].val().convert_to<uint32_t>();
    uint32_t mpu_load_cnt = mpu_csr.CNT_phv_executed[mpu].counter().convert_to<uint32_t>();
    if ((te_mpu_cnt & 0xfffff) != mpu_load_cnt) {
      if (mpu_csr.count_stage.stop_on_saturate() == 0) {
        PLOG_ERR("STG[" << inst_id << hex << "] TE sent 0x" << te_mpu_cnt << " but MPU " << mpu << " received 0x" << mpu_load_cnt << " programs" << dec << endl);
      }
      else {
        PLOG_WARN("STG[" << inst_id << hex << "] TE sent 0x" << te_mpu_cnt << " but MPU " << mpu << " received 0x" << mpu_load_cnt << " programs" << dec << " but some other debug counter may have saturated " << endl);
      }
    }
    else {
      PLOG_API_MSG("STG[" << inst_id << "]", "MPU " << mpu << hex << " executed 0x" << mpu_load_cnt << " programs" << dec << endl);
    }
  }

  for (auto mpu = 0; mpu < 4; mpu++) {
    mpu_csr.CNT_fence_stall[mpu].read();
    mpu_csr.CNT_fence_stall[mpu].show();
    mpu_csr.CNT_tblwr_stall[mpu].read();
    mpu_csr.CNT_tblwr_stall[mpu].show();
    mpu_csr.CNT_memwr_stall[mpu].read();
    mpu_csr.CNT_memwr_stall[mpu].show();
    mpu_csr.CNT_phvwr_stall[mpu].read();
    mpu_csr.CNT_phvwr_stall[mpu].show();
    mpu_csr.CNT_hazard_stall[mpu].read();
    mpu_csr.CNT_hazard_stall[mpu].show();
    mpu_csr.CNT_icache_fill_stall[mpu].read();
    mpu_csr.CNT_icache_fill_stall[mpu].show();
    mpu_csr.CNT_phv_executed[mpu].read();
    mpu_csr.CNT_phv_executed[mpu].show();
    mpu_csr.CNT_cycles[mpu].read();
    mpu_csr.CNT_cycles[mpu].show();
  }

  PLOG_API_MSG("STG #" <<inst_id, "Done with eos cnt" << endl);
}

void cap_stg_eos_int(int chip_id, int inst_id) {

  cap_te_eos_int(chip_id, inst_id);
  cap_mpu_eos_int(chip_id, inst_id);
  PLOG_API_MSG("STG #" <<inst_id, "Done with eos int" << endl);
}


void cap_stg_eos_sta(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu_csr = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);

  cap_te_eos_sta(chip_id, inst_id);
  // can't call this because it looks at only one MPU
  // cap_mpu_eos_sta(chip_id, inst_id);

  for (auto mpu = 0; mpu < 4; mpu++) {
    mpu_csr.sta_mpu[mpu].read();
    mpu_csr.sta_mpu[mpu].show();
  }

  // mpu_csr.sta_mpu0_icache_data0.read();
  // mpu_csr.sta_mpu0_icache_data0.show();
  // mpu_csr.sta_mpu0_icache_data1.read();
  // mpu_csr.sta_mpu0_icache_data1.show();
  // mpu_csr.sta_mpu1_icache_data0.read();
  // mpu_csr.sta_mpu1_icache_data0.show();
  // mpu_csr.sta_mpu1_icache_data1.read();
  // mpu_csr.sta_mpu1_icache_data1.show();
  // mpu_csr.sta_mpu2_icache_data0.read();
  // mpu_csr.sta_mpu2_icache_data0.show();
  // mpu_csr.sta_mpu2_icache_data1.read();
  // mpu_csr.sta_mpu2_icache_data1.show();
  // mpu_csr.sta_mpu3_icache_data0.read();
  // mpu_csr.sta_mpu3_icache_data0.show();
  // mpu_csr.sta_mpu3_icache_data1.read();
  // mpu_csr.sta_mpu3_icache_data1.show();

  PLOG_API_MSG("STG #" << inst_id, "Done with eos sta" << endl);
}

void cap_stg_bist_start(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
  PLOG_API_MSG("STG #" << inst_id, "Starting SDP MEM BIST" << endl);
  //uint32_t cnt;

  mpu.cfg_sdp_mem.bist_run(1);
  mpu.cfg_sdp_mem.write();

  PLOG_API_MSG("STG #" << inst_id, "Starting MPU MEM BIST" << endl);

  mpu.cfg_mpu0_icache_data0.bist_run(1);
  mpu.cfg_mpu0_icache_data0.write();
  mpu.cfg_mpu0_icache_data1.bist_run(1);
  mpu.cfg_mpu0_icache_data1.write();
  mpu.cfg_mpu1_icache_data0.bist_run(1);
  mpu.cfg_mpu1_icache_data0.write();
  mpu.cfg_mpu1_icache_data1.bist_run(1);
  mpu.cfg_mpu1_icache_data1.write();
  mpu.cfg_mpu2_icache_data0.bist_run(1);
  mpu.cfg_mpu2_icache_data0.write();
  mpu.cfg_mpu2_icache_data1.bist_run(1);
  mpu.cfg_mpu2_icache_data1.write();
  mpu.cfg_mpu3_icache_data0.bist_run(1);
  mpu.cfg_mpu3_icache_data0.write();
  mpu.cfg_mpu3_icache_data1.bist_run(1);
  mpu.cfg_mpu3_icache_data1.write();

  mpu.cfg_mpu0_icache_tag.bist_run(1);
  mpu.cfg_mpu0_icache_tag.write();
  mpu.cfg_mpu1_icache_tag.bist_run(1);
  mpu.cfg_mpu1_icache_tag.write();
  mpu.cfg_mpu2_icache_tag.bist_run(1);
  mpu.cfg_mpu2_icache_tag.write();
  mpu.cfg_mpu3_icache_tag.bist_run(1);
  mpu.cfg_mpu3_icache_tag.write();
}

void cap_stg_bist_clear(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
  PLOG_API_MSG("STG #" << inst_id, "Starting SDP MEM BIST" << endl);
  //uint32_t cnt;

  mpu.cfg_sdp_mem.bist_run(0);
  mpu.cfg_sdp_mem.write();

  PLOG_API_MSG("STG #" << inst_id, "Starting MPU MEM BIST" << endl);

  mpu.cfg_mpu0_icache_data0.bist_run(0);
  mpu.cfg_mpu0_icache_data0.write();
  mpu.cfg_mpu0_icache_data1.bist_run(0);
  mpu.cfg_mpu0_icache_data1.write();
  mpu.cfg_mpu1_icache_data0.bist_run(0);
  mpu.cfg_mpu1_icache_data0.write();
  mpu.cfg_mpu1_icache_data1.bist_run(0);
  mpu.cfg_mpu1_icache_data1.write();
  mpu.cfg_mpu2_icache_data0.bist_run(0);
  mpu.cfg_mpu2_icache_data0.write();
  mpu.cfg_mpu2_icache_data1.bist_run(0);
  mpu.cfg_mpu2_icache_data1.write();
  mpu.cfg_mpu3_icache_data0.bist_run(0);
  mpu.cfg_mpu3_icache_data0.write();
  mpu.cfg_mpu3_icache_data1.bist_run(0);
  mpu.cfg_mpu3_icache_data1.write();

  mpu.cfg_mpu0_icache_tag.bist_run(0);
  mpu.cfg_mpu0_icache_tag.write();
  mpu.cfg_mpu1_icache_tag.bist_run(0);
  mpu.cfg_mpu1_icache_tag.write();
  mpu.cfg_mpu2_icache_tag.bist_run(0);
  mpu.cfg_mpu2_icache_tag.write();
  mpu.cfg_mpu3_icache_tag.bist_run(0);
  mpu.cfg_mpu3_icache_tag.write();
}

void cap_stg_bist_check(int chip_id, int inst_id) {

  cap_mpu_csr_t & mpu = CAP_BLK_REG_MODEL_ACCESS(cap_mpu_csr_t, chip_id, inst_id);
  PLOG_API_MSG("STG #" << inst_id, "Checking SDP MEM BIST" << endl);
  uint32_t cnt;

  for (cnt = 0; cnt < 1000; cnt++) {
    mpu.sta_sdp_mem.read();
    if (mpu.sta_sdp_mem.bist_done_fail() || mpu.sta_sdp_mem.bist_done_pass())
      break;
  }

  if (cnt == 1000) {
    PLOG_ERR("BIST did not return a status after " << cnt << " cycles for SDP [" << inst_id << "]" << endl);
  }
  if (mpu.sta_sdp_mem.bist_done_fail()) {
    PLOG_ERR("BIST failed for SDP [" << inst_id << "]" << endl);
  }
  if (mpu.sta_sdp_mem.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for SDP" << endl);
  }

  PLOG_API_MSG("STG #" << inst_id, "Checking MPU MEM BIST" << endl);
  uint16_t done = 0;
  for (cnt = 0; cnt < 1000; cnt++) {
    if ((done & (0x1 << 0)) == 0) {
      mpu.sta_mpu0_icache_data0.read();
      if (mpu.sta_mpu0_icache_data0.bist_done_fail() || mpu.sta_mpu0_icache_data0.bist_done_pass()) {
        done |= (0x1 << 0);
      }
    }
    if ((done & (0x1 << 1)) == 0) {
      mpu.sta_mpu0_icache_data1.read();
      if (mpu.sta_mpu0_icache_data1.bist_done_fail() || mpu.sta_mpu0_icache_data1.bist_done_pass()) {
        done |= (0x1 << 1);
      }
    }
    if ((done & (0x1 << 2)) == 0) {
      mpu.sta_mpu1_icache_data0.read();
      if (mpu.sta_mpu1_icache_data0.bist_done_fail() || mpu.sta_mpu1_icache_data0.bist_done_pass()) {
        done |= (0x1 << 2);
      }
    }
    if ((done & (0x1 << 3)) == 0) {
      mpu.sta_mpu1_icache_data1.read();
      if (mpu.sta_mpu1_icache_data1.bist_done_fail() || mpu.sta_mpu1_icache_data1.bist_done_pass()) {
        done |= (0x1 << 3);
      }
    }
    if ((done & (0x1 << 4)) == 0) {
      mpu.sta_mpu2_icache_data0.read();
      if (mpu.sta_mpu2_icache_data0.bist_done_fail() || mpu.sta_mpu2_icache_data0.bist_done_pass()) {
        done |= (0x1 << 4);
      }
    }
    if ((done & (0x1 << 5)) == 0) {
      mpu.sta_mpu2_icache_data1.read();
      if (mpu.sta_mpu2_icache_data1.bist_done_fail() || mpu.sta_mpu2_icache_data1.bist_done_pass()) {
        done |= (0x1 << 5);
      }
    }
    if ((done & (0x1 << 6)) == 0) {
      mpu.sta_mpu3_icache_data0.read();
      if (mpu.sta_mpu3_icache_data0.bist_done_fail() || mpu.sta_mpu3_icache_data0.bist_done_pass()) {
        done |= (0x1 << 6);
      }
    }
    if ((done & (0x1 << 7)) == 0) {
      mpu.sta_mpu3_icache_data1.read();
      if (mpu.sta_mpu3_icache_data1.bist_done_fail() || mpu.sta_mpu3_icache_data1.bist_done_pass()) {
        done |= (0x1 << 7);
      }
    }

    if ((done & (0x1 << 8)) == 0) {
      mpu.sta_mpu0_icache_tag.read();
      if (mpu.sta_mpu0_icache_tag.bist_done_fail() || mpu.sta_mpu0_icache_tag.bist_done_pass()) {
        done |= (0x1 << 8);
      }
    }
    if ((done & (0x1 << 9)) == 0) {
      mpu.sta_mpu1_icache_tag.read();
      if (mpu.sta_mpu1_icache_tag.bist_done_fail() || mpu.sta_mpu1_icache_tag.bist_done_pass()) {
        done |= (0x1 << 9);
      }
    }
    if ((done & (0x1 << 10)) == 0) {
      mpu.sta_mpu2_icache_tag.read();
      if (mpu.sta_mpu2_icache_tag.bist_done_fail() || mpu.sta_mpu2_icache_tag.bist_done_pass()) {
        done |= (0x1 << 10);
      }
    }
    if ((done & (0x1 << 11)) == 0) {
      mpu.sta_mpu3_icache_tag.read();
      if (mpu.sta_mpu3_icache_tag.bist_done_fail() || mpu.sta_mpu3_icache_tag.bist_done_pass()) {
        done |= (0x1 << 11);
      }
    }
    if (done == 0xfff) {
      break;
    }
  }

  if (cnt == 1000) {
    PLOG_ERR("MPU BIST did not return a status after " << cnt << " cycles for STG #" << inst_id << endl);
  }
  if (mpu.sta_mpu0_icache_data0.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 0 icache_data0" << endl);
  }
  if (mpu.sta_mpu0_icache_data0.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 0 icache_data0" << endl);
  }
  if (mpu.sta_mpu0_icache_data1.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 0 icache_data1" << endl);
  }
  if (mpu.sta_mpu0_icache_data1.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 0 icache_data1" << endl);
  }

  if (mpu.sta_mpu1_icache_data0.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 1 icache_data0" << endl);
  }
  if (mpu.sta_mpu1_icache_data0.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 1 icache_data0" << endl);
  }
  if (mpu.sta_mpu1_icache_data1.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 1 icache_data1" << endl);
  }
  if (mpu.sta_mpu1_icache_data1.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 1 icache_data1" << endl);
  }

  if (mpu.sta_mpu2_icache_data0.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 2 icache_data0" << endl);
  }
  if (mpu.sta_mpu2_icache_data0.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 2 icache_data0" << endl);
  }
  if (mpu.sta_mpu2_icache_data1.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 2 icache_data1" << endl);
  }
  if (mpu.sta_mpu2_icache_data1.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 2 icache_data1" << endl);
  }

  if (mpu.sta_mpu3_icache_data0.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 3 icache_data0" << endl);
  }
  if (mpu.sta_mpu3_icache_data0.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 3 icache_data0" << endl);
  }
  if (mpu.sta_mpu3_icache_data1.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 3 icache_data1" << endl);
  }
  if (mpu.sta_mpu3_icache_data1.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 3 icache_data1" << endl);
  }

  if (mpu.sta_mpu0_icache_tag.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 0 icache_tag" << endl);
  }
  if (mpu.sta_mpu0_icache_tag.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 0 icache_tag" << endl);
  }
  if (mpu.sta_mpu1_icache_tag.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 1 icache_tag" << endl);
  }
  if (mpu.sta_mpu1_icache_tag.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 1 icache_tag" << endl);
  }
  if (mpu.sta_mpu2_icache_tag.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 2 icache_tag" << endl);
  }
  if (mpu.sta_mpu2_icache_tag.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 2 icache_tag" << endl);
  }
  if (mpu.sta_mpu3_icache_tag.bist_done_fail()) {
    PLOG_ERR("BIST failed for STG #" << inst_id << " MPU 3 icache_tag" << endl);
  }
  if (mpu.sta_mpu3_icache_tag.bist_done_pass()) {
    PLOG_API_MSG("STG #" << inst_id, "BIST passed for STG #" << inst_id << " MPU 3 icache_tag" << endl);
  }
}
#endif
