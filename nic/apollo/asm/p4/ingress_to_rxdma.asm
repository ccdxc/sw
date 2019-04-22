#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_ingress_to_rxdma_k.h"

struct ingress_to_rxdma_k_ k;
struct phv_ p;

%%

ingress_to_rxdma:
    seq             c1, k.service_header_local_ip_mapping_done, FALSE
    seq.!c1         c1, k.service_header_flow_done, FALSE
    bcf             [c1], recirc_packet
    phvwr.!c1       p.capri_intrinsic_tm_span_session, \
                        k.control_metadata_mirror_session
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr           p.capri_intrinsic_lif, APOLLO_SERVICE_LIF
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                         APOLLO_P4_TO_RXDMA_HDR_SZ + APOLLO_PREDICATE_HDR_SZ)
    /*
    phvwr           p.service_header_valid, FALSE
    phvwr           p.p4plus_to_p4_vlan_valid, FALSE
    phvwr           p.p4plus_to_p4_valid, FALSE
    phvwr           p.capri_txdma_intrinsic_valid, FALSE
    phvwr           p.p4i_apollo_i2e_valid, TRUE
    phvwr           p.p4_to_txdma_header_valid, TRUE
    phvwr           p.predicate_header2_valid, TRUE
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.p4_to_rxdma_header_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    */
    phvwrmi         p.{service_header_valid, \
                       p4plus_to_p4_vlan_valid, \
                       p4plus_to_p4_valid, \
                       capri_txdma_intrinsic_valid, \
                       p4i_apollo_i2e_valid, \
                       p4_to_txdma_header_valid, \
                       p4_to_arm_valid, \
                       p4_to_p4plus_classic_nic_ip_valid, \
                       p4_to_p4plus_classic_nic_valid, \
                       predicate_header2_valid, \
                       predicate_header_valid, \
                       p4_to_rxdma_header_valid, \
                       capri_rxdma_intrinsic_valid, \
                       capri_p4_intrinsic_valid}, 0x03FF, 0x3F1F
    add             r1, k.capri_p4_intrinsic_packet_len, APOLLO_I2E_HDR_SZ
    phvwr           p.p4_to_rxdma_header_table3_valid, TRUE
    phvwr           p.p4_to_rxdma_header_direction, k.control_metadata_direction
    phvwr           p.p4_to_txdma_header_payload_len, r1
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    seq.!c1         c1, k.p4_to_txdma_header_lpm_addr, r0
    phvwr.c1        p.predicate_header_lpm_bypass, TRUE
    phvwr.e         p.service_header_valid, FALSE
    phvwr           p.predicate_header_direction, k.control_metadata_direction

recirc_packet:
    phvwrmi         p.{predicate_header_valid, \
                       p4_to_rxdma_header_valid, \
                       capri_rxdma_intrinsic_valid, \
                       capri_p4_intrinsic_valid}, 0x1, 0xF
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwrmi         p.{service_header_valid,p4plus_to_p4_vlan_valid, \
                       p4plus_to_p4_valid,capri_txdma_intrinsic_valid}, 0x8, 0xF

.align
classic_nic_app:
    seq             c1, k.capri_intrinsic_tm_oport, TM_PORT_DMA
    bcf             [c1], classic_nic_to_rxdma
    phvwr           p.capri_p4_intrinsic_valid, TRUE

classic_nic_to_uplink:
    phvwr.e         p.{p4plus_to_p4_vlan_valid,p4plus_to_p4_valid}, 0
    phvwr           p.capri_txdma_intrinsic_valid, 0

classic_nic_to_rxdma:
    // r1 : packet_len
    add             r1, r0, k.capri_p4_intrinsic_packet_len
    phvwr           p.{p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid}, 3
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
classic_nic_to_rxdma_common:
    seq             c1, k.ctag_1_valid, TRUE
    seq             c2, k.control_metadata_vlan_strip, TRUE
    bcf             ![c1&c2], classic_nic_to_rxdma_post_vlan_strip
    nop
    phvwr           p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.{p4_to_p4plus_classic_nic_vlan_pcp...\
                        p4_to_p4plus_classic_nic_vlan_dei}, \
                        k.{ctag_1_pcp,ctag_1_dei,ctag_1_vid}
    phvwr           p.p4_to_p4plus_classic_nic_vlan_valid, TRUE
    phvwr           p.ctag_1_valid, FALSE
    sub             r1, r1, 4
classic_nic_to_rxdma_post_vlan_strip:
    phvwr           p.p4_to_p4plus_classic_nic_packet_len, r1
    phvwr           p.p4_to_p4plus_classic_nic_p4plus_app_id, \
                        P4PLUS_APPTYPE_CLASSIC_NIC
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

.align
redirect_to_arm:
    /*
    phvwr           p.capri_txdma_intrinsic, FALSE
    phvwr           p.p4_to_arm_valid, TRUE
    phvwr           p4_to_p4plus_classic_nic_ip_valid, TRUE
    phvwr           p4_to_p4plus_classic_nic_valid, TRUE
    phvwr           p.predicate_header2_valid, FALSE
    phvwr           p.predicate_header_valid, FALSE
    phvwr           capri_rxdma_intrinsic_valid,TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    */
    phvwrmi         p.{capri_txdma_intrinsic_valid,\
                       p4i_apollo_i2e_valid, \
                       p4_to_txdma_header_valid, \
                       p4_to_arm_valid, \
                       p4_to_p4plus_classic_nic_ip_valid, \
                       p4_to_p4plus_classic_nic_valid, \
                       predicate_header2_valid, \
                       predicate_header_valid, \
                       p4_to_rxdma_header_valid, \
                       capri_rxdma_intrinsic_valid, \
                       capri_p4_intrinsic_valid}, 0x0E3, 0x4FB
    or              r1, k.ctag_1_valid, k.ipv4_1_valid, \
                        APOLLO_CPU_FLAGS_IPV4_1_VALID_BIT_POS
    or              r1, r1, k.ipv6_1_valid, \
                        APOLLO_CPU_FLAGS_IPV6_1_VALID_BIT_POS
    or              r1, r1, k.ethernet_2_valid, \
                        APOLLO_CPU_FLAGS_ETH_2_VALID_BIT_POS
    or              r1, r1, k.ipv4_2_valid, \
                        APOLLO_CPU_FLAGS_IPV4_2_VALID_BIT_POS
    or              r1, r1, k.ipv6_2_valid, \
                        APOLLO_CPU_FLAGS_IPV6_2_VALID_BIT_POS
    or              r1, r1, k.control_metadata_direction, \
                        APOLLO_CPU_FLAGS_DIRECTION_BIT_POS
    phvwr           p.p4_to_arm_flags, r1
    phvwr           p.p4_to_arm_packet_len, k.capri_p4_intrinsic_packet_len
    phvwr           p.p4_to_arm_flow_hash, k.p4i_apollo_i2e_entropy_hash
    phvwr           p.p4_to_arm_payload_offset, k.offset_metadata_payload_offset
    b               classic_nic_to_rxdma_common
    add             r1, k.capri_p4_intrinsic_packet_len, APOLLO_P4_TO_ARM_HDR_SZ

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ingress_to_rxdma_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
