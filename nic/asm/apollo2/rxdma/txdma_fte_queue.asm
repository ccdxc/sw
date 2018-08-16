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
    seq         c2, d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.c_index0
    bcf         [c2], txdma_q_full
    // compute entry offset for current p_index
    //RE_ADDR(r2, d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.ring_base0)
    mul         r2, d.pkt_enqueue_d.sw_pindex0, PKTQ_PAGE_SIZE

    // update sw_pindex0, unlock
    tblmincri.f d.pkt_enqueue_d.sw_pindex0, d.pkt_enqueue_d.ring_sz_mask0, 1
    add         r2, r2, d.pkt_enqueue_d.ring_base0

    // dma pkt to pkt buffer
    phvwr       p.capri_rxdma_intr_dma_cmd_ptr, CAPRI_PHV_START_OFFSET(dma_cmd_pkt2mem_dma_cmd_type)
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_size, 0 // XXX use pkt size info from P4 pipeline
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_addr, r2 // XXX use pkt size info from P4 pipeline
    phvwr       p.dma_cmd_pkt2mem_dma_cmd_eop, 0
    phvwri      p.dma_cmd_pkt2mem_dma_cmd_type, 4 // DMA_CMD_TYPE_PKT2MEM


    // dma write doorbell w/ sw_pindex0
    // the macro below can be used but it wastes one instruction since it does not use .e on the last
    // CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(dma_cmd_phv2mem_dma_cmd, LIF_APOLLO_BIW, 0,
    //                                  k.capri_rxdma_intr_qid,
    //                                  0,
    //                                  r1, doorbell_data_pid, doorbell_data_index)
    phvwri      p.{dma_cmd_phv2mem_dma_cmd_phv_end_addr...dma_cmd_phv2mem_dma_cmd_type},    \
                     ((CAPRI_PHV_END_OFFSET(doorbell_data_pid) << 18) |                     \
                      (CAPRI_PHV_START_OFFSET(doorbell_data_index) << 8) |                  \
                      CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri      p.dma_cmd_phv2mem_dma_cmd_addr, \
                    CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, LIF_APOLLO_BIW);

    // Doorbell data (done by the macro when used)
    CAPRI_RING_DOORBELL_DATA(0, k.capri_rxdma_intr_qid, 0, r1)
    phvwr.e     p.{doorbell_data_pid...doorbell_data_index}, r3.dx;
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
