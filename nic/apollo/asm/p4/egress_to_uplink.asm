#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_to_uplink_k.h"

struct egress_to_uplink_k_ k;
struct phv_ p;

%%

egress_to_uplink:
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    phvwr       p.capri_txdma_intrinsic_valid, FALSE
    phvwr       p.txdma_to_p4e_header_valid, FALSE
    bbeq        k.control_metadata_local_switching, TRUE, egress_to_ingress
    phvwr       p.p4e_apollo_i2e_valid, FALSE
egress_to_uplink2:
    phvwr       p.capri_intrinsic_tm_span_session, \
                    k.control_metadata_mirror_session
    seq         c1, k.capri_intrinsic_tm_oport, TM_PORT_DMA
    nop.!c1.e
    phvwr       p.predicate_header_valid, FALSE
egress_to_dma:
    phvwrmi     p.{p4_to_p4plus_classic_nic_ip_valid, \
                   p4_to_p4plus_classic_nic_valid, \
                   p4e_apollo_i2e_valid, \
                   txdma_to_p4e_header_valid, \
                   predicate_header_valid, \
                   capri_txdma_intrinsic_valid, \
                   capri_rxdma_intrinsic_valid, \
                   egress_service_header_valid, \
                   mirror_blob_valid, \
                   capri_p4_intrinsic_valid}, 0x309, 0x309
    phvwr       p.p4_to_p4plus_classic_nic_packet_len, \
                    k.capri_p4_intrinsic_packet_len
    phvwr.e     p.p4_to_p4plus_classic_nic_p4plus_app_id, \
                    P4PLUS_APPTYPE_CLASSIC_NIC
    phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, \
                    (ASICPD_GLOBAL_INTRINSIC_HDR_SZ + \
                    ASICPD_RXDMA_INTRINSIC_HDR_SZ + \
                    P4PLUS_CLASSIC_NIC_HDR_SZ)

egress_to_ingress:
    seq         c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1    p.{predicate_header_pad0...predicate_header_direction}, \
                    TX_FROM_HOST
    phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwr.!c1   p.{predicate_header_pad0...predicate_header_direction}, \
                    RX_FROM_SWITCH

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_to_uplink_error:
    phvwr.e     p.capri_intrinsic_drop, 1
    phvwr       p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
