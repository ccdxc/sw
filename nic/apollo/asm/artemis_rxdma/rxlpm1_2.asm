#include "../../p4/include/lpm_defines.h"

#include "artemis_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                p;
struct rxlpm1_2_k          k;
struct rxlpm1_2_d          d;

// Define Table Name and Action Names
#define table_name         rxlpm1_2
#define action_keys32b     match1_2_32b
#define action_keys128b    match1_2_128b
#define action_data32b     match1_2_32b_retrieve

// Define table field names for the selected actions
#define keys32b(a)         d.u.match1_2_32b_d.key ## a
#define keys128bhi(a)      d.u.match1_2_128b_d.key ## a[127:64]
#define keys128blo(a)      d.u.match1_2_128b_d.key ## a[63:0]
#define keys32b_(a)        d.u.match1_2_32b_retrieve_d.key ## a
#define data32b(a)         d.u.match1_2_32b_retrieve_d.data ## a

// Define key field names
#define key                k.lpm_metadata_lpm1_key
#define keylo              k.lpm_metadata_lpm1_key[63:0]
#define keyhi              k.lpm_metadata_lpm1_key[127:64]
#define base_addr          k.{lpm_metadata_lpm1_base_addr_sbit0_ebit1...\
                              lpm_metadata_lpm1_base_addr_sbit2_ebit33}
#define curr_addr          k.{lpm_metadata_lpm1_next_addr_sbit0_ebit1...\
                              lpm_metadata_lpm1_next_addr_sbit2_ebit33}

// Define PHV field names
#define next_addr          p.lpm_metadata_lpm1_next_addr

// Define result register and handler function name
#define res_reg            r7
#define result_handler32b  rxlpm1_res_handler

%%

#include "../include/lpm.h"

rxlpm1_res_handler:
    /* Is this the first pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 0
    /* If not, go to not_0 */
    bcf              [!c1], not_0
    /* First Pass. Write SIP classid result */
    phvwr            p.lpm_metadata_sip_classid, res_reg
    /* Is direction == TX_FROM_HOST */
    seq              c1, k.p4_to_rxdma_direction, TX_FROM_HOST
    /* If so, reset LPM1 key to DIP */
    phvwr.c1         p.lpm_metadata_lpm1_key, k.lpm_metadata_lpm2_key
    /* Reset LPM1 base address to TAG root */
    add              r1, r0, k.{lpm_metadata_tag_base_addr_sbit0_ebit1...\
                                lpm_metadata_tag_base_addr_sbit2_ebit9}, 24
    add.e            r1, r1, k.lpm_metadata_tag_base_addr_sbit10_ebit33
    phvwr            p.lpm_metadata_lpm1_base_addr, r1

not_0:
    /* Is this the second pass? */
    seq              c1, k.capri_p4_intr_recirc_count, 1
    /* If so, write the TAG classid result */
    phvwr.c1         p.lpm_metadata_tag_classid, res_reg
    nop.e
    nop
