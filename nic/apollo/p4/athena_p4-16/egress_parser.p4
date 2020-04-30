parser AthenaEgressParser(packet_in packet,
                         inout cap_phv_intr_global_h intr_global,
                         inout cap_phv_intr_p4_h intr_p4,
                         out headers hdr,
                         out metadata_t metadata) {

  bit<16>     l3_1_hdr_offset;
  bit<16>     l4_1_len;
  bit<16>     l4_1_hdr_offset;
  bit<16>     ip_1_total_len;

  bit<16>     l3_2_hdr_offset;
  bit<16>     l4_2_len;
  bit<16>     l4_2_hdr_offset;
  bit<16>     ip_2_total_len;

  
  bit<8>      tcp_counter;
  bit<16>     icmp_1_hdr_offset;
  bit<16>     icmp_2_hdr_offset;

  
  PensParser()    pensParser;

  bit<8>      options_len;
  bit<1>      pkt_from_host;
  bit<12> default_vlan;
  
  bit <16> geneve_options_len;
  bit <16> geneve_prototype;

  state start {
    //    metadata.cntrl.tm_iport  = intr_global.tm_iport;
    packet.extract(intr_p4);
    transition select(intr_global.tm_iport) {
      default         : parse_egress;
    }
  }

  state parse_egress {
    packet.extract(hdr.p4i_to_p4e_header);
    //    metadata.cntrl.direction = hdr.p4i_to_p4e_header.direction;
    // metadata.cntrl.session_index = hdr.p4i_to_p4e_header.index;

    //  metadata.cntrl.flow_miss = hdr.p4i_to_p4e_header.flow_miss;
    metadata.cntrl.l2_vnic = hdr.p4i_to_p4e_header.l2_vnic;
    /* Skip flow lookup for now for packets injected from ARM */
    transition select(hdr.p4i_to_p4e_header.flow_miss) {
      TRUE : parse_egress_flow_miss;
      default : parse_packet; 
    }
  }

  state parse_egress_flow_miss {
    metadata.cntrl.flow_miss = TRUE;
    transition parse_packet;  
  }


/******************************************************************************
 * Layer 1
 *****************************************************************************/
  state parse_packet {
    packet.extract(hdr.ethernet_1);
    //   metadata.cntrl.vnic_by_vlan_ptr = 1;
    transition select(hdr.ethernet_1.etherType) {
      ETHERTYPE_CTAG : parse_ctag_1;
      ETHERTYPE_IPV4 : parse_ipv4_1;
      ETHERTYPE_IPV6 : parse_ipv6_1;
      default: accept;
    }
  }
  
  state parse_ctag_1 {
    bit<12>  vlan = packet.lookahead<bit<16>>()[11:0];
    transition select(vlan) {
        0 : parse_ctag_1_ptag;
        default: parse_ctag_1_vtag;
    }
  }

  state parse_ctag_1_ptag {
    packet.extract(hdr.ctag_1);
    //  metadata.cntrl.vnic_by_vlan_ptr = 1;
    transition select(hdr.ctag_1.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: accept;
    }
  }

  state parse_ctag_1_vtag {
    packet.extract(hdr.ctag_1);
    //    metadata.cntrl.vnic_by_vlan_ptr = (bit<16>)hdr.ctag_1.vid;
    transition select(hdr.ctag_1.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: accept;
    }
  }

  state parse_ipv4_1 {
    bit<8>  ver_len = packet.lookahead<bit<8>>();
    bit<16> flags_fragoffset = (packet.lookahead<bit<64>>())[15:0];
    transition select(ver_len, flags_fragoffset) {
    (0x45 &&& 0xFF, 0x0 &&& 0x3FFF) : parse_base_ipv4_1;
    (0x44 &&& 0xFF, 0x0 &&& 0x3FFF) : accept;
    (0x40 &&& 0x4C, 0x0 &&& 0x3FFF) : accept;
    (0x40 &&& 0x40, 0x0 &&& 0x3FFF) : parse_ipv4_with_options_1;
    (0x00 &&& 0x00, 0x0 &&& 0x3FFF) : accept;
      default                         : parse_ipv4_fragment_1;
    }
  }

  //ASSUMPTION: ipv4 fragments go to ARM
  state parse_ipv4_fragment_1 {
    packet.extract(hdr.ip_1.ipv4);
//    metadata.cntrl.ip_frag_1 = 1;
    transition accept;
  }
  
  //Assumption: ipv4 options go to ARM
  state parse_ipv4_with_options_1 {
    l3_1_hdr_offset = packet.state_byte_offset();
    //    metadata.cntrl.ip_options_1 = 1;
    packet.extract(hdr.ip_1.ipv4);
    transition accept;
  }

  state parse_base_ipv4_1 {
    l3_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.ip_1.ipv4);
    transition parse_ipv4_checksum_1;
  }
  
  state parse_ipv4_checksum_1 {
    bit<16>     ip_1_header_len;
    
    ip_1_header_len = ((bit<16>) hdr.ip_1.ipv4.ihl << 2);
    ip_1_total_len  = hdr.ip_1.ipv4.totalLen;
    l4_1_len        = ip_1_total_len - ip_1_header_len;
    
    
    transition select(hdr.ip_1.ipv4.protocol) {
      IP_PROTO_ICMP       : parse_icmp_v4_1;
      IP_PROTO_TCP        : parse_tcp_1;
      IP_PROTO_UDP        : parse_udp_1;
      //      IP_PROTO_GRE        : parse_gre_1;
      //      IP_PROTO_IPV4       : parse_ipv4_in_ip_1;
      // IP_PROTO_IPV6       : parse_ipv6_in_ip_1;
      default             : accept;
    }
  }
  

  state parse_ipv4_in_ip_1 {
    //    metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_IP_IN_IP;
    transition parse_ipv4_2;
  }
  
  state parse_ipv6_in_ip_1 {
    //    metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_IP_IN_IP;
    transition parse_ipv6_2;
  }
  
  state parse_ipv6_1 {
    l3_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.ip_1.ipv6);
    
    l4_1_len        =  hdr.ip_1.ipv6.payloadLen;
    

    //   metadata.cntrl.ipv6_ulp_1 = hdr.ip_1.ipv6.nextHdr;
    
    transition select(hdr.ip_1.ipv6.nextHdr) {
      IP_PROTO_ICMPV6 : parse_icmp_v6_1;
      IP_PROTO_TCP    : parse_tcp_1;
      IP_PROTO_UDP    : parse_udp_1;
      IP_PROTO_GRE    : parse_gre_1;
      IP_PROTO_IPV4   : parse_ipv4_in_ip_1;
      IP_PROTO_IPV6   : parse_ipv6_in_ip_1;
      default : accept;
      }
    
   }
  
  state parse_icmp_v4_1 {
    icmp_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.icmpv4);
    metadata.l4.icmp_valid = TRUE;
    
    
    transition accept;
  }

  
  state parse_icmp_v6_1 {
    icmp_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.icmpv6);
    metadata.l4.icmp_valid = TRUE;

    
    transition accept;
  }
  
  state parse_tcp_1 {
    l4_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.tcp);
    metadata.l4.l4_sport_1 = hdr.l4_u.tcp.srcPort;
    metadata.l4.l4_dport_1 = hdr.l4_u.tcp.dstPort;
    
//    pensParser.capture_payload_offset(true);
    tcp_counter = ((bit<8>)(hdr.l4_u.tcp.dataOffset) << 2) - 20;
    //need to define which options are needed for NVme
    transition accept;
    /*    transition select(tcp_counter) {
      0               : accept;
      0x80 &&& 0x80 : parse_tcp_option_error;
      default         : parse_tcp_options_blob;
    }
    */
  }


    state parse_tcp_options_blob {
        pensParser.capture_payload_offset(true);
        packet.no_advance();
        packet.extract_bytes(hdr.tcp_options_blob, (bit<16>)tcp_counter);
        transition parse_tcp_options;
    }

    state parse_tcp_options {
        transition select(tcp_counter, packet.lookahead<bit<8>>()) {
            (0x00 &&& 0xFF, 0x00 &&& 0x00)  : accept;
            (0x80 &&& 0x80, 0x00 &&& 0x00)  : parse_tcp_option_error;
            (0x00 &&& 0x00, 0x00 &&& 0xFF)  : parse_tcp_option_eol;
            (0x00 &&& 0x00, 0x01 &&& 0xFF)  : parse_tcp_option_nop;
            default                         : parse_tcp_multibyte_options;
        }
    }

    state parse_tcp_option_eol {
        packet.extract(hdr.tcp_option_eol);
        tcp_counter = tcp_counter - 1;
        transition select(tcp_counter) {
            0x00    : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_nop {
        packet.extract(hdr.tcp_option_nop);
        tcp_counter = tcp_counter - 1;
        transition select(tcp_counter) {
            0x00    : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_multibyte_options {
        transition select(packet.lookahead<bit<16>>()) {
            0x0204 &&& 0xffff : parse_tcp_option_mss;
            0x0200 &&& 0xff00 : parse_tcp_option_error2;
            0x0303 &&& 0xffff : parse_tcp_option_ws;
            0x0300 &&& 0xff00 : parse_tcp_option_error2;
            0x0402 &&& 0xffff : parse_tcp_option_sack_perm;
            0x0400 &&& 0xff00 : parse_tcp_option_error2;
            0x050a &&& 0xffff : parse_tcp_option_one_sack;
            0x0512 &&& 0xffff : parse_tcp_option_two_sack;
            0x051a &&& 0xffff : parse_tcp_option_three_sack;
            0x0522 &&& 0xffff : parse_tcp_option_four_sack;
            0x0500 &&& 0xff00 : parse_tcp_option_error2;
            0x080a &&& 0xffff : parse_tcp_option_timestamp;
            0x0800 &&& 0xff00 : parse_tcp_option_error2;
            0x0000 &&& 0x00ff : parse_tcp_option_error2;
            default           : parse_tcp_option_unknown;
        }
    }
    state parse_tcp_option_mss {
        packet.extract(hdr.tcp_option_mss);
        tcp_counter = tcp_counter - 4;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_ws {
        packet.extract(hdr.tcp_option_ws);
        tcp_counter = tcp_counter - 3;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_sack_perm {
        packet.extract(hdr.tcp_option_sack_perm);
        tcp_counter = tcp_counter - 2;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_timestamp {
        packet.extract(hdr.tcp_option_timestamp);
        tcp_counter = tcp_counter - 10;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_one_sack {
        packet.extract(hdr.tcp_option_one_sack);
        tcp_counter = tcp_counter - 10;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_two_sack {
        packet.extract(hdr.tcp_option_two_sack);
        tcp_counter = tcp_counter - 18;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_three_sack {
        packet.extract(hdr.tcp_option_three_sack);
        tcp_counter = tcp_counter - 26;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_four_sack {
        packet.extract(hdr.tcp_option_four_sack);
        tcp_counter = tcp_counter - 34;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_unknown {
        bit<8> optLen = (packet.lookahead<bit<16>>())[7:0];
        packet.extract_bytes(hdr.tcp_option_unknown, (bit<16>)optLen);
        tcp_counter = \
            tcp_counter - hdr.tcp_option_unknown.optLength;
        transition select(tcp_counter) {
            0x00 : accept;
            default : parse_tcp_options;
        }
    }

    state parse_tcp_option_error {
        pensParser.capture_payload_offset(false);
        metadata.cntrl.parse_tcp_option_error = 1;
        transition accept;
    }

    state parse_tcp_option_error2 {
        pensParser.capture_payload_offset(false);
        metadata.cntrl.parse_tcp_option_error = 1;
        transition accept;
    }

    state parse_udp_1 {
      packet.extract(hdr.udp);
      l4_1_hdr_offset = packet.state_byte_offset();
      

      metadata.l4.l4_sport_1 = hdr.udp.srcPort;
      metadata.l4.l4_dport_1 = hdr.udp.dstPort;
      
      transition select(hdr.udp.dstPort) {
	//    UDP_PORT_VXLAN      : parse_vxlan_1;
        UDP_PORT_GENV       : parse_geneve_1;
        UDP_PORT_MPLS       : parse_udp_mpls_1;
        default             : accept;
      }
      
    }
 
  state parse_geneve_1 {
    //    metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_GENEVE;
     //TODO
     packet.extract(hdr.geneve_1);
     geneve_options_len = (bit<16>)(hdr.geneve_1.optLen << 3);
     geneve_prototype = hdr.geneve_1.protoType;
     transition select(geneve_options_len) {
        0                    : parse_geneve_ulp;
        default              : parse_geneve_options_blob;
     }
  }
 

  state parse_geneve_options_blob {
     pensParser.capture_payload_offset(true);
     //     packet.no_advance();
     packet.extract_bytes(hdr.geneve_options_blob, geneve_options_len);
     transition parse_geneve_ulp;
    
  }
 
  state parse_geneve_ulp {
     
     transition select(geneve_prototype) {
       ETHERTYPE_ETHERNET   : parse_ethernet_2;
       ETHERTYPE_IPV4       : parse_ipv4_2;
       ETHERTYPE_IPV6       : parse_ipv6_2;
       default              : accept;
     }
  }
  

  state parse_udp_mpls_1 {
    //     metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_UDP_MPLS;
     transition  parse_mpls;
  }
  
  state parse_gre_mpls_1 {
    //   metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_GRE_MPLS;
    transition  parse_mpls;
  }
  
  state parse_gre_1 {
    packet.extract(hdr.gre_1);
#ifdef IPV6_SUPPORT
    //    metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_GRE;
#endif
    
    transition select(hdr.gre_1.C, hdr.gre_1.R, hdr.gre_1.K, hdr.gre_1.S, hdr.gre_1.s,
		      hdr.gre_1.recurse, hdr.gre_1.flags, hdr.gre_1.ver, hdr.gre_1.proto) {
    (0,0,0,0,0,0,0,0,ETHERTYPE_IPV4): parse_gre_ipv4_1;
    (0,0,0,0,0,0,0,0,ETHERTYPE_IPV6): parse_gre_ipv6_1;
    (0,0,0,0,0,0,0,0,ETHERTYPE_MPLS_UNICAST): parse_gre_mpls_1;
    (0,0,0,0,0,0,0,0,ETHERTYPE_MPLS_MULTICAST): parse_gre_mpls_1;
      default: no_gre_tunnel_ingress_1;
    }
  }
  
  state no_gre_tunnel_ingress_1 {
    //    metadata.tunnel.tunnel_type_1 = 0;
    transition accept;
  }
  
  state parse_gre_ipv4_1 {
    // following set_meta causes phv allocation problem for tunnel_type_1 (state flit violation)
    // comment from p4-14, to confirm in p4-16
    // TODO -> to verify if the issue exists in Sorrento
    // metadata.tunnel.tunnel_type_1 =  INGRESS_TUNNEL_TYPE_GRE;
    transition parse_ipv4_2;
  }
  
  state parse_gre_ipv6_1 {
    // comment from p4-14, to confirm in p4-16
    // metadata.tunnel.tunnel_type_1 =  INGRESS_TUNNEL_TYPE_GRE;
    transition parse_ipv6_2;
  }

  /*  
  state parse_vxlan_1 {
    packet.extract(hdr.vxlan_1);
    metadata.tunnel.tunnel_type_1 =  INGRESS_TUNNEL_TYPE_VXLAN;
    metadata.tunnel.tunnel_vni_1 =  hdr.vxlan_1.vni;
    transition parse_ethernet_2;
  }
  */
  
  state parse_mpls {
    bit<1> bos = (packet.lookahead<bit<32>>())[8:8];
    //  packet.extract(hdr.mpls_label1_1);
    //    metadata.cntrl.mpls_vnic_label = hdr.mpls_label1_1.label;
    transition select(bos) {
      0                   : parse_mpls_src;
      default             : parse_mpls_dst;
   }
      
  }
  
  state parse_mpls_src {
    //    bit<1> bos = (packet.lookahead<bit<32>>())[8:8];
    packet.extract(hdr.mpls_src);
    transition parse_mpls_dst;   
    //    metadata.cntrl.mpls_vnic_label = (bit<32>)hdr.mpls_label2_1.label;
    // transition select(bos) {
    //  0                   : parse_mpls_done_1;
    //  1                   : parse_mpls3_1;
    
  }

  state parse_mpls_dst {
    //    bit<1> bos = (packet.lookahead<bit<32>>())[8:8];
    packet.extract(hdr.mpls_dst);
    //    metadata.cntrl.mpls_vnic_label = (bit<32>)hdr.mpls_label2_1.label;
    transition select(hdr.mpls_dst.bos) {
        0                   : parse_mpls3_1;
        default             : parse_mpls_payload;
    }
  }

  state parse_mpls3_1 {
    packet.extract(hdr.mpls_label3_1);   
    transition select(hdr.mpls_label3_1.bos) {
      1                   : parse_mpls_payload;
      0                   : parse_encap_error;
    }
  }

  state parse_encap_error {
    metadata.cntrl.parser_encap_error = 1;
    transition accept;
  }

  
  state parse_mpls_payload {
    bit<4> protocol = packet.lookahead<bit<4>>();
    transition select(protocol){
      0x4 : parse_ipv4_2;
      0x6 : parse_ipv6_2;
      default: parse_ethernet_2;
    }
   
  }

 
 /******************************************************************************
 * Layer 2
 *****************************************************************************/

  state parse_ethernet_2 {
    packet.extract(hdr.ethernet_2);
    transition select(hdr.ethernet_2.etherType) {
      ETHERTYPE_CTAG : parse_ctag_2;
      ETHERTYPE_IPV4 : parse_ipv4_2;
      ETHERTYPE_IPV6 : parse_ipv6_2;
      default: accept;
    }
  }
  
  state parse_ctag_2 {
    packet.extract(hdr.ctag_2);
    transition select(hdr.ctag_2.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_2;
        ETHERTYPE_IPV6 : parse_ipv6_2;
        default: accept;
    }
  }

  state parse_ipv4_2 {
    bit<8>  ver_len = packet.lookahead<bit<8>>();
    bit<16> flags_fragoffset = (packet.lookahead<bit<64>>())[15:0];
    transition select(ver_len, flags_fragoffset) {
    (0x45 &&& 0xFF, 0x0 &&& 0x3FFF) : parse_base_ipv4_2;
    (0x44 &&& 0xFF, 0x0 &&& 0x3FFF) : accept;
    (0x40 &&& 0x4C, 0x0 &&& 0x3FFF) : accept;
    (0x40 &&& 0x40, 0x0 &&& 0x3FFF) : parse_ipv4_with_options_2; 
    (0x00 &&& 0x00, 0x0 &&& 0x3FFF) : accept;
    default                         : parse_ipv4_fragment_2;
    }
  }

  //ASSUMPTION: ipv4 fragments go to ARM
  state parse_ipv4_fragment_2 {
    packet.extract(hdr.ip_2.ipv4);
    //    metadata.cntrl.ip_frag_2 = 1;
    transition accept;
  }
  
  //Assumption: ipv4 options go to ARM
  state parse_ipv4_with_options_2 {
    l3_2_hdr_offset = packet.state_byte_offset();
    //    metadata.cntrl.ip_options_2 = 1;
    packet.extract(hdr.ip_2.ipv4);
    transition accept;
  }

  state parse_base_ipv4_2 {
    l3_2_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.ip_2.ipv4);
    transition parse_ipv4_checksum_2;
  }
  
  state parse_ipv4_checksum_2 {
    bit<16>     ip_2_header_len;
    
    ip_2_header_len = ((bit<16>) hdr.ip_2.ipv4.ihl << 2);
    ip_2_total_len  = hdr.ip_2.ipv4.totalLen;
    l4_2_len        = ip_2_total_len - ip_2_header_len;
    
    
    transition select(hdr.ip_2.ipv4.protocol) {
      IP_PROTO_ICMP       : parse_icmp_v4_1;
      IP_PROTO_TCP        : parse_tcp_1;
      IP_PROTO_UDP        : parse_udp_2;
      default             : accept;
    }
  }
  

  
  state parse_ipv6_2 {
    l3_2_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.ip_2.ipv6);
    
    l4_2_len        =  hdr.ip_2.ipv6.payloadLen;
    
    //   metadata.cntrl.ipv6_ulp_2 = hdr.ip_2.ipv6.nextHdr;
    
    transition select(hdr.ip_2.ipv6.nextHdr) {
      IP_PROTO_ICMPV6 : parse_icmp_v6_1;
      IP_PROTO_TCP    : parse_tcp_1;
      IP_PROTO_UDP    : parse_udp_2;
      default : accept;
      }
    
   }
 
 

  state parse_udp_2 {
    packet.extract(hdr.l4_u.udp);
    l4_2_hdr_offset = packet.state_byte_offset();


    metadata.l4.l4_sport_2 = hdr.l4_u.udp.srcPort;
    metadata.l4.l4_dport_2 = hdr.l4_u.udp.dstPort;
    
    transition accept;
    
    
  }
      
 
}

control AthenaEgressDeparser(packet_out packet,
                            inout cap_phv_intr_global_h intr_global,
                            inout cap_phv_intr_p4_h intr_p4,
                            inout headers hdr,
                            in metadata_t metadata) {
   apply {
       //to arm
        packet.emit(hdr.capri_rxdma_intrinsic);
	//classic nic application
        packet.emit(hdr.p4_to_p4plus_classic_nic);	
        packet.emit(hdr.p4_to_p4plus_classic_nic_ip);
	
        // Packet to uplink - "Push" header - layer 0	
	packet.emit(hdr.ethernet_0);
	packet.emit(hdr.ctag_0);


	packet.emit(hdr.ip_0.ipv4);
	packet.emit(hdr.ip_0.ipv6);
	packet.emit(hdr.gre_0);
	packet.emit(hdr.l4_0.udp);
	packet.emit(hdr.mpls_label1_0);
	packet.emit(hdr.mpls_label2_0);
	packet.emit(hdr.mpls_label3_0);
        packet.emit(hdr.geneve_0);
	packet.emit(hdr.geneve_option_srcSlotId);
	packet.emit(hdr.geneve_option_dstSlotId);
	packet.emit(hdr.geneve_option_srcSecGrpList_1);
	packet.emit(hdr.geneve_option_srcSecGrpList_2);
	packet.emit(hdr.geneve_option_srcSecGrpList_3);
	packet.emit(hdr.geneve_option_origPhysicalIp);
	
	ipv4HdrCsumDepEg_0.update_len(hdr.ip_0.ipv4, metadata.csum.ip_hdr_len_0);
	hdr.ip_0.ipv4.hdrChecksum = ipv4HdrCsumDepEg_0.get();

	packet.emit(hdr.ethernet_1);
	packet.emit(hdr.ctag_1);
	packet.emit(hdr.ip_1.ipv4);
	packet.emit(hdr.ip_1.ipv6);
	
	ipv4HdrCsumDepEg_1.update_len(hdr.ip_1.ipv4, metadata.csum.ip_hdr_len_1);
	hdr.ip_1.ipv4.hdrChecksum = ipv4HdrCsumDepEg_1.get();
	
	packet.emit(hdr.gre_1);
	
	packet.emit(hdr.udp);
	packet.emit(hdr.mpls_src);
	packet.emit(hdr.mpls_dst);
	packet.emit(hdr.mpls_label3_1);
        packet.emit(hdr.geneve_1);
	packet.emit(hdr.geneve_options_blob);
	
       
	
	packet.emit(hdr.ethernet_2);
	packet.emit(hdr.ctag_2);
	packet.emit(hdr.ip_2.ipv4);
	packet.emit(hdr.ip_2.ipv6);
	
	
	
	ipv4HdrCsumDepEg_2.update_len(hdr.ip_2.ipv4, metadata.csum.ip_hdr_len_2);
	hdr.ip_2.ipv4.hdrChecksum = ipv4HdrCsumDepEg_2.get();
     	
   
	udpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_1.ipv4, {hdr.ip_1.ipv4.srcAddr,
	      hdr.ip_1.ipv4.dstAddr, metadata.csum.udp_len_1});
	udpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_1.ipv6, {hdr.ip_1.ipv6.srcAddr,
	      hdr.ip_1.ipv6.dstAddr, metadata.csum.udp_len_1});
	//	udpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_2.ipv4, {hdr.ip_2.ipv4.srcAddr,
	//      hdr.ip_2.ipv4.dstAddr, metadata.csum.udp_len_1});
	// udpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
	//      hdr.ip_2.ipv6.dstAddr, metadata.csum.udp_len_1});
	udpCsumDepEg_1.update_len(hdr.udp, metadata.csum.udp_len_1);
	udpCsumDepEg_1.update_pseudo_hdr_constant(IP_PROTO_UDP);
	hdr.udp.checksum = udpCsumDepEg_1.get();	

        if(hdr.l4_u.icmpv4.isValid()) {
	  packet.emit(hdr.l4_u.icmpv4);
	  icmpv4CsumDepEg_1.update_len(hdr.l4_u.icmpv4, metadata.csum.icmp_len_1);
	  hdr.l4_u.icmpv4.hdrChecksum = icmpv4CsumDepEg_1.get();
	} else if (hdr.l4_u.icmpv6.isValid()) {
	  packet.emit(hdr.l4_u.icmpv6);
	  
	  icmpv6CsumDepEg_1.update_len(hdr.l4_u.icmpv6, metadata.csum.icmp_len_1);
	  icmpv6CsumDepEg_1.update_pseudo_header_fields(hdr.ip_1.ipv6, {hdr.ip_1.ipv6.srcAddr,
		hdr.ip_1.ipv6.dstAddr, metadata.csum.icmp_len_1});
	  icmpv6CsumDepEg_1.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
		hdr.ip_2.ipv6.dstAddr, metadata.csum.icmp_len_1});
	  icmpv6CsumDepEg_1.update_pseudo_hdr_constant(IP_PROTO_ICMPV6);
	  hdr.l4_u.icmpv6.hdrChecksum = icmpv6CsumDepEg_1.get();
	} else if (hdr.l4_u.tcp.isValid()) {
	  packet.emit(hdr.l4_u.tcp);
	  packet.emit(hdr.tcp_options_blob);
	
	  tcpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_1.ipv4, {hdr.ip_1.ipv4.srcAddr,
		hdr.ip_1.ipv4.dstAddr, metadata.csum.tcp_len_1});
	  tcpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_1.ipv6, {hdr.ip_1.ipv6.srcAddr,
		hdr.ip_1.ipv6.dstAddr, metadata.csum.tcp_len_1});
	  tcpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_2.ipv4, {hdr.ip_2.ipv4.srcAddr,
	  	hdr.ip_2.ipv4.dstAddr, metadata.csum.tcp_len_1});
	  tcpCsumDepEg_1.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
	  	hdr.ip_2.ipv6.dstAddr, metadata.csum.tcp_len_1});
	  tcpCsumDepEg_1.update_len(hdr.l4_u.tcp, metadata.csum.tcp_len_1);
	  tcpCsumDepEg_1.update_pseudo_hdr_constant(IP_PROTO_TCP);
	  hdr.l4_u.tcp.checksum = tcpCsumDepEg_1.get();
	} else if (hdr.l4_u.udp.isValid()) {
	  packet.emit(hdr.l4_u.udp);
	  udpCsumDepEg_2.update_pseudo_header_fields(hdr.ip_2.ipv4, {hdr.ip_2.ipv4.srcAddr,
		hdr.ip_2.ipv4.dstAddr, metadata.csum.udp_len_2});
	  udpCsumDepEg_2.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
		hdr.ip_2.ipv6.dstAddr, metadata.csum.udp_len_2});
	  udpCsumDepEg_2.update_len(hdr.l4_u.udp, metadata.csum.udp_len_2);
	  udpCsumDepEg_2.update_pseudo_hdr_constant(IP_PROTO_UDP);
	  hdr.l4_u.udp.checksum = udpCsumDepEg_2.get();	
	  
	}
	
   }
}
