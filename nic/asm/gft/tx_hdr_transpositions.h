tx_hdr_transpositions:
    sne             c1, d.tx_hdr_transpositions_d.hdr1_bits, 0
    nop.!c1.e
    crestore        [c7-c1], d.tx_hdr_transpositions_d.hdr1_bits[6:0], 0x3F

tx_hdr_transpositions_layer1:
    phvwr.c1        p.ethernet_1_valid, FALSE
    phvwr.c2        p.ctag_1_valid, FALSE
    phvwr.c3        p.ipv4_1_valid, FALSE
    phvwr.c4        p.ipv6_1_valid, FALSE
    phvwr.c5        p.ethernet_1_srcAddr, d.tx_hdr_transpositions_d.ethernet_src
    phvwr.c6        p.ethernet_1_dstAddr, d.tx_hdr_transpositions_d.ethernet_dst
    phvwr.c7        p.ethernet_1_etherType, d.tx_hdr_transpositions_d.ethernet_type
    crestore        [c7-c1], d.tx_hdr_transpositions_d.hdr1_bits[13:7], 0x3F
    phvwr.c1        p.ctag_1_valid, TRUE
    phvwr.c1        p.ethernet_1_etherType, ETHERTYPE_CTAG
    phvwrpair.c1    p.ctag_1_vid, d.tx_hdr_transpositions_d.ctag, \
                        p.ctag_1_etherType, d.tx_hdr_transpositions_d.ethernet_type
    phvwr.c2        p.ipv4_1_srcAddr, d.tx_hdr_transpositions_d.ip_src
    phvwr.c3        p.ipv4_1_dstAddr, d.tx_hdr_transpositions_d.ip_dst
    phvwr.c4        p.ipv4_1_diffserv, d.tx_hdr_transpositions_d.ip_dscp
    phvwr.c5        p.ipv4_1_protocol, d.tx_hdr_transpositions_d.ip_proto
    phvwr.c6        p.ipv4_1_ttl, d.tx_hdr_transpositions_d.ip_ttl
    phvwr.c7        p.ipv6_1_srcAddr, d.tx_hdr_transpositions_d.ip_src
    crestore        [c7-c1], d.tx_hdr_transpositions_d.hdr1_bits[20:14], 0x3F
    phvwr.c1        p.ipv6_1_dstAddr, d.tx_hdr_transpositions_d.ip_dst
    phvwr.c2        p.ipv6_1_trafficClass, d.tx_hdr_transpositions_d.ip_dscp
    phvwr.c3        p.ipv6_1_nextHdr, d.tx_hdr_transpositions_d.ip_proto
    nop.e
    phvwr.c4        p.ipv6_1_hopLimit, d.tx_hdr_transpositions_d.ip_ttl
