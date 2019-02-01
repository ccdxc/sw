#ifndef CAP_WA_API_H
#define CAP_WA_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "dpi_registry.h"
#include "cap_blk_reg_model.h"
#include "cap_wa_csr.h"
#include "cap_wa_csr_define.h"
#include "msg_man.h"
#include "mem_access.h"
#include "cap_qstate_decoders.h"

enum db_src_e {DB_HOST = 0, DB_LOCAL = 1, DB_32B = 2, DB_TIMER = 3, SRC_ARM4K = 4};

void cap_wa_soft_reset(int chip_id, int inst_id);
void cap_wa_set_soft_reset(int chip_id, int inst_id, int value);
void cap_wa_sw_reset(int chip_id, int inst_id);
void cap_wa_init_start(int chip_id, int inst_id);
void cap_wa_init_done(int chip_id, int inst_id);
void cap_wa_load_from_cfg(int chip_id, int inst_id);
//pint inval_master filters information
void cap_wa_print_inval_master_cfg_info(int chip_id, int inst_id);

void cap_wa_eos(int chip_id, int inst_id);
void cap_wa_eos_cnt(int chip_id, int inst_id);
void cap_wa_eos_int(int chip_id, int inst_id);
void cap_wa_eos_int_sim(int chip_id, int inst_id);
void cap_wa_eos_sta(int chip_id, int inst_id);

void wa_ring_doorbell(int upd, int lif, int type, int pid, int qid, int ring, int index, int db_type, uint32_t &addr, uint64_t &data);
void wa_ring_db(int upd, int lif, int type, int pid, int qid, int ring, int index, int db_type);

void wa_err_activity_log(int times=0);
void show_qstate(int lif, int type, int qid);

#endif // CAP_WA_API_H
