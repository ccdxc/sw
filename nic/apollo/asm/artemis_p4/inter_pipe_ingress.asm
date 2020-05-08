#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_inter_pipe_ingress_k.h"

struct inter_pipe_ingress_k_ k;
struct inter_pipe_ingress_d  d;
struct phv_ p;

%%

ingress_to_egress:
    /*
    phvwr           p.p4i_i2e_valid, TRUE
    phvwr           p.txdma_to_p4e_valid, TRUE
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          1110 0000 0000 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        txdma_to_p4e_valid, \
                        predicate_header_valid, \
                        p4_to_rxdma3_valid, \
                        p4_to_rxdma2_valid, \
                        p4_to_rxdma_valid, \
                        cps_blob_valid, \
                        p4_to_arm_valid, \
                        p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid, \
                        capri_rxdma_intrinsic_valid, \
                        ingress_recirc_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0xE008
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    nop.!c1.e
    phvwr           p.predicate_header_direction, k.control_metadata_direction
    seq             c1, k.vxlan_1_valid, TRUE
    phvwr.c1        p.{vxlan_1_valid,udp_1_valid,ipv4_1_valid,ipv6_1_valid, \
                        ctag_1_valid,ethernet_1_valid}, 0
    seq             c1, k.vxlan_2_valid, TRUE
    phvwr.c1        p.{vxlan_2_valid,udp_2_valid,ipv4_2_valid,ipv6_2_valid, \
                        ctag_2_valid,ethernet_2_valid}, 0
    seq             c1, k.ctag_1_valid, TRUE
    phvwr.e         p.ctag_1_valid, FALSE
    phvwr.c1        p.ethernet_1_etherType, k.ctag_1_etherType

.align
ingress_to_cps:
    /*
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.p4_to_rxdma3_valid, TRUE
    phvwr           p.p4_to_rxdma2_valid, TRUE
    phvwr           p.p4_to_rxdma_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          0011 1100 0010 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        txdma_to_p4e_valid, \
                        predicate_header_valid, \
                        p4_to_rxdma3_valid, \
                        p4_to_rxdma2_valid, \
                        p4_to_rxdma_valid, \
                        cps_blob_valid, \
                        p4_to_arm_valid, \
                        p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid, \
                        capri_rxdma_intrinsic_valid, \
                        ingress_recirc_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x3C28
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr           p.capri_intrinsic_lif, ARTEMIS_SERVICE_LIF
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (ASICPD_GLOBAL_INTRINSIC_HDR_SZ + \
                         ASICPD_RXDMA_INTRINSIC_HDR_SZ + \
                         ARTEMIS_P4_TO_RXDMA_HDR_SZ)
    phvwr           p.predicate_header_direction, k.control_metadata_direction
    phvwr           p.p4_to_rxdma_vnic_info_en, TRUE
    phvwr.e         p.p4_to_rxdma_cps_path_en, TRUE
    phvwr.f         p.p4_to_rxdma_direction, k.control_metadata_direction

.align
ingress_to_classic_nic:
    phvwr           p.{capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x8
    // r1 : packet_len
    add             r1, r0, k.capri_p4_intrinsic_packet_len
    phvwr           p.{p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid}, 0x3
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
ingress_to_classic_nic_common:
    seq             c1, k.ctag_1_valid, TRUE
    seq             c2, k.control_metadata_vlan_strip, TRUE
    bcf             ![c1&c2], ingress_to_classic_nic_post_vlan_strip
    nop
    phvwr           p.ethernet_1_etherType, k.ctag_1_etherType
    phvwr           p.{p4_to_p4plus_classic_nic_vlan_pcp...\
                        p4_to_p4plus_classic_nic_vlan_dei}, \
                        k.{ctag_1_pcp,ctag_1_dei,ctag_1_vid}
    phvwr           p.p4_to_p4plus_classic_nic_vlan_valid, TRUE
    phvwr           p.ctag_1_valid, FALSE
    sub             r1, r1, 4
ingress_to_classic_nic_post_vlan_strip:
    phvwr           p.p4_to_p4plus_classic_nic_packet_len, r1
    phvwr           p.p4_to_p4plus_classic_nic_p4plus_app_id, \
                        P4PLUS_APPTYPE_CLASSIC_NIC
    phvwr           p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (ASICPD_GLOBAL_INTRINSIC_HDR_SZ + \
                        ASICPD_RXDMA_INTRINSIC_HDR_SZ + \
                        P4PLUS_CLASSIC_NIC_HDR_SZ)
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV4
    bcf             [c1], ingress_to_classic_nic_ipv4
    seq             c2, k.key_metadata_proto, IP_PROTO_TCP
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV6
    nop.!c1.e
ingress_to_classic_nic_ipv6:
    seq             c3, k.key_metadata_proto, IP_PROTO_UDP
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_TCP
    phvwr.c3.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6_UDP
    nop.e
    phvwr.f         p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV6

ingress_to_classic_nic_ipv4:
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_TCP
    seq             c2, k.key_metadata_proto, IP_PROTO_UDP
    phvwr.c2.e      p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4_UDP
    nop.e
    phvwr.f         p.p4_to_p4plus_classic_nic_pkt_type, \
                        CLASSIC_NIC_PKT_TYPE_IPV4

.align
ingress_to_uplink:
    phvwr.e.f       p.{capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x8
    nop

.align
ingress_to_arm:
    /*
    phvwr           p.cps_blob_valid, TRUE
    phvwr           p.p4_to_arm_valid, TRUE
    phvwr           p4_to_p4plus_classic_nic_ip_valid, TRUE
    phvwr           p4_to_p4plus_classic_nic_valid, TRUE
    phvwr           capri_rxdma_intrinsic_valid,TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          0000 0011 1110 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        txdma_to_p4e_valid, \
                        predicate_header_valid, \
                        p4_to_rxdma3_valid, \
                        p4_to_rxdma2_valid, \
                        p4_to_rxdma_valid, \
                        cps_blob_valid, \
                        p4_to_arm_valid, \
                        p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid, \
                        capri_rxdma_intrinsic_valid, \
                        ingress_recirc_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x03E8
    or              r1, k.ctag_1_valid, k.ipv4_1_valid, \
                        ARTEMIS_CPU_FLAGS_IPV4_1_VALID_BIT_POS
    or              r1, r1, k.ipv6_1_valid, \
                        ARTEMIS_CPU_FLAGS_IPV6_1_VALID_BIT_POS
    or              r1, r1, k.ethernet_2_valid, \
                        ARTEMIS_CPU_FLAGS_ETH_2_VALID_BIT_POS
    or              r1, r1, k.ipv4_2_valid, \
                        ARTEMIS_CPU_FLAGS_IPV4_2_VALID_BIT_POS
    or              r1, r1, k.ipv6_2_valid, \
                        ARTEMIS_CPU_FLAGS_IPV6_2_VALID_BIT_POS
    or              r1, r1, k.control_metadata_direction, \
                        ARTEMIS_CPU_FLAGS_DIRECTION_BIT_POS
    phvwr           p.p4_to_arm_flags, r1
    phvwr           p.p4_to_arm_packet_len, k.capri_p4_intrinsic_packet_len
    phvwr           p.p4_to_arm_local_vnic_tag, k.vnic_metadata_vpc_id
    phvwr           p.p4_to_arm_flow_hash, k.p4i_i2e_entropy_hash
    phvwr           p.p4_to_arm_payload_offset, k.offset_metadata_payload_offset
    b               ingress_to_classic_nic_common
    add             r1, k.capri_p4_intrinsic_packet_len, ARTEMIS_P4_TO_ARM_HDR_SZ

.align
ingress_to_ingress:
    /*
    phvwr           p.predicate_header_valid, TRUE
    phvwr           p.ingress_recirc_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          0010 0000 0001 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        txdma_to_p4e_valid, \
                        predicate_header_valid, \
                        p4_to_rxdma3_valid, \
                        p4_to_rxdma2_valid, \
                        p4_to_rxdma_valid, \
                        cps_blob_valid, \
                        p4_to_arm_valid, \
                        p4_to_p4plus_classic_nic_ip_valid, \
                        p4_to_p4plus_classic_nic_valid, \
                        capri_rxdma_intrinsic_valid, \
                        ingress_recirc_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x2018
    phvwr.e         p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
    phvwr.f         p.predicate_header_direction, k.control_metadata_direction

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
inter_pipe_ingress_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
