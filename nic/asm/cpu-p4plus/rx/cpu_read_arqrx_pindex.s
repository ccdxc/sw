#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_arqrx_k k;

%%
    .param cpu_rx_write_arq_start
    .param ARQRX_QIDXR_BASE
    .align
cpu_rx_read_arqrx_pindex_start:
    CAPRI_CLEAR_TABLE0_VALID
    
#ifdef DO_NOT_USE_CPU_SEM    
    addui   r5, r0, hiword(ARQRX_QIDXR_BASE)   
    addi    r5, r5, loword(ARQRX_QIDXR_BASE)   
    CPU_ARQRX_QIDX_ADDR(0, r3, r5)
    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_EN,
                          cpu_rx_write_arq_start,
                          r3,
                          TABLE_SIZE_512_BITS)
#else
    CPU_ARQ_SEM_IDX_INC_ADDR(RX, k.t0_s2s_arqrx_id, r3)

    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_DIS,
                          cpu_rx_write_arq_start,
                          r3,
                          TABLE_SIZE_64_BITS)
#endif
    nop.e
    nop
