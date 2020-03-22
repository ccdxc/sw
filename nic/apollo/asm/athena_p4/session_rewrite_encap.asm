#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"

#include "EGRESS_session_rewrite_encap_k.h"

struct session_rewrite_encap_k_     k;
struct session_rewrite_encap_d      d;
struct phv_                         p;

#define D(_field)   d.u._ACTION_.##_field
#define D_R(_field1, _field2)   d.{u._ACTION_.##_field1...u._ACTION_.##_field2}


#define SESSION_REWRITE_ENCAP_COMMON()                                  \
        sne             c1, D(valid_flag), TRUE;                        \
        b.c1            session_rewrite_encap_invalid;

#define SESSION_REWRITE_ENCAP_L2(_ethertype)                            \
        phvwr           p.{ethernet_0_dstAddr...ethernet_0_srcAddr},    \
                        D_R(dmac, smac);                                \
        phvwr           p.ethernet_0_valid, TRUE;                       \
                                                                        \
        seq             c1, D(add_vlan_tag_flag), TRUE;                 \
        phvwr.c1        p.ctag_0_vid, D(vlan);                          \
        phvwr.c1        p.ctag_0_etherType, _ethertype;                 \
        phvwr.c1        p.ctag_0_valid, TRUE;                           \
        phvwr.c1        p.ethernet_0_etherType, ETHERTYPE_VLAN;         \
        phvwr.!c1       p.ethernet_0_etherType, _ethertype;
    
#define SESSION_REWRITE_ENCAP_IP(_ip_proto_, _tot_len_)                 \
        phvwrpair       p.{ipv4_0_version...ipv4_0_ihl}, 0x45,          \
                        p.ipv4_0_totalLen, _tot_len_;                   \
        phvwrpair       p.ipv4_0_ttl, 64,                               \
                        p.ipv4_0_protocol, _ip_proto_;                  \
        phvwr           p.{ipv4_0_srcAddr...ipv4_0_dstAddr},            \
                        D_R(ipv4_sa, ipv4_da);                          \
        phvwr           p.capri_deparser_len_ipv4_0_hdr_len, 20;


#define SESSION_REWRITE_ENCAP_UDP(_sport_, _dport_, _len_)              \
        phvwr           p.udp_0_srcPort, _sport_;                       \
        phvwr           p.udp_0_dstPort, _dport_;                       \
        phvwr           p.udp_0_len, _len_;

%%

session_rewrite_encap_l2:
#undef _ACTION_
#define _ACTION_    session_rewrite_encap_l2_d
        SESSION_REWRITE_ENCAP_COMMON();
        /*
         * For now asssume L2 encap to be always towards Host
         * and for the packet that was received encapped
         * i.e. the user packet is at Layer 2
         */
        seq             c1, k.ipv4_2_valid, TRUE
        add.c1          r1, r0, ETHERTYPE_IPV4
        seq             c1, k.ipv6_2_valid, TRUE
        add.c1          r1, r0, ETHERTYPE_IPV6
        SESSION_REWRITE_ENCAP_L2(r1);
        nop.e
        nop


.align
session_rewrite_encap_mplsoudp:
#undef _ACTION_
#define _ACTION_    session_rewrite_encap_mplsoudp_d
        SESSION_REWRITE_ENCAP_COMMON();
        SESSION_REWRITE_ENCAP_L2(ETHERTYPE_IPV4);

        phvwrpair       p.{mpls_label1_0_label_b20_b4...mpls_label1_0_label_b3_b0}, \
                        D(mpls_label1),                                             \
                        p.mpls_label1_0_ttl, 64
        phvwr           p.mpls_label1_0_valid, TRUE
        add             r2, r0, 4

        sne             c1, D(mpls_label2), r0
        b.!c1           session_rewrite_encap_mplsoudp_no_more_mpls_labels
        phvwr.!c1       p.mpls_label1_0_bos, TRUE
        phvwrpair.c1    p.{mpls_label2_0_label_b20_b4...mpls_label2_0_label_b3_b0}, \
                        D(mpls_label2),                                             \
                        p.mpls_label2_0_ttl, 64
        phvwr.c1        p.mpls_label2_0_valid, TRUE
        add.c1          r2, r2, 4

        sne             c1, D(mpls_label3), r0
        b.!c1           session_rewrite_encap_mplsoudp_no_more_mpls_labels
        phvwr.!c1       p.mpls_label2_0_bos, TRUE
        phvwrpair.c1    p.{mpls_label3_0_label_b20_b4...mpls_label3_0_label_b3_b0}, \
                        D(mpls_label3),                                             \
                        p.mpls_label3_0_ttl, 64
        phvwr.c1        p.mpls_label3_0_valid, TRUE
        add.c1          r2, r2, 4
        phvwr.c1        p.mpls_label3_0_bos, TRUE

session_rewrite_encap_mplsoudp_no_more_mpls_labels:

        /* Adjust for incoming L2 header that will be stipped */
        /* FIXME: Include all rewrites from a single table */
        seq             c1, k.ctag_1_valid, TRUE
        sub.c1          r3, k.p4i_to_p4e_header_packet_len, 18
        sub.!c1         r3, k.p4i_to_p4e_header_packet_len, 14

        /* Setup UDP length in r1 */
        add             r2, r2, 8
        add             r1, r3, r2

        /* Setup IP length in r2 */
        add             r2, r1, 20

        SESSION_REWRITE_ENCAP_UDP(k.p4i_to_p4e_header_flow_hash[15:0], 6635, r1);
        SESSION_REWRITE_ENCAP_IP(IP_PROTO_UDP, r2);

        /*
        phvwr           p.udp_0_valid, TRUE
        phvwr           p.ipv6_0_valid, FALSE
        phvwr           p.ipv4_0_valid, TRUE
        phvwr           p.ipv4_0_udp_csum, FALSE
        phvwr           p.ipv4_0_tcp_csum, FALSE
        phvwr           p.ipv4_0_csum, TRUE
        */
        phvwr.e         p.{udp_0_valid...ipv4_0_csum}, 0x29
        nop

.align
session_rewrite_encap_geneve:



.align
session_rewrite_encap_invalid:
        phvwr.e         p.control_metadata_flow_miss, TRUE
        nop
