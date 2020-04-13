/*
 *    Get an index and auto increment it.
 *      This stage will be used to get
 *        - RNMDR alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t1_tcp_rx_k.h"

struct phv_ p;
struct s2_t1_tcp_rx_k_ k;
struct s2_t1_tcp_rx_read_rnmdr_d d;
    
%%
    .param          tcp_rx_rdesc_alloc_start
    .param          RNMDPR_BIG_TABLE_BASE
    .param          TCP_PROXY_STATS
    .align
tcp_rx_read_rnmdr_start:

    CAPRI_CLEAR_TABLE1_VALID

    seq             c1, d.rnmdr_pidx_full, 1
    b.c1            tcp_read_rnmdr_fatal_error

    add             r4, r0, d.{rnmdr_pidx}.wx
    andi            r4, r4, ((1 << ASIC_RNMDPR_BIG_RING_SHIFT) - 1)

table_read_RNMDR_DESC:
    addui           r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi            r3, r3, loword(RNMDPR_BIG_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(1, r4, TABLE_LOCK_DIS,
                        tcp_rx_rdesc_alloc_start,
                        r3, RNMDPR_BIG_TABLE_ENTRY_SIZE_SHFT,
                        TABLE_SIZE_64_BITS)
    nop.e
    nop

tcp_read_rnmdr_fatal_error:
    /*
     * Ring full is a fatal condition. We are out of memory
     * TODO : interrupt ARM to perform appropriate action
     */
    addui           r3, r0, hiword(TCP_PROXY_STATS)
    addi            r3, r3, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TCP_PROXY_STATS_RNMDR_FULL, 1)
    phvwr p.common_phv_fatal_error, 1
    phvwr p.common_phv_pending_txdma, 0
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
