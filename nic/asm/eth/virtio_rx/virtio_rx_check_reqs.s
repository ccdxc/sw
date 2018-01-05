/*
 * Stage 2, Table 0
 * Implementation of Virtio virtq_avail read
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"


struct phv_                 p;
struct rx_table_s2_t0_k     k;
struct rx_table_s2_t0_d     d;

#define D(field)    d.{u.check_rx_reqs_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_rx_read_head_desc_idx_start

virtio_rx_check_reqs_start:

    add         r1, r0, K(to_s2_rx_virtq_avail_idx)
    add         r2, r0, D(rx_virtq_avail_ci).hx
    beq         r1, r2, virtio_rx_check_reqs_done_no_reqs
    nop

    add         r5, r2, 1
    tblwr       D(rx_virtq_avail_ci), r5.hx

    phvwr       p.virtio_rx_global_phv_rx_virtq_used_pi, D(rx_virtq_used_pi).hx
    tbladd      D(rx_virtq_used_pi).hx, 1

virtio_rx_check_reqs_done:

    /* Compute address to the ring offset */
    add         r1, K(to_s2_rx_virtq_avail_addr), VIRTIO_VIRTQ_AVAIL_RING_OFFSET
    /* r2 -> D(rx_virtq_avail_ci).hx */
    and         r2, r2, D(rx_queue_size_mask).hx /* rx_virtq_avail_ci % queue_size */
    sll         r2, r2, VIRTIO_VIRTQ_AVAIL_RING_ELEM_SHIFT
    add         r1, r1, r2

    add         r2, r0, 0
    addui       r2, r2, 0x80000000
    add         r1, r1, r2

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_rx_read_head_desc_idx_start,
	                    r1, TABLE_SIZE_32_BITS)

virtio_rx_check_reqs_done_no_reqs:
    phvwri.c1   p.p4_intr_global_drop, 1
	nop.e
    nop
