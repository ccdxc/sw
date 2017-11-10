/*
 *    Get an index and auto increment it.
 *      This stage will be used to get
 *        - RNMPR alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s2_t2_read_rnmpr_k k;
struct s2_t2_read_rnmpr_read_rnmpr_d d;
    
%%
    .param          tcp_rx_rpage_alloc_stage3_start
    .param          RNMPR_TABLE_BASE
    .align
tcp_rx_read_rnmpr_stage2_start:
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_2, CAPRI_MPU_TABLE_2)

    // TODO : check for semaphore full
    add             r4, r0, d.{rnmpr_pidx}.wx
    andi            r4, r4, ((1 << CAPRI_RNMPR_RING_SHIFT) - 1)
    phvwr           p.s3_t1_s2s_rnmdr_pidx, r4

table_read_RNMPR_PAGE:
    addui           r3, r0, hiword(RNMPR_TABLE_BASE)
    addi            r3, r0, loword(RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(2, r4, TABLE_LOCK_DIS,
                    tcp_rx_rpage_alloc_stage3_start, r3,
                    RNMPR_TABLE_ENTRY_SIZE_SHFT, TABLE_SIZE_64_BITS)
    nop.e
    nop

