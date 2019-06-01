#include "../../p4/include/artemis_sacl_defines.h"
#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct vnic_info_rxdma_k  k;
struct vnic_info_rxdma_d  d;
struct phv_             p;

%%

vnic_info_rxdma:
    // Disable this lookup for further passes
    phvwr        p.p4_to_rxdma_vnic_info_en, FALSE

    // Copy the LPM roots to PHV based on AF
    addi         r1, r0, SACL_PROTO_DPORT_TABLE_OFFSET
    seq          c1, k.p4_to_rxdma_iptype, IPTYPE_IPV4
    phvwr.c1     p.lpm_metadata_sacl_base_addr, d.vnic_info_rxdma_d.lpm_base1
    phvwr.c1     p.lpm_metadata_meter_base_addr, d.vnic_info_rxdma_d.lpm_base3
    add.c1       r1, r1, d.vnic_info_rxdma_d.lpm_base1
    phvwr.!c1    p.lpm_metadata_sacl_base_addr, d.vnic_info_rxdma_d.lpm_base2
    phvwr.!c1    p.lpm_metadata_meter_base_addr, d.vnic_info_rxdma_d.lpm_base4
    add.!c1      r1, r1, d.vnic_info_rxdma_d.lpm_base2
    phvwr        p.lpm_metadata_lpm2_base_addr, r1

    // Always fill the remote_ip from p4 keys based on the direction
    seq          c1, k.p4_to_rxdma_direction, TX_FROM_HOST
    phvwr.c1     p.lpm_metadata_remote_ip[127:64], k.p4_to_rxdma_flow_dst_sbit0_ebit87[87:24]
    phvwr.c1     p.lpm_metadata_remote_ip[63:40], k.p4_to_rxdma_flow_dst_sbit0_ebit87[23:0]
    phvwr.c1     p.lpm_metadata_remote_ip[39:0], k.p4_to_rxdma_flow_dst_sbit88_ebit127
    phvwr.!c1    p.lpm_metadata_remote_ip[127:120], k.p4_to_rxdma_flow_src_sbit0_ebit7
    phvwr.!c1    p.lpm_metadata_remote_ip[119:64], k.p4_to_rxdma_flow_src_sbit8_ebit127[119:64]
    phvwr.!c1    p.lpm_metadata_remote_ip[63:0], k.p4_to_rxdma_flow_src_sbit8_ebit127[63:0]

    // Setup key for DPORT lookup
    phvwr        p.lpm_metadata_lpm2_key[23:16], k.p4_to_rxdma_flow_proto
    phvwr.e      p.lpm_metadata_lpm2_key[15:0], k.p4_to_rxdma_flow_dport
    // Enable LPM2
    phvwr        p.p4_to_rxdma_lpm2_enable, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_info_rxdma_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
