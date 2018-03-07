/*
 *    Get an index and auto increment it.
 *      This stage will be used to get
 *        - RNMDR alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t1_tcp_rx_k.h"

struct phv_ p;
struct s3_t1_tcp_rx_k_ k;
struct s3_t1_tcp_rx_read_rnmdr_d d;
    
%%
    .param          tcp_rx_rdesc_alloc_start
    .param          RNMDR_TABLE_BASE
    .align
tcp_rx_read_rnmdr_start:

    CAPRI_CLEAR_TABLE1_VALID

    seq             c1, d.rnmdr_pidx_full, 1
    b.c1            tcp_read_rnmdr_fatal_error

    add             r4, r0, d.{rnmdr_pidx}.wx
    andi            r4, r4, ((1 << CAPRI_RNMDR_RING_SHIFT) - 1)
    phvwr           p.s4_t1_s2s_rnmdr_pidx, r4

table_read_RNMDR_DESC:
    addui           r3, r0, hiword(RNMDR_TABLE_BASE)
    addi            r3, r3, loword(RNMDR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(1, r4, TABLE_LOCK_DIS,
                        tcp_rx_rdesc_alloc_start,
                        r3, RNMDR_TABLE_ENTRY_SIZE_SHFT,
                        TABLE_SIZE_64_BITS)
    nop.e
    nop

tcp_read_rnmdr_fatal_error:
    /*
     * Ring full is a fatal condition. We are out of memory
     * TODO : interrupt ARM to perform appropriate action
     */
    phvwr p.common_phv_fatal_error, 1
    phvwr p.common_phv_pending_txdma, 0
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    illegal
    nop.e
    nop
