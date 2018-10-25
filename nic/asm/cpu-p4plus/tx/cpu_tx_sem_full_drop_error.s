#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct tx_table_s5_t0_k k;
struct tx_table_s5_t0_cpu_tx_sem_full_drop_d d;

        
%%
    .param ARQRX_BASE
    .align
cpu_tx_sem_full_drop_error:
    CAPRI_CLEAR_TABLE0_VALID
    tbladd d.ascq_sem_full_drops, 1
    phvwri.e  p.p4_intr_global_drop, 1
    nop
        
