#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"
#include "capri.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_req_tx_sqcb_process_d d;

%%

.align
nvme_req_tx_sqcb_process:
    // set ci to pi
    tblwr       d.ci_0, d.pi_0
    DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF, CAPRI_TXDMA_INTRINSIC_QTYPE, CAPRI_TXDMA_INTRINSIC_QID, r2, r3)
    phvwr.e     p.p4_intr_global_drop, 1
    nop             //Exit Slot
    
