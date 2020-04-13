/*
 * 	Implements the reading of SESQ entry to queue the desc to TCP tx q
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t1_tcp_tx_k.h"

struct phv_ p;
struct s1_t1_tcp_tx_k_ k;
struct s1_t1_tcp_tx_read_sesq_ci_d d;


%%
    .align
    .param          tcp_tx_read_descr_start
    .param          tcp_tx_read_tcp_flags_start
    .param          TCP_PROXY_STATS
tcp_tx_sesq_read_ci_stage1_start:
    CAPRI_CLEAR_TABLE_VALID(1)

    sne             c1, k.common_phv_pending_asesq, r0

    add.c1          r3, d.{pad...descr_addr}.dx, NIC_DESC_ENTRY_0_OFFSET
    add.!c1         r3, r0, d.descr_addr

    blti            r3, ASIC_HBM_BASE, tcp_tx_read_sesq_ci_fatal_error

    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                    tcp_tx_read_descr_start, r3, TABLE_SIZE_512_BITS)

    b.c1            read_sesq_ci_end
    /*
     * Launch stage to read tcp flags
     */
    sub             r3, r3, (NIC_DESC_ENTRY_0_OFFSET - NIC_DESC_ENTRY_TCP_FLAGS_OFFSET)
    CAPRI_NEXT_TABLE_READ_e(1, TABLE_LOCK_DIS,
                    tcp_tx_read_tcp_flags_start, r3, TABLE_SIZE_8_BITS)

    nop
read_sesq_ci_end:
    nop.e
    nop

tcp_tx_read_sesq_ci_fatal_error:
    addui           r3, r0, hiword(TCP_PROXY_STATS)
    addi            r3, r3, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TCP_PROXY_STATS_INVALID_SESQ_DESCR, 1)
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
