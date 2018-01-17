#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "common_defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_ecn_process_k_t k;

%%

.align
resp_rx_dcqcn_cnp_process:

    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field.
    // Ring doorbell to cut-rate and reset dcqcn params.
    DOORBELL_INC_PINDEX(k.global.lif,  k.global.qtype, k.global.qid, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop
