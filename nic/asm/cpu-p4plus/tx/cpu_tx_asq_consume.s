#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_asq_consume_k k;

%%
    .align
cpu_tx_asq_consume_start:
    CAPRI_CLEAR_TABLE_VALID(1)
    
    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_CPU)
    
    /* data will be in r3 */
    add     r5, k.to_s2_asq_cidx, 1
    CAPRI_RING_DOORBELL_DATA(0, 0, CPU_SCHED_RING_ASQ, r5)
    memwr.dx        r4, r3
    nop.e
    nop

