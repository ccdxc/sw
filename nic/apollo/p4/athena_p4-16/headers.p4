#include "app_headers.p4"

header capri_i2e_metadata_h {
    bit<24> info_idx;
    bit<16> rx_pkt_len;
    bit<1>  flow_miss;
    bit<10> vnic_id;
    bit<5>  pad;
    bit<32> _dummy;
}

header p4i_to_p4e_header_h {
    bit<24> l2_index;
    bit<24> index;
    bit<1>  index_type;
    bit<1>  flow_miss;
    bit<1>  direction;
    bit<1>  update_checksum;
    bit<1>  l2_vnic;
    bit<3>  pad1;
    //    bit<1>  pad1;
    bit<16> packet_len;
    bit<16> dnat_epoch;
    bit<32> flow_hash;
}

header ingress_recirc_header_h {
    bit<32> l2_flow_ohash;
    bit<32> flow_ohash;
    bit<32> dnat_ohash;
    bit<4>  pad1;
    bit<1>  direction;
    bit<1>  l2_flow_done;
    bit<1>  flow_done;
    bit<1>  dnat_done;
}

header predicate_header_h {
    bit<7>  pad0;
    bit<1>  direction;
}

header ethernet_h {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

header vlan_h {
    bit<3>  pcp;
    bit<1>  dei;
    bit<12> vid;
    bit<16> etherType;
}

header ipv4_h {
    bit<4>     version;
    bit<4>     ihl;
    bit<8>     diffserv;
    bit<16>    totalLen;
    bit<16>    identification;
    bit<3>     flags;
    bit<13>    fragOffset;
    bit<8>     ttl;
    bit<8>     protocol;
    bit<16>    hdrChecksum;
    bit<32>    srcAddr;
    bit<32>    dstAddr;
}


header mpls_h {
    bit<8> label_b20_b12;
    bit<8> label_b11_b4;
    bit<4>  label_b3_b0;
    bit<3>  exp;
    bit<1>  bos;
    bit<8>  ttl;
}


header mpls_orig_h {
    bit<20> label;
    bit<3>  exp;
    bit<1>  bos;
    bit<8>  ttl;
}


header ipv6_h {
    bit<4>    version;
    bit<8>    trafficClass;
    bit<20>   flowLabel;
    bit<16>   payloadLen;
    bit<8>    nextHdr;
    bit<8>    hopLimit;
    bit<128>  srcAddr;
    bit<128>  dstAddr;
}

/*
header icmp_h {
    bit<8>    icmp_type;
    bit<8>    icmp_code;
    bit<16>   hdrChecksum;
}
*/

header icmp_h {
    bit<16>    icmp_typeCode;
    bit<16>   hdrChecksum;
}

header icmp_echo_h {
    bit<16>   identifier;
    bit<16>   seqNum;
}

header tcp_h {
    bit<16>    srcPort;
    bit<16>    dstPort;
    bit<32>    seqNo;
    bit<32>    ackNo;
    bit<4>     dataOffset;
    bit<4>     res;
    bit<8>     flags;
    bit<16>    window;
    bit<16>    checksum;
    bit<16>    urgentPtr;
}

header tcp_options_blob_h {
    varbit<320>     data;
}

header tcp_option_eol_h {
    bit<8>          optType;
}

header tcp_option_nop_h {
    bit<8>          optType;
}

header tcp_option_mss_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<16>         value;
}

header tcp_option_ws_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<8>          value;
}

header tcp_option_sack_perm_h {
    bit<8>          optType;
    bit<8>          optLength;
}

header tcp_option_timestamp_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<32>         ts;
    bit<32>         prev_echo_ts;
}

header tcp_option_one_sack_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<32>         first_le;
    bit<32>         first_re;
}

header tcp_option_two_sack_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<32>         first_le;
    bit<32>         first_re;
    bit<32>         second_le;
    bit<32>         second_re;
}

header tcp_option_three_sack_h {
    bit<8>          optType;
    bit<8>          optLength;
    bit<32>         second_le;
    bit<32>         second_re;
    bit<32>         third_le;
    bit<32>         third_re;
}

header tcp_option_four_sack_h {
    bit<8>           optType;
    bit<8>           optLength;
    bit<32>          first_le;
    bit<32>          first_re;
    bit<32>          second_le;
    bit<32>          second_re;
    bit<32>          third_le;
    bit<32>          third_re;
    bit<32>          fourth_le;
    bit<32>          fourth_re;
}

header tcp_option_unknown_h {
    bit<8>            optType;
    bit<8>            optLength;
    varbit<320>       data;
}

header udp_h {
    bit<16>    srcPort;
    bit<16>    dstPort;
    bit<16>    len;
    bit<16>    checksum;
}

header udp_payload_h {
    bit<8>          hdr_len;
    varbit<65535>   data;
}

header udp_opt_ocs_h {
    bit<8>   kind;
    bit<8>   chksum;
}

header udp_opt_timestamp_h {
    bit<8>    kind;
    bit<8>    len;
    bit<32>   ts_value;
    bit<32>   ts_echo;
}

header udp_opt_mss_h {
    bit<8>    kind;
    bit<8>    len;
    bit<16>   mss;
}

header udp_opt_nop_h {
    bit<8>    kind;
}

header udp_opt_eol_h {
    bit<8>    kind;
}

header udp_opt_unknown_h {
    bit<8>      kind;
    bit<8>      length;
    varbit<40>  data;
} 

header gre_h {
    bit<1>    C;
    bit<1>    R;
    bit<1>    K;
    bit<1>    S;
    bit<1>    s;
    bit<3>    recurse;
    bit<5>    flags;
    bit<3>    ver;
    bit<16>   proto;
}

header erspan_header_t3_h {
   bit<4>     version;
   bit<12>    vlan;
   bit<6>     priority;
   bit<10>    span_id;
   bit<32>    timestamp;
   bit<16>    sgt;
   bit<16>    ft_d_other;
}

header vxlan_h {
   bit<8>     flags;
   bit<24>    reserved;
   bit<24>    vni;
   bit<8>     reserved2;
}

header geneve_h {
    bit<2>   ver;
    bit<6>   optLen;
    bit<1>   oam;
    bit<1>   critical;
    bit<6>   reserved;
    bit<16>  protoType;
    bit<24>  vni;
    bit<8>   reserved2;
}

header geneve_options_blob_h {
    varbit<252> data;
}

header geneve_options_generic_h {
    // only the fixed portion of a genericoption definition
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
}


header geneve_option_srcSlotId_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<32>     srcSlotId;
}

header geneve_option_dstSlotId_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<32>     dstSlotId;
}

header geneve_option_dstSlotId_split_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<12>     dstSlotId_b32_b21;
    bit<8>      dstSlotId_b20_b12;
    bit<8>      dstSlotId_b11_b4;
    bit<4>      dstSlotId_b3_b0;
}

header geneve_option_srcSecGrpList_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    varbit<96>  srcSecGrpList;
}

header geneve_option_srcSecGrpList_1_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<16>     srcSecGrp0;
    bit<16>     srcSecGrp1;
}

header geneve_option_srcSecGrpList_2_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<16>     srcSecGrp0;
    bit<16>     srcSecGrp1;
    bit<16>     srcSecGrp2;
    bit<16>     srcSecGrp3;
}
header geneve_option_srcSecGrpList_3_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<16>     srcSecGrp0;
    bit<16>     srcSecGrp1;
    bit<16>     srcSecGrp2;
    bit<16>     srcSecGrp3;
    bit<16>     srcSecGrp4;
    bit<16>     srcSecGrp5;
}

header geneve_option_origPhysicalIp_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    bit<32>     origPhysicalIp;
}


header geneve_option_unknown_hdr_h {
    bit<16>     optionClass;
    bit<8>      type;
    bit<3>      res;
    bit<5>      Lenght;
    varbit<256>  optData;
}


header_union ip_1_uh {
    @name(".ipv4_1")    
    ipv4_h  ipv4;
    @name(".ipv6_1")
    ipv6_h  ipv6; 
}

header_union ip_2_uh {
    @name(".ipv4_2")    
    ipv4_h  ipv4;
    @name(".ipv6_2")
    ipv6_h  ipv6; 
}

header_union l4_uh {
  @name(".udp_2")
    udp_h   udp;
    @name(".tcp")    
    tcp_h   tcp;
    //    @name(".icmp")    
    // icmp_h  icmp;
    @name(".icmpv4")    
    icmp_h  icmpv4;
    @name(".icmpv6")    
    icmp_h  icmpv6;
}

// egress only
// header_union p4_to_p4plus_roce_eth_1 ethernet_h
// @pragma pa_header_union xgress ipv4_1 ipv6_1 p4_to_p4plus_roce_ip_1
// @pragma no_ohi xgress udp_1
// @pragma pa_header_union ingress p4_to_p4plus_roce_eth_2
// ethernet_h ethernet_2;

header_union ethernet_uh {
    ethernet_h ethernet;
    ethernet_h p4_to_p4plus_roce_eth;
}

header_union encap_uh {
    udp_h  udp;
    gre_h  gre; 
}

/*
header capri_deparser_len_t {
    bit<16>     trunc_pkt_len;
    bit<16>     icrc_payload_len;
    bit<16>     rx_l4_payload_len;
    bit<16>     tx_l4_payload_len;
    bit<16>     udp_opt_l2_checksum_len;
}
*/
header p4plus_to_p4_h {
    bit<4>  p4plus_app_id;
    bit<4>  pad;
    bit<24> flow_index;
    bit<1>  lkp_inst;
    bit<1>  compute_inner_l4_csum;
    bit<1>  compute_l4_csum;
    bit<1>  insert_vlan_tag;
    bit<1>  update_udp_len;
    bit<1>  update_tcp_seq_no;
    bit<1>  update_ip_len;
    bit<1>  update_ip_id;
    bit<8>  udp_opt_bytes;
    bit<11> dst_lport;
    bit<1>  dst_lport_valid;
    bit<1>  pad1;
    bit<1>  tso_last_segment;
    bit<1>  tso_first_segment;
    bit<1>  tso_valid;
    bit<16> ip_id_delta;
    bit<32> tcp_seq_delta;
    bit<14> gso_start;
    bit<1>  compute_inner_ip_csum;
    bit<1>  compute_ip_csum;
    bit<14> gso_offset;
    bit<1>  flow_index_valid;
    bit<1>  gso_valid;
}

header p4_to_p4plus_classic_nic_h {
    bit<4>  p4plus_app_id;
    bit<1>  table0_valid;
    bit<1>  table1_valid;
    bit<1>  table2_valid;
    bit<1>  table3_valid;
    bit<24> flow_id;
    bit<3>  vlan_pcp;
    bit<1>  vlan_dei;
    bit<12> vlan_vid;
    bit<16> packet_len;
    bit<16> csum;      
    bit<1>  csum_ip_bad;
    bit<1>  csum_ip_ok; 
    bit<1>  csum_udp_bad;
    bit<1>  csum_udp_ok; 
    bit<1>  csum_tcp_bad;
    bit<1>  csum_tcp_ok; 
    bit<1>  vlan_valid;  
    bit<1>  pad;
    bit<2>  l2_pkt_type;
    bit<6>  pkt_type;
    bit<16> l4_sport;
    bit<16> l4_dport;
}

struct headers {
    // Capri Intrinsic header definitions
    cap_phv_intr_global_h capri_intrinsic;
    cap_phv_intr_p4_h capri_p4_intrinsic;
    cap_phv_intr_rxdma_h capri_rxdma_intrinsic;
    cap_phv_intr_txdma_h capri_txdma_intrinsic;
    ingress_recirc_header_h ingress_recirc_header;
    p4i_to_p4e_header_h p4i_to_p4e_header;
    p4_to_p4plus_classic_nic_header_h  p4_to_p4plus_classic_nic;
    p4_to_p4plus_ip_addr_h p4_to_p4plus_classic_nic_ip;
    // layer 0
    ethernet_h ethernet_0;
    vlan_h ctag_0;
    encap_uh encapl4_0;
    ip_1_uh  ip_0;
    l4_uh  l4_0;
    
    vxlan_h vxlan_0;
    gre_h gre_0;
    mpls_orig_h mpls_label1_0;
    mpls_orig_h mpls_label2_0;
    mpls_orig_h mpls_label3_0;
    geneve_h geneve_0;

    erspan_header_t3_h erspan_0;

  // layer 01
    ethernet_h ethernet_01;
    vlan_h ctag_01;
    ip_1_uh ip_01;
    l4_uh  l4_u;
    @name(".udp_1")
    udp_h  udp;
    icmp_echo_h icmp_echo;
    encap_uh encapl4_1;
    geneve_h geneve_1;
    geneve_options_blob_h geneve_options_blob;
    geneve_option_srcSlotId_h geneve_option_srcSlotId;
    geneve_option_dstSlotId_h geneve_option_dstSlotId;
    geneve_option_dstSlotId_split_h geneve_option_dstSlotIdSplit;
  geneve_option_srcSecGrpList_h geneve_option_srcSecGrpList;
  geneve_option_srcSecGrpList_1_h geneve_option_srcSecGrpList_1;
  geneve_option_srcSecGrpList_2_h geneve_option_srcSecGrpList_2;
  geneve_option_srcSecGrpList_3_h geneve_option_srcSecGrpList_3;
  geneve_option_origPhysicalIp_h geneve_option_origPhysicalIp;
  geneve_option_unknown_hdr_h geneve_option_unknown;



//    l4_uh l4_01; 
    vxlan_h vxlan_01;
    gre_h gre_01;
    mpls_h mpls_src;
    mpls_h mpls_dst;
    mpls_h mpls_label3_1;
    erspan_header_t3_h erspan_01;
    tcp_options_blob_h tcp_option_blob;
    tcp_options_blob_h                  tcp_options_blob;
    tcp_option_eol_h                    tcp_option_eol;
    tcp_option_nop_h                    tcp_option_nop;
    tcp_option_mss_h                    tcp_option_mss;
    tcp_option_ws_h                     tcp_option_ws;
    tcp_option_sack_perm_h              tcp_option_sack_perm;
    tcp_option_timestamp_h              tcp_option_timestamp;
    tcp_option_one_sack_h               tcp_option_one_sack;
    tcp_option_two_sack_h               tcp_option_two_sack;
    tcp_option_three_sack_h             tcp_option_three_sack;
    tcp_option_four_sack_h              tcp_option_four_sack;
    tcp_option_unknown_h                tcp_option_unknown;

    // layer 1
  @name(".ethernet_1")
    ethernet_h ethernet_1;
    //ethernet_h ethernet_1;

  @name(".ctag_1")
    vlan_h ctag_1;
    ip_1_uh  ip_1;
//    l4_uh  l4_1;
    vxlan_h vxlan_1;
    gre_h gre_1;
    mpls_h mpls_0_1;
    mpls_h mpls_1_1;
    mpls_h mpls_2_1;
 
    // layer 2
  @name(".ethernet_2")
    ethernet_h  ethernet_2;
    //ethernet_h  ethernet_2;
    vlan_h ctag_2;
    ip_2_uh ip_2;
    l4_uh l4_2;
//    icmp_h icmpv4_2;
//    icmp_h icmpv6_2;




#if 0
    // p4 to p4plus headers
    @synthetic_field("p4_to_p4plus_roce.roce_opt_ts_value", "udp_option_ts.ts_value")
    @synthetic_field("p4_to_p4plus_roce.roce_opt_ts_echo", "udp_option_ts.ts_echo")
    @synthetic_field("p4_to_p4plus_roce.roce_opt_mss", "udp_option_mss.mss")
#endif

    // p4plus to p4 headers
    p4plus_to_p4_h p4plus_to_p4;
  p4plus_to_p4_s2_h   p4plus_to_p4_vlan;
}

/******************************************************************************
 * Header definitions
 *****************************************************************************/


