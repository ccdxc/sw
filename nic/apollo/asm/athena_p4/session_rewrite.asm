#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

#include "EGRESS_session_rewrite_k.h"

struct session_rewrite_k_           k;
struct session_rewrite_d            d;
struct phv_                         p;



#define D(_field)   d.u._ACTION_.##_field
#define D_R(_field1, _field2)   d.{u._ACTION_.##_field1...u._ACTION_.##_field2}


#define SESSION_REWRITE_COMMON()                                \
    sne             c1, D(valid_flag), TRUE;                    \
    b.c1            session_rewrite_invalid;                    \
    nop;                                                        \
                                                                \
    seq             c1, D(strip_l2_header_flag), TRUE;          \
    phvwr.c1        p.ethernet_1_valid, FALSE;                  \
                                                                \
    seq             c1, D(strip_vlan_tag_flag), TRUE;           \
    phvwr.c1        p.ctag_1_valid, FALSE;                      \
                                                                \
    seq             c1, D(strip_outer_encap_flag), TRUE;        \
    /* TODO: Include Geneve fields when supported */            \
    /* NOTE: No sanity check done here for tunnel encap and     \
         assumes that the control-plane                         \
        knows what it is doing.                                 \
    */                                                          \
                                                                \
    /*                                                          \
    phvwr.c1        p.mpls_label3_1_valid, FALSE                \
    phvwr.c1        p.mpls_dst_valid, FALSE                     \
    phvwr.c1        p.mpls_src_valid, FALSE                     \
    phvwr.c1        p.udp_1_valid, FALSE                        \
    phvwr.c1        p.udp_1_csum, FALSE                         \
    phvwr.c1        p.ipv4_1_valid, FALSE                       \
    phvwr.c1        p.ipv4_1_udp_csum, FALSE                    \
    phvwr.c1        p.ipv4_1_tcp_csum, FALSE                    \
    phvwr.c1        p.ipv4_1_csum, FALSE                        \
    phvwr.c1        p.gre_1_valid, FALSE                        \
    phvwr.c1        p.ipv6_1_valid, FALSE                       \
    phvwr.c1        p.ipv6_1_icmp_csum, FALSE                   \
    phvwr.c1        p.ipv6_1_udp_csum, FALSE                    \
    phvwr.c1        p.ipv6_1_tcp_csum, FALSE                    \
    */                                                          \
    phvwr.c1        p.{mpls_label3_1_valid...ipv6_1_tcp_csum}, 0;
    


%%


session_rewrite:
#undef _ACTION_
#define _ACTION_    session_rewrite_d
    SESSION_REWRITE_COMMON();
    nop.e
    nop


.align
session_rewrite_ipv4_snat:
#undef _ACTION_
#define _ACTION_    session_rewrite_ipv4_snat_d
    SESSION_REWRITE_COMMON();

    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.ipv4_2_srcAddr, D(ipv4_addr_snat)
    phvwr.!c1       p.ipv4_1_srcAddr, D(ipv4_addr_snat)
    nop.e
    nop


.align
session_rewrite_ipv4_dnat:
#undef _ACTION_
#define _ACTION_    session_rewrite_ipv4_dnat_d
    SESSION_REWRITE_COMMON();

    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.ipv4_2_dstAddr, D(ipv4_addr_dnat)
    phvwr.!c1       p.ipv4_1_dstAddr, D(ipv4_addr_dnat)
    nop.e
    nop


.align
session_rewrite_ipv4_pat:
#undef _ACTION_
#define _ACTION_    session_rewrite_ipv4_pat_d
    SESSION_REWRITE_COMMON();

    /* NOTE: Assumes PAT requirement for TCP (iSCSI) alone for now */
    sne             c2, D(l4_port_spat), r0
    phvwr.c2        p.tcp_srcPort, D(l4_port_spat)

    sne             c2, D(l4_port_dpat), r0
    phvwr.c2        p.tcp_dstPort, D(l4_port_dpat)

    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    b.c1            session_rewrite_ipv4_pat_s2h

session_rewrite_ipv4_pat_h2s:

    sne             c2, D(ipv4_addr_spat), r0
    phvwr.c2        p.ipv4_1_srcAddr, D(ipv4_addr_spat)

    sne             c2, D(ipv4_addr_dpat), r0
    phvwr.c2        p.ipv4_1_dstAddr, D(ipv4_addr_dpat)
    
    nop.e
    nop

session_rewrite_ipv4_pat_s2h:

    phvwr.c2        p.ipv4_2_srcAddr, D(ipv4_addr_spat)

    sne             c2, D(ipv4_addr_dpat), r0
    phvwr.c2        p.ipv4_2_dstAddr, D(ipv4_addr_dpat)

    nop.e
    nop


.align
session_rewrite_ipv6_snat:
#undef _ACTION_
#define _ACTION_    session_rewrite_ipv6_snat_d
    SESSION_REWRITE_COMMON();

    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.ipv6_2_srcAddr, D(ipv6_addr_snat)
    phvwr.!c1       p.ipv6_1_srcAddr, D(ipv6_addr_snat)
    nop.e
    nop


.align
session_rewrite_ipv6_dnat:
#undef _ACTION_
#define _ACTION_    session_rewrite_ipv6_dnat_d
    SESSION_REWRITE_COMMON();

    seq             c1, k.control_metadata_direction, RX_FROM_SWITCH
    phvwr.c1        p.ipv6_2_dstAddr, D(ipv6_addr_dnat)
    phvwr.!c1       p.ipv6_1_dstAddr, D(ipv6_addr_dnat)
    nop.e
    nop


.align
session_rewrite_invalid:
    phvwr.e         p.control_metadata_flow_miss, TRUE
    nop
