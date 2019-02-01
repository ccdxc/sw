#ifndef CAP_TXS_API_H
#define CAP_TXS_API_H


#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_txs_csr.h"
#include "LogMsg.h"
#include "sknobs.h"
#ifndef CAPRI_HAL
#include "mem_access.h"
#endif

#define TXS_PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        txs_print_msg(l_ss.str()); \
}

void txs_print_msg(string msg);

// soft reset sequence 
void cap_txs_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_txs_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_txs_init_start(int chip_id, int inst_id);
// poll for init done
void cap_txs_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_txs_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_txs_eos(int chip_id, int inst_id);
void cap_txs_eos_cnt(int chip_id, int inst_id);
void cap_txs_eos_int(int chip_id, int inst_id);
void cap_txs_eos_sta(int chip_id, int inst_id);

void txs_cnt_show(int chip_id, int inst_id);
void tmr_cnt_show(int chip_id, int inst_id);


// backdoor
void cap_txs_csr_set_hdl_path(int chip_id, int inst_id, string path);

void cap_txs_sch_doorbell_wr(int chip_id, int inst_id, int lif, int qid, int cos, int set);
int  cap_txs_sch_doorbell_rd(int chip_id, int inst_id, int lif, int qid, int cos);

// sch enable
void cap_txs_sch_enable_wr(int chip_id, int inst_id, int enable);

// tmr
void txs_tmr_start_no_stop(int chip_id, int inst_id, int ftmr, int dtime, int typ, int ring, int qid, int entry);
void txs_tmr_force_ctime(int chip_id, int inst_id, int ftmr, int enable, int ctime);

void cap_txs_bist_test(int chip_id, int inst_id);
void cap_txs_bist_start(int chip_id, int inst_id);
void cap_txs_bist_check(int chip_id, int inst_id);

#ifndef CAPRI_HAL
void cap_txs_dump_tmr_pending(int chip_id, int inst_id, int tmr_fast, int print_only_nonzero_bcnt = 0);

void cap_tmr_status(int chip_id, int inst_id);
void cap_tmr_cfg(int chip_id, int inst_id);

void cap_txs_sch_status(int chip_id, int inst_id);

void cap_txs_sg_active_status(int chip_id, int inst_id, int active_txdma_only=1);

// display cntr
void cap_txs_dump_tx_cos(int chip_id, int inst_id);
void cap_txs_clr_tx_cos(int chip_id, int inst_id);

void cap_txs_dump_cntr(int chip_id, int inst_id);
void cap_txs_clr_cntr(int chip_id, int inst_id);

void cap_tmr_dump_cntr(int chip_id, int inst_id);
void cap_tmr_clr_cntr(int chip_id, int inst_id);

void cap_tmr_bcnt_lcnt_cntr(int chip_id, int inst_id);

void cap_txs_dump_sch_grp_cntr(int chip_id, int inst_id, int print_nonzero_drb = 1);
//void cap_txs_clr_sch_grp_pop_cntr(int chip_id, int inst_id);

void cap_txs_mon_xoff(int chip_id, int inst_id, int sample = 1000);
void cap_txs_mon_txdma_drdy(int chip_id, int inst_id, int sample = 1000);

///void cap_txs_doorbell_set(int set, int lif, int qid, int cos);
///void cap_txs_doorbell_cosidx_set(int set, int lif, int qid, int cos_idx);
int  cap_txs_drb_get(int chip_id, int inst_id, int lif, int cos, int qid);
void cap_txs_drb_set(int chip_id, int inst_id, int lif, int cos, int qid, int set);
int  cap_txs_get_cos(int chip_id, int inst_id, int lif, int cos_idx);
int  cap_txs_get_cos_idx(int chip_id, int inst_id, int lif, int cos);

void cap_sch_cfg(int chip_id, int inst_id);

void cap_txs_cfg_dtdm(int chip_id, int inst_id);

void cap_txs_get_all_active_q(int chip_id, int inst_id, int sample = 1);
void cap_txs_get_active_q(int chip_id, int inst_id, int sch_grp, int sample = 1);

void cap_lif_sch_cfg_chk(int chip_id, int inst_id, int sg);

#endif // CAPRI_HAL

#endif

