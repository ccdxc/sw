/*
 * Stage 4, Table 0
 * Implementation of Virtio virtq_desc read for the head descriptor
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"


struct phv_                 p;
struct rx_table_s4_t0_k     k;
struct rx_table_s4_t0_d     d;

#define D(field)    d.{u.read_rx_head_desc_d.##field}
#define K(field)    k.{##field}



%%

    .param      virtio_rx_read_frag_desc_start

virtio_rx_read_head_desc_start:
    add         r1, r0, D(flags).hx
    crestore    [c2], K(to_s4_rx_virtq_used_addr[63]), 1

    /* Verify writable */
    smeqh       c1, r1, VIRTQ_DESC_F_WRITE, VIRTQ_DESC_F_WRITE
    bcf         [!c1], virtio_rx_read_head_desc_drop_pkt
    nop

    /* Verify that this is not the indirect scheme */
    smeqh       c1, r1, VIRTQ_DESC_F_INDIRECT, VIRTQ_DESC_F_INDIRECT
    bcf         [c1], virtio_rx_read_head_desc_drop_pkt
    nop

    /* If chained, evaluate total lenth of the chain */
    smeqh       c1, r1, VIRTQ_DESC_F_NEXT, VIRTQ_DESC_F_NEXT
    bcf         [!c1], virtio_rx_read_head_desc_no_chained_buf
    nop

    /* Verify length fits at least the virtio net header v1 */
    add         r1, r0, D(len).wx
    bnei        r1, VIRTIO_NET_HDR_SIZE_V1, \
                virtio_rx_read_head_desc_drop_pkt
    nop

    /* Setup DMA command for virtio_net_hdr while we have the address */
    /* Nothing to be setup in virtio_net_hdr */
    add         r2, r0, D(addr).dx
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vnet_hdr_v1_rxbuf_cmd_dma_cmd, r2,
                vnet_hdr_v1_rxbuf_flags, vnet_hdr_v1_rxbuf_num_buffers)
    phvwri.c2   p.vnet_hdr_v1_rxbuf_cmd_dma_cmd_host_addr, 1

    /* Setup read of the next descriptor */
    add         r2, r0, D(nextidx).hx

    /* Compute address to the virtq_desc ring offset */
    add         r1, r0, K(to_s4_rx_virtq_desc_addr)
    add         r1, r1, r2, VIRTIO_VIRTQ_DESC_RING_ELEM_SHIFT

	CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        virtio_rx_read_frag_desc_start,
	                    r1, TABLE_SIZE_128_BITS)

virtio_rx_read_head_desc_done:
	nop.e
    nop

virtio_rx_read_head_desc_no_chained_buf:
    /* Verify length */
    add         r1, r0, D(len).wx
    add         r2, K(virtio_s2s_t0_phv_pkt_len), VIRTIO_NET_HDR_SIZE_V1
    blt         r1, r2, \
                virtio_rx_read_head_desc_drop_pkt
    nop

    /* Start of DMA Commands */
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, \
                (CAPRI_PHV_START_OFFSET(vnet_hdr_v1_rxbuf_cmd_dma_cmd_type) / 16)

    /* Clear the virtio_net_hdr */
    phvwr       p.{vnet_hdr_v1_rxbuf_flags...vnet_hdr_v1_rxbuf_num_buffers}, r0

    /* DMA virtio_net_hdr to host memory */
    add         r2, r0, D(addr).dx
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vnet_hdr_v1_rxbuf_cmd_dma_cmd, r2,
                vnet_hdr_v1_rxbuf_flags, vnet_hdr_v1_rxbuf_num_buffers)
    phvwri.c2   p.vnet_hdr_v1_rxbuf_cmd_dma_cmd_host_addr, 1

    /* DMA packet to host memory */
    add         r2, r2, VIRTIO_NET_HDR_SIZE_V1
    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_cmd_dma_cmd, r2, K(virtio_s2s_t0_phv_pkt_len))
    phvwri.c2   p.pkt_cmd_dma_cmd_host_addr, 1

    /* DMA command to update virtq_used.ring element */
    phvwr       p.vq_used_elem_id, K(virtio_rx_global_phv_head_desc_idx).wx
    add         r1, K(virtio_s2s_t0_phv_pkt_len), VIRTIO_NET_HDR_SIZE_V1
    phvwr       p.vq_used_elem_len, r1.wx
    add         r3, r0, K(to_s4_rx_virtq_used_addr)
    and         r2, K(virtio_rx_global_phv_rx_virtq_used_pi), K(virtio_s2s_t0_phv_rx_queue_size_mask)
    sll         r2, r2, VIRTIO_VIRTQ_USED_RING_ELEM_SHIFT
    add         r2, r2, VIRTIO_VIRTQ_USED_RING_OFFSET
    add         r2, r2, r3
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_ring_elem_cmd_dma_cmd, r2, vq_used_elem_id, vq_used_elem_len)
    phvwri.c2   p.vq_used_ring_elem_cmd_dma_cmd_host_addr, 1

    /* DMA to update virtq_used.idx */
    add         r2, K(virtio_rx_global_phv_rx_virtq_used_pi), 1
    phvwr       p.vq_used_idx_idx, r2.hx
    add         r2, r3, VIRTIO_VIRTQ_USED_IDX_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_idx_cmd_dma_cmd, r2, vq_used_idx_idx, vq_used_idx_idx)
    phvwri.c2   p.vq_used_idx_cmd_dma_cmd_host_addr, 1

    /* Skip interrupt, end dma chain here? */
    seq         c1, K(virtio_s2s_t0_phv_no_interrupt), 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_wr_fence, 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_eop, 1
    bcf         [c1], virtio_rx_read_head_desc_no_interrupt
    nop

    /* DMA command to interrupt host */
    add         r1, K(virtio_s2s_t0_phv_rx_intr_assert_addr), r0
    CAPRI_DMA_CMD_PHV2MEM_SETUP(interrupt_cmd_dma_cmd, r1, \
        virtio_s2s_t0_phv_rx_intr_assert_data, virtio_s2s_t0_phv_rx_intr_assert_data)
    CAPRI_DMA_CMD_STOP_FENCE(interrupt_cmd_dma_cmd)

virtio_rx_read_head_desc_no_interrupt:

    /* End of program */
    CAPRI_CLEAR_TABLE0_VALID
	nop.e
    nop

virtio_rx_read_head_desc_drop_pkt:
    /* TODO: We need to write back the consumed descriptor to virtq_used ring with 0 len */
    phvwri      p.p4_intr_global_drop, 1
	nop.e
    nop
