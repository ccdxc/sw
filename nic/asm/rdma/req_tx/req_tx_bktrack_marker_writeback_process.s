#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s6_t2_k k;
struct sqcb0_t d;

#define IN_TO_S_P   to_s6_sqcb_wb_add_hdr_info

%%

.align
req_tx_bktrack_marker_writeback_process:

    // If to-stage fence bit is set then fence condition is not met when bktrack was triggered.
    // So set dcqcn-rl-failure so that stage 0 will reset spec-cindex to cindex before starting bktrack.
    seq         c1, CAPRI_KEY_FIELD(IN_TO_S_P, fence), 1
    tblwr.c1    d.dcqcn_rl_failure, 1
    tblwr       d.bktrack_marker_in_progress, 0
    tblwr       d.fence, 0
    phvwr.e     p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)
