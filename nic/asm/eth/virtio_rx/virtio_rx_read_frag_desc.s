/*
 * Stage 5, Table 0
 * Implementation of Virtio virtq_desc read for the first fragment descriptor
 */
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "virtio_defines.h"
#include "../../p4/nw/include/defines.h"


struct phv_                 p;
struct rx_table_s5_t0_k     k;
struct rx_table_s5_t0_d     d;

#define D(field)    d.{u.read_rx_payload_desc_d.##field}
#define K(field)    k.{##field}



%%

virtio_rx_read_frag_desc_start:

    /* Assumes that this should be the last fragment in the list */
    add         r1, r0, D(flags).hx
    smeqh       c1, r1, VIRTQ_DESC_F_NEXT, VIRTQ_DESC_F_NEXT
    bcf         [c1], virtio_rx_read_frag_desc_drop_pkt
    nop

    /* Verify writable */
    smeqh       c1, r1, VIRTQ_DESC_F_WRITE, VIRTQ_DESC_F_WRITE
    bcf         [!c1], virtio_rx_read_frag_desc_drop_pkt
    nop

    /* Verify that this is not the indirect scheme */
    smeqh       c1, r1, VIRTQ_DESC_F_INDIRECT, VIRTQ_DESC_F_INDIRECT
    bcf         [c1], virtio_rx_read_frag_desc_drop_pkt
    nop

    /* Verify if we can accomodate the packet in the buffer */
    add         r1, r0, D(len).wx
    add         r2, K(virtio_s2s_t0_phv_pkt_len), r0
    blt         r1, r2, \
                virtio_rx_read_frag_desc_drop_pkt
    nop

    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, \
                (CAPRI_PHV_START_OFFSET(vnet_hdr_no_mrg_rxbuf_cmd_dma_cmd_type) / 16)

    /* DMA virtio_net_hdr to host memory */
    /* DMA command for virtio_net_hdr already setup in previous stage in the 
     * first fragment
     */

    /* DMA packet to host memory */
    add         r2, r0, D(addr).dx
    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_cmd_dma_cmd, r2, K(virtio_s2s_t0_phv_pkt_len))
    phvwri      p.pkt_cmd_dma_cmd_host_addr, 1

    /* DMA command to update virtq_used.ring element */
    phvwr       p.vq_used_elem_id, K(virtio_rx_global_phv_head_desc_idx).wx
    add         r1, K(virtio_s2s_t0_phv_pkt_len), VIRTIO_NET_HDR_SIZE_NO_MRG_RXBUF
    phvwr       p.vq_used_elem_len, r1.wx
    add         r3, r0, K(to_s5_rx_virtq_used_addr)
    and         r2, K(virtio_rx_global_phv_rx_virtq_used_pi), K(virtio_s2s_t0_phv_rx_queue_size_mask)
    sll         r2, r2, VIRTIO_VIRTQ_USED_RING_ELEM_SHIFT
    add         r2, r2, VIRTIO_VIRTQ_USED_RING_OFFSET
    add         r2, r2, r3
    phvwri      p.vq_used_ring_elem_cmd_dma_cmd_host_addr, 1
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_ring_elem_cmd_dma_cmd, r2, vq_used_elem_id, vq_used_elem_len)

    /* DMA to update virtq_used.idx */
    add         r2, K(virtio_rx_global_phv_rx_virtq_used_pi), 1
    phvwr       p.vq_used_idx_idx, r2.hx
    add         r2, r3, VIRTIO_VIRTQ_USED_IDX_OFFSET
    phvwri      p.vq_used_idx_cmd_dma_cmd_host_addr, 1
    CAPRI_DMA_CMD_PHV2MEM_SETUP(vq_used_idx_cmd_dma_cmd, r2, vq_used_idx_idx, vq_used_idx_idx)

    seq         c1, K(virtio_s2s_t0_phv_no_interrupt), 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_wr_fence, 1
    phvwri.c1   p.vq_used_idx_cmd_dma_cmd_eop, 1
    bcf         [c1], virtio_rx_read_frag_desc_no_interrupt
    nop

    add         r1, K(virtio_s2s_t0_phv_rx_intr_assert_addr), r0
    CAPRI_DMA_CMD_PHV2MEM_SETUP(interrupt_cmd_dma_cmd, r1, \
        virtio_s2s_t0_phv_rx_intr_assert_data, virtio_s2s_t0_phv_rx_intr_assert_data)
    CAPRI_DMA_CMD_STOP_FENCE(interrupt_cmd_dma_cmd)

virtio_rx_read_frag_desc_no_interrupt:

    /* End of program */
    CAPRI_CLEAR_TABLE0_VALID
	nop.e
    nop

virtio_rx_read_frag_desc_drop_pkt:
    /* TODO: We need to write back the consumed descriptor to virtq_used ring with 0 len */
    phvwri      p.p4_intr_global_drop, 1
	nop.e
    nop
