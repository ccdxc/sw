#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "common_defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_s1_t2_k k;

%%

.align
resp_rx_dcqcn_cnp_process:

    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field.
    // Ring doorbell to cut-rate and reset dcqcn params. Also drop PHV since further 
    // processing will be done in TxDMA rate-compute-ring.
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    phvwr       p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop
