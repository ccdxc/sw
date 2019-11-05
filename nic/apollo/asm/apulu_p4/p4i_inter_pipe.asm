#include "apulu.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4i_inter_pipe_k.h"

struct p4i_inter_pipe_k_    k;
struct phv_                 p;

%%

p4i_inter_pipe:
    seq             c1, k.ingress_recirc_flow_done, FALSE
    seq.!c1         c1, k.ingress_recirc_local_mapping_done, FALSE
    bcf             [c1], ingress_recirc
    seq             c1, k.control_metadata_tunneled_packet, TRUE
    balcf           r7, [c1], tunnel_decap
    add             r6, r0, k.capri_p4_intrinsic_packet_len
    seq             c1, k.control_metadata_flow_miss_redirect, TRUE
    bcf             [c1], ingress_to_rxdma
    nop

ingress_to_egress:
    /*
    phvwr           p.p4i_i2e_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          1 0000 1000
    */
    phvwr.e         p.{p4i_i2e_valid, \
                        p4i_to_arm_valid, \
                        p4i_to_rxdma_valid, \
                        ingress_recirc_valid, \
                        capri_rxdma_intrinsic_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x108
    phvwr.f         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS

ingress_to_rxdma:
    /*
    phvwr           p.p4i_to_arm_valid, TRUE
    phvwr           p.p4i_to_rxdma_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          1 1101 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        p4i_to_arm_valid, \
                        p4i_to_rxdma_valid, \
                        ingress_recirc_valid, \
                        capri_rxdma_intrinsic_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x1D8

    phvwr           p.p4i_to_arm_packet_len, k.capri_p4_intrinsic_packet_len
    phvwr           p.p4i_to_arm_flow_hash, k.p4i_i2e_entropy_hash
    phvwr           p.p4i_to_arm_payload_offset, k.offset_metadata_payload_offset
    or              r1, k.p4i_i2e_xlate_id, k.capri_intrinsic_lif, 48
    phvwr           p.{p4i_to_arm_lif, \
                        p4i_to_arm_egress_bd_id,\
                        p4i_to_arm_service_xlate_id, \
                        p4i_to_arm_mapping_xlate_id}, r1

    phvwr           p.p4i_to_rxdma_apulu_p4plus, TRUE
    or              r1, k.vnic_metadata_vnic_id, \
                        k.control_metadata_rx_packet, 10
    seq             c1, k.key_metadata_ktype, KEY_TYPE_IPV6
    or.c1           r1, 1, r1, 11
    phvwr           p.p4i_to_rxdma_vnic_info_key, r1
    seq.!c1         c1, k.key_metadata_ktype, KEY_TYPE_IPV4
    phvwr.c1        p.p4i_to_rxdma_vnic_info_en, TRUE

    add             r1, k.capri_p4_intrinsic_packet_len, \
                        (APULU_I2E_HDR_SZ + APULU_P4_TO_ARM_HDR_SZ)
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    phvwr           p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr.e         p.capri_intrinsic_lif, APULU_SERVICE_LIF
    phvwr.f         p.capri_rxdma_intrinsic_rx_splitter_offset, \
                        (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                         CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                         APULU_P4I_TO_RXDMA_HDR_SZ)

ingress_recirc:
    /*
    phvwr           p.ingress_recirc_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          0 0010 1000
    */
    phvwr.e         p.{p4i_i2e_valid, \
                        p4i_to_arm_valid, \
                        p4i_to_rxdma_valid, \
                        ingress_recirc_valid, \
                        capri_rxdma_intrinsic_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x028
    phvwr.f         p.capri_intrinsic_tm_oport, TM_PORT_INGRESS

tunnel_decap:
    phvwr           p.{vxlan_1_valid,udp_1_valid,ipv4_1_valid,ipv6_1_valid, \
                        ctag_1_valid,ethernet_1_valid}, 0
    sub             r6, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_2
    jr              r7
    phvwr           p.capri_p4_intrinsic_packet_len, r6

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_inter_pipe_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
