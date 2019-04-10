#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_             p;
struct pkt_dma_setup_k  k;

%%

pkt_dma_setup:
    phvwr       p.capri_txdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(intrinsic_dma_dma_cmd_phv2pkt_pad)/16
    sub         r1, k.{p4_to_txdma_header_payload_len_sbit0_ebit5, \
                       p4_to_txdma_header_payload_len_sbit6_ebit13}, APOLLO_I2E_HDR_SZ
    phvwr       p.capri_p4_intr_packet_len, r1

    CAPRI_RING_DOORBELL_DATA(0, k.capri_txdma_intr_qid, 0, k.txdma_control_cindex)
    phvwr       p.{doorbell_data_pid...doorbell_data_index}, r3.dx

    bbeq        k.predicate_header_redirect_to_arm, TRUE, pkt_dma_to_ingress_setup
    phvwr       p.capri_intr_tm_iport, TM_PORT_DMA
pkt_dma_to_egress_setup:
    phvwr.e     p.capri_intr_tm_oport, TM_PORT_EGRESS
    phvwr       p.txdma_to_p4e_header_vcn_id, k.p4_to_txdma_header_vcn_id

pkt_dma_to_ingress_setup:
    phvwr       p.capri_intr_tm_oport, TM_PORT_INGRESS
    phvwr       p.p4_to_txdma_header_payload_len, r1
    add.e       r1, k.txdma_control_payload_addr, APOLLO_I2E_HDR_SZ
    phvwr       p.txdma_control_payload_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
pkt_dma_setup_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
