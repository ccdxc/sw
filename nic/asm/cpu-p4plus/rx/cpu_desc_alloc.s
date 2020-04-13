#include "ingress.h"
#include "INGRESS_p.h"
#include "capri-macros.h"
#include "cpu-table.h"
struct phv_ p;
struct cpu_rx_desc_alloc_k k;
struct cpu_rx_desc_alloc_d d;

%%
    .align
cpu_rx_desc_alloc_start:
    CAPRI_CLEAR_TABLE1_VALID

    add         r3, r0, d.u.desc_alloc_d.desc 
    phvwr       p.t0_s2s_descr, d.u.desc_alloc_d.desc

    add         r2, d.u.desc_alloc_d.desc, ASIC_CPU_RX_DPR_PAGE_OFFSET
    phvwr       p.t0_s2s_page, r2
    nop.e
    nop
