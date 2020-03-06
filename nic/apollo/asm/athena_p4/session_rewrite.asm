#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

#include "EGRESS_session_rewrite_k.h"

struct session_rewrite_k_           k;
struct session_rewrite_d            d;
struct phv_                         p;



#define D(_field)   d.u._ACTION_.##_field
#define D_R(_field1, _field2)   d.{u._ACTION_.##_field1...u._ACTION_.##_field2}


%%


session_rewrite:
#undef _ACTION_
#define _ACTION_    session_rewrite_d
    sne             c1, D(valid_flag), TRUE
    b.c1            session_rewrite_invalid
    nop

    seq             c2, k.control_metadata_direction, TX_FROM_HOST

    seq             c1, D(strip_outer_encap_flag), TRUE
    /*
    phvwr.c1        p.mpls_label3_1_valid, FALSE
    phvwr.c1        p.mpls_dst_valid, FALSE
    phvwr.c1        p.mpls_src_valid, FALSE
    phvwr.c1        p.udp_1_valid, FALSE
    phvwr.c1        p.udp_1_csum, FALSE
    phvwr.c1        p.ipv4_1_valid, FALSE
    phvwr.c1        p.ipv4_1_udp_csum, FALSE
    phvwr.c1        p.ipv4_1_tcp_csum, FALSE
    phvwr.c1        p.ipv4_1_csum, FALSE
    phvwr.c1        p.gre_1_valid, FALSE
    phvwr.c1        p.ipv6_1_valid, FALSE
    phvwr.c1        p.ipv6_1_icmp_csum, FALSE
    phvwr.c1        p.ipv6_1_udp_csum, FALSE
    phvwr.c1        p.ipv6_1_tcp_csum, FALSE
    phvwr.c1        p.ctag_1_valid, FALSE
    phvwr.c1        p.ethernet_1_valid, FALSE
    */
    phvwr.c1        p.{mpls_label3_1_valid...ethernet_1_valid}, 0
#if 0    
    phvwr           p.{ethernet_0_dstAddr...ethernet_0_srcAddr},    \
                    D_R(ipv4_da, ipv4_sa)

    seq             c1, D(add_vlan_tag_flag), TRUE
    phvwr.c1        p.ctag_0_vid, D(vlan)
    phvwr.c1        p.ctag_0_etherType, ETHERTYPE_IPV4
    phvwr.c1        p.ctag_0_valid, TRUE
    phvwr.c1        p.ethernet_0_etherType, ETHERTYPE_VLAN
    phvwr.!c1       p.ethernet_0_etherType, ETHERTYPE_IPV4
#endif

    
    
    



.align
session_rewrite_ipv4_snat:




.align
session_rewrite_ipv4_dnat:




.align
session_rewrite_ipv4_pat:





.align
session_rewrite_ipv6_snat:




.align
session_rewrite_ipv6_dnat:



.align
session_rewrite_invalid:
    phvwr           p.control_metadata_flow_miss, TRUE
