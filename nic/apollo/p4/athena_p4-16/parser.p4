parser AthenaIngressParser(packet_in packet,
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
    transition select(intr_global.tm_iport) {
    TM_PORT_DMA      : parse_txdma_to_ingress;
    TM_PORT_INGRESS  : parse_ingress_recirc_header;      
      default         : parse_uplink;
    }
  }

  state parse_txdma_to_ingress {
    metadata.cntrl.from_arm = TRUE;
    transition parse_txdma_to_ingress_split;
  }
  
  state parse_txdma_to_ingress_split {
    packet.extract(hdr.capri_txdma_intrinsic);
    packet.extract(hdr.p4plus_to_p4);
    packet.extract(hdr.p4plus_to_p4_vlan);    
    /* Skip flow lookup for now for packets injected from ARM */
    metadata.cntrl.skip_flow_lkp = TRUE;
    metadata.cntrl.skip_l2_flow_lkp = TRUE;

    transition select((bit<1>)hdr.p4plus_to_p4.gso_valid, (bit<4>)hdr.p4plus_to_p4.p4plus_app_id) {
    ( 0x1 &&& 0x1, 0x0 &&& 0x0) : parse_txdma_gso;      
    ( 0x0 &&& 0x1, P4PLUS_APPTYPE_CPU) : parse_packet;
      default : parse_packet;
    }
  }

  
  
state parse_txdma_gso {
    transition parse_packet;
}


 state parse_cpu_packet {
    transition parse_packet;  /* TODO: Assumption made that the direction information is derived from the NACL */
  }

  state parse_ingress_recirc_header {
    packet.extract(intr_p4);
    packet.extract(hdr.ingress_recirc_header);
    transition select(hdr.ingress_recirc_header.direction) {
       RX_FROM_SWITCH : parse_packet_from_switch;
       TX_FROM_HOST : parse_packet_from_host;
    }  
  }

  /* 
  state parse_ingress_pass2 {
    packet.extract(hdr.predicate_header);
    transition select(hdr.predicate_header.direction) {
        RX_FROM_SWITCH : parse_packet_from_switch;
        TX_FROM_HOST : parse_packet_from_host;
    }
  }
  */
  state parse_uplink {
    transition select(intr_global.tm_iport) {
        UPLINK_HOST : parse_packet_from_host;
        UPLINK_SWITCH : parse_packet_from_switch;
        default : parse_packet;
    }    
  }
  
  state parse_packet_from_host {
    metadata.cntrl.direction = TX_FROM_HOST;
    //   metadata.cntrl.uplink  = TRUE;
    transition parse_packet;
  }
  
  state parse_packet_from_switch {
    metadata.cntrl.direction = RX_FROM_SWITCH;
    //    metadata.cntrl.uplink  = TRUE;
    transition parse_packet;
  }

/******************************************************************************
 * Layer 1
 *****************************************************************************/
  state parse_packet {
    packet.extract(hdr.ethernet_1);
    //    metadata.cntrl.vnic_by_vlan_ptr_l = (bit<8>)1;
    metadata.offset.l2_1 = packet.state_byte_offset()[7:0];
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
    //    metadata.cntrl.vnic_by_vlan_ptr = (bit<16>)1;
    transition select(hdr.ctag_1.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: accept;
    }
  }

  state parse_ctag_1_vtag {
    packet.extract(hdr.ctag_1);
    //    metadata.cntrl.vnic_by_vlan_ptr_h = (bit<8>)hdr.ctag_1.vid[11:8];
    // metadata.cntrl.vnic_by_vlan_ptr_l = (bit<8>)hdr.ctag_1.vid[7:0];
    
    transition select(hdr.ctag_1.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: accept;
    }
  }

  state parse_ipv4_1 {
    bit<8>  ver_len = packet.lookahead<bit<8>>();
    bit<16> flags_fragoffset = (packet.lookahead<bit<64>>())[15:0];
    metadata.offset.l3_1 = packet.state_byte_offset()[7:0];    
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

    
    tcpCsum_1.update_pseudo_header_offset(hdr.ip_1.ipv4, l3_1_hdr_offset);
    udpCsum_1.update_pseudo_header_offset(hdr.ip_1.ipv4, l3_1_hdr_offset);
    tcpCsum_1.update_pseudo_header_fields(hdr.ip_1.ipv4,
					{hdr.ip_1.ipv4.srcAddr, hdr.ip_1.ipv4.dstAddr, l4_1_len});
    tcpCsum_1.update_pseudo_hdr_constant(IP_PROTO_TCP);
    udpCsum_1.update_pseudo_header_fields(hdr.ip_1.ipv4,
					{hdr.ip_1.ipv4.srcAddr, hdr.ip_1.ipv4.dstAddr, l4_1_len});
    
    udpCsum_1.update_pseudo_hdr_constant(IP_PROTO_UDP);
    /*
    icmpv4Csum_1.update_pseudo_header_offset(hdr.ip_1.ipv4, l3_1_hdr_offset);
    icmpv4Csum_1.update_pseudo_header_fields(hdr.ip_1.ipv4,
					     {hdr.ip_1.ipv4.srcAddr, hdr.ip_1.ipv4.dstAddr, l4_1_len});
    icmpv4Csum_1.update_pseudo_hdr_constant(IP_PROTO_ICMP);
    */

    ipv4HdrCsum_1.update_len(l3_1_hdr_offset, ip_1_header_len);
    ipv4HdrCsum_1.validate(hdr.ip_1.ipv4.hdrChecksum);
    
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
    //   metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_IP_IN_IP;
    transition parse_ipv4_2;
  }
  
  state parse_ipv6_in_ip_1 {
    //   metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_IP_IN_IP;
    transition parse_ipv6_2;
  }
  
  state parse_ipv6_1 {
    l3_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.ip_1.ipv6);
    
    l4_1_len        =  hdr.ip_1.ipv6.payloadLen;
  
  
    tcpCsum_1.update_pseudo_header_offset(hdr.ip_1.ipv6, l3_1_hdr_offset);
    udpCsum_1.update_pseudo_header_offset(hdr.ip_1.ipv6, l3_1_hdr_offset);
    tcpCsum_1.update_pseudo_header_fields(hdr.ip_1.ipv6,
					{hdr.ip_1.ipv6.srcAddr, hdr.ip_1.ipv6.dstAddr, l4_1_len});
    udpCsum_1.update_pseudo_header_fields(hdr.ip_1.ipv6,
    					{hdr.ip_1.ipv6.srcAddr, hdr.ip_1.ipv6.dstAddr, l4_1_len});
    
    tcpCsum_1.update_pseudo_hdr_constant(IP_PROTO_TCP);
    udpCsum_1.update_pseudo_hdr_constant(IP_PROTO_UDP);

    
    icmpv6Csum_1.update_pseudo_header_offset(hdr.ip_1.ipv6, l3_1_hdr_offset);
    icmpv6Csum_1.update_pseudo_header_fields(hdr.ip_1.ipv6,
					     {hdr.ip_1.ipv6.srcAddr, hdr.ip_1.ipv6.dstAddr, l4_1_len});
    icmpv6Csum_1.update_pseudo_hdr_constant(IP_PROTO_ICMPV6);
    
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
    metadata.l4.l4_dport_1 = hdr.l4_u.icmpv4.icmp_typeCode;
    metadata.l4.icmp_valid = TRUE;
    
    
    icmpv4Csum_1.update_len(icmp_1_hdr_offset, l4_1_len);
    icmpv4Csum_1.validate(hdr.l4_u.icmpv4.hdrChecksum);
    
    transition select(hdr.l4_u.icmpv4.icmp_typeCode) {
        ICMP_ECHO_REQ_TYPE_CODE : parse_icmp_echo_1;
        ICMP_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_1;
        default : accept;
       
    }
  }

  

  state parse_icmp_v6_1 {
    icmp_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.icmpv6);
    metadata.l4.l4_dport_1 = hdr.l4_u.icmpv6.icmp_typeCode;
    metadata.l4.icmp_valid = TRUE;


    //    icmpv6Csum_1.update_pseudo_header_offset(hdr.ip_1.ipv6, l3_1_hdr_offset);
    //icmpv6Csum_1.update_pseudo_header_fields(hdr.ip_1.ipv6,
    // 					{hdr.ip_1.ipv6.srcAddr, hdr.ip_1.ipv6.dstAddr, l4_1_len});
    // icmpv6Csum_1.update_pseudo_hdr_constant(IP_PROTO_ICMPV6);

    icmpv6Csum_1.update_len(icmp_1_hdr_offset, l4_1_len);
    icmpv6Csum_1.validate(hdr.l4_u.icmpv6.hdrChecksum);
    
    transition select(hdr.l4_u.icmpv6.icmp_typeCode) {
        ICMP6_ECHO_REQ_TYPE_CODE : parse_icmp_echo_1;
        ICMP6_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_1;
        default : accept;
       
    }
  }

  state parse_icmp_echo_1 {
    packet.extract(hdr.icmp_echo);
    metadata.l4.l4_sport_1 = hdr.icmp_echo.identifier; 
    transition accept;
   
  }
  
  state parse_tcp_1 {
    l4_1_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.tcp);
    metadata.l4.l4_sport_1 = hdr.l4_u.tcp.srcPort;
    metadata.l4.l4_dport_1 = hdr.l4_u.tcp.dstPort;

    // Pseudo header fields
    tcpCsum_1.update_len(l4_1_hdr_offset, l4_1_len); // l4_len should be OHI since capri target requires it
    tcpCsum_1.validate(hdr.l4_u.tcp.checksum);
    
    pensParser.capture_payload_offset(true);
    tcp_counter = ((bit<8>)(hdr.l4_u.tcp.dataOffset) << 2) - 20;
    //need to define which options are needed for NVme
    transition select(tcp_counter) {
      0               : accept;
      0x80 &&& 0x80 : parse_tcp_option_error;
      default         : parse_tcp_options_blob;
    }
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

    //   udpCsum_1.update_pseudo_header_offset(hdr.ip_1.ipv4, l3_1_hdr_offset);
    // udpCsum_1.update_pseudo_header_fields(hdr.ip_1.ipv4,
    //					{hdr.ip_1.ipv4.srcAddr, hdr.ip_1.ipv4.dstAddr, hdr.ip_1.ipv4.protocol, l4_1_len});
   
    // Pseudo header fields
    udpCsum_1.update_len(l4_1_hdr_offset, l4_1_len); // l4_len should be OHI since capri target requires it
    udpCsum_1.validate(hdr.udp.checksum);

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
     packet.no_advance();
     packet.extract_bytes(hdr.geneve_options_blob, geneve_options_len);
     transition parse_geneve_options;
    
  }
  
  state parse_geneve_options {
    bit<8> opt_type = packet.lookahead<bit<24>>()[7:0];
    //    bit<8> opt_len = (bit<8>)packet.lookahead<bit<32>>()[4:0];
    transition select(geneve_options_len, opt_type) {
      (0x00 &&& 0xFF, 0x00 &&& 0x00)              : parse_geneve_ulp;
      (0x00 &&& 0x00, GENEVE_OPTION_SRC_SLOT_ID ) : parse_geneve_option_src_slot_id; 
      (0x00 &&& 0x00, GENEVE_OPTION_DST_SLOT_ID ) : parse_geneve_option_dst_slot_id; 
      (0x00 &&& 0x00, GENEVE_OPTION_ORIGINATOR_PHYSICAL_IP ) : parse_geneve_option_originator_physical_ip; 
      (0x00 &&& 0x00, GENEVE_OPTION_SRC_SECURITY_GRP_LIST &&& 0xfe ) : parse_geneve_option_src_security_grp_list;
      default : parse_geneve_option_unknown;
      //      (0x00 &&& 0x00, GENEVE_OPTION_SRC_SECURITY_GRP_LIST_EVEN ) : parse_geneve_option_src_security_grp_list_even;
      //      (0x00 &&& 0x00, GENEVE_OPTION_SRC_SECURITY_GRP_LIST_ODD ) : parse_geneve_option_src_security_grp_list_odd;

    }   
  }
  
  state parse_geneve_option_src_slot_id {
    packet.extract(hdr.geneve_option_srcSlotId);
    geneve_options_len = geneve_options_len - 8;
    transition select(geneve_options_len) {
    0x00 : parse_geneve_ulp;
       default : parse_geneve_options;
    }
  }

  state parse_geneve_option_dst_slot_id {
    bit<8> dstSlotId_b3_b0 = (bit<8>)(packet.lookahead<bit<32>>()[3:0]);
    packet.extract(hdr.geneve_option_dstSlotIdSplit);
    //    metadata.cntrl.mpls_vnic_label = hdr.geneve_option_dstSlotIdSplit.dstSlotId_b20_b0;

    metadata.cntrl.mpls_label_b20_b12 = hdr.geneve_option_dstSlotIdSplit.dstSlotId_b20_b12;
    metadata.cntrl.mpls_label_b11_b4 = hdr.geneve_option_dstSlotIdSplit.dstSlotId_b11_b4;
    metadata.cntrl.mpls_label_b3_b0 = hdr.geneve_option_dstSlotIdSplit.dstSlotId_b3_b0;
    geneve_options_len = geneve_options_len - 8;
    transition select(geneve_options_len) {
        0x00 :  parse_geneve_ulp;
       default : parse_geneve_options;
    }
  }


  state parse_geneve_option_originator_physical_ip {
    packet.extract(hdr.geneve_option_origPhysicalIp);
    geneve_options_len = geneve_options_len - 8;
    transition select(geneve_options_len) {
    0x00 : parse_geneve_ulp;
       default : parse_geneve_options;
    }
  }

  state parse_geneve_option_src_security_grp_list {
    bit<8> src_sec_grp_list_opt_len = (bit<8>)(packet.lookahead<bit<32>>()[4:0]) << 3; 
    packet.extract_bytes(hdr.geneve_option_srcSecGrpList, (bit<16>)src_sec_grp_list_opt_len);
    geneve_options_len = geneve_options_len - (bit<16>)src_sec_grp_list_opt_len;
    transition select(geneve_options_len) {
    0x00 : parse_geneve_ulp;
       default : parse_geneve_options;
    }
  }

  state parse_geneve_option_unknown {
    bit<8> unk_opt_len = (bit<8>)(packet.lookahead<bit<32>>()[4:0]) << 3;
    packet.extract_bytes(hdr.geneve_option_unknown, (bit<16>)unk_opt_len);
    geneve_options_len = geneve_options_len - (bit<16>)unk_opt_len;
    transition select(geneve_options_len) {
    0x00 : parse_geneve_ulp;
       default : parse_geneve_options;
    }
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
    //metadata.tunnel.tunnel_type_1 = INGRESS_TUNNEL_TYPE_GRE;
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
    //  metadata.tunnel.tunnel_type_1 = 0;
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
    //    packet.extract(hdr.mpls_label1_1);
    //  metadata.cntrl.mpls_vnic_label = (bit<32>)hdr.mpls_label1_1.label;
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
    //    metadata.cntrl.mpls_label_b20_b4 = hdr.mpls_dst.label_b20_b4;
    // metadata.cntrl.mpls_label_b3_b0 = (bit<8>)hdr.mpls_dst.label_b3_b0;
        metadata.cntrl.mpls_label_b20_b12 = hdr.mpls_dst.label_b20_b12;
        metadata.cntrl.mpls_label_b11_b4 = hdr.mpls_dst.label_b11_b4;
        metadata.cntrl.mpls_label_b3_b0 = hdr.mpls_dst.label_b3_b0;
    // metadata.cntrl.mpls_vnic_label = hdr.mpls_dst.label;
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
    metadata.offset.l2_2 = packet.state_byte_offset()[7:0];    
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
    metadata.offset.l3_2 = packet.state_byte_offset()[7:0];    
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
    
    tcpCsum_2.update_pseudo_header_offset(hdr.ip_2.ipv4, l3_2_hdr_offset);
    udpCsum_2.update_pseudo_header_offset(hdr.ip_2.ipv4, l3_2_hdr_offset);
    tcpCsum_2.update_pseudo_header_fields(hdr.ip_2.ipv4,
					{hdr.ip_2.ipv4.srcAddr, hdr.ip_2.ipv4.dstAddr, l4_2_len});
    udpCsum_2.update_pseudo_header_fields(hdr.ip_2.ipv4,
					{hdr.ip_2.ipv4.srcAddr, hdr.ip_2.ipv4.dstAddr, l4_2_len});
    
    tcpCsum_2.update_pseudo_hdr_constant(IP_PROTO_TCP);
    udpCsum_2.update_pseudo_hdr_constant(IP_PROTO_UDP);
    ipv4HdrCsum_2.update_len(l3_2_hdr_offset, ip_2_header_len);
    ipv4HdrCsum_2.validate(hdr.ip_2.ipv4.hdrChecksum);
    
    transition select(hdr.ip_2.ipv4.protocol) {
      IP_PROTO_ICMP       : parse_icmp_v4_2;
      IP_PROTO_TCP        : parse_tcp_2;
      IP_PROTO_UDP        : parse_udp_2;
      default             : accept;
    }
  }
  

  
  state parse_ipv6_2 {
    l3_2_hdr_offset = packet.state_byte_offset();
    metadata.offset.l3_2 = packet.state_byte_offset()[7:0];    
    packet.extract(hdr.ip_2.ipv6);
    
    l4_2_len        =  hdr.ip_2.ipv6.payloadLen;
    
    tcpCsum_2.update_pseudo_header_offset(hdr.ip_2.ipv6, l3_2_hdr_offset);
    udpCsum_2.update_pseudo_header_offset(hdr.ip_2.ipv6, l3_2_hdr_offset);
    tcpCsum_2.update_pseudo_header_fields(hdr.ip_2.ipv6,
					{hdr.ip_2.ipv6.srcAddr, hdr.ip_2.ipv6.dstAddr, l4_2_len});
    udpCsum_2.update_pseudo_header_fields(hdr.ip_2.ipv6,
					{hdr.ip_2.ipv6.srcAddr, hdr.ip_2.ipv6.dstAddr, l4_2_len});
    
    tcpCsum_2.update_pseudo_hdr_constant(IP_PROTO_TCP);
    udpCsum_2.update_pseudo_hdr_constant(IP_PROTO_UDP);

    icmpv6Csum_2.update_pseudo_header_offset(hdr.ip_2.ipv6, l3_2_hdr_offset);
    icmpv6Csum_2.update_pseudo_header_fields(hdr.ip_2.ipv6,
					     {hdr.ip_2.ipv6.srcAddr, hdr.ip_2.ipv6.dstAddr, l4_2_len});
    icmpv6Csum_2.update_pseudo_hdr_constant(IP_PROTO_ICMPV6); //

    //   metadata.cntrl.ipv6_ulp_2 = hdr.ip_2.ipv6.nextHdr;
    
    transition select(hdr.ip_2.ipv6.nextHdr) {
      IP_PROTO_ICMPV6 : parse_icmp_v6_2;
      IP_PROTO_TCP    : parse_tcp_2;
      IP_PROTO_UDP    : parse_udp_2;
      default : accept;
      }
    
   }
  
    
 state parse_icmp_v4_2 {
    icmp_2_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.icmpv4);
    metadata.l4.l4_dport_2 = hdr.l4_u.icmpv4.icmp_typeCode;
    metadata.l4.icmp_valid = TRUE;
   

    icmpv4Csum_2.update_len(icmp_2_hdr_offset, l4_2_len);
    icmpv4Csum_2.validate(hdr.l4_u.icmpv4.hdrChecksum);
    
    transition select(hdr.l4_u.icmpv4.icmp_typeCode) {
        ICMP_ECHO_REQ_TYPE_CODE : parse_icmp_echo_2;
        ICMP_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_2;
        default : accept;
       
    }
  }

  
  state parse_icmp_v6_2 {
    icmp_2_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.icmpv6);
    metadata.l4.l4_dport_2 = hdr.l4_u.icmpv6.icmp_typeCode;
    metadata.l4.icmp_valid = TRUE;

    icmpv6Csum_2.update_len(icmp_2_hdr_offset, l4_2_len);
    icmpv6Csum_2.validate(hdr.l4_u.icmpv6.hdrChecksum);
    
    transition select(hdr.l4_u.icmpv6.icmp_typeCode) {
        ICMP6_ECHO_REQ_TYPE_CODE : parse_icmp_echo_2;
        ICMP6_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_2;
        default : accept;
       
    }

  }
  
 
  state parse_icmp_echo_2 {
    packet.extract(hdr.icmp_echo);
    metadata.l4.l4_sport_2 = hdr.icmp_echo.identifier; 
    transition accept;
  }

  state parse_tcp_2 {
    l4_2_hdr_offset = packet.state_byte_offset();
    packet.extract(hdr.l4_u.tcp);
    metadata.l4.l4_sport_2 = hdr.l4_u.tcp.srcPort;
    metadata.l4.l4_dport_2 = hdr.l4_u.tcp.dstPort;

    // Pseudo header fields
    tcpCsum_2.update_len(l4_2_hdr_offset, l4_2_len); // l4_len should be OHI since capri target requires it
    tcpCsum_2.validate(hdr.l4_u.tcp.checksum);
    
    pensParser.capture_payload_offset(true);
    tcp_counter = ((bit<8>)(hdr.l4_u.tcp.dataOffset) << 2) - 20;
    //need to define which options are needed for NVme
    transition select(tcp_counter) {
      0               : accept;
      0x80 &&& 0x80 : parse_tcp_option_error;
      default         : parse_tcp_options_blob;
    }
  }
 

  state parse_udp_2 {
    packet.extract(hdr.l4_u.udp);
    l4_2_hdr_offset = packet.state_byte_offset();

    // Pseudo header fields
    udpCsum_2.update_len(l4_2_hdr_offset, l4_2_len); // l4_len should be OHI since capri target requires it
    udpCsum_2.validate(hdr.l4_u.udp.checksum);

    metadata.l4.l4_sport_2 = hdr.l4_u.udp.srcPort;
    metadata.l4.l4_dport_2 = hdr.l4_u.udp.dstPort;
    
    transition accept;
    
    
  }
      
 
}

control AthenaIngressDeparser(packet_out packet,
                            inout cap_phv_intr_global_h intr_global,
                            inout cap_phv_intr_p4_h intr_p4,
                            inout headers hdr,
                            in metadata_t metadata) {
   apply {
        packet.emit(hdr.ingress_recirc_header);	
        packet.emit(hdr.p4i_to_p4e_header);
        packet.emit(hdr.ethernet_1);
        packet.emit(hdr.ctag_1);
        packet.emit(hdr.ip_1.ipv4);
        packet.emit(hdr.ip_1.ipv6);
        packet.emit(hdr.gre_1);
	//	packet.emit(hdr.l4_1.icmp);
	//	packet.emit(hdr.l4_1.tcp);
	//       packet.emit(hdr.tcp_options_blob);
        packet.emit(hdr.udp);
        packet.emit(hdr.mpls_src);
        packet.emit(hdr.mpls_dst);
        packet.emit(hdr.mpls_label3_1);
        packet.emit(hdr.geneve_1);
	packet.emit(hdr.geneve_options_blob);

	/*
        ipv4HdrCsumDep_1.update_len(hdr.ip_1.ipv4, metadata.cntrl.ip_hdr_len_1);
        hdr.ip_1.ipv4.hdrChecksum = ipv4HdrCsumDep_1.get();

        tcpCsumDep_1.update_pseudo_header_fields(hdr.ip_1.ipv4, {hdr.ip_1.ipv4.srcAddr,
               hdr.ip_1.ipv4.dstAddr, hdr.ip_1.ipv4.protocol, metadata.cntrl.tcp_len_1});
        tcpCsumDep_1.update_pseudo_header_fields(hdr.ip_1.ipv6, {hdr.ip_1.ipv6.srcAddr,
               hdr.ip_1.ipv6.dstAddr, hdr.ip_1.ipv6.nextHdr, metadata.cntrl.tcp_len_1});
        tcpCsumDep_1.update_len(hdr.l4_1.tcp, metadata.cntrl.tcp_len_1);
        hdr.l4_1.tcp.checksum = tcpCsumDep_1.get();

        udpCsumDep_1.update_pseudo_header_fields(hdr.ip_1.ipv4, {hdr.ip_1.ipv4.srcAddr,
               hdr.ip_1.ipv4.dstAddr, hdr.ip_1.ipv4.protocol, metadata.cntrl.udp_len_1});
        udpCsumDep_1.update_pseudo_header_fields(hdr.ip_1.ipv6, {hdr.ip_1.ipv6.srcAddr,
               hdr.ip_1.ipv6.dstAddr, hdr.ip_1.ipv6.nextHdr, metadata.cntrl.udp_len_1});
        udpCsumDep_1.update_len(hdr.l4_1.udp, metadata.cntrl.udp_len_1);
        udpCsumDep_1.include_checksum_result(hdr.ip_2.ipv4);
        udpCsumDep_1.include_checksum_result(hdr.l4_2.udp);
        udpCsumDep_1.include_checksum_result(hdr.l4_2.tcp); // Inner TCP
        hdr.l4_1.udp.checksum = udpCsumDep_1.get();

        icmpCsumDep_1.update_len(hdr.l4_1.icmp, metadata.cntrl.icmp_len_1);
        hdr.l4_1.icmp.hdrChecksum = icmpCsumDep_1.get();
	*/
	
        packet.emit(hdr.ethernet_2);
        packet.emit(hdr.ctag_2);
        packet.emit(hdr.ip_2.ipv4);
        packet.emit(hdr.ip_2.ipv6);
	packet.emit(hdr.l4_u.icmpv4);
	packet.emit(hdr.l4_u.icmpv6);
	packet.emit(hdr.icmp_echo);
        packet.emit(hdr.l4_u.tcp);
        packet.emit(hdr.l4_u.udp);
	packet.emit(hdr.tcp_options_blob);	

	/*	
        ipv4HdrCsumDep_2.update_len(hdr.ip_2.ipv4, metadata.cntrl.ip_hdr_len_2);
        hdr.ip_2.ipv4.hdrChecksum = ipv4HdrCsumDep_2.get();

        tcpCsumDep_2.update_pseudo_header_fields(hdr.ip_2.ipv4, {hdr.ip_2.ipv4.srcAddr,
               hdr.ip_2.ipv4.dstAddr, hdr.ip_2.ipv4.protocol, metadata.cntrl.tcp_len_2});
        tcpCsumDep_2.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
               hdr.ip_2.ipv6.dstAddr, hdr.ip_2.ipv6.nextHdr, metadata.cntrl.tcp_len_2});
        tcpCsumDep_2.update_len(hdr.l4_2.tcp, metadata.cntrl.tcp_len_2);
        hdr.l4_2.tcp.checksum = tcpCsumDep_2.get();

        udpCsumDep_2.update_pseudo_header_fields(hdr.ip_2.ipv4, {hdr.ip_2.ipv4.srcAddr,
               hdr.ip_2.ipv4.dstAddr, hdr.ip_2.ipv4.protocol, metadata.cntrl.udp_len_2});
        udpCsumDep_2.update_pseudo_header_fields(hdr.ip_2.ipv6, {hdr.ip_2.ipv6.srcAddr,
               hdr.ip_2.ipv6.dstAddr, hdr.ip_2.ipv6.nextHdr, metadata.cntrl.udp_len_2});
        udpCsumDep_2.update_len(hdr.l4_2.udp, metadata.cntrl.udp_len_2);
        hdr.l4_2.udp.checksum = udpCsumDep_2.get();

        icmpCsumDep_2.update_len(hdr.l4_2.icmp, metadata.cntrl.icmp_len_2);
        hdr.l4_2.icmp.hdrChecksum = icmpCsumDep_2.get();
	*/
   }
}
