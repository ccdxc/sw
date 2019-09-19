#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_pkt_dma_k.h"

struct pkt_dma_k_   k;
struct phv_         p;

%%

pkt_dma:
    // clear hardware error (hack)
    phvwr           p.capri_intr_hw_error, 0

    /* Is this the first pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 0
    /* Then enable Recirc */
    phvwr.c1.e      p.capri_p4_intr_recirc, TRUE
    phvwr           p.txdma_predicate_pass_two, TRUE

    /* Else disable Recirc */
    phvwr           p.txdma_predicate_pass_two, FALSE
    phvwr           p.txdma_predicate_flow_enable, TRUE

    // Setup Intrisic fields and DMA commands to generate packet to P4IG
    phvwr           p.capri_intr_tm_oport, TM_PORT_INGRESS

    // DMA Commands
    //   1) Intrisic P4 header, TXDMA Intr, Session/iFlow/rFlow Hints Info (phv2pkt)
    //   2) Packet Payload (mem2pkt)
    //   3) CI Update (consume the event) and DB Sched Eval
    //   4) Update the rxdma copy of cindex once every 64 pkts

    //  1) Intrisic P4 header, TXDMA Intr, Session/iFlow/rFlow Hints (phv2pkt)
    CAPRI_DMA_CMD_PHV2PKT_SETUP2(intrinsic_dma_dma_cmd,
                                 capri_intr_tm_iport, \
                                 capri_intr_tm_instance_type, \
                                 capri_txdma_intr_qid, \
                                 capri_txdma_intr_txdma_rsv)

    // 2) DMA command for payload
    // mem2pkt has an implicit fence. all subsequent dma is blocked
    phvwr           p.payload_dma_dma_cmd_addr, k.txdma_control_payload_addr
    phvwri          p.{payload_dma_dma_cmd_cache...payload_dma_dma_cmd_type}, \
                        (1 << 4) | (1 << 6) | CAPRI_DMA_COMMAND_MEM_TO_PKT
    add             r1, k.rx_to_tx_hdr_payload_len, 1
    phvwr           p.payload_dma_dma_cmd_size, r1

    // 3) Update the rxdma copy of cindex once every 64 pkts
    seq             c1, k.txdma_control_cindex[5:0], 0
    CAPRI_DMA_CMD_PHV2MEM_SETUP_COND(rxdma_ci_update_dma_cmd, \
                                     k.txdma_control_rxdma_cindex_addr, \
                                     txdma_control_cindex, \
                                     txdma_control_cindex, c1)

    // 4) DMA command setup for Doorbell Sched Eval
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, \
                             k.capri_intr_lif)
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
