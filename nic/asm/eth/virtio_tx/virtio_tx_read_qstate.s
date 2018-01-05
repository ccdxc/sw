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

    .brbegin
        /* Prioritize processing of pending work ring */
        brpri       r7[1:0], [0,1]
        nop
            .brcase 0
                /* Doorbell from host */
                tblwr       D(ci_0), D(pi_0)
                /* Acknowledge the doorbell
                 * NOTE: virtq_avail.idx provides the correct count of requests to be 
                 * processed
                 */
                add         r1, r0, \
                            K(p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10)
                /* address will be in r4 */
                CAPRI_RING_DOORBELL_ADDR2(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, K(p4_txdma_intr_qtype), r1)
                add         r1, K(p4_txdma_intr_qid), r0
                add         r5, D(pi_0).hx, r0
                /* data will be in r3 */
                CAPRI_RING_DOORBELL_DATA(0, r1, VIRTIO_TX_HOST_RING, r5)
                memwr.dx    r4, r3 

                setcf       c1, [!c0]
                b           virtio_tx_read_qstate_ring_demux_end
                nop
            .brcase 1
                /* Pending work scheduled */
                tblwr       D(ci_1), D(pi_1)
                /* Acknowledge the doorbell
                 * NOTE: virtq_avail.idx provides the correct count of requests to be 
                 * processed
                 */
                add         r1, r0, \
                            K(p4_intr_global_lif_sbit0_ebit2...p4_intr_global_lif_sbit3_ebit10)
                /* address will be in r4 */
                CAPRI_RING_DOORBELL_ADDR2(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, K(p4_txdma_intr_qtype), r1)
                add         r1, K(p4_txdma_intr_qid), r0
                add         r5, D(pi_1).hx, r0
                /* data will be in r3 */
                CAPRI_RING_DOORBELL_DATA(0, r1, VIRTIO_TX_PEND_RING, r5)
                memwr.dx    r4, r3 

                setcf   c1, [c0]
                phvwri  p.virtio_s2s_t0_phv_pend_work_scheduled, 1
                b   virtio_tx_read_qstate_ring_demux_end
                nop
            .brcase 2
                CAPRI_CLEAR_TABLE0_VALID
                CAPRI_CLEAR_TABLE1_VALID
                CAPRI_CLEAR_TABLE2_VALID
                CAPRI_CLEAR_TABLE3_VALID
                nop.e
                nop
    .brend
    
virtio_tx_read_qstate_ring_demux_end:

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

    add         r1, r0, 0
    addui       r1, r1, 0x80000000
    add         r1, r1, D(tx_virtq_avail_addr).dx

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_tx_read_virtq_avail_start,
	                    r1, TABLE_SIZE_32_BITS)
	nop.e
    nop
