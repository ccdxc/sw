#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_session_info_rewrite_k.h"

struct session_info_rewrite_k_      k;
struct session_info_rewrite_d       d;
struct phv_     p;

%%

session_info_rewrite:

    seq             c1, k.ipv4_1_valid, TRUE
    add             r1, d.session_info_rewrite_d.user_pkt_rewrite_type, r0

    .brbegin

        br              r1[1:0]
        nop

        .brcase L3REWRITE_NONE
        b               session_info_pkt_rewrite_done
        nop

        .brcase L3REWRITE_SNAT
        phvwr.c1        p.ipv4_1_srcAddr,\
                        d.session_info_rewrite_d.user_pkt_rewrite_ip[31:0]
        b               session_info_pkt_rewrite_done
        phvwr.!c1       p.ipv6_1_srcAddr, d.session_info_rewrite_d.user_pkt_rewrite_ip

        .brcase L3REWRITE_DNAT
        phvwr.c1        p.ipv4_1_dstAddr,\
                        d.session_info_rewrite_d.user_pkt_rewrite_ip[31:0]
        b               session_info_pkt_rewrite_done
        phvwr.!c1       p.ipv6_1_dstAddr, d.session_info_rewrite_d.user_pkt_rewrite_ip

        .brcase 3
        /* Unused */

    .brend

session_info_pkt_rewrite_done:

    add             r1, d.session_info_rewrite_d.encap_type, r0
    beq             r1, r0, session_info_pkt_encap_done
    nop

    phvwr           p.{ethernet_0_dstAddr...ethernet_0_srcAddr}, \
                    d.{session_info_rewrite_d.dmac...session_info_rewrite_d.smac}
    phvwr           p.ethernet_0_valid, TRUE


    /* TODO: Assumes no CTAG if VLAN ID == 0, do we need a flag instead? */
    sne             c1, d.session_info_rewrite_d.vlan, r0
    phvwr.c1        p.ctag_0_vid, d.session_info_rewrite_d.vlan
    phvwr.c1        p.ctag_0_etherType, ETHERTYPE_IPV4
    phvwr.c1        p.ctag_0_valid, TRUE
    phvwr.c1        p.ethernet_0_etherType, ETHERTYPE_VLAN
    phvwr.!c1       p.ethernet_0_etherType, ETHERTYPE_IPV4
    

    .brbegin

        br              r1[1:0]
        nop

        .brcase REWRITE_ENCAP_NONE
        b               session_info_pkt_encap_done
        nop

        .brcase REWRITE_ENCAP_L2
        /* Already setup */
        b               session_info_pkt_encap_done
        nop

        .brcase REWRITE_ENCAP_MPLSOUDP
        phvwr           p.{ipv4_0_version...ipv4_0_ihl}, 0x45
        //phvwr           p.ipv4_0_totalLen,  TODO
        phvwr           p.ipv4_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr           p.ipv4_0_protocol, IP_PROTO_UDP
        phvwr           p.{ipv4_0_srcAddr...ipv4_0_dstAddr}, \
                        d.{session_info_rewrite_d.ip_saddr...session_info_rewrite_d.ip_daddr}

        phvwr           p.{udp_0_srcPort...udp_0_dstPort}, \
                        d.{session_info_rewrite_d.udp_sport...session_info_rewrite_d.udp_dport}
        //phvwr           p.udp_0_len, TODO
        phvwr           p.{udp_0_valid...ipv4_0_valid}, 0x5

        phvwr           p.mpls_label1_0_label, d.session_info_rewrite_d.mpls1_label
        /* TODO: Confirm TTL value */
        phvwr           p.mpls_label1_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr           p.mpls_label1_0_valid, TRUE

        sne             c1, d.session_info_rewrite_d.mpls2_label, r0
        phvwr.!c1       p.mpls_label1_0_bos, TRUE
        phvwr.c1        p.mpls_label2_0_label, d.session_info_rewrite_d.mpls1_label
        /* TODO: Confirm TTL value */
        phvwr.c1        p.mpls_label2_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr.c1        p.mpls_label2_0_bos, TRUE
        b               session_info_pkt_encap_done
        phvwr.c1        p.mpls_label2_0_valid, TRUE

        .brcase REWRITE_ENCAP_MPLSOGRE
        phvwr           p.{ipv4_0_version...ipv4_0_ihl}, 0x45
        //phvwr           p.ipv4_0_totalLen,  TODO
        phvwr           p.ipv4_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr           p.ipv4_0_protocol, IP_PROTO_GRE
        phvwr           p.{ipv4_0_srcAddr...ipv4_0_dstAddr}, \
                        d.{session_info_rewrite_d.ip_saddr...session_info_rewrite_d.ip_daddr}
        phvwr           p.gre_0_proto, ETHERTYPE_MPLS_UNICAST
        phvwr           p.{gre_0_valid...ipv4_0_valid}, 0x9

        phvwr           p.mpls_label1_0_label, d.session_info_rewrite_d.mpls1_label
        /* TODO: Confirm TTL value */
        phvwr           p.mpls_label1_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr           p.mpls_label1_0_valid, TRUE

        sne             c1, d.session_info_rewrite_d.mpls2_label, r0
        phvwr.!c1       p.mpls_label1_0_bos, TRUE
        phvwr.c1        p.mpls_label2_0_label, d.session_info_rewrite_d.mpls1_label
        /* TODO: Confirm TTL value */
        phvwr.c1        p.mpls_label2_0_ttl, d.session_info_rewrite_d.ip_ttl
        phvwr.c1        p.mpls_label2_0_bos, TRUE
        b               session_info_pkt_encap_done
        phvwr.c1        p.mpls_label2_0_valid, TRUE

    .brend

session_info_pkt_encap_done:
        nop.e
        nop
