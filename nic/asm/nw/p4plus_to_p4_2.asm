#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

%%

f_p4plus_to_p4_2:
  // update IP length
  seq           c2, k.vlan_tag_valid, TRUE
  cmov          r1, c2, 18, 14
  sub           r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                       capri_p4_intrinsic_packet_len_sbit6_ebit13}, r1
  seq           c3, k.ipv4_valid, TRUE
  sub.c3        r2, r1, k.ipv4_ihl, 2
  seq           c4, k.ipv6_valid, TRUE
  sub.c4        r2, r1, 40
  smeqb         c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN, \
                    P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN
  andcf         c3, [c2]
  andcf         c4, [c2]
  phvwr.c3      p.ipv4_totalLen, r1
  phvwr.c4      p.ipv6_payloadLen, r2

  // update UDP length
  smeqb         c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                    P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
  phvwr.c2      p.udp_len, r2

  // update compute checksum flags (classic nic)
  seq           c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
  bcf           [!c1], f_p4plus_to_p4_2_other_apps
  phvwrpair.c1  p.control_metadata_checksum_ctl[CHECKSUM_CTL_INNER_L4_CHECKSUM], \
                    k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_COMPUTE_INNER_L4_CSUM_BIT_POS], \
                    p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], \
                    k.p4plus_to_p4_flags[P4PLUS_TO_P4_FLAGS_COMPUTE_L4_CSUM_BIT_POS]

  // remove the headers
  phvwr.e       p.capri_txdma_intrinsic_valid, FALSE
  phvwr         p.p4plus_to_p4_valid, FALSE

f_p4plus_to_p4_2_other_apps:
  // update compute checksum flags (all other apps)
  // set compute_icrc flag (rdma)
  seq           c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
  phvwr.c1      p.control_metadata_checksum_ctl, (1 << CHECKSUM_CTL_IP_CHECKSUM)
  //phvwr.c1      p.control_metadata_checksum_ctl, \
  //                  ((1 << CHECKSUM_CTL_IP_CHECKSUM) | (1 << CHECKSUM_CTL_ICRC))

  phvwr.!c1     p.control_metadata_checksum_ctl, \
                    ((1 << CHECKSUM_CTL_IP_CHECKSUM) | (1 << CHECKSUM_CTL_L4_CHECKSUM))

  // remove the headers
  phvwr.e       p.capri_txdma_intrinsic_valid, FALSE
  phvwr         p.p4plus_to_p4_valid, FALSE
