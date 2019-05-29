#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_nvme_sessprexts_tx_k.h"
#include "capri.h"

struct phv_ p;
struct s0_t0_nvme_sessprexts_tx_k_ k;
struct s0_t0_nvme_sessprexts_tx_cb_process_d d;

%%

.align
nvme_sessxts_tx_empty_ring_process:
    // we come here only when none of the rings have any work to do
    
    bbeq        d.ring_empty_sched_eval_done, 1, exit
    nop         //BD Slot

    // ring doorbell to re-evalue the scheduler
    DOORBELL_NO_UPDATE(CAPRI_TXDMA_INTRINSIC_LIF,
                       CAPRI_TXDMA_INTRINSIC_QTYPE,
                       CAPRI_TXDMA_INTRINSIC_QID,
                       r2, r3)
    tblwr       d.ring_empty_sched_eval_done, 1        

exit:
    phvwr.e     p.p4_intr_global_drop, 1
    nop         //Exit Slot
