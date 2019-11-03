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
    // Disable this lookup for further passes
    phvwr       p.p4_to_rxdma_vnic_info_en, FALSE

    // Copy the data that need to go to txdma
    phvwr        p.rx_to_tx_hdr_rx_packet, k.p4_to_rxdma_rx_packet
    phvwr        p.rx_to_tx_hdr_payload_len, k.capri_p4_intr_packet_len
    phvwr        p.rx_to_tx_hdr_vpc_id, k.p4_to_rxdma_vpc_id
    phvwr        p.rx_to_tx_hdr_vnic_id, k.p4_to_rxdma_vnic_info_key[9:0]
    phvwr        p.rx_to_tx_hdr_iptype, k.p4_to_rxdma_iptype
    phvwr        p.rx_to_tx_hdr_route_base_addr, d.vnic_info_rxdma_d.lpm_base1

    // Fill the remote_ip based on the direction
    seq          c1, k.p4_to_rxdma_rx_packet, 0
    phvwr.c1     p.rx_to_tx_hdr_remote_ip[127:64], k.p4_to_rxdma_flow_dst[127:64]
    phvwr.c1     p.rx_to_tx_hdr_remote_ip[63:0], k.p4_to_rxdma_flow_dst[63:0]
    phvwr.!c1    p.rx_to_tx_hdr_remote_ip[127:64], k.p4_to_rxdma_flow_src[127:64]
    phvwr.!c1    p.rx_to_tx_hdr_remote_ip[63:0], k.p4_to_rxdma_flow_src[63:0]

    // if sacl_base_address == NULL, stop!
    add          r1, d.vnic_info_rxdma_d.lpm_base2, r0
    seq          c1, r1, r0
    nop.c1.e

    // Copy the root and sacl roots to PHV
    phvwr        p.lpm_metadata_sacl_base_addr,  d.vnic_info_rxdma_d.lpm_base2
    phvwr        p.rx_to_tx_hdr_sacl_base_addr0, d.vnic_info_rxdma_d.lpm_base2
    phvwr        p.rx_to_tx_hdr_sacl_base_addr1, d.vnic_info_rxdma_d.lpm_base3
    phvwr        p.rx_to_tx_hdr_sacl_base_addr2, d.vnic_info_rxdma_d.lpm_base4
    phvwr        p.rx_to_tx_hdr_sacl_base_addr3, d.vnic_info_rxdma_d.lpm_base5
    phvwr        p.rx_to_tx_hdr_sacl_base_addr4, d.vnic_info_rxdma_d.lpm_base6
    phvwr        p.rx_to_tx_hdr_sacl_base_addr5, d.vnic_info_rxdma_d.lpm_base7

    // Setup and Enable LPM1 for SPORT lookup
    addi         r2, r1, SACL_SPORT_TABLE_OFFSET
    phvwr        p.lpm_metadata_lpm1_base_addr, r2
    phvwr        p.lpm_metadata_lpm1_key[15:8], k.p4_to_rxdma_flow_sport_s0_e7
    phvwr        p.lpm_metadata_lpm1_key[7:0], k.p4_to_rxdma_flow_sport_s8_e15

    // Setup and Enable LPM2 for DPORT lookup
    addi         r2, r1, SACL_PROTO_DPORT_TABLE_OFFSET
    phvwr        p.lpm_metadata_lpm2_base_addr, r2
    phvwr        p.lpm_metadata_lpm2_key[15:0], k.p4_to_rxdma_flow_dport
    phvwr        p.lpm_metadata_lpm2_key[23:16], k.p4_to_rxdma_flow_proto

    // Enable both LPMs
    phvwr.e      p.p4_to_rxdma_lpm1_enable, TRUE
    phvwr        p.p4_to_rxdma_lpm2_enable, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
