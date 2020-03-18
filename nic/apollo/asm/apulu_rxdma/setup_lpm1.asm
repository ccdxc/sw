#include "../../p4/include/apulu_sacl_defines.h"

#include "nic/apollo/asm/include/apulu.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_setup_lpm1_k.h"

struct phv_             p;
struct setup_lpm1_k_    k;

%%

setup_lpm1:
    // Load the correct sacl_base_address into r1 based on recirc count
    seq              c1, k.lpm_metadata_recirc_count, 1
    bcf              [c1], setup_lpm_dport_sport
    seq              c1, k.lpm_metadata_recirc_count, 3
    bcf              [c1], setup_lpm_dport_sport
    seq              c1, k.lpm_metadata_recirc_count, 5
    bcf              [c1], setup_lpm_dport_sport
    seq              c1, k.lpm_metadata_recirc_count, 7
    bcf              [c1], setup_lpm_dport_sport
    seq              c1, k.lpm_metadata_recirc_count, 9
    bcf              [c1], setup_lpm_dport_sport
    slt              c1, k.lpm_metadata_recirc_count, 11
    phvwr.!c1        p.capri_p4_intr_recirc, FALSE
    nop.e
    nop

setup_lpm_dport_sport:
    // if sacl_base_address == NULL, stop!
    add              r1, r0, k.lpm_metadata_sacl_base_addr
    seq              c1, r1, r0
    phvwr.c1.e       p.capri_p4_intr_recirc, FALSE

    // Setup root for SPORT lookup on LPM1
    addi             r2, r1, SACL_SPORT_TABLE_OFFSET
    phvwr            p.lpm_metadata_lpm1_base_addr, r2

    // Setup key for SPORT lookup on LPM1
    phvwr            p.lpm_metadata_lpm1_key, k.p4_to_rxdma_flow_sport

    // Setup root for DPORT lookup on LPM2
    addi             r2, r1, SACL_PROTO_DPORT_TABLE_OFFSET
    phvwr            p.lpm_metadata_lpm2_base_addr, r2

    // Setup key for DPORT lookup on LPM2
    phvwr            p.lpm_metadata_lpm2_key[15:0], k.p4_to_rxdma_flow_dport
    phvwr            p.lpm_metadata_lpm2_key[31:16], k.p4_to_rxdma_flow_proto

    // Enable both LPMs and recirc again
    phvwr            p.p4_to_rxdma_lpm1_enable, TRUE
    phvwr.e          p.p4_to_rxdma_lpm2_enable, TRUE
    phvwr            p.capri_p4_intr_recirc, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
setup_lpm1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
