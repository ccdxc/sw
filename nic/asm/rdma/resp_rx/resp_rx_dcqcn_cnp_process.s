#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "nic/p4/common/defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_s1_t2_k k;

#define TO_S_WB1_P to_s5_wb1_info

%%
    .param    resp_rx_rqcb1_write_back_mpu_only_process

.align
resp_rx_dcqcn_cnp_process:

    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field.
    // Ring doorbell to cut-rate and reset dcqcn params. Also drop PHV since further 
    // processing will be done in TxDMA rate-compute-ring.
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    phvwr       p.common.p4_intr_global_drop, 1
    phvwr       CAPRI_PHV_FIELD(TO_S_WB1_P, incr_nxt_to_go_token_id), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqcb1_write_back_mpu_only_process, r0)
