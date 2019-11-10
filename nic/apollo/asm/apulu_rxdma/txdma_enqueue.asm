#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_txdma_enqueue_k.h"

struct txdma_enqueue_k_ k;
struct txdma_enqueue_d  d;
struct phv_             p;

%%

pkt_enqueue:

    // Clear the intrinsic recirc count to prevent TTL drop
    phvwr        p.capri_p4_intr_recirc_count, r0

    // Increment the local_recirc_count.
    add          r1, k.lpm_metadata_recirc_count, 1
    phvwr        p.lpm_metadata_recirc_count, r1

    // Are we done with processing SACLs...?!
    seq          c1, k.lpm_metadata_sacl_base_addr, r0
    // If not, stop
    nop.!c1.e

    // Yes. Copy the data that need to go to txdma
    phvwr        p.rx_to_tx_hdr_rx_packet, k.p4_to_rxdma_rx_packet
    phvwr        p.rx_to_tx_hdr_payload_len, k.capri_p4_intr_packet_len
    phvwr        p.rx_to_tx_hdr_vpc_id, k.p4_to_rxdma_vpc_id
    phvwr        p.rx_to_tx_hdr_vnic_id, k.p4_to_rxdma_vnic_info_key[10:1]
    phvwr        p.rx_to_tx_hdr_iptype, k.p4_to_rxdma_iptype

    // check q full
    add         r1, r0, d.pkt_enqueue_d.sw_pindex0
    mincr       r1, d.{pkt_enqueue_d.ring_size}.hx, 1
    seq         c2, r1, d.{pkt_enqueue_d.sw_cindex0}
    bcf         [c2], txdma_q_full
    // compute entry offset for current p_index
    mul         r2, d.pkt_enqueue_d.sw_pindex0, PKTQ_PAGE_SIZE
    add         r3, d.{pkt_enqueue_d.ring1_base}.dx, \
                    d.pkt_enqueue_d.sw_pindex0, APULU_PKT_DESC_SHIFT
    // update sw_pindex0, unlock the table
    tblwr.f     d.pkt_enqueue_d.sw_pindex0, r1

    // setup dma_cmd pointer to 1st dma cmd
    phvwr       p.capri_rxdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(pktdesc_phv2mem_dma_cmd_pad) / 16
    // dma pkt desc
    CAPRI_DMA_CMD_PHV2MEM_SETUP(pktdesc_phv2mem_dma_cmd, r3, \
                                rx_to_tx_hdr_remote_ip, \
                                rx_to_tx_hdr_pad8)
    phvwr       p.pktdesc_phv2mem_dma_cmd_round, 1

    // use Qid1 to ring door-bell. Qid0 is used as a completionQ between txdma
    // and rxdma this avoids contention on the same qstate0 addr from rxdma,
    // txdma and DB hardware
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(doorbell_phv2mem_dma_cmd, \
        APULU_SERVICE_LIF, 0, 1, 0, \
        d.pkt_enqueue_d.sw_pindex0, doorbell_data_pid, doorbell_data_index)

    // dma pkt
    add         r1, r0, k.capri_p4_intr_packet_len
    add         r2, r2, d.{pkt_enqueue_d.ring0_base}.dx
    phvwr       p.pktbuf_pkt2mem_dma_cmd_size, r1
    phvwr.e     p.pktbuf_pkt2mem_dma_cmd_addr, r2
    // enable pkt round
    phvwr.f     p.{pktbuf_pkt2mem_dma_cmd_round...pktbuf_pkt2mem_dma_cmd_type}, \
                    (0x10 | CAPRI_DMA_COMMAND_PKT_TO_MEM)

txdma_q_full:
    phvwr.e     p.capri_intr_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
txdma_enqueue_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
