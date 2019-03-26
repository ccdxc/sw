#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct ingress_to_rxdma_k k;
struct phv_ p;

%%

ingress_to_rxdma:
    seq             c1, k.service_header_local_ip_mapping_done, FALSE
    seq.!c1         c1, k.service_header_flow_done, FALSE
    bcf             [c1], recirc_packet
    phvwr.!c1       p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr           p.capri_intrinsic_lif, APOLLO_SERVICE_LIF
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                         APOLLO_P4_TO_ARM_HDR_SZ + \
                         APOLLO_P4_TO_RXDMA_HDR_SZ)
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.p4_to_arm_header_valid, TRUE
    phvwr           p.p4_to_rxdma_header_valid, TRUE
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.p4_to_txdma_header_valid, TRUE
    phvwr           p.p4i_apollo_i2e_valid, TRUE
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                        capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        APOLLO_I2E_HDR_SZ
    phvwr           p.p4_to_rxdma_header_table3_valid, TRUE
    phvwr           p.p4_to_rxdma_header_direction, k.control_metadata_direction
    phvwr           p.p4_to_txdma_header_payload_len, r1
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.predicate_header_lpm_bypass, TRUE
    phvwr.e         p.service_header_valid, FALSE
    phvwr           p.predicate_header_direction, k.control_metadata_direction

recirc_packet:
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwr           p.service_header_valid, TRUE

.align
classic_nic_app:
    seq             c1, k.capri_intrinsic_tm_oport, TM_PORT_DMA
    bcf             [c1], classic_nic_to_rxdma
    phvwr           p.capri_p4_intrinsic_valid, TRUE

classic_nic_to_uplink:
    phvwr.e         p.{p4plus_to_p4_vlan_valid,p4plus_to_p4_valid}, 0
    phvwr           p.capri_txdma_intrinsic_valid, 0

classic_nic_to_rxdma:
    phvwr           p.{p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid}, 3
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    // r7 : packet_len
    or              r7, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                        k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
    seq             c1, k.ctag_1_valid, TRUE
    seq             c2, k.control_metadata_vlan_strip, TRUE
    bcf             ![c1&c2], classic_nic_to_rxdma_post_vlan_strip
    nop
    phvwr           p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.{p4_to_p4plus_classic_nic_vlan_pcp...\
                        p4_to_p4plus_classic_nic_vlan_dei}, \
                        k.{ctag_1_pcp,ctag_1_dei,ctag_1_vid_sbit0_ebit3,\
                        ctag_1_vid_sbit4_ebit11}
    phvwr           p.p4_to_p4plus_classic_nic_vlan_valid, TRUE
    phvwr           p.ctag_1_valid, FALSE
    sub             r7, r7, 4
classic_nic_to_rxdma_post_vlan_strip:
    phvwr           p.p4_to_p4plus_classic_nic_packet_len, r7
    phvwr           p.p4_to_p4plus_classic_nic_p4plus_app_id, \
                        k.control_metadata_p4plus_app_id
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                        CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                        P4PLUS_CLASSIC_NIC_HDR_SZ)
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV4
    bcf             [c1], classic_nic_to_rxdma_ipv4
    seq             c2, k.key_metadata_proto, IP_PROTO_TCP
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV6
    nop.!c1.e
classic_nic_to_rxdma_ipv6:
    seq             c3, k.key_metadata_proto, IP_PROTO_UDP
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_TCP
    phvwr.c3.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_UDP
    nop.e
    phvwr           p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6

classic_nic_to_rxdma_ipv4:
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_TCP
    seq             c2, k.key_metadata_proto, IP_PROTO_UDP
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_UDP
    nop.e
    phvwr           p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_to_rxdma_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
