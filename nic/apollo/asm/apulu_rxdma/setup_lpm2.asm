#include "../../p4/include/apulu_sacl_defines.h"

#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_setup_lpm2_k.h"

struct phv_             p;
struct setup_lpm2_k_    k;

%%

setup_lpm2:
    // Load the correct sacl_base_address into r1 based on recirc count
    seq              c1, k.lpm_metadata_recirc_count, 0
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, k.rx_to_tx_hdr_sacl_base_addr1
    seq              c1, k.lpm_metadata_recirc_count, 2
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, k.rx_to_tx_hdr_sacl_base_addr2
    seq              c1, k.lpm_metadata_recirc_count, 4
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, k.rx_to_tx_hdr_sacl_base_addr3
    seq              c1, k.lpm_metadata_recirc_count, 6
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, k.rx_to_tx_hdr_sacl_base_addr4
    seq              c1, k.lpm_metadata_recirc_count, 8
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, k.rx_to_tx_hdr_sacl_base_addr5
    seq              c1, k.lpm_metadata_recirc_count, 10
    bcf              [c1], setup_lpm_dip_sip
    phvwr.c1         p.lpm_metadata_sacl_base_addr, r0
    nop.e
    nop

setup_lpm_dip_sip:
    // if sacl_base_address == NULL, stop
    add              r1, r0, k.lpm_metadata_sacl_base_addr
    seq              c1, r1, r0
    phvwr.c1.e       p.capri_p4_intr_recirc, FALSE

    // Load SIP table offset into r1 based on IP family
    seq              c1, k.p4_to_rxdma_iptype, IPTYPE_IPV4
    addi.c1          r2, r0, SACL_IPV4_SIP_TABLE_OFFSET
    addi.!c1         r2, r0, SACL_IPV6_SIP_TABLE_OFFSET

    // Setup root for SIP lookup on LPM1
    add              r2, r1, r2
    phvwr            p.lpm_metadata_lpm1_base_addr, r2

    // Setup key for SIP lookup on LPM1
    phvwr            p.lpm_metadata_lpm1_key[127:64], k.p4_to_rxdma_flow_src[127:64]
    phvwr            p.lpm_metadata_lpm1_key[63:0],  k.p4_to_rxdma_flow_src[63:0]

    // Setup root for DIP lookup on LPM2
    addi             r2, r1, SACL_DIP_TABLE_OFFSET
    phvwr            p.lpm_metadata_lpm2_base_addr, r2

    // Setup key for DIP lookup on LPM2
    phvwr            p.lpm_metadata_lpm2_key[127:64], k.p4_to_rxdma_flow_dst_s0_e119[119:56]
    phvwr            p.lpm_metadata_lpm2_key[63:8], k.p4_to_rxdma_flow_dst_s0_e119[55:0]
    phvwr            p.lpm_metadata_lpm2_key[7:0], k.p4_to_rxdma_flow_dst_s120_e127

    // Enable both LPMs and recirc again
    phvwr            p.p4_to_rxdma_lpm1_enable, TRUE
    phvwr.e          p.p4_to_rxdma_lpm2_enable, TRUE
    phvwr            p.capri_p4_intr_recirc, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
setup_lpm2_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
