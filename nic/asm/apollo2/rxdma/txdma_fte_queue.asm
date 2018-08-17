#include "apollo_rxdma.h"
#include "../../../p4/apollo2/include/defines.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "capri_common.h"

struct phv_                 p;
struct txdma_fte_queue_k    k;
struct txdma_fte_queue_d    d;

%%

pkt_enqueue:
    // k.p4_to_rxdma_header_sl_result,
    // == 00 : drop, used as predicate for applying this table
    // == 01 : txdma
    // == 10 : fte
    // == 11 : fte
    bbeq         k.p4_to_rxdma_header_sl_result[1], 1, q_pkt_to_fte
    // common case, send to txdma
    //seq         c2, d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.c_index0
    //bcf         [c2], txdma_q_full
    // compute entry offset for current p_index
    //RE_ADDR(r2, d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.ring_base0)
    mul         r2, d.pkt_enqueue_d.sw_pindex0, PKTQ_PAGE_SIZE

    // update sw_pindex0, unlock
    tblmincri.f d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.ring_sz_mask0, 1
    add         r2, r2, d.{pkt_enqueue_d.ring_base0}.dx

    // dma pkt to pkt buffer
    phvwr       p.capri_rxdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(dma_cmd_pkt2mem_dma_cmd_pad) / 16
    add         r1, k.{capri_p4_intr_packet_len_sbit0_ebit5, \
                    capri_p4_intr_packet_len_sbit6_ebit13}, \
                    (APOLLO_PREDICATE_HDR_SZ + APOLLO_P4_TO_TXDMA_HDR_SZ + \
                     APOLLO_I2E_HDR_SZ)
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_size, r1
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_addr, r2
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_eop, 0
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM

    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(dma_cmd_phv2mem_dma_cmd, \
        APOLLO_SERVICE_LIF, 0, k.capri_rxdma_intr_qid, 0, \
        d.pkt_enqueue_d.sw_pindex0, doorbell_data_pid, doorbell_data_index)
    phvwr.e     p.dma_cmd_phv2mem_dma_cmd_eop, 1
    nop

q_pkt_to_fte:
    // TBD
    nop.e
    nop

txdma_q_full:
    // drop - do nothing
    nop.e
    nop


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
txdma_fte_queue_error:
    nop.e
    nop
