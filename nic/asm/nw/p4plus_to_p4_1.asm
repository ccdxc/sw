#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_1_k k;
struct phv_             p;

%%

f_p4plus_to_p4_1:
  // update IP id
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID
  add         r1, k.ipv4_identification, k.p4plus_to_p4_ip_id_delta
  phvwr.c2    p.ipv4_identification, r1

  // update IP length
  bbne        k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN_BIT_POS], 1, \
                p4plus_to_p4_1_upd_tcp_seq
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
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO
  add         r1, k.{tcp_seqNo_sbit0_ebit15,tcp_seqNo_sbit16_ebit31}, \
                  k.p4plus_to_p4_tcp_seq_delta
  phvwr.c2    p.tcp_seqNo, r1

  // update from CPU flag
  seq         c2, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
  phvwr.c2    p.control_metadata_from_cpu, TRUE

  // insert vlan tag
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG, \
                  P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG
  nop.!c2.e
  add         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                     capri_p4_intrinsic_packet_len_sbit6_ebit13}, 4
  phvwr       p.vlan_tag_valid, TRUE
  phvwr       p.{vlan_tag_pcp...vlan_tag_vid}, k.p4plus_to_p4_vlan_tag
  phvwr       p.vlan_tag_etherType, k.ethernet_etherType
  phvwr.e     p.ethernet_etherType, ETHERTYPE_VLAN
  phvwr       p.capri_p4_intrinsic_packet_len, r1
