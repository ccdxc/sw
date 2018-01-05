/*
 * Stage 4, Table 0
 * Implementation of Virtio virtq_desc read for the head descriptor
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"
#include "../../p4/nw/include/defines.h"


struct phv_                 p;
struct tx_table_s4_t0_k     k;
struct tx_table_s4_t0_d     d;

#define D(field)    d.{u.read_tx_head_desc_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_tx_read_frag_desc_start
#if 0
    .param      virtio_tx_read_virtio_net_hdr_start
#endif

virtio_tx_read_head_desc_start:
    add         r1, r0, D(flags).hx

    /* Verify read-only */
    smeqh       c1, r1, VIRTQ_DESC_F_WRITE, VIRTQ_DESC_F_WRITE
    bcf         [c1], virtio_tx_read_head_desc_err
    nop

    /* Verify that this is not the indirect scheme */
    smeqh       c1, r1, VIRTQ_DESC_F_INDIRECT, VIRTQ_DESC_F_INDIRECT
    bcf         [c1], virtio_tx_read_head_desc_err
    nop

    smeqh       c1, r1, VIRTQ_DESC_F_NEXT, VIRTQ_DESC_F_NEXT
    bcf         [!c1], virtio_tx_read_head_desc_contiguous_payload
    nop


    /* Verify length: Assumes no VIRTIO_NET_F_MRG_RXBUF support for now */
    add         r1, r0, D(len).wx
    blti        r1, VIRTIO_NET_HDR_SIZE_NO_MRG_RXBUF, \
                virtio_tx_read_head_desc_err
    nop

    /* Update Global K with the bytes read from the virtq_avail ring */
    phvwr       p.virtio_tx_global_phv_bytes_consumed, D(len).wx

    /* Setup read of the next descriptor */
    add         r2, r0, D(nextidx).hx

    /* Compute address to the virtq_desc ring offset */
    add         r1, r0, K(to_s4_tx_virtq_desc_addr)
    sll         r2, r2, VIRTIO_VIRTQ_DESC_RING_ELEM_SHIFT
    add         r1, r1, r2

    add         r2, r0, 0
    addui       r2, r2, 0x80000000
    add         r1, r1, r2

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_tx_read_frag_desc_start,
	                    r1, TABLE_SIZE_128_BITS)
#if 0
	CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS,
                        virtio_tx_read_virtio_net_hdr_start,
	                    D(addr), TABLE_SIZE_128_BITS)
#endif
virtio_tx_read_head_desc_done:
	nop.e
    nop

virtio_tx_read_head_desc_err:
    /* TODO: Error handling, put the descriptor back on the used ring */

	nop.e
    nop

virtio_tx_read_head_desc_contiguous_payload:
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, \
                (CAPRI_PHV_START_OFFSET(intrinsic_cmd_dma_cmd_type) / 16)

    /* Intrinsic */
    /* FIXME: This should be derived from the packet itself */
    phvwri      p.eth_tx_app_hdr_flags, P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG
    phvwri      p.eth_tx_app_hdr_vlan_tag, 0x01

    phvwri      p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri      p.p4_intr_global_tm_oport, TM_PORT_INGRESS

    phvwri      p.intrinsic_cmd_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri      p.intrinsic_cmd_dma_cmd_cmdsize, 2
    phvwri      p.intrinsic_cmd_dma_cmd_phv_start_addr, \
                CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwri      p.intrinsic_cmd_dma_cmd_phv_end_addr, \
                CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    phvwri      p.intrinsic_cmd_dma_cmd_phv_start_addr1, \
                CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwri      p.intrinsic_cmd_dma_cmd_phv_end_addr1, \
                CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)
    phvwri      p.intrinsic_cmd_dma_cmd_phv_start_addr2, \
                CAPRI_PHV_START_OFFSET(eth_tx_app_hdr_p4plus_app_id)
    phvwri      p.intrinsic_cmd_dma_cmd_phv_end_addr2, \
                CAPRI_PHV_END_OFFSET(eth_tx_app_hdr_vlan_tag)

    /* Packet */
    phvwri      p.packet_cmd_dma_cmd_host_addr, 1
    add         r1, D(addr).dx, VIRTIO_NET_HDR_SIZE_NO_MRG_RXBUF
    sub         r2, D(len).wx, VIRTIO_NET_HDR_SIZE_NO_MRG_RXBUF
    CAPRI_DMA_CMD_MEM2PKT_SETUP(packet_cmd_dma_cmd, r1, r2)
    phvwri      p.packet_cmd_dma_pkt_eop, 1


    /* Update the total bytes consumed for this request */
    add         r2, r0, D(len).wx

    /* DMA command to update virtq_used.ring element */
    phvwr       p.vq_used_elem_id, K(virtio_tx_global_phv_head_desc_idx).wx
    phvwr       p.vq_used_elem_len, r2.wx
    add         r3, r0, K(to_s4_tx_virtq_used_addr)
    and         r2, K(virtio_tx_global_phv_tx_virtq_used_pi), K(virtio_s2s_t0_phv_tx_queue_size_mask)
    sll         r2, r2, VIRTIO_VIRTQ_USED_RING_ELEM_SHIFT
    add         r2, r2, VIRTIO_VIRTQ_USED_RING_OFFSET
    add         r2, r2, r3
    phvwri      p.vq_used_ring_elem_cmd_dma_cmd_host_addr, 1
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_ring_elem_cmd_dma_cmd, r2, vq_used_elem_id, vq_used_elem_len)
    
    /* DMA command to update virtq_used.idx */
    add         r2, K(virtio_tx_global_phv_tx_virtq_used_pi), 1
    phvwr       p.vq_used_idx_idx, r2.hx
    add         r2, r3, VIRTIO_VIRTQ_USED_IDX_OFFSET
    phvwri      p.vq_used_idx_cmd_dma_cmd_host_addr, 1
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_idx_cmd_dma_cmd, r2, vq_used_idx_idx, vq_used_idx_idx)

    seq         c1, K(virtio_s2s_t0_phv_no_interrupt), 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_wr_fence, 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_eop, 1
    bcf         [c1], virtio_tx_read_head_desc_no_interrupt
    nop

    CAPRI_DMA_CMD_PHV2MEM_SETUP(interrupt_cmd_dma_cmd, K(virtio_s2s_t0_phv_tx_intr_assert_addr), \
        virtio_s2s_t0_phv_tx_intr_assert_data, virtio_s2s_t0_phv_tx_intr_assert_data)
    CAPRI_DMA_CMD_STOP_FENCE(interrupt_cmd_dma_cmd)

virtio_tx_read_head_desc_no_interrupt:

    /* End of program */
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
	nop.e
    nop
