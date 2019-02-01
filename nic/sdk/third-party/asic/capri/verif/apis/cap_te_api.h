#ifndef CAP_TE_API_H
#define CAP_TE_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "dpi_registry.h"
#include "cap_blk_reg_model.h"
#include "cap_te_csr.h"
#include "cap_te_hal_api.h"

#define TE_BYPASS 0
#define TE_MPU_ONLY 1
#define TE_HBM_ONLY 2
#define TE_SRAM_ONLY 3
#define TE_TCAM_ONLY 4
#define TE_TCAM_SRAM 5


void cap_te_set_soft_reset(int chip_id, int inst_id);
void cap_te_load_from_cfg(int chip_id, int inst_id);
void cap_te_eos(int chip_id, int inst_id);
void cap_te_eos_cnt(int chip_id, int inst_id);
void cap_te_eos_int(int chip_id, int inst_id);
void cap_te_eos_sta(int chip_id, int inst_id);
void cap_te_get_sta_entries(int chip_id, int inst_id);
void cap_te_check_progr_sanity(int chip_id, int inst_id, bool downgrade = false);
void cap_te_csr_set_hdl_path(int chip_id, int inst_id, string path);

void prog_te(int chip_id, int inst_id, int phv_offset, uint64_t pc); 

#endif // CAP_TE_API_H
