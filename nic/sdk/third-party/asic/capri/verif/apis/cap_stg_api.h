#ifndef CAP_STG_API_H
#define CAP_STG_API_H

#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_te_csr.h"
#include "cap_mpu_csr.h"

void cap_stg_soft_reset(int chip_id, int inst_id);
void cap_stg_set_soft_reset(int chip_id, int inst_id, int value);
void cap_stg_init_start(int chip_id, int inst_id);
void cap_stg_init_done(int chip_id, int inst_id);
void cap_stg_load_from_cfg(int chip_id, int inst_id);
void cap_stg_eos(int chip_id, int inst_id);
void cap_stg_eos_cnt(int chip_id, int inst_id);
void cap_stg_eos_int(int chip_id, int inst_id);
void cap_stg_eos_sta(int chip_id, int inst_id);
void cap_stg_bist_start(int chip_id, int inst_id);
void cap_stg_bist_check(int chip_id, int inst_id);
void cap_stg_bist_clear(int chip_id, int inst_id);

#endif
