#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"
#include "capri.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_sessprexts_tx_cb_process_d d;

%%

.align
nvme_sessprexts_tx_cb_process:
    tblwr   d.ring_empty_sched_eval_done, 0
    // for now make ci = pi
    tblwr   d.ci_0, d.pi_0
    phvwr.e p.p4_intr_global_drop, 1
    nop             //Exit Slot
    
