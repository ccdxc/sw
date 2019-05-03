#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s0_t0_k.h"

struct phv_ p;
struct s0_t0_k_ k;
struct s0_t0_nvme_sessprexts_tx_cb_process_d d;

%%

.align
nvme_sessprexts_tx_cb_process:
    nop.e
    nop             //Exit Slot
    
