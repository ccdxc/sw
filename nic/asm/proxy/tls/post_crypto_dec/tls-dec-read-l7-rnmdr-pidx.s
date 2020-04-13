/*
 *  Stage 4 Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s4_t3_k k;
struct phv_ p;
struct tx_table_s4_t3_d d;

%%
    .param      tls_read_l7_descr_alloc
    .param      RNMDPR_BIG_TABLE_BASE
    .align
tls_dec_post_read_l7_rnmdr_pidx:
    CAPRI_SET_DEBUG_STAGE4_7(p.stats_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_3)

    // TODO : check for semaphore full
    add         r4, r0, d.{u.tls_read_l7_rnmdr_pidx_d.rnmdr_pidx}.wx
    andi        r4, r4, ((1 << ASIC_RNMDPR_SMALL_RING_SHIFT) - 1)

table_read_RNMDR_DESC:
    addui       r3, r0, hiword(RNMDPR_BIG_TABLE_BASE)
    addi        r3, r3, loword(RNMDPR_BIG_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(3,
                                r4, 
                                TABLE_LOCK_DIS,
                                tls_read_l7_descr_alloc,
                                r3, 
                                RNMDPR_SMALL_TABLE_ENTRY_SIZE_SHFT,
                                TABLE_SIZE_64_BITS)
    nop.e
    nop
