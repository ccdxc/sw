/*
 *    Implements the descr allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t1_tcp_rx_k.h"

struct phv_ p;
struct s3_t1_tcp_rx_k_ k;
struct s3_t1_tcp_rx_rdesc_alloc_d d;

%%
    .align
tcp_rx_rdesc_alloc_start:
    CAPRI_CLEAR_TABLE1_VALID

    CAPRI_OPERAND_DEBUG(d.desc)
    sne             c1, k.common_phv_ooo_rcv, r0
    add             r2, d.desc, CAPRI_NMDPR_PAGE_OFFSET
    blti            r2, CAPRI_HBM_BASE, tcp_rx_rdesc_alloc_fatal_error
    phvwr.c1        p.to_s5_descr, d.desc
    phvwr.c1        p.to_s5_page, r2
    phvwr           p.to_s6_descr, d.desc
    phvwr.e         p.to_s6_page, r2
    nop

tcp_rx_rdesc_alloc_fatal_error:
    phvwr p.common_phv_fatal_error, 1
    phvwr p.common_phv_pending_txdma, 0
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    illegal
    nop.e
    nop
