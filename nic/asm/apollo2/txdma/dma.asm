#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_             p;
struct pkt_dma_k        k;

%%

pkt_dma:
    phvwr       p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
    phvwr       p.txdma_to_p4e_header_vcn_id, k.p4_to_txdma_header_vcn_id

    CAPRI_DMA_CMD_PHV2PKT_SETUP(intrinsic_dma_dma_cmd, capri_intrinsic_tm_iport, capri_txdma_intrinsic_txdma_rsv)
    CAPRI_DMA_CMD_PHV2PKT_SETUP(header_dma_dma_cmd, predicate_header_txdma_drop_event, txdma_to_p4e_header_vcn_id)
    //CAPRI_DMA_CMD_MEM2PKT_SETUP_PKT(payload_dma_dma_cmd, k.txdma_control_payload_addr, k.p4_to_txdma_header_payload_len)
    phvwr       p.payload_dma_dma_cmd_addr, k.txdma_control_payload_addr
    .assert((offsetof(k, p4_to_txdma_header_payload_len_sbit0_ebit5) - \
            offsetof(k, p4_to_txdma_header_payload_len_sbit6_ebit13)) == 8);
    phvwr       p.payload_dma_dma_cmd_size, \
                    k.{p4_to_txdma_header_payload_len_sbit0_ebit5, \
                       p4_to_txdma_header_payload_len_sbit6_ebit13}
    phvwri      p.{payload_dma_dma_pkt_eop...payload_dma_dma_cmd_type}, CAPRI_DMA_COMMAND_MEM_TO_PKT
    // TODO: How to setup fence? mem2pkt doesn't have fence option
    // TODO: Ring doorbell to update ci
    nop.e
    nop
    

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
pkt_dma_error:
    nop.e
    nop
