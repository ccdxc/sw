/*
 *    Get an index and auto increment it.
 *      This stage will be used to get
 *        - OOQ alloc idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t2_tcp_rx_k.h"

struct phv_ p;
struct s3_t2_tcp_rx_k_ k;
struct s3_t2_tcp_rx_ooo_qbase_sem_idx_d d;
    
%%
    .param          tcp_rx_ooq_alloc_start
    .param          TCP_OOQ_TABLE_BASE
    .param          TCP_PROXY_STATS
    .align
tcp_ooq_alloc_idx_start:

    seq             c1, d.ooo_qbase_pindex_full, 1
    b.c1            tcp_ooq_alloc_fatal_error

    add             r4, r0, d.{ooo_qbase_pindex}.wx
    andi            r4, r4, ASIC_TCP_ALLOC_OOQ_RING_MASK

table_read_OOQ_DESC:
    addui           r3, r0, hiword(TCP_OOQ_TABLE_BASE)
    addi            r3, r3, loword(TCP_OOQ_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ_INDEX(2, r4, TABLE_LOCK_DIS,
                        tcp_rx_ooq_alloc_start,
                        r3, TCP_OOQ_TABLE_ENTRY_SIZE_SHFT,
                        TABLE_SIZE_64_BITS)
    nop.e
    nop

tcp_ooq_alloc_fatal_error:
    /*
     * TODO : We have to somehow rollback work done in bookkeeping
     * stage that assumed allocation will succeed
     */

    /*
     * Out of OOQ buffers
     */
    addui           r3, r0, hiword(TCP_PROXY_STATS)
    addi            r3, r3, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TCP_PROXY_STATS_OOQ_FULL, 1)
    phvwr p.common_phv_ooo_alloc_fail, 1
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop
