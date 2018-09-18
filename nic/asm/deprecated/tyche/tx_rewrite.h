tx_rewrite:
    sne             c1, d.tx_rewrite_d.hdr1_bits, 0
    balcf           r7, [c1], tx_rewrite_layer1
    crestore        [c7-c1], d.tx_rewrite_d.hdr1_bits[6:0], 0x7F

    sne             c1, d.tx_rewrite_d.hdr0_bits[3:0], 0
    nop.!c1.e
    crestore        [c5-c1], d.tx_rewrite_d.hdr0_bits[3:0], 0xF

tx_rewrite_layer0:
    // ethernet
    phvwr.c1        p.ethernet_0_valid, TRUE
    phvwr.c1        p.{ethernet_0_dstAddr...ethernet_0_etherType}, \
                        d.{tx_rewrite_d.ethernet_dst...tx_rewrite_d.ethernet_type}

    // ctag
    phvwr.c4        p.ctag_0_valid, TRUE
    phvwr.c4        p.ethernet_0_etherType, ETHERTYPE_CTAG
    phvwrpair.c4    p.ctag_0_vid, d.tx_rewrite_d.ctag, \
                        p.ctag_0_etherType, d.tx_rewrite_d.ethernet_type

    bcf             [c3], tx_rewrite_layer0_ipv6
    add             r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5,\
                           capri_p4_intrinsic_packet_len_sbit6_ebit13}, \
                        d.tx_rewrite_d.hdr0_bits[23:16]

tx_rewrite_layer0_ipv4:
    phvwr.c2        p.ipv4_0_valid, TRUE
    phvwrpair.c2    p.{ipv4_0_version,ipv4_0_ihl}, 0x45, \
                        p.ipv4_0_totalLen, r1
    phvwrpair.c2    p.ipv4_0_diffserv, d.tx_rewrite_d.ip_dscp, \
                        p.{ipv4_0_ttl,ipv4_0_protocol}, \
                        d.{tx_rewrite_d.ip_ttl,tx_rewrite_d.ip_proto}
    nop.e
    phvwrpair.c2    p.ipv4_0_srcAddr, d.tx_rewrite_d.ip_src[31:0], \
                        p.ipv4_0_dstAddr, d.tx_rewrite_d.ip_dst[31:0]

tx_rewrite_layer0_ipv6:
    phvwr           p.ipv6_0_valid, TRUE
    phvwrpair       p.ipv6_0_version, 6, p.ipv6_0_trafficClass, \
                        d.tx_rewrite_d.ip_dscp
    phvwrpair       p.ipv6_0_nextHdr, d.tx_rewrite_d.ip_proto, \
                        p.ipv6_0_hopLimit, d.tx_rewrite_d.ip_ttl
    phvwr           p.ipv6_0_payloadLen, r1
    phvwr.e         p.ipv6_0_srcAddr, d.tx_rewrite_d.ip_src
    phvwr           p.ipv6_0_dstAddr, d.tx_rewrite_d.ip_dst

tx_rewrite_layer1:
    bcf             [!c1&!c2&!c3&!c4&!c5&!c6&!c7], tx_rewrite_layer1_next
    phvwr.c1        p.ethernet_1_valid, FALSE
    phvwr.c2        p.ctag_1_valid, FALSE
    phvwr.c3        p.ipv4_1_valid, FALSE
    phvwr.c4        p.ipv6_1_valid, FALSE
    phvwr.c5        p.ethernet_1_srcAddr, d.tx_rewrite_d.ethernet_src
    phvwr.c6        p.ethernet_1_dstAddr, d.tx_rewrite_d.ethernet_dst
    phvwr.c7        p.ethernet_1_etherType, d.tx_rewrite_d.ethernet_type
tx_rewrite_layer1_next:
    crestore        [c7-c1], d.tx_rewrite_d.hdr1_bits[13:7], 0x7F
    phvwr.c1        p.ctag_1_valid, TRUE
    phvwr.c1        p.ethernet_1_etherType, ETHERTYPE_CTAG
    jrcf            [!c2&!c3&!c4&!c5&!c6], r7
    phvwrpair.c1    p.ctag_1_vid, d.tx_rewrite_d.ctag, \
                        p.ctag_1_etherType, d.tx_rewrite_d.ethernet_type
    bbeq            k.ipv4_1_valid, TRUE, tx_rewrite_layer1_ipv4
    bbeq            k.ipv6_1_valid, TRUE, tx_rewrite_layer1_ipv6
    nop
    nop.e
    nop

tx_rewrite_layer1_ipv4:
    phvwr.c2        p.ipv4_1_srcAddr, d.tx_rewrite_d.ip_src
    phvwr.c3        p.ipv4_1_dstAddr, d.tx_rewrite_d.ip_dst
    phvwr.c4        p.ipv4_1_diffserv, d.tx_rewrite_d.ip_dscp
    phvwr.c5        p.ipv4_1_protocol, d.tx_rewrite_d.ip_proto
    jr              r7
    phvwr.c6        p.ipv4_1_ttl, d.tx_rewrite_d.ip_ttl

tx_rewrite_layer1_ipv6:
    phvwr.c2        p.ipv6_1_srcAddr, d.tx_rewrite_d.ip_src
    phvwr.c3        p.ipv6_1_dstAddr, d.tx_rewrite_d.ip_dst
    phvwr.c4        p.ipv6_1_trafficClass, d.tx_rewrite_d.ip_dscp
    phvwr.c5        p.ipv6_1_nextHdr, d.tx_rewrite_d.ip_proto
    jr              r7
    phvwr.c6        p.ipv6_1_hopLimit, d.tx_rewrite_d.ip_ttl
