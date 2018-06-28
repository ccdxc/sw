#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_arqrx_k k;

%%
    .param cpu_rx_write_arq_start
    .align
cpu_rx_read_arqrx_pindex_start:
    CAPRI_CLEAR_TABLE0_VALID
    
    CPU_ARQ_SEM_INF_ADDR(k.t0_s2s_arqrx_id, r3)

    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_DIS,
                          cpu_rx_write_arq_start,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop
