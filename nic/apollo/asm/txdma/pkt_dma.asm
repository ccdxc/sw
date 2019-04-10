#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_pkt_dma_k.h"

struct phv_         p;
struct pkt_dma_k_   k;

%%

pkt_dma:
    bbeq        k.predicate_header_redirect_to_arm, FALSE, pkt_dma_to_egress
    nop
pkt_dma_to_ingress:
    CAPRI_DMA_CMD_PHV2PKT_SETUP3(intrinsic_dma_dma_cmd,
                                 capri_intr_tm_iport, \
                                 capri_intr_tm_instance_type, \
                                 capri_txdma_intr_qid, \
                                 capri_txdma_intr_txdma_rsv, \
                                 predicate_header_pad0, \
                                 predicate_header_direction)
    b           pkt_dma_common
    nop

pkt_dma_to_egress:
    CAPRI_DMA_CMD_PHV2PKT_SETUP3(intrinsic_dma_dma_cmd,
                                 capri_intr_tm_iport, \
                                 capri_txdma_intr_txdma_rsv, \
                                 predicate_header_pad0, \
                                 predicate_header_direction, \
                                 txdma_to_p4e_header_p4plus_app_id, \
                                 txdma_to_p4e_header_vcn_id)

pkt_dma_common:
    // mem2pkt has an implicit fence. all subsequent dma is blocked
    phvwr       p.payload_dma_dma_cmd_addr, k.txdma_control_payload_addr
    phvwri      p.{payload_dma_dma_cmd_cache...payload_dma_dma_cmd_type}, \
                    (1 << 4) | (1 << 6) | CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr       p.payload_dma_dma_cmd_size, k.p4_to_txdma_header_payload_len

    // update the rxdma copy of cindex once every 64 pkts
    seq         c1, k.txdma_control_cindex[5:0], 0
    // 2 phvwr
    CAPRI_DMA_CMD_PHV2MEM_SETUP_COND(rxdma_ci_update_dma_cmd, \
                                     k.txdma_control_rxdma_cindex_addr, \
                                     txdma_control_cindex, \
                                     txdma_control_cindex, c1)

    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, \
                             k.capri_intr_lif)
    // 2 phvwr
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_END(doorbell_ci_update_dma_cmd, \
                                         r4, \
                                         doorbell_data_pid, \
                                         doorbell_data_index)

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
pkt_dma_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
