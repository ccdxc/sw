#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

%%

f_p4plus_to_p4_2:
  // update IP length
  seq         c2, k.vlan_tag_valid, TRUE
  cmov        r1, c2, 18, 14
  sub         r1, k.control_metadata_packet_len, r1
  seq         c3, k.ipv4_valid, TRUE
  sub.c3      r2, r1, k.ipv4_ihl, 2
  seq         c4, k.ipv6_valid, TRUE
  sub.c4      r2, r1, 40
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN
  andcf       c3, [c2]
  andcf       c4, [c2]
  phvwr.c3    p.ipv4_totalLen, r1
  phvwr.c4    p.ipv6_payloadLen, r2

  // update UDP length
  smeqb       c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                  P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
  phvwr.c2    p.udp_len, r2

  // remove the headers
  phvwr.e     p.capri_txdma_intrinsic_valid, FALSE
  phvwr       p.p4plus_to_p4_valid, FALSE
