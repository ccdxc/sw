#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/gft/include/defines.h"

struct rx_gft_hash_k k;
struct rx_gft_hash_d d;
struct phv_ p;

%%

rx_gft_hash_info:
    bbne        d.rx_gft_hash_info_d.entry_valid, TRUE, rx_gft_flow_miss
    nop
    bcf         [c1], rx_gft_flow_hit

    // check hash1 and hint1
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash1
    sne         c2, d.rx_gft_hash_info_d.hint1, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint1
    // check hash2 and hint2
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash2
    sne         c2, d.rx_gft_hash_info_d.hint2, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint2
    // check hash3 and hint3
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash3
    sne         c2, d.rx_gft_hash_info_d.hint3, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint3
    // check hash4 and hint4
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash4
    sne         c2, d.rx_gft_hash_info_d.hint4, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint4
    // check hash5 and hint5
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash5
    sne         c2, d.rx_gft_hash_info_d.hint5, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint5
    // check hash6 and hint6
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash6
    sne         c2, d.rx_gft_hash_info_d.hint6, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint6
    // check hash7 and hint7
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash7
    sne         c2, d.rx_gft_hash_info_d.hint7, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint7
    // check hash7 and hint7
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash7
    sne         c2, d.rx_gft_hash_info_d.hint7, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint7
    // check hash9 and hint9
    seq         c1, r1[31:24], d.rx_gft_hash_info_d.hash9
    sne         c2, d.rx_gft_hash_info_d.hint9, r0
    bcf         [c1&c2], rx_gft_hash_hit
    add         r2, r0, d.rx_gft_hash_info_d.hint9
rx_gft_flow_miss:
    phvwr.e     p.flow_action_metadata_flow_index, 0
    phvwr       p.capri_intrinsic_drop, 1

rx_gft_flow_hit:
    phvwr.e     p.flow_action_metadata_flow_index, \
                    d.rx_gft_hash_info_d.flow_index
    or          r2, d.rx_gft_hash_info_d.policer_index_sbit7_ebit12, \
                    d.rx_gft_hash_info_d.policer_index_sbit0_ebit6, 6
    phvwr       p.flow_action_metadata_policer_index, r2

rx_gft_hash_hit:
    // set bit 31 for overflow hash lookup to work
    ori         r2, r2, 0x80000000
    phvwr.e     p.flow_action_metadata_overflow_lkp, TRUE
    phvwr       p.flow_lkp_metadata_overflow_hash, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_gft_hash_error:
    nop.e
    nop
