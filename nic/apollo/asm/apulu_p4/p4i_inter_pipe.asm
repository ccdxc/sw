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
    seq             c1, k.control_metadata_flow_miss, TRUE
    bcf             [c1], ingress_to_rxdma
    nop

ingress_to_egress:
    /*
    phvwr           p.p4i_i2e_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          X100 0000 1000
    */
    phvwr.e         p.{p4i_i2e_valid, \
                        p4i_to_rxdma_valid, \
                        p4_to_arm_valid, \
                        p4i_to_p4plus_classic_nic_ip_valid, \
                        p4i_to_p4plus_classic_nic_valid, \
                        ingress_recirc_valid, \
                        capri_rxdma_intrinsic_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x408
    phvwr.f         p.capri_intrinsic_tm_oport, TM_PORT_EGRESS

ingress_to_rxdma:
    /*
    phvwr           p.p4i_to_rxdma_valid, TRUE
    phvwr           p.capri_rxdma_intrinsic_valid, TRUE
    phvwr           p.capri_p4_intrinsic_valid, TRUE
    bitmap          X010 0001 1000
    */
    phvwr           p.{p4i_i2e_valid, \
                        p4i_to_rxdma_valid, \
                        p4_to_arm_valid, \
                        p4i_to_p4plus_classic_nic_ip_valid, \
                        p4i_to_p4plus_classic_nic_valid, \
                        ingress_recirc_valid, \
                        capri_rxdma_intrinsic_valid, \
                        capri_p4_intrinsic_valid, \
                        p4plus_to_p4_vlan_valid, \
                        p4plus_to_p4_valid, \
                        capri_txdma_intrinsic_valid}, 0x218
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
    bitmap          X000 0010 1000
    */
    phvwr.e         p.{p4i_i2e_valid, \
                        p4i_to_rxdma_valid, \
                        p4_to_arm_valid, \
                        p4i_to_p4plus_classic_nic_ip_valid, \
                        p4i_to_p4plus_classic_nic_valid, \
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
