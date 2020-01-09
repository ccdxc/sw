#include "apulu.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_mirror_k.h"

struct mirror_k_    k;
struct mirror_d     d;
struct phv_         p;

%%

lspan:
    add             r6, r0, d.u.lspan_d.truncate_len
    bal             r7, mirror_truncate
    sne             c7, r0, r0
    phvwr           p.capri_p4_intrinsic_packet_len, r5
    phvwr           p.rewrite_metadata_nexthop_type, d.u.lspan_d.nexthop_type
    phvwr           p.p4e_i2e_nexthop_id, d.u.lspan_d.nexthop_id
    phvwr.e         p.mirror_blob_valid, FALSE
    phvwr.f         p.capri_intrinsic_tm_span_session, 0

.align
rspan:
    phvwr           p.ctag_1_valid, 1
    add             r1, k.ethernet_1_etherType, d.u.rspan_d.ctag, 16
    phvwr           p.{ctag_1_pcp,ctag_1_dei,ctag_1_vid,ctag_1_etherType}, r1
    phvwr           p.ethernet_1_etherType, ETHERTYPE_CTAG
    add             r6, r0, d.u.rspan_d.truncate_len
    bal             r7, mirror_truncate
    sne             c7, r0, r0
    phvwr           p.capri_p4_intrinsic_packet_len, r5
    phvwr           p.rewrite_metadata_nexthop_type, d.u.rspan_d.nexthop_type
    phvwr           p.p4e_i2e_nexthop_id, d.u.rspan_d.nexthop_id
    phvwr.e         p.mirror_blob_valid, FALSE
    phvwr.f         p.capri_intrinsic_tm_span_session, 0

.align
erspan:
    phvwr           p.erspan_timestamp, r6

    // truncate
    add             r6, r0, d.u.erspan_d.truncate_len
    bal             r7, mirror_truncate
    seq             c7, r0, r0

    // headers
    .assert(offsetof(p, erspan_valid) - offsetof(p, ethernet_0_valid) == 10)
    phvwrmi         p.{erspan_valid,gre_0_valid,vxlan_0_valid,udp_0_valid, \
                       ipv6_0_valid,ipv4_0_valid,ipv4_0_udp_csum, \
                       ipv4_0_tcp_csum,ipv4_0_csum,ctag_0_valid, \
                       ethernet_0_valid}, 0x627, 0xFFF

    // ethernet and ctag
    phvwr           p.ethernet_0_dstAddr, d.u.erspan_d.dmac
    or              r1, ETHERTYPE_CTAG, d.u.erspan_d.smac, 16
    phvwr           p.{ethernet_0_srcAddr,ethernet_0_etherType}, r1
    or              r1, ETHERTYPE_IPV4, d.u.erspan_d.ctag, 16
    phvwr           p.{ctag_0_pcp,ctag_0_dei,ctag_0_vid,ctag_0_etherType}, r1

    // ipv4
    phvwr           p.{ipv4_0_version,ipv4_0_ihl}, 0x45
    phvwr           p.{ipv4_0_srcAddr,ipv4_0_dstAddr}, \
                        d.{u.erspan_d.sip,u.erspan_d.dip}
    phvwr           p.ipv4_0_ttl, 64
    phvwr           p.ipv4_0_protocol, IP_PROTO_GRE
    add             r1, r5, 36
    phvwr           p.ipv4_0_totalLen, r1
    phvwr           p.capri_deparser_len_ipv4_0_hdr_len, 20

    // gre
    phvwri          p.{gre_0_C...gre_0_proto}, GRE_PROTO_ERSPAN_T3

    // erspan
    phvwrpair        p.erspan_version, 0x2, p.erspan_bso, 0
    seq             c1, k.ctag_1_valid, TRUE
    phvwrpair.c1    p.erspan_vlan, k.ctag_1_vid, p.erspan_cos, k.ctag_1_pcp
    phvwr           p.erspan_span_id, k.capri_intrinsic_tm_span_session
    seq             c1, k.capri_intrinsic_tm_iport, TM_PORT_EGRESS
    phvwr.c1        p.erspan_direction, 1
    phvwrpair       p.{erspan_sgt...erspan_hw_id}, 0, \
                        p.{erspan_granularity,erspan_options}, 0x6

    add             r1, r5, 54
    phvwr           p.capri_p4_intrinsic_packet_len, r1
    phvwr           p.rewrite_metadata_nexthop_type, d.u.rspan_d.nexthop_type
    phvwr           p.p4e_i2e_nexthop_id, d.u.rspan_d.nexthop_id
    phvwr           p.ctag_1_valid, FALSE
    phvwr.e         p.mirror_blob_valid, FALSE
    phvwr.f         p.capri_intrinsic_tm_span_session, 0

mirror_truncate:
    phvwr           p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
    sub             r5, k.capri_p4_intrinsic_frame_size, k.offset_metadata_l2_1
    sub             r1, r5, 14
    sne             c1, r6, r0
    slt.c1          c1, r6, r1
    jr.!c1          r7
    phvwr.c1        p.capri_deparser_len_trunc_pkt_len, r6
    add             r5, r6, 14
    phvwr.c7        p.erspan_truncated, TRUE
    jr              r7
    phvwr           p.{capri_intrinsic_payload,capri_deparser_len_trunc}, 0x1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
mirror_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr.f         p.capri_intrinsic_tm_iq, k.capri_intrinsic_tm_oq
