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

    seq c2, k.common_phv_dpr_sem_full_drop, 1
    b.c2 cpu_rx_read_arqrx_pindex_abort
    nop
		
    CAPRI_CLEAR_TABLE0_VALID
    
    CPU_ARQ_SEM_INF_ADDR(k.t0_s2s_arqrx_id, r3)

    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_DIS,
                          cpu_rx_write_arq_start,
                          r3,
                          TABLE_SIZE_64_BITS)
    nop.e
    nop

cpu_rx_read_arqrx_pindex_abort:
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
