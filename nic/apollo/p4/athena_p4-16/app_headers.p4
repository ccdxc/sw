header p4_to_p4plus_ipsec_header_h {
    bit<4>  p4plus_app_id;
    bit<1>  table0_valid;
    bit<1>  table1_valid;
    bit<1>  table2_valid;
    bit<1>  table3_valid;
    bit<24> flow_id;
    bit<16> ipsec_payload_start;
    bit<16> ipsec_payload_end;
    bit<8>  l4_protocol;
    bit<8>  ip_hdr_size;
    bit<32> seq_no;
    bit<32> spi;
}

header p4_to_p4plus_p4pt_header_h {
    bit<4>  p4plus_app_id;
    bit<1>  table0_valid;
    bit<1>  table1_valid;
    bit<1>  table2_valid;
    bit<1>  table3_valid;
    bit<24> p4pt_idx;
    bit<16> payload_len;
    bit<6>  pad;
    bit<1>  flow_role;
    bit<1>  flow_dir;
}

header p4_to_p4plus_roce_header_h {
    bit<4>  p4plus_app_id;
    bit<1>  table0_valid;
    bit<1>  table1_valid;
    bit<1>  table2_valid;
    bit<1>  table3_valid;
    bit<1>  roce_opt_ts_valid;
    //Parsed UDP options valid flags
    bit<1>  roce_opt_mss_valid;
    bit<6>  rdma_hdr_len; // copied directly from p4 rdma table
    bit<16> raw_flags; // copied directly from p4 rdma table
    bit<2>  ecn;
    bit<14> payload_len;
    //Parsed UDP options
    bit<32> roce_opt_ts_value;
    bit<32> roce_opt_ts_echo;
    bit<16> roce_opt_mss;
    bit<16> roce_opt_pad;
    bit<16> roce_int_recirc_hdr;
}

header p4_to_p4plus_tcp_proxy_base_header_h {
    bit<4>    p4plus_app_id;
    bit<1>    table0_valid;
    bit<1>    table1_valid;
    bit<1>    table2_valid;
    bit<1>    table3_valid;
    bit<1>    from_ooq_txdma;
    bit<1>    pad;
    bit<2>    ecn;
    bit<1>    timestamp_valid;
    bit<3>    num_sack_blocks;
    bit<16>   payload_len;
    bit<16>   srcPort;
    bit<16>   dstPort;
    bit<32>   seqNo;
    bit<32>   ackNo;
    bit<4>    dataOffset;
    bit<4>    res;
    bit<8>    flags;
    bit<16>   window;
    bit<16>   urgentPtr;
    bit<32>   ts;
    bit<32>   prev_echo_ts;
}

header p4_to_p4plus_tcp_proxy_sack_header_h {
    bit<32>    start_seq0;
    bit<32>    end_seq0;
    bit<32>    start_seq1;
    bit<32>    end_seq1;
    bit<32>    start_seq2;
    bit<32>    end_seq2;
    bit<32>    start_seq3;
    bit<32>    end_seq3;
}

header p4_to_p4plus_classic_nic_header_h {
    bit<4>    p4plus_app_id;
    bit<1>    table0_valid;
    bit<1>    table1_valid;
    bit<1>    table2_valid;
    bit<1>    table3_valid;
    bit<24>   flow_id;
    bit<3>    vlan_pcp;
    bit<1>    vlan_dei;
    bit<12>   vlan_vid;
    bit<16>   packet_len;
    bit<16>   csum;
    bit<1>    csum_ip_bad;
    bit<1>    csum_ip_ok;
    bit<1>    csum_udp_bad;
    bit<1>    csum_udp_ok;
    bit<1>    csum_tcp_bad;
    bit<1>    csum_tcp_ok;
    bit<1>    vlan_valid;
    bit<1>    pad;
    bit<2>    l2_pkt_type;
    bit<6>    pkt_type;
    bit<16>   l4_sport;
    bit<16>   l4_dport;
}

header p4_to_p4plus_ip_addr_h {
    bit<128>  ip_sa;
    bit<128>  ip_da;
}

header  p4_to_p4plus_cpu_header_h {
    bit<4>    p4plus_app_id;
    bit<1>    table0_valid;
    bit<1>    table1_valid;
    bit<1>    table2_valid;
    bit<1>    table3_valid;
    bit<8>    ip_proto;
    bit<16>   l4_sport;
    bit<16>   l4_dport;
    bit<2>    packet_type;
    bit<14>   packet_len;
}

/*
 * flags bits:
 *  0       : vlan valid
 *  1       : ipv4 valid
 *  2       : ipv6 valid
 *  3       : ip options present
 *  4       : tcp options present
 *  5       : from ipsec app
 *  6-7     : unused
 */
header   p4_to_p4plus_cpu_pkt_h {
    bit<16>    src_lif;
    bit<16>    lif;
    bit<8>     qtype;
    bit<32>    qid;
    bit<16>    lkp_vrf;
    bit<4>     src_app_id;
    bit<1>     lkp_dir;
    bit<1>     lkp_inst;
    bit<2>     lkp_type;
    bit<8>     flags;
    // offsets
    bit<16>    l2_offset;
    bit<16>    l3_offset;
    bit<16>    l4_offset;
    bit<16>     payload_offset;
    // qos
    bit<5>      src_tm_iq;
    bit<3>      pad_1;
    // flow hash
    bit<32>     flow_hash;
}

header   p4_to_p4plus_cpu_tcp_pkt_h {
    bit<8>     tcp_flags;
    bit<32>    tcp_seqNo;
    bit<32>    tcp_ackNo;
    bit<16>    tcp_window;
    bit<8>     tcp_options;
    bit<16>    tcp_mss;
    bit<8>     tcp_ws;
}

header p4_to_p4plus_mirror_h {
    bit<4>    p4plus_app_id;
    bit<1>    table0_valid;
    bit<1>    table1_valid;
    bit<1>    table2_valid;
    bit<1>    table3_valid;
    bit<16>   payload_len;

    bit<8>    session_id;
    bit<8>    direction;
    bit<16>   lif;
    bit<64>   timestamp;
    bit<16>   capture_len;
    bit<16>   original_len;
}

/*
 * flag bits:
 *  0 : update ip id
 *  1 : update ip len
 *  2 : update tcp seq number
 *  3 : update udp len
 *  4 : insert vlan header
 *  5 : compute outer checksums
 *  6 : compute inner checksums
 *  7 : lkp_inst
 */
header    p4plus_to_p4_header_h {
    bit<4>    p4plus_app_id;
    bit<1>    table0_valid;
    bit<1>    table1_valid;
    bit<1>    table2_valid;
    bit<1>    table3_valid;
    bit<24>   flow_index;
    bit<8>    flags;
    bit<8>    udp_opt_bytes;
    bit<11>   dst_lport;
    bit<1>    dst_lport_valid;
    bit<1>    pad1;
    bit<1>    tso_last_segment;
    bit<1>    tso_first_segment;
    bit<1>    tso_valid;
    bit<16>   ip_id_delta;
    bit<32>   tcp_seq_delta;
    bit<14>   gso_start;
    bit<1>    compute_inner_ip_csum;
    bit<1>    compute_ip_csum;
    bit<14>   gso_offset;
    bit<1>    flow_index_valid;
    bit<1>    gso_valid;
    bit<16>   vlan_tag;
}
header  p4plus_to_p4_s1_h {
    bit<4>    p4plus_app_id;
    bit<4>    pad;
    bit<24>   flow_index;
    bit<1>    lkp_inst;
    bit<1>    compute_inner_l4_csum;
    bit<1>    compute_l4_csum;
    bit<1>    insert_vlan_tag;
    bit<1>    update_udp_len;
    bit<1>    update_tcp_seq_no;
    bit<1>    update_ip_len;
    bit<1>    update_ip_id;
    bit<8>    udp_opt_bytes;
    bit<11>   dst_lport;
    bit<1>    dst_lport_valid;
    bit<1>    pad1;
    bit<1>    tso_last_segment;
    bit<1>    tso_first_segment;
    bit<1>    tso_valid;
    bit<16>   ip_id_delta;
    bit<32>   tcp_seq_delta;
    bit<14>   gso_start;
    bit<1>    compute_inner_ip_csum;
    bit<1>    compute_ip_csum;
    bit<14>   gso_offset;
    bit<1>    flow_index_valid;
    bit<1>    gso_valid;
}

header  p4plus_to_p4_s2_h {
    bit<3>    pcp;
    bit<1>    dei;
    bit<12>   vid;
}

header_union p4_to_p4plus_uh {
    p4_to_p4plus_ipsec_header_h             p4_to_p4plus_ipsec;
    p4_to_p4plus_p4pt_header_h              p4_to_p4plus_p4pt;
    p4_to_p4plus_mirror_h                   p4_to_p4plus_mirror;
    p4_to_p4plus_cpu_pkt_h                  p4_to_p4plus_cpu_pkt;
}
