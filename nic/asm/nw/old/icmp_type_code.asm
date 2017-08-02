/******************************************************************************/
TABLE : icmp_type_code
/******************************************************************************/
struct k {
  // table key : 16 bits
  phv.lkp_sport : 16;

  // Other fields that are needed in action routines.
  phv.l4_icmp_src_quench_msg_action :1;
  phv.l4_icmp_redirect_msg_action :1;
  phv.l4_icmp_info_req_msg_action :1;
  phv.l4_icmp_addr_req_msg_action :1;
  phv.l4_icmp_traceroute_msg_action :1;
  phv.l4_icmp_rsvd_type_msg_action :1;
}

ACTION : icmp_type_code.validate_icmp_src_quench_msg:
struct d {
}
seq.e c1 k.l4_icmp_src_quench_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_src_quench_msg_drop, TRUE

ACTION : icmp_type_code.validate_icmp_redirect_msg:
struct d {
}
seq.e c1 k.l4_icmp_redirect_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_redirect_msg_drop, TRUE

ACTION : icmp_type_code.validate_icmp_info_req_msg:
struct d {
}
seq.e c1 k.l4_icmp_info_req_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_info_req_msg_drop, TRUE

ACTION : icmp_type_code.validate_icmp_addr_req_msg:
struct d {
}
seq.e c1 k.l4_icmp_addr_req_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_addr_req_msg_drop, TRUE

ACTION : icmp_type_code.validate_icmp_traceroute_msg:
struct d {
}
seq.e c1 k.l4_icmp_traceroute_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_traceroute_msg_drop, TRUE

ACTION : icmp_type_code.validate_icmp_rsvd_type_msg:
struct d {
}
seq.e c1 k.l4_icmp_rsvd_type_msg_action, ACT_DROP
phvwr.c1 phv.control_icmp_rsvd_type_msg_drop, TRUE

