#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

#if 0
#include "EGRESS_session_info_s2h_k.h"

struct session_info_s2h_k_          k;
struct session_info_s2h_d           d;
struct phv_                         p;

%%

/*
 * TODO:
 * Should the packet be validated against the same AF type
 */
session_info_s2h:
    sne             c1, d.session_info_per_direction_d.valid_flag, TRUE
    b.c1            session_info_s2h_invalid

    seq             c1, d.session_info_per_direction_d.strip_outer_encap_flag, TRUE
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

    seq             c1, k.ipv4_2_valid, TRUE

    add             r1, d.session_info_per_direction_d.nat_type, r0
    .brbegin

        br              r1[1:0]
        nop

        .brcase L3REWRITE_NONE
        b               session_info_s2h_user_pkt_rewrite_done
        nop

        .brcase L3REWRITE_SNAT
        phvwr           p.control_metadata_update_checksum, TRUE
        phvwr.c1        p.ipv4_2_srcAddr,\
                        d.session_info_per_direction_d.nat_address[31:0]
        b               session_info_s2h_user_pkt_rewrite_done
        phvwr.!c1       p.ipv6_2_srcAddr, d.session_info_per_direction_d.nat_address

        .brcase L3REWRITE_DNAT
        phvwr           p.control_metadata_update_checksum, TRUE
        phvwr.c1        p.ipv4_2_dstAddr,\
                        d.session_info_per_direction_d.nat_address[31:0]
        b               session_info_s2h_user_pkt_rewrite_done
        phvwr.!c1       p.ipv6_2_dstAddr, d.session_info_per_direction_d.nat_address

        .brcase 3
        /* Unused */
        b               session_info_s2h_user_pkt_rewrite_done
        nop

    .brend

session_info_s2h_user_pkt_rewrite_done:

    add             r1, d.session_info_per_direction_d.encap_type, r0
    beq             r1, r0, session_info_s2h_user_pkt_encap_done
    nop

    phvwr           p.{ethernet_0_dstAddr...ethernet_0_srcAddr}, \
                    d.{session_info_per_direction_d.dmac...session_info_per_direction_d.smac}
    phvwr           p.ethernet_0_valid, TRUE

    seq             c1, d.session_info_per_direction_d.add_vlan_tag_flag, TRUE
    phvwr.c1        p.ctag_0_vid, d.session_info_per_direction_d.vlan
    phvwr.c1        p.ctag_0_etherType, ETHERTYPE_IPV4
    phvwr.c1        p.ctag_0_valid, TRUE
    phvwr.c1        p.ethernet_0_etherType, ETHERTYPE_VLAN
    phvwr.!c1       p.ethernet_0_etherType, ETHERTYPE_IPV4

session_info_s2h_user_pkt_encap_done:
        phvwr.e         p.control_metadata_redir_type, NACL_REDIR_UPLINK
        phvwr           p.control_metadata_redir_oport, TM_PORT_UPLINK_0

session_info_s2h_invalid:
#else

%%
session_info_s2h:
    nop.e
    nop

#endif
