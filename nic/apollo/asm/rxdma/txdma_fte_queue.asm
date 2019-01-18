#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct txdma_fte_queue_k    k;
struct txdma_fte_queue_d    d;

%%

pkt_enqueue:
    // k.p4_to_rxdma_header_slacl_result,
    // == 00 : drop, used as predicate for applying this table
    // == 01 : txdma
    // == 10 : fte
    // == 11 : fte
    bbeq         k.p4_to_rxdma_header_slacl_result[1], 1, q_pkt_to_fte
    // common case, send to txdma
    // check q full
    add         r1, r0, d.pkt_enqueue_d.sw_pindex0
    mincr       r1, d.{pkt_enqueue_d.ring_size0}.hx, 1
    seq         c2, r1, d.{pkt_enqueue_d.sw_cindex0}
    bcf         [c2], txdma_q_full
    // compute entry offset for current p_index
    mul         r2, d.pkt_enqueue_d.sw_pindex0, PKTQ_PAGE_SIZE
    // update sw_pindex0, unlock the table
    tblwr.f     d.pkt_enqueue_d.sw_pindex0, r1

    add         r2, r2, d.{pkt_enqueue_d.ring_base0}.dx

    // dma pkt to pkt_buffer
    add         r1, k.{capri_p4_intr_packet_len_sbit0_ebit5, \
                    capri_p4_intr_packet_len_sbit6_ebit13}, \
                    (APOLLO_PREDICATE_HDR_SZ + APOLLO_P4_TO_TXDMA_HDR_SZ + \
                     APOLLO_I2E_HDR_SZ)
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_size, r1
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_addr, r2
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_eop, 0
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM

    // use Qid1 to ring door-bell. Qid0 is used as a completionQ between txdma and rxdma
    // this avoids contention on the same qstate0 addr from rxdma, txdma and DB hardware
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(dma_cmd_phv2mem_dma_cmd, \
        APOLLO_SERVICE_LIF, 0, 1, 0, \
        d.pkt_enqueue_d.sw_pindex0, doorbell_data_pid, doorbell_data_index)

    // setup dma_cmd pointer to 1st dma cmd
    phvwr.e     p.capri_rxdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(dma_cmd_pkt2mem_dma_cmd_pad) / 16
    nop

q_pkt_to_fte:
    // select fte ring based on the qid hash - TBD
    nop.e
    nop

txdma_q_full:
    phvwr.e         p.capri_intr_drop, 1
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
txdma_fte_queue_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
