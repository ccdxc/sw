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
    tbladd d.u.cpu_rx_ring_full_drop_action_d.rx_ring_full_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
        
