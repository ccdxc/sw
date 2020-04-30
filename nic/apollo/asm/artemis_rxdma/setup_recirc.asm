#include "../../p4/include/artemis_sacl_defines.h"

#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct setup_recirc_k      k;

%%

setup_recirc:
    /* Is this the first pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 0
    /* If so, go to pass0 */
    bcf              [c1], pass0
    /* Is this the second pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 1
    /* If so, go to pass1 */
    bcf              [c1], pass1
    /* Third pass. Stop */
    nop
    /* Disable Recirc */
    phvwr.e          p.capri_p4_intr_recirc, FALSE
    nop

pass0:
    /* Setup key for SIP lookup on LPM1 */
    phvwr            p.lpm_metadata_lpm1_key[127:120], k.p4_to_rxdma_flow_src_sbit0_ebit7
    phvwr            p.lpm_metadata_lpm1_key[119:64], k.p4_to_rxdma_flow_src_sbit8_ebit127[119:64]
    phvwr            p.lpm_metadata_lpm1_key[63:0],  k.p4_to_rxdma_flow_src_sbit8_ebit127[63:0]

    /* Setup root for SIP lookup on LPM1 */
    seq              c1, k.p4_to_rxdma_iptype, IPTYPE_IPV4
    addi.c1          r1, r0, SACL_IPV4_SIP_TABLE_OFFSET
    addi.!c1         r1, r0, SACL_IPV6_SIP_TABLE_OFFSET
    add              r1, r1, k.rx_to_tx_hdr_sacl_base_addr
    phvwr            p.lpm_metadata_lpm1_base_addr, r1

    /* Setup key for DIP lookup on LPM2 */
    phvwr            p.lpm_metadata_lpm2_key[127:64], k.p4_to_rxdma_flow_dst_sbit0_ebit79[79:16]
    phvwr            p.lpm_metadata_lpm2_key[63:48], k.p4_to_rxdma_flow_dst_sbit0_ebit79[15:0]
    phvwr            p.lpm_metadata_lpm2_key[47:40], k.p4_to_rxdma_flow_dst_sbit80_ebit87
    phvwr            p.lpm_metadata_lpm2_key[39:0], k.p4_to_rxdma_flow_dst_sbit88_ebit127

    /* Setup root for DIP lookup on LPM2 */
    addi             r1, r0, SACL_DIP_TABLE_OFFSET
    add              r1, r1, k.rx_to_tx_hdr_sacl_base_addr
    phvwr            p.lpm_metadata_lpm2_base_addr, r1

    /* Enable LPM1 */
    phvwr.e          p.p4_to_rxdma_lpm1_enable, TRUE
    /* Enable Recirc */
    phvwr            p.capri_p4_intr_recirc, TRUE

pass1:
    /* Setup key for TAG Lookup on LPM1 */
    phvwr            p.lpm_metadata_lpm1_key[127:0], k.{rx_to_tx_hdr_remote_ip_sbit0_ebit15...rx_to_tx_hdr_remote_ip_sbit96_ebit127}

    /* Setup root for TAG Lookup on LPM1 */
    phvwr            p.lpm_metadata_lpm1_base_addr, k.{p4_to_rxdma_tag_root_sbit0_ebit7...\
                                                       p4_to_rxdma_tag_root_sbit32_ebit39}

    /* Setup key for METER Lookup on LPM1 */
    phvwr            p.lpm_metadata_lpm2_key[127:0], k.{rx_to_tx_hdr_remote_ip_sbit0_ebit15...rx_to_tx_hdr_remote_ip_sbit96_ebit127}

    /* Setup root for METER Lookup on LPM2 */
    phvwr.e          p.lpm_metadata_lpm2_base_addr, k.lpm_metadata_meter_base_addr
    /* Enable Recirc */
    phvwr            p.capri_p4_intr_recirc, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
setup_recirc_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
