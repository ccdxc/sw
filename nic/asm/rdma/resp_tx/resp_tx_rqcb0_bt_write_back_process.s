#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s3_t1_k k;
struct rqcb0_t d;

#define WB_K_P              t1_s2s_rqcb0_bt_write_back_info
#define K_UPDATE_FLAGS      CAPRI_KEY_RANGE(WB_K_P, update_read_rsp_in_progress, update_bt_rsq_cindex)
#define K_READ_RSP_IN_PROGRESS  CAPRI_KEY_FIELD(WB_K_P, read_rsp_in_progress)
#define K_BT_IN_PROGRESS        CAPRI_KEY_FIELD(WB_K_P, bt_in_progress)
#define K_CURR_READ_RSP_PSN     CAPRI_KEY_FIELD(WB_K_P, curr_read_rsp_psn)
#define K_RSQ_CINDEX            CAPRI_KEY_FIELD(WB_K_P, rsq_cindex)
#define K_BT_CINDEX             CAPRI_KEY_FIELD(WB_K_P, bt_cindex)
#define K_BT_RSQ_CINDEX         CAPRI_KEY_FIELD(WB_K_P, bt_rsq_cindex)

%%

resp_tx_rqcb0_bt_write_back_process:

    // since no DMA commands are enqueued in backtrack logic,
    // we should drop the phv explicitly
    phvwr   p.common.p4_intr_global_drop, 1

    // c5 - update_read_rsp_in_progress
    // c4 - update_rsq_cindex
    // c3 - update_bt_cindex
    // c2 - update_bt_in_progress
    // c1 - update_bt_rsq_cindex

    crestore    [c5,c4,c3,c2,c1], K_UPDATE_FLAGS, 0x1F

    // update read_rsp_in_progress/curr_read_rsp_psn if reqd
    tblwr.c5    d.read_rsp_in_progress, K_READ_RSP_IN_PROGRESS
    tblwr.c5    d.curr_read_rsp_psn, K_CURR_READ_RSP_PSN

    // update rsq_cindex if reqd
    tblwr.c4    RSQ_C_INDEX, K_RSQ_CINDEX

    // update bt_cindex if reqd
    bcf         [!c3], skip_update_bt_cindex
    tblwr.c3    BT_C_INDEX, K_BT_CINDEX     //BD Slot
    
    // updating bt_cindex means we are terminating backtrack.
    // release rqcb1_bt_in_progress variable thru memwrite
    RQCB1_ADDR_GET(r7)
    add         r7, r7, FIELD_OFFSET(rqcb1_t, bt_in_progress)
    memwr.b     r7, r0 
    
skip_update_bt_cindex:
    // update bt_in_progress if reqd
    tblwr.c2    d.bt_in_progress, K_BT_IN_PROGRESS

    // update bt_rsq_cindex if reqd
    tblwr.c1    d.bt_rsq_cindex, K_BT_RSQ_CINDEX

    // release backtrack lock always as we are reaching write back state and exit
    tblwr.e     d.bt_lock, 0
    CAPRI_SET_TABLE_1_VALID(0)
