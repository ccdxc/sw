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
    .param          TCP_PROXY_STATS
tcp_rx_rdesc_alloc_start:
    CAPRI_CLEAR_TABLE1_VALID

    CAPRI_OPERAND_DEBUG(d.desc)
    add             r2, d.desc, ASIC_NMDPR_PAGE_OFFSET
    blti            r2, ASIC_HBM_BASE, tcp_rx_rdesc_alloc_fatal_error
    phvwr           p.to_s6_descr, d.desc
    phvwr.e         p.to_s6_page, r2
    nop

tcp_rx_rdesc_alloc_fatal_error:
    addui           r3, r0, hiword(TCP_PROXY_STATS)
    addi            r3, r3, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TCP_PROXY_STATS_INVALID_NMDR_DESCR, 1)
    phvwr p.common_phv_fatal_error, 1
    phvwr p.common_phv_pending_txdma, 0
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
