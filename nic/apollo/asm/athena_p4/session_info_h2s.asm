#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

#include "EGRESS_session_info_h2s_k.h"

struct session_info_h2s_k_          k;
struct session_info_h2s_d           d;
struct phv_                         p;

%%


/*
 * TODO:
 * Should the packet be validated against the same AF type
 */
session_info_h2s:
    sne             c1, d.session_info_per_direction_d.valid_flag, TRUE
    b.c1            session_info_h2s_invalid

    seq             c1, d.session_info_per_direction_d.strip_l2_header_flag, TRUE
    phvwr.c1        p.ethernet_1_valid, FALSE

    seq             c1, d.session_info_per_direction_d.strip_vlan_tag_flag, TRUE
    phvwr.c1        p.ctag_1_valid, FALSE

    seq             c1, k.ipv4_1_valid, TRUE

    add             r1, d.session_info_per_direction_d.nat_type, r0
    .brbegin

        br              r1[1:0]
        nop

        .brcase L3REWRITE_NONE
        b               session_info_h2s_user_pkt_rewrite_done
        nop

        .brcase L3REWRITE_SNAT
        phvwr           p.control_metadata_update_checksum, TRUE
        phvwr.c1        p.ipv4_1_srcAddr,\
                        d.session_info_per_direction_d.nat_address[31:0]
        b               session_info_h2s_user_pkt_rewrite_done
        phvwr.!c1       p.ipv6_1_srcAddr, d.session_info_per_direction_d.nat_address

        .brcase L3REWRITE_DNAT
        phvwr           p.control_metadata_update_checksum, TRUE
        phvwr.c1        p.ipv4_1_dstAddr,\
                        d.session_info_per_direction_d.nat_address[31:0]
        b               session_info_h2s_user_pkt_rewrite_done
        phvwr.!c1       p.ipv6_1_dstAddr, d.session_info_per_direction_d.nat_address

        .brcase 3
        /* Unused */
        b               session_info_h2s_user_pkt_rewrite_done
        nop

    .brend

session_info_h2s_user_pkt_rewrite_done:

    add             r1, d.session_info_per_direction_d.encap_type, r0
    beq             r1, r0, session_info_h2s_user_pkt_encap_done
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

    .brbegin

        br              r1[1:0]
        nop

        .brcase REWRITE_ENCAP_NONE
        b               session_info_h2s_user_pkt_encap_done
        nop

        .brcase REWRITE_ENCAP_L2
        /* Already setup */
        b               session_info_h2s_user_pkt_encap_done
        nop

        .brcase REWRITE_ENCAP_MPLSOUDP
        
        phvwr           p.{ipv4_0_version...ipv4_0_ihl}, 0x45
        phvwr           p.ipv4_0_ttl, 64
        phvwr           p.ipv4_0_protocol, IP_PROTO_UDP
        phvwr           p.{ipv4_0_srcAddr...ipv4_0_dstAddr}, \
                        d.{session_info_per_direction_d.ipv4_sa...session_info_per_direction_d.ipv4_da}

        phvwr           p.{udp_0_srcPort...udp_0_dstPort}, \
                        d.{session_info_per_direction_d.udp_sport...session_info_per_direction_d.udp_dport}
        /*
        phvwr           p.udp_0_valid, TRUE
        phvwr           p.ipv6_0_valid, FALSE
        phvwr           p.ipv4_0_valid, TRUE
        phvwr           p.ipv4_0_udp_csum, FALSE
        phvwr           p.ipv4_0_tcp_csum, FALSE
        phvwr           p.ipv4_0_csum, TRUE
        */
        phvwr           p.{udp_0_valid...ipv4_0_csum}, 0x29
        phvwr           p.capri_deparser_len_ipv4_0_hdr_len, 20

        phvwr           p.{mpls_label1_0_label_b20_b4...mpls_label1_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label1
        phvwr           p.mpls_label1_0_ttl, 64
        phvwr           p.mpls_label1_0_valid, TRUE
        add             r2, r0, 4

        sne             c1, d.session_info_per_direction_d.mpls_label2, r0
        b.!c1           session_info_h2s_mplsoudp_no_more_mpls_labels
        phvwr.!c1       p.mpls_label1_0_bos, TRUE
        phvwr.c1        p.{mpls_label2_0_label_b20_b4...mpls_label2_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label2
        phvwr.c1        p.mpls_label2_0_ttl, 64
        phvwr.c1        p.mpls_label2_0_valid, TRUE
        add.c1          r2, r2, 4

        sne             c1, d.session_info_per_direction_d.mpls_label3, r0
        b.!c1           session_info_h2s_mplsoudp_no_more_mpls_labels
        phvwr.!c1       p.mpls_label2_0_bos, TRUE
        phvwr.c1        p.{mpls_label3_0_label_b20_b4...mpls_label3_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label3
        phvwr.c1        p.mpls_label3_0_ttl, 64
        phvwr.c1        p.mpls_label3_0_valid, TRUE
        add.c1          r2, r2, 4
        phvwr.c1        p.mpls_label3_0_bos, TRUE

session_info_h2s_mplsoudp_no_more_mpls_labels:

        /* Setup IP and UDP lengths */
        add             r2, r2, (20 + 8)
        add             r1, k.p4i_to_p4e_header_packet_len, r2
        phvwr           p.ipv4_0_totalLen, r1
        sub             r2, r1, 20
        b               session_info_h2s_user_pkt_encap_done
        phvwr           p.udp_0_len, r2

        .brcase REWRITE_ENCAP_MPLSOGRE

        phvwr           p.{ipv4_0_version...ipv4_0_ihl}, 0x45
        phvwr           p.ipv4_0_ttl, 64
        phvwr           p.ipv4_0_protocol, IP_PROTO_GRE
        phvwr           p.{ipv4_0_srcAddr...ipv4_0_dstAddr}, \
                        d.{session_info_per_direction_d.ipv4_sa...session_info_per_direction_d.ipv4_da}
        phvwr           p.gre_0_proto, ETHERTYPE_MPLS_UNICAST
        /*
        phvwr           p.gre_0_valid, TRUE
        phvwr           p.udp_0_valid, FALSE
        phvwr           p.ipv6_0_valid, FALSE
        phvwr           p.ipv4_0_valid, TRUE
        phvwr           p.ipv4_0_udp_csum, FALSE
        phvwr           p.ipv4_0_tcp_csum, FALSE
        phvwr           p.ipv4_0_csum, TRUE
        */
        phvwr           p.{gre_0_valid...ipv4_0_valid}, 0x49
        phvwr           p.capri_deparser_len_ipv4_0_hdr_len, 20

        phvwr           p.{mpls_label1_0_label_b20_b4...mpls_label1_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label1
        phvwr           p.mpls_label1_0_ttl, 64
        phvwr           p.mpls_label1_0_valid, TRUE
        add             r2, r0, 4

        sne             c1, d.session_info_per_direction_d.mpls_label2, r0
        b.!c1           session_info_h2s_mplsogre_no_more_mpls_labels
        phvwr.!c1       p.mpls_label1_0_bos, TRUE
        phvwr.c1        p.{mpls_label2_0_label_b20_b4...mpls_label2_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label2
        phvwr.c1        p.mpls_label2_0_ttl, 64
        phvwr.c1        p.mpls_label2_0_valid, TRUE
        add.c1          r2, r2, 4

        sne             c1, d.session_info_per_direction_d.mpls_label3, r0
        b.!c1           session_info_h2s_mplsogre_no_more_mpls_labels
        phvwr.!c1       p.mpls_label2_0_bos, TRUE
        phvwr.c1        p.{mpls_label3_0_label_b20_b4...mpls_label3_0_label_b3_b0} ,         \
                        d.session_info_per_direction_d.mpls_label3
        phvwr.c1        p.mpls_label3_0_ttl, 64
        phvwr.c1        p.mpls_label3_0_valid, TRUE
        add.c1          r2, r2, 4
        phvwr.c1        p.mpls_label3_0_bos, TRUE

session_info_h2s_mplsogre_no_more_mpls_labels:

        /* Setup IP and UDP lengths */
        add             r2, r2, (20 + 8)
        add             r1, k.p4i_to_p4e_header_packet_len, r2
        phvwr           p.ipv4_0_totalLen, r1
        sub             r2, r1, 20
        b               session_info_h2s_user_pkt_encap_done
        phvwr           p.udp_0_len, r2

    .brend

session_info_h2s_user_pkt_encap_done:
        phvwr.e         p.control_metadata_redir_type, NACL_REDIR_UPLINK
        phvwr           p.control_metadata_redir_oport, TM_PORT_UPLINK_1

session_info_h2s_invalid:
    /* If session info is invalid, redirect packet to slow-path */
