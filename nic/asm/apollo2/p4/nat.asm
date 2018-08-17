#include "apollo.h"
#include "egress.h"
#include "EGRESS_p.h"

struct nat_k k;
struct nat_d d;
struct phv_ p;

%%

nat:
    phvwr       p.capri_intrinsic_tm_oport, TM_PORT_UPLINK_0
    phvwr       p.capri_txdma_intrinsic_valid, FALSE
    phvwr       p.predicate_header_valid, FALSE
    phvwr       p.txdma_to_p4e_header_valid, FALSE
    phvwr       p.apollo_i2e_metadata_valid, FALSE

    bbeq        k.nat_metadata_snat_required, TRUE, snat
    seq         c1, k.ipv4_1_valid, TRUE
    bbeq        k.apollo_i2e_metadata_dnat_required, TRUE, dnat
    seq         c1, k.ipv4_2_valid, TRUE
    nop.e
    nop

snat:
    phvwr.c1.e  p.ipv4_1_srcAddr, d.nat_d.nat_ip
    seq.e       c2, k.ipv6_1_valid, TRUE
    phvwr.c2    p.ipv6_1_srcAddr, d.nat_d.nat_ip

dnat:
    phvwr.c1.e  p.ipv4_2_dstAddr, d.nat_d.nat_ip
    seq.e       c2, k.ipv6_2_valid, TRUE
    phvwr.c2    p.ipv6_2_dstAddr, d.nat_d.nat_ip


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nat_error:
    nop.e
    nop
