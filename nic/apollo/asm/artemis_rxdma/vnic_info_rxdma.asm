#include "../../p4/include/artemis_sacl_defines.h"
#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_vnic_info_rxdma_k.h"

struct vnic_info_rxdma_k_ k;
struct vnic_info_rxdma_d  d;
struct phv_               p;

%%

vnic_info_rxdma:
    // Pass payload_len from rxdma to txdma
    phvwr        p.rx_to_tx_hdr_payload_len, k.capri_p4_intr_packet_len

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

    // Copy the data that need to go to txdma
    phvwr        p.rx_to_tx_hdr_vpc_id, k.p4_to_rxdma_vpc_id
    phvwr        p.rx_to_tx_hdr_vnic_id, k.p4_to_rxdma_vnic_id
    phvwr        p.rx_to_tx_hdr_direction, k.p4_to_rxdma_direction
    // Pass iptype (address family) to txdma
    phvwr        p.rx_to_tx_hdr_iptype, k.p4_to_rxdma_iptype

    // Setup key for DPORT lookup
    phvwr        p.lpm_metadata_lpm2_key[127:24], k.p4_to_rxdma_flow_proto
    phvwr        p.lpm_metadata_lpm2_key[23:0], k.p4_to_rxdma_flow_dport
    // Enable LPM2
    phvwr        p.p4_to_rxdma_lpm2_enable, TRUE

    // Initialize xlate_port to zero
    add          r2, r0, r0
    // Fill the remote_ip, tag classid, xlate idx's etc based on the direction
    add          r1, k.p4_to_rxdma_direction, r0 // BD Slot
    bnei         r1, TX_FROM_HOST, vnic_info_rx_to_host
    nop          //BD Slot
    phvwr        p.rx_to_tx_hdr_stag_classid, k.p4_to_rxdma_service_tag
    phvwr        p.rx_to_tx_hdr_remote_ip[127:112], k.p4_to_rxdma_flow_dst_s0_e15
    phvwr        p.rx_to_tx_hdr_remote_ip[111:0], k.p4_to_rxdma_flow_dst_s16_e127[111:0]
    // NAT table is not overidding the flow sip/sport so no need to save original
    // flow keys from P4, but just to simply deriving the rflow keys in iflow_rx2tx in
    // non-NAT cases, we can save original keys in nat_ip/xlate_port here, and they
    // get overwritten in NAT if NAT is enabled. These are used for dip/dport of rflow keys
    phvwr        p.rx_to_tx_hdr_nat_ip[127:64], k.p4_to_rxdma_flow_src[127:64]
    phvwr        p.rx_to_tx_hdr_nat_ip[63:0], k.p4_to_rxdma_flow_src[63:0]
    phvwr        p.rx_to_tx_hdr_xlate_port, k.p4_to_rxdma_flow_sport

    // Pick the correct xlate_idx based on dir
    // Rx: svc or pa_or_ca xlate idx
    // Tx: svc or public xlate idx
    // If no NAT, then xlate_idx would be zero and NAT table will still be launched
    sne          c1, k.p4_to_rxdma2_public_xlate_idx, r0
    add.c1       r2, k.p4_to_rxdma2_public_xlate_idx, r0
    phvwr.c1     p.rx_to_tx_hdr_public_xlate_valid, 1
    sne          c1, k.p4_to_rxdma2_service_xlate_idx, r0
    add.c1       r2, k.p4_to_rxdma2_service_xlate_idx, r0
    phvwr.c1     p.rx_to_tx_hdr_svc_xlate_valid, 1
    phvwr.e      p.rxdma_control_xlate_idx, r2
    nop

vnic_info_rx_to_host:
    phvwr        p.rx_to_tx_hdr_dtag_classid, k.p4_to_rxdma_service_tag
    phvwr        p.rx_to_tx_hdr_remote_ip[127:64], k.p4_to_rxdma_flow_src[127:64]
    phvwr        p.rx_to_tx_hdr_remote_ip[63:0], k.p4_to_rxdma_flow_src[63:0]
    // Save the original src or dst ip/port from P4 into nat_ip and xlate_port and pass
    // to txdma, as its needed  for the rflow keys derivation
    phvwr        p.rx_to_tx_hdr_nat_ip[127:112], k.p4_to_rxdma_flow_dst_s0_e15
    phvwr        p.rx_to_tx_hdr_nat_ip[111:0], k.p4_to_rxdma_flow_dst_s16_e127[111:0]
    phvwr        p.rx_to_tx_hdr_xlate_port, k.p4_to_rxdma_flow_dport

    // Pick the correct xlate_idx based on dir
    // Rx: svc or pa_or_ca xlate idx
    // Tx: svc or public xlate idx
    // If no NAT, then xlate_idx would be zero and NAT table will still be launched
    sne          c1, k.p4_to_rxdma2_pa_or_ca_xlate_idx, r0
    add.c1       r2, k.p4_to_rxdma2_pa_or_ca_xlate_idx, r0
    phvwr.c1     p.rx_to_tx_hdr_public_xlate_valid, 1
    sne          c1, k.p4_to_rxdma2_service_xlate_idx, r0
    add.c1       r2, k.p4_to_rxdma2_service_xlate_idx, r0
    phvwr.c1     p.rx_to_tx_hdr_svc_xlate_valid, 1
    phvwr.e      p.rxdma_control_xlate_idx, r2
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
