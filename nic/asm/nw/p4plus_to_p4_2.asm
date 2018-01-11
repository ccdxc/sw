#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

%%

f_p4plus_to_p4_2:
  // update UDP length
  seq           c3, k.ipv4_valid, TRUE
  sub.c3        r2, k.ipv4_totalLen, k.ipv4_ihl, 2
  seq           c4, k.ipv6_valid, TRUE
  sub.c4        r2, k.ipv6_payloadLen, r0
  sub           r2, r2, k.p4plus_to_p4_udp_opt_bytes
  smeqb         c2, k.p4plus_to_p4_flags, P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN, \
                    P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN
  phvwr.c2      p.udp_len, r2
  phvwr         p.control_metadata_udp_opt_bytes, k.p4plus_to_p4_udp_opt_bytes

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
  // compute IP header checksum if ipv4.valid is true
  phvwr.c3      p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

  // set compute_icrc flag (rdma)
  seq           c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
  phvwr.c1      p.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  seq           c2, k.udp_valid, TRUE
  seq           c3, k.tcp_valid, TRUE
  setcf.!c1     c1, [!c2 & !c3]
  phvwr.!c1     p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE

  // remove the headers
  phvwr.e       p.capri_txdma_intrinsic_valid, FALSE
  phvwr         p.p4plus_to_p4_valid, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_to_p4_2_error:
  nop.e
  nop
