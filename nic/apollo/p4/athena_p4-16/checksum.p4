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
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
	ipv4HdrCsumDepEg_0.enable_update();
    }

    @name(".update_ipv4_udp_checksum") action update_ipv4_udp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.udp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
	ipv4HdrCsumDepEg_0.enable_update();
	udpCsumDepEg_1.enable_update(hdr.ip_1.ipv4);

    }

    @name(".update_ipv4_tcp_checksum") action update_ipv4_tcp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.tcp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
	ipv4HdrCsumDepEg_0.enable_update();
	tcpCsumDepEg_1.enable_update(hdr.ip_1.ipv4);

    }

    @name(".update_ipv4_icmp_checksum") action update_ipv4_icmp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.icmp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
	ipv4HdrCsumDepEg_0.enable_update();
	icmpCsumDepEg_1.enable_update(hdr.ip_1.ipv4);

    }

    @name(".update_ipv6_udp_checksum") action update_ipv6_udp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.udp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
    }

    @name(".update_ipv6_tcp_checksum") action update_ipv6_tcp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.tcp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
    }

    @name(".update_ipv6_icmp_checksum") action update_ipv6_icmp_checksum() {
        metadata.csum.ip_hdr_len_0        = PKT_IPV4_HDRLEN_0;
        metadata.csum.icmp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
    }


    /*
	metadata.csum.tcp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
        metadata.csum.udp_len_0           = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;
        metadata.csum.icmp_len_0          = PKT_IPV4_TOTAL_LEN_0 - PKT_IPV4_HDRLEN_0;

        if(metadata.csum.update_checksum_0 == TRUE) { 	
	  if (hdr.ip_0.ipv4.isValid()) {
            ipv4HdrCsumDep_0.enable_update();

	    
	    if (hdr.l4_0.tcp.isValid()) {
	      tcpCsumDep_0.enable_update(hdr.ip_0.ipv4);
	    }
	    if (hdr.l4_0.udp.isValid()) {
	      udpCsumDep_0.enable_update(hdr.ip_0.ipv4);
	    }
	    
	  }

	 	  if (hdr.ip_0.ipv6.isValid()) {
	    
	    if (hdr.l4_0.tcp.isValid()) {
	      tcpCsumDep_0.enable_update(hdr.ip_0.ipv6);
	    }
	    if (hdr.l4_0.udp.isValid()) {
	      udpCsumDep_0.enable_update(hdr.ip_0.ipv6);
	    }
	    
	  }
	  
	  if (hdr.l4_0.icmp.isValid()) {
            icmpCsumDep_0.enable_update();
	  }
	}
	  
	}
    }
    

    @name(".compute_checksum1") action compute_checksum1 () {
        metadata.csum.ip_hdr_len_1        = PKT_IPV4_HDRLEN_1;
	metadata.csum.tcp_len_1           = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;
        metadata.csum.udp_len_1           = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;
        metadata.csum.icmp_len_1          = PKT_IPV4_TOTAL_LEN_1 - PKT_IPV4_HDRLEN_1;

 
        if(metadata.csum.update_checksum_0 == TRUE) { 	
	  if (hdr.ip_1.ipv4.isValid()) {
            ipv4HdrCsumDep_1.enable_update();
	    
	    if (hdr.l4_u.tcp.isValid()) {
	      tcpCsumDep_1.enable_update(hdr.ip_1.ipv4);
	    }
	    if (hdr.l4_u.udp.isValid()) {
	      udpCsumDep_1.enable_update(hdr.ip_1.ipv4);
	    }
	    
	  }
	  
	  if (hdr.ip_1.ipv6.isValid()) {
	    
	    if (hdr.l4_u.tcp.isValid()) {
	      tcpCsumDep_1.enable_update(hdr.ip_1.ipv6);
	    }
	    if (hdr.l4_u.udp.isValid()) {
	      udpCsumDep_1.enable_update(hdr.ip_1.ipv6);
	    }
	    
	  }
	  
	  if (hdr.l4_u.icmp.isValid()) {
            icmpCsumDep_1.enable_update();
	  }
	}
    }
    */

    @name(".checksum") table checksum {
      key = {
            hdr.ip_1.ipv4.isValid()                   : exact;
            hdr.ip_1.ipv6.isValid()                   : exact;
            hdr.udp.isValid()         : exact;
            hdr.l4_u.tcp.isValid()                   : exact;
            hdr.l4_u.icmp.isValid()                   : exact;

      }

       actions = {
        update_ipv4_checksum;
        update_ipv4_udp_checksum;
        update_ipv4_tcp_checksum;
        update_ipv4_icmp_checksum;
        update_ipv6_udp_checksum;
        update_ipv6_tcp_checksum;
        update_ipv6_icmp_checksum;
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

