rx_hdr_transpositions:
    sne             c1, d.rx_hdr_transpositions_d.hdr2_bits, 0
    balcf           r7, [c1], rx_hdr_transpositions_layer2
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr2_bits[6:0], 0x3F

    sne             c1, d.rx_hdr_transpositions_d.hdr3_bits, 0
    balcf           r7, [c1], rx_hdr_transpositions_layer3
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr3_bits[6:0], 0x3F

    sne             c1, d.rx_hdr_transpositions_d.hdr1_bits, 0
    nop.!c1.e
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr1_bits[6:0], 0x3F

rx_hdr_transpositions_layer1:
    phvwr.c1        p.ethernet_1_valid, FALSE
    phvwr.c2        p.ctag_1_valid, FALSE
    phvwr.c3        p.ipv4_1_valid, FALSE
    phvwr.c4        p.ipv6_1_valid, FALSE
    phvwr.c5        p.ethernet_1_srcAddr, d.rx_hdr_transpositions_d.ethernet_src
    phvwr.c6        p.ethernet_1_dstAddr, d.rx_hdr_transpositions_d.ethernet_dst
    phvwr.c7        p.ethernet_1_etherType, d.rx_hdr_transpositions_d.ethernet_type
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr1_bits[13:7], 0x3F
    phvwr.c1        p.ctag_1_valid, TRUE
    phvwr.c1        p.ethernet_1_etherType, ETHERTYPE_CTAG
    phvwrpair.c1    p.ctag_1_vid, d.rx_hdr_transpositions_d.ctag, \
                        p.ctag_1_etherType, d.rx_hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ipv4_1_srcAddr, d.rx_hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv4_1_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv4_1_diffserv, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv4_1_protocol, d.rx_hdr_transpositions_d.ip_proto
    phvwr.c6        p.ipv4_1_ttl, d.rx_hdr_transpositions_d.ip_ttl
    phvwr.c7        p.ipv6_1_srcAddr, d.rx_hdr_transpositions_d.ip_src
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr1_bits[20:14], 0x3F
    phvwr.c1        p.ipv6_1_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c2        p.ipv6_1_trafficClass, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c3        p.ipv6_1_nextHdr, d.rx_hdr_transpositions_d.ip_proto
    nop.e
    phvwr.c4        p.ipv6_1_hopLimit, d.rx_hdr_transpositions_d.ip_ttl

rx_hdr_transpositions_layer2:
    phvwr.c1        p.ethernet_2_valid, FALSE
    phvwr.c2        p.ctag_2_valid, FALSE
    phvwr.c3        p.ipv4_2_valid, FALSE
    phvwr.c4        p.ipv6_2_valid, FALSE
    phvwr.c5        p.ethernet_2_srcAddr, d.rx_hdr_transpositions_d.ethernet_src
    phvwr.c6        p.ethernet_2_dstAddr, d.rx_hdr_transpositions_d.ethernet_dst
    phvwr.c7        p.ethernet_2_etherType, d.rx_hdr_transpositions_d.ethernet_type
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr2_bits[13:7], 0x3F
    phvwr.c1        p.ctag_2_valid, TRUE
    phvwr.c1        p.ethernet_2_etherType, ETHERTYPE_CTAG
    phvwrpair.c1    p.ctag_2_vid, d.rx_hdr_transpositions_d.ctag, \
                        p.ctag_2_etherType, d.rx_hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ipv4_2_srcAddr, d.rx_hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv4_2_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv4_2_diffserv, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv4_2_protocol, d.rx_hdr_transpositions_d.ip_proto
    phvwr.c6        p.ipv4_2_ttl, d.rx_hdr_transpositions_d.ip_ttl
    phvwr.c7        p.ipv6_2_srcAddr, d.rx_hdr_transpositions_d.ip_src
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr2_bits[20:14], 0x3F
    phvwr.c1        p.ipv6_2_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c2        p.ipv6_2_trafficClass, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c3        p.ipv6_2_nextHdr, d.rx_hdr_transpositions_d.ip_proto
    jr              r7
    phvwr.c4        p.ipv6_2_hopLimit, d.rx_hdr_transpositions_d.ip_ttl

rx_hdr_transpositions_layer3:
    phvwr.c1        p.ethernet_3_valid, FALSE
    phvwr.c2        p.ctag_3_valid, FALSE
    phvwr.c3        p.ipv4_3_valid, FALSE
    phvwr.c4        p.ipv6_3_valid, FALSE
    phvwr.c5        p.ethernet_3_srcAddr, d.rx_hdr_transpositions_d.ethernet_src
    phvwr.c6        p.ethernet_3_dstAddr, d.rx_hdr_transpositions_d.ethernet_dst
    phvwr.c7        p.ethernet_3_etherType, d.rx_hdr_transpositions_d.ethernet_type
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr3_bits[13:7], 0x3F
    phvwr.c1        p.ctag_3_valid, TRUE
    phvwr.c1        p.ethernet_3_etherType, ETHERTYPE_CTAG
    phvwrpair.c1    p.ctag_3_vid, d.rx_hdr_transpositions_d.ctag, \
                        p.ctag_3_etherType, d.rx_hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ipv4_3_srcAddr, d.rx_hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv4_3_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv4_3_diffserv, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv4_3_protocol, d.rx_hdr_transpositions_d.ip_proto
    phvwr.c6        p.ipv4_3_ttl, d.rx_hdr_transpositions_d.ip_ttl
    phvwr.c7        p.ipv6_3_srcAddr, d.rx_hdr_transpositions_d.ip_src
    crestore        [c7-c1], d.rx_hdr_transpositions_d.hdr3_bits[20:14], 0x3F
    phvwr.c1        p.ipv6_3_dstAddr, d.rx_hdr_transpositions_d.ip_dst
    phvwr.c2        p.ipv6_3_trafficClass, d.rx_hdr_transpositions_d.ip_dscp
    phvwr.c3        p.ipv6_3_nextHdr, d.rx_hdr_transpositions_d.ip_proto
    jr              r7
    phvwr.c4        p.ipv6_3_hopLimit, d.rx_hdr_transpositions_d.ip_ttl

#if 0
    sne             c1, d.hdr_transpositions_d.hdr0_bits[7:4], 0
    balcf           r7, [c1], hdr_transpositions_layer01
    crestore        [c5-c1], d.hdr_transpositions_d.hdr0_bits[7:4], 0x1F

    sne             c1, d.hdr_transpositions_d.hdr0_bits[3:0], 0
    nop.!c1.e
    crestore        [c5-c1], d.hdr_transpositions_d.hdr0_bits[3:0], 0x1F

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

    bcf             [c3], hdr_transpositions_layer00_ipv6
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.hdr_transpositions_d.hdr0_bits[23:18]

hdr_transpositions_layer00_ipv4:
    phvwr.c2        p.ipv4_00_valid, TRUE
    phvwrpair.c2    p.{ipv4_00_version,ipv4_00_ihl}, 0x45, \
                        p.ipv4_00_totalLen, r1
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
    phvwr           p.ipv6_00_payloadLen, r1
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

    bcf             [c3], hdr_transpositions_layer01_ipv6
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.hdr_transpositions_d.hdr0_bits[17:12]

hdr_transpositions_layer01_ipv4:
    phvwr.c2        p.ipv4_01_valid, TRUE
    phvwrpair.c2    p.{ipv4_01_version,ipv4_01_ihl}, 0x45, \
                        p.ipv4_01_totalLen, r1
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
    phvwr           p.ipv6_01_payloadLen, r1
    phvwr           p.ipv6_01_srcAddr, d.hdr_transpositions_d.ip_src
    jr              r7
    phvwr           p.ipv6_01_dstAddr, d.hdr_transpositions_d.ip_dst
#endif
