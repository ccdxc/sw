f_p4plus_to_p4_1:
  seq         c1, k.p4plus_to_p4_valid, TRUE
  nop.!c1.e

  // update IP id
  seq         c2, k.p4plus_to_p4_update_ip_id, TRUE
  add         r1, k.ipv4_identification, k.p4plus_to_p4_ip_id_delta
  phvwr.c2    p.ipv4_identification, r1

  // update IP length
  bbne        k.p4plus_to_p4_update_ip_len, TRUE, p4plus_to_p4_1_upd_tcp_seq
  seq         c2, k.vlan_tag_valid, TRUE
  cmov        r1, c2, 18, 14
  sub         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1
  seq         c3, k.ipv4_valid, TRUE
  sub.c3      r2, r1, k.ipv4_ihl, 2
  phvwr.c3    p.ipv4_totalLen, r1
  seq         c4, k.ipv6_valid, TRUE
  sub.c4      r2, r1, 40
  phvwr.c4    p.ipv6_payloadLen, r2

  // update tcp_data_len
  sub         r2, r2, k.tcp_dataOffset, 2
  phvwr       p.l4_metadata_tcp_data_len, r2

p4plus_to_p4_1_upd_tcp_seq:
  // update TCP sequence number
  seq         c2, k.p4plus_to_p4_update_tcp_seq_no, TRUE
  add         r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, \
                  k.p4plus_to_p4_tcp_seq_delta
  phvwr.c2    p.tcp_seqNo, r1

  // update from CPU flag
  seq         c2, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
  phvwr.c2    p.control_metadata_from_cpu, TRUE

  // Update the flow_index given by ARM CPU 
  // In this case we will skip the flow_hash lookup
  seq         c2, k.p4plus_to_p4_flow_index_valid, TRUE
  phvwr.c2    p.flow_info_metadata_flow_index, k.p4plus_to_p4_flow_index
  
  // insert vlan tag
  seq         c2, k.p4plus_to_p4_insert_vlan_tag, TRUE
  nop.!c2.e
  add         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                     capri_p4_intrinsic_packet_len_sbit6_ebit13}, 4
  phvwr       p.vlan_tag_valid, TRUE
  phvwrpair   p.{vlan_tag_pcp...vlan_tag_vid}, \
                k.{p4plus_to_p4_vlan_pcp...p4plus_to_p4_vlan_vid_sbit4_ebit11}, \
                p.vlan_tag_etherType, k.ethernet_etherType
  phvwr.e     p.ethernet_etherType, ETHERTYPE_VLAN
  phvwr       p.capri_p4_intrinsic_packet_len, r1
