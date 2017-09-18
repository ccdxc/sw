#include "ingress.h"
#include "INGRESS_p.h"
#include "capri-macros.h"
#include "cpu-table.h"
struct phv_ p;
struct cpu_rx_desc_alloc_k k;
struct cpu_rx_desc_alloc_d d;

%%
    .param cpu_rx_write_arq_start
    .align
cpu_rx_desc_alloc_start:
    CAPRI_CLEAR_TABLE1_VALID

    add         r3, r0, d.u.desc_alloc_d.desc 
    phvwr       p.to_s3_descr, d.u.desc_alloc_d.desc

desc_alloc_done:
    add r3, r0, k.common_phv_qstate_addr 
    CAPRI_NEXT_TABLE_READ_OFFSET(0, 
                                 TABLE_LOCK_EN,
                                 cpu_rx_write_arq_start,
                                 k.common_phv_qstate_addr,
                                 CPU_CB_WRITE_ARQRX_OFFSET,
                                 TABLE_SIZE_512_BITS)
    nop.e
    nop
