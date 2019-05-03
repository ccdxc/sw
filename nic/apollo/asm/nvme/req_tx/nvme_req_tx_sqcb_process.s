#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_req_tx_sqcb_process_d d;

%%

.align
nvme_req_tx_sqcb_process:
    nop.e
    nop             //Exit Slot
    
