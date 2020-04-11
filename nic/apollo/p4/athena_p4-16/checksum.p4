#define PKT_IPV4_HDRLEN_0            ((bit<16>) hdr.ip_0.ipv4.ihl << 2)
#define PKT_IPV4_HDRLEN_1            ((bit<16>) hdr.ip_1.ipv4.ihl << 2)
#define PKT_IPV4_TOTAL_LEN_0         hdr.ip_0.ipv4.totalLen
#define PKT_IPV4_TOTAL_LEN_1         hdr.ip_1.ipv4.totalLen
#define PKT_IPV4_HDRLEN_2            ((bit<16>) hdr.ip_2.ipv4.ihl << 2)
#define PKT_IPV4_TOTAL_LEN_2         hdr.ip_2.ipv4.totalLen
#define PKT_IPV6_PAYLOAD_LEN_0       hdr.ip_0.ipv6.payloadLen
#define PKT_IPV6_PAYLOAD_LEN_1       hdr.ip_1.ipv6.payloadLen
#define PKT_IPV6_PAYLOAD_LEN_2       hdr.ip_2.ipv6.payloadLen

control update_checksums(inout cap_phv_intr_global_h capri_intrinsic,
                 inout cap_phv_intr_p4_h intr_p4,
                 inout headers hdr, 
                 inout metadata_t metadata) {

    
    @name(".update_ipv4_checksum") action update_ipv4_checksum() {
        metadata.csum.ip_hdr_len_1        = PKT_IPV4_HDRLEN_1;
	ipv4HdrCsumDepEg_1.enable_update();
    }

    @name(".update_ipv4_udp_checksum") action update_ipv4_udp_checksum() {
        metadata.csum.ip_hdr_len_1        = PKT_IPV4_HDRLEN_1;
        metadata.csum.udp_len_1           = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;
	ipv4HdrCsumDepEg_1.enable_update();
        udpCsumDepEg_1.enable_update(hdr.ip_1.ipv4);

    }

    @name(".update_ipv4_tcp_checksum") action update_ipv4_tcp_checksum() {
        metadata.csum.ip_hdr_len_1        = PKT_IPV4_HDRLEN_1;
        metadata.csum.tcp_len_1           = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;
	ipv4HdrCsumDepEg_1.enable_update();
	tcpCsumDepEg_1.enable_update(hdr.ip_1.ipv4);

    }

    @name(".update_ipv4_icmp_checksum") action update_ipv4_icmp_checksum() {
        metadata.csum.ip_hdr_len_1        = PKT_IPV4_HDRLEN_1;
        metadata.csum.icmp_len_1           = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;
	ipv4HdrCsumDepEg_1.enable_update();
	//icmpv4CsumDepEg_1.enable_update();

    }

    @name(".update_ipv6_udp_checksum") action update_ipv6_udp_checksum() {
        metadata.csum.udp_len_1           = PKT_IPV6_PAYLOAD_LEN_1;
	udpCsumDepEg_1.enable_update(hdr.ip_1.ipv6);

    }

    @name(".update_ipv6_tcp_checksum") action update_ipv6_tcp_checksum() {
        metadata.csum.tcp_len_1           = PKT_IPV6_PAYLOAD_LEN_1;
	tcpCsumDepEg_1.enable_update(hdr.ip_1.ipv6);
    }

    @name(".update_ipv6_icmp_checksum") action update_ipv6_icmp_checksum() {
        metadata.csum.icmp_len_1           = PKT_IPV6_PAYLOAD_LEN_1;
	icmpv6CsumDepEg_1.enable_update(hdr.ip_1.ipv6);
    }

    @name(".update_l2_ipv4_checksum") action update_l2_ipv4_checksum() {
        metadata.csum.ip_hdr_len_2        = PKT_IPV4_HDRLEN_2;
	ipv4HdrCsumDepEg_2.enable_update();
    }

    @name(".update_l2_ipv4_udp_checksum") action update_l2_ipv4_udp_checksum() {
        metadata.csum.ip_hdr_len_2        = PKT_IPV4_HDRLEN_2;
        metadata.csum.udp_len_2           = PKT_IPV4_TOTAL_LEN_2 - PKT_IPV4_HDRLEN_2;
	ipv4HdrCsumDepEg_2.enable_update();
	udpCsumDepEg_2.enable_update(hdr.ip_2.ipv4);

    }

    @name(".update_l2_ipv4_tcp_checksum") action update_l2_ipv4_tcp_checksum() {
        metadata.csum.ip_hdr_len_2        = PKT_IPV4_HDRLEN_2;
        metadata.csum.tcp_len_1           = PKT_IPV4_TOTAL_LEN_2 - PKT_IPV4_HDRLEN_2;
	ipv4HdrCsumDepEg_2.enable_update();
	tcpCsumDepEg_1.enable_update(hdr.ip_2.ipv4);

    }

    @name(".update_l2_ipv4_icmp_checksum") action update_l2_ipv4_icmp_checksum() {
        metadata.csum.ip_hdr_len_2        = PKT_IPV4_HDRLEN_2;
        metadata.csum.icmp_len_1           = PKT_IPV4_TOTAL_LEN_2 - PKT_IPV4_HDRLEN_2;
	ipv4HdrCsumDepEg_2.enable_update();
	icmpv4CsumDepEg_1.enable_update();

    }

    @name(".update_l2_ipv6_udp_checksum") action update_l2_ipv6_udp_checksum() {
        metadata.csum.udp_len_2           = PKT_IPV6_PAYLOAD_LEN_2;
	udpCsumDepEg_2.enable_update(hdr.ip_2.ipv6);

    }

    @name(".update_l2_ipv6_tcp_checksum") action update_l2_ipv6_tcp_checksum() {
        metadata.csum.tcp_len_1           = PKT_IPV6_PAYLOAD_LEN_2;
	tcpCsumDepEg_1.enable_update(hdr.ip_2.ipv6);
    }

    @name(".update_l2_ipv6_icmp_checksum") action update_l2_ipv6_icmp_checksum() {
        metadata.csum.icmp_len_1           = PKT_IPV6_PAYLOAD_LEN_2;
	icmpv6CsumDepEg_1.enable_update(hdr.ip_2.ipv6);
    }



    @name(".checksum") table checksum {
      key = {
            hdr.ip_1.ipv4.isValid()                   : ternary;
            hdr.ip_1.ipv6.isValid()                   : ternary;
            hdr.udp.isValid()                         : ternary;
            hdr.ip_2.ipv4.isValid()                   : ternary;
            hdr.ip_2.ipv6.isValid()                   : ternary;
            hdr.l4_u.udp.isValid()                    : ternary;
            hdr.l4_u.tcp.isValid()                    : ternary;
            metadata.l4.icmp_valid                 : ternary @name(".icmp_valid");

      }

       actions = {
	 update_ipv4_checksum;
	 update_ipv4_udp_checksum;
	 update_ipv4_tcp_checksum;
	 update_ipv4_icmp_checksum;
	 update_ipv6_udp_checksum;
	 update_ipv6_tcp_checksum;
	 update_ipv6_icmp_checksum;
	 update_l2_ipv4_checksum;
	 update_l2_ipv4_udp_checksum;
	 update_l2_ipv4_tcp_checksum;
	 update_l2_ipv4_icmp_checksum;
	 update_l2_ipv6_udp_checksum;
	 update_l2_ipv6_tcp_checksum;
	 update_l2_ipv6_icmp_checksum;
        }

	size = CHECKSUM_TABLE_SIZE;
	default_action = update_ipv4_checksum;
        stage = 5;
    }

    apply {
      if(metadata.cntrl.update_checksum == TRUE) {
	checksum.apply();
      }
    }
}

