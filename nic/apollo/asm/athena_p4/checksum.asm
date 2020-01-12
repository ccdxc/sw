#include "athena.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_checksum_k.h"

struct checksum_k_  k;
struct phv_         p;

%%

update_ipv4_checksum:
    add             r1, r0, k.ipv4_1_ihl, 2
    phvwr.e         p.capri_deparser_len_ipv4_1_hdr_len, r1
    phvwr           p.ipv4_1_csum, TRUE

.align
update_ipv4_udp_checksum:
    add             r1, r0, k.ipv4_1_ihl, 2
    phvwr           p.capri_deparser_len_ipv4_1_hdr_len, r1
    phvwr           p.capri_deparser_len_l4_payload_len, k.udp_1_len
    phvwrpair.e     p.udp_1_csum, TRUE, p.ipv4_1_udp_csum, TRUE
    phvwr           p.ipv4_1_csum, TRUE

.align
update_ipv4_tcp_checksum:
    add             r1, r0, k.ipv4_1_ihl, 2
    phvwr           p.capri_deparser_len_ipv4_1_hdr_len, r1
    sub             r1, k.ipv4_1_totalLen, r1
    phvwr           p.capri_deparser_len_l4_payload_len, r1
    phvwrpair.e     p.tcp_csum, TRUE, p.ipv4_1_tcp_csum, TRUE
    phvwr           p.ipv4_1_csum, TRUE

.align
update_ipv4_icmp_checksum:
    add             r1, r0, k.ipv4_1_ihl, 2
    phvwr           p.capri_deparser_len_ipv4_1_hdr_len, r1
    sub             r1, k.ipv4_1_totalLen, r1
    phvwr           p.capri_deparser_len_l4_payload_len, r1
    phvwr.e         p.icmp_csum, TRUE
    phvwr           p.ipv4_1_csum, TRUE

.align
update_ipv6_udp_checksum:
    phvwr.e         p.capri_deparser_len_l4_payload_len, k.udp_1_len
    phvwrpair       p.udp_1_csum, TRUE, p.ipv6_1_udp_csum, TRUE

.align
update_ipv6_tcp_checksum:
    add             r1, r0, k.ipv6_1_payloadLen
    phvwr.e         p.capri_deparser_len_l4_payload_len, r1
    phvwrpair       p.tcp_csum, TRUE, p.ipv6_1_tcp_csum, TRUE

.align
update_ipv6_icmp_checksum:
    add             r1, r0, k.ipv6_1_payloadLen
    phvwr.e         p.capri_deparser_len_l4_payload_len, r1
    phvwrpair       p.icmp_csum, TRUE, p.ipv6_1_icmp_csum, TRUE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
checksum_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
