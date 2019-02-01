#ifndef CAP_MPU_API_H
#define CAP_MPU_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_mpu_csr.h"



// soft reset sequence 
void cap_mpu_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_mpu_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_mpu_init_start(int chip_id, int inst_id);
// poll for init done
void cap_mpu_init_done(int chip_id, int inst_id);
void cap_mpu_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_mpu_eos(int chip_id, int inst_id);
void cap_mpu_eos_cnt(int chip_id, int inst_id);
void cap_mpu_eos_int(int chip_id, int inst_id);
void cap_mpu_eos_sta(int chip_id, int inst_id);
void cap_mpu_csr_set_hdl_path(int chip_id, int inst_id, string path);

#endif // CAP_MPU_API_H
