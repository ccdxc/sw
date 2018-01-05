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

    add         r1, r0, K(to_s2_tx_virtq_avail_idx)
    add         r2, r0, D(tx_virtq_avail_ci).hx
    beq         r1, r2, virtio_tx_check_reqs_done_no_reqs
    nop

    add         r5, r2, 1
    tblwr       D(tx_virtq_avail_ci), r5.hx
    beq         r1, r5, virtio_tx_check_reqs_no_pending_reqs
    nop

    /* We have more TX requests, ring doorbell on VIRTIO_TX_PEND_RING */
    add         r1, r0, K(to_s2_lif_sbit0_ebit7...to_s2_lif_sbit8_ebit10)
    /* address will be in r4 */
    /* TODO: Relay the qtype from stage 0 to be used here for doorbell */
    CAPRI_RING_DOORBELL_ADDR2(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_EVAL, 0, r1)
    add         r1, K(to_s2_qid), r0
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, r1, VIRTIO_TX_PEND_RING, r0)
    memwr.dx    r4, r3
    
virtio_tx_check_reqs_no_pending_reqs:

    phvwr       p.virtio_tx_global_phv_tx_virtq_used_pi, D(tx_virtq_used_pi).hx
    tbladd      D(tx_virtq_used_pi).hx, 1

virtio_tx_check_reqs_done:

    /* Compute address to the ring offset */
    add         r1, K(to_s2_tx_virtq_avail_addr), VIRTIO_VIRTQ_AVAIL_RING_OFFSET
    /* r2 -> D(tx_virtq_avail_ci).hx */
    and         r2, r2, D(tx_queue_size_mask).hx /* tx_virtq_avail_ci % queue_size */
    sll         r2, r2, VIRTIO_VIRTQ_AVAIL_RING_ELEM_SHIFT
    add         r1, r1, r2

    add         r2, r0, 0
    addui       r2, r2, 0x80000000
    add         r1, r1, r2

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_tx_read_head_desc_idx_start,
	                    r1, TABLE_SIZE_32_BITS)

virtio_tx_check_reqs_done_no_reqs:
	nop.e
    nop
