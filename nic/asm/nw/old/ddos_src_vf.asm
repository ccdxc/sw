/******************************************************************************/
TABLE : ddos_src_vf
/******************************************************************************/
struct k {


  // Total size : 
  standard_ingress_port : 10;
  tcp_flags : 8;
  
  
}

ACTION : ddos_src_vf.flow_hit:
struct d {

  ddos_type_en : 8; 
  ddos_type_notify : 8;
  ddos_type_drop : 8;
  ddos_type_current_state : 8;
  tcp_syn_thresh : 20;
  tcp_syn_ack_thresh : 20;
  tcp_rst_thresh : 20;
  tcp_fin_thresh : 20;
  icmp_thresh : 20;
  udp_thresh : 20;
  ip_frag_thresh : 20;
  any_thresh : 20;
  tcp_syn_cnt : 24;
  tcp_syn_ack_cnt : 24;
  tcp_rst_cnt : 24;
  tcp_fin_cnt : 24;
  icmp_cnt : 24;
  udp_cnt : 24;
  ip_frag_cnt : 24;
  any_cnt : 24;
}

  // Frag Handling

lb_ddos_type_not_frag:
  // SYN handling

smeqb c1 d.ddos_type_en, DDOS_TYPE_SYN, DDOS_TYPE_SYN
smeqb c2 k.tcp_flags, TCP_FLAGS_SYN_ACK, TCP_FLAGS_SYN
bcf [!c1 | !c2] lb_ddos_type_not_syn
lb_ddos_type_syn:
  smeqb c3 d.ddos_type_current_state, DDOS_TYPE_SYN, 0
  bcf [!c3] lb_ddos_tcp_syn_thresh_hit
  nop
    tbladd d.tcp_syn_cnt, -1
    nop // Since i want to access the updated d.tcp_syn_cnt 
    seq c4 d.tcp_syn_cnt, r0
    tblor.c4 ddos_type_current_state, DDOS_TYPE_SYN
    smeqb c5 d.ddos_type_notify, DDOS_TYPE_SYN, DDOS_TYPE_SYN
    setcf c6 [c4 & c5]
    // Set metadata based on c6 to send the digest to CPU
    // Need to see if we can do it from Egress stage.
    nop.e
    nop
  lb_ddos_tcp_syn_thresh_hit:
    tbladd d.tcp_syn_cnt, 1
    smeqb.e c4 d.ddos_type_drop, DDOS_TYPE_SYN, DDOS_TYPE_SYN
    // On egress it might not be control_metadata for drop.
    phvwr.c4 control_metadata.ddos_drop, 1

lb_ddos_type_not_syn:
  // SYN + ACK handling

lb_ddos_type_not_syn_ack:
  // RST  handling

lb_ddos_type_not_rst:
  // FIN  handling

lb_ddos_type_not_fin:
  // ICMP  handling

lb_ddos_type_not_icmp:
  // UDP  handling

lb_ddos_type_not_udp:
lb_ddos_type_any:
  // Every Packet handling


