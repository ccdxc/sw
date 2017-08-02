/******************************************************************************/
TABLE : ingress_policer_action
/******************************************************************************/
struct k {
  phv.ingress_policer_index : 10;
  phv.ingress_policer_color : 2;
  phv.standard_packet_length : 16;
}

policer_stats:
  tbladd.e d.packets, 1
  tbladd d.bytes, k.standard_packet_length

ACTION ingress_policer_action.ingress_policer_deny:
struct d {
}
b policer_stats
phvwr phv.ingress_policer_drop, TRUE

ACTION ingress_policer_permit.ingress_policer_permit:
struct d {
  cos_en : 1;
  cos : 3;
  dscp_en : 1;
  dscp : 8;
  packets : 32;
  bytes : 40;
}
b policer_stats
phvwr phv[qos_cos_en, qos_cos, qos_dscp_en, qos_dscp], d[cos_en, cos, dscp_en, dscp]
