/******************************************************************************/
TABLE : l4_global_config
/******************************************************************************/
struct k {
  // table key : 0 bits
}

ACTION : l4_global_config.l4_global_properties:
struct d{
  icmp_src_quench_msg_action : 1;
  icmp_redirect_msg_action : 1;
  icmp_info_req_msg_action : 1;
  icmp_addr_req_msg_action : 1;
  icmp_traceroute_msg_action : 1;
  icmp_rsvd_type_msg_action : 1;
}
phvwr.e phv[icmp_src_quench_msg_action...rsvd_type_msg_action], d[icmp_src_quench_msg_action...icmp_rsvd_type_msg_action]
nop
