hdr_transpositions:
    sne             c1, d.hdr_transpositions_d.hdr1_bits, 0
    balcf           r7, [c1], hdr_transpositions_layer1
    crestore        [c7-c1], d.hdr_transpositions_d.hdr1_bits[6:0], 0x3F

    sne             c1, d.hdr_transpositions_d.hdr2_bits, 0
    balcf           r7, [c1], hdr_transpositions_layer2
    crestore        [c7-c1], d.hdr_transpositions_d.hdr2_bits[6:0], 0x3F

    sne             c1, d.hdr_transpositions_d.hdr3_bits, 0
    balcf           r7, [c1], hdr_transpositions_layer3
    crestore        [c7-c1], d.hdr_transpositions_d.hdr3_bits[7:0], 0x3F

    sne             c1, d.hdr_transpositions_d.hdr0_bits[9:5], 0
    balcf           r7, [c1], hdr_transpositions_layer01
    crestore        [c5-c1], d.hdr_transpositions_d.hdr0_bits[9:5], 0x1F

    sne             c1, d.hdr_transpositions_d.hdr0_bits[4:0], 0
    nop.!c1.e
    crestore        [c5-c1], d.hdr_transpositions_d.hdr0_bits[4:0], 0x1F

hdr_transpositions_layer00:
    // ethernet
    phvwr.c1        p.ethernet_00_valid, TRUE
    phvwr.c1        p.{ethernet_00_dstAddr...ethernet_00_etherType}, \
                        d.{hdr_transpositions_d.ethernet_dst...hdr_transpositions_d.ethernet_type}

    // ctag
    phvwr.c4        p.ctag_00_valid, TRUE
    phvwr.c4        p.ethernet_00_etherType, ETHERTYPE_CTAG
    phvwrpair.c4    p.ctag_00_vid, d.hdr_transpositions_d.ctag, \
                        p.ctag_00_etherType, d.hdr_transpositions_d.ethernet_type

    // stag
    phvwr.c5        p.stag_00_valid, TRUE
    phvwr.c5        p.ethernet_00_etherType, ETHERTYPE_STAG
    phvwr.c5        p.stag_00_vid, d.hdr_transpositions_d.stag
    phvwr.c5        p.stag_00_etherType, ETHERTYPE_CTAG

    bcf             [c3], hdr_transpositions_layer00_ipv6

hdr_transpositions_layer00_ipv4:
    phvwr.c2        p.ipv4_00_valid, TRUE
    phvwr.c2        p.{ipv4_00_version,ipv4_00_ihl}, 0x45
    phvwrpair.c2    p.ipv4_00_diffserv, d.hdr_transpositions_d.ip_dscp, \
                        p.{ipv4_00_ttl,ipv4_00_protocol}, \
                        d.{hdr_transpositions_d.ip_ttl,hdr_transpositions_d.ip_proto}
    nop.e
    phvwrpair.c2    p.ipv4_00_srcAddr, d.hdr_transpositions_d.ip_src[31:0], \
                        p.ipv4_00_dstAddr, d.hdr_transpositions_d.ip_dst[31:0]

hdr_transpositions_layer00_ipv6:
    phvwr           p.ipv6_00_valid, TRUE
    phvwrpair       p.ipv6_00_version, 6, p.ipv6_00_trafficClass, \
                        d.hdr_transpositions_d.ip_dscp
    phvwrpair       p.ipv6_00_nextHdr, d.hdr_transpositions_d.ip_proto, \
                        p.ipv6_00_hopLimit, d.hdr_transpositions_d.ip_ttl
    phvwr.e         p.ipv6_00_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr           p.ipv6_00_dstAddr, d.hdr_transpositions_d.ip_dst

hdr_transpositions_layer01:
    // ethernet
    phvwr.c1        p.ethernet_01_valid, TRUE
    phvwr.c1        p.{ethernet_01_dstAddr...ethernet_01_etherType}, \
                        d.{hdr_transpositions_d.ethernet_dst...hdr_transpositions_d.ethernet_type}

    // ctag
    phvwr.c4        p.ctag_01_valid, TRUE
    phvwr.c4        p.ethernet_01_etherType, ETHERTYPE_CTAG
    phvwrpair.c4    p.ctag_01_vid, d.hdr_transpositions_d.ctag, \
                        p.ctag_01_etherType, d.hdr_transpositions_d.ethernet_type

    // stag
    phvwr.c5        p.stag_01_valid, TRUE
    phvwr.c5        p.ethernet_01_etherType, ETHERTYPE_STAG
    phvwr.c5        p.stag_01_vid, d.hdr_transpositions_d.stag
    phvwr.c5        p.stag_01_etherType, ETHERTYPE_CTAG

    bcf             [c3], hdr_transpositions_layer01_ipv6

hdr_transpositions_layer01_ipv4:
    phvwr.c2        p.ipv4_01_valid, TRUE
    phvwr.c2        p.{ipv4_01_version,ipv4_01_ihl}, 0x45
    phvwrpair.c2    p.ipv4_01_diffserv, d.hdr_transpositions_d.ip_dscp, \
                        p.{ipv4_01_ttl,ipv4_01_protocol}, \
                        d.{hdr_transpositions_d.ip_ttl,hdr_transpositions_d.ip_proto}
    jr              r7
    phvwrpair.c2    p.ipv4_01_srcAddr, d.hdr_transpositions_d.ip_src[31:0], \
                        p.ipv4_01_dstAddr, d.hdr_transpositions_d.ip_dst[31:0]

hdr_transpositions_layer01_ipv6:
    phvwr           p.ipv6_01_valid, TRUE
    phvwrpair       p.ipv6_01_version, 6, p.ipv6_01_trafficClass, \
                        d.hdr_transpositions_d.ip_dscp
    phvwrpair       p.ipv6_01_nextHdr, d.hdr_transpositions_d.ip_proto, \
                        p.ipv6_01_hopLimit, d.hdr_transpositions_d.ip_ttl
    phvwr           p.ipv6_01_srcAddr, d.hdr_transpositions_d.ip_src
    jr              r7
    phvwr           p.ipv6_01_dstAddr, d.hdr_transpositions_d.ip_dst

hdr_transpositions_layer1:
    phvwr.c1        p.ethernet_1_valid, FALSE
    phvwr.c2        p.ctag_1_valid, FALSE
    phvwr.c3        p.stag_1_valid, FALSE
    phvwr.c4        p.ipv4_1_valid, FALSE
    phvwr.c5        p.ipv6_1_valid, FALSE
    phvwr.c6        p.ethernet_1_srcAddr, d.hdr_transpositions_d.ethernet_src
    phvwr.c7        p.ethernet_1_dstAddr, d.hdr_transpositions_d.ethernet_dst
    crestore        [c7-c1], d.hdr_transpositions_d.hdr1_bits[13:7], 0x3F
    phvwr.c1        p.ethernet_1_etherType, d.hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ctag_1_vid, d.hdr_transpositions_d.ctag
    phvwr.c3        p.stag_1_vid, d.hdr_transpositions_d.stag
    phvwr.c4        p.ipv4_1_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c5        p.ipv4_1_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c6        p.ipv4_1_diffserv, d.hdr_transpositions_d.ip_dscp
    phvwr.c7        p.ipv4_1_protocol, d.hdr_transpositions_d.ip_proto
    crestore        [c7-c1], d.hdr_transpositions_d.hdr1_bits[20:8], 0x3F
    phvwr.c1        p.ipv4_1_ttl, d.hdr_transpositions_d.ip_ttl
    phvwr.c2        p.ipv6_1_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv6_1_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv6_1_trafficClass, d.hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv6_1_nextHdr, d.hdr_transpositions_d.ip_proto
    jr              r7
    phvwr.c6        p.ipv6_1_hopLimit, d.hdr_transpositions_d.ip_ttl

hdr_transpositions_layer2:
    phvwr.c1        p.ethernet_2_valid, FALSE
    phvwr.c2        p.ctag_2_valid, FALSE
    phvwr.c3        p.stag_2_valid, FALSE
    phvwr.c4        p.ipv4_2_valid, FALSE
    phvwr.c5        p.ipv6_2_valid, FALSE
    phvwr.c6        p.ethernet_2_srcAddr, d.hdr_transpositions_d.ethernet_src
    phvwr.c7        p.ethernet_2_dstAddr, d.hdr_transpositions_d.ethernet_dst
    crestore        [c7-c1], d.hdr_transpositions_d.hdr2_bits[13:7], 0x3F
    phvwr.c1        p.ethernet_2_etherType, d.hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ctag_2_vid, d.hdr_transpositions_d.ctag
    phvwr.c3        p.stag_2_vid, d.hdr_transpositions_d.stag
    phvwr.c4        p.ipv4_2_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c5        p.ipv4_2_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c6        p.ipv4_2_diffserv, d.hdr_transpositions_d.ip_dscp
    phvwr.c7        p.ipv4_2_protocol, d.hdr_transpositions_d.ip_proto
    crestore        [c7-c1], d.hdr_transpositions_d.hdr2_bits[20:8], 0x3F
    phvwr.c1        p.ipv4_2_ttl, d.hdr_transpositions_d.ip_ttl
    phvwr.c2        p.ipv6_2_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv6_2_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv6_2_trafficClass, d.hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv6_2_nextHdr, d.hdr_transpositions_d.ip_proto
    jr              r7
    phvwr.c6        p.ipv6_2_hopLimit, d.hdr_transpositions_d.ip_ttl

hdr_transpositions_layer3:
    phvwr.c1        p.ethernet_3_valid, FALSE
    phvwr.c2        p.ctag_3_valid, FALSE
    phvwr.c3        p.stag_3_valid, FALSE
    phvwr.c4        p.ipv4_3_valid, FALSE
    phvwr.c5        p.ipv6_3_valid, FALSE
    phvwr.c6        p.ethernet_3_srcAddr, d.hdr_transpositions_d.ethernet_src
    phvwr.c7        p.ethernet_3_dstAddr, d.hdr_transpositions_d.ethernet_dst
    crestore        [c7-c1], d.hdr_transpositions_d.hdr3_bits[13:7], 0x3F
    phvwr.c1        p.ethernet_3_etherType, d.hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ctag_3_vid, d.hdr_transpositions_d.ctag
    phvwr.c3        p.stag_3_vid, d.hdr_transpositions_d.stag
    phvwr.c4        p.ipv4_3_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c5        p.ipv4_3_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c6        p.ipv4_3_diffserv, d.hdr_transpositions_d.ip_dscp
    phvwr.c7        p.ipv4_3_protocol, d.hdr_transpositions_d.ip_proto
    crestore        [c7-c1], d.hdr_transpositions_d.hdr3_bits[20:8], 0x3F
    phvwr.c1        p.ipv4_3_ttl, d.hdr_transpositions_d.ip_ttl
    phvwr.c2        p.ipv6_3_srcAddr, d.hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv6_3_dstAddr, d.hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv6_3_trafficClass, d.hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv6_3_nextHdr, d.hdr_transpositions_d.ip_proto
    jr              r7
    phvwr.c6        p.ipv6_3_hopLimit, d.hdr_transpositions_d.ip_ttl
