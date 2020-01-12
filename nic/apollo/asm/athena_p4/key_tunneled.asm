#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_key_tunneled_k.h"

struct key_tunneled_k_  k;
struct key_tunneled_d   d;
struct phv_             p;

%%

nop:
    nop.e
    nop

.align
tunneled_ipv4_packet:
    phvwr           p.key_metadata_ingress_port, k.control_metadata_direction
    phvwr           p.key_metadata_tcp_flags, k.tcp_flags
    phvwr           p.key_metadata_ktype, KEY_TYPE_IPV4

    seq             c1, k.mpls_label1_1_valid, TRUE
    add.c1          r1, k.mpls_label1_1_label, r0
    seq             c1, k.mpls_label2_1_valid, TRUE
    add.c1          r1, r1, k.mpls_label2_1_label, 20
    phvwr           p.key_metadata_tenant_id, r1
    
    phvwr           p.key_metadata_src, k.ipv4_2_srcAddr
    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_2_dstPort
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.tcp_srcPort
    phvwr.c1        p.key_metadata_dport, k.tcp_dstPort
    phvwr.e         p.key_metadata_dst, k.ipv4_2_dstAddr
    phvwr.f         p.key_metadata_proto, k.ipv4_2_protocol


.align
tunneled_ipv6_packet:
    phvwr           p.key_metadata_ingress_port, k.control_metadata_direction

    seq             c1, k.mpls_label1_1_valid, TRUE
    add.c1          r1, k.mpls_label1_1_label, r0
    seq             c1, k.mpls_label2_1_valid, TRUE
    add.c1          r1, r1, k.mpls_label2_1_label, 20
    phvwr           p.key_metadata_tenant_id, r1

    phvwr           p.key_metadata_tcp_flags, k.tcp_flags
    phvwr           p.key_metadata_src, \
                        k.ipv6_2_srcAddr
    phvwr           p.key_metadata_dst[127:120], k.ipv6_2_dstAddr_s120_e127
    phvwr           p.key_metadata_dst[119:0], k.ipv6_2_dstAddr_s0_e119
    seq             c1, k.udp_2_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.udp_2_srcPort
    phvwr.c1        p.key_metadata_dport, k.udp_2_dstPort
    seq             c1, k.tcp_valid, TRUE
    phvwr.c1        p.key_metadata_sport, k.tcp_srcPort
    phvwr.c1        p.key_metadata_dport, k.tcp_dstPort
    phvwr.e         p.key_metadata_proto, k.ipv6_2_nextHdr
    phvwr.f         p.key_metadata_ktype, KEY_TYPE_IPV6

.align
tunneled_nonip_packet:
    phvwr.e.f       p.control_metadata_skip_flow_lkp, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
key_tunneled_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
