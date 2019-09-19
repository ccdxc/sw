#include "../../p4/include/apulu_sacl_defines.h"
#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_vnic_info_rxdma_k.h"

struct vnic_info_rxdma_k_ k;
struct vnic_info_rxdma_d  d;
struct phv_               p;

%%

vnic_info_rxdma:

    // Copy the LPM roots to PHV based on AF
    addi         r1, r0, SACL_PROTO_DPORT_TABLE_OFFSET
    seq          c1, k.p4_to_rxdma_iptype, IPTYPE_IPV4
    phvwr.c1     p.rx_to_tx_hdr_sacl_base_addr, d.vnic_info_rxdma_d.lpm_base1
    phvwr.c1     p.lpm_metadata_meter_base_addr, d.vnic_info_rxdma_d.lpm_base3
    phvwr.c1     p.rx_to_tx_hdr_route_base_addr, d.vnic_info_rxdma_d.lpm_base5
    add.c1       r1, r1, d.vnic_info_rxdma_d.lpm_base1
    phvwr.!c1    p.rx_to_tx_hdr_sacl_base_addr, d.vnic_info_rxdma_d.lpm_base2
    phvwr.!c1    p.lpm_metadata_meter_base_addr, d.vnic_info_rxdma_d.lpm_base4
    phvwr.!c1    p.rx_to_tx_hdr_route_base_addr, d.vnic_info_rxdma_d.lpm_base6
    add.!c1      r1, r1, d.vnic_info_rxdma_d.lpm_base2
    phvwr        p.lpm_metadata_lpm2_base_addr, r1

    // Fill the remote_ip, tag classid etc based on the direction
    seq          c1, k.p4_to_rxdma_rx_packet, 0
    phvwr.c1     p.rx_to_tx_hdr_stag_classid, k.p4_to_rxdma_service_tag
    phvwr.c1     p.rx_to_tx_hdr_remote_ip[127:64], k.p4_to_rxdma_flow_dst[127:64]
    phvwr.c1     p.rx_to_tx_hdr_remote_ip[63:0], k.p4_to_rxdma_flow_dst[63:0]
    phvwr.!c1    p.rx_to_tx_hdr_dtag_classid, k.p4_to_rxdma_service_tag
    phvwr.!c1    p.rx_to_tx_hdr_remote_ip[127:64], k.p4_to_rxdma_flow_src[127:64]
    phvwr.!c1    p.rx_to_tx_hdr_remote_ip[63:0], k.p4_to_rxdma_flow_src[63:0]

    // Copy the data that need to go to txdma
    phvwr        p.rx_to_tx_hdr_vpc_id, k.p4_to_rxdma_vpc_id
    phvwr        p.rx_to_tx_hdr_vnic_id, k.p4_to_rxdma_vnic_id
    phvwr        p.rx_to_tx_hdr_rx_packet, k.p4_to_rxdma_rx_packet
    phvwr        p.rx_to_tx_hdr_iptype, k.p4_to_rxdma_iptype
    phvwr        p.rx_to_tx_hdr_payload_len, k.capri_p4_intr_packet_len

    // Enable LPM2
    phvwr        p.p4_to_rxdma_lpm2_enable, TRUE

    // Setup key for DPORT lookup
    phvwr.e      p.lpm_metadata_lpm2_key[23:16], k.p4_to_rxdma_flow_proto
    phvwr        p.lpm_metadata_lpm2_key[15:0], k.p4_to_rxdma_flow_dport

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
