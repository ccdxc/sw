/*
 * Stage 0, Table 0
 * Implementation of Virtio LIF TX qstate read
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"


struct phv_                 p;
struct tx_table_s0_t0_k     k;
struct tx_table_s0_t0_d     d;

#define D(field)    d.{u.read_virtq_tx_qstate_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_tx_read_virtq_avail_start

virtio_tx_read_qstate_start:

    /* ack host doorbell */
    tblwr.f     D(ci_0), D(pi_0)

    phvwr       p.to_s2_qid, K(p4_txdma_intr_qid)
    phvwr       p.to_s2_lif, K(p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10)
    phvwr       p.to_s1_qstate_addr, \
                K(p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33)

    phvwr       p.virtio_s2s_t0_phv_tx_intr_assert_addr, D(tx_intr_assert_addr).wx
    phvwri      p.virtio_s2s_t0_phv_tx_intr_assert_data, 0x01000000

    phvwr       p.to_s2_tx_virtq_avail_addr, D(tx_virtq_avail_addr).dx
    phvwr       p.to_s3_tx_virtq_desc_addr, D(tx_virtq_desc_addr).dx
    phvwr       p.to_s4_tx_virtq_desc_addr, D(tx_virtq_desc_addr).dx

    phvwr       p.to_s4_tx_virtq_used_addr, D(tx_virtq_used_addr).dx
    phvwr       p.to_s5_tx_virtq_used_addr, D(tx_virtq_used_addr).dx
    phvwr       p.virtio_s2s_t0_phv_tx_queue_size_mask, D(tx_queue_size_mask).hx

virtio_tx_read_qstate_done:

    add         r1, r0, D(tx_virtq_avail_addr).dx

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_tx_read_virtq_avail_start,
	                    r1, TABLE_SIZE_32_BITS)
	nop.e
    nop
