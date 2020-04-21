#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_p4plus_to_p4_2_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct p4plus_to_p4_2_k_ k;
struct phv_ p;

%%

f_p4plus_to_p4_2:
  // copy gso_valid to capri_gso_csum.gso field
  phvwr         p.capri_gso_csum_gso, k.p4plus_to_p4_gso_valid

  // update compute checksum flags (classic nic)
  seq           c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
  seq.!c1       c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_CPU
  bcf           [!c1], f_p4plus_to_p4_2_other_apps
  crestore      [c2-c1], k.{p4plus_to_p4_compute_inner_l4_csum,\
                            p4plus_to_p4_compute_l4_csum}, 0x3
  phvwr.c2      p.control_metadata_checksum_ctl[CHECKSUM_CTL_INNER_L4_CHECKSUM], TRUE
  phvwr.c1      p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE
  crestore      [c2-c1], k.{p4plus_to_p4_compute_inner_ip_csum,\
                            p4plus_to_p4_compute_ip_csum}, 0x3
  phvwr.c2      p.control_metadata_checksum_ctl[CHECKSUM_CTL_INNER_IP_CHECKSUM], TRUE
  phvwr.c1      p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

  // remove the headers
  phvwr.e       p.capri_txdma_intrinsic_valid, FALSE
  phvwrpair     p.p4plus_to_p4_vlan_valid, FALSE, p.p4plus_to_p4_valid, FALSE

f_p4plus_to_p4_2_other_apps:
  // compute IP header checksum if ipv4.valid is true
  seq           c1, k.ipv4_valid, TRUE
  phvwr.c1      p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

  // set compute_icrc flag (rdma)
  seq           c1, k.p4plus_to_p4_p4plus_app_id, P4PLUS_APPTYPE_RDMA
  phvwr.c1      p.control_metadata_checksum_ctl[CHECKSUM_CTL_ICRC], TRUE
  seq           c2, k.udp_valid, TRUE
  seq           c3, k.tcp_valid, TRUE
  seq           c4, k.icmp_valid, TRUE
  setcf.!c1     c1, [!c2 & !c3 & !c4]
  phvwr.!c1     p.control_metadata_checksum_ctl[CHECKSUM_CTL_L4_CHECKSUM], TRUE

  // remove the headers
  phvwr.e       p.capri_txdma_intrinsic_valid, FALSE
  phvwrpair     p.p4plus_to_p4_vlan_valid, FALSE, p.p4plus_to_p4_valid, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_to_p4_2_error:
  nop.e
  nop
