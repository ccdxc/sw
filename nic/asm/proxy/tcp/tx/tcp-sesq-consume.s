/*
 * 	Doorbell write to clear the sched bit for the SESQ having
 *      finished the consumption processing.
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s2_t1_tcp_tx_k k;

%%
    .align

tcp_tx_sesq_consume_stage2_start:
    CAPRI_CLEAR_TABLE_VALID(1)
    /* store address in r4 */
    addi            r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
    sne             c1, k.common_phv_pending_asesq, r0
    bcf             [c1], asesq_consume
    nop
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid, TCP_SCHED_RING_SESQ, k.to_s2_sesq_cidx)
    add             r3, r3, 1
    memwr.dx        r4, r3
    nop.e
    nop
asesq_consume:  
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid, TCP_SCHED_RING_ASESQ, k.to_s2_sesq_cidx)
    add             r3, r3, 1
    memwr.dx        r4, r3
    nop.e
    nop

