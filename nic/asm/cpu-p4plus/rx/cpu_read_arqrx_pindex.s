#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;

%%
    .param cpu_rx_write_arq_start
    .param ARQRX_QIDXR_BASE
    .align
cpu_rx_read_arqrx_pindex_start:

CAPRI_CLEAR_TABLE0_VALID
    addui   r5, r0, hiword(ARQRX_QIDXR_BASE)   
    addi    r5, r5, loword(ARQRX_QIDXR_BASE)   
    CPU_ARQRX_QIDX_ADDR(0, r3, r5)
    CAPRI_NEXT_TABLE_READ(0, 
                          TABLE_LOCK_EN,
                          cpu_rx_write_arq_start,
                          r3,
                          TABLE_SIZE_512_BITS)
    nop.e
    nop
