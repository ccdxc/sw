#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_ring_full_drop_k k;
struct cpu_rx_ring_full_drop_d d;

        
%%
    .param ARQRX_BASE
    .align
cpu_rx_ring_full_drop_error:
    CAPRI_CLEAR_TABLE0_VALID
    tbladd d.u.cpu_rx_ring_full_drop_action_d.rx_ring_full_drop, 1
    phvwri  p.p4_intr_global_drop, 1
    nop.e
    nop
        
