#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct rx_table_s2_t0_k k;
struct rx_table_s2_t0_cpu_rx_semaphore_full_drop_action_d d;
        
%%
    .param ARQRX_BASE
    .align
cpu_rx_semaphore_full_drop_error:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    tbladd d.rx_sema_full_drop, 1
    phvwri.e  p.p4_intr_global_drop, 1
    nop
        
