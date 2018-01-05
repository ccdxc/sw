/*
 * Stage 1, Table 0
 * Implementation of Virtio virtq_avail read
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"


struct phv_                 p;
struct rx_table_s1_t0_k     k;
struct rx_table_s1_t0_d     d;

#define D(field)    d.{u.read_rx_virtq_avail_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_rx_check_reqs_start

virtio_rx_read_virtq_avail_start:

    /* FIXME: Assumes little-endian for now */
    phvwr       p.to_s2_rx_virtq_avail_idx, D(idx).hx

    /* virtq_avail.flags no_interrupt bit */
    add         r1, r0, D(flags)
    smeqh       c2, r1, VIRTQ_AVAIL_F_NO_INTERRUPT, VIRTQ_AVAIL_F_NO_INTERRUPT
    phvwri.c2   p.virtio_s2s_t0_phv_no_interrupt, 1

virtio_rx_read_virtq_avail_done:

    add         r1, r0, K(to_s1_qstate_addr_sbit0_ebit31...to_s1_qstate_addr_sbit32_ebit35)

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN,
                        virtio_rx_check_reqs_start,
	                    r1, TABLE_SIZE_512_BITS)

	nop.e
    nop
