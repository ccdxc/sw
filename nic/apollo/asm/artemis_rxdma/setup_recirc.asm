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
    /* If not, go to not_0 */
    bcf              [!c1], not_0
    /* First Pass. Is direction == TX_FROM_HOST? */
    sne              c1, k.p4_to_rxdma_direction, TX_FROM_HOST
    /* If so, reset LPM2 key to SIP */
    phvwr.c1         p.lpm_metadata_lpm2_key, k.lpm_metadata_lpm1_key
    /* Reset LPM2 base address to METER root */
    phvwr.e          p.lpm_metadata_lpm2_base_addr, k.{lpm_metadata_meter_base_addr_sbit0_ebit1...\
                                                       lpm_metadata_meter_base_addr_sbit2_ebit33}
    /* Set Recirc bit */
    phvwr            p.capri_p4_intr_recirc, 1

not_0:
    /* Is this the second pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 1
    /* If not, go to not_0_or_1 */
    bcf              [!c1], not_0_or_1
    /* Second Pass. Create Proto+DPort Key = ((Proto<<16)|(DPort)) */
    or               r1, k.p4_to_rxdma_flow_dport, k.p4_to_rxdma_flow_proto, 16
    /* Reset LPM2 key to Proto_DPort */
    phvwr            p.lpm_metadata_lpm2_key, r1
    /* Reset LPM2 base address to DPORT root */
    add              r1, r0, k.{lpm_metadata_lpm2_base_addr_sbit0_ebit1...\
                                lpm_metadata_lpm2_base_addr_sbit2_ebit33}
    add              r1, r1, SACL_PROTO_DPORT_TABLE_OFFSET
    phvwr.e          p.lpm_metadata_lpm2_base_addr, r1
    /* Set Recirc bit */
    phvwr            p.capri_p4_intr_recirc, 1

not_0_or_1:
    /* Nothing to do. Quit! */
    nop.e
    nop