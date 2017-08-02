/******************************************************************************/
TABLE : copp_policer
/******************************************************************************/
struct k {
  phv.copp_policer_index : 5;
  phv.standard_packet_length : 16;
}

ACTION copp_policer.execute_copp_policer:
struct d {
  policer_color : 2;
  permit_packets : 32;
  permit_bytes : 40;
  deny_packets : 32;
  deny_bytes : 40;
}

seq c1, d.policer_color, PACKET_COLOR_RED
br d.policer_color, 4, permit_stats
phvwr.c1 phv.mark_for_drop, TRUE

permit_stats:
  tbwr.e d.permit_packets, 1
  tbwr d.permit_bytes, k.standard_packet_length
deny_stats:
  tbwr.e d.deny_packets, 1
  tbwr d.deny_bytes, k.standard_packet_length
