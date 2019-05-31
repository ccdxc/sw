#include "../../p4/include/lpm_defines.h"
#include "../../p4/include/artemis_sacl_defines.h"

#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct rxlpm2_2_k          k;
struct rxlpm2_2_d          d;

// Define Table Name and Action Names
#define table_name         rxlpm2_2
#define action_keys32b     match2_2_32b
#define action_keys128b    match2_2_128b
#define action_data16b     match2_2_16b_retrieve
#define action_data32b     match2_2_32b_retrieve

// Define table field names for the selected actions
#define keys16b(a)         d.u.match2_2_16b_d.key ## a
#define keys32b(a)         d.u.match2_2_32b_d.key ## a
#define keys128bhi(a)      d.u.match2_2_128b_d.key ## a[127:64]
#define keys128blo(a)      d.u.match2_2_128b_d.key ## a[63:0]
#define keys16b_(a)        d.u.match2_2_16b_retrieve_d.key ## a
#define data16b(a)         d.u.match2_2_16b_retrieve_d.data ## a
#define keys32b_(a)        d.u.match2_2_32b_retrieve_d.key ## a
#define data32b(a)         d.u.match2_2_32b_retrieve_d.data ## a

// Define key field names
#define key                k.lpm_metadata_lpm2_key
#define keylo              k.lpm_metadata_lpm2_key[63:0]
#define keyhi              k.lpm_metadata_lpm2_key[127:64]
#define base_addr          k.lpm_metadata_lpm2_base_addr
#define curr_addr          k.lpm_metadata_lpm2_next_addr

// Define PHV field names
#define next_addr          p.lpm_metadata_lpm2_next_addr

// Define result register and handler function names
#define res_reg            r7
#define result_handler16b  nop_res_handler
#define result_handler32b  rxlpm2_res_handler

%%

#include "../include/lpm.h"

nop_res_handler:
    nop.e
    nop

rxlpm2_res_handler:
    /* Is this the first pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 0
    /* If so, go to pass0 */
    bcf              [c1], pass0
    /* Is this the second pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 1
    /* If so, write the DIP classid result, and stop */
    phvwr.c1.e       p.lpm_metadata_dip_classid, res_reg
    /* Is this the third pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 2
    /* If so, write the METER classid result, and stop */
    phvwr.c1         p.lpm_metadata_meter_result, res_reg
    /* Else, Stop anyway */
    nop.e
    nop

pass0:
    /* Write the DPORT classid */
    phvwr            p.lpm_metadata_dport_classid, res_reg
    /* Setup key for SPORT Lookup on LPM2 */
    phvwr            p.lpm_metadata_lpm2_key, k.p4_to_rxdma_flow_sport
    /* Setup root for SPORT Lookup on LPM2 */
    add.e            r1, k.lpm_metadata_sacl_base_addr, SACL_SPORT_TABLE_OFFSET
    phvwr            p.lpm_metadata_lpm2_base_addr, r1
