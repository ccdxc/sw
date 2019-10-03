/*
 * Stage 2, Table 0
 * Implementation of Virtio virtq_avail read
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"


struct phv_                 p;
struct tx_table_s2_t0_k     k;
struct tx_table_s2_t0_d     d;

#define D(field)    d.{u.check_tx_reqs_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_tx_read_head_desc_idx_start

virtio_tx_check_reqs_start:

    /* update pi_1 with current avail_idx from host */
    tblwr       D(pi_1).hx, K(to_s2_tx_virtq_avail_idx)

    /* finished with all the host requests? */
    seq         c7, D(ci_1), D(pi_1)
    b.c7        virtio_tx_check_reqs_done_no_reqs
    nop

    /* claim a tx request for this phv */
    phvwr       p.virtio_tx_global_phv_tx_virtq_used_pi, D(ci_1).hx
    add         r2, r0, D(ci_1).hx
    tbladd      D(ci_1).hx, 1

    /* these are unused... only for printing in eth_dbgtool */
    tblwr       D(tx_virtq_avail_ci), D(pi_1)
    tblwr       D(tx_virtq_used_pi), D(ci_1)

virtio_tx_check_reqs_done:

    /* Compute address to the ring offset */
    add         r1, K(to_s2_tx_virtq_avail_addr), VIRTIO_VIRTQ_AVAIL_RING_OFFSET
    /* r2 -> D(tx_virtq_avail_ci).hx */
    and         r2, r2, D(tx_queue_size_mask).hx /* tx_virtq_avail_ci % queue_size */
    add         r1, r1, r2, VIRTIO_VIRTQ_AVAIL_RING_ELEM_SHIFT

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_tx_read_head_desc_idx_start,
	                    r1, TABLE_SIZE_32_BITS)

    /* was it the last host request? */
    seq         c7, D(ci_1), D(pi_1)
    b.c7        virtio_tx_check_reqs_done_last_req
    nop.!c7.e
    nop

virtio_tx_check_reqs_done_no_reqs:

    /* We have no TX request for this phv, drop it */
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    phvwr       p.p4_intr_global_drop, 1

virtio_tx_check_reqs_done_last_req:

    /* No more, or the last TX request: reeval the rings */
    /* TODO: Relay the qtype (1) from stage 0 to be used here for doorbell */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 1, K(to_s2_lif_sbit0_ebit7...to_s2_lif_sbit8_ebit10))
    CAPRI_RING_DOORBELL_DATA(0, K(to_s2_qid), 0, 0)
    memwr.dx    r4, r3

    nop.e
    nop
